/*
 * wired_channel_switch.c
 *
 * wired channel switch
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

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/wired_channel_switch.h>
#ifdef CONFIG_WIRELESS_CHARGER
#include <huawei_platform/power/wireless_transmitter.h>
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG wired_chsw
HWLOG_REGIST();

static struct wired_chsw_device_ops *g_chsw_ops;

int wired_chsw_ops_register(struct wired_chsw_device_ops *ops)
{
	if (ops && !g_chsw_ops) {
		g_chsw_ops = ops;
		hwlog_info("wired_chsw ops register ok\n");
		return 0;
	}

	hwlog_err("wired_chsw ops register fail\n");
	return -EPERM;
}

int wired_chsw_get_wired_channel(void)
{
	if (!g_chsw_ops || !g_chsw_ops->get_wired_channel) {
		hwlog_err("g_chsw_ops or get_wired_channel is null\n");
		return WIRED_CHANNEL_RESTORE;
	}

	return g_chsw_ops->get_wired_channel();
}

int wired_chsw_set_wired_channel(int state)
{
	int ret;
	int new_state;

	if (!g_chsw_ops || !g_chsw_ops->set_wired_channel) {
		hwlog_err("g_chsw_ops or set_wired_channel is null\n");
		return 0;
	}

	new_state = wired_chsw_get_wired_channel();
	if (state == new_state) {
		hwlog_info("wired channel is already %s\n",
			((new_state == WIRED_CHANNEL_RESTORE) ? "on" : "off"));
		return 0;
	}

#ifdef CONFIG_WIRELESS_CHARGER
	if (state == WIRED_CHANNEL_RESTORE)
		wireless_tx_cancel_work(PWR_SW_BY_VBUS_ON);
	else
		wireless_tx_cancel_work(PWR_SW_BY_VBUS_OFF);
#endif /* CONFIG_WIRELESS_CHARGER */

	ret = g_chsw_ops->set_wired_channel(state);
	if (!ret) {
		new_state = wired_chsw_get_wired_channel();
		hwlog_info("wired channel is set to %s\n",
			((new_state == WIRED_CHANNEL_RESTORE) ? "on" : "off"));
	}

#ifdef CONFIG_WIRELESS_CHARGER
	if (state == WIRED_CHANNEL_RESTORE)
		wireless_tx_restart_check(PWR_SW_BY_VBUS_ON);
	else
		wireless_tx_restart_check(PWR_SW_BY_VBUS_OFF);
#endif /* CONFIG_WIRELESS_CHARGER */

	return 0;
}

int wired_chsw_set_wired_reverse_channel(int state)
{
	if (!g_chsw_ops || !g_chsw_ops->set_wired_reverse_channel) {
		hwlog_err("g_chsw_ops or set_wired_reverse_channel is null\n");
		return -1;
	}

	return g_chsw_ops->set_wired_reverse_channel(state);
}

static int wired_chsw_check_ops(void)
{
	int ret = 0;

	if (!g_chsw_ops || !g_chsw_ops->set_wired_channel) {
		hwlog_err("g_chsw_ops ops is null\n");
		ret = -EINVAL;
	}

	return ret;
}

static int wired_chsw_probe(struct platform_device *pdev)
{
	int ret;

	hwlog_info("probe begin\n");

	ret = wired_chsw_check_ops();
	if (ret)
		return -1;

	hwlog_info("probe end\n");
	return 0;
}

static const struct of_device_id wired_chsw_match_table[] = {
	{
		.compatible = "huawei,wired_channel_switch",
		.data = NULL,
	},
	{},
};

static struct platform_driver wired_chsw_driver = {
	.probe = wired_chsw_probe,
	.driver = {
		.name = "huawei,wired_channel_switch",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(wired_chsw_match_table),
	},
};

static int __init wired_chsw_init(void)
{
	return platform_driver_register(&wired_chsw_driver);
}

static void __exit wired_chsw_exit(void)
{
	platform_driver_unregister(&wired_chsw_driver);
}

module_init(wired_chsw_init);
module_exit(wired_chsw_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wired channel switch module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
