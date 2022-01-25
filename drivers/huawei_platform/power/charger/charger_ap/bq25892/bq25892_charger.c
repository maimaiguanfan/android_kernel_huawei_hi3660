/*
 * bq25892_charger.c
 *
 * bq25892 driver
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
#include <linux/hisi/usb/hisi_usb.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <linux/raid/pq.h>
#include <huawei_platform/power/huawei_charger.h>
#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif
#include <bq25892_charger.h>
#include <linux/hisi/hisi_adc.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG bq25892_charger
HWLOG_REGIST();

struct bq25892_device_info *g_bq25892_dev;

/* configured in dts based on the real value of the iin limit resistance */
static unsigned int rilim = BQ25892_RILIM_124_OHM;
/* configured in dts based on the real adc channel number */
static unsigned int adc_channel_iin = BQ25892_ADC_CHANNEL_IIN_10;
static unsigned int adc_channel_vbat_sys = BQ25892_ADC_CHANNEL_VBAT_SYS;
static u32 is_board_type; /*0:sft 1:udp 2:asic */
static int ico_current_mode;

static int iin_set = IINLIM_MIN_100;

#define MSG_LEN                      (2)
#define BUF_LEN                      (26)

static int bq25892_write_block(struct bq25892_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[1];
	int ret = 0;

	if (di == NULL || value == NULL) {
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
		hwlog_err("write_block failed[%x]\n", reg);
		if (ret < 0)
			return ret;
		else
			return -EIO;
	} else {
		return 0;
	}
}

static int bq25892_read_block(struct bq25892_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[MSG_LEN];
	u8 buf = 0;
	int ret = 0;

	if (di == NULL || value == NULL) {
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
		hwlog_err("read_block failed[%x]\n", reg);
		if (ret < 0)
			return ret;
		else
			return -EIO;
	} else {
		return 0;
	}
}

static int bq25892_write_byte(u8 reg, u8 value)
{
	struct bq25892_device_info *di = g_bq25892_dev;
	/* 2 bytes offset 1 contains the data offset 0 is used by i2c_write */
	u8 temp_buffer[MSG_LEN] = {0};

	/* offset 1 contains the data */
	temp_buffer[1] = value;
	return bq25892_write_block(di, temp_buffer, reg, 1);
}

static int bq25892_read_byte(u8 reg, u8 *value)
{
	struct bq25892_device_info *di = g_bq25892_dev;

	return bq25892_read_block(di, value, reg, 1);
}

static int bq25892_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret = 0;
	u8 val = 0;

	ret = bq25892_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	ret = bq25892_write_byte(reg, val);

	return ret;
}

static int bq25892_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret = 0;
	u8 val = 0;

	ret = bq25892_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return 0;
}

#ifdef CONFIG_SYSFS
/*
 * There are a numerous options that are configurable on the bq25892
 * that go well beyond what the power_supply properties provide access to.
 * Provide sysfs access to them so they can be examined and possibly modified
 * on the fly.
 */
#define BQ25892_SYSFS_FIELD(_name, r, f, m, store) \
{ \
	.attr = __ATTR(_name, m, bq25892_sysfs_show, store), \
	.reg = BQ25892_REG_##r, \
	.mask = BQ25892_REG_##r##_##f##_MASK, \
	.shift = BQ25892_REG_##r##_##f##_SHIFT, \
}

#define BQ25892_SYSFS_FIELD_RW(_name, r, f) \
	BQ25892_SYSFS_FIELD(_name, r, f, 0644, bq25892_sysfs_store)

#define BQ25892_SYSFS_FIELD_RO(_name, r, f) \
	BQ25892_SYSFS_FIELD(_name, r, f, 0444, NULL)

static ssize_t bq25892_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t bq25892_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

struct bq25892_sysfs_field_info {
	struct device_attribute attr;
	u8 reg;
	u8 mask;
	u8 shift;
};

static struct bq25892_sysfs_field_info bq25892_sysfs_field_tbl[] = {
	/* sysfs name reg field in reg */
	BQ25892_SYSFS_FIELD_RW(en_hiz, 00, EN_HIZ),
	BQ25892_SYSFS_FIELD_RW(en_ilim, 00, EN_ILIM),
	BQ25892_SYSFS_FIELD_RW(iinlim, 00, IINLIM),
	BQ25892_SYSFS_FIELD_RW(dpm_os, 01, VINDPM_OS),
	BQ25892_SYSFS_FIELD_RW(conv_start, 02, CONV_START),
	BQ25892_SYSFS_FIELD_RW(ico_en, 02, ICO_EN),
	BQ25892_SYSFS_FIELD_RW(force_dpdm, 02, FORCE_DPDM),
	BQ25892_SYSFS_FIELD_RW(auto_dpdm_en, 02, AUTO_DPDM_EN),
	BQ25892_SYSFS_FIELD_RW(chg_config, 03, CHG_CONFIG),
	BQ25892_SYSFS_FIELD_RW(otg_config, 03, OTG_CONFIG),
	BQ25892_SYSFS_FIELD_RW(sys_min, 03, SYS_MIN),
	BQ25892_SYSFS_FIELD_RW(ichg, 04, ICHG),
	BQ25892_SYSFS_FIELD_RW(iprechg, 05, IPRECHG),
	BQ25892_SYSFS_FIELD_RW(iterm, 05, ITERM),
	BQ25892_SYSFS_FIELD_RW(vreg, 06, VREG),
	BQ25892_SYSFS_FIELD_RW(batlowv, 06, BATLOWV),
	BQ25892_SYSFS_FIELD_RW(vrechg, 06, VRECHG),
	BQ25892_SYSFS_FIELD_RW(en_term, 07, EN_TERM),
	BQ25892_SYSFS_FIELD_RW(watchdog, 07, WATCHDOG),
	BQ25892_SYSFS_FIELD_RW(en_timer, 07, EN_TIMER),
	BQ25892_SYSFS_FIELD_RW(chg_timer, 07, CHG_TIMER),
	BQ25892_SYSFS_FIELD_RW(jeta_iset, 07, JEITA_ISET),
	BQ25892_SYSFS_FIELD_RW(bat_comp, 08, BAT_COMP),
	BQ25892_SYSFS_FIELD_RW(vclamp, 08, VCLAMP),
	BQ25892_SYSFS_FIELD_RW(treg, 08, TREG),
	BQ25892_SYSFS_FIELD_RW(force_ico, 09, FORCE_ICO),
	BQ25892_SYSFS_FIELD_RW(batfet_disable, 09, BATFET_DISABLE),
	BQ25892_SYSFS_FIELD_RW(jeita_vset, 09, JEITA_VSET),
	BQ25892_SYSFS_FIELD_RW(boost_vol, 0A, BOOSTV),
	BQ25892_SYSFS_FIELD_RW(boost_lim, 0A, BOOST_LIM),
	BQ25892_SYSFS_FIELD_RW(vindpm, 0D, VINDPM),
	BQ25892_SYSFS_FIELD_RW(force_vindpm, 0D, FORCE_VINDPM),
	BQ25892_SYSFS_FIELD_RW(reg_rst, 14, REG_RST),
	BQ25892_SYSFS_FIELD_RO(vbus_stat, 0B, VBUS_STAT),
	BQ25892_SYSFS_FIELD_RO(chrg_stat, 0B, CHRG_STAT),
	BQ25892_SYSFS_FIELD_RO(pg_stat, 0B, PG_STAT),
	BQ25892_SYSFS_FIELD_RO(sdp_stat, 0B, SDP_STAT),
	BQ25892_SYSFS_FIELD_RO(vsys_stat, 0B, VSYS_STAT),
	BQ25892_SYSFS_FIELD_RO(watchdog_fault, 0C, WATCHDOG_FAULT),
	BQ25892_SYSFS_FIELD_RO(boost_fault, 0C, BOOST_FAULT),
	BQ25892_SYSFS_FIELD_RO(chrg_fault, 0C, CHRG_FAULT),
	BQ25892_SYSFS_FIELD_RO(bat_fault, 0C, BAT_FAULT),
	BQ25892_SYSFS_FIELD_RO(ntc_fault, 0C, NTC_FAULT),
	BQ25892_SYSFS_FIELD_RO(therm_stat, 0E, THERM_STAT),
	BQ25892_SYSFS_FIELD_RO(bat_vol, 0E, BATV),
	BQ25892_SYSFS_FIELD_RO(sys_volt, 0F, SYSV),
	BQ25892_SYSFS_FIELD_RO(vbus_volt, 11, VBUSV),
	BQ25892_SYSFS_FIELD_RO(ichg_adc, 12, ICHGR),
	BQ25892_SYSFS_FIELD_RO(vdpm_stat, 13, VDPM_STAT),
	BQ25892_SYSFS_FIELD_RO(idpm_stat, 13, IDPM_STAT),
	BQ25892_SYSFS_FIELD_RO(idpm_lim, 13, IDPM_LIM),
	BQ25892_SYSFS_FIELD_RO(ico_optimized, 14, ICO_OPTIMIZED),
	BQ25892_SYSFS_FIELD_RW(reg_addr, NONE, NONE),
	BQ25892_SYSFS_FIELD_RW(reg_value, NONE, NONE),
};

#define BQ25892_SYSFS_ATTRS_SIZE  (ARRAY_SIZE(bq25892_sysfs_field_tbl) + 1)

static struct attribute *bq25892_sysfs_attrs[BQ25892_SYSFS_ATTRS_SIZE];

static const struct attribute_group bq25892_sysfs_attr_group = {
	.attrs = bq25892_sysfs_attrs,
};

static void bq25892_sysfs_init_attrs(void)
{
	int i, limit = ARRAY_SIZE(bq25892_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		bq25892_sysfs_attrs[i] = &bq25892_sysfs_field_tbl[i].attr.attr;

	bq25892_sysfs_attrs[limit] = NULL;
}

static struct bq25892_sysfs_field_info *bq25892_sysfs_field_lookup(
	const char *name)
{
	int i, limit = ARRAY_SIZE(bq25892_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strcmp(name, bq25892_sysfs_field_tbl[i].attr.attr.name))
			break;
	}

	if (i >= limit)
		return NULL;

	return &bq25892_sysfs_field_tbl[i];
}

static ssize_t bq25892_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bq25892_sysfs_field_info *info;
	struct bq25892_sysfs_field_info *info2;
	int ret;
	u8 v;

	info = bq25892_sysfs_field_lookup(attr->attr.name);
	if (info == NULL) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	if (!strncmp("reg_addr", attr->attr.name, strlen("reg_addr")))
		return scnprintf(buf, PAGE_SIZE, "0x%hhx\n", info->reg);

	if (!strncmp(("reg_value"), attr->attr.name, strlen("reg_value"))) {
		info2 = bq25892_sysfs_field_lookup("reg_addr");
		if (info2 == NULL) {
			hwlog_err("get sysfs entries failed\n");
			return -EINVAL;
		}

		info->reg = info2->reg;
	}

	ret = bq25892_read_mask(info->reg, info->mask, info->shift, &v);
	if (ret)
		return ret;

	return scnprintf(buf, PAGE_SIZE, "%hhx\n", v);
}

static ssize_t bq25892_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct bq25892_sysfs_field_info *info;
	struct bq25892_sysfs_field_info *info2;
	int ret;
	u8 v;

	info = bq25892_sysfs_field_lookup(attr->attr.name);
	if (info == NULL) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	ret = kstrtou8(buf, 0, &v);
	if (ret < 0) {
		hwlog_err("get kstrtou8 failed\n");
		return ret;
	}

	if (!strncmp(("reg_value"), attr->attr.name, strlen("reg_value"))) {
		info2 = bq25892_sysfs_field_lookup("reg_addr");
		if (info2 == NULL) {
			hwlog_err("get sysfs entries failed\n");
			return -EINVAL;
		}

		info->reg = info2->reg;
	}
	if (!strncmp(("reg_addr"), attr->attr.name, strlen("reg_addr"))) {
		if (v < (u8) BQ25892_REG_TOTAL_NUM) {
			info->reg = v;
			return count;
		} else {
			return -EINVAL;
		}
	}

	ret = bq25892_write_mask(info->reg, info->mask, info->shift, v);
	if (ret)
		return ret;

	return count;
}

static int bq25892_sysfs_create_group(struct bq25892_device_info *di)
{
	bq25892_sysfs_init_attrs();

	return sysfs_create_group(&di->dev->kobj, &bq25892_sysfs_attr_group);
}

static void bq25892_sysfs_remove_group(struct bq25892_device_info *di)
{
	sysfs_remove_group(&di->dev->kobj, &bq25892_sysfs_attr_group);
}

#else

static inline int bq25892_sysfs_create_group(struct bq25892_device_info *di)
{
	return 0;
}

static inline void bq25892_sysfs_remove_group(struct bq25892_device_info *di)
{
}

#endif /* CONFIG_SYSFS */

static int bq25892_device_check(void)
{
	int ret = 0;
	u8 reg = 0xff;

	ret |= bq25892_read_byte(BQ25892_REG_14, &reg);
	if (ret)
		return CHARGE_IC_BAD;

	hwlog_info("device_check [%x]=0x%x\n", BQ25892_REG_14, reg);

	if ((BQ25892 == (reg & CHIP_VERSION_MASK)) &&
		(CHIP_REVISION == (reg & CHIP_REVISION_MASK))) {
		hwlog_info("bq25892 is good\n");
		return CHARGE_IC_GOOD;
	}

	hwlog_err("bq25892 is bad\n");
	return CHARGE_IC_BAD;
}

static int bq25892_set_bat_comp(int value)
{
	int bat_comp = 0;
	u8 bat_comp_reg = 0;

	bat_comp = value;

	if (bat_comp < BAT_COMP_MIN_0) {
		hwlog_err("set bat_comp %d, out of range:%d",
			value, BAT_COMP_MIN_0);
		bat_comp = BAT_COMP_MIN_0;
	} else if (bat_comp > BAT_COMP_MAX_140) {
		hwlog_err("set bat_comp %d, out of range:%d",
			value, BAT_COMP_MAX_140);
		bat_comp = BAT_COMP_MAX_140;
	} else {
		/* do nothing */
	}

	bat_comp_reg = (bat_comp - BAT_COMP_MIN_0) / BAT_COMP_STEP_20;

	hwlog_info("set_bat_comp [%x]=0x%x\n", BQ25892_REG_08, bat_comp_reg);

	return bq25892_write_mask(BQ25892_REG_08,
			BQ25892_REG_08_BAT_COMP_MASK,
			BQ25892_REG_08_BAT_COMP_SHIFT,
			bat_comp_reg);
}

static int bq25892_set_vclamp(int value)
{
	int vclamp = 0;
	u8 vclamp_reg = 0;

	vclamp = value;

	if (vclamp < VCLAMP_MIN_0) {
		hwlog_err("set vclamp %d, out of range:%d",
			value, VCLAMP_MIN_0);
		vclamp = VCLAMP_MIN_0;
	} else if (vclamp > VCLAMP_MAX_224) {
		hwlog_err("set vclamp %d, out of range:%d",
			value, VCLAMP_MAX_224);
		vclamp = VCLAMP_MAX_224;
	} else {
		/* do nothing */
	}

	vclamp_reg = (vclamp - VCLAMP_MIN_0) / VCLAMP_STEP_32;

	hwlog_info("set_vclamp [%x]=0x%x\n", BQ25892_REG_08, vclamp_reg);

	return bq25892_write_mask(BQ25892_REG_08,
			BQ25892_REG_08_VCLAMP_MASK,
			BQ25892_REG_08_VCLAMP_SHIFT,
			vclamp_reg);
}

static int bq25892_set_covn_start(int enable)
{
	int ret = 0;
	u8 reg = 0;
	int i = 0;

	ret = bq25892_read_byte(BQ25892_REG_0B, &reg);
	if (ret)
		return -1;

	if (!(reg & BQ25892_NOT_PG_STAT)) {
		hwlog_err("bq25892 PG NOT GOOD, can not set covn,reg:%x\n",
			reg);
		return -1;
	}

	ret = bq25892_write_mask(BQ25892_REG_02,
			BQ25892_REG_02_CONV_START_MASK,
			BQ25892_REG_02_CONV_START_SHIFT,
			enable);
	if (ret)
		return -1;

	/* The conversion result is ready after tCONV, max (10*100)ms */
	for (i = 0; i < 10; i++) {
		ret = bq25892_read_mask(BQ25892_REG_02,
				BQ25892_REG_02_CONV_START_MASK,
				BQ25892_REG_02_CONV_START_SHIFT,
				&reg);
		if (ret)
			return -1;

		/* if ADC Conversion finished, CONV_RATE bit will be 0 */
		if (reg == 0)
			break;

		msleep(100); /* sleep 100ms */
	}

	hwlog_info("one-shot covn start is set %d\n", enable);
	return 0;
}

static int bq25892_5v_chip_init(struct bq25892_device_info *di)
{
	int ret = 0;

	/* reg init */
	/*
	 * bq25892_write_mask(REG0x14,
	 * BQ25892_REG_RST_MASK, BQ25892_REG_RST_SHIFT,
	 * 0x01);
	 */
	/*
	 * do not init input current 500 ma(REG00)
	 * to support lpt without battery
	 */

	/* 02 enable Start 1s Continuous Conversion ,others as default */
	ret |= bq25892_write_byte(BQ25892_REG_02, 0x1D); /*adc off*/

	/* 03 WD_RST 1,CHG_CONFIG 0,SYS_MIN 3.5 */
	ret |= bq25892_write_byte(BQ25892_REG_03, 0x4A);

	/* 04 Fast Charge Current Limit 2048mA */
	ret |= bq25892_write_byte(BQ25892_REG_04, 0x20);

	/* 05 Precharge Current Limit 256mA,Termination Current Limit 256mA */
	ret |= bq25892_write_byte(BQ25892_REG_05, 0x33);

	/*
	 * 06 Charge Voltage Limit 4.4,
	 * Battery Precharge to Fast Charge Threshold 3, Battery Recharge 100mV
	 */
	ret |= bq25892_write_byte(BQ25892_REG_06, 0x8e);

	/*
	 * 07 EN_TERM 0, Watchdog Timer 80s, EN_TIMER 1, Charge Timer 20h,
	 * JEITA Low Temperature Current Setting 1
	 */
	ret |= bq25892_write_byte(BQ25892_REG_07, 0x2f);

	/*
	 * 08 IR compensation voatge clamp = 224mV,
	 * IR compensation resistor setting = 80mohm
	 */
	ret |= bq25892_set_bat_comp(di->param_dts.bat_comp);
	ret |= bq25892_set_vclamp(di->param_dts.vclamp);

	/* boost mode current limit = 500mA,boostv 4.998v */
	ret |= bq25892_write_byte(BQ25892_REG_0A, 0x70);

	/* VINDPM Threshold Setting Method 1, Absolute VINDPM Threshold 4.4v */
	/* ret = bq25892_write_byte(BQ25892_REG_0D,0x92); */

	/* enable charging */
	gpio_set_value(di->gpio_cd, 0);

	return ret;
}

static int bq25892_set_adc_conv_rate(int mode)
{
	if (mode)
		mode = 1;

	hwlog_info("adc conversion rate mode is set to %d\n", mode);

	return bq25892_write_mask(BQ25892_REG_02,
			BQ25892_REG_02_CONV_RATE_MASK,
			BQ25892_REG_02_CONV_RATE_SHIFT,
			mode);
}

/*
 * ONLY TI bq25892 charge chip has a bug.
 * When charger is in charging done state and under PFM mode,
 * there is risk to slowly drive Q4 to open when unplug charger.
 * The result is VSYS drops to 2.0V and reach milliseconds interval.
 * The VSYS drop can be captured by hi6421 that result in SMPL.
 * The solution that TI suggest: when charger chip is bq25892,
 * and under charging done state, set IINLIM(reg00 bit0-bit5) to 400mA or
 * below to force chargeIC to exit PFM
 */
static int bq25892_exit_pfm_when_cd(unsigned int limit_default)
{
	u8 reg0b = 0;
	u8 reg14 = 0;
	int ret = 0;

	ret |= bq25892_read_byte(BQ25892_REG_0B, &reg0b);
	ret |= bq25892_read_byte(BQ25892_REG_14, &reg14);
	if (ret)
		return limit_default;

	if (((reg14 & BQ25892_REG_14_REG_PN_MASK) ==
		BQ25892_REG_14_REG_PN_IS_25892) && /* bq25892 */
		(limit_default > IINLIM_FOR_BQ25892_EXIT_PFM)) {
		if ((reg0b & BQ25892_REG_0B_CHRG_STAT_MASK) ==
			BQ25892_CHGR_STAT_CHARGE_DONE) { /* charging done */
			limit_default = IINLIM_FOR_BQ25892_EXIT_PFM;
			hwlog_err("limit input current to %dmA to force exit PFM\n",
				limit_default);
		}
	}

	return limit_default;
}

static int bq25892_set_input_current(int value)
{
	unsigned int limit_current = 0;
	u8 iin_limit = 0;

	limit_current = value;

	if (limit_current < IINLIM_MIN_100) {
		hwlog_err("set input_current %d, out of range:%d",
			value, IINLIM_MIN_100);
		limit_current = IINLIM_MIN_100;
	} else if (limit_current > IINLIM_MAX_3250) {
		hwlog_err("set input_current %d, out of range:%d",
			value, IINLIM_MAX_3250);
		limit_current = IINLIM_MAX_3250;
	} else {
		/* do nothing */
	}

	hwlog_info("input current is set %dmA\n", limit_current);

	/*
	 * in order to avoid smpl because bq25892 bug,
	 * set iinlim to 400mA if under charging done and chip ic is bq25892,
	 * otherwise keep it change
	 */
	if (!strstr(saved_command_line, "androidboot.mode=charger"))
		limit_current = bq25892_exit_pfm_when_cd(limit_current);

	hwlog_info("input current is set %dmA\n", limit_current);

	iin_limit = (limit_current - IINLIM_MIN_100) / IINLIM_STEP_50;
	iin_set = iin_limit * IINLIM_STEP_50 + IINLIM_MIN_100;

	return bq25892_write_mask(BQ25892_REG_00,
			BQ25892_REG_00_IINLIM_MASK,
			BQ25892_REG_00_IINLIM_SHIFT,
			iin_limit);
}

static int bq25892_get_input_current_set(void)
{
	return iin_set;
}

static int bq25892_set_charge_current(int value)
{
	int currentma = 0;
	u8 ichg = 0;

	currentma = value;

	if (currentma < 0) {
		currentma = 0;
	} else if (currentma > ICHG_MAX_5056) {
		hwlog_err("set charge_current %d, out of range:%d!",
			value, ICHG_MAX_5056);
		currentma = ICHG_MAX_5056;
	} else {
		/* do nothing */
	}

	hwlog_info("charge current is set %dmA\n", currentma);

	ichg = currentma / ICHG_STEP_64;

	return bq25892_write_mask(BQ25892_REG_04,
			BQ25892_REG_04_ICHG_MASK,
			BQ25892_REG_04_ICHG_SHIFT,
			ichg);
}

static int bq25892_set_terminal_voltage(int value)
{
	unsigned int voltagemv = 0;
	u8 voreg = 0;

	voltagemv = value;

	if (voltagemv < VCHARGE_MIN_3840) {
		hwlog_err("set terminal_voltage %d, out of range:%d",
			value, VCHARGE_MIN_3840);
		voltagemv = VCHARGE_MIN_3840;
	} else if (voltagemv > VCHARGE_MAX_4496) {
		hwlog_err("set terminal_voltage %d, out of range:%d",
			value, VCHARGE_MAX_4496);
		voltagemv = VCHARGE_MAX_4496;
	}

	hwlog_info("terminal voltage is set %dmV\n", voltagemv);

	voreg = (voltagemv - VCHARGE_MIN_3840) / VCHARGE_STEP_16;

	return bq25892_write_mask(BQ25892_REG_06,
			BQ25892_REG_06_VREG_MASK,
			BQ25892_REG_06_VREG_SHIFT,
			voreg);
}

static int bq25892_set_dpm_voltage(int value)
{
	int vindpm_voltage;
	u8 vindpm;
	u8 reg0d;

	vindpm_voltage = value;

	if (vindpm_voltage > VINDPM_MAX_15300) {
		hwlog_err("set dpm_voltage %d, out of range:%d",
			value, VINDPM_MAX_15300);
		vindpm_voltage = VINDPM_MAX_15300;
	} else if (vindpm_voltage < VINDPM_MIN_3900) {
		hwlog_err("set dpm_voltage %d, out of range:%d",
			value, VINDPM_MIN_3900);
		vindpm_voltage = VINDPM_MIN_3900;
	} else {
		/* do nothing */
	}

	hwlog_info("vindpm absolute voltage is set %dmV\n", vindpm_voltage);

	vindpm = (vindpm_voltage - VINDPM_OFFSET_2600) / VINDPM_STEP_100;

	/* 1 absolute dpm */
	reg0d = (1 << BQ25892_REG_0D_FORCE_VINDPM_SHIFT) | vindpm;

	return bq25892_write_byte(BQ25892_REG_0D, reg0d);
}

static int bq25892_set_terminal_current(int value)
{
	unsigned int term_currentma = 0;
	u8 iterm_reg = 0;

	term_currentma = value;

	if (term_currentma < ITERM_MIN_64) {
		hwlog_err("set terminal_current %d, out of range:%d",
			value, ITERM_MIN_64);
		term_currentma = ITERM_MIN_64;
	} else if (term_currentma > ITERM_MAX_1024) {
		hwlog_err("set terminal_current %d, out of range:%d",
			value, ITERM_MAX_1024);
		term_currentma = ITERM_MAX_1024;
	} else {
		/* do nothing */
	}

	hwlog_info("term current is set %dmA\n", term_currentma);

	iterm_reg = term_currentma / ITERM_STEP_64;

	return bq25892_write_mask(BQ25892_REG_05,
			BQ25892_REG_05_ITERM_MASK,
			BQ25892_REG_05_ITERM_SHIFT,
			iterm_reg);
}

static int bq25892_set_charge_enable(int enable)
{
	struct bq25892_device_info *di = g_bq25892_dev;

	gpio_set_value(di->gpio_cd, !enable);

	return bq25892_write_mask(BQ25892_REG_03,
			BQ25892_REG_03_CHG_CONFIG_MASK,
			BQ25892_REG_03_CHG_CONFIG_SHIFT,
			enable);
}

static int bq25892_set_otg_enable(int enable)
{
	int val = 0;
	struct bq25892_device_info *di = g_bq25892_dev;

	gpio_set_value(di->gpio_cd, !enable);

	val = enable << 1;
	return bq25892_write_mask(BQ25892_REG_03,
			BQ25892_REG_03_CHG_CONFIG_MASK,
			BQ25892_REG_03_CHG_CONFIG_SHIFT,
			val);
}

static int bq25892_set_otg_current(int value)
{
	unsigned int temp_currentma = 0;
	u8 reg = 0;

	temp_currentma = value;

	if (temp_currentma < BOOST_LIM_MIN_500 ||
		temp_currentma > BOOST_LIM_MAX_2450)
		hwlog_err("set otg_current %d, out of range", value);

	if (temp_currentma < BOOST_LIM_750)
		reg = 0;
	else if (temp_currentma >= BOOST_LIM_750 &&
		temp_currentma < BOOST_LIM_1200)
		reg = 1;
	else if (temp_currentma >= BOOST_LIM_1200 &&
		temp_currentma < BOOST_LIM_1400)
		reg = 2;
	else if (temp_currentma >= BOOST_LIM_1400 &&
		temp_currentma < BOOST_LIM_1650)
		reg = 3;
	else if (temp_currentma >= BOOST_LIM_1650 &&
		temp_currentma < BOOST_LIM_1875)
		reg = 4;
	else if (temp_currentma >= BOOST_LIM_1875 &&
		temp_currentma < BOOST_LIM_2150)
		reg = 5;
	else if (temp_currentma >= BOOST_LIM_2150 &&
		temp_currentma < BOOST_LIM_MAX_2450)
		reg = 6;
	else
		reg = 7;

	hwlog_info("otg current is set %dmA\n", temp_currentma);

	return bq25892_write_mask(BQ25892_REG_0A,
			BQ25892_REG_0A_BOOST_LIM_MASK,
			BQ25892_REG_0A_BOOST_LIM_SHIFT,
			reg);
}

static int bq25892_set_term_enable(int enable)
{
	return bq25892_write_mask(BQ25892_REG_07,
			BQ25892_REG_07_EN_TERM_MASK,
			BQ25892_REG_07_EN_TERM_SHIFT,
			enable);
}

static int bq25892_get_charge_state(unsigned int *state)
{
	u8 reg = 0;
	int ret = 0;

	ret = bq25892_read_byte(BQ25892_REG_0B, &reg);

	hwlog_info("get_charge_state [%x]=0x%x\n", BQ25892_REG_0B, reg);

	if (!(reg & BQ25892_NOT_PG_STAT))
		*state |= CHAGRE_STATE_NOT_PG;

	if ((reg & BQ25892_CHGR_STAT_CHARGE_DONE) ==
		BQ25892_CHGR_STAT_CHARGE_DONE)
		*state |= CHAGRE_STATE_CHRG_DONE;

	ret |= bq25892_read_byte(BQ25892_REG_0C, &reg);
	ret |= bq25892_read_byte(BQ25892_REG_0C, &reg);

	hwlog_info("get_charge_state [%x]=0x%x\n", BQ25892_REG_0C, reg);

	if ((reg & BQ25892_WATCHDOG_FAULT) == BQ25892_WATCHDOG_FAULT)
		*state |= CHAGRE_STATE_WDT_FAULT;

	if ((reg & BQ25892_POWER_SUPPLY_OVP_MASK) == BQ25892_POWER_SUPPLY_OVP)
		*state |= CHAGRE_STATE_VBUS_OVP;

	if ((reg & BQ25892_BAT_FAULT_OVP) == BQ25892_BAT_FAULT_OVP)
		*state |= CHAGRE_STATE_BATT_OVP;

	if ((reg & BQ25892_NTC_FAULT) > BQ25892_NTC_NORMAL)
		*state |= CHAGRE_STATE_NTC_FAULT;

	if (reg != 0)
		hwlog_err("state is not normal, BQ25892 REG[0x0C] is %x\n",
			reg);

	return ret;
}

static int bq25892_get_vbus_mv(unsigned int *vbus_mv)
{
	u8 reg = 0;
	int ret = 0;

	ret = bq25892_read_byte(BQ25892_REG_11, &reg);

	reg = reg & BQ25892_REG_11_VBUSV_MASK;
	*vbus_mv = (unsigned int)reg * BQ25892_REG_11_VBUSV_STEP_MV +
		BQ25892_REG_11_VBUSV_OFFSET_MV;

	hwlog_info("vbus mv is %dmV\n", *vbus_mv);

	return ret;
}

static int bq25892_reset_watchdog_timer(void)
{
	return bq25892_write_mask(BQ25892_REG_03,
			BQ25892_REG_03_WDT_RESET_MASK,
			BQ25892_REG_03_WDT_RESET_SHIFT,
			0x01);
}

static int bq25892_get_vilim_sample(void)
{
	int i = 0;
	int retry_times = 3;
	int v_sample = -1;

	for (i = 0; i < retry_times; ++i) {
		v_sample = hisi_adc_get_value(adc_channel_iin);

		if (v_sample < 0)
			hwlog_err("adc read channel 10 fail\n");
		else
			break;
	}

	return v_sample;
}

static int bq25892_get_ilim(void)
{
	int i;
	int cnt = 0;
	int v_temp;
	int delay_times = 100;
	int sample_num = 5; /* use 5 samples to get an average value */
	int sum = 0;
	int kilim = 355; /* based bq25892 spec */

	for (i = 0; i < sample_num; ++i) {
		v_temp = bq25892_get_vilim_sample();
		if (v_temp >= 0) {
			sum += v_temp;
			++cnt;
		} else {
			hwlog_err("bq25892 get v_temp fail\n");
		}

		msleep(delay_times);
	}

	if (cnt > 0) {
		 /* I =(K* Vlim)/(rilim*0.8) new spec from TI */
		return ((10 * sum * kilim) / (8 * (int)(rilim) * cnt));
	}

	hwlog_err("use 0 as default Vilim\n");
	return 0;
}

static int bq25892_vbat_sys(void)
{
	int i = 0;
	int retry_times = 3;
	int v_sample = -1;

	for (i = 0; i < retry_times; ++i) {
		v_sample = hisi_adc_get_value(adc_channel_vbat_sys);

		if (v_sample < 0)
			hwlog_err("adc read channel 15 fail\n");
		else
			break;
	}

	return v_sample;
}

static int bq25892_get_vbat_sys(void)
{
	int i;
	int cnt = 0;
	int v_temp;
	int delay_times = 100;
	int sample_num = 5; /* use 5 samples to get an average value*/
	int sum = 0;

	for (i = 0; i < sample_num; ++i) {
		v_temp = bq25892_vbat_sys();
		if (v_temp >= 0) {
			sum += v_temp;
			++cnt;
		} else {
			hwlog_err("bq25892 get v_temp fail\n");
		}

		msleep(delay_times);
	}

	if (cnt > 0)
		return (3 * 1000 * sum / cnt);

	hwlog_err("use 0 as default Vvlim\n");
	return 0;
}

static int bq25892_dump_register(char *reg_value)
{
	u8 reg[BQ25892_REG_TOTAL_NUM] = {0};
	char buff[BUF_LEN] = {0};
	int i = 0;
	int vbus = 0;
	int ret = 0;

	memset(reg_value, 0, CHARGELOG_SIZE);

	snprintf(buff, BUF_LEN, "%-8.2d", bq25892_get_ilim());
	strncat(reg_value, buff, strlen(buff));

	ret = bq25892_get_vbus_mv((unsigned int *)&vbus);
	if (ret)
		hwlog_err("bq25892_get_vbus_mv failed\n");

	snprintf(buff, BUF_LEN, "%-8.2d", vbus);
	strncat(reg_value, buff, strlen(buff));

	for (i = 0; i < BQ25892_REG_TOTAL_NUM; i++) {
		bq25892_read_byte(i, &reg[i]);
		bq25892_read_byte(i, &reg[i]);
		snprintf(buff, BUF_LEN, "0x%-8x", reg[i]);
		strncat(reg_value, buff, strlen(buff));
	}

	return 0;
}

static int bq25892_get_register_head(char *reg_head)
{
	char buff[BUF_LEN] = {0};
	int i = 0;

	memset(reg_head, 0, CHARGELOG_SIZE);

	snprintf(buff, BUF_LEN, "Ibus    ");
	strncat(reg_head, buff, strlen(buff));

	snprintf(buff, BUF_LEN, "Vbus    ");
	strncat(reg_head, buff, strlen(buff));

	for (i = 0; i < BQ25892_REG_TOTAL_NUM; i++) {
		snprintf(buff, BUF_LEN, "Reg[0x%2x] ", i);
		strncat(reg_head, buff, strlen(buff));
	}

	return 0;
}

static int bq25892_set_batfet_disable(int disable)
{
	return bq25892_write_mask(BQ25892_REG_09,
			BQ25892_REG_09_BATFET_DISABLE_MASK,
			BQ25892_REG_09_BATFET_DISABLE_SHIFT,
			disable);
}

static int bq25892_set_watchdog_timer(int value)
{
	u8 val = 0;
	u8 dog_time = value;

	if (dog_time >= WATCHDOG_TIMER_160_S)
		val = BQ25892_REG_07_WATCHDOG_160;
	else if (dog_time >= WATCHDOG_TIMER_80_S)
		val = BQ25892_REG_07_WATCHDOG_80;
	else if (dog_time >= WATCHDOG_TIMER_40_S)
		val = BQ25892_REG_07_WATCHDOG_40;
	else
		val = BQ25892_REG_07_WATCHDOG_DIS;

	hwlog_info("watch dog timer is %d ,the register value is set %d\n",
		dog_time, val);

	return bq25892_write_mask(BQ25892_REG_07,
			BQ25892_REG_07_WATCHDOG_MASK,
			BQ25892_REG_07_WATCHDOG_SHIFT,
			val);
}

static int bq25892_set_charger_hiz(int enable)
{
	int ret = 0;

	if (enable > 0) {
		ret |= bq25892_write_mask(BQ25892_REG_00,
				BQ25892_REG_00_EN_HIZ_MASK,
				BQ25892_REG_00_EN_HIZ_SHIFT,
				TRUE);
	} else {
		ret |= bq25892_write_mask(BQ25892_REG_00,
				BQ25892_REG_00_EN_HIZ_MASK,
				BQ25892_REG_00_EN_HIZ_SHIFT,
				FALSE);
	}

	return ret;
}

static int bq25892_get_ico_current(struct ico_output *output)
{
	int ret = 0;
	u8 reg = 0;
	int check_ico_optimized_times = 10;
	int idpm_limit = -1;

	ret = bq25892_read_byte(BQ25892_REG_09, &reg);
	if (ret)
		return ret;

	ret = bq25892_write_mask(BQ25892_REG_09,
			BQ25892_REG_09_FORCE_ICO_MASK,
			BQ25892_REG_09_FORCE_ICO_SHIFT,
			TRUE);
	if (ret)
		return ret;

	while (check_ico_optimized_times--) {
		ret = bq25892_read_byte(BQ25892_REG_14, &reg);
		if (ret)
			return ret;

		if (reg & BQ25892_REG_14_ICO_OPTIMIZED_MASK) {
			ret = bq25892_read_mask(BQ25892_REG_13,
					BQ25892_REG_13_IDPM_LIM_MASK,
					BQ25892_REG_13_IDPM_LIM_SHIFT,
					&reg);
			if (ret)
				return ret;

			idpm_limit = reg * BQ25892_REG_13_IDPM_STEP_MV +
				BQ25892_REG_13_IDPM_OFFSET_MV;
			hwlog_info("ico result: reg is 0x%x, current is %dmA\n",
				reg, idpm_limit);

			switch (ico_current_mode) {
			case 0:
				output->input_current =
				(idpm_limit > (int)(output->input_current) ?
				idpm_limit : (int)(output->input_current));
				output->charge_current =
				(idpm_limit > (int)(output->charge_current) ?
				idpm_limit : (int)(output->input_current));
				break;

			case 1:
				output->input_current = idpm_limit;
				output->charge_current = idpm_limit;
				break;

			default:
				break;
			}

			return ret;
		}

		hwlog_info("bq25892_start_force_ico: msleep 200\n");
		msleep(200);
	}

	return -1;
}

static int bq25892_turn_on_ico(struct ico_input *input,
	struct ico_output *output)
{
	int ret = 0;

	ret = bq25892_set_charge_enable(TRUE);
	if (ret)
		return -1;

	ret = bq25892_set_input_current(input->iin_max);
	if (ret)
		return -1;

	ret = bq25892_set_charge_current(input->ichg_max);
	if (ret)
		return -1;

	ret = bq25892_set_terminal_voltage(input->vterm);
	if (ret)
		return -1;

	ret = bq25892_get_ico_current(output);

	return ret;
}

static int bq25892_9v_chip_init(struct bq25892_device_info *di)
{
	int ret = 0;

	/* reg init */
	/*
	 * bq25892_write_mask(REG0x14,
	 * BQ25892_REG_RST_MASK, BQ25892_REG_RST_SHIFT,
	 * 0x01);
	 */
	/*
	 * do not init input current 500 ma(REG00)
	 * to support lpt without battery
	 */

	/*02 enable Start 1s Continuous Conversion ,others as default */
	ret |= bq25892_write_byte(BQ25892_REG_02, 0x1D); /* adc off */

	/* 03 WD_RST 1,CHG_CONFIG 0,SYS_MIN 3.5 */
	ret |= bq25892_write_byte(BQ25892_REG_03, 0x4A);

	/* 04 Fast Charge Current Limit 2048mA */
	ret |= bq25892_write_byte(BQ25892_REG_04, 0x20);

	/* 05 Precharge Current Limit 256mA,Termination Current Limit 256mA */
	ret |= bq25892_write_byte(BQ25892_REG_05, 0x33);

	/*
	 * 06 Charge Voltage Limit 4.4,
	 * Battery Precharge to Fast Charge Threshold 3,Battery Recharge 100mV
	 */
	ret |= bq25892_write_byte(BQ25892_REG_06, 0x8e);

	/*
	 * 07 EN_TERM 1, Watchdog Timer 80s, EN_TIMER 1,
	 * Charge Timer 20h,JEITA Low Temperature Current Setting 1
	 */
	ret |= bq25892_write_byte(BQ25892_REG_07, 0x2f);

	/*
	 * 08 IR compensation voatge clamp = 224mV,
	 * IR compensation resistor setting = 80mohm
	 */
	ret |= bq25892_set_bat_comp(di->param_dts.bat_comp);
	ret |= bq25892_set_vclamp(di->param_dts.vclamp);

	/*
	 * 09 FORCE_ICO 0, TMR2X_EN 1, BATFET_DIS 0,
	 * JEITA_VSET 0, BATFET_RST_EN 1
	 */
	ret |= bq25892_write_byte(BQ25892_REG_09, 0x44);

	/* boost mode current limit = 500mA,boostv 4.998v */
	ret |= bq25892_write_byte(BQ25892_REG_0A, 0x70);

	/* VINDPM Threshold Setting Method 1,Absolute VINDPM Threshold 4.4v */
	/* ret = bq25892_write_byte(BQ25892_REG_0D,0x92); */

	/*
	 * set dpm voltage as 4700mv instead of 7600mv
	 * because chargerIC cannot reset dpm after watchdog time out
	 */
	ret = bq25892_set_dpm_voltage(4700);

	/*enable charging*/
	gpio_set_value(di->gpio_cd, 0);

	return ret;
}

static int bq25892_chip_init(struct chip_init_crit *init_crit)
{
	int ret = -1;
	struct bq25892_device_info *di = g_bq25892_dev;

	if (di == NULL || init_crit == NULL) {
		hwlog_err("di or init_crit is null\n");
		return -ENOMEM;
	}

	switch (init_crit->vbus) {
	case ADAPTER_5V:
		ret = bq25892_5v_chip_init(di);
		break;

	case ADAPTER_9V:
		ret = bq25892_9v_chip_init(di);
		break;

	default:
		hwlog_err("invaid init_crit vbus mode\n");
		break;
	}

	return ret;
}

static int bq25892_check_input_vdpm_state(void)
{
	u8 reg = 0;
	int ret = -1;

	ret = bq25892_read_byte(BQ25892_REG_13, &reg);
	if (ret < 0)
		return ret;

	hwlog_info("check_input_vdpm_state [%x]=0x%x\n", BQ25892_REG_13, reg);

	if (reg & BQ25892_REG_13_VDPM_STAT_MASK)
		return TRUE;
	else
		return FALSE;
}

static int bq25892_check_input_idpm_state(void)
{
	u8 reg = 0;
	int ret = -1;

	ret = bq25892_read_byte(BQ25892_REG_13, &reg);
	if (ret < 0)
		return ret;

	hwlog_info("check_input_idpm_state [%x]=0x%x\n", BQ25892_REG_13, reg);

	if (reg & BQ25892_REG_13_IDPM_STAT_MASK)
		return TRUE;
	else
		return FALSE;
}

static int bq25892_check_input_dpm_state(void)
{
	u8 reg = 0;
	int ret = -1;

	ret = bq25892_read_byte(BQ25892_REG_13, &reg);
	if (ret < 0)
		return ret;

	if ((reg & BQ25892_REG_13_VDPM_STAT_MASK) ||
		(reg & BQ25892_REG_13_IDPM_STAT_MASK))
		return TRUE;
	else
		return FALSE;
}

static int bq25892_stop_charge_config(void)
{
	/*
	 * as vindpm of bq25892 won't be reset after watchdog timer out,
	 * if vindpm is higher than 5v, IC will not supply power with USB/AC
	 */
	return bq25892_set_dpm_voltage(CHARGE_VOLTAGE_4520_MV);
}

static int bq25892_force_set_term_enable(int enable)
{
	return 0;
}

static int bq25892_get_charger_state(void)
{
	return 0;
}

static struct charge_device_ops bq25892_ops = {
	.chip_init = bq25892_chip_init,
	.dev_check = bq25892_device_check,
	.set_adc_conv_rate = bq25892_set_adc_conv_rate,
	.set_input_current = bq25892_set_input_current,
	.set_charge_current = bq25892_set_charge_current,
	.set_terminal_voltage = bq25892_set_terminal_voltage,
	.set_dpm_voltage = bq25892_set_dpm_voltage,
	.set_terminal_current = bq25892_set_terminal_current,
	.set_charge_enable = bq25892_set_charge_enable,
	.set_otg_enable = bq25892_set_otg_enable,
	.set_term_enable = bq25892_set_term_enable,
	.get_charge_state = bq25892_get_charge_state,
	.reset_watchdog_timer = bq25892_reset_watchdog_timer,
	.dump_register = bq25892_dump_register,
	.get_register_head = bq25892_get_register_head,
	.set_watchdog_timer = bq25892_set_watchdog_timer,
	.set_batfet_disable = bq25892_set_batfet_disable,
	.get_ibus = bq25892_get_ilim,
	.get_vbus = bq25892_get_vbus_mv,
	.get_vbat_sys = bq25892_get_vbat_sys,
	.set_covn_start = bq25892_set_covn_start,
	.set_charger_hiz = bq25892_set_charger_hiz,
	.check_input_dpm_state = bq25892_check_input_dpm_state,
	.check_input_vdpm_state = bq25892_check_input_vdpm_state,
	.check_input_idpm_state = bq25892_check_input_idpm_state,
	.set_otg_current = bq25892_set_otg_current,
	.stop_charge_config = bq25892_stop_charge_config,
	.turn_on_ico = bq25892_turn_on_ico,
	.set_force_term_enable = bq25892_force_set_term_enable,
	.get_charger_state = bq25892_get_charger_state,
	.soft_vbatt_ovp_protect = NULL,
	.rboost_buck_limit = NULL,
	.get_charge_current = NULL,
	.get_iin_set = bq25892_get_input_current_set,
};

static void bq25892_irq_work(struct work_struct *work)
{
	struct bq25892_device_info *di;
	u8 reg = 0, reg1 = 0;

	di = container_of(work, struct bq25892_device_info, irq_work);

	msleep(100); /* sleep 100ms */

	bq25892_read_byte(BQ25892_REG_0B, &reg1);
	bq25892_read_byte(BQ25892_REG_0C, &reg);
	hwlog_info("1st reg[0xB]:0x%x,reg[0xC]:0x%0x\n", reg1, reg);

	bq25892_read_byte(BQ25892_REG_0C, &reg);
	hwlog_info("2nd reg[0xB]:0x%x,reg[0xC]:0x%0x\n", reg1, reg);

	if (reg & BQ25892_REG_0C_BOOST) {
		hwlog_info("CHARGE_FAULT_BOOST_OCP happened\n");

		atomic_notifier_call_chain(&fault_notifier_list,
			CHARGE_FAULT_BOOST_OCP, NULL);
	}

	if (di->irq_active == 0) {
		di->irq_active = 1;
		enable_irq(di->irq_int);
	}
}

static irqreturn_t bq25892_interrupt(int irq, void *_di)
{
	struct bq25892_device_info *di = _di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	hwlog_info("bq25892 int happened (%d)\n", di->irq_active);

	if (di->irq_active == 1) {
		di->irq_active = 0;
		disable_irq_nosync(di->irq_int);
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("the irq is not enable,do nothing\n");
	}

	return IRQ_HANDLED;
}

static void bq25892_prase_dts(struct device_node *np,
	struct bq25892_device_info *di)
{
	struct device_node *batt_node;
	int ret = 0;

	di->param_dts.bat_comp = 80;
	di->param_dts.vclamp = 224;

	ret = of_property_read_u32(np, "bat_comp", &(di->param_dts.bat_comp));
	if (ret) {
		hwlog_err("bat_comp dts read failed\n");
		return;
	}
	hwlog_info("bat_comp=%d\n", di->param_dts.bat_comp);

	ret = of_property_read_u32(np, "vclamp", &(di->param_dts.vclamp));
	if (ret) {
		hwlog_err("vclamp dts read failed\n");
		return;
	}
	hwlog_info("vclamp=%d\n", di->param_dts.vclamp);

	ret = of_property_read_u32(np, "ico_current_mode", &ico_current_mode);
	if (ret) {
		hwlog_err("ico_current_mode dts read failed\n");
		ico_current_mode = 0;
	}
	hwlog_info("ico_current_mode=%d\n", ico_current_mode);

	ret = of_property_read_u32(np, "rilim", &rilim);
	if (ret)
		hwlog_err("rilim dts read failed\n");

	hwlog_info("rilim=%d\n", rilim);

	ret = of_property_read_u32(np, "adc_channel_iin", &adc_channel_iin);
	if (ret)
		hwlog_err("adc_channel_iin dts read failed\n");

	hwlog_info("adc_channel_iin=%d\n", adc_channel_iin);

	ret = of_property_read_u32(np, "adc_channel_vbat_sys",
		&adc_channel_vbat_sys);
	if (ret)
		hwlog_err("adc_channel_vbat_sys dts read failed\n");

	hwlog_info("adc_channel_vbat_sys=%d\n", adc_channel_vbat_sys);

	batt_node = of_find_compatible_node(NULL, NULL,
		"huawei,hisi_bci_battery");
	if (batt_node != NULL) {
		ret = of_property_read_u32(batt_node, "battery_board_type",
			&is_board_type);
		if (ret) {
			hwlog_err("battery_board_type dts read failed\n");
			is_board_type = BAT_BOARD_ASIC;
		}
	} else {
		hwlog_err("hisi_bci_battery dts read failed\n");
		is_board_type = BAT_BOARD_ASIC;
	}
}

static int bq25892_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret = 0;
	struct bq25892_device_info *di = NULL;
	struct device_node *np = NULL;
	struct class *power_class = NULL;

	hwlog_info("probe begin\n");

	if (client == NULL || id == NULL) {
		hwlog_err("client or id is null\n");
		return -ENOMEM;
	}

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (di == NULL)
		return -ENOMEM;

	g_bq25892_dev = di;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	INIT_WORK(&di->irq_work, bq25892_irq_work);

	bq25892_prase_dts(np, di);

	di->gpio_cd = of_get_named_gpio(np, "gpio_cd", 0);
	hwlog_info("gpio_cd=%d\n", di->gpio_cd);

	if (!gpio_is_valid(di->gpio_cd)) {
		hwlog_err("gpio(gpio_cd) is not valid\n");
		ret = -EINVAL;
		goto bq25892_fail_0;
	}

	ret = gpio_request(di->gpio_cd, "charger_cd");
	if (ret) {
		hwlog_err("gpio(gpio_cd) request fail\n");
		goto bq25892_fail_0;
	}

	/* set gpio to control CD pin to disable/enable bq25892 IC */
	gpio_direction_output(di->gpio_cd, 0);
	if (ret) {
		hwlog_err("gpio(gpio_cd) set output fail\n");
		goto bq25892_fail_1;
	}

	di->gpio_int = of_get_named_gpio(np, "gpio_int", 0);
	hwlog_info("gpio_int=%d\n", di->gpio_int);

	if (!gpio_is_valid(di->gpio_int)) {
		hwlog_err("gpio(gpio_int) is not valid\n");
		ret = -EINVAL;
		goto bq25892_fail_1;
	}

	ret = gpio_request(di->gpio_int, "charger_int");
	if (ret) {
		hwlog_err("gpio(gpio_int) request fail\n");
		goto bq25892_fail_1;
	}

	gpio_direction_input(di->gpio_int);
	if (ret) {
		hwlog_err("gpio(gpio_int) set input fail\n");
		goto bq25892_fail_2;
	}

	di->irq_int = gpio_to_irq(di->gpio_int);
	if (di->irq_int < 0) {
		hwlog_err("gpio(gpio_int) map to irq fail\n");
		ret = -EINVAL;
		goto bq25892_fail_2;
	}

	if (is_board_type != BAT_BOARD_UDP) {
		ret = request_irq(di->irq_int, bq25892_interrupt,
			IRQF_TRIGGER_FALLING, "charger_int_irq", di);
		if (ret) {
			hwlog_err("gpio(gpio_int) irq request fail\n");
			di->irq_int = -1;
			goto bq25892_fail_3;
		}

		di->irq_active = 1;
	}

	ret = charge_ops_register(&bq25892_ops);
	if (ret) {
		hwlog_err("bq25892 charge ops register fail\n");
		goto bq25892_fail_3;
	}

	ret = bq25892_sysfs_create_group(di);
	if (ret) {
		hwlog_err("sysfs group create failed\n");
		goto bq25892_fail_3;
	}

	power_class = hw_power_get_class();
	if (power_class != NULL) {
		if (charge_dev == NULL)
			charge_dev = device_create(power_class, NULL, 0, NULL,
				"charger");

		ret = sysfs_create_link(&charge_dev->kobj, &di->dev->kobj,
			"bq25892");
		if (ret) {
			hwlog_err("sysfs link create failed\n");
			goto bq25892_fail_4;
		}
	}

	hwlog_info("probe end\n");
	return 0;

bq25892_fail_4:
	bq25892_sysfs_remove_group(di);
bq25892_fail_3:
	free_irq(di->irq_int, di);
bq25892_fail_2:
	gpio_free(di->gpio_int);
bq25892_fail_1:
	gpio_free(di->gpio_cd);
bq25892_fail_0:
	g_bq25892_dev = NULL;
	np = NULL;

	return ret;
}

static int bq25892_remove(struct i2c_client *client)
{
	struct bq25892_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	bq25892_sysfs_remove_group(di);

	gpio_set_value(di->gpio_cd, 1);

	if (di->gpio_cd)
		gpio_free(di->gpio_cd);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	hwlog_info("remove end\n");
	return 0;
}

MODULE_DEVICE_TABLE(i2c, bq25892);
static const struct of_device_id bq25892_of_match[] = {
	{
		.compatible = "huawei,bq25892_charger",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id bq25892_i2c_id[] = {
	{"bq25892_charger", 0}, {}
};

static struct i2c_driver bq25892_driver = {
	.probe = bq25892_probe,
	.remove = bq25892_remove,
	.id_table = bq25892_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bq25892_charger",
		.of_match_table = of_match_ptr(bq25892_of_match),
	},
};

static int __init bq25892_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&bq25892_driver);
	if (ret)
		hwlog_err("i2c_add_driver error\n");

	return ret;
}

static void __exit bq25892_exit(void)
{
	i2c_del_driver(&bq25892_driver);
}

module_init(bq25892_init);
module_exit(bq25892_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bq25892 charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
