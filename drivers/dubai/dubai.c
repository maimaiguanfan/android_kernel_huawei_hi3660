/*
 * DUBAI drvier.
 *
 * Copyright (C) 2017 Huawei Device Co.,Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <chipset_common/dubai/dubai_common.h>

#define DUBAI_MAGIC 'k'
#define IOCTL_GPU_ENABLE _IOW(DUBAI_MAGIC, 1, bool)
#define IOCTL_GPU_INFO_GET _IOR(DUBAI_MAGIC, 2, struct dev_transmit_t)
#define IOCTL_PROC_CPUTIME_REQUEST _IOW(DUBAI_MAGIC, 3, long long)
#define IOCTL_PROC_NAME_GET _IOWR(DUBAI_MAGIC, 4, struct dev_transmit_t)
#define IOCTL_LOG_STATS_ENABLE _IOW(DUBAI_MAGIC, 5, bool)
#define IOCTL_KWORKER_INFO_REQUEST _IOW(DUBAI_MAGIC, 6, long long)
#define IOCTL_UEVENT_INFO_REQUEST _IOW(DUBAI_MAGIC, 7, long long)
#define IOCTL_BRIGHTNESS_ENABLE _IOW(DUBAI_MAGIC, 8, bool)
#define IOCTL_BRIGHTNESS_GET _IOR(DUBAI_MAGIC, 9, uint32_t)
#define IOCTL_PROC_CPUTIME_ENABLE _IOW(DUBAI_MAGIC, 10, bool)
#define IOCTL_BINDER_STATS_ENABLE _IOW(DUBAI_MAGIC, 11, bool)
#define IOCTL_BINDER_STATS_LIST_SET _IOW(DUBAI_MAGIC, 12, struct dev_transmit_t)
#define IOCTL_BINDER_STATS_REQUEST _IOW(DUBAI_MAGIC, 13, long long)
#define IOCTL_TASK_CPUPOWER_ENABLE _IOR(DUBAI_MAGIC, 14, bool)
#define IOCTL_AOD_GET_DURATION _IOR(DUBAI_MAGIC, 15, uint64_t)
#define IOCTL_BATTERY_RM_GET _IOR(DUBAI_MAGIC, 16, int32_t)
#define IOCTL_WAKEUP_SOURCE_NAME_REQUEST _IOW(DUBAI_MAGIC, 17, long long)
#define IOCTL_PROC_DECOMPOSE_SET _IOW(DUBAI_MAGIC, 18, struct dev_transmit_t)

static long dubai_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int rc = 0;
	void __user *argp = (void __user *)arg;

	switch (cmd) {
	case IOCTL_GPU_ENABLE:
		rc = dubai_set_gpu_enable(argp);
		break;
	case IOCTL_GPU_INFO_GET:
		rc = dubai_get_gpu_info(argp);
		break;
	case IOCTL_PROC_CPUTIME_ENABLE:
		rc = dubai_proc_cputime_enable(argp);
		break;
	case IOCTL_PROC_CPUTIME_REQUEST:
		rc = dubai_get_proc_cputime(argp);
		break;
	case IOCTL_PROC_NAME_GET:
		rc = dubai_get_proc_name(argp);
		break;
	case IOCTL_LOG_STATS_ENABLE:
		rc = dubai_log_stats_enable(argp);
		break;
	case IOCTL_KWORKER_INFO_REQUEST:
		rc = dubai_get_kworker_info(argp);
		break;
	case IOCTL_UEVENT_INFO_REQUEST:
		rc = dubai_get_uevent_info(argp);
		break;
	case IOCTL_BRIGHTNESS_ENABLE:
		rc = dubai_set_brightness_enable(argp);
		break;
	case IOCTL_BRIGHTNESS_GET:
		rc = dubai_get_brightness_info(argp);
		break;
	case IOCTL_BINDER_STATS_ENABLE:
		rc = dubai_binder_stats_enable(argp);
		break;
	case IOCTL_BINDER_STATS_LIST_SET:
		rc = dubai_set_binder_list(argp);
		break;
	case IOCTL_BINDER_STATS_REQUEST:
		rc = dubai_get_binder_stats(argp);
		break;
	case IOCTL_TASK_CPUPOWER_ENABLE:
		rc = dubai_get_task_cpupower_enable(argp);
		break;
	case IOCTL_AOD_GET_DURATION:
		rc = dubai_get_aod_duration(argp);
		break;
	case IOCTL_BATTERY_RM_GET:
		rc = dubai_get_battery_rm(argp);
		break;
	case IOCTL_WAKEUP_SOURCE_NAME_REQUEST:
		rc = dubai_get_ws_lasting_name(argp);
		break;
	case IOCTL_PROC_DECOMPOSE_SET:
		rc = dubai_set_proc_decompose(argp);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

#ifdef CONFIG_COMPAT
static long dubai_compat_ioctl(struct file *filp,
			unsigned int cmd, unsigned long arg)
{
	return dubai_ioctl(filp, cmd, (unsigned long) compat_ptr(arg));
}
#endif

static int dubai_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int dubai_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations dubai_device_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = dubai_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= dubai_compat_ioctl,
#endif
	.open = dubai_open,
	.release = dubai_release,
};

static struct miscdevice dubai_device = {
	.name = "dubai",
	.fops = &dubai_device_fops,
	.minor = MISC_DYNAMIC_MINOR,
};

static int __init dubai_init(void)
{
	int ret = 0;

	dubai_gpu_init();
	dubai_proc_cputime_init();
	dubai_stats_init();

	ret = misc_register(&dubai_device);
	if (ret) {
		DUBAI_LOGE("Failed to register dubai device");
		goto out;
	}

	DUBAI_LOGI("DUBAI module initialize success");
out:
	return ret;
}

static void __exit dubai_exit(void)
{
	dubai_gpu_exit();
	dubai_proc_cputime_exit();
	dubai_stats_exit();
	buffered_log_release();
}

late_initcall(dubai_init);
module_exit(dubai_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yu Peng, <pengyu7@huawei.com>");
MODULE_DESCRIPTION("Huawei Device Usage Big-data Analytics Initiative Driver");
