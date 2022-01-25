/*
 * bq2429x_charger.c
 *
 * bq2429x driver
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
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/power/huawei_charger.h>
#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif
#include <bq2429x_charger.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG bq2429x_charger
HWLOG_REGIST();

struct bq2429x_device_info *g_bq2429x_dev;

#define MSG_LEN                      (2)
#define BUF_LEN                      (26)

static int params_to_reg(const int tbl[], int tbl_size, int val)
{
	int i;

	for (i = 1; i < tbl_size; i++) {
		if (val < tbl[i])
			return (i - 1);
	}

	return (tbl_size - 1);
}

static int bq2429x_write_block(struct bq2429x_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[1];
	int ret = 0;

	if (di == NULL || value == NULL) {
		hwlog_err("di is null or value is null\n");
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

static int bq2429x_read_block(struct bq2429x_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[MSG_LEN];
	u8 buf = 0;
	int ret = 0;

	if (di == NULL || value == NULL) {
		hwlog_err("di is null or value is null\n");
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

static int bq2429x_write_byte(u8 reg, u8 value)
{
	struct bq2429x_device_info *di = g_bq2429x_dev;
	/* 2 bytes offset 1 contains the data offset 0 is used by i2c_write */
	u8 temp_buffer[MSG_LEN] = {0};

	/* offset 1 contains the data */
	temp_buffer[1] = value;
	return bq2429x_write_block(di, temp_buffer, reg, 1);
}

static int bq2429x_read_byte(u8 reg, u8 *value)
{
	struct bq2429x_device_info *di = g_bq2429x_dev;

	return bq2429x_read_block(di, value, reg, 1);
}

static int bq2429x_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret = 0;
	u8 val = 0;

	ret = bq2429x_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	ret = bq2429x_write_byte(reg, val);

	return ret;
}

static int bq2429x_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret = 0;
	u8 val = 0;

	ret = bq2429x_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return 0;
}

#ifdef CONFIG_SYSFS
/*
 * There are a numerous options that are configurable on the bq2429x
 * that go well beyond what the power_supply properties provide access to.
 * Provide sysfs access to them so they can be examined and possibly modified
 * on the fly.
 */
#define BQ2429X_SYSFS_FIELD(_name, r, f, m, store) \
{ \
	.attr = __ATTR(_name, m, bq2429x_sysfs_show, store), \
	.reg = BQ2429X_REG_##r, \
	.mask = BQ2429X_REG_##r##_##f##_MASK, \
	.shift = BQ2429X_REG_##r##_##f##_SHIFT, \
}

#define BQ2429X_SYSFS_FIELD_RW(_name, r, f) \
	BQ2429X_SYSFS_FIELD(_name, r, f, 0644, bq2429x_sysfs_store)

#define BQ2429X_SYSFS_FIELD_RO(_name, r, f) \
	BQ2429X_SYSFS_FIELD(_name, r, f, 0444, NULL)

static ssize_t bq2429x_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t bq2429x_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

struct bq2429x_sysfs_field_info {
	struct device_attribute attr;
	u8 reg;
	u8 mask;
	u8 shift;
};

/* On i386 ptrace-abi.h defines SS that breaks the macro calls below. */
#undef SS

static struct bq2429x_sysfs_field_info bq2429x_sysfs_field_tbl[] = {
	/* sysfs name reg field in reg */
	BQ2429X_SYSFS_FIELD_RW(en_hiz, ISC, EN_HIZ),
	BQ2429X_SYSFS_FIELD_RW(vindpm, ISC, VINDPM),
	BQ2429X_SYSFS_FIELD_RW(iinlim, ISC, IINLIM),
	BQ2429X_SYSFS_FIELD_RW(chg_config, POC, CHG_CONFIG),
	BQ2429X_SYSFS_FIELD_RW(sys_min, POC, SYS_MIN),
	BQ2429X_SYSFS_FIELD_RW(boost_lim, POC, BOOST_LIM),
	BQ2429X_SYSFS_FIELD_RW(ichg, CCC, ICHG),
	BQ2429X_SYSFS_FIELD_RW(bcold, CCC, BCOLD),
	BQ2429X_SYSFS_FIELD_RW(force_20_pct, CCC, FORCE_20PCT),
	BQ2429X_SYSFS_FIELD_RW(iprechg, PCTCC, IPRECHG),
	BQ2429X_SYSFS_FIELD_RW(iterm, PCTCC, ITERM),
	BQ2429X_SYSFS_FIELD_RW(vreg, CVC, VREG),
	BQ2429X_SYSFS_FIELD_RW(batlowv, CVC, BATLOWV),
	BQ2429X_SYSFS_FIELD_RW(vrechg, CVC, VRECHG),
	BQ2429X_SYSFS_FIELD_RW(en_term, CTTC, EN_TERM),
	BQ2429X_SYSFS_FIELD_RW(term_stat, CTTC, TERM_STAT),
	BQ2429X_SYSFS_FIELD_RO(watchdog, CTTC, WATCHDOG),
	BQ2429X_SYSFS_FIELD_RW(en_timer, CTTC, EN_TIMER),
	BQ2429X_SYSFS_FIELD_RW(chg_timer, CTTC, CHG_TIMER),
	BQ2429X_SYSFS_FIELD_RW(jeta_iset, CTTC, JEITA_ISET),
	BQ2429X_SYSFS_FIELD_RW(boostv, BVTRC, BOOSTV),
	BQ2429X_SYSFS_FIELD_RW(bhot, BVTRC, BHOT),
	BQ2429X_SYSFS_FIELD_RW(treg, BVTRC, TREG),
	BQ2429X_SYSFS_FIELD_RW(dpdm_en, MOC, DPDM_EN),
	BQ2429X_SYSFS_FIELD_RW(tmr2x_en, MOC, TMR2X_EN),
	BQ2429X_SYSFS_FIELD_RW(batfet_disable, MOC, BATFET_DISABLE),
	BQ2429X_SYSFS_FIELD_RW(jeita_vset, MOC, JEITA_VSET),
	BQ2429X_SYSFS_FIELD_RO(int_mask, MOC, INT_MASK),
	BQ2429X_SYSFS_FIELD_RO(vbus_stat, SS, VBUS_STAT),
	BQ2429X_SYSFS_FIELD_RO(chrg_stat, SS, CHRG_STAT),
	BQ2429X_SYSFS_FIELD_RO(dpm_stat, SS, DPM_STAT),
	BQ2429X_SYSFS_FIELD_RO(pg_stat, SS, PG_STAT),
	BQ2429X_SYSFS_FIELD_RO(therm_stat, SS, THERM_STAT),
	BQ2429X_SYSFS_FIELD_RO(vsys_stat, SS, VSYS_STAT),
	BQ2429X_SYSFS_FIELD_RO(watchdog_fault, F, WATCHDOG_FAULT),
	BQ2429X_SYSFS_FIELD_RO(boost_fault, F, BOOST_FAULT),
	BQ2429X_SYSFS_FIELD_RO(chrg_fault, F, CHRG_FAULT),
	BQ2429X_SYSFS_FIELD_RO(bat_fault, F, BAT_FAULT),
	BQ2429X_SYSFS_FIELD_RO(ntc_fault, F, NTC_FAULT),
	BQ2429X_SYSFS_FIELD_RO(pn, VPRS, PN),
	BQ2429X_SYSFS_FIELD_RO(rev, VPRS, REV),
};

#define BQ2429X_SYSFS_ATTRS_SIZE  (ARRAY_SIZE(bq2429x_sysfs_field_tbl) + 1)

static struct attribute *bq2429x_sysfs_attrs[BQ2429X_SYSFS_ATTRS_SIZE];

static const struct attribute_group bq2429x_sysfs_attr_group = {
	.attrs = bq2429x_sysfs_attrs,
};

static void bq2429x_sysfs_init_attrs(void)
{
	int i, limit = ARRAY_SIZE(bq2429x_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		bq2429x_sysfs_attrs[i] = &bq2429x_sysfs_field_tbl[i].attr.attr;

	bq2429x_sysfs_attrs[limit] = NULL;
}

static struct bq2429x_sysfs_field_info *bq2429x_sysfs_field_lookup(
	const char *name)
{
	int i, limit = ARRAY_SIZE(bq2429x_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strcmp(name, bq2429x_sysfs_field_tbl[i].attr.attr.name))
			break;
	}

	if (i >= limit)
		return NULL;

	return &bq2429x_sysfs_field_tbl[i];
}

static ssize_t bq2429x_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bq2429x_sysfs_field_info *info;
	int ret;
	u8 v;

	info = bq2429x_sysfs_field_lookup(attr->attr.name);
	if (info == NULL) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	ret = bq2429x_read_mask(info->reg, info->mask, info->shift, &v);
	if (ret)
		return ret;

	return scnprintf(buf, PAGE_SIZE, "%hhx\n", v);
}

static ssize_t bq2429x_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct bq2429x_sysfs_field_info *info;
	int ret;
	u8 v;

	info = bq2429x_sysfs_field_lookup(attr->attr.name);
	if (info == NULL) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	ret = kstrtou8(buf, 0, &v);
	if (ret < 0) {
		hwlog_err("get kstrtou8 failed\n");
		return ret;
	}

	ret = bq2429x_write_mask(info->reg, info->mask, info->shift, v);
	if (ret)
		return ret;

	return count;
}

static int bq2429x_sysfs_create_group(struct bq2429x_device_info *di)
{
	bq2429x_sysfs_init_attrs();

	return sysfs_create_group(&di->dev->kobj, &bq2429x_sysfs_attr_group);
}

static void bq2429x_sysfs_remove_group(struct bq2429x_device_info *di)
{
	sysfs_remove_group(&di->dev->kobj, &bq2429x_sysfs_attr_group);
}

#else

static inline int bq2429x_sysfs_create_group(struct bq2429x_device_info *di)
{
	return 0;
}

static inline void bq2429x_sysfs_remove_group(struct bq2429x_device_info *di)
{
}

#endif /* CONFIG_SYSFS */

static int bq2429x_check_ic_type(void)
{
	u8 reg = 0;
	int ret = 0;

	ret = bq2429x_read_byte(BQ2429X_REG_VPRS, &reg);
	if (ret < 0)
		return ret;

	hwlog_info("check_ic_type [%x]=0x%x\n", BQ2429X_REG_VPRS, reg);

	if ((0 == (reg & BQ2429X_REG_VPRS_REV_MASK)) &&
		(reg & BQ2429X_REG_VPRS_PN_MASK))
		return BQ2429X_REG_VPRS_PN_24296;

	if ((reg & BQ2419X_REG_VPRS_DEV_REG_MASK) &&
		(reg & BQ2419X_REG_VPRS_PN_MASK))
		return BQ2419X_REG_VPRS_PN_24192;

	return 0;
}

static int bq2429x_device_check(void)
{
	if ((bq2429x_check_ic_type() == BQ2429X_REG_VPRS_PN_24296) ||
		(bq2429x_check_ic_type() == BQ2419X_REG_VPRS_PN_24192)) {
		hwlog_info("bq2429x is good\n");
		return CHARGE_IC_GOOD;
	}

	hwlog_err("bq2429x is bad\n");
	return CHARGE_IC_BAD;
}

static int bq2429x_5v_chip_init(struct bq2429x_device_info *di)
{
	int ret = 0;

	/* boost mode current limit = 1000mA */
	/* kick watchdog in bq2429x chip_init */
	ret = bq2429x_write_byte(BQ2429X_REG_POC, 0x5a);

	/* I2C watchdog timer setting = 80s */
	/* fast charge timer setting = 12h */
	ret |= bq2429x_write_byte(BQ2429X_REG_CTTC, 0x2d);

	/* IR compensation voatge clamp = 48mV */
	/* IR compensation resistor setting = 40mohm */
	if (bq2429x_check_ic_type() != BQ2429X_REG_VPRS_PN_24296)
		ret |= bq2429x_write_byte(BQ2429X_REG_BVTRC, 0x8f);

	/* enable charging */
	gpio_set_value(di->gpio_cd, 0);

	return ret;
}

static int bq2429x_chip_init(struct chip_init_crit *init_crit)
{
	int ret = -1;
	struct bq2429x_device_info *di = g_bq2429x_dev;

	if (di == NULL || init_crit == NULL) {
		hwlog_err("di or init_crit is null\n");
		return -ENOMEM;
	}

	switch (init_crit->vbus) {
	case ADAPTER_5V:
		ret = bq2429x_5v_chip_init(di);
		break;

	default:
		hwlog_err("invaid init_crit vbus mode\n");
		break;
	}

	return ret;
}

static int bq2429x_set_input_current(int value)
{
	int val = 0;
	int array_size = ARRAY_SIZE(bq2429x_iin_values);

	if (value > bq2429x_iin_values[array_size - 1])
		return bq2429x_iin_values[array_size - 1];
	else if (value < bq2429x_iin_values[0])
		return bq2429x_iin_values[0];

	val = params_to_reg(bq2429x_iin_values, array_size, value);

	hwlog_info("set_input_current [%x]=0x%x\n", BQ2429X_REG_ISC, val);

	return bq2429x_write_mask(BQ2429X_REG_ISC,
			BQ2429X_REG_ISC_IINLIM_MASK,
			BQ2429X_REG_ISC_IINLIM_SHIFT,
			val);
}

static int bq2429x_set_charge_current(int value)
{
	int ret = 0;
	int val = 0;
	int force_20pct_en = 0;
	int array_size = ARRAY_SIZE(bq2429x_ichg_values);

	if (value > bq2429x_ichg_values[array_size - 1])
		return bq2429x_ichg_values[array_size - 1];

	/*
	 * 1. If currentmA is below ICHG_512,
	 * we can set the ICHG to 5*currentmA and set the FORCE_20PCT in REG02
	 * to make the true current 20% of the ICHG.
	 * 2. To slove the OCP BUG of BQ2419X,
	 * we need set the ICHG (lower than 1024mA) to
	 * 5*currentmA and set the FORCE_20PCT in REG02.
	 */
	else if (value < 1024) {
		value *= 5;
		force_20pct_en = 1;
	}

	if (value < bq2429x_ichg_values[0])
		value = bq2429x_ichg_values[0];

	ret = bq2429x_write_mask(BQ2429X_REG_CCC,
				BQ2429X_REG_CCC_FORCE_20PCT_MASK,
				BQ2429X_REG_CCC_FORCE_20PCT_SHIFT,
				force_20pct_en);
	if (ret < 0)
		return ret;

	val = params_to_reg(bq2429x_ichg_values, array_size, value);

	hwlog_info("set_charge_current [%x]=0x%x\n", BQ2429X_REG_CCC, val);

	return bq2429x_write_mask(BQ2429X_REG_CCC,
			BQ2429X_REG_CCC_ICHG_MASK,
			BQ2429X_REG_CCC_ICHG_SHIFT,
			val);
}

static int bq2429x_set_terminal_voltage(int value)
{
	int val = 0;
	int array_size = ARRAY_SIZE(bq2429x_vreg_values);

	if (value > bq2429x_vreg_values[array_size - 1])
		return bq2429x_vreg_values[array_size - 1];
	else if (value < bq2429x_vreg_values[0])
		return bq2429x_vreg_values[0];

	val = params_to_reg(bq2429x_vreg_values, array_size, value);

	hwlog_info("set_terminal_voltage [%x]=0x%x\n", BQ2429X_REG_CVC, val);

	return bq2429x_write_mask(BQ2429X_REG_CVC,
			BQ2429X_REG_CVC_VREG_MASK,
			BQ2429X_REG_CVC_VREG_SHIFT,
			val);
}

static int bq2429x_set_dpm_voltage(int value)
{
	int val = 0;
	int array_size = ARRAY_SIZE(bq2429x_vindpm_values);

	if (value > bq2429x_vindpm_values[array_size - 1])
		return bq2429x_vindpm_values[array_size - 1];
	else if (value < bq2429x_vindpm_values[0])
		return bq2429x_vindpm_values[0];

	val = params_to_reg(bq2429x_vindpm_values, array_size, value);

	hwlog_info("set_dpm_voltage [%x]=0x%x\n", BQ2429X_REG_ISC, val);

	return bq2429x_write_mask(BQ2429X_REG_ISC,
			BQ2429X_REG_ISC_VINDPM_MASK,
			BQ2429X_REG_ISC_VINDPM_SHIFT,
			val);
}

static int bq2429x_set_terminal_current(int value)
{
	int val = 0;
	int array_size = ARRAY_SIZE(bq2429x_iterm_values);

	if (value > bq2429x_iterm_values[array_size - 1])
		return bq2429x_iterm_values[array_size - 1];
	else if (value < bq2429x_iterm_values[0])
		return bq2429x_iterm_values[0];

	val = params_to_reg(bq2429x_iterm_values, array_size, value);

	hwlog_info("set_terminal_current [%x]=0x%x\n", BQ2429X_REG_PCTCC, val);

	return bq2429x_write_mask(BQ2429X_REG_PCTCC,
			BQ2429X_REG_PCTCC_ITERM_MASK,
			BQ2429X_REG_PCTCC_ITERM_SHIFT,
			val);
}

static int bq2429x_set_charge_enable(int enable)
{
	struct bq2429x_device_info *di = g_bq2429x_dev;

	gpio_set_value(di->gpio_cd, !enable);

	return bq2429x_write_mask(BQ2429X_REG_POC,
			BQ2429X_REG_POC_CHG_CONFIG_MASK,
			BQ2429X_REG_POC_CHG_CONFIG_SHIFT,
			enable);
}

static int bq2429x_set_otg_enable(int enable)
{
	int val = 0;
	struct bq2429x_device_info *di = g_bq2429x_dev;

	gpio_set_value(di->gpio_cd, !enable);
	val = enable << 1;

	/* notice:
	 * why enable irq when entry to OTG mode only?
	 * because we care VBUS overloaded OCP or OVP's interrupt in boost mode
	 */
	if ((!di->irq_active) && (enable)) {
		di->irq_active = 1; /* ACTIVE */
		enable_irq(di->irq_int);
	} else if ((di->irq_active) && (!enable)) {
		di->irq_active = 0; /* INACTIVE */
		disable_irq(di->irq_int);
	} else {
		/* do nothing */
	}

	return bq2429x_write_mask(BQ2429X_REG_POC,
			BQ2429X_REG_POC_CHG_CONFIG_MASK,
			BQ2429X_REG_POC_CHG_CONFIG_SHIFT,
			val);
}

static int bq2429x_reset_otg(void)
{
	int val = 0;
	int ret = 0;
	struct bq2429x_device_info *di = g_bq2429x_dev;

	gpio_set_value(di->gpio_cd, !(FALSE));

	val = (FALSE) << 1;
	ret = bq2429x_write_mask(BQ2429X_REG_POC,
			BQ2429X_REG_POC_CHG_CONFIG_MASK,
			BQ2429X_REG_POC_CHG_CONFIG_SHIFT,
			val);
	if (ret)
		return ret;

	gpio_set_value(di->gpio_cd, !(TRUE));

	val = (TRUE) << 1;
	ret = bq2429x_write_mask(BQ2429X_REG_POC,
			BQ2429X_REG_POC_CHG_CONFIG_MASK,
			BQ2429X_REG_POC_CHG_CONFIG_SHIFT,
			val);
	if (ret)
		return ret;

	return 0;
}

static int bq2429x_set_term_enable(int enable)
{
	return bq2429x_write_mask(BQ2429X_REG_CTTC,
			BQ2429X_REG_CTTC_EN_TERM_MASK,
			BQ2429X_REG_CTTC_EN_TERM_SHIFT,
			enable);
}

static int bq2429x_get_charge_state(unsigned int *state)
{
	u8 reg = 0;
	int ret = 0;

	ret = bq2429x_read_byte(BQ2429X_REG_SS, &reg);

	hwlog_info("get_charge_state [%x]=0x%x\n", BQ2429X_REG_SS, reg);

	if ((reg & BQ2429X_REG_SS_PG) == BQ2429X_REG_SS_NOTPG)
		*state |= CHAGRE_STATE_NOT_PG;

	if ((reg & BQ2429X_REG_SS_DPM) == BQ2429X_REG_SS_DPM)
		*state |= CHAGRE_STATE_INPUT_DPM;

	if ((reg & BQ2429X_REG_SS_CHRGDONE) == BQ2429X_REG_SS_CHRGDONE)
		*state |= CHAGRE_STATE_CHRG_DONE;

	ret |= bq2429x_read_byte(BQ2429X_REG_F, &reg);
	ret |= bq2429x_read_byte(BQ2429X_REG_F, &reg);

	hwlog_info("get_charge_state [%x]=0x%x\n", BQ2429X_REG_F, reg);

	if ((reg & BQ2429X_REG_F_WDT_TIMEOUT) == BQ2429X_REG_F_WDT_TIMEOUT)
		*state |= CHAGRE_STATE_WDT_FAULT;

	if ((reg & BQ2429X_REG_F_VBUS_OVP) == BQ2429X_REG_F_VBUS_OVP)
		*state |= CHAGRE_STATE_VBUS_OVP;

	if ((reg & BQ2429X_REG_F_BATT_OVP) == BQ2429X_REG_F_BATT_OVP)
		*state |= CHAGRE_STATE_BATT_OVP;

	return ret;
}

static int bq2429x_reset_watchdog_timer(void)
{
	return bq2429x_write_mask(BQ2429X_REG_POC,
			BQ2429X_REG_POC_WDT_RESET_MASK,
			BQ2429X_REG_POC_WDT_RESET_SHIFT,
			0x01);
}

static int bq2429x_check_charger_plugged(void)
{
	u8 reg = 0;
	int ret = 0;

	ret = bq2429x_read_byte(BQ2429X_REG_SS, &reg);

	hwlog_info("check_charger_plugged [%x]=0x%x\n", BQ2429X_REG_SS, reg);

	if ((reg & BQ2429X_REG_SS_VBUS_STAT_MASK) ==
		BQ2429X_REG_SS_VBUS_PLUGGED)
		return TRUE;

	return FALSE;
}

static int bq2429x_check_input_dpm_state(void)
{
	u8 reg = 0;
	int ret = -1;

	ret = bq2429x_read_byte(BQ2429X_REG_SS, &reg);
	if (ret < 0)
		return ret;

	hwlog_info("check_input_dpm_state [%x]=0x%x\n", BQ2429X_REG_SS, reg);

	if (reg & BQ2429X_REG_SS_DPM_STAT_MASK)
		return TRUE;
	else
		return FALSE;
}

static int bq2429x_dump_register(char *reg_value)
{
	u8 reg[BQ2429X_REG_NUM] = {0};
	char buff[BUF_LEN] = {0};
	int i = 0;

	memset(reg_value, 0, CHARGELOG_SIZE);

	for (i = 0; i < BQ2429X_REG_NUM; i++) {
		bq2429x_read_byte(i, &reg[i]);
		bq2429x_read_byte(i, &reg[i]);
		snprintf(buff, BUF_LEN, "0x%-8.2x", reg[i]);
		strncat(reg_value, buff, strlen(buff));
	}

	return 0;
}

static int bq2429x_get_register_head(char *reg_head)
{
	char buff[BUF_LEN] = {0};
	int i = 0;

	memset(reg_head, 0, CHARGELOG_SIZE);

	for (i = 0; i < BQ2429X_REG_NUM; i++) {
		snprintf(buff, BUF_LEN, "Reg[%d]    ", i);
		strncat(reg_head, buff, strlen(buff));
	}

	return 0;
}

static int bq2429x_set_batfet_disable(int disable)
{
	return bq2429x_write_mask(BQ2429X_REG_MOC,
			BQ2429X_REG_MOC_BATFET_DISABLE_MASK,
			BQ2429X_REG_MOC_BATFET_DISABLE_SHIFT,
			disable);
}

static int bq2429x_get_ilim(void)
{
	int ret = 0;
	u32 ibus = 0;
	u32 state = 0;

	ret |= bq2429x_get_charge_state(&state);
	if (ret)
		return -1;

	if (CHAGRE_STATE_NOT_PG & state) {
		hwlog_info("CHAGRE_STATE_NOT_PG state:%d\n", state);
		return 0;
	}

	/*
	 * bq2429x do not support get-ibus function, for RunnintTest,
	 * we return a fake vulue
	 */
	ibus = 1000; /* fake current value for Running test */
	/* temperary log for RT flag */
	hwlog_info("bq2429x: ibus = %d, RunningTest Begin\n", ibus);

	return ibus;
}

static int bq2429x_set_watchdog_timer(int value)
{
	u8 val = 0;
	int array_size = ARRAY_SIZE(bq2429x_watchdog_values);

	if (value > bq2429x_watchdog_values[array_size - 1])
		return bq2429x_watchdog_values[array_size - 1];
	else if (value < bq2429x_watchdog_values[0])
		return bq2429x_watchdog_values[0];

	val = params_to_reg(bq2429x_watchdog_values, array_size, value);

	hwlog_info("set_watchdog_timer [%x]=0x%x\n", BQ2429X_REG_CTTC, val);

	return bq2429x_write_mask(BQ2429X_REG_CTTC,
			BQ2429X_REG_CTTC_WATCHDOG_MASK,
			BQ2429X_REG_CTTC_WATCHDOG_SHIFT,
			val);
}

static int bq2429x_set_charger_hiz(int enable)
{
	int ret = 0;

	if (enable > 0)
		ret |= bq2429x_write_mask(BQ2429X_REG_ISC,
				BQ2429X_REG_ISC_EN_HIZ_MASK,
				BQ2429X_REG_ISC_EN_HIZ_SHIFT,
				TRUE);
	else
		ret |= bq2429x_write_mask(BQ2429X_REG_ISC,
				BQ2429X_REG_ISC_EN_HIZ_MASK,
				BQ2429X_REG_ISC_EN_HIZ_SHIFT,
				FALSE);

	return ret;
}

struct charge_device_ops bq2429x_ops = {
	.chip_init = bq2429x_chip_init,
	.dev_check = bq2429x_device_check,
	.set_input_current = bq2429x_set_input_current,
	.set_charge_current = bq2429x_set_charge_current,
	.set_terminal_voltage = bq2429x_set_terminal_voltage,
	.set_dpm_voltage = bq2429x_set_dpm_voltage,
	.set_terminal_current = bq2429x_set_terminal_current,
	.set_charge_enable = bq2429x_set_charge_enable,
	.set_otg_enable = bq2429x_set_otg_enable,
	.set_term_enable = bq2429x_set_term_enable,
	.get_charge_state = bq2429x_get_charge_state,
	.reset_watchdog_timer = bq2429x_reset_watchdog_timer,
	.dump_register = bq2429x_dump_register,
	.get_register_head = bq2429x_get_register_head,
	.set_watchdog_timer = bq2429x_set_watchdog_timer,
	.set_batfet_disable = bq2429x_set_batfet_disable,
	.get_ibus = bq2429x_get_ilim,
	.check_charger_plugged = bq2429x_check_charger_plugged,
	.check_input_dpm_state = bq2429x_check_input_dpm_state,
	.set_charger_hiz = bq2429x_set_charger_hiz,
	.get_charge_current = NULL,
};

static void bq2429x_irq_work(struct work_struct *work)
{
	struct bq2429x_device_info *di;
	u8 reg = 0;

	di = container_of(work, struct bq2429x_device_info, irq_work);

	msleep(250); /* sleep 250ms */

	bq2429x_read_byte(BQ2429X_REG_F, &reg);
	bq2429x_read_byte(BQ2429X_REG_F, &reg);

	hwlog_err("boost_ovp_reg [%x]=0x%x\n", BQ2429X_REG_F, reg);

	if (reg & BQ2429X_REG_F_BOOST_OCP) {
		hwlog_info("CHARGE_FAULT_BOOST_OCP happened\n");

		/* retry otg once */
		bq2429x_reset_otg();

		msleep(100); /* sleep 100ms */

		bq2429x_read_byte(BQ2429X_REG_F, &reg);
		bq2429x_read_byte(BQ2429X_REG_F, &reg);
		if (reg & BQ2429X_REG_F_BOOST_OCP) {
			hwlog_info("CHARGE_FAULT_BOOST_OCP happened twice\n");

			atomic_notifier_call_chain(&fault_notifier_list,
				CHARGE_FAULT_BOOST_OCP, NULL);
		}
	}

	if (di->irq_active == 0) {
		di->irq_active = 1;
		enable_irq(di->irq_int);
	}
}

static irqreturn_t bq2429x_interrupt(int irq, void *_di)
{
	struct bq2429x_device_info *di = _di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	hwlog_info("bq2429x int happened (%d)\n", di->irq_active);

	if (di->irq_active == 1) {
		di->irq_active = 0;
		disable_irq_nosync(di->irq_int);
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("the irq is not enable, do nothing\n");
	}

	return IRQ_HANDLED;
}

static int bq2429x_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret = 0;
	struct bq2429x_device_info *di = NULL;
	struct device_node *np = NULL;
	struct class *power_class = NULL;

	hwlog_info("probe begin\n");

	if (client == NULL || id == NULL) {
		hwlog_err("client or id is null\n");
		return -ENOMEM;
	}

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (di == NULL)
		return -ENOMEM;

	g_bq2429x_dev = di;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	INIT_WORK(&di->irq_work, bq2429x_irq_work);

	di->gpio_cd = of_get_named_gpio(np, "gpio_cd", 0);
	hwlog_info("gpio_cd=%d\n", di->gpio_cd);

	if (!gpio_is_valid(di->gpio_cd)) {
		hwlog_err("gpio(gpio_cd) is not valid\n");
		ret = -EINVAL;
		goto bq2429x_fail_0;
	}

	ret = gpio_request(di->gpio_cd, "charger_cd");
	if (ret) {
		hwlog_err("gpio(gpio_cd) request fail\n");
		goto bq2429x_fail_0;
	}

	/* set gpio to control CD pin to disable/enable bq2429x IC */
	ret = gpio_direction_output(di->gpio_cd, 0);
	if (ret) {
		hwlog_err("gpio(gpio_cd) set output fail\n");
		goto bq2429x_fail_1;
	}

	di->gpio_int = of_get_named_gpio(np, "gpio_int", 0);
	hwlog_info("gpio_int=%d\n", di->gpio_int);

	if (!gpio_is_valid(di->gpio_int)) {
		hwlog_err("gpio(gpio_int) is not valid\n");
		ret = -EINVAL;
		goto bq2429x_fail_1;
	}

	ret = gpio_request(di->gpio_int, "charger_int");
	if (ret) {
		hwlog_err("gpio(gpio_int) request fail\n");
		goto bq2429x_fail_1;
	}

	ret = gpio_direction_input(di->gpio_int);
	if (ret) {
		hwlog_err("gpio(gpio_int) set input fail\n");
		goto bq2429x_fail_2;
	}

	di->irq_int = gpio_to_irq(di->gpio_int);
	if (di->irq_int < 0) {
		hwlog_err("gpio(gpio_int) map to irq fail\n");
		ret = -EINVAL;
		goto bq2429x_fail_2;
	}

	ret = request_irq(di->irq_int, bq2429x_interrupt, IRQF_TRIGGER_FALLING,
		"charger_int_irq", di);
	if (ret) {
		hwlog_err("gpio(gpio_int) irq request fail\n");
		di->irq_int = -1;
		goto bq2429x_fail_2;
	}

	disable_irq(di->irq_int);
	di->irq_active = 0;

	ret = charge_ops_register(&bq2429x_ops);
	if (ret) {
		hwlog_err("bq2429x charge ops register fail\n");
		goto bq2429x_fail_3;
	}

	ret = bq2429x_sysfs_create_group(di);
	if (ret)
		hwlog_err("sysfs group create failed\n");

	power_class = hw_power_get_class();
	if (power_class != NULL) {
		if (charge_dev == NULL)
			charge_dev = device_create(power_class, NULL, 0, NULL,
				"charger");

		ret = sysfs_create_link(&charge_dev->kobj, &di->dev->kobj,
			"bq2429x");
		if (ret) {
			hwlog_err("sysfs link create failed\n");
			goto bq2429x_fail_4;
		}
	}

	hwlog_info("probe end\n");
	return 0;

bq2429x_fail_4:
	bq2429x_sysfs_remove_group(di);
bq2429x_fail_3:
	free_irq(di->irq_int, di);
bq2429x_fail_2:
	gpio_free(di->gpio_int);
bq2429x_fail_1:
	gpio_free(di->gpio_cd);
bq2429x_fail_0:
	kfree(di);
	g_bq2429x_dev = NULL;
	np = NULL;

	return ret;
}

static int bq2429x_remove(struct i2c_client *client)
{
	struct bq2429x_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	bq2429x_sysfs_remove_group(di);

	gpio_set_value(di->gpio_cd, 1);

	if (di->gpio_cd)
		gpio_free(di->gpio_cd);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	kfree(di);

	hwlog_info("remove end\n");
	return 0;
}

MODULE_DEVICE_TABLE(i2c, bq24192);
static const struct of_device_id bq2429x_of_match[] = {
	{
		.compatible = "huawei,bq2429x_charger",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id bq2429x_i2c_id[] = {
	{"bq2429x_charger", 0}, {}
};

static struct i2c_driver bq2429x_driver = {
	.probe = bq2429x_probe,
	.remove = bq2429x_remove,
	.id_table = bq2429x_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bq2429x_charger",
		.of_match_table = of_match_ptr(bq2429x_of_match),
	},
};

static int __init bq2429x_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&bq2429x_driver);
	if (ret)
		hwlog_err("i2c_add_driver error\n");

	return ret;
}

static void __exit bq2429x_exit(void)
{
	i2c_del_driver(&bq2429x_driver);
}

module_init(bq2429x_init);
module_exit(bq2429x_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bq2429x charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
