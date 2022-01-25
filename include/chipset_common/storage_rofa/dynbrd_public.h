/*
 * dynbrd_public.h
 *
 * dynbrd routines header file
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

#ifndef DYNBRD_PUBLIC_H
#define DYNBRD_PUBLIC_H

struct bfmr_dbrd_ioctl_block;

int create_dynamic_ramdisk(struct bfmr_dbrd_ioctl_block *arg_u);
int delete_dynamic_ramdisk(struct bfmr_dbrd_ioctl_block *arg_u);

#endif // DYNBRD_PUBLIC_H

