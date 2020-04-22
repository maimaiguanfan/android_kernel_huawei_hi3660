/*
 * direct_charger_power_supply.c
 *
 * power supply for direct charger
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
#include <huawei_platform/power/direct_charger_power_supply.h>
#include <huawei_platform/power/direct_charger.h>
#include <huawei_platform/power/huawei_charger.h>
#ifdef CONFIG_BOOST_5V
#include <huawei_platform/power/boost_5v.h>
#endif
#ifdef CONFIG_USB_AUDIO_POWER
#include <huawei_platform/audio/usb_audio_power.h>
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG direct_charger_ps
HWLOG_REGIST();

static int scp_ps_by_5vboost;
static int scp_ps_by_charger;

static int is_need_bst_ctrl;
static int bst_ctrl;
static int bst_ctrl_use_common_gpio;

int direct_charge_set_bst_ctrl(int enable)
{
	int ret = 0;

	if (is_need_bst_ctrl) {
		if (!bst_ctrl_use_common_gpio) {
			ret |= gpio_direction_output(bst_ctrl, enable);
		} else {
#ifdef CONFIG_USB_AUDIO_POWER
			ret |= bst_ctrl_enable(enable, VBOOST_CONTROL_PM);
#endif /* CONFIG_USB_AUDIO_POWER */
		}
	}

	return ret;
}

static int scp_power_enable_by_5vboost(int enable)
{
	int ret = 0;

	hwlog_info("by_5vboost=%d,%d,%d, enable=%d\n",
		scp_ps_by_5vboost, is_need_bst_ctrl, bst_ctrl_use_common_gpio,
		enable);

	if (scp_ps_by_5vboost) {
#ifdef CONFIG_BOOST_5V
		ret |= boost_5v_enable(enable, BOOST_CTRL_DC);
#endif /* CONFIG_BOOST_5V */

		ret |= direct_charge_set_bst_ctrl(enable);
	}

	return ret;
}

static int scp_power_enable_by_charger(int enable)
{
	int ret = 0;

	hwlog_info("by_charger=%d, enable=%d\n", scp_ps_by_charger, enable);

	if (scp_ps_by_charger) {
#ifdef CONFIG_HUAWEI_CHARGER_AP
		ret |= charge_otg_mode_enable(enable, OTG_CTRL_DC);
#endif /* CONFIG_HUAWEI_CHARGER_AP */
	}

	return ret;
}

static int scp_power_enable_by_dummy(int enable)
{
	hwlog_info("by_dummy=dummy, enable=%d\n", enable);

	return 0;
}

static struct scp_power_supply_ops scp_ps_dummy_ops = {
	.scp_power_enable = scp_power_enable_by_dummy,
};

static struct scp_power_supply_ops scp_ps_5vboost_ops = {
	.scp_power_enable = scp_power_enable_by_5vboost,
};

static struct scp_power_supply_ops scp_ps_charger_ops = {
	.scp_power_enable = scp_power_enable_by_charger,
};

int direct_charge_ps_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *np = (&pdev->dev)->of_node;

	hwlog_info("probe begin\n");

	if (of_property_read_u32(np, "boost_5v_support_scp_power",
		&scp_ps_by_5vboost)) {
		hwlog_err("boost_5v_support_scp_power dts read failed\n");
		scp_ps_by_5vboost = 0;
	}
	hwlog_info("scp_ps_by_5vboost=%d\n", scp_ps_by_5vboost);

	if (of_property_read_u32(np, "huawei_charger_support_scp_power",
		&scp_ps_by_charger)) {
		hwlog_err("huawei_charger_support_scp_power dts read failed\n");
		scp_ps_by_charger = 0;
	}
	hwlog_info("scp_ps_by_charger=%d\n", scp_ps_by_charger);

	if (of_property_read_u32(np, "is_need_bst_ctrl", &is_need_bst_ctrl)) {
		hwlog_err("is_need_bst_ctrl dts read failed\n");
		is_need_bst_ctrl = 0;
	}
	hwlog_info("is_need_bst_ctrl=%d\n", is_need_bst_ctrl);

	if (of_property_read_u32(np, "bst_ctrl_use_common_gpio",
		&bst_ctrl_use_common_gpio)) {
		hwlog_err("bst_ctrl_use_common_gpio dts read failed\n");
		bst_ctrl_use_common_gpio = 0;
	}
	hwlog_info("bst_ctrl_use_common_gpio=%d\n", bst_ctrl_use_common_gpio);

	if ((is_need_bst_ctrl) && (!bst_ctrl_use_common_gpio)) {
		bst_ctrl = of_get_named_gpio(np, "bst_ctrl", 0);
		hwlog_info("bst_ctrl=%d\n", bst_ctrl);

		if (!gpio_is_valid(bst_ctrl)) {
			hwlog_err("gpio(bst_ctrl) is not valid\n");
			return -1;
		}

		ret = gpio_request(bst_ctrl, "bst_ctrl");
		if (ret) {
			hwlog_err("gpio(bst_ctrl) request fail\n");
			gpio_free(bst_ctrl);
			return -1;
		}
	}

	/* regisger dummy power supply ops */
	scp_power_supply_ops_register(&scp_ps_dummy_ops);

	/* regisger 5vboost power supply ops */
	if (scp_ps_by_5vboost) {
		ret = scp_power_supply_ops_register(&scp_ps_5vboost_ops);
		if (ret) {
			hwlog_err("(5vboost)scp power supply ops register fail\n");
			goto fail_register_ops;
		} else {
			hwlog_info("(5vboost)scp power supply ops register ok\n");
		}
	}

	/* regisger charger power supply ops */
	if (scp_ps_by_charger) {
		ret = scp_power_supply_ops_register(&scp_ps_charger_ops);
		if (ret) {
			hwlog_err("(charger)scp power supply ops register fail\n");
			goto fail_register_ops;
		} else {
			hwlog_info("(charger)scp power supply ops register ok\n");
		}
	}

	hwlog_info("probe end\n");
	return 0;

fail_register_ops:
	if ((is_need_bst_ctrl) && (!bst_ctrl_use_common_gpio))
		gpio_free(bst_ctrl);

	return ret;
}

static int direct_charge_ps_remove(struct platform_device *pdev)
{
	hwlog_info("remove begin\n");

	if ((is_need_bst_ctrl) && (!bst_ctrl_use_common_gpio))
		gpio_free(bst_ctrl);

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id direct_charge_ps_match_table[] = {
	{
		.compatible = "huawei,direct_charge_ps",
		.data = NULL,
	},
	{},
};

static struct platform_driver direct_charge_ps_driver = {
	.probe = direct_charge_ps_probe,
	.remove = direct_charge_ps_remove,
	.driver = {
		.name = "huawei,direct_charge_ps",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(direct_charge_ps_match_table),
	},
};

static int __init direct_charge_ps_init(void)
{
	return platform_driver_register(&direct_charge_ps_driver);
}

static void __exit direct_charge_ps_exit(void)
{
	platform_driver_unregister(&direct_charge_ps_driver);
}

device_initcall_sync(direct_charge_ps_init);
module_exit(direct_charge_ps_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("direct charge power supply module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
