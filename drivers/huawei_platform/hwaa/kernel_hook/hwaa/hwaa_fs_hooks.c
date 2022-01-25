
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

static hwaa_result_t default_get_fek(pid_t pid, uid_t uid, const u8 *encoded_wfek,
	u32 encoded_len, u8 **fek, u32 *fek_len, gfp_t kmem_flag)
{
	return -HWAA_ERR_NO_FS_CALLBACKS;
}

static struct hwaa_fs_callbacks_t g_fs_callbacks = {
	.get_fek = default_get_fek,
};

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

void hwaa_register_fs_callbacks(struct hwaa_fs_callbacks_t *callbacks)
{
	down_write(&g_fs_callbacks_lock);
	if (callbacks) {
		if (callbacks->get_fek)
			g_fs_callbacks.get_fek = callbacks->get_fek;
	}
	up_write(&g_fs_callbacks_lock);
}
EXPORT_SYMBOL(hwaa_register_fs_callbacks);

void hwaa_unregister_fs_callbacks(void)
{
	down_write(&g_fs_callbacks_lock);
	g_fs_callbacks.get_fek = default_get_fek;
	up_write(&g_fs_callbacks_lock);
}
EXPORT_SYMBOL(hwaa_unregister_fs_callbacks);

#endif

