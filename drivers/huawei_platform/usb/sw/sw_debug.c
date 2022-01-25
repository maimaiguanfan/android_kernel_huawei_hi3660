/*
 * sw_debug.c
 *
 * sw debug fuction
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

#include "sw_debug.h"
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
#include <linux/crc16.h>
#include <linux/crc-ccitt.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/version.h>
#include <huawei_platform/log/log_exception.h>

#define SW_DEBUG_BUFSIZE             (512)
#define SW_DEBUG_DATA_COUNT_ONE_LINE (10)

enum sw_loglevel_type g_sw_loglevel = SW_LOG_DEBUG;

void sw_debug_dump_data(const u8 *data, int count)
{
	int i;

	if ((count < 1) || (data == NULL))
		return;

	SW_PRINT_DBG("[sw_report]len %d\n", count);
	SW_PRINT_DBG("{ ");
	for (i = 0; i < count; i++) {
		SW_PRINT_DBG("%x ", data[i]);
		if ((i > 0) && (i % SW_DEBUG_DATA_COUNT_ONE_LINE == 0))
			SW_PRINT_DBG("\n");
	}
	SW_PRINT_DBG("}\n");
}
