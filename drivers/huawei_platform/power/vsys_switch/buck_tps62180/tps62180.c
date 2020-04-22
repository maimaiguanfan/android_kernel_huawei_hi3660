/*
 * tps62180.c
 *
 * driver of vsys buck tps62180
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
#include <huawei_platform/power/vsys_switch/vsys_switch.h>

#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif

#ifdef CONFIG_HUAWEI_POWER_DEBUG
#include <huawei_platform/power/power_debug.h>
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG tps62180
HWLOG_REGIST();

/* tps62180 vout */
#define TPS62180_VOUT_3500                 (3500)
#define TPS62180_VOUT_4300                 (4300)
/* tps62180 state */
#define TPS62180_CHIP_DISABLE              (0)
#define TPS62180_CHIP_ENABLE               (1)

struct tps62180_device_info {
	struct device *dev;
	int gpio_en;
	int gpio_vout_ctrl;
	int state;
	int vout;
};

static struct tps62180_device_info *g_tps62180_di;

static int tps62180_get_state(void)
{
	struct tps62180_device_info *di = g_tps62180_di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	return di->state;
}

static int tps62180_get_vout(void)
{
	struct tps62180_device_info *di = g_tps62180_di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (di->vout == BUCK_VOUT_LOW)
		return TPS62180_VOUT_3500;

	if (di->vout == BUCK_VOUT_HIGH)
		return TPS62180_VOUT_4300;

	return 0;
}

static int tps62180_set_state(int enable)
{
	int gpio_val;
	struct tps62180_device_info *di = g_tps62180_di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (!di->gpio_en) {
		hwlog_err("no device\n");
		return -ENODEV;
	}

	enable = !!enable;

	if (di->state == enable) {
		hwlog_info("set_state state is already %d\n", enable);
		return 0;
	}

	gpio_val = !enable; /* gpio_val = 0:enable gpio_val = 1: disable */
	gpio_set_value(di->gpio_en, gpio_val);
	di->state = enable;
	hwlog_info("set_state set gpio_en %d\n", gpio_val);

	return 0;
}

static int tps62180_set_vout(int vout)
{
	int gpio_val;
	struct tps62180_device_info *di = g_tps62180_di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (!di->gpio_vout_ctrl) {
		hwlog_err("no device\n");
		return -ENODEV;
	}

	vout = !!vout;

	if (di->vout == vout) {
		hwlog_info("set_vout already %d\n", vout);
		return 0;
	}

	gpio_val = vout; /* 0: vout=3500 1:vout=4300 */
	gpio_set_value(di->gpio_vout_ctrl, gpio_val);
	di->vout = vout;
	hwlog_info("set_vout set gpio_vout_ctrl %d\n", gpio_val);

	return 0;
}

static int tps62180_gpio_init(struct device_node *np)
{
	int ret = 0;
	struct tps62180_device_info *di = g_tps62180_di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	di->gpio_en = of_get_named_gpio(np, "gpio_en", 0);
	hwlog_info("gpio_en=%d\n", di->gpio_en);

	if (!gpio_is_valid(di->gpio_en)) {
		hwlog_err("gpio(gpio_en) is not valid\n");
		ret = -EINVAL;
		goto gpio_init_fail_0;
	}

	ret = gpio_request(di->gpio_en, "gpio_en");
	if (ret) {
		hwlog_err("gpio(gpio_en) request fail\n");
		goto gpio_init_fail_0;
	}

	/* 0:enable 1:disable */
	ret = gpio_direction_output(di->gpio_en, 0);
	if (ret) {
		hwlog_err("gpio(gpio_en) set output fail\n");
		goto gpio_init_fail_1;
	}

	di->gpio_vout_ctrl = of_get_named_gpio(np, "gpio_vout_ctrl", 0);
	hwlog_info("gpio_vout_ctrl=%d\n", di->gpio_vout_ctrl);

	if (!gpio_is_valid(di->gpio_vout_ctrl)) {
		hwlog_err("gpio(gpio_vout_ctrl) is not valid\n");
		ret = -EINVAL;
		goto gpio_init_fail_1;
	}

	ret = gpio_request(di->gpio_vout_ctrl, "gpio_vout_ctrl");
	if (ret) {
		hwlog_err("gpio(gpio_vout_ctrl) request fail\n");
		goto gpio_init_fail_1;
	}

	/* 0:vout=3500 1:vout=4300 */
	ret = gpio_direction_output(di->gpio_vout_ctrl, 1);
	if (ret) {
		hwlog_err("gpio(gpio_vout_ctrl) set output fail\n");
		goto gpio_init_fail_2;
	}
	return 0;

gpio_init_fail_2:
	gpio_free(di->gpio_vout_ctrl);
gpio_init_fail_1:
	gpio_free(di->gpio_en);
gpio_init_fail_0:
	return ret;
}

static void tps62180_para_init(struct tps62180_device_info *di)
{
	di->state = TPS62180_CHIP_ENABLE;
	di->vout = BUCK_VOUT_HIGH;
}

static struct vsys_buck_device_ops tps62180_ops = {
	.chip_name = "tps62180",

	.get_state = tps62180_get_state,
	.set_state = tps62180_set_state,
	.get_vout = tps62180_get_vout,
	.set_vout = tps62180_set_vout,
};

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static ssize_t tps62180_dbg_show_buck_vout(void *dev_data,
	char *buf, size_t size)
{
	struct tps62180_device_info *dev_p;
	int vout = 0;

	dev_p = (struct tps62180_device_info *)dev_data;
	if (dev_p == NULL) {
		hwlog_err("dev_p is null\n");
		return scnprintf(buf, size, "dev_p is null\n");
	}

	vout = tps62180_get_vout();

	return scnprintf(buf, size, "buck vout = %d\n", vout);
}

static ssize_t tps62180_dbg_store_buck_vout(void *dev_data,
	const char *buf, size_t size)
{
	struct tps62180_device_info *dev_p;
	int vout = 0;
	int ret = 0;

	dev_p = (struct tps62180_device_info *)dev_data;
	if (dev_p == NULL) {
		hwlog_err("dev_p is null\n");
		return -EINVAL;
	}

	ret = kstrtoint(buf, 0, &vout);
	if (ret < 0) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	if (vout == BUCK_VOUT_LOW || vout == BUCK_VOUT_HIGH) {
		ret = tps62180_set_vout(vout);
		if (ret) {
			hwlog_err("dbg set vout %d failed\n", vout);
			return -EINVAL;
		}
	} else {
		hwlog_info("dbg vout to be set is wrong value\n");
		return -EINVAL;
	}

	hwlog_info("dbg set vout %d\n", vout);

	return size;
}

static ssize_t tps62180_dbg_show_buck_state(void *dev_data,
	char *buf, size_t size)
{
	struct tps62180_device_info *dev_p;
	int state = 0;

	dev_p = (struct tps62180_device_info *)dev_data;
	if (dev_p == NULL) {
		hwlog_err("dev_p is null\n");
		return scnprintf(buf, size, "dev_p is null\n");
	}

	state = tps62180_get_state();

	return scnprintf(buf, size, "buck state = %d\n", state);
}

static ssize_t tps62180_dbg_store_buck_state(void *dev_data,
	const char *buf, size_t size)
{
	struct tps62180_device_info *dev_p;
	int state = 0;
	int ret = 0;

	dev_p = (struct tps62180_device_info *)dev_data;
	if (dev_p == NULL) {
		hwlog_err("dev_p is null\n");
		return -EINVAL;
	}

	ret = kstrtoint(buf, 0, &state);
	if (ret < 0) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	if (state == TPS62180_CHIP_ENABLE || state == TPS62180_CHIP_DISABLE) {
		ret = tps62180_set_state(state);
		if (ret) {
			hwlog_err("dbg set state %d failed\n", state);
			return -EINVAL;
		}
	} else {
		hwlog_info("dbg state to be set is wrong value\n");
		return -EINVAL;
	}

	hwlog_info("dbg set buck state %d\n", state);

	return size;
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static int tps62180_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct tps62180_device_info *di = NULL;
	struct device_node *np = (&pdev->dev)->of_node;

	hwlog_info("probe begin\n");

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (di == NULL)
		return -ENOMEM;

	g_tps62180_di = di;
	di->dev = &pdev->dev;
	np = di->dev->of_node;
	platform_set_drvdata(pdev, di);

	ret = tps62180_gpio_init(np);
	if (ret)
		goto tps62180_fail_0;

	tps62180_para_init(di);

	ret = vsys_buck_ops_register(&tps62180_ops);
	if (ret) {
		hwlog_err("register tps62180 ops failed\n");
		goto tps62180_fail_1;
	}

#ifdef CONFIG_HUAWEI_POWER_DEBUG
	power_dbg_ops_register("tps62180_vout", platform_get_drvdata(pdev),
		(power_dgb_show)tps62180_dbg_show_buck_vout,
		(power_dgb_store)tps62180_dbg_store_buck_vout);
	power_dbg_ops_register("tps62180_state", platform_get_drvdata(pdev),
		(power_dgb_show)tps62180_dbg_show_buck_state,
		(power_dgb_store)tps62180_dbg_store_buck_state);
#endif

	hwlog_info("probe end\n");
	return 0;

tps62180_fail_1:
	gpio_free(di->gpio_en);
	gpio_free(di->gpio_vout_ctrl);
tps62180_fail_0:
	devm_kfree(&pdev->dev, di);
	g_tps62180_di = NULL;
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static const struct of_device_id tps62180_match_table[] = {
	{
		.compatible = "huawei,tps62180",
		.data = NULL,
	},
	{},
};

static struct platform_driver tps62180_driver = {
	.probe = tps62180_probe,
	.driver = {
		.name = "huawei,tps62180",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(tps62180_match_table),
	},
};

static int __init tps62180_init(void)
{
	return platform_driver_register(&tps62180_driver);
}

static void __exit tps62180_exit(void)
{
	platform_driver_unregister(&tps62180_driver);
}

module_init(tps62180_init);
module_exit(tps62180_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("tps62180 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
