/*
 * dynbrd.c
 *
 * Dynamic brd creation interface
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

#include <linux/errno.h>
#include <linux/uaccess.h>

#include <chipset_common/bfmr/bfm/core/bfm_core.h>
#include "brd_import.h"

int create_dynamic_ramdisk(struct bfmr_dbrd_ioctl_block *arg_u)
{
	struct bfmr_dbrd_ioctl_block ioc;
	int rc = -EFAULT;

	if (copy_from_user(&ioc, arg_u, sizeof(ioc)))
		return rc;

	ioc.number = brd_create(ioc.kbytes);
	if (ioc.number == -1)
		return -EINVAL;

	if (copy_to_user(arg_u, &ioc, sizeof(ioc)))
		return rc;

	return 0;
}

int delete_dynamic_ramdisk(struct bfmr_dbrd_ioctl_block *arg_u)
{
	struct bfmr_dbrd_ioctl_block ioc;
	int rc = -EFAULT;

	if (copy_from_user(&ioc, arg_u, sizeof(struct bfmr_dbrd_ioctl_block)))
		return rc;

	return brd_delete(ioc.number);
}
