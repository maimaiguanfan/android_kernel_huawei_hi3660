/*
 * series_batt_charger.h
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

#ifndef _SERIES_BATT_CHARGER_H_
#define _SERIES_BATT_CHARGER_H_

#define CHARGERLOG_SIZE (256)

#ifdef CONFIG_SERIES_BATT
int series_batt_ops_register(struct charge_device_ops *ops);
int get_series_batt_chargelog_head(char *chargelog_head);
int get_series_batt_chargelog(char *chargelog);

#else

static inline int series_batt_ops_register(struct charge_device_ops *ops)
{
	return 0;
}

static inline int get_series_batt_chargelog_head(char *chargelog_head)
{
	return 0;
}

static inline int get_series_batt_chargelog(char *chargelog)
{
	return 0;
}
#endif /* CONFIG_SERIES_BATT */

#endif /* _SERIES_BATT_CHARGER_H_ */
