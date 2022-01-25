/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: This file contains the function definations required for
 *             operations about adapter
 * Create: 2018-03-18
 * History: 2020-10-10 add list
 */

#include "inc/tee/hwaa_adapter.h"
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/random.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <securec.h>
#include "inc/base/hwaa_utils.h"
#include "inc/tee/base_alg.h"
#include "inc/tee/hwaa_alg.h"
#include "inc/tee/hwaa_tee.h"

static DEFINE_RWLOCK(g_phase1_key_lock);

struct list_head g_head;

void init_list(void)
{
	INIT_LIST_HEAD(&g_head);
}

void purge_phase1_key(void)
{
	purge_list(&g_head);
}

static s32 get_phase1_key_from_map(u64 profile_id, u8 **phase1_key,
	u32 *phase1_key_size)
{
	u8 *data = NULL;
	u32 data_size = 0;
	u8 *phase1_key_map = NULL;
	s32 err_code;

	read_lock(&g_phase1_key_lock);
	err_code = retrieve_from_list(profile_id, &g_head, &data, &data_size);
	if ((err_code == ERR_MSG_SUCCESS) && data) {
		if (data_size != PHASE_1_KEY_LENGTH) {
			read_unlock(&g_phase1_key_lock);
			return ERR_MSG_KERNEL_PHASE1_KEY_NULL;
		}
		phase1_key_map = kzalloc(data_size, GFP_KERNEL);
		if (!phase1_key_map) {
			read_unlock(&g_phase1_key_lock);
			return ERR_MSG_OUT_OF_MEMORY;
		}
		if (memcpy_s(phase1_key_map, data_size, data,
			data_size) != EOK) {
			read_unlock(&g_phase1_key_lock);
			kzfree(phase1_key_map);
			return ERR_MSG_GENERATE_FAIL;
		}
		data = NULL;
	} else {
		read_unlock(&g_phase1_key_lock);
		return ERR_MSG_KERNEL_PHASE1_KEY_NULL;
	}
	*phase1_key = phase1_key_map;
	*phase1_key_size = PHASE_1_KEY_LENGTH;
	read_unlock(&g_phase1_key_lock);
	return err_code;
}

s32 kernel_init_credential(u64 profile_id, const u8 *cred,
	s32 cred_length)
{
	s32 err_code;
	u8 *phase1_key = NULL;
	u32 phase1_key_size = 0;

	if (!cred || (cred_length != PHASE_1_KEY_LENGTH)) {
		err_code = ERR_MSG_BAD_PARAM;
		return err_code;
	}
	err_code = get_phase1_key_from_map(profile_id, &phase1_key,
		&phase1_key_size);
	if (err_code == ERR_MSG_SUCCESS) {
		hwaa_pr_info("phase1_key in map");
		goto cleanup;
	}
	err_code = hwaa_init_tee();
	if (err_code != ERR_MSG_SUCCESS) {
		hwaa_pr_err("hwaa init tee failed");
		goto cleanup;
	}
	err_code = send_credential_request(profile_id, cred,
		cred_length, HWAA_INIT_USER, &phase1_key, &phase1_key_size);

	if ((err_code != ERR_MSG_SUCCESS) ||
		(phase1_key_size != PHASE_1_KEY_LENGTH)) {
		hwaa_pr_err("hwaa init tee failed");
		goto cleanup;
	}
	write_lock(&g_phase1_key_lock);
	err_code = insert_into_list(&g_head, phase1_key, phase1_key_size,
		profile_id);
	write_unlock(&g_phase1_key_lock);
	if (err_code == ERR_MSG_SUCCESS) {
		hwaa_pr_info("phase1_key into list success");
		phase1_key = NULL;
	} else {
		hwaa_pr_err("phase1_key into list failed");
		goto cleanup;
	}
	hwaa_pr_info("cred check success");

cleanup:
	kzfree(phase1_key);
	phase1_key = NULL;
	return err_code;
}

s32 kernel_clear_credential(u64 profile_id)
{
	s32 err_code;
	u8 *data = NULL;

	write_lock(&g_phase1_key_lock);
	err_code = delete_from_list(profile_id, &g_head, &data);
	write_unlock(&g_phase1_key_lock);
	if (err_code == ERR_MSG_SUCCESS) {
		kzfree(data);
		data = NULL;
	} else if (err_code == ERR_MSG_LIST_NODE_NOT_EXIST) {
		err_code = ERR_MSG_SUCCESS;
	}

	return err_code;
}

s32 kernel_get_fek(u64 profile_id, const u8 *key_material,
	s32 key_material_length, u8 **fek, s32 *fek_length)
{
	s32 err_code;
	u8 *phase1_key = NULL;
	u32 phase1_key_size = 0;
	u8 plaintext_fek[FEK_LENGTH] = {0};
	u32 plaintext_fek_len = FEK_LENGTH;

	if (!fek || !key_material || !fek_length) {
		hwaa_pr_err("invalid argument");
		err_code = ERR_MSG_NULL_PTR;
		return err_code;
	} else if (key_material_length != PHASE3_CIPHERTEXT_LENGTH) {
		hwaa_pr_err("invalid param!");
		err_code = ERR_MSG_BAD_PARAM;
		return err_code;
	}
	err_code = get_phase1_key_from_map(profile_id, &phase1_key,
		&phase1_key_size);
	if (err_code != ERR_MSG_SUCCESS) {
		hwaa_pr_err("get phase1_key failed");
		return err_code;
	}
	err_code = hwaa_kdf_get_fek(phase1_key, phase1_key_size,
		key_material, key_material_length,
		plaintext_fek, &plaintext_fek_len);
	if ((err_code != ERR_MSG_SUCCESS) ||
		(plaintext_fek_len != FEK_LENGTH)) {
		hwaa_pr_err("hwaa_kdf_get_fek:%d", err_code);
		goto cleanup;
	}
	*fek = kzalloc(plaintext_fek_len, GFP_KERNEL);
	if (*fek == NULL) {
		hwaa_pr_err("plaintext_fek malloc failed");
		err_code = ERR_MSG_OUT_OF_MEMORY;
		goto cleanup;
	}
	if (memcpy_s(*fek, plaintext_fek_len,
		plaintext_fek, FEK_LENGTH) != EOK) {
		err_code = ERR_MSG_GENERATE_FAIL;
		goto cleanup;
	}
	*fek_length = plaintext_fek_len;
	hwaa_pr_info("get fek success");
cleanup:
	kzfree(phase1_key);
	phase1_key = NULL;

	return err_code;
}
