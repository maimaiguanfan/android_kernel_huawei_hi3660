/*
 * power_common_sh.h
 *
 * power common include file for sensorhub or inputhub
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

#ifndef _POWER_COMMON_SH_H_
#define _POWER_COMMON_SH_H_

#ifdef CONFIG_INPUTHUB
#include <protocol.h>

extern sys_status_t iom3_sr_status;
extern atomic_t iom3_rec_state;
#endif

#endif /* _POWER_COMMON_SH_H_ */
