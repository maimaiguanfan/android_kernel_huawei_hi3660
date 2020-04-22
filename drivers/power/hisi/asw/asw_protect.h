/*
 * asw_protect.h
 *
 * asw protect driver
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

#ifndef _ASW_PROTECT_H_
#define _ASW_PROTECT_H_

#include "../coul/hisi_coul_core.h"

#define ANDROID_BOOT_FACTORY             "androidboot.swtype=factory"
#define ASW_PROTECT_DELTA_VOLT           300

#define NORMAL_SCENE_ASW_PROTECT_VOLT    0
#define H1_SCENE_ASW_PROTECT_VOLT        400
#define H2_SCENE_ASW_PROTECT_VOLT        200

#define ASW_PROTECT_IIN_LIMIT_SET        100
#define MAX_INT                          0x0FFFFFFF
#define VOLT_LOWEST                      3200

#define ASW_REAL_FCC_NEED_BACKUP         1
#define ASW_REAL_FCC_NO_NEED_BACKUP      0
#define ASW_PRINT_LINE                   8

#define ASW_PROTECT_IIN_LIMIT            100
#define ASW_PROTECT_IIN_LIMIT_NO         0

enum asw_protect_flags {
	ASW_PROTECT_FLAG_BEGINE = 0,

	ASW_PROTECT_FLAG_DEFAULT = ASW_PROTECT_FLAG_BEGINE,
	ASW_PROTECT_FLAG_START,         /* asw protect happened */
	ASW_PROTECT_FLAG_DO,            /* battery voltage below vbat_max */
	ASW_PROTECT_FLAG_DONE,          /* asw protect battery charge done */

	ASW_PROTECT_FLAG_END
};

int asw_get_iin_limit(void);
void asw_set_iin_limit(int value);

int asw_get_protect_status(void);
int asw_get_protect_voltage(void);
int asw_get_protect_backup_flag(void);

void asw_protect_charge_done(struct smartstar_coul_device *di,
	struct hw_coul_nv_info *nv_info);

void asw_protect_do_work(struct work_struct *work);

int asw_protect_load_batt_data(void);

void asw_protect_do(struct smartstar_coul_device *di, long value);
void asw_protect_check(struct smartstar_coul_device *di);
void asw_protect_exit(struct smartstar_coul_device *di);

#endif /* _ASW_PROTECT_H_ */
