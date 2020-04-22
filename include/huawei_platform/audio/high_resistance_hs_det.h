/*
 * high_resistance_hs_det.h
 *
 * support for high_res_hs_det module
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

#ifndef __HIGH_RESISTANCE_HS_DET__
#define __HIGH_RESISTANCE_HS_DET__

#include <linux/mutex.h>

/* codec operate used by high_resistance_hs_det driver */
struct high_res_hs_codec_ops {
	void (*high_res_feedback_cal)(void);
};

struct high_res_hs_dev {
	char *name;
	struct high_res_hs_codec_ops ops;
};

enum {
	HIGH_RES_INVALLID_VAL = 0,
	HIGH_RES_GET_FB_VAL = 1,
	HIGH_RES_GET_HS_STATE = 2,
	HIGH_RES_GET_CALIB_STATE = 3,
	HIGH_RES_GET_CALIB_VAL = 4,
	HIGH_RES_GET_MIN_THRESHOLD = 5,
	HIGH_RES_GET_MAX_THRESHOLD = 6,
	HIGH_RES_GET_OUTPUT_AMP = 7,
	HIGH_RES_SET_HS_STATE = 10,
	HIGH_RES_SET_CALIB_STATE = 11,
	HIGH_RES_SET_CALIB_VAL = 12,
};

enum {
	NORMAL_HS = 0,
	HIGH_RES_HS = 1,
};

enum {
	RES_NOT_CALIBRATED = 0,
	RES_CALIBRATED = 1,
};

struct high_res_hs_data {
	unsigned int high_res_min_threshold;
	unsigned int high_res_max_threshold;
	unsigned int high_res_feedback_value; /* R_FB */
	unsigned int output_amplitude; /* V_PEAK */
	unsigned int high_res_hs_status;
	unsigned int res_calib_status;
	unsigned int res_calib_val;
	struct mutex mutex;
	struct high_res_hs_dev *codec_ops_dev;
	void *private_data; /* codec data */
};

bool check_high_res_hs_det_support(void);
int get_high_res_data(const unsigned int cmd);
void set_high_res_data(const unsigned int cmd, const unsigned int val);
int high_res_dev_register(struct high_res_hs_dev *dev, void *codec_data);
#endif /* HIGH_RESISTANCE_HS_DET */

