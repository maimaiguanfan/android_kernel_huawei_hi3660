
#ifndef _HWAA_USER_KEY_H
#define _HWAA_USER_KEY_H

#include <linux/fs.h>

#define HWAA_USER_KEY_DESC_PREFIX_STR  FS_KEY_DESC_PREFIX
#define HWAA_USER_KEY_DESC_PREFIX_STR_SIZE  FS_KEY_DESC_PREFIX_SIZE

#define HWAA_USER_KEY_DESC_SUFFIX_SIZE  FS_KEY_DESCRIPTOR_SIZE
#define HWAA_USER_KEY_DESC_SUFFIX_STR_SIZE ((HWAA_USER_KEY_DESC_SUFFIX_SIZE * 2) + 1)

#define HWAA_USER_KEY_DESC_STR_SIZE  (HWAA_USER_KEY_DESC_PREFIX_STR_SIZE + \
	HWAA_USER_KEY_DESC_SUFFIX_STR_SIZE)

#define HWAA_USER_KEY_SIZE_MAX FS_MAX_KEY_SIZE

void make_user_key_desc_str(u8 *user_key_desc_str, u8 *user_key_desc_suffix);

s32 get_user_key(u8 *user_key_desc_str, u8 *user_key, u32 *user_key_len);
#endif
