/*
 * hw_ycable.c
 *
 * huawei ycable driver
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
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/usb/hw_ycable.h>
#include "../otg_gpio_id/hw_otg_id.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG hw_ycable
HWLOG_REGIST();

static struct ycable_info *g_ycable;

static const char * const ycable_id_status_table[] = {
	[YCABLE_ID_HIGH] = "id_high",
	[YCABLE_ID_LOW_OTG] = "id_low_otg",
	[YCABLE_ID_LOW_WITH_CHARGE] = "id_low_with_charge",
	[YCABLE_ID_LOW_INVALID] = "id_low_invalid",
};

static const char *ycable_get_id_status_name(unsigned int index)
{
	if ((index >= YCABLE_ID_BEGIN) && (index < YCABLE_ID_END))
		return ycable_id_status_table[index];

	return "id_invalid";
}

int ycable_register_event_notifier(struct notifier_block *nb)
{
	struct ycable_info *di = g_ycable;

	if (!di || !nb) {
		hwlog_err("di or nb is null\n");
		return -EINVAL;
	}

	return blocking_notifier_chain_register(&di->ycable_evt_nh, nb);
}
EXPORT_SYMBOL(ycable_register_event_notifier);

int ycable_unregister_event_notifier(struct notifier_block *nb)
{
	struct ycable_info *di = g_ycable;

	if (!di || !nb) {
		hwlog_err("di or nb is null\n");
		return -EINVAL;
	}

	return blocking_notifier_chain_unregister(&di->ycable_evt_nh, nb);
}
EXPORT_SYMBOL(ycable_unregister_event_notifier);

static int ycable_event_notifier_call(struct ycable_info *di,
	unsigned long event, void *data)
{
	int ret = -1;
	unsigned long y_event = CHARGER_TYPE_NONE;

	if (!di) {
		hwlog_err("di is null\n");
		return ret;
	}

	if ((event < YCABLE_STATUS_BEGIN) || (event >= YCABLE_STATUS_END)) {
		hwlog_err("ycable event is invalid\n");
		return ret;
	}

	if (event == YCABLE_UNKNOW)
		y_event = CHARGER_TYPE_NONE;
	else if (event == YCABLE_CHARGER)
		y_event = CHARGER_TYPE_UNKNOWN;
	else if (event == YCABLE_OTG)
		y_event = PLEASE_PROVIDE_POWER;

	return blocking_notifier_call_chain(&di->ycable_evt_nh, y_event, data);
}

int ycable_is_support(void)
{
	struct ycable_info *di = g_ycable;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	return di->ycable_support;
}

enum ycable_status ycable_get_status(void)
{
	struct ycable_info *di = g_ycable;

	if (!di) {
		hwlog_err("di is null\n");
		return YCABLE_UNKNOW;
	}

	return di->ycable_status;
}

int ycable_get_input_current(void)
{
	struct ycable_info *di = g_ycable;

	if (!di) {
		hwlog_err("di is null\n");
		return YCABLE_CURR_0MA;
	}

	return di->ycable_iin_curr;
}

int ycable_get_charge_current(void)
{
	struct ycable_info *di = g_ycable;

	if (!di) {
		hwlog_err("di is null\n");
		return YCABLE_CURR_0MA;
	}

	return di->ycable_ichg_curr;
}

int ycable_get_gpio_adc_min(void)
{
	return YCABLE_CHG_THRESHOLD_VOLTAGE_MAX;
}

static bool ycable_start_charging(struct ycable_info *di)
{
	if (!di) {
		hwlog_err("di is null\n");
		return false;
	}

	if (di->ycable_status == YCABLE_CHARGER)
		return false;

	hwlog_info("ycable_status=%d, charge_status=%d, otg_status=%d\n",
		di->ycable_status, di->ycable_charger_enable_flag,
		di->ycable_otg_enable_flag);

	/* if now is otg boost mode, stop it first */
	if (di->ycable_otg_enable_flag) {
		ycable_event_notifier_call(di, YCABLE_UNKNOW, NULL);
		di->ycable_otg_enable_flag = false;
		di->ycable_status = YCABLE_UNKNOW;
		schedule_delayed_work(&di->ycable_work,
			msecs_to_jiffies(YCABLE_OTG_ENABLE_WORK_TIMEOUT));
		return true;
	}

	di->ycable_status = YCABLE_CHARGER;
	di->ycable_charger_enable_flag = true;
	ycable_event_notifier_call(di, YCABLE_CHARGER, NULL);
	return false;
}

static bool ycable_start_otg(struct ycable_info *di)
{
	if (!di) {
		hwlog_err("di is null\n");
		return false;
	}

	if (di->ycable_status == YCABLE_OTG)
		return false;

	hwlog_info("ycable_status=%d, charge_status=%d, otg_status=%d\n",
		di->ycable_status, di->ycable_charger_enable_flag,
		di->ycable_otg_enable_flag);

	if (di->ycable_charger_enable_flag) {
		ycable_event_notifier_call(di, YCABLE_UNKNOW, NULL);
		di->ycable_charger_enable_flag = false;
		di->ycable_status = YCABLE_UNKNOW;
		schedule_delayed_work(&di->ycable_work,
			msecs_to_jiffies(YCABLE_OTG_ENABLE_WORK_TIMEOUT));
		return true;
	}

	di->ycable_status = YCABLE_OTG;
	di->ycable_otg_enable_flag = true;
	ycable_event_notifier_call(di, YCABLE_OTG, NULL);
	return false;
}

static void ycable_stop_charging_and_otg(struct ycable_info *di)
{
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	di->ycable_status = YCABLE_UNKNOW;
	di->ycable_otg_enable_flag = false;
	di->ycable_charger_enable_flag = false;
	ycable_event_notifier_call(di, YCABLE_UNKNOW, NULL);
	reinit_completion(&di->dev_off_completion);

	hwlog_info("stop charging and otg\n");
}

enum ycable_id_status ycable_get_id_status(int vol_value)
{
	enum ycable_id_status status = YCABLE_ID_LOW_INVALID;

	if ((vol_value >= YCABLE_CHG_THRESHOLD_VOLTAGE_MIN) &&
		(vol_value <= YCABLE_CHG_THRESHOLD_VOLTAGE_MAX))
		status = YCABLE_ID_LOW_WITH_CHARGE;
	else if ((vol_value >= YCABLE_OTG_THRESHOLD_VOLTAGE_MIN) &&
		(vol_value <= YCABLE_OTG_THRESHOLD_VOLTAGE_MAX))
		status = YCABLE_ID_LOW_OTG;
	else if (vol_value >= YCABLE_CHG_THRESHOLD_VOLTAGE_MAX)
		status = YCABLE_ID_HIGH;
	else if ((vol_value >= YCABLE_OTG_THRESHOLD_VOLTAGE_MAX) &&
		(vol_value <= YCABLE_CHG_THRESHOLD_VOLTAGE_MIN))
		status = YCABLE_ID_LOW_INVALID;
	else if (vol_value == YCABLE_INVALID_THRESHOLD_VOLTAGE)
		status = YCABLE_ID_LOW_INVALID;

	return status;
}

bool ycable_is_charge_connect(int vol_value)
{
	enum ycable_id_status id_status;

	id_status = ycable_get_id_status(vol_value);

	if (id_status == YCABLE_ID_LOW_WITH_CHARGE) {
		hwlog_info("now ycable is with charger\n");
		return true;
	}

	return false;
}

bool ycable_is_with_charger(void)
{
	struct ycable_info *di = g_ycable;
	int adc_value;
	bool charge_connect = false;

	if (!di) {
		hwlog_err("di is null\n");
		return false;
	}

	if (di->otg_adc_channel != YCABLE_ADC_INVALID)
		adc_value = hisi_adc_get_value(di->otg_adc_channel);
	else
		adc_value = YCABLE_INVALID_THRESHOLD_VOLTAGE;

	charge_connect = ycable_is_charge_connect(adc_value);

	return charge_connect;
}

static void ycable_monitor_work(struct work_struct *work)
{
	int ret;
	struct ycable_info *di = g_ycable;
	int vol_value;
	int gpio_value = GPIO_HIGH;
	bool need_return = false;
	enum ycable_id_status id_status = YCABLE_ID_LOW_INVALID;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	ret = hw_get_otg_id_gpio_value(&gpio_value);
	if (ret) {
		hwlog_err("ycable get gpio_value fail\n");
		ycable_stop_charging_and_otg(di);
		return;
	}

	if (gpio_value == GPIO_HIGH) {
		hwlog_info("ycable charger or otg removed\n");
		ycable_stop_charging_and_otg(di);
		return;
	}

	if (di->otg_adc_channel != YCABLE_ADC_INVALID)
		vol_value = hisi_adc_get_value(di->otg_adc_channel);
	else
		vol_value = YCABLE_INVALID_THRESHOLD_VOLTAGE;

	id_status = ycable_get_id_status(vol_value);

	if ((id_status == YCABLE_ID_LOW_WITH_CHARGE) &&
		(di->ycable_status != YCABLE_CHARGER)) {
		hwlog_info("ycable charger plugin,id_status=%s vol_value=%d\n",
			ycable_get_id_status_name(id_status), vol_value);

		need_return = ycable_start_charging(di);
		if (need_return)
			return;
	} else if ((id_status == YCABLE_ID_LOW_OTG) &&
		(di->ycable_status != YCABLE_OTG)) {
		hwlog_info("ycable otg device insert,id_status=%s vol_value=%d\n",
			ycable_get_id_status_name(id_status), vol_value);

		need_return = ycable_start_otg(di);
		if (need_return)
			return;
	} else if ((id_status == YCABLE_ID_HIGH) ||
		(id_status == YCABLE_ID_LOW_INVALID)) {
		ycable_stop_charging_and_otg(di);
	}

	schedule_delayed_work(&di->ycable_work,
		msecs_to_jiffies(YCABLE_WORK_TIMEOUT));
}

static int ycable_usb_notifier_call(struct notifier_block *usb_nb,
	unsigned long event, void *data)
{
	struct ycable_info *di = g_ycable;

	if (!di) {
		hwlog_err("di is null\n");
		return NOTIFY_OK;
	}

	hwlog_info("usb notifier %ld\n", event);

	switch (event) {
	case CHARGER_TYPE_NONE:
		di->ycable_otg_enable_flag = false;
		di->ycable_charger_enable_flag = false;
		break;

	/* fall through: SDP,CDP,DCP,UNKNOWN all are charging event */
	case CHARGER_TYPE_SDP:
	case CHARGER_TYPE_CDP:
	case CHARGER_TYPE_DCP:
	case CHARGER_TYPE_UNKNOWN:
		di->ycable_charger_enable_flag = true;
		break;

	case PLEASE_PROVIDE_POWER:
		complete(&di->dev_off_completion);

		if (di->ycable_status == YCABLE_UNKNOW)
			di->ycable_otg_enable_flag = true;
		break;

	default:
		hwlog_err("ignore other type %ld\n", event);
		break;
	}

	return NOTIFY_OK;
}

void ycable_init_devoff_completion(void)
{
	struct ycable_info *di = g_ycable;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	reinit_completion(&di->dev_off_completion);
}

int ycable_handle_otg_event(enum otg_dev_event_type event_type, bool need_wait)
{
	int ret = -1;
	struct ycable_info *di = g_ycable;
	unsigned long timeout;

	if (!di) {
		hwlog_err("di is null\n");
		return ret;
	}

	switch (event_type) {
	case ID_FALL_EVENT:
		if (need_wait) {
			timeout = wait_for_completion_timeout(
				&di->dev_off_completion,
				msecs_to_jiffies(YCABLE_WAIT_COMPLETE_TIMEOUT));
			hwlog_info("dev_off timeout(%ld)\n", timeout);
		}

		if (di->ycable_status == YCABLE_UNKNOW)
			schedule_delayed_work(&di->ycable_work, 0);

		reinit_completion(&di->dev_off_completion);
		break;

	case ID_RISE_EVENT:
		cancel_delayed_work(&di->ycable_work);
		ycable_stop_charging_and_otg(di);
		break;

	default:
		hwlog_info("no valid event for ycable\n");
		cancel_delayed_work(&di->ycable_work);
		break;
	}

	return 0;
}

static int ycable_parse_dts(struct ycable_info *di, struct device_node *np)
{
	int ret = -1;

	if (!di || !np) {
		hwlog_err("di or np is null\n");
		return ret;
	}

	if (!of_property_read_bool(np, "ycable_support")) {
		hwlog_err("ycable_support dts read failed\n");
		return ret;
	}
	di->ycable_support = true;

	/* ycable input current  */
	ret = of_property_read_u32(np, "ycable_iin_curr",
		&(di->ycable_iin_curr));
	if (ret) {
		hwlog_err("ycable_iin_curr dts read failed\n");
		di->ycable_iin_curr = YCABLE_CURR_DEFAULT;
	}

	hwlog_info("ycable_iin_curr=%d\n", di->ycable_iin_curr);

	/* ycable charge current */
	ret = of_property_read_u32(np, "ycable_ichg_curr",
		&(di->ycable_ichg_curr));
	if (ret) {
		hwlog_err("ycable_ichg_curr dts read failed\n");
		di->ycable_ichg_curr = YCABLE_CURR_DEFAULT;
	}

	hwlog_info("ycable_ichg_curr=%d\n", di->ycable_ichg_curr);

	ret = of_property_read_u32(np, "ycable_adc_channel",
		&(di->otg_adc_channel));
	if (ret) {
		hwlog_err("ycable_adc_channel dts read failed\n");
		return ret;
	}

	hwlog_info("ycable_adc_channel=%d\n", di->otg_adc_channel);

	return ret;
}

static int ycable_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *np = NULL;
	struct device *dev = NULL;
	struct ycable_info *di = NULL;

	hwlog_info("probe begin\n");

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_ycable = di;
	platform_set_drvdata(pdev, di);

	di->pdev = pdev;
	dev = &pdev->dev;
	np = dev->of_node;

	ret = ycable_parse_dts(di, np);
	if (ret)
		goto fail_parse_dts;

	di->usb_nb.notifier_call = ycable_usb_notifier_call;
	ret = hisi_charger_type_notifier_register(&di->usb_nb);
	if (ret) {
		hwlog_err("notifier register failed\n");
		goto fail_register_notifier;
	}

	init_completion(&di->dev_off_completion);
	BLOCKING_INIT_NOTIFIER_HEAD(&di->ycable_evt_nh);

	INIT_DELAYED_WORK(&di->ycable_work, ycable_monitor_work);

	hwlog_info("probe end\n");
	return 0;

fail_register_notifier:
fail_parse_dts:
	devm_kfree(&pdev->dev, di);
	g_ycable = NULL;

	return ret;
}

static int ycable_remove(struct platform_device *pdev)
{
	struct ycable_info *di = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_ycable = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id ycable_match_table[] = {
	{
		.compatible = "huawei,ycable",
		.data = NULL,
	},
	{},
};

static struct platform_driver ycable_driver = {
	.probe = ycable_probe,
	.remove = ycable_remove,
	.driver = {
		.name = "huawei,ycable",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ycable_match_table),
	},
};

static int __init ycable_init(void)
{
	return platform_driver_register(&ycable_driver);
}

static void __exit ycable_exit(void)
{
	platform_driver_unregister(&ycable_driver);
}

module_init(ycable_init);
module_exit(ycable_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei ycable driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
