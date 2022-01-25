/*
 * huawei_battery_temp.c
 *
 * battery temp driver
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
#include <linux/thermal.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/huawei_battery_temp.h>

#define HWLOG_TAG huawei_battery_temp
HWLOG_REGIST();

#define UTEMP_PER_MTEMP                      (1000)
#define ABNORMAL_BATT_TEMPERATURE_POWEROFF   (67000)
#define LOW_BATT_TEMP_CHECK_THRESHOLD        (10000)
#define DELTA_TEMP                           (15000)
#define DEFAULT_TEMP                         (25000)
#define TEMP_LOW                             (15000)
#define TEMP_HIGH                            (40000)
#define BATT_TEMP_NTC_INVALID_THRE           (10)
#define BATT_TEMP_NTC_SAMPLES                (3)
#define COMP_PARA_ARRAYSIZE                  (6)
#define TEMP_SAMPLES                         (3)
#define RETRY_TIMES                          (5)
#define SENSOR_NAME_BATT_ID_0                "bat_0"
#define SENSOR_NAME_BATT_ID_1                "bat_1"

enum batt_temp_comp_para_info {
	BATT_TEMP_COMP_PARA_ICHG = 0,
	BATT_TEMP_COMP_PARA_VALUE,
	BATT_TEMP_COMP_PARA_TOTAL,
};

enum batt_temp_type {
	HW_BATT_TEMP_SYSFS_BATT_TEMP_0 = 0,
	HW_BATT_TEMP_SYSFS_BATT_TEMP_1,
	HW_BATT_TEMP_SYSFS_BATT_TEMP_MIXED,
	HW_BATT_TEMP_SYSFS_BATT_TEMP_0_COMP,
	HW_BATT_TEMP_SYSFS_BATT_TEMP_1_COMP,
	HW_BATT_TEMP_SYSFS_BATT_TEMP_MIXED_COMP,
};

struct batt_temp_comp_para_data {
	int batt_temp_comp_ichg;
	int batt_temp_comp_value;
};

struct hw_batt_temp_info {
	struct device *dev;
	int ntc_compensation_is;
	int batt_temp_low;
	int batt_temp_high;
	struct batt_temp_comp_para_data comp_para[COMP_PARA_ARRAYSIZE];
};

struct hw_batt_temp_info *g_di;

#ifdef CONFIG_SYSFS
#define HW_BATT_TEMP_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, hw_batt_temp_sysfs_show, store), \
	.name = HW_BATT_TEMP_SYSFS_##n, \
}

#define HW_BATT_TEMP_SYSFS_FIELD_RO(_name, n) \
	HW_BATT_TEMP_SYSFS_FIELD(_name, n, 0444, NULL)

struct hw_batt_temp_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static ssize_t hw_batt_temp_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct hw_batt_temp_sysfs_field_info hw_batt_temp_sysfs_field_tbl[] = {
	HW_BATT_TEMP_SYSFS_FIELD_RO(batt_temp_0,          BATT_TEMP_0),
	HW_BATT_TEMP_SYSFS_FIELD_RO(batt_temp_1,          BATT_TEMP_1),
	HW_BATT_TEMP_SYSFS_FIELD_RO(batt_temp_mixed,      BATT_TEMP_MIXED),
	HW_BATT_TEMP_SYSFS_FIELD_RO(batt_temp_0_comp,     BATT_TEMP_0_COMP),
	HW_BATT_TEMP_SYSFS_FIELD_RO(batt_temp_1_comp,     BATT_TEMP_1_COMP),
	HW_BATT_TEMP_SYSFS_FIELD_RO(batt_temp_mixed_comp, BATT_TEMP_MIXED_COMP),
};

static struct attribute *hw_batt_temp_sysfs_attrs
	[ARRAY_SIZE(hw_batt_temp_sysfs_field_tbl) + 1];

static const struct attribute_group hw_batt_temp_sysfs_attr_group = {
	.attrs = hw_batt_temp_sysfs_attrs,
};

static void hw_batt_temp_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(hw_batt_temp_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		hw_batt_temp_sysfs_attrs[i] =
			&hw_batt_temp_sysfs_field_tbl[i].attr.attr;

	hw_batt_temp_sysfs_attrs[limit] = NULL;
}

static struct hw_batt_temp_sysfs_field_info
	*hw_batt_temp_sysfs_field_lookup(const char *name)
{
	int i;
	int limit = ARRAY_SIZE(hw_batt_temp_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name,
			hw_batt_temp_sysfs_field_tbl[i].attr.attr.name,
				strlen(name)))
			break;
	}

	if (i >= limit)
		return NULL;

	return &hw_batt_temp_sysfs_field_tbl[i];
}

static int hw_batt_temp_sysfs_create_group(struct hw_batt_temp_info *di)
{
	hw_batt_temp_sysfs_init_attrs();
	return sysfs_create_group(&di->dev->kobj,
		&hw_batt_temp_sysfs_attr_group);
}

static void hw_batt_temp_sysfs_remove_group(struct hw_batt_temp_info *di)
{
	sysfs_remove_group(&di->dev->kobj, &hw_batt_temp_sysfs_attr_group);
}

#else

static int hw_batt_temp_sysfs_create_group(struct hw_batt_temp_info *di)
{
	return 0;
}

static void hw_batt_temp_sysfs_remove_group(struct hw_batt_temp_info *di)
{
}
#endif /* CONFIG_SYSFS */

static ssize_t hw_batt_temp_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct hw_batt_temp_sysfs_field_info *info = NULL;
	struct hw_batt_temp_info *di = g_di;
	int batt_temp = 0;
	int len = 0;

	info = hw_batt_temp_sysfs_field_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case HW_BATT_TEMP_SYSFS_BATT_TEMP_0:
		huawei_battery_temp(BAT_TEMP_0, &batt_temp);
		len = snprintf(buf, PAGE_SIZE, "%d\n", batt_temp);
		break;

	case HW_BATT_TEMP_SYSFS_BATT_TEMP_1:
		huawei_battery_temp(BAT_TEMP_1, &batt_temp);
		len = snprintf(buf, PAGE_SIZE, "%d\n", batt_temp);
		break;

	case HW_BATT_TEMP_SYSFS_BATT_TEMP_MIXED:
		huawei_battery_temp(BAT_TEMP_MIXED, &batt_temp);
		len = snprintf(buf, PAGE_SIZE, "%d\n", batt_temp);
		break;

	case HW_BATT_TEMP_SYSFS_BATT_TEMP_0_COMP:
		huawei_battery_temp_with_comp(BAT_TEMP_0, &batt_temp);
		len = snprintf(buf, PAGE_SIZE, "%d\n", batt_temp);
		break;

	case HW_BATT_TEMP_SYSFS_BATT_TEMP_1_COMP:
		huawei_battery_temp_with_comp(BAT_TEMP_1, &batt_temp);
		len = snprintf(buf, PAGE_SIZE, "%d\n", batt_temp);
		break;

	case HW_BATT_TEMP_SYSFS_BATT_TEMP_MIXED_COMP:
		huawei_battery_temp_with_comp(BAT_TEMP_MIXED, &batt_temp);
		len = snprintf(buf, PAGE_SIZE, "%d\n", batt_temp);
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		break;
	}

	return len;
}

static int get_batt_temp_by_sensor_name(char *sensor_name)
{
	struct thermal_zone_device *tz = NULL;
	int temp_array[BATT_TEMP_NTC_SAMPLES] = {0};
	int temp_samples = TEMP_SAMPLES;
	int temp_invalid_flag = 0;
	int max, min, i;
	int temp = 0;
	int ret = 0;
	int sum = 0;

	if (!sensor_name) {
		hwlog_err("sensor name is null\n");
		goto fail_get_temp;
	}

	tz = thermal_zone_get_zone_by_name(sensor_name);
	if (IS_ERR(tz)) {
		hwlog_err("get %s thermal zone fail\n", sensor_name);
		goto fail_get_temp;
	}

	while (temp_samples--) {
		temp_invalid_flag = 0;

		for (i = 0; i < BATT_TEMP_NTC_SAMPLES; ++i) {
			ret = thermal_zone_get_temp(tz, &temp_array[i]);
			temp_array[i] = temp_array[i];
			if (ret) {
				hwlog_err("get batt temp fail\n");
				temp_invalid_flag = 1;
				break;
			}

			hwlog_info("battID:%s,temp:%d\n",
				sensor_name, temp_array[i]);
		}

		if (temp_invalid_flag == 1)
			continue;

		/* check temperature value is valid */
		max = temp_array[0];
		min = temp_array[0];

		for (i = 0; i < BATT_TEMP_NTC_SAMPLES; ++i) {
			if (max < temp_array[i])
				max = temp_array[i];

			if (min > temp_array[i])
				min = temp_array[i];
		}

		if (max - min > BATT_TEMP_NTC_INVALID_THRE * UTEMP_PER_MTEMP) {
			hwlog_err("invalid temp temp_max:%d temp_min:%d\n",
				max, min);
			temp_invalid_flag = 1;
		}

		if (temp_invalid_flag == 0)
			break;

	}

	if (temp_invalid_flag == 1) {
		hwlog_err("get batt temp fail, use default temperature\n");
		goto fail_get_temp;
	}

	/* get average temperature */
	for (i = 0; i < BATT_TEMP_NTC_SAMPLES; ++i)
		sum += temp_array[i];

	temp = sum / BATT_TEMP_NTC_SAMPLES;
	hwlog_info("sensor_name:%s, temp is:%d\n", sensor_name, temp);
	return temp;

fail_get_temp:
	return DEFAULT_TEMP;
}

static int get_batt_temp_with_comp(int temp_without_compensation)
{
	struct hw_batt_temp_info *di = g_di;
	int temp_with_compensation = 0;
	int ichg = 0;
	int i = 0;

	if (!di) {
		hwlog_err("di is null\n");
		return temp_without_compensation;
	}

	if (!di->ntc_compensation_is) {
		hwlog_err("temp_without_compensation is 0\n");
		return temp_without_compensation;
	}

	ichg = hisi_battery_current();
	ichg = abs(ichg);

	for (i = 0; i < COMP_PARA_ARRAYSIZE; i++) {
		if (ichg >= di->comp_para[i].batt_temp_comp_ichg) {
			temp_with_compensation = temp_without_compensation -
				di->comp_para[i].batt_temp_comp_value;
			hwlog_info("ichg is %dma, batt_temp_comp_value is %d\n",
				ichg, di->comp_para[i].batt_temp_comp_value);
			break;
		}
	}

	hwlog_info("ichg is %dma, old temp is %d, temp with comp is %d\n",
		ichg, temp_without_compensation, temp_with_compensation);
	return temp_with_compensation;
}

int get_batt_temp_stably(char *sensor_name)
{
	int retry_times = RETRY_TIMES;
	int cnt = 0;
	int temperature = 0;
	int delta = 0;
	int batt_temp = 0;

	batt_temp = get_batt_temp_by_sensor_name(sensor_name);

	while (cnt++ < retry_times) {
		temperature = get_batt_temp_with_comp(batt_temp);
		delta = abs(batt_temp - temperature);

		if ((delta > DELTA_TEMP) ||
			(temperature > ABNORMAL_BATT_TEMPERATURE_POWEROFF) ||
			(temperature <= LOW_BATT_TEMP_CHECK_THRESHOLD)) {
			continue;
		}

		hwlog_info("stably temp!, old_temp =%d, cnt =%d, temp = %d\n",
			batt_temp, cnt, temperature);
		return temperature;
	}

	return temperature;
}

static int get_batt_temp_mixed(int bat0_temp, int bat1_temp)
{
	int temp_mixed = 0;
	int batt_temp_high = g_di->batt_temp_high;
	int batt_temp_low = g_di->batt_temp_low;

	hwlog_info("bat0_temp is %d, bat1_temp is %d\n",
		bat0_temp, bat1_temp);

	if ((bat0_temp < batt_temp_low) && (bat1_temp > batt_temp_high)) {
		temp_mixed = (batt_temp_low - bat0_temp) >
			(bat1_temp - batt_temp_high) ? bat0_temp : bat1_temp;
	} else if ((bat0_temp > batt_temp_high) &&
		(bat1_temp < batt_temp_low)) {
		temp_mixed = (bat0_temp - batt_temp_high) >
			(batt_temp_low - bat1_temp) ? bat0_temp : bat1_temp;
	} else if (((bat0_temp < batt_temp_low) &&
		(bat1_temp < batt_temp_low)) ||
		((bat0_temp - batt_temp_low) *
		(bat1_temp - batt_temp_low) < 0)) {
		temp_mixed = bat0_temp < bat1_temp ? bat0_temp : bat1_temp;
	} else if (((bat0_temp > batt_temp_high) &&
		(bat1_temp > batt_temp_high)) ||
		((bat0_temp - batt_temp_high) *
		(bat1_temp - batt_temp_high) < 0)) {
		temp_mixed = bat0_temp > bat1_temp ? bat0_temp : bat1_temp;
	} else {
		/* temp 0 temp 1 between batt_temp_high and batt_temp_low */
		temp_mixed = (bat0_temp + bat1_temp) / 2; /* average value */
	}

	hwlog_info("mixed batt temp is %d", temp_mixed);
	return temp_mixed;
}

int huawei_battery_temp(enum battery_temp_id id, int *temp)
{
	int batt_temp_0 = 0;
	int batt_temp_1 = 0;
	int batt_temp = 0;

	if (!temp) {
		hwlog_err("temp is null\n");
		return -1;
	}

	if (hw_battery_get_series_num() == HW_ONE_BAT) {
		*temp = hisi_battery_temperature();
		hwlog_info("one batt, batt_temp is [%d]\n", *temp);
		return 0;
	}

	if (!g_di) {
		*temp = hisi_battery_temperature();
		hwlog_err("g_di is null, batt_temp = [%d]\n", *temp);
		return 0;
	}

	switch (id) {
	case BAT_TEMP_0:
		batt_temp = get_batt_temp_by_sensor_name(
			SENSOR_NAME_BATT_ID_0);
		break;

	case BAT_TEMP_1:
		batt_temp = get_batt_temp_by_sensor_name(
			SENSOR_NAME_BATT_ID_1);
		break;

	case BAT_TEMP_MIXED:
		batt_temp_0 = get_batt_temp_by_sensor_name(
			SENSOR_NAME_BATT_ID_0);
		batt_temp_1 = get_batt_temp_by_sensor_name(
			SENSOR_NAME_BATT_ID_1);
		batt_temp = get_batt_temp_mixed(batt_temp_0, batt_temp_1);
		break;

	default:
		hwlog_err("invalid battery_temp_id: %d", id);
		return -1;
	}

	*temp = batt_temp / UTEMP_PER_MTEMP;
	hwlog_info("hw_batt_temp: temp = [%d]", *temp);
	return 0;
}

int huawei_battery_temp_with_comp(enum battery_temp_id id, int *temp)
{
	int batt_temp = 0;
	int batt_temp_0 = 0;
	int batt_temp_1 = 0;

	if (!temp) {
		hwlog_err("temp is null\n");
		return -1;
	}

	if (hw_battery_get_series_num() == HW_ONE_BAT) {
		*temp = hisi_battery_temperature_for_charger();
		hwlog_info("one batt, batt_temp is [%d]\n", *temp);
		return 0;
	}

	if (!g_di) {
		*temp = hisi_battery_temperature_for_charger();
		hwlog_err("g_di is null, batt_temp is [%d]\n", *temp);
		return 0;
	}

	switch (id) {
	case BAT_TEMP_0:
		batt_temp = get_batt_temp_stably(SENSOR_NAME_BATT_ID_0);
		break;

	case BAT_TEMP_1:
		batt_temp = get_batt_temp_stably(SENSOR_NAME_BATT_ID_1);
		break;

	case BAT_TEMP_MIXED:
		batt_temp_0 = get_batt_temp_stably(SENSOR_NAME_BATT_ID_0);
		batt_temp_1 = get_batt_temp_stably(SENSOR_NAME_BATT_ID_1);
		batt_temp = get_batt_temp_mixed(batt_temp_0, batt_temp_1);
		break;

	default:
		hwlog_err("invalid battery_temp_id: %d", id);
		return -1;
	}

	*temp = batt_temp / UTEMP_PER_MTEMP;
	hwlog_info("hw_batt_comp_temp: temp = [%d]", *temp);
	return 0;
}

static void batt_temp_parse_dts(struct device_node *np,
	struct hw_batt_temp_info *di)
{
	const char *compensation_data_string = NULL;
	int ntc_compensation_is = 0;
	int array_len = 0;
	int i = 0;
	int idata = 0;
	int ret = 0;

	ret = of_property_read_u32(np, "battery_temp_high",
		&di->batt_temp_high);
	if (ret) {
		hwlog_err("battery_temp_high dts read failed\n");
		di->batt_temp_high = TEMP_HIGH;
	}
	hwlog_info("batt_temp_high=%d\n", di->batt_temp_high);

	ret = of_property_read_u32(np, "battery_temp_low", &di->batt_temp_low);
	if (ret) {
		hwlog_err("batt_temp_low dts read failed\n");
		di->batt_temp_low = TEMP_LOW;
	}
	hwlog_info("batt_temp_low=%d\n", di->batt_temp_low);

	ret = of_property_read_u32(np, "ntc_compensation_is",
		&ntc_compensation_is);
	if (ret) {
		ntc_compensation_is = 0;
		hwlog_err("ntc_compensation_is dts read failed\n");
		return;
	}

	di->ntc_compensation_is = ntc_compensation_is;
	memset(di->comp_para, 0,
		COMP_PARA_ARRAYSIZE * sizeof(struct batt_temp_comp_para_data));

	array_len = of_property_count_strings(np, "ntc_temp_compensation_para");
	if ((array_len <= 0) || (array_len % BATT_TEMP_COMP_PARA_TOTAL != 0)) {
		hwlog_err("batt temp comp para is invaild\n");
		return;
	}

	if (array_len > COMP_PARA_ARRAYSIZE * BATT_TEMP_COMP_PARA_TOTAL) {
		array_len = COMP_PARA_ARRAYSIZE * BATT_TEMP_COMP_PARA_TOTAL;
		hwlog_err("para too long, use %d paras\n", array_len);
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np,
			"ntc_temp_compensation_para", i,
			&compensation_data_string);

		if (ret) {
			hwlog_err("ntc_temp_compensation_para dts read failed\n");
			di->ntc_compensation_is = 0;
			return;
		}

		if (kstrtol(compensation_data_string, 10, (long *)&idata)) {
			hwlog_err("kstrtol fail\n");
			di->ntc_compensation_is = 0;
			return;
		}

		switch (i % BATT_TEMP_COMP_PARA_TOTAL) {
		case BATT_TEMP_COMP_PARA_ICHG:
			di->comp_para[i / BATT_TEMP_COMP_PARA_TOTAL]
				.batt_temp_comp_ichg = idata;
			break;

		case BATT_TEMP_COMP_PARA_VALUE:
			di->comp_para[i / BATT_TEMP_COMP_PARA_TOTAL]
				.batt_temp_comp_value = idata;
			break;

		default:
			hwlog_err("ntc_temp_compensation_para get failed\n");
			break;
		}

		hwlog_info("di->comp_para[%d][%d] is %d\n",
			i / BATT_TEMP_COMP_PARA_TOTAL,
			i % BATT_TEMP_COMP_PARA_TOTAL, idata);
	}

}

static int batt_temp_probe(struct platform_device *pdev)
{
	struct hw_batt_temp_info *di = NULL;
	struct device_node *np = NULL;
	struct class *power_class = NULL;
	int ret = -1;

	hwlog_info("probe begin\n");

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_di = di;
	di->dev = &pdev->dev;
	np = pdev->dev.of_node;
	batt_temp_parse_dts(np, di);
	platform_set_drvdata(pdev, di);

	ret = hw_batt_temp_sysfs_create_group(di);
	if (ret) {
		hwlog_err("sysfs group create failed\n");
		goto fail_free_mem;
	}

	power_class = hw_power_get_class();
	if (power_class) {
		if (!charge_dev)
			charge_dev = device_create(power_class, NULL, 0, NULL,
				"charger");
		if (IS_ERR(charge_dev)) {
			hwlog_err("sysfs device create failed\n");
			ret = PTR_ERR(charge_dev);
			goto fail_free_mem;
		}

		ret = sysfs_create_link(&charge_dev->kobj, &di->dev->kobj,
			"hw_batt_temp");
		if (ret) {
			hwlog_err("sysfs link create failed\n");
			hw_batt_temp_sysfs_remove_group(di);
			goto fail_free_mem;
		}
	}

	hwlog_info("probe end\n");
	return 0;

fail_free_mem:
	devm_kfree(&pdev->dev, di);
	g_di = NULL;

	return ret;
}

static int  batt_temp_remove(struct platform_device *pdev)
{
	struct hw_batt_temp_info *di = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_di = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id batt_temp_match_table[] = {
	{
		.compatible = "huawei,battery_temp",
		.data = NULL,
	},
	{},
};

static struct platform_driver batt_temp_driver = {
	.probe = batt_temp_probe,
	.remove = batt_temp_remove,
	.driver = {
		.name = "huawei,battery_temp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(batt_temp_match_table),
	},
};

int __init batt_temp_init(void)
{
	return platform_driver_register(&batt_temp_driver);
}

void __exit batt_temp_exit(void)
{
	platform_driver_unregister(&batt_temp_driver);
}

fs_initcall_sync(batt_temp_init);
module_exit(batt_temp_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei battery temp module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
