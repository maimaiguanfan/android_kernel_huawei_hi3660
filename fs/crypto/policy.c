/*
 * Encryption policy functions for per-file encryption support.
 *
 * Copyright (C) 2015, Google, Inc.
 * Copyright (C) 2015, Motorola Mobility.
 *
 * Written by Michael Halcrow, 2015.
 * Modified by Jaegeuk Kim, 2015.
 */

#include <linux/random.h>
#include <linux/string.h>
#include <keys/user-type.h>
#include <uapi/linux/keyctl.h>
#include <linux/mount.h>
#include "fscrypt_private.h"

#ifdef CONFIG_HWAA
#include <linux/security.h>
#include <huawei_platform/hwaa/hwaa_fs_hooks.h>
#endif

/*
 * check whether an encryption policy is consistent with an encryption context
 */
static bool is_encryption_context_consistent_with_policy(
				const struct fscrypt_context *ctx,
				const struct fscrypt_policy *policy)
{
	return memcmp(ctx->master_key_descriptor, policy->master_key_descriptor,
		      FS_KEY_DESCRIPTOR_SIZE) == 0 &&
		(ctx->flags == policy->flags) &&
		(ctx->contents_encryption_mode ==
		 policy->contents_encryption_mode) &&
		(ctx->filenames_encryption_mode ==
		 policy->filenames_encryption_mode);
}

static int create_encryption_context_from_policy(struct inode *inode,
				const struct fscrypt_policy *policy)
{
	struct fscrypt_context ctx;
	int res;
	u8 nonce[FS_KEY_DERIVATION_NONCE_SIZE];
	u8 plain_text[FS_KEY_DERIVATION_CIPHER_SIZE] = {0};
	struct key *keyring_key = NULL;
	struct fscrypt_key *master_key;
	const struct user_key_payload *ukp;
	struct crypto_aead *tfm = NULL;

	ctx.format = FS_ENCRYPTION_CONTEXT_FORMAT_V2;
	memcpy(ctx.master_key_descriptor, policy->master_key_descriptor,
					FS_KEY_DESCRIPTOR_SIZE);

	if (!fscrypt_valid_enc_modes(policy->contents_encryption_mode,
				     policy->filenames_encryption_mode))
		return -EINVAL;

	if (policy->flags & ~FS_POLICY_FLAGS_VALID)
		return -EINVAL;

	ctx.contents_encryption_mode = policy->contents_encryption_mode;
	ctx.filenames_encryption_mode = policy->filenames_encryption_mode;
	ctx.flags = policy->flags;
	BUILD_BUG_ON(sizeof(ctx.nonce) != FS_KEY_DERIVATION_CIPHER_SIZE);

	/* get nonce and iv */
	get_random_bytes(nonce, FS_KEY_DERIVATION_NONCE_SIZE);
	get_random_bytes(ctx.iv, FS_KEY_DERIVATION_IV_SIZE);
	memcpy(plain_text, nonce, FS_KEY_DERIVATION_NONCE_SIZE);

	/* get DEK */
	keyring_key = fscrypt_request_key(ctx.master_key_descriptor,
				FS_KEY_DESC_PREFIX, FS_KEY_DESC_PREFIX_SIZE);
	if (IS_ERR(keyring_key)) {
		if (inode->i_sb->s_cop->key_prefix) {
			const u8 *prefix = inode->i_sb->s_cop->key_prefix;
			int prefix_size;

			prefix_size = strlen(prefix);
			keyring_key = fscrypt_request_key(ctx.master_key_descriptor,
						prefix, prefix_size);
			if (!IS_ERR(keyring_key))
				goto got_key;
		}
		return PTR_ERR(keyring_key);
	}

got_key:
	if (keyring_key->type != &key_type_logon) {
		printk_once(KERN_WARNING
				"%s: key type must be logon\n", __func__);
		res = -ENOKEY;
		goto out;
	}

	down_read(&keyring_key->sem);

	ukp = user_key_payload_rcu(keyring_key);
	if (ukp->datalen != sizeof(struct fscrypt_key)) {
		res = -EINVAL;
		up_read(&keyring_key->sem);
		goto out;
	}

	master_key = (struct fscrypt_key *)ukp->data;
	//force the size equal to FS_AES_256_GCM_KEY_SIZE since user space might pass FS_AES_256_XTS_KEY_SIZE
	master_key->size = FS_AES_256_GCM_KEY_SIZE;
	if (master_key->size != FS_AES_256_GCM_KEY_SIZE) {
		printk_once(KERN_WARNING
				"%s: key size incorrect: %d\n",
				__func__, master_key->size);
		res = -ENOKEY;
		up_read(&keyring_key->sem);
		goto out;
	}

	tfm = (struct crypto_aead *)crypto_alloc_aead("gcm(aes)", 0, 0);
	if (IS_ERR(tfm)) {
		up_read(&keyring_key->sem);
		res = PTR_ERR(tfm);
		tfm = NULL;
		pr_err("fscrypt %s : tfm allocation failed!\n", __func__);
		goto out;
	}

	res = fscrypt_set_gcm_key(tfm, master_key->raw);
	up_read(&keyring_key->sem);
	if (res)
		goto out;

	res = fscrypt_derive_gcm_key(tfm, plain_text, ctx.nonce, ctx.iv, 1);
	if (res)
		goto out;

	res = inode->i_sb->s_cop->set_context(inode, &ctx, sizeof(ctx), NULL);
	if (res)
		goto out;
	fscrypt_set_verify_context(inode, &ctx, sizeof(ctx), NULL, 1);

out:
	if (tfm)
		crypto_free_aead(tfm);
	key_put(keyring_key);
	return res;
}

int fscrypt_ioctl_set_policy(struct file *filp, const void __user *arg)
{
	struct fscrypt_policy policy;
	struct inode *inode = file_inode(filp);
	int ret;
	struct fscrypt_context ctx;

	if (copy_from_user(&policy, arg, sizeof(policy)))
		return -EFAULT;

	if (!inode_owner_or_capable(inode))
		return -EACCES;

	if (policy.version != 0)
		return -EINVAL;

	ret = mnt_want_write_file(filp);
	if (ret)
		return ret;

	inode_lock(inode);

	ret = inode->i_sb->s_cop->get_context(inode, &ctx, sizeof(ctx), NULL);
	if (ret == -ENODATA) {
		if (!S_ISDIR(inode->i_mode))
			ret = -ENOTDIR;
		else if (!inode->i_sb->s_cop->empty_dir(inode))
			ret = -ENOTEMPTY;
		else
			ret = create_encryption_context_from_policy(inode,
								    &policy);
	} else if (ret == sizeof(ctx) &&
		   is_encryption_context_consistent_with_policy(&ctx,
								&policy)) {
		/* The file already uses the same encryption policy. */
		ret = 0;
	} else if (ret >= 0 || ret == -ERANGE) {
		/* The file already uses a different encryption policy. */
		ret = -EEXIST;
	}

	inode_unlock(inode);

	mnt_drop_write_file(filp);
	return ret;
}
EXPORT_SYMBOL(fscrypt_ioctl_set_policy);

int fscrypt_ioctl_get_policy(struct file *filp, void __user *arg)
{
	struct inode *inode = file_inode(filp);
	struct fscrypt_context ctx;
	struct fscrypt_policy policy;
	int res;

	if (!inode->i_sb->s_cop->is_encrypted(inode))
		return -ENODATA;

	res = inode->i_sb->s_cop->get_context(inode, &ctx, sizeof(ctx), NULL);
	if (res < 0 && res != -ERANGE)
		return res;
	if (res != sizeof(ctx))
		return -EINVAL;
	if (ctx.format != FS_ENCRYPTION_CONTEXT_FORMAT_V2)
		return -EINVAL;

	policy.version = 0;
	policy.contents_encryption_mode = ctx.contents_encryption_mode;
	policy.filenames_encryption_mode = ctx.filenames_encryption_mode;
	policy.flags = ctx.flags;
	memcpy(policy.master_key_descriptor, ctx.master_key_descriptor,
				FS_KEY_DESCRIPTOR_SIZE);

	if (copy_to_user(arg, &policy, sizeof(policy)))
		return -EFAULT;
	return 0;
}
EXPORT_SYMBOL(fscrypt_ioctl_get_policy);

/**
 * fscrypt_has_permitted_context() - is a file's encryption policy permitted
 *				     within its directory?
 *
 * @parent: inode for parent directory
 * @child: inode for file being looked up, opened, or linked into @parent
 *
 * Filesystems must call this before permitting access to an inode in a
 * situation where the parent directory is encrypted (either before allowing
 * ->lookup() to succeed, or for a regular file before allowing it to be opened)
 * and before any operation that involves linking an inode into an encrypted
 * directory, including link, rename, and cross rename.  It enforces the
 * constraint that within a given encrypted directory tree, all files use the
 * same encryption policy.  The pre-access check is needed to detect potentially
 * malicious offline violations of this constraint, while the link and rename
 * checks are needed to prevent online violations of this constraint.
 *
 * Return: 1 if permitted, 0 if forbidden.  If forbidden, the caller must fail
 * the filesystem operation with EPERM.
 */
int fscrypt_has_permitted_context(struct inode *parent, struct inode *child)
{
	const struct fscrypt_operations *cops = parent->i_sb->s_cop;
	const struct fscrypt_info *parent_ci, *child_ci;
	struct fscrypt_context parent_ctx, child_ctx;
	int res;

	/* No restrictions on file types which are never encrypted */
	if (!S_ISREG(child->i_mode) && !S_ISDIR(child->i_mode) &&
	    !S_ISLNK(child->i_mode))
		return 1;

	/* No restrictions if the parent directory is unencrypted */
	if (!cops->is_encrypted(parent))
		return 1;

	/* Encrypted directories must not contain unencrypted files */
	if (!cops->is_encrypted(child))
		return 0;

	if (cops->is_permitted_context) {
		if (cops->is_permitted_context(parent, child) == 1)
			return 1;
	}
	/*
	 * Both parent and child are encrypted, so verify they use the same
	 * encryption policy.  Compare the fscrypt_info structs if the keys are
	 * available, otherwise retrieve and compare the fscrypt_contexts.
	 *
	 * Note that the fscrypt_context retrieval will be required frequently
	 * when accessing an encrypted directory tree without the key.
	 * Performance-wise this is not a big deal because we already don't
	 * really optimize for file access without the key (to the extent that
	 * such access is even possible), given that any attempted access
	 * already causes a fscrypt_context retrieval and keyring search.
	 *
	 * In any case, if an unexpected error occurs, fall back to "forbidden".
	 */

	res = fscrypt_get_encryption_info(parent);
	if (res)
		return 0;
	res = fscrypt_get_encryption_info(child);
	if (res)
		return 0;
	parent_ci = parent->i_crypt_info;
	child_ci = child->i_crypt_info;

	/* TicketNo:AR000B5MBD -- For HWAA file we just use the i_crypt_info
	 *     since the ci_master_key in struct i_crypt_info is not changed. */
	/* TicketNo:AR0009DF3P -- For SDP file we use original CE context since
	 *     the ci_master_key in struct i_crypt_info is changed.
	 * The file is protected by SDP if (ci_hw_enc_flag & 0x0F) is not 0 */
	if (parent_ci && child_ci && !(child_ci->ci_hw_enc_flag & 0x0F)) {
		return memcmp(parent_ci->ci_master_key, child_ci->ci_master_key,
			      FS_KEY_DESCRIPTOR_SIZE) == 0 &&
			(parent_ci->ci_data_mode == child_ci->ci_data_mode) &&
			(parent_ci->ci_filename_mode ==
			 child_ci->ci_filename_mode) &&
			(parent_ci->ci_flags == child_ci->ci_flags);
	}
	/* TicketNo:AR0009DF3P END */
	/* TicketNo:AR000B5MBD END */

	res = cops->get_context(parent, &parent_ctx, sizeof(parent_ctx), NULL);
	if (res != sizeof(parent_ctx))
		return 0;

	res = cops->get_context(child, &child_ctx, sizeof(child_ctx), NULL);
	if (res != sizeof(child_ctx))
		return 0;

	return memcmp(parent_ctx.master_key_descriptor,
		      child_ctx.master_key_descriptor,
		      FS_KEY_DESCRIPTOR_SIZE) == 0 &&
		(parent_ctx.contents_encryption_mode ==
		 child_ctx.contents_encryption_mode) &&
		(parent_ctx.filenames_encryption_mode ==
		 child_ctx.filenames_encryption_mode) &&
		(parent_ctx.flags == child_ctx.flags);
}
EXPORT_SYMBOL(fscrypt_has_permitted_context);

/**
 * fscrypt_inherit_context() - Sets a child context from its parent
 * @parent: Parent inode from which the context is inherited.
 * @child:  Child inode that inherits the context from @parent.
 * @fs_data:  private data given by FS.
 * @preload:  preload child i_crypt_info if true
 *
 * Return: 0 on success, -errno on failure
 */
int fscrypt_inherit_context(struct inode *parent, struct inode *child,
						void *fs_data, bool preload)
{
	struct fscrypt_context ctx;
	struct fscrypt_info *ci;
	int res;
	u8 nonce[FS_KEY_DERIVATION_NONCE_SIZE];
	u8 plain_text[FS_KEY_DERIVATION_CIPHER_SIZE] = {0};

	res = fscrypt_get_encryption_info(parent);
	if (res < 0)
		return res;

	ci = parent->i_crypt_info;
	if (ci == NULL)
		return -ENOKEY;

	ctx.format = FS_ENCRYPTION_CONTEXT_FORMAT_V2;
	ctx.contents_encryption_mode = ci->ci_data_mode;
	ctx.filenames_encryption_mode = ci->ci_filename_mode;
	ctx.flags = ci->ci_flags;
	memcpy(ctx.master_key_descriptor, ci->ci_master_key,
	       FS_KEY_DESCRIPTOR_SIZE);

	/* get nonce and iv */
	get_random_bytes(nonce, FS_KEY_DERIVATION_NONCE_SIZE);
	get_random_bytes(ctx.iv, FS_KEY_DERIVATION_IV_SIZE);
	memcpy(plain_text, nonce, FS_KEY_DERIVATION_NONCE_SIZE);


	res = fscrypt_derive_gcm_key(ci->ci_gtfm, plain_text, ctx.nonce, ctx.iv, 1);
	if (res)
		return res;

	res = parent->i_sb->s_cop->set_context(child, &ctx,
						sizeof(ctx), fs_data);
	if (res)
		return res;
	fscrypt_set_verify_context(child, &ctx, sizeof(ctx), fs_data, 1);

	return preload ? fscrypt_get_encryption_info(child): 0;
}
EXPORT_SYMBOL(fscrypt_inherit_context);

#ifdef CONFIG_HWAA
static int f2fs_set_hwaa_enable_flags(struct inode *inode, void *fs_data)
{
	u32 flags;
	int res = 0;

	res = inode->i_sb->s_cop->get_hwaa_flags(inode, fs_data, &flags);
	if (res) {
		printk_once(KERN_ERR "%s:get inode (%lu) hwaa flags err (%d).\n",
			__func__, inode->i_ino, res);
		return -EINVAL;
	}
	flags |= HWAA_XATTR_ENABLE_FLAG;
	res = inode->i_sb->s_cop->set_hwaa_flags(inode, fs_data, &flags);
	if(res) {
		printk_once(KERN_ERR "%s:set inode (%lu) hwaa flags err (%d).\n",
			__func__, inode->i_ino, res);
		return -EINVAL;
	}

	return res;
}

/*
 * Code is mainly copied from fscrypt_inherit_context
 *
 * funcs except hwaa_create_fek must not return EAGAIN
 *
 * Return:
 *  o 0: SUCC
 *  o other errno: the file is not supported by policy
 */
int hwaa_inherit_context(struct inode *parent, struct inode *inode,
	struct dentry *dentry, void *fs_data, bool preload)
{
	uint8_t *encoded_wfek = NULL;
	uint8_t *fek = NULL;
	uint32_t encoded_len, fek_len;
	int err;
	/*
	 * called by __recover_do_dentries or
	 * f2fs_add_inline_entries or recover_dentry?
	 */
	if (!dentry)
		return -EAGAIN;
	if (!S_ISREG(inode->i_mode))
		return 0;
	/* create fek from hwaa, may delete fek later */
	err = hwaa_create_fek(inode, dentry, &encoded_wfek, &encoded_len,
		&fek, &fek_len, GFP_NOFS);
	if (err == -HWAA_ERR_NOT_SUPPORTED) {
		pr_info_once("hwaa ino %lu not protected\n", inode->i_ino);
		err = 0;
		goto free_buf;
	} else if (err) {
		pr_err("hwaa ino %lu create fek err %d\n", inode->i_ino, err);
		goto free_buf;
	}
	if (parent->i_sb->s_cop->set_hwaa_attr) {
		err = parent->i_sb->s_cop->set_hwaa_attr(inode, encoded_wfek,
			encoded_len, fs_data);
	} else {
		pr_info_once("hwaa ino %lu no setxattr\n", inode->i_ino);
		err = 0;
		goto free_hwaa;
	}
	if (err) {
		pr_err("hwaa ino %lu setxattr err %d\n", inode->i_ino, err);
		goto free_hwaa;
	}
	/* set new user type xattr and flags for HWAA */
	err = f2fs_set_hwaa_enable_flags(inode, fs_data);
	if (err) {
		pr_err_once("hwaa ino %lu set hwaa enable flags err %d\n",
			inode->i_ino, err);
		goto free_hwaa;
	}
	if (preload)
		err = hwaa_get_context(inode);
free_hwaa:
	kfree(encoded_wfek);
free_buf:
	kzfree(fek); //may delete fek later
	return err;
}
EXPORT_SYMBOL(hwaa_inherit_context);
#endif
