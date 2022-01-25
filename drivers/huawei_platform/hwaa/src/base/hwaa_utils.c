
#include "inc/base/hwaa_utils.h"
#include <linux/err.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/limits.h>
#include <linux/mm.h>
#include <linux/path.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/rcupdate.h>
#include <securec.h>
#include "inc/base/hwaa_define.h"
#include "huawei_platform/hwaa/hwaa_limits.h"

#define EXE_MAX_ARG_STRLEN 256
#define MAX_JSON_STR_LEN 256
#define HEX_STR_PER_BYTE 2
#define HWAA_PERMISSION_LEN 8
#define PROCESS_INIT_TAG "init="
#define PROCESS_INIT_TAG_LEN 5

s64 hwaa_utils_get_ausn(uid_t uid)
{
	return (s64)(uid / HWAA_PER_USER_RANGE);
}

static s8 hex2char(u8 hex)
{
	s8 c;
	u8 lower = hex & 0x0f;

	if (lower >= 0x0a)
		c = ('a' + lower - 0x0a);
	else
		c = '0' + lower;
	return c;
}

void bytes2string(u8 *bytes, u32 len, s8 *str, u32 str_len)
{
	s32 i;

	if (!bytes || !str || ((HEX_STR_PER_BYTE * len + 1) > str_len))
		return;
	for (i = 0; i < len; i++) {
		str[HEX_STR_PER_BYTE * i] = hex2char((bytes[i] & 0xf0) >> 4);
		str[HEX_STR_PER_BYTE * i + 1] = hex2char(bytes[i] & 0x0f);
	}
	str[HEX_STR_PER_BYTE * len] = '\0';
}

s32 valid_init_user(struct hwaa_init_user_t *usr)
{
	s32 ret = 0;

	if (!usr || !usr->key_desc ||
		(usr->key_desc_len < HWAA_USER_KEY_DESC_MIN) ||
		(usr->key_desc_len > HWAA_USER_KEY_DESC_MAX)) {
		hwaa_pr_err("hwaa_init_user_t was invalid!");
		ret = -EINVAL;
	}
	return ret;
}
