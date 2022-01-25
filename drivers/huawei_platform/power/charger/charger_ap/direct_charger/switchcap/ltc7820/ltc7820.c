/*
 * ltc7820.c
 *
 * ltc7820 driver
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
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/raid/pq.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/direct_charger.h>
#include "ltc7820.h"
#ifdef CONFIG_WIRELESS_CHARGER
#include <huawei_platform/power/wireless_direct_charger.h>
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG ltc7820
HWLOG_REGIST();

static struct ltc7820_device_info *g_ltc7820_dev;

static int ltc7820_init_finish_flag = LTC7820_NOT_INIT;
static int ltc7820_int_notify_enable_flag = LTC7820_DISABLE_INT_NOTIFY;

static int ltc7820_discharge(int enable)
{
	return 0;
}

static int ltc7820_is_tsbat_disabled(void)
{
	return 0;
}

static int ltc7820_config_watchdog_ms(int time)
{
	return 0;
}

static int ltc7820_get_device_id(void)
{
	return SWITCHCAP_LTC7820;
}

#ifdef POWER_MODULE_DEBUG_FUNCTION
static int ltc7820_set_freq(int freq)
{
	struct ltc7820_device_info *di = g_ltc7820_dev;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	gpio_set_value(di->gpio_freq, freq);

	hwlog_info("set_freq freq=%d\n", freq);
	return 0;
}
#endif

static void ltc7820_int_en(bool en)
{
	struct ltc7820_device_info *di = g_ltc7820_dev;
	unsigned long flags;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return;
	}

	spin_lock_irqsave(&di->int_lock, flags);
	if (en != di->is_int_en) {
		di->is_int_en = en;
		if (en)
			enable_irq(di->irq_int);
		else
			disable_irq_nosync(di->irq_int);
	}
	spin_unlock_irqrestore(&di->int_lock, flags);
}

static int ltc7820_charge_enable(int enable)
{
	struct ltc7820_device_info *di = g_ltc7820_dev;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (enable) {
		gpio_set_value(di->gpio_en, enable);
		ltc7820_int_en(!!enable);
		/* wait ltc7820 startup */
		msleep(100);
		ltc7820_init_finish_flag = LTC7820_INIT_FINISH;
	} else {
		ltc7820_int_en(!!enable);
		gpio_set_value(di->gpio_en, enable);
		ltc7820_init_finish_flag = LTC7820_NOT_INIT;
		ltc7820_int_notify_enable_flag = LTC7820_DISABLE_INT_NOTIFY;
	}
	hwlog_info("charge_enable enable=%d\n", enable);
	return 0;
}

static int ltc7820_is_device_close(void)
{
	struct ltc7820_device_info *di = g_ltc7820_dev;
	int gpio_value = LTC7820_CHIP_DISABLE;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return 1;
	}

	gpio_value = gpio_get_value(di->gpio_en);

	return (gpio_value == LTC7820_CHIP_ENABLE) ? 0 : 1;
}

static int ltc7820_charge_init(void)
{
	struct ltc7820_device_info *di = g_ltc7820_dev;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	hwlog_info("switchcap ltc7820 device id is %d\n",
		ltc7820_get_device_id());

	return 0;
}

static int ltc7820_charge_exit(void)
{
	int ret = 0;
	struct ltc7820_device_info *di = g_ltc7820_dev;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = ltc7820_charge_enable(LTC7820_CHIP_DISABLE);

	return ret;
}

static void ltc7820_interrupt_work(struct work_struct *work)
{
	struct ltc7820_device_info *di;
	struct nty_data *data;
	struct atomic_notifier_head *direct_charge_fault_notifier_list;

	di = container_of(work, struct ltc7820_device_info, irq_work);
	data = &(di->nty_data);

	direct_charge_sc_get_fault_notifier(&direct_charge_fault_notifier_list);

	data->event1 = 0;
	data->event2 = 0;
	data->addr = 0;

	if (ltc7820_int_notify_enable_flag == LTC7820_ENABLE_INT_NOTIFY) {
		hwlog_info("ltc7820 chip error happened\n");

		atomic_notifier_call_chain(direct_charge_fault_notifier_list,
			DIRECT_CHARGE_FAULT_LTC7820, data);
	}

	/* clear irq */
	ltc7820_int_en(true);
}

static irqreturn_t ltc7820_interrupt(int irq, void *_di)
{
	struct ltc7820_device_info *di = _di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (di->chip_already_init == 0)
		hwlog_err("chip not init\n");

	if (ltc7820_init_finish_flag == LTC7820_INIT_FINISH)
		ltc7820_int_notify_enable_flag = LTC7820_ENABLE_INT_NOTIFY;
	else
		hwlog_info("ltc7820 ignore init int\n");

	hwlog_info("ltc7820 int happened(%d)\n", ltc7820_init_finish_flag);

	ltc7820_int_en(false);

	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}

static struct loadswitch_ops ltc7820_sysinfo_ops = {
	.ls_init = ltc7820_charge_init,
	.ls_exit = ltc7820_charge_exit,
	.ls_enable = ltc7820_charge_enable,
	.ls_discharge = ltc7820_discharge,
	.is_ls_close = ltc7820_is_device_close,
	.get_ls_id = ltc7820_get_device_id,
	.watchdog_config_ms = ltc7820_config_watchdog_ms,
	.ls_status = ltc7820_is_tsbat_disabled,
};

static int ltc7820_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct ltc7820_device_info *di = NULL;

	hwlog_info("probe begin\n");

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (di == NULL)
		return -ENOMEM;

	g_ltc7820_dev = di;

	di->pdev = pdev;
	di->dev = &pdev->dev;
	if (di->pdev == NULL || di->dev == NULL || di->dev->of_node == NULL) {
		hwlog_err("device_node is null\n");
		ret = -EINVAL;
		goto ltc7820_fail_0;
	}
	di->dev_node = di->dev->of_node;

	INIT_WORK(&di->irq_work, ltc7820_interrupt_work);

	di->gpio_en = of_get_named_gpio(di->dev_node, "gpio_en", 0);
	hwlog_info("gpio_en=%d\n", di->gpio_en);

	if (!gpio_is_valid(di->gpio_en)) {
		hwlog_err("gpio(gpio_en) is not valid\n");
		ret = -EINVAL;
		goto ltc7820_fail_0;
	}

	ret = gpio_request(di->gpio_en, "ltc7820_gpio_en");
	if (ret) {
		hwlog_err("gpio(gpio_en) request fail\n");
		goto ltc7820_fail_0;
	}

	ret = gpio_direction_output(di->gpio_en, LTC7820_CHIP_DISABLE);
	if (ret) {
		hwlog_err("gpio(gpio_en) set output fail\n");
		goto ltc7820_fail_1;
	}

	di->gpio_freq = of_get_named_gpio(di->dev_node, "gpio_freq", 0);
	hwlog_info("gpio_freq=%d\n", di->gpio_freq);

	if (!gpio_is_valid(di->gpio_freq)) {
		hwlog_err("gpio(gpio_freq) is not valid\n");
		ret = -EINVAL;
		goto ltc7820_fail_1;
	}

	ret = gpio_request(di->gpio_freq, "ltc7820_gpio_freq");
	if (ret) {
		hwlog_err("gpio(gpio_freq) request fail\n");
		goto ltc7820_fail_1;
	}

	ret = gpio_direction_output(di->gpio_freq, LTC7820_FREQ_DISABLE);
	if (ret) {
		hwlog_err("gpio(gpio_freq) set output fail\n");
		goto ltc7820_fail_2;
	}

	di->gpio_int = of_get_named_gpio(di->dev_node, "gpio_int", 0);
	hwlog_info("gpio_int=%d\n", di->gpio_int);

	if (!gpio_is_valid(di->gpio_int)) {
		hwlog_err("gpio(gpio_int) is not valid\n");
		ret = -EINVAL;
		goto ltc7820_fail_2;
	}

	ret = gpio_request(di->gpio_int, "ltc7820_gpio_int");
	if (ret) {
		hwlog_err("gpio(gpio_int) request fail\n");
		goto ltc7820_fail_2;
	}

	ret = gpio_direction_input(di->gpio_int);
	if (ret) {
		hwlog_err("gpio(gpio_int) set input fail\n");
		goto ltc7820_fail_3;
	}

	di->irq_int = gpio_to_irq(di->gpio_int);
	if (di->irq_int < 0) {
		hwlog_err("gpio(gpio_int) map to irq fail\n");
		ret = -EINVAL;
		goto ltc7820_fail_3;
	}

	ret = request_irq(di->irq_int, ltc7820_interrupt,
		IRQF_TRIGGER_FALLING, "ltc7820_int_irq", di);
	if (ret) {
		hwlog_err("gpio(gpio_int) irq request fail\n");
		di->irq_int = -1;
		goto ltc7820_fail_3;
	}

	spin_lock_init(&di->int_lock);
	di->is_int_en = true;

	ltc7820_int_en(false);

	ret = sc_ops_register(&ltc7820_sysinfo_ops);
	if (ret) {
		hwlog_err("ltc7820 sysinfo ops register fail\n");
		goto ltc7820_fail_4;
	}

	/* ltc7820 not support batinfo register */

#ifdef CONFIG_WIRELESS_CHARGER
	ret = wireless_sc_ops_register(&ltc7820_sysinfo_ops);
	if (ret) {
		hwlog_err("ltc7820 wireless_sc sysinfo ops register fail\n");
		goto ltc7820_fail_4;
	}

	/* ltc7820 not support batinfo register */
#endif

	di->chip_already_init = 1;

	platform_set_drvdata(pdev, di);

	hwlog_info("probe end\n");
	return 0;

ltc7820_fail_4:
	free_irq(di->irq_int, di);
ltc7820_fail_3:
	gpio_free(di->gpio_int);
ltc7820_fail_2:
	gpio_free(di->gpio_freq);
ltc7820_fail_1:
	gpio_free(di->gpio_en);
ltc7820_fail_0:
	g_ltc7820_dev = NULL;
	devm_kfree(&pdev->dev, di);
	return ret;
}

static int ltc7820_remove(struct platform_device *pdev)
{
	struct ltc7820_device_info *di = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	if (di->gpio_en)
		gpio_free(di->gpio_en);

	if (di->gpio_freq)
		gpio_free(di->gpio_freq);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_ltc7820_dev = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id ltc7820_match_table[] = {
	{
	 .compatible = "huawei,ltc7820",
	 .data = NULL,
	},
	{},
};

static struct platform_driver ltc7820_driver = {
	.probe = ltc7820_probe,
	.remove = ltc7820_remove,
	.driver = {
		.name = "huawei,ltc7820",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ltc7820_match_table),
	},
};

static int __init ltc7820_init(void)
{
	return platform_driver_register(&ltc7820_driver);
}

static void __exit ltc7820_exit(void)
{
	platform_driver_unregister(&ltc7820_driver);
}

module_init(ltc7820_init);
module_exit(ltc7820_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ltc7820 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
