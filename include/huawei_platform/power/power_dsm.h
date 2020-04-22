/*
 * power_dsm.h
 *
 * dsm for power module
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

#ifndef _POWER_DSM_H_
#define _POWER_DSM_H_

#include <dsm/dsm_pub.h>

/* define dsm buffer size */
#define POWER_DSM_BUF_SIZE_0016      (16)
#define POWER_DSM_BUF_SIZE_0128      (128)
#define POWER_DSM_BUF_SIZE_0256      (256)
#define POWER_DSM_BUF_SIZE_0512      (512)
#define POWER_DSM_BUF_SIZE_1024      (1024)
#define POWER_DSM_BUF_SIZE_2048      (2048)

enum power_dsm_type {
	/* type begin anchor, can not modify */
	POWER_DSM_TYPE_BEGIN = 0,

	POWER_DSM_CPU_BUCK = POWER_DSM_TYPE_BEGIN,
	POWER_DSM_USB,
	POWER_DSM_BATTERY_DETECT,
	POWER_DSM_BATTERY,
	POWER_DSM_CHARGE_MONITOR,
	POWER_DSM_SUPERSWITCH,
	POWER_DSM_SMPL,
	POWER_DSM_PD_RICHTEK,
	POWER_DSM_PD,
	POWER_DSM_USCP,
	POWER_DSM_PMU_OCP,
	POWER_DSM_PMU_IRQ,
	POWER_DSM_DIRECT_CHARGE_SC,

	/* type end anchor, can not modify */
	POWER_DSM_TYPE_END,
};

struct power_dsm_data_info {
	enum power_dsm_type type;
	const char *name;
	struct dsm_client *client;
	struct dsm_dev *dev;
};

#ifdef CONFIG_HUAWEI_DSM
struct dsm_client *power_dsm_get_dclient(enum power_dsm_type type);
int power_dsm_dmd_report(enum power_dsm_type type, int err_no, void *buf);

#ifdef CONFIG_HUAWEI_DATA_ACQUISITION
int power_dsm_bigdata_report(enum power_dsm_type type, int err_no, void *msg);

#else

static inline int power_dsm_bigdata_report(enum power_dsm_type type,
	int err_no, void *msg)
{
	return 0;
}
#endif /* CONFIG_HUAWEI_DATA_ACQUISITION */

#define power_dsm_dmd_report_format(type, err_no, fmt, args...) do { \
if (power_dsm_get_dclient(type)) { \
	if (!dsm_client_ocuppy(power_dsm_get_dclient(type))) { \
		dsm_client_record(power_dsm_get_dclient(type), fmt, ##args); \
		dsm_client_notify(power_dsm_get_dclient(type), err_no); \
		pr_info("report type:%d, err_no:%d\n", type, err_no); \
	} else { \
		pr_err("power dsm client is busy\n"); \
	} \
} \
} while (0)

#else

static inline struct dsm_client *power_dsm_get_dclient(enum power_dsm_type type)
{
	return NULL;
}

static inline int power_dsm_dmd_report(enum power_dsm_type type,
	int err_no, void *buf)
{
	return 0;
}

static inline int power_dsm_bigdata_report(enum power_dsm_type type,
	int err_no, void *msg)
{
	return 0;
}

#define power_dsm_dmd_report_format(type, err_no, fmt, args...)
#endif /* CONFIG_HUAWEI_DSM */

#endif /* _POWER_DSM_H_ */
