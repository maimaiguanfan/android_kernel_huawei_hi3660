/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: This file contains the function required for init_user and
 *              package management
 * Author: xuejikan 370596
 * Create: 2018-12-10
 */
#include "inc/ioctl/hwaa_ioctl_impl.h"
#include <securec.h>
#include "huawei_platform/hwaa/hwaa_ioctl.h"
#include "inc/ioctl/hwaa_user_key.h"
#include "inc/base/hwaa_utils.h"
#include "inc/base/hwaa_define.h"
#include "inc/base/hwaa_utils.h"
#include "inc/tee/hwaa_adapter.h"

void init_user(struct hwaa_init_user_t *iusr)
{
	s32 ret;
	u8 user_key[HWAA_USER_KEY_SIZE_MAX] = {0};
	u32 user_key_len = HWAA_USER_KEY_SIZE_MAX;
	u8 user_key_desc_str[HWAA_USER_KEY_DESC_STR_SIZE] = {0};
#ifdef HWAA_DEBUG
	print_init_user_input(iusr);
#endif
	make_user_key_desc_str(user_key_desc_str, iusr->key_desc);
	ret = get_user_key(user_key_desc_str, user_key, &user_key_len);
	// we only use the the first half of user key bytes
	user_key_len = (user_key_len == USER_KEY_LENGTH) ? (user_key_len / 2) :
		user_key_len;
	if (ret) {
		hwaa_pr_err("Failed to get user key for ausn = %lld,",
			iusr->ausn);
		iusr->ret = HWAA_ERR_UNKNOWN_USER;
	} else {
		hwaa_pr_info("hwaastart check cred");
		ret = kernel_init_credential(iusr->ausn, user_key,
			user_key_len);
		hwaa_pr_info("end check cred");
		if (ret == ERR_MSG_SUCCESS) {
			iusr->ret = HWAA_SUCCESS;
		} else {
			hwaa_pr_err(" ausn = %lld! ret = %d", iusr->ausn, ret);
			iusr->ret = HWAA_ERR_INTERNAL;
		}
	}
	hwaa_pr_info("init_user returned %d, ausn:%llu", ret, iusr->ausn);

	if (memset_s(user_key, sizeof(user_key), 0, sizeof(user_key)) != EOK)
		hwaa_pr_err("Unexpected, memset_s fail, should never happen!");
}
