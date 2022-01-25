/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2018. All rights reserved.
 * Description: the hw_rscan_scanner.c for kernel space root scan
 * Author: likun <quentin.lee@huawei.com>
 *         likan <likan82@huawei.com>
 * Create: 2016-06-18
 */

#include <chipset_common/security/hw_kernel_stp_interface.h>
#include "./include/hw_rscan_scanner.h"
#include "./include/hw_rscan_utils.h"
#include "./include/hw_rscan_whitelist.h"

#define VAR_NOT_USED(variable)  do { (void)(variable); } while (0)
#define KCODE_OFFSET 0
#define SYSCALL_OFFSET 1
#define SEHOOKS_OFFSET 2
#define SESTATUS_OFFSET 3

static DEFINE_MUTEX(scanner_lock);      /* lint -save -e64 -e785 -e708 -e570 */
static DEFINE_MUTEX(whitelist_lock);    /* lint -save -e64 -e785 -e708 -e570 */
static const char *TAG = "hw_rscan_scanner";
static const char *DEFAULT_PROC = "/init";
static char *g_whitelist_proc = RPROC_WHITE_LIST_STR;
static int g_rs_data_init = RSCAN_UNINIT;
static int g_root_scan_hot_fix;

static struct item_bits itembits[MAX_NUM_OF_ITEM] = {
	// kcode
	{
		RS_KCODE,
		KERNELCODEBIT,
		D_RSOPID_KCODE,
	},
	// syscall
	{
		RS_SYS_CALL,
		SYSTEMCALLBIT,
		D_RSOPID_SYS_CALL,
	},
	// selinux
	{
		RS_SE_STATUS,
		SESTATUSBIT,
		D_RSOPID_SE_STATUS,
	},
	// se_hook
	{
		RS_SE_HOOKS,
		SEHOOKBIT,
		D_RSOPID_SE_HOOKS,
	},
	// root_proc
	{
		RS_RRPOCS,
		ROOTPROCBIT,
		D_RSOPID_RRPOCS,
	},
	// set_id
	{
		RS_SETID,
		SETIDBIT,
		D_RSOPID_SETID,
	},
};

struct rscan_skip_flags g_rscan_skip_flag = {
	.skip_kcode = NOT_SKIP,
	.skip_syscall = NOT_SKIP,
	.skip_se_hooks = NOT_SKIP,
	.skip_se_status = NOT_SKIP,
	.skip_rprocs = NOT_SKIP,
	.skip_setid = NOT_SKIP,
};

static struct rscan_result_dynamic g_rscan_clean_scan_result;
#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
static int g_r_p_flag;
static int g_test_count;
static struct rscan_result_dynamic g_rscan_orig_result;
#define FAULT_PRIVATE_LENTH 500
struct fault_private {
	size_t len;
	char buf[FAULT_PRIVATE_LENTH];
};
#endif

static int rscan_trigger_by_stp(char *upload_rootproc)
{
	int scan_err_code = 0;
	int root_masks;
	int dynamic_ops;
	int root_proc_length;
	struct rscan_result_dynamic *scan_result_buf;

	scan_result_buf = vmalloc(sizeof(struct rscan_result_dynamic));
	if (scan_result_buf == NULL) {
		RSLogError(TAG, "no enough space for scan_result_buf");
		return -ENOSPC;
	}
	memset(scan_result_buf, 0, sizeof(struct rscan_result_dynamic));

	dynamic_ops = RSOPID_ALL;
	mutex_lock(&scanner_lock);
	root_masks = rscan_dynamic(dynamic_ops, scan_result_buf, &scan_err_code);
	mutex_unlock(&scanner_lock);
	if (root_masks != 0) {
		RSLogDebug(TAG, "root status trigger by stp is %d.",
			root_masks);
	}

	if ((upload_rootproc != NULL) &&
		(strlen(scan_result_buf->rprocs) > 0)) {
		root_proc_length = strnlen(scan_result_buf->rprocs,
					sizeof(scan_result_buf->rprocs));
		if (root_proc_length >= RPROC_VALUE_LEN_MAX) {
			root_proc_length = RPROC_VALUE_LEN_MAX - 1;
			scan_result_buf->rprocs[root_proc_length] = '\0';
		}
		strncpy(upload_rootproc, scan_result_buf->rprocs,
				RPROC_VALUE_LEN_MAX);
	}

	vfree(scan_result_buf);

	return root_masks;
}

static int get_credible_of_item(int item_ree_status, int item_tee_status)
{
	if ((item_ree_status == 0) && (item_tee_status == 1)) {
		return STP_REFERENCE;
	} else {
		return STP_CREDIBLE;
	}
}

static int need_to_upload(unsigned int masks, unsigned int mask,
			int ree_status, int tee_status, int flag)
{
	if (flag == 1) {
		return 1;
	}

	if ((masks & mask) && ((ree_status != 0) || (tee_status != 0))) {
		return 1;
	}

	return 0;
}

/* flag = 0, just upload the abnormal items; flag = 1, upload all items */
static void upload_to_stp(int ree_status, int tee_status,
			char *rootproc, unsigned int mask, int flag)
{
	int item_status;
	int item_version = 0;
	int item_credible;
	int item_tee_status;
	int need_upload;
	int i;

	struct stp_item_info *stp_item_info = NULL;
	struct stp_item item = { 0 };

	for (i = 0; i < MAX_NUM_OF_ITEM; i++) {
		item_status = check_status(ree_status,
					itembits[i].item_ree_bit);
		item_tee_status = check_status(tee_status,
					itembits[i].item_tee_bit);
		need_upload = need_to_upload(mask, itembits[i].item_ree_mask,
					item_status, item_tee_status, flag);
		if (need_upload == 0) {
			continue;
		}
		stp_item_info = get_item_info_by_idx(i);
		if (stp_item_info == NULL) {
			RSLogError(TAG, "idx is %d, get item info by index failed", i);
			return;
		}

		item_credible = get_credible_of_item(item_status, item_tee_status);
		if ((i == ROOT_PROCS) || (i == SE_HOOK)) {
			item_credible = STP_REFERENCE;
		}

		if (i == KCODE) {
			if ((item_credible == STP_REFERENCE) &&
					(g_root_scan_hot_fix != 0)) {
				item_credible = STP_CREDIBLE;
			}
		}

		item.id = stp_item_info->id;
		item.status = item_status;
		item.credible = item_credible;
		item.version = item_version;

		if (strlen(stp_item_info->name) >= STP_ITEM_NAME_LEN) {
			RSLogError(TAG, "the length of the item name [%s] has exceeded the max allowed value",
				stp_item_info->name);
			return;
		}
		strncpy(item.name, stp_item_info->name, STP_ITEM_NAME_LEN - 1);
		item.name[STP_ITEM_NAME_LEN - 1] = '\0';

		if (i == ROOT_PROCS) {
			(void)kernel_stp_upload(item, rootproc);
		} else {
			(void)kernel_stp_upload(item, NULL);
		}
	}

	return;
}

int stp_rscan_trigger(void)
{
	int ree_status;
	int tee_status;
	char *upload_rootproc;

	upload_rootproc = kzalloc(RPROC_VALUE_LEN_MAX, GFP_KERNEL);
	if (upload_rootproc == NULL) {
		RSLogError(TAG, "failed to alloc upload_rootproc");
		return -ENOSPC;
	}

	ree_status = rscan_trigger_by_stp(upload_rootproc);
	tee_status = get_tee_status();

	/* 1 is flag, 1 mean upload all items */
	upload_to_stp(ree_status, tee_status, upload_rootproc, RSOPID_ALL, 1);

	kfree(upload_rootproc);

	return 0;
}

static int rscan_dynamic_raw_unlock(uint op_mask,
				struct rscan_result_dynamic *result)
{
	int ret = 0;
	int error_code = 0;

#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
	if (g_r_p_flag == 1) {
		if (g_rscan_skip_flag.skip_kcode == SKIP) {
			RSLogDebug(TAG, "skip kcode scan.");
		}
		if (g_rscan_skip_flag.skip_syscall == SKIP) {
			RSLogDebug(TAG, "skip syscall scan.");
		}
		if (g_rscan_skip_flag.skip_se_hooks == SKIP) {
			RSLogDebug(TAG, "skip se hooks scan.");
		}
		if (g_rscan_skip_flag.skip_se_status == SKIP) {
			RSLogDebug(TAG, "skip se status scan.");
		}
		if (g_rscan_skip_flag.skip_setid == SKIP) {
			RSLogDebug(TAG, "skip setid scan.");
		}
	}
#endif

	if (op_mask & D_RSOPID_KCODE) {
		ret = kcode_scan(result->kcode);
		if (ret != 0) {
			error_code |= D_RSOPID_KCODE;
			RSLogError(TAG, "kcode_scan failed");
		}
	}

	if (op_mask & D_RSOPID_SYS_CALL) {
		ret = kcode_syscall_scan(result->syscalls);
		if (ret != 0) {
			error_code |= D_RSOPID_SYS_CALL;
			RSLogError(TAG, "kcode system call scan failed");
		}
	}

	if (op_mask & D_RSOPID_SE_HOOKS) {
		ret = sescan_hookhash(result->sehooks);
		if (ret != 0) {
			error_code |= D_RSOPID_SE_HOOKS;
			RSLogError(TAG, "sescan_hookhash scan failed");
		}
	}

	if (op_mask & D_RSOPID_SE_STATUS) {
		result->seenforcing = get_selinux_enforcing();
	}

	if (op_mask & D_RSOPID_RRPOCS) {
#ifdef CONFIG_RSCAN_SKIP_RPROCS
		strncpy(result->rprocs, "/init", sizeof(result->rprocs));
#else
		ret = get_root_procs(result->rprocs, sizeof(result->rprocs));
		if (ret == 0) {
			error_code |= D_RSOPID_RRPOCS;
			RSLogError(TAG, "root processes scan failed!");
		}
#endif
	}

	if (op_mask & D_RSOPID_SETID) {
		result->setid = get_setids();
	}

	return error_code;
}

/* return: mask of abnormal scans items result */
int rscan_dynamic(uint op_mask, struct rscan_result_dynamic *result,
		int *error_code)
{
	int bad_mask = 0;

	if ((result == NULL) || (error_code == NULL)) {
		RSLogError(TAG, "input parameters error!");
		return -EINVAL;
	}

	*error_code = rscan_dynamic_raw_unlock(op_mask, result);
	if (*error_code != 0) {
		RSLogWarning(TAG, "some item of root scan failed");
	}

	if ((op_mask & D_RSOPID_KCODE) &&
		(g_rscan_skip_flag.skip_kcode == NOT_SKIP) &&
		(memcmp(result->kcode, g_rscan_clean_scan_result.kcode,
					sizeof(result->kcode)) != 0)) {
		bad_mask |= D_RSOPID_KCODE;
		RSLogDebug(TAG, "kernel code is abnormal");
	}

	if ((op_mask & D_RSOPID_SYS_CALL) &&
		(g_rscan_skip_flag.skip_syscall == NOT_SKIP) &&
		(memcmp(result->syscalls, g_rscan_clean_scan_result.syscalls,
			sizeof(result->syscalls)) != 0)) {
		bad_mask |= D_RSOPID_SYS_CALL;
		RSLogDebug(TAG, "kernel system call is abnormal");
	}

	if ((op_mask & D_RSOPID_SE_HOOKS) &&
		(g_rscan_skip_flag.skip_se_hooks == NOT_SKIP) &&
		(memcmp(result->sehooks, g_rscan_clean_scan_result.sehooks,
			sizeof(result->sehooks)) != 0)) {
		bad_mask |= D_RSOPID_SE_HOOKS;
		RSLogDebug(TAG, "SeLinux hooks is abnormal");
	}

	if ((op_mask & D_RSOPID_SE_STATUS) &&
			(g_rscan_skip_flag.skip_se_status == NOT_SKIP) &&
			(result->seenforcing !=
			 g_rscan_clean_scan_result.seenforcing)) {
		bad_mask |= D_RSOPID_SE_STATUS;
		RSLogDebug(TAG, "SeLinux enforcing status is abnormal");
	}

	if ((op_mask & D_RSOPID_RRPOCS) &&
			(g_rscan_skip_flag.skip_rprocs == NOT_SKIP)) {
		rprocs_strip_whitelist(result->rprocs,
				(ssize_t)sizeof(result->rprocs));
		if (result->rprocs[0]) {
			bad_mask |= D_RSOPID_RRPOCS;
			RSLogDebug(TAG, "root processes are abnormal");
		}
	}

	if ((op_mask & D_RSOPID_SETID) &&
			(g_rscan_skip_flag.skip_se_status == NOT_SKIP) &&
			(result->setid != g_rscan_clean_scan_result.setid)) {
		bad_mask |= D_RSOPID_SETID;
		RSLogDebug(TAG, "SeLinux enforcing status is abnormal");
	}

	RSLogTrace(TAG, "root scan finished.");
	return bad_mask;
}

/* just get the measurement, return the error mask */
int rscan_dynamic_raw(uint op_mask, struct rscan_result_dynamic *result)
{
	int error_code;

	if (result == NULL) {
		RSLogError(TAG, "input parameter is invalid");
		return -EINVAL;
	}

	mutex_lock(&scanner_lock);
	error_code = rscan_dynamic_raw_unlock(op_mask, result);
	mutex_unlock(&scanner_lock);

	return error_code;
}

/* call by CA to send dynamic measurement and upload abnormal item */
int rscan_dynamic_raw_and_upload(uint op_mask,
				struct rscan_result_dynamic *result)
{
	int ree_status;
	int tee_status;
	int error_code = 0;

	if (result == NULL) {
		RSLogError(TAG, "input parameter is invalid");
		return -EINVAL;
	}

	mutex_lock(&scanner_lock);
	ree_status = rscan_dynamic(op_mask, result, &error_code);
	mutex_unlock(&scanner_lock);

	tee_status = get_tee_status();

	/* 0 in upload_to_stp mean just upload abnormal items */
	if ((ree_status != 0) || (tee_status != 0)) {
		upload_to_stp(ree_status, tee_status, NULL, op_mask, 0);
	}

	return error_code;
}

#ifdef CONFIG_ARCH_MSM
int get_battery_status(int *is_charging, int *percentage)
{
	union power_supply_propval status;
	union power_supply_propval capacity;
	struct power_supply *psy = power_supply_get_by_name(BATTERY_NAME);

	if (psy == NULL) {
		return -EINVAL;
	}

	if (is_charging &&
		!psy->get_property(psy, POWER_SUPPLY_PROP_STATUS, &status)) {
		*is_charging = (status.intval == POWER_SUPPLY_STATUS_CHARGING) ||
				(status.intval == POWER_SUPPLY_STATUS_FULL);
	}

	if (percentage &&
		!psy->get_property(psy, POWER_SUPPLY_PROP_CAPACITY, &capacity)) {
		*percentage = capacity.intval;
	}

	return 0;
}
#else
int get_battery_status(int *is_charging, int *percentage)
{
	union power_supply_propval status;
	union power_supply_propval capacity;
	struct power_supply *psy;

	if ((is_charging == NULL) || (percentage == NULL)) {
		RSLogError(TAG, "input parameters invalid");
		return -EINVAL;
	}

	psy = power_supply_get_by_name(BATTERY_NAME);
	if (psy == NULL) {
		return -1;
	}

	/* is_charging never be NULL because of input parameters check */
	if (!power_supply_get_property(psy, POWER_SUPPLY_PROP_STATUS, &status)) {
		*is_charging = (status.intval == POWER_SUPPLY_STATUS_CHARGING) ||
				(status.intval == POWER_SUPPLY_STATUS_FULL);
	}

	/* percentage never be NULL because of input parameters check */
	if (!power_supply_get_property(psy, POWER_SUPPLY_PROP_CAPACITY, &capacity)) {
		*percentage = capacity.intval;
	}

	return 0;
}
#endif

int rscan_get_status(struct rscan_status *status)
{
	int is_charging = 0;
	int percentage = 0;
	int result = 0;
	struct timeval tv;

	if (status == NULL) {
		RSLogError(TAG, "input parameter is invalid");
		return -1;
	}

	status->cpuload = 0;

	if (get_battery_status(&is_charging, &percentage) == 0) {
		status->battery  = (uint32_t)percentage;
		status->charging = (uint32_t)is_charging;
	} else {
		RSLogWarning(TAG, "rootscan: get_battery_status failed");
		status->battery  = 0;
		status->charging = 0;
		result = 1;
	}

	do_gettimeofday(&tv);
	status->time = (uint32_t)tv.tv_sec;
	status->timezone = (uint32_t)sys_tz.tz_minuteswest;

	return result;
}

int load_rproc_whitelist(char *whitelist, size_t len)
{
	size_t min_len = strlen(g_whitelist_proc);

	if (whitelist == NULL) {
		RSLogError(TAG, "input parameter is invalid");
		return -EINVAL;
	}

	if (min_len >= len) {
		RSLogWarning(TAG, "The g_whitelist_proc lenth is too long");
		return -1;
	} else if (min_len <= 0) {
		RSLogWarning(TAG, "g_whitelist_proc is null");
		return -1;
	} else {
		strncpy(whitelist, g_whitelist_proc, min_len);
		whitelist[min_len] = '\0';
	}

	return 0;
}

int rscan_init_data(void)
{
	int ret;

	/* initialize g_rscan_clean_scan_result */
	memset(&g_rscan_clean_scan_result, 0, sizeof(struct rscan_result_dynamic));

	/* 1:selinux closed  0:selinux opened */
	g_rscan_clean_scan_result.seenforcing = 1;
	g_rscan_clean_scan_result.setid = get_setids();

	ret = load_rproc_whitelist(g_rscan_clean_scan_result.rprocs,
				sizeof(g_rscan_clean_scan_result.rprocs));
	if ((ret != 0) || (!init_rprocs_whitelist(g_rscan_clean_scan_result.rprocs))) {
		RSLogError(TAG, "load root whitelist failed, rproc will skip");
		strncpy(g_rscan_clean_scan_result.rprocs, DEFAULT_PROC,
			strlen(DEFAULT_PROC));
		g_rscan_skip_flag.skip_rprocs = SKIP;
	}

	ret = rscan_dynamic_raw(D_RSOPID_KCODE |
				D_RSOPID_SYS_CALL |
				D_RSOPID_SE_HOOKS,
				&g_rscan_clean_scan_result);
	if (ret != 0) {
		if (ret & D_RSOPID_KCODE) {
			RSLogError(TAG, "rscan D_RSOPID_KCODE init failed");
			g_rscan_skip_flag.skip_kcode = SKIP;
		}

		if (ret & D_RSOPID_SYS_CALL) {
			RSLogError(TAG, "rscan D_RSOPID_SYS_CALL init failed");
			g_rscan_skip_flag.skip_syscall = SKIP;
		}

		if (ret & D_RSOPID_SE_HOOKS) {
			RSLogError(TAG, "rscan D_RSOPID_SE_HOOKS init failed");
			g_rscan_skip_flag.skip_se_hooks = SKIP;
		}
	}

	g_rs_data_init = RSCAN_INIT;
	return 0;
}

int rscan_trigger(void)
{
	int result = stp_rscan_trigger();

	RSLogTrace(TAG, "scan and upload finished. result: %d", result);
	return result;
}

static int dynamic_call(unsigned int mask)
{
	int root_status = rscan_dynamic_raw_unlock(mask,
						&g_rscan_clean_scan_result);

	RSLogDebug(TAG, "set %d scan resume", mask);

	if (root_status != 0) {
		return root_status;
	}

	if (mask & D_RSOPID_KCODE) {
		g_rscan_skip_flag.skip_kcode = NOT_SKIP;
	}

	if (mask & D_RSOPID_SYS_CALL) {
		g_rscan_skip_flag.skip_syscall = NOT_SKIP;
	}

	if (mask & D_RSOPID_SE_HOOKS) {
		g_rscan_skip_flag.skip_se_hooks = NOT_SKIP;
	}

	if (mask & D_RSOPID_SE_STATUS) {
		g_rscan_skip_flag.skip_se_status = NOT_SKIP;
	}

	return root_status;
}

/* @reserved is reserved parameters for external module */
static int __root_scan_pause(unsigned int op_mask, void *reserved)
{
	VAR_NOT_USED(reserved);

	g_rscan_skip_flag.skip_kcode     = SKIP &
			((op_mask & D_RSOPID_KCODE) >> KCODE_OFFSET);
	g_rscan_skip_flag.skip_syscall   = SKIP &
			((op_mask & D_RSOPID_SYS_CALL) >> SYSCALL_OFFSET);
	g_rscan_skip_flag.skip_se_hooks  = SKIP & ((op_mask &
			D_RSOPID_SE_HOOKS) >> SEHOOKS_OFFSET);
	g_rscan_skip_flag.skip_se_status = SKIP &
			((op_mask & D_RSOPID_SE_STATUS) >> SESTATUS_OFFSET);

	RSLogDebug(TAG, "set scan pause, pause mask %d", op_mask);

#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
	g_r_p_flag = 1;
#endif

	return 0;
}

/* @reserved is reserved parameters for external module */
static int __root_scan_resume(unsigned int op_mask, void *reserved)
{
	unsigned int resume_mask = 0;

	VAR_NOT_USED(reserved);
#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
	g_r_p_flag = 0;
#endif

	if ((op_mask & D_RSOPID_KCODE) &&
			(g_rscan_skip_flag.skip_kcode == SKIP)) {
		resume_mask |= D_RSOPID_KCODE;
	}

	if ((op_mask & D_RSOPID_SYS_CALL) &&
			(g_rscan_skip_flag.skip_syscall == SKIP)) {
		resume_mask |= D_RSOPID_SYS_CALL;
	}

	if ((op_mask & D_RSOPID_SE_HOOKS) &&
			(g_rscan_skip_flag.skip_se_hooks == SKIP)) {
		resume_mask |= D_RSOPID_SE_HOOKS;
	}

	if ((op_mask & D_RSOPID_SE_STATUS) &&
			(g_rscan_skip_flag.skip_se_status == SKIP)) {
		resume_mask |= D_RSOPID_SE_STATUS;
	}

	return dynamic_call(resume_mask);
}

/* @reserved is reserved parameters for external module */
int root_scan_pause(unsigned int op_mask, void *reserved)
{
	int scan_err_code = 0;
	int result = 0;
	int root_status;
	int dynamic_ops;
	struct rscan_result_dynamic *scan_result_buf;
	struct timeval tv;

	VAR_NOT_USED(reserved);

	do_gettimeofday(&tv);
	RSLogTrace(TAG, "pause item:%d, time:%ld:%ld",
		op_mask, tv.tv_sec, tv.tv_usec);

	mutex_lock(&scanner_lock);
	scan_result_buf = vmalloc(sizeof(struct rscan_result_dynamic));
	if (scan_result_buf == NULL) {
		RSLogError(TAG, "no enough space for scan_result_buf");
		mutex_unlock(&scanner_lock);
		return -ENOSPC;
	}
	memset(scan_result_buf, 0, sizeof(struct rscan_result_dynamic));

	/* do scan before pause rootscan */
	dynamic_ops = D_RSOPID_KCODE | D_RSOPID_SYS_CALL |
			D_RSOPID_SE_HOOKS | D_RSOPID_SE_STATUS;
	root_status = rscan_dynamic(dynamic_ops, scan_result_buf, &scan_err_code);
	if (root_status == 0) {
		RSLogTrace(TAG, "environment clean ,pause root scan go");
		result = __root_scan_pause(op_mask, reserved);
	} else {
		RSLogTrace(TAG, "already rooted, skip pause.");
		result = root_status;
	}
	mutex_unlock(&scanner_lock);

	/* scan_result_buf never evaluates to NULL */
	vfree(scan_result_buf);

	return result;
}

/* @reserved is reserved parameters for external module */
int root_scan_resume(unsigned int op_mask, void *reserved)
{
	struct timeval tv;
	int result;

	VAR_NOT_USED(reserved);

	g_root_scan_hot_fix = 1;    /* have been done HotFix */

	do_gettimeofday(&tv);
	RSLogTrace(TAG, "resume item:%d, time:%ld:%ld",
		op_mask, tv.tv_sec, tv.tv_usec);

	mutex_lock(&scanner_lock);
	result = __root_scan_resume(op_mask, reserved);
	mutex_unlock(&scanner_lock);
	return result;
}

#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
static ssize_t copy_private_data_to_user(struct file *file,
				char __user *buf, size_t count, loff_t *offp)
{
	struct fault_private *priv = file->private_data;
	size_t tocopy = 0;

	if (priv == NULL) {
		RSLogError(TAG, "priv is NULL");
		return -ENOMEM;
	}

	if (*offp < priv->len) {
		tocopy = ((priv->len - *offp) < count) ? (priv->len - *offp) : count;

		if (copy_to_user(buf, priv->buf + *offp, tocopy)) {
			RSLogError(TAG, "copy_to_user failed");
			return -ENOMEM;
		}

		*offp += tocopy;
	}
	return tocopy;
}

static int open_private_file(struct file *file, char *strBuf)
{
	struct fault_private *priv;

	WARN_ON(file->private_data);
	priv = vmalloc(sizeof(struct fault_private));
	if (priv == NULL) {
		return -ENOMEM;
	}
	file->private_data = priv;
	memset(priv, 0, sizeof(struct fault_private));
	priv->len = strlen(strBuf);
	if (priv->len >= FAULT_PRIVATE_LENTH) {
		return -EINVAL;
	}

	strncpy(priv->buf, strBuf, priv->len);

	return 0;
}

static int kcode_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "kernel code scan testing\n");

	RSLogDebug(TAG, "root scan kernel code result:%d", ret);
	return ret;
}

static ssize_t kcode_read(struct file *file, char __user *buf,
			size_t count, loff_t *offp)
{
	memset(g_rscan_clean_scan_result.kcode, 0,
		sizeof(g_rscan_clean_scan_result.kcode));
	RSLogDebug(TAG, "rootscan: dead_code injected\n");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int kcode_release(struct inode *inode, struct file *file)
{
	vfree(file->private_data);
	file->private_data = NULL;
	RSLogDebug(TAG, "xxx. kcode_release succ!");
	return 0;
}

static int rs_pause_test_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t rs_pause_test_read(struct file *file, char __user *buf,
				size_t count, loff_t *offp)
{
	int test = 0;
	int pause_result = root_scan_pause(D_RSOPID_KCODE | D_RSOPID_SYS_CALL |
				D_RSOPID_SE_HOOKS | D_RSOPID_SE_STATUS, &test);
	g_test_count++;
	RSLogDebug(TAG, "root scan test, rs_pause_test, count:%d, result:%d",
			g_test_count, pause_result);

	return 0;
}

static int rs_pause_test_release(struct inode *inode, struct file *file)
{
	vfree(file->private_data);
	file->private_data = NULL;
	RSLogDebug(TAG, "xxx. pause release succ!");
	return 0;
}

static int rs_resume_test_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t rs_resume_test_read(struct file *file, char __user *buf,
				size_t count, loff_t *offp)
{
	int test = 0;
	int resume_result = root_scan_resume((D_RSOPID_KCODE |
					D_RSOPID_SYS_CALL |
					D_RSOPID_SE_HOOKS |
					D_RSOPID_SE_STATUS), &test);
	g_test_count++;
	RSLogDebug(TAG, "root scan test, rs_resume_test, count%d, result:%d",
		g_test_count, resume_result);

	return 0;
}

static int rs_resume_test_release(struct inode *inode, struct file *file)
{
	vfree(file->private_data);
	file->private_data = NULL;
	RSLogDebug(TAG, "xxx. resume release succ!");
	return 0;
}

/* revert kcode test items */
static int rev_kcode_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "revert kernel code scan testing\n");

	RSLogDebug(TAG, "root scan revert kernel code result:%d", ret);
	return ret;
}

static ssize_t rev_kcode_read(struct file *file, char __user *buf,
			size_t count, loff_t *offp)
{
	memcpy(g_rscan_clean_scan_result.kcode,
		g_rscan_orig_result.kcode, sizeof(g_rscan_orig_result.kcode));

	RSLogDebug(TAG, "revert rootscan: dead_code reverted\n");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int rev_kcode_release(struct inode *inode, struct file *file)
{
	vfree(file->private_data);
	file->private_data = NULL;
	RSLogDebug(TAG, "3. rev_sehooks_release succ!");
	return 0;
}

static int se_enforcing_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "se_enforcing scan testing\n");

	RSLogDebug(TAG, "root scan se_enforcing result:%d", ret);
	return ret;
}

static ssize_t se_enforcing_read(struct file *file, char __user *buf,
				size_t count, loff_t *offp)
{
	g_rscan_clean_scan_result.seenforcing = 0;
	RSLogDebug(TAG, "root scan test, se_enforcing broken");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int se_enforcing_release(struct inode *inode, struct file *file)
{
	vfree(file->private_data);
	file->private_data = NULL;
	RSLogDebug(TAG, "8. se enforcing release succ!");
	return 0;
}

static int syscall_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "system call scan testing\n");

	RSLogDebug(TAG, "root scan system call result:%d", ret);
	return ret;
}
static ssize_t syscall_read(struct file *file, char __user *buf,
			size_t count, loff_t *offp)
{
	memset(g_rscan_clean_scan_result.syscalls, 0,
		sizeof(g_rscan_clean_scan_result.syscalls));
	RSLogDebug(TAG, "rootscan: syscall injected\n");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int syscall_release(struct inode *inode, struct file *file)
{
	struct fault_private *priv = file->private_data;

	if (priv == NULL) {
		return 0;
	}

	vfree(file->private_data);
	file->private_data = NULL;
	RSLogDebug(TAG, "7. syscall_release succ!");
	return 0;
}

/* set selinux hooks to all 0, for test */
static int sehooks_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "se_hooks scan testing\n");

	RSLogDebug(TAG, "root scan se_hooks result:%d", ret);
	return ret;
}

static ssize_t sehooks_read(struct file *file, char __user *buf,
			size_t count, loff_t *offp)
{
	memset(g_rscan_clean_scan_result.sehooks, 0,
		sizeof(g_rscan_clean_scan_result.sehooks));
	RSLogDebug(TAG, "rootscan: sec->sem_semctl injected");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int sehooks_release(struct inode *inode, struct file *file)
{
	vfree(file->private_data);
	file->private_data = NULL;
	RSLogDebug(TAG, "5. sehooks_release succ!");
	return 0;
}

/* revert syscall test items */
static int rev_syscall_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "revert system call scan testing\n");

	RSLogDebug(TAG, "root scan revert system call result:%d", ret);
	return ret;
}
static ssize_t rev_syscall_read(struct file *file, char __user *buf,
				size_t count, loff_t *offp)
{
	memcpy(g_rscan_clean_scan_result.syscalls,
		g_rscan_orig_result.syscalls, sizeof(g_rscan_orig_result.syscalls));

	RSLogDebug(TAG, "revert rootscan: syscall reverted");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int rev_syscall_release(struct inode *inode, struct file *file)
{
	struct fault_private *priv = file->private_data;

	if (priv == NULL) {
		return 0;
	}

	vfree(file->private_data);
	file->private_data = NULL;
	RSLogDebug(TAG, "4. rev_syscall_release succ!");
	return 0;
}

/* revert selinux test items make selinux hooks data original */
static int rev_sehooks_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "revert se_hooks scan testing\n");

	RSLogDebug(TAG, "root scan revert se_hooks result:%d", ret);
	return ret;
}

static ssize_t rev_sehooks_read(struct file *file, char __user *buf,
				size_t count, loff_t *offp)
{
	memcpy(g_rscan_clean_scan_result.sehooks,
		g_rscan_orig_result.sehooks, sizeof(g_rscan_orig_result.sehooks));

	RSLogDebug(TAG, "revert root scan test, sehooks reverted");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int rev_sehooks_release(struct inode *inode, struct file *file)
{
	vfree(file->private_data);
	file->private_data = NULL;
	RSLogDebug(TAG, "2. rev_sehooks_release succ!");
	return 0;
}

/* revert selinux status test items */
static int rev_se_enforcing_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "revert se_enforcing scan testing\n");

	RSLogDebug(TAG, "root scan revert se_enforcing result:%d", ret);
	return ret;
}

static ssize_t rev_se_enforcing_read(struct file *file, char __user *buf,
				size_t count, loff_t *offp)
{
	g_rscan_clean_scan_result.seenforcing = 1;
	RSLogDebug(TAG, "revert root scan test, se_enforcing reverted");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int rev_se_enforcing_release(struct inode *inode, struct file *file)
{
	vfree(file->private_data);
	file->private_data = NULL;
	RSLogDebug(TAG, "1. rev_se_enforcing_release succ!");
	return 0;
}

static const struct file_operations rs_pause_test_fops = {
	.owner = THIS_MODULE,
	.open  = rs_pause_test_open,
	.read  = rs_pause_test_read,
	.release = rs_pause_test_release,
};

static const struct file_operations rs_resume_test_fops = {
	.owner = THIS_MODULE,
	.open  = rs_resume_test_open,
	.read  = rs_resume_test_read,
	.release = rs_resume_test_release,
};

static const struct file_operations kcode_fops = {
	.owner = THIS_MODULE,
	.open  = kcode_open,
	.read  = kcode_read,
	.release = kcode_release,
};

static const struct file_operations rev_kcode_fops = {
	.owner = THIS_MODULE,
	.open  = rev_kcode_open,
	.read  = rev_kcode_read,
	.release = rev_kcode_release,
};

static const struct file_operations se_enforcing_fops = {
	.owner = THIS_MODULE,
	.open  = se_enforcing_open,
	.read  = se_enforcing_read,
	.release = se_enforcing_release,
};

static const struct file_operations syscall_fops = {
	.owner = THIS_MODULE,
	.open  = syscall_open,
	.read  = syscall_read,
	.release = syscall_release,
};

static const struct file_operations sehooks_fops = {
	.owner = THIS_MODULE,
	.open  = sehooks_open,
	.read  = sehooks_read,
	.release = sehooks_release,
};

static const struct file_operations rev_syscall_fops = {
	.owner = THIS_MODULE,
	.open  = rev_syscall_open,
	.read  = rev_syscall_read,
	.release = rev_syscall_release,
};

static const struct file_operations rev_sehooks_fops = {
	.owner = THIS_MODULE,
	.open  = rev_sehooks_open,
	.read  = rev_sehooks_read,
	.release = rev_sehooks_release,
};

static const struct file_operations rev_se_enforcing_fops = {
	.owner = THIS_MODULE,
	.open  = rev_se_enforcing_open,
	.read  = rev_se_enforcing_read,
	.release = rev_se_enforcing_release,
};
#endif

int rscan_dynamic_init(void)
{
	if (rscan_init_data() != 0) {
		RSLogError(TAG, "rootscan: rscan init data failed");
		return RSCAN_ERR_SCANNER_INIT;
	}
#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
	g_rscan_orig_result = g_rscan_clean_scan_result;
	/* 0777 is the permissions format of file, Read, write and excute */
	proc_create("rs_pause_test",  0777, NULL, &rs_pause_test_fops);
	proc_create("rs_resume_test", 0777, NULL, &rs_resume_test_fops);

	proc_create("rev_sehooks",      0777, NULL, &rev_sehooks_fops);
	proc_create("rev_kcode",        0777, NULL, &rev_kcode_fops);
	proc_create("rev_syscall",      0777, NULL, &rev_syscall_fops);
	proc_create("rev_se_enforcing", 0777, NULL, &rev_se_enforcing_fops);

	proc_create("rs_se_enforcing", 0777, NULL, &se_enforcing_fops);
	proc_create("rs_sehooks",      0777, NULL, &sehooks_fops);
	proc_create("rs_kcode",        0777, NULL, &kcode_fops);
	proc_create("rs_syscall",      0777, NULL, &syscall_fops);
#endif
	return 0;
}
