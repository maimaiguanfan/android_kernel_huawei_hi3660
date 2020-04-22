/*
 * vbus_channel_boost_gpio.c
 *
 * boost gpio for vbus channel driver
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

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/vbus_channel/vbus_channel.h>
#include <huawei_platform/power/vbus_channel/vbus_channel_boost_gpio.h>
#include <huawei_platform/power/wired_channel_switch.h>
#include <huawei_platform/power/wireless_transmitter.h>
#include <huawei_platform/power/boost_5v.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG vbus_ch_boost_gpio
HWLOG_REGIST();

static struct boost_gpio_dev *g_boost_gpio_dev;

static struct boost_gpio_dev *boost_gpio_get_dev(void)
{
	if (!g_boost_gpio_dev) {
		hwlog_err("g_boost_gpio_dev is null\n");
		return NULL;
	}

	return g_boost_gpio_dev;
}

/* fix a hardware issue, has leakage when open boost gpio */
static void boost_gpio_charge_otg_close_work(struct work_struct *w)
{
	hwlog_info("fix hw issue: close charger otg on work\n");
	charge_otg_mode_enable(OTG_DISABLE, OTG_CTRL_WIRED_OTG);
}

static int boost_gpio_start_config(int flag)
{
	struct boost_gpio_dev *l_dev = NULL;

	l_dev = boost_gpio_get_dev();
	if (!l_dev)
		return -EINVAL;

	l_dev->mode = VBUS_CH_IN_OTG_MODE;

	wired_chsw_set_wired_channel(WIRED_CHANNEL_CUTOFF);
#ifdef CONFIG_WIRELESS_CHARGER
	wireless_tx_cancel_work(PWR_SW_BY_OTG_ON);
#endif /* CONFIG_WIRELESS_CHARGER */

	msleep(100); /* delay 100ms for hardware */
	boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_BOOST_GPIO_OTG);
	gpio_set_value(l_dev->gpio_en, BOOST_GPIO_SWITCH_ENABLE);

#ifdef CONFIG_WIRELESS_CHARGER
	wireless_tx_restart_check(PWR_SW_BY_OTG_ON);
#endif /* CONFIG_WIRELESS_CHARGER */

	/* fix a hardware issue, has leakage when open boost gpio */
	if (l_dev->charge_otg_ctl_flag == 1) {
		hwlog_info("fix hw issue: open charger otg\n");
		charge_otg_mode_enable(OTG_ENABLE, OTG_CTRL_WIRED_OTG);

		cancel_delayed_work_sync(&l_dev->charge_otg_close_work);
		schedule_delayed_work(&l_dev->charge_otg_close_work,
			msecs_to_jiffies(CHARGE_OTG_CLOSE_WORK_TIMEOUT));
	}

	hwlog_info("start reverse_vbus(boost_gpio,%d)\n", flag);
	return 0;
}

static int boost_gpio_stop_config(int flag)
{
	struct boost_gpio_dev *l_dev = NULL;

	l_dev = boost_gpio_get_dev();
	if (!l_dev)
		return -EINVAL;

	if (flag) {
		l_dev->mode = VBUS_CH_NOT_IN_OTG_MODE;
#ifdef CONFIG_WIRELESS_CHARGER
		wireless_tx_cancel_work(PWR_SW_BY_OTG_OFF);
#endif /* CONFIG_WIRELESS_CHARGER */
	}

	gpio_set_value(l_dev->gpio_en, BOOST_GPIO_SWITCH_DISABLE);
	boost_5v_enable(BOOST_5V_DISABLE, BOOST_CTRL_BOOST_GPIO_OTG);

	if (flag) {
#ifdef CONFIG_WIRELESS_CHARGER
		wireless_tx_restart_check(PWR_SW_BY_OTG_OFF);
#endif /* CONFIG_WIRELESS_CHARGER */
	}

	/* fix a hardware issue, has leakage when open boost gpio */
	if (l_dev->charge_otg_ctl_flag == 1) {
		hwlog_info("fix hw issue: close charger otg\n");
		charge_otg_mode_enable(OTG_DISABLE, OTG_CTRL_WIRED_OTG);

		cancel_delayed_work_sync(&l_dev->charge_otg_close_work);
	}

	hwlog_info("stop reverse_vbus(boost_gpio,%d)\n", flag);
	return 0;
}

static int boost_gpio_open(unsigned int user, int flag)
{
	struct boost_gpio_dev *l_dev = NULL;

	l_dev = boost_gpio_get_dev();
	if (!l_dev)
		return -EINVAL;

	if (boost_gpio_start_config(flag))
		return -EINVAL;

	l_dev->user |= (1 << user);

	hwlog_info("boost_gpio(%x) open ok\n", l_dev->user);
	return 0;
}

static int boost_gpio_close(unsigned int user, int flag, int force)
{
	struct boost_gpio_dev *l_dev = NULL;

	l_dev = boost_gpio_get_dev();
	if (!l_dev)
		return -EINVAL;

	l_dev->user &= ~(1 << user);

	if (l_dev->user == VBUS_CH_NO_OP_USER) {
		if (boost_gpio_stop_config(flag))
			return -EINVAL;
	}

	hwlog_info("boost_gpio(%x) close ok\n", l_dev->user);
	return 0;
}

static int boost_gpio_get_state(unsigned int user, int *state)
{
	struct boost_gpio_dev *l_dev = NULL;

	l_dev = boost_gpio_get_dev();
	if (!l_dev)
		return -EINVAL;

	if (l_dev->user == VBUS_CH_NO_OP_USER)
		*state = VBUS_CH_STATE_CLOSE;
	else
		*state = VBUS_CH_STATE_OPEN;

	return 0;
}

static int boost_gpio_get_mode(unsigned int user, int *mode)
{
	struct boost_gpio_dev *l_dev = NULL;

	l_dev = boost_gpio_get_dev();
	if (!l_dev)
		return -EINVAL;

	*mode = l_dev->mode;

	return 0;
}

struct vbus_ch_ops boost_gpio_ops = {
	.type_name = "boost_gpio",
	.open = boost_gpio_open,
	.close = boost_gpio_close,
	.get_state = boost_gpio_get_state,
	.get_mode = boost_gpio_get_mode,
	.set_switch_mode = NULL,
	.set_voltage = NULL,
	.get_voltage = NULL,
};

static int boost_gpio_parse_dts(struct device_node *np,
	struct boost_gpio_dev *l_dev)
{
	int ret;

	l_dev->gpio_en = of_get_named_gpio(np, "gpio_otg_switch", 0);
	hwlog_info("gpio_otg_switch=%d\n", l_dev->gpio_en);

	if (!gpio_is_valid(l_dev->gpio_en)) {
		hwlog_err("gpio(gpio_otg_switch) is not valid\n");
		return -EINVAL;
	}

	ret = gpio_request(l_dev->gpio_en, "gpio_otg_switch");
	if (ret) {
		hwlog_err("gpio(gpio_otg_switch) request fail\n");
		return ret;
	}

	ret = gpio_direction_output(l_dev->gpio_en,
		BOOST_GPIO_SWITCH_DISABLE);
	if (ret) {
		hwlog_err("gpio(gpio_otg_switch) set output fail\n");
		gpio_free(l_dev->gpio_en);
		return ret;
	}

	/* fix a hardware issue, has leakage when open boost gpio */
	ret = of_property_read_u32(np, "charge_otg_ctl_flag",
		&l_dev->charge_otg_ctl_flag);
	if (ret) {
		hwlog_err("charge_otg_ctl_flag dts read failed\n");
		l_dev->charge_otg_ctl_flag = 0;
	}
	hwlog_info("charge_otg_ctl_flag=%d\n", l_dev->charge_otg_ctl_flag);

	return 0;
}

static int boost_gpio_probe(struct platform_device *pdev)
{
	int ret;
	struct boost_gpio_dev *l_dev = NULL;

	hwlog_info("probe begin\n");

	l_dev = devm_kzalloc(&pdev->dev, sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_boost_gpio_dev = l_dev;

	l_dev->pdev = pdev;
	l_dev->dev = &pdev->dev;
	if (!l_dev->pdev || !l_dev->dev || !l_dev->dev->of_node) {
		hwlog_err("device_node is null\n");
		ret = -EINVAL;
		goto fail_free_mem;
	}

	ret = boost_gpio_parse_dts(l_dev->dev->of_node, l_dev);
	if (ret)
		goto fail_parse_dts;

	ret = vbus_ch_ops_register(&boost_gpio_ops);
	if (ret)
		goto fail_register_ops;

	/* fix a hardware issue, has leakage when open boost gpio */
	/* 1: init a work for this issue */
	if (l_dev->charge_otg_ctl_flag == 1)
		INIT_DELAYED_WORK(&l_dev->charge_otg_close_work,
			boost_gpio_charge_otg_close_work);

	platform_set_drvdata(pdev, l_dev);

	hwlog_info("probe end\n");
	return 0;

fail_register_ops:
	gpio_free(l_dev->gpio_en);

fail_parse_dts:
fail_free_mem:
	devm_kfree(&pdev->dev, l_dev);
	g_boost_gpio_dev = NULL;

	return ret;
}

static int boost_gpio_remove(struct platform_device *pdev)
{
	struct boost_gpio_dev *l_dev = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	if (!l_dev) {
		hwlog_err("l_dev is null\n");
		return -EINVAL;
	}

	if (l_dev->gpio_en)
		gpio_free(l_dev->gpio_en);

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id boost_gpio_match_table[] = {
	{
		.compatible = "huawei,vbus_channel_boost_gpio",
		.data = NULL,
	},
	{},
};

static struct platform_driver boost_gpio_driver = {
	.probe = boost_gpio_probe,
	.remove = boost_gpio_remove,
	.driver = {
		.name = "huawei,vbus_channel_boost_gpio",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(boost_gpio_match_table),
	},
};

static int __init boost_gpio_init(void)
{
	return platform_driver_register(&boost_gpio_driver);
}

static void __exit boost_gpio_exit(void)
{
	platform_driver_unregister(&boost_gpio_driver);
}

fs_initcall(boost_gpio_init);
module_exit(boost_gpio_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("boost gpio for vbus channel module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
