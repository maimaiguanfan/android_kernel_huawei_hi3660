/*
 * hisi_coul_imonitor.h
 *
 * header file of coul imonitor
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

#ifndef _HISI_COUL_IMONITOR_H_
#define _HISI_COUL_IMONITOR_H_

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/time64.h>
#include <huawei_platform/log/imonitor.h>
#include <huawei_platform/log/imonitor_keys.h>

/* coul imonitor log level define */
#define COUL_IMONITOR_INFO
#ifndef COUL_IMONITOR_INFO
#define coul_imonitor_debug(fmt, args...)do {} while (0)
#define coul_imonitor_info(fmt, args...) do {} while (0)
#define coul_imonitor_warn(fmt, args...) do {} while (0)
#define coul_imonitor_err(fmt, args...)  do {} while (0)
#else
#define coul_imonitor_debug(fmt, args...) \
	pr_debug("[hisi_coul_imonitor]" fmt, ## args)
#define coul_imonitor_info(fmt, args...) \
	pr_info("[hisi_coul_imonitor]" fmt, ## args)
#define coul_imonitor_warn(fmt, args...) \
	pr_warn("[hisi_coul_imonitor]" fmt, ## args)
#define coul_imonitor_err(fmt, args...) \
	pr_err("[hisi_coul_imonitor]" fmt, ## args)
#endif

#define BASP_BASIC_INFO_MAX_LEN 32

/* interval of imonitor report: one week  unit: s */
#define COUL_IMONITOR_REPORT_INTERVAL_WEEKLY (7 * 24 * 60 * 60)

/* imonitor no list */
#define COUL_IMONITOR_BASP_BASIC_INFO_NO (930001007)

enum coul_imonitor_type {
	COUL_IMONITOR_TYPE,
	COUL_IMONITOR_BASP_BASIC_INFO = COUL_IMONITOR_TYPE,
	COUL_IMONITOR_TYPE_NUM,
};

struct coul_imonitor_basp_basic_info {
	unsigned long act_days; /* activation days of terminal devices */
	unsigned int bat_cap; /* battery capacity in mAh */
	unsigned int bat_tv_set; /* charging terminate setting voltage */
	unsigned long bat_cyc; /* Battery charging cycles */
	unsigned int basp_lv; /* Battery aging safe policy level */
	unsigned long basp_1_days; /* Days of Battery aging safe policy 1 */
	unsigned long basp_2_days; /* Days of Battery aging safe policy 2 */
	unsigned long basp_3_days; /* Days of Battery aging safe policy 3 */
	unsigned long basp_4_days; /* Days of Battery aging safe policy 4 */
	unsigned int q_max; /* battery QMAX */
	unsigned int fg_fcc1; /* battery capacity caculated by FG in mAh */
	unsigned int fg_fcc2; /* battery capacity caculated by FG in mAh */
	unsigned int fg_fcc3; /* battery capacity caculated by FG in mAh */
	unsigned int cha_ic_tp; /* Charger IC type */
	char bat_man[BASP_BASIC_INFO_MAX_LEN]; /* battery manufactor id */
	char bat_mode[BASP_BASIC_INFO_MAX_LEN]; /* batt model manufactor id */
	char bat_par_f[BASP_BASIC_INFO_MAX_LEN]; /* batt Flag bit of para */
	char fcc_time1[BASP_BASIC_INFO_MAX_LEN]; /* fcc1 update time */
	char fcc_time2[BASP_BASIC_INFO_MAX_LEN]; /* fcc2 update time */
	char fcc_time3[BASP_BASIC_INFO_MAX_LEN]; /* fcc3 update time */
};

typedef int (*coul_imonitor_prepare_param_cb)(
	struct imonitor_eventobj *, void *);

struct coul_imonitor_event_id {
	enum coul_imonitor_type type;
	unsigned int event_id;
	coul_imonitor_prepare_param_cb prepare_cb;
	struct timespec64 ts64_imoitor_last;
	long  interval;
};

void coul_imonitor_periodic_report(enum coul_imonitor_type type, void *data);
void coul_imonitor_report(enum coul_imonitor_type type, void *data);

#endif /* _HISI_COUL_IMONITOR_H_ */
