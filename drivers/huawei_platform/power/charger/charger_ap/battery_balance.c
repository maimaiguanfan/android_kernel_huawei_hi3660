/*
 * battery_balance.c
 *
 * battery balance driver
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
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/raid/pq.h>
#include <linux/kernel.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>

#define HWLOG_TAG battery_balance
HWLOG_REGIST();

#define GPIO_ENABLE   1
#define GPIO_DISABLE  0

enum balance_batt_id {
	BATT_ID_0 = 0,
	BATT_ID_1 = 1,
};

enum BATTERY_BALANCE_SYSFS_TYPE {
	BATTERY_BALANCE_SYSFS_BALANCE_CHANNEL_0,
	BATTERY_BALANCE_SYSFS_BALANCE_CHANNEL_1,
};

struct battery_balance_info {
	struct device *dev;
	int gpio_bat0_balance_en;
	int gpio_bat1_balance_en;
	int sysfs_bat0_balance_en;
	int sysfs_bat1_balance_en;
};

static struct battery_balance_info *g_balance_di;

static int set_gpio_value_batt(enum balance_batt_id balance_batt_id, int value)
{
	int gpio_num;
	int val;

	if (!g_balance_di) {
		hwlog_err("g_balance_di is null\n");
		return -1;
	}

	if (balance_batt_id == BATT_ID_0) {
		gpio_num = g_balance_di->gpio_bat0_balance_en;
	} else if (balance_batt_id == BATT_ID_1) {
		gpio_num = g_balance_di->gpio_bat1_balance_en;
	} else {
		hwlog_err("balance_batt_id is error [%d]\n", balance_batt_id);
		return -1;
	}

	val = value ? GPIO_ENABLE : GPIO_DISABLE;
	gpio_set_value(gpio_num, val);
	return 0;
}

#ifdef CONFIG_SYSFS

#define BATTERY_BALANCE_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, balance_sysfs_show, store), \
	.name = BATTERY_BALANCE_SYSFS_##n, \
}

#define BATTERY_BALANCE_SYSFS_FIELD_RW(_name, n) \
	BATTERY_BALANCE_SYSFS_FIELD(_name, n, 0644, balance_sysfs_store)

struct balance_sysfs_info {
	struct device_attribute attr;
	u8 name;
};

static ssize_t balance_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static ssize_t balance_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct balance_sysfs_info balance_sysfs_tbl[] = {
	BATTERY_BALANCE_SYSFS_FIELD_RW(balance_channel_0, BALANCE_CHANNEL_0),
	BATTERY_BALANCE_SYSFS_FIELD_RW(balance_channel_1, BALANCE_CHANNEL_1),
};

static struct attribute *balance_sysfs_attrs[ARRAY_SIZE(balance_sysfs_tbl) + 1];

static const struct attribute_group balance_sysfs_attr_group = {
	.attrs = balance_sysfs_attrs,
};

static void balance_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(balance_sysfs_tbl);

	for (i = 0; i < limit; i++)
		balance_sysfs_attrs[i] = &balance_sysfs_tbl[i].attr.attr;

	balance_sysfs_attrs[limit] = NULL;
}

static struct balance_sysfs_info *balance_sysfs_lookup(const char *name)
{
	int i;
	int limit = ARRAY_SIZE(balance_sysfs_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name, balance_sysfs_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= limit)
		return NULL;

	return &balance_sysfs_tbl[i];
}

static int balance_sysfs_create_group(struct battery_balance_info *di)
{
	balance_sysfs_init_attrs();
	return sysfs_create_group(&di->dev->kobj, &balance_sysfs_attr_group);
}

static void balance_sysfs_remove_group(struct battery_balance_info *di)
{
	sysfs_remove_group(&di->dev->kobj, &balance_sysfs_attr_group);
}

#else

static int balance_sysfs_create_group(struct battery_balance_info *di)
{
	return 0;
}

static void balance_sysfs_remove_group(struct battery_balance_info *di)
{
}

#endif /* CONFIG_SYSFS */

static ssize_t balance_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct balance_sysfs_info *info = NULL;
	struct battery_balance_info *di = dev_get_drvdata(dev);
	int len = 0;

	info = balance_sysfs_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case BATTERY_BALANCE_SYSFS_BALANCE_CHANNEL_0:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			di->sysfs_bat0_balance_en);
		break;

	case BATTERY_BALANCE_SYSFS_BALANCE_CHANNEL_1:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			di->sysfs_bat1_balance_en);
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		break;
	}

	return len;
}

static ssize_t balance_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct balance_sysfs_info *info = NULL;
	struct battery_balance_info *di = dev_get_drvdata(dev);
	long val;

	info = balance_sysfs_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case BATTERY_BALANCE_SYSFS_BALANCE_CHANNEL_0:
		if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
			return -EINVAL;

		hwlog_info("battery_balance_sysfs_store,val = [%ld]\n", val);
		di->sysfs_bat0_balance_en = val;
		set_gpio_value_batt(BATT_ID_0, val);
		break;

	case BATTERY_BALANCE_SYSFS_BALANCE_CHANNEL_1:
		if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
			return -EINVAL;

		hwlog_info("battery_balance_sysfs_store,val = [%ld]\n", val);
		di->sysfs_bat1_balance_en = val;
		set_gpio_value_batt(BATT_ID_1, val);
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		break;
	}

	return count;
}

static int battery_balance_gpio_init
	(struct battery_balance_info *di, struct device_node *np)
{
	int ret = 0;

	di->gpio_bat0_balance_en = of_get_named_gpio(np,
		"gpio_bat0_balance_en", 0);
	hwlog_info("gpio_bat0_balance_en=%d\n", di->gpio_bat0_balance_en);

	if (!gpio_is_valid(di->gpio_bat0_balance_en)) {
		hwlog_err("gpio(gpio_bat0_balance_en) is not valid\n");
		ret = -EINVAL;
		return ret;
	}

	ret = gpio_request(di->gpio_bat0_balance_en, "gpio_bat0_balance_en");
	if (ret) {
		hwlog_err("gpio(gpio_bat0_balance_en) request fail\n");
		return ret;
	}

	ret = gpio_direction_output(di->gpio_bat0_balance_en, 0);
	if (ret) {
		hwlog_err("gpio(gpio_bat0_balance_en) set output fail\n");
		return ret;
	}

	di->gpio_bat1_balance_en = of_get_named_gpio(np,
		"gpio_bat1_balance_en", 0);
	hwlog_info("gpio_bat1_balance_en=%d\n", di->gpio_bat1_balance_en);

	if (!gpio_is_valid(di->gpio_bat1_balance_en)) {
		hwlog_err("gpio(gpio_bat1_balance_en) is not valid\n");
		ret = -EINVAL;
		goto gpio_init_err;
	}

	ret = gpio_request(di->gpio_bat1_balance_en, "gpio_bat1_balance_en");
	if (ret) {
		hwlog_err("gpio(gpio_bat1_balance_en) request fail\n");
		goto gpio_init_err;
	}

	ret = gpio_direction_output(di->gpio_bat1_balance_en, 0);
	if (ret) {
		hwlog_err("gpio(gpio_bat1_balance_en) set output fail\n");
		goto gpio_init_err;
	}

	return 0;

gpio_init_err:
	gpio_free(di->gpio_bat0_balance_en);
	return ret;
}

static int battery_balance_probe(struct platform_device *pdev)
{
	struct battery_balance_info *di = NULL;
	struct device_node *np = NULL;
	struct class *power_class = NULL;
	int ret = 0;

	hwlog_info("probe begin\n");

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_balance_di = di;
	di->dev = &pdev->dev;

	np = di->dev->of_node;
	if (!np) {
		hwlog_err("device_node is null\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, di);

	ret = battery_balance_gpio_init(di, np);
	if (ret)
		goto battery_balance_fail_0;

	ret = balance_sysfs_create_group(di);
	if (ret) {
		hwlog_err("sysfs group create failed\n");
		goto battery_balance_fail_1;
	}

	power_class = hw_power_get_class();
	if (power_class) {
		if (!charge_dev)
			charge_dev = device_create(power_class, NULL, 0, NULL,
				"charger");

		ret = sysfs_create_link(&charge_dev->kobj, &di->dev->kobj,
			"battery_balance");
		if (ret) {
			hwlog_err("sysfs link create failed\n");
			goto battery_balance_fail_2;
		}

	}

	hwlog_info("probe end\n");
	return 0;

battery_balance_fail_2:
	balance_sysfs_remove_group(di);
battery_balance_fail_1:
	gpio_free(di->gpio_bat0_balance_en);
	gpio_free(di->gpio_bat1_balance_en);
battery_balance_fail_0:
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	di = NULL;
	g_balance_di = NULL;
	return ret;
}

static int battery_balance_remove(struct platform_device *pdev)
{
	struct battery_balance_info *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("di is null\n");
		return -ENODEV;
	}

	platform_set_drvdata(pdev, NULL);

	if (di->gpio_bat0_balance_en)
		gpio_free(di->gpio_bat0_balance_en);

	if (di->gpio_bat1_balance_en)
		gpio_free(di->gpio_bat1_balance_en);

	kfree(di);
	di = NULL;
	g_balance_di = NULL;
	return 0;
}

static const struct of_device_id battery_balance_match_table[] = {
	{
		.compatible = "huawei,battery_balance",
		.data = NULL,
	},
	{},
};

static struct platform_driver battery_balance_driver = {
	.probe = battery_balance_probe,
	.remove = battery_balance_remove,
	.driver = {
		.name = "huawei,battery_balance",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(battery_balance_match_table),
	},
};

int __init battery_balance_init(void)
{
	return platform_driver_register(&battery_balance_driver);
}

void __exit battery_balance_exit(void)
{
	platform_driver_unregister(&battery_balance_driver);
}

module_init(battery_balance_init);
module_exit(battery_balance_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("charger balance module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
