
#include "inc/tee/base_alg.h"
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/completion.h>
#include <crypto/aes.h>
#include <crypto/sha.h>
#include <crypto/hash.h>
#include <crypto/skcipher.h>
#include <crypto/aead.h>
#include <securec.h>
#include "huawei_platform/hwaa/hwaa_error.h"
#include "inc/base/hwaa_utils.h"

#define HWAA_CBC_AES_ALG "cbc(aes)"
#define HWAA_HKDF_HMAC_ALG "hmac(sha256)"
#define HWAA_HKDF_HASHLEN SHA256_DIGEST_SIZE
#define HWAA_HMAC_TAG_LEN SHA256_DIGEST_SIZE
#define HWAA_CBC_AES_MAX_INPUT 512
#define HWAA_HKDF_MAX_INPUT_LEN 256
#define HWAA_HKDF_MAX_OUTPUT_LEN 256
#define HWAA_HMAC_MAX_MESSAGE_LEN 512
#define HWAA_CRYPTO_ALLOC_SHASH 0
#define HWAA_CRYPTO_ALLOC_MASK 0

struct tcrypt_result_t {
	struct completion completion;
	s32 err;
};

static void tcrypt_complete(struct crypto_async_request *req, s32 err)
{
	struct tcrypt_result_t *res;

	if ((err == -EINPROGRESS) || (req == NULL))
		return;
	res = req->data;
	res->err = err;
	complete(&res->completion);
}

static s32 wait_async_op(s32 ret, struct tcrypt_result_t *tr)
{
	s32 result = ret;

	if ((result == -EINPROGRESS) || (result == -EBUSY)) {
		wait_for_completion(&tr->completion);
		reinit_completion(&tr->completion);
		result = tr->err;
	}
	return result;
}

static bool check_aes_cbc_para(const u8 *key, u32 key_len, const u8 *iv,
	u32 iv_len, const u8 *in, u32 in_len, u8 *out, u32 *out_len, bool enc)
{
	if (!key || (key_len != AES_KEYSIZE_256) || !iv ||
		(iv_len != AES_BLOCK_SIZE) || !in || (in_len == 0) ||
		((in_len % AES_BLOCK_SIZE) != 0) ||
		(in_len > HWAA_CBC_AES_MAX_INPUT) || !out || !out_len ||
		(*out_len < in_len))
		return false;
	return true;
}

/* This function realize the zes cbc algorithm */
static s32 _aes_cbc_inter(const u8 *key, u32 key_len, const u8 *iv, u32 iv_len,
	const u8 *in, u32 in_len, u8 *out, u32 *out_len, bool enc)
{
	s32 res;
	struct tcrypt_result_t result;
	struct scatterlist src_sg;
	struct scatterlist dst_sg;
	struct crypto_skcipher *tfm;
	struct skcipher_request *req;
	u8 *tmp_iv;

	if (!check_aes_cbc_para(key, key_len, iv, iv_len, in, in_len, out,
		out_len, enc))
		return -EINVAL;

	tfm = crypto_alloc_skcipher(HWAA_CBC_AES_ALG, 0, 0);
	if (IS_ERR(tfm)) {
		res = PTR_ERR(tfm);
		hwaa_pr_err("alloc %s cipher failed res(%d).\n",
			HWAA_CBC_AES_ALG, res);
		return res;
	}

	init_completion(&result.completion);

	crypto_skcipher_set_flags(tfm, CRYPTO_TFM_REQ_WEAK_KEY);
	req = skcipher_request_alloc(tfm, GFP_NOFS);
	if (!req) {
		res = -ENOMEM;
		goto free_1;
	}
	skcipher_request_set_callback(req,
		CRYPTO_TFM_REQ_MAY_BACKLOG | CRYPTO_TFM_REQ_MAY_SLEEP,
		tcrypt_complete, &result);
	res = crypto_skcipher_setkey(tfm, key, key_len);
	if (res < 0)
		goto free_2;

	sg_init_one(&src_sg, in, in_len);
	sg_init_one(&dst_sg, out, in_len);
	tmp_iv = kmalloc(AES_BLOCK_SIZE, GFP_NOFS);
	if (!tmp_iv) {
		res = -ENOMEM;
		goto free_2;
	}
	if (memcpy_s(tmp_iv, AES_BLOCK_SIZE, iv, iv_len) != EOK) {
		res = -EINVAL;
		kzfree(tmp_iv);
		goto free_2;
	}

	skcipher_request_set_crypt(req, &src_sg, &dst_sg, in_len, tmp_iv);

	if (enc)
		res = wait_async_op(crypto_skcipher_encrypt(req), &result);
	else
		res = wait_async_op(crypto_skcipher_decrypt(req), &result);

	if (!res)
		*out_len = in_len;

	kzfree(tmp_iv);
free_2:
	skcipher_request_free(req);
free_1:
	crypto_free_skcipher(tfm);
	return res;
}

static s32 aes_cbc_check_para(const u8 *key, u32 key_len, const u8 *iv,
	u32 iv_len, const u8 *in, u32 in_len, u8 **out, u32 *out_len)
{
	if (!key || (key_len != AES_KEYSIZE_256) || !iv ||
		(iv_len != AES_BLOCK_SIZE) || !in || in_len == 0 ||
		(in_len > HWAA_CBC_AES_MAX_INPUT) || !out || !out_len)
		return false;
	return true;
}

s32 aes_cbc(const u8 *key, u32 key_len, const u8 *iv, u32 iv_len, const u8 *in,
	u32 in_len, u8 **out, u32 *out_len, bool enc, bool is_pad)
{
	s32 res;
	u32 act_ret_len;
	u8 *tmp_in = NULL;
	u8 *tmp_out = NULL;
	s32 i;
	u8 pad_num = 0;

	if (!aes_cbc_check_para(key, key_len, iv, iv_len, in, in_len, out,
		out_len))
		return -EINVAL;

	if (enc && is_pad) {
		act_ret_len = in_len + (AES_BLOCK_SIZE -
			(in_len % AES_BLOCK_SIZE));
		tmp_in = kmalloc(act_ret_len, GFP_NOFS);
		tmp_out = kmalloc(act_ret_len, GFP_NOFS);
		if (!tmp_in || !tmp_out) {
			res = -ENOMEM;
			goto out;
		}
		*out_len = act_ret_len;
		pad_num = act_ret_len - in_len;
		// lenth enough no risk
		if (memcpy_s(tmp_in, act_ret_len, in, in_len) != EOK) {
			res = -EINVAL;
			goto out;
		}
		// length just enough just padding
		if (memset_s(tmp_in + in_len, act_ret_len - in_len, pad_num,
			pad_num) != EOK) {
			res = -EINVAL;
			goto out;
		}
	} else {
		if ((in_len % AES_BLOCK_SIZE) != 0) {
			res = -EINVAL;
			goto out;
		}
		act_ret_len = in_len;
		tmp_in = (u8 *)in;
		tmp_out = kmalloc(in_len, GFP_NOFS);
		if (!tmp_out) {
			res = -ENOMEM;
			goto out;
		}
		*out_len = in_len;
	}

	res = _aes_cbc_inter(key, key_len, iv, iv_len, tmp_in, act_ret_len,
		tmp_out, out_len, enc);
	if (res)
		goto out;

	if (!enc && is_pad) {
		pad_num = tmp_out[in_len - 1];

		if (pad_num > AES_BLOCK_SIZE) {
			res = -EINVAL;
			goto out;
		} else {
			for (i = 0; i < pad_num; i++) {
				if (tmp_out[in_len - 1 - i] != pad_num) {
					res = -EINVAL;
					goto out;
				}
			}
		}
		*out_len = in_len - pad_num;
	}

	*out = tmp_out;
	tmp_out = NULL;
out:
	if (enc && is_pad) {
		kzfree(tmp_in);
		tmp_in = NULL;
	}
	// if NULL, kzfree does nothing
	kzfree(tmp_out);
	tmp_out = NULL;

	return res;
}

static s32 hkdf_extract(const u8 *salt, u32 salt_len, const u8 *ikm,
	u32 ikm_len, u8 *prk, u32 prk_len)
{
	s32 res;
	struct crypto_shash *hmac_alg;

	if (!salt || (salt_len != HWAA_HKDF_HASHLEN) || !ikm ||
		(ikm_len != HWAA_HKDF_HASHLEN) || !prk ||
		(prk_len != HWAA_HKDF_HASHLEN))
		return -EINVAL;

	hmac_alg = crypto_alloc_shash(HWAA_HKDF_HMAC_ALG,
		HWAA_CRYPTO_ALLOC_SHASH,
		HWAA_CRYPTO_ALLOC_MASK);
	if (IS_ERR(hmac_alg)) {
		res = (s32)PTR_ERR(hmac_alg);
		return res;
	}

	if (crypto_shash_digestsize(hmac_alg) != prk_len) {
		crypto_free_shash(hmac_alg);
		return -EINVAL;
	}

	SHASH_DESC_ON_STACK(desc, hmac_alg);
	desc->tfm = hmac_alg;
	desc->flags = 0;
	res = crypto_shash_setkey(hmac_alg, salt, salt_len);
	if (res)
		goto free;

	res = crypto_shash_digest(desc, ikm, ikm_len, prk);

free:
	shash_desc_zero(desc);
	crypto_free_shash(hmac_alg);
	return res;
}

static s32 hkdf_expand(const u8 *prk, u32 prk_len, const u8 *info, u32 info_len,
	u8 *okm, u32 okm_len)
{
	s32 res;
	const u8 *prev = NULL;
	u8 tmp[HWAA_HKDF_HASHLEN] = {0};
	u8 counter = 1; // counter starts from 1
	u32 i = 0;
	struct crypto_shash *hmac_tfm;

	if (!prk || (prk_len != HWAA_HKDF_HASHLEN) || !info ||
		(info_len == 0) || (info_len > HWAA_HKDF_MAX_INPUT_LEN) ||
		!okm || (okm_len == 0) || (okm_len > HWAA_HKDF_MAX_OUTPUT_LEN))
		return -EINVAL;
	hmac_tfm = crypto_alloc_shash(HWAA_HKDF_HMAC_ALG,
		HWAA_CRYPTO_ALLOC_SHASH,
		HWAA_CRYPTO_ALLOC_MASK);
	if (IS_ERR(hmac_tfm)) {
		res = (s32)PTR_ERR(hmac_tfm);
		return res;
	}
	if (crypto_shash_digestsize(hmac_tfm) != prk_len) {
		crypto_free_shash(hmac_tfm);
		return -EINVAL;
	}
	SHASH_DESC_ON_STACK(desc, hmac_tfm);
	desc->tfm = hmac_tfm;
	desc->flags = 0;
	res = crypto_shash_setkey(hmac_tfm, prk, prk_len);
	if (res)
		goto free;
	for (i = 0; i < okm_len; i += HWAA_HKDF_HASHLEN) {
		res = crypto_shash_init(desc);
		if (res)
			goto free;
		if (prev) {
			res = crypto_shash_update(desc, prev,
				HWAA_HKDF_HASHLEN);
			if (res)
				goto free;
		}
		res = crypto_shash_update(desc, info, info_len);
		if (res)
			goto free;
		// okm_len is always bigger than i
		if ((okm_len - i) < HWAA_HKDF_HASHLEN) {
			res = crypto_shash_finup(desc, &counter,
				sizeof(counter), tmp);
			if (res)
				goto free;
			// tmp has enough len and so is ok
			if (memcpy_s(&okm[i], okm_len - i,
				tmp, okm_len - i) != EOK)
				goto free;
			memzero_explicit(tmp, sizeof(tmp));
		} else {
			res = crypto_shash_finup(desc, &counter,
				sizeof(counter), &okm[i]);
			if (res)
				goto free;
		}
		counter++;
		prev = &okm[i];
	}
free:
	shash_desc_zero(desc);
	crypto_free_shash(hmac_tfm);
	return res;
}

s32 hkdf_256(const u8 *salt, u32 salt_len, const u8 *ikm, u32 ikm_len,
	const u8 *info, u32 info_len, u8 *okm, u32 okm_len)
{
	u8 tmp_key[HWAA_HKDF_HASHLEN] = {0};
	u32 tmp_key_len = sizeof(tmp_key);
	s32 res;

	if (!salt || (salt_len != HWAA_HKDF_HASHLEN) || !ikm ||
		(ikm_len != HWAA_HKDF_HASHLEN) || !info || (info_len == 0) ||
		(info_len > HWAA_HKDF_MAX_INPUT_LEN) || !okm ||
		(okm_len == 0) || (okm_len > HWAA_HKDF_MAX_OUTPUT_LEN)) {
		return -EINVAL;
	}

	res = hkdf_extract(salt, salt_len, ikm, ikm_len, tmp_key, tmp_key_len);
	if (res) {
		hwaa_pr_err("hkdf_extract failed res(%d).\n", res);
		res = -HWAA_ERR_HKDF_EXTRACT;
		goto out;
	}

	res = hkdf_expand(tmp_key, tmp_key_len, info, info_len, okm, okm_len);
	if (res) {
		hwaa_pr_err("hkdf_expand failed res(%d).\n", res);
		res = -HWAA_ERR_HKDF_EXPAND;
		goto out;
	}
out:
	return res;
}

s32 hash_generate_mac(const u8 *phase1_key, u32 phase1_key_len, const u8 *msg,
	u32 msg_len, u8 *tag, u32 tag_len)
{
	s32 res;
	struct crypto_shash *hmac_tfm;

	if (!phase1_key || (phase1_key_len != HWAA_HKDF_HASHLEN) ||
		!msg || (msg_len == 0) ||
		(msg_len > HWAA_HMAC_MAX_MESSAGE_LEN) ||
		!tag || (tag_len != HWAA_HMAC_TAG_LEN))
		return -EINVAL;

	hmac_tfm = crypto_alloc_shash(HWAA_HKDF_HMAC_ALG,
		HWAA_CRYPTO_ALLOC_SHASH,
		HWAA_CRYPTO_ALLOC_MASK);
	if (IS_ERR(hmac_tfm)) {
		res = (s32)PTR_ERR(hmac_tfm);
		return res;
	}

	if (crypto_shash_digestsize(hmac_tfm) != tag_len) {
		crypto_free_shash(hmac_tfm);
		return -EINVAL;
	}
	SHASH_DESC_ON_STACK(desc, hmac_tfm);
	desc->tfm = hmac_tfm;
	desc->flags = 0;
	res = crypto_shash_setkey(hmac_tfm, phase1_key, phase1_key_len);
	if (res)
		goto free;

	res = crypto_shash_digest(desc, msg, msg_len, tag);

free:
	shash_desc_zero(desc);
	crypto_free_shash(hmac_tfm);
	return res;
}
