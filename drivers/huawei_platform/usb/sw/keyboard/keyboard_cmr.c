/*
 * keyboard_cmr.c
 *
 * cmr keyboard hid driver
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
#include <huawei_platform/inputhub/kbhub.h>
#include "sw_core.h"
#include "sw_debug.h"

#define HALL_COVERD             (1)

#define MOUSE_V1_MAINVER        (0)
#define MOUSE_V1_SUBVER_LIMITE  (3)
#define MOUSE_V2_MAINVER        (0)
#define MOUSE_V2_SUBVER_LIMITE  (5)

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

/*
 * Report ID : Mouse buttons + X + Y + Z
 * Used by keyboard firmware version < 0.3
 */
static u8 g_mouse_hid_report_desc[] = {
	0x05, 0x01,       /* USAGE_PAGE (Generic Desktop) */
	0x09, 0x02,       /* USAGE (Mouse) */
	0xa1, 0x01,       /* COLLECTION (Application) */
	0x09, 0x01,       /* USAGE (Pointer) */

	0xa1, 0x00,       /* COLLECTION (Physical) */
	0x05, 0x09,       /* USAGE_PAGE (Button) */
	0x19, 0x01,       /* USAGE_MINIMUM (Button 1) */
	0x29, 0x03,       /* USAGE_MAXIMUM (Button 3) */

	0x15, 0x00,       /* LOGICAL_MINIMUM (0) */
	0x25, 0x01,       /* LOGICAL_MAXIMUM (1) */
	0x75, 0x01,       /* REPORT_SIZE (1) */
	0x95, 0x03,       /* REPORT_COUNT (3) */
	0x81, 0x02,       /* INPUT (Data, Var, Abs) */

	0x95, 0x01,       /* REPORT_COUNT (1) */
	0x75, 0x05,       /* REPORT_SIZE (5) */
	0x81, 0x01,       /* INPUT (Cnst, Var, Abs) */

	0x05, 0x01,       /* USAGE_PAGE (Generic Desktop) */
	0x09, 0x30,       /* USAGE (X) */
	0x09, 0x31,       /* USAGE (Y) */
	0x16, 0x01, 0x80, /* LOGICAL_MINIMUM (-32767) */
	0x26, 0xff, 0x7f, /* LOGICAL_MAXIMUM (32767) */
	0x75, 0x10,       /* REPORT_SIZE (16) */
	0x95, 0x02,       /* REPORT_COUNT (2) */
	0x81, 0x06,       /* INPUT (Data, Var, Abs) */
	0xc0,             /* END_COLLECTION */
	0xc0              /* END_COLLECTION */
};

/*
 * Report ID New: Mouse buttons + X + Y + Z + Wheel
 * Used by keyboard firmware version >=0.3  <=0.4
 * only support Up-down Wheel Feature
 */
static u8 g_mouse_hid_report_desc_v2[] = {
	0x05, 0x01,       /* USAGE_PAGE (Generic Desktop) */
	0x09, 0x02,       /* USAGE (Mouse) */
	0xa1, 0x01,       /* COLLECTION (Application) */
	0x09, 0x01,       /* USAGE (Pointer) */

	0xa1, 0x00,       /* COLLECTION (Physical) */
	0x05, 0x09,       /* USAGE_PAGE (Button) */
	0x19, 0x01,       /* USAGE_MINIMUM (Button 1) */
	0x29, 0x03,       /* USAGE_MAXIMUM (Button 3) */

	0x15, 0x00,       /* LOGICAL_MINIMUM (0) */
	0x25, 0x01,       /* LOGICAL_MAXIMUM (1) */
	0x75, 0x01,       /* REPORT_SIZE (1) */
	0x95, 0x03,       /* REPORT_COUNT (3) */
	0x81, 0x02,       /* INPUT (Data, Var, Abs) */

	0x95, 0x01,       /* REPORT_COUNT (1) */
	0x75, 0x05,       /* REPORT_SIZE (5) */
	0x81, 0x01,       /* INPUT (Cnst, Var, Abs) */

	0x05, 0x01,       /* USAGE_PAGE (Generic Desktop) */
	0x09, 0x30,       /* USAGE (X) */
	0x09, 0x31,       /* USAGE (Y) */
	0x16, 0x01, 0x80, /* LOGICAL_MINIMUM (-32767) */
	0x26, 0xff, 0x7f, /* LOGICAL_MAXIMUM (32767) */
	0x75, 0x10,       /* REPORT_SIZE (16) */
	0x95, 0x02,       /* Report Count (2) */
	0x81, 0x06,       /* INPUT (Data, Var, Abs) */

	0x09, 0x38,       /* Usage (Wheel) */
	0x15, 0x81,       /* LOGICAL_MINIMUM (-127) */
	0x25, 0x7f,       /* LOGICAL_MAXIMUM (127) */
	0x75, 0x08,       /* REPORT_SIZE (8) */
	0x95, 0x01,       /* REPORT_COUNT (1) */
	0x81, 0x06,       /* INPUT (Data, Var, Abs) */

	0xc0,             /* END_COLLECTION */
	0xc0              /* END_COLLECTION */
};

/*
 * Report ID New: Mouse buttons + X + Y + Z + Wheel
 * Used by keyboard firmware version >=0.5
 * Support Up-Down  Left-Right Wheel Feature
 */
static u8 g_mouse_hid_report_desc_v3[] = {
	0x05, 0x01,       /* USAGE_PAGE (Generic Desktop) */
	0x09, 0x02,       /* USAGE (Mouse) */
	0xa1, 0x01,       /* COLLECTION (Application) */
	0x09, 0x01,       /* USAGE (Pointer) */

	0xa1, 0x00,       /* COLLECTION (Physical) */
	0x05, 0x09,       /* USAGE_PAGE (Button) */
	0x19, 0x01,       /* USAGE_MINIMUM (Button 1) */
	0x29, 0x03,       /* USAGE_MAXIMUM (Button 3) */

	0x15, 0x00,       /* LOGICAL_MINIMUM (0) */
	0x25, 0x01,       /* LOGICAL_MAXIMUM (1) */
	0x75, 0x01,       /* REPORT_SIZE (1) */
	0x95, 0x03,       /* REPORT_COUNT (3) */
	0x81, 0x02,       /* INPUT (Data, Var, Abs) */

	0x95, 0x01,       /* REPORT_COUNT (1) */
	0x75, 0x05,       /* REPORT_SIZE (5) */
	0x81, 0x01,       /* INPUT (Cnst, Var, Abs) */

	0x05, 0x01,       /* USAGE_PAGE (Generic Desktop) */
	0x09, 0x30,       /* USAGE (X) */
	0x09, 0x31,       /* USAGE (Y) */
	0x16, 0x01, 0x80, /* LOGICAL_MINIMUM (-32767) */
	0x26, 0xff, 0x7f, /* LOGICAL_MAXIMUM (32767) */
	0x75, 0x10,       /* REPORT_SIZE (16) */
	0x95, 0x02,       /* Report Count (2) */
	0x81, 0x06,       /* INPUT (Data, Var, Abs) */

	0x09, 0x38,       /* Usage (Wheel) */
	0x15, 0x81,       /* LOGICAL_MINIMUM (-127) */
	0x25, 0x7f,       /* LOGICAL_MAXIMUM (127) */
	0x75, 0x08,       /* REPORT_SIZE (8) */
	0x95, 0x01,       /* REPORT_COUNT (1) */
	0x81, 0x06,       /* INPUT (Data, Var, Abs) */

	0x05, 0x0C,       /* Usage Page (Consumer Devices) */
	0x0A, 0x38, 0x02, /* Usage (AC Pan) */
	0x15, 0x80,       /* Logical Minimum (-128) */
	0x25, 0x7F,       /* Logical Maximum (127) */
	0x75, 0x08,       /* Report Size (8) */
	0x95, 0x01,       /* Report Count (1) */
	0x81, 0x06,       /* Input (Data,Var,Rel,NWrp,Lin,Pref,NNul,Bit) */

	0xc0,             /* END_COLLECTION */
	0xc0              /* END_COLLECTION */
};

static struct sw_device_id cmrkb_ids[] = {
	{
		.type = SW_DEV_TYPE,
		.bus = SW_BUS_ANY,
		.group = SW_GROUP_ANY,
		.vendor = KIHITECH_KEYBOARD_VID,
		.product = CMR_KEYBOARD_PID,
	},
	{ 0 }
};

struct cmrkb_device {
	struct sw_device *commonkeys_dev;
	struct sw_device *customkeys_dev;
	struct sw_device *mousekeys_dev;
};

static int cmrkb_relese_hid_devices(struct cmrkb_device *cmrdev)
{
	struct sw_device *device;
	int err = 0;

	if (cmrdev == NULL)
		return 0;

	if (cmrdev->commonkeys_dev != NULL) {
		device = cmrdev->commonkeys_dev;
		cmrdev->commonkeys_dev = NULL;
		err += sw_release_device(device);
	}

	if (cmrdev->mousekeys_dev != NULL) {
		device = cmrdev->mousekeys_dev;
		cmrdev->mousekeys_dev = NULL;
		err += sw_release_device(device);
	}

	if (cmrdev->customkeys_dev != NULL) {
		device = cmrdev->customkeys_dev;
		cmrdev->customkeys_dev = NULL;
		err += sw_release_device(device);
	}

	return err;
}

static int cmrkb_create_hid_devices(struct cmrkb_device *cmrdev,
	struct sw_device *rootdev)
{
	int ret;
	/* Report descriptor */
	u8 *rd_data;
	u32 rd_size;

	if (cmrdev->commonkeys_dev == NULL) {
		cmrdev->commonkeys_dev = sw_register_new_hiddevice(KB_COMMON_KEY_DEV,
			rootdev, g_keyboard_hid_report_desc,
			sizeof(g_keyboard_hid_report_desc) / sizeof(u8));
		if (cmrdev->commonkeys_dev == NULL) {
			ret = -1;
			goto create_hid_failed;
		}
	}

	/*
	 * keyboard firmware version < 0.3 , use old mouse desc
	 * >= 0.3  <=0.4, use v2 mouse desc support wheel button
	 * >=0.5, use v3 mouse desc support wheel button
	 */
	if ((rootdev->mainver == MOUSE_V1_MAINVER) &&
		(rootdev->subver < MOUSE_V1_SUBVER_LIMITE)) {
		rd_data = g_mouse_hid_report_desc;
		rd_size = sizeof(g_mouse_hid_report_desc) / sizeof(u8);
	} else if ((rootdev->mainver == MOUSE_V2_MAINVER) &&
		(rootdev->subver < MOUSE_V2_SUBVER_LIMITE)) {
		rd_data = g_mouse_hid_report_desc_v2;
		rd_size = sizeof(g_mouse_hid_report_desc_v2) / sizeof(u8);
	} else {
		rd_data = g_mouse_hid_report_desc_v3;
		rd_size = sizeof(g_mouse_hid_report_desc_v3) / sizeof(u8);
	}

	if (cmrdev->customkeys_dev == NULL) {
		cmrdev->customkeys_dev = sw_register_new_hiddevice(KB_CUSTOM_KEY_DEV,
			rootdev, g_custom_key_hid_report_desc,
			sizeof(g_custom_key_hid_report_desc) / sizeof(u8));
		if (cmrdev->customkeys_dev == NULL) {
			ret = -1;
			goto create_hid_failed;
		}
	}

	if (cmrdev->mousekeys_dev == NULL) {
		cmrdev->mousekeys_dev = sw_register_new_hiddevice(KB_MOUSE_DEV,
			rootdev, rd_data, rd_size);
		if (cmrdev->mousekeys_dev == NULL) {
			ret = -1;
			goto create_hid_failed;
		}
	}

	return 0;

create_hid_failed:
	cmrkb_relese_hid_devices(cmrdev);

	return ret;
}

static int cmrkb_probe(struct sw_device *dev)
{
	struct cmrkb_device *cmrdev;
	int ret;

	SW_PRINT_FUNCTION_NAME;

	cmrdev = kzalloc(sizeof(*cmrdev), GFP_KERNEL);
	if (cmrdev == NULL) {
		ret = -ENOMEM;
		goto err_ret;
	}

	if (cmrkb_create_hid_devices(cmrdev, dev)) {
		ret = -ENODEV;
		goto err_free;
	}

	dev_set_drvdata(&dev->dev, cmrdev);
	return 0;

err_free:
	kfree(cmrdev);
err_ret:
	return ret;
}

static void cmrkb_disconnect(struct sw_device *dev)
{
	struct cmrkb_device *cmrdev = dev_get_drvdata(&dev->dev);

	if (WARN_ON(cmrdev == NULL))
		return;

	cmrkb_relese_hid_devices(cmrdev);

	kfree(cmrdev);
}

static void cmrkb_process_work_mode(struct sw_device *dev,
	struct sk_buff *skb)
{
	struct cmrkb_device *cmrdev = dev_get_drvdata(&dev->dev);

	SW_PRINT_DBG("sw_core_event change keyboard mode\n");
	if (skb->len < KBMODE_DATA_MIN_LEN)
		return;

	switch (skb->data[KB_MODE_BYTE]) {
	case KBMODE_PCMODE:
		cmrkb_create_hid_devices(cmrdev, dev);
		break;
	case KBMODE_PADMODE:
		cmrkb_relese_hid_devices(cmrdev);
		break;
	default:
		break;
	}
}

static void cmrkb_recvframe(struct sw_device *dev, struct sk_buff *skb)
{
	unsigned char hdr;
	struct cmrkb_device *cmrdev = dev_get_drvdata(&dev->dev);

	SW_PRINT_FUNCTION_NAME;

	if (WARN_ON(cmrdev == NULL))
		return;

	if (skb == NULL)
		return;

	hdr = skb->data[0];

	/* key */
	if (hdr == PROTO_CMD_KEY_NORAML) {
		sw_hid_input_report(cmrdev->commonkeys_dev,
			skb->data + KB_KEY_DATA_HEAD_LEN,
			skb->len - KB_KEY_DATA_HEAD_LEN);
		return;
	}

	/* mouse */
	if (hdr == PROTO_CMD_MOUSE) {
		sw_hid_input_report(cmrdev->mousekeys_dev,
			skb->data + KB_KEY_DATA_HEAD_LEN,
			skb->len - KB_KEY_DATA_HEAD_LEN);
		return;
	}

	/* consumer key */
	if ((hdr == PROTO_CMD_KEY_CONSUMER) ||
		(hdr == PROTO_CMD_KEY_CONSUMER_1)) {
		sw_hid_input_report(cmrdev->customkeys_dev,
			skb->data + KB_KEY_DATA_HEAD_LEN,
			skb->len - KB_KEY_DATA_HEAD_LEN);
		return;
	}

	/* work-mode */
	if (hdr == PROTO_CMD_WORKMODE) {
		cmrkb_process_work_mode(dev, skb);
		return;
	}
}

/*
 * if keyboard coverd, we mean some messages from keyborad may
 * not be processed. Such as key msg\touch msg\work-mode msg etc
 * but except connect or disconnect msg.
 * 0 -- need process
 * 1 -- ignore process
 */
static int cmrkb_pre_recvframe(struct sw_device *dev, char *data, int count)
{
	unsigned char hdr;
	int hall_val = kbhb_get_hall_value();
	struct cmrkb_device *cmrdev = dev_get_drvdata(&dev->dev);

	SW_PRINT_FUNCTION_NAME;

	if (WARN_ON(cmrdev == NULL)) {
		SW_PRINT_ERR("cmrdev null\n");
		return 1;
	}

	if (unlikely(data == NULL))
		return 1;

	if (count < 1) {
		SW_PRINT_ERR("received count from kb err\n");
		return 1;
	}

	/*
	 * if keyboard coverd , don't respond some msg ,
	 * except Disconnect and handshake msg
	 */
	if (hall_val & HALL_COVERD) {
		hdr = (unsigned char)data[0];
		if ((hdr != PROTO_CMD_DISCONNECT) &&
			(hdr != PROTO_CMD_HANDSHAKE))
			return 1;
	}

	return 0;
}

static struct sw_driver cmrkb_driver = {
	.name = "cmrkb",
	.driver = {
		.name = "cmrkb",
	},

	.id_table = cmrkb_ids,
	.probe = cmrkb_probe,
	.disconnect = cmrkb_disconnect,
	.pre_recvframe = cmrkb_pre_recvframe,
	.recvframe = cmrkb_recvframe,
};

int cmrkb_init(void)
{
	int retval;

	SW_PRINT_FUNCTION_NAME;

	retval = sw_register(&cmrkb_driver);
	if (retval)
		SW_PRINT_ERR("sw_register fail\n");

	return retval;
}

void cmrkb_exit(void)
{
	sw_deregister(&cmrkb_driver);
}
