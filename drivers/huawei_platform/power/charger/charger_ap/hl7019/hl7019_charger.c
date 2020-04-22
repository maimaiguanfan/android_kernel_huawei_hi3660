/*
 * hl7019_charger.c
 *
 * hl7019 driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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

#include "hl7019_charger.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/wakelock.h>
#include <linux/usb/otg.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/power_supply.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/raid/pq.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/power/huawei_charger.h>
#include <linux/power/hisi/hisi_bci_battery.h>
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
#include <huawei_platform/power/usb_short_circuit_protect.h>
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG hl7019_charger
HWLOG_REGIST();

struct hl7019_device_info *g_hl7019_dev;
static int g_hiz_iin_limit_flag;

static const u32 g_iinlimit_tbl[] = {
	100, 150, 500, 900, 1000, 1500, 2000, 3000,
}; /* mA */

static const u32 g_vdpm_tbl[] = {
	4200, 4290, 4350, 4440, 4520, 4610, 4670, 4760, 4840,
}; /* mV */

static int hl7019_set_dpm_voltage(int value);
static int hl7019_set_charger_hiz(int enable);
static int hl7019_check_input_dpm_state(void);

static u8 hl7019_closest_reg_via_tbl(const u32 *tbl, u32 tbl_size, u32 target)
{
	u32 i;

	if (!tbl || target < tbl[0])
		return 0;

	for (i = 0; i < tbl_size - 1; i++) {
		if (target >= tbl[i] && target < tbl[i + 1])
			return i;
	}

	return tbl_size - 1;
}

static int hl7019_write_block(struct hl7019_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[1];
	int ret;

	if (!di || !di->client || !value) {
		hwlog_err("di or value is null\n");
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
		hwlog_err("write_block failed: %x\n", reg);
		return -EIO;
	}

	return 0;
}

static int hl7019_read_block(struct hl7019_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[MSG_LEN];
	u8 buf;
	int ret;

	if (!di || !di->client || !value) {
		hwlog_err("di or value is null\n");
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
		hwlog_err("read_block failed: %x\n", reg);
		return -EIO;
	}

	return 0;
}

static int hl7019_write_byte(u8 reg, u8 value)
{
	struct hl7019_device_info *di = g_hl7019_dev;
	/* 2 bytes offset 1 contains the data offset 0 is used by i2c_write */
	u8 temp_buffer[MSG_LEN] = {0};

	/* offset 1 contains the data */
	temp_buffer[1] = value;
	return hl7019_write_block(di, temp_buffer, reg, 1);
}

static int hl7019_read_byte(u8 reg, u8 *value)
{
	struct hl7019_device_info *di = g_hl7019_dev;

	if (!di || !value)
		return -EINVAL;

	return hl7019_read_block(di, value, reg, 1);
}

static int hl7019_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = hl7019_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return hl7019_write_byte(reg, val);
}

static int hl7019_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret;
	u8 val = 0;

	ret = hl7019_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return 0;
}

#ifdef CONFIG_SYSFS
/*
 * There are a numerous options that are configurable on the hl7019
 * that go well beyond what the power_supply properties provide access to.
 * Provide sysfs access to them so they can be examined and possibly modified
 * on the fly.
 */
#define HL7019_SYSFS_FIELD(_name, r, f, m, store) \
{ \
	.attr = __ATTR(_name, m, hl7019_sysfs_show, store), \
	.reg = HL7019_REG_##r, \
	.mask = HL7019_REG_##f##_MASK, \
	.shift = HL7019_REG_##f##_SHIFT, \
}

#define HL7019_SYSFS_FIELD_RW(_name, r, f) \
	HL7019_SYSFS_FIELD(_name, r, f, 0640, hl7019_sysfs_store)

#define HL7019_SYSFS_FIELD_RO(_name, r, f) \
	HL7019_SYSFS_FIELD(_name, r, f, 0440, NULL)

static ssize_t hl7019_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t hl7019_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

struct hl7019_sysfs_field_info {
	struct device_attribute attr;
	u8 reg;
	u8 mask;
	u8 shift;
};

static struct hl7019_sysfs_field_info g_hl7019_sysfs_field_tbl[] = {
	/* sysfs name reg field in reg */
	HL7019_SYSFS_FIELD_RW(reg_addr, NONE, NONE),
	HL7019_SYSFS_FIELD_RW(reg_value, NONE, NONE),
};

#define HL7019_SYSFS_ATTRS_SIZE (ARRAY_SIZE(g_hl7019_sysfs_field_tbl) + 1)

static struct attribute *g_hl7019_sysfs_attrs[HL7019_SYSFS_ATTRS_SIZE];

static const struct attribute_group g_hl7019_sysfs_attr_group = {
	.attrs = g_hl7019_sysfs_attrs,
};

static void hl7019_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(g_hl7019_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		g_hl7019_sysfs_attrs[i] =
			&g_hl7019_sysfs_field_tbl[i].attr.attr;

	g_hl7019_sysfs_attrs[limit] = NULL;
}

static struct hl7019_sysfs_field_info *hl7019_sysfs_field_lookup(
	const char *name)
{
	int i;
	int limit = ARRAY_SIZE(g_hl7019_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strcmp(name, g_hl7019_sysfs_field_tbl[i].attr.attr.name))
			break;
	}

	if (i >= limit)
		return NULL;

	return &g_hl7019_sysfs_field_tbl[i];
}

static ssize_t hl7019_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct hl7019_sysfs_field_info *info = NULL;
	struct hl7019_sysfs_field_info *info2 = NULL;
	int ret;
	u8 v = 0;

	if (!dev || !attr || !buf)
		return -EINVAL;

	info = hl7019_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	if (!strncmp("reg_addr", attr->attr.name, strlen("reg_addr")))
		return scnprintf(buf, PAGE_SIZE, "0x%hhx\n", info->reg);

	if (!strncmp(("reg_value"), attr->attr.name, strlen("reg_value"))) {
		info2 = hl7019_sysfs_field_lookup("reg_addr");
		if (!info2) {
			hwlog_err("get sysfs entries failed\n");
			return -EINVAL;
		}

		info->reg = info2->reg;
	}

	ret = hl7019_read_mask(info->reg, info->mask, info->shift, &v);
	if (ret)
		return ret;

	return scnprintf(buf, PAGE_SIZE, "%hhx\n", v);
}

static ssize_t hl7019_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hl7019_sysfs_field_info *info = NULL;
	struct hl7019_sysfs_field_info *info2 = NULL;
	int ret;
	u8 v = 0;

	if (!dev || !attr || !buf)
		return -EINVAL;

	info = hl7019_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	ret = kstrtou8(buf, 0, &v);
	if (ret < 0) {
		hwlog_err("get kstrtou8 failed\n");
		return ret;
	}

	if (!strncmp(("reg_value"), attr->attr.name, strlen("reg_value"))) {
		info2 = hl7019_sysfs_field_lookup("reg_addr");
		if (!info2) {
			hwlog_err("get sysfs entries failed\n");
			return -EINVAL;
		}

		info->reg = info2->reg;
	}

	if (!strncmp(("reg_addr"), attr->attr.name, strlen("reg_addr"))) {
		if (v < (u8) HL7019_REG_NUM) {
			info->reg = v;
			return count;
		} else {
			return -EINVAL;
		}
	}

	ret = hl7019_write_mask(info->reg, info->mask, info->shift, v);
	if (ret)
		return ret;

	return count;
}

static int hl7019_sysfs_create_group(struct hl7019_device_info *di)
{
	if (!di)
		return -EINVAL;

	hl7019_sysfs_init_attrs();

	return sysfs_create_group(&di->dev->kobj, &g_hl7019_sysfs_attr_group);
}

static void hl7019_sysfs_remove_group(struct hl7019_device_info *di)
{
	if (!di)
		return;

	sysfs_remove_group(&di->dev->kobj, &g_hl7019_sysfs_attr_group);
}

#else

static inline int hl7019_sysfs_create_group(struct hl7019_device_info *di)
{
	return 0;
}

static inline void hl7019_sysfs_remove_group(struct hl7019_device_info *di)
{
}
#endif /* CONFIG_SYSFS */

static int hl7019_device_check(void)
{
	u8 reg = 0;
	int ret;

	ret = hl7019_read_byte(HL7019_REG_CTRL0A, &reg);
	if (ret)
		return CHARGE_IC_BAD;

	hwlog_info("device_check %x=0x%x\n", HL7019_REG_CTRL0A, reg);

	if (reg == HL7019_VENDOR_ID) {
		hwlog_info("hl7019 is good\n");
		return CHARGE_IC_GOOD;
	}

	hwlog_err("hl7019 is bad\n");
	return CHARGE_IC_BAD;
}

static bool hl7019_check_pwr_good_state(void)
{
	u8 val = 0;
	int ret;

	ret = hl7019_read_mask(HL7019_REG_CTRL8,
		HL7019_REG_PG_STAT_MASK,
		HL7019_REG_PG_STAT_SHIFT,
		&val);
	if (ret < 0)
		return false;

	hwlog_info("check_charger_plugged %x=0x%x\n",
		HL7019_REG_CTRL8, val);

	if (val == REG08_POWER_GOOD)
		return true;

	return false;
}

static bool check_chip_init_done(void)
{
	bool pwr_active = false;
	int dpm_active;

	dpm_active = hl7019_check_input_dpm_state();
	pwr_active = hl7019_check_pwr_good_state();
	if (dpm_active && pwr_active)
		return true;
	else
		return false;
}

static int hl7019_5v_chip_init(struct hl7019_device_info *di)
{
	int ret, i;
	bool chip_init = false;
	int retval;

	/* boost mode current limit = 1000mA */
	ret = hl7019_write_mask(HL7019_REG_CHG_CTRL1,
		HL7019_REG_BOOST_ILIM_MASK,
		HL7019_REG_BOOST_ILIM_SHIFT,
		REG01_BOOST_ILIM_1A);

	/* I2C watchdog timer setting = 80s */
	ret |= hl7019_write_mask(HL7019_REG_CTRL5,
		HL7019_REG_WDT_MASK,
		HL7019_REG_WDT_SHIFT,
		REG05_WDT_80S);

	/* 0x10 present iprechg = 256ma,iterm current = 128ma */
	ret |= hl7019_write_byte(HL7019_REG_CTRL3, 0x10);

	/* init IR setting: 25moum, 32mv */
	ret |= hl7019_write_byte(HL7019_REG_CTRL0C, 0x48);

	/* init vdpm */
	for (i = 0; i < PWR_CHECK_CNT; i++) {
		chip_init = check_chip_init_done();
		if (chip_init) {
			msleep(CHIP_INIT_INTERVAL);
			retval = hl7019_set_dpm_voltage(VINDPM_DEFAULT);
			retval |= hl7019_write_mask(HL7019_REG_CTRL0D,
				HL7019_REG_VINDPM_OFFSET_MASK,
				HL7019_REG_VINDPM_OFFSET_SHIFT,
				REG0D_5V_VDPM);
			if (retval)
				hwlog_err("chip_init set 5v vdpm fail\n");
			break;
		}
		retval = hl7019_write_mask(HL7019_REG_CTRL0D,
			HL7019_REG_VINDPM_OFFSET_MASK,
			HL7019_REG_VINDPM_OFFSET_SHIFT,
			REG0D_10V_VDPM);
		if (retval)
			hwlog_err("chip_init set 10v vdpm fail\n");
		msleep(PWR_CHECK_INTERVAL);
	}

	if (i == PWR_CHECK_CNT) {
		retval = hl7019_set_dpm_voltage(VINDPM_DEFAULT);
		retval |= hl7019_write_mask(HL7019_REG_CTRL0D,
			HL7019_REG_VINDPM_OFFSET_MASK,
			HL7019_REG_VINDPM_OFFSET_SHIFT,
			REG0D_5V_VDPM);
		if (retval)
			hwlog_err("chip_init set 5v vdpm fail\n");
	}

	g_hiz_iin_limit_flag = HIZ_IIN_FLAG_FALSE;

	/* enable charging */
	gpio_set_value(di->gpio_cd, 0);

	return ret;
}

static int hl7019_chip_init(struct chip_init_crit *init_crit)
{
	int ret = 0;
	struct hl7019_device_info *di = g_hl7019_dev;

	if (!di || !init_crit) {
		hwlog_err("di or init_crit is null\n");
		return -ENOMEM;
	}

	switch (init_crit->vbus) {
	case ADAPTER_5V:
		ret = hl7019_5v_chip_init(di);
		break;
	default:
		hwlog_err("invaid init_crit vbus mode\n");
		break;
	}

	return ret;
}

static int hl7019_set_input_current(int value)
{
	int val;

	if (g_hiz_iin_limit_flag == HIZ_IIN_FLAG_TRUE) {
		hwlog_err("g_hiz_iin_limit_flag,just set 100mA\n");
		value = IINLIM_0P1A;
	}

	val = hl7019_closest_reg_via_tbl(g_iinlimit_tbl,
		ARRAY_SIZE(g_iinlimit_tbl), value);

	hwlog_info("set_input_current %x=0x%x\n", HL7019_REG_CHG_CTRL0, val);

	return hl7019_write_mask(HL7019_REG_CHG_CTRL0,
		HL7019_REG_IINLIM_MASK,
		HL7019_REG_IINLIM_SHIFT,
		val);
}

static int hl7019_set_charge_current(int value)
{
	u8 val;
	u8 ichg_20pct;

	if (value < ICHG_BASE) {
		value *= ICHG_LOW_MULTI_FAC;
		ichg_20pct = ENABLE_ICHG_20PCT;
	} else {
		ichg_20pct = DISABLE_ICHG_20PCT;
	}

	value -= ICHG_LSB * ICHG_REFIT_FAC;
	if (value > ICHG_MAX)
		value = ICHG_MAX;
	else if (value < ICHG_BASE)
		value = ICHG_BASE;

	val = ((value - ICHG_BASE) / ICHG_LSB) << HL7019_REG_ICHG_SHIFT;
	val |= ichg_20pct;

	hwlog_info("setchgcur value:%d %x=0x%x\n", value,
		HL7019_REG_CTRL2, val);

	return hl7019_write_byte(HL7019_REG_CTRL2, val);
}

static int hl7019_set_boost_current(int curr)
{
	int val;

	val = (curr < BOOST_ILIM_1P5A) ?
		REG01_BOOST_ILIM_1A : REG01_BOOST_ILIM_2P1A;
	hwlog_info("setboostcur %x=0x%x\n", HL7019_REG_CHG_CTRL1, val);

	return hl7019_write_mask(HL7019_REG_CHG_CTRL1,
		HL7019_REG_BOOST_ILIM_MASK,
		HL7019_REG_BOOST_ILIM_SHIFT,
		val);
}

static int hl7019_set_terminal_voltage(int value)
{
	int val;
	struct hl7019_device_info *di = g_hl7019_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -ENOMEM;
	}

	if ((di->cust_cv > CUST_MIN_CV) && (value > di->cust_cv)) {
		hwlog_info("set cv to custom_cv=%d\n", di->cust_cv);
		value = di->cust_cv;
	}

	val = (value - VREG_BASE) / VREG_LSB;

	hwlog_info("set_cv reg%x=0x%x\n", HL7019_REG_CTRL4, val);

	return hl7019_write_mask(HL7019_REG_CTRL4,
		HL7019_REG_VREG_MASK,
		HL7019_REG_VREG_SHIFT,
		val);
}

static int hl7019_set_dpm_voltage(int value)
{
	int val;

	val = hl7019_closest_reg_via_tbl(g_vdpm_tbl,
		ARRAY_SIZE(g_vdpm_tbl), value);

	val += VDPM_BASE;

	hwlog_info("dpmvol %x=0x%x\n", HL7019_REG_CHG_CTRL0, val);

	return hl7019_write_mask(HL7019_REG_CHG_CTRL0,
		HL7019_REG_VINDPM_MASK,
		HL7019_REG_VINDPM_SHIFT,
		val);
}

static int hl7019_set_terminal_current(int value)
{
	int val;

	val = (value - ITERM_BASE) / ITERM_LSB;

	hwlog_info("reused_set_iterm %x=0x%x\n", HL7019_REG_CTRL3, val);

	return hl7019_write_mask(HL7019_REG_CTRL3,
		HL7019_REG_ITERM_MASK,
		HL7019_REG_ITERM_SHIFT,
		val);
}

static int hl7019_set_charge_enable(int enable)
{
	int val;

	val = (enable > 0) ? REG01_CHG_ENABLE : REG01_CHG_OTG_DISABLE;
	return hl7019_write_mask(HL7019_REG_CHG_CTRL1,
		HL7019_REG_CHG_OTG_CFG_MASK,
		HL7019_REG_CHG_OTG_CFG_SHIFT,
		val);
}

static int hl7019_set_boost_voltage(int voltage)
{
	int val;
	int value;

	if (voltage <= BOOSTV_4550)
		value = BOOSTV_4550;
	else if (voltage >= BOOSTV_5510)
		value = BOOSTV_5510;
	else
		value = voltage;

	val = (value - BOOSTV_BASE) / BOOSTV_LSB;
	hwlog_info("set_boost_voltage %x=0x%x\n", HL7019_REG_CTRL6, val);

	return hl7019_write_mask(HL7019_REG_CTRL6,
		HL7019_REG_BOOSTV_MASK,
		HL7019_REG_BOOSTV_SHIFT,
		val);
}

static int hl7019_set_otg_enable(int enable)
{
	struct hl7019_device_info *di = g_hl7019_dev;
	int val;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	if ((!di->irq_active) && (enable)) {
		di->irq_active = 1;
		enable_irq(di->irq_int);
	} else if ((di->irq_active) && (!enable)) {
		di->irq_active = 0;
		disable_irq(di->irq_int);
	}

	val = (enable > 0) ? REG01_OTG_ENABLE : REG01_CHG_OTG_DISABLE;
	return hl7019_write_mask(HL7019_REG_CHG_CTRL1,
		HL7019_REG_CHG_OTG_CFG_MASK,
		HL7019_REG_CHG_OTG_CFG_SHIFT,
		val);
}

static int hl7019_set_term_enable(int enable)
{
	return hl7019_write_mask(HL7019_REG_CTRL5,
		HL7019_REG_EN_TERM_MASK,
		HL7019_REG_EN_TERM_SHIFT,
		enable);
}

static int hl7019_get_charge_state(unsigned int *state)
{
	u8 reg = 0;
	int ret;

	if (!state)
		return -EINVAL;

	ret = hl7019_read_byte(HL7019_REG_CTRL8, &reg);

	hwlog_info("get_charge_state %x=0x%x\n",
		HL7019_REG_CTRL8, reg);

	if (!(reg & HL7019_REG_PG_STAT_MASK))
		*state |= CHAGRE_STATE_NOT_PG;

	if ((reg & HL7019_REG_CHRG_STAT_MASK) ==
		HL7019_REG_CHRG_STAT_MASK)
		*state |= CHAGRE_STATE_CHRG_DONE;

	ret |= hl7019_read_byte(HL7019_REG_CTRL9, &reg);

	hwlog_info("get_charge_fault_state %x=0x%x\n",
		HL7019_REG_CTRL9, reg);

	if ((reg & HL7019_REG_FAULT_WDT_MASK) ==
		HL7019_REG_FAULT_WDT_MASK)
		*state |= CHAGRE_STATE_WDT_FAULT;

	if ((reg & HL7019_REG_FAULT_CHRG_MASK) ==
		FAULT_CHRG_INPUT)
		*state |= CHAGRE_STATE_VBUS_OVP;

	if ((reg & HL7019_REG_FAULT_BAT_MASK) ==
		HL7019_REG_FAULT_BAT_MASK)
		*state |= CHAGRE_STATE_BATT_OVP;

	return ret;
}

static int hl7019_reset_watchdog_timer(void)
{
	return hl7019_write_mask(HL7019_REG_CHG_CTRL1,
		HL7019_REG_WDT_RESET_MASK,
		HL7019_REG_WDT_RESET_SHIFT,
		REG01_WDT_RESET);
}

static int hl7019_get_ilim(void)
{
	return 0;
}

static int hl7019_check_charger_plugged(void)
{
	u8 val = 0;
	int ret;

	ret = hl7019_read_mask(HL7019_REG_CTRL8,
		HL7019_REG_PG_STAT_MASK,
		HL7019_REG_PG_STAT_SHIFT,
		&val);
	if (ret < 0)
		return 0;

	hwlog_info("check_charger_plugged %x=0x%x\n",
		HL7019_REG_CTRL8, val);

	if (val == REG08_POWER_GOOD)
		return REG08_POWER_GOOD;

	return 0;
}

static int hl7019_check_input_dpm_state(void)
{
	u8 reg = 0;
	int ret;

	ret = hl7019_read_byte(HL7019_REG_CTRL8, &reg);
	if (ret < 0)
		return FALSE;

	hwlog_info("check_input_dpm_state %x=0x%x\n", HL7019_REG_CTRL8, reg);

	if (reg & HL7019_REG_DPM_STAT_MASK)
		return TRUE;
	else
		return FALSE;
}

static int hl7019_check_input_vdpm_state(void)
{
	return hl7019_check_input_dpm_state();
}

static int hl7019_check_input_idpm_state(void)
{
	return hl7019_check_input_dpm_state();
}

static int hl7019_dump_register(char *reg_value)
{
	u8 reg[HL7019_REG_NUM] = {0};
	char buff[BUF_LEN] = {0};
	int i, ret;
	int len = 0;

	if (!reg_value)
		return 0;

	memset(reg_value, 0, CHARGELOG_SIZE);

	for (i = 0; i < HL7019_REG_NUM; i++) {
		ret = hl7019_read_byte(i, &reg[i]);
		if (ret)
			hwlog_err("dump_register read fail\n");

		snprintf(buff, BUF_LEN, "0x%-8.2x", reg[i]);
		len += strlen(buff);
		if (len < CHARGELOG_SIZE)
			strncat(reg_value, buff, strlen(buff));
	}

	return 0;
}

static int hl7019_get_register_head(char *reg_head)
{
	char buff[BUF_LEN] = {0};
	int i;
	int len = 0;

	if (!reg_head)
		return 0;

	memset(reg_head, 0, CHARGELOG_SIZE);

	for (i = 0; i < HL7019_REG_NUM; i++) {
		snprintf(buff, BUF_LEN, "Reg[0x%x]  ", i);
		len += strlen(buff);
		if (len < CHARGELOG_SIZE)
			strncat(reg_head, buff, strlen(buff));
	}

	return 0;
}

static int hl7019_set_batfet_disable(int disable)
{
	return hl7019_write_mask(HL7019_REG_CTRL7,
		HL7019_REG_BATFET_DISABLE_MASK,
		HL7019_REG_BATFET_DISABLE_SHIFT,
		disable);
}

static int hl7019_set_watchdog_timer(int value)
{
	int ret;
	int val;

	if (value == WDT_BASE)
		val = REG05_WDT_DISABLE;
	else if (value <= WDT_40S)
		val = REG05_WDT_40S;
	else if (value <= WDT_80S)
		val = REG05_WDT_80S;
	else
		val = REG05_WDT_160S;

	hwlog_info("set_watchdog_timer reg%x=0x%x\n", HL7019_REG_CTRL5, val);

	ret = hl7019_write_mask(HL7019_REG_CTRL5,
		HL7019_REG_WDT_MASK,
		HL7019_REG_WDT_SHIFT,
		val);
	if (ret)
		hwlog_err("set_watchdog_timer write fail\n");

	if (value > 0)
		ret = hl7019_reset_watchdog_timer();

	return ret;
}

static int hl7019_set_charger_hiz(int enable)
{
	int ret;
	struct hl7019_device_info *di = g_hl7019_dev;
	static int first_in = 1;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}
	hwlog_info("set_charger_hiz %d\n", enable);

	if (enable > 0) {
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
		if (di->hiz_iin_limit == 1 && is_uscp_hiz_mode() &&
			!is_in_rt_uscp_mode()) {
			g_hiz_iin_limit_flag = HIZ_IIN_FLAG_TRUE;
			if (first_in) {
				hwlog_info("is_uscp_hiz_mode, set 100mA\n");
				first_in = 0;
				return hl7019_set_input_current(IINLIM_0P1A);
			} else {
				return 0;
			}
		} else {
#endif /* CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT */
			ret = hl7019_write_mask(HL7019_REG_CHG_CTRL0,
				HL7019_REG_EN_HIZ_MASK,
				HL7019_REG_EN_HIZ_SHIFT, TRUE);
			ret |= hl7019_set_watchdog_timer(
				WATCHDOG_TIMER_DISABLE);
			if (ret)
				hwlog_err("set_charger_hiz write fail\n");
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
		}
#endif /* CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT */
	} else {
		g_hiz_iin_limit_flag = HIZ_IIN_FLAG_FALSE;
		first_in = 1;
		ret = hl7019_write_mask(HL7019_REG_CHG_CTRL0,
			HL7019_REG_EN_HIZ_MASK,
			HL7019_REG_EN_HIZ_SHIFT, FALSE);
	}

	return ret;
}

static int hl7019_stop_charge_config(void)
{
	return hl7019_write_mask(HL7019_REG_CTRL0D,
		HL7019_REG_VINDPM_OFFSET_MASK,
		HL7019_REG_VINDPM_OFFSET_SHIFT,
		REG0D_10V_VDPM);
}

struct charge_device_ops g_hl7019_ops = {
	.chip_init = hl7019_chip_init,
	.dev_check = hl7019_device_check,
	.set_input_current = hl7019_set_input_current,
	.set_charge_current = hl7019_set_charge_current,
	.set_terminal_voltage = hl7019_set_terminal_voltage,
	.set_dpm_voltage = hl7019_set_dpm_voltage,
	.set_terminal_current = hl7019_set_terminal_current,
	.set_charge_enable = hl7019_set_charge_enable,
	.set_otg_enable = hl7019_set_otg_enable,
	.set_otg_current = hl7019_set_boost_current,
	.set_term_enable = hl7019_set_term_enable,
	.get_charge_state = hl7019_get_charge_state,
	.reset_watchdog_timer = hl7019_reset_watchdog_timer,
	.dump_register = hl7019_dump_register,
	.get_register_head = hl7019_get_register_head,
	.set_watchdog_timer = hl7019_set_watchdog_timer,
	.set_batfet_disable = hl7019_set_batfet_disable,
	.get_ibus = hl7019_get_ilim,
	.check_charger_plugged = hl7019_check_charger_plugged,
	.check_input_dpm_state = hl7019_check_input_dpm_state,
	.check_input_vdpm_state = hl7019_check_input_vdpm_state,
	.check_input_idpm_state = hl7019_check_input_idpm_state,
	.set_charger_hiz = hl7019_set_charger_hiz,
	.get_charge_current = NULL,
	.set_boost_voltage = hl7019_set_boost_voltage,
	.stop_charge_config = hl7019_stop_charge_config,
};

static void hl7019_irq_work(struct work_struct *work)
{
	struct hl7019_device_info *di = NULL;
	u8 reg = 0;
	int ret;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct hl7019_device_info, irq_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	msleep(100); /* sleep 100ms */

	ret = hl7019_read_byte(HL7019_REG_CTRL9, &reg);
	if (ret)
		hwlog_err("irq_work read fail\n");

	hwlog_err("fault_reg %x=0x%x\n", HL7019_REG_CTRL9, reg);

	if (reg & HL7019_REG_FAULT_BOOST_MASK) {
		hwlog_info("CHARGE_FAULT_BOOST_OCP happened\n");

		atomic_notifier_call_chain(&fault_notifier_list,
			CHARGE_FAULT_BOOST_OCP, NULL);
	}

	if (di->irq_active == 0) {
		di->irq_active = 1;
		enable_irq(di->irq_int);
	}
}

static irqreturn_t hl7019_interrupt(int irq, void *_di)
{
	struct hl7019_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return IRQ_HANDLED;
	}

	hwlog_info("hl7019 int happened %d\n", di->irq_active);

	if (di->irq_active == 1) {
		di->irq_active = 0;
		disable_irq_nosync(di->irq_int);
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("the irq is not enable, do nothing\n");
	}

	return IRQ_HANDLED;
}

static int hl7019_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct hl7019_device_info *di = NULL;
	struct device_node *np = NULL;
	struct class *power_class = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_hl7019_dev = di;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	INIT_WORK(&di->irq_work, hl7019_irq_work);

	/* check if hl7019 exist */
	if (hl7019_device_check() == CHARGE_IC_BAD) {
		hwlog_err("hl7019 not exists\n");
		ret = -EINVAL;
		goto err_no_dev;
	}

	ret = of_property_read_u32(np, "custom_cv", &(di->cust_cv));
	if (ret) {
		hwlog_err("custom_cv dts read failed\n");
		di->cust_cv = 0;
	}
	hwlog_info("custom_cv=%d\n", di->cust_cv);

	ret = of_property_read_u32(np, "hiz_iin_limit", &(di->hiz_iin_limit));
	if (ret) {
		hwlog_err("hiz_iin_limit dts read failed\n");
		di->hiz_iin_limit = 0;
	}
	hwlog_info("hiz_iin_limit=%d\n", di->hiz_iin_limit);

	di->gpio_cd = of_get_named_gpio(np, "gpio_cd", 0);
	hwlog_info("gpio_cd=%d\n", di->gpio_cd);

	if (!gpio_is_valid(di->gpio_cd)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto err_no_dev;
	}

	ret = gpio_request(di->gpio_cd, "charger_cd");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto err_no_dev;
	}

	/* set gpio to control CD pin to disable/enable hl7019 IC */
	ret = gpio_direction_output(di->gpio_cd, 0);
	if (ret) {
		hwlog_err("gpio set output fail\n");
		goto err_gpio_cd;
	}

	di->gpio_int = of_get_named_gpio(np, "gpio_int", 0);
	hwlog_info("gpio_int=%d\n", di->gpio_int);

	if (!gpio_is_valid(di->gpio_int)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto err_gpio_cd;
	}

	ret = gpio_request(di->gpio_int, "charger_int");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto err_gpio_cd;
	}

	ret = gpio_direction_input(di->gpio_int);
	if (ret) {
		hwlog_err("gpio set input fail\n");
		goto err_gpio_int;
	}

	di->irq_int = gpio_to_irq(di->gpio_int);
	if (di->irq_int < 0) {
		hwlog_err("gpio map to irq fail\n");
		ret = -EINVAL;
		goto err_gpio_int;
	}

	ret = request_irq(di->irq_int, hl7019_interrupt, IRQF_TRIGGER_FALLING,
		"charger_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		goto err_gpio_int;
	}

	disable_irq(di->irq_int);
	di->irq_active = 0;

	ret = charge_ops_register(&g_hl7019_ops);
	if (ret) {
		hwlog_err("hl7019 charge ops register fail\n");
		goto err_irq_int;
	}

	ret = hl7019_sysfs_create_group(di);
	if (ret) {
		hwlog_err("sysfs group create failed\n");
		goto err_irq_int;
	}

	power_class = hw_power_get_class();
	if (power_class) {
		if (!charge_dev)
			charge_dev = device_create(power_class, NULL, 0, NULL,
				"charger");
		if (IS_ERR(charge_dev)) {
			hwlog_err("sysfs device create failed\n");
			ret = PTR_ERR(charge_dev);
			goto err_sysfs;
		}

		ret = sysfs_create_link(&charge_dev->kobj, &di->dev->kobj,
			"hl7019");
		if (ret) {
			hwlog_err("sysfs link create failed\n");
			goto err_sysfs;
		}
	}

	/* set hl7019 boost voltage */
	ret = hl7019_set_boost_voltage(BOOSTV_5000);
	if (ret < 0)
		hwlog_err("set hl7019 boost voltage fail\n");

	hwlog_info("hl7019 probe success\n");
	return 0;

err_sysfs:
	hl7019_sysfs_remove_group(di);
err_irq_int:
	free_irq(di->irq_int, di);
err_gpio_int:
	gpio_free(di->gpio_int);
err_gpio_cd:
	gpio_free(di->gpio_cd);
err_no_dev:
	kfree(di);
	g_hl7019_dev = NULL;

	return ret;
}

static int hl7019_remove(struct i2c_client *client)
{
	struct hl7019_device_info *di = NULL;

	if (!client)
		return -ENODEV;

	di = i2c_get_clientdata(client);
	if (!di)
		return -ENODEV;

	hl7019_sysfs_remove_group(di);

	gpio_set_value(di->gpio_cd, 1);

	if (di->gpio_cd)
		gpio_free(di->gpio_cd);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	kfree(di);
	g_hl7019_dev = NULL;

	return 0;
}

static const struct of_device_id g_hl7019_of_match[] = {
	{
		.compatible = "huawei,hl7019_charger",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id g_hl7019_i2c_id[] = {
	{ "hl7019_charger", 0 }, {}
};
MODULE_DEVICE_TABLE(i2c, g_hl7019_i2c_id);

static struct i2c_driver g_hl7019_driver = {
	.probe = hl7019_probe,
	.remove = hl7019_remove,
	.id_table = g_hl7019_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "hl7019_charger",
		.of_match_table = of_match_ptr(g_hl7019_of_match),
	},
};

static int __init hl7019_init(void)
{
	return i2c_add_driver(&g_hl7019_driver);
}

static void __exit hl7019_exit(void)
{
	i2c_del_driver(&g_hl7019_driver);
}

module_init(hl7019_init);
module_exit(hl7019_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hl7019 charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
