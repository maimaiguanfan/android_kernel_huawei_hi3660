/*
 * hw_usb.c
 *
 * usb driver
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

#include <linux/device.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/usb.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/usb/hw_usb.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG hw_usb
HWLOG_REGIST();

struct hw_usb_device *g_hw_usb_di;

static struct class *hw_usb_class;
static struct device *hw_usb_dev;

static unsigned int g_hw_usb_ldo_status;
static DEFINE_MUTEX(g_hw_usb_ldo_op_mutex);

static unsigned int g_hw_usb_speed = USB_SPEED_UNKNOWN;
static unsigned int g_hw_usb_abnormal_event = USB_HOST_EVENT_NORMAL;

static const char *hw_usb_ldo_ctrl_strings(enum hw_usb_ldo_ctrl_type type)
{
	static const char * const string_table[] = {
		[HW_USB_LDO_CTRL_USB] = "USB",
		[HW_USB_LDO_CTRL_COMBOPHY] = "COMBOPHY",
		[HW_USB_LDO_CTRL_DIRECT_CHARGE] = "DC",
		[HW_USB_LDO_CTRL_HIFIUSB] = "HIFIUSB",
		[HW_USB_LDO_CTRL_TYPECPD] = "TYPECPD",
	};

	if (type < HW_USB_LDO_CTRL_BEGIN || type >= HW_USB_LDO_CTRL_MAX)
		return "illegal type";

	return string_table[type];
}
int hw_usb_ldo_supply_enable(enum hw_usb_ldo_ctrl_type type)
{
	int ret;

	if (type >= HW_USB_LDO_CTRL_MAX) {
		hwlog_err("type(%d) is invalid\n", type);
		return -EINVAL;
	}

	hwlog_info("count(%d), type(%s)\n",
		g_hw_usb_ldo_status, hw_usb_ldo_ctrl_strings(type));

	if (!g_hw_usb_di || !g_hw_usb_di->usb_phy_ldo) {
		hwlog_err("g_hw_usb_di or usb_phy_ldo is null\n");
		return -EINVAL;
	}

	mutex_lock(&g_hw_usb_ldo_op_mutex);

	if (g_hw_usb_ldo_status == 0) {
		ret = regulator_enable(g_hw_usb_di->usb_phy_ldo);
		if (ret) {
			hwlog_err("enable failed(%d)\n", ret);
			mutex_unlock(&g_hw_usb_ldo_op_mutex);
			return -EPERM;
		}
	}
	g_hw_usb_ldo_status =  g_hw_usb_ldo_status | (1 << type);

	mutex_unlock(&g_hw_usb_ldo_op_mutex);

	hwlog_info("enable(%s) success\n", hw_usb_ldo_ctrl_strings(type));
	return 0;
}
EXPORT_SYMBOL_GPL(hw_usb_ldo_supply_enable);

int hw_usb_ldo_supply_disable(enum hw_usb_ldo_ctrl_type type)
{
	int ret;

	if (type >= HW_USB_LDO_CTRL_MAX) {
		hwlog_err("type(%d) is invalid\n", type);
		return -EINVAL;
	}

	hwlog_info("count(%d), type(%s)\n",
		g_hw_usb_ldo_status, hw_usb_ldo_ctrl_strings(type));

	if (!g_hw_usb_di || !g_hw_usb_di->usb_phy_ldo) {
		hwlog_err("g_hw_usb_di or usb_phy_ldo is null\n");
		return -EINVAL;
	}

	mutex_lock(&g_hw_usb_ldo_op_mutex);

	if (g_hw_usb_ldo_status != 0) {
		g_hw_usb_ldo_status = g_hw_usb_ldo_status & (~(1 << type));
		if (g_hw_usb_ldo_status == 0) {
			ret = regulator_disable(g_hw_usb_di->usb_phy_ldo);
			if (ret) {
				hwlog_err("disable failed(%d)\n", ret);
				mutex_unlock(&g_hw_usb_ldo_op_mutex);
				return -EPERM;
			}
		}
	}

	mutex_unlock(&g_hw_usb_ldo_op_mutex);

	hwlog_info("disable(%s) success\n", hw_usb_ldo_ctrl_strings(type));
	return 0;
}
EXPORT_SYMBOL_GPL(hw_usb_ldo_supply_disable);

void hw_usb_set_usb_speed(unsigned int usb_speed)
{
	g_hw_usb_speed = usb_speed;

	hwlog_info("usb_speed=%d\n", usb_speed);

	if (usb_speed == USB_SPEED_UNKNOWN)
		pd_dpm_send_event(USB31_CABLE_OUT_EVENT);

	if ((usb_speed == USB_SPEED_SUPER) ||
		(usb_speed == USB_SPEED_SUPER_PLUS))
		pd_dpm_send_event(USB31_CABLE_IN_EVENT);
}
EXPORT_SYMBOL_GPL(hw_usb_set_usb_speed);

static unsigned int hw_usb_get_usb_speed(void)
{
	hwlog_info("g_hw_usb_speed=%d\n", g_hw_usb_speed);

	return g_hw_usb_speed;
}

static ssize_t hw_usb_speed_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int len;

	switch (hw_usb_get_usb_speed()) {
	case USB_SPEED_UNKNOWN:
		len = scnprintf(buf, PAGE_SIZE, "%s", "unknown");
		break;

	/* fall through: low speed also defined full speed */
	case USB_SPEED_LOW:
	case USB_SPEED_FULL:
		len = scnprintf(buf, PAGE_SIZE, "%s", "full-speed");
		break;

	case USB_SPEED_HIGH:
		len = scnprintf(buf, PAGE_SIZE, "%s", "high-speed");
		break;

	case USB_SPEED_WIRELESS:
		len = scnprintf(buf, PAGE_SIZE, "%s", "wireless-speed");
		break;

	case USB_SPEED_SUPER:
		len = scnprintf(buf, PAGE_SIZE, "%s", "super-speed");
		break;

	case USB_SPEED_SUPER_PLUS:
		len = scnprintf(buf, PAGE_SIZE, "%s", "super-speed-plus");
		break;

	default:
		len = scnprintf(buf, PAGE_SIZE, "%s", "unknown");
		break;
	}

	return len;
}

void hw_usb_host_abnormal_event_notify(unsigned int event)
{
	hwlog_info("event=%d\n", event);

	if ((g_hw_usb_abnormal_event == USB_HOST_EVENT_HUB_TOO_DEEP) &&
		(event == USB_HOST_EVENT_UNKNOW_DEVICE))
		g_hw_usb_abnormal_event = USB_HOST_EVENT_HUB_TOO_DEEP;
	else
		g_hw_usb_abnormal_event = event;
}
EXPORT_SYMBOL_GPL(hw_usb_host_abnormal_event_notify);

static unsigned int usb_host_get_abnormal_event(void)
{
	hwlog_info("g_hw_usb_abnormal_event=%d\n", g_hw_usb_abnormal_event);

	return g_hw_usb_abnormal_event;
}

static ssize_t hw_usb_host_abnormal_event_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int len;

	switch (usb_host_get_abnormal_event()) {
	case USB_HOST_EVENT_NORMAL:
		len = scnprintf(buf, PAGE_SIZE, "%s", "normal");
		break;

	case USB_HOST_EVENT_POWER_INSUFFICIENT:
		len = scnprintf(buf, PAGE_SIZE, "%s", "power_insufficient");
		break;

	case USB_HOST_EVENT_HUB_TOO_DEEP:
		len = scnprintf(buf, PAGE_SIZE, "%s", "hub_too_deep");
		break;

	case USB_HOST_EVENT_UNKNOW_DEVICE:
		len = scnprintf(buf, PAGE_SIZE, "%s", "unknown_device");
		break;

	default:
		len = scnprintf(buf, PAGE_SIZE, "%s", "invalid");
		break;
	}

	return len;
}

static DEVICE_ATTR(usb_speed, 0444, hw_usb_speed_show, NULL);
static DEVICE_ATTR(usb_event, 0444, hw_usb_host_abnormal_event_show, NULL);

static struct attribute *hw_usb_ctrl_attributes[] = {
	&dev_attr_usb_speed.attr,
	&dev_attr_usb_event.attr,
	NULL,
};

static const struct attribute_group hw_usb_attr_group = {
	.attrs = hw_usb_ctrl_attributes,
};

static int hw_usb_parse_dts(struct hw_usb_device *di)
{
	int ret;
	int volt;
	const char *speed = NULL;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = of_property_read_string(di->dev->of_node, "maximum-speed",
		&speed);
	if (ret) {
		hwlog_err("maximum-speed dts read failed\n");
		return -1;
	}
	strncpy(di->usb_speed, speed, (HW_USB_STR_MAX_LEN - 1));

	hwlog_info("maximum-speed=%s\n", di->usb_speed);

	di->usb_phy_ldo = devm_regulator_get(di->dev, "usb_phy_ldo_33v");
	if (IS_ERR(di->usb_phy_ldo)) {
		hwlog_err("usb_phy_ldo_33v regulator dts read failed\n");
		return -1;
	}

	volt = regulator_get_voltage(di->usb_phy_ldo);
	hwlog_info("usb_phy_ldo_33v=%d\n", volt);

	return 0;
}

static int hw_usb_probe(struct platform_device *pdev)
{
	struct hw_usb_device *di = NULL;
	int ret = -1;

	hwlog_info("probe begin\n");

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_hw_usb_di = di;

	di->pdev = pdev;
	di->dev = &pdev->dev;
	if (!di->pdev || !di->dev || !di->dev->of_node) {
		hwlog_err("device_node is null\n");
		goto fail_free_mem;
	}

	ret = hw_usb_parse_dts(di);
	if (ret)
		hwlog_err("parse dts failed\n");

	hw_usb_class = class_create(THIS_MODULE, "hw_usb");
	if (IS_ERR(hw_usb_class)) {
		hwlog_err("cannot create class\n");
		ret = PTR_ERR(hw_usb_class);
		goto fail_create_class;
	}

	if (hw_usb_class) {
		hw_usb_dev = device_create(hw_usb_class, NULL, 0, NULL, "usb");
		if (IS_ERR(hw_usb_dev)) {
			hwlog_err("sysfs device create failed\n");
			ret = PTR_ERR(hw_usb_dev);
			goto fail_create_device;
		}

		ret = sysfs_create_group(&hw_usb_dev->kobj, &hw_usb_attr_group);
		if (ret) {
			hwlog_err("sysfs group create failed\n");
			goto fail_create_sysfs;
		}
	}

	platform_set_drvdata(pdev, di);

	hwlog_info("probe end\n");
	return 0;

fail_create_sysfs:
	hw_usb_dev = NULL;

fail_create_device:
	hw_usb_class = NULL;

fail_create_class:
	if (!IS_ERR(di->usb_phy_ldo))
		regulator_put(di->usb_phy_ldo);

fail_free_mem:
	devm_kfree(&pdev->dev, di);
	g_hw_usb_di = NULL;

	return ret;
}

static int hw_usb_remove(struct platform_device *pdev)
{
	struct hw_usb_device *di = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	if (!IS_ERR(di->usb_phy_ldo))
		regulator_put(di->usb_phy_ldo);

	sysfs_remove_group(&hw_usb_dev->kobj, &hw_usb_attr_group);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	hw_usb_dev = NULL;
	hw_usb_class = NULL;
	g_hw_usb_di = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id hw_usb_match_table[] = {
	{
		.compatible = "huawei,huawei_usb",
		.data = NULL,
	},
	{},
};

static struct platform_driver hw_usb_driver = {
	.probe = hw_usb_probe,
	.remove = hw_usb_remove,
	.driver = {
		.name = "huawei_usb",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hw_usb_match_table),
	},
};

static int __init hw_usb_init(void)
{
	return platform_driver_register(&hw_usb_driver);
}

static void __exit hw_usb_exit(void)
{
	platform_driver_unregister(&hw_usb_driver);
}

fs_initcall_sync(hw_usb_init);
module_exit(hw_usb_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei usb module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
