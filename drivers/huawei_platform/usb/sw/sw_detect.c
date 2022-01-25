/*
 * sw_detect.c
 *
 * single wire device detect
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

#define GPIO_VAL_HIGH           (1)
#define GPIO_VAL_LOW            (0)
#define DEFAULT_KB_ADC_CHANNEL  (7)

static int sw_gpio_detect_notifier_register(
	struct sw_gpio_detector *detector, struct notifier_block *nb)
{
	int ret = -1;

	if ((detector != NULL) && (nb != NULL))
		ret = blocking_notifier_chain_register(
			&detector->detect_notifier, nb);
	else
		SW_PRINT_ERR("notifier head not yet init\n");
	return ret;
}

static int sw_gpio_detect_notifier_unregister(
	struct sw_gpio_detector *detector, struct notifier_block *nb)
{
	int ret = -1;

	if ((detector != NULL) && (nb != NULL))
		ret = blocking_notifier_chain_unregister(
			&detector->detect_notifier, nb);
	else
		SW_PRINT_ERR("notifier head not yet init\n");
	return ret;
}

static void sw_gpio_detect_notify(struct sw_gpio_detector *detector)
{
	SW_PRINT_INFO("sw_gpio_detect_notify+\n");
	blocking_notifier_call_chain(&detector->detect_notifier,
		detector->dev_state, detector);
	SW_PRINT_INFO("sw_gpio_detect_notify-\n");
}

static void sw_detect_int_disable_irq(struct sw_gpio_detector *detector)
{
	unsigned long flags;

	spin_lock_irqsave(&detector->irq_enabled_lock, flags);
	if (detector->irq_enabled) {
		disable_irq_nosync(detector->detect_int_irq);
		detector->irq_enabled = false;
	}
	spin_unlock_irqrestore(&detector->irq_enabled_lock, flags);
}

static void sw_detect_int_enable_irq(struct sw_gpio_detector *detector)
{
	unsigned long flags;

	spin_lock_irqsave(&detector->irq_enabled_lock, flags);
	if (!detector->irq_enabled) {
		enable_irq(detector->detect_int_irq);
		detector->irq_enabled = true;
	}
	spin_unlock_irqrestore(&detector->irq_enabled_lock, flags);
}

static void sw_enable_detectintirq(struct sw_gpio_detector *detector,
	bool enable)
{
	if (enable)
		sw_detect_int_enable_irq(detector);
	else
		sw_detect_int_disable_irq(detector);
}

static irqreturn_t sw_detect_int_irq(int irq, void *dev_data)
{
	struct sw_gpio_detector *detector = dev_data;
	int gp;

	if (detector == NULL) {
		SW_PRINT_INFO("sw_detect_int_irq detector is null\n");
		return IRQ_HANDLED;
	}

	if (detector->start_detect == 0) {
		SW_PRINT_INFO("sw_detect_int_irq detector->start_detect = 0\n");
		return IRQ_HANDLED;
	}
	gp = gpio_get_value(detector->detect_int_gpio);
	SW_PRINT_INFO("sw_wakeup_irq gpio %x\n", gp);

	sw_detect_int_disable_irq(detector);
	schedule_work(&detector->irq_work);

	return IRQ_HANDLED;
}

/*
 * handler for detect irq
 * detect by charge detector if defined
 * detect by keyboard detector if defined
 */
static void sw_kb_detect_irq_work(struct work_struct *work)
{
	int cur_devstate;
	struct sw_gpio_detector *detector = container_of(work,
		struct sw_gpio_detector, irq_work);

	SW_PRINT_INFO("sw_kb_detect_irq_work in\n");
	if (detector == NULL)
		return;

	cur_devstate = detector->dev_state;

	if (detector->chg_detecor != NULL)
		detector->chg_detecor->detect_call(detector,
			detector->chg_detecor);

	if (detector->kb_detecor != NULL)
		detector->kb_detecor->detect_call(detector,
			detector->kb_detecor);

	if (cur_devstate != detector->dev_state)
		sw_gpio_detect_notify(detector);

}

static void sw_setup_kbint_config(struct sw_gpio_detector *pdev)
{
	int irq;
	int ret;

	if (pdev == NULL)
		return;

	if (!gpio_is_valid(pdev->detect_int_gpio)) {
		SW_PRINT_ERR("detect_int_gpio no valid\n");
		pdev->detect_int_gpio = -1;
		return;
	}

	irq = gpio_to_irq(pdev->detect_int_gpio);
	if (irq < 0) {
		SW_PRINT_ERR("detect_int_gpio gpio_to_irq fail %x\n", irq);
		goto out_free;
	}

	ret = request_irq(irq, sw_detect_int_irq,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND | IRQF_ONESHOT,
		"sw_kb_int", pdev);
	if (ret) {
		SW_PRINT_ERR("detect_int_gpio request_irq fail\n");
		goto out_free;
	}
	pdev->detect_int_irq = irq;

	/*
	 * default disable irq ,
	 * because sensorhub maybe not ready when sys start
	 */
	sw_detect_int_disable_irq(pdev);

	return;

out_free:
	gpio_free(pdev->detect_int_gpio);
	pdev->detect_int_gpio = -1;
}

int sw_get_named_gpio(struct device_node *np,
	const char *propname, enum gpiod_flags flags)
{
	int gpio;
	int ret;

	if ((np == NULL) || (propname == NULL))
		return -1;

	gpio = of_get_named_gpio(np, propname, 0);
	if (gpio == -EPROBE_DEFER)
		gpio = of_get_named_gpio(np, propname, 0);

	if (!gpio_is_valid(gpio)) {
		SW_PRINT_ERR("get gpio [%s] fail\n", propname);
		return -1;
	}

	ret = gpio_request(gpio, propname);
	if (ret < 0) {
		SW_PRINT_ERR("request gpio [%s] fail\n", propname);
		gpio_free(gpio);
		return -1;
	}

	if (flags == GPIOD_OUT_LOW)
		ret = gpio_direction_output(gpio, GPIO_VAL_LOW);
	else if (flags == GPIOD_OUT_HIGH)
		ret = gpio_direction_output(gpio, GPIO_VAL_HIGH);
	else if (flags == GPIOD_IN)
		ret = gpio_direction_input(gpio);

	if (ret < 0) {
		SW_PRINT_ERR("set gpio [%s] flags fail\n", propname);
		gpio_free(gpio);
		return -1;
	}

	return gpio;
}

static int sw_parse_devtree(struct sw_gpio_detector *detector)
{
	struct device_node *np;
	u32 val;
	int ret;

	/* should be the same as dts node compatible property */
	np = of_find_compatible_node(NULL, NULL, "huawei,sw_kb");
	if (np == NULL) {
		SW_PRINT_ERR("unable to find %s\n", "huawei,sw_kb");
		return -ENOENT;
	}

	/* read adc channel */
	if (of_property_read_u32(np, "adc_kb_detect", &val)) {
		SW_PRINT_ERR("dts:can not get adc_kb_detect\n");
		val = DEFAULT_KB_ADC_CHANNEL;
	}
	detector->detect_adc_no = val;

	/* read id int gpio */
	detector->detect_int_gpio = sw_get_named_gpio(np,
		"gpio_kb_int", GPIOD_IN);
	sw_setup_kbint_config(detector);
	if (detector->detect_int_gpio < 0) {
		SW_PRINT_ERR("detect_int_gpio failed\n");
		ret = -EINVAL;
		goto err_free_gpio;
	}

	/* read kb detect */
	detector->kb_detecor = sw_load_kb_detect(np);
	if (detector->kb_detecor == NULL) {
		ret = -EINVAL;
		goto err_free_irq;
	}

	/* read chg detect */
	if (of_property_read_u32(np, "enable_chg", &val)) {
		SW_PRINT_ERR("dts:can not get enable_chg\n");
		detector->chg_detecor = NULL;
	} else {
		detector->chg_detecor = sw_load_chg_detect(np, val);
	}

	return 0;

err_free_irq:
	free_irq(detector->detect_int_irq, detector);
	detector->detect_int_irq = -1;

err_free_gpio:
	if (detector->detect_int_gpio >= 0) {
		gpio_free(detector->detect_int_gpio);
		detector->detect_int_gpio = -1;
	}

	return ret;
}

static void sw_gpio_notifyevent(struct sw_gpio_detector *sw_detector,
	unsigned long event, void *pdata)
{
	if ((sw_detector->chg_detecor != NULL) &&
		(sw_detector->chg_detecor->event_call != NULL)) {
		sw_detector->chg_detecor->event_call(sw_detector,
			sw_detector->chg_detecor, event, pdata);
	}

	if ((sw_detector->kb_detecor != NULL) &&
		(sw_detector->kb_detecor->event_call != NULL)) {
		sw_detector->kb_detecor->event_call(sw_detector,
			sw_detector->kb_detecor, event, pdata);
	}
}

static void sw_gpio_detect_exit(struct sw_gpio_detector *detector,
	void *pdata)
{
	struct sw_detector_ops *ops = (struct sw_detector_ops *)pdata;

	if (ops != NULL)
		sw_gpio_detect_notifier_unregister(detector, &ops->detect_nb);

	if (detector->detect_int_irq >= 0) {
		free_irq(detector->detect_int_irq, detector);
		detector->detect_int_irq = -1;
	}

	if (detector->detect_int_gpio >= 0) {
		gpio_free(detector->detect_int_gpio);
		detector->detect_int_gpio = -1;
	}

	if ((detector->kb_detecor != NULL) &&
		(detector->kb_detecor->event_call != NULL)) {
		detector->kb_detecor->event_call(detector,
			detector->kb_detecor, SW_NOTIFY_EVENT_DESTORY, NULL);
		detector->kb_detecor = NULL;
	}

	if ((detector->chg_detecor != NULL) &&
		(detector->chg_detecor->event_call != NULL)) {
		detector->chg_detecor->event_call(detector,
			detector->chg_detecor, SW_NOTIFY_EVENT_DESTORY, NULL);
		detector->chg_detecor = NULL;
	}
	kfree(detector);
}

static void sw_gpio_detect_do_events(void *detector,
	unsigned long event, void *pdata)
{
	struct sw_gpio_detector *sw_detector =
		(struct sw_gpio_detector *)detector;

	if (sw_detector == NULL)
		return;

	switch (event) {
	case SW_NOTIFY_EVENT_DISCONNECT:
		sw_gpio_notifyevent(sw_detector, event, pdata);
		break;
	case SW_NOTIFY_EVENT_REDETECT:
		schedule_work(&sw_detector->irq_work);
		break;
	case SW_NOTIFY_EVENT_DESTORY:
		sw_gpio_detect_exit(sw_detector, pdata);
		break;
	default:
		break;
	}
}

int sw_gpio_detect_register(struct platform_device *pdev,
	struct sw_detector_ops *ops)
{
	struct sw_gpio_detector *detector;
	int ret;

	SW_PRINT_FUNCTION_NAME;
	if ((pdev == NULL) || (ops == NULL)) {
		SW_PRINT_ERR("param failed\n");
		return -EINVAL;
	}

	detector = kzalloc(sizeof(*detector), GFP_KERNEL);
	if (detector == NULL)
		return -ENOMEM;

	ops->detector = detector;
	ops->notify = sw_gpio_detect_do_events;

	/* init kb int irq control */
	spin_lock_init(&detector->irq_enabled_lock);
	detector->irq_enabled = true;
	detector->control_irq = sw_enable_detectintirq;

	ret = sw_parse_devtree(detector);
	if (ret < 0) {
		SW_PRINT_ERR("dts parse failed\n");
		goto err_core_init;
	}

	INIT_WORK(&detector->irq_work, sw_kb_detect_irq_work);
	BLOCKING_INIT_NOTIFIER_HEAD(&detector->detect_notifier);
	sw_gpio_detect_notifier_register(detector, &ops->detect_nb);

	return 0;

err_core_init:
	kfree(detector);
	return ret;
}
