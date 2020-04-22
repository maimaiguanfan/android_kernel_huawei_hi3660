/*
 * Copyright (C) BicDroid Inc. 2018-2019. All Rights Reserved.
 * Description: Implementation of convenient wrap functions to facilitate kernel
 *              to submit requests to TEE and interpret replies from TEE.
 *              Those functions utilize functions whose implementation depends
 *              on OP-TEE platform.
 * Author: Xiang Yu, <yuxiang@bicdroid.com>
 *         Lorant Polya, <lpolya@bicdroid.com>
 * Create: 2018-03
 */
#include "inc/tee/wrappers.h"
#include <linux/rwsem.h>
#include <linux/mutex.h>
#include "inc/base/hwaa_utils.h"
#include "inc/base/macros.h"
#include "inc/tee/khandler.h"

s32 init_qzone_key(u64 profile_id, const u8 *qzone_code, s32 qzone_code_length)
{
	s32 l_err_code = handle_kernel_init_qzone_code(profile_id,
		qzone_code, qzone_code_length);

	return l_err_code;
}

s32 clear_qzone_key(u64 profile_id)
{
	s32 l_err_code = handle_kernel_clear_qzone_key(profile_id);
	if (l_err_code != ERR_MSG_SUCCESS) {
		hwaa_pr_err("clear_qzone_key failed!");
		return l_err_code;
	}

	return l_err_code;
}

/*
 * This function new file key to f2fs
 * @return 0 for success
 */
s32 get_new_file_key(u64 profile_id, u8 *key_material, s32 key_material_len,
	u8 *fek, s32 fek_length, const u8 *app_id, s32 app_id_length,
	s32 class_id)
{
	return handle_kernel_new_kdf(profile_id, key_material, key_material_len,
		fek, fek_length, app_id, app_id_length);
}

s32 get_existing_file_key(u64 profile_id, const u8 *key_material,
	s32 key_material_length, u8 **fek, s32 *fek_length)
{
	return handle_kernel_exist_kdf(profile_id, key_material,
		key_material_length, fek, fek_length);
}