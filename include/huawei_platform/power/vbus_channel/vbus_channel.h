/*
 * vbus_channel.h
 *
 * vbus channel driver
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

#ifndef _VBUS_CHANNEL_H_
#define _VBUS_CHANNEL_H_

#define VBUS_CH_RD_BUF_SIZE        (32)
#define VBUS_CH_WR_BUF_SIZE        (32)

#define VBUS_CH_NO_OP_USER         (0)

#define VBUS_CH_NOT_FORCE_CLOSE    (0)
#define VBUS_CH_FORCE_CLOSE        (1)

#define VBUS_CH_NOT_IN_OTG_MODE    (0)
#define VBUS_CH_IN_OTG_MODE        (1)

enum vbus_ch_sysfs_type {
	VBUS_CH_SYSFS_BEGIN = 0,

	VBUS_CH_SYSFS_SUPPORT_TYPE = VBUS_CH_SYSFS_BEGIN,

	VBUS_CH_SYSFS_END,
};

enum vbus_ch_user {
	VBUS_CH_USER_BEGIN = 0,

	VBUS_CH_USER_WIRED_OTG = VBUS_CH_USER_BEGIN, /* for wired otg */
	VBUS_CH_USER_WR_TX, /* for wireless tx */
	VBUS_CH_USER_DC, /* for direct charger */
	VBUS_CH_USER_PD, /* for pd */
	VBUS_CH_USER_AUDIO, /* for audio */

	VBUS_CH_USER_END,
};

enum vbus_ch_type {
	VBUS_CH_TYPE_BEGIN = 0,

	/* charger for vbus channel */
	VBUS_CH_TYPE_CHARGER = VBUS_CH_TYPE_BEGIN,
	VBUS_CH_TYPE_BOOST_GPIO, /* boost with gpio for vbus channel */

	VBUS_CH_TYPE_END,
};

enum vbus_ch_state {
	VBUS_CH_STATE_BEGIN = 0,

	VBUS_CH_STATE_CLOSE = VBUS_CH_STATE_BEGIN,
	VBUS_CH_STATE_OPEN,

	VBUS_CH_STATE_END,
};

struct vbus_ch_ops {
	const char *type_name;
	int (*open)(unsigned int user, int flag);
	int (*close)(unsigned int user, int flag, int force);
	int (*get_state)(unsigned int user, int *state);
	int (*get_mode)(unsigned int user, int *mode);
	int (*set_switch_mode)(unsigned int user, int mode);
	int (*set_voltage)(unsigned int user, int volt);
	int (*get_voltage)(unsigned int user, int *volt);
};

struct vbus_ch_dev {
	struct device *dev;
	unsigned int support_type;
	unsigned int total_ops;
	struct vbus_ch_ops *p_ops[VBUS_CH_TYPE_END];
};

#ifdef CONFIG_VBUS_CHANNEL

int vbus_ch_ops_register(struct vbus_ch_ops *ops);

int vbus_ch_open(unsigned int user, unsigned int type, int flag);
int vbus_ch_close(unsigned int user, unsigned int type, int flag, int force);
int vbus_ch_get_state(unsigned int user, unsigned int type, int *state);
int vbus_ch_get_mode(unsigned int user, unsigned int type, int *mode);
int vbus_ch_set_switch_mode(unsigned int user, unsigned int type, int mode);
int vbus_ch_set_voltage(unsigned int user, unsigned int type, int volt);
int vbus_ch_get_voltage(unsigned int user, unsigned int type, int *volt);

#else

static inline int vbus_ch_ops_register(struct vbus_ch_ops *ops)
{
	return -1;
}

static inline int vbus_ch_open(unsigned int user, unsigned int type, int flag)
{
	return -1;
}

static inline int vbus_ch_close(unsigned int user,
	unsigned int type, int flag, int force)
{
	return -1;
}

static inline int vbus_ch_get_state(unsigned int user,
	unsigned int type, int *state)
{
	return -1;
}

static inline int vbus_ch_get_mode(unsigned int user,
	unsigned int type, int *mode)
{
	return -1;
}

static inline int vbus_ch_set_switch_mode(unsigned int user,
	unsigned int type, int mode)
{
	return -1;
}

static inline int vbus_ch_set_voltage(unsigned int user,
	unsigned int type, int volt)
{
	return -1;
}

static inline int vbus_ch_get_voltage(unsigned int user,
	unsigned int type, int *volt)
{
	return -1;
}

#endif /* CONFIG_VBUS_CHANNEL */

#endif /* _VBUS_CHANNEL_H_ */
