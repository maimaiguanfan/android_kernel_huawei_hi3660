
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
#include "huawei_platform/hwaa/hwaa_proc_hooks.h"
#include "huawei_platform/hwaa/hwaa_limits.h"

#include "inc/data/hwaa_data.h"
#include "inc/fek/hwaa_fs_callbacks.h"
#include "inc/policy/hwaa_policy.h"
#include "inc/proc_mgmt/hwaa_proc_callbacks.h"
#include "inc/proc_mgmt/hwaa_proc_mgmt.h"
#include "inc/tee/wrappers.h"
#include "inc/base/amap.h"
#include "inc/tee/hwaa_tee.h"
#include "inc/base/macros.h"
#include "inc/base/hwaa_utils.h"
#include "inc/tee/khandler.h"

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

	ret = clear_qzone_key(kdata.ausn);
	if (ret != 0) {
		hwaa_pr_err("Failed clear_qzone_key!");
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
	case HWAA_SYNC_INSTALLED_PACKAGES:
		hwaa_pr_info("hwaa_ioctl HWAA_SYNC_INSTALLED_PACKAGES");
		break;
	case HWAA_CLEAR_KEY:
		hwaa_pr_info("hwaa_ioctl HWAA_CLEAR_KEY");
		break;
	case HWAA_INSTALL_PACKAGE:
		hwaa_pr_info("hwaa_ioctl HWAA_INSTALL_PACKAGE");
		break;
	case HWAA_UNINSTALL_PACKAGE:
		hwaa_pr_info("hwaa_ioctl HWAA_UNINSTALL_PACKAGE");
		break;
	default:
		hwaa_pr_info("hwaa_ioctl UNKNOWN CMD");
	}
}

static bool handle_install_package_init(struct hwaa_install_package_t *kdata,
	struct hwaa_install_package_t *kdata_store)
{
	if (memset_s(kdata, sizeof(struct hwaa_install_package_t), 0,
		sizeof(struct hwaa_install_package_t)) != EOK)
		return false;
	if (memset_s(kdata_store, sizeof(struct hwaa_install_package_t), 0,
		sizeof(struct hwaa_install_package_t)) != EOK)
		return false;
	return true;
}

static s32 handle_install_package(struct hwaa_install_package_t __user *udata)
{
	s32 ret = 0;

	struct hwaa_install_package_t kdata;
	struct hwaa_install_package_t kdata_store_addr;

	if (!udata)
		return -EINVAL;

	if (!handle_install_package_init(&kdata, &kdata_store_addr))
		return -EINVAL;

	if (copy_from_user(&kdata, udata, sizeof(struct hwaa_install_package_t))
	    || copy_from_user(&kdata_store_addr, udata,
	    sizeof(struct hwaa_install_package_t))) {
		hwaa_pr_err("Failed while copying from user space!");
		ret = -EFAULT;
		kdata.ret = HWAA_ERR_INTERNAL;
		goto do_copy_ret;
	}

	if (hwaa_utils_copy_package_info_from_user(&kdata.pinfo,
		&kdata_store_addr.pinfo, true)) {
		ret = -EINVAL;
		kdata.ret = HWAA_ERR_INVALID_ARGS;
		goto do_copy_ret;
	}

	hwaa_install_package(&kdata);
	hwaa_utils_free_package_info(&kdata.pinfo);

do_copy_ret:
	if (copy_to_user(&udata->ret, &kdata.ret, sizeof(hwaa_result_t))) {
		hwaa_pr_err("Failed while copying to user space!");
		if (!ret)
			ret = -EFAULT;
	}

	return ret;
}

static bool handle_uninstall_package_init(struct hwaa_uninstall_package_t *kdata,
	struct hwaa_uninstall_package_t *kdata_store)
{
	if (memset_s(kdata, sizeof(struct hwaa_uninstall_package_t), 0,
		sizeof(struct hwaa_uninstall_package_t)) != EOK)
		return false;
	if (memset_s(kdata_store, sizeof(struct hwaa_uninstall_package_t), 0,
		sizeof(struct hwaa_uninstall_package_t)) != EOK)
		return false;
	return true;
}

static s32 handle_uninstall_package(
	struct hwaa_uninstall_package_t __user *udata)
{
	s32 ret = 0;
	struct hwaa_uninstall_package_t kdata;
	struct hwaa_uninstall_package_t kdata_store_addr;

	if (!udata)
		return -EINVAL;
	if (!handle_uninstall_package_init(&kdata, &kdata_store_addr))
		return -EINVAL;
	if (copy_from_user(&kdata, udata,
		sizeof(struct hwaa_uninstall_package_t)) ||
		copy_from_user(&kdata_store_addr, udata,
		sizeof(struct hwaa_uninstall_package_t))) {
		hwaa_pr_err("Failed while copying from user space!");
		ret = -EFAULT;
		kdata.ret = HWAA_ERR_INTERNAL;
		goto do_copy_ret;
	}
	if (hwaa_utils_copy_package_info_from_user(&kdata.pinfo,
		&kdata_store_addr.pinfo, false)) {
		ret = -EINVAL;
		kdata.ret = HWAA_ERR_INVALID_ARGS;
		goto do_copy_ret;
	}

	hwaa_uninstall_package(&kdata);
	hwaa_utils_free_package_info(&kdata.pinfo);
do_copy_ret:
	if (copy_to_user(&udata->ret, &kdata.ret, sizeof(hwaa_result_t))) {
		hwaa_pr_err("Failed while copying to user space!");
		if (!ret)
			ret = -EFAULT;
	}

	return ret;
}

static void free_kdata_packages(struct hwaa_sync_installed_packages_t *kdata)
{
	s32 i;
	if (kdata->packages) {
		for (i = 0; i < kdata->package_count; i++)
			hwaa_utils_free_package_info(&kdata->packages[i]);
		kfree(kdata->packages);
	}
}

static s32 copy_sync_pkgs_level1(struct hwaa_sync_installed_packages_t *kdata,
	struct hwaa_sync_installed_packages_t __user *udata,
	struct hwaa_package_info_t **packages_user_addr)
{
	s32 ret = 0;

	if (memset_s(kdata, sizeof(*kdata), 0, sizeof(*kdata)) != EOK) {
		kdata->ret = HWAA_ERR_INTERNAL;
		kdata->package_count = 0;
		ret = -EFAULT;
		goto out;
	}
	if (copy_from_user(kdata, udata,
		sizeof(struct hwaa_sync_installed_packages_t))) {
		hwaa_pr_err("Failed while copying from user space!");
		kdata->ret = HWAA_ERR_INTERNAL;
		kdata->package_count = 0;
		ret = -EFAULT;
		goto out;
	}
	if ((kdata->package_count <= 0) ||
		(kdata->package_count > SYNC_PKG_CNT_MAX)) {
		kdata->ret = HWAA_ERR_INVALID_ARGS;
		kdata->package_count = 0;
		ret = -EFAULT;
		goto out;
	}
	*packages_user_addr = kdata->packages;
out:
	kdata->packages = NULL;
	return ret;
}

static s32 copy_sync_pkgs_level2(
	struct hwaa_package_info_t **packages_transform,
	struct hwaa_package_info_t *packages_user_addr,
	struct hwaa_sync_installed_packages_t *kdata)
{
	*packages_transform = kcalloc(kdata->package_count,
		sizeof(struct hwaa_package_info_t), GFP_KERNEL);// outside free
	if (!(*packages_transform)) {
		kdata->ret = HWAA_ERR_INVALID_ARGS;
		kdata->package_count = 0;
		return -ENOMEM;
	}
	if (copy_from_user(*packages_transform, packages_user_addr,
		sizeof(struct hwaa_package_info_t) * kdata->package_count)) {
		hwaa_pr_err("Failed while copying addr_pkgs from user space!");
		kdata->ret = HWAA_ERR_INVALID_ARGS;
		kdata->package_count = 0;
		return -EFAULT;
	}
	return 0;
}

static s32 copy_sync_pkgs_level3(struct hwaa_sync_installed_packages_t *kdata,
	struct hwaa_package_info_t *packages_transform)
{
	s32 ret;

	kdata->packages = kcalloc(kdata->package_count,
		sizeof(struct hwaa_package_info_t),GFP_KERNEL);
	if (!kdata->packages) {
		kdata->ret = HWAA_ERR_NO_MEMORY;
		kdata->package_count = 0;
		return -ENOMEM;
	}
	ret = hwaa_utils_copy_packages_from_user(kdata->packages,
		packages_transform, kdata->package_count);
	if (ret != 0) {
		if (ret == -ENOMEM)
			kdata->ret = HWAA_ERR_NO_MEMORY;
		else if (ret == -EINVAL)
			kdata->ret = HWAA_ERR_INVALID_ARGS;
		else
			kdata->ret = HWAA_ERR_INTERNAL;
	}
	return ret;
}

static s32 handle_sync_installed_packages(
	struct hwaa_sync_installed_packages_t __user *udata)
{
	s32 ret;
	struct hwaa_sync_installed_packages_t kdata;
	struct hwaa_package_info_t *packages_user_addr = NULL;
	struct hwaa_package_info_t *packages_transform = NULL;

	if (!udata)
		return -EINVAL;
	// copy 1st level data
	ret = copy_sync_pkgs_level1(&kdata, udata, &packages_user_addr);
	if (ret != 0)
		goto do_copy_ret;
	// copy 2nd level data
	ret = copy_sync_pkgs_level2(&packages_transform, packages_user_addr,
		&kdata);
	if (ret != 0)
		goto do_free_pkg;
	// copy 3rd level data
	ret = copy_sync_pkgs_level3(&kdata, packages_transform);
	if (ret != 0)
		goto do_free_pkg;
	hwaa_sync_installed_packages(&kdata);

do_free_pkg:
	free_kdata_packages(&kdata);
	kfree(packages_transform);
do_copy_ret:
	if (copy_to_user(&udata->ret, &kdata.ret, sizeof(hwaa_result_t))) {
		hwaa_pr_err("Failed while copying to user space!");
		ret = -EFAULT;
	}
	return ret;
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
	case HWAA_INSTALL_PACKAGE:
		ret = handle_install_package(
			(struct hwaa_install_package_t *)arg);
		break;
	case HWAA_UNINSTALL_PACKAGE:
		ret = handle_uninstall_package(
			(struct hwaa_uninstall_package_t *)arg);
		break;
	case HWAA_SYNC_INSTALLED_PACKAGES:
		ret = handle_sync_installed_packages(
			(struct hwaa_sync_installed_packages_t *)arg);
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
	hwaa_data_init();

	hwaa_register_fs_callbacks_proxy();
	hwaa_register_proc_callbacks_proxy();

	if (!init_hwaa_work_queue()) {
		hwaa_pr_err("create_singlethread_workqueue failed");
		class_destroy(g_cdev_class);
		cdev_del(&g_cdev);
		unregister_chrdev_region(g_dev, HWAA_MINOR_CNT);
		return -EINVAL;
	}
	hwaa_pr_info(HWAA_DEVICE_NAME" installed");
	return 0;
}

static void __exit hwaa_exit(void)
{
	destory_hwaa_work_queue();
	purge_phase1_key_map();
	hwaa_data_deinit();
	hwaa_unregister_proc_callbacks_proxy();
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
