/*
 * vbat_hkadc.c
 *
 * get battery voltage by hkadc
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
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/huawei_charger.h>
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#include "vbat_hkadc.h"

#define HWLOG_TAG vbat_hkadc
HWLOG_REGIST();

static struct vbat_hkadc_info *g_vbat_hkadc_di;
static long v_offset_a = V_OFFSET_A_DEFAULT;
static long v_offset_b = V_OFFSET_B_DEFAULT;

static int __init early_parse_vbat_hkadc_cmdline(char *p)
{
	char *token = NULL;

	if (!p) {
		hwlog_err("point is null\n");
		return -1;
	}
	hwlog_info("point=%s\n", p);

	token = strsep(&p, ",");
	if (token) {
		if (kstrtoul(token, 10, &v_offset_a) != 0) {
			hwlog_err("prase v_offset_a=%ld\n", v_offset_a);
			return -1;
		}
	}
	token = strsep(&p, ",");
	if (token) {
		if (kstrtol(token, 10, &v_offset_b) != 0) {
			hwlog_err("prase v_offset_b=%ld\n", v_offset_b);
			return -1;
		}
	}

	if (V_OFFSET_A_MIN > v_offset_a || V_OFFSET_A_MAX < v_offset_a) {
		v_offset_a = V_OFFSET_A_DEFAULT;
		hwlog_err("v_offset_a invalid when prase_cmdline\n");
	}

	if (V_OFFSET_B_MIN > v_offset_b || V_OFFSET_B_MAX < v_offset_b) {
		v_offset_b = V_OFFSET_B_DEFAULT;
		hwlog_err("v_offset_a invalid when prase_cmdline\n");
	}

	hwlog_info("prase cmdline: v_offset_a=%ld,v_offset_b=%ld\n",
		v_offset_a, v_offset_b);
	return 0;
}

early_param("vbat_hkadc", early_parse_vbat_hkadc_cmdline);

static int get_cali_vbat_hkadc_mv(void)
{
	int i;
	int adc_vbat;
	int vol_temp;
	s64 temp;
	int vol_cali;

	if (!g_vbat_hkadc_di) {
		hwlog_err("g_vbat_hkadc_di is null\n");
		return -1;
	}

	for (i = 0; i < VBAT_HKADC_RETRY_TIMES; i++) {
		adc_vbat = hisi_adc_get_adc(g_vbat_hkadc_di->adc_channel);

		if (adc_vbat < 0)
			hwlog_err("hisi adc read fail\n");
		else
			break;
	}

	if (adc_vbat < 0)
		return -1;

	vol_temp = adc_vbat * (g_vbat_hkadc_di->coef) /
		VBAT_HKADC_COEF_MULTIPLE;

	/* get voltage(mv) by calibration */
	temp = vol_temp * (s64)(v_offset_a);
	/* v_offset_b uV to mV */
	vol_cali = (int)div_s64(temp, V_OFFSET_A_DEFAULT) + v_offset_b / 1000;

	hwlog_info("adc_vbat=%d, coef=%d, vol_cali=%d\n",
		adc_vbat, g_vbat_hkadc_di->coef, vol_cali);
	hwlog_info("v_offset_a=%ld, v_offset_b=%ld\n", v_offset_a, v_offset_b);

	return vol_cali;
}

static int get_compensate_vbat_hkadc_mv(void)
{
	int vol_cali;
	int vol_comp;
	int cur;

	vol_cali = get_cali_vbat_hkadc_mv();
	cur = hisi_battery_current();

	/* cur:mA, r_pcb:uOhm */
	vol_comp = vol_cali + cur * (g_vbat_hkadc_di->r_pcb) / 1000000;

	hwlog_info("cur=%d, vol_comp=%d\n", cur, vol_comp);

	return vol_comp;
}

struct hw_batt_vol_ops vbat_hkadc_ops = {
	.get_batt_vol = get_compensate_vbat_hkadc_mv,
};

#ifdef CONFIG_SYSFS
#define VBAT_HKADC_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, vbat_hkadc_sysfs_show, store), \
	.name = VBAT_HKADC_SYSFS_##n, \
}

#define VBAT_HKADC_SYSFS_FIELD_RW(_name, n) \
	VBAT_HKADC_SYSFS_FIELD(_name, n, 0644, vbat_hkadc_sysfs_store)

#define VBAT_HKADC_SYSFS_FIELD_RO(_name, n) \
	VBAT_HKADC_SYSFS_FIELD(_name, n, 0444, NULL)

struct vbat_hkadc_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static ssize_t vbat_hkadc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t vbat_hkadc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct vbat_hkadc_sysfs_field_info vbat_hkadc_sysfs_field_tbl[] = {
	VBAT_HKADC_SYSFS_FIELD_RW(v_offset_a, V_OFFSET_A),
	VBAT_HKADC_SYSFS_FIELD_RW(v_offset_b, V_OFFSET_B),
	VBAT_HKADC_SYSFS_FIELD_RO(bat1_cali_vol, BAT1_CALI_VOL),
};

static struct attribute *vbat_hkadc_sysfs_attrs
	[ARRAY_SIZE(vbat_hkadc_sysfs_field_tbl) + 1];

static const struct attribute_group vbat_hkadc_sysfs_attr_group = {
	.attrs = vbat_hkadc_sysfs_attrs,
};

static void vbat_hkadc_sysfs_init_attrs(void)
{
	int i, limit = ARRAY_SIZE(vbat_hkadc_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		vbat_hkadc_sysfs_attrs[i] =
		&vbat_hkadc_sysfs_field_tbl[i].attr.attr;

	vbat_hkadc_sysfs_attrs[limit] = NULL;
}

static struct vbat_hkadc_sysfs_field_info *vbat_hkadc_sysfs_field_lookup(
	const char *name)
{
	int i, limit = ARRAY_SIZE(vbat_hkadc_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name, vbat_hkadc_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= limit)
		return NULL;

	return &vbat_hkadc_sysfs_field_tbl[i];
}

static ssize_t vbat_hkadc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct vbat_hkadc_sysfs_field_info *info = NULL;
	int len = 0;

	info = vbat_hkadc_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case VBAT_HKADC_SYSFS_V_OFFSET_A:
		len = snprintf(buf, PAGE_SIZE, "%ld\n", v_offset_a);
		break;
	case VBAT_HKADC_SYSFS_V_OFFSET_B:
		len = snprintf(buf, PAGE_SIZE, "%ld\n", v_offset_b);
		break;
	case VBAT_HKADC_SYSFS_BAT1_CALI_VOL:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			get_compensate_vbat_hkadc_mv());
		break;
	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		break;
	}

	return len;
}

static ssize_t vbat_hkadc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	struct vbat_hkadc_sysfs_field_info *info = NULL;

	info = vbat_hkadc_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case VBAT_HKADC_SYSFS_V_OFFSET_A:
		if ((kstrtol(buf, 10, &val) < 0) ||
			(val < V_OFFSET_A_MIN) || (val > V_OFFSET_A_MAX)) {
			hwlog_err("store v_offset_a = %ld\n", val);
			return -EINVAL;
		}
		hwlog_info("v_offset_a_sysfs_store,val=%ld\n", val);
		v_offset_a = val;
		break;
	case VBAT_HKADC_SYSFS_V_OFFSET_B:
		if ((kstrtol(buf, 10, &val) < 0) ||
			(val < V_OFFSET_B_MIN) || (val > V_OFFSET_B_MAX)) {
			hwlog_err("store v_offset_b = %ld\n", val);
			return -EINVAL;
		}
		hwlog_info("v_offset_b_sysfs_store,val=%ld\n", val);
		v_offset_b = val;
		break;
	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		break;
	}

	return count;
}

static int vbat_hkadc_sysfs_create_group(struct vbat_hkadc_info *di)
{
	vbat_hkadc_sysfs_init_attrs();

	return sysfs_create_group(&di->dev->kobj, &vbat_hkadc_sysfs_attr_group);
}

static void vbat_hkadc_sysfs_remove_group(struct vbat_hkadc_info *di)
{
	sysfs_remove_group(&di->dev->kobj, &vbat_hkadc_sysfs_attr_group);
}

#else

static inline int vbat_hkadc_sysfs_create_group(struct vbat_hkadc_info *di)
{
	return 0;
}

static inline void vbat_hkadc_sysfs_remove_group(struct vbat_hkadc_info *di)
{
}
#endif /* CONFIG_SYSFS */

static int vbat_hkadc_create_sysfs(struct vbat_hkadc_info *di)
{
	int ret;
	struct class *power_class = NULL;

	ret = vbat_hkadc_sysfs_create_group(di);
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
			"vbat_hkadc");
		if (ret) {
			hwlog_err("sysfs link create failed\n");
			vbat_hkadc_sysfs_remove_group(di);
			return ret;
		}
	}

	return 0;
}

static int vbat_hkadc_parse_dts(struct device_node *np,
	struct vbat_hkadc_info *di)
{
	int ret;

	ret = of_property_read_u32(np, "adc_channel", &di->adc_channel);
	if (ret) {
		hwlog_err("adc_channel dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("adc_channel=%d\n", di->adc_channel);

	ret = of_property_read_u32(np, "coef", &di->coef);
	if (ret) {
		hwlog_err("coef dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("coef=%d\n", di->coef);

	ret = of_property_read_u32(np, "r_pcb", &di->r_pcb);
	if (ret) {
		hwlog_err("r_pcb dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("r_pcb=%d\n", di->r_pcb);

	return 0;

}

static int vbat_hkadc_probe(struct platform_device *pdev)
{
	struct vbat_hkadc_info *di = NULL;
	struct device_node *np = NULL;
	int ret = -1;

	hwlog_info("probe begin\n");

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_vbat_hkadc_di = di;

	di->pdev = pdev;
	di->dev = &pdev->dev;
	np = pdev->dev.of_node;
	if (!di->pdev || !di->dev || !np) {
		hwlog_err("device_node is null\n");
		goto fail_free_mem;
	}

	ret = vbat_hkadc_parse_dts(np, di);
	if (ret)
		goto fail_parse_dts;

	ret = hw_battery_voltage_ops_register(&vbat_hkadc_ops, "hisi_hkadc");
	if (ret)
		hwlog_err("hw_battery_voltage ops register failed\n");

	ret = vbat_hkadc_create_sysfs(di);
	if (ret)
		goto fail_create_sysfs;

	platform_set_drvdata(pdev, di);

	hwlog_info("probe end\n");
	return 0;

fail_create_sysfs:
fail_parse_dts:
fail_free_mem:
	devm_kfree(&pdev->dev, di);
	g_vbat_hkadc_di = NULL;

	return ret;
}

static int vbat_hkadc_remove(struct platform_device *pdev)
{
	struct vbat_hkadc_info *info = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, info);
	g_vbat_hkadc_di = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id vbat_hkadc_match_table[] = {
	{
		.compatible = "huawei,vbat_hkadc",
		.data = NULL,
	},
	{},
};

static struct platform_driver vbat_hkadc_driver = {
	.probe = vbat_hkadc_probe,
	.remove = vbat_hkadc_remove,
	.driver = {
		.name = "huawei,vbat_hkadc",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(vbat_hkadc_match_table),
	},
};

static int __init vbat_hkadc_init(void)
{
	return platform_driver_register(&vbat_hkadc_driver);
}

static void __exit vbat_hkadc_exit(void)
{
	platform_driver_unregister(&vbat_hkadc_driver);
}

fs_initcall_sync(vbat_hkadc_init);
module_exit(vbat_hkadc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei vbat_hkadc module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
