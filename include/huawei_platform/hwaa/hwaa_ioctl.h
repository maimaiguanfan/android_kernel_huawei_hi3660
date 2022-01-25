
#ifndef _HWAA_IOCTL_H
#define _HWAA_IOCTL_H

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/ioctl.h>
#include "huawei_platform/hwaa/hwaa_error.h"
#include "huawei_platform/hwaa/hwaa_limits.h"
#else
#include <stdbool.h>
#include <sys/ioctl.h>
#include "hwaa_error.h"
#include "hwaa_limits.h"
#endif

/* device name */
#ifndef HWAA_DEVICE_NAME
#define HWAA_DEVICE_NAME "hwaa"
#endif

#define SHA256_HASH_SIZE 32

struct hwaa_init_user_t {
	s64 ausn; /* in: user serial number */
	u8 *key_desc; /* in: key descriptor */
	u64 key_desc_len; /* in: length of key_desc key */
	hwaa_result_t ret; /* out: return code */
};

struct hwaa_init_tee_t {
	hwaa_result_t ret; /* out: return code */
};

struct hwaa_clear_key_t {
	u64 ausn; /* in: user serial number */
	/* in: remove key in file or only remove key in cache */
	s32 is_remove_file;
	hwaa_result_t ret; /* out: return code */
};

/* magic number */
#define HWAA_MAGIC 0x2001

/* ioctl commands */
#define HWAA_INITIALIZE_USER _IOWR(HWAA_MAGIC, 1, struct hwaa_init_user_t *)
#define HWAA_INIT_TEE _IOR(HWAA_MAGIC, 12, struct hwaa_init_tee_t *)
#define HWAA_CLEAR_KEY _IOR(HWAA_MAGIC, 13, struct hwaa_clear_key_t *)

void init_user(struct hwaa_init_user_t *iusr);

#endif
