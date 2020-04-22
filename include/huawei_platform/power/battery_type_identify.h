/*
 * battery_type_identify.h
 *
 * Interfaces for switch mode between adc and
 * onewire communication to Identify battery type.
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef _BATTERY_TYPE_IDENTIFY_H_
#define _BATTERY_TYPE_IDENTIFY_H_

struct security_ic_ops {
	int (*open_ic)(void);
	int (*close_ic)(void);
};

enum batt_type_identify_mode {
	BAT_ID_VOLTAGE = 0,
	BAT_ID_SN,
	BAT_INVALID_MODE,
};

#ifdef CONFIG_HUAWEI_BATTERY_TYPE_IDENTIFY
void apply_batt_type_mode(enum batt_type_identify_mode mode);
void release_batt_type_mode(void);
void register_security_ic_ops(const struct security_ic_ops *ops);
void unregister_security_ic_ops(const struct security_ic_ops *ops);
#else
static inline void apply_batt_type_mode(enum batt_type_identify_mode mode)
{
}

static inline void release_batt_type_mode(void)
{
}

static inline void register_security_ic_ops(const struct security_ic_ops *ops)
{
}

static inline void unregister_security_ic_ops(const struct security_ic_ops *ops)
{
}
#endif /* CONFIG_HUAWEI_BATTERY_TYPE_IDENTIFY */

#endif /* _BATTERY_TYPE_IDENTIFY_H_ */

