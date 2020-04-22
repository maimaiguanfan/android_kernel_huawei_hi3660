
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

s8 *turn_byte_into_hex(u8 *in, s32 size);

s32 valid_init_user(struct hwaa_init_user_t *usr);

void print_init_user_input(struct hwaa_init_user_t *usr);

s8 *hwaa_hexstring_to_bytes(s8 *hex, s32 len);

void bytes2string(u8 *bytes, u32 len, s8 *str, u32 str_len);

bool hwaa_utils_exe_check(pid_t pid, const s8 *exe_path,
	s32 exe_path_len);

void hwaa_utils_free_package_info(struct hwaa_package_info_t *pinfo);

s32 hwaa_utils_copy_package_info(struct hwaa_package_info_t *dst,
	struct hwaa_package_info_t *src);

s32 hwaa_utils_get_app_data(u8 **app_data, u32 *app_data_len,
	struct hwaa_package_info_t *pinfo, uid_t uid);

s32 hwaa_utils_copy_package_info_from_user(struct hwaa_package_info_t *kpinfo,
	struct hwaa_package_info_t *pinfo, bool copy_signer);

s32 hwaa_utils_copy_packages_from_user(struct hwaa_package_info_t *kpackages,
	struct hwaa_package_info_t *upackages, u32 package_count);

s64 hwaa_utils_get_ausn(uid_t uid);

/*
 * the function is called under very limited situations,
 * key is always a hardcoded const string while str is gotton from outside
 * exmple {"path":"/data/user/0/com.pkg.xx","ext":".db","subdirs":"true"}
 */
s8 *hwaa_utils_get_json_str(const s8 *str, const s8 *key, u32 *out_len);

s8 *get_cmdline_value(const s8 *cmdline);
#endif
