/*
 * usb_headset_core.c
 *
 * usb headset core driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/wakelock.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/notifier.h>
#include <linux/usb.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>

#include "huawei_platform/audio/usb_audio_power.h"

#define HWLOG_TAG usb_headset_core
HWLOG_REGIST();

enum {
	USB_HEADSET_PLUG_IN                 = 0,
	USB_HEADSET_PLUG_OUT                = 1,
};

enum {
	USB_SCHARGE_BOOTST_ON               = 0,
	USB_SCHARGE_BOOTST_OFF              = 1,
};

struct usb_headset_core_data {
	/* headset plug state */
	bool hs_state;
	/* ctl scharge 5v flag */
	bool scharge_ctl_flag;
	/* switch for usb headset hw issue */
	bool hs_hw_issus_enable;

	struct notifier_block usb_nb;
};

static struct usb_headset_core_data *uh_pdata;

int usb_headset_plug_in(void)
{
	if (!uh_pdata) {
		hwlog_warn("uh_pdata is NULL\n");
		return -ENOMEM;
	}

	uh_pdata->hs_state = USB_HEADSET_PLUG_IN;
	hwlog_info("usb headset plug in\n");

	return 0;
}

int usb_headset_plug_out(void)
{
	if (!uh_pdata) {
		hwlog_warn("uh_pdata is NULL\n");
		return -ENOMEM;
	}

	if ((uh_pdata->hs_hw_issus_enable == 1) &&
		(uh_pdata->scharge_ctl_flag == USB_SCHARGE_BOOTST_ON)) {
		/* turn off scharge 5v boost for hs hw issue */
		charge_otg_mode_enable(OTG_DISABLE, OTG_CTRL_AUDIO);
		uh_pdata->scharge_ctl_flag = USB_SCHARGE_BOOTST_OFF;
		hwlog_info("headset: turn off scharge 5vboost\n");
	}

	uh_pdata->hs_state = USB_HEADSET_PLUG_OUT;
	hwlog_info("usb headset plug out\n");

	return 0;
}


static int xhci_notifier_fn(struct notifier_block *nb,
				unsigned long action, void *data)
{
	struct usb_device *udev = data;

	if (!uh_pdata) {
		hwlog_warn("uh_pdata or udev is NULL\n");
		return -ENOMEM;
	}
	if (!udev) {
		hwlog_warn("udev is null,just return\n");
		return 0;
	}

	if (uh_pdata->hs_hw_issus_enable == 0)
		return 0;


	if ((action == USB_DEVICE_REMOVE) &&
		(uh_pdata->hs_state == USB_HEADSET_PLUG_IN)) {
		if (uh_pdata->scharge_ctl_flag == USB_SCHARGE_BOOTST_OFF) {
			/* turn on scharge 5v boost for hs hw issue */
			charge_otg_mode_enable(OTG_ENABLE, OTG_CTRL_AUDIO);
			uh_pdata->scharge_ctl_flag = USB_SCHARGE_BOOTST_ON;
			hwlog_info("headset: turn on scharge 5vboost\n");
		}
	}

	return 0;
}

static const struct of_device_id usb_headset_core_of_match[] = {
	{ .compatible = "huawei,usb_headset_core", },
	{},
};
MODULE_DEVICE_TABLE(of, usb_headset_core_of_match);

static int usb_headset_core_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	int temp = 0;

	uh_pdata = devm_kzalloc(dev, sizeof(*uh_pdata), GFP_KERNEL);
	if (!uh_pdata)
		return -ENOMEM;

	uh_pdata->hs_state = USB_HEADSET_PLUG_OUT;
	uh_pdata->scharge_ctl_flag = USB_SCHARGE_BOOTST_OFF;


	if (!of_property_read_u32(node, "hs_hw_issus_enable", &temp))
		uh_pdata->hs_hw_issus_enable = temp;
	else
		uh_pdata->hs_hw_issus_enable = 0;

	uh_pdata->usb_nb.notifier_call = xhci_notifier_fn;
	usb_register_notify(&uh_pdata->usb_nb);

	hwlog_info("usb_headset_core probe success\n");

	return 0;
}

static int usb_headset_core_remove(struct platform_device *pdev)
{
	if (!uh_pdata)
		return -ENOMEM;

	usb_unregister_notify(&uh_pdata->usb_nb);

	devm_kfree(&pdev->dev, uh_pdata);
	uh_pdata = NULL;
	return 0;
}

static struct platform_driver usb_headset_core_driver = {
	.driver = {
		.name   = "usb_headset_core",
		.owner  = THIS_MODULE,
		.of_match_table = usb_headset_core_of_match,
	},
	.probe  = usb_headset_core_probe,
	.remove = usb_headset_core_remove,
};

static int __init usb_headset_core_init(void)
{
	return platform_driver_register(&usb_headset_core_driver);
}

static void __exit usb_headset_core_exit(void)
{
	platform_driver_unregister(&usb_headset_core_driver);
}

fs_initcall(usb_headset_core_init);
module_exit(usb_headset_core_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("usb headset core driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
