/*
 * hisi_coul_imonitor.c
 *
 * define the imonitor report interface for coul
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

#include "hisi_coul_imonitor.h"

/* prepare for imonitor para packet */
static int coul_imonitor_basp_prepare_basic_info(
	struct imonitor_eventobj *obj, void *data);

/* imonitor table list */
static struct coul_imonitor_event_id imonitor_event_id[] = {
	{
		COUL_IMONITOR_BASP_BASIC_INFO,
		COUL_IMONITOR_BASP_BASIC_INFO_NO,
		.prepare_cb = coul_imonitor_basp_prepare_basic_info,
		{.tv_sec = 0, .tv_nsec = 0},
		COUL_IMONITOR_REPORT_INTERVAL_WEEKLY
	},
};

static int coul_imonitor_basp_prepare_basic_info(
	struct imonitor_eventobj *obj, void *data)
{
	unsigned int ret = 0;
	struct coul_imonitor_basp_basic_info *basp_data = NULL;

	if (!obj || !data) {
		coul_imonitor_err("%s invalid argument\n", __func__);
		return -1;
	}

	basp_data = (struct coul_imonitor_basp_basic_info *)data;

	ret |= imonitor_set_param_integer_v2(obj, "ActDays",
		basp_data->act_days);
	ret |= imonitor_set_param_integer_v2(obj, "BatCap",
		basp_data->bat_cap);
	ret |= imonitor_set_param_integer_v2(obj, "BatTVSet",
		basp_data->bat_tv_set);
	ret |= imonitor_set_param_integer_v2(obj, "BatCyc",
		basp_data->bat_cyc);
	ret |= imonitor_set_param_integer_v2(obj, "BaspLv",
		basp_data->basp_lv);
	ret |= imonitor_set_param_integer_v2(obj, "Basp1Days",
		basp_data->basp_1_days);
	ret |= imonitor_set_param_integer_v2(obj, "Basp2Days",
		basp_data->basp_2_days);
	ret |= imonitor_set_param_integer_v2(obj, "Basp3Days",
		basp_data->basp_3_days);
	ret |= imonitor_set_param_integer_v2(obj, "Basp4Days",
		basp_data->basp_4_days);
	ret |= imonitor_set_param_integer_v2(obj, "Qmax",
		basp_data->q_max);
	ret |= imonitor_set_param_integer_v2(obj, "FGFcc1",
		basp_data->fg_fcc1);
	ret |= imonitor_set_param_integer_v2(obj, "FGFcc2",
		basp_data->fg_fcc2);
	ret |= imonitor_set_param_integer_v2(obj, "FGFcc3",
		basp_data->fg_fcc3);
	ret |= imonitor_set_param_integer_v2(obj, "ChaICTp",
		basp_data->cha_ic_tp);

	ret |= imonitor_set_param_string_v2(obj, "BatMan",
		basp_data->bat_man);
	ret |= imonitor_set_param_string_v2(obj, "BatMod",
		basp_data->bat_mode);
	ret |= imonitor_set_param_string_v2(obj, "BatParF",
		basp_data->bat_par_f);
	ret |= imonitor_set_param_string_v2(obj, "HTime1",
		basp_data->fcc_time1);
	ret |= imonitor_set_param_string_v2(obj, "HTime2",
		basp_data->fcc_time2);
	ret |= imonitor_set_param_string_v2(obj, "HTime3",
		basp_data->fcc_time3);

	coul_imonitor_info("%s success\n", __func__);
	return ret;
}

/*
 * Function:        coul_imonitor_get_event_id
 * Description:     get imonitor eventid and prepare callback by type
 * Input:           coul_imonitor_type_t type
 * Output:          NULL
 * Return:          NULL
 */
static unsigned int coul_imonitor_get_event_id(enum coul_imonitor_type type,
	coul_imonitor_prepare_param_cb *prepare)
{
	int count = ARRAY_SIZE(imonitor_event_id);
	unsigned int event_id = 0;
	int i = 0;

	if (type >= COUL_IMONITOR_TYPE_NUM)
		goto err_out;

	for (i = 0; i < count; i++) {
		if (imonitor_event_id[i].type == type) {
			event_id = imonitor_event_id[i].event_id;
			*prepare = imonitor_event_id[i].prepare_cb;
			break;
		}
	}

err_out:
	return event_id;
}

/*
 * Function:        coul_imonitor_report
 * Description:     report imonitor data
 * Input:           coul_imonitor_type_t type
 * Output:          NULL
 * Return:          NULL
 */
void coul_imonitor_report(enum coul_imonitor_type type, void *data)
{
	struct imonitor_eventobj *obj = NULL;
	coul_imonitor_prepare_param_cb prepare = NULL;
	unsigned int event_id = 0;
	int ret = 0;

	if (!data)
		return;

	event_id = coul_imonitor_get_event_id(type, &prepare);
	if ((event_id == 0) || (!prepare)) {
		coul_imonitor_err("%s: invalid argument\n", __func__);
		return;
	}

	obj = imonitor_create_eventobj(event_id);
	if (!obj) {
		coul_imonitor_err("%s: imonitor_create_eventobj %u failed\n",
			__func__, event_id);
		return;
	}

	ret = prepare(obj, data);
	if (ret < 0) {
		coul_imonitor_err("%s: prepare param %u skip %d\n",
			__func__, event_id, ret);
		goto err_out;
	}

	ret = imonitor_send_event(obj);
	if (ret < 0) {
		coul_imonitor_err("%s: imonitor_send_event %u failed %d\n",
			__func__, event_id, ret);
		goto err_out;
	}
	coul_imonitor_info("%s event_id %u success\n", __func__, event_id);

err_out:
	if (obj)
		imonitor_destroy_eventobj(obj);
}


/*
 * Function:        coul_imonitor_weekly_report
 * Description:     report imonitor data once a week
 * Input:           coul_imonitor_type_t type
 * Output:          NULL
 * Return:          NULL
 */
void coul_imonitor_periodic_report(enum coul_imonitor_type type, void *data)
{
	struct timespec64 ts64_imoitor_now;
	struct timespec64 ts64_imoitor_sum;
	struct timespec64 ts64_imoitor_interval;

	if (!data)
		return;

	ts64_imoitor_interval.tv_sec = imonitor_event_id[type].interval;
	ts64_imoitor_interval.tv_nsec = 0;
	ts64_imoitor_now = current_kernel_time64();
	ts64_imoitor_sum = timespec64_add_safe(
		imonitor_event_id[type].ts64_imoitor_last,
		ts64_imoitor_interval);

	if (ts64_imoitor_sum.tv_sec == TIME_T_MAX) {
		coul_imonitor_err(
			"%s time overflow happend when add 24*7 hours\n",
			__func__);
	} else if (timespec64_compare(&ts64_imoitor_sum,
		&ts64_imoitor_now) < 0) {
		coul_imonitor_report(type, data);
		imonitor_event_id[type].ts64_imoitor_last = ts64_imoitor_now;
	}
}
