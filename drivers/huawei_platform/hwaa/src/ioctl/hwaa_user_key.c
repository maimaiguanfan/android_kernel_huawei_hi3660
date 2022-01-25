
#include "inc/ioctl/hwaa_user_key.h"
#include <uapi/linux/keyctl.h>
#include <keys/user-type.h>
#include <securec.h>
#include "inc/base/hwaa_utils.h"

static s32 get_user_key_from_keyring(struct key *keyring_key, u8 *user_key,
	u32 *user_key_len)
{
	s32 ret = 0;
	const struct user_key_payload *ukp;
	struct fscrypt_key *master_key;

	ukp = user_key_payload_locked(keyring_key);
	if (!ukp) {
		hwaa_pr_err("The key was revoked before it could be acquired!");
		ret = -EKEYREVOKED;
		goto do_return;
	}

	master_key = (struct fscrypt_key *)ukp->data;
	if ((ukp->datalen != sizeof(struct fscrypt_key)) ||
		(master_key->size < 1) ||
		(master_key->size > FS_MAX_KEY_SIZE)) {
		hwaa_pr_err("size of the fscrypt is incorrect! datalen = %d",
			ukp->datalen);
		ret = -EINVAL;
		goto do_return;
	}
	if (master_key->size > *user_key_len) {
		hwaa_pr_err("user k %d is greater than %d  allocated buffer",
			master_key->size, *user_key_len);
		ret = -EINVAL;
	} else {
		if (memcpy_s(user_key, *user_key_len, master_key->raw,
			master_key->size) != EOK) {
			return -EINVAL;
		}
		*user_key_len = master_key->size;
	}
do_return:
	return ret;
}

void make_user_key_desc_str(u8 *user_key_desc_str, u8 *user_key_desc_suffix)
{
	if (memcpy_s(user_key_desc_str, HWAA_USER_KEY_DESC_STR_SIZE,
		HWAA_USER_KEY_DESC_PREFIX_STR,
		HWAA_USER_KEY_DESC_PREFIX_STR_SIZE) != EOK)
		return;

	bytes2string(user_key_desc_suffix, HWAA_USER_KEY_DESC_SUFFIX_SIZE,
		user_key_desc_str + HWAA_USER_KEY_DESC_PREFIX_STR_SIZE,
		HWAA_USER_KEY_DESC_SUFFIX_STR_SIZE);
}

s32 get_user_key(u8 *user_key_desc_str, u8 *user_key, u32 *user_key_len)
{
	s32 ret;
	struct key *keyring_key;

	keyring_key = request_key(&key_type_logon, user_key_desc_str, NULL);
	if (IS_ERR(keyring_key)) {
		hwaa_pr_err("Failed to get keyring!");
		ret = PTR_ERR(keyring_key);
		goto do_return;
	}
	down_read(&keyring_key->sem);
	if (keyring_key->type != &key_type_logon) {
		hwaa_pr_err("The keyring key type is not logon!");
		ret = -ENOKEY;
		goto invalid;
	}
	ret = get_user_key_from_keyring(keyring_key, user_key, user_key_len);
	if (ret) {
		hwaa_pr_err("Failed to get user key from keyring,ret:%d", ret);
		goto invalid;
	}

invalid:
	up_read(&keyring_key->sem);
	key_put(keyring_key);

do_return:
	return ret;
}
