/*
 * Copyright (C) BicDroid Inc. 2018-2019. All Rights Reserved.
 * Description: This file contains the function definations required for
 *             operations about khandler
 * Author: Xiang Yu, <yuxiang@bicdroid.com>
 *         Lorant Polya, <lpolya@bicdroid.com>
 * Create: 2018-03
 */
#ifndef _INC_TEE_KHANDLER_H
#define _INC_TEE_KHANDLER_H
#include <linux/types.h>
#include "inc/base/amap.h"

void purge_phase1_key_map(void);

s32 handle_kernel_init_qzone_code(u64 profile_id, const u8 *qcode,
	s32 qcode_length);

s32 handle_kernel_clear_qzone_key(u64 profile_id);

s32 handle_kernel_new_kdf(u64 profile_id, u8 *key_material,
	s32 key_material_len, u8 *fek, s32 fek_length, const u8 *app_id,
	s32 app_id_length);

s32 handle_kernel_exist_kdf(u64 profile_id, const u8 *key_material,
	s32 key_material_length, u8 **fek, s32 *fek_length);

#endif