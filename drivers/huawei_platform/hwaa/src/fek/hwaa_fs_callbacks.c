
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
#include "inc/base/hwaa_utils.h"
#include "inc/fek/hwaa_fs_callbacks.h"
#include "inc/tee/hwaa_adapter.h"
#include "inc/base/hwaa_define.h"

#define VERSION_LENGTH 1
#define PHASE2_INDEX_LENGTH 32
#define AES_IV_LENGTH 16
#define FEK_LENGTH 64
#define SHA256_TAG_LENGTH 32
#define PERMS_LENGTH 8
#define APP_UID_LENGTH 4
#define PATH_OF_INSTALLD "/system/bin/installd"
#define PATH_LEN_OF_INSTALLD 20

void safe_free(void **pointer)
{
	kzfree(*pointer);
	*pointer = NULL;
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

static bool check_param_get_fek(const u8 *encoded_wfek, u32 encoded_len,
	u8 **fek, const u32 *fek_len)
{
	bool ret = true;

	if (!encoded_wfek || (encoded_len < HWAA_ENCODED_WFEK_SIZE) || !fek ||
		!fek_len)
		ret = false;
	return ret;
}

void check_ret_kernel_get_fek(hwaa_result_t *res_inout, u8 **fek,
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
	const u8 *app_data_xattr = NULL;
	s32 app_data_xattr_len;
	if (!check_param_get_fek(encoded_wfek, encoded_len, fek, fek_len))
		return HWAA_ERR_INVALID_ARGS;
	get_app_data_from_encoded_wfek(encoded_wfek, encoded_len,
		&app_data_xattr, &app_data_xattr_len, &profile_id_from_encoded);
	res = kernel_get_fek(hwaa_utils_get_ausn(uid), encoded_wfek,
		encoded_len, fek, fek_len);
	check_ret_kernel_get_fek(&res, fek, fek_len);
	if (res != HWAA_SUCCESS)
		goto priv;
	goto out;
priv:
	if ((*fek == NULL) || (*fek_len == 0)) {
		res = kernel_get_fek(profile_id_from_encoded,
			(const u8 *)encoded_wfek, encoded_len, fek, fek_len);
		if (!res)
			goto out;
	}
	safe_free((void **)fek);
	*fek_len = 0;
out:
	hwaa_pr_debug("hwaa open file get fek res (%d).\n", -res);
	safe_free((void **)&app_data); // kzfree counld recognize NULL
	return (res > 0) ? -res : res;
}

struct hwaa_fs_callbacks_t g_fs_callbacks = {
	.get_fek = handle_get_fek,
};

void hwaa_register_fs_callbacks_proxy(void)
{
	hwaa_register_fs_callbacks(&g_fs_callbacks);
}

void hwaa_unregister_fs_callbacks_proxy(void)
{
	hwaa_unregister_fs_callbacks();
}
