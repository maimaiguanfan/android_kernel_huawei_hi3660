
#ifndef _HWAA_FS_HOOKS_H_
#define _HWAA_FS_HOOKS_H_

#include <linux/types.h>
#include <linux/fs.h>
#include "huawei_platform/hwaa/hwaa_error.h"
#include "huawei_platform/hwaa/hwaa_ioctl.h"

#ifdef CONFIG_HWAA

/* filesystem callback functions */
struct hwaa_fs_callbacks_t {
	hwaa_result_t (*create_fek)(pid_t pid, uid_t uid, s32 gid,
		const s8 *fsname, struct dentry *dentry, umode_t mode,
		u8 **encoded_wfek, u32 *encoded_len, u8 **fek, u32 *fek_len,
		gfp_t kmem_flag);
	hwaa_result_t (*get_fek)(pid_t pid, uid_t uid, const u8 *encoded_wfek,
		u32 encoded_len, u8 **fek, u32 *fek_len,
		gfp_t kmem_flag);
	hwaa_result_t (*hwaa_has_access)(pid_t pid, uid_t uid,
		const s8 *encoded_wfek, u32 encoded_len);
};

/*
 * Function: hwaa_fs_create_fek
 * Description: Creates a new file encryption key. Note: memory for the output
 *              parameters encoded_wfek and fek will be allocated inside the
 *              function and it is the responsibility of the caller to release
 *              this memory. Furthermore, the contents of the memory address
 *              pointed to by the output parameters encoded_len and fek_len
 *              will be overwritten.
 * Input: inode: the inode requesting encoded_wfek and fek
 *        path: full path name of the new inode
 *        kmem_flag: kernel memory malloc flag
 * Output: encoded_wfek: wrapped (i.e., encrypted) file encryption key with aad
 *         encoded_len: length of encoded_wfek
 *         fek: raw file encryption key
 *         fek_len: size of raw file encryption key
 * Return: HWAA_SUCCESS: successfully protected by hwaa
 *         -HWAA_ERR_NOT_SUPPORTED:  not protected by hwaa
 *         -HWAA_ERR_NO_FS_CALLBACKS: should wait?
 *         other negative value: error
 */
hwaa_result_t hwaa_create_fek(struct inode *inode, struct dentry *dentry,
	u8 **encoded_wfek, u32 *encoded_len, u8 **fek, u32 *fek_len,
	gfp_t kmem_flag);

/*
 * Function: hwaa_fs_get_fek
 * Description: Decrypts a file encryption key. Note: memory for the output
 *              parameter fek will be allocated inside the function and it
 *              is the responsibility of the caller to release this memory.
 *              Furthermore, the contents of the memory address pointed to
 *              by the output parameter fek_len will be overwritten.
 * Input: inode: the inode requesting fek
 *        encoded_wfek: the wrapped file encryption key with aad
 *        encoded_len: length of encoded_wfek
 *        kmem_flag: kernel memory malloc flag
 * Output: fek: raw file encryption key
 *         fek_len: size of raw file encryption key
 * Return: HWAA_SUCCESS: successfully get raw fek
 *         -HWAA_ERR_NOT_SUPPORTED:  not protected by hwaa
 *         -HWAA_ERR_NO_FS_CALLBACKS: should wait?
 *         other negative value: error
 */
hwaa_result_t hwaa_get_fek(struct inode *inode, const u8 *encoded_wfek,
	u32 encoded_len, u8 **fek, u32 *fek_len, gfp_t kmem_flag);

hwaa_result_t hwaa_has_access(const s8 *encoded_wfek, u32 encoded_len);


/*
 * Function: hwaa_register_fs_callbacks
 * Description: Loads filesystem callback functions.
 * Input: struct hwaa_fs_callbacks_t *callbacks: callback functions to be
 *        registered
 * Output: None
 * Return: None
 */
void hwaa_register_fs_callbacks(struct hwaa_fs_callbacks_t *callbacks);

/*
 * Function: hwaa_unregister_fs_callbacks
 * Description: Unloads filesystem callback functions.
 * Input: None
 * Output: None
 * Return: None
 */
void hwaa_unregister_fs_callbacks(void);

#endif /* CONFIG_HWAA */
#endif
