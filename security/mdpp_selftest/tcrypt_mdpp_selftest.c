/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
 * Description: the tcrypt_mdpp_selftest.c - execute kenel mdpp sleftest
 * Author: wangtienan <wangtienan@huawei.com>
 * Create: 2017-09-23
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/mutex.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <huawei_platform/log/hw_log.h>

/* ----ioctrl cmd macroes--------------------------------------------------- */
#define TCRYPT_IOCTL_BASE 0xEE
#define TCRYPT_START_SELFTEST _IO(TCRYPT_IOCTL_BASE, 1)
#define TCRYPT_READ_RESULT _IOR(TCRYPT_IOCTL_BASE, 2, int)

static ssize_t tcrypt_result_show(struct device *dev,
				struct device_attribute *attr, char *buf);

DEFINE_MUTEX(test_lock);
static DEVICE_ATTR(tcrypt, 0440, tcrypt_result_show, NULL);
static int g_latest_result = -1;
#define HWLOG_TAG       tcrypt_selftest
HWLOG_REGIST();

static ssize_t tcrypt_result_show(struct device *pdev,
				struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", g_latest_result);
}

/*
 * crypto algs of kenel mdpp sleftest.
 */
static int do_alg_tests(void)
{
	int i;
	const char *test_algos[] = {"cbc(aes)", "gcm(aes)", "xts(aes)"};

	for (i = 0; i < ARRAY_SIZE(test_algos); i++) {
		int ret;

		ret = alg_test(test_algos[i], test_algos[i], 0, 0);
		if (ret != 0) {
			hwlog_err("%s alg self test failed in mdpp mode!\n",
				test_algos[i]);
			return -1;
		} else {
			hwlog_info("%s alg self test success\n", test_algos[i]);
		}
	}

	return 0;
}

static long tcrypt_base_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	mutex_lock(&test_lock);

	switch (cmd) {
	case TCRYPT_START_SELFTEST:
		hwlog_info("tcrypt start selftest\n");
		g_latest_result = do_alg_tests();
		break;
	case TCRYPT_READ_RESULT:
		if (copy_to_user((int *)arg, &g_latest_result,
				sizeof(g_latest_result))) {
			ret = -EFAULT;
		}
		hwlog_info("tcrypt read selftest ret=%d result: %d\n",
				ret, g_latest_result);
		break;
	default:
		hwlog_err("Invalid CMD: 0x%x\n", cmd);
		ret = -EFAULT;
		break;
	}

	mutex_unlock(&test_lock);

	return ret;
}

static long tcrypt_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return tcrypt_base_ioctl(file, cmd, arg);
}

static long tcrypt_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return tcrypt_base_ioctl(file, cmd, (unsigned long)compat_ptr(arg));
}

static int tcrypt_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}

/* fops define */
static const struct file_operations tcrypt_fops = {
	.owner	 = THIS_MODULE,
	.unlocked_ioctl = tcrypt_ioctl,
	.open = tcrypt_open,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = tcrypt_compat_ioctl,
#endif
};

/* miscdev define */
static struct miscdevice tcrypt_miscdev = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= "tcrypt",
	.fops		= &tcrypt_fops,
};

static int __init tcrypt_selftest_init(void)
{
	int ret = -1;

	/* (1) register misc dev */
	ret = misc_register(&tcrypt_miscdev);
	if (ret != 0) {
		hwlog_err("tcrypt_miscdev register failed, ret: %d.\n", ret);
		return ret;
	}

	/* (2) create device */
	ret = device_create_file(tcrypt_miscdev.this_device, &dev_attr_tcrypt);
	if (ret != 0) {
		hwlog_err("Failed to create device file for tcrypt\n");
		return ret;
	}

	return 0;
}

/*
 * If an init function is provided, an exit function must also be provided
 * to allow module unload.
 */
static void __exit tcrypt_selftest_exit(void)
{
	misc_deregister(&tcrypt_miscdev);
}

device_initcall(tcrypt_selftest_init);
module_exit(tcrypt_selftest_exit);

MODULE_AUTHOR("HUAWEI Technologies Co., Ltd.");
MODULE_DESCRIPTION("Tests for a subset of crypto algo");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL v2");

