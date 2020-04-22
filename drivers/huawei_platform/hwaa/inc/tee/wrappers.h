/*
 * Copyright (C) BicDroid Inc. 2018-2019. All Rights Reserved.
 * Description: Implementation of convenient wrap functions to
 *              facilitate kernel to submit requests to TEE and interpret
 *              replies from TEE. Those functions utilize functions whose
 *              implementation depend on OP-TEE platform.
 * Author: Xiang Yu, <yuxiang@bicdroid.com>
 *         Lorant Polya, <lpolya@bicdroid.com>
 * Create: 2018-03
 */
#ifndef _KERNEL_WRAPPERS_H
#define _KERNEL_WRAPPERS_H

#include <linux/types.h>

s32 init_qzone_key(u64 profile_id, const u8 *qzone_code, s32 qzone_code_length);

s32 clear_qzone_key(u64 profile_id);

/*
 * This function new file key to f2fs
 * @return 0 for success
 */
s32 get_new_file_key(u64 profile_id, u8 *key_material, s32 key_material_len,
	u8 *fek, s32 fek_length, const u8 *app_id, s32 app_id_length,
	s32 class_id);

s32 get_existing_file_key(u64 profile_id, const u8 *key_material,
	s32 key_material_length, u8 **fek, s32 *fek_length);

#endif
