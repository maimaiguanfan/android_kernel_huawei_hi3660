
#include "inc/tee/hwaa_alg.h"
#include <linux/string.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <securec.h>
#include "inc/base/hwaa_utils.h"
#include "inc/tee/base_alg.h"

static const u8 HWAA_KDF_PHASE1_PHARSE[] = {
	'H', 'W', 'A', 'A', 'P', 'H', 'A', 'S', 'E', '1', '!' };
static const u32 HWAA_KDF_PHASE1_LEN = sizeof(HWAA_KDF_PHASE1_PHARSE);
static const u8 HWAA_KDF_PHASE2_PHARSE[] = {
	'H', 'W', 'A', 'A', 'P', 'H', 'A', 'S', 'E', '2', '!' };
static const u32 HWAA_KDF_PHASE2_LEN = sizeof(HWAA_KDF_PHASE2_PHARSE);

static bool hwaa_kdf_encrypt_keygen_checkpara(const u8 *phase1_key,
	u32 phase1_key_len, const u8 *cached_aad,
	u32 cached_aad_len, u8 *out_index, u32 *out_index_len,
	u8 *out_key_index, u32 *out_key_index_len)
{
	if (!phase1_key || (phase1_key_len != SHA256_HASH_BYTES) ||
		!cached_aad || (cached_aad_len != sizeof(struct auth_data_t)) ||
		!out_index || !out_index_len ||
		(*out_index_len != SHA256_HASH_BYTES) ||
		!out_key_index || !out_key_index_len ||
		(*out_key_index_len != AES256_KEY_LEN))
		return false;
	return true;
}

s32 hwaa_kdf_encrypt_keygen(const u8 *phase1_key, u32 phase1_key_len,
	const u8 *cached_aad, u32 cached_aad_len, u8 *out_index,
	u32 *out_index_len, u8 *out_key_index, u32 *out_key_index_len)
{
	struct auth_data_t aad = {0};
	s32 ret;

	if (!hwaa_kdf_encrypt_keygen_checkpara(phase1_key, phase1_key_len,
		cached_aad, cached_aad_len, out_index, out_index_len,
		out_key_index, out_key_index_len))
		return -EINVAL;

	get_random_bytes(out_index, *out_index_len);
	if (memcpy_s(&aad, sizeof(aad), cached_aad,
		sizeof(aad) - sizeof(aad.perms)) != EOK)
		return -EINVAL;

	if (memset_s(aad.perms, sizeof(aad.perms), 0, sizeof(aad.perms)) != EOK)
		return -EINVAL;

	ret = hkdf_256(out_index, *out_index_len, phase1_key, phase1_key_len,
		(u8 *)&aad, sizeof(aad), out_key_index, *out_key_index_len);
	return ret;
}

static bool hwaa_kdf_encrypt_checkpara(const u8 *phase1_key,
	u32 phase1_key_len, const u8 *phase2_key, u32 phase2_key_len,
	const u8 *phase2_index, u32 phase2_index_len, const u8 *cached_aad,
	u32 cached_aad_len, u64 new_perms, u8 *ciphertext, u32 *ciphertext_len,
	u8 *plaintext_fek, u32 *plaintext_fek_len)
{
	if (!phase1_key || (phase1_key_len != SHA256_HASH_BYTES))
		return false;

	if (!phase2_key || (phase2_key_len != AES256_KEY_LEN))
		return false;

	if (!phase2_index || (phase2_index_len != PHASE2_INDEX_LENGTH))
		return false;

	if (!cached_aad || (cached_aad_len != AAD_LENGTH))
		return false;

	if (!ciphertext || !ciphertext_len ||
		(*ciphertext_len != PHASE3_CIPHERTEXT_LENGTH))
		return false;

	if (!plaintext_fek || !plaintext_fek_len ||
		(*plaintext_fek_len != FEK_LENGTH))
		return false;
	return true;
}

s32 hwaa_kdf_encrypt(const u8 *phase1_key, u32 phase1_key_len,
	const u8 *phase2_key, u32 phase2_key_len,
	const u8 *phase2_index, u32 phase2_index_len,
	const u8 *cached_aad, u32 cached_aad_len,
	const u64 new_perms, u8 *ciphertext, u32 *ciphertext_len,
	u8 *plaintext_fek, u32 *plaintext_fek_len)
{
	u32 cipher_fek_len = FEK_LENGTH; // AES-256-XTS
	u32 tag_len = SHA256_TAG_LENGTH;
	u8 *en_ret = NULL;
	struct xattribs_t *xattr;
	s32 ret;

	if (!hwaa_kdf_encrypt_checkpara(phase1_key, phase1_key_len,
		phase2_key, phase2_key_len, phase2_index, phase2_index_len,
		cached_aad, cached_aad_len, new_perms, ciphertext,
		ciphertext_len, plaintext_fek, plaintext_fek_len))
		return -EINVAL;
	xattr = (struct xattribs_t *)ciphertext;

	xattr->version[0] = VERSION_1; // VERSION
	if (memcpy_s(xattr->index, sizeof(xattr->index), (void *)phase2_index,
		phase2_index_len) != EOK)
		return -EINVAL;
	get_random_bytes(xattr->iv, sizeof(xattr->iv));
	get_random_bytes(plaintext_fek, *plaintext_fek_len);
	if (memcpy_s(&(xattr->aad), sizeof(xattr->aad), (void *)cached_aad,
		cached_aad_len) != EOK)
		return -EINVAL;
	if (memcpy_s(xattr->aad.perms, sizeof(xattr->aad.perms), &new_perms,
		sizeof(new_perms)))
		return -EINVAL;

	ret = aes_cbc(phase2_key, phase2_key_len, xattr->iv, sizeof(xattr->iv),
		plaintext_fek, *plaintext_fek_len, &en_ret,
		(u32 *)&cipher_fek_len, true, false);
	if ((ret != 0) || (cipher_fek_len != FEK_LENGTH)) {
		kzfree(en_ret);
		en_ret = NULL;
		return (ret == 0) ? -EINVAL : ret;
	}
	if (memcpy_s(xattr->enc_fek, sizeof(xattr->enc_fek), en_ret,
		cipher_fek_len) != EOK) {
		kzfree(en_ret);
		en_ret = NULL;
		return -EINVAL;
	}
	kzfree(en_ret);
	en_ret = NULL;
	ret = hash_generate_mac(phase1_key, phase1_key_len, ciphertext,
		*ciphertext_len - sizeof(xattr->tag), xattr->tag, tag_len);
	return ret;
}

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
	u8 aad[sizeof(struct auth_data_t)] = {0};
	struct xattribs_t *xattr;
	s32 ret;

	if (!check_hwaa_kdef_extractkey_para(phase1_key, phase1_key_len,
		ciphertext, ciphertext_len, key_index, key_index_len))
		return -EINVAL;

	xattr = (struct xattribs_t *)ciphertext;
	/*
	 * VERSION (1 byte) | INDEX (32 bytes) | IV (16 bytes)
	 * | Enc(FEK) (64 bytes) | AAD (138 bytes) | HMAC Tag (32 bytes)
	 *
	 * sizeof(xattr->aad.perms) is the permissions flag
	 */
	if (memcpy_s(aad, sizeof(aad), &xattr->aad,
		sizeof(xattr->aad) - sizeof(xattr->aad.perms)) != EOK)
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
