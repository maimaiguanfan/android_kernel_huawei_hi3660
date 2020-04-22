/*
 * series_batt_charger.c
 *
 * series batt charger driver
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

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/series_batt_charger.h>
#include <huawei_platform/power/huawei_battery_temp.h>

#define BATTERY_NUM                 (2)
#define IAVG_MAX                    (3000)
#define IRCOMP_R_DEFAULT            (25)
#define IRCOMP_I_MIN_DEFAULT        (200)
#define IRCOMP_I_MAX_DEFAULT        (1000)
#define UV_TO_MV                    (1000)

#define HWLOG_TAG series_batt_charger
HWLOG_REGIST();

struct series_batt_info {
	struct charge_device_ops *g_ops;
	struct charge_device_ops local_ops;
	struct platform_device *pdev;
	struct device *dev;
	int chg_cur;
	int ircomp_r;
	int ircomp_i_min;
	int ircomp_i_max;
};

static struct series_batt_info *g_series_batt_di;

static int series_batt_set_terminal_voltage(int vol)
{
	struct series_batt_info *di = g_series_batt_di;
	int vbat_max = hw_battery_voltage(BAT_ID_MAX);
	int i_avg = hisi_battery_current_avg();
	int vbat_min;
	int term_vol;
	int ichg;
	int ir_comp_i_max;

	if (!di || !di->g_ops || !di->g_ops->set_terminal_voltage) {
		hwlog_err("di or g_ops or set_terminal_voltage is null\n");
		return -1;
	}

	if ((vol < 0) || (vol > hisi_battery_vbat_max())) {
		hwlog_err("vol is out of range\n");
		return -1;
	}

	vbat_min = hw_battery_voltage(BAT_ID_MIN);
	if (vbat_min < 0) {
		hwlog_err("vbat_min is out of range\n");
		return -1;
	}

	term_vol = vbat_min + vol;
	term_vol = term_vol > vol * BATTERY_NUM ? vol * BATTERY_NUM : term_vol;

	ichg = -hisi_battery_current();

	hwlog_info("ichg:%d, i_avg:%d, vbat_max:%d, vbat_min:%d, term_vol:%d\n",
		ichg, i_avg, vbat_max, vbat_min, term_vol);

	if ((i_avg > di->ircomp_i_min) && (i_avg < IAVG_MAX)) {
		ir_comp_i_max = di->ircomp_i_max < di->chg_cur ?
			di->ircomp_i_max : di->chg_cur;
		if (i_avg > ir_comp_i_max)
			i_avg = ir_comp_i_max;

		term_vol += (i_avg * di->ircomp_r) / UV_TO_MV;
		hwlog_info("ir vterm:%d, IRCOMP curr:%d\n", term_vol, i_avg);
	}

	if (vbat_max > vol) {
		term_vol -= vbat_max - vol;
		hwlog_info("ir vterm drop to %dmv\n", term_vol);
	}

	return di->g_ops->set_terminal_voltage(term_vol);
}

static int series_batt_set_charge_current(int value)
{
	struct series_batt_info *di = g_series_batt_di;

	if (!di || !di->g_ops || !di->g_ops->set_charge_current) {
		hwlog_err("di or g_ops or set_charge_current is null\n");
		return -1;
	}

	di->chg_cur = value;
	hwlog_info("charger current limit is [%d]ma\n", value);
	return di->g_ops->set_charge_current(value);
}

int series_batt_ops_register(struct charge_device_ops *ops)
{
	struct series_batt_info *di = g_series_batt_di;
	int ret;

	if (!ops || !di) {
		hwlog_err("ops or di is null\n");
		return -1;
	}

	di->g_ops = ops;
	di->local_ops = *di->g_ops;
	di->local_ops.set_terminal_voltage = series_batt_set_terminal_voltage;
	di->local_ops.set_charge_current = series_batt_set_charge_current;

	ret = charge_ops_register(&di->local_ops);
	if (ret) {
		hwlog_err("register charge ops failed\n");
		return ret;
	}

	return ret;
}

int get_series_batt_chargelog_head(char *chargelog_head)
{
	if (!chargelog_head) {
		hwlog_err("chargelog_head is null\n");
		return -1;
	}

	snprintf(chargelog_head, CHARGERLOG_SIZE,
		"bat0_temp   bat1_temp   mixed_temp   bat0_vol   bat1_vol   series_vol   ");
	return 0;
}

int get_series_batt_chargelog(char *chargelog)
{
	int bat0_temp = 0;
	int bat1_temp = 0;
	int mixed_temp = 0;
	int bat0_vol;
	int bat1_vol;
	int series_vol;

	if (!chargelog) {
		hwlog_err("chargelog is null\n");
		return -1;
	}

	huawei_battery_temp(BAT_TEMP_1, &bat1_temp);
	huawei_battery_temp(BAT_TEMP_0, &bat0_temp);
	huawei_battery_temp(BAT_TEMP_MIXED, &mixed_temp);
	bat1_vol = hw_battery_voltage(BAT_ID_1);
	bat0_vol = hw_battery_voltage(BAT_ID_0);
	series_vol = hw_battery_voltage(BAT_ID_ALL);

	snprintf(chargelog, CHARGERLOG_SIZE,
		"%-9d   %-9d   %-10d   %-8d   %-8d   %-10d   ",
		bat0_temp, bat1_temp, mixed_temp,
		bat0_vol, bat1_vol, series_vol);
	return 0;
}

static int series_batt_parse_dts(struct device_node *np,
	struct series_batt_info *di)
{
	if (of_property_read_u32(np, "ircomp_r", &di->ircomp_r)) {
		hwlog_err("ircomp_r dts read failed\n");
		di->ircomp_r = IRCOMP_R_DEFAULT;
	}
	hwlog_info("ir_comp_r=%d\n", di->ircomp_r);

	if (of_property_read_u32(np, "ircomp_i_min", &di->ircomp_i_min)) {
		hwlog_err("ircomp_i_min dts read failed\n");
		di->ircomp_i_min = IRCOMP_I_MIN_DEFAULT;
	}
	hwlog_info("ircomp_i_min=%d\n", di->ircomp_i_min);

	if (of_property_read_u32(np, "ircomp_i_max", &di->ircomp_i_max)) {
		hwlog_err("ircomp_i_max dts read failed\n");
		di->ircomp_i_max = IRCOMP_I_MAX_DEFAULT;
	}
	hwlog_info("ircomp_i_max=%d\n", di->ircomp_i_max);

	return 0;
}

static int series_batt_probe(struct platform_device *pdev)
{
	struct series_batt_info *di = NULL;
	struct device_node *np = NULL;
	int ret = -1;

	hwlog_info("probe begin\n");

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->g_ops = NULL;
	g_series_batt_di = di;
	di->dev = &pdev->dev;
	np = di->dev->of_node;

	if (!np) {
		hwlog_err("device_node is null\n");
		goto fail_free_mem;
	}

	series_batt_parse_dts(np, di);
	platform_set_drvdata(pdev, di);

	hwlog_info("probe end\n");
	return 0;

fail_free_mem:
	devm_kfree(&pdev->dev, di);
	g_series_batt_di = NULL;

	return ret;
}

static int series_batt_remove(struct platform_device *pdev)
{
	struct series_batt_info *info = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, info);
	g_series_batt_di = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id series_batt_match_table[] = {
	{
		.compatible = "huawei,series_batt_charger",
		.data = NULL,
	},
	{},
};

static struct platform_driver series_batt_driver = {
	.probe = series_batt_probe,
	.remove = series_batt_remove,
	.driver = {
		.name = "huawei,series_batt_charger",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(series_batt_match_table),
	},
};

static int __init series_batt_init(void)
{
	return platform_driver_register(&series_batt_driver);
}

static void __exit series_batt_exit(void)
{
	platform_driver_unregister(&series_batt_driver);
}

rootfs_initcall(series_batt_init);
module_exit(series_batt_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("series batt charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
