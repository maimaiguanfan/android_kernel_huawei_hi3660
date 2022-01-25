
#ifndef _HWAA_ALG_H
#define _HWAA_ALG_H
#include <linux/types.h>
#include "inc/base/hwaa_define.h"

#define SHA256_MAX_OBJECT_LENGTH 256
#define SHA256_HASH_BYTES 32
#define VERSION_1 0x01
#define PER_USER_RANGE 100000
#define AAD_LENGTH 106
#define PHASE2_INDEX_LENGTH 32
#define AES_IV_LENGTH 16
#define SHA256_TAG_LENGTH 32
#define VERSION_LENGTH 1
#define PERMS_LENGTH 8
#define FEK_LENGTH 64
#define SHA256_LEN 32
#define AES256_KEY_LEN 32
#define PHASE3_CIPHERTEXT_LENGTH (VERSION_LENGTH + PHASE2_INDEX_LENGTH + \
	AES_IV_LENGTH + FEK_LENGTH + AAD_LENGTH + SHA256_HASH_BYTES)

#pragma pack(1)
struct xattribs_t {
	u8 version[VERSION_LENGTH];
	u8 index[PHASE2_INDEX_LENGTH];
	u8 iv[AES_IV_LENGTH];
	u8 enc_fek[FEK_LENGTH];
	u8 aad[AAD_LENGTH];
	u8 tag[SHA256_TAG_LENGTH];
};
#pragma pack()

s32 hwaa_kdf_is_valid(const u8 *phase1_key, const u32 phase1_key_len,
	const u8 *ciphertext, u32 ciphertext_len);

s32 hwaa_kdf_get_fek(const u8 *phase1_key, u32 phase1_key_len,
	const u8 *ciphertext, u32 ciphertext_len, u8 *plaintext_fek,
	u32 *plaintext_fek_len);

#endif

