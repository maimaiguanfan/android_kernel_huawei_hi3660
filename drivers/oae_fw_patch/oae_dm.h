/*
 * Copyright (C) 2017 Qihoo, Inc.
 */
#include <linux/blkdev.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>

#ifndef OAE_DM_COMMON_DEF
#define OAE_DM_COMMON_DEF
#define OAE_DM_PATCH_CMDLINE_ERR -4
#define OAE_DM_PATCH_AVB_ERR -3
#define OAE_DM_PATCH_MOUNT_ERR -2
#define OAE_DM_DEFAULT_ERR -1
#define OAE_DM_OK 0
#define OAE_DM_PATCH_NOT_EXIST 1
#define OAE_DM_SETUP_PATCH_ERR 2
#define OAE_DM_PARSE_PATCH_ERR 3
#define OAE_DM_MOUNT_PATCH_ERR 4
#define OAE_DM_PATCH_VERITY_ERR 5
#define OAE_DM_SETUP_LOOP_ERR 6
#define OAE_DM_SNAPSHOT_PARAMS_ERR 7
#define OAE_DM_SNAPSHOT_ERR 8
#define OAE_DM_PATCH_ROLLBACK_ERR 9

#define OAE_DM_INFO(format, args...) printk(KERN_INFO format, ##args)
#define OAE_DM_WARRING(format, args...) printk(KERN_WARNING format, ##args)
#define OAE_DM_ERROR(format, args...) printk(KERN_ERR format, ##args)

#define OAE_DM_PATCH_ROOT "patch_hw"

#ifdef CONFIG_OAE_DM_ROOT
	extern char *g_root_sha_para;
#endif

static inline int oae_create_dev(char *name, dev_t dev)
{
	sys_unlink(name);
	return sys_mknod(name, S_IFBLK|0600, new_encode_dev(dev));
}

extern dev_t name_to_dev_t(const char *name);

dev_t patch_run_setup(void);

int oae_dm_mount(const char *name, const char *mnt_point,
		 int flags, const char *options);

void oae_dm_save_log(int major_code, int minor_code);

void log_partition_umount(void);

int get_last_result(void);

#ifdef CONFIG_OAE_DM_ROOT
dev_t oae_dm_setup_root(dev_t orgin_dev, const char *patch_mnt_dir);

void oae_dm_cleanup(const char *patch_mnt_dir, int patched);

#else

static inline dev_t oae_dm_setup_root(dev_t orgin_dev,
				      const char *patch_mnt_dir) { return 0; }

static inline void oae_dm_cleanup(const char *patch_mnt_dir, int patched) {}

#endif // CONFIG_OAE_DM_ROOT

#endif // OAE_DM_COMMON_DEF
