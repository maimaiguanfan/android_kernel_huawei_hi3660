/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2018. All rights reserved.
 * Description: the hw_kernel_stp_uploader.c for kernel data uploading through uevent
 * Author: sunhongqing <sunhongqing@huawei.com>
 * Create: 2018-03-31
 */

#include "hw_kernel_stp_uploader.h"

static const char *TAG = "kernel_stp_uploader";

static struct kobject *g_kernel_stp_kobj;
static struct kset *g_kernel_stp_kset;
static DEFINE_MUTEX(upload_mutex);

struct stp_item_info item_info[] = {
	[KCODE]        = { STP_ID_KCODE, STP_NAME_KCODE },
	[SYSCALL]      = { STP_ID_KCODE_SYSCALL, STP_NAME_KCODE_SYSCALL },
	[SE_ENFROCING] = { STP_ID_SE_ENFROCING, STP_NAME_SE_ENFROCING },
	[SE_HOOK]      = { STP_ID_SE_HOOK, STP_NAME_SE_HOOK },
	[ROOT_PROCS]   = { STP_ID_ROOT_PROCS, STP_NAME_ROOT_PROCS },
	[SETIDS]       = { STP_ID_SETIDS, STP_NAME_SETIDS },
	[EIMA]         = { STP_ID_EIMA, STP_NAME_EIMA },
	[MOD_SIGN]     = { STP_ID_MOD_SIGN, STP_NAME_MOD_SIGN },
	[PTRACE]       = { STP_ID_PTRACE, STP_NAME_PTRACE },
	[HKIP]         = { STP_ID_HKIP, STP_NAME_HKIP },
	[CFI]          = { STP_ID_CFI, STP_NAME_CFI },
	[ITRUSTEE]     = { STP_ID_ITRUSTEE, STP_NAME_ITRUSTEE },
	[DOUBLE_FREE]  = { STP_ID_DOUBLE_FREE, STP_NAME_DOUBLE_FREE },
};

struct stp_item_info *get_item_info_by_idx(int idx)
{
	if (idx < STP_ITEM_MAX) {
		return &item_info[idx];
	} else {
		return NULL;
	}
};

int kernel_stp_uploader_init(void)
{
	const char *kernel_stp_kobj_name = "hw_kernel_stp_scanner";
	const char *kernel_stp_kset_name = "hw_kernel_stp_kset";
	int ret;

	do {
		g_kernel_stp_kobj = kobject_create_and_add(kernel_stp_kobj_name,
							kernel_kobj);
		if (g_kernel_stp_kobj == NULL) {
			KSTPLogError(TAG, "creat kobject failed");
			ret = KSTP_ERRCODE;
			break;
		}

		g_kernel_stp_kset =  kset_create_and_add(kernel_stp_kset_name,
							NULL, kernel_kobj);
		if (g_kernel_stp_kset == NULL) {
			KSTPLogError(TAG, "creat kset failed");
			ret = KSTP_ERRCODE;
			break;
		}
		g_kernel_stp_kobj->kset = g_kernel_stp_kset;

		ret = kobject_uevent(g_kernel_stp_kobj, KOBJ_ADD);
		if (ret != 0) {
			KSTPLogError(TAG, "kobj_uevent add failed, result is %d", ret);
			break;
		}
	} while (0);

	if (ret != 0) {
		kernel_stp_uploader_exit();

		KSTPLogError(TAG, "kernel stp kobj init failed");
		return ret;
	}

	KSTPLogTrace(TAG, "kernel_stp_kobj_init ok!");
	return ret;
}

void kernel_stp_uploader_exit(void)
{
	if (g_kernel_stp_kobj != NULL) {
		kobject_put(g_kernel_stp_kobj);
		g_kernel_stp_kobj = NULL;
	}

	if (g_kernel_stp_kset != NULL) {
		kset_unregister(g_kernel_stp_kset);
		g_kernel_stp_kset = NULL;
	}

	KSTPLogTrace(TAG, "kernel_stp_kobj_deinit ok!");
}

int kernel_stp_upload_parse(struct stp_item result, char *addition_info,
			char *upload_info)
{
	if (upload_info == NULL) {
		KSTPLogError(TAG, "input arguments invalid");
		return -EINVAL;
	}
	if (addition_info == NULL) {
		snprintf(upload_info, STP_INFO_MAXLEN, "stpinfo=%u:%u:%u:%u:%s",
			result.id, result.status, result.credible,
			result.version, result.name);
	} else {
		snprintf(upload_info, STP_INFO_MAXLEN, "stpinfo=%u:%u:%u:%u:%s:%s",
			result.id, result.status, result.credible,
			result.version, result.name, addition_info);
	}
	return 0;
}

/* concatenate kernel_stp data of type int and type char array */
static int kernel_stp_data_adapter(char **uevent_envp, char *result)
{
	int index = 0;

	if ((uevent_envp == NULL) || (result == NULL)) {
		KSTPLogError(TAG, "input arguments invalid");
		return -EINVAL;
	}

	for (index = 0; index < KERNEL_STP_UEVENT_LEN - 1; index++) {
		uevent_envp[index] = result;
		KSTPLogDebug(TAG, "uevent_envp[%d] is %s",
				index, uevent_envp[index]);
	}

	return 0;
}

int kernel_stp_upload(struct stp_item result, char *addition_info)
{
	int ret;
	char *upload_info;
	char *uevent_envp[KERNEL_STP_UEVENT_LEN] = { NULL };

	if (g_kernel_stp_kobj == NULL) {
		KSTPLogDebug(TAG, "kernel stp kobj no creat");
		return KSTP_ERRCODE;
	}

	do {
		upload_info = kzalloc(STP_INFO_MAXLEN, GFP_KERNEL);
		if (upload_info == NULL) {
			KSTPLogError(TAG, "failed to alloc upload_info");
			return -EINVAL;
		}

		ret = kernel_stp_upload_parse(result, addition_info, upload_info);
		if (ret != 0) {
			KSTPLogError(TAG, "data parse failed, ret is %d", ret);
			break;
		}

		ret = kernel_stp_data_adapter(uevent_envp, upload_info);
		if (ret != 0) {
			KSTPLogError(TAG, "data adpter failed, ret is %d", ret);
			break;
		}

		ret = kobject_uevent_env(g_kernel_stp_kobj, KOBJ_CHANGE,
					uevent_envp);
		if (ret != 0) {
			KSTPLogError(TAG, "kobj upload failed, ret is %d", ret);
			break;
		}

		KSTPLogTrace(TAG, "event upload finished. result: %d", ret);
	} while (0);

	if (upload_info != NULL) {
		kfree(upload_info);
	}

	return ret;
}
