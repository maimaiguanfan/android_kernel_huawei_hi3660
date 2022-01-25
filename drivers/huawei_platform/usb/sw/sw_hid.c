/*
 * sw_hid.c
 *
 * single-wire hid driver
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

/*
 * usb-specific hid struct, to be pointed to
 * from struct hid_device->driver_data
 */
struct swhid_device {
	struct hid_device *hid; /* pointer to corresponding hid dev */
	struct sw_device *dev;
};

void sw_hid_input_report(struct sw_device *dev, u8 *data, int size)
{
	struct swhid_device *swhid;
	/* 1 means interrupt, 0 means control transfer */
	int hid_interrupt = 1;

	if ((dev == NULL) || (dev->context == NULL) || (data == NULL))
		return;

	SW_PRINT_FUNCTION_NAME;

	swhid = (struct swhid_device *)dev->context;

	hid_input_report(swhid->hid, HID_INPUT_REPORT,
					data, size, hid_interrupt);
}

static int swhid_get_raw_report(struct hid_device *hid,
	unsigned char report_number, unsigned char *data,
	size_t count, unsigned char report_type)
{
	struct swhid_device *swhid = hid->driver_data;

	if (swhid == NULL)
		return -ENODEV;

	SW_PRINT_INFO("swhid_get_raw_report report num and type [ %x,%x ]\n",
		report_number, report_type);
	sw_debug_dump_data(data, count);

	return 0;
}

static int swhid_set_raw_report(struct hid_device *hid,
	unsigned char report_number, unsigned char *data,
	size_t count, unsigned char report_type)
{
	struct swhid_device *swhid = hid->driver_data;

	if (swhid == NULL)
		return -ENODEV;

	SW_PRINT_INFO("swhid_set_raw_report report num and type [ %x,%x ]\n",
		report_number, report_type);
	sw_debug_dump_data(data, count);

	return 0;
}

static int swhid_raw_request(struct hid_device *hid,
	unsigned char reportnum, unsigned char *buf, size_t len,
	unsigned char rtype, int reqtype)
{
	SW_PRINT_FUNCTION_NAME;
	switch (reqtype) {
	case HID_REQ_GET_REPORT:
		return swhid_get_raw_report(hid, reportnum, buf, len, rtype);
	case HID_REQ_SET_REPORT:
		return swhid_set_raw_report(hid, reportnum, buf, len, rtype);
	default:
		return -EIO;
	}
}

static int swhid_output_report(struct hid_device *hid,
	unsigned char *data, size_t count)
{
	struct swhid_device *swhid = hid->driver_data;

	SW_PRINT_FUNCTION_NAME;

	if (swhid == NULL)
		return -ENODEV;

	SW_PRINT_INFO("swhid_output_report\n");
	sw_debug_dump_data(data, count);

	kernel_send_kb_report_event(KBHB_IOCTL_CMD, data, count);
	return 0;
}

static int swhid_open(struct hid_device *hid)
{
	SW_PRINT_FUNCTION_NAME;
	return 0;
}

static void swhid_close(struct hid_device *hid)
{
	SW_PRINT_FUNCTION_NAME;
}

static int swhid_parse(struct hid_device *hid)
{
	struct swhid_device *session = hid->driver_data;
	struct sw_device *dev;

	SW_PRINT_FUNCTION_NAME;

	if (session == NULL)
		return -ENODEV;

	dev = session->dev;
	if (dev == NULL)
		return -ENODEV;

	return hid_parse_report(session->hid, dev->rd_data,
		dev->rd_size);
}

static int swhid_start(struct hid_device *hid)
{
	SW_PRINT_FUNCTION_NAME;
	return 0;
}

static void swhid_stop(struct hid_device *hid)
{
	struct swhid_device *session;

	SW_PRINT_FUNCTION_NAME;

	if (hid == NULL)
		return;

	session = hid->driver_data;
	if ((session != NULL) && (session->hid != NULL))
		session->hid->claimed = 0;
}

static struct hid_ll_driver sw_hid_driver = {
	.parse = swhid_parse,
	.start = swhid_start,
	.stop = swhid_stop,
	.open = swhid_open,
	.close = swhid_close,

	.raw_request = swhid_raw_request,
	.output_report = swhid_output_report,
};

static int swhid_probe(struct sw_device *dev)
{
	struct hid_device *hid;
	struct swhid_device *swhid;
	int ret;

	SW_PRINT_FUNCTION_NAME;
	hid = hid_allocate_device();
	if (IS_ERR(hid))
		return PTR_ERR(hid);

	hid->ll_driver = &sw_hid_driver;
	hid->dev.parent = &dev->dev;
	hid->bus = BUS_VIRTUAL;
	hid->vendor = le16_to_cpu(dev->vendor);
	hid->product = le16_to_cpu(dev->product);
	hid->name[0] = 0;
	snprintf(hid->phys, sizeof(hid->phys), "SW%xMR", 0x1);
	snprintf(hid->uniq, sizeof(hid->uniq), "SW%xMR", 0x2);

	swhid = kzalloc(sizeof(*swhid), GFP_KERNEL);
	if (swhid == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	hid->driver_data = swhid;
	swhid->hid = hid;
	swhid->dev = dev;

	dev_set_drvdata(&dev->dev, hid);
	ret = hid_add_device(hid);
	if (ret) {
		SW_PRINT_ERR("hid_add_device error\n");
		goto err_free;
	}

	dev->context = (void *)swhid;
	return 0;

err_free:
	kfree(swhid);
err:
	hid_destroy_device(hid);
	return ret;
}

static void swhid_disconnect(struct sw_device *dev)
{
	struct hid_device *hid = dev_get_drvdata(&dev->dev);
	struct swhid_device *swhid;

	if (WARN_ON(hid == NULL))
		return;

	swhid = hid->driver_data;
	hid_destroy_device(hid);

	kfree(swhid);
}

static struct sw_device_id sw_hid_ids[] = {
	{
		.type = SW_HID_TYPE,
		.bus = SW_BUS_ANY,
		.group = SW_GROUP_ANY,
		.vendor = SW_ANY_ID,
		.product = SW_ANY_ID,
	},
	{ 0 }
};

static struct sw_driver hid_driver = {
	.name = "swhid",
	.driver = {
		.name = "swhid",
	},

	.id_table = sw_hid_ids,
	.probe = swhid_probe,
	.disconnect = swhid_disconnect,
};

int swhid_init(void)
{
	SW_PRINT_FUNCTION_NAME;

	return sw_register(&hid_driver);
}

void swhid_exit(void)
{
	sw_deregister(&hid_driver);
}
