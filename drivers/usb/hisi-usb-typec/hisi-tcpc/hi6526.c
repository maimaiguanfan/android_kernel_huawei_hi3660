/*
 * Copyright (C) 2018 Hisilicon Technology Corp.
 * Author: Hisilicon <>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

#include <linux/hisi/usb/hisi_tcpc_ops.h>
#include "hi6526.h"

#ifdef CONFIG_HISI_TCPC_DEBUG
#define D(format, arg...) pr_info("[hi6526][DBG][%s]" format, __func__, ##arg)
#else
#define D(format, arg...) do {} while(0)
#endif
#define I(format, arg...) pr_info("[hi6526][INF][%s]" format, __func__, ##arg)
#define E(format, arg...) pr_err("[hi6526][ERR][%s]" format, __func__, ##arg)

struct i2c_client *hi6526_i2c_client;
int hi6526_irq_gpio;

static struct hisi_tcpc_reg_ops *hi6526_tcpc_reg_ops;

/*
 * Return 0 on sucess, others for failed.
 */
int hisi_tcpc_block_read(u32 reg, int len, void *dst)
{
	return hi6526_tcpc_reg_ops->block_read((u16)reg, (u8 *)dst, (unsigned)len);
}
/*
 * Return 0 on sucess, others for failed.
 */
int hisi_tcpc_block_write(u32 reg, int len, void *src)
{
	return hi6526_tcpc_reg_ops->block_write((u16)reg, (u8 *)src, (unsigned)len);
}
s32 hisi_tcpc_i2c_read8(struct i2c_client *client, u32 reg)
{
	u8 value;
	int ret;
	ret = hi6526_tcpc_reg_ops->block_read((u16)reg, &value, 1);
	if (ret < 0) {
		WARN_ON(1);
		return (s32)ret;
	} else
		return (s32)(u32)value;
}
int hisi_tcpc_i2c_write8(struct i2c_client *client, u32 reg, u8 value)
{
	u8 data = value;
	int ret;
	ret = hi6526_tcpc_reg_ops->block_write((u16)reg, &data, 1);
	if (ret)
		WARN_ON(1);
	return ret;
}
s32 hisi_tcpc_i2c_read16(struct i2c_client *client, u32 reg)
{
	u16 value;
	int ret;
	ret = hi6526_tcpc_reg_ops->block_read((u16)reg, (u8 *)&value, 2);
	if (ret < 0) {
		WARN_ON(1);
		return (s32)ret;
	} else
		return (s32)(u32)le16_to_cpu(value);
}
int hisi_tcpc_i2c_write16(struct i2c_client *client, u32 reg, u16 value)
{
	u16 data = value;
	int ret;
	ret = hi6526_tcpc_reg_ops->block_write((u16)reg, (u8 *)&data, 2);
	if (ret)
		WARN_ON(ret);
	return ret;
}

void hisi_tcpc_reg_ops_register(struct i2c_client *client,
			struct hisi_tcpc_reg_ops *reg_ops)
{
	D("+\n");
	hi6526_i2c_client = client;
	hi6526_tcpc_reg_ops = reg_ops;
	D("-\n");
}
void hisi_tcpc_irq_gpio_register(struct i2c_client *client, int irq_gpio)
{
	D("+\n");
	if (!hi6526_i2c_client)
		hi6526_i2c_client = client;
	hi6526_irq_gpio = irq_gpio;
	D("-\n");
}
