
#ifndef _BASE_ALG_H
#define _BASE_ALG_H
#include <linux/types.h>

/* AES-256-CBC encryption and decryption
 *
 * @[in]key: AES-256 input key
 * @[in]key_len: Should be AES_KEYSIZE_256 in bytes
 * @[in]iv: AES iv
 * @[in]iv_len: Should be AES_BLOCK_SIZE in bytes
 * @[in]in: Input data, should be multiple of AES_BLOCK_SIZE
 * @[in]in_len: Input length, should be non-zero and multiple of AES_BLOCK_SIZE.
 * @[out]out: Pointer to cipher text(decryption) or plain text(encryption)
 * @[in/out]out_len: Length of output. For padding, the value may not be same
 *                   with @in_len
 * @[in]enc: true(encryption) or false(decryption)
 * @[in]is_pad: whether a padding is needed
 *
 * @return 0 for success, < 0 if an error occurred
 */
s32 aes_cbc(const u8 *key, u32 key_len, const u8 *iv, u32 iv_len, const u8 *in,
	u32 in_len, u8 **out, u32 *out_len, bool enc, bool is_pad);

/* HMAC(SHA256)
 *
 * This is something different with general HMAC(SHA256). We have strict
 * limitation to the input parameters.
 *
 * @[in] phase1_key: 'Salt' in HKDF, can not be null.
 * @[in] phase1_key_len: Length of phase1_key, should be SHA256_DIGEST_SIZE.
 * @[in] msg: Message needs to be digested.
 * @[in] msg_len: Length of message, can not be zero, limited max value to 512.
 * @[out] tag: Tag of HMAC, can not be null, should have enough space.
 * @[in] tag_len: Length of tag, should be SHA256_DIGEST_SIZE.
 *
 * @return 0 for success, < 0 if an error occurred
 */
s32 hash_generate_mac(const u8 *phase1_key, u32 phase1_key_len, const u8 *msg,
	u32 msg_len, u8 *tag, u32 tag_len);

/* HKDF -- RFC 5869
 *
 * This is something different with RFC 5869. We have strict limitation to the
 * input parameters.
 *
 * @[in] salt: 'Salt' in HKDF, can not be null.
 * @[in] salt_len: Length of salt, should be SHA256_DIGEST_SIZE.
 * @[in] ikm: 'Input key material', can not be null.
 * @[in] ikm_len: Length of input key material, should be SHA256_DIGEST_SIZE.
 * @[in] info: 'Info' in HKDF, can not be null.
 * @[in] info_len: Length of info, can not be zero, limited max value to 256.
 * @[out] okm: 'Output key material', can not be null, should have enough space.
 * @[in] okm_len: Length of output key material wanted, can not be zero, limited
 *                max value to 256.
 *
 * @return 0 for success, < 0 if an error occurred
 */
s32 hkdf_256(const u8 *salt, u32 salt_len, const u8 *ikm, u32 ikm_len,
	const u8 *info, u32 info_len, u8 *okm, u32 okm_len);

#endif

