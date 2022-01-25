/*
 * usbswitch_fsa9685.c
 *
 * usbswitch with fsa9685 driver
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

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/param.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/wakelock.h>

#include "switch_chip.h"
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/usb/switch/switch_fsa9685.h>

#define HWLOG_TAG usbswitch_fsa9685
HWLOG_REGIST();

static const int usbswitch_fsa9685_regaddrs[] = {
	FSA9685_REG_DEVICE_ID,
	FSA9685_REG_CONTROL,
	/* FSA9685_REG_INTERRUPT, */
	FSA9685_REG_INTERRUPT_MASK,
	FSA9685_REG_ADC,
	FSA9685_REG_TIMING_SET_1,
	FSA9685_REG_DETACH_CONTROL,
	FSA9685_REG_DEVICE_TYPE_1,
	FSA9685_REG_DEVICE_TYPE_2,
	FSA9685_REG_DEVICE_TYPE_3,
	FSA9685_REG_MANUAL_SW_1,
	FSA9685_REG_MANUAL_SW_2,

	FSA9685_REG_TIMING_SET_2,
};

static void usbswitch_fsa9685_detach_work(void)
{
	int ret;

	hwlog_info("detach_work entry\n");

	ret = fsa9685_common_read_reg(FSA9685_REG_DETACH_CONTROL);
	if (ret < 0)
		return;

	ret = fsa9685_common_write_reg(FSA9685_REG_DETACH_CONTROL, 1);
	if (ret < 0)
		return;

	hwlog_info("detach_work end\n");
}

static int usbswitch_fsa9685_manual_switch(int input_select)
{
	int value;
	int ret;

	switch (input_select) {
	case FSA9685_USB1_ID_TO_IDBYPASS:
		value = REG_VAL_FSA9685_USB1_ID_TO_IDBYPASS;
		break;

	case FSA9685_USB2_ID_TO_IDBYPASS:
		value = REG_VAL_FSA9685_USB2_ID_TO_IDBYPASS;
		break;

	case FSA9685_UART_ID_TO_IDBYPASS:
		value = REG_VAL_FSA9685_UART_ID_TO_IDBYPASS;
		break;

	case FSA9685_MHL_ID_TO_CBUS:
		value = REG_VAL_FSA9685_MHL_ID_TO_CBUS;
		break;

	case FSA9685_USB1_ID_TO_VBAT:
		value = REG_VAL_FSA9685_USB1_ID_TO_VBAT;
		break;

	/* fall through: default is open */
	case FSA9685_OPEN:
	default:
		value = REG_VAL_FSA9685_OPEN;
		break;
	}

	ret = fsa9685_common_write_reg(FSA9685_REG_MANUAL_SW_1, value);
	if (ret < 0) {
		ret = -ERR_FSA9685_REG_MANUAL_SW_1;
		return ret;
	}

	value = fsa9685_common_read_reg(FSA9685_REG_CONTROL);
	if (value < 0) {
		ret = -ERR_FSA9685_READ_REG_CONTROL;
		return ret;
	}

	value &= (~FSA9685_MANUAL_SW); /* 0: manual switching */

	ret = fsa9685_common_write_reg(FSA9685_REG_CONTROL, value);
	if (ret < 0) {
		ret = -ERR_FSA9685_WRITE_REG_CONTROL;
		return ret;
	}

	return 0;
}

static int usbswitch_fsa9685_switchctrl_store(struct i2c_client *client,
	int action)
{
	switch (action) {
	case MANUAL_DETACH:
		hwlog_info("manual_detach\n");
		usbswitch_common_manual_detach();
		break;

	case MANUAL_SWITCH:
		hwlog_info("manual_switch(usb1_id_to_vbat)\n");
		usbswitch_common_manual_sw(FSA9685_USB1_ID_TO_VBAT);
		break;

	default:
		hwlog_err("wrong input action\n");
		return -1;
	}

	return 0x60; /* 0x60 is default store size */
}

static int usbswitch_fsa9685_switchctrl_show(char *buf)
{
	int device_type1;
	int device_type2;
	int device_type3;
	int mode = -1;
	int tmp;

	device_type1 = fsa9685_common_read_reg(FSA9685_REG_DEVICE_TYPE_1);
	if (device_type1 < 0)
		goto fail_read_reg;

	device_type2 = fsa9685_common_read_reg(FSA9685_REG_DEVICE_TYPE_2);
	if (device_type2 < 0)
		goto fail_read_reg;

	device_type3 = fsa9685_common_read_reg(FSA9685_REG_DEVICE_TYPE_3);
	if (device_type3 < 0)
		goto fail_read_reg;

	hwlog_info("type1=0x%x type2=0x%x type3=0x%x\n",
		device_type1, device_type2, device_type3);

	/* 16: word, 8: byte */
	tmp = device_type3 << 16 | device_type2 << 8 | device_type1;
	mode = 0;
	while (tmp >> mode)
		mode++;

fail_read_reg:
	return scnprintf(buf, PAGE_SIZE, "%d\n", mode);
}

static int usbswitch_fsa9685_jigpin_ctrl_store(struct i2c_client *client,
	int jig_val)
{
	int ret;

	ret = fsa9685_common_write_reg_mask(FSA9685_REG_CONTROL,
		0, FSA9685_MANUAL_SW);
	if (ret < 0)
		return ret;

	if (client->addr == FSA9683_I2C_ADDR ||
		client->addr == CBTL9689_I2C_ADDR) {
		ret = fsa9685_common_write_reg_mask(
			FSA9685_REG_WD_CTRL,
			FSA9685_WD_CTRL_JIG_MANUAL_EN,
			FSA9685_WD_CTRL_JIG_MANUAL_EN);
		if (ret < 0)
			return ret;
	}

	switch (jig_val) {
	case JIG_PULL_DEFAULT_DOWN:
		hwlog_info("pull down jig pin to default state\n");

		if (client->addr == FSA9683_I2C_ADDR) {
			ret = fsa9685_common_write_reg_mask(
				FSA9685_REG_MANUAL_SW_2,
				FSA9683_REG_JIG_DEFAULT_DOWN,
				FSA9685_REG_JIG_MASK);
			if (ret < 0)
				return ret;
		} else {
			ret = fsa9685_common_write_reg_mask(
				FSA9685_REG_MANUAL_SW_2,
				FSA9685_REG_JIG_DEFAULT_DOWN,
				FSA9685_REG_JIG_MASK);
			if (ret < 0)
				return ret;
		}
		break;

	case JIG_PULL_UP:
		hwlog_info("pull up jig pin to cut battery\n");

		if (client->addr == FSA9683_I2C_ADDR) {
			ret = fsa9685_common_write_reg_mask(
				FSA9685_REG_MANUAL_SW_2,
				FSA9683_REG_JIG_UP,
				FSA9685_REG_JIG_MASK);
			if (ret < 0)
				return ret;
		} else {
			ret = fsa9685_common_write_reg_mask(
				FSA9685_REG_MANUAL_SW_2,
				FSA9685_REG_JIG_UP,
				FSA9685_REG_JIG_MASK);
			if (ret < 0)
				return ret;
		}
		break;

	default:
		hwlog_err("wrong input action\n");
		return -1;
	}

	return 0x60; /* 0x60 is default store size */
}

static int usbswitch_fsa9685_jigpin_ctrl_show(char *buf)
{
	int manual_sw2_val;

	manual_sw2_val = fsa9685_common_read_reg(FSA9685_REG_MANUAL_SW_2);

	return scnprintf(buf, PAGE_SIZE, "%02x\n", manual_sw2_val);
}

static int usbswitch_fsa9685_dump_regs(char *buf)
{
	int i;
	int val;
	char rd_buf[FSA9685_RD_BUF_SIZE] = {0};
	int size = ARRAY_SIZE(usbswitch_fsa9685_regaddrs);

	scnprintf(rd_buf, FSA9685_RD_BUF_SIZE, "dump_regs: fsa9685\n");
	strncat(buf, rd_buf, strlen(rd_buf));

	for (i = 0; i < size; i++) {
		val = fsa9685_common_read_reg(usbswitch_fsa9685_regaddrs[i]);

		memset(rd_buf, 0, FSA9685_RD_BUF_SIZE);
		scnprintf(rd_buf, FSA9685_RD_BUF_SIZE, "[0x%02x]: 0x%02x\n",
			usbswitch_fsa9685_regaddrs[i], val);
		strncat(buf, rd_buf, strlen(rd_buf));
	}

	return strlen(buf);
}

static struct fsa9685_device_ops usbswitch_fsa9685_ops = {
	.dump_regs = usbswitch_fsa9685_dump_regs,
	.jigpin_ctrl_show = usbswitch_fsa9685_jigpin_ctrl_show,
	.jigpin_ctrl_store = usbswitch_fsa9685_jigpin_ctrl_store,
	.switchctrl_store = usbswitch_fsa9685_switchctrl_store,
	.switchctrl_show = usbswitch_fsa9685_switchctrl_show,

	.manual_switch = usbswitch_fsa9685_manual_switch,

	.detach_work = usbswitch_fsa9685_detach_work,
};

struct fsa9685_device_ops *usbswitch_fsa9685_get_device_ops(void)
{
	return &usbswitch_fsa9685_ops;
}
