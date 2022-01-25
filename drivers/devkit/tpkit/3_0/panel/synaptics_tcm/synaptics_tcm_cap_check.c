/*
 * Synaptics TCM touchscreen driver
 *
 * Copyright (C) 2017-2018 Synaptics Incorporated. All rights reserved.
 *
 * Copyright (C) 2017-2018 Scott Lin <scott.lin@tw.synaptics.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS," AND SYNAPTICS
 * EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.
 * IN NO EVENT SHALL SYNAPTICS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, PUNITIVE, OR CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED
 * AND BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF SYNAPTICS WAS ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF COMPETENT JURISDICTION DOES
 * NOT PERMIT THE DISCLAIMER OF DIRECT DAMAGES OR ANY OTHER DAMAGES, SYNAPTICS'
 * TOTAL CUMULATIVE LIABILITY TO ANY PARTY SHALL NOT EXCEED ONE HUNDRED U.S.
 * DOLLARS.
 */

#include <linux/gpio.h>
#include <linux/interrupt.h>

#include "synaptics_tcm_core.h"
#include "synaptics_tcm_cap_check.h"

#include "../../huawei_ts_kit_algo.h"
#include "../../tpkit_platform_adapter.h"
#include "../../huawei_ts_kit_api.h"

#if defined (CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif
#if defined (CONFIG_TEE_TUI)
#include "tui.h"
#endif
#include "../../huawei_ts_kit.h"

#define SYSFS_DIR_NAME "testing"
#define SYNA_TCM_LIMITS_CSV_FILE "/odm/etc/firmware/ts/syna_tcm_cap_limits.csv"

#define SYNA_TCM_TEST_BUF_LEN 50
#define RES_TIMEOUT_MS 50
#define REPORT_TIMEOUT_MS 200
#define REPORT_TIMEOUT_MS_1 800

#define READ_REPORT_RETRY_TIMES 3

#define _TEST_PASS_ 1
#define _TEST_FAIL_ 0
struct syna_tcm_test_params *test_params = NULL;

static int syna_tcm_abs(int value)
{
	return value < 0 ? -value : value;
}

//extern char synaptics_raw_data_limit_flag;
extern struct ts_kit_platform_data g_ts_kit_platform_data;

enum test_code {
	TEST_TRX_TRX_SHORTS = 0,
	TEST_TRX_SENSOR_OPENS = 1,
	TEST_TRX_GROUND_SHORTS = 2,
	TEST_FULL_RAW_CAP = 5,
	TEST_DYNAMIC_RANGE = 7,
	TEST_OPEN_SHORT_DETECTOR = 8,
	TEST_NOISE = 10,
	TEST_PT11 = 11,
	TEST_PT12 = 12,
	TEST_PT13 = 13,
	TEST_PT18 = 18, //abs raw cap
	TEST_DISCRETE_PT196 = 196,       /* support discrete production test */
};

/* support discrete production test + */
#define MAX_PINS (64)
#define CFG_IMAGE_TXES_OFFSET (3184)
#define CFG_IMAGE_TXES_LENGTH (256)
#define CFG_IMAGE_RXES_OFFSET (2640)
#define CFG_IMAGE_RXES_LENGTH (544)
#define CFG_NUM_TXGUARD_OFFSET (3568)
#define CFG_NUM_TXGUARD_LENGTH (16)
#define CFG_NUM_RXGUARD_OFFSET (3552)
#define CFG_NUM_RXGUARD_LENGTH (16)
#define CFG_TX_GUARD_PINS_OFFSET (6640)
#define CFG_TX_GUARD_PINS_LENGTH (48)
#define CFG_RX_GUARD_PINS_OFFSET (6576)
#define CFG_RX_GUARD_PINS_LENGTH (64)

#define CFG_IMAGE_CBCS_OFFSET (2032)
#define CFG_IMAGE_CBCS_LENGTH (544)
#define CFG_REF_LO_TRANS_CAP_OFFSET (3664)
#define CFG_REF_LO_TRANS_CAP_LENGTH (16)
#define CFG_REF_LO_XMTR_PL_OFFSET (6560)
#define CFG_REF_LO_XMTR_PL_LENGTH (16)
#define CFG_REF_HI_TRANS_CAP_OFFSET (3648)
#define CFG_REF_HI_TRANS_CAP_LENGTH (16)
#define CFG_REF_HI_XMTR_PL_OFFSET (6528)
#define CFG_REF_HI_XMTR_PL_LENGTH (16)
#define CFG_REF_GAIN_CTRL_OFFSET (3632)
#define CFG_REF_GAIN_CTRL_LENGTH (16)

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
/* support discrete production test - */

struct testing_hcd {
	short *mmi_buf;
	bool result;
	unsigned char report_type;
	unsigned int report_index;
	unsigned int num_of_reports;
	struct kobject *sysfs_dir;
	struct syna_tcm_buffer out;
	struct syna_tcm_buffer resp;
	struct syna_tcm_buffer report;
	struct syna_tcm_buffer process;
	struct syna_tcm_buffer output;
	struct syna_tcm_hcd *tcm_hcd;
	/* support discrete production test + */
	unsigned char *satic_cfg_buf;
	short tx_pins[MAX_PINS];
	short tx_assigned;
	short rx_pins[MAX_PINS];
	short rx_assigned;
	short guard_pins[MAX_PINS];
	short guard_assigned;
	/* support discrete production test - */

};
extern int reflash_after_fw_update_do_reset(void);

static struct testing_hcd *testing_hcd = NULL;

static void syna_tcm_get_thr_from_csvfile(void)
{
	int ret = NO_ERR;
	unsigned int rows = 0;
	unsigned int cols = 0;
	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;
	struct syna_tcm_hcd *tcm_hcd = testing_hcd->tcm_hcd;
	char file_path[MAX_STR_LEN*5] = {0};
	char file_name[MAX_STR_LEN*4] = {0};


	snprintf(file_name, (MAX_STR_LEN*4), "%s_%s_%s_limits.csv",
		tcm_hcd->syna_tcm_chip_data->ts_platform_data->product_name, SYNAPTICS_VENDER_NAME,
		tcm_hcd->tcm_mod_info.project_id_string);

	#ifdef BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE
	snprintf(file_path, sizeof(file_path), "/product/etc/firmware/ts/%s", file_name);
	#else
	snprintf(file_path, sizeof(file_path), "/odm/etc/firmware/ts/%s", file_name);
	#endif

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);

	TS_LOG_INFO("rows: %d, cols: %d\n", rows, cols);
	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_MIN_ARRAY,
			threshold->raw_data_min_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_MIN_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_MAX_ARRAY,
			threshold->raw_data_max_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_MAX_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_OPEN_SHORT_MIN_ARRAY,
			threshold->open_short_min_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_OPEN_SHORT_MIN_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_OPEN_SHORT_MAX_ARRAY,
			threshold->open_short_max_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_OPEN_SHORT_MAX_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_LCD_NOISE_ARRAY,
			threshold->lcd_noise_max_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_LCD_NOISE_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_TRX_DELTA_MAX_ARRAY,
			threshold->raw_data_Trx_delta_max_limits, rows*2, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_TRX_DELTA_MAX_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_COL_MIN_ARRAY,
			threshold->raw_data_col_min_limits, 1, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_COL_MIN_ARRAY);
	}
	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_COL_MAX_ARRAY,
			threshold->raw_data_col_max_limits, 1, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_COL_MAX_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_ROW_MIN_ARRAY,
			threshold->raw_data_row_min_limits, 1, rows);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_COL_MIN_ARRAY);
	}
	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_ROW_MAX_ARRAY,
			threshold->raw_data_row_max_limits, 1, rows);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_COL_MAX_ARRAY);
	}

	test_params->cap_thr_is_parsed = true;
}

static void syna_tcm_get_frame_size_words(unsigned int *size, bool image_only)
{
	unsigned int rows = 0;
	unsigned int cols = 0;
	unsigned int hybrid = 0;
	unsigned int buttons = 0;
	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = testing_hcd->tcm_hcd;

	app_info = &tcm_hcd->app_info;

	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	hybrid = le2_to_uint(app_info->has_hybrid_data);
	buttons = le2_to_uint(app_info->num_of_buttons);

	*size = rows * cols;

	if (!image_only) {
		if (hybrid)
			*size += rows + cols;
		*size += buttons;
	}

	return;
}

static void syna_tcm_put_device_info(struct ts_rawdata_info_new *info)
{
	char buf_fw_ver[CHIP_INFO_LENGTH] = {0};
	struct syna_tcm_hcd *tcm_hcd = testing_hcd->tcm_hcd;

	strncpy(info->deviceinfo, "-syna_tcm-", sizeof(info->deviceinfo));
	syna_tcm_strncat(info->deviceinfo, tcm_hcd->tcm_mod_info.project_id_string,
		sizeof(info->deviceinfo));

	syna_tcm_strncat(info->deviceinfo, "-", sizeof(info->deviceinfo));
	snprintf(buf_fw_ver, CHIP_INFO_LENGTH, "%d", tcm_hcd->packrat_number);
	TS_LOG_INFO("buf_fw_ver = %s", buf_fw_ver);
	syna_tcm_strncat(info->deviceinfo, buf_fw_ver, sizeof(info->deviceinfo));
	syna_tcm_strncat(info->deviceinfo, ";", sizeof(info->deviceinfo));
}

static int syna_tcm_open_short_test(struct ts_rawdata_info_new *info)
{
	int retval = 0;
	unsigned int timeout = 0;
	unsigned int data_length = 0;
	signed short data = 0;
	unsigned int idx = 0;
	unsigned int row = 0;
	unsigned int col = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	unsigned int frame_size_words = 0;
	unsigned char *temp_buf = NULL;
	int open_short_data_max = 0;
	int open_short_data_min = 0;
	int open_short_data_avg = 0;
	int open_short_data_size = 0;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int retry = READ_REPORT_RETRY_TIMES;

	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = testing_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo* pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s called\n", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	open_short_data_size = rows * cols;

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	pts_node->values = kzalloc(open_short_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		retval = -ENOMEM;
		goto exit;
	}

	temp_buf = kzalloc(((rows * cols + MESSAGE_HEADER_SIZE) * 2), GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		retval = -ENOMEM;
		goto exit;
	}

	retval = syna_tcm_alloc_mem(tcm_hcd,
			&testing_hcd->out,
			1);
	if (retval < 0) {
		TS_LOG_ERR(
				"Failed to allocate memory for testing_hcd->out.buf\n");
		goto exit;
	}

	testing_hcd->out.buf[0] = TEST_PT11;
	retval = syna_tcm_write_hdl_message(tcm_hcd,
			CMD_PRODUCTION_TEST,
			testing_hcd->out.buf,
			1,
			&testing_hcd->resp.buf,
			&testing_hcd->resp.buf_size,
			&testing_hcd->resp.data_length,
			NULL,
			0);
	if (retval < 0) {
		TS_LOG_ERR(
				"Failed to write command %s\n",
				STR(CMD_PRODUCTION_TEST));
		goto exit;
	}

	while(retry){
		timeout = REPORT_TIMEOUT_MS_1;
		msleep(timeout);

		retval = syna_tcm_read(tcm_hcd,
				temp_buf,
				((rows * cols + MESSAGE_HEADER_SIZE) * 2));
		if (retval < 0) {
			TS_LOG_ERR("Failed to read raw data\n");
			goto exit;
		}

		if (temp_buf[0] != MESSAGE_MARKER) {
			TS_LOG_ERR("incorrect Header Marker!");
			retval = -EINVAL;
			goto exit;
		}

		if (temp_buf[1] != STATUS_OK) {
			TS_LOG_INFO("status = 0x%02x", temp_buf[1]);
		}else{
			break;
		}
		retry--;
	}

	data_length = temp_buf[2] | temp_buf[3] << 8;
	TS_LOG_INFO("%d\n", data_length);

	syna_tcm_get_frame_size_words(&frame_size_words, false);

	if (frame_size_words != data_length / 2) {
		TS_LOG_ERR("Frame size mismatch\n");
		goto exit;
	}

	idx = 0;
	open_short_data_max = open_short_data_min = le2_to_uint(&temp_buf[MESSAGE_HEADER_SIZE]);
	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (signed short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			pts_node->values[idx] = (int)data;
			open_short_data_max = data > open_short_data_max ? data : open_short_data_max;
			open_short_data_min = data < open_short_data_min ? data : open_short_data_min;
			open_short_data_avg += data;
			TS_LOG_DEBUG("open_short = %d", data);
			idx++;
		}
	}
	if (open_short_data_size)
		open_short_data_avg /= open_short_data_size;

	idx = 0;
	testing_hcd->result = true;
	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (signed short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			TS_LOG_DEBUG("open_short data=%d", data);
			if (data > threshold->open_short_max_limits[idx] ||
					data < threshold->open_short_min_limits[idx]) {
				TS_LOG_ERR("%s overlow_data = %d, row = %d, col = %d , limits [%d, %d]\n", __func__, data, row, col, threshold->open_short_min_limits[idx], threshold->open_short_max_limits[idx]);
				testing_hcd->result = false;
				testresult = CAP_TEST_FAIL_CHAR;
				strncpy(failedreason, "-panel_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
				goto exit;
			}
			idx++;
		}
	}

exit:
	pts_node->size = open_short_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_OpenShort;		
	strncpy(pts_node->test_name, "Open_short", TS_RAWDATA_TEST_NAME_LEN-1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
			"[%d,%d,%d]",
			open_short_data_min, open_short_data_max, open_short_data_avg);
	if (CAP_TEST_FAIL_CHAR == testresult) {
		strncpy(pts_node->tptestfailedreason, failedreason, TS_RAWDATA_FAILED_REASON_LEN-1);
	}
	list_add_tail(&pts_node->node, &info->rawdata_head);
	
	if(temp_buf) {
		kfree(temp_buf);
		temp_buf = NULL;
	}
	TS_LOG_INFO("%s end retval = %d\n", __func__, retval);

	return retval;
}

static int syna_tcm_noise_test(struct ts_rawdata_info_new *info)
{
	int retval = 0;
	unsigned int idx = 0;
	signed short data = 0;
	unsigned int timeout = 0;
	unsigned int data_length = 0;
	unsigned int row = 0;
	unsigned int col = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	unsigned int frame_size_words = 0;
	unsigned char *temp_buf = NULL;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int noise_data_max = 0;
	int noise_data_min = 0;
	int noise_data_avg = 0;
	int noise_data_size = 0;
	int retry = READ_REPORT_RETRY_TIMES;

	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = testing_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo* pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		TS_LOG_ERR("%s Application firmware not running\n", __func__);
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	noise_data_size = rows * cols;

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	pts_node->values = kzalloc(noise_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		goto exit;
	}

	temp_buf = kzalloc(((rows * cols + MESSAGE_HEADER_SIZE) * 2), GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		retval = -ENOMEM;
		goto exit;
	}

	retval = syna_tcm_alloc_mem(tcm_hcd,
			&testing_hcd->out,
			1);
	if (retval < 0) {
		TS_LOG_ERR(
				"Failed to allocate memory for testing_hcd->out.buf\n");
		goto exit;
	}

	testing_hcd->out.buf[0] = TEST_NOISE;

	retval = syna_tcm_write_hdl_message(tcm_hcd,
			CMD_PRODUCTION_TEST,
			testing_hcd->out.buf,
			1,
			&testing_hcd->resp.buf,
			&testing_hcd->resp.buf_size,
			&testing_hcd->resp.data_length,
			NULL,
			0);
	if (retval < 0) {
		TS_LOG_ERR(
				"Failed to write command %s\n",
				STR(CMD_PRODUCTION_TEST));
		goto exit;
	}

	while(retry) {
		timeout = REPORT_TIMEOUT_MS_1;   //REPORT_TIMEOUT_MS
		msleep(timeout);
		retval = syna_tcm_read(tcm_hcd,
				temp_buf,
				((rows * cols + MESSAGE_HEADER_SIZE) * 2));
		if (retval < 0) {
			TS_LOG_ERR("Failed to read raw data\n");
			goto exit;
		}

		if (temp_buf[0] != MESSAGE_MARKER) {
			TS_LOG_ERR("incorrect Header Marker!");
			retval = -EINVAL;
			goto exit;
		}

		if (temp_buf[1] != STATUS_OK) {
			TS_LOG_INFO("status = 0x%02x", temp_buf[1]);
		}else{
			break;
		}
		retry--;
	}

	data_length = temp_buf[2] | temp_buf[3] << 8;
	TS_LOG_INFO(" data_length =%d\n", data_length);
	if (TS_BUS_SPI == tcm_hcd->syna_tcm_chip_data->ts_platform_data->bops->btype) {
		syna_tcm_get_frame_size_words(&frame_size_words, false);
	} else {
		syna_tcm_get_frame_size_words(&frame_size_words, true);
	}

	if (frame_size_words != data_length / 2) {
		TS_LOG_ERR(
				"Frame size mismatch\n");
		retval = -EINVAL;
		goto exit;
	}

	idx = 0;
	noise_data_max = noise_data_min = (int)le2_to_uint(&temp_buf[MESSAGE_HEADER_SIZE]);
	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (signed short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			pts_node->values[idx] = (int)data;
			noise_data_max = data > noise_data_max ? data : noise_data_max;
			noise_data_min = data < noise_data_min ? data : noise_data_min;
			noise_data_avg += data;
			TS_LOG_DEBUG("noise_buf = %d", data);
			idx++;
		}
	}
	if (noise_data_size)
		noise_data_avg /= noise_data_size;
	
	idx = 0;
	testing_hcd->result = true;

	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (signed short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			TS_LOG_DEBUG("noise data=%d", data);
			if (data > threshold->lcd_noise_max_limits[idx]) {
				TS_LOG_ERR("%s overlow_data = %d, limits = %d, row = %d, col = %d\n", __func__, data,threshold->lcd_noise_max_limits[idx], row, col);
				testing_hcd->result = false;
				testresult = CAP_TEST_FAIL_CHAR;
				strncpy(failedreason, "-panel_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
				goto exit;
			}
			idx++;
		}
	}

exit:
	TS_LOG_INFO("%s end retval = %d", __func__, retval);

	pts_node->size = noise_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_Noise;		
	strncpy(pts_node->test_name, "Noise_delta", TS_RAWDATA_TEST_NAME_LEN-1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
			"[%d,%d,%d]",
			noise_data_min, noise_data_max, noise_data_avg);
	if (CAP_TEST_FAIL_CHAR == testresult) {
		strncpy(pts_node->tptestfailedreason, failedreason, TS_RAWDATA_FAILED_REASON_LEN-1);
	}
	list_add_tail(&pts_node->node, &info->rawdata_head);

	if(temp_buf) {
		kfree(temp_buf);
		temp_buf = NULL;
	}
	return retval;
}

static int syna_tcm_dynamic_range_test(struct ts_rawdata_info_new *info)
{
	int retval = 0;
	unsigned int idx = 0;
	signed short data = 0;
	unsigned int timeout = 0;
	unsigned int data_length = 0;
	unsigned int row = 0;
	unsigned int col = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	unsigned int frame_size_words = 0;
	unsigned char *temp_buf = NULL;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int raw_data_size= 0;
	int raw_data_max = 0;
	int raw_data_min = 0;
	int raw_data_avg = 0;
	int retry = READ_REPORT_RETRY_TIMES;

	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = testing_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo* pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	raw_data_size = rows * cols;

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	pts_node->values = kzalloc(raw_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		goto exit;
	}

	temp_buf = kzalloc(((rows * cols + MESSAGE_HEADER_SIZE) * 2), GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		retval = -ENOMEM;
		goto exit;
	}

	retval = syna_tcm_alloc_mem(tcm_hcd,
			&testing_hcd->out,
			1);
	if (retval < 0) {
		TS_LOG_ERR(
				"Failed to allocate memory for testing_hcd->out.buf\n");
		goto exit;
	}

	testing_hcd->out.buf[0] = TEST_DYNAMIC_RANGE;

	retval = syna_tcm_write_hdl_message(tcm_hcd,
			CMD_PRODUCTION_TEST,
			testing_hcd->out.buf,
			1,
			&testing_hcd->resp.buf,
			&testing_hcd->resp.buf_size,
			&testing_hcd->resp.data_length,
			NULL,
			0);
	if (retval < 0) {
		TS_LOG_ERR(
				"Failed to write command %s\n",
				STR(CMD_PRODUCTION_TEST));
		goto exit;
	}

	while(retry) {
		timeout = REPORT_TIMEOUT_MS_1;   //REPORT_TIMEOUT_MS
		msleep(timeout);
		retval = syna_tcm_read(tcm_hcd,
				temp_buf,
				((rows * cols + MESSAGE_HEADER_SIZE) * 2));
		if (retval < 0) {
			TS_LOG_ERR("Failed to read raw data\n");
			goto exit;
		}

		if (temp_buf[0] != MESSAGE_MARKER) {
			TS_LOG_ERR("incorrect Header Marker!");
			retval = -EINVAL;
			goto exit;
		}

		if (temp_buf[1] != STATUS_OK) {
			TS_LOG_INFO("status = 0x%02x", temp_buf[1]);
		}else{
			break;
		}
		retry--;
	}

	data_length = temp_buf[2] | temp_buf[3] << 8;
	TS_LOG_INFO(" data_length =%d\n", data_length);

	syna_tcm_get_frame_size_words(&frame_size_words, false);
	if (frame_size_words != data_length / 2) {
		TS_LOG_ERR(
				"Frame size mismatch\n");
		retval = -EINVAL;
		goto exit;
	}
	idx = 0;
	raw_data_max = raw_data_min = (int)le2_to_uint(&temp_buf[MESSAGE_HEADER_SIZE]);
	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			pts_node->values[idx] = (int)data;
			raw_data_max = data > raw_data_max ? data : raw_data_max;
			raw_data_min = data < raw_data_min ? data : raw_data_min;
			raw_data_avg += data;
			idx++;
		}
	}
	if (raw_data_size)
		raw_data_avg /= raw_data_size;

	idx = 0;
	testing_hcd->result = true;

	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			if (data > threshold->raw_data_max_limits[idx] ||
				data < threshold->raw_data_min_limits[idx]) {
				TS_LOG_ERR("%s overlow_data = %d, row = %d, col = %d, limits [%d,%d]\n", __func__, data, row, col, threshold->raw_data_max_limits[idx], threshold->raw_data_min_limits[idx]);
				testing_hcd->result = false;
				testresult = CAP_TEST_FAIL_CHAR;
				strncpy(failedreason, "-panel_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
				goto exit;
			}
			idx++;
		}
	}

exit:
	TS_LOG_INFO("%s end retval = %d\n", __func__, retval);
	pts_node->size = raw_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_CAPRAWDATA;
	strncpy(pts_node->test_name, "Cap_Rawdata", TS_RAWDATA_TEST_NAME_LEN-1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
			"[%d,%d,%d]",
			raw_data_min, raw_data_max, raw_data_avg);
	if (CAP_TEST_FAIL_CHAR == testresult) {
		strncpy(pts_node->tptestfailedreason, failedreason, TS_RAWDATA_FAILED_REASON_LEN-1);
	}
	list_add_tail(&pts_node->node, &info->rawdata_head);

	testing_hcd->report_type = 0;
	if(temp_buf) {
		kfree(temp_buf);
		temp_buf = NULL;
	}

	return retval;
}

static int syna_tcm_delta_test(struct ts_rawdata_info_new *info, int* rawdata)
{
	int retval = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int raw_data_size= 0;
	int delta_data_max = 0;
	int delta_data_min = 0;
	int delta_data_avg = 0;
	int i = 0;
	int j = 0;
	int delta_data_size = 0;
	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = testing_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo* pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		TS_LOG_ERR("%s Application firmware not running\n", __func__);
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	raw_data_size = rows * cols;

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	pts_node->values = kzalloc(raw_data_size*sizeof(int) * 2, GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed	for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		goto exit;
	}
	memset(pts_node->values, 0 ,raw_data_size*sizeof(int) * 2);
	testing_hcd->result = true;
	delta_data_max = delta_data_min = syna_tcm_abs(rawdata[0]-rawdata[1]);

	for(i = 0, j=0; i < rows * cols ; i++) {
		if (i < cols) {
			/* nothing to do*/
		} else {
			pts_node->values[j] =
				syna_tcm_abs(rawdata[i] -
				rawdata[i - cols]);
			delta_data_max = delta_data_max < pts_node->values[j] ? pts_node->values[j] : delta_data_max;
			delta_data_min = delta_data_min > pts_node->values[j] ? pts_node->values[j] : delta_data_min;
			delta_data_avg += pts_node->values[j];
			j++;
		}
	}
	delta_data_size += j;

	for(i = 0, j=0 ; i < rows * cols ; i++) {
		if (i%cols == 0) {
			/* nothing to do*/
		} else {
			pts_node->values[j + raw_data_size] = syna_tcm_abs(rawdata[i]
				-rawdata[i - 1]);
			delta_data_max = delta_data_max < pts_node->values[j] ? pts_node->values[j] : delta_data_max;
			delta_data_min = delta_data_min > pts_node->values[j] ? pts_node->values[j] : delta_data_min;
			delta_data_avg += pts_node->values[j];
			j++;
		}
	}
	delta_data_size += j;

	TS_LOG_ERR("delta_data_size = %d\n", delta_data_size);

	if(delta_data_size){
		delta_data_avg /= delta_data_size;
	}

	for (i = 0; i < raw_data_size*2; i++) {
		TS_LOG_DEBUG("delta value is[%d, %d] %d > %d\n", i/cols, i%cols,
				pts_node->values[i],
				threshold->raw_data_Trx_delta_max_limits[i]);
		if (pts_node->values[i] > threshold->raw_data_Trx_delta_max_limits[i]) {
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-panel_reason",TS_RAWDATA_FAILED_REASON_LEN-1);
			TS_LOG_ERR("delta value is[%d, %d] %d > %d\n", i/cols, i%cols,
				pts_node->values[i],
				threshold->raw_data_Trx_delta_max_limits[i]);
		}
	}

exit:
	TS_LOG_INFO("%s end retval = %d\n", __func__, retval);
	pts_node->size = raw_data_size*2;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_TrxDelta;
	strncpy(pts_node->test_name, "delta_Rawdata", TS_RAWDATA_TEST_NAME_LEN-1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
			"[%d,%d,%d]",
			delta_data_min, delta_data_max, delta_data_avg);
	if (CAP_TEST_FAIL_CHAR == testresult) {
		strncpy(pts_node->tptestfailedreason, failedreason, TS_RAWDATA_FAILED_REASON_LEN-1);
	}
	list_add_tail(&pts_node->node, &info->rawdata_head);
	info->listnodenum++;

	testing_hcd->report_type = 0;

	return retval;
}

static int syna_tcm_full_raw_cap_test(struct ts_rawdata_info_new *info)
{
	int retval = 0;
	unsigned int idx = 0;
	signed short data = 0;
	unsigned int timeout = 0;
	unsigned int data_length = 0;
	unsigned int row = 0;
	unsigned int col = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	unsigned int frame_size_words = 0;
	unsigned char *temp_buf = NULL;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int raw_data_size= 0;
	int raw_data_max = 0;
	int raw_data_min = 0;
	int raw_data_avg = 0;
	int retry = READ_REPORT_RETRY_TIMES;

	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = testing_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo* pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		TS_LOG_ERR("%s Application firmware not running\n", __func__);
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	raw_data_size = rows * cols;

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	pts_node->values = kzalloc(raw_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		goto exit;
	}

	temp_buf = kzalloc(((rows * cols + MESSAGE_HEADER_SIZE) * 2), GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		retval = -ENOMEM;
		goto exit;
	}

	retval = syna_tcm_alloc_mem(tcm_hcd,
			&testing_hcd->out,
			1);
	if (retval < 0) {
		TS_LOG_ERR("Failed to allocate memory for testing_hcd->out.buf\n");
		goto exit;
	}

	testing_hcd->out.buf[0] = TEST_FULL_RAW_CAP;

	retval = syna_tcm_write_hdl_message(tcm_hcd,
			CMD_PRODUCTION_TEST,
			testing_hcd->out.buf,
			1,
			&testing_hcd->resp.buf,
			&testing_hcd->resp.buf_size,
			&testing_hcd->resp.data_length,
			NULL,
			0);
	if (retval < 0) {
		TS_LOG_ERR("Failed to write command %s\n", STR(CMD_PRODUCTION_TEST));
		goto exit;
	}

	while(retry) {
		timeout = REPORT_TIMEOUT_MS_1;   //REPORT_TIMEOUT_MS
		msleep(timeout);
		retval = syna_tcm_read(tcm_hcd,
				temp_buf,
				((rows * cols + MESSAGE_HEADER_SIZE) * 2));
		if (retval < 0) {
			TS_LOG_ERR("Failed to read raw data\n");
			goto exit;
		}

		if (temp_buf[0] != MESSAGE_MARKER) {
			TS_LOG_ERR("incorrect Header Marker!");
			retval = -EINVAL;
			goto exit;
		}

		if (temp_buf[1] != STATUS_OK) {
			TS_LOG_INFO("status = 0x%02x", temp_buf[1]);
		}else{
			break;
		}
		retry--;
	}

	data_length = temp_buf[2] | temp_buf[3] << 8;
	TS_LOG_INFO(" data_length =%d\n", data_length);

	syna_tcm_get_frame_size_words(&frame_size_words, true);
	if (frame_size_words != data_length / 2) {
		TS_LOG_ERR("Frame size mismatch\n");
		retval = -EINVAL;
		goto exit;
	}
	idx = 0;
	raw_data_max = raw_data_min = (int)le2_to_uint(&temp_buf[MESSAGE_HEADER_SIZE]);
	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			pts_node->values[idx] = (int)data;
			raw_data_max = data > raw_data_max ? data : raw_data_max;
			raw_data_min = data < raw_data_min ? data : raw_data_min;
			raw_data_avg += data;
			idx++;
		}
	}
	if (raw_data_size)
		raw_data_avg /= raw_data_size;

	idx = 0;
	testing_hcd->result = true;

	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			if (data > threshold->raw_data_max_limits[idx] ||
				data < threshold->raw_data_min_limits[idx]) {
				TS_LOG_ERR("%s overlow_data = %d, row = %d, col = %d, limits [%d,%d]\n", __func__, data, row, col, threshold->raw_data_max_limits[idx], threshold->raw_data_min_limits[idx]);
				testing_hcd->result = false;
				testresult = CAP_TEST_FAIL_CHAR;
				strncpy(failedreason, "-panel_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
			}
			idx++;
		}
	}
	retval = syna_tcm_delta_test(info, pts_node->values);
	if(retval) {
		TS_LOG_INFO("%s syna_tcm_delta_test FAILED = %d\n", __func__, retval);
	}
exit:
	TS_LOG_INFO("%s end retval = %d\n", __func__, retval);
	pts_node->size = raw_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_CAPRAWDATA;
	strncpy(pts_node->test_name, "Cap_Rawdata", TS_RAWDATA_TEST_NAME_LEN-1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
			"[%d,%d,%d]",
			raw_data_min, raw_data_max, raw_data_avg);
	if (CAP_TEST_FAIL_CHAR == testresult) {
		strncpy(pts_node->tptestfailedreason, failedreason, TS_RAWDATA_FAILED_REASON_LEN-1);
	}
	list_add_tail(&pts_node->node, &info->rawdata_head);

	testing_hcd->report_type = 0;
	if(temp_buf) {
		kfree(temp_buf);
		temp_buf = NULL;
	}
	return retval;
}

static int syna_tcm_abs_raw_cap_test(struct ts_rawdata_info_new *info)
{
	int retval = 0;
	unsigned int idx = 0;
	unsigned int timeout = 0;
	unsigned int data_length = 0;
	unsigned int row = 0;
	unsigned int col = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	unsigned char *temp_buf = NULL;
	int *temp_buf_row_int_p= NULL;
	int *temp_buf_col_int_p = NULL;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int noise_data_max = 0;
	int noise_data_min = 0;
	int noise_data_avg = 0;
	int abs_raw_data_size = 0;
	int retry = READ_REPORT_RETRY_TIMES;

	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = testing_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo* pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		TS_LOG_ERR("%s Application firmware not running\n", __func__);
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	abs_raw_data_size = rows + cols;

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	pts_node->values = kzalloc(abs_raw_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		goto exit;
	}
	temp_buf_row_int_p = kzalloc(rows*sizeof(int), GFP_KERNEL);
	if (!temp_buf_row_int_p) {
		TS_LOG_ERR("%s malloc temp_buf_row_int_p failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		retval = -ENOMEM;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		goto exit;
	}
	temp_buf_col_int_p = kzalloc(cols*sizeof(int), GFP_KERNEL);
	if (!temp_buf_col_int_p) {
		TS_LOG_ERR("%s malloc temp_buf_col_int_p failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		retval = -ENOMEM;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		goto exit;
	}

	temp_buf = kzalloc(((rows + cols) * 4 + MESSAGE_HEADER_SIZE + 1), GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		retval = -ENOMEM;
		goto exit;
	}

	retval = syna_tcm_alloc_mem(tcm_hcd,
			&testing_hcd->out,
			1);
	if (retval < 0) {
		TS_LOG_ERR("Failed to allocate memory for testing_hcd->out.buf\n");
		goto exit;
	}

	testing_hcd->out.buf[0] = TEST_PT18;

	retval = syna_tcm_write_hdl_message(tcm_hcd,
			CMD_PRODUCTION_TEST,
			testing_hcd->out.buf,
			1,
			&testing_hcd->resp.buf,
			&testing_hcd->resp.buf_size,
			&testing_hcd->resp.data_length,
			NULL,
			0);
	if (retval < 0) {
		TS_LOG_ERR(
				"Failed to write command %s\n",
				STR(CMD_PRODUCTION_TEST));
		goto exit;
	}

	while(retry) {
		timeout = REPORT_TIMEOUT_MS_1;   //REPORT_TIMEOUT_MS
		msleep(timeout);
		retval = syna_tcm_read(tcm_hcd,
				temp_buf,
				((rows + cols) * 4 + MESSAGE_HEADER_SIZE + 1));
		if (retval < 0) {
			TS_LOG_ERR("Failed to read raw data\n");
			goto exit;
		}

		if (temp_buf[0] != MESSAGE_MARKER) {
			TS_LOG_ERR("incorrect Header Marker!");
			retval = -EINVAL;
			goto exit;
		}

		if (temp_buf[1] != STATUS_OK) {
			TS_LOG_INFO("status = 0x%02x", temp_buf[1]);
		}else{
			break;
		}
		retry--;
	}

	data_length = temp_buf[2] | temp_buf[3] << 8;
	TS_LOG_INFO(" data_length =%d\n", data_length);

	if ((temp_buf[1] != STATUS_OK) || (data_length != ((rows + cols) * 4))) {
		TS_LOG_ERR("not correct status or data_length status:%x  length:%x\n", temp_buf[1], data_length);
		retval = -EINVAL;
		goto exit;
	}

	idx = MESSAGE_HEADER_SIZE;
	testing_hcd->result = true;

	for (col = 0; col < cols; col++) {
		temp_buf_col_int_p[col] = (unsigned int)(temp_buf[idx] & 0xff) | (unsigned int)(temp_buf[idx+1] << 8) |
					(unsigned int)(temp_buf[idx+2] << 16) | (unsigned int)(temp_buf[idx+3] << 24);
			TS_LOG_DEBUG("col=%-2d data = %d , limit = [%d, %d]\n",
					col, temp_buf_col_int_p[col],
					threshold->raw_data_col_min_limits[col],
					threshold->raw_data_col_max_limits[col]);
		if (temp_buf_col_int_p[col] < threshold->raw_data_col_min_limits[col]
				|| temp_buf_col_int_p[col] > threshold->raw_data_col_max_limits[col]) {
			testing_hcd->result = false;
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-panel_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
			retval = -EINVAL;
			TS_LOG_ERR("fail at col=%-2d data = %d (0x%2x 0x%2x 0x%2x 0x%2x), limit = [%d,%d]\n",
					col, temp_buf_col_int_p[col],
					temp_buf[idx], temp_buf[idx+1], temp_buf[idx+2], temp_buf[idx+3],
					threshold->raw_data_col_min_limits[col],
					threshold->raw_data_col_max_limits[col]);
		}
		idx+=4;
	}

	for (row = 0; row < rows; row++) {
		temp_buf_row_int_p[row] = (unsigned int)(temp_buf[idx] & 0xff) | (unsigned int)(temp_buf[idx+1] << 8) |
					(unsigned int)(temp_buf[idx+2] << 16) | (unsigned int)(temp_buf[idx+3] << 24);
			TS_LOG_DEBUG("row=%-2d data = %d, limit = [%d, %d]\n",
					row, temp_buf_row_int_p[row],
					threshold->raw_data_row_min_limits[row],
					threshold->raw_data_row_max_limits[row]);
		if (temp_buf_row_int_p[row] < threshold->raw_data_row_min_limits[row]
				|| temp_buf_row_int_p[row] > threshold->raw_data_row_max_limits[row]) {
			testing_hcd->result = false;
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-panel_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
			retval = -EINVAL;
			TS_LOG_ERR("fail at row=%-2d data = %d (0x%2x 0x%2x 0x%2x 0x%2x), limit = [%d, %d]\n",
					row, temp_buf_row_int_p[row],
					temp_buf[idx], temp_buf[idx+1], temp_buf[idx+2], temp_buf[idx+3],
					threshold->raw_data_row_min_limits[row],
					threshold->raw_data_row_max_limits[row]);
		}
		idx+=4;
	}

	memcpy(pts_node->values, temp_buf_row_int_p, rows*sizeof(int));
	memcpy(&(pts_node->values[rows]), temp_buf_col_int_p, cols*sizeof(int));

exit:
	TS_LOG_INFO("%s end retval = %d", __func__, retval);

	pts_node->size = abs_raw_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_SelfCap;
	strncpy(pts_node->test_name, "abs raw data", TS_RAWDATA_TEST_NAME_LEN-1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
			"[%d,%d,%d]",
			noise_data_min, noise_data_max, noise_data_avg);
	if (CAP_TEST_FAIL_CHAR == testresult) {
		strncpy(pts_node->tptestfailedreason, failedreason, TS_RAWDATA_FAILED_REASON_LEN-1);
	}
	list_add_tail(&pts_node->node, &info->rawdata_head);

	if(temp_buf) {
		kfree(temp_buf);
		temp_buf = NULL;
	}
	if(temp_buf_row_int_p) {
		kfree(temp_buf_row_int_p);
		temp_buf_row_int_p = NULL;
	}
	if(temp_buf_col_int_p) {
		kfree(temp_buf_col_int_p);
		temp_buf_col_int_p = NULL;
	}
	return retval;
}

int syna_tcm_cap_test(struct ts_rawdata_info_new *info,
			struct ts_cmd_node *out_cmd)
{
	int retval = NO_ERR;
	unsigned int rows = 0;
	unsigned int cols = 0;
	struct syna_tcm_hcd *tcm_hcd = testing_hcd->tcm_hcd;
	struct syna_tcm_app_info *app_info = NULL;
	struct ts_rawdata_newnodeinfo* pts_node = NULL;

	TS_LOG_INFO("%s called\n", __func__);

	testing_hcd->tcm_hcd = tcm_hcd;
	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	if(0 == rows || 0 == cols) {
		TS_LOG_ERR("rows or cols is NULL\n");
		return -ENOMEM;
	}
	if (!test_params->cap_thr_is_parsed)
		syna_tcm_get_thr_from_csvfile();

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc failed\n");
		return -ENOMEM;
	}

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		pts_node->typeindex = RAW_DATA_TYPE_IC;	
		pts_node->testresult = CAP_TEST_FAIL_CHAR;
		list_add_tail(&pts_node->node, &info->rawdata_head);
		return -ENODEV;
	} else {
		pts_node->typeindex = RAW_DATA_TYPE_IC;	
		pts_node->testresult = CAP_TEST_PASS_CHAR;
		list_add_tail(&pts_node->node, &info->rawdata_head);
	}

	syna_tcm_put_device_info(info);

	syna_tcm_enable_touch(tcm_hcd, false);

	TS_LOG_INFO("%s: before disable doze\n", __func__);
	retval = testing_hcd->tcm_hcd->set_dynamic_config(testing_hcd->tcm_hcd,
			DC_NO_DOZE, 1);
	if (retval < 0) {
		TS_LOG_ERR("%s failed to set dc\n", __func__);
	}
	retval = syna_tcm_noise_test(info);

	if (retval < 0) {
		TS_LOG_ERR("%s lcd noise testing failed.\n", __func__);
	}
	info->listnodenum++;
	if (TS_BUS_SPI == tcm_hcd->syna_tcm_chip_data->ts_platform_data->bops->btype) {
		retval = syna_tcm_dynamic_range_test(info);
	} else {
		retval = syna_tcm_full_raw_cap_test(info);
	}
	if (retval < 0) {
		TS_LOG_ERR("%s raw data testing failed.\n", __func__);
	}
	info->listnodenum++;
	if (TS_BUS_SPI == tcm_hcd->syna_tcm_chip_data->ts_platform_data->bops->btype) {
		/* do nothing */
	} else {
		retval = syna_tcm_abs_raw_cap_test(info);
		if (retval < 0) {
			TS_LOG_ERR("%s raw data testing failed.\n", __func__);
		}
		info->listnodenum++;
	}

	if (TS_BUS_SPI == tcm_hcd->syna_tcm_chip_data->ts_platform_data->bops->btype) {
		retval = syna_tcm_open_short_test(info);
		if (retval < 0) {
			TS_LOG_ERR("%s open short testing failed.\n", __func__);
		}
	} else {
		/* retval = testing_discrete_ex_trx_short(info); */
		/* if (retval < 0) {
			TS_LOG_ERR("%s fail to do testing_discrete_ex_trx_short", __func__);
		} */
	}
	info->listnodenum++;

	retval = testing_hcd->tcm_hcd->set_dynamic_config(testing_hcd->tcm_hcd,
			DC_NO_DOZE, 0);
	if (retval < 0) {
		TS_LOG_ERR("%s failed to set dc\n", __func__);
	}
	TS_LOG_INFO("%s: after enable doze\n", __func__);

	syna_tcm_enable_touch(tcm_hcd, true);
	
	info->tx = cols;
	info->rx = rows;

	if (TS_BUS_I2C == tcm_hcd->syna_tcm_chip_data->ts_platform_data->bops->btype) {
		gpio_set_value(tcm_hcd->syna_tcm_chip_data->ts_platform_data->reset_gpio, 0);
		msleep(10);
		gpio_set_value(tcm_hcd->syna_tcm_chip_data->ts_platform_data->reset_gpio, 1);
	}

	return 0;
}

int syna_tcm_cap_test_init(struct syna_tcm_hcd *tcm_hcd)
{
	int retval = 0;

	if (!testing_hcd) {
		testing_hcd = kzalloc(sizeof(*testing_hcd), GFP_KERNEL);
		if (!testing_hcd) {
			TS_LOG_ERR(
					"Failed to allocate memory for testing_hcd\n");
			return -ENOMEM;
		}
	}

	if (!test_params) {
		test_params = vmalloc(sizeof(struct syna_tcm_test_params));
		if (!test_params) {
			TS_LOG_ERR("%s:alloc mem for params fail\n", __func__);
			retval = -ENOMEM;
			goto err_alloc_testing_hcd;
		}
		memset(test_params, 0, sizeof(struct syna_tcm_test_params));
	}

	testing_hcd->tcm_hcd = tcm_hcd;

	return 0;

err_alloc_testing_hcd:
	if (testing_hcd) {
		kfree(testing_hcd);
		testing_hcd = NULL;
	}

	return retval;
}


int syna_tcm_cap_test_remove(struct syna_tcm_hcd *tcm_hcd)
{
	if (testing_hcd) {
		if (testing_hcd->satic_cfg_buf) {
			kfree(testing_hcd->satic_cfg_buf);
			testing_hcd->satic_cfg_buf = NULL;
		}

		kfree(testing_hcd);
		testing_hcd = NULL;
	}

	return 0;
}

MODULE_AUTHOR("Synaptics, Inc.");
MODULE_DESCRIPTION("Synaptics TCM Testing Module");
MODULE_LICENSE("GPL v2");
