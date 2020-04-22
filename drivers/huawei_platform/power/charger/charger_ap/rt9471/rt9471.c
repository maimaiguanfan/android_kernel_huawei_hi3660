/*
 * rt9471.c
 *
 * rt9471 driver
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

#include "rt9471.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/i2c.h>
#include <linux/of_device.h>
#include <linux/mutex.h>
#include <linux/power_supply.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>
#include <../charging_core.h>
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
#include <huawei_platform/power/usb_short_circuit_protect.h>
#endif

#define HWLOG_TAG rt9471_charger
HWLOG_REGIST();

#define BUF_LEN                      26

static struct i2c_client *g_rt9471_i2c;
static int g_hiz_iin_limit_flag;

enum rt9471_stat_idx {
	RT9471_STATIDX_STAT0 = 0,
	RT9471_STATIDX_STAT1,
	RT9471_STATIDX_STAT2,
	RT9471_STATIDX_STAT3,
	RT9471_STATIDX_MAX,
};

enum rt9471_irq_idx {
	RT9471_IRQIDX_IRQ0 = 0,
	RT9471_IRQIDX_IRQ1,
	RT9471_IRQIDX_IRQ2,
	RT9471_IRQIDX_IRQ3,
	RT9471_IRQIDX_MAX,
};

enum rt9471_ic_stat {
	RT9471_ICSTAT_SLEEP = 0,
	RT9471_ICSTAT_VBUSRDY,
	RT9471_ICSTAT_TRICKLECHG,
	RT9471_ICSTAT_PRECHG,
	RT9471_ICSTAT_FASTCHG,
	RT9471_ICSTAT_IEOC,
	RT9471_ICSTAT_BGCHG,
	RT9471_ICSTAT_CHGDONE,
	RT9471_ICSTAT_CHGFAULT,
	RT9471_ICSTAT_OTG = 15,
	RT9471_ICSTAT_MAX,
};

static const char *g_rt9471_ic_stat_name[RT9471_ICSTAT_MAX] = {
	"hz/sleep", "ready", "trickle-charge", "pre-charge",
	"fast-charge", "ieoc-charge", "background-charge",
	"done", "fault", "RESERVED", "RESERVED", "RESERVED",
	"RESERVED", "RESERVED", "RESERVED", "OTG",
};

enum rt9471_mivr_track {
	RT9471_MIVRTRACK_REG = 0,
	RT9471_MIVRTRACK_VBAT_200MV,
	RT9471_MIVRTRACK_VBAT_250MV,
	RT9471_MIVRTRACK_VBAT_300MV,
	RT9471_MIVRTRACK_MAX,
};

enum rt9471_port_stat {
	RT9471_PORTSTAT_NOINFO = 0,
	RT9471_PORTSTAT_APPLE_10W = 8,
	RT9471_PORTSTAT_SAMSUNG_10W,
	RT9471_PORTSTAT_APPLE_5W,
	RT9471_PORTSTAT_APPLE_12W,
	RT9471_PORTSTAT_NSDP,
	RT9471_PORTSTAT_SDP,
	RT9471_PORTSTAT_CDP,
	RT9471_PORTSTAT_DCP,
	RT9471_PORTSTAT_MAX,
};

struct rt9471_desc {
	const char *rm_name;
	u8 rm_slave_addr;
	u32 acov;
	u32 cust_cv;
	u32 hiz_iin_limit;
	u32 ichg;
	u32 aicr;
	u32 mivr;
	u32 cv;
	u32 ieoc;
	u32 safe_tmr;
	u32 wdt;
	u32 mivr_track;
	bool en_safe_tmr;
	bool en_te;
	bool en_jeita;
	bool ceb_invert;
	bool dis_i2c_tout;
	bool en_qon_rst;
	bool auto_aicr;
	const char *chg_name;
};

/* These default values will be applied if there's no property in dts */
static struct rt9471_desc g_rt9471_default_desc = {
	.rm_name = "rt9471",
	.rm_slave_addr = RT9471_SLAVE_ADDR,
	.acov = RT9471_ACOV_10P9,
	.cust_cv = 0,
	.hiz_iin_limit = 0,
	.ichg = RT9471_ICHG_2000,
	.aicr = RT9471_AICR_500,
	.mivr = RT9471_MIVR_4P5,
	.cv = RT9471_CV_4P2,
	.ieoc = RT9471_IEOC_200,
	.safe_tmr = RT9471_SAFETMR_15,
	.wdt = WDT_40S,
	.mivr_track = RT9471_MIVRTRACK_REG,
	.en_safe_tmr = true,
	.en_te = true,
	.en_jeita = true,
	.ceb_invert = false,
	.dis_i2c_tout = false,
	.en_qon_rst = true,
	.auto_aicr = true,
	.chg_name = "rt9471",
};

static const u8 g_rt9471_irq_maskall[RT9471_IRQIDX_MAX] = {
	0xFF, 0xFE, 0xF3, 0xE7,
};

static const u32 g_rt9471_wdt[] = {
	0, 40, 80, 160,
};

static const u32 g_rt9471_otgcc[] = {
	500, 1200,
};

static const u8 g_rt9471_val_en_hidden_mode[] = {
	0x69, 0x96,
};

static const u32 g_rt9471_acov_th[] = {
	5800, 6500, 10900, 14000,
};

struct rt9471_chip {
	struct i2c_client *client;
	struct device *dev;
	struct mutex io_lock;
	struct mutex hidden_mode_lock;
	u32 hidden_mode_cnt;
	u8 dev_id;
	u8 dev_rev;
	u8 chip_rev;
	struct rt9471_desc *desc;
	u32 intr_gpio;
	u32 ceb_gpio;
	int irq;
	u8 irq_mask[RT9471_IRQIDX_MAX];
	struct kthread_work irq_work;
	struct kthread_worker irq_worker;
	struct task_struct *irq_worker_task;
};

static const u8 rt9471_reg_addr[] = {
	RT9471_REG_OTGCFG,
	RT9471_REG_TOP,
	RT9471_REG_FUNCTION,
	RT9471_REG_IBUS,
	RT9471_REG_VBUS,
	RT9471_REG_PRECHG,
	RT9471_REG_REGU,
	RT9471_REG_VCHG,
	RT9471_REG_ICHG,
	RT9471_REG_CHGTIMER,
	RT9471_REG_EOC,
	RT9471_REG_INFO,
	RT9471_REG_JEITA,
	RT9471_REG_STATUS,
	RT9471_REG_STAT0,
	RT9471_REG_STAT1,
	RT9471_REG_STAT2,
	RT9471_REG_STAT3,
	RT9471_REG_MASK0,
	RT9471_REG_MASK1,
	RT9471_REG_MASK2,
	RT9471_REG_MASK3,
};

static int rt9471_set_aicr(int aicr);

static int rt9471_read_device(void *client, u32 addr, int len, void *dst)
{
	return i2c_smbus_read_i2c_block_data(client, addr, len, dst);
}

static int rt9471_write_device(void *client, u32 addr, int len,
	const void *src)
{
	return i2c_smbus_write_i2c_block_data(client, addr, len, src);
}

static int __rt9471_i2c_write_byte(struct rt9471_chip *chip, u8 cmd,
	u8 data)
{
	return rt9471_write_device(chip->client, cmd, 1, &data);
}

static int rt9471_i2c_write_byte(struct rt9471_chip *chip, u8 cmd, u8 data)
{
	int ret;

	mutex_lock(&chip->io_lock);
	ret = __rt9471_i2c_write_byte(chip, cmd, data);
	mutex_unlock(&chip->io_lock);

	return ret;
}

static int __rt9471_i2c_read_byte(struct rt9471_chip *chip, u8 cmd,
	u8 *data)
{
	int ret;
	u8 regval = 0;

	ret = rt9471_read_device(chip->client, cmd, 1, &regval);
	if (ret < 0) {
		hwlog_err("reg0x%02X fail %d\n", cmd, ret);
		return ret;
	}

	hwlog_debug("reg0x%02X = 0x%02x\n", cmd, regval);
	*data = regval & RT9471_REG_NONE_MASK;
	return 0;
}

static int rt9471_i2c_read_byte(struct rt9471_chip *chip, u8 cmd, u8 *data)
{
	int ret;

	mutex_lock(&chip->io_lock);
	ret = __rt9471_i2c_read_byte(chip, cmd, data);
	mutex_unlock(&chip->io_lock);

	return ret;
}

static int __rt9471_i2c_block_write(struct rt9471_chip *chip, u8 cmd,
	u32 len, const u8 *data)
{
	return rt9471_write_device(chip->client, cmd, len, data);
}

static int rt9471_i2c_block_write(struct rt9471_chip *chip, u8 cmd, u32 len,
	const u8 *data)
{
	int ret;

	mutex_lock(&chip->io_lock);
	ret = __rt9471_i2c_block_write(chip, cmd, len, data);
	mutex_unlock(&chip->io_lock);

	return ret;
}

static inline int __rt9471_i2c_block_read(struct rt9471_chip *chip, u8 cmd,
	u32 len, u8 *data)
{
	return rt9471_read_device(chip->client, cmd, len, data);
}

static int rt9471_i2c_block_read(struct rt9471_chip *chip, u8 cmd, u32 len,
	u8 *data)
{
	int ret;

	mutex_lock(&chip->io_lock);
	ret = __rt9471_i2c_block_read(chip, cmd, len, data);
	mutex_unlock(&chip->io_lock);

	return ret;
}

static int rt9471_i2c_test_bit(struct rt9471_chip *chip, u8 cmd, u8 shift,
	bool *is_one)
{
	int ret;
	u8 data = 0;

	ret = rt9471_i2c_read_byte(chip, cmd, &data);
	if (ret < 0) {
		*is_one = false;
		return ret;
	}

	data &= 1 << shift;
	*is_one = (data ? true : false);

	return 0;
}

static int rt9471_i2c_update_bits(struct rt9471_chip *chip, u8 cmd, u8 data,
	u8 mask)
{
	int ret;
	u8 regval = 0;

	mutex_lock(&chip->io_lock);
	ret = __rt9471_i2c_read_byte(chip, cmd, &regval);
	if (ret < 0)
		goto fail_i2c_err;

	regval &= ~mask;
	regval |= (data & mask);

	ret = __rt9471_i2c_write_byte(chip, cmd, regval);
fail_i2c_err:
	mutex_unlock(&chip->io_lock);
	return ret;
}

static inline int rt9471_set_bit(struct rt9471_chip *chip, u8 reg, u8 mask)
{
	return rt9471_i2c_update_bits(chip, reg, mask, mask);
}

static inline int rt9471_clr_bit(struct rt9471_chip *chip, u8 reg, u8 mask)
{
	return rt9471_i2c_update_bits(chip, reg, 0x00, mask);
}

static u8 rt9471_closest_reg(u32 min, u32 max, u32 step, u32 target)
{
	if (target < min)
		return 0;

	if (target >= max)
		return (max - min) / step;

	return (target - min) / step;
}

static u8 rt9471_closest_reg_via_tbl(const u32 *tbl, u32 tbl_size,
	u32 target)
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

static u32 rt9471_closest_value(u32 min, u32 max, u32 step, u8 reg_val)
{
	u32 ret_val;

	ret_val = min + reg_val * step;
	if (ret_val > max)
		ret_val = max;

	return ret_val;
}

static int rt9471_enable_hidden_mode(struct rt9471_chip *chip, bool en)
{
	int ret = 0;

	mutex_lock(&chip->hidden_mode_lock);

	if (en) {
		if (chip->hidden_mode_cnt == 0) {
			/* enter hidden mode and init hidden regs */
			ret = rt9471_i2c_block_write(chip, 0xA0,
				ARRAY_SIZE(g_rt9471_val_en_hidden_mode),
				g_rt9471_val_en_hidden_mode);
			if (ret < 0)
				goto hidden_ops_err;
		}
		chip->hidden_mode_cnt++;
	} else {
		if (chip->hidden_mode_cnt == 1) {
			/* exit hidden mode by write 0xA0 to zero */
			ret = rt9471_i2c_write_byte(chip, 0xA0, 0x00);
			if (ret < 0)
				goto hidden_ops_err;
		}
		chip->hidden_mode_cnt--;
	}
	hwlog_debug("%s en = %d\n", __func__, en);
	goto hidden_unlock;

hidden_ops_err:
	hwlog_err("%s en = %d fail %d\n", __func__, en, ret);
hidden_unlock:
	mutex_unlock(&chip->hidden_mode_lock);
	return ret;
}

static int __rt9471_get_ic_stat(struct rt9471_chip *chip,
	enum rt9471_ic_stat *stat)
{
	int ret;
	u8 regval = 0;

	ret = rt9471_i2c_read_byte(chip, RT9471_REG_STATUS, &regval);
	if (ret < 0)
		return ret;
	*stat = (regval & RT9471_ICSTAT_MASK) >> RT9471_ICSTAT_SHIFT;
	return 0;
}

static int __rt9471_get_mivr(struct rt9471_chip *chip, u32 *mivr)
{
	int ret;
	u8 regval = 0;

	ret = rt9471_i2c_read_byte(chip, RT9471_REG_VBUS, &regval);
	if (ret < 0)
		return ret;

	regval = (regval & RT9471_MIVR_MASK) >> RT9471_MIVR_SHIFT;
	*mivr = rt9471_closest_value(RT9471_MIVR_MIN, RT9471_MIVR_MAX,
		RT9471_MIVR_STEP, regval);

	return 0;
}

static int __rt9471_get_ichg(struct rt9471_chip *chip, u32 *ichg)
{
	int ret;
	u8 regval = 0;

	ret = rt9471_i2c_read_byte(chip, RT9471_REG_ICHG, &regval);
	if (ret < 0)
		return ret;

	regval = (regval & RT9471_ICHG_MASK) >> RT9471_ICHG_SHIFT;
	*ichg = rt9471_closest_value(RT9471_ICHG_MIN, RT9471_ICHG_MAX,
		RT9471_ICHG_STEP, regval);

	return 0;
}

static int __rt9471_get_aicr(struct rt9471_chip *chip, u32 *aicr)
{
	int ret;
	u8 regval = 0;

	ret = rt9471_i2c_read_byte(chip, RT9471_REG_IBUS, &regval);
	if (ret < 0)
		return ret;

	regval = (regval & RT9471_AICR_MASK) >> RT9471_AICR_SHIFT;
	*aicr = rt9471_closest_value(RT9471_AICR_MIN, RT9471_AICR_MAX,
		RT9471_AICR_STEP, regval);
	if (*aicr > RT9471_AICR_MIN && *aicr < RT9471_AICR_MAX)
		*aicr -= RT9471_AICR_STEP;

	return ret;
}

static int __rt9471_get_cv(struct rt9471_chip *chip, u32 *cv)
{
	int ret;
	u8 regval = 0;

	ret = rt9471_i2c_read_byte(chip, RT9471_REG_VCHG, &regval);
	if (ret < 0)
		return ret;

	regval = (regval & RT9471_CV_MASK) >> RT9471_CV_SHIFT;
	*cv = rt9471_closest_value(RT9471_CV_MIN, RT9471_CV_MAX, RT9471_CV_STEP,
		regval);

	return ret;
}

static int __rt9471_get_ieoc(struct rt9471_chip *chip, u32 *ieoc)
{
	int ret;
	u8 regval = 0;

	ret = rt9471_i2c_read_byte(chip, RT9471_REG_EOC, &regval);
	if (ret < 0)
		return ret;

	regval = (regval & RT9471_IEOC_MASK) >> RT9471_IEOC_SHIFT;
	*ieoc = rt9471_closest_value(RT9471_IEOC_MIN, RT9471_IEOC_MAX,
		RT9471_IEOC_STEP, regval);

	return ret;
}

static int __rt9471_is_chg_enabled(struct rt9471_chip *chip, bool *en)
{
	return rt9471_i2c_test_bit(chip, RT9471_REG_FUNCTION,
		RT9471_CHG_EN_SHIFT, en);
}

static int __rt9471_enable_shipmode(struct rt9471_chip *chip, bool en)
{
	hwlog_info("%s en = %d\n", __func__, en);
	return (en ? rt9471_set_bit : rt9471_clr_bit)
		(chip, RT9471_REG_FUNCTION, RT9471_BATFETDIS_MASK);
}

static int __rt9471_enable_safe_tmr(struct rt9471_chip *chip, bool en)
{
	hwlog_info("%s en = %d\n", __func__, en);
	return (en ? rt9471_set_bit : rt9471_clr_bit)
		(chip, RT9471_REG_CHGTIMER, RT9471_SAFETMR_EN_MASK);
}

static int __rt9471_enable_te(struct rt9471_chip *chip, bool en)
{
	hwlog_info("%s en = %d\n", __func__, en);
	return (en ? rt9471_set_bit : rt9471_clr_bit)
		(chip, RT9471_REG_EOC, RT9471_TE_MASK);
}

static int __rt9471_enable_jeita(struct rt9471_chip *chip, bool en)
{
	hwlog_info("%s en = %d\n", __func__, en);
	return (en ? rt9471_set_bit : rt9471_clr_bit)
		(chip, RT9471_REG_JEITA, RT9471_JEITA_EN_MASK);
}

static int __rt9471_disable_i2c_tout(struct rt9471_chip *chip, bool en)
{
	hwlog_info("%s en = %d\n", __func__, en);
	return (en ? rt9471_set_bit : rt9471_clr_bit)
		(chip, RT9471_REG_TOP, RT9471_DISI2CTO_MASK);
}

static int __rt9471_enable_qon_rst(struct rt9471_chip *chip, bool en)
{
	hwlog_info("%s en = %d\n", __func__, en);
	return (en ? rt9471_set_bit : rt9471_clr_bit)
		(chip, RT9471_REG_TOP, RT9471_QONRST_MASK);
}

static int __rt9471_enable_autoaicr(struct rt9471_chip *chip, bool en)
{
	hwlog_info("%s en = %d\n", __func__, en);
	return (en ? rt9471_set_bit : rt9471_clr_bit)
		(chip, RT9471_REG_IBUS, RT9471_AUTOAICR_MASK);
}

static int __rt9471_enable_hz(struct rt9471_chip *chip, bool en)
{
	hwlog_info("%s en = %d\n", __func__, en);
	return (en ? rt9471_set_bit : rt9471_clr_bit)
		(chip, RT9471_REG_FUNCTION, RT9471_HZ_MASK);
}

static int __rt9471_enable_otg(struct rt9471_chip *chip, bool en)
{
	hwlog_info("%s en = %d\n", __func__, en);
	return (en ? rt9471_set_bit : rt9471_clr_bit)
		(chip, RT9471_REG_FUNCTION, RT9471_OTG_EN_MASK);
}

static int __rt9471_enable_chg(struct rt9471_chip *chip, bool en)
{
	hwlog_info("%s en = %d\n", __func__, en);
	return (en ? rt9471_set_bit : rt9471_clr_bit)
		(chip, RT9471_REG_FUNCTION, RT9471_CHG_EN_MASK);
}

static int __rt9471_set_wdt(struct rt9471_chip *chip, u32 sec)
{
	u8 regval;

	/* 40s is the minimum, set to 40 except sec == 0 */
	if (sec <= WDT_40S && sec > 0)
		sec = WDT_40S;
	regval = rt9471_closest_reg_via_tbl(g_rt9471_wdt,
		ARRAY_SIZE(g_rt9471_wdt), sec);

	hwlog_info("%s time = %d reg: 0x%02X\n", __func__, sec, regval);

	return rt9471_i2c_update_bits(chip, RT9471_REG_TOP,
		regval << RT9471_WDT_SHIFT,
		RT9471_WDT_MASK);
}

static int __rt9471_set_otgcc(struct rt9471_chip *chip, u32 cc)
{
	hwlog_info("%s cc = %d\n", __func__, cc);
	return (cc <= g_rt9471_otgcc[0] ? rt9471_clr_bit : rt9471_set_bit)
		(chip, RT9471_REG_OTGCFG, RT9471_OTGCC_MASK);
}

static int __rt9471_set_ichg(struct rt9471_chip *chip, u32 ichg)
{
	u8 regval;

	regval = rt9471_closest_reg(RT9471_ICHG_MIN, RT9471_ICHG_MAX,
		RT9471_ICHG_STEP, ichg);

	hwlog_info("%s ichg = %d reg: 0x%02X\n", __func__, ichg, regval);

	return rt9471_i2c_update_bits(chip, RT9471_REG_ICHG,
		regval << RT9471_ICHG_SHIFT,
		RT9471_ICHG_MASK);
}

static int __rt9471_set_acov(struct rt9471_chip *chip, u32 vth)
{
	u8 regval;

	regval = rt9471_closest_reg_via_tbl(g_rt9471_acov_th,
		ARRAY_SIZE(g_rt9471_acov_th), vth);

	hwlog_info("%s vth = %d reg: 0x%02x\n", __func__, vth, regval);

	return rt9471_i2c_update_bits(chip, RT9471_REG_VBUS,
		regval << RT9471_ACOV_SHIFT,
		RT9471_ACOV_MASK);
}

static int __rt9471_set_aicr(struct rt9471_chip *chip, u32 aicr)
{
	int ret;
	u8 regval;

	regval = rt9471_closest_reg(RT9471_AICR_MIN, RT9471_AICR_MAX,
		RT9471_AICR_STEP, aicr);
	/* 0 & 1 are both 50mA */
	if (aicr < RT9471_AICR_MAX)
		regval += 1;

	hwlog_info("%s aicr = %d reg: 0x%02X\n", __func__, aicr, regval);

	ret = rt9471_i2c_update_bits(chip, RT9471_REG_IBUS,
		regval << RT9471_AICR_SHIFT,
		RT9471_AICR_MASK);
	/* Store AICR */
	__rt9471_get_aicr(chip, &chip->desc->aicr);
	return ret;
}

static int __rt9471_set_mivr(struct rt9471_chip *chip, u32 mivr)
{
	u8 regval;

	regval = rt9471_closest_reg(RT9471_MIVR_MIN, RT9471_MIVR_MAX,
		RT9471_MIVR_STEP, mivr);

	hwlog_info("%s mivr = %d reg: 0x%02x\n", __func__, mivr, regval);

	return rt9471_i2c_update_bits(chip, RT9471_REG_VBUS,
		regval << RT9471_MIVR_SHIFT,
		RT9471_MIVR_MASK);
}

static int __rt9471_set_cv(struct rt9471_chip *chip, u32 cv)
{
	u8 regval;

	regval = rt9471_closest_reg(RT9471_CV_MIN, RT9471_CV_MAX,
		RT9471_CV_STEP, cv);

	hwlog_info("%s cv = %d reg: 0x%02X\n", __func__, cv, regval);

	return rt9471_i2c_update_bits(chip, RT9471_REG_VCHG,
		regval << RT9471_CV_SHIFT,
		RT9471_CV_MASK);
}

static int __rt9471_set_ieoc(struct rt9471_chip *chip, u32 ieoc)
{
	u8 regval;

	regval = rt9471_closest_reg(RT9471_IEOC_MIN, RT9471_IEOC_MAX,
		RT9471_IEOC_STEP, ieoc);

	hwlog_info("%s ieoc = %d reg: 0x%02X\n", __func__, ieoc, regval);

	return rt9471_i2c_update_bits(chip, RT9471_REG_EOC,
		regval << RT9471_IEOC_SHIFT,
		RT9471_IEOC_MASK);
}

static int __rt9471_set_safe_tmr(struct rt9471_chip *chip, u32 hr)
{
	u8 regval;

	regval = rt9471_closest_reg(RT9471_SAFETMR_MIN, RT9471_SAFETMR_MAX,
		RT9471_SAFETMR_STEP, hr);

	hwlog_info("%s time = %d reg: 0x%02X\n", __func__, hr, regval);

	return rt9471_i2c_update_bits(chip, RT9471_REG_CHGTIMER,
		regval << RT9471_SAFETMR_SHIFT,
		RT9471_SAFETMR_MASK);
}

static int __rt9471_set_mivrtrack(struct rt9471_chip *chip, u32 mivr_track)
{
	if (mivr_track >= RT9471_MIVRTRACK_MAX)
		mivr_track = RT9471_MIVRTRACK_VBAT_300MV;

	hwlog_info("%s mivrtrack = %d\n", __func__, mivr_track);

	return rt9471_i2c_update_bits(chip, RT9471_REG_VBUS,
		mivr_track << RT9471_MIVRTRACK_SHIFT,
		RT9471_MIVRTRACK_MASK);
}

static int __rt9471_kick_wdt(struct rt9471_chip *chip)
{
	hwlog_info("%s\n", __func__);
	return rt9471_set_bit(chip, RT9471_REG_TOP, RT9471_WDTCNTRST_MASK);
}

static int rt9471_detach_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_rechg_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_chg_done_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_bg_chg_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_ieoc_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_vbus_gd_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_chg_batov_irq_handler(struct rt9471_chip *chip)
{
	hwlog_info("%s\n", __func__);
	return 0;
}

static int rt9471_chg_sysov_irq_handler(struct rt9471_chip *chip)
{
	hwlog_info("%s\n", __func__);
	return 0;
}

static int rt9471_chg_tout_irq_handler(struct rt9471_chip *chip)
{
	hwlog_info("%s\n", __func__);
	return 0;
}

static int rt9471_chg_busuv_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_chg_threg_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_chg_aicr_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_chg_mivr_irq_handler(struct rt9471_chip *chip)
{
	int ret;
	bool mivr = false;

	ret = rt9471_i2c_test_bit(chip, RT9471_REG_STAT1, RT9471_ST_MIVR_SHIFT,
		&mivr);
	if (ret < 0) {
		hwlog_err("check stat fail %d\n", ret);
		return ret;
	}
	hwlog_info("%s mivr = %d\n", __func__, mivr);
	return 0;
}

static int rt9471_sys_short_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_sys_min_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_jeita_cold_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_jeita_cool_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_jeita_warm_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_jeita_hot_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_otg_fault_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_otg_lbp_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_otg_cc_irq_handler(struct rt9471_chip *chip)
{
	return 0;
}

static int rt9471_wdt_irq_handler(struct rt9471_chip *chip)
{
	hwlog_info("%s\n", __func__);
	return __rt9471_kick_wdt(chip);
}

static int rt9471_vac_ov_irq_handler(struct rt9471_chip *chip)
{
	int ret;
	bool vacov = false;

	ret = rt9471_i2c_test_bit(chip, RT9471_REG_STAT3, RT9471_ST_VACOV_SHIFT,
		&vacov);
	if (ret < 0) {
		hwlog_err("check stat fail %d\n", ret);
		return ret;
	}
	hwlog_info("%s vacov = %d\n", __func__, vacov);
	if (vacov) {
		/* Rewrite AICR */
		ret = __rt9471_set_aicr(chip, chip->desc->aicr);
		if (ret < 0)
			hwlog_err("set aicr fail %d\n", ret);
	}
	return ret;
}

static int rt9471_otp_irq_handler(struct rt9471_chip *chip)
{
	hwlog_info("%s\n", __func__);
	return 0;
}

struct irq_mapping_tbl {
	const char *name;
	int (*hdlr)(struct rt9471_chip *chip);
	int num;
};

#define RT9471_IRQ_MAPPING(_name, _num) \
{ \
	.name = #_name, \
	.hdlr = rt9471_##_name##_irq_handler, \
	.num = (_num), \
}

static const struct irq_mapping_tbl rt9471_irq_mapping_tbl[] = {
	RT9471_IRQ_MAPPING(vbus_gd, 7),
	RT9471_IRQ_MAPPING(detach, 1),
	RT9471_IRQ_MAPPING(rechg, 2),
	RT9471_IRQ_MAPPING(chg_done, 3),
	RT9471_IRQ_MAPPING(bg_chg, 4),
	RT9471_IRQ_MAPPING(ieoc, 5),
	RT9471_IRQ_MAPPING(chg_batov, 9),
	RT9471_IRQ_MAPPING(chg_sysov, 10),
	RT9471_IRQ_MAPPING(chg_tout, 11),
	RT9471_IRQ_MAPPING(chg_busuv, 12),
	RT9471_IRQ_MAPPING(chg_threg, 13),
	RT9471_IRQ_MAPPING(chg_aicr, 14),
	RT9471_IRQ_MAPPING(chg_mivr, 15),
	RT9471_IRQ_MAPPING(sys_short, 16),
	RT9471_IRQ_MAPPING(sys_min, 17),
	RT9471_IRQ_MAPPING(jeita_cold, 20),
	RT9471_IRQ_MAPPING(jeita_cool, 21),
	RT9471_IRQ_MAPPING(jeita_warm, 22),
	RT9471_IRQ_MAPPING(jeita_hot, 23),
	RT9471_IRQ_MAPPING(otg_fault, 24),
	RT9471_IRQ_MAPPING(otg_lbp, 25),
	RT9471_IRQ_MAPPING(otg_cc, 26),
	RT9471_IRQ_MAPPING(wdt, 29),
	RT9471_IRQ_MAPPING(vac_ov, 30),
	RT9471_IRQ_MAPPING(otp, 31),
};

static void rt9471_irq_work_handler(struct kthread_work *work)
{
	int ret, i, irq_num, irq_bit;
	u8 evt[RT9471_IRQIDX_MAX] = {0};
	u8 mask[RT9471_IRQIDX_MAX] = {0};
	struct rt9471_chip *chip = NULL;

	hwlog_info("%s\n", __func__);

	chip = container_of(work, struct rt9471_chip, irq_work);
	if (!chip)
		return;

	ret = rt9471_i2c_block_read(chip, RT9471_REG_IRQ0, RT9471_IRQIDX_MAX,
		evt);
	if (ret < 0) {
		hwlog_err("%s read evt fail %d\n", __func__, ret);
		goto irq_i2c_err;
	}

	ret = rt9471_i2c_block_read(chip, RT9471_REG_MASK0, RT9471_IRQIDX_MAX,
		mask);
	if (ret < 0) {
		hwlog_err("%s read mask fail %d\n", __func__, ret);
		goto irq_i2c_err;
	}

	for (i = 0; i < RT9471_IRQIDX_MAX; i++)
		evt[i] &= ~mask[i];
	for (i = 0; i < ARRAY_SIZE(rt9471_irq_mapping_tbl); i++) {
		irq_num = rt9471_irq_mapping_tbl[i].num / BIT_LEN;
		if (irq_num >= RT9471_IRQIDX_MAX)
			continue;
		irq_bit = rt9471_irq_mapping_tbl[i].num % BIT_LEN;
		if (evt[irq_num] & (1 << irq_bit))
			rt9471_irq_mapping_tbl[i].hdlr(chip);
	}
irq_i2c_err:
	enable_irq(chip->irq);
}

static irqreturn_t rt9471_irq_handler(int irq, void *data)
{
	struct rt9471_chip *chip = data;

	if (!chip)
		return IRQ_HANDLED;

	disable_irq_nosync(chip->irq);
	kthread_queue_work(&chip->irq_worker, &chip->irq_work);

	return IRQ_HANDLED;
}

static int rt9471_register_irq(struct rt9471_chip *chip)
{
	int ret, len;
	char *name = NULL;
	struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };

	hwlog_info("%s\n", __func__);

	len = strlen(chip->desc->chg_name);
	name = devm_kzalloc(chip->dev, len + CHG_NAME_EX_LEN_10, GFP_KERNEL);
	if (!name)
		return -ENOMEM;
	snprintf(name, len + CHG_NAME_EX_LEN_10, "%s-irq-gpio",
		chip->desc->chg_name);
	ret = devm_gpio_request_one(chip->dev, chip->intr_gpio, GPIOF_IN, name);
	if (ret < 0) {
		hwlog_err("gpio request fail %d\n", ret);
		return ret;
	}
	chip->irq = gpio_to_irq(chip->intr_gpio);
	if (chip->irq < 0) {
		hwlog_err("%s gpio2irq fail %d\n", __func__,
			chip->irq);
		return chip->irq;
	}
	hwlog_info("%s irq = %d\n", __func__, chip->irq);

	/* Request IRQ */
	len = strlen(chip->desc->chg_name);
	name = devm_kzalloc(chip->dev, len + CHG_NAME_EX_LEN_5, GFP_KERNEL);
	if (!name)
		return -ENOMEM;
	snprintf(name, len + CHG_NAME_EX_LEN_5, "%s-irq", chip->desc->chg_name);

	kthread_init_work(&chip->irq_work, rt9471_irq_work_handler);
	kthread_init_worker(&chip->irq_worker);
	chip->irq_worker_task = kthread_run(kthread_worker_fn,
		&chip->irq_worker, chip->desc->chg_name);
	if (IS_ERR(chip->irq_worker_task)) {
		ret = PTR_ERR(chip->irq_worker_task);
		hwlog_err("kthread run fail %d\n", ret);
		return ret;
	}
	sched_setscheduler(chip->irq_worker_task, SCHED_FIFO, &param);

	ret = devm_request_irq(chip->dev, chip->irq, rt9471_irq_handler,
		IRQF_TRIGGER_FALLING, name, chip);
	if (ret < 0) {
		hwlog_err("request thread irq fail %d\n", ret);
		return ret;
	}
	device_init_wakeup(chip->dev, true);

	return 0;
}

static int rt9471_init_irq(struct rt9471_chip *chip)
{
	hwlog_info("%s\n", __func__);
	return rt9471_i2c_block_write(chip, RT9471_REG_MASK0,
		ARRAY_SIZE(chip->irq_mask),
		chip->irq_mask);
}

static int rt9471_get_irq_number(struct rt9471_chip *chip,
	const char *name)
{
	int i;

	if (!name) {
		hwlog_err("%s null name\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(rt9471_irq_mapping_tbl); i++) {
		if (!strcmp(name, rt9471_irq_mapping_tbl[i].name))
			return rt9471_irq_mapping_tbl[i].num;
	}

	return -EINVAL;
}

static const char *rt9471_get_irq_name(int irq_num)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(rt9471_irq_mapping_tbl); i++) {
		if (rt9471_irq_mapping_tbl[i].num == irq_num)
			return rt9471_irq_mapping_tbl[i].name;
	}
	return "not found";
}

static inline void rt9471_irq_mask(struct rt9471_chip *chip, int irq_num)
{
	chip->irq_mask[irq_num / BIT_LEN] |= (1 << (irq_num % BIT_LEN));
}

static inline void rt9471_irq_unmask(struct rt9471_chip *chip, int irq_num)
{
	hwlog_info("%s irq %d, %s\n", __func__, irq_num,
		rt9471_get_irq_name(irq_num));
	chip->irq_mask[irq_num / RT9471_IRQ_GROUP_LEN] &=
		~(1 << (irq_num % BIT_LEN));
}

static int rt9471_parse_dt(struct rt9471_chip *chip)
{
	int ret, irq_num, len;
	int irq_cnt = 0;
	struct rt9471_desc *desc = NULL;
	struct device_node *np = NULL;
	const char *name = NULL;
	char *ceb_name = NULL;

	hwlog_info("%s\n", __func__);

	if (!chip->dev->of_node) {
		hwlog_info("%s no device node\n", __func__);
		return -EINVAL;
	}

	np = of_get_child_by_name(chip->dev->of_node, "rt9471");
	if (!np) {
		hwlog_info("%s no rt9471 device node\n", __func__);
		return -EINVAL;
	}

	chip->desc = &g_rt9471_default_desc;

	desc = devm_kzalloc(chip->dev, sizeof(*desc), GFP_KERNEL);
	if (!desc)
		return -ENOMEM;
	memcpy(desc, &g_rt9471_default_desc, sizeof(*desc));

	if (of_property_read_string(np, "charger_name", &desc->chg_name) < 0)
		hwlog_err("%s no charger name\n", __func__);
	hwlog_info("%s name %s\n", __func__, desc->chg_name);

	ret = of_get_named_gpio(chip->dev->of_node, "rt,intr_gpio", 0);
	if (ret < 0)
		return ret;
	chip->intr_gpio = ret;
	ret = of_get_named_gpio(chip->dev->of_node, "rt,ceb_gpio", 0);
	if (ret < 0)
		return ret;
	chip->ceb_gpio = ret;
	hwlog_info("%s intr_gpio %u\n", __func__, chip->intr_gpio);

	/* ceb gpio */
	len = strlen(chip->desc->chg_name);
	ceb_name = devm_kzalloc(chip->dev, len + CHG_NAME_EX_LEN_10,
		GFP_KERNEL);
	if (!ceb_name)
		return -ENOMEM;
	snprintf(ceb_name, len + CHG_NAME_EX_LEN_10, "%s-ceb-gpio",
		chip->desc->chg_name);
	ret = devm_gpio_request_one(chip->dev, chip->ceb_gpio, GPIOF_DIR_OUT,
		ceb_name);
	if (ret < 0) {
		hwlog_info("%s gpio request fail %d\n",
			__func__, ret);
		return ret;
	}

	/* Charger parameter */
	if (of_property_read_u32(np, "acov", &desc->acov) < 0)
		hwlog_info("%s no ichg\n", __func__);

	if (of_property_read_u32(np, "custom_cv", &desc->cust_cv) < 0)
		hwlog_info("%s no custom_cv\n", __func__);

	if (of_property_read_u32(np, "hiz_iin_limit", &desc->hiz_iin_limit) < 0)
		hwlog_info("%s no hiz_iin_limit\n", __func__);

	if (of_property_read_u32(np, "ichg", &desc->ichg) < 0)
		hwlog_info("%s no ichg\n", __func__);

	if (of_property_read_u32(np, "aicr", &desc->aicr) < 0)
		hwlog_info("%s no aicr\n", __func__);

	if (of_property_read_u32(np, "mivr", &desc->mivr) < 0)
		hwlog_info("%s no mivr\n", __func__);

	if (of_property_read_u32(np, "cv", &desc->cv) < 0)
		hwlog_info("%s no cv\n", __func__);

	if (of_property_read_u32(np, "ieoc", &desc->ieoc) < 0)
		hwlog_info("%s no ieoc\n", __func__);

	if (of_property_read_u32(np, "safe-tmr", &desc->safe_tmr) < 0)
		hwlog_info("%s no safety timer\n", __func__);

	if (of_property_read_u32(np, "wdt", &desc->wdt) < 0)
		hwlog_info("%s no wdt\n", __func__);

	if (of_property_read_u32(np, "mivr-track", &desc->mivr_track) < 0)
		hwlog_info("%s no mivr track\n", __func__);
	if (desc->mivr_track >= RT9471_MIVRTRACK_MAX)
		desc->mivr_track = RT9471_MIVRTRACK_VBAT_300MV;

	desc->en_te = of_property_read_bool(np, "en-te");
	desc->en_jeita = of_property_read_bool(np, "en-jeita");
	desc->ceb_invert = of_property_read_bool(np, "ceb-invert");
	desc->dis_i2c_tout = of_property_read_bool(np, "dis-i2c-tout");
	desc->en_qon_rst = of_property_read_bool(np, "en-qon-rst");
	desc->auto_aicr = of_property_read_bool(np, "auto-aicr");

	memcpy(chip->irq_mask, g_rt9471_irq_maskall, RT9471_IRQIDX_MAX);
	while (true) {
		ret = of_property_read_string_index(np, "interrupt-names",
			irq_cnt, &name);
		if (ret < 0)
			break;
		irq_cnt++;
		irq_num = rt9471_get_irq_number(chip, name);
		if (irq_num >= 0)
			rt9471_irq_unmask(chip, irq_num);
	}

	chip->desc = desc;
	return 0;
}

static int rt9471_sw_workaround(struct rt9471_chip *chip)
{
	int ret;
	u8 regval = 0;

	hwlog_info("%s\n", __func__);

	ret = rt9471_enable_hidden_mode(chip, true);
	if (ret < 0) {
		hwlog_err("enter hidden mode fail %d\n", ret);
		return ret;
	}

	ret = rt9471_i2c_read_byte(chip, RT9471_REG_HIDDEN_0, &regval);
	if (ret < 0) {
		hwlog_err("read HIDDEN_0 fail %d\n", ret);
		goto hidden_mode_exit;
	}
	chip->chip_rev = (regval & RT9471_CHIP_REV_MASK) >>
		RT9471_CHIP_REV_SHIFT;
	hwlog_info("chip_rev = %d\n", chip->chip_rev);

	/* OTG load transient improvement */
	if (chip->chip_rev <= RT9471_CHIP_VER_3)
		ret = rt9471_i2c_update_bits(chip, RT9471_REG_OTG_HDEN2, 0x10,
			RT9471_REG_OTG_RES_COMP_MASK);

hidden_mode_exit:
	rt9471_enable_hidden_mode(chip, false);
	return ret;
}

static int rt9471_init_setting(struct rt9471_chip *chip)
{
	int ret;
	struct rt9471_desc *desc = chip->desc;
	u8 evt[RT9471_IRQIDX_MAX] = {0};

	hwlog_info("%s\n", __func__);

	/* Disable WDT during IRQ masked period */
	ret = __rt9471_set_wdt(chip, 0);
	if (ret < 0)
		hwlog_err("%s set wdt fail %d\n", __func__, ret);

	/* Mask all IRQs */
	ret = rt9471_i2c_block_write(chip, RT9471_REG_MASK0,
		ARRAY_SIZE(g_rt9471_irq_maskall), g_rt9471_irq_maskall);
	if (ret < 0) {
		hwlog_err("%s mask irq fail %d\n", __func__, ret);
		return ret;
	}

	/* Clear all IRQs */
	ret = rt9471_i2c_block_read(chip, RT9471_REG_IRQ0, RT9471_IRQIDX_MAX,
		evt);
	if (ret < 0) {
		hwlog_err("%s clear irq fail %d\n", __func__, ret);
		return ret;
	}
	ret = __rt9471_set_acov(chip, desc->acov);
	if (ret < 0)
		hwlog_err("%s set ac_ov fail %d\n", __func__, ret);

	ret = __rt9471_set_ichg(chip, desc->ichg);
	if (ret < 0)
		hwlog_err("%s set ichg fail %d\n", __func__, ret);

	ret = __rt9471_set_aicr(chip, desc->aicr);
	if (ret < 0)
		hwlog_err("%s set aicr fail %d\n", __func__, ret);

	ret = __rt9471_set_mivr(chip, desc->mivr);
	if (ret < 0)
		hwlog_err("%s set mivr fail %d\n", __func__, ret);

	ret = __rt9471_set_cv(chip, desc->cv);
	if (ret < 0)
		hwlog_err("%s set cv fail %d\n", __func__, ret);

	ret = __rt9471_set_ieoc(chip, desc->ieoc);
	if (ret < 0)
		hwlog_err("%s set ieoc fail %d\n", __func__, ret);

	ret = __rt9471_set_safe_tmr(chip, desc->safe_tmr);
	if (ret < 0)
		hwlog_err("%s set safe tmr fail %d\n",
			__func__, ret);

	ret = __rt9471_set_wdt(chip, desc->wdt);
	if (ret < 0)
		hwlog_err("%s set wdt fail %d\n", __func__, ret);

	ret = __rt9471_set_mivrtrack(chip, desc->mivr_track);
	if (ret < 0)
		hwlog_err("%s set mivrtrack fail %d\n", __func__,
			ret);

	ret = __rt9471_enable_safe_tmr(chip, desc->en_safe_tmr);
	if (ret < 0)
		hwlog_err("%s en safe tmr fail %d\n",
			__func__, ret);

	ret = __rt9471_enable_te(chip, desc->en_te);
	if (ret < 0)
		hwlog_err("%s en te fail %d\n", __func__, ret);

	ret = __rt9471_enable_jeita(chip, desc->en_jeita);
	if (ret < 0)
		hwlog_err("%s en jeita fail %d\n", __func__, ret);

	ret = __rt9471_disable_i2c_tout(chip, desc->dis_i2c_tout);
	if (ret < 0)
		hwlog_err("%s dis i2c tout fail %d\n",
			__func__, ret);

	ret = __rt9471_enable_qon_rst(chip, desc->en_qon_rst);
	if (ret < 0)
		hwlog_err("%s en qon rst fail %d\n",
			__func__, ret);

	ret = __rt9471_enable_autoaicr(chip, desc->auto_aicr);
	if (ret < 0)
		hwlog_err("%s en autoaicr fail %d\n",
			__func__, ret);

	ret = rt9471_sw_workaround(chip);
	if (ret < 0)
		hwlog_err("set sw workaround fail %d\n", ret);

	return 0;
}

static int rt9471_reset_register(struct rt9471_chip *chip)
{
	hwlog_info("%s\n", __func__);
	return rt9471_set_bit(chip, RT9471_REG_INFO, RT9471_REGRST_MASK);
}

static bool rt9471_check_devinfo(struct rt9471_chip *chip)
{
	int ret;

	ret = i2c_smbus_read_byte_data(chip->client, RT9471_REG_INFO);
	if (ret < 0) {
		hwlog_err("get devinfo fail %d\n", ret);
		return false;
	}
	chip->dev_id = (ret & RT9471_DEVID_MASK) >> RT9471_DEVID_SHIFT;
	if (chip->dev_id != RT9471_DEVID && chip->dev_id != RT9471D_DEVID) {
		hwlog_err("incorrect devid 0x%02X\n", chip->dev_id);
		return false;
	}

	chip->dev_rev = (ret & RT9471_DEVREV_MASK) >> RT9471_DEVREV_SHIFT;
	hwlog_info("%s id = 0x%02X, rev = 0x%02X\n", __func__,
		chip->dev_id, chip->dev_rev);
	return true;
}

static int __rt9471_dump_registers(struct rt9471_chip *chip)
{
	int i, ret;
	u32 ichg = 0;
	u32 aicr = 0;
	u32 mivr = 0;
	u32 ieoc = 0;
	u32 cv = 0;
	bool chg_en = false;
	enum rt9471_ic_stat ic_stat = RT9471_ICSTAT_VBUSRDY;
	u8 stats[RT9471_STATIDX_MAX] = {0};
	u8 regval = 0;

	ret = __rt9471_kick_wdt(chip);

	ret |= __rt9471_get_ichg(chip, &ichg);
	ret |= __rt9471_get_aicr(chip, &aicr);
	ret |= __rt9471_get_mivr(chip, &mivr);
	ret |= __rt9471_get_ieoc(chip, &ieoc);
	ret |= __rt9471_get_cv(chip, &cv);
	ret |= __rt9471_is_chg_enabled(chip, &chg_en);
	ret |= __rt9471_get_ic_stat(chip, &ic_stat);
	if (ret)
		hwlog_info("%s fail\n", __func__);

	ret = rt9471_i2c_block_read(chip, RT9471_REG_STAT0,
		RT9471_STATIDX_MAX, stats);
	if (ret < 0)
		hwlog_info("%s read block fail\n", __func__);

	if (ic_stat == RT9471_ICSTAT_CHGFAULT) {
		for (i = 0; i < ARRAY_SIZE(rt9471_reg_addr); i++) {
			ret = rt9471_i2c_read_byte(chip, rt9471_reg_addr[i],
				&regval);
			if (ret < 0)
				continue;
			hwlog_info("%s reg0x%02X = 0x%02X\n", __func__,
				rt9471_reg_addr[i], regval);
		}
	}

	hwlog_info("ICHG = %dmA, AICR = %dmA, MIVR = %dmV\n",
		ichg / CONVERT_MULTI_1000, aicr / CONVERT_MULTI_1000,
		mivr / CONVERT_MULTI_1000);

	hwlog_info("IEOC = %dmA, CV = %dmV\n",
		ieoc / CONVERT_MULTI_1000, cv / CONVERT_MULTI_1000);

	hwlog_info("CHG_EN = %d, IC_STAT = %s\n",
		chg_en, g_rt9471_ic_stat_name[ic_stat]);

	hwlog_info("STAT0 = 0x%02X, STAT1 = 0x%02X\n",
		stats[RT9471_STATIDX_STAT0], stats[RT9471_STATIDX_STAT1]);

	hwlog_info("STAT2 = 0x%02X, STAT3 = 0x%02X\n",
		stats[RT9471_STATIDX_STAT2], stats[RT9471_STATIDX_STAT3]);

	return 0;
}

static int rt9471_enable_hz(int en)
{
	struct rt9471_chip *chip = NULL;
	struct rt9471_desc *desc = NULL;
	static int first_in = 1;
	int ret;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	desc = chip->desc;
	if (!desc)
		return -EINVAL;

	if (en > 0) {
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
		if (desc->hiz_iin_limit == 1 && is_uscp_hiz_mode() &&
			!is_in_rt_uscp_mode()) {
			g_hiz_iin_limit_flag = HIZ_IIN_FLAG_TRUE;
			if (first_in) {
				hwlog_info("is_uscp_hiz_mode, set 100mA\n");
				first_in = 0;
				return rt9471_set_aicr(RT9471_AICR_100);
			} else {
				return 0;
			}
		} else {
#endif /* CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT */
			ret = __rt9471_enable_hz(chip, en);
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
		}
#endif /* CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT */
	} else {
		ret = __rt9471_enable_hz(chip, en);
		g_hiz_iin_limit_flag = HIZ_IIN_FLAG_FALSE;
		first_in = 1;
	}

	return ret;
}

static int rt9471_enable_chg(int en)
{
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	return __rt9471_enable_chg(chip, en);
}

static int rt9471_set_aicr(int aicr)
{
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	if (g_hiz_iin_limit_flag == HIZ_IIN_FLAG_TRUE) {
		hwlog_err("g_hiz_iin_limit_flag,just set 100mA\n");
		aicr = RT9471_AICR_100;
	}

	return __rt9471_set_aicr(chip, aicr);
}

static int rt9471_set_ichg(int ichg)
{
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	return __rt9471_set_ichg(chip, ichg);
}

static int rt9471_set_cv(int cv)
{
	struct rt9471_chip *chip = NULL;
	struct rt9471_desc *desc = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	desc = chip->desc;
	if (!desc)
		return -EINVAL;

	if ((desc->cust_cv > CUST_MIN_CV) && (cv > desc->cust_cv)) {
		hwlog_info("set cv to custom_cv=%d\n", desc->cust_cv);
		cv = desc->cust_cv;
	}

	return __rt9471_set_cv(chip, cv);
}

static int rt9471_set_mivr(int mivr)
{
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	return __rt9471_set_mivr(chip, mivr);
}

static int rt9471_set_ieoc(int ieoc)
{
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	return __rt9471_set_ieoc(chip, ieoc);
}

static int rt9471_set_otgcc(int cc)
{
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	return __rt9471_set_otgcc(chip, cc);
}

static int rt9471_enable_otg(int en)
{
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	return __rt9471_enable_otg(chip, en);
}

static int rt9471_enable_te(int en)
{
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	return __rt9471_enable_te(chip, en);
}

static int rt9471_set_wdt(int wdt)
{
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	if (wdt == WDT_STOP)
		return __rt9471_set_wdt(chip, 0);
	return __rt9471_set_wdt(chip, wdt);
}

static int rt9471_get_charge_state(unsigned int *state)
{
	int ret;
	u8 stat[RT9471_IRQIDX_MAX];
	struct rt9471_chip *chip = NULL;
	enum rt9471_ic_stat ic_stat = RT9471_ICSTAT_VBUSRDY;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	if (!state)
		return -EINVAL;

	ret = __rt9471_get_ic_stat(chip, &ic_stat);
	if (ret < 0)
		return ret;

	ret = rt9471_i2c_block_read(chip, RT9471_REG_STAT0, RT9471_IRQIDX_MAX,
		stat);
	if (ret < 0)
		return ret;

	if (ic_stat == RT9471_ICSTAT_CHGDONE)
		*state |= CHAGRE_STATE_CHRG_DONE;
	if (!(stat[RT9471_IRQIDX_IRQ0] & RT9471_ST_VBUSGD_MASK))
		*state |= CHAGRE_STATE_NOT_PG;
	if (stat[RT9471_IRQIDX_IRQ1] & RT9471_ST_BATOV_MASK)
		*state |= CHAGRE_STATE_BATT_OVP;
	if (stat[RT9471_IRQIDX_IRQ3] & RT9471_ST_VACOV_MASK)
		*state |= CHAGRE_STATE_VBUS_OVP;

	return 0;
}

static int rt9471_kick_wdt(void)
{
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	return __rt9471_kick_wdt(chip);
}

static int rt9471_check_input_dpm_state(void)
{
	int ret;
	u8 stat = 0;
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return FALSE;

	ret = rt9471_i2c_read_byte(chip, RT9471_REG_STAT1, &stat);
	if (ret < 0)
		return FALSE;

	if ((stat & RT9471_ST_MIVR_MASK) || (stat & RT9471_ST_AICR_MASK))
		return TRUE;
	return FALSE;
}

static int rt9471_check_input_vdpm_state(void)
{
	int ret;
	u8 stat = 0;
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return FALSE;

	ret = rt9471_i2c_read_byte(chip, RT9471_REG_STAT1, &stat);
	if (ret < 0)
		return FALSE;

	if (stat & RT9471_ST_MIVR_MASK)
		return TRUE;
	return FALSE;
}

static int rt9471_check_input_idpm_state(void)
{
	int ret;
	u8 stat = 0;
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return FALSE;

	ret = rt9471_i2c_read_byte(chip, RT9471_REG_STAT1, &stat);
	if (ret < 0)
		return FALSE;

	if (stat & RT9471_ST_AICR_MASK)
		return TRUE;
	return FALSE;
}

static int rt9471_get_register_head(char *reg_head)
{
	char buff[BUF_LEN] = {0};
	int i;
	int len = 0;

	memset(reg_head, 0, CHARGELOG_SIZE);
	for (i = 0; i < ARRAY_SIZE(rt9471_reg_addr); i++) {
		snprintf(buff, sizeof(buff), "Reg[0x%02X] ",
			rt9471_reg_addr[i]);

		len += strlen(buff);
		if (len < CHARGELOG_SIZE)
			strncat(reg_head, buff, strlen(buff));
	}
	return 0;
}

static int rt9471_dump_register(char *reg_value)
{
	int i, ret;
	struct rt9471_chip *chip = NULL;
	char buff[BUF_LEN] = {0};
	u8 regval;
	int len = 0;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	memset(reg_value, 0, CHARGELOG_SIZE);
	for (i = 0; i < ARRAY_SIZE(rt9471_reg_addr); i++) {
		ret = rt9471_i2c_read_byte(chip, rt9471_reg_addr[i], &regval);
		if (ret < 0)
			regval = 0;

		snprintf(buff, sizeof(buff), "0x%-8.2x", regval);
		len += strlen(buff);
		if (len < CHARGELOG_SIZE)
			strncat(reg_value, buff, strlen(buff));
	}

	return 0;
}

static int rt9471_enable_shipmode(int en)
{
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return -EINVAL;

	return __rt9471_enable_shipmode(chip, en);
}

static int rt9471_5v_chip_init(void)
{
	g_hiz_iin_limit_flag = HIZ_IIN_FLAG_FALSE;

	return 0;
}

static int rt9471_chip_init(struct chip_init_crit *init_crit)
{
	int ret = 0;

	if (!init_crit)
		return -ENOMEM;

	switch (init_crit->vbus) {
	case ADAPTER_5V:
		ret = rt9471_5v_chip_init();
		break;
	default:
		break;
	}

	return ret;
}

static int rt9471_device_check(void)
{
	struct rt9471_chip *chip = NULL;

	chip = i2c_get_clientdata(g_rt9471_i2c);
	if (!chip)
		return CHARGE_IC_BAD;

	if (!rt9471_check_devinfo(chip))
		return CHARGE_IC_BAD;

	return CHARGE_IC_GOOD;
}

static struct charge_device_ops rt9471_ops = {
	.set_charger_hiz = rt9471_enable_hz,
	.set_charge_enable = rt9471_enable_chg,
	.set_input_current = rt9471_set_aicr,
	.set_charge_current = rt9471_set_ichg,
	.set_terminal_voltage = rt9471_set_cv,
	.set_dpm_voltage = rt9471_set_mivr,
	.set_terminal_current = rt9471_set_ieoc,
	.set_otg_current = rt9471_set_otgcc,
	.set_otg_enable = rt9471_enable_otg,
	.set_term_enable = rt9471_enable_te,
	.set_watchdog_timer = rt9471_set_wdt,
	.get_charge_state = rt9471_get_charge_state,
	.reset_watchdog_timer = rt9471_kick_wdt,
	.check_input_dpm_state = rt9471_check_input_dpm_state,
	.check_input_vdpm_state = rt9471_check_input_vdpm_state,
	.check_input_idpm_state = rt9471_check_input_idpm_state,
	.get_register_head = rt9471_get_register_head,
	.dump_register = rt9471_dump_register,
	.set_batfet_disable = rt9471_enable_shipmode,
	.chip_init = rt9471_chip_init,
	.dev_check = rt9471_device_check,
};

static int rt9471_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct rt9471_chip *chip = NULL;

	hwlog_info("%s begin\n", __func__);

	if (!client)
		return -EINVAL;

	chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;
	chip->client = client;
	chip->dev = &client->dev;
	mutex_init(&chip->io_lock);
	mutex_init(&chip->hidden_mode_lock);
	chip->hidden_mode_cnt = 0;
	i2c_set_clientdata(client, chip);
	g_rt9471_i2c = client;

	if (!rt9471_check_devinfo(chip)) {
		ret = -ENODEV;
		goto err_nodev;
	}

	ret = rt9471_parse_dt(chip);
	if (ret < 0) {
		hwlog_err("parse dt fail %d\n", ret);
		goto err_parse_dt;
	}

	ret = rt9471_reset_register(chip);
	if (ret < 0)
		hwlog_err("reset register fail %d\n", ret);

	ret = rt9471_init_setting(chip);
	if (ret < 0) {
		hwlog_err("init setting fail %d\n", ret);
		goto err_init;
	}

	ret = rt9471_register_irq(chip);
	if (ret < 0) {
		hwlog_err("register irq fail %d\n", ret);
		goto err_register_irq;
	}

	ret = rt9471_init_irq(chip);
	if (ret < 0) {
		hwlog_err("init irq fail %d\n", ret);
		goto err_init_irq;
	}

	ret = charge_ops_register(&rt9471_ops);
	if (ret < 0) {
		hwlog_err("register ops fail %d\n", ret);
		goto err_chgops;
	}

	__rt9471_dump_registers(chip);
	hwlog_info("%s probe end\n", __func__);
	return 0;

err_chgops:
err_init_irq:
err_register_irq:
err_init:
err_parse_dt:
err_nodev:
	mutex_destroy(&chip->io_lock);
	mutex_destroy(&chip->hidden_mode_lock);
	devm_kfree(chip->dev, chip);
	return ret;
}

static void rt9471_shutdown(struct i2c_client *client)
{
	struct rt9471_chip *chip = i2c_get_clientdata(client);

	hwlog_info("%s\n", __func__);
	if (chip)
		rt9471_reset_register(chip);
}

static int rt9471_remove(struct i2c_client *client)
{
	struct rt9471_chip *chip = i2c_get_clientdata(client);

	if (chip) {
		hwlog_info("%s\n", __func__);
		mutex_destroy(&chip->io_lock);
		mutex_destroy(&chip->hidden_mode_lock);
	}
	return 0;
}

static int rt9471_suspend(struct device *dev)
{
	struct rt9471_chip *chip = dev_get_drvdata(dev);

	hwlog_info("%s\n", __func__);
	if (device_may_wakeup(dev))
		enable_irq_wake(chip->irq);
	return 0;
}

static int rt9471_resume(struct device *dev)
{
	struct rt9471_chip *chip = dev_get_drvdata(dev);

	hwlog_info("%s\n", __func__);
	if (device_may_wakeup(dev))
		disable_irq_wake(chip->irq);
	return 0;
}

static SIMPLE_DEV_PM_OPS(rt9471_pm_ops, rt9471_suspend, rt9471_resume);

static const struct of_device_id rt9471_of_device_id[] = {
	{ .compatible = "richtek,rt9466", },
	{},
};
MODULE_DEVICE_TABLE(of, rt9471_of_device_id);

static const struct i2c_device_id rt9471_i2c_device_id[] = {
	{ "rt9471", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, rt9471_i2c_device_id);

static struct i2c_driver rt9471_i2c_driver = {
	.driver = {
		.name = "rt9471",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(rt9471_of_device_id),
		.pm = &rt9471_pm_ops,
	},
	.probe = rt9471_probe,
	.shutdown = rt9471_shutdown,
	.remove = rt9471_remove,
	.id_table = rt9471_i2c_device_id,
};

static int __init rt9471_init(void)
{
	return i2c_add_driver(&rt9471_i2c_driver);
}

static void __exit rt9471_exit(void)
{
	i2c_del_driver(&rt9471_i2c_driver);
}

module_init(rt9471_init);
module_exit(rt9471_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("rt9471 charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
