/*
 * ovp_switch.c
 *
 * ovp switch to control wired channel
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

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG ovp_chsw
HWLOG_REGIST();

static int wired_channel_by_ovp;
static int gpio_ovp_chsw_en;
static int ovp_gpio_initialized;
static int wired_channel_ovp_status = WIRED_CHANNEL_RESTORE;
static int g_gpio_low_by_set_input = 1;

static int ovp_chsw_get_wired_channel(void)
{
	return wired_channel_ovp_status;
}

static int ovp_chsw_set_gpio_low(void)
{
	int ret;

	if (g_gpio_low_by_set_input)
		ret = gpio_direction_input(gpio_ovp_chsw_en); /* restore */
	else
		ret = gpio_direction_output(gpio_ovp_chsw_en, 0); /* restore */

	return ret;
}

static int ovp_chsw_set_wired_channel(int flag)
{
	int ret;

	if (!ovp_gpio_initialized) {
		hwlog_err("ovp channel switch not initialized\n");
		return -ENODEV;
	}

	if (flag == WIRED_CHANNEL_CUTOFF) {
		/* cutoff */
		ret = gpio_direction_output(gpio_ovp_chsw_en, 1);
	} else {
		/* restore */
		ret = ovp_chsw_set_gpio_low();
	}

	hwlog_info("ovp channel switch set en(%d)\n",
		(flag == WIRED_CHANNEL_CUTOFF) ? 1 : 0);

	wired_channel_ovp_status = flag;

	return ret;
}

static struct wired_chsw_device_ops ovp_chsw_ops = {
	.get_wired_channel = ovp_chsw_get_wired_channel,
	.set_wired_channel = ovp_chsw_set_wired_channel,
};

static void ovp_chsw_parse_dts(struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(of_find_compatible_node(NULL, NULL,
			"huawei,wired_channel_switch"),
			"use_ovp_cutoff_wired_channel",
			&wired_channel_by_ovp);
	if (ret)
		wired_channel_by_ovp = 0;

	hwlog_info("wired_channel_by_ovp=%d\n", wired_channel_by_ovp);

	ret = of_property_read_u32(np, "gpio_low_by_set_input",
			&g_gpio_low_by_set_input);
	if (ret)
		g_gpio_low_by_set_input = 1;

	hwlog_info("gpio_low_by_set_input=%d\n", g_gpio_low_by_set_input);
}

static int ovp_chsw_gpio_init(struct device_node *np)
{
	int ret;

	gpio_ovp_chsw_en = of_get_named_gpio(np, "gpio_ovp_chsw_en", 0);
	hwlog_info("gpio_ovp_chsw_en=%d\n", gpio_ovp_chsw_en);

	if (!gpio_is_valid(gpio_ovp_chsw_en)) {
		hwlog_err("gpio(gpio_ovp_chsw_en) is not valid\n");
		return -EINVAL;
	}

	if (gpio_request(gpio_ovp_chsw_en, "gpio_ovp_chsw_en")) {
		hwlog_err("gpio(gpio_ovp_chsw_en) request fail\n");
		return -ENOMEM;
	}

	ret = ovp_chsw_set_gpio_low();
	if (ret) {
		hwlog_err("gpio(gpio_ovp_chsw_en) set input fail\n");
		gpio_free(gpio_ovp_chsw_en);
		return -1;
	}

	ovp_gpio_initialized = 1;

	return 0;
}

static int ovp_chsw_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *np = (&pdev->dev)->of_node;

	hwlog_info("probe begin\n");

	ovp_chsw_parse_dts(np);

	if (wired_channel_by_ovp) {
		ret = ovp_chsw_gpio_init(np);
		if (ret)
			return -1;

		ret = wired_chsw_ops_register(&ovp_chsw_ops);
		if (ret) {
			gpio_free(gpio_ovp_chsw_en);
			return -1;
		}
	}

	hwlog_info("probe end\n");
	return 0;
}

static int ovp_chsw_remove(struct platform_device *pdev)
{
	hwlog_info("remove begin\n");

	if (!gpio_is_valid(gpio_ovp_chsw_en))
		gpio_free(gpio_ovp_chsw_en);

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id ovp_chsw_match_table[] = {
	{
		.compatible = "huawei,ovp_channel_switch",
		.data = NULL,
	},
	{},
};

static struct platform_driver ovp_chsw_driver = {
	.probe = ovp_chsw_probe,
	.remove = ovp_chsw_remove,
	.driver = {
		.name = "huawei,ovp_channel_switch",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ovp_chsw_match_table),
	},
};

static int __init ovp_chsw_init(void)
{
	return platform_driver_register(&ovp_chsw_driver);
}

static void __exit ovp_chsw_exit(void)
{
	platform_driver_unregister(&ovp_chsw_driver);
}

fs_initcall_sync(ovp_chsw_init);
module_exit(ovp_chsw_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ovp switch module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
