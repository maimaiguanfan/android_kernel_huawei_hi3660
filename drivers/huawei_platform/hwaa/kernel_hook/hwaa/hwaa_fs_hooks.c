
#include "huawei_platform/hwaa/hwaa_fs_hooks.h"
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/fscrypt_common.h>
#include <linux/rwsem.h>
#include <uapi/linux/stat.h>
#include "huawei_platform/hwaa/hwaa_limits.h"

#ifdef CONFIG_HWAA
static DECLARE_RWSEM(g_fs_callbacks_lock);

static hwaa_result_t default_create_fek(pid_t pid, uid_t uid, s32 gid,
	const s8 *fsname, struct dentry *dent, umode_t mode,
	u8 **encoded_wfek, u32 *encoded_len, u8 **fek,
	u32 *fek_len, gfp_t kmem_flag)
{
	return -HWAA_ERR_NO_FS_CALLBACKS;
}

static hwaa_result_t default_get_fek(pid_t pid, uid_t uid, const u8 *encoded_wfek,
	u32 encoded_len, u8 **fek, u32 *fek_len, gfp_t kmem_flag)
{
	return -HWAA_ERR_NO_FS_CALLBACKS;
}

static hwaa_result_t default_hwaa_has_access(pid_t pid, uid_t uid,
	const s8 *encoded_wfek, u32 encoded_len)
{
	return -HWAA_ERR_NO_FS_CALLBACKS;
}

static struct hwaa_fs_callbacks_t g_fs_callbacks = {
	.create_fek = default_create_fek,
	.get_fek = default_get_fek,
	.hwaa_has_access = default_hwaa_has_access,
};

hwaa_result_t hwaa_create_fek(struct inode *inode, struct dentry *dentry,
	u8 **encoded_wfek, u32 *encoded_len, u8 **fek,
	u32 *fek_len, gfp_t kmem_flag)
{
	hwaa_result_t res;
	pid_t pid;
	uid_t uid;
	gid_t gid;
	const s8 *fsname;
	umode_t mode;
	const struct cred *cred;

	if (!inode || !dentry || !encoded_wfek || !encoded_len ||
	    !fek || !fek_len)
		return -HWAA_ERR_INVALID_ARGS;

	fsname = inode->i_sb->s_type->name;
	mode = inode->i_mode;
	pid = task_tgid_nr(current);
	cred = get_current_cred();
	uid = cred->uid.val;
	gid = cred->gid.val;
	put_cred(cred);
	down_read(&g_fs_callbacks_lock);
	res = g_fs_callbacks.create_fek(pid, uid, gid, fsname, dentry, mode,
		encoded_wfek, encoded_len, fek, fek_len, kmem_flag);
	up_read(&g_fs_callbacks_lock);

	return res;
}

hwaa_result_t hwaa_get_fek(struct inode *inode, const u8 *encoded_wfek,
	u32 encoded_len, u8 **fek, u32 *fek_len, gfp_t kmem_flag)
{
	hwaa_result_t res;
	uid_t uid;
	pid_t pid;
	const struct cred *cred;

	if (!inode || !encoded_wfek || encoded_len != HWAA_ENCODED_WFEK_SIZE ||
	    !fek || !fek_len) {
		res = -HWAA_ERR_INVALID_ARGS;
		goto out;
	}

	pid = task_tgid_nr(current);
	cred = get_current_cred();
	uid = cred->uid.val;
	put_cred(cred);

	down_read(&g_fs_callbacks_lock);
	res = g_fs_callbacks.get_fek(pid, uid, encoded_wfek, encoded_len, fek,
		fek_len, kmem_flag);
	up_read(&g_fs_callbacks_lock);

out:
	return res;
}

hwaa_result_t hwaa_has_access(const s8 *encoded_wfek, u32 encoded_len)
{
	hwaa_result_t res;
	pid_t pid;
	uid_t uid;
	const struct cred *cred;

	if (!encoded_wfek || (encoded_len != HWAA_ENCODED_WFEK_SIZE))
		return -HWAA_ERR_INVALID_ARGS;

	pid = task_tgid_nr(current);
	cred = get_current_cred();
	uid = cred->uid.val;
	put_cred(cred);

	down_read(&g_fs_callbacks_lock);
	res = g_fs_callbacks.hwaa_has_access(pid, uid, encoded_wfek, encoded_len);
	up_read(&g_fs_callbacks_lock);

	return res;
}

void hwaa_register_fs_callbacks(struct hwaa_fs_callbacks_t *callbacks)
{
	down_write(&g_fs_callbacks_lock);
	if (callbacks) {
		if (callbacks->create_fek)
			g_fs_callbacks.create_fek = callbacks->create_fek;
		if (callbacks->get_fek)
			g_fs_callbacks.get_fek = callbacks->get_fek;
		if (callbacks->hwaa_has_access)
			g_fs_callbacks.hwaa_has_access =
				callbacks->hwaa_has_access;
	}
	up_write(&g_fs_callbacks_lock);
}
EXPORT_SYMBOL(hwaa_register_fs_callbacks);

void hwaa_unregister_fs_callbacks(void)
{
	down_write(&g_fs_callbacks_lock);
	g_fs_callbacks.create_fek = default_create_fek;
	g_fs_callbacks.get_fek = default_get_fek;
	g_fs_callbacks.hwaa_has_access = default_hwaa_has_access;
	up_write(&g_fs_callbacks_lock);
}
EXPORT_SYMBOL(hwaa_unregister_fs_callbacks);

#endif

