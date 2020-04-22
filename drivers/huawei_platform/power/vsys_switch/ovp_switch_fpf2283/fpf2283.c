/*
 * fpf2283.c
 *
 * vsys ovp switch fpf2283 driver
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
#include <huawei_platform/power/vsys_switch/vsys_switch.h>
#include "fpf2283.h"

#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif

#ifdef CONFIG_HUAWEI_POWER_DEBUG
#include <huawei_platform/power/power_debug.h>
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG fpf2283_vsys
HWLOG_REGIST();

struct fpf2283_device_info {
	struct device *dev;
	struct i2c_client *client;
	int chip_already_init;
	int get_id_time;
	int device_id;
	int gpio_en;
	int state;
};

static struct fpf2283_device_info *g_fpf2283_dev;

#define MSG_LEN                      (2)

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static int fpf2283_write_block(struct fpf2283_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[1];
	int ret = 0;

	if (di == NULL || value == NULL) {
		hwlog_err("di or value is null\n");
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
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static int fpf2283_read_block(struct fpf2283_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[MSG_LEN];
	u8 buf = 0;
	int ret = 0;

	if (di == NULL || value == NULL) {
		hwlog_err("di or value is null\n");
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

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static int fpf2283_write_byte(u8 reg, u8 value)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;
	/* 2 bytes offset 1 contains the data offset 0 is used by i2c_write */
	u8 temp_buffer[MSG_LEN] = {0};

	/* offset 1 contains the data */
	temp_buffer[1] = value;
	return fpf2283_write_block(di, temp_buffer, reg, 1);
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static int fpf2283_read_byte(u8 reg, u8 *value)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;

	return fpf2283_read_block(di, value, reg, 1);
}

#ifdef POWER_MODULE_DEBUG_FUNCTION
static int fpf2283_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret = 0;
	u8 val = 0;

	ret = fpf2283_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	ret = fpf2283_write_byte(reg, val);

	return ret;
}

static int fpf2283_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret = 0;
	u8 val = 0;

	ret = fpf2283_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return 0;
}

static void fpf2283_dump_register(void)
{
	int ret = 0;
	u8 i = 0;
	u8 val = 0;

	for (i = 0; i < FPF2283_MAX_REGS; ++i) {
		ret = fpf2283_read_byte(i, &val);
		if (ret)
			hwlog_err("dump_register read fail\n");

		hwlog_info("reg [%x]=0x%x\n", i, val);
	}
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

static int fpf2283_get_device_id(int *id)
{
	u8 id_info = 0;
	int ret = 0;
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (di == NULL || id == NULL) {
		hwlog_err("di or id is null\n");
		return -1;
	}

	if (di->get_id_time == FPF2283_USED) {
		*id = di->device_id;
		return 0;
	}

	di->get_id_time = FPF2283_USED;
	ret = fpf2283_read_byte(FPF2283_ID_REG, &id_info);
	if (ret) {
		di->get_id_time = FPF2283_NOT_USED;
		return -1;
	}
	hwlog_info("get_device_id [%x]=0x%x\n", FPF2283_ID_REG, id_info);

	id_info = id_info & FPF2283_ID_VID_MASK;
	switch (id_info) {
	case FPF2283_DEVICE_ID_FPF2283:
		di->device_id = VSYS_OVP_SWITCH_FPF2283;
		break;

	default:
		di->device_id = -1;
		ret = -1;
		hwlog_err("ovp switch get dev_id fail\n");
		break;
	}

	*id = di->device_id;
	return ret;
}

static int fpf2283_set_state(int enable)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (!di->gpio_en) {
		hwlog_err("no device\n");
		return -ENODEV;
	}

	enable = !!enable;

	if (di->state == enable) {
		hwlog_info("set_state state is already %d\n", enable);
		return 0;
	}

	gpio_set_value(di->gpio_en, enable);
	di->chip_already_init = enable ? 1 : 0;
	di->state = enable;
	hwlog_info("set_state: set gpio_en %d\n", enable);

	return 0;
}

static int fpf2283_get_state(void)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	return di->state;
}

static void fpf2283_para_init(struct fpf2283_device_info *di)
{
	di->chip_already_init = 0;
	di->state = FPF2283_CHIP_DISABLE;
	di->get_id_time = FPF2283_NOT_USED;
}

static struct vsys_ovp_switch_device_ops fpf2283_ops = {
	.chip_name = "fpf2283",

	.set_state = fpf2283_set_state,
	.get_state = fpf2283_get_state,
	.get_id = fpf2283_get_device_id,
};

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static ssize_t fpf2283_dbg_show_reg_value(void *dev_data,
	char *buf, size_t size)
{
	u8 val = 0;
	int ret = 0;
	int i = 0;
	char rd_buf[FPF2283_RD_BUF_SIZE] = {0};
	struct fpf2283_device_info *dev_p;

	dev_p = (struct fpf2283_device_info *)dev_data;
	if (dev_p == NULL) {
		hwlog_err("dev_p is null\n");
		return scnprintf(buf, size, "dev_p is null\n");
	}

	for (i = 0; i < FPF2283_MAX_REGS; i++) {
		val = 0;
		memset(rd_buf, 0, FPF2283_RD_BUF_SIZE);
		ret = fpf2283_read_byte(i, &val);
		if (ret == 0)
			scnprintf(rd_buf, FPF2283_RD_BUF_SIZE,
				"regaddr: 0x%x regval: 0x%x\n", i, val);
		else
			scnprintf(rd_buf, FPF2283_RD_BUF_SIZE,
				"regaddr: 0x%x regval: invalid\n", i);

		strncat(buf, rd_buf, strlen(rd_buf));
	}

	return strlen(buf);
}

static ssize_t fpf2283_dbg_store_reg_value(void *dev_data,
	const char *buf, size_t size)
{
	int regaddr = 0;
	int regval = 0;
	int ret = 0;
	struct fpf2283_device_info *dev_p;

	dev_p = (struct fpf2283_device_info *)dev_data;
	if (dev_p == NULL) {
		hwlog_err("dev_p is null\n");
		return -EINVAL;
	}

	if (sscanf(buf, "%d %d", &regaddr, &regval) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	/* maximum value of 8-bit num is 255 */
	if (regaddr < 0 || regaddr >= FPF2283_MAX_REGS
		|| regval < 0 || regval > 255) {
		hwlog_err("regaddr 0x%x or regval 0x%x invalid\n",
			regaddr, regval);
		return -EINVAL;
	}

	ret = fpf2283_write_byte((u8)regaddr, (u8)regval);
	if (ret)
		return -EINVAL;

	hwlog_info("regaddr 0x%x regval 0x%x succeed\n", regaddr, regval);

	return size;
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static int fpf2283_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret = 0;
	struct fpf2283_device_info *di = NULL;
	struct device_node *np = NULL;

	hwlog_info("probe begin\n");

	if (client == NULL || id == NULL) {
		hwlog_err("client or id is null\n");
		return -ENOMEM;
	}

	if (!i2c_check_functionality(client->adapter,
		I2C_FUNC_SMBUS_WORD_DATA)) {
		hwlog_err("i2c_check failed\n");
		return -ENODEV;
	}

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (di == NULL)
		return -ENOMEM;

	g_fpf2283_dev = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	di->gpio_en = of_get_named_gpio(np, "gpio_en", 0);
	hwlog_info("gpio_en=%d\n", di->gpio_en);

	if (!gpio_is_valid(di->gpio_en)) {
		hwlog_err("gpio(gpio_en) is not valid\n");
		ret = -EINVAL;
		goto fpf2283_fail_0;
	}

	ret = gpio_request(di->gpio_en, "gpio_en");
	if (ret) {
		hwlog_err("gpio(gpio_en) request fail\n");
		goto fpf2283_fail_0;
	}

	ret = gpio_direction_output(di->gpio_en, FPF2283_CHIP_DISABLE);
	if (ret) {
		hwlog_err("gpio(gpio_en) set output fail\n");
		goto fpf2283_fail_1;
	}

	fpf2283_para_init(di);

	ret = vsys_ovp_switch_ops_register(&fpf2283_ops);
	if (ret) {
		hwlog_err("register fpf2283 ops failed\n");
		goto fpf2283_fail_1;
	}

#ifdef CONFIG_HUAWEI_POWER_DEBUG
	power_dbg_ops_register("fpf2283_regval", i2c_get_clientdata(client),
		(power_dgb_show)fpf2283_dbg_show_reg_value,
		(power_dgb_store)fpf2283_dbg_store_reg_value);
#endif

	hwlog_info("probe end\n");
	return 0;

fpf2283_fail_1:
	gpio_free(di->gpio_en);
fpf2283_fail_0:
	devm_kfree(&client->dev, di);
	g_fpf2283_dev = NULL;
	return ret;
}

static int fpf2283_remove(struct i2c_client *client)
{
	struct fpf2283_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	/* reset fpf2283 */
	gpio_set_value(di->gpio_en, FPF2283_CHIP_DISABLE);

	if (di->gpio_en)
		gpio_free(di->gpio_en);

	hwlog_info("remove end\n");
	return 0;
}

MODULE_DEVICE_TABLE(i2c, fpf2283);
static const struct of_device_id fpf2283_of_match[] = {
	{
		.compatible = "huawei,fpf2283_vsys",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id fpf2283_i2c_id[] = {
	{"fpf2283_vsys", 0}, {}
};

static struct i2c_driver fpf2283_driver = {
	.probe = fpf2283_probe,
	.remove = fpf2283_remove,
	.id_table = fpf2283_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "huawei,fpf2283_vsys",
		.of_match_table = of_match_ptr(fpf2283_of_match),
	},
};

static int __init fpf2283_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&fpf2283_driver);
	if (ret)
		hwlog_err("i2c_add_driver error\n");

	return ret;
}

static void __exit fpf2283_exit(void)
{
	i2c_del_driver(&fpf2283_driver);
}

module_init(fpf2283_init);
module_exit(fpf2283_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("fpf2283_vsys module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
