/*
 * battery_voltage.c
 *
 * battery voltage interface for power module
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

#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/battery_voltage.h>

#define HWLOG_TAG hw_batt_vol
HWLOG_REGIST();

static struct hw_batt_vol_info *g_hw_batt_di;

int hw_battery_get_series_num(void)
{
	if (!g_hw_batt_di)
		return HW_ONE_BAT;

	return g_hw_batt_di->batt_series_num;
}

int hw_battery_voltage(enum hw_batt_id batt_id)
{
	int i = 0;
	int max = -1;
	int min = MAX_VOL_MV;
	int vol = 0;

	if (!g_hw_batt_di)
		return hisi_battery_voltage();

	if (g_hw_batt_di->total_vol == HW_BATT_VOL_SINGLE_BATTERY) {
		return g_hw_batt_di->vol_buff[0].get_batt_vol();
	} else if (g_hw_batt_di->total_vol == 0) {
		hwlog_err("use default hisi_battery_voltage\n");
		return hisi_battery_voltage();
	}

	switch (batt_id) {
	/* fall through: diff voltage source by case BAT_ID_ALL */
	case BAT_ID_0:
	case BAT_ID_1:
	case BAT_ID_ALL:
		for (i = 0; i < g_hw_batt_di->total_vol; i++) {
			if (g_hw_batt_di->vol_buff[i].batt_id != batt_id)
				continue;

			if (g_hw_batt_di->vol_buff[i].get_batt_vol) {
				vol = g_hw_batt_di->vol_buff[i].get_batt_vol();
			} else {
				hwlog_err("vol_buff[%d].get_batt_vol is null\n",
					batt_id);
				return -1;
			}

			hwlog_info("get hw_batt_vol[%d]=%dmv\n", batt_id, vol);
			return vol;
		}
		break;

	case BAT_ID_MAX:
		for (i = 0; i < g_hw_batt_di->total_vol; i++) {
			if (g_hw_batt_di->vol_buff[i].batt_id == BAT_ID_ALL)
				continue;

			if (g_hw_batt_di->vol_buff[i].get_batt_vol) {
				vol = g_hw_batt_di->vol_buff[i].get_batt_vol();
			} else {
				hwlog_err("vol_buff[%d].get_batt_vol is null\n",
					batt_id);
				return -1;
			}

			if (vol < 0) {
				hwlog_err("get_batt_vol fail, batt_id=%d\n",
					batt_id);
				return -1;
			}

			if (vol > max)
				max = vol;
		}

		hwlog_info("get hw_batt_vol_max[%d]=%dmv\n", batt_id, max);
		return max;
		/* break; */

	case BAT_ID_MIN:
		for (i = 0; i < g_hw_batt_di->total_vol; i++) {
			if (g_hw_batt_di->vol_buff[i].batt_id == BAT_ID_ALL)
				continue;

			if (g_hw_batt_di->vol_buff[i].get_batt_vol) {
				vol = g_hw_batt_di->vol_buff[i].get_batt_vol();
			} else {
				hwlog_err("vol_buff[%d].get_batt_vol is null\n",
					batt_id);
				return -1;
			}

			if (vol < 0) {
				hwlog_err("get_batt_vol fail, batt_id=%d\n",
					batt_id);
				return -1;
			}

			if (vol < min)
				min = vol;
		}

		if (min < MAX_VOL_MV) {
			hwlog_info("get hw_batt_vol_min[%d]=%dmv\n",
				batt_id, min);
			return min;
		} else {
			return -1;
		}
		break;

	default:
		hwlog_err("invalid batt_id:%d\n", batt_id);
		break;
	}

	return -1;
}

int hw_battery_voltage_ops_register(struct hw_batt_vol_ops *ops,
	char *ops_name)
{
	int i;

	if (!g_hw_batt_di || !ops) {
		hwlog_err("g_hw_batt_di or ops is null\n");
		return -EPERM;
	}

	for (i = 0; i < g_hw_batt_di->total_vol; i++) {
		if (!strncmp(ops_name, g_hw_batt_di->vol_buff[i].ops_name,
			strlen(g_hw_batt_di->vol_buff[i].ops_name))) {
			g_hw_batt_di->vol_buff[i].get_batt_vol =
				ops->get_batt_vol;
			break;
		}
	}

	if (i >= g_hw_batt_di->total_vol) {
		hwlog_err("%s ops register fail\n", ops_name);
		return -EPERM;
	}

	hwlog_info("%s ops register ok\n", ops_name);
	return 0;
}

#ifdef CONFIG_SYSFS
#define HW_BATT_VOL_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, hw_batt_vol_sysfs_show, store), \
	.name = HW_BATT_VOL_SYSFS_##n, \
}

#define HW_BATT_VOL_SYSFS_FIELD_RO(_name, n) \
	HW_BATT_VOL_SYSFS_FIELD(_name, n, 0444, NULL)

struct hw_batt_vol_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static ssize_t hw_batt_vol_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct hw_batt_vol_sysfs_field_info hw_batt_vol_sysfs_field_tbl[] = {
	HW_BATT_VOL_SYSFS_FIELD_RO(bat_id_0, BAT_ID_0),
	HW_BATT_VOL_SYSFS_FIELD_RO(bat_id_1, BAT_ID_1),
	HW_BATT_VOL_SYSFS_FIELD_RO(bat_id_all, BAT_ID_ALL),
	HW_BATT_VOL_SYSFS_FIELD_RO(bat_id_max, BAT_ID_MAX),
	HW_BATT_VOL_SYSFS_FIELD_RO(bat_id_min, BAT_ID_MIN),
};

static struct attribute *hw_batt_vol_sysfs_attrs
	[ARRAY_SIZE(hw_batt_vol_sysfs_field_tbl) + 1];

static const struct attribute_group hw_batt_vol_sysfs_attr_group = {
	.attrs = hw_batt_vol_sysfs_attrs,
};

static void hw_batt_vol_sysfs_init_attrs(void)
{
	int i, limit = ARRAY_SIZE(hw_batt_vol_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		hw_batt_vol_sysfs_attrs[i] =
		&hw_batt_vol_sysfs_field_tbl[i].attr.attr;

	hw_batt_vol_sysfs_attrs[limit] = NULL;
}

static struct hw_batt_vol_sysfs_field_info *hw_batt_vol_sysfs_field_lookup(
	const char *name)
{
	int i, limit = ARRAY_SIZE(hw_batt_vol_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name,
			hw_batt_vol_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= limit)
		return NULL;

	return &hw_batt_vol_sysfs_field_tbl[i];
}

static ssize_t hw_batt_vol_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct hw_batt_vol_sysfs_field_info *info = NULL;
	int len = 0;

	info = hw_batt_vol_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case HW_BATT_VOL_SYSFS_BAT_ID_0:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			hw_battery_voltage(BAT_ID_0));
		break;

	case HW_BATT_VOL_SYSFS_BAT_ID_1:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			hw_battery_voltage(BAT_ID_1));
		break;

	case HW_BATT_VOL_SYSFS_BAT_ID_ALL:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			hw_battery_voltage(BAT_ID_ALL));
		break;

	case HW_BATT_VOL_SYSFS_BAT_ID_MAX:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			hw_battery_voltage(BAT_ID_MAX));
		break;

	case HW_BATT_VOL_SYSFS_BAT_ID_MIN:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			hw_battery_voltage(BAT_ID_MIN));
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		break;
	}

	return len;
}

static int hw_batt_vol_sysfs_create_group(struct hw_batt_vol_info *di)
{
	hw_batt_vol_sysfs_init_attrs();

	return sysfs_create_group(&di->dev->kobj,
		&hw_batt_vol_sysfs_attr_group);
}

static void hw_batt_vol_sysfs_remove_group(struct hw_batt_vol_info *di)
{
	sysfs_remove_group(&di->dev->kobj, &hw_batt_vol_sysfs_attr_group);
}

#else

static inline int hw_batt_vol_sysfs_create_group(struct hw_batt_vol_info *di)
{
	return 0;
}

static inline void hw_batt_vol_sysfs_remove_group(struct hw_batt_vol_info *di)
{
}
#endif /* CONFIG_SYSFS */

static int hw_batt_vol_create_sysfs(struct hw_batt_vol_info *di)
{
	int ret;
	struct class *power_class = NULL;

	ret = hw_batt_vol_sysfs_create_group(di);
	if (ret) {
		hwlog_err("sysfs group create failed\n");
		return ret;
	}

	power_class = hw_power_get_class();
	if (power_class) {
		if (!charge_dev)
			charge_dev = device_create(power_class, NULL, 0, NULL,
				"charger");
		if (IS_ERR(charge_dev)) {
			hwlog_err("sysfs device create failed\n");
			return PTR_ERR(charge_dev);
		}

		ret = sysfs_create_link(&charge_dev->kobj, &di->dev->kobj,
			"hw_batt_vol");
		if (ret) {
			hwlog_err("sysfs link create failed\n");
			hw_batt_vol_sysfs_remove_group(di);
			return ret;
		}
	}

	return 0;
}

static int hw_batt_vol_parse_dts(struct device_node *np,
	struct hw_batt_vol_info *di)
{
	int i = 0;
	int ret;
	const char *ops_name = NULL;
	struct device_node *child_node = NULL;

	di->total_vol = of_get_child_count(np);
	if (di->total_vol <= 0) {
		hwlog_err("total_vol dts read failed\n");
		return -EINVAL;
	}

	for_each_child_of_node(np, child_node) {
		ret = of_property_read_u32(child_node, "batt_id",
			&di->vol_buff[i].batt_id);
		if (ret) {
			hwlog_err("batt_id dts read failed\n");
			return -EINVAL;
		}

		ret = of_property_read_string(child_node, "ops_name",
			&ops_name);
		if (ret) {
			hwlog_err("ops_name dts read failed\n");
			return -EINVAL;
		}

		if (!strncmp(ops_name, HW_BATT_HISI_COUL,
			strlen(HW_BATT_HISI_COUL)))
			di->vol_buff[i].get_batt_vol = hisi_battery_voltage;

		strncpy(di->vol_buff[i].ops_name, ops_name,
			(HW_BATT_VOL_STR_MAX_LEM - 1));

		i++;
	}

	for (i = 0; i < di->total_vol; i++) {
		hwlog_info("para[%d]: ops_name:%s, batt_id:%d\n",
			i, di->vol_buff[i].ops_name, di->vol_buff[i].batt_id);
	}

	return 0;
}

static int hw_batt_vol_probe(struct platform_device *pdev)
{
	struct hw_batt_vol_info *di = NULL;
	struct device_node *np = NULL;
	struct device_node *dev_node = NULL;
	int ret = -1;

	hwlog_info("probe begin\n");

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_hw_batt_di = di;

	di->pdev = pdev;
	di->dev = &pdev->dev;
	np = pdev->dev.of_node;
	if (!di->pdev || !di->dev || !np) {
		hwlog_err("device_node is null\n");
		goto fail_free_mem;
	}

	if (of_property_read_u32(np, "batt_series_num", &di->batt_series_num)) {
		hwlog_err("batt_series_num dts read failed\n");
		di->batt_series_num = 1; /* default one battery */
	}
	hwlog_info("batt_series_num=%d\n", di->batt_series_num);

	dev_node = of_find_node_by_name(np, "batt_vol");
	if (!dev_node) {
		hwlog_err("batt_vol dts read failed\n");
		goto fail_free_mem;
	}

	ret = hw_batt_vol_parse_dts(dev_node, di);
	if (ret)
		goto fail_parse_dts;

	ret = hw_batt_vol_create_sysfs(di);
	if (ret)
		goto fail_parse_dts;

	platform_set_drvdata(pdev, di);

	hwlog_info("probe end\n");
	return 0;

fail_parse_dts:
	of_node_put(dev_node);

fail_free_mem:
	devm_kfree(&pdev->dev, di);
	g_hw_batt_di = NULL;

	return ret;
}

static int hw_batt_vol_remove(struct platform_device *pdev)
{
	struct hw_batt_vol_info *info = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, info);
	g_hw_batt_di = NULL;

	hwlog_info("remove end\n");
	return 0;

}

static const struct of_device_id hw_batt_vol_match_table[] = {
	{
		.compatible = "huawei,battery_voltage",
		.data = NULL,
	},
	{},
};

static struct platform_driver hw_batt_vol_driver = {
	.probe = hw_batt_vol_probe,
	.remove = hw_batt_vol_remove,
	.driver = {
		.name = "huawei,battery_voltage",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hw_batt_vol_match_table),
	},
};

static int __init hw_batt_vol_init(void)
{
	return platform_driver_register(&hw_batt_vol_driver);
}

static void __exit hw_batt_vol_exit(void)
{
	platform_driver_unregister(&hw_batt_vol_driver);
}

fs_initcall(hw_batt_vol_init);
module_exit(hw_batt_vol_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei battery voltage module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
