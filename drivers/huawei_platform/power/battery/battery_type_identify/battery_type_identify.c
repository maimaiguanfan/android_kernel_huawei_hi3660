/*
 * battery_type_identify.c
 *
 * Interfaces for switch mode between adc and
 * onewire communication to Identify battery type.
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/device.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/battery_type_identify.h>
#ifdef BATTERY_IDENTIFY_DEBUG
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#include <huawei_platform/power/batt_info_pub.h>
#endif /* BATTERY_IDENTIFY_DEBUG */

#define HWLOG_TAG batt_type_identify
HWLOG_REGIST();

#define GPIO_OUT     0
#define GPIO_IN      1

#define ID_SN_SIZE 6
#define TEST_TIMES 10

struct gpio_state {
	int direction;
	int value;
};

struct mode_channel_cfg {
	struct mutex lock;
	int gpio;
	struct gpio_state id_voltage;
	struct gpio_state id_sn;
	const struct security_ic_ops *ops;
	enum batt_type_identify_mode cur_mode;
};
static struct mode_channel_cfg *g_mode_cfgs;

void register_security_ic_ops(const struct security_ic_ops *ops)
{
	if (g_mode_cfgs)
		g_mode_cfgs->ops = ops;
}

void unregister_security_ic_ops(const struct security_ic_ops *ops)
{
	if (!g_mode_cfgs)
		return;

	if (g_mode_cfgs->ops == ops)
		g_mode_cfgs->ops = NULL;
}

static int set_gpio(int gpio, int direction, int value)
{
	if (direction == GPIO_IN)
		return gpio_direction_input(gpio);

	return gpio_direction_output(gpio, value);
}

void apply_batt_type_mode(enum batt_type_identify_mode mode)
{
	int ret = -1;

	if (!g_mode_cfgs)
		return;

	mutex_lock(&g_mode_cfgs->lock);
	if (g_mode_cfgs->cur_mode == mode)
		return;

	switch (mode) {
	case BAT_ID_VOLTAGE:
		/* close mos */
		ret = set_gpio(g_mode_cfgs->gpio,
			g_mode_cfgs->id_voltage.direction,
			g_mode_cfgs->id_voltage.value);

		if (g_mode_cfgs->ops &&
			g_mode_cfgs->ops->close_ic)
			g_mode_cfgs->ops->close_ic();
		break;

	case BAT_ID_SN:
		/* open mos */
		ret = set_gpio(g_mode_cfgs->gpio,
			g_mode_cfgs->id_sn.direction,
			g_mode_cfgs->id_sn.value);

		if (g_mode_cfgs->ops &&
			g_mode_cfgs->ops->open_ic)
			g_mode_cfgs->ops->open_ic();
		break;

	default:
		break;
	}

	if (ret) {
		hwlog_err("switch mode to %d error\n", mode);
		return;
	}
	g_mode_cfgs->cur_mode = mode;
}

void release_batt_type_mode(void)
{
	if (g_mode_cfgs)
		mutex_unlock(&g_mode_cfgs->lock);
}

#ifdef BATTERY_IDENTIFY_DEBUG
static ssize_t identify_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!g_mode_cfgs)
		return snprintf(buf, PAGE_SIZE, "driver init error");

	return snprintf(buf, PAGE_SIZE, "current mode is %d",
		g_mode_cfgs->cur_mode);
}

static ssize_t id_voltage_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int voltage;

	if (!g_mode_cfgs)
		return snprintf(buf, PAGE_SIZE, "driver init error");

	voltage = hisi_battery_id_voltage();
	return snprintf(buf, PAGE_SIZE, "read id voltage is %d",
		voltage);
}

static ssize_t id_sn_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned char id_sn[ID_SN_SIZE] = { 0 };

	if (!g_mode_cfgs)
		return snprintf(buf, PAGE_SIZE, "driver init error");

	if (get_battery_type(id_sn))
		return snprintf(buf, PAGE_SIZE, "get id sn error");

	return snprintf(buf, PAGE_SIZE, "read battery type is %s", id_sn);
}

static ssize_t id_sn_voltage_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;
	int len = 0;
	int voltage;
	unsigned char id_sn[ID_SN_SIZE] = { 0 };

	if (!g_mode_cfgs)
		return snprintf(buf, PAGE_SIZE, "driver init error");

	for (i = 0; i < TEST_TIMES; i++) {
		len += snprintf(buf + len, PAGE_SIZE, "%d--", i);
		if (get_battery_type(id_sn)) {
			len += snprintf(buf + len, PAGE_SIZE, "read error.");
			return len;
		}

		len += snprintf(buf + len, PAGE_SIZE, "%s--", id_sn);
		voltage = hisi_battery_id_voltage();
		len += snprintf(buf + len, PAGE_SIZE, "%d++", voltage);
	}

	len += snprintf(buf + len, PAGE_SIZE, "read fine.");
	return len;
}

static ssize_t open_ic_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (g_mode_cfgs &&
		g_mode_cfgs->ops &&
		g_mode_cfgs->ops->open_ic) {
		g_mode_cfgs->ops->open_ic();
		return snprintf(buf, PAGE_SIZE, "open ic pass");
	}

	return snprintf(buf, PAGE_SIZE, "driver init error");
}

static ssize_t close_ic_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (g_mode_cfgs &&
		g_mode_cfgs->ops &&
		g_mode_cfgs->ops->close_ic) {
		g_mode_cfgs->ops->close_ic();
		return snprintf(buf, PAGE_SIZE, "close ic pass");
	}

	return snprintf(buf, PAGE_SIZE, "driver init error");
}

static ssize_t open_mos_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	if (g_mode_cfgs) {
		ret = gpio_direction_output(g_mode_cfgs->gpio, 0);
		return snprintf(buf, PAGE_SIZE, "open mos ret %d", ret);
	}

	return snprintf(buf, PAGE_SIZE, "driver init error");
}

static ssize_t close_mos_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	if (g_mode_cfgs) {
		ret = gpio_direction_output(g_mode_cfgs->gpio, 1);
		return snprintf(buf, PAGE_SIZE, "close mos ret %d", ret);
	}

	return snprintf(buf, PAGE_SIZE, "driver init error");
}

static const DEVICE_ATTR_RO(identify_mode);
static const DEVICE_ATTR_RO(id_voltage);
static const DEVICE_ATTR_RO(id_sn);
static const DEVICE_ATTR_RO(id_sn_voltage);
static const DEVICE_ATTR_RO(open_ic);
static const DEVICE_ATTR_RO(close_ic);
static const DEVICE_ATTR_RO(open_mos);
static const DEVICE_ATTR_RO(close_mos);

static const struct attribute *g_debug_attrs[] = {
	&dev_attr_identify_mode.attr,
	&dev_attr_id_voltage.attr,
	&dev_attr_id_sn.attr,
	&dev_attr_id_sn_voltage.attr,
	&dev_attr_open_ic.attr,
	&dev_attr_close_ic.attr,
	&dev_attr_open_mos.attr,
	&dev_attr_close_mos.attr,
	NULL,
};

static void create_debug_node(struct platform_device *pdev)
{
	if (sysfs_create_files(&pdev->dev.kobj, g_debug_attrs))
		hwlog_err("create debug device node Error\n");
}
#endif /* BATTERY_IDENTIFY_DEBUG */

static int parse_dts_gpios(struct device_node *np)
{
	int gpio;
	int ret;

	gpio = of_get_named_gpio(np, "gpios", 0);
	if (gpio < 0) {
		hwlog_err("gpios dts read failed\n");
		return -1;
	}

	if (!gpio_is_valid(gpio)) {
		hwlog_err("gpio(%d) is not valid\n", gpio);
		return -1;
	}

	ret = gpio_request(gpio, "battery_type_gpio");
	if (ret) {
		hwlog_err("gpio(%d) request fail\n", gpio);
		return -1;
	}
	hwlog_info("gpio(%d) request success\n", gpio);

	return gpio;
}

static int parse_dts_channel(struct device_node *np,
	const char *property, struct gpio_state *states)
{
	if (of_property_read_u32_index(np, property, 0, &states->direction)) {
		hwlog_err("parse %s-first property in dts fail\n", property);
		return -1;
	}

	if (of_property_read_u32_index(np, property, 1, &states->value)) {
		hwlog_err("parse %s-second property in dts fail\n", property);
		return -1;
	}

	return 0;
}

static int parse_dts(struct device_node *np, struct mode_channel_cfg *cfgs)
{
	cfgs->gpio = parse_dts_gpios(np);
	if (cfgs->gpio < 0)
		return -1;

	if (parse_dts_channel(np, "id_voltage_gpiov", &cfgs->id_voltage) ||
	    parse_dts_channel(np, "id_sn_gpiov", &cfgs->id_sn))
		return -1;

	return 0;
}

static int batt_type_identify_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct mode_channel_cfg *cfgs = NULL;

	hwlog_info("probe start\n");
	if (!pdev || !pdev->dev.of_node) {
		hwlog_err("device_node is null\n");
		return -1;
	}

	cfgs = devm_kzalloc(&pdev->dev, sizeof(*cfgs),
		GFP_KERNEL);
	if (!cfgs)
		return -1;

	np = pdev->dev.of_node;
	cfgs->cur_mode = BAT_INVALID_MODE;
	if (parse_dts(np, cfgs))
		goto free_mem;

	mutex_init(&cfgs->lock);
	g_mode_cfgs = cfgs;

#ifdef BATTERY_IDENTIFY_DEBUG
	create_debug_node(pdev);
#endif /* BATTERY_IDENTIFY_DEBUG */
	hwlog_info("probe end\n");
	return 0;

free_mem:
	devm_kfree(&pdev->dev, cfgs);
	return -1;
}

static int batt_type_identify_remove(struct platform_device *pdev)
{
	if (g_mode_cfgs) {
		mutex_destroy(&g_mode_cfgs->lock);
		gpio_free(g_mode_cfgs->gpio);
	}

	return 0;
}

static const struct of_device_id batt_type_match_table[] = {
	{
		.compatible = "huawei,battery-identify",
		.data = NULL,
	},
	{},
};

static struct platform_driver batt_type_identify_driver = {
	.probe = batt_type_identify_probe,
	.remove = batt_type_identify_remove,
	.driver = {
		.name = "battery-type-identify",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(batt_type_match_table),
	},
};

static int __init batt_type_identify_init(void)
{
	return platform_driver_register(&batt_type_identify_driver);
}

static void __exit batt_type_identify_exit(void)
{
	platform_driver_unregister(&batt_type_identify_driver);
}

subsys_initcall(batt_type_identify_init);
module_exit(batt_type_identify_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei battery type identify driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
