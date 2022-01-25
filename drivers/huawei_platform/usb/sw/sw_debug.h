/*
 * sw_debug.h
 *
 * sw debug function define
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

#ifndef _SW_DEBUG_H_
#define _SW_DEBUG_H_

#include "sw_core.h"

#ifdef CONFIG_HUAWEI_PLATFORM
#include <huawei_platform/log/log_exception.h>
#include <huawei_platform/log/hw_log.h>
#define HWLOG_TAG sw_core
HWLOG_REGIST();
#else
#define hwlog_debug(fmt, args...) pr_debug("[sw_core]" fmt, ## args)
#define hwlog_info(fmt, args...)  pr_info("[sw_core]" fmt, ## args)
#define hwlog_warn(fmt, args...)  pr_warn("[sw_core]" fmt, ## args)
#define hwlog_err(fmt, args...)   pr_err("[sw_core]" fmt, ## args)
#endif /* CONFIG_HUAWEI_PLATFORM */

enum sw_loglevel_type {
	SW_LOG_ALERT = 0,
	SW_LOG_ERR,
	SW_LOG_WARNING,
	SW_LOG_INFO,
	SW_LOG_DEBUG,
};

extern enum sw_loglevel_type g_sw_loglevel;

#define SW_PRINT_FUNCTION_NAME do { \
		if (g_sw_loglevel >= SW_LOG_DEBUG) { \
			hwlog_debug("func: %s", __func__); \
		} \
	} while (0)

#define SW_PRINT_DBG(s, args...) do { \
		if (g_sw_loglevel >= SW_LOG_DEBUG) { \
			hwlog_debug("func: %s," s, \
				__func__, ## args); \
		} \
	} while (0)

#define SW_PRINT_INFO(s, args...) do { \
		if (g_sw_loglevel >= SW_LOG_INFO) { \
			hwlog_info("func: %s," s, \
				__func__, ## args); \
		} \
	} while (0)

#define SW_PRINT_SUC(s, args...) do { \
		if (g_sw_loglevel >= SW_LOG_INFO) { \
			hwlog_info("func: %s," s, \
				__func__, ## args); \
		} \
	} while (0)

#define SW_PRINT_WARNING(s, args...) do { \
		if (g_sw_loglevel >= SW_LOG_WARNING) { \
			hwlog_warn("func: %s," s, \
				__func__, ## args); \
		} \
	} while (0)

#define SW_PRINT_ERR(s, args...) do { \
		if (g_sw_loglevel >= SW_LOG_ERR) { \
			hwlog_err("func: %s," s, \
				__func__, ## args); \
		} \
	} while (0)

#define SW_PRINT_ALERT(s, args...) do { \
		if (g_sw_loglevel >= SW_LOG_ALERT) { \
			hwlog_warn("func: %s," s, \
				__func__, ## args); \
		} \
	} while (0)

struct sw_debug_list {
	char *hid_debug_buf;
	int head;
	int tail;
	struct fasync_struct *fasync;
	struct sw_device *hdev;
	struct list_head node;
	struct mutex read_mutex;
};

extern void sw_debug_dump_data(const u8 *data, int count);

#endif /* _SW_DEBUG_H_ */
