/*
 * sw_hid_vendor.c
 *
 * single-wire hid vendor
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

#include <linux/kref.h>
#include <linux/module.h>
#include <linux/file.h>
#include <linux/kthread.h>
#include <linux/hidraw.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/tty.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <huawei_platform/log/log_exception.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include "sw_core.h"
#include "sw_debug.h"

int sw_drivers_init(void)
{
	int ret;

	ret = sw_bus_init();
	if (ret)
		goto err_bus;

	swhid_init();
	cmrkb_init();
	scmkb_init();

	return 0;

err_bus:
	sw_bus_exit();
	return ret;
}

void sw_drivers_exit(void)
{
	scmkb_exit();
	cmrkb_exit();
	swhid_exit();
	sw_bus_exit();
}
