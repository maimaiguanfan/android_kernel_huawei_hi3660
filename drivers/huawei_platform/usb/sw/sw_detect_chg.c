/*
 * sw_detect_kb.c
 *
 * Single wire UART Keyboard driver
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
#include <linux/hidraw.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/tty.h>
#include <linux/crc16.h>
#include <linux/crc-ccitt.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/hisi/hisi_adc.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include "sw_detect.h"
#include "sw_debug.h"

#define CHG_MAGIC_CODE   (0x1)

struct sw_chg_detectparam {
	int chg_connect_adc_min;
	int chg_connect_adc_max;
	int chg_online_adc_min;
	int chg_online_adc_max;
};

static void sw_chg_disconnected(struct sw_gpio_detector *detector,
	struct sw_chg_detectparam *param)
{
	SW_PRINT_FUNCTION_NAME;
}

static void sw_chg_connected(struct sw_gpio_detector *detector,
	struct sw_chg_detectparam *param)
{
	SW_PRINT_FUNCTION_NAME;
}

static bool sw_is_chg_online(int detect_adc, struct sw_chg_detectparam *param)
{
	return false;
}

static int sw_chg_devdetect(struct sw_gpio_detector *detector,
	struct sw_dev_detector *devdetector)
{
	bool chg_isonline;
	struct sw_chg_detectparam *param;

	param = (struct sw_chg_detectparam *)devdetector->param;
	if (param == NULL) {
		SW_PRINT_INFO("param is null\n");
		return -EINVAL;
	}

	chg_isonline = sw_is_chg_online(detector->detect_adc_no, param);
	if (chg_isonline)
		sw_chg_connected(detector, param);
	else
		sw_chg_disconnected(detector, param);

	return 0;
}

static int sw_chg_notifyevent(struct sw_gpio_detector *detector,
	struct sw_dev_detector *devdetector,
	unsigned long event, void *pData)
{
	struct sw_chg_detectparam *chg_param;

	if ((detector == NULL) || (devdetector == NULL))
		return -EINVAL;

	chg_param = (struct sw_chg_detectparam *)devdetector->param;
	if (event == SW_NOTIFY_EVENT_DESTORY) {
		kfree(chg_param);
		kfree(devdetector);
		return 0;
	}

	return 0;
}

static int sw_parse_chgdetectparam(struct device_node *np,
	struct sw_chg_detectparam *chg_param)
{
	return -1;
}

struct sw_dev_detector *sw_load_chg_detect(struct device_node *np, u32 val)
{
	struct sw_dev_detector *dev_detector;
	struct sw_chg_detectparam *chg_param;
	int ret;

	SW_PRINT_FUNCTION_NAME;
	if (val != CHG_MAGIC_CODE) {
		SW_PRINT_ERR("magic code error\n");
		return NULL;
	}

	if (np == NULL) {
		SW_PRINT_ERR("param failed\n");
		return NULL;
	}

	dev_detector = kzalloc(sizeof(*dev_detector), GFP_KERNEL);
	if (dev_detector == NULL)
		return NULL;

	chg_param = kzalloc(sizeof(*chg_param), GFP_KERNEL);
	if (chg_param == NULL)
		goto err_chg_param;

	ret = sw_parse_chgdetectparam(np, chg_param);
	if (ret < 0) {
		SW_PRINT_ERR("dts parse failed\n");
		goto err_core_init;
	}

	dev_detector->detect_call = sw_chg_devdetect;
	dev_detector->event_call = sw_chg_notifyevent;
	dev_detector->param = chg_param;

	return dev_detector;

err_core_init:
	kfree(chg_param);
err_chg_param:
	kfree(dev_detector);
	return NULL;
}
