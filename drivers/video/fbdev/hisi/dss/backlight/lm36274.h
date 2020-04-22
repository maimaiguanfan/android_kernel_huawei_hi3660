/*
* Simple driver for Texas Instruments LM3630 LED Flash driver chip
* Copyright (C) 2012 Texas Instruments
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
*/

#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
extern struct dsm_client *lcd_dclient;
#endif

#ifndef __LINUX_LM36274_H
#define __LINUX_LM36274_H

#define LM36274_NAME "lm36274"
#define DTS_COMP_LM36274 "ti,lm36274"
#define LM36274_HIDDEN_REG_SUPPORT "lm36274_hidden_reg_support"
#define LM36274_RUNNING_RESUME_BL_TIMMER   "lm36274_running_resume_bl_timmer"
#define LM36274_UPDATE_RESUME_BL_TIMMER   "lm36274_update_resume_bl_timmer"

#define MAX_RATE_NUM 9
/* base reg */
#define REG_BL_CONFIG_1 0x02
#define REG_BL_CONFIG_2 0x03
#define REG_BL_BRIGHTNESS_LSB 0x04
#define REG_BL_BRIGHTNESS_MSB 0x05
#define REG_AUTO_FREQ_LOW 0x06
#define REG_AUTO_FREQ_HIGH 0x07
#define REG_BL_ENABLE 0x08
#define REG_DISPLAY_BIAS_CONFIG_1 0x09
#define REG_DISPLAY_BIAS_CONFIG_2 0x0A
#define REG_DISPLAY_BIAS_CONFIG_3 0x0B
#define REG_LCM_BOOST_BIAS 0x0C
#define REG_VPOS_BIAS 0x0D
#define REG_VNEG_BIAS 0x0E
#define REG_FLAGS 0x0F
#define REG_BL_OPTION_1 0x10
#define REG_BL_OPTION_2 0x11
#define REG_PWM_TO_DIGITAL_LSB 0x12
#define REG_PWM_TO_DIGITAL_MSB 0x13
#define REG_HIDDEN_ADDR 0x6A
#define REG_SET_SECURITYBIT_ADDR 0x50
#define REG_SET_SECURITYBIT_VAL  0x08
#define REG_CLEAR_SECURITYBIT_VAL  0x00

/* mask code */
#define MASK_BL_LSB 0x07
#define MASK_LCM_EN 0xE0
#define MASK_SOFTWARE_RESET 0x80

/* update bit val */
#define DEVICE_FAULT_OCCUR 0
#define DEVICE_RESET 0x1

#define BL_MIN 0

#define MSB 3
#define LSB 0x07

#ifndef BIT
#define BIT(x)  (1<<(x))
#endif

#define LM36274_EMERG(msg, ...)    \
	do { if (lm36274_msg_level > 0)  \
		printk(KERN_EMERG "[lm36274]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define LM36274_ALERT(msg, ...)    \
	do { if (lm36274_msg_level > 1)  \
		printk(KERN_ALERT "[lm36274]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define LM36274_CRIT(msg, ...)    \
	do { if (lm36274_msg_level > 2)  \
		printk(KERN_CRIT "[lm36274]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define LM36274_ERR(msg, ...)    \
	do { if (lm36274_msg_level > 3)  \
		printk(KERN_ERR "[lm36274]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define LM36274_WARNING(msg, ...)    \
	do { if (lm36274_msg_level > 4)  \
		printk(KERN_WARNING "[lm36274]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define LM36274_NOTICE(msg, ...)    \
	do { if (lm36274_msg_level > 5)  \
		printk(KERN_NOTICE "[lm36274]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define LM36274_INFO(msg, ...)    \
	do { if (lm36274_msg_level > 6)  \
		printk(KERN_INFO "[lm36274]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define LM36274_DEBUG(msg, ...)    \
	do { if (lm36274_msg_level > 7)  \
		printk(KERN_DEBUG "[lm36274]%s: "msg, __func__, ## __VA_ARGS__); } while (0)

#define LM36274_VOL_400 (0x00) //4.0V
#define LM36274_VOL_405 (0x01) //4.05V
#define LM36274_VOL_410 (0x02) //4.1V
#define LM36274_VOL_415 (0x03) //4.15V
#define LM36274_VOL_420 (0x04) //4.2V
#define LM36274_VOL_425 (0x05) //4.25V
#define LM36274_VOL_430 (0x06) //4.3V
#define LM36274_VOL_435 (0x07) //4.35V
#define LM36274_VOL_440 (0x08) //4.4V
#define LM36274_VOL_445 (0x09) //4.45V
#define LM36274_VOL_450 (0x0A) //4.5V
#define LM36274_VOL_455 (0x0B) //4.55V
#define LM36274_VOL_460 (0x0C) //4.6V
#define LM36274_VOL_465 (0x0D) //4.65V
#define LM36274_VOL_470 (0x0E) //4.7V
#define LM36274_VOL_475 (0x0F) //4.75V
#define LM36274_VOL_480 (0x10) //4.8V
#define LM36274_VOL_485 (0x11) //4.85V
#define LM36274_VOL_490 (0x12) //4.9V
#define LM36274_VOL_495 (0x13) //4.95V
#define LM36274_VOL_500 (0x14) //5.0V
#define LM36274_VOL_505 (0x15) //5.05V
#define LM36274_VOL_510 (0x16) //5.1V
#define LM36274_VOL_515 (0x17) //5.15V
#define LM36274_VOL_520 (0x18) //5.2V
#define LM36274_VOL_525 (0x19) //5.25V
#define LM36274_VOL_560 (0x20) //5.6V
#define LM36274_VOL_565 (0x21) //5.65V
#define LM36274_VOL_570 (0x22) //5.7V
#define LM36274_VOL_575 (0x23) //5.75V
#define LM36274_VOL_580 (0x24) //5.8V
#define LM36274_VOL_585 (0x25) //5.85V
#define LM36274_VOL_590 (0x26) //5.9V
#define LM36274_VOL_595 (0x27) //5.95V
#define LM36274_VOL_600 (0x28) //6.0V
#define LM36274_VOL_605 (0x29) //6.05V
#define LM36274_VOL_640 (0x30) //6.4V
#define LM36274_VOL_645 (0x31) //6.45V
#define LM36274_VOL_650 (0x32) //6.5V


struct lm36274_chip_data {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct semaphore test_sem;
	struct work_struct bl_resume_worker;
	struct workqueue_struct *bl_resume_wq;
	struct hrtimer bl_resume_hrtimer;
};

#define LM36274_RW_REG_MAX 13

struct lm36274_vsp_vsn_voltage {
    u32 voltage;
    int value;
};

enum bl_enable{
    EN_2_SINK = 0x15,
    EN_4_SINK = 0x1F,
    BL_RESET = 0x80,
    BL_DISABLE = 0x00,
};

enum lcm_en {
    NORMAL_MODE = 0x80,
    BIAS_SUPPLY_OFF = 0x00,
};

enum {
    BL_OVP_25V = 0x40,
    BL_OVP_29V = 0x60,
};

enum {
    CURRENT_RAMP_0US = 0x85,
    CURRENT_RAMP_5MS = 0xAD,
};

enum {
    LSB_MIN = 0x00,
    LSB_10MA = 0x05,
    LSB_30MA = 0x07,
};

enum {
    MSB_MIN = 0x00,
    MSB_10MA = 0xD2,
    MSB_30MA = 0xFF,
};

enum bl_option_2{
    BL_OCP_1 = 0x35,    /*1.2A */
    BL_OCP_2 = 0x36,    /*1.5A */
    BL_OCP_3 = 0x37,    /*1.8A */
};

enum {
    BIAS_BOOST_TIME_0 = 0x00, /*156ns*/
    BIAS_BOOST_TIME_1 = 0x10, /*181ns*/
    BIAS_BOOST_TIME_2 = 0x20, /*206ns*/
    BIAS_BOOST_TIME_3 = 0x30, /*231ns*/
};

enum resume_type {
	RESUME_IDLE = 0x0,
	RESUME_2_SINK = 0x1,
	RESUME_REMP_OVP_OCP = 0x2,
};

#define BL_CONFIG_MODE_REG_NUM 3
#define BL_CONFIG_CURR_REG_NUM 2
#define BL_CONFIG_ENABLE_REG_NUM 1
#define BL_LOWER_POW_DELAY 6
#define BL_MAX_PREFLASHING_TIMER 800

#define FLAG_CHECK_NUM         3
enum {
	OVP_FAULT_FLAG = 0x02,
	OCP_FAULT_FLAG = 0x01,
	TSD_FAULT_FLAG = 0x40,
};

struct lm36274_check_flag {
	u8 flag;
	int err_no;
};

static struct lm36274_check_flag err_table[] = {
	{OVP_FAULT_FLAG, DSM_LCD_OVP_ERROR_NO},
	{OCP_FAULT_FLAG, DSM_LCD_BACKLIGHT_OCP_ERROR_NO},
	{TSD_FAULT_FLAG, DSM_LCD_BACKLIGHT_TSD_ERROR_NO},
};

/* bl_mode_config reg */
#define BL_MAX_CONFIG_REG_NUM 3

struct bl_config_reg
{
    unsigned int reg_element_num;
    unsigned int reg_addr[BL_MAX_CONFIG_REG_NUM];
    unsigned int normal_reg_var[BL_MAX_CONFIG_REG_NUM];
    unsigned int enhance_reg_var[BL_MAX_CONFIG_REG_NUM];
};

struct backlight_work_mode_reg_info
{
    struct bl_config_reg bl_mode_config_reg;
    struct bl_config_reg bl_current_config_reg;
    struct bl_config_reg bl_enable_config_reg;
};

int lm36274_set_backlight_reg(unsigned int bl_level);
ssize_t lm36274_set_reg(u8 bl_reg,u8 bl_mask,u8 bl_val);

#endif /* __LINUX_LM36274_H */

