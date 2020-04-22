/*
 * storage_rochk.c
 *
 * emmc/ufs Storage Read Only Check
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/reboot.h>

#include <scsi/scsi.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>

#include <chipset_common/bfmr/bfm/core/bfm_core.h>
#include <chipset_common/storage_rofa/storage_rofa.h>
#include "storage_rochk.h"
#include "crbroi.h"

#define STORAGE_ROCHK_NAME "storage_rochk"

#define PRINT_ERR(f, arg...) \
	pr_err(STORAGE_ROCHK_NAME ": " f, ## arg)

#define PRINT_INFO(f, arg...) \
	pr_info(STORAGE_ROCHK_NAME ": " f, ## arg)

#define PRINT_DEBUG(f, arg...) \
	pr_devel(STORAGE_ROCHK_NAME ": " f, ## arg)

#define PRINT_INFO_ONCE(format...)	({	\
	static bool __warned;			\
						\
	if (!__warned) {			\
		PRINT_INFO(format);		\
		__warned = true;		\
	}					\
})

#ifndef UFSHCD
#define UFSHCD "ufshcd"
#endif

#define BOOTDEVICE_DISK_NAME BFMR_SIZE_DISK_NAME

#define BOOTDEVICE_MODEL_SIZE   32
#define BOOTDEVICE_FWREV_SIZE   32

#define PARTITION_WRTRY_PATH    "/dev/block/by-name/rrecord"
#define PARTITION_WRTRY_LEN     512

#define WRITE_TRY_ERRCODE_OK        0
#define WRITE_TRY_ERRCODE_BLOCK     1
#define WRITE_TRY_ERRCODE_STUCK     2
#define WRITE_TRY_ERRCODE_OTHERS    3

#define PRE_EOL_NORMAL    1
#define PRE_EOL_SERIOUS   2
#define PRE_EOL_CRITICAL  3

struct bootdevice_info {
	int type;   /* 0 for emmc and 1 for ufs */
	unsigned int manfid;
	char model[BOOTDEVICE_MODEL_SIZE];
	char fwrev[BOOTDEVICE_FWREV_SIZE];
	unsigned int pre_eol_info;
};

struct bootdevice_disk_info {
	struct list_head disk_list;
	struct bfmr_disk_info disk_info;
};

struct storage_rochk_completion {
	struct completion event;
	unsigned int result;
};

static LIST_HEAD(g_bootdev_disks);
static DEFINE_MUTEX(g_bootdev_disks_mutex);
static struct bootdevice_info g_bootdev_info = {
	.type = -1,
	.model = { 0 },
	.fwrev = { 0 },
};

static atomic_t g_monitor_enabled = ATOMIC_INIT(0);
static atomic_t g_monitor_times = ATOMIC_INIT(10);
static atomic_t g_action_once = ATOMIC_INIT(0); /* check sense once */

static DEFINE_SPINLOCK(g_crbroi_lock);

/*
 * routines to match and statistic storage read only write-fail with
 * method that provided by chip manufacturer
 */
static inline int match_ufs_rofault_samsung(const char *model,
	unsigned int eol,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	(void)model;

	return (eol >= PRE_EOL_CRITICAL &&
		sense_key == NO_SENSE && asc == 0x0 && ascq == 0x0);
}

static inline int match_ufs_rofault_hynix(const char *model,
	unsigned int eol,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	(void)model;

	return (eol >= PRE_EOL_CRITICAL &&
		sense_key == NO_SENSE && asc == 0x0 && ascq == 0x0);
}

static inline int match_ufs_rofault_sandisk(const char *model,
	unsigned int eol,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	(void)model;

	/* for Eagle 8521 */
	return (eol >= PRE_EOL_CRITICAL &&
		sense_key == MEDIUM_ERROR && asc == 0x3 && ascq == 0x2);
}

static inline int match_ufs_rofault_toshiba(const char *model,
	unsigned int eol,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	(void)model;
	(void)eol;
	(void)sense_key;
	(void)asc;
	(void)ascq;

	return 0;
}

static inline int match_ufs_rofault_micron(const char *model,
	unsigned int eol,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	(void)model;

	return (eol >= PRE_EOL_CRITICAL && sense_key == ILLEGAL_REQUEST);
}

static inline int match_ufs_rofault_hisi(const char *model,
	unsigned int eol,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	(void)model;

	return (eol >= PRE_EOL_NORMAL &&
		sense_key == NO_SENSE && asc == 0x0 && ascq == 0x0);
}

static const char *get_ufs_manf_str(unsigned int manfid)
{
	if (manfid == UFS_MANUFACTURER_ID_TOSHIBA)
		return "tx";
	else if (manfid == UFS_MANUFACTURER_ID_HYNIX)
		return "hy";
	else if (manfid == UFS_MANUFACTURER_ID_SAMSUNG)
		return "ss";
	else if (manfid == UFS_MANUFACTURER_ID_MICRON)
		return "mt";
	else if (manfid == UFS_MANUFACTURER_ID_SANDISK)
		return "sd";
	else if (manfid == UFS_MANUFACTURER_ID_HI1861)
		return "hi";
	else
		return "xx";
}

static inline const char *str_match_result(int result)
{
	return result ? "[matched]" : "[unmatched]";
}

static int match_ufs_rofault(unsigned int eol,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	int res = 0;

	if (g_bootdev_info.manfid == UFS_MANUFACTURER_ID_TOSHIBA)
		res = match_ufs_rofault_toshiba(g_bootdev_info.model, eol,
			sense_key, asc, ascq);
	else if (g_bootdev_info.manfid == UFS_MANUFACTURER_ID_HYNIX)
		res = match_ufs_rofault_hynix(g_bootdev_info.model, eol,
			sense_key, asc, ascq);
	else if (g_bootdev_info.manfid == UFS_MANUFACTURER_ID_SAMSUNG)
		res = match_ufs_rofault_samsung(g_bootdev_info.model, eol,
			sense_key, asc, ascq);
	else if (g_bootdev_info.manfid == UFS_MANUFACTURER_ID_MICRON)
		res = match_ufs_rofault_micron(g_bootdev_info.model, eol,
			sense_key, asc, ascq);
	else if (g_bootdev_info.manfid == UFS_MANUFACTURER_ID_SANDISK)
		res = match_ufs_rofault_sandisk(g_bootdev_info.model, eol,
			sense_key, asc, ascq);
	else if (g_bootdev_info.manfid == UFS_MANUFACTURER_ID_HI1861)
		res = match_ufs_rofault_hisi(g_bootdev_info.model, eol,
			sense_key, asc, ascq);

	PRINT_INFO("%s ufs rofault <%d, %d, %d> %s\n",
		get_ufs_manf_str(g_bootdev_info.manfid),
		sense_key, asc, ascq, str_match_result(res));

	return res;
}

/*
 * External interfaces to kernel
 */
void storage_rochk_record_bootdevice_type(int type)
{
	g_bootdev_info.type = type;
}

void storage_rochk_record_bootdevice_manfid(unsigned int manfid)
{
	g_bootdev_info.manfid = manfid;
}

void storage_rochk_record_bootdevice_model(char *model)
{
	if (model != NULL && strlen(model) < BOOTDEVICE_MODEL_SIZE) {
		PRINT_INFO("%s: record the device model %c***\n",
			__func__, model[0]);
		strncpy(g_bootdev_info.model, model,
			BOOTDEVICE_MODEL_SIZE - 1);
	}
}

void storage_rochk_record_bootdevice_fwrev(char *rev)
{
	if (rev != NULL && strlen(rev) < BOOTDEVICE_FWREV_SIZE) {
		PRINT_INFO("%s: record the device rev %c***\n",
			__func__, rev[0]);
		strncpy(g_bootdev_info.fwrev, rev,
			BOOTDEVICE_FWREV_SIZE - 1);
	}
}

void storage_rochk_record_bootdevice_pre_eol_info(int eol)
{
	g_bootdev_info.pre_eol_info = eol;
}

/*
 * filter out the ufs scsi_disk
 */
static unsigned int bootdevice_sd_is_ufssd(const struct scsi_device *sdp)
{
	struct Scsi_Host *shost = sdp->host;

	if (shost != NULL && shost->hostt != NULL &&
	    shost->hostt->name != NULL &&
	    strncmp(shost->hostt->name, UFSHCD, strlen(UFSHCD) + 1) == 0)
		return 1;
	else
		return 0;
}

unsigned int storage_rochk_filter_sd(const struct scsi_device *sdp)
{
	return bootdevice_sd_is_ufssd(sdp);
}

/*
 * MUST called with filtered scsi_disk.
 * So, secondly guard it with internal storage_rochk_filter_sd.
 */
int storage_rochk_record_sd(const struct scsi_device *sdp,
	const char *name, int major, int minor)
{
	struct bootdevice_disk_info *dinfo = NULL;
	struct bootdevice_disk_info *next = NULL;
	int res = 0;

	PRINT_INFO("%s: add scsi_disk %s\n", __func__, name);

	if (storage_rochk_filter_sd(sdp) == 0)
		return 0;

	mutex_lock(&g_bootdev_disks_mutex);

	list_for_each_entry_safe(dinfo, next, &g_bootdev_disks, disk_list) {
		if (strncmp(dinfo->disk_info.name, name,
		    BOOTDEVICE_DISK_NAME) == 0)
			goto out;
	}

	/* not existing, allocate it */
	dinfo = kzalloc(sizeof(*dinfo), GFP_KERNEL);
	if (dinfo == NULL) {
		PRINT_ERR("%s: allocate memory failed\n", __func__);
		res = -ENOMEM;
		goto out;
	}

	strncpy(dinfo->disk_info.name, name, BOOTDEVICE_DISK_NAME - 1);
	dinfo->disk_info.major = major;
	dinfo->disk_info.minor = minor;
	list_add_tail(&dinfo->disk_list, &g_bootdev_disks);

out:
	mutex_unlock(&g_bootdev_disks_mutex);
	return res;
}

/*
 * called in module_exit routine.
 */
static void storage_rochk_release_disk_info_list(void)
{
	struct bootdevice_disk_info *dinfo = NULL;
	struct bootdevice_disk_info *next = NULL;

	list_for_each_entry_safe(dinfo, next, &g_bootdev_disks, disk_list) {
		list_del(&dinfo->disk_list);
		kfree(dinfo);
	}
}

static atomic_t g_sd_rev_got = ATOMIC_INIT(0);

void storage_rochk_record_sd_rev_once(const struct scsi_device *sdev)
{
	char rev[8] = { 0 };

	if (sdev && sdev->inquiry_len >= 36 && sdev->inquiry != NULL) {
		if (atomic_cmpxchg(&g_sd_rev_got, 0, 1) == 0) {
			snprintf(rev, 5, "%.4s", sdev->inquiry + 32);
			storage_rochk_record_bootdevice_fwrev(rev);
		}
	}
}

/*
 * record storage write-protect status when device enumerated.
 * for ufs, record the status of device write-protect field from mode sense.
 * for emmc, record status read from csd[12|13].
 */
void storage_rochk_record_disk_wp_status(const struct scsi_device *sdp,
	const char *name, unsigned char wp)
{
	struct bootdevice_disk_info *dinfo = NULL;
	struct bootdevice_disk_info *next = NULL;
	unsigned int found = 0;

	mutex_lock(&g_bootdev_disks_mutex);

	list_for_each_entry_safe(dinfo, next, &g_bootdev_disks, disk_list) {
		if (strncmp(dinfo->disk_info.name, name,
		    BOOTDEVICE_DISK_NAME) == 0) {
			PRINT_INFO("%s: record disk %s wp status to %d\n",
				__func__, name, wp);
			dinfo->disk_info.write_prot = wp;
			found = 1;
		}
	}

	mutex_unlock(&g_bootdev_disks_mutex);

	if (found == 0)
		PRINT_ERR("%s: disk %s not found\n", __func__, name);
}

void storage_rochk_record_disk_capacity(const struct scsi_device *sdp,
	const char *name, unsigned long long capacity)
{
	struct bootdevice_disk_info *dinfo = NULL;
	struct bootdevice_disk_info *next = NULL;
	unsigned int found = 0;

	mutex_lock(&g_bootdev_disks_mutex);

	list_for_each_entry_safe(dinfo, next, &g_bootdev_disks, disk_list) {
		if (strncmp(dinfo->disk_info.name, name,
		    BOOTDEVICE_DISK_NAME) == 0) {
			PRINT_INFO("%s: record disk %s capacity to %llu\n",
				__func__, name, capacity);
			dinfo->disk_info.capacity = capacity;
			found = 1;
		}
	}

	mutex_unlock(&g_bootdev_disks_mutex);

	if (found == 0)
		PRINT_ERR("%s: disk %s not found\n", __func__, name);
}

/*
 * return -1 for non disk found, 0 for rw, 1 for wp.
 */
static int bootdevice_get_disk_wp_status(const char *name)
{
	struct bootdevice_disk_info *dinfo = NULL;
	struct bootdevice_disk_info *next = NULL;
	struct bootdevice_disk_info *dinfo_target = NULL;
	unsigned long long max_cap = 0;
	int res = -1;

	mutex_lock(&g_bootdev_disks_mutex);

	if (name != NULL) {
		list_for_each_entry_safe(dinfo, next, &g_bootdev_disks,
				disk_list) {
			if (strncmp(dinfo->disk_info.name, name,
			    BOOTDEVICE_DISK_NAME) == 0)
				dinfo_target = dinfo;
		}
	} else {
		list_for_each_entry_safe(dinfo, next, &g_bootdev_disks,
				disk_list) {
			if (dinfo->disk_info.capacity > max_cap) {
				max_cap = dinfo->disk_info.capacity;
				dinfo_target = dinfo;
			}
		}
	}

	if (dinfo_target != NULL) {
		res = (int)dinfo_target->disk_info.write_prot;
		PRINT_INFO("%s: got disk %s wp status %d\n",
			__func__, dinfo_target->disk_info.name, res);
	} else {
		PRINT_ERR("%s: wp target disk not found\n", __func__);
	}

	mutex_unlock(&g_bootdev_disks_mutex);

	return res;
}

static int storage_rochk_action_rofault(unsigned int method,
	unsigned int action)
{
	unsigned long flags;

	PRINT_INFO("%s: set crbroi content <%x, %x>\n",
		__func__, method, action);

	if (method != STORAGE_ROINFO_UNKNOWN_OR_UNCARE) {
		spin_lock_irqsave(&g_crbroi_lock, flags);
		get_storage_crbroi_func()->transfer_crbroi(method);
		spin_unlock_irqrestore(&g_crbroi_lock, flags);
	}

	if (action == STORAGE_ROCHK_ACTION_REBOOT) {
		PRINT_INFO("%s: machine restart...\n", __func__);
		machine_restart(NULL);
	}

	return 0;
}

/*
 * The following three routines are provided to eMMC/UFS driver
 * to judge early if the ro monitor is enabled and report chip write-fail
 * when request is done in ISR context.
 */
int storage_rochk_is_monitor_enabled(void)
{
	return atomic_read(&g_monitor_enabled);
}

/*
 * externally ensure sense_valid && !sense_deferred.
 * MUST called with filtered scsi_disk.
 * So, secondly guard it with internal storage_rochk_filter_sd.
 */
void storage_rochk_monitor_sd_readonly(const struct scsi_device *sdp,
	const struct request *req, int result,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	unsigned int eol = g_bootdev_info.pre_eol_info;
	int matched;
	int driver_res = driver_byte(result);
	int status_res = status_byte(result);
	unsigned int method;
	unsigned int action;

	if (atomic_read(&g_monitor_enabled) == 0)
		return;

	if (storage_rochk_filter_sd(sdp) == 0)
		return;
	if (req->cmd_type != REQ_TYPE_FS || rq_data_dir(req) != WRITE)
		return;

	if (atomic_add_unless(&g_monitor_times, -1, 0) == 0) {
		atomic_set(&g_monitor_enabled, 0);
		return;
	}

	if (driver_res != DRIVER_SENSE || status_res != CHECK_CONDITION)
		return;

	matched = match_ufs_rofault(eol, sense_key, asc, ascq);
	if (matched) {
		action = STORAGE_ROCHK_ACTION_REBOOT;
		method = STORAGE_ROINFO_KNL_WRMON_DRIVER;
	} else {
		action = STORAGE_ROCHK_ACTION_NONE;
		method = STORAGE_ROINFO_KNL_WRMON_SUSPICIOUS;
	}

	/* action as early as possible for LLD rofault */
	if (atomic_cmpxchg(&g_action_once, 0, 1) == 0)
		storage_rochk_action_rofault(method, action);
}

/*
 * Internal routines to implement storage readonly detect by writing test
 */
static int write_buffer(int fd, const char *buffer, int size)
{
	int bytes = -EINTR;

	while (bytes != 0) {
		bytes = sys_write(fd, buffer, size);
		if ((bytes < 0) && (bytes != -EINTR)) {
			return bytes;
		} else if (bytes == size) {
			break;
		} else if (bytes > 0) {
			buffer += bytes;
			size -= bytes;
		}
	}

	return 0;
}

/*
 * return value:
 * 0 for ok
 * WRITE_TRY_ERRCODE_OTHERS for non-storage error
 * WRITE_TRY_ERRCODE_BLOCK for storage error
 */
static unsigned int storage_ro_write_try(void)
{
	mm_segment_t old_fs;
	int fd_wr = -1;
	char *buffer = NULL;
	int res = -1;
	unsigned int ret = 0;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd_wr = sys_open(PARTITION_WRTRY_PATH, O_RDWR, 0);
	if (fd_wr < 0) {
		PRINT_ERR("%s: open partition failed\n", __func__);
		goto out;
	}

	buffer = kmalloc(PARTITION_WRTRY_LEN, GFP_KERNEL);
	if (buffer == NULL) {
		PRINT_ERR("%s: alloc buffer failed\n", __func__);
		goto out;
	}

	/* write try */
	res = sys_lseek(fd_wr, -1024, SEEK_END);
	if (res < 0) {
		PRINT_ERR("%s: seek file failed : %d\n", __func__, res);
		goto out;
	}

	res = write_buffer(fd_wr, buffer, PARTITION_WRTRY_LEN);
	if (res < 0) {
		PRINT_ERR("%s: write returns error : %d\n", __func__, res);
		goto err_out;
	}

	res = sys_fdatasync(fd_wr);
	if (res < 0) {
		PRINT_ERR("%s: fdatasync file failed : %d\n", __func__, res);
		goto err_out;
	}

err_out:
	if (res == -ENOLINK || res == -EREMOTEIO || res == -ETIMEDOUT ||
	    res == -EBADE || res == -ENOSPC || res == -ENODATA || res == -EIO)
		ret = WRITE_TRY_ERRCODE_BLOCK;
	else if (res < 0)
		ret = WRITE_TRY_ERRCODE_OTHERS;

out:
	if (fd_wr >= 0)
		sys_close(fd_wr);
	kfree(buffer);

	set_fs(old_fs);

	return ret;
}

static int storage_ro_write_try_thread(void *param)
{
	struct storage_rochk_completion *wrtry_comp = param;

	PRINT_INFO("%s: storage write-try thread start\n", __func__);

	wrtry_comp->result = storage_ro_write_try();
	complete(&wrtry_comp->event);

	return 0;
}

/*
 * ioctl implementation routines.
 */
int storage_rochk_ioctl_check_bootdisk_wp(
	struct bfmr_bootdisk_wp_status_iocb *arg_u)
{
	struct bfmr_bootdisk_wp_status_iocb ioc;
	char *disk_name = NULL;
	int wp;
	unsigned int method;
	unsigned int action;

	if (copy_from_user(&ioc, arg_u, sizeof(ioc)))
		return -EFAULT;

	if (ioc.use_name)
		disk_name = ioc.disk_name;
	else
		disk_name = NULL;

	wp = bootdevice_get_disk_wp_status(disk_name);
	if (wp == 1)
		ioc.write_prot = 1;
	else
		ioc.write_prot = 0;     /* set to 0 if not get */

	if (copy_to_user(arg_u, &ioc, sizeof(ioc)))
		return -EFAULT;

	if (wp == 1) {
		method = STORAGE_ROINFO_KNL_WRITE_PROT;
		action = STORAGE_ROCHK_ACTION_REBOOT;
		storage_rochk_action_rofault(method, action);
	}

	return 0;
}

int storage_rochk_ioctl_enable_monitor(struct bfmr_storage_rochk_iocb *arg_u)
{
	struct bfmr_storage_rochk_iocb ioc;
	int value;

	if (copy_from_user(&ioc, arg_u, sizeof(ioc)))
		return -EFAULT;

	value = ioc.data ? 1 : 0;
	atomic_set(&g_monitor_enabled, value);

	return 0;
}

int storage_rochk_ioctl_run_storage_wrtry_sync(
	struct bfmr_storage_rochk_iocb *arg_u)
{
	struct bfmr_storage_rochk_iocb ioc;
	struct storage_rochk_completion wrtry_comp;
	unsigned long complete_intime;
	unsigned int wr_res;
	unsigned int method;
	unsigned int action;

	init_completion(&wrtry_comp.event);
	wrtry_comp.result = 0;

	kthread_run(storage_ro_write_try_thread, (void *)&wrtry_comp,
		"storage_wrtry");

	complete_intime = wait_for_completion_timeout(&wrtry_comp.event,
		msecs_to_jiffies(500));
	if (complete_intime == 0) {
		wr_res = WRITE_TRY_ERRCODE_STUCK;
		PRINT_INFO("%s: wait for result timeout\n", __func__);
	} else {
		wr_res = wrtry_comp.result;
		PRINT_INFO("%s: get write-try result %u\n",
			__func__, wrtry_comp.result);
	}

	ioc.data = wr_res;
	if (copy_to_user(arg_u, &ioc, sizeof(ioc)))
		return -EFAULT;

	if (wr_res == WRITE_TRY_ERRCODE_OK) {
		PRINT_INFO("%s: storage write-try returns ok : rw\n",
			__func__);
		return 0;
	} else if (wr_res == WRITE_TRY_ERRCODE_BLOCK) {
		PRINT_INFO("%s: storage write-try returns ok : blk error\n",
			__func__);
		method = STORAGE_ROINFO_KNL_WRTRY_BLOCK;
		action = STORAGE_ROCHK_ACTION_REBOOT;
	} else if (wr_res == WRITE_TRY_ERRCODE_STUCK) {
		PRINT_INFO("%s: storage write-try returns ok : stuck\n",
			__func__);
		method = STORAGE_ROINFO_KNL_WRTRY_STUCK;
		action = STORAGE_ROCHK_ACTION_REBOOT;
	} else if (wr_res == WRITE_TRY_ERRCODE_OTHERS) {
		PRINT_INFO("%s: storage write-try returns ok : upper error\n",
			__func__);
		method = STORAGE_ROINFO_KNL_WRTRY_MISC;
		action = STORAGE_ROCHK_ACTION_NONE;
	} else {
		PRINT_ERR("%s: storage write-try returns unknown errcode %u\n",
			__func__, wr_res);
		return 0;
	}

	return storage_rochk_action_rofault(method, action);
}

int storage_rofa_ioctl_get_rofa_info(struct bfmr_storage_rofa_info_iocb *arg_u)
{
	struct bfmr_storage_rofa_info_iocb ioc;
	int res;

	memset(&ioc, 0, sizeof(ioc));

	ioc.mode = get_storage_rofa_bootopt();

	if (ioc.mode != STORAGE_ROFA_BOOTOPT_NOP) {
		res = get_storage_crbroi_func()->get_crbroi_method(
			&ioc.method);
		if (res == -1)
			return -EINVAL;

		res = get_storage_crbroi_func()->get_crbroi_round(
			&ioc.round);
		if (res == -1)
			return -EINVAL;
	}

	if (copy_to_user(arg_u, &ioc, sizeof(ioc)))
		return -EFAULT;

	return 0;
}

int storage_rochk_ioctl_get_bootdevice_disk_count(
	struct bfmr_storage_rochk_iocb *arg_u)
{
	struct bfmr_storage_rochk_iocb ioc;
	struct bootdevice_disk_info *dinfo = NULL;
	struct bootdevice_disk_info *next = NULL;

	mutex_lock(&g_bootdev_disks_mutex);

	ioc.data = 0;
	list_for_each_entry_safe(dinfo, next, &g_bootdev_disks, disk_list) {
		++ioc.data;
	}

	mutex_unlock(&g_bootdev_disks_mutex);

	if (copy_to_user(arg_u, &ioc, sizeof(ioc)))
		return -EFAULT;

	return 0;
}

int storage_rochk_ioctl_get_bootdevice_disk_info(
	struct bfmr_bootdevice_disk_info_iocb *arg_u)
{
	struct bfmr_bootdevice_disk_info_iocb iocb;
	struct bootdevice_disk_info *dinfo = NULL;
	struct bootdevice_disk_info *next = NULL;
	unsigned int disk_count;

	if (copy_from_user(&iocb, arg_u, sizeof(iocb)))
		return -EFAULT;

	mutex_lock(&g_bootdev_disks_mutex);

	disk_count = 0;
	list_for_each_entry_safe(dinfo, next, &g_bootdev_disks, disk_list) {
		++disk_count;
	}

	if (iocb.in_cnt < disk_count) {
		PRINT_ERR("%s: output buffer size insufficient\n", __func__);
		mutex_unlock(&g_bootdev_disks_mutex);
		return -EINVAL;
	}

	iocb.out_cnt = 0;
	list_for_each_entry_safe(dinfo, next, &g_bootdev_disks, disk_list) {
		if (copy_to_user(&arg_u->info_arr[iocb.out_cnt++],
		    &dinfo->disk_info, sizeof(dinfo->disk_info))) {
			mutex_unlock(&g_bootdev_disks_mutex);
			return -EFAULT;
		}
	}

	mutex_unlock(&g_bootdev_disks_mutex);

	if (copy_to_user(arg_u, &iocb, sizeof(iocb)))
		return -EFAULT;

	return 0;
}

int storage_rochk_ioctl_get_bootdevice_prod_info(
	struct bfmr_bootdevice_prod_info_iocb *arg_u)
{
	struct bfmr_bootdevice_prod_info_iocb info;

	memset(&info, 0, sizeof(info));
	snprintf(info.prod_info, sizeof(info.prod_info), "%s,%s",
		g_bootdev_info.model, g_bootdev_info.fwrev);

	if (copy_to_user(arg_u, &info, sizeof(info)))
		return -EFAULT;

	return 0;
}

static int __init storage_rochk_misc_init(void)
{
	int r;

	r = get_storage_crbroi_func()->crbroi_func_init();
	if (r) {
		PRINT_ERR("%s: init crbroi_func failed\n", __func__);
		return r;
	}

	return r;
}

static void __exit storage_rochk_misc_exit(void)
{
	int r;

	storage_rochk_release_disk_info_list();

	r = get_storage_crbroi_func()->crbroi_func_exit();
	if (r)
		PRINT_ERR("%s: exit crbroi_func failed\n", __func__);

}

module_init(storage_rochk_misc_init);
module_exit(storage_rochk_misc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("storage readonly check driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

#ifdef CONFIG_HUAWEI_STORAGE_ROFA_FAULT_INJECT

/*
 * Storage Read Only Fault Injection module
 */
#include <linux/hisi/rdr_hisi_platform.h>

#define STORAGE_ROCHK_FAULT_INJECT_PHYMEM_ADDR  \
	(HISI_SUB_RESERVED_UNUSED_PHYMEM_BASE + 16)

#define FASTBOOT_STORAGE_ROCHK_ROFAULT_SAMSUNG_UFS      0x46490001
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_HYNIX_UFS        0x46490002
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_MICRON_UFS       0x46490003
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_SANDISK_UFS      0x46490004
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_TOSHIBA_UFS      0x46490005
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_HI1861_UFS       0x46490006
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_FORCE_ROCHK      0x46490007
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_DATA_PROTECT     0x46490008
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_DOORBELL_TIMEOUT 0x46490010

#define KERNEL_STORAGE_ROCHK_ROFAULT_SAMSUNG_UFS        0x46490020
#define KERNEL_STORAGE_ROCHK_ROFAULT_HYNIX_UFS          0x46490021
#define KERNEL_STORAGE_ROCHK_ROFAULT_MICRON_UFS         0x46490022
#define KERNEL_STORAGE_ROCHK_ROFAULT_SANDISK_UFS        0x46490023
#define KERNEL_STORAGE_ROCHK_ROFAULT_TOSHIBA_UFS        0x46490024
#define KERNEL_STORAGE_ROCHK_ROFAULT_HI1861_UFS         0x46490025
#define KERNEL_STORAGE_ROCHK_ROFAULT_DATA_PROTECT       0x46490026
#define KERNEL_STORAGE_ROCHK_ROFAULT_MEDIUM_ERROR       0x46490027
#define KERNEL_STORAGE_ROCHK_ROFAULT_RECOVER_ERROR      0x46490028
#define KERNEL_STORAGE_ROCHK_ROFAULT_WRITE_PROT         0x4649002A

static unsigned int g_rofi_value;

unsigned int storage_rofi_should_inject_check_condition_sense(void)
{
	if (g_rofi_value >= KERNEL_STORAGE_ROCHK_ROFAULT_SAMSUNG_UFS &&
	    g_rofi_value <= KERNEL_STORAGE_ROCHK_ROFAULT_RECOVER_ERROR)
		return 1;
	else
		return 0;
}

static void storage_rofi_inject_fault_ccs_samsung(struct scsi_sense_hdr *sshdr)
{
	storage_rochk_record_bootdevice_manfid(UFS_MANUFACTURER_ID_SAMSUNG);
	storage_rochk_record_bootdevice_pre_eol_info(3);
	sshdr->sense_key = NO_SENSE;
	sshdr->asc = 0x0;
	sshdr->ascq = 0x0;
}

static void storage_rofi_inject_fault_ccs_hynix(struct scsi_sense_hdr *sshdr)
{
	storage_rochk_record_bootdevice_manfid(UFS_MANUFACTURER_ID_HYNIX);
	storage_rochk_record_bootdevice_pre_eol_info(3);
	sshdr->sense_key = NO_SENSE;
	sshdr->asc = 0x0;
	sshdr->ascq = 0x0;
}

static void storage_rofi_inject_fault_ccs_micron(struct scsi_sense_hdr *sshdr)
{
	storage_rochk_record_bootdevice_manfid(UFS_MANUFACTURER_ID_MICRON);
	storage_rochk_record_bootdevice_pre_eol_info(3);
	sshdr->sense_key = ILLEGAL_REQUEST;
	sshdr->asc = 0x0;
	sshdr->ascq = 0x0;
}

static void storage_rofi_inject_fault_ccs_sandisk(struct scsi_sense_hdr *sshdr)
{
	storage_rochk_record_bootdevice_manfid(UFS_MANUFACTURER_ID_SANDISK);
	storage_rochk_record_bootdevice_pre_eol_info(3);
	sshdr->sense_key = MEDIUM_ERROR;
	sshdr->asc = 0x3;
	sshdr->ascq = 0x2;
}

static void storage_rofi_inject_fault_ccs_toshiba(struct scsi_sense_hdr *sshdr)
{
	(void)sshdr;
	storage_rochk_record_bootdevice_manfid(UFS_MANUFACTURER_ID_TOSHIBA);
	storage_rochk_record_bootdevice_pre_eol_info(3);
	/* chip unsupport now */
}

static void storage_rofi_inject_fault_ccs_hisi(struct scsi_sense_hdr *sshdr)
{
	(void)sshdr;
	storage_rochk_record_bootdevice_manfid(UFS_MANUFACTURER_ID_HI1861);
	storage_rochk_record_bootdevice_pre_eol_info(3);
	/* chip unsupport now */
}

static void storage_rofi_inject_fault_ccs_data_prot(
	struct scsi_sense_hdr *sshdr)
{
	sshdr->sense_key = DATA_PROTECT;
	sshdr->asc = 0x0;
	sshdr->ascq = 0x0;
}

static void storage_rofi_inject_fault_ccs_medium_error(
	struct scsi_sense_hdr *sshdr)
{
	sshdr->sense_key = MEDIUM_ERROR;
	sshdr->asc = 0x0;
	sshdr->ascq = 0x0;
}

static void storage_rofi_inject_fault_ccs_recovered_error(
	struct scsi_sense_hdr *sshdr)
{
	sshdr->sense_key = RECOVERED_ERROR;
	sshdr->asc = 0x0;
	sshdr->ascq = 0x0;
}

void storage_rofi_inject_fault_check_condition_sense(int *result,
	int *sense_valid, int *sense_deferred, unsigned int *good_bytes,
	struct request *req, struct scsi_device *sdp,
	struct scsi_sense_hdr *sshdr)
{
	unsigned int sense_data_injected = 0;
	unsigned int rofi = g_rofi_value;

	if (req->cmd_type == REQ_TYPE_FS && rq_data_dir(req) == WRITE) {
		if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_SAMSUNG_UFS) {
			storage_rofi_inject_fault_ccs_samsung(sshdr);
			sense_data_injected = 1;
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_HYNIX_UFS) {
			storage_rofi_inject_fault_ccs_hynix(sshdr);
			sense_data_injected = 1;
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_MICRON_UFS) {
			storage_rofi_inject_fault_ccs_micron(sshdr);
			sense_data_injected = 1;
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_SANDISK_UFS) {
			storage_rofi_inject_fault_ccs_sandisk(sshdr);
			sense_data_injected = 1;
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_TOSHIBA_UFS) {
			storage_rofi_inject_fault_ccs_toshiba(sshdr);
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_HI1861_UFS) {
			storage_rofi_inject_fault_ccs_hisi(sshdr);
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_DATA_PROTECT) {
			storage_rofi_inject_fault_ccs_data_prot(sshdr);
			sense_data_injected = 1;
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_MEDIUM_ERROR) {
			/* good_bytes will to sd_completed_bytes */
			storage_rofi_inject_fault_ccs_medium_error(sshdr);
			sense_data_injected = 1;
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_RECOVER_ERROR) {
			/* good bytes will to scsi_bufflen */
			storage_rofi_inject_fault_ccs_recovered_error(sshdr);
			sense_data_injected = 1;
		}

		if (sense_data_injected) {
			/* driver_byte | status_byte */
			*result = ((DRIVER_SENSE) << 24) |
				((CHECK_CONDITION) << 1);
			*sense_valid = 1;
			*sense_deferred = 0;
			*good_bytes = 0;

			PRINT_INFO_ONCE("rofi inject fault ccs <%d, %d, %d>\n",
				sshdr->sense_key, sshdr->asc, sshdr->ascq);
		}
	}
}

/*
 * write_prot is read in scsi_disk module init early than this module.
 * read rofi from the phys_mem directly.
 */
unsigned int storage_rofi_should_inject_write_prot_status(void)
{
	unsigned char *la = NULL;
	unsigned int rofi = 0;

	la = ioremap_nocache(STORAGE_ROCHK_FAULT_INJECT_PHYMEM_ADDR,
		sizeof(unsigned int));
	if (la != NULL) {
		rofi = readl(la);
		iounmap(la);
	}

	if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_WRITE_PROT)
		return 1;
	else
		return 0;
}

static int __init storage_rofi_init(void)
{
	unsigned char *la = NULL;

	la = ioremap_nocache(STORAGE_ROCHK_FAULT_INJECT_PHYMEM_ADDR,
		sizeof(unsigned int));
	if (la != NULL) {
		g_rofi_value = readl(la);
		iounmap(la);
		return 0;
	}

	return -1;
}

static void __exit storage_rofi_exit(void)
{
}

module_init(storage_rofi_init);
module_exit(storage_rofi_exit);

#endif
