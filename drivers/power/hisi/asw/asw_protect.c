/*
 * asw_protect.c
 *
 * asw protect driver
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

#include <linux/power/hisi/hisi_battery_data.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/bug.h>
#include <linux/module.h>
#include <huawei_platform/power/power_dsm.h>
#include <securec.h>
#include "asw_protect.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG asw_protect
HWLOG_REGIST();

static u32 g_asw_protect_support;
static int g_asw_iin_limit;
static int g_is_nv_need_save = 1;
static struct wake_lock asw_lock;

int asw_get_iin_limit(void)
{
	return g_asw_iin_limit;
}

void asw_set_iin_limit(int value)
{
	g_asw_iin_limit = value;
}

static void asw_protect_wake_lock(void)
{
	if (!wake_lock_active(&asw_lock)) {
		wake_lock(&asw_lock);
		hwlog_info("asw_protect_wake_lock\n");
	}
}

static void asw_protect_wake_unlock(void)
{
	if (wake_lock_active(&asw_lock)) {
		wake_unlock(&asw_lock);
		hwlog_info("asw_protect_wake_unlock\n");
	}
}

/*
 * asw_print_battery_data - Print battery modeling data
 * @pdat: pdat is battery data
 *
 * Input raw battery modeling  print for debug
 */
static void asw_print_battery_data(struct hisi_coul_battery_data *pdat)
{
	int i;
	int j;
	int ret;
	char buf[1024] = { 0 }; /* 1024:battery data len */

	hwlog_info("pdat->id_voltage_min = %d\n", pdat->id_voltage_min);
	hwlog_info("pdat->id_voltage_max = %d\n", pdat->id_voltage_max);
	hwlog_info("pdat->id_identify_min = %d\n", pdat->id_identify_min);
	hwlog_info("pdat->id_identify_max = %d\n", pdat->id_identify_max);
	hwlog_info("pdat->fcc = %d\n", pdat->fcc);

	/* pdat->fcc_temp_lut */
	hwlog_info("pdat->fcc_temp_lut->x[%d]:\n", MAX_SINGLE_LUT_COLS);
	for (i = 0; i < MAX_SINGLE_LUT_COLS; i++) {
		ret = snprintf_s(buf + strlen(buf), sizeof(buf) - strlen(buf),
			ASW_PRINT_LINE, "%6d ", pdat->fcc_temp_lut->x[i]);
		if (ret == -1) {
			hwlog_err("snprintf_s fcc_temp_lut fail\n");
			return;
		}
	}
	hwlog_info("%s\n", buf);

	hwlog_info("pdat->fcc_temp_lut->y[%d]:\n", MAX_SINGLE_LUT_COLS);
	buf[0] = '\0'; /* clean buf */
	for (i = 0; i < MAX_SINGLE_LUT_COLS; i++) {
		ret = snprintf_s(buf + strlen(buf), sizeof(buf) - strlen(buf),
			ASW_PRINT_LINE, "%6d ", pdat->fcc_temp_lut->y[i]);
		if (ret == -1) {
			hwlog_err("snprintf_s fcc_temp_lut fail\n");
			return;
		}
	}
	hwlog_info("%s\n", buf);
	hwlog_info("pdat->fcc_temp_lut->cols = %d\n", pdat->fcc_temp_lut->cols);

	/* pdat->fcc_sf_lut */
	hwlog_info("pdat->fcc_sf_lut->x[%d]\n", MAX_SINGLE_LUT_COLS);
	buf[0] = '\0'; /* clean buf */
	for (i = 0; i < MAX_SINGLE_LUT_COLS; i++) {
		ret = snprintf_s(buf + strlen(buf), sizeof(buf) - strlen(buf),
			ASW_PRINT_LINE, "%6d ", pdat->fcc_sf_lut->x[i]);
		if (ret == -1) {
			hwlog_err("snprintf_s fcc_temp_lut fail\n");
			return;
		}
	}
	hwlog_info("%s\n", buf);

	hwlog_info("pdat->fcc_sf_lut->y[%d]\n", MAX_SINGLE_LUT_COLS);
	buf[0] = '\0'; /* clean buf */
	for (i = 0; i < MAX_SINGLE_LUT_COLS; i++) {
		ret = snprintf_s(buf + strlen(buf), sizeof(buf) - strlen(buf),
			ASW_PRINT_LINE, "%6d ", pdat->fcc_sf_lut->y[i]);
		if (ret == -1) {
			hwlog_err("snprintf_s fcc_temp_lut fail\n");
			return;
		}
	}
	hwlog_info("%s\n", buf);
	hwlog_info("pdat->fcc_sf_lut->cols = %d\n", pdat->fcc_sf_lut->cols);

	/* pdat->pc_temp_ocv_lut */
	hwlog_info("pdat->pc_temp_ocv_lut->rows = %d\n",
		pdat->pc_temp_ocv_lut->rows);
	hwlog_info("pdat->pc_temp_ocv_lut->cols = %d\n",
		pdat->pc_temp_ocv_lut->cols);
	hwlog_info("pdat->pc_temp_ocv_lut->temp[%d]:\n", PC_TEMP_COLS);
	buf[0] = '\0'; /* clean buf */
	for (i = 0; i < PC_TEMP_COLS; i++) {
		ret = snprintf_s(buf + strlen(buf), sizeof(buf) - strlen(buf),
			ASW_PRINT_LINE, "%6d ",
			pdat->pc_temp_ocv_lut->temp[i]);
		if (ret == -1) {
			hwlog_err("snprintf_s fcc_temp_lut fail\n");
			return;
		}
	}
	hwlog_info("%s\n", buf);

	hwlog_info("pdat->pc_temp_cov_lut->percent[%d]:\n", PC_TEMP_ROWS);
	buf[0] = '\0'; /* clean buf */
	for (i = 0; i < PC_TEMP_ROWS; i++) {
		ret = snprintf_s(buf + strlen(buf), sizeof(buf) - strlen(buf),
			ASW_PRINT_LINE, "%6d ",
			pdat->pc_temp_ocv_lut->percent[i]);
		if (ret == -1) {
			hwlog_err("snprintf_s fcc_temp_lut fail\n");
			return;
		}
	}
	hwlog_info("%s\n", buf);
	hwlog_info("pdat->pc_temp_ocv_lut->ocv[%d][%d]:\n",
		PC_TEMP_ROWS, PC_TEMP_COLS);
	for (i = 0; i < PC_TEMP_ROWS; i++) {
		buf[0] = '\0';
		for (j = 0; j < PC_TEMP_COLS; j++) {
			ret = snprintf_s(buf + strlen(buf),
				sizeof(buf) - strlen(buf), ASW_PRINT_LINE,
				"%6d ", pdat->pc_temp_ocv_lut->ocv[i][j]);
			if (ret == -1) {
				hwlog_err("snprintf_s fcc_temp_lut fail\n");
				return;
			}
		}
		hwlog_info("%s\n", buf);
	}

	/* pdat->vol_dec1 */
	hwlog_info("pdat->vol_dec1 = %d\n", pdat->vol_dec1);
	hwlog_info("pdat->vol_dec2 = %d\n", pdat->vol_dec2);
	hwlog_info("pdat->vol_dec3 = %d\n", pdat->vol_dec3);

	/* pdat->pc_sf_lut */
	hwlog_info("pdat->pc_sf_lut->rows = %d\n", pdat->pc_sf_lut->rows);
	hwlog_info("pdat->pc_sf_lut->rows = %d\n", pdat->pc_sf_lut->cols);
	hwlog_info("pdat->pc_sf_lut->row_entries[%d]:\n", PC_CC_COLS);
	buf[0] = '\0'; /* clean buf */
	for (i = 0; i < PC_CC_COLS; i++) {
		ret = snprintf_s(buf + strlen(buf),
			sizeof(buf) - strlen(buf), ASW_PRINT_LINE,
			"%6d ", pdat->pc_sf_lut->row_entries[i]);
		if (ret == -1) {
			hwlog_err("snprintf_s fcc_temp_lut fail\n");
			return;
		}
	}
	hwlog_info("%s\n", buf);

	hwlog_info("pdat->pc_sf_lut->percent[%d]:\n", PC_CC_ROWS);
	buf[0] = '\0'; /* clean buf */
	for (i = 0; i < PC_CC_ROWS; i++) {
		ret = snprintf_s(buf + strlen(buf),
			sizeof(buf) - strlen(buf), ASW_PRINT_LINE,
			"%6d ", pdat->pc_sf_lut->percent[i]);
		if (ret == -1) {
			hwlog_err("snprintf_s fcc_temp_lut fail\n");
			return;
		}
	}
	hwlog_info("%s\n", buf);
	hwlog_info("pdat->pc_sf_lut[%d][%d]:\n", PC_CC_ROWS, PC_CC_COLS);
	for (i = 0; i < PC_CC_ROWS; i++) {
		buf[0] = '\0'; /* clean buf */
		for (j = 0; j < PC_CC_COLS; j++) {
			ret = snprintf_s(buf + strlen(buf),
				sizeof(buf) - strlen(buf), ASW_PRINT_LINE,
				"%6d ", pdat->pc_sf_lut->sf[i][j]);
			if (ret == -1) {
				hwlog_err("snprintf_s fcc_temp_lut fail\n");
				return;
			}
		}
		hwlog_info("%s\n", buf);
	}

	/* pdat->rbatt_sf_lut */
	hwlog_info("pdat->rbatt_sf_lut->rows = %d\n", pdat->rbatt_sf_lut->rows);
	hwlog_info("pdat->rbatt_sf_lut->rows = %d\n", pdat->rbatt_sf_lut->cols);
	hwlog_info("pdat->rbatt_sf_lut->row_entries[%d]:\n", PC_CC_COLS);
	buf[0] = '\0'; /* clean buf */
	for (i = 0; i < PC_CC_COLS; i++) {
		ret = snprintf_s(buf + strlen(buf),
			sizeof(buf) - strlen(buf), ASW_PRINT_LINE,
			"%6d ", pdat->rbatt_sf_lut->row_entries[i]);
		if (ret == -1) {
			hwlog_err("snprintf_s fcc_temp_lut fail\n");
			return;
		}
	}
	hwlog_info("%s\n", buf);

	hwlog_info("pdat->rbatt_sf_lut->percent[%d]:\n", PC_CC_ROWS);
	buf[0] = '\0'; /* clean buf */
	for (i = 0; i < PC_CC_ROWS; i++) {
		ret = snprintf_s(buf + strlen(buf),
			sizeof(buf) - strlen(buf), ASW_PRINT_LINE,
			"%6d ", pdat->rbatt_sf_lut->percent[i]);
		if (ret == -1) {
			hwlog_err("snprintf_s fcc_temp_lut fail\n");
			return;
		}
	}
	hwlog_info("%s\n", buf);
	hwlog_info("pdat->rbatt_sf_lut->sf[%d][%d]\n", PC_CC_ROWS, PC_CC_COLS);
	for (i = 0; i < PC_CC_ROWS; i++) {
		buf[0] = '\0'; /* clean buf */
		for (j = 0; j < PC_CC_COLS; j++) {
			ret = snprintf_s(buf + strlen(buf),
				sizeof(buf) - strlen(buf), ASW_PRINT_LINE,
				"%6d ", pdat->rbatt_sf_lut->sf[i][j]);
			if (ret == -1) {
				hwlog_err("snprintf_s fcc_temp_lut fail\n");
				return;
			}
		}
		hwlog_info("%s\n", buf);
	}

	hwlog_info("pdat->default_rbatt_mohm = %d\n", pdat->default_rbatt_mohm);
	hwlog_info("pdat->delta_rbatt_mohm = %d\n", pdat->delta_rbatt_mohm);
	hwlog_info("pdat->vbatt_max = %d\n", pdat->vbatt_max);
	hwlog_info("pdat->chrg_para->temp_len = %d\n",
		pdat->chrg_para->temp_len);
	hwlog_info("pdat->chrg_para->temp_data[%d][%d]\n",
		TEMP_PARA_LEVEL, TEMP_PARA_TOTAL);

	for (i = 0; i < TEMP_PARA_LEVEL; i++) {
		buf[0] = '\0'; /* clean buf */
		for (j = 0; j < TEMP_PARA_TOTAL; j++) {
			ret = snprintf_s(buf + strlen(buf),
				sizeof(buf) - strlen(buf), ASW_PRINT_LINE,
				"%6d ", pdat->chrg_para->temp_data[i][j]);
			if (ret == -1) {
				hwlog_err("snprintf_s fcc_temp_lut fail\n");
				return;
			}
		}
		hwlog_info("%s\n", buf);
	}
	hwlog_info("pdat->chrg_para->volt_len = %d\n",
		pdat->chrg_para->volt_len);
	hwlog_info("pdat->chrg_para->volt_data[%d][%d]\n",
		VOLT_PARA_LEVEL, VOLT_PARA_TOTAL);
	for (i = 0; i < VOLT_PARA_LEVEL; i++) {
		buf[0] = '\0'; /* clean buf */
		for (j = 0; j < VOLT_PARA_TOTAL; j++) {
			ret = snprintf_s(buf + strlen(buf),
				sizeof(buf) - strlen(buf), ASW_PRINT_LINE,
				"%6d ", pdat->chrg_para->volt_data[i][j]);
			if (ret == -1) {
				hwlog_err("snprintf_s chrg_para->volt_data fail\n");
				return;
			}
		}
		hwlog_info("%s\n", buf);
	}
	hwlog_info("pdat->chrg_para->segment_len = %d\n",
		pdat->chrg_para->segment_len);
	hwlog_info("pdat->chrg_para->segment_data[%d][%d]\n",
		SEGMENT_PARA_LEVEL, SEGMENT_PARA_TOTAL);
	for (i = 0; i < SEGMENT_PARA_LEVEL; i++) {
		buf[0] = '\0'; /* clean buf */
		for (j = 0; j < SEGMENT_PARA_TOTAL; j++) {
			ret = snprintf_s(buf + strlen(buf),
				sizeof(buf) - strlen(buf), ASW_PRINT_LINE,
				"%6d ", pdat->chrg_para->segment_data[i][j]);
			if (ret == -1) {
				hwlog_err("snprintf_s chrg_para->segment_data fail\n");
				return;
			}
		}
		hwlog_info("%s\n", buf);
	}
	hwlog_info("pdat->batt_brand=%s\n", pdat->batt_brand);
}

/*
 * interpolation_calc
 * calculate y by x
 * @x_array Array x for input
 * @y_array Array y for input
 * @length  The length of the array x and y
 * @x       Input data x in order to calculate y
 *
 * The x array and the y array have a corresponding relationship,
 * giving x, calculating the y value by the difference and retrun
 */
static int interpolation_calc(const int *x_array, const int *y_array,
	int length, int x)
{
	int i;
	int y;
	int temp = MAX_INT;            /* Int maximum value */
	int x_c;                       /* x difference */
	int y_c;                       /* y difference */
	int x1_index = 0;              /* Difference algorithm start node */
	int x2_index = 1;              /* Difference algorithm end node */
	int n = 0;

	if (!x_array || !y_array) {
		hwlog_err("get linear null point\n");
		return -EINVAL;
	}

	/* Find the difference point */
	for (i = 0; i < length - 1; i++) {
		if (x_array[i] - x > 0)
			n += (x_array[i] - x);
		else
			n += (x - x_array[i]);

		if (x_array[i + 1] - x > 0)
			n += (x_array[i + 1] - x);
		else
			n += (x - x_array[i + 1]);

		if (n < temp) {
			temp = n;
			x1_index = i;
			x2_index = i + 1;
		}
	}

	/* Get the difference node and calculate the slope */
	y_c = (y_array[x1_index] - y_array[x2_index]);
	x_c = (x_array[x1_index] - x_array[x2_index]);

	/* The denominator is not zero */
	if (x_c == 0)
		return 0;

	/* Calculate the y value based on the slope */
	y = (y_c * x + y_array[x1_index] * x_c - x_array[x1_index] * y_c) / x_c;
	return y;
}

/*
 * asw_calculate_battery_data - Convert raw battery modeling
 * data based on value
 * @asw reset the battery data at asw_volt
 * @pdat: pdat is battery data
 * @di: coulomb data
 * @value: the battery asw voltage value
 * @return:NOTIFY_OK-success or others
 *
 * Input raw battery modeling is modeled based on the buck value,
 * and if the buck value is 0, the battery modeling data is restored
 */
static int asw_calculate_battery_data(struct hisi_coul_battery_data *pdat,
		struct smartstar_coul_device *di, int value)
{
	int ret;
	int cols;
	int rows;
	int asw_protect_capacity_100;
	int asw_protect_capacity_0;
	int asw_protect_capacity;

	int original_capacity_cols[PC_TEMP_ROWS] = { 0 };
	int original_rbatt_cols[PC_CC_ROWS] = { 0 };
	int original_volt_cols[PC_TEMP_ROWS] = { 0 };

	int asw_capacity_cols[PC_TEMP_ROWS] = { 0 };
	int asw_volt_cols[PC_TEMP_ROWS] = { 0 };
	int asw_rbatt_cols[PC_CC_ROWS] = { 0 };

	int (*ocv)[PC_TEMP_ROWS][PC_TEMP_COLS] = NULL;
	int (*rbatt)[PC_CC_ROWS][PC_CC_COLS] = NULL;
	int *fcc_temp = NULL;
	int *percent = NULL;

	if (!di || !pdat) {
		hwlog_err("asw_calculate_battery_data null point\n");
		return -EINVAL;
	}

	ocv = &(pdat->pc_temp_ocv_lut->ocv);
	rbatt = &(pdat->rbatt_sf_lut->sf);

	fcc_temp = pdat->fcc_temp_lut->y;
	percent = pdat->pc_temp_ocv_lut->percent;

	/* load the batt data from batt_name dtsi */
	ret = asw_protect_load_batt_data();
	if (ret) {
		hwlog_err("asw load batt data fail\n");
		return -EINVAL;
	}

	if (value == NORMAL_SCENE_ASW_PROTECT_VOLT) {
		hwlog_info("back to normal scene, do nothing\n");
		return 0;
	}

	for (cols = 0; cols < PC_TEMP_COLS; cols++) {
		/* get a column batt_data voltages to a row */
		for (rows = 0; rows < PC_TEMP_ROWS; rows++)
			original_volt_cols[rows] = (*ocv)[rows][cols];

		/* get a column batt_data rbatt to a row */
		for (rows = 0; rows < PC_CC_ROWS; rows++)
			original_rbatt_cols[rows] = (*rbatt)[rows][cols];

		/* get a column batt_data capacity to a row, 100: full soc */
		for (rows = 0; rows < PC_TEMP_ROWS; rows++)
			original_capacity_cols[rows] =
				fcc_temp[cols] * (100 - percent[rows]) / 100;

		/* asw battery data corresponding capacity 100% */
		asw_protect_capacity_100 = interpolation_calc(
				original_volt_cols, original_capacity_cols,
				PC_TEMP_ROWS, original_volt_cols[0] - value);

		/* battery data 3200 voltage corresponding capacity 0% */
		asw_protect_capacity_0 = interpolation_calc(
				original_volt_cols, original_capacity_cols,
				PC_TEMP_ROWS, VOLT_LOWEST);

		/* asw battery fcc capacity as asw_protect_voltage */
		asw_protect_capacity = asw_protect_capacity_0 -
				asw_protect_capacity_100;
		fcc_temp[cols] = asw_protect_capacity;

		/* get a column asw protect capacity , 100: full SOC */
		for (rows = 0; rows < PC_TEMP_ROWS; rows++)
			asw_capacity_cols[rows] = asw_protect_capacity_0 -
				(asw_protect_capacity * percent[rows]) / 100;

		/* get a column voltage from asw_protect_capacity */
		for (rows = 0; rows < PC_TEMP_ROWS; rows++)
			asw_volt_cols[rows] = interpolation_calc(
				original_capacity_cols, original_volt_cols,
				PC_TEMP_ROWS, asw_capacity_cols[rows]);

		/* get a column rbatt from asw_protect_capacity */
		for (rows = 0; rows < PC_CC_ROWS; rows++)
			asw_rbatt_cols[rows] = interpolation_calc(
				original_volt_cols, original_rbatt_cols,
				PC_CC_ROWS, asw_volt_cols[rows]);

		/* set a raw rbatt to a column for battery data */
		for (rows = 0; rows < PC_CC_ROWS; rows++)
			(*rbatt)[rows][cols] = asw_rbatt_cols[rows];

		/* set a raw ocv to a column for battery data */
		for (rows = 0; rows < PC_TEMP_ROWS; rows++)
			(*ocv)[rows][cols] = asw_volt_cols[rows];
	}

	pdat->fcc = pdat->fcc_temp_lut->y[3];    /* 25 degree fcc */
	di->batt_data->vbatt_max = di->batt_data->vbatt_max_backup -
			di->hw_nv_info.asw_protect_voltage;
	asw_print_battery_data(pdat);
	return 0;
}

static int asw_dsm_report_info(struct smartstar_coul_device *di, int err_num)
{
	int voltage;
	int ret;
	char dsm_buf[DSM_BUFF_SIZE_MAX] = { 0 };

	if (!di) {
		hwlog_err("asw dsm null point\n");
		return -EINVAL;
	}

	voltage = hisi_battery_voltage();

	ret = snprintf_s(dsm_buf, DSM_BUFF_SIZE_MAX, DSM_BUFF_SIZE_MAX - 1,
		"asw batt_name=%s, temp:%d, volt:%d, charge_cycles:%d\n",
		di->batt_data->batt_brand, di->batt_temp / 10,
		voltage, di->batt_chargecycles / PERCENT);
	if (ret < 0)
		hwlog_err("snprintf dsm_buf fail\n");

	hwlog_info("%s\n", dsm_buf);
	return power_dsm_dmd_report(POWER_DSM_BATTERY, err_num, dsm_buf);
}

static int asw_hw_coul_fcc_operate(struct smartstar_coul_device *di,
	struct hw_coul_nv_info *hw_info, enum nv_operation_type type)
{
	int ret;
	int i;
	int j;
	struct hisi_nve_info_user nve;
	struct ss_coul_nv_info *hi_info = NULL;

	if (!di || !hw_info) {
		hwlog_err("hw coul operate is NULL\n");
		return -EINVAL;
	}

	hi_info = &di->nv_info;
	ret = memset_s(&nve, sizeof(nve), 0, sizeof(nve));
	if (ret) {
		hwlog_err("memset_s fail\n");
		return ret;
	}

	ret = strncpy_s(nve.nv_name, sizeof(HW_COUL_NV_NAME),
		HW_COUL_NV_NAME, sizeof(HW_COUL_NV_NAME) - 1);
	if (ret) {
		hwlog_err("strncpy_s fail\n");
		return ret;
	}

	nve.nv_number = HW_COUL_NV_NUM;
	nve.valid_size = sizeof(*hw_info);
	if (nve.valid_size > NVE_NV_DATA_SIZE) {
		hwlog_err("struct info is too big for nve\n");
		return -EINVAL;
	}

	di->is_nv_need_save = 1;
	di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);

	if (type == NV_WRITE_TYPE) {
		for (i = 0; i < MAX_TEMPS; i++) {
			hw_info->asw_temp_backup[i] = hi_info->temp[i];
			hw_info->asw_real_fcc_backup[i] = hi_info->real_fcc[i];
		}

		nve.nv_operation = NV_WRITE;
		ret = memcpy_s(nve.nv_data, sizeof(*hw_info), hw_info,
			 sizeof(*hw_info));
		if (ret) {
			hwlog_err("memcpy_s fail\n");
			return ret;
		}

		asw_protect_wake_lock();
		ret = hisi_nve_direct_access(&nve);
		asw_protect_wake_unlock();
		if (ret)
			hwlog_err("write real_fcc nv failed, ret = %d\n", ret);

		for (i = 0; i < MAX_TEMPS; i++) {
			hi_info->temp[i] = 0;
			hi_info->real_fcc[i] = 0;
		}
		di->batt_limit_fcc = 0;
		di->adjusted_fcc_temp_lut = NULL;
	} else {
		nve.nv_operation = NV_READ;
		asw_protect_wake_lock();
		ret = hisi_nve_direct_access(&nve);
		asw_protect_wake_unlock();
		if (ret) {
			hwlog_err("read nv fail\n");
		} else {
			ret = memcpy_s(hw_info, sizeof(*hw_info),
				nve.nv_data, sizeof(*hw_info));
			if (ret) {
				hwlog_err("memcpy_s fail\n");
				return ret;
			}

			for (i = 0; i < MAX_TEMPS; i++) {
				/* resotre nv from backup nv */
				hi_info->temp[i] = hw_info->asw_temp_backup[i];
				hi_info->real_fcc[i] =
					hw_info->asw_real_fcc_backup[i];
			}
			hisi_asw_refresh_fcc();
		}
	}

	return 0;
}

static int asw_protect_save_info_to_nv(struct smartstar_coul_device *di)
{
	int ret;
	struct hisi_nve_info_user nve;
	struct hw_coul_nv_info *hw_info = NULL;

	if (!di) {
		hwlog_info("NULL point\n");
		return -EINVAL;
	}

	hw_info = &di->hw_nv_info;

	ret = memset_s(&nve, sizeof(nve), 0, sizeof(nve));
	if (ret) {
		hwlog_info("memset_s fail\n");
		return ret;
	}
	ret = strncpy_s(nve.nv_name, sizeof(HW_COUL_NV_NAME),
		       HW_COUL_NV_NAME, sizeof(HW_COUL_NV_NAME) - 1);
	if (ret) {
		hwlog_info("strncpy_s fail\n");
		return ret;
	}

	nve.nv_number = HW_COUL_NV_NUM;
	nve.valid_size = sizeof(*hw_info);
	nve.nv_operation = NV_WRITE;

	hw_info->asw_protect_status = di->hw_nv_info.asw_protect_status;
	hw_info->asw_protect_voltage = di->hw_nv_info.asw_protect_voltage;
	hw_info->asw_real_fcc_backup_flag =
		di->hw_nv_info.asw_real_fcc_backup_flag;

	ret = memcpy_s(nve.nv_data, sizeof(*hw_info),
			hw_info, sizeof(*hw_info));
	if (ret) {
		hwlog_info("memcpy_s fail\n");
		return ret;
	}

	/* here save info in register */
	asw_protect_wake_lock();
	ret = hisi_nve_direct_access(&nve);
	asw_protect_wake_unlock();
	if (ret)
		hwlog_err("save asw nv partion failed, ret=%d\n", ret);
	else
		hwlog_info("save asw nv partion success, ret=%d\n", ret);

	return ret;
}

void asw_protect_charge_done(struct smartstar_coul_device *di,
		struct hw_coul_nv_info *batt_backup_nv_info)
{
	int ret;
	int voltage;

	if (!di || !batt_backup_nv_info) {
		hwlog_err("NULL point\n");
		return;
	}

	if (!g_asw_protect_support) {
		hwlog_info("asw support not, do nothing\n");
		return;
	}

	if (di->hw_nv_info.asw_protect_status != ASW_PROTECT_FLAG_DO) {
		hwlog_info("asw status not right, do nothing\n");
		return;
	}

	/* calculate batt_data with asw_protect_voltage */
	ret = asw_calculate_battery_data(di->batt_data, di,
		di->hw_nv_info.asw_protect_voltage);
	if (ret) {
		hwlog_err("asw calculate battery data fail\n");
		return;
	}

	if (di->hw_nv_info.asw_protect_voltage ==
			NORMAL_SCENE_ASW_PROTECT_VOLT) {
		di->hw_nv_info.asw_protect_status = ASW_PROTECT_FLAG_DEFAULT;
		if (asw_hw_coul_fcc_operate(di, batt_backup_nv_info,
			NV_READ_TYPE) == NV_OPERATE_SUCC) {
			hwlog_info("hwcoul real_fcc read nv succ\n");
		} else {
			hwlog_err("hwcoul real_fcc read nv fail\n");
			return;
		}
	} else {
		di->hw_nv_info.asw_protect_status = ASW_PROTECT_FLAG_DONE;
		if (di->hw_nv_info.asw_real_fcc_backup_flag ==
			ASW_REAL_FCC_NEED_BACKUP) {
			if (asw_hw_coul_fcc_operate(di, batt_backup_nv_info,
				NV_WRITE_TYPE) == NV_OPERATE_SUCC) {
				hwlog_info("hwcoul real_fcc write nv succ\n");
			} else {
				hwlog_err("hwcoul real_fcc write nv fail\n");
				return;
			}
		}
	}

	/* save asw protect flag and voltage to hw_coul nv */
	ret = asw_protect_save_info_to_nv(di);
	if (ret) {
		hwlog_err("save asw info to nv fail\n");
		return;
	}

	voltage = hisi_battery_voltage();
	hwlog_info("asw done, status = %d, batt_volt = %d, vbatt_max = %d\n",
		di->hw_nv_info.asw_protect_status, voltage,
		di->batt_data->vbatt_max);
}

void asw_protect_do(struct smartstar_coul_device *di, long value)
{
	if (!di) {
		hwlog_err("di is NULL!\n");
		return;
	}

	if (!g_asw_protect_support) {
		hwlog_info("asw protect support not, do nothing\n");
		return;
	}

	if (strstr(saved_command_line, ANDROID_BOOT_FACTORY)) {
		hwlog_info("factory_version, asw protect do nothing\n");
		return;
	}

	switch (value) {
	case H1_SCENE_ASW_PROTECT_VOLT:
		asw_dsm_report_info(di,
			DSM_BATTERY_ASW_PROTECT_H1_SCENE);
		break;
	case H2_SCENE_ASW_PROTECT_VOLT:
		asw_dsm_report_info(di,
			DSM_BATTERY_ASW_PROTECT_H2_SCENE);
		break;
	case NORMAL_SCENE_ASW_PROTECT_VOLT:
		asw_dsm_report_info(di,
			DSM_BATTERY_ASW_PROTECT_NORMAL_SCENE);
		break;
	default:
		hwlog_err("invalid ASW protect voltage\n");
		return;
	}

	if (di->batt_data->vbatt_max == di->batt_data->vbatt_max_backup)
		di->hw_nv_info.asw_real_fcc_backup_flag =
			ASW_REAL_FCC_NEED_BACKUP;
	else
		di->hw_nv_info.asw_real_fcc_backup_flag =
			ASW_REAL_FCC_NO_NEED_BACKUP;

	di->hw_nv_info.asw_protect_status = ASW_PROTECT_FLAG_START;
	di->hw_nv_info.asw_protect_voltage = value;
	di->batt_data->vbatt_max = di->batt_data->vbatt_max_backup - value;

	hwlog_info("asw do, status = %d, voltage = %d, flag = %d; vbatt_max = %d\n",
		di->hw_nv_info.asw_protect_status,
		di->hw_nv_info.asw_protect_voltage,
		di->hw_nv_info.asw_real_fcc_backup_flag,
		di->batt_data->vbatt_max);
	queue_delayed_work(system_power_efficient_wq,
		&di->asw_protect_do_delayed_work,
		round_jiffies_relative(msecs_to_jiffies(0)));
}

void asw_protect_do_work(struct work_struct *work)
{
	int voltage;
	struct smartstar_coul_device *di = NULL;

	if (!work) {
		hwlog_err("work NULL point\n");
		return;
	}

	di = container_of(work, struct smartstar_coul_device,
		asw_protect_do_delayed_work.work);
	if (!di) {
		hwlog_err("di NULL point\n");
		return;
	}

	if (di->hw_nv_info.asw_protect_status != ASW_PROTECT_FLAG_START) {
		hwlog_info("do nothing\n");
		return;
	}


	voltage = hisi_battery_voltage();
	if (voltage >= (di->batt_data->vbatt_max -
			ASW_PROTECT_DELTA_VOLT)) {
		di->hw_nv_info.asw_protect_status = ASW_PROTECT_FLAG_START;
		if (g_is_nv_need_save) {
			asw_protect_save_info_to_nv(di);
			g_is_nv_need_save = 0;
		}
		asw_set_iin_limit(ASW_PROTECT_IIN_LIMIT);
		queue_delayed_work(system_power_efficient_wq,
			&di->asw_protect_do_delayed_work,
			round_jiffies_relative(msecs_to_jiffies(
			ASW_PROTECT_CHECK_TIME_MS)));
	} else {
		di->hw_nv_info.asw_protect_status = ASW_PROTECT_FLAG_DO;
		asw_protect_save_info_to_nv(di);
		asw_set_iin_limit(ASW_PROTECT_IIN_LIMIT_NO);
	}

	hwlog_info("asw do work, status = %d, iin = %d, bat_vol = %d, vbatt_max = %d\n",
		di->hw_nv_info.asw_protect_status, asw_get_iin_limit(),
		voltage, di->batt_data->vbatt_max);
}

void asw_protect_check(struct smartstar_coul_device *di)
{
	if (!di) {
		hwlog_err("di is NULL!\n");
		return;
	}

	if (strstr(saved_command_line, ANDROID_BOOT_FACTORY)) {
		hwlog_info("factory_version, asw protect no need\n");
		return;
	}

	if (of_property_read_u32(of_find_compatible_node(NULL, NULL,
			"huawei,asw_protect"), "asw_protect_support",
			&g_asw_protect_support))
		hwlog_info("dts: not set asw_protect_support\n");
	hwlog_info("dts asw_protect_support = %d\n", g_asw_protect_support);

	/* if no support asw just return */
	if (!g_asw_protect_support) {
		hwlog_info("asw protect support not, check do nothing\n");
		return;
	}

	wake_lock_init(&asw_lock, WAKE_LOCK_SUSPEND, "asw_wakelock");
	INIT_DELAYED_WORK(&di->asw_protect_do_delayed_work,
		asw_protect_do_work);

	/* asw get protect flag and voltage from command line */
	di->hw_nv_info.asw_protect_status = asw_get_protect_status();
	di->hw_nv_info.asw_protect_voltage = asw_get_protect_voltage();
	di->hw_nv_info.asw_real_fcc_backup_flag = asw_get_protect_backup_flag();

	switch (di->hw_nv_info.asw_protect_status) {
	case ASW_PROTECT_FLAG_DEFAULT:
		hwlog_info("use default batt_data\n");
		break;
	/* asw protect happened , but battery not chargedone yet*/
	case ASW_PROTECT_FLAG_START:
	case ASW_PROTECT_FLAG_DO:
		hwlog_info("asw protect happened, but not chargedone yet\n");
		di->batt_data->vbatt_max = di->batt_data->vbatt_max_backup -
			di->hw_nv_info.asw_protect_voltage;
		queue_delayed_work(system_power_efficient_wq,
			&di->asw_protect_do_delayed_work,
			round_jiffies_relative(msecs_to_jiffies(
			ASW_PROTECT_CHECK_TIME_MS)));
		break;
	case ASW_PROTECT_FLAG_DONE:
		hwlog_info("asw battery chargedone, recalculate batt_data\n");
		asw_calculate_battery_data(di->batt_data,
			di, di->hw_nv_info.asw_protect_voltage);
		break;
	default:
		hwlog_info("asw protect status unvalid!\n");
		break;
	}

	hwlog_info("asw check, status = %d, voltage = %d, flag = %d; vbatt_max = %d\n",
		di->hw_nv_info.asw_protect_status,
		di->hw_nv_info.asw_protect_voltage,
		di->hw_nv_info.asw_real_fcc_backup_flag,
		di->batt_data->vbatt_max);
}

void asw_protect_exit(struct smartstar_coul_device *di)
{
	if (!di) {
		hwlog_err("di is NULL!\n");
		return;
	}

	if (!g_asw_protect_support) {
		hwlog_info("asw protect support not, exit do nothing\n");
		return;
	}

	wake_lock_destroy(&asw_lock);
	cancel_delayed_work(&di->asw_protect_do_delayed_work);
}
