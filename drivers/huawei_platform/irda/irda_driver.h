/*
 * irda_driver.h
 *
 * irda module registe interface
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
 */

#ifndef __IRDA_DRIVER_H
#define __IRDA_DRIVER_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/kdev_t.h>
#include <linux/mutex.h>
#include <huawei_platform/log/hw_log.h>

#define IRDA_DRIVER_COMPATIBLE_ID     "irda,config"
#define IRDA_CHIP_TYPE                "irda,chiptype"

/* struct for the chip type */
enum irda_chiptype {
	DEFAULT = 0,
	MAXIM_616,
	HI11xx,
	HI64XX,
	OTHERS,
};

/*
 * The function of register Maxim irda chip, configure it through DTS.
 * return 0: successful registration or no DTS configuration.
 * return negative value: registration failure.
 */
int irda_maxim_power_config_regist(void);
void irda_maxim_power_config_unregist(void);

/*
 * The function of register hisi irda chip, configure it through DTS.
 * return 0: successful registration or no DTS configuration.
 * return negative value: registration failure.
 */
int irda_chip_type_regist(void);
void irda_chip_type_unregist(void);

#endif
