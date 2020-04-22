/*
 * rt9748_main.c
 *
 * rt9748 driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/raid/pq.h>

#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/power/direct_charger.h>
#include "../dual_loadswitch.h"
#include "rt9748.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG rt9748_main
HWLOG_REGIST();

static struct rt9748_device_info *g_rt9748_dev;

static int g_get_id_time;

#define MSG_LEN                      (2)

static int rt9748_write_block(struct rt9748_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[1];
	int ret = 0;

	if (di == NULL || value == NULL) {
		hwlog_err("di is null or value is null\n");
		return -EIO;
	}

	if (di->chip_already_init == 0) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	*value = reg;

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = value;
	msg[0].len = num_bytes + 1;

	ret = i2c_transfer(di->client->adapter, msg, 1);

	/* i2c_transfer returns number of messages transferred */
	if (ret != 1) {
		hwlog_err("write_block failed[%x]\n", reg);
		if (ret < 0)
			return ret;
		else
			return -EIO;
	} else {
		return 0;
	}
}

static int rt9748_read_block(struct rt9748_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[MSG_LEN];
	u8 buf = 0;
	int ret = 0;

	if (di == NULL || value == NULL) {
		hwlog_err("di is null or value is null\n");
		return -EIO;
	}

	if (di->chip_already_init == 0) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	buf = reg;

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = &buf;
	msg[0].len = 1;

	msg[1].addr = di->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = value;
	msg[1].len = num_bytes;

	ret = i2c_transfer(di->client->adapter, msg, MSG_LEN);

	/* i2c_transfer returns number of messages transferred */
	if (ret != MSG_LEN) {
		hwlog_err("read_block failed[%x]\n", reg);
		if (ret < 0)
			return ret;
		else
			return -EIO;
	} else {
		return 0;
	}
}

static int rt9748_write_byte(u8 reg, u8 value)
{
	struct rt9748_device_info *di = g_rt9748_dev;
	/* 2 bytes offset 1 contains the data offset 0 is used by i2c_write */
	u8 temp_buffer[MSG_LEN] = {0};

	/* offset 1 contains the data */
	temp_buffer[1] = value;
	return rt9748_write_block(di, temp_buffer, reg, 1);
}

static int rt9748_read_byte(u8 reg, u8 *value)
{
	struct rt9748_device_info *di = g_rt9748_dev;

	return rt9748_read_block(di, value, reg, 1);
}

static int rt9748_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret = 0;
	u8 val = 0;

	ret = rt9748_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	ret = rt9748_write_byte(reg, val);

	return ret;
}

static int rt9748_reg_init(void)
{
	int ret;
	u8 val;

	switch (g_rt9748_dev->device_id) {
	case loadswitch_rt9748:
		ret = rt9748_write_byte(RT9748_EVENT_1_MASK,
			RT9748_EVENT_1_MASK_INIT);
		ret |= rt9748_write_byte(RT9748_EVENT_2_MASK,
			RT9748_EVENT_2_MASK_INIT);
		ret |= rt9748_write_byte(RT9748_EVENT_1_EN,
			RT9748_EVENT_1_EN_INIT);
		ret |= rt9748_write_byte(RT9748_CONTROL,
			RT9748_CONTROL_INIT);
		ret |= rt9748_write_byte(RT9748_ADC_CTRL,
			RT9748_ADC_CTRL_INIT);
		ret |= rt9748_write_byte(RT9748_SAMPLE_EN,
			RT9748_SAMPLE_EN_INIT);
		ret |= rt9748_write_mask(RT9748_PROT_DLY_OCP,
			RT9748_REG_INIT_MASK, RT9748_REG_INIT_SHIFT,
			RT9748_PROT_DLY_OCP_INIT);
		if (ret)
			return -1;

		rt9748_write_byte(RT9748_REV_CURRENT_SELECT,
			RT9748_REV_CURRENT_SELECT_INIT);
		rt9748_read_byte(RT9748_REV_CURRENT_SELECT, &val);
		hwlog_info("rt9748 reg0x26 = 0x%x\n", val);
		if (val != RT9748_REV_CURRENT_SELECT_INIT)
			hwlog_err("rt9748 write 0x26 fail\n");

		break;

	case loadswitch_bq25870:
		ret = rt9748_write_byte(BQ25870_EVENT_1_MASK,
			BQ25870_EVENT_1_MASK_INIT);
		ret |= rt9748_write_byte(BQ25870_EVENT_2_MASK,
			BQ25870_EVENT_2_MASK_INIT);
		ret |= rt9748_write_byte(BQ25870_EVENT_1_EN,
			BQ25870_EVENT_1_EN_INIT);
		ret |= rt9748_write_byte(BQ25870_CONTROL,
			BQ25870_CONTROL_INIT);
		ret |= rt9748_write_byte(BQ25870_ADC_CTRL,
			BQ25870_ADC_CTRL_INIT);
		ret |= rt9748_write_byte(BQ25870_SAMPLE_EN,
			BQ25870_SAMPLE_EN_INIT);
		ret |= rt9748_write_mask(BQ25870_PROT_DLY_OCP,
			BQ25870_REG_INIT_MASK, BQ25870_REG_INIT_SHIFT,
			BQ25870_PROT_DLY_OCP_INIT);
		if (ret)
			return -1;

		break;

	default:
		hwlog_err("device_id is not found\n");
		return -1;
	}

	return 0;
}

static int rt9748_adc_enable(int enable)
{
	int ret;
	u8 value = enable ? 0x1 : 0x0;

	ret = rt9748_write_mask(RT9748_ADC_CTRL,
		RT9748_ADC_EN_MASK, RT9748_ADC_EN_SHIFT,
		value);
	if (ret)
		return -1;

	return 0;
}

static int rt9748_charge_enable(int enable)
{
	u8 reg = 0;
	int ret;
	u8 value = enable ? 0x1 : 0x0;

	ret = rt9748_write_mask(RT9748_CONTROL,
		RT9748_CHARGE_EN_MASK, RT9748_CHARGE_EN_SHIFT,
		value);
	if (ret)
		return -1;

	ret = rt9748_read_byte(RT9748_CONTROL, &reg);
	if (ret)
		return -1;

	hwlog_info("charge_enable [%x]=0x%x\n", RT9748_CONTROL, reg);
	return 0;
}

static int rt9748_discharge(int enable)
{
	u8 reg = 0;
	int ret;
	u8 value = enable ? 0x1 : 0x0;

	ret = rt9748_write_mask(RT9748_EVENT_1_EN,
		RT9748_PD_EN_MASK, RT9748_PD_EN_SHIFT,
		value);
	if (ret)
		return -1;

	ret = rt9748_read_byte(RT9748_EVENT_1_EN, &reg);
	if (ret)
		return -1;

	hwlog_info("event1_en reg = 0x%x\n", reg);
	return 0;
}

static int rt9748_config_ioc_ocp_threshold_ma(int ocp_threshold)
{
	u8 value;
	int ret = 0;

	switch (g_rt9748_dev->device_id) {
	case loadswitch_rt9748:
		if (ocp_threshold < RT9748_IOC_OCP_MIN_0_MA)
			ocp_threshold = RT9748_IOC_OCP_MIN_0_MA;

		if (ocp_threshold > RT9748_IOC_OCP_MAX_6500_MA)
			ocp_threshold = RT9748_IOC_OCP_MAX_6500_MA;

		value = (u8)((ocp_threshold -
			RT9748_IOC_OCP_OFFSET_0_MA) / RT9748_IOC_OCP_STEP);
		hwlog_info("rt9748 ocp_threshold=%d, value=0x%x\n",
			ocp_threshold, value);
		ret = rt9748_write_mask(RT9748_PROT_DLY_OCP,
			RT9748_IOC_OCP_MASK, RT9748_IOC_OCP_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	case loadswitch_bq25870:
		if (ocp_threshold < BQ25870_IOC_OCP_MIN_0_MA)
			ocp_threshold = BQ25870_IOC_OCP_MIN_0_MA;

		if (ocp_threshold > BQ25870_IOC_OCP_MAX_7500_MA)
			ocp_threshold = BQ25870_IOC_OCP_MAX_7500_MA;

		value = (u8)((ocp_threshold -
			BQ25870_IOC_OCP_OFFSET_0_MA) /
			BQ25870_IOC_OCP_STEP);
		hwlog_info("bq25870 ocp_threshold=%d, value=0x%x\n",
			ocp_threshold, value);
		ret = rt9748_write_mask(BQ25870_PROT_DLY_OCP,
			BQ25870_IOC_OCP_MASK, BQ25870_IOC_OCP_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	default:
		hwlog_err("device_id is not found\n");
		return -1;
	}

	return 0;
}

static int rt9748_config_vbus_ovp_threshold_mv(int ovp_threshold)
{
	u8 value;
	int ret = 0;

	switch (g_rt9748_dev->device_id) {
	case loadswitch_rt9748:
		if (ovp_threshold < RT9748_VBUS_OVP_MIN_4200_MV)
			ovp_threshold = RT9748_VBUS_OVP_MIN_4200_MV;

		if (ovp_threshold > RT9748_VBUS_OVP_MAX_6500_MV)
			ovp_threshold = RT9748_VBUS_OVP_MAX_6500_MV;

		value = (u8)((ovp_threshold -
			RT9748_VBUS_OVP_OFFSET_4200_MV) /
			RT9748_VBUS_OVP_STEP);
		hwlog_info("rt9748 ovp_threshold=%d, value=0x%x\n",
			ovp_threshold, value);
		ret = rt9748_write_mask(RT9748_VBUS_OVP,
			RT9748_VBUS_OVP_MASK, RT9748_VBUS_OVP_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	case loadswitch_bq25870:
		if (ovp_threshold < BQ25870_VBUS_OVP_MIN_4200_MV)
			ovp_threshold = BQ25870_VBUS_OVP_MIN_4200_MV;

		if (ovp_threshold > BQ25870_VBUS_OVP_MAX_6510_MV)
			ovp_threshold = BQ25870_VBUS_OVP_MAX_6510_MV;

		value = (u8)((ovp_threshold -
			BQ25870_VBUS_OVP_OFFSET_4200_MV) /
			BQ25870_VBUS_OVP_STEP);
		hwlog_info("bq25870 ovp_threshold=%d, value=0x%x\n",
			ovp_threshold, value);
		ret = rt9748_write_mask(BQ25870_VBUS_OVP,
			BQ25870_VBUS_OVP_MASK, BQ25870_VBUS_OVP_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	default:
		hwlog_err("device_id is not found\n");
		return -1;
	}

	return 0;
}

static int rt9748_config_vout_reg_threshold_mv(int vout_reg_threshold)
{
	u8 value;
	int ret = 0;

	switch (g_rt9748_dev->device_id) {
	case loadswitch_rt9748:
		if (vout_reg_threshold < RT9748_VOUT_REG_MIN_4200_MV)
			vout_reg_threshold = RT9748_VOUT_REG_MIN_4200_MV;

		if (vout_reg_threshold > RT9748_VOUT_REG_MAX_5000_MV)
			vout_reg_threshold = RT9748_VOUT_REG_MAX_5000_MV;

		value = (u8)((vout_reg_threshold -
			RT9748_VOUT_REG_OFFSET_4200_MV) /
			RT9748_VOUT_REG_STEP);
		hwlog_info("rt9748 vout_reg_threshold=%d, value=0x%x\n",
			vout_reg_threshold, value);
		ret = rt9748_write_mask(RT9748_VOUT_REG,
			RT9748_VOUT_REG_MASK, RT9748_VOUT_REG_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	case loadswitch_bq25870:
		if (vout_reg_threshold < BQ25870_VOUT_REG_MIN_4200_MV)
			vout_reg_threshold = BQ25870_VOUT_REG_MIN_4200_MV;

		if (vout_reg_threshold > BQ25870_VOUT_REG_MAX_4975_MV)
			vout_reg_threshold = BQ25870_VOUT_REG_MAX_4975_MV;

		value = (u8)((vout_reg_threshold -
			BQ25870_VOUT_REG_OFFSET_4200_MV) /
			BQ25870_VOUT_REG_STEP);
		hwlog_info("bq25870 vout_reg_threshold=%d, value=0x%x\n",
			vout_reg_threshold, value);
		ret = rt9748_write_mask(BQ25870_VOUT_REG,
			BQ25870_VOUT_REG_MASK, BQ25870_VOUT_REG_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	default:
		hwlog_err("device_id is not found\n");
		return -1;
	}

	return 0;
}

static int rt9748_config_vdrop_ovp_reg_threshold_mv(int vdrop_ovp_threshold)
{
	u8 value;
	int ret = 0;

	switch (g_rt9748_dev->device_id) {
	case loadswitch_rt9748:
		if (vdrop_ovp_threshold < RT9748_VDROP_OVP_MIN_0_MV)
			vdrop_ovp_threshold = RT9748_VDROP_OVP_MIN_0_MV;

		if (vdrop_ovp_threshold > RT9748_VDROP_OVP_MAX_1000_MV)
			vdrop_ovp_threshold = RT9748_VDROP_OVP_MAX_1000_MV;

		value = (u8)((vdrop_ovp_threshold -
			RT9748_VDROP_OVP_OFFSET_0_MV) /
			RT9748_VDROP_OVP_STEP);
		hwlog_info("rt9748 vdrop_ovp_threshold=%d, value=0x%x\n",
			vdrop_ovp_threshold, value);
		ret = rt9748_write_mask(RT9748_VDROP_OVP,
			RT9748_VDROP_OVP_MASK, RT9748_VDROP_OVP_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	case loadswitch_bq25870:
		if (vdrop_ovp_threshold < BQ25870_VDROP_OVP_MIN_0_MV)
			vdrop_ovp_threshold = BQ25870_VDROP_OVP_MIN_0_MV;

		if (vdrop_ovp_threshold > BQ25870_VDROP_OVP_MAX_1000_MV)
			vdrop_ovp_threshold = BQ25870_VDROP_OVP_MAX_1000_MV;

		value = (u8)((vdrop_ovp_threshold -
			BQ25870_VDROP_OVP_OFFSET_0_MV) /
			BQ25870_VDROP_OVP_STEP);
		hwlog_info("bq25870 vdrop_ovp_threshold=%d, value=0x%x\n",
			vdrop_ovp_threshold, value);
		ret = rt9748_write_mask(BQ25870_VDROP_OVP,
			BQ25870_VDROP_OVP_MASK, BQ25870_VDROP_OVP_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	default:
		hwlog_err("device_id is not found\n");
		return -1;
	}

	return 0;
}

static int rt9748_config_vdrop_alm_reg_threshold_mv(int vdrop_alm_threshold)
{
	u8 value;
	int ret = 0;

	switch (g_rt9748_dev->device_id) {
	case loadswitch_rt9748:
		if (vdrop_alm_threshold < RT9748_VDROP_ALM_MIN_0_MV)
			vdrop_alm_threshold = RT9748_VDROP_ALM_MIN_0_MV;

		if (vdrop_alm_threshold > RT9748_VDROP_ALM_MAX_1000_MV)
			vdrop_alm_threshold = RT9748_VDROP_ALM_MAX_1000_MV;

		value = (u8)((vdrop_alm_threshold -
			RT9748_VDROP_ALM_OFFSET_0_MV) /
			RT9748_VDROP_ALM_STEP);
		hwlog_info("rt9748 vdrop_alm_threshold=%d, value=0x%x\n",
			vdrop_alm_threshold, value);
		ret = rt9748_write_mask(RT9748_VDROP_ALM,
			RT9748_VDROP_ALM_MASK, RT9748_VDROP_ALM_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	case loadswitch_bq25870:
		if (vdrop_alm_threshold < BQ25870_VDROP_ALM_MIN_0_MV)
			vdrop_alm_threshold = BQ25870_VDROP_ALM_MIN_0_MV;

		if (vdrop_alm_threshold > BQ25870_VDROP_ALM_MAX_1000_MV)
			vdrop_alm_threshold = BQ25870_VDROP_ALM_MAX_1000_MV;

		value = (u8)((vdrop_alm_threshold -
			BQ25870_VDROP_ALM_OFFSET_0_MV) /
			BQ25870_VDROP_ALM_STEP);
		hwlog_info("bq25870 vdrop_alm_threshold=%d, value=0x%x\n",
			vdrop_alm_threshold, value);
		ret = rt9748_write_mask(BQ25870_VDROP_ALM,
			BQ25870_VDROP_ALM_MASK, BQ25870_VDROP_ALM_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	default:
		hwlog_err("device_id is not found\n");
		return -1;
	}

	return 0;
}

static int rt9748_config_vbat_reg_threshold_mv(int vbat_reg_threshold)
{
	u8 value;
	int ret = 0;

	switch (g_rt9748_dev->device_id) {
	case loadswitch_rt9748:
		if (vbat_reg_threshold < RT9748_VBAT_REG_MIN_4200_MV)
			vbat_reg_threshold = RT9748_VBAT_REG_MIN_4200_MV;

		if (vbat_reg_threshold > RT9748_VBAT_REG_MAX_5000_MV)
			vbat_reg_threshold = RT9748_VBAT_REG_MAX_5000_MV;

		value = (u8)((vbat_reg_threshold -
			RT9748_VBAT_REG_OFFSET_4200_MV) /
			RT9748_VBAT_REG_STEP);
		hwlog_info("rt9748 vbat_reg_threshold=%d, value=0x%x\n",
			vbat_reg_threshold, value);
		ret = rt9748_write_mask(RT9748_VBAT_REG,
			RT9748_VBAT_REG_MASK, RT9748_VBAT_REG_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	case loadswitch_bq25870:
		if (vbat_reg_threshold < BQ25870_VBAT_REG_MIN_4200_MV)
			vbat_reg_threshold = BQ25870_VBAT_REG_MIN_4200_MV;

		if (vbat_reg_threshold > BQ25870_VBAT_REG_MAX_4975_MV)
			vbat_reg_threshold = BQ25870_VBAT_REG_MAX_4975_MV;

		value = (u8)((vbat_reg_threshold -
			BQ25870_VBAT_REG_OFFSET_4200_MV) /
			BQ25870_VBAT_REG_STEP);
		hwlog_info("bq25870 vbat_reg_threshold=%d, value=0x%x\n",
			vbat_reg_threshold, value);
		ret = rt9748_write_mask(BQ25870_VBAT_REG,
			BQ25870_VBAT_REG_MASK, BQ25870_VBAT_REG_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	default:
		hwlog_err("device_id is not found\n");
		return -1;
	}

	return ret;
}

static int rt9748_config_ibat_ocp_threshold_ma(int ocp_threshold)
{
	u8 value;
	int ret = 0;

	switch (g_rt9748_dev->device_id) {
	case loadswitch_rt9748:
		if (ocp_threshold < RT9748_IBAT_OCP_MIN_400_MA)
			ocp_threshold = RT9748_IBAT_OCP_MIN_400_MA;

		if (ocp_threshold > RT9748_IBAT_OCP_MAX_6350_MA)
			ocp_threshold = RT9748_IBAT_OCP_MAX_6350_MA;

		value = (u8)((ocp_threshold -
			RT9748_IBAT_OCP_OFFSET_0_MA) / RT9748_IBAT_OCP_STEP);
		hwlog_info("rt9748 ocp_threshold=%d, value=0x%x\n",
			ocp_threshold, value);
		ret = rt9748_write_mask(RT9748_IBAT_OCP,
			RT9748_IBAT_OCP_MASK, RT9748_IBAT_OCP_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	case loadswitch_bq25870:
		if (ocp_threshold < BQ25870_IBAT_OCP_MIN_0_MA)
			ocp_threshold = BQ25870_IBAT_OCP_MIN_0_MA;

		if (ocp_threshold > BQ25870_IBAT_OCP_MAX_6350_MA)
			ocp_threshold = BQ25870_IBAT_OCP_MAX_6350_MA;

		value = (u8)((ocp_threshold -
			BQ25870_IBAT_OCP_OFFSET_0_MA) / BQ25870_IBAT_OCP_STEP);
		hwlog_info("bq25870 ocp_threshold=%d, value=0x%x\n",
			ocp_threshold, value);
		ret = rt9748_write_mask(BQ25870_IBAT_OCP,
			BQ25870_IBAT_OCP_MASK, BQ25870_IBAT_OCP_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	default:
		hwlog_err("device_id is not found\n");
		return -1;
	}

	return ret;
}

static int rt9748_config_ibus_ocp_threshold_ma(int ocp_threshold)
{
	u8 value;
	int ret = 0;

	switch (g_rt9748_dev->device_id) {
	case loadswitch_rt9748:
		if (ocp_threshold < RT9748_IBUS_OCP_MIN_400_MA)
			ocp_threshold = RT9748_IBUS_OCP_MIN_400_MA;

		if (ocp_threshold > RT9748_IBUS_OCP_MAX_6350_MA)
			ocp_threshold = RT9748_IBUS_OCP_MAX_6350_MA;

		value = (u8)((ocp_threshold -
			RT9748_IBUS_OCP_OFFSET_0_MA) / RT9748_IBUS_OCP_STEP);
		hwlog_info("rt9748 ocp_threshold=%d, value=0x%x\n",
			ocp_threshold, value);
		ret = rt9748_write_mask(RT9748_IBUS_OCP,
			RT9748_IBUS_OCP_MASK, RT9748_IBUS_OCP_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	case loadswitch_bq25870:
		if (ocp_threshold < BQ25870_IBUS_OCP_MIN_0_MA)
			ocp_threshold = BQ25870_IBUS_OCP_MIN_0_MA;

		if (ocp_threshold > BQ25870_IBUS_OCP_MAX_6300_MA)
			ocp_threshold = BQ25870_IBUS_OCP_MAX_6300_MA;

		value = (u8)((ocp_threshold -
			BQ25870_IBUS_OCP_OFFSET_0_MA) / BQ25870_IBUS_OCP_STEP);
		hwlog_info("bq25870 ocp_threshold=%d, value=0x%x\n",
			ocp_threshold, value);
		ret = rt9748_write_mask(BQ25870_IBUS_OCP,
			BQ25870_IBUS_OCP_MASK, BQ25870_IBUS_OCP_SHIFT,
			value);
		if (ret)
			return -1;

		break;

	default:
		hwlog_err("device_id is not found\n");
		return -1;
	}

	return ret;
}

static int rt9748_get_vbus_voltage_mv(int *vbus)
{
	int ret = 0;
	u8 reg_high = 0;
	u8 reg_low = 0;
	int polarity = 0;

	switch (g_rt9748_dev->device_id) {
	case loadswitch_rt9748:
		ret = rt9748_read_byte(RT9748_VBUS_ADC2, &reg_high);
		ret |= rt9748_read_byte(RT9748_VBUS_ADC1, &reg_low);
		if (ret)
			return -1;

		polarity = (reg_high & RT9748_VBUS_POLARITY_MASK) >>
			(LENTH_OF_BYTE - 1);
		*vbus = (reg_high & RT9748_VBUS_ADC_MASK) *
			RT9748_VBUS_HIGH_LSB;
		*vbus += reg_low * RT9748_VBUS_LOW_LSB;
		if (polarity == 1)
			*vbus *= -1;
		break;

	case loadswitch_bq25870:
		ret = rt9748_read_byte(BQ25870_VBUS_ADC2, &reg_high);
		ret |= rt9748_read_byte(BQ25870_VBUS_ADC1, &reg_low);
		if (ret)
			return -1;

		polarity = (reg_high & BQ25870_VBUS_POLARITY_MASK) >>
			(LENTH_OF_BYTE - 1);
		*vbus = (reg_high & BQ25870_VBUS_ADC_MASK) *
			BQ25870_VBUS_HIGH_LSB;
		*vbus += reg_low * BQ25870_VBUS_LOW_LSB;
		if (polarity == 1)
			*vbus *= -1;
		break;

	default:
		hwlog_err("device_id is not found\n");
		return -1;
	}

	return 0;
}

static int rt9748_get_bat_voltage_mv(void)
{
	int ret = 0;
	int polarity = 0;
	u8 reg_high = 0;
	u8 reg_low = 0;
	int vbat = 0;

	switch (g_rt9748_dev->device_id) {
	case loadswitch_rt9748:
		ret = rt9748_read_byte(RT9748_VBAT_ADC2, &reg_high);
		ret |= rt9748_read_byte(RT9748_VBAT_ADC1, &reg_low);
		if (ret)
			return -1;

		polarity = (reg_high & RT9748_VBAT_POLARITY_MASK) >>
			(LENTH_OF_BYTE - 1);
		vbat = (reg_high & RT9748_VBAT_ADC_MASK) *
			RT9748_VBAT_HIGH_LSB;
		vbat += reg_low * RT9748_VBAT_LOW_LSB;
		if (polarity == 1)
			vbat *= -1;
		break;

	case loadswitch_bq25870:
		ret = rt9748_read_byte(BQ25870_VBAT_ADC2, &reg_high);
		ret |= rt9748_read_byte(BQ25870_VBAT_ADC1, &reg_low);
		if (ret)
			return -1;

		polarity = (reg_high & BQ25870_VBAT_POLARITY_MASK) >>
			(LENTH_OF_BYTE - 1);
		vbat = (reg_high & BQ25870_VBAT_ADC_MASK) *
			BQ25870_VBAT_HIGH_LSB;
		vbat += reg_low * BQ25870_VBAT_LOW_LSB;
		if (polarity == 1)
			vbat *= -1;
		break;

	default:
		hwlog_err("device_id is not found\n");
		return -1;
	}

	return vbat;
}

static int rt9748_get_bat_current_ma(int *ibat)
{
	int ret = 0;
	u8 reg_high = 0;
	u8 reg_low = 0;
	int polarity = 0;

	switch (g_rt9748_dev->device_id) {
	case loadswitch_rt9748:
		ret = rt9748_read_byte(RT9748_IBAT_ADC2, &reg_high);
		ret |= rt9748_read_byte(RT9748_IBAT_ADC1, &reg_low);
		hwlog_info("rt9748 IBAT_ADC2 = 0x%x\n", reg_high);
		hwlog_info("rt9748 IBAT_ADC1 = 0x%x\n", reg_low);
		if (ret)
			return -1;

		polarity = (reg_high & RT9748_IBAT_POLARITY_MASK) >>
			(LENTH_OF_BYTE - 1);
		*ibat = (reg_high & RT9748_IBAT_ADC_MASK) *
			RT9748_IBAT_HIGH_LSB;
		*ibat += reg_low * RT9748_IBAT_LOW_LSB;
		if (polarity == 1)
			*ibat *= -1;
		break;

	case loadswitch_bq25870:
		ret = rt9748_read_byte(BQ25870_IBAT_ADC2, &reg_high);
		ret |= rt9748_read_byte(BQ25870_IBAT_ADC1, &reg_low);
		hwlog_info("bq25870 IBAT_ADC2 = 0x%x\n", reg_high);
		hwlog_info("bq25870 IBAT_ADC1 = 0x%x\n", reg_low);
		if (ret)
			return -1;

		polarity = (reg_high & BQ25870_IBAT_POLARITY_MASK) >>
			(LENTH_OF_BYTE - 1);
		*ibat = (reg_high & BQ25870_IBAT_ADC_MASK) *
			BQ25870_IBAT_HIGH_LSB;
		*ibat += reg_low * BQ25870_IBAT_LOW_LSB;
		if (polarity == 1)
			*ibat *= -1;
		break;

	default:
		hwlog_err("device_id is not found\n");
		return -1;
	}

	return 0;
}

static int rt9748_get_ls_temp(int *temp)
{
	u8 reg;
	int ret;

	ret = rt9748_read_byte(RT9748_TDIE_ADC1, &reg);
	if (ret)
		return -1;

	*temp = (int)reg;
	return 0;
}

static int rt9748_get_ls_ibus(int *ibus)
{
	u8 reg_high = 0;
	u8 reg_low = 0;
	int polarity = 0;
	int ret;

	switch (g_rt9748_dev->device_id) {
	case loadswitch_rt9748:
		ret = rt9748_read_byte(RT9748_IBUS_ADC2, &reg_high);
		ret |= rt9748_read_byte(RT9748_IBUS_ADC1, &reg_low);
		hwlog_info("rt9748 IBUS_ADC2 = 0x%x\n", reg_high);
		hwlog_info("rt9748 IBUS_ADC1 = 0x%x\n", reg_low);
		if (ret)
			return -1;

		polarity = (reg_high & RT9748_IBUS_POLARITY_MASK) >>
			(LENTH_OF_BYTE - 1);
		*ibus = (reg_high & RT9748_IBUS_ADC_MASK) *
			RT9748_IBUS_HIGH_LSB;
		*ibus += reg_low * RT9748_IBUS_LOW_LSB;
		if (polarity == 1)
			*ibus *= -1;
		break;

	case loadswitch_bq25870:
		ret = rt9748_read_byte(BQ25870_IBUS_ADC2, &reg_high);
		ret |= rt9748_read_byte(BQ25870_IBUS_ADC1, &reg_low);
		hwlog_info("bq25870 IBUS_ADC2 = 0x%x\n", reg_high);
		hwlog_info("bq25870 IBUS_ADC1 = 0x%x\n", reg_low);
		if (ret)
			return -1;

		polarity = (reg_high & BQ25870_IBUS_POLARITY_MASK) >>
			(LENTH_OF_BYTE - 1);
		*ibus = (reg_high & BQ25870_IBUS_ADC_MASK) *
			BQ25870_IBUS_HIGH_LSB;
		*ibus += reg_low * BQ25870_IBUS_LOW_LSB;
		if (polarity == 1)
			*ibus *= -1;
		break;

	default:
		hwlog_err("device_id is not found\n");
		return -1;
	}

	return 0;
}

static int loadswitch_get_device_id(void)
{
	u8 reg = 0;
	int ret = 0;
	int bit3;
	int dev_id = -1;
	struct rt9748_device_info *di = g_rt9748_dev;

	if (g_get_id_time == 0) {
		g_get_id_time = 1;
		ret = rt9748_read_byte(RT9748_ADC_CTRL, &reg);
		if (ret)
			return -1;

		hwlog_info("reg07=%x\n", reg);

		bit3 = (reg >> LS_GET_DEV_ID_SHIFT) & 1;
		if (bit3 == 1) {
			dev_id = loadswitch_rt9748;
			hwlog_info("dev_id=%d\n", dev_id);
			return dev_id;
		}

		ret = rt9748_read_byte(LOADSWITCH_DEV_INFO_REG0, &reg);
		if (ret)
			return -1;

		hwlog_info("reg00=%x\n", reg);

		switch (reg) {
		case 0:
			dev_id = loadswitch_rt9748;
			break;

		case 1:
			dev_id = loadswitch_bq25870;
			break;

		case 2:
			dev_id = loadswitch_fair_child;
			break;

		case 3:
			dev_id = loadswitch_nxp;
			break;

		default:
			dev_id = -1;
			break;
		}

		hwlog_info("dev_id=%d\n", dev_id);
		return dev_id;
	}

	hwlog_info("dev_id=%d\n", di->device_id);
	return di->device_id;
}

static int chip_init(void)
{
	int ret = 0;
	struct rt9748_device_info *di = g_rt9748_dev;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (di->chip_already_init == 1)
		return 0;

	ret = gpio_direction_output(di->gpio_en, 1);
	if (ret) {
		hwlog_err("gpio_en fail\n");
		return -1;
	}

	msleep(200); /* delay 200ms */
	di->chip_already_init = 1;
	return 0;
}

static int rt9748_charge_status(void)
{
	struct rt9748_device_info *di = g_rt9748_dev;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (di->chip_already_init == 1)
		return 0;

	hwlog_err("charge_status=%d\n", di->chip_already_init);
	return -1;
}

static int rt9748_charge_init(void)
{
	int ret = 0;
	struct rt9748_device_info *di = g_rt9748_dev;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = chip_init();
	di->device_id = loadswitch_get_device_id();
	if (di->device_id == -1)
		return -1;

	hwlog_info("loadswitch device id is %d\n", di->device_id);

	ret |= rt9748_reg_init();
	ret |= rt9748_config_ioc_ocp_threshold_ma(6000);
	ret |= rt9748_config_vbus_ovp_threshold_mv(5800);
	ret |= rt9748_config_vout_reg_threshold_mv(4740);
	ret |= rt9748_config_vdrop_alm_reg_threshold_mv(1000);
	ret |= rt9748_config_vdrop_ovp_reg_threshold_mv(1000);
	ret |= rt9748_config_vbat_reg_threshold_mv(4450);
	ret |= rt9748_config_ibat_ocp_threshold_ma(5800);
	ret |= rt9748_config_ibus_ocp_threshold_ma(5800);
	if (ret)
		return -1;

	return 0;
}

static int batinfo_init(void)
{
	int ret = 0;

	ret = chip_init();
	ret |= rt9748_adc_enable(1);
	if (ret) {
		hwlog_err("adc_enable\n");
		return -1;
	}

	return 0;
}

static int rt9748_charge_exit(void)
{
	int ret = 0;
	struct rt9748_device_info *di = g_rt9748_dev;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	di->chip_already_init = 0;

	ret = rt9748_charge_enable(0);
	if (ret) {
		hwlog_err("close fail\n");
		/*
		 * here do not return,
		 * cause reset pin can also close the switch
		 */
	}

	/* pull down reset pin to reset fan54151 */
	ret = gpio_direction_output(di->gpio_en, 0);
	if (ret) {
		hwlog_err("reset pull down fail\n");
		return -1;
	}

	usleep_range(10000, 11000); /* wait for 10ms */
	return ret;
}

static int rt9748_batinfo_exit(void)
{
	return 0;
}

static int rt9748_is_ls_close(void)
{
	u8 reg = 0;
	int ret = 0;

	ret = rt9748_read_byte(RT9748_CONTROL, &reg);
	if (ret)
		return 1;

	if (reg & RT9748_CHARGE_EN_MASK)
		return 0;

	return 1;
}

static struct loadswitch_ops rt9748_sysinfo_ops = {
	.ls_init = rt9748_charge_init,
	.ls_exit = rt9748_charge_exit,
	.ls_enable = rt9748_charge_enable,
	.ls_discharge = rt9748_discharge,
	.is_ls_close = rt9748_is_ls_close,
	.get_ls_id = loadswitch_get_device_id,
	.ls_status = rt9748_charge_status,
};

static struct batinfo_ops rt9748_batinfo_ops = {
	.init = batinfo_init,
	.exit = rt9748_batinfo_exit,
	.get_bat_btb_voltage = rt9748_get_bat_voltage_mv,
	.get_bat_package_voltage = rt9748_get_bat_voltage_mv,
	.get_vbus_voltage = rt9748_get_vbus_voltage_mv,
	.get_bat_current = rt9748_get_bat_current_ma,
	.get_ls_ibus = rt9748_get_ls_ibus,
	.get_ls_temp = rt9748_get_ls_temp,
};

static void rt9748_irq_work(struct work_struct *work)
{
	struct rt9748_device_info *di;
	struct nty_data *data;
	u8 event1;
	u8 event2;
	u8 status;
	struct atomic_notifier_head *direct_charge_fault_notifier_list;

	di = container_of(work, struct rt9748_device_info, irq_work);
	data = &(di->nty_data);

	direct_charge_lvc_get_fault_notifier(
		&direct_charge_fault_notifier_list);

	rt9748_read_byte(RT9748_EVENT_1, &event1);
	rt9748_read_byte(RT9748_EVENT_2, &event2);
	rt9748_read_byte(RT9748_EVENT_STATUS, &status);

	data->event1 = event1;
	data->event2 = event2;
	data->addr = di->client->addr;

	if (event1 & RT9748_VBUS_OVP_FLT) {
		hwlog_err("vbus ovp happened\n");

		atomic_notifier_call_chain(
			direct_charge_fault_notifier_list,
			DIRECT_CHARGE_FAULT_VBUS_OVP, data);
	} else if (event1 & RT9748_IBUS_REVERSE_OCP_FLT) {
		hwlog_err("ibus reverse ocp happened\n");

		atomic_notifier_call_chain(
			direct_charge_fault_notifier_list,
			DIRECT_CHARGE_FAULT_REVERSE_OCP, data);
	} else if (event2 & RT9748_OTP_FLT) {
		hwlog_err("otp happened\n");

		atomic_notifier_call_chain(
			direct_charge_fault_notifier_list,
			DIRECT_CHARGE_FAULT_OTP, data);
	} else if (event2 & RT9748_INPUT_OCP_FLT) {
		hwlog_err("input ocp happened\n");

		atomic_notifier_call_chain(
			direct_charge_fault_notifier_list,
			DIRECT_CHARGE_FAULT_INPUT_OCP, data);
	} else if (event2 & RT9748_VDROP_OVP_FLT) {
		hwlog_err("vdrop ovp happened\n");

		atomic_notifier_call_chain(
			direct_charge_fault_notifier_list,
			DIRECT_CHARGE_FAULT_VDROP_OVP, data);
	} else {
		/* do nothing */
	}

	hwlog_info("event1 [%x]=0x%x\n", RT9748_EVENT_1, event1);
	hwlog_info("event2 [%x]=0x%x\n", RT9748_EVENT_2, event2);
	hwlog_info("status [%x]=0x%x\n", RT9748_EVENT_STATUS, status);

	/* clear irq */
	enable_irq(di->irq_int);
}

static irqreturn_t rt9748_interrupt(int irq, void *_di)
{
	struct rt9748_device_info *di = _di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	hwlog_info("rt9748 int happened\n");

	disable_irq_nosync(di->irq_int);
	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}

static void rt9748_parse_dts(struct device_node *np,
	struct rt9748_device_info *di)
{
}

static int rt9748_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret = 0;
	struct rt9748_device_info *di = NULL;
	struct device_node *np = NULL;

	hwlog_info("probe begin\n");

	if (client == NULL || id == NULL) {
		hwlog_err("client or id is null\n");
		return -ENOMEM;
	}

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (di == NULL)
		return -ENOMEM;

	g_rt9748_dev = di;

	di->chip_already_init = 0;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);
	INIT_WORK(&di->irq_work, rt9748_irq_work);

	rt9748_parse_dts(np, di);

	di->gpio_int = of_get_named_gpio(np, "loadswitch_int", 0);
	hwlog_info("gpio_int=%d\n", di->gpio_int);

	if (!gpio_is_valid(di->gpio_int)) {
		hwlog_err("gpio(gpio_int) is not valid\n");
		ret = -EINVAL;
		goto rt9748_fail_0;
	}

	di->gpio_en = of_get_named_gpio(np, "loadswitch_en", 0);
	hwlog_err("gpio_en=%d\n", di->gpio_en);

	if (!gpio_is_valid(di->gpio_en)) {
		hwlog_err("gpio(gpio_en) is not valid\n");
		ret = -EINVAL;
		goto rt9748_fail_0;
	}

	ret = gpio_request(di->gpio_int, "loadswitch_int");
	if (ret) {
		hwlog_err("gpio(gpio_int) request fail\n");
		goto rt9748_fail_0;
	}

	ret = gpio_request(di->gpio_en, "loadswitch_en");
	if (ret) {
		hwlog_err("gpio(gpio_en) request fail\n");
		goto rt9748_fail_1;
	}

	ret = gpio_direction_output(di->gpio_en, 0);
	if (ret) {
		hwlog_err("gpio(gpio_en) set output fail\n");
		goto rt9748_fail_2;
	}

	ret = gpio_direction_input(di->gpio_int);
	if (ret) {
		hwlog_err("gpio(gpio_int) set input fail\n");
		goto rt9748_fail_2;
	}

	di->irq_int = gpio_to_irq(di->gpio_int);
	if (di->irq_int < 0) {
		hwlog_err("gpio(gpio_int) map to irq fail\n");
		ret = -EINVAL;
		goto rt9748_fail_2;
	}

	ret = request_irq(di->irq_int, rt9748_interrupt,
		IRQF_TRIGGER_FALLING, "loadswitch_int_irq", di);
	if (ret) {
		hwlog_err("gpio(gpio_int) irq request fail\n");
		di->irq_int = -1;
		goto rt9748_fail_2;
	}

	ret = loadswitch_main_ops_register(&rt9748_sysinfo_ops);
	if (ret) {
		hwlog_err("rt9748 sysinfo ops register fail\n");
		goto rt9748_fail_3;
	}

	ret = batinfo_lvc_ops_register(&rt9748_batinfo_ops);
	if (ret) {
		hwlog_err("rt9748 batinfo ops register fail\n");
		goto rt9748_fail_3;
	}

	hwlog_info("probe end\n");
	return 0;

rt9748_fail_3:
	free_irq(di->irq_int, di);
rt9748_fail_2:
	gpio_free(di->gpio_en);
rt9748_fail_1:
	gpio_free(di->gpio_int);
rt9748_fail_0:
	devm_kfree(&client->dev, di);
	g_rt9748_dev = NULL;
	np = NULL;
	return ret;
}

static int rt9748_remove(struct i2c_client *client)
{
	struct rt9748_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	/* reset rt9748 */
	gpio_set_value(di->gpio_en, 0);

	if (di->gpio_en)
		gpio_free(di->gpio_en);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	hwlog_info("remove end\n");
	return 0;
}

MODULE_DEVICE_TABLE(i2c, rt9748);
static const struct of_device_id rt9748_of_match[] = {
	{
		.compatible = "rt9748_main",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id rt9748_i2c_id[] = {
	{"rt9748_main", 0}, {}
};

static struct i2c_driver rt9748_driver = {
	.probe = rt9748_probe,
	.remove = rt9748_remove,
	.id_table = rt9748_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "rt9748_main",
		.of_match_table = of_match_ptr(rt9748_of_match),
	},
};

static int __init rt9748_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&rt9748_driver);
	if (ret)
		hwlog_err("i2c_add_driver error\n");

	return ret;
}

static void __exit rt9748_exit(void)
{
	i2c_del_driver(&rt9748_driver);
}

module_init(rt9748_init);
module_exit(rt9748_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("rt9748 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
