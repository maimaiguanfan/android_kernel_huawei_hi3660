/*
 * dual_charger.h
 *
 * dual charger driver
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

#ifndef _DUAL_CHARGER_H_
#define _DUAL_CHARGER_H_

#define DEFAULT_IIN_MAX_EACH_CHARGER      1100
#define DEFAULT_ICHG_NEED_AUX_CHARGER     1000
#define DEFAULT_ICHG_MAX                  3500

enum dual_charger_sysfs_type {
	DUAL_CHARGER_SYSFS_ENABLE_CHARGER_MAIN = 0,
	DUAL_CHARGER_SYSFS_ENABLE_CHARGER_AUX,
};

struct dual_charger_info {
	struct device *dev;

	int iin_thermal_main;
	int iin_thermal_aux;
	int ichg_thermal_main;
	int ichg_thermal_aux;
	int ichg;
	int ichg_max;

	int input_current_main;
	int input_current_aux;
	int charge_current_main;
	int charge_current_aux;

	int charge_enable_main;
	int charge_enable_aux;
	int charge_enable_sysfs_main;
	int charge_enable_sysfs_aux;

	int iin_max_each_charger;
	int ichg_need_aux_charger;

	int charge_done;
};

extern int bq25892_main_get_ichg_reg(void);
extern int bq25892_aux_get_ichg_reg(void);
extern int bq25892_main_get_ichg_adc(void);
extern int bq25892_aux_get_ichg_adc(void);

#ifdef CONFIG_DUAL_CHARGER

extern int charge_main_ops_register(struct charge_device_ops *ops);
extern int charge_aux_ops_register(struct charge_device_ops *ops);

#else

static inline int charge_main_ops_register(struct charge_device_ops *ops)
{
	return -1;
}

static inline int charge_aux_ops_register(struct charge_device_ops *ops)
{
	return -1;
}

#endif /* CONFIG_DUAL_CHARGER */

#endif /* _DUAL_CHARGER_H_ */
