/*
 * crbroi.h
 *
 * cross-reboot readonly info trasfer head file
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#ifndef __CRBROI_H__
#define __CRBROI_H__

/*
 * storage cross-reboot readonly info.
 */
union storage_crbroi {
	unsigned int crbroi;

	struct {
		unsigned short round;
		unsigned short method;
	} crbroi_mem;

	struct {
		unsigned char round : 3;
		unsigned char method : 5;
	} crbroi_pmu;
};

struct storage_crbroi_func {
	int (*crbroi_func_init)(void);
	int (*transfer_crbroi)(unsigned int method);
	int (*get_crbroi_method)(unsigned int *method);
	int (*get_crbroi_round)(unsigned int *round);
	int (*clr_crbroi)(void);
	int (*crbroi_func_exit)(void);
};

/*
 * return the single instance pointer
 */
struct storage_crbroi_func *get_storage_crbroi_func(void);

#endif
