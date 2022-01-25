
#include "huawei_platform/hwaa/hwaa_ioctl.h"
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/rwsem.h>
#include <securec.h>

#include "huawei_platform/hwaa/hwaa_error.h"
#include "huawei_platform/hwaa/hwaa_fs_hooks.h"
#include "huawei_platform/hwaa/hwaa_limits.h"

#include "inc/fek/hwaa_fs_callbacks.h"
#include "inc/base/hwaa_list.h"
#include "inc/tee/hwaa_tee.h"
#include "inc/base/hwaa_define.h"
#include "inc/base/hwaa_utils.h"
#include "inc/tee/hwaa_adapter.h"

#define HWAA_FIRST_MINOR 0
#define HWAA_MINOR_CNT 1
#define SYNC_PKG_CNT_MAX 200
#define HWAA_DEVICE_IOCTL "hwaa_ioctl"

static dev_t g_dev;
static struct cdev g_cdev;
static struct class *g_cdev_class;

/*
 * This function init tee environment
 * @return 0 for success
 */
static s32 handle_init_tee(struct hwaa_init_tee_t __user *udata)
{
	s32 ret = 0;
	struct hwaa_init_tee_t kdata = {0};

	if (!udata)
		return -EINVAL;
	if (copy_from_user(&kdata, udata, sizeof(struct hwaa_init_tee_t))) {
		hwaa_pr_err("Failed while copying from user space!");
		ret = -EFAULT;
		kdata.ret = HWAA_ERR_INTERNAL;
		goto do_copy_ret;
	}
	kdata.ret = HWAA_SUCCESS;
	kdata.ret = hwaa_init_tee();
	if (kdata.ret != HWAA_SUCCESS) {
		hwaa_pr_err("hwaa_init_tee failed!");
		ret = -EFAULT;
		kdata.ret = HWAA_ERR_INTERNAL;
		goto do_copy_ret;
	}

do_copy_ret:
	if (copy_to_user(&udata->ret, &kdata.ret, sizeof(hwaa_result_t))) {
		hwaa_pr_err("Failed while copying to user space!");
		if (ret == 0)
			ret = -EFAULT;
	}

	return ret;
}

/*
 * This function copy ce key desc to kernel
 * @param udata      [in] ce key desc from user space
 */
static s32 handle_init_user(struct hwaa_init_user_t __user *udata)
{
	s32 ret = 0;
	u8 *addr_key_desc;
	struct hwaa_init_user_t kdata = {0};

	if (!udata)
		return -EINVAL;
	if (copy_from_user(&kdata, udata, sizeof(struct hwaa_init_user_t))) {
		hwaa_pr_err("Failed while copying from user space!");
		ret = -EFAULT;
		kdata.ret = HWAA_ERR_INTERNAL;
		goto do_copy_ret;
	}
	kdata.ret = HWAA_SUCCESS;
	if (kdata.key_desc_len != HWAA_USER_KEY_DESC_MAX) {
		ret = -EINVAL;
		kdata.ret = HWAA_ERR_INVALID_ARGS;
		goto do_copy_ret;
	}

	addr_key_desc = kdata.key_desc;

	kdata.key_desc = kzalloc(kdata.key_desc_len, GFP_KERNEL);
	if (!kdata.key_desc) {
		ret = -ENOMEM;
		kdata.ret = HWAA_ERR_NO_MEMORY;
		goto do_copy_ret;
	}
	if (copy_from_user(kdata.key_desc, addr_key_desc,
		sizeof(u8) * kdata.key_desc_len)) {
		hwaa_pr_err("Failed while copying to user space!");
		ret = -EFAULT;
		kdata.ret = HWAA_ERR_INTERNAL;
		goto do_free_ret;
	}
	init_user(&kdata);

do_free_ret:
	kzfree(kdata.key_desc);
	kdata.key_desc = NULL;

do_copy_ret:
	if (copy_to_user(&udata->ret, &kdata.ret, sizeof(hwaa_result_t))) {
		hwaa_pr_err("Failed while copying to user space!");
		if (ret == 0)
			ret = -EFAULT;
	}

	return ret;
}

static s32 handle_clear_key(struct hwaa_clear_key_t __user *udata)
{
	s32 ret;
	struct hwaa_clear_key_t kdata;

	if (memset_s(&kdata, sizeof(kdata), 0, sizeof(kdata)) != EOK)
		return -EINVAL;
	if (!udata)
		return -EINVAL;
	if (copy_from_user(&kdata, udata, sizeof(struct hwaa_clear_key_t))) {
		hwaa_pr_err("Failed while copying policy from user space!");
		ret = -EFAULT;
		kdata.ret = HWAA_ERR_INTERNAL;
		goto do_copy_ret;
	}

	ret = kernel_clear_credential(kdata.ausn);
	if (ret != 0) {
		hwaa_pr_err("Failed clear_credential!");
		ret = -EFAULT;
		kdata.ret = HWAA_ERR_INTERNAL;
		goto do_copy_ret;
	}

	hwaa_pr_info("clear user(%llu) key", kdata.ausn);
do_copy_ret:
	if (copy_to_user(&udata->ret, &kdata.ret, sizeof(hwaa_result_t))) {
		hwaa_pr_err("Failed while copying to user space!");
		if (ret == 0)
			ret = -EFAULT;
	}
	return ret;
}

static void print_ioctl_cmd_info(u32 cmd)
{
	switch (cmd) {
	case HWAA_INIT_TEE:
		hwaa_pr_info("hwaa_ioctl HWAA_INIT_TEE");
		break;
	case HWAA_INITIALIZE_USER:
		hwaa_pr_info("hwaa_ioctl HWAA_INITIALIZE_USER");
		break;
	case HWAA_CLEAR_KEY:
		hwaa_pr_info("hwaa_ioctl HWAA_CLEAR_KEY");
		break;
	default:
		hwaa_pr_info("hwaa_ioctl UNKNOWN CMD");
	}
}

// we have to use long instead of s64 to avoid warnings, but i promise nowhere else
static long hwaa_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	s32 ret;
	print_ioctl_cmd_info(cmd);

	switch (cmd) {
	case HWAA_INIT_TEE:
		ret = handle_init_tee((struct hwaa_init_tee_t *)arg);
		break;
	case HWAA_INITIALIZE_USER:
		ret = handle_init_user((struct hwaa_init_user_t *)arg);
		break;
	case HWAA_CLEAR_KEY:
		ret = handle_clear_key((struct hwaa_clear_key_t *)arg);
		break;
	default:
		hwaa_pr_warn("Invalid command: %d", cmd);
		ret = -EINVAL;
	}
	return (long)ret;
}

static const struct file_operations g_hwaa_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = hwaa_ioctl
};

static s32 __init hwaa_init(void)
{
	s32 ret;
	struct device *dev_ret;

	hwaa_pr_info("Installing "HWAA_DEVICE_NAME" ...");
	ret = alloc_chrdev_region(&g_dev, HWAA_FIRST_MINOR,
		HWAA_MINOR_CNT, HWAA_DEVICE_NAME);
	if (ret < 0)
		return ret;

	cdev_init(&g_cdev, &g_hwaa_fops);
	ret = cdev_add(&g_cdev, g_dev, HWAA_MINOR_CNT);
	if (ret < 0)
		return ret;

	g_cdev_class = class_create(THIS_MODULE, "s8");
	if (IS_ERR(g_cdev_class)) {
		cdev_del(&g_cdev);
		unregister_chrdev_region(g_dev, HWAA_MINOR_CNT);
		return PTR_ERR(g_cdev_class);
	}

	dev_ret = device_create(g_cdev_class, NULL, g_dev, NULL,
		HWAA_DEVICE_NAME);
	if (IS_ERR(dev_ret)) {
		class_destroy(g_cdev_class);
		cdev_del(&g_cdev);
		unregister_chrdev_region(g_dev, HWAA_MINOR_CNT);
		return PTR_ERR(dev_ret);
	}
	hwaa_register_fs_callbacks_proxy();
	init_list();
	if (!init_hwaa_work_queue()) {
		hwaa_pr_err("create_singlethread_workqueue failed");
		class_destroy(g_cdev_class);
		cdev_del(&g_cdev);
		unregister_chrdev_region(g_dev, HWAA_MINOR_CNT);
		purge_phase1_key();
		return -EINVAL;
	}
	hwaa_pr_info(HWAA_DEVICE_NAME" installed");
	return 0;
}

static void __exit hwaa_exit(void)
{
	destory_hwaa_work_queue();
	purge_phase1_key();
	hwaa_unregister_fs_callbacks_proxy();
	device_destroy(g_cdev_class, g_dev);
	class_destroy(g_cdev_class);
	cdev_del(&g_cdev);
	unregister_chrdev_region(g_dev, HWAA_MINOR_CNT);
}

module_init(hwaa_init);
module_exit(hwaa_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei, 2018");
MODULE_DESCRIPTION("Huawei Application Authentication");
MODULE_VERSION("0.1");
