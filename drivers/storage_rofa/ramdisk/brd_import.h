/*
 * brd_import.h
 *
 * routines imported from drivers/block/brd.c
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

#ifndef BRD_IMPORT_H
#define BRD_IMPORT_H

#ifdef CONFIG_HUAWEI_DYNAMIC_BRD
int brd_create(unsigned int size);
int brd_delete(int nr);
#endif

#endif
