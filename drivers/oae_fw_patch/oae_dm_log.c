/*
 * Copyright (C) 2017 Qihoo, Inc.
 */
#include <linux/fs.h>
#include <linux/string.h>

#include "oae_dm.h"
#ifdef CONFIG_HUAWEI_STORAGE_ROFA
#include <chipset_common/storage_rofa/storage_rofa.h>
#endif

static int log_partion_fail __initdata = -1;
#define LOG_FILE "last_hota_fwkcold"
#define LOG_MNT_DIR "/oae_log"
#define OAE_LOG_LEN 64
#define OAE_DEV_LEN 64
#define OAE_LOG_SUCCESS "HOTA_SUCCESS:0,0"
static int __init cache_setup(char *str);
static char CACHE_DEV[OAE_DEV_LEN] __initdata = {0};
static char LOG_DIR[OAE_LOG_LEN] __initdata = {0};

static int __init log_partition_prepair(void)
{
	int rc;
	int flags = MS_NOSUID | MS_NODEV | MS_NOATIME;
	char *cache_dev_name = "/dev/cache";
	dev_t dev;

#ifdef CONFIG_HUAWEI_STORAGE_ROFA
	// ignore mount cache partition and output log
	if (get_storage_rofa_bootopt() == STORAGE_ROFA_BOOTOPT_BYPASS) {
		OAE_DM_WARRING("oae_dm: storage_rofa_bootopt bypass\n");
		return -1;
	}
#endif

	if (!log_partion_fail)
		return 0;
	dev = name_to_dev_t(CACHE_DEV);
	if (!dev)
		return -1;

	rc = oae_create_dev(cache_dev_name, dev);
	if (rc) {
		OAE_DM_ERROR("oae_dm: create log dev fail %d\n", rc);
		return -1;
	}
	rc = sys_mkdir(LOG_MNT_DIR, 0751);
	if (rc) {
		OAE_DM_ERROR("oae_dm: create log mount point fail%d\n", rc);
		return -2;
	}

	rc = oae_dm_mount(cache_dev_name, LOG_MNT_DIR, flags, "data=ordered");
	if (rc) {
		OAE_DM_ERROR("oae_dm: mount log partition fail %d\n", rc);
		return -3;
	}
	log_partion_fail = 0;
	return 0;
}

void __init log_partition_umount(void)
{
	if (!log_partion_fail) {
		int rc = sys_umount(LOG_MNT_DIR, MNT_DETACH);

		if (rc) {
			OAE_DM_ERROR("oae_dm: umount error %d\n", rc);
		} else {
			OAE_DM_INFO("oae_dm: umount okay\n");
			sys_unlink("/dev/cache");
			sys_rmdir(LOG_MNT_DIR);
		}
	}
}

void __init oae_dm_save_log(int major_code, int minor_code)
{
	mm_segment_t old_fs;
	loff_t offset = 0;
	struct file *filp = NULL;
	char data[64] = {0};
	char log_file[64] = {0};
	char log_dir[64] = {0};
	int flags = O_RDWR | O_TRUNC;

	if (0 == major_code && 0 == minor_code)
		snprintf(data, sizeof(data) - 1, "HOTA_SUCCESS:%d,%d",
			major_code, minor_code);
	else
		snprintf(data, sizeof(data) - 1, "HOTA_ERR_STACK:%d,%d",
			major_code, minor_code);

	if (log_partition_prepair())
		return;
	old_fs = get_fs();
	set_fs(get_ds());
	OAE_DM_INFO("oae_dm: oae_dm_save_log %d:%d\n", major_code, minor_code);

	snprintf(log_file, sizeof(log_file) - 1, "%s/%s/%s",
		LOG_MNT_DIR, LOG_DIR, LOG_FILE);
	snprintf(log_dir, sizeof(log_dir) - 1, "%s/%s", LOG_MNT_DIR, LOG_DIR);

	if (sys_access(log_file, 0)) {
		flags = O_WRONLY | O_CREAT;
		sys_mkdir(log_dir, 0755);
	}
	filp = filp_open(log_file, flags, 0644);

	if (IS_ERR(filp)) {
		OAE_DM_WARRING("oae_dm: open log file %s %ld\n", log_file,
			       PTR_ERR(filp));
		goto out;
	}

	if (vfs_write(filp, data, strlen(data), &offset) != strlen(data)) {
		OAE_DM_WARRING("oae_dm: save log data fail\n");
		goto out;
	}

out:
	if (!IS_ERR_OR_NULL(filp))
		filp_close(filp, NULL);

	sys_sync();
	set_fs(old_fs);
}

int __init get_last_result(void)
{
	mm_segment_t old_fs;
	char log_file[OAE_LOG_LEN] = {0};
	char log_data[OAE_LOG_LEN] = {0};
	struct file *filp = NULL;
	loff_t offset = 0;
	int rc = 1;

	if (log_partition_prepair())
		return 2;
	snprintf(log_file, sizeof(log_file) - 1, "%s/%s/%s",
		 LOG_MNT_DIR, LOG_DIR, LOG_FILE);

	old_fs = get_fs();
	set_fs(get_ds());
	if (sys_access(log_file, 0) != 0) {
		OAE_DM_WARRING("oae_dm: result file not exist\n");
		rc = 3;
		goto out;
	}

	filp = filp_open(log_file, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		OAE_DM_WARRING("oae_dm: open file %s fail\n", log_file);
		rc = 4;
		goto out;
	}

	vfs_read(filp, log_data, OAE_LOG_LEN - 1, &offset);
	if (strcmp(log_data, OAE_LOG_SUCCESS) == 0) {
		OAE_DM_WARRING("oae_dm: get %s result success\n", log_file);
		rc = 0;
	}
	OAE_DM_WARRING("oae_dm: last result %s\n", log_data);

out:
	if (!IS_ERR_OR_NULL(filp))
		filp_close(filp, NULL);
	set_fs(old_fs);
	return rc;
}

static int __init cache_setup(char *str)
{
	char *endp = NULL;
	size_t len;

	if (!str) {
		OAE_DM_ERROR("oae_dm: str is NULL\n");
		return 1;
	}

	len = strlen(str);
	OAE_DM_INFO("oae_dm: cache str is %s\n", str);

	if (strncmp(str, "PARTUUID=", 9) == 0) { // 9 means PARTUUID= length
		endp = strpbrk(str, "/");
		if (!endp || len <= (endp - str)) {
			CACHE_DEV[0] = '\0';
			return 1;
		}

		*endp = '\0';
		strlcpy(CACHE_DEV, str, sizeof(CACHE_DEV));
		OAE_DM_INFO("oae_dm: cache dev is %s\n", str);
		endp++;
		strlcpy(LOG_DIR, endp, sizeof(LOG_DIR));
	} else if (strncmp(str, "/dev/", 5) == 0) { // 5 means /dev/ length
		endp = strpbrk(str + 5, "/");
		if (!endp || len <= (endp - str)) {
			CACHE_DEV[0] = '\0';
			return 2;
		}
		*endp = '\0';
		strlcpy(CACHE_DEV, str, sizeof(CACHE_DEV));
		OAE_DM_INFO("oae_dm: cache dev is %s\n", str);
		endp++;
		strlcpy(LOG_DIR, endp, sizeof(LOG_DIR));
	} else {
		CACHE_DEV[0] = '\0';
		return 3;
	}
	log_partion_fail = -1;
	OAE_DM_INFO("oae_dm: cache dir is %s\n", LOG_DIR);
	return 0;
}
__setup("cache=", cache_setup);
