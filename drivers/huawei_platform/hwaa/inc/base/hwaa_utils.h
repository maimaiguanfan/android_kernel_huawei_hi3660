
#ifndef _HWAA_UTILS_H
#define _HWAA_UTILS_H

#include "huawei_platform/hwaa/hwaa_ioctl.h"
#include <linux/kernel.h>

/* debug log tag */
#define HWAA_DEBUG_TAG "HWAA"
#define HWAA_PRINT_TAG  HWAA_DEBUG_TAG
#define hwaa_pr_err(fmt, args...) pr_err(" %s: " fmt "\n", \
	HWAA_PRINT_TAG, ## args)
#define hwaa_pr_warn(fmt, args...) pr_warn(" %s: " fmt "\n", \
	HWAA_PRINT_TAG,  ## args)
#define hwaa_pr_warn_once(fmt, args...) pr_warn_once("%s: " fmt "\n", \
	HWAA_PRINT_TAG, ## args)
#define hwaa_pr_info(fmt, args...) pr_info(" %s: " fmt "\n", \
	HWAA_PRINT_TAG,  ## args)
#define hwaa_pr_debug(fmt, args...) pr_debug(" %s: " fmt "\n", \
	HWAA_PRINT_TAG,  ## args)
#define HWAA_PER_USER_RANGE 100000
#define HWAA_MAX_BUFFER 10240
#define SYNC_PKG_CNT_MAX 200

s32 valid_init_user(struct hwaa_init_user_t *usr);

void print_init_user_input(struct hwaa_init_user_t *usr);

void bytes2string(u8 *bytes, u32 len, s8 *str, u32 str_len);

s64 hwaa_utils_get_ausn(uid_t uid);
#endif
