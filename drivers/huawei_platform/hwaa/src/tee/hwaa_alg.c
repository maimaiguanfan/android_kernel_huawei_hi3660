
#include "inc/tee/hwaa_alg.h"
#include <linux/string.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <securec.h>
#include "inc/base/hwaa_utils.h"
#include "inc/tee/base_alg.h"

s32 hwaa_kdf_is_valid(const u8 *phase1_key, u32 phase1_key_len,
	const u8 *ciphertext, u32 ciphertext_len)
{
	u8 tmp_tag[SHA256_TAG_LENGTH] = {0};
	u32 tmp_tag_len = SHA256_TAG_LENGTH;
	s32 ret;

	if (!phase1_key || (phase1_key_len != SHA256_HASH_BYTES) ||
		!ciphertext || (ciphertext_len != PHASE3_CIPHERTEXT_LENGTH))
		return -EINVAL;

	ret = hash_generate_mac(phase1_key, phase1_key_len, ciphertext,
		ciphertext_len - SHA256_TAG_LENGTH, tmp_tag, tmp_tag_len);
	if (ret) {
		hwaa_pr_err("hash_generate_mac res(%d).\n", ret);
		return ret;
	}

	ret = memcmp(tmp_tag, &ciphertext[ciphertext_len - SHA256_TAG_LENGTH],
		tmp_tag_len);
	if ((ciphertext[0] == VERSION_1) && (ret == 0))
		ret = 0;
	else
		ret = -HWAA_ERR_TAG_NOT_MATCH;
	return ret;
}

/*
 * This function check hwaa_kdf_extractkey input params whether is invalid
 * @return true for success
 */
static bool check_hwaa_kdef_extractkey_para(const u8 *phase1_key,
	u32 phase1_key_len, const u8 *ciphertext,
	u32 ciphertext_len, u8 *key_index, u32 *key_index_len)
{
	if (!phase1_key || (phase1_key_len != SHA256_HASH_BYTES) ||
		!ciphertext || (ciphertext_len != PHASE3_CIPHERTEXT_LENGTH) ||
		!key_index || !key_index_len ||
		(*key_index_len != AES256_KEY_LEN))
		return false;
	return true;
}

static s32 hwaa_kdf_extractkey(const u8 *phase1_key, u32 phase1_key_len,
	const u8 *ciphertext, u32 ciphertext_len, u8 *key_index,
	u32 *key_index_len)
{
	u8 aad[AAD_LENGTH] = {0};
	struct xattribs_t *xattr = NULL;
	s32 ret;

	if (!check_hwaa_kdef_extractkey_para(phase1_key, phase1_key_len,
		ciphertext, ciphertext_len, key_index, key_index_len))
		return -EINVAL;

	xattr = (struct xattribs_t *)ciphertext;
	if (memcpy_s(aad, sizeof(aad), &xattr->aad,
		sizeof(xattr->aad) - PERMS_LENGTH) != EOK)
		return -EINVAL;

	ret = hkdf_256(xattr->index, sizeof(xattr->index), phase1_key,
		phase1_key_len, aad, sizeof(aad), key_index, *key_index_len);
	return ret;
}

static s32 hwaa_kdf_decrypt(const u8 *phase1_key, u32 phase1_key_len,
	const u8 *ciphertext, u32 ciphertext_len, u8 *plaintext_fek,
	u32 *plaintext_fek_len)
{
	u8 key_index[AES256_KEY_LEN] = {0};
	u32 key_index_len = sizeof(key_index);
	struct xattribs_t *xattr = (struct xattribs_t *)ciphertext;
	u8 *de_ret = NULL;
	s32 ret;

	if (!plaintext_fek || !plaintext_fek_len ||
		(*plaintext_fek_len != FEK_LENGTH))
		return -EINVAL;

	ret = hwaa_kdf_extractkey(phase1_key, phase1_key_len, ciphertext,
		ciphertext_len, key_index, &key_index_len);
	if (ret != 0)
		return ret;

	ret = aes_cbc(key_index, key_index_len, xattr->iv, sizeof(xattr->iv),
		xattr->enc_fek, sizeof(xattr->enc_fek), &de_ret,
		plaintext_fek_len, false, false);
	if ((ret != 0) || (*plaintext_fek_len != FEK_LENGTH)) {
		kzfree(de_ret);
		return (ret == 0) ? -EINVAL : ret;
	}

	if (memcpy_s(plaintext_fek, *plaintext_fek_len, de_ret, FEK_LENGTH)
		!= EOK) {
		kzfree(de_ret);
		de_ret = NULL;
		return -EINVAL;
	}
	kzfree(de_ret);
	de_ret = NULL;
	return ret;
}

s32 hwaa_kdf_get_fek(const u8 *phase1_key, u32 phase1_key_len,
	const u8 *ciphertext, u32 ciphertext_len,
	u8 *plaintext_fek, u32 *plaintext_fek_len)
{
	s32 ret = hwaa_kdf_is_valid(phase1_key, phase1_key_len, ciphertext,
		ciphertext_len);
	if (ret != 0) {
		hwaa_pr_err("%s res(%d)\n", __func__, ret);
		return ret;
	}

	ret = hwaa_kdf_decrypt(phase1_key, phase1_key_len, ciphertext,
		ciphertext_len, plaintext_fek, plaintext_fek_len);
	return ret;
}
