/*
 * Copyright (C) 2017 Qihoo, Inc.
 */
#include <linux/blkdev.h>
#include <linux/device-mapper.h>
#include <linux/fs.h>
#include <linux/string.h>

#include <crypto/hash.h>
#include <crypto/hash_info.h>
#include <crypto/sha.h>
#include <linux/crypto.h>
#include <linux/version.h>
#include <linux/verification.h>

#include "../drivers/block/loop.h"
#include <linux/miscdevice.h>
#include "../drivers/md/dm.h"
#include "oae_dm.h"
#include "mntn_public_interface.h"
#include "hisi_bootup_keypoint.h"

#ifndef OAE_DM_SNAPSHOT_DEF
#define OAE_DM_SNAPSHOT_DEF

#define OAE_DM_SNAP_TYPE "snapshot_read"
#define OAE_DM_PATCH_DIR "coldpatch"
#define OAE_DM_SQUASH_MNT "oae_sfs"
#define OAE_DM_DEF_LOOP OAE_DM_PATCH_DIR "/system.loop"
#define OAE_DM_SQUASH_FILE OAE_DM_PATCH_DIR "/system.sfs"
#define OAE_DM_SQUASH_LOOP OAE_DM_SQUASH_MNT "/system.loop"
#define OAE_DM_CONFIG_FILE OAE_DM_PATCH_DIR "/system.cfg"
#define OAE_DM_LOOP_PART_SHIFT 3
#define OAE_DM_LOOP_DEF_POS 0
#define OAE_DM_LOOP_SFS_POS 1
#define OAE_DM_LOOP_MAX 2
#define OAE_DM_TRUNK_MUTI 8
#define OAE_DM_HASH_BUFFER_SIZE 65536

#define OAE_DM_LOOP_FILE_HASH "sha256"

#define OAE_DM_CONFIG_FILE_MAGIC "!#oaeplf"
#define OAE_DM_CONFIG_FILE_MAGIC_SIZE 8
#define OAE_DM_CONFIG_FILE_VERSION 3
#define OAE_DM_CONFIG_FILE_HEAD_SIZE \
	(OAE_DM_CONFIG_FILE_MAGIC_SIZE + 2 * sizeof(int))
// the max size of cfg file
#define OAE_DM_CONFIG_FILE_MAX_SIZE 4096
// the length of sha256 hash is 32 bytes
#define OAE_DM_SHA256_SIZE 32
#define OAE_DM_ROLLBACK_ERR 30
#define OAE_DM_ROOT_HASH_MAX_LEN 2048
#define OAE_DM_ROOT_HASH_MIN_LEN 20
#define OAE_DM_SIGN_MIN_LEN 4
#define OAE_DM_MAX_LOOP_ID 20
#define OAE_DM_PATCH_FILE_LEN 64
#endif // OAE_DM_SNAPSHOT_DEF

static int __init file_hash(const char *file,
			    unsigned char **hash)
{
	int rc;
	size_t digest_size;
	struct shash_desc *desc = NULL;
	struct crypto_shash *tfm = NULL;
	loff_t offset = 0;
	unsigned char *data = NULL;

	mm_segment_t old_fs;
	int size;
	struct file *filp = NULL;

	old_fs = get_fs();
	set_fs(get_ds());

	data = kzalloc(OAE_DM_HASH_BUFFER_SIZE, GFP_KERNEL);
	if (!data) {
		OAE_DM_ERROR("oae_dm: alloc data fail\n");
		rc = -10;
		goto out;
	}
	tfm = crypto_alloc_shash(OAE_DM_LOOP_FILE_HASH, 0, 0);
	digest_size = crypto_shash_digestsize(tfm);
	if (digest_size != SHA256_DIGEST_SIZE) {
		OAE_DM_ERROR("oae_dm: digest_size %zu error\n", digest_size);
		rc = -11;
		goto out;
	}
	rc = digest_size;

	desc = kzalloc(sizeof(*desc) + crypto_shash_descsize(tfm), GFP_KERNEL);
	if (!desc) {
		OAE_DM_ERROR("oae_dm: alloc shash desc fail\n");
		rc = -12;
		goto out;
	}
	desc->tfm = tfm;
	desc->flags = CRYPTO_TFM_REQ_MAY_SLEEP;
	crypto_shash_init(desc);

	filp = filp_open(file, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		OAE_DM_ERROR("oae_dm: open %s file %ld\n", file, PTR_ERR(filp));
		rc = -13;
		goto out;
	}

	do {
		size = vfs_read(filp, data, OAE_DM_HASH_BUFFER_SIZE, &offset);
		crypto_shash_update(desc, data, size);
	} while (size == OAE_DM_HASH_BUFFER_SIZE);

	*hash = kzalloc(digest_size, GFP_KERNEL);

	if (!*hash) {
		OAE_DM_ERROR("oae_dm: alloc hash fail\n");
		rc = -14;
		goto out;
	}
	crypto_shash_final(desc, *hash);

out:
	if (tfm)
		crypto_free_shash(tfm);
	if (data)
		kfree(data);
	if (desc)
		kfree(desc);
	if (!IS_ERR_OR_NULL(filp))
		filp_close(filp, NULL);
	set_fs(old_fs);
	return rc;
}

static int __init vertify_config(const char *patch_mnt_dir)
{
#ifndef CONFIG_SYSTEM_DATA_VERIFICATION
	return 0;
#else
	int rc = 0;
	int version;

	unsigned char *sig = NULL;
	unsigned char *data = NULL;
	char filename[OAE_DM_PATCH_FILE_LEN] = {0};

	mm_segment_t old_fs;
	size_t size;
	int data_len;
	int sig_len;
	int root_hash_len;
	int ptr;
	loff_t offset = 0;
	struct kstat *stat = NULL;
	struct file *filp = NULL;

	old_fs = get_fs();
	set_fs(get_ds());

	stat = (struct kstat *)kmalloc(sizeof(struct kstat), GFP_KERNEL);
	snprintf(filename, sizeof(filename) - 1, "%s/%s", patch_mnt_dir,
		 OAE_DM_CONFIG_FILE);
	vfs_stat(filename, stat);

	/* head: 8-magic 4-cfg ver 4-patch ver
	 * data: 32-patch hash 4-root hash len n-root hash
	 */
	size = OAE_DM_CONFIG_FILE_HEAD_SIZE + OAE_DM_SHA256_SIZE + sizeof(int);
	if (stat->size < size ||
	    stat->size > OAE_DM_CONFIG_FILE_MAX_SIZE) {
		OAE_DM_WARRING(
		    "oae_dm: get oae dm loop config file size error %d\n",
		    (int)stat->size);
		rc = 31;
		goto out;
	}

	filp = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		OAE_DM_WARRING("oae_dm: open oae dm loop config file %ld\n",
			       PTR_ERR(filp));
		rc = 32;
		goto out;
	}

	data = kzalloc(stat->size, GFP_KERNEL);
	if (!data) {
		OAE_DM_WARRING("oae_dm: read config alloc error\n");
		rc = 33;
		goto out;
	}

	size = vfs_read(filp, data, stat->size, &offset);
	if (size != stat->size) {
		OAE_DM_WARRING("oae_dm: read config file error\n");
		rc = 34;
		goto out;
	}

	if (strncmp(OAE_DM_CONFIG_FILE_MAGIC, data, 8)) {
		OAE_DM_WARRING("oae_dm: config file magic error\n");
		rc = 35;
		goto out;
	}

	ptr = OAE_DM_CONFIG_FILE_MAGIC_SIZE;
	memcpy(&version, data + ptr, sizeof(int));
	if (version < OAE_DM_CONFIG_FILE_VERSION) {
		OAE_DM_WARRING("oae_dm: config file version not support\n");
		rc = 36;
		goto out;
	}

	// cfg version / patch version
	ptr += 2 * sizeof(int);

	// patch hash
	ptr += OAE_DM_SHA256_SIZE;

	// root hash
	memcpy(&root_hash_len, data + ptr, sizeof(int));
	ptr += sizeof(int) + root_hash_len;
	if (root_hash_len > OAE_DM_ROOT_HASH_MAX_LEN ||
	    root_hash_len < OAE_DM_ROOT_HASH_MIN_LEN) {
		OAE_DM_ERROR("oae_dm: root hash len %d err\n", root_hash_len);
		rc = 37;
		goto out;
	}

	// sign start
	sig = data + ptr;
	if (stat->size < (ptr + OAE_DM_SIGN_MIN_LEN)) {
		OAE_DM_ERROR("oae_dm: cfg len %d err\n", stat->size);
		rc = 38;
		goto out;
	}
	sig_len = stat->size - ptr;
	data_len = ptr;

	OAE_DM_INFO("oae_dm: sig:%02x%02x%02x%02x--%02x%02x%02x%02x\n", sig[0],
		    sig[1], sig[2], sig[3], sig[sig_len - 4], sig[sig_len - 3],
		    sig[sig_len - 2], sig[sig_len - 1]);
	rc = verify_pkcs7_signature(data, data_len, sig, sig_len, NULL,
				    VERIFYING_MODULE_SIGNATURE, NULL, NULL);
	if (rc)
		rc = 39;

out:
	if (!IS_ERR_OR_NULL(filp))
		filp_close(filp, NULL);
	if (data)
		kfree(data);
	if (stat)
		kfree(stat);
	set_fs(old_fs);
	OAE_DM_INFO("oae_dm: vertify_config %d\n", rc);
	return rc;
#endif
}

static int __init vertify_loop_file(const char *patch_mnt_dir,
				    const char *lfile)
{
	int rc = 0;
	size_t digest_size;
	unsigned char *r_hash = NULL;
	unsigned char *a_hash = NULL;
	char filename[OAE_DM_PATCH_FILE_LEN] = {0};
	mm_segment_t old_fs;
	struct file *filp = NULL;
	loff_t offset = OAE_DM_CONFIG_FILE_HEAD_SIZE;

	digest_size = file_hash(lfile, &a_hash);
	if (digest_size != SHA256_DIGEST_SIZE) {
		OAE_DM_WARRING("oae_dm: file_hash fail\n");
		oae_dm_save_log(OAE_DM_PATCH_VERITY_ERR, digest_size);
		return 2;
	}

	old_fs = get_fs();
	set_fs(get_ds());

	OAE_DM_INFO("oae_dm: verity %s\n", lfile);

	snprintf(filename, sizeof(filename) - 1, "%s/%s", patch_mnt_dir,
		 OAE_DM_CONFIG_FILE);
	filp = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		OAE_DM_ERROR("oae_dm: open oae dm loop config file %ld\n",
			     PTR_ERR(filp));
		rc = 3;
		goto out;
	}

	r_hash = kzalloc(digest_size, GFP_KERNEL);
	if (!r_hash) {
		OAE_DM_ERROR("oae_dm: alloc r_hash fail\n");
		rc = 4;
		goto out;
	}

	rc = vfs_read(filp, r_hash, digest_size, &offset);
	if (rc != digest_size) {
		rc = 5;
		goto out;
	}

	filp_close(filp, NULL);
	filp = NULL;
	OAE_DM_INFO("oae_dm: verity r_sha %02x%02x%02x%02x--%02x%02x%02x%02x\n",
		    r_hash[0], r_hash[1], r_hash[2], r_hash[3],
		    r_hash[digest_size - 4], r_hash[digest_size - 3],
		    r_hash[digest_size - 2], r_hash[digest_size - 1]);
	OAE_DM_INFO("oae_dm: verity a_sha %02x%02x%02x%02x--%02x%02x%02x%02x\n",
		    a_hash[0], a_hash[1], a_hash[2], a_hash[3],
		    a_hash[digest_size - 4], a_hash[digest_size - 3],
		    a_hash[digest_size - 2], a_hash[digest_size - 1]);
	rc = memcmp(r_hash, a_hash, digest_size);
	if (rc)
		rc = 6;
out:
	if (!IS_ERR_OR_NULL(filp))
		filp_close(filp, NULL);
	if (r_hash)
		kfree(r_hash);
	if (a_hash)
		kfree(a_hash);
	set_fs(old_fs);
	return rc;
}

static int __init vertify_root_hash(const char *patch_mnt_dir)
{
	int rc = 0;
	int digest_size;
	unsigned char *r_hash = NULL;
	char *a_hash = NULL;
	char *sha_start = NULL;
	char *div_start = NULL;
	char filename[OAE_DM_PATCH_FILE_LEN] = {0};
	mm_segment_t old_fs;
	struct file *filp = NULL;
	loff_t offset = OAE_DM_CONFIG_FILE_HEAD_SIZE + OAE_DM_SHA256_SIZE;
	int i, root_hash_head, hash_size;

	old_fs = get_fs();
	set_fs(get_ds());

	snprintf(filename, sizeof(filename) - 1, "%s/%s", patch_mnt_dir,
		 OAE_DM_CONFIG_FILE);
	filp = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		OAE_DM_ERROR("oae_dm: open oae dm loop config file %ld\n",
			     PTR_ERR(filp));
		rc = 10;
		goto out;
	}

	rc = vfs_read(filp, (char *)&digest_size, sizeof(int), &offset);
	if (rc != sizeof(int) ||
	    digest_size < OAE_DM_ROOT_HASH_MIN_LEN ||
	    digest_size > OAE_DM_ROOT_HASH_MAX_LEN) {
		rc = 11;
		goto out;
	}

	r_hash = kzalloc(digest_size, GFP_KERNEL);
	if (!r_hash) {
		OAE_DM_ERROR("oae_dm: alloc r_hash fail\n");
		rc = 12;
		goto out;
	}

	// cfg file len checked at verify_config
	rc = vfs_read(filp, r_hash, digest_size, &offset);
	if (rc != digest_size) {
		rc = 13;
		goto out;
	}

	filp_close(filp, NULL);
	filp = NULL;

	hash_size = digest_size * 2 + 2;
	a_hash = kzalloc(hash_size, GFP_KERNEL);
	if (!a_hash) {
		OAE_DM_ERROR("oae_dm: alloc r_hash fail\n");
		rc = 14;
		goto out;
	}
	memset(a_hash, 0, hash_size);
	for (i = 0; i < digest_size; i++)
		snprintf(a_hash + strlen(a_hash),
			 hash_size - strlen(a_hash) - 1, "%02x", r_hash[i]);

	// get root hash
	if (g_root_sha_para == NULL) {
		OAE_DM_INFO("oae_dm: no root hash\n");
		rc = 0;
		goto out;
	}

	sha_start = strstr(g_root_sha_para, "sha"); // sha is cmdline keywords
	if (sha_start == NULL) {
		OAE_DM_INFO("oae_dm: root hash without sha\n");
		rc = 0;
		goto out;
	}
	div_start = strchr(sha_start, ' ');
	if (div_start == NULL) {
		OAE_DM_INFO("oae_dm: root hash without div\n");
		rc = 0;
		goto out;
	}

	root_hash_head = div_start - g_root_sha_para;
	if (root_hash_head <= 0 ||
	    strlen(g_root_sha_para) < (root_hash_head + 1 + digest_size * 2)) {
		OAE_DM_INFO("oae_dm: root hash err %s\n", g_root_sha_para);
		rc = 0;
		goto out;
	}

	OAE_DM_INFO("oae_dm: verity root_sha %s--%s\n", div_start + 1, a_hash);
	rc = strncmp(a_hash, div_start + 1, digest_size * 2);
	if (rc)
		rc = 15;
out:
	if (!IS_ERR_OR_NULL(filp))
		filp_close(filp, NULL);
	if (r_hash)
		kfree(r_hash);
	if (a_hash)
		kfree(a_hash);
	set_fs(old_fs);
	return rc;
}


static int __init vertify_patch_file(const char *patch_mnt_dir,
				     const char *lfile)
{
	int rc = 0;

	// sign verify
	rc = vertify_config(patch_mnt_dir);
	if (rc) {
		OAE_DM_WARRING(
		    "oae_dm: oae dm loop config file vertify fail\n");
		return rc;
	}

	// loop file hash check
	rc = vertify_loop_file(patch_mnt_dir, lfile);
	if (rc) {
		OAE_DM_WARRING("oae_dm: vertify_loop_file fail\n");
		return rc;
	}

	// root hash check
	rc = vertify_root_hash(patch_mnt_dir);
	if (rc) {
		OAE_DM_WARRING("oae_dm: vertify_root_hash fail\n");
		return rc;
	}

	return rc;
}

static int loop_index[OAE_DM_LOOP_MAX] __initdata = {-1, -1};

static int loop_cur_idx __initdata = -1;

static int __init get_loopid(void)
{
	loop_cur_idx++;
	return loop_cur_idx;
}

static int __init setup_loop_device(const char *lfile, int lid)
{
	int file_fd = -1;
	int dev_fd = -1;
	int rc = 0;
	struct loop_info64 li;
	mm_segment_t old_fs;
	char loopname[OAE_DM_PATCH_FILE_LEN] = {0};

	if (lid < 0 ||
	    lid > OAE_DM_MAX_LOOP_ID) {
		OAE_DM_WARRING("oae_dm: lid error %d\n", lid);
		rc = -1;
		goto loop_file_fail;
	}

	old_fs = get_fs();
	set_fs(get_ds());

	file_fd = sys_open(lfile, O_RDONLY, 0);
	if (file_fd < 0) {
		OAE_DM_WARRING("oae_dm: open %s fail %d\n", lfile, file_fd);
		rc = 1;
		goto loop_file_fail;
	}

	OAE_DM_INFO("oae_dm: loop file %s, loop id %d\n", lfile, lid);

	snprintf(loopname, sizeof(loopname) - 1, "/dev/block/loop%d", lid);
	rc = oae_create_dev(loopname,
			    MKDEV(LOOP_MAJOR, lid << OAE_DM_LOOP_PART_SHIFT));
	if (rc) {
		OAE_DM_ERROR("oae_dm: create %s = %d\n", loopname, rc);
		rc = 2;
		goto create_dev_fail;
	}

	dev_fd = sys_open(loopname, O_RDONLY, 0);
	if (dev_fd < 0) {
		OAE_DM_ERROR("oae_dm: open %s = %d\n", loopname, dev_fd);
		rc = 3;
		goto create_dev_fail;
	}

	rc = sys_ioctl(dev_fd, LOOP_SET_FD, file_fd);
	if (rc) {
		OAE_DM_ERROR("oae_dm: LOOP_SET_FD = %d\n", rc);
		rc = 4;
		goto set_dev_fail;
	}

	memset(&li, 0, sizeof(li));
	strlcpy((char *)li.lo_file_name, lfile, LO_NAME_SIZE);

	rc = sys_ioctl(dev_fd, LOOP_SET_STATUS64, (unsigned long)&li);
	if (rc) {
		OAE_DM_ERROR("oae_dm: LOOP_SET_STATUS = %d\n", rc);
		rc = 5;
		goto set_dev_fail;
	}
	sys_close(file_fd);
	sys_close(dev_fd);
	set_fs(old_fs);

	return 0;
set_dev_fail:
	sys_close(dev_fd);
	sys_unlink(loopname);
create_dev_fail:
	sys_close(file_fd);
loop_file_fail:
	set_fs(old_fs);
	return rc;
}

static int __init remove_loop(int lid)
{
	int dev_fd = -1;
	int file_fd = -1;
	int rc = 0;
	char loopname[OAE_DM_PATCH_FILE_LEN] = {0};
	mm_segment_t old_fs = get_fs();

	set_fs(get_ds());
	snprintf(loopname, sizeof(loopname) - 1, "/dev/block/loop%d", lid);

	dev_fd = sys_open(loopname, O_RDONLY, 0);
	if (dev_fd < 0) {
		OAE_DM_WARRING("oae_dm: open %s = %d\n", loopname, dev_fd);
		rc = 1;
		goto out;
	}

	rc = sys_ioctl(dev_fd, LOOP_CLR_FD, file_fd);
	if (rc) {
		OAE_DM_ERROR("oae_dm: LOOP_SET_FD = %d\n", rc);
		rc = -1;
		goto out;
	}
	sys_unlink(loopname);
	rc = 0;
out:
	if (dev_fd >= 0)
		sys_close(dev_fd);
	set_fs(old_fs);
	return rc;
}

static int __init get_patch_file(const char *patch_mnt_dir, char *lfile,
				 size_t max_len)
{
	char loopname[OAE_DM_PATCH_FILE_LEN] = {0};
	char filename[OAE_DM_PATCH_FILE_LEN] = {0};
	char squash_mnt_dir[OAE_DM_PATCH_FILE_LEN] = {0};
	mm_segment_t old_fs;
	int rc = 0;

	old_fs = get_fs();
	set_fs(get_ds());
	OAE_DM_INFO("oae_dm: get loop 1\n");

	snprintf(filename, sizeof(filename) - 1, "%s/%s", patch_mnt_dir,
		 OAE_DM_DEF_LOOP);
	rc = sys_access(filename, 0);
	if (!rc) {
		strlcpy(lfile, filename, max_len);
		rc = 0;
		goto clean;
	}
	OAE_DM_INFO("oae_dm: get loop 2\n");

	snprintf(filename, sizeof(filename) - 1, "%s/%s",
		 patch_mnt_dir, OAE_DM_SQUASH_FILE);
	rc = sys_access(filename, 0);
	if (rc) {
		rc = 1;
		goto clean;
	}

	loop_index[OAE_DM_LOOP_SFS_POS] = get_loopid();
	rc = setup_loop_device(filename, loop_index[OAE_DM_LOOP_SFS_POS]);
	if (rc) {
		rc = 10 + rc;
		goto clean;
	}
	OAE_DM_INFO("oae_dm: get loop 3\n");

	snprintf(loopname, sizeof(loopname) - 1, "/dev/block/loop%d",
		 loop_index[OAE_DM_LOOP_SFS_POS]);
	snprintf(squash_mnt_dir, sizeof(squash_mnt_dir) - 1, "%s/%s",
		 patch_mnt_dir, OAE_DM_SQUASH_MNT);
	sys_mkdir(squash_mnt_dir, 0751);
	rc = oae_dm_mount(loopname, squash_mnt_dir,
			  MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_NOATIME |
			  MS_NODIRATIME | MS_SILENT | MS_RDONLY,
			  "data=ordered");
	if (rc) {
		remove_loop(loop_index[OAE_DM_LOOP_SFS_POS]);
		sys_unlink(loopname);
		loop_index[OAE_DM_LOOP_SFS_POS] = -1;
		rc = 2;
		goto clean;
	}
	OAE_DM_INFO("oae_dm: get loop 4\n");
	snprintf(lfile, max_len - 1, "%s/%s",
		 patch_mnt_dir, OAE_DM_SQUASH_LOOP);
	rc = sys_access(lfile, 0);
	if (rc)
		rc = 3;
clean:
	OAE_DM_INFO("oae_dm: get loop %s end %d\n", lfile, rc);
	set_fs(old_fs);
	return rc;
}

static void __init restore(const char *patch_mnt_dir)
{
	int i;

	if (loop_index[OAE_DM_LOOP_MAX - 1] > 0) {
		char squash_mnt_dir[OAE_DM_PATCH_FILE_LEN] = {0};

		snprintf(squash_mnt_dir, sizeof(squash_mnt_dir) - 1, "%s/%s",
			 patch_mnt_dir, OAE_DM_SQUASH_MNT);
		sys_umount(squash_mnt_dir, 0);
		OAE_DM_INFO("oae_dm: umount sfs\n");
	}

	for (i = 0; i < OAE_DM_LOOP_MAX; i++) {
		if (loop_index[i] >= 0)
			remove_loop(loop_index[i]);
		OAE_DM_INFO("oae_dm: remove loop %d\n", loop_index[i]);
	}
}

static int patch_rollback(void)
{
	int rc;
	u32 last_boot_keypoint;

	rc = get_last_result();
	if (rc) {
		OAE_DM_INFO("oae_dm: get last not success, continue\n");
		return 0;
	}
	last_boot_keypoint = get_last_boot_keypoint();
	if (last_boot_keypoint > STAGE_FASTBOOT_END &&
	    last_boot_keypoint < STAGE_ANDROID_BOOT_SUCCESS) {
		OAE_DM_WARRING("oae_dm: key %d, roll\n", last_boot_keypoint);
		return (int)last_boot_keypoint;
	}
	OAE_DM_INFO("oae_dm: key %d, continue\n", last_boot_keypoint);
	return 0;
}

static dev_t __init setup_patch_loop_device(const char *patch_mnt_dir)
{
	int rc;
	char patch_file[OAE_DM_PATCH_FILE_LEN] = {0};

	OAE_DM_INFO("oae_dm: get patch file\n");
	rc = get_patch_file(patch_mnt_dir, patch_file, sizeof(patch_file));
	if (rc) {
		OAE_DM_WARRING("oae_dm: get patch fail\n");
		oae_dm_save_log(OAE_DM_PATCH_NOT_EXIST, rc);
		return 0;
	}

	OAE_DM_INFO("oae_dm: begin vertify_patch_file\n");
	rc = vertify_patch_file(patch_mnt_dir, patch_file);
	OAE_DM_INFO("oae_dm: end vertify_patch_file\n");
	if (rc) {
		OAE_DM_WARRING("oae_dm: vertify %s fail\n", patch_file);
		oae_dm_save_log(OAE_DM_PATCH_VERITY_ERR, rc);
		return 0;
	}

	rc = patch_rollback();
	if (rc) {
		OAE_DM_WARRING("oae_dm: patch roll back\n");
		oae_dm_save_log(OAE_DM_PATCH_ROLLBACK_ERR, rc);
		return 0;
	}

	OAE_DM_INFO("oae_dm: setup loop\n");
	loop_index[OAE_DM_LOOP_DEF_POS] = get_loopid();
	if (loop_index[OAE_DM_LOOP_DEF_POS] < 0 ||
	    loop_index[OAE_DM_LOOP_DEF_POS] > OAE_DM_MAX_LOOP_ID) {
		OAE_DM_ERROR("oae_dm: loop id err, %d\n,",
			     loop_index[OAE_DM_LOOP_DEF_POS]);
		oae_dm_save_log(OAE_DM_SETUP_LOOP_ERR, 0);
		return 0;
	}
	rc = setup_loop_device(patch_file, loop_index[OAE_DM_LOOP_DEF_POS]);
	if (rc) {
		OAE_DM_ERROR("oae_dm: setup_loop fail\n");
		oae_dm_save_log(OAE_DM_SETUP_LOOP_ERR, rc);
		return 0;
	}

	OAE_DM_INFO("oae_dm: setup loop ok\n");
	return MKDEV(LOOP_MAJOR,
		     loop_index[OAE_DM_LOOP_DEF_POS] << OAE_DM_LOOP_PART_SHIFT);
}

static int __init dev_size(dev_t dev, u64 *device_size)
{
	struct block_device *bdev = NULL;

	bdev = blkdev_get_by_dev(dev, FMODE_READ, NULL);
	if (IS_ERR_OR_NULL(bdev)) {
		OAE_DM_ERROR("oae_dm: blkdev_get_by_dev failed\n");
		return PTR_ERR(bdev);
	}

	*device_size = i_size_read(bdev->bd_inode);
	*device_size >>= SECTOR_SHIFT;

	OAE_DM_INFO("oae_dm: blkdev size in sectors: %llu\n", *device_size);
	blkdev_put(bdev, FMODE_READ);
	return 0;
}

static size_t __init gcd(size_t a, size_t b)
{
	return !b ? a : gcd(b, a % b);
}
static size_t __init lcm(size_t a, size_t b)
{
	return a / gcd(a, b) * b;
}

static unsigned short __init dev_block_size(dev_t blk_dev)
{
	struct block_device *bdev = NULL;
	unsigned short size;

	bdev = blkdev_get_by_dev(blk_dev, FMODE_READ, NULL);
	if (IS_ERR_OR_NULL(bdev))
		return 0;
	size = bdev_logical_block_size(bdev);
	blkdev_put(bdev, FMODE_READ);
	return size >> 9;
}

static dev_t __init setup_snapshot_device(const char *type, u64 end,
					  const char *params)
{
	struct mapped_device *md = NULL;
	struct dm_table *table = NULL;
	int error = 1;

	if (dm_create(DM_ANY_MINOR, &md)) {
		OAE_DM_ERROR("oae_dm: failed to create the device\n");
		error = 2;
		goto dm_create_fail;
	}
	OAE_DM_INFO("oae_dm: created device '%s'\n", dm_device_name(md));

	set_disk_ro(dm_disk(md), true);

	if (dm_table_create(&table, FMODE_READ, 1, md)) {
		OAE_DM_ERROR("oae_dm: failed  create the table\n");
		error = 3;
		goto dm_table_create_fail;
	}

	dm_lock_md_type(md);

	OAE_DM_INFO("oae_dm: add target '0 %llu %s %s'\n", end, type, params);

	if (dm_table_add_target(table, type, 0, end,
				(char __user __force *)params)) {
		OAE_DM_ERROR("oae_dm: failed to add the target to the table\n");
		error = 4;
		goto add_target_fail;
	}

	if (dm_table_complete(table)) {
		OAE_DM_ERROR("oae_dm: failed to complete the table\n");
		error = 5;
		goto table_complete_fail;
	}

	if (dm_suspend(md, 0)) {
		OAE_DM_ERROR("oae_dm: failed to suspend the device pre-bind\n");
		error = 6;
		goto suspend_fail;
	}

	dm_set_md_type(md, dm_table_get_type(table));

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0)
	if (dm_setup_md_queue(md)) {
#else
	if (dm_setup_md_queue(md, table)) {
#endif
		OAE_DM_ERROR(
		    "oae_dm: unable to set up device queue for new table.\n");
		error = 7;
		goto setup_md_queue_fail;
	}

	if (dm_swap_table(md, table)) {
		OAE_DM_ERROR(
		    "oae_dm: failed to bind the device to the table\n");
		error = 8;
		goto table_bind_fail;
	}

	if (dm_resume(md)) {
		OAE_DM_ERROR("oae_dm: failed to resume the device\n");
		error = 9;
		goto resume_fail;
	}

	OAE_DM_INFO("oae_dm: dm-%d is ready\n", dm_disk(md)->first_minor);
	dm_unlock_md_type(md);
	return MKDEV(dm_disk(md)->major, dm_disk(md)->first_minor);

resume_fail:
table_bind_fail:
suspend_fail:
setup_md_queue_fail:
table_complete_fail:
add_target_fail:
	dm_unlock_md_type(md);
dm_table_create_fail:
	dm_put(md);
dm_create_fail:
	OAE_DM_ERROR("oae_dm: starting oae-dm failed\n");
	oae_dm_save_log(OAE_DM_SETUP_PATCH_ERR, error);
	return 0;
}

dev_t __init oae_dm_setup_root(dev_t orginal_dev,
			       const char *patch_mnt_dir)
{
	char params[OAE_DM_PATCH_FILE_LEN] = {0};
	u64 device_size = 0;
	dev_t cow_dev;
	dev_t dev;
	int orginal_blk_size, cow_blk_size;
	int error;

	OAE_DM_INFO("oae_dm: oae_dm_setup_root start\n");

	if (!patch_mnt_dir) {
		OAE_DM_ERROR("oae_dm: patch_mnt_dir is null\n");
		error = 1;
		goto params_error;
	}

	if (!orginal_dev) {
		OAE_DM_ERROR("oae_dm: orgin device not ready\n");
		error = 2;
		goto params_error;
	}

	cow_dev = setup_patch_loop_device(patch_mnt_dir);
	if (!cow_dev) {
		OAE_DM_ERROR("oae_dm: cow device not ready\n");
		return 0;
	}

	orginal_blk_size = dev_block_size(orginal_dev);
	cow_blk_size = dev_block_size(cow_dev);

	if (dev_size(orginal_dev, &device_size)) {
		OAE_DM_ERROR("oae_dm: get device size fail\n");
		error = 3;
		goto params_error;
	}

	snprintf(params, sizeof(params) - 1, "%d:%d %d:%d P %zu",
		 MAJOR(orginal_dev), MINOR(orginal_dev),
		 MAJOR(cow_dev), MINOR(cow_dev),
		 lcm(orginal_blk_size, cow_blk_size) * OAE_DM_TRUNK_MUTI);
	OAE_DM_INFO("snapshot params %s\n", params);
	oae_dm_save_log(OAE_DM_DEFAULT_ERR, -1);
	dev = setup_snapshot_device(OAE_DM_SNAP_TYPE,
				    device_size, params);

	// don't add erro log here
	if (!dev)
		restore(patch_mnt_dir);
	OAE_DM_INFO("oae_dm: oae_dm_setup_root end\n");
	return dev;
params_error:
	oae_dm_save_log(OAE_DM_SNAPSHOT_PARAMS_ERR, error);
	return 0;
}

void __init oae_dm_cleanup(const char *patch_mnt_dir, int patched)
{
	char squash_mnt_dir[OAE_DM_PATCH_FILE_LEN] = {0};
	char path[OAE_DM_PATCH_FILE_LEN] = {0};

	if (g_root_sha_para) {
		OAE_DM_ERROR("oae_dm: test test clean\n");
		kfree(g_root_sha_para);
		g_root_sha_para = NULL;
	}

	if (!patch_mnt_dir) {
		OAE_DM_ERROR("oae_dm: oae_dm_cleanup patch_mnt_dir is null\n");
		return;
	}

	if (loop_index[OAE_DM_LOOP_SFS_POS] > 0) {
		snprintf(squash_mnt_dir, sizeof(squash_mnt_dir) - 1, "%s/%s",
			 patch_mnt_dir, OAE_DM_SQUASH_MNT);
		snprintf(path, sizeof(path) - 1, "%s/%s",
			 patch_mnt_dir + 1, OAE_DM_SQUASH_MNT);
		if (sys_mount(squash_mnt_dir, path, NULL, MS_MOVE, NULL))
			OAE_DM_ERROR("oae_dm: move squash fail\n");
	}
	if (patched)
		oae_dm_save_log(OAE_DM_OK, 0);
}
