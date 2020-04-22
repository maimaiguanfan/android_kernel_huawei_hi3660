/*
 * boost_5v.c
 *
 * boost with 5v driver
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
#include <huawei_platform/power/boost_5v.h>
#include <media/huawei/camera/pmic/hw_pmic.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG boost_5v
HWLOG_REGIST();

static int gpio_5v_boost;
static unsigned int boost_status;
static DEFINE_MUTEX(boost_op_mutex);
static int boost_initialized;
static int boost_5v_use_common_pmic;

static const char * const boost_ctrl_source_table[] = {
	[BOOST_CTRL_BOOST_GPIO_OTG] = "BOOST_GPIO_OTG",
	[BOOST_CTRL_PD_VCONN]       = "PD",
	[BOOST_CTRL_DC]             = "DC",
	[BOOST_CTRL_MOTOER]         = "MOTOR",
	[BOOST_CTRL_AUDIO]          = "AUDIO",
	[BOOST_CTRL_AT_CMD]         = "AT CMD",
	[BOOST_CTRL_FCP]            = "FCP",
	[BOOST_CTRL_WLDC]           = "WLDC",
	[BOOST_CTRL_WLTX]           = "WLTX",
	[BOOST_CTRL_WLC]            = "WIRELESS_CHARGE"
};

static const char *boost_5v_get_ctrl_source(enum boost_ctrl_source_type type)
{
	if (type < BOOST_CTRL_BEGIN || type >= BOOST_CTRL_END)
		return "illegal type";

	return boost_ctrl_source_table[type];
}

static int boost_5v_output(int value)
{
	int ret;

	if (boost_5v_use_common_pmic) {
		ret = pmic_enable_boost(value);
		if (ret) {
			hwlog_err("pmic enable boost fail(ret=%d)\n", ret);
			return -1;
		}
	} else {
		gpio_set_value(gpio_5v_boost, value);
	}

	return 0;
}

static int boost_set_enable(enum boost_ctrl_source_type type)
{
	if (boost_status == 0)
		boost_5v_output(BOOST_5V_ENABLE);

	boost_status = boost_status | (1 << type);

	hwlog_info("boost_5v enable(%s) success\n",
		boost_5v_get_ctrl_source(type));
	return 0;
}

static int boost_set_disable(enum boost_ctrl_source_type type)
{
	if (boost_status != 0) {
		boost_status = boost_status & (~(1 << type));

		if (boost_status == 0)
			boost_5v_output(BOOST_5V_DISABLE);
	}

	hwlog_info("boost_5v disable(%s) success\n",
		boost_5v_get_ctrl_source(type));
	return 0;
}

int boost_5v_enable(bool enable, enum boost_ctrl_source_type type)
{
	hwlog_info("the module %s set boost 5v to(%d)\n",
		boost_5v_get_ctrl_source(type), enable);

	if (!(boost_initialized || boost_5v_use_common_pmic)) {
		hwlog_err("boost 5v not initialized\n");
		return -EINVAL;
	}

	if (type >= BOOST_CTRL_END) {
		hwlog_err("invalid type(%d)\n", type);
		return -EINVAL;
	}

	mutex_lock(&boost_op_mutex);

	if (enable)
		boost_set_enable(type);
	else
		boost_set_disable(type);

	mutex_unlock(&boost_op_mutex);

	return 0;
}

unsigned int boost_5v_status(void)
{
	return boost_status;
}

static int boost_5v_gpio_init(struct device_node *np)
{
	int ret;

	gpio_5v_boost = of_get_named_gpio(np, "gpio_5v_boost", 0);
	hwlog_info("gpio_5v_boost=%d\n", gpio_5v_boost);

	if (!gpio_is_valid(gpio_5v_boost)) {
		hwlog_err("gpio(gpio_5v_boost) is not valid\n");
		return -1;
	}

	ret = gpio_request(gpio_5v_boost, "gpio_5v_boost");
	if (ret) {
		hwlog_err("gpio(gpio_5v_boost) request fail\n");
		return -1;
	}

	ret = gpio_direction_output(gpio_5v_boost, 0);
	if (ret) {
		gpio_free(gpio_5v_boost);
		hwlog_err("gpio(gpio_5v_boost) set output fail\n");
		return -1;
	}

	boost_initialized = 1;
	return 0;
}

#ifdef CONFIG_SYSFS
static ssize_t boost_5v_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%u\n", boost_status);
}

static ssize_t boost_5v_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;

	if (!strstr(saved_command_line, "androidboot.swtype=factory")) {
		hwlog_info("only factory version can ctrl boost 5v\n");
		return count;
	}

	/*
	 * 10: decimal
	 * 0: disable; 1: enable; others: invaid
	 */
	if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1)) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	if (val) {
		boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_AT_CMD);
	} else {
		mutex_lock(&boost_op_mutex);
		boost_5v_output(BOOST_5V_DISABLE);
		boost_status = 0;
		mutex_unlock(&boost_op_mutex);
	}

	hwlog_info("ctrl boost 5v by sys class\n");
	return count;
}

static DEVICE_ATTR(boost_5v_enable, 0644,
	boost_5v_sysfs_show, boost_5v_sysfs_store);

static struct attribute *boost_5v_attributes[] = {
	&dev_attr_boost_5v_enable.attr,
	NULL,
};

static const struct attribute_group boost_5v_attr_group = {
	.attrs = boost_5v_attributes,
};

static int boost_5v_sysfs_create_group(struct platform_device *pdev)
{
	return sysfs_create_group(&pdev->dev.kobj, &boost_5v_attr_group);
}

static void boost_5v_sysfs_remove_group(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &boost_5v_attr_group);
}

#else

static inline int boost_5v_sysfs_create_group(struct platform_device *pdev)
{
	return 0;
}

static inline void boost_5v_sysfs_remove_group(struct platform_device *pdev)
{
}
#endif /* CONFIG_SYSFS */

static int boost_5v_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *np = NULL;

	hwlog_info("probe begin\n");

	if (!pdev || !(&pdev->dev) || !(&pdev->dev)->of_node) {
		hwlog_err("device_node is null\n");
		return -1;
	}
	np = (&pdev->dev)->of_node;

	if (of_property_read_u32(np, "boost_5v_use_common_pmic",
		&boost_5v_use_common_pmic)) {
		hwlog_err("boost_5v_use_common_pmic dts read failed\n");
		boost_5v_use_common_pmic = 0;
	}
	hwlog_info("boost_5v_use_common_pmic=%d\n", boost_5v_use_common_pmic);

	if (!boost_5v_use_common_pmic) {
		ret = boost_5v_gpio_init(np);
		if (ret)
			return -1;
	}

	ret = boost_5v_sysfs_create_group(pdev);
	if (ret) {
		hwlog_err("sysfs group create failed\n");
		return -1;
	}

	hwlog_info("probe end\n");
	return 0;
}

static int boost_5v_remove(struct platform_device *pdev)
{
	hwlog_info("remove begin\n");

	if (!gpio_is_valid(gpio_5v_boost))
		gpio_free(gpio_5v_boost);

	mutex_destroy(&boost_op_mutex);
	boost_5v_sysfs_remove_group(pdev);

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id boost_5v_match_table[] = {
	{
		.compatible = "huawei,boost_5v",
		.data = NULL,
	},
	{},
};

static struct platform_driver boost_5v_driver = {
	.probe = boost_5v_probe,
	.remove = boost_5v_remove,
	.driver = {
		.name = "huawei,boost_5v",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(boost_5v_match_table),
	},
};

static int __init boost_5v_init(void)
{
	return platform_driver_register(&boost_5v_driver);
}

static void __exit boost_5v_exit(void)
{
	platform_driver_unregister(&boost_5v_driver);
}

fs_initcall(boost_5v_init);
module_exit(boost_5v_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("boost with 5v driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
