/*
 * soc_control.h
 *
 * battery capacity(soc: state of charge) control driver
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef _SOC_CONTROL_H_
#define _SOC_CONTROL_H_

#define SOC_CTL_START_TIME      30000 /* 30s */
#define SOC_CTL_LOOP_TIME       30000 /* 30s */

#define SOC_CTL_CHG_ENABLE      1
#define SOC_CTL_CHG_DISABLE     0

#define SOC_CTL_IIN_LIMIT       100   /* 100ma */
#define SOC_CTL_IIN_UNLIMIT     0

#define SOC_CTL_RW_BUF_SIZE     32

enum soc_ctl_op_user {
	SOC_CTL_OP_USER_BEGIN = 0,

	SOC_CTL_OP_USER_DEFAULT = SOC_CTL_OP_USER_BEGIN, /* for default */
	SOC_CTL_OP_USER_RC, /* for rc file */
	SOC_CTL_OP_USER_HIDL, /* for hidl interface */
	SOC_CTL_OP_USER_CHARGE_MONITOR, /* for charge_monitor native */
	SOC_CTL_OP_USER_SHELL, /* for shell command */
	SOC_CTL_OP_USER_CUST, /* for cust */

	SOC_CTL_OP_USER_END,
};

enum soc_ctl_sysfs_type {
	SOC_CTL_SYSFS_BEGIN = 0,

	SOC_CTL_SYSFS_CONTROL,

	SOC_CTL_SYSFS_END,
};

enum soc_ctl_event_type {
	SOC_CTL_DEFAULT_EVENT,
	SOC_CTL_START_EVENT, /* start event when usb insert */
	SOC_CTL_STOP_EVENT,  /* stop event when usb not insert */
};

enum soc_ctl_work_mode {
	WORK_IN_DEFAULT_MODE,
	WORK_IN_ENABLE_CHG_MODE,
	WORK_IN_DISABLE_CHG_MODE,
};

struct soc_ctl_dev {
	struct device *dev;
	struct notifier_block nb;
	struct delayed_work work;
	int work_mode;
	int event;
	int user;
	int enable;
	int min_soc;
	int max_soc;
};

#endif /* _SOC_CONTROL_H_ */
