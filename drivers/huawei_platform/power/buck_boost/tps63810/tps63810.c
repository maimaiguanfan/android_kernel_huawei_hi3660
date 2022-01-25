/*
 * tps63810.c
 *
 * tps63810 driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/buck_boost.h>
#include "tps63810.h"

#define HWLOG_TAG buck_boost_tps63810
HWLOG_REGIST();

struct tps63810_device_info *g_tps63810_di;

static int tps63810_read_block(struct tps63810_device_info *di,
			       u8 reg, u8 *val, u8 num_bytes)
{
	int i;
	int ret;
	struct i2c_msg msg[I2C_RD_MSG_LEN];

	if (!di || !val) {
		hwlog_err("%s: di or val is null\n", __func__);
		return -EINVAL;
	}

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = &reg;
	msg[0].len = 1;

	msg[1].addr = di->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = val;
	msg[1].len = num_bytes;

	for (i = 0; i < TPS_I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(di->client->adapter, msg, I2C_RD_MSG_LEN);
		if (ret == I2C_RD_MSG_LEN)
			break;
		usleep_range(5000, 5100); /* sleep 5ms */
	}

	if (i == TPS_I2C_RETRY_CNT) {
		hwlog_err("%s: fail, start_reg = 0x%x\n", __func__, reg);
		return -EIO;
	}

	return 0;
}

static int tps63810_write_block(struct tps63810_device_info *di,
				u8 reg, u8 *val, u8 num_bytes)
{
	int i;
	struct i2c_msg msg[I2C_WR_MSG_LEN];
	int ret;

	if (!di || !val) {
		hwlog_err("%s: di or val is null\n", __func__);
		return -EINVAL;
	}

	*val = reg;

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = val;
	msg[0].len = num_bytes + 1;

	for (i = 0; i < TPS_I2C_RETRY_CNT; i++) {
		/* i2c_transfer returns number of messages transferred */
		ret = i2c_transfer(di->client->adapter, msg, I2C_WR_MSG_LEN);
		if (ret == I2C_WR_MSG_LEN)
			break;
		usleep_range(5000, 5100); /* sleep 5ms */
	}

	if (i == TPS_I2C_RETRY_CNT) {
		hwlog_err("%s: fail, start_reg = 0x%x\n", __func__, reg);
		return -EIO;
	}

	return 0;
}

static int tps63810_write_byte(u8 reg, u8 val)
{
	u8 buff[TPS63810_ADDR_LEN + BYTE_LEN];
	struct tps63810_device_info *di = g_tps63810_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -EINVAL;
	}

	buff[TPS63810_ADDR_LEN] = val;
	return tps63810_write_block(di, reg, buff, BYTE_LEN);
}

static int tps63810_read_byte(u8 reg, u8 *val)
{
	struct tps63810_device_info *di = g_tps63810_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -EINVAL;
	}

	return tps63810_read_block(di, reg, val, BYTE_LEN);
}

static int tps63810_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret;
	u8 val;

	ret = tps63810_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return 0;
}

static int tps63810_set_vout1(unsigned int vout1)
{
	u8 vset1;

	if (vout1 < TPS63810_MIN_VOUT1 || vout1 > TPS63810_MAX_VOUT1) {
		hwlog_err("%s: vout out of range [%d, %d]mV\n",
			  __func__, TPS63810_MIN_VOUT1, TPS63810_MAX_VOUT1);
		return -EINVAL;
	}

	hwlog_info("[%s] %dmV\n", __func__, vout1);
	vset1 = (u8)((vout1 - TPS63810_MIN_VOUT1) / TPS63810_VOUT1_STEP);
	return tps63810_write_byte(TPS63810_VOUT1_ADDR, vset1);
}

static int tps63810_set_vout2(unsigned int vout2)
{
	u8 vset2;

	if (vout2 < TPS63810_MIN_VOUT2 || vout2 > TPS63810_MAX_VOUT2) {
		hwlog_err("%s: vout out of range [%d, %d]mV\n",
			  __func__, TPS63810_MIN_VOUT2, TPS63810_MAX_VOUT2);
		return -EINVAL;
	}

	hwlog_info("[%s] %dmV\n", __func__, vout2);
	vset2 = (u8)((vout2 - TPS63810_MIN_VOUT2) / TPS63810_VOUT2_STEP);
	return tps63810_write_byte(TPS63810_VOUT2_ADDR, vset2);
}

static int tps63810_set_vout(unsigned int vout)
{
	struct tps63810_device_info *di = g_tps63810_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -EINVAL;
	}

	if (!di->vsel_state)
		return tps63810_set_vout1(vout);
	else
		return tps63810_set_vout2(vout);
}

static bool tps63810_power_good(void)
{
	int ret;
	u8 pg_val;

	/* Bit[0] 0: power good; 1: power not good */
	ret = tps63810_read_mask(TPS63810_STATUS_ADDR, TPS63810_STATUS_PG_MASK,
				 TPS63810_STATUS_PG_SHIFT, &pg_val);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return false;
	}
	if (!pg_val)
		return true;
	else
		return false;
}

static bool tps63810_device_check(void)
{
	return tps63810_power_good();
}

static void tps63810_parse_dts(struct device_node *np,
			       struct tps63810_device_info *di)
{
	int ret;

	ret = of_property_read_u32(np, "vsel_state", &di->vsel_state);
	if (ret) {
		hwlog_err("%s: get vsel_state, set high\n", __func__);
		di->vsel_state = 1;
	}
	hwlog_info("[%s] vsel_state = %d\n", __func__, di->vsel_state);
}

static struct bbst_device_ops tps63810_ops = {
	.device_check   = tps63810_device_check,
	.pwr_good       = tps63810_power_good,
	.set_vout       = tps63810_set_vout,
};

static int tps63810_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	int ret;
	struct device_node *np = NULL;
	struct tps63810_device_info *di = NULL;

	if (!client)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_tps63810_di = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	tps63810_parse_dts(np, di);
	ret = buck_boost_ops_register(&tps63810_ops);
	if (ret) {
		hwlog_err("%s: register bbst ops failed\n", __func__);
		return ret;
	}

	hwlog_info("[%s] ok\n", __func__);

	return 0;
}

MODULE_DEVICE_TABLE(i2c, bbst_tps63810);
static const struct of_device_id tps63810_of_match[] = {
	{
		.compatible = "huawei, buckboost_tps63810",
		.data = NULL,
	},
	{
	},
};
static const struct i2c_device_id tps63810_i2c_id[] = {
	{"bbst_tps63810", 0}, {}
};

static struct i2c_driver tps63810_driver = {
	.probe = tps63810_probe,
	.shutdown = NULL,
	.id_table = tps63810_i2c_id,
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "bbst_tps63810",
		   .of_match_table = of_match_ptr(tps63810_of_match),
	},
};

static int __init tps63810_init(void)
{
	return i2c_add_driver(&tps63810_driver);
}

static void __exit tps63810_exit(void)
{
	i2c_del_driver(&tps63810_driver);
}

fs_initcall_sync(tps63810_init);
module_exit(tps63810_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("tps63810 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
