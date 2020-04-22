/*
 * vbus_channel_boost_gpio.h
 *
 * boost with gpio for vbus channel driver
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

#ifndef _VBUS_CHANNEL_BOOST_GPIO_H_
#define _VBUS_CHANNEL_BOOST_GPIO_H_

#define BOOST_GPIO_SWITCH_DISABLE       (0)
#define BOOST_GPIO_SWITCH_ENABLE        (1)

/* fix a hardware issue, has leakage when open boost gpio */
#define CHARGE_OTG_CLOSE_WORK_TIMEOUT   (3000)

struct boost_gpio_dev {
	struct platform_device *pdev;
	struct device *dev;
	int gpio_en;
	unsigned int user;
	unsigned int mode;
	/* fix a hardware issue, has leakage when open boost gpio */
	int charge_otg_ctl_flag;
	struct delayed_work charge_otg_close_work;
};

#endif /* _VBUS_CHANNEL_BOOST_GPIO_H_ */
