/*
 * hw_hishow.c
 *
 * hishow driver
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <huawei_platform/usb/hw_hishow.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG hw_hishow
HWLOG_REGIST();

static struct class *hishow_class;
static struct device *hishow_device;

struct hishow_info *g_hishow_di;

static const char * const hishow_device_table[] = {
	[HISHOW_UNKNOWN_DEVICE] = "unknown_hishow",
	[HISHOW_USB_DEVICE] = "usb_hishow",
	[HISHOW_HALL_DEVICE] = "hall_hishow",
};

static ssize_t hishow_dev_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	static const char * const dev_state[] = {
		[HISHOW_UNKNOWN] = "UNKNOWN",
		[HISHOW_DISCONNECTED] = "DISCONNECTED",
		[HISHOW_CONNECTED] = "CONNECTED",
	};
	const char *cur_state = dev_state[HISHOW_UNKNOWN];

	if (!g_hishow_di) {
		hwlog_err("g_hishow_di is null\n");
		return scnprintf(buf, PAGE_SIZE, "%s\n", cur_state);
	}

	hwlog_info("%d:%x\n", g_hishow_di->dev_state, g_hishow_di->dev_no);

	switch (g_hishow_di->dev_state) {
	case HISHOW_DEVICE_OFFLINE:
		cur_state = dev_state[HISHOW_DISCONNECTED];
		break;

	case HISHOW_DEVICE_ONLINE:
		cur_state = dev_state[HISHOW_CONNECTED];
		break;

	default:
		cur_state = dev_state[HISHOW_UNKNOWN];
		break;
	}

	return scnprintf(buf, PAGE_SIZE, "%s:%d\n",
		cur_state, g_hishow_di->dev_no);
}

static DEVICE_ATTR(dev, 0444, hishow_dev_info_show, NULL);

static struct attribute *hishow_ctrl_attributes[] = {
	&dev_attr_dev.attr,
	NULL,
};
static const struct attribute_group hishow_attr_group = {
	.attrs = hishow_ctrl_attributes,
};

void hishow_notify_android_uevent(int dev_state, int dev_no)
{
	char *offline[HISHOW_STATE_MAX] = {
		"HISHOWDEV_STATE=DISCONNECTED", NULL, NULL
	};
	char *online[HISHOW_STATE_MAX] = {
		"HISHOWDEV_STATE=CONNECTED", NULL, NULL
	};
	char *unknown[HISHOW_STATE_MAX] = {
		"HISHOWDEV_STATE=UNKNOWN", NULL, NULL
	};
	char device_data[HISHOW_DEV_DATA_MAX] = {0};

	if (IS_ERR(hishow_device) || !g_hishow_di) {
		hwlog_err("hishow_device or g_hishow_di is null\n");
		return;
	}

	if ((dev_no <= HISHOW_DEVICE_BEGIN) ||
		(dev_no >= HISHOW_DEVICE_END)) {
		hwlog_err("invalid hishow_devno(%d)\n", dev_no);
		return;
	}

	g_hishow_di->dev_state = dev_state;
	g_hishow_di->dev_no = dev_no;

	snprintf(device_data, HISHOW_DEV_DATA_MAX, "DEVICENO=%d", dev_no);

	switch (dev_state) {
	case HISHOW_DEVICE_ONLINE:
		online[1] = device_data;
		kobject_uevent_env(&hishow_device->kobj, KOBJ_CHANGE, online);
		hwlog_info("hishow_notify kobject_uevent_env connected\n");
		break;

	case HISHOW_DEVICE_OFFLINE:
		offline[1] = device_data;
		kobject_uevent_env(&hishow_device->kobj, KOBJ_CHANGE, offline);
		hwlog_info("hishow_notify kobject_uevent_env disconnected\n");
		break;

	default:
		unknown[1] = device_data;
		kobject_uevent_env(&hishow_device->kobj, KOBJ_CHANGE, unknown);
		hwlog_info("hishow_notify kobject_uevent_env unknown\n");
		break;
	}
}
EXPORT_SYMBOL_GPL(hishow_notify_android_uevent);

static void hishow_destroy_monitor_device(struct platform_device *pdev)
{
	if (!pdev)
		return;

	if (!IS_ERR(hishow_device)) {
		sysfs_remove_group(&hishow_device->kobj, &hishow_attr_group);
		device_destroy(hishow_device->class, hishow_device->devt);
	}

	if (!IS_ERR(hishow_class))
		class_destroy(hishow_class);

	hishow_device = NULL;
	hishow_class = NULL;
}

static int hishow_init_monitor_device(struct platform_device *pdev)
{
	int ret;

	if (hishow_device || hishow_class)
		hishow_destroy_monitor_device(pdev);

	hishow_class = class_create(THIS_MODULE, "hishow");
	if (IS_ERR(hishow_class)) {
		hwlog_err("cannot create class\n");
		ret = PTR_ERR(hishow_class);
		goto fail_create_class;
	}

	if (hishow_class) {
		hishow_device = device_create(hishow_class, NULL, 0, NULL,
			"monitor");
		if (IS_ERR(hishow_device)) {
			hwlog_err("sysfs device create failed\n");
			ret = PTR_ERR(hishow_device);
			goto fail_create_device;
		}
	}

	ret = sysfs_create_group(&hishow_device->kobj, &hishow_attr_group);
	if (ret) {
		hwlog_err("sysfs group create failed\n");
		goto fail_create_sysfs;
	}

	return 0;

fail_create_sysfs:
fail_create_device:
fail_create_class:
	hishow_destroy_monitor_device(pdev);

	return ret;
}

static int hishow_probe(struct platform_device *pdev)
{
	struct hishow_info *di = NULL;
	int ret = -1;

	hwlog_info("probe begin\n");

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_hishow_di = di;
	g_hishow_di->dev_state = HISHOW_DEVICE_OFFLINE;
	g_hishow_di->dev_no = HISHOW_UNKNOWN_DEVICE;

	di->pdev = pdev;
	di->dev = &pdev->dev;
	if (!di->pdev || !di->dev) {
		hwlog_err("device_node is null\n");
		goto fail_free_mem;
	}

	ret = hishow_init_monitor_device(pdev);
	if (ret)
		goto fail_free_mem;

	platform_set_drvdata(pdev, di);

	hwlog_info("probe end\n");
	return 0;

fail_free_mem:
	devm_kfree(&pdev->dev, di);
	g_hishow_di = NULL;

	return ret;
}

static int hishow_remove(struct platform_device *pdev)
{
	struct hishow_info *di = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	hishow_destroy_monitor_device(pdev);

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_hishow_di = NULL;

	hwlog_info("remove end\n");
	return 0;
}


static const struct of_device_id hishow_match_table[] = {
	{
		.compatible = "huawei,hishow",
		.data = NULL,
	},
	{},
};

static struct platform_driver hishow_driver = {
	.probe = hishow_probe,
	.remove = hishow_remove,
	.driver = {
		.name = "huawei,hishow",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hishow_match_table),
	},
};

static int __init hishow_init(void)
{
	return platform_driver_register(&hishow_driver);
}

static void __exit hishow_exit(void)
{
	platform_driver_unregister(&hishow_driver);
}

late_initcall(hishow_init);
module_exit(hishow_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei hishow event module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
