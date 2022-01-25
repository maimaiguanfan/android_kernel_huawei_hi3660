/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: Header file of Access Control Module.
 *
 *     This file is released under the GPL v2.
 */
#ifndef __DRIVERS_CHAR_ACM_H__
#define __DRIVERS_CHAR_ACM_H__

#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/hash.h>
#include <asm-generic/unistd.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/path.h>
#include <linux/mount.h>
#include <linux/fs_struct.h>
#include <uapi/linux/limits.h>
#include <uapi/asm-generic/errno.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/acm_f2fs.h>

#define ACM_DEV_NAME "acm"
#define ACM_DEV_BASE_MINOR 0
#define ACM_DEV_COUNT 1

#define ACM_MAGIC       'a'
#define ACM_ADD         _IOW(ACM_MAGIC, 0, struct acm_fwk_pkg)
#define ACM_DEL         _IOW(ACM_MAGIC, 1, struct acm_fwk_pkg)
#define ACM_SEARCH      _IOR(ACM_MAGIC, 2, struct acm_mp_node)
#define ACM_ADD_DIR     _IOR(ACM_MAGIC, 3, struct acm_fwk_dir)
#define ACM_DEL_DIR     _IOR(ACM_MAGIC, 4, struct acm_fwk_dir)
#ifdef CONFIG_ACM_DSM
#define ACM_ADD_DSM     _IOR(ACM_MAGIC, 5, struct acm_mp_node)
#define ACM_CMD_MAXNR   5
#else
#define ACM_CMD_MAXNR   4
#endif

#define ACM_HASHTBL_SZ 512
#define HASHTBL_MAX_SZ 4096
#define ACM_PATH_MAX 1024
#define DEPTH_INIT (-1)
#define ACM_DIR_MAX 64
#define ACM_DIR_LIST_MAX 1024
#define ACM_DNAME_MAX 256
#define ACM_DMD_LIST_MAX_NODES 2048
#define MAX_CACHED_DELETE_LOG 3
#define DELETE_LOG_UPLOAD_INTERVAL_MS 100

#define PATH_PREFIX_MEDIA "/media"
#define PATH_PREFIX_STORAGE_EMULATED "/storage/emulated"
#define PATH_UNKNOWN "unknown_path"

#define UEVENT_KEY_STR_MAX 16
#define ENV_DSM_PKGNAME_MAX (UEVENT_KEY_STR_MAX + ACM_PKGNAME_MAX)
#define ENV_DSM_PATH_MAX 1024
#define ENV_DSM_NR_STR_MAX 32
#define ENV_DSM_DEPTH_STR_MAX 32
#define ENV_DSM_FILE_TYPE_STR_MAX 32

#define ERR_PATH_MAX_DENTRIES 6
#define ERR_PATH_LAST_DENTRY 0

#define ACM_HASH_LEFT_SHIFT 4
#define ACM_HASH_MASK 0xF0000000L
#define ACM_HASH_RESULT_MASK 0x7FFFFFFF
#define ACM_HASH_RIGHT_SHIFT 24

#define DEL_ALLOWED 0
#define ACM_SUCCESS 0

/* white list node */
struct acm_hnode {
	struct hlist_node hnode;
	char pkgname[ACM_PKGNAME_MAX];
};

/* a hash table for white list */
struct acm_htbl {
	struct hlist_head *head;
	spinlock_t spinlock;
	int nr_nodes;
};

/* data node for directory */
struct acm_dnode {
	struct list_head lnode;
	char dir[ACM_DIR_MAX];
};

/* data node for framework and DMD */
struct acm_lnode {
	struct list_head lnode;
	char pkgname[ACM_PKGNAME_MAX];
	char path[ACM_PATH_MAX];
	int file_type;
	int depth;
	int op;
	/*
	 * Number of deleted files in a period of time,
	 * only used in cache
	 */
	int nr;
};

struct acm_list {
	struct list_head head;
	unsigned long nr_nodes;
	spinlock_t spinlock;
};

struct acm_cache {
	struct acm_lnode cache[MAX_CACHED_DELETE_LOG];
	int count;
};

struct acm_env {
	char pkgname[ENV_DSM_PKGNAME_MAX];
	char path[ENV_DSM_PATH_MAX];
	char depth[ENV_DSM_DEPTH_STR_MAX];
	char file_type[ENV_DSM_FILE_TYPE_STR_MAX];
	char nr[ENV_DSM_NR_STR_MAX];
	char op[ENV_DSM_NR_STR_MAX];
	char *envp[UEVENT_NUM_ENVP];
};

/* package name received from framework */
struct acm_fwk_pkg {
	char pkgname[ACM_PKGNAME_MAX];
};

/* directory received from framework */
struct acm_fwk_dir {
	char dir[ACM_DIR_MAX];
};

/* data received from mediaprovider */
struct acm_mp_node {
	char pkgname[ACM_PKGNAME_MAX];
	char path[ACM_PATH_MAX];
	int file_type;
	int flag;
};

#endif /* __DRIVERS_CHAR_ACM_H__ */
