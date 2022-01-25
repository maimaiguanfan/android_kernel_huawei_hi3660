/*
 * boost_5v.h
 *
 * boost with 5v driver
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

#ifndef _BOOST_5V_H_
#define _BOOST_5V_H_

#define BOOST_5V_ENABLE      (1)
#define BOOST_5V_DISABLE     (0)

enum boost_ctrl_source_type {
	BOOST_CTRL_BEGIN = 0,

	BOOST_CTRL_BOOST_GPIO_OTG = BOOST_CTRL_BEGIN,
	BOOST_CTRL_PD_VCONN,
	BOOST_CTRL_DC,
	BOOST_CTRL_MOTOER,
	BOOST_CTRL_AUDIO,
	BOOST_CTRL_AT_CMD,
	BOOST_CTRL_FCP,
	BOOST_CTRL_WLDC,
	BOOST_CTRL_WLTX,
	BOOST_CTRL_WLC,

	BOOST_CTRL_END,
};

#ifdef CONFIG_BOOST_5V
extern int boost_5v_enable(bool enable, enum boost_ctrl_source_type type);
extern unsigned int boost_5v_status(void);

#else
static inline int boost_5v_enable(bool enable, enum boost_ctrl_source_type type)
{
	return 0;
}

static inline unsigned int boost_5v_status(void)
{
	return 0;
}
#endif /* CONFIG_BOOST_5V */

#endif /* _BOOST_5V_H_ */
