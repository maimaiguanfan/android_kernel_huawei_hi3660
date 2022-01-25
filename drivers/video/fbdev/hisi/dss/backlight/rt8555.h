/*
 * rt8555.h
 *
 * rt8555 driver for backlight
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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

#ifndef _BL_RT8555_H_
#define _BL_RT8555_H_

#include "hisi_fb.h"
#include "../hisi_fb_panel.h"
#if defined(CONFIG_LCDKIT_DRIVER)
#include "lcdkit_panel.h"
#endif
#include <linux/hisi/hw_cmdline_parse.h>

#define RT8555_NAME             "rt8555"
#define DTS_COMP_RT8555         "realtek,rt8555"
#define GPIO_RT8555_EN_NAME     "rt8555_hw_en"

#define RT8555_BL_MIN        0
#define RT8555_BL_MAX        1023
#define RT8555_RW_REG_MAX    10

/* rt8555 reg address */
#define RT8555_CONTROL_MODE_ADDR           0x00
#define RT8555_CURRENT_PROTECTION_ADDR     0x01
#define RT8555_CURRENT_SETTING_ADDR        0x02
#define RT8555_VOLTAGE_SETTING_ADDR        0x03
#define RT8555_BRIGHTNESS_SETTING_ADDR     0x08
#define RT8555_TIME_CONTROL_ADDR           0x09
#define RT8555_MODE_DEVISION_ADDR          0x0A
#define RT8555_COMPENSATION_DUTY_ADDR      0x0B
#define RT8555_CLK_PFM_ENABLE_ADDR         0x0D
#define RT8555_LED_PROTECTION_ADDR         0x0E

#define PARSE_FAILED        0xffff
#define DELAY_0_US          0
#define DELAY_5_MS          5
#define DELAY_10_US         10
#define DELAY_50_US         50
#define DEFAULT_MSG_LEVEL   7
#define RT8555_DISABLE_DELAY               60

#ifndef BIT
#define BIT(x)  (1<<(x))
#endif

#define RT8555_EMERG(dev, msg, ...) \
	do { \
		if (g_rt8555_msg_level > 0) \
			dev_emerg(dev, "%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)
#define RT8555_ALERT(dev, msg, ...) \
	do { \
		if (g_rt8555_msg_level > 1) \
			dev_alert(dev, "%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)
#define RT8555_CRIT(dev, msg, ...) \
	do { \
		if (g_rt8555_msg_level > 2) \
			dev_crit(dev, "%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)
#define RT8555_ERR(dev, msg, ...) \
	do { \
		if (g_rt8555_msg_level > 3) \
			dev_err(dev, "%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)
#define RT8555_WARNING(dev, msg, ...) \
	do { \
		if (g_rt8555_msg_level > 4) \
			dev_warn(dev, "%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)
#define RT8555_NOTICE(dev, msg, ...) \
	do { \
		if (g_rt8555_msg_level > 5) \
			dev_notice(dev, "%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)
#define RT8555_INFO(dev, msg, ...) \
	do { \
		if (g_rt8555_msg_level > 6) \
			_dev_info(dev, "%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)
#define RT8555_DEBUG(dev, msg, ...) \
	do { \
		if (g_rt8555_msg_level > 7) \
			_dev_info(dev, "%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

struct rt8555_chip_data {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct semaphore test_sem;
};
struct rt8555_info {
	/* whether support rt8555 or not */
	int rt8555_support;
	/* which i2c bus controller rt8555 mount */
	int rt8555_i2c_bus_id;
	/* rt8555 hw_en gpio */
	int rt8555_hw_en_gpio;
	uint32_t reg[RT8555_RW_REG_MAX];
	int bl_on_kernel_mdelay;
	int rt8555_level_lsb;
	int rt8555_level_msb;
	int bl_led_num;
};

ssize_t rt8555_set_backlight_init(uint32_t bl_value);
bool is_rt8555_used(void);

#endif /* _BL_RT8555_H */


