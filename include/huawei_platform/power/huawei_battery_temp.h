/*
 * huawei_battery_temp.h
 *
 * huawei battery temp driver
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

#ifndef _HUAWEI_BATTERY_TEMP_H_
#define _HUAWEI_BATTERY_TEMP_H_

#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif

enum battery_temp_id {
	BAT_TEMP_0 = 0,
	BAT_TEMP_1,
	BAT_TEMP_MIXED,
};

#ifdef CONFIG_HUAWEI_BATTERY_TEMP
int huawei_battery_temp(enum battery_temp_id id, int *temp);
int huawei_battery_temp_with_comp(enum battery_temp_id id, int *temp);

#else

static inline int huawei_battery_temp(enum battery_temp_id id, int *temp)
{
	if (!temp)
		return -1;

	*temp = hisi_battery_temperature();
	return 0;
}

static inline int huawei_battery_temp_with_comp(enum battery_temp_id id,
	int *temp)
{
	if (!temp)
		return -1;

	*temp = hisi_battery_temperature_for_charger();
	return 0;
}
#endif /* CONFIG_HUAWEI_BATTERY_TEMP */

#endif /* _HUAWEI_BATTERY_TEMP_H_ */
