/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function definations required for
 *             operations about hwaa_adapter
 * Create: 2020-10-10
 */

#ifndef _HWAA_ADAPTER_H
#define _HWAA_ADAPTER_H
#include <linux/types.h>
#include "inc/base/hwaa_list.h"

void init_list(void);

void purge_phase1_key(void);

s32 kernel_init_credential(u64 profile_id, const u8 *cred,
	s32 cred_length);

s32 kernel_clear_credential(u64 profile_id);

s32 kernel_get_fek(u64 profile_id, const u8 *key_material,
	s32 key_material_length, u8 **fek, s32 *fek_length);

#endif
