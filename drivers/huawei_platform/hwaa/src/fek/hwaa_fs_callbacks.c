
#include "inc/fek/hwaa_fs_callbacks.h"
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/dcache.h>
#include <linux/mm_types.h>
#include <linux/file.h>
#include <linux/list.h>
#include <linux/rcupdate.h>
#include <linux/mm.h>
#include <securec.h>
#include "huawei_platform/hwaa/hwaa_limits.h"
#include "huawei_platform/hwaa/hwaa_ioctl.h"
#include "huawei_platform/hwaa/hwaa_file.h"
#include "inc/base/hwaa_utils.h"
#include "inc/data/hwaa_data.h"
#include "inc/data/hwaa_trusted_pids.h"
#include "inc/data/hwaa_packages.h"
#include "inc/fek/hwaa_fs_callbacks.h"
#include "inc/policy/hwaa_policy.h"
#include "inc/tee/wrappers.h"
#include "inc/base/macros.h"

#define VERSION_LENGTH 1
#define PHASE2_INDEX_LENGTH 32
#define AES_IV_LENGTH 16
#define FEK_LENGTH 64
#define SHA256_TAG_LENGTH 32
#define PERMS_LENGTH 8
#define APP_UID_LENGTH 4
#define PATH_NO_EXT_SDCARD 2
#define PATH_OF_INSTALLD "/system/bin/installd"
#define PATH_LEN_OF_INSTALLD 20

void safe_free(void **pointer)
{
	kzfree(*pointer);
	*pointer = NULL;
}

static hwaa_result_t get_path(struct dentry *dentry, s8 *path)
{
	s8 *path_tmp;
	hwaa_result_t ret = HWAA_SUCCESS;
	s8 *buf = kzalloc(PATH_MAX, GFP_NOFS);

	if (!buf)
		goto err;
	path_tmp = dentry_path_raw(dentry, buf, PATH_MAX);
	if (IS_ERR(path_tmp) || (strlen(path_tmp) >= PATH_MAX))
		goto err;
	if (memcpy_s(path, PATH_MAX, path_tmp, strlen(path_tmp)) != EOK)
		goto err;
	goto out;
err:
	ret = HWAA_ERR_NOT_SUPPORTED;
out:
	safe_free((void **)&buf);
	return ret;
}

static hwaa_result_t precheck_new_key_data(struct dentry *dentry, pid_t pid,
	uid_t uid, const s8 *fsname, u8 **app_data, umode_t mode, u64 *perms,
	s32 *app_data_len)
{
	struct package_hashnode_t *package_node;
	struct package_info_listnode_t *package_info_node = NULL;
	struct package_info_listnode_t *tmp = NULL;
	hwaa_result_t res;
	s8 *path = NULL;
	*app_data = NULL;
	hwaa_data_read_lock();
	if (!hwaa_trusted_pids_exists(HWAA_TRUSTED_APP, pid)) {
		res = -HWAA_ERR_NOT_SUPPORTED;
		hwaa_pr_warn_once("pid  %d not trusted", pid);
		goto out;
	}
	package_node = get_hwaa_package_hashnode(uid);
	if (package_node == NULL) {
		res = -HWAA_ERR_NOT_SUPPORTED;
		hwaa_pr_err("package empty");
		goto out;
	}
	path = kzalloc(PATH_MAX, GFP_NOFS);
	if (!path) {
		res = -HWAA_ERR_NOT_SUPPORTED;
		hwaa_pr_err("path alloc failed");
		goto out;
	}
	// must do this after pid check for better performance
	res = get_path(dentry, path);
	if (res != HWAA_SUCCESS)
		goto out;
	list_for_each_entry_safe(package_info_node, tmp,
		&package_node->pinfo_list, list) {
		if (hwaa_evaluate_policies(package_info_node, fsname,
			path, mode, perms)) {
			if (hwaa_packages_get_app_data(uid, package_info_node,
				app_data, app_data_len)) {
				res = -HWAA_ERR_UID_NOT_TRUST;
				hwaa_pr_err("uid(%d) not under protect\n", uid);
				goto out;
			}
			res = HWAA_SUCCESS; // success case
			goto out;
		}
	}
	res = -HWAA_ERR_NOT_SUPPORTED; // policy never matched
out:
	safe_free((void **)&path);
	hwaa_data_read_unlock(); // app_data no need to free, outside will do
	return res;
}

static hwaa_result_t precheck_existing_key_data(pid_t pid, uid_t uid,
	u8 **app_data, s32 *app_data_len)
{
	struct package_hashnode_t *package_node;
	struct package_info_listnode_t *package_info_node = NULL;
	hwaa_result_t res = HWAA_SUCCESS;
	*app_data = NULL;
	hwaa_data_read_lock();
	if (!hwaa_trusted_pids_exists(HWAA_TRUSTED_APP, pid)) {
		// Untrusted pid can only create unprotected file. If the file
		// should be protected but pid is untrusted, the file will be
		// unprotected.
		res = -HWAA_ERR_NOT_SUPPORTED;
		hwaa_pr_warn_once("pid  %d not trusted", pid);
		goto error;
	}
	package_node = get_hwaa_package_hashnode(uid);
	if (package_node == NULL) {
		res = -HWAA_ERR_NOT_SUPPORTED;
		hwaa_pr_err("package empty");
		goto error;
	}
	if (list_empty(&package_node->pinfo_list)) {
		res = -HWAA_ERR_NOT_SUPPORTED;
		hwaa_pr_err("list empty");
		goto error;
	}
	package_info_node = list_first_entry(&package_node->pinfo_list,
		struct package_info_listnode_t, list);
	if (hwaa_packages_get_app_data(uid, package_info_node, app_data,
		app_data_len)) {
		res = -HWAA_ERR_UID_NOT_TRUST;
		hwaa_pr_err("hwaa app(%d) is not under prot.\n", uid);
		goto error;
	} else {
		goto out;
	}
error:
	hwaa_data_read_unlock();
	safe_free((void **)app_data);
	return res;
out:
	hwaa_data_read_unlock();
	return res;
}

/* This function is called when a new inode is created. */
static bool is_create_fek_param_invalid(struct dentry *dentry,
	const u8 **encoded_wfek, const u32 *encoded_len, const u8 **fek,
	const u32 *fek_len)
{
	return (!dentry || !encoded_wfek || !encoded_len || !fek || !fek_len);
}

static hwaa_result_t create_fek_copy_appdata(u8 *app_data, s32 app_data_len,
	u64 *perms, gfp_t kmem_flag, u8 **app_data_with_permission)
{
	/* params are prechecked in createfek, so no checking params. */
	*app_data_with_permission = kzalloc(sizeof(struct appid_without_ext_signer_t) +
		sizeof(u64), kmem_flag);
	if (*app_data_with_permission == NULL) {
		hwaa_pr_err("hwaa malloc app_data_with_permisson failed.\n");
		return -HWAA_ERR_NO_MEMORY;
	}
	if (memcpy_s(*app_data_with_permission,
		sizeof(struct appid_without_ext_signer_t), app_data, app_data_len)
		!= EOK)
		return -HWAA_ERR_INTERNAL; // memory free outside
	if (memcpy_s(*app_data_with_permission +
		sizeof(struct appid_without_ext_signer_t), sizeof(u64),
		(void *)perms, sizeof(u64)) != EOK)
		return -HWAA_ERR_INTERNAL;  // memory free outside
	return HWAA_SUCCESS;
}

static hwaa_result_t create_fek_inner(uid_t uid, u8 *app_data, s32 app_data_len,
	u64 *perms, gfp_t kmem_flag, u8 **encoded_wfek, u8 **fek, u32 *fek_len,
	u32 *encoded_len)
{
	hwaa_result_t res;
	u8 *app_data_with_permission = NULL;
	u32 fek_len_tmp;
	u32 encoded_len_tmp;
	u8 *fek_tmp = NULL;

	res = create_fek_copy_appdata(app_data, app_data_len, perms, kmem_flag,
		&app_data_with_permission);
	if (res != HWAA_SUCCESS)
		goto done;
	fek_len_tmp = HWAA_FEK_SIZE;
	encoded_len_tmp = HWAA_ENCODED_WFEK_SIZE;
	fek_tmp = kzalloc(sizeof(s8) * (fek_len_tmp), GFP_NOFS);
	*encoded_wfek = kzalloc(sizeof(s8) * encoded_len_tmp, GFP_NOFS);
	if ((*encoded_wfek == NULL) || (fek_tmp == NULL)) {
		res = -HWAA_ERR_NO_MEMORY;
		goto err;
	}
	if (get_new_file_key(hwaa_utils_get_ausn(uid), *encoded_wfek,
		encoded_len_tmp, fek_tmp,
		HWAA_FEK_SIZE, app_data_with_permission,
		app_data_len + sizeof(u64), PATH_NO_EXT_SDCARD)
		!= ERR_MSG_SUCCESS) {
		res = -HWAA_ERR_INTERNAL;
		goto err;
	}
	*fek = fek_tmp;
	fek_tmp = NULL;
	*fek_len = fek_len_tmp;
	*encoded_len = encoded_len_tmp;
	goto done;
err:
	safe_free((void **)encoded_wfek);
done:
	safe_free((void **)&fek_tmp);
	safe_free((void **)&app_data_with_permission);
	return res;
}

static hwaa_result_t handle_create_fek(pid_t pid, uid_t uid, s32 gid,
	const s8 *fsname, struct dentry *dentry, umode_t mode,
	u8 **encoded_wfek, u32 *encoded_len, u8 **fek, u32 *fek_len,
	gfp_t kmem_flag)
{
	hwaa_result_t res;
	u8 *app_data = NULL;
	s32 app_data_len = 0;
	u64 perms;

	if (is_create_fek_param_invalid(dentry, (const u8 **)encoded_wfek,
		encoded_len, (const u8 **)fek, fek_len)) {
		return HWAA_ERR_INVALID_ARGS;
	}
	res = precheck_new_key_data(dentry, pid, uid, fsname, &app_data, mode,
		&perms, &app_data_len);
	if (res != HWAA_SUCCESS)
		goto out;
	if (app_data_len != sizeof(struct appid_without_ext_signer_t)) {
		hwaa_pr_err("app_data_len is invalid %d", app_data_len);
		goto out;
	}
	res = create_fek_inner(uid, app_data, app_data_len, &perms, kmem_flag,
		encoded_wfek, fek, fek_len, encoded_len);
	if (res != HWAA_SUCCESS)
		hwaa_pr_err("create_fek_inner returned %d", res);
	else
		hwaa_pr_debug("create_fek_inner returned success");
out:
	safe_free((void **)&app_data);
	return res;
}

/*
 * the following 2 functions are very unreasonable but we have
 * to write them to defend csec
 * app_id_s>>gotten from user pass  app_id_t>>gotten from htable
 * app_id_s from file xattr only have the latest certificate
 * app_id_t from htable may have multi certificates
 */
static bool is_owner_param_pointer_invalid(const u8 *app_data_s,
	const u8 *app_data_t)
{
	return ((app_data_s == NULL) || (app_data_t == NULL));
}

static bool is_owner_param_len_invalid(s32 len_app_data_s, s32 len_app_data_t)
{
	return ((len_app_data_t != len_app_data_s) ||
		(len_app_data_s != (s32)sizeof(struct appid_without_ext_signer_t)));
}

static bool is_owner_check_uid_and_profileid(
	struct appid_without_ext_signer_t app_data_s_data,
	struct appid_without_ext_signer_t app_data_t_data)
{
	if (app_data_s_data.profile_id != app_data_t_data.profile_id) {
		hwaa_pr_err("profile_id different s(%lld) t(%lld).\n",
			app_data_s_data.profile_id,
			app_data_t_data.profile_id);
		return false;
	}
	return true;
}

static bool is_owner_check_shuid_and_name(
	struct appid_without_ext_signer_t app_data_s_data,
	struct appid_without_ext_signer_t app_data_t_data)
{
	/*
	 * When the file's shared user id is not NULL, the package name of the
	 * file can be different from that of the app's, but the shared user id
	 * must be the same for both.
	 * Alternatively, when the file's shared user id is NULL, both file and
	 * app must have the same package name.
	 */
	// share uid is  signed
	const s8 empty_shuid[SHA256_HASH_SIZE] = {0};
	if (memcmp(app_data_s_data.shuid, empty_shuid, SHA256_HASH_SIZE)) {
		if (memcmp(app_data_s_data.shuid, app_data_t_data.shuid,
			SHA256_HASH_SIZE) == 0)
			return true;// no need to further check name

		return false;// share uid different return false directly
	}
	if (memcmp(app_data_s_data.name, app_data_t_data.name,
		SHA256_HASH_SIZE) != 0) {
		hwaa_pr_err("package name different.\n");
		return false;
	}
	return true;// name match
}

static bool is_owner(const u8 *app_data_s, s32 len_app_data_s, u8 *app_data_t,
	s32 len_app_data_t)
{
	struct appid_without_ext_signer_t app_data_s_data;
	struct appid_without_ext_signer_t app_data_t_data;

	if (is_owner_param_pointer_invalid(app_data_s, app_data_t) ||
		is_owner_param_len_invalid(len_app_data_s, len_app_data_t)) {
		return false;
	}
	if (memcpy_s(&app_data_s_data, sizeof(app_data_s_data), app_data_s,
		len_app_data_s) != EOK)
		return false;
	if (memcpy_s(&app_data_t_data, sizeof(app_data_t_data), app_data_t,
		(s32)sizeof(struct appid_without_ext_signer_t)) != EOK)
		return false;
	if (!is_owner_check_uid_and_profileid(app_data_s_data,
		app_data_t_data)) {
		return false;
	}
	return is_owner_check_shuid_and_name(app_data_s_data, app_data_t_data);
}

static void get_app_data_from_encoded_wfek(const u8 *encoded_wfek,
	u32 len_encoded_wfek, const u8 **app_data, u32 *len_app_data,
	u64 *profile_id)
{
	u32 app_data_offset = VERSION_LENGTH + PHASE2_INDEX_LENGTH +
		AES_IV_LENGTH + FEK_LENGTH;

	*len_app_data = len_encoded_wfek - PERMS_LENGTH - SHA256_TAG_LENGTH -
		app_data_offset;
	/* the *app_data should be changed before use profile_id */
	*app_data = encoded_wfek + app_data_offset;
	if (profile_id)
		*profile_id = *((u64 *)(*app_data + APP_UID_LENGTH));
}

#ifdef CONFIG_HWAA_ENG_DEBUG
static bool is_pid_privilege(pid_t pid)
{
	bool is_priv = false;
	s8 task_name[TASK_COMM_LEN] = {0};
	size_t priv_task_len;
	s32 priv_num;
	s32 i;
	static const s8 * const priv_cmds[] = {
		/*
		 * echo --> "sh"
		 * adb pull --> "sync svc 44", "sync svc 66"
		 */
		"cat", "sh", "sync svc"
	};
	get_task_comm(task_name, current);
	priv_num = ARRAY_SIZE(priv_cmds);
	for (i = 0; i < priv_num; ++i) {
		if (!strcmp(priv_cmds[i], task_name)) {
			is_priv = true;
			break;
		}
	}
	priv_task_len = strlen(priv_cmds[priv_num-1]);
	if (!strncmp(priv_cmds[priv_num-1], task_name, priv_task_len))
		is_priv = true;
	return is_priv;
}
#endif

static bool is_pid_installd(pid_t pid)
{
	return hwaa_utils_exe_check(pid, PATH_OF_INSTALLD,
		PATH_LEN_OF_INSTALLD);
}

static bool check_param_get_fek(const u8 *encoded_wfek, u32 encoded_len,
	u8 **fek, const u32 *fek_len)
{
	bool ret = true;

	if (!encoded_wfek || (encoded_len < HWAA_ENCODED_WFEK_SIZE) || !fek ||
		!fek_len)
		ret = false;
	return ret;
}

void check_ret_of_get_existing_file_key(hwaa_result_t *res_inout, u8 **fek,
	u32 *fek_len)
{
	if (*res_inout || !(*fek) || (*fek_len != HWAA_FEK_SIZE)) {
		if (*res_inout > 0) {
			*res_inout = -*res_inout;
			hwaa_pr_err("res %d fek len %d\n", *res_inout, *fek_len);
		}
	}
}

static hwaa_result_t handle_get_fek(pid_t pid, uid_t uid,
	const u8 *encoded_wfek, u32 encoded_len, u8 **fek, u32 *fek_len,
	gfp_t kmem_flag)
{
	hwaa_result_t res;
	u64 profile_id_from_encoded = 0;
	u8 *app_data = NULL;
	s32 app_data_len = 0;
	const u8 *app_data_xattr = NULL;
	s32 app_data_xattr_len;
	if (!check_param_get_fek(encoded_wfek, encoded_len, fek, fek_len))
		return HWAA_ERR_INVALID_ARGS;
	get_app_data_from_encoded_wfek(encoded_wfek, encoded_len,
		&app_data_xattr, &app_data_xattr_len, &profile_id_from_encoded);
	res = precheck_existing_key_data(pid, uid, &app_data, &app_data_len);
	if (res != HWAA_SUCCESS) {
		goto priv;
	}
	res = get_existing_file_key(hwaa_utils_get_ausn(uid), encoded_wfek,
		encoded_len, fek, fek_len);
	check_ret_of_get_existing_file_key(&res, fek, fek_len);
	if (res != HWAA_SUCCESS)
		goto err;
	if (!is_owner(app_data_xattr, app_data_xattr_len, app_data,
		app_data_len)) {
		res = -HWAA_ERR_NOT_OWNER;
		goto err;
	}
	res = HWAA_SUCCESS;
	goto out;
priv:
#ifdef CONFIG_HWAA_ENG_DEBUG
	if (((*fek == NULL) || (*fek_len == 0)) &&
		(is_pid_privilege(pid) || is_pid_installd(pid))) {
#else
	if (((*fek == NULL) || (*fek_len == 0)) && is_pid_installd(pid)) {
#endif
		res = get_existing_file_key(profile_id_from_encoded,
			(const u8 *)encoded_wfek, encoded_len, fek, fek_len);
		if (!res)
			goto out;
	}
err:
	safe_free((void **)fek);
	*fek_len = 0;
out:
	hwaa_pr_debug("hwaa open file get fek res (%d).\n", -res);
	safe_free((void **)&app_data);
	return (res > 0) ? -res : res;
}

static hwaa_result_t handle_hwaa_has_access(pid_t pid, uid_t uid,
	const s8 *encoded_wfek, u32 encoded_len)
{
	hwaa_result_t res;
	u8 *app_data = NULL;
	s32 app_data_len = 0;
	const u8 *app_data_xattr = NULL;
	u32 app_data_xattr_len = 0;

	if (!encoded_wfek || (encoded_len != HWAA_ENCODED_WFEK_SIZE))
		return -HWAA_ERR_INVALID_ARGS;
	res = precheck_existing_key_data(pid, uid, &app_data, &app_data_len);
	if (res != HWAA_SUCCESS)
		goto priv;
	get_app_data_from_encoded_wfek((u8 *)encoded_wfek, encoded_len,
		&app_data_xattr, &app_data_xattr_len, NULL);
	if (!is_owner(app_data_xattr, app_data_xattr_len, app_data,
		app_data_len)) {
		res = -HWAA_ERR_NOT_OWNER;
		goto out;
	}
	res = HWAA_SUCCESS;
	goto out;
priv:
#ifdef CONFIG_HWAA_ENG_DEBUG
	if (is_pid_privilege(pid) || is_pid_installd(pid))
#else
	if (is_pid_installd(pid))
#endif
		res = HWAA_SUCCESS;
out:
	safe_free((void **)&app_data);
	return res;
}

struct hwaa_fs_callbacks_t g_fs_callbacks = {
	.create_fek = handle_create_fek,
	.get_fek = handle_get_fek,
	.hwaa_has_access = handle_hwaa_has_access,
};

void hwaa_register_fs_callbacks_proxy(void)
{
	hwaa_register_fs_callbacks(&g_fs_callbacks);
}

void hwaa_unregister_fs_callbacks_proxy(void)
{
	hwaa_unregister_fs_callbacks();
}
