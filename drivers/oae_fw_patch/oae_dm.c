/*
 * Copyright (C) 2017 Qihoo, Inc.
 */
#include <linux/blkdev.h>
#include <linux/device-mapper.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/mount.h>
#include <linux/string.h>

#include "oae_dm.h"

#define OAE_DM_MAX_NAME_LEN 64
#define OAE_ROOT_NAME_LEN 64
static char saved_patch_name[OAE_DM_MAX_NAME_LEN] __initdata;
static dev_t PATCH_DEV __initdata;
static dev_t VPATCH_DEV __initdata;
static int oae_dm_result __initdata;

static int __init patch_dev_setup(char *line)
{
	OAE_DM_INFO("oae_dm: patch_dev_setup:%s\n", line);
	strlcpy(saved_patch_name, line, sizeof(saved_patch_name));
	return 1;
}

__setup("patch=", patch_dev_setup);

static void __init oae_dm_get_fs_names(char *page)
{
	char *s = page;
	int len;
	char *p = NULL;
	char *next = NULL;

	len = get_filesystem_list(page);
	page[len] = '\0';
	for (p = page - 1; p; p = next) {
		next = strchr(++p, '\n');
		if (*p++ != '\t')
			continue;
		while ((*s++ = *p++) != '\n')
			;
		s[-1] = '\0';
	}
	*s = '\0';
}

int __init oae_dm_mount(const char *name, const char *mnt_point, int flags,
			const char *options)
{
	int rc;
	char *fs_names = NULL;
	char *p = NULL;
	struct page *page = NULL;

	if (!name || !mnt_point) {
		OAE_DM_ERROR("oae_dm: input parameter is NULL\n");
		return -1;
	}

	page = alloc_page(GFP_KERNEL | __GFP_NOTRACK_FALSE_POSITIVE);

	if (!page) {
		OAE_DM_ERROR("oae_dm: alloc_page  err\n");
		return -1;
	}
	fs_names = page_address(page);
	if (!fs_names) {
		OAE_DM_ERROR("oae_dm: page_address get err\n");
		rc = -1;
		goto out;
	}

	oae_dm_get_fs_names(fs_names);
retry:
	for (p = fs_names; *p; p += strlen(p) + 1) {
		if (p >= (fs_names + PAGE_SIZE)) {
			OAE_DM_ERROR("oae_dm: mount %s err\n", name);
			rc = -1;
			goto out;
		}
		rc = sys_mount((char __user __force *)name,
			       (char __user __force *)mnt_point, p, flags,
			       (char __user __force *)options);
		switch (rc) {
		case 0:
			goto out;
		case -EACCES:
			flags |= MS_RDONLY;
			goto retry;
		case -EINVAL:
			continue;
		}
	}
out:
	put_page(page);
	return rc;
}

static int __init mount_patch(const char *mnt_point, dev_t dev)
{
	int rc;
	char patch_dev_name[OAE_DM_MAX_NAME_LEN] = {0};
	struct block_device *bdev = NULL;
	char name_buf[BDEVNAME_SIZE] = {0};

	int flags = MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_NOATIME |
		    MS_NODIRATIME | MS_SILENT | MS_RDONLY;

	sys_mkdir("/dev/block", 0755);

	bdev = blkdev_get_by_dev(dev, FMODE_READ, NULL);
	if (IS_ERR_OR_NULL(bdev))
		return 1;
	snprintf(patch_dev_name, sizeof(patch_dev_name) - 1, "/dev/block/%s",
		 bdevname(bdev, name_buf));
	blkdev_put(bdev, FMODE_READ);

	rc = oae_create_dev(patch_dev_name, dev); // not del dev
	if (rc) {
		OAE_DM_ERROR("oae_dm: oae_dm: mkdir %s fail %d\n", mnt_point,
			     rc);
		rc = 2;
		goto out;
	}

	rc = sys_mkdir(mnt_point, 0751);
	if (rc) {
		OAE_DM_ERROR("oae_dm: oae_dm: mkdir %s fail %d\n", mnt_point,
			     rc);
		rc = 3;
		goto out;
	}

	rc = oae_dm_mount(patch_dev_name, (char __user __force *)mnt_point,
			  flags, NULL);
	if (rc) {
		OAE_DM_WARRING("oae_dm: mount %s on %s fail %d\n",
			       patch_dev_name, mnt_point, rc);
		rc = 4;
	}
out:
	return rc;
}

static void __init patch_cleanup(const char *mnt_point)
{
	char path[OAE_DM_MAX_NAME_LEN] = {0};

	strlcpy(path, mnt_point + 1, sizeof(path));
	if (sys_mount((char __user __force *)mnt_point, path, NULL, MS_MOVE,
		      NULL))
		OAE_DM_ERROR("oae_dm: oae_dm: move path fail\n");
	log_partition_umount();
}

dev_t __init begin_oae_dm(dev_t orginal_dev, char *saved_root_name,
			  char *root_device_name)
{
	int err;
	dev_t dev = orginal_dev;

	oae_dm_result = 0;

	if (!saved_root_name) {
		OAE_DM_ERROR("begin_oae_dm para error\n");
		return dev;
	}

	VPATCH_DEV = patch_run_setup();
	PATCH_DEV = name_to_dev_t(saved_patch_name);
	OAE_DM_INFO("oae_dm: PATCH_DEV=%d\n", PATCH_DEV);

	do {
		if (!PATCH_DEV) {
			OAE_DM_INFO("oae_dm: patch cmdline params err\n");
			oae_dm_save_log(OAE_DM_PATCH_CMDLINE_ERR, 0);
			break;
		}

		if (VPATCH_DEV == 0) {
			OAE_DM_INFO("oae_dm: patch avb err\n");
			break;
		} else if (VPATCH_DEV == -1) {
			OAE_DM_INFO("oae_dm: patch avb not exist\n");
		}

		err = mount_patch("/" OAE_DM_PATCH_ROOT, PATCH_DEV);
		if (err) {
			OAE_DM_INFO("oae_dm: patch cmdline params err\n");
			oae_dm_save_log(OAE_DM_PATCH_MOUNT_ERR, err);
			break;
		}
#ifdef CONFIG_OAE_DM_ROOT
		dev = oae_dm_setup_root(orginal_dev, "/" OAE_DM_PATCH_ROOT);
		if (dev) {
			err = snprintf(saved_root_name, OAE_ROOT_NAME_LEN - 1,
				       "/dev/dm-%d", MINOR(dev));
			if (err < 0)
				OAE_DM_ERROR("oae_dm: snprintf root err\n");
			 // 5 means "/dev/" length
			root_device_name = saved_root_name + 5;
			oae_dm_result = 1;
		} else {
			dev = orginal_dev;
		}
#endif // CONFIG_OAE_DM_ROOT
	} while (0);
	return dev;
}

void __init end_oae_dm(void)
{
	oae_dm_cleanup("/" OAE_DM_PATCH_ROOT, oae_dm_result);
	if (PATCH_DEV && PATCH_DEV != -1)
		patch_cleanup("/" OAE_DM_PATCH_ROOT);
}

