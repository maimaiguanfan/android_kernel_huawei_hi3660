/*
 * sw_detect_kb.c
 *
 * Single wire UART Keyboard detect driver
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

#define KB_ONLINE_CONN_MIN_ADC_LIMIT    (200)
#define KB_ONLINE_CONN_MAX_ADC_LIMIT    (350)

#define KB_ONLINE_MIN_ADC_LIMIT         (1450)
#define KB_ONLINE_MAX_ADC_LIMIT         (1550)

#define KB_DETECT_DELAY_TIME_MS         (300)
#define RECHECK_ADC_DELAY_MS            (1)

struct sw_kb_detectparam {
	int kb_tx_gpio;
	int kb_vdd_ctrl;

	int kb_connect_adc_min;
	int kb_connect_adc_max;
	int kb_online_adc_min;
	int kb_online_adc_max;
};

static void sw_keyboard_disconnected(struct sw_gpio_detector *detector,
	struct sw_kb_detectparam *param)
{
	SW_PRINT_FUNCTION_NAME;
	if (detector->dev_state == DEVSTAT_KBDEV_ONLINE) {
		SW_PRINT_INFO("sw_keyboard_disconnected, enable irq\n");
		detector->dev_state = DEVSTAT_NONEDEV;
		gpio_set_value(param->kb_vdd_ctrl, 0);
	}
	if (detector->control_irq != NULL)
		detector->control_irq(detector, true);

	detector->start_detect = 1;
}

static void sw_keyboard_connected(struct sw_gpio_detector *detector,
	struct sw_kb_detectparam *param)
{
	SW_PRINT_FUNCTION_NAME;
	if (detector->dev_state == DEVSTAT_NONEDEV) {
		gpio_set_value(param->kb_vdd_ctrl, 1);
		detector->dev_state = DEVSTAT_KBDEV_ONLINE;
		SW_PRINT_INFO("sw_keyboard_connected, notify sensorhub\n");
	}

	detector->start_detect = 0;
}

static bool sw_is_kb_online(int detect_adc, struct sw_kb_detectparam *param)
{
	int adc_val;
	int val;
	int count = 5; /* check adc val 5 times */
	int i;
	int check_ok = 0;
	int ret_check = 3; /* adc check fail retry times */

	msleep(KB_DETECT_DELAY_TIME_MS);

retry_check:

	for (i = 0; i < count; i++) {
		val = gpio_get_value(param->kb_vdd_ctrl);
		adc_val = hkadc_detect_value(detect_adc);

		/*
		 * VDD is disabled , kb online adc in
		 * [KB_ONLINE_MIN_ADC_LIMIT KB_ONLINE_CONN_MAX_ADC_LIMIT]
		 */
		if (val == 0) {
			if ((adc_val > param->kb_connect_adc_min) &&
				(adc_val < param->kb_connect_adc_max))
				check_ok++;
		} else {
			/*
			 * VDD is enable , adc maybe in
			 * [KB_ONLINE_MIN_ADC_LIMIT KB_ONLINE_MAX_ADC_LIMIT]
			 */
			if ((adc_val > param->kb_online_adc_min) &&
				(adc_val < param->kb_online_adc_max))
				check_ok++;
		}
	}

	/* if adc check all success, mean connected, return true */
	if (check_ok == count)
		return true;

	/*
	 * if adc check had failed ,need retry check ;
	 * if retry_check < 0 , mean disconnect ,
	 * but this checked will have some mistake
	 */
	if (ret_check > 0) {
		ret_check--;
		check_ok = 0;
		msleep(RECHECK_ADC_DELAY_MS);
		goto retry_check;
	}

	return false;
}

static int sw_kb_devdetect(struct sw_gpio_detector *detector,
	struct sw_dev_detector *devdetector)
{
	bool kb_isonline;
	struct sw_kb_detectparam *kb_param;

	if ((detector == NULL) || (devdetector == NULL))
		return -EINVAL;

	if ((detector->dev_state != DEVSTAT_NONEDEV) &&
		(detector->dev_state != DEVSTAT_KBDEV_ONLINE))
		return -EINVAL;

	kb_param = (struct sw_kb_detectparam *)devdetector->param;
	if (kb_param == NULL) {
		SW_PRINT_INFO("sw_is_kb_online param is null\n");
		return -EINVAL;
	}

	kb_isonline = sw_is_kb_online(detector->detect_adc_no, kb_param);
	if (kb_isonline)
		sw_keyboard_connected(detector, kb_param);
	else
		sw_keyboard_disconnected(detector, kb_param);

	return 0;
}

static void sw_kb_free_param(struct sw_kb_detectparam *kb_param)
{
	if (kb_param == NULL)
		return;

	if (kb_param->kb_tx_gpio >= 0) {
		gpio_free(kb_param->kb_tx_gpio);
		kb_param->kb_tx_gpio = -1;
	}

	if (kb_param->kb_vdd_ctrl >= 0) {
		gpio_free(kb_param->kb_vdd_ctrl);
		kb_param->kb_vdd_ctrl = -1;
	}
	kfree(kb_param);
}

static int sw_kb_notifyevent(struct sw_gpio_detector *detector,
	struct sw_dev_detector *devdetector,
	unsigned long event, void *pData)
{
	struct sw_kb_detectparam *kb_param;

	if ((detector == NULL) || (devdetector == NULL))
		return -EINVAL;

	kb_param = (struct sw_kb_detectparam *)devdetector->param;

	/* in first , process destroy event */
	if (event == SW_NOTIFY_EVENT_DESTORY) {
		sw_kb_free_param(kb_param);
		kfree(devdetector);
		return 0;
	}

	/* for bussiness */
	if ((detector->dev_state != DEVSTAT_NONEDEV) &&
			(detector->dev_state != DEVSTAT_KBDEV_ONLINE))
		return -EINVAL;

	if (kb_param == NULL) {
		SW_PRINT_INFO("sw_is_kb_online param is null\n");
		return -EINVAL;
	}

	if (event == SW_NOTIFY_EVENT_DISCONNECT)
		sw_keyboard_disconnected(detector, kb_param);

	return 0;
}

static int sw_parse_kbdetectparam(struct device_node *np,
	struct sw_kb_detectparam *kb_param)
{
	kb_param->kb_tx_gpio = -1;
	kb_param->kb_vdd_ctrl = -1;
	kb_param->kb_connect_adc_min = -1;
	kb_param->kb_connect_adc_max = -1;
	kb_param->kb_online_adc_min = -1;
	kb_param->kb_online_adc_max = -1;

	if (of_property_read_u32(np, "kb_connect_adc_min",
				&kb_param->kb_connect_adc_min)) {
		SW_PRINT_ERR("dts:can not get kb_connect_adc_min\n");
		kb_param->kb_connect_adc_min = KB_ONLINE_CONN_MIN_ADC_LIMIT;
	}

	if (of_property_read_u32(np, "kb_connect_adc_max",
				&kb_param->kb_connect_adc_max)) {
		SW_PRINT_ERR("dts:can not get kb_connect_adc_max\n");
		kb_param->kb_connect_adc_max = KB_ONLINE_CONN_MAX_ADC_LIMIT;
	}

	if (of_property_read_u32(np, "kb_online_adc_min",
				&kb_param->kb_online_adc_min)) {
		SW_PRINT_ERR("dts:can not get kb_online_adc_min\n");
		kb_param->kb_online_adc_min = KB_ONLINE_MIN_ADC_LIMIT;
	}

	if (of_property_read_u32(np, "kb_online_adc_max",
				&kb_param->kb_online_adc_max)) {
		SW_PRINT_ERR("dts:can not get kb_online_adc_max\n");
		kb_param->kb_online_adc_max = KB_ONLINE_MAX_ADC_LIMIT;
	}

	/* read keyborad TX gpio,default HIGH */
	kb_param->kb_tx_gpio = sw_get_named_gpio(np,
		"gpio_kb_tx", GPIOD_OUT_HIGH);
	if (kb_param->kb_tx_gpio < 0) {
		SW_PRINT_ERR("kb_tx_gpio failed\n");
		goto err_free_gpio;
	}

	/* read keyboard VDD control ,default LOW */
	kb_param->kb_vdd_ctrl = sw_get_named_gpio(np,
		"gpio_kb_vdd_ctrl", GPIOD_OUT_LOW);
	if (kb_param->kb_vdd_ctrl < 0) {
		SW_PRINT_ERR("kb_vdd_ctrl failed\n");
		goto err_free_gpio;
	}

	return 0;

err_free_gpio:
	if (kb_param->kb_tx_gpio >= 0) {
		gpio_free(kb_param->kb_tx_gpio);
		kb_param->kb_tx_gpio = -1;
	}

	if (kb_param->kb_vdd_ctrl >= 0) {
		gpio_free(kb_param->kb_vdd_ctrl);
		kb_param->kb_vdd_ctrl = -1;
	}

	return -EINVAL;
}

struct sw_dev_detector *sw_load_kb_detect(struct device_node *np)
{
	struct sw_dev_detector *dev_detector;
	struct sw_kb_detectparam *kb_param;
	int ret;

	if (np == NULL) {
		SW_PRINT_ERR("param failed\n");
		return NULL;
	}

	dev_detector = kzalloc(sizeof(*dev_detector), GFP_KERNEL);
	if (dev_detector == NULL)
		return NULL;

	kb_param = kzalloc(sizeof(*kb_param), GFP_KERNEL);
	if (kb_param == NULL)
		goto err_kb_param;

	ret = sw_parse_kbdetectparam(np, kb_param);
	if (ret < 0) {
		SW_PRINT_ERR("dts parse failed\n");
		goto err_core_init;
	}

	dev_detector->detect_call = sw_kb_devdetect;
	dev_detector->event_call = sw_kb_notifyevent;
	dev_detector->param = kb_param;

	return dev_detector;

err_core_init:
	kfree(kb_param);
err_kb_param:
	kfree(dev_detector);
	return NULL;
}
