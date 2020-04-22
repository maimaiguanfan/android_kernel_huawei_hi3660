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
#include "inc/tee/khandler.h"
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

struct map_entry_t *g_hwaa_phase1_key_map[NUMMAPENTRY] = {0};

void purge_phase1_key_map(void)
{
	purge_map((struct map_entry_t **)g_hwaa_phase1_key_map, NUMMAPENTRY);
}

static s32 get_phase1_key_from_map(u64 profile_id, u8 **phase1_key,
	u32 *phase1_key_size)
{
	u8 *data = NULL;
	u32 data_size = 0;
	u8 *phase1_key_map;
	s32 err_code;

	read_lock(&g_phase1_key_lock);
	err_code = retrieve_from_map(profile_id, g_hwaa_phase1_key_map,
		NUMMAPENTRY, &data, &data_size);
	if ((err_code == ERR_MSG_SUCCESS) && data) {
		if (data_size != KERNEL_QZONE_SECRET_LENGTH) {
			read_unlock(&g_phase1_key_lock);
			return ERR_MSG_KERNEL_QZONE_SECRET_NULL;
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
		return ERR_MSG_KERNEL_QZONE_SECRET_NULL;
	}
	*phase1_key = phase1_key_map;
	*phase1_key_size = KERNEL_QZONE_SECRET_LENGTH;
	read_unlock(&g_phase1_key_lock);
	return err_code;
}

s32 handle_kernel_init_qzone_code(u64 profile_id, const u8 *qcode,
	s32 qcode_length)
{
	s32 err_code;
	u8 *phase1_key = NULL;
	u32 phase1_key_size = 0;

	if (!qcode || (qcode_length != KERNEL_QZONE_SECRET_LENGTH)) {
		err_code = ERR_MSG_PAYLOAD_FORMAT_ERROR;
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
	err_code = send_qcode_request_towards_tee(profile_id, qcode,
		qcode_length, HWAA_INIT_USER, &phase1_key, &phase1_key_size);

	if ((err_code != ERR_MSG_SUCCESS) ||
		(phase1_key_size != KERNEL_QZONE_SECRET_LENGTH)) {
		hwaa_pr_err("hwaa init tee failed");
		goto cleanup;
	}
	write_lock(&g_phase1_key_lock);
	err_code = insert_into_amap(g_hwaa_phase1_key_map, NUMMAPENTRY,
		phase1_key, phase1_key_size, profile_id);
	write_unlock(&g_phase1_key_lock);
	if (err_code == ERR_MSG_SUCCESS) {
		hwaa_pr_info("phase1_key into map success");
		phase1_key = NULL;
	} else {
		hwaa_pr_err("phase1_key into map failed");
		goto cleanup;
	}
	hwaa_pr_info("qocde check success");

cleanup:
	kzfree(phase1_key);
	phase1_key = NULL;
	return err_code;
}

s32 handle_kernel_clear_qzone_key(u64 profile_id)
{
	s32 err_code;
	u8 *data = NULL;

	write_lock(&g_phase1_key_lock);
	err_code = delete_from_map(profile_id, g_hwaa_phase1_key_map,
		NUMMAPENTRY, &data);
	write_unlock(&g_phase1_key_lock);
	if (err_code == ERR_MSG_SUCCESS) {
		kzfree(data);
		data = NULL;
	} else if (err_code == ERR_MSG_APP_MAP_ENTRY_NOTEXIST) {
		err_code = ERR_MSG_SUCCESS;
	}

	return err_code;
}

s32 handle_kernel_new_kdf(u64 profile_id, u8 *bp_key_material,
	s32 key_material_len, u8 *fek, s32 fek_length, const u8 *app_id,
	s32 app_id_length)
{
	s32 err_code;
	u8 *phase1_key = NULL;
	u32 phase1_key_size = 0;
	u8 phase2_index[SHA256_LEN] = {0};
	u32 phase2_index_len = sizeof(phase2_index);
	u8 aes_key[AES256_KEY_LEN] = {0};
	u32 aes_key_len = sizeof(aes_key);
	u8 encoded_ciphertext[PHASE3_CIPHERTEXT_LENGTH] = {0};
	u32 encoded_ciphertext_len = sizeof(encoded_ciphertext);
	u8 plaintext_fek[FEK_LENGTH] = {0};
	u32 plaintext_fek_len = sizeof(plaintext_fek);
	u64 new_perms = 0;

	if (!bp_key_material || !fek || !app_id) {
		hwaa_pr_err("invalid param1!");
		err_code = ERR_MSG_NULL_PTR;
		goto cleanup;
	} else if ((fek_length <= 0) || (app_id_length != AAD_LENGTH)) {
		hwaa_pr_err("invalid param2!");
		err_code = ERR_MSG_PAYLOAD_FORMAT_ERROR;
		goto cleanup;
	}
	if (memcpy_s(&new_perms, sizeof(u64),
		app_id + app_id_length - sizeof(u64),
		sizeof(u64)) != EOK) {
		err_code =  ERR_MSG_GENERATE_FAIL;
		goto cleanup;
	}
	err_code = get_phase1_key_from_map(profile_id, &phase1_key,
		&phase1_key_size);
	if (err_code != ERR_MSG_SUCCESS) {
		hwaa_pr_err("Get phase1_key failed");
		goto cleanup;
	}
	err_code = hwaa_kdf_encrypt_keygen(phase1_key,
		phase1_key_size, app_id, app_id_length,
		phase2_index, &phase2_index_len, aes_key, &aes_key_len);
	if (err_code != ERR_MSG_SUCCESS) {
		hwaa_pr_err("BicDroid_KDF_Encrypt_KeyGen");
		err_code = ERR_MSG_KERNEL_WRONG_APP_ID;
		goto cleanup;
	}
	err_code = hwaa_kdf_encrypt(phase1_key, phase1_key_size,
		aes_key, aes_key_len, phase2_index, phase2_index_len,
		app_id, app_id_length, new_perms, encoded_ciphertext,
		&encoded_ciphertext_len, plaintext_fek, &plaintext_fek_len);
	if ((err_code != ERR_MSG_SUCCESS) ||
		(plaintext_fek_len != FEK_LENGTH) ||
		(encoded_ciphertext_len != PHASE3_CIPHERTEXT_LENGTH)) {
		hwaa_pr_err("BicDroid_KDF_Encrypt");
		err_code = ERR_MSG_KERNEL_APP_NOT_RUN;
		goto cleanup;
	}
	if (memcpy_s(fek, fek_length, plaintext_fek,
		plaintext_fek_len) != EOK) {
		err_code = ERR_MSG_GENERATE_FAIL;
		goto cleanup;
	}
	if (memcpy_s(bp_key_material, key_material_len,
		encoded_ciphertext, encoded_ciphertext_len) != EOK) {
		err_code = ERR_MSG_GENERATE_FAIL;
		goto cleanup;
	}
	hwaa_pr_info("new fek success");
cleanup:
	kzfree(phase1_key);
	phase1_key = NULL;

	return err_code;

}

s32 handle_kernel_exist_kdf(u64 profile_id, const u8 *key_material,
	s32 key_material_length, u8 **fek, s32 *fek_length)
{
	s32 err_code;
	u8 *phase1_key = NULL;
	u32 phase1_key_size = 0;
	u8 plaintext_fek[FEK_LENGTH] = {0};
	u32 plaintext_fek_len = FEK_LENGTH;

	if (!fek || !key_material || !fek_length) {
		hwaa_pr_err("invalid stp_msg or stp_out");
		err_code = ERR_MSG_NULL_PTR;
		return err_code;
	} else if (key_material_length != PHASE3_CIPHERTEXT_LENGTH) {
		hwaa_pr_err("invalid param!");
		err_code = ERR_MSG_PAYLOAD_FORMAT_ERROR;
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
		hwaa_pr_err("bp_file_key malloc");
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
