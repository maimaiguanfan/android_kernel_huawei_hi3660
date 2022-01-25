/*
 * keyboard_scm.c
 *
 * scm keyboard hid driver
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

#include <linux/kref.h>
#include <linux/module.h>
#include <linux/file.h>
#include <linux/kthread.h>
#include <linux/hid.h>
#include <linux/hiddev.h>
#include <linux/hid-debug.h>
#include <linux/hidraw.h>

#include "sw_core.h"
#include "sw_debug.h"

/* Standard Keycode */
static u8 g_keyboard_hid_report_desc[] = {
	0x05, 0x01,       /* Usage Page (Generic Desktop) */
	0x09, 0x06,       /* Usage (Keyboard) */
	0xA1, 0x01,       /* Collection (Application) */
	0x05, 0x07,       /* Usage Page (Keyboard/Keypad) */
	0x19, 0xE0,       /* Usage Minimum (Keyboard Left Control) */
	0x29, 0xE7,       /* Usage Maximum (Keyboard Right GUI) */
	0x15, 0x00,       /* Logical Minimum (0) */
	0x25, 0x01,       /* Logical Maximum (1) */
	0x75, 0x01,       /* Report Size (1) */
	0x95, 0x08,       /* Report Count (8) */
	0x81, 0x02,       /* Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit) */
	0x95, 0x01,       /* Report Count (1) */
	0x75, 0x08,       /* Report Size (8) */
	0x81, 0x01,       /* Input (Cnst,Ary,Abs) */
	0x95, 0x05,       /* Report Count (5) */
	0x75, 0x01,       /* Report Size (1) */
	0x05, 0x08,       /* Usage Page (LEDs) */
	0x19, 0x01,       /* Usage Minimum (Num Lock) */
	0x29, 0x05,       /* Usage Maximum (Kana) */
	0x91, 0x02,       /* Output (Data,Var,Abs,NWrp,Lin,Pref,NNul,NVol) */
	0x95, 0x01,       /* Report Count (1) */
	0x75, 0x03,       /* Report Size (3) */
	0x91, 0x01,       /* Output (Cnst,Ary,Abs,NWrp,Lin,Pref,NNul,NVol) */
	0x95, 0x06,       /* Report Count (6) */
	0x75, 0x08,       /* Report Size (8) */
	0x15, 0x00,       /* Logical Minimum (0) */
	0x26, 0xFF, 0x00, /* Logical Maximum (255) */
	0x05, 0x07,       /* Usage Page (Keyboard/Keypad) */
	0x19, 0x00,       /* Usage Minimum (Undefined) */
	0x2A, 0xFF, 0x00, /* Usage Maximum (Keyboard ExSel) */
	0x81, 0x00,       /* Input (Data,Ary,Abs) */
	0x09, 0x05,       /* Usage (Vendor Defined) */
	0x15, 0x00,       /* Logical Minimum (0) */
	0x26, 0xFF, 0x00, /* Logical Maximum (255) */
	0x75, 0x08,       /* Report Size (8 bit) */
	0x95, 0x02,       /* Report Count (2) */
	0xB1, 0x02,       /* Feature (Data, Variable, Absolute) */
	0xC0              /* End Collection */
};

/* Advanced buttons */
static u8 g_custom_key_hid_report_desc[] = {
	0x05, 0x0C,       /* Usage Page */
	0x09, 0x01,       /* Usage */
	0xA1, 0x01,       /* Collection */
	0x19, 0x00,       /* Usage Minimum */
	0x2A, 0x9C, 0x02, /* Usage Maximum */
	0x15, 0x00,       /* Logical Minimum */
	0x26, 0x9C, 0x02, /* Logical Maximum */
	0x75, 0x10,       /* Report Size */
	0x95, 0x01,       /* Report Count */
	0x81, 0x00,       /* Input */
	0xC0,             /* End Collection */
};

static struct sw_device_id scmkb_ids[] = {
	{
		.type = SW_DEV_TYPE,
		.bus = SW_BUS_ANY,
		.group = SW_GROUP_ANY,
		.vendor = KIHITECH_KEYBOARD_VID,
		.product = SCM_KEYBOARD_PID,
	},
	{ 0 }
};

struct scmkb_device {
	struct sw_device *commonkeys_dev;
	struct sw_device *customkeys_dev;
};

static int scmkb_relese_hid_devices(struct scmkb_device *scmdev)
{
	struct sw_device *device;
	int err = 0;

	if (scmdev == NULL)
		return 0;

	if (scmdev->commonkeys_dev != NULL) {
		device = scmdev->commonkeys_dev;
		scmdev->commonkeys_dev = NULL;
		err += sw_release_device(device);
	}

	if (scmdev->customkeys_dev != NULL) {
		device = scmdev->customkeys_dev;
		scmdev->customkeys_dev = NULL;
		err += sw_release_device(device);
	}

	return err;
}

static int scmkb_create_hid_devices(struct scmkb_device *scmdev,
	struct sw_device *rootdev)
{
	int ret;

	scmdev->commonkeys_dev = sw_register_new_hiddevice(KB_COMMON_KEY_DEV,
		rootdev, g_keyboard_hid_report_desc,
		sizeof(g_keyboard_hid_report_desc) / sizeof(u8));
	if (scmdev->commonkeys_dev == NULL) {
		ret = -1;
		goto create_hid_failed;
	}

	scmdev->customkeys_dev = sw_register_new_hiddevice(KB_CUSTOM_KEY_DEV,
		rootdev, g_custom_key_hid_report_desc,
		sizeof(g_custom_key_hid_report_desc) / sizeof(u8));
	if (scmdev->customkeys_dev == NULL) {
		ret = -1;
		goto create_hid_failed;
	}

	return 0;

create_hid_failed:
	scmkb_relese_hid_devices(scmdev);

	return ret;
}

static int scmkb_probe(struct sw_device *dev)
{
	struct scmkb_device *scmdev;
	int ret;

	SW_PRINT_FUNCTION_NAME;

	scmdev = kzalloc(sizeof(*scmdev), GFP_KERNEL);
	if (scmdev == NULL) {
		ret = -ENOMEM;
		goto err_ret;
	}

	if (scmkb_create_hid_devices(scmdev, dev)) {
		ret = -1;
		goto err_free;
	}

	dev_set_drvdata(&dev->dev, scmdev);
	return 0;

err_free:
	kfree(scmdev);
err_ret:
	return ret;
}

static void scmkb_disconnect(struct sw_device *dev)
{
	struct scmkb_device *scmdev = dev_get_drvdata(&dev->dev);

	if (WARN_ON(scmdev == NULL))
		return;

	scmkb_relese_hid_devices(scmdev);

	kfree(scmdev);
}

static void scmkb_recvframe(struct sw_device *dev, struct sk_buff *skb)
{
	unsigned char hdr;
	struct scmkb_device *scmdev = dev_get_drvdata(&dev->dev);

	SW_PRINT_FUNCTION_NAME;

	if (WARN_ON(scmdev == NULL))
		return;

	if (skb == NULL)
		return;

	hdr = skb->data[0];

	/* key */
	if (hdr == PROTO_CMD_KEY_NORAML)
		sw_hid_input_report(scmdev->commonkeys_dev,
			skb->data + KB_KEY_DATA_HEAD_LEN,
			skb->len - KB_KEY_DATA_HEAD_LEN);

	/* consumer key */
	if ((hdr == PROTO_CMD_KEY_CONSUMER) ||
		(hdr == PROTO_CMD_KEY_CONSUMER_1))
		sw_hid_input_report(scmdev->customkeys_dev,
			skb->data + KB_KEY_DATA_HEAD_LEN,
			skb->len - KB_KEY_DATA_HEAD_LEN);
}

static struct sw_driver scmkb_driver = {
	.name = "scmkb",
	.driver = {
		.name = "scmkb",
	},

	.id_table = scmkb_ids,
	.probe = scmkb_probe,
	.disconnect = scmkb_disconnect,
	.recvframe = scmkb_recvframe,
};

int scmkb_init(void)
{
	int retval;

	SW_PRINT_FUNCTION_NAME;

	retval = sw_register(&scmkb_driver);
	if (retval)
		SW_PRINT_ERR("sw_register fail\n");

	return retval;
}

void scmkb_exit(void)
{
	sw_deregister(&scmkb_driver);
}
