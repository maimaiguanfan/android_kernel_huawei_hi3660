/*
 * ltc7820.h
 *
 * ltc7820 driver
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

#ifndef _LTC7820_H_
#define _LTC7820_H_

#include <linux/spinlock.h>

#define LTC7820_CHIP_ENABLE               (1)
#define LTC7820_CHIP_DISABLE              (0)
#define LTC7820_FREQ_ENABLE               (1)
#define LTC7820_FREQ_DISABLE              (0)

#define LTC7820_INIT_FINISH               (1)
#define LTC7820_NOT_INIT                  (0)
#define LTC7820_ENABLE_INT_NOTIFY         (1)
#define LTC7820_DISABLE_INT_NOTIFY        (0)

struct ltc7820_device_info {
	struct platform_device *pdev;
	struct device *dev;
	struct device_node *dev_node;
	struct work_struct irq_work;
	struct nty_data nty_data;
	spinlock_t int_lock;
	int gpio_en;
	int gpio_freq;
	int gpio_int;
	int irq_int;
	bool is_int_en;
	int chip_already_init;
	int device_id;
};

#endif /* _LTC7820_H_ */
