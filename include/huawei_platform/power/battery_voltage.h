/*
 * battery_voltage.h
 *
 * battery boltage for power module
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

#ifndef _BATTERY_VOLTAGE_H_
#define _BATTERY_VOLTAGE_H_

#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif

#define HW_BATT_VOL_NUM                  (10)
#define HW_BATT_VOL_STR_MAX_LEM          (20)
#define HW_BATT_VOL_SINGLE_BATTERY       (1)
#define MAX_VOL_MV                       (100000)
#define HW_BATT_HISI_COUL                "hisi_coul"

enum hw_batt_vol_sysfs_type {
	HW_BATT_VOL_SYSFS_BAT_ID_0,
	HW_BATT_VOL_SYSFS_BAT_ID_1,
	HW_BATT_VOL_SYSFS_BAT_ID_ALL,
	HW_BATT_VOL_SYSFS_BAT_ID_MAX,
	HW_BATT_VOL_SYSFS_BAT_ID_MIN,
};

/*
 * enum number must be matched with device tree,
 * and new member must be added to last line.
 */
enum hw_batt_id {
	BAT_ID_0 = 0,
	BAT_ID_1 = 1,
	BAT_ID_ALL = 2,
	BAT_ID_MAX = 3,
	BAT_ID_MIN = 4,
};

enum hw_batt_num {
	HW_ONE_BAT = 1,
	HW_TWO_SERIES_BAT = 2,
};

struct hw_batt_get_vol {
	int batt_id;
	char ops_name[HW_BATT_VOL_STR_MAX_LEM];
	int (*get_batt_vol)(void);
};

struct hw_batt_vol_info {
	struct platform_device *pdev;
	struct device *dev;
	struct hw_batt_get_vol vol_buff[HW_BATT_VOL_NUM];
	int total_vol;
	int batt_series_num;
};

struct hw_batt_vol_ops {
	int (*get_batt_vol)(void);
};

#ifdef CONFIG_HUAWEI_BATTERY_VOLTAGE
int hw_battery_voltage(enum hw_batt_id batt_id);
int hw_battery_get_series_num(void);
int hw_battery_voltage_ops_register(struct hw_batt_vol_ops *ops,
	char *ops_name);

#else

static inline int hw_battery_voltage(enum hw_batt_id batt_id)
{
	return hisi_battery_voltage();
}

static inline int hw_battery_get_series_num(void)
{
	return 1; /* default one battery */
}

static inline int hw_battery_voltage_ops_register(struct hw_batt_vol_ops *ops,
	char *ops_name)
{
	return 0;
}
#endif /* CONFIG_HUAWEI_BATTERY_VOLTAGE */

#endif /* _BATTERY_VOLTAGE_H_ */
