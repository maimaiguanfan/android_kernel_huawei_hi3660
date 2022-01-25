/*
 * Copyright (C) 2018 Hisilicon
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

#ifndef _HISI_TCPC_REG_OPS_H_
#define _HISI_TCPC_REG_OPS_H_

#include <linux/i2c.h>

struct hisi_tcpc_reg_ops {
	int (*block_read)(u16 reg, u8 *dst, unsigned len);
	int (*block_write)(u16 reg, u8 *src, unsigned len);
};

#ifdef CONFIG_HISI_TCPC
void hisi_tcpc_reg_ops_register(struct i2c_client *client, struct hisi_tcpc_reg_ops *reg_ops);
void hisi_tcpc_irq_gpio_register(struct i2c_client *client, int irq_gpio);
#else
static inline void hisi_tcpc_reg_ops_register(struct i2c_client *client,
		struct hisi_tcpc_reg_ops *reg_ops){}
static inline void hisi_tcpc_irq_gpio_register(struct i2c_client *client, int irq_gpio){}
#endif

#endif /* _HISI_TCPC_REG_OPS_H_ */
