/*
 * usbswitch_common.h
 *
 * common interface for usbswitch driver
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

#ifndef _USBSWITCH_COMMON_H_
#define _USBSWITCH_COMMON_H_

#include <linux/types.h>

struct usbswitch_common_ops {
	int (*manual_switch)(int input_select);
	int (*dcd_timeout_enable)(bool enable_flag);
	int (*dcd_timeout_status)(void);
	int (*manual_detach)(void);
};

int usbswitch_common_ops_register(struct usbswitch_common_ops *ops);

int usbswitch_common_manual_sw(int input_select);
int usbswitch_common_manual_detach(void);
int usbswitch_common_dcd_timeout_enable(bool enable_flag);
int usbswitch_common_dcd_timeout_status(void);

#endif /* _USBSWITCH_COMMON_H_ */
