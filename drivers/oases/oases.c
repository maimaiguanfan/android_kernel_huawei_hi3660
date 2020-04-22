/*
 * initialize OASES framework
 *
 * Copyright (C) 2016 Baidu, Inc. All Rights Reserved.
 *
 * You should have received a copy of license along with this program;
 * if not, ask for it from Baidu, Inc.
 *
 */

#include <linux/device.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "patch_mgr.h"
#include "oases_signing.h"
#include "patch_file.h"
#include "util.h"
#include "inlinehook.h"
#include "plts.h"
#include "sysfs.h"

#define DEVNAME "oases"
#define PATCHSLABNAME "oases_patch_cache"

#define OASES_MAX_PATCH_SIZE (PAGE_SIZE * 4)
#define OASES_MIN_PATCH_SIZE sizeof(struct oases_patch_header)

#define	OASES_IOCTL_BASE 0xBB
#define OASES_IOCTL_REMOVE_PATCH _IOW(OASES_IOCTL_BASE, 1, struct oases_unpatch)

#ifdef CONFIG_HISI_HHEE
unsigned long oases_hkip_token;
#endif

struct oases_dev
{
	struct cdev cdev;
	struct class *dev_cls;
	struct device *device;
};

DEFINE_MUTEX(oases_mutex);

static dev_t devno;
static struct oases_dev *oases_devp = NULL;
static atomic_t oases_avail = ATOMIC_INIT(1);

static __always_inline int oases_op_patch(struct oases_patch_file *pfile)
{
	int ret;
	struct oases_attack_log *plog;
	struct oases_patch_info *info = NULL;
	int (*code_entry)(struct oases_patch_info *info) = NULL;
	void (*init)(void) = NULL;

	info = kmem_cache_alloc(oases_patch_slab, __GFP_ZERO);

	if (!info) {
		oases_error("kmem_cache_alloc info fail\n");
		return -ENOMEM;
	}

	(void)strncpy(info->id, pfile->pheader->id, PATCH_ID_LEN - 1);
	info->version = pfile->pheader->patch_version;
	info->status = STATUS_DISABLED;
	INIT_LIST_HEAD(&info->patches);

	plog = kzalloc(sizeof(struct oases_attack_log) * OASES_LOG_NODE_MAX,
		GFP_KERNEL);
	if (!plog) {
		oases_error("kzalloc log fail\n");
		ret = -ENOMEM;
		goto fail_alloc_log;;
	}
	plog->uid = OASES_INVALID_UID;
	info->log_index = 1;
	info->plog = plog;
	spin_lock_init(&info->log_lock);

	ret = oases_build_code(info, pfile);
	if (ret < 0) {
		oases_error("oases_build_code fail\n");
		goto fail_build_code;
	}

	code_entry = info->code_entry;
	ret = (*code_entry)(info);
	oases_debug("code_entry ret=%d\n", ret);
	if (ret != OASES_PATCH_SUCCESS) {
		if (ret > 0) {
			/* OASES_PATCH_FAILURE here */
			ret = -EINVAL;
		}
		goto fail_entry;
	}

	oases_sysfs_init_patch(info);
	ret = oases_sysfs_add_patch(info);
	if (ret < 0) {
		oases_error("oases_sysfs_add_patch fail\n");
		goto fail_sysfs;
	}

	info->attached = 1;
	list_add_tail(&info->list, &patchinfo_list);

	init = info->cbs.init;
	if (init)
		init();

	return 0;

fail_sysfs:
	oases_sysfs_del_patch(info);
	return ret;
fail_entry:
	vfree(info->code_base);
fail_build_code:
	kfree(plog);
fail_alloc_log:
	kmem_cache_free(oases_patch_slab, info);
	return ret;
}

static __always_inline int oases_op_unpatch(struct oases_unpatch *p)
{
	int ret;
	struct oases_patch_info *info = NULL;
	struct oases_unpatch oup;
	void (*exit)(void) = NULL;

	memset(&oup, 0, sizeof(struct oases_unpatch));
	ret = copy_from_user(&oup, p, sizeof(*p));
	if (ret)
		return -EFAULT;

	oup.id[PATCH_ID_LEN -1] = '\0';
	info = patch_info_from_id(&oup.id[0]);
	if (info == NULL) {
		oases_error("warning: no patch: %s\n", oup.id);
		return -ENOENT;
	}
	if (info->status == STATUS_ENABLED) {
		oases_error("error: patch enabled %s\n", oup.id);
		return -EINVAL;
	}
	ret = oases_remove_patch(info);
	if (ret)
		return ret;
	if (info->attached) {
		list_del(&info->list);
		info->attached = 0;
	}
	exit = info->cbs.exit;
	if (exit)
		exit();
	oases_sysfs_del_patch(info);
	return 0;
}

static ssize_t oases_write(struct file *filp, const char __user *buf,
		size_t size, loff_t *ppos)
{
	int ret;
	void *data;
	struct oases_patch_file pfile = {0};

	if (size < OASES_MIN_PATCH_SIZE || size > OASES_MAX_PATCH_SIZE) {
		oases_error("bad patch size\n");
		return -EINVAL;
	}

	mutex_lock(&oases_mutex);

	data = kzalloc(size, GFP_KERNEL);
	if (!data) {
		oases_error("kzalloc failed\n");
		ret = -ENOMEM;
		goto fail;
	}

	if (copy_from_user(data, buf, size)) {
		oases_error("copy_from_user failed\n");
		ret = -EFAULT;
		goto fail;
	}

	pfile.len = size;
	ret = oases_init_patch_file(&pfile, data);
	if (ret < 0) {
		oases_error("oases_init_patch_file failed\n");
		goto fail;
	}

	ret = oases_op_patch(&pfile);
	if (ret < 0) {
		oases_error("oases_op_patch failed\n");
		goto fail;
	}
	ret = size;

fail:
	if (data)
		kfree(data);
	mutex_unlock(&oases_mutex);
	return ret;
}

/* only one process can open the device */
static int oases_open(struct inode *inode, struct file *filp)
{
	if (!atomic_dec_and_test(&oases_avail)) {
		atomic_inc(&oases_avail);
		return -EBUSY;
	}

	filp->private_data = oases_devp;
	return 0;
}

static long oases_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	int ret;

	oases_debug("OASES_IOCTL_REMOVE_PATCH=%#lx, cmd=%#x\n",
			(long)OASES_IOCTL_REMOVE_PATCH, cmd);

	switch (cmd) {
	case OASES_IOCTL_REMOVE_PATCH:
		mutex_lock(&oases_mutex);
		ret = oases_op_unpatch((void __user *)arg);
		mutex_unlock(&oases_mutex);
		break;
	default:
		oases_error("unrecognized ioctl cmd %d\n", cmd);
		ret = -EINVAL;
		break;
	}

	return ret;
}

#if defined(__aarch64__) && defined(CONFIG_COMPAT)
static long oases_compat_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	/* current oases ioctl/compat_ioctl shares the same cmds/args */
	return oases_ioctl(filp, cmd, arg);
}
#endif

static int oases_release(struct inode *inode, struct file *filp)
{
	atomic_inc(&oases_avail);
	oases_debug("oases_release\n");
	return 0;
}

static const struct file_operations oases_fops =
{
	.write = oases_write,
	.open = oases_open,
	.unlocked_ioctl = oases_ioctl,
#if defined(__aarch64__) && defined(CONFIG_COMPAT)
	.compat_ioctl = oases_compat_ioctl,
#endif
	.release = oases_release,
};

static int __init oases_init(void)
{
	int ret;

	ret = alloc_chrdev_region(&devno, 0, 1, DEVNAME);
	if (ret < 0) {
		oases_error("alloc_chrdev_region failed\n");
		return ret;
	}

	oases_patch_slab = kmem_cache_create(PATCHSLABNAME,
		sizeof(struct oases_patch_info), 0, SLAB_HWCACHE_ALIGN, NULL);

	if (oases_patch_slab == NULL) {
		oases_error("%s: Can't create OasesPatch SLAB cache!\n",
			PATCHSLABNAME);
		ret = -ENOBUFS;
		goto patchslab_create_fail;
	}

	oases_devp = kzalloc(sizeof(*oases_devp), GFP_KERNEL);
	if (!oases_devp) {
		ret = -ENOMEM;
		oases_error("kzalloc failed\n");
		goto kmalloc_fail;
	}

	cdev_init(&oases_devp->cdev, &oases_fops);
	oases_devp->cdev.owner = THIS_MODULE;
	oases_devp->cdev.ops = &oases_fops;

	ret = cdev_add(&oases_devp->cdev, devno, 1);
	if (ret < 0) {
		oases_error("cdev_add failed\n");
		goto cdev_add_fail;
	}

	oases_devp->dev_cls = class_create(THIS_MODULE, DEVNAME);
	if (IS_ERR(oases_devp->dev_cls)) {
		ret = PTR_ERR(oases_devp->dev_cls);
		oases_error("class_create failed\n");
		goto class_create_fail;
	}

	oases_devp->device = device_create(oases_devp->dev_cls, NULL, devno, NULL, DEVNAME);
	if (IS_ERR(oases_devp->device)) {
		ret = PTR_ERR(oases_devp->device);
		oases_error("device_create failed\n");
		goto device_create_fail;
	}

	ret = oases_sysfs_init();
	if (ret < 0) {
		oases_error("oases_sysfs_init failed\n");
		goto sysfs_init_fail;
	}

	ret = oases_plts_init();
	if (ret < 0) {
		oases_error("oases_plts_init failed\n");
		goto init_plts_fail;
	}

	ret = oases_init_signing_keys();
	if (ret < 0) {
		oases_error("oases_init_signing_keys failed\n");
		goto init_keys_fail;
	}

#ifdef CONFIG_HISI_HHEE
	if (is_hkip_enabled()) {
		ret = get_oases_hkip_token(&oases_hkip_token);
		if (ret < 0) {
			oases_error("oases_get_hkip_token failed\n");
		}
	}
#endif

	oases_info("oases_init success\n");
	return 0;

init_keys_fail:
	oases_plts_free();
init_plts_fail:
	oases_sysfs_destroy();
sysfs_init_fail:
	device_destroy(oases_devp->dev_cls, devno);
device_create_fail:
	class_destroy(oases_devp->dev_cls);
class_create_fail:
	cdev_del(&oases_devp->cdev);
cdev_add_fail:
	kfree(oases_devp);
kmalloc_fail:
	kmem_cache_destroy(oases_patch_slab);
patchslab_create_fail:
	unregister_chrdev_region(devno, 1);
	return ret;
}

/*
 * This function won't be compiled into kernel when builtin.
 */
static void __exit oases_exit(void)
{
	oases_destroy_signing_keys();
	oases_plts_free();
	oases_sysfs_destroy();
	cdev_del(&oases_devp->cdev);
	device_destroy(oases_devp->dev_cls, devno);
	class_destroy(oases_devp->dev_cls);
	unregister_chrdev_region(devno, 1);
	mutex_destroy(&oases_mutex);
	kfree(oases_devp);
	oases_debug("oases_exit success\n");
}

module_init(oases_init);
module_exit(oases_exit);

MODULE_AUTHOR("Baidu, Inc.");
MODULE_DESCRIPTION("OASES - Open Adaptive Security Extensions");
