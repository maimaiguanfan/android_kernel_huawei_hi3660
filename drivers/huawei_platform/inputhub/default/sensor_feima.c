#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <asm/memory.h>
#include <linux/time64.h>
#include <asm/io.h>
#include <asm/memory.h>
#include <asm/cacheflush.h>
#include <huawei_platform/inputhub/sensor_feima_ext.h>
#include <linux/semaphore.h>
#include <linux/freezer.h>
#include <linux/of.h>
#include "contexthub_route.h"
#include "contexthub_boot.h"
#include "sensor_config.h"
#include "sensor_feima.h"
#include "sensor_sysfs.h"
#include "sensor_detect.h"

#define MIN_CAP_PROX_MODE 0
#define MAZ_CAP_PROX_MODE 2
#define ALS_UNDER_TP_RAWDATA_LEN 4
#define ALS_UNDER_TP_RGB_DATA_LEN 16
#define ELE_PHONE_TYPE 47
#define VOGUE_PHONE_TYPE 48
#define HALF_LENGTH 45
#define MAX_ALS_UD_CMD_BUF_ARGC (2)
#define ALS_UNDER_TP_RGB_DATA 4

struct class *sensors_class;
int sleeve_test_enabled = 0;
uint64_t als_rgb_pa_to_sh;

static als_ud_cmd_map_t als_ud_cmd_map[] = {
	{"ADDR", ALS_UD_CMD_SET_ADDR},
	{"BUFF", ALS_UD_CMD_BUFFER_UPDATE},
};

static bool rpc_motion_request;
static time_t get_data_last_time;
static unsigned long sar_service_info = 0;
static int als_under_tp_cal_rawdata[ALS_UNDER_TP_RAWDATA_LEN] = {0};
static int als_under_tp_rgb_data[ALS_UNDER_TP_RGB_DATA_LEN] = {0};
extern u8 phone_color;
extern struct sleeve_detect_pare sleeve_detect_paremeter[MAX_PHONE_COLOR_NUM];
extern volatile int vibrator_shake;
extern volatile int hall_value;
extern BH1745_ALS_PARA_TABLE als_para_diff_tp_color_table[];
extern APDS9251_ALS_PARA_TABLE apds_als_para_diff_tp_color_table[];
extern TMD2745_ALS_PARA_TABLE tmd2745_als_para_diff_tp_color_table[];
extern TMD3725_ALS_PARA_TABLE tmd3725_als_para_diff_tp_color_table[];
extern LTR582_ALS_PARA_TABLE ltr582_als_para_diff_tp_color_table[];
extern APDS9999_ALS_PARA_TABLE apds9999_als_para_diff_tp_color_table[];
extern TMD3702_ALS_PARA_TABLE tmd3702_als_para_diff_tp_color_table[];
extern TCS3701_ALS_PARA_TABLE tcs3701_als_para_diff_tp_color_table[];
extern VCNL36658_ALS_PARA_TABLE vcnl36658_als_para_diff_tp_color_table[];
static RET_TYPE airpress_calibration_res = RET_INIT;	/*airpress  calibrate result*/
extern int rohm_rgb_flag;
extern int avago_rgb_flag;
extern int tmd2745_flag;
extern int  ams_tmd3725_rgb_flag;
extern int liteon_ltr582_rgb_flag;
extern int als_para_table;
extern int apds9999_rgb_flag;
extern int  ams_tmd3702_rgb_flag;
extern int  apds9253_rgb_flag;
extern int ams_tcs3701_rgb_flag;
extern int vishay_vcnl36658_als_flag;
extern uint8_t gyro_position;
extern struct als_platform_data als_data;
extern bool fingersense_enabled;
extern struct sensorlist_info sensorlist_info[SENSOR_MAX];
extern char sensor_chip_info[SENSOR_MAX][MAX_CHIP_INFO_LEN];
extern struct sar_sensor_detect semtech_sar_detect;
extern struct sar_sensor_detect semtech_9335_sar_detect;
extern struct sar_sensor_detect adi_sar_detect;
extern struct sar_sensor_detect cypress_sar_detect;
extern struct sar_sensor_detect g_abov_sar_detect;
extern bool fingersense_data_ready;
extern s16 fingersense_data[FINGERSENSE_DATA_NSAMPLES];
extern struct compass_platform_data mag_data;
extern int gsensor_offset[ACC_CALIBRATE_DATA_LENGTH];	/*g-sensor calibrate data*/
extern int gsensor1_offset[ACC1_OFFSET_DATA_LENGTH];
extern int gyro_sensor_offset[GYRO_CALIBRATE_DATA_LENGTH];
extern int gyro1_sensor_offset[GYRO1_CALIBRATE_DATA_LENGTH];
extern int32_t ps_sensor_offset[PS_CALIBRATE_DATA_LENGTH];
extern uint16_t als_offset[ALS_CALIBRATE_DATA_LENGTH];
extern int mag_threshold_for_als_calibrate;
extern int ps_support_abs_threshold;
extern struct airpress_touch_calibrate_data pressure_touch_calibrate_data;
extern struct airpress_platform_data airpress_data;
extern struct als_under_tp_calidata als_under_tp_cal_data;
extern int hall_sen_type;
extern void create_debug_files(void);
extern bool str_fuzzy_match(const char *cmd_buf, const char *target);
extern const char *get_str_begin(const char *cmd_buf);
extern const char *get_str_end(const char *cmd_buf);
extern bool get_arg(const char *str, int *arg);
extern als_run_stop_para_t als_ud_data_upload;
extern int als_ud_rgbl_block;

extern uint8_t sem_als_ud_rgbl_block_flag;

static bool camera_set_rpc_flag = false;
static DECLARE_WAIT_QUEUE_HEAD(sensorhub_als_block_waitq);
static uint8_t g_stepcountdocm;

static int rpc_commu(unsigned int cmd, unsigned int pare, uint16_t motion)
{
	int ret = -1;
	write_info_t pkg_ap;
	rpc_ioctl_t pkg_ioctl;
	memset(&pkg_ap, 0, sizeof(pkg_ap));

	pkg_ap.tag = TAG_RPC;
	pkg_ap.cmd = cmd;
	pkg_ioctl.sub_cmd = pare;
	pkg_ioctl.sar_info = motion;
	pkg_ap.wr_buf = &pkg_ioctl;
	pkg_ap.wr_len = sizeof(pkg_ioctl);

	ret = write_customize_cmd(&pkg_ap, NULL, true);
	if (ret) {
		hwlog_err("send rpc cmd(%d) to mcu fail,ret=%d\n", cmd, ret);
		return ret;
	}
	return ret;
}

static int rpc_motion(uint16_t motion)
{
	unsigned int cmd = 0;
	unsigned int sub_cmd = 0;
	unsigned int ret = -1;
	if (rpc_motion_request) {
		cmd = CMD_CMN_CONFIG_REQ;
		sub_cmd = SUB_CMD_RPC_START_REQ;
		ret = rpc_commu(cmd, sub_cmd, motion);
		if (ret) {
			hwlog_err("%s: rpc motion enable fail\n", __FUNCTION__);
			return ret;
		}
		hwlog_info("%s: rpc motion start succsess\n", __FUNCTION__);
	} else {
		cmd = CMD_CMN_CONFIG_REQ;
		sub_cmd = SUB_CMD_RPC_STOP_REQ;
		ret = rpc_commu(cmd,sub_cmd, motion);
		if (ret) {
			hwlog_info("%s: rpc motion close fail\n", __FUNCTION__);
			return ret;
		}
		hwlog_info("%s: rpc motion stop succsess\n", __FUNCTION__);
	}

	return ret;
}

#define CHECK_SENSOR_COOKIE(data) \
do {\
	if (NULL == data || (!(TAG_SENSOR_BEGIN <= data->tag && data->tag < TAG_SENSOR_END)) || (NULL == data->name)) {\
		hwlog_err("error in %s\n", __func__);\
		return -EINVAL;\
	} \
} while (0)

static ssize_t show_enable(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);
	return snprintf(buf, MAX_STR_SIZE, "%d\n", sensor_status.status[data->tag]);
}

static ssize_t store_enable(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret = -1;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	const char *operation = NULL;

	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);

	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

	if (ap_sensor_enable(data->tag, (1 == val)))
		return size;

	operation = ((1 == val) ? "enable" : "disable");
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	pkg_ap.tag = data->tag;
	pkg_ap.cmd = (1 == val) ? CMD_CMN_OPEN_REQ : CMD_CMN_CLOSE_REQ;
	pkg_ap.wr_buf = NULL;
	pkg_ap.wr_len = 0;
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret != 0) {
		hwlog_err("%s %s failed, ret = %d in %s\n", operation, data->name, ret, __func__);
		return size;
	}

	if (pkg_mcu.errno != 0) {
		hwlog_err("%s %s failed errno = %d in %s\n", operation, data->name, pkg_mcu.errno, __func__);
	} else {
		hwlog_info("%s %s success\n", operation, data->name);
	}

	return size;
}
static int rpc_status_change(void)
{
	int ret= 0;
        sar_service_info = (sar_service_info&~BIT(9)) | ((bool)rpc_motion_request<<9);
	hwlog_info("sar_service_info is %lu\n", sar_service_info);
	ret = rpc_motion(sar_service_info);
	if (ret) {
		hwlog_err("rpc motion fail: %d \n", ret);
		return ret;
	}
	return ret;
}

/*
*add for Txx front&wide camera radio frequency interference
*status:1--enable camera
*status:0--disable camera
*/
int rpc_status_change_for_camera(unsigned int status)
{
	int ret = 0;
	if (status == 1) {
		sar_service_info = sar_service_info | 0x400;//set bit10
		camera_set_rpc_flag = true;
	} else if (status == 0) {
		sar_service_info = sar_service_info & 0xFBFF;//release bit10
		camera_set_rpc_flag = false;
	} else {
		camera_set_rpc_flag = false;
		hwlog_err("error status \n");
		return ret;
	}

	hwlog_info("status %d, sar_service_info is %lu\n", status, sar_service_info);
	ret = rpc_status_change();
	if (ret) {
		hwlog_err("rpc status change fail: %d \n", ret);
		return ret;
	}

	return ret;
}

EXPORT_SYMBOL_GPL(rpc_status_change_for_camera);

static ssize_t store_rpc_motion_req(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long value = 0;
	if (strict_strtoul(buf, 10, &value)) {
		hwlog_err("%s: rpc motion request val(%lu) invalid", __FUNCTION__, value);
	}

	hwlog_info("%s: rpc motion request val (%lu)\n", __FUNCTION__, value);
	if ((value != 0) && (value != 1)) {
		hwlog_err("%s: set enable fail, invalid val\n", __FUNCTION__);
		return size;
	}
	rpc_motion_request = value;
	rpc_status_change();
	return size;

}
static ssize_t store_rpc_sar_service_req(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long sar_service = 0;

	if (strict_strtoul(buf, 10, &sar_service))
	{
	    hwlog_err("rpc_sar_service_req strout error.\n");
	}
	if (sar_service > 65535) {
		hwlog_err("%s: set enable fail, invalid val\n", __FUNCTION__);
		return size;
	}
	hwlog_info("%s: rpc sar service request val (%lu), buf is %s.\n", __FUNCTION__, sar_service, buf);
	if (camera_set_rpc_flag) {
		sar_service = sar_service | 0x400;//camera set bit10
		hwlog_info("%s: camera_set_rpc_flag, rpc sar service val (%lu).\n", __FUNCTION__, sar_service);
	}
	sar_service_info = sar_service;
	rpc_status_change();
	return size;
}
static ssize_t show_rpc_motion_req(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", rpc_motion_request);
}
static ssize_t show_set_delay(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);
	return snprintf(buf, MAX_STR_SIZE, "%d\n", sensor_status.delay[data->tag]);
}

static ssize_t store_set_delay(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret = 0;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	pkt_cmn_interval_req_t cpkt;
	pkt_header_t *hd = (pkt_header_t *)&cpkt;

	struct sensor_cookie *data =
	    (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	memset(&cpkt, 0, sizeof(cpkt));
	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

	if (ap_sensor_setdelay(data->tag, val))
		return size;

	if (val >= 10 && val <= 1000) {	/*range [10, 1000]*/
		pkg_ap.tag = data->tag;
		pkg_ap.cmd = CMD_CMN_INTERVAL_REQ;
		cpkt.param.period = val;
		pkg_ap.wr_buf = &hd[1];
		pkg_ap.wr_len = sizeof(cpkt.param);
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
		if (ret != 0) {
			hwlog_err("set %s delay cmd to mcu fail, ret = %d in %s\n", data->name, ret, __func__);
			return ret;
		}
		if (pkg_mcu.errno != 0) {
			hwlog_err("set %s delay failed errno %d in %s\n", data->name, pkg_mcu.errno, __func__);
			return -EINVAL;
		} else {
			hwlog_info("set %s delay (%ld)success\n", data->name, val);
		}
	} else {
		hwlog_err("set %s delay_ms %d ms range error in %s\n", data->name, (int)val, __func__);
		return -EINVAL;
	}

	return size;
}

static const char *get_sensor_info_by_tag(int tag)
{
	SENSOR_DETECT_LIST sname = SENSOR_MAX;
	sname = get_id_by_sensor_tag(tag);
	return (sname != SENSOR_MAX) ? sensor_chip_info[sname] : "";
}

static ssize_t show_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);

	return snprintf(buf, MAX_STR_SIZE, "%s\n", get_sensor_info_by_tag(data->tag));
}

extern uint8_t tag_to_hal_sensor_type[TAG_SENSOR_END];
static ssize_t show_get_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
        unsigned int hal_sensor_tag = tag_to_hal_sensor_type[data->tag];
	CHECK_SENSOR_COOKIE(data);

	{
		struct t_sensor_get_data *get_data = &sensor_status.get_data[hal_sensor_tag];
		unsigned int offset = 0;
		int i = 0, mem_num = 0;

		atomic_set(&get_data->reading, 1);
		reinit_completion(&get_data->complete);
		if (0 == wait_for_completion_interruptible(&get_data->complete)) {/*return -ERESTARTSYS if interrupted, 0 if completed.*/
			for (mem_num = get_data->data.length /sizeof(get_data->data.value[0]); i < mem_num; i++) {
				if(((data->tag == TAG_ALS)||(data->tag == TAG_ALS1))&& (i == 0)){
					get_data->data.value[0] = get_data->data.value[0] / ALS_MCU_HAL_CONVER;
				}
				if(data->tag == TAG_ACCEL || data->tag == TAG_ACC1 || data->tag == TAG_ACC2){
					get_data->data.value[i] = get_data->data.value[i]/ACC_CONVERT_COFF;//need be devicdd by 1000.0 for high resolu
				}
				offset += sprintf(buf + offset, "%d\t", get_data->data.value[i]);
			}
			offset += sprintf(buf + offset, "\n");
		}

		return offset;
	}

	return 0;
}

static ssize_t store_get_data(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);

	{
		struct timeval tv;
		struct sensor_data event;
		int arg;
		int argc = 0;
		time_t get_data_current_time;

		memset(&tv, 0, sizeof(struct timeval));
		do_gettimeofday(&tv);
		get_data_current_time = tv.tv_sec;
		if((get_data_current_time - get_data_last_time) < 1){
			hwlog_info("%s:time interval is less than 1s(from %u to %u), skip\n",
				__func__, (uint32_t)get_data_last_time, (uint32_t)get_data_current_time);
			return size;
		}
		get_data_last_time = get_data_current_time;

		/*parse cmd buffer*/
		for (; (buf = get_str_begin(buf)) != NULL; buf = get_str_end(buf)) {
			if (get_arg(buf, &arg)) {
				if (argc < (sizeof(event.value) /sizeof(event.value[0]))) {
					event.value[argc++] = arg;
				} else {
					hwlog_err("too many args, %d will be ignored\n", arg);
				}
			}
		}

		/*fill sensor event and write to sensor event buffer*/
		report_sensor_event(data->tag, event.value, sizeof(event.value[0]) * argc);
	}

	return size;
}

static ssize_t show_selftest(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);
	return snprintf(buf, MAX_STR_SIZE, "%s\n", sensor_status.selftest_result[data->tag]);
}

static ssize_t store_selftest(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = 0;
	pkt_subcmd_req_t cpkt;
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);
	memcpy(sensor_status.selftest_result[data->tag], "1", 2);

	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

	if (1 == val) {
		pkt_header_resp_t resp_pkt;
		cpkt.hd.tag = data->tag;
		cpkt.hd.cmd = CMD_CMN_CONFIG_REQ;
		cpkt.subcmd = SUB_CMD_SELFTEST_REQ;
		cpkt.hd.resp = RESP;
		cpkt.hd.length = SUBCMD_LEN;
		if (0 == WAIT_FOR_MCU_RESP_DATA_AFTER_SEND(&cpkt,
							   inputhub_mcu_write_cmd(&cpkt, sizeof(cpkt)), 4000, &resp_pkt, sizeof(resp_pkt))) {
			hwlog_err("wait for %s selftest timeout\n", data->name);
			memcpy(sensor_status.selftest_result[data->tag], "1", 2);/*flyhorse k : SUC-->"0", OTHERS-->"1"*/
			return size;
		} else {
			if (resp_pkt.errno != 0) {
				hwlog_err("%s selftest fail\n", data->name);
				memcpy(sensor_status.selftest_result[data->tag], "1", 2);
			} else {
				hwlog_info("%s selftest success\n", data->name);
				memcpy(sensor_status.selftest_result[data->tag], "0", 2);
			}
		}
	}

	return size;
}

static ssize_t show_read_airpress(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);

	return show_sensor_read_airpress_common(dev, attr, buf);
}

static ssize_t show_get_sensor_id(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);
	hwlog_info("feima %s tag=%d\n", __func__, data->tag);
	return show_get_sensors_id(data->tag, dev, attr, buf);
}

static ssize_t show_calibrate(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);
	hwlog_info("feima %s tag=%d\n", __func__, data->tag);

	return sensors_calibrate_show(data->tag, dev, attr, buf);
}

static ssize_t store_calibrate(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);

	return sensors_calibrate_store(data->tag, dev, attr, buf, size);
}

/***************************************************************
* modify als para online
* als pattern : vendor1--bh1745,  vendor2-- apds9251
* step 1:   write data to node in ap. eg:echo xx,xx, ... xx  >  /sys/class/sensors/als_debug_data
* step 2:   reboot sensorhub. eg: echo 1 > /sys/devices/platform/huawei_sensor/iom3_recovery
* then sensorhub restart and read als parameter into itself. so als para refreshed.
****************************************************************/
static ssize_t show_als_debug_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	short als_debug_para[ALS_DBG_PARA_SIZE]={0,0,0,0,0,0,0,0};

	if (buf == NULL)
		return 0;

	hwlog_info("%s: show cofficient.\n", __FUNCTION__);

	if (rohm_rgb_flag == 1) { //bh745_para
		als_debug_para[0] = als_para_diff_tp_color_table[als_para_table].bh745_para[0];//cofficient_judge
		als_debug_para[1] = als_para_diff_tp_color_table[als_para_table].bh745_para[1];//cofficient_red[0]
		als_debug_para[2] = als_para_diff_tp_color_table[als_para_table].bh745_para[3];//cofficient_green[0]
		als_debug_para[3] = als_para_diff_tp_color_table[als_para_table].bh745_para[2];//cofficient_red[1]
		als_debug_para[4] = als_para_diff_tp_color_table[als_para_table].bh745_para[4];//cofficient_green[1]
		hwlog_info("%s:rohm_rgb_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
	} else if (avago_rgb_flag == 1 || apds9253_rgb_flag == 1) { //apds251_para or apds9253_para
		als_debug_para[1] = apds_als_para_diff_tp_color_table[als_para_table].apds251_para[9];//avago_cofficient[1]
		als_debug_para[2] = apds_als_para_diff_tp_color_table[als_para_table].apds251_para[10];//avago_cofficient[2]
		als_debug_para[3] = apds_als_para_diff_tp_color_table[als_para_table].apds251_para[20];//avago_cofficient[3]
		als_debug_para[4] = apds_als_para_diff_tp_color_table[als_para_table].apds251_para[2];//LUX_P
		als_debug_para[5] = apds_als_para_diff_tp_color_table[als_para_table].apds251_para[4];//LUX_R
		als_debug_para[6] = apds_als_para_diff_tp_color_table[als_para_table].apds251_para[3];//LUX_Q
		als_debug_para[7] = apds_als_para_diff_tp_color_table[als_para_table].apds251_para[19];//lux_mix
		hwlog_info("%s:avago_rgb_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
	} else if (apds9999_rgb_flag == 1) { //apds9999_para
		als_debug_para[1] = apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para[9];//avago_cofficient[1]
		als_debug_para[2] = apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para[10];//avago_cofficient[2]
		als_debug_para[3] = apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para[20];//avago_cofficient[3]
		als_debug_para[4] = apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para[2];//LUX_P
		als_debug_para[5] = apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para[4];//LUX_R
		als_debug_para[6] = apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para[3];//LUX_Q
		als_debug_para[7] = apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para[19];//lux_mix
		hwlog_info("%s:apds9999_rgb_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
	}else if (ams_tmd3725_rgb_flag == 1) { //tmd3725_para
		als_debug_para[1] = tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para[0];//atime
		als_debug_para[2] = tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para[1];//again
		als_debug_para[3] = tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para[2];//dgf
		als_debug_para[4] = tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para[3];//c_coef
		als_debug_para[5] = tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para[4];//r_coef
		als_debug_para[6] = tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para[5];//g_coef
		als_debug_para[7] = tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para[6];//b_coef
		hwlog_info("%s:ams_tmd3725_rgb_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
	}else if (ams_tmd3702_rgb_flag == 1) { //tmd3702_para
		als_debug_para[1] = tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para[0];//atime
		als_debug_para[2] = tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para[1];//again
		als_debug_para[3] = tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para[2];//dgf
		als_debug_para[4] = tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para[3];//c_coef
		als_debug_para[5] = tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para[4];//r_coef
		als_debug_para[6] = tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para[5];//g_coef
		als_debug_para[7] = tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para[6];//b_coef
		hwlog_info("%s:ams_tmd3702_rgb_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
	}else if (ams_tcs3701_rgb_flag == 1) { //tmd3702_para
		als_debug_para[1] = tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para[0];//atime
		als_debug_para[2] = tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para[1];//again
		als_debug_para[3] = tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para[2];//dgf
		als_debug_para[4] = tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para[3];//c_coef
		als_debug_para[5] = tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para[4];//r_coef
		als_debug_para[6] = tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para[5];//g_coef
		als_debug_para[7] = tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para[6];//b_coef
		hwlog_info("%s:ams_tcs3701_rgb_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
	} else if (vishay_vcnl36658_als_flag ==1){
		als_debug_para[1] = vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para[0];
		als_debug_para[2] = vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para[1];
		als_debug_para[3] = vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para[2];
		als_debug_para[4] = vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para[3];
		als_debug_para[5] = vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para[4];
		als_debug_para[6] = vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para[5];
		als_debug_para[7] = vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para[6];
		hwlog_info("%s:vishay_vcnl36658_als_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
	}else if (liteon_ltr582_rgb_flag == 1) { //liteon_ltr582_rgb_flag
		als_debug_para[1] = ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para[3];//ad_radio
		als_debug_para[2] = ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para[4];//dc_radio
		als_debug_para[3] = ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para[5];//a_winfac
		als_debug_para[4] = ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para[6];//d_winfac
		als_debug_para[5] = ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para[7];//c_winfac
		als_debug_para[6] = ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para[8];//slope
		als_debug_para[7] = ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para[9];//slope_offset
		hwlog_info("%s:liteon_ltr582_rgb_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
	}else if (tmd2745_flag == 1) {
		als_debug_para[0] = tmd2745_als_para_diff_tp_color_table[als_para_table].als_para[0];//D_factor
		als_debug_para[1] = tmd2745_als_para_diff_tp_color_table[als_para_table].als_para[1];//B_Coef
		als_debug_para[2] = tmd2745_als_para_diff_tp_color_table[als_para_table].als_para[2];//C_Coef
		als_debug_para[3] = tmd2745_als_para_diff_tp_color_table[als_para_table].als_para[3];//D_Coef
		als_debug_para[4] = tmd2745_als_para_diff_tp_color_table[als_para_table].als_para[4];//is_min_algo
		als_debug_para[5] = tmd2745_als_para_diff_tp_color_table[als_para_table].als_para[5];//is_auto_gain
		hwlog_info("%s:tmd2745_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
	}

	return snprintf(buf, BUF_SIZE, "%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd\n", als_debug_para[0],als_debug_para[1],als_debug_para[2],
			   als_debug_para[3],als_debug_para[4],als_debug_para[5],als_debug_para[6],als_debug_para[7]);

}

static ssize_t store_als_debug_data(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	short als_debug_para[ALS_DBG_PARA_SIZE]={0,0,0,0,0,0,0,0};

	if (buf == NULL)
		return 0;

	if (sscanf(buf,"%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd",&als_debug_para[0],&als_debug_para[1],&als_debug_para[2],\
		           &als_debug_para[3],&als_debug_para[4],&als_debug_para[5],&als_debug_para[6],&als_debug_para[7])){
		hwlog_info("%s: get parameter success.\n", __FUNCTION__);
	} else {
		hwlog_info("%s: get parameter fail.\n", __FUNCTION__);
	}

	if (rohm_rgb_flag == 1) { //bh745_para
		als_para_diff_tp_color_table[als_para_table].bh745_para[0]=als_debug_para[0];//cofficient_judge
		als_para_diff_tp_color_table[als_para_table].bh745_para[1]=als_debug_para[1];//cofficient_red[0]
		als_para_diff_tp_color_table[als_para_table].bh745_para[3]=als_debug_para[2];//cofficient_green[0]
		als_para_diff_tp_color_table[als_para_table].bh745_para[2]=als_debug_para[3];//cofficient_red[1]
		als_para_diff_tp_color_table[als_para_table].bh745_para[4]=als_debug_para[4];//cofficient_green[1]
		hwlog_info("%s:rohm_rgb_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
		memcpy(als_data.als_extend_data, als_para_diff_tp_color_table[als_para_table].bh745_para,
			sizeof(als_para_diff_tp_color_table[als_para_table].bh745_para)>SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE?SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE:sizeof(als_para_diff_tp_color_table[als_para_table].bh745_para));
	} else if (avago_rgb_flag == 1) {//apds251_para
		apds_als_para_diff_tp_color_table[als_para_table].apds251_para[9]= als_debug_para[1];//avago_cofficient[1]
		apds_als_para_diff_tp_color_table[als_para_table].apds251_para[10]= als_debug_para[2];//avago_cofficient[2]
		apds_als_para_diff_tp_color_table[als_para_table].apds251_para[20]= als_debug_para[3];//avago_cofficient[3]
		apds_als_para_diff_tp_color_table[als_para_table].apds251_para[2]= als_debug_para[4];//LUX_P
		apds_als_para_diff_tp_color_table[als_para_table].apds251_para[4]= als_debug_para[5];//LUX_R
		apds_als_para_diff_tp_color_table[als_para_table].apds251_para[3]= als_debug_para[6];//LUX_Q
		apds_als_para_diff_tp_color_table[als_para_table].apds251_para[19]= als_debug_para[7];//lux_mix
		hwlog_info("%s:avago_rgb_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
		memcpy(als_data.als_extend_data, apds_als_para_diff_tp_color_table[als_para_table].apds251_para,
			sizeof(apds_als_para_diff_tp_color_table[als_para_table].apds251_para)>SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE?SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE:sizeof(apds_als_para_diff_tp_color_table[als_para_table].apds251_para));
	}else if (apds9999_rgb_flag == 1) {//apds9999_para
		apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para[9]= als_debug_para[1];//avago_cofficient[1]
		apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para[10]= als_debug_para[2];//avago_cofficient[2]
		apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para[20]= als_debug_para[3];//avago_cofficient[3]
		apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para[2]= als_debug_para[4];//LUX_P
		apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para[4]= als_debug_para[5];//LUX_R
		apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para[3]= als_debug_para[6];//LUX_Q
		apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para[19]= als_debug_para[7];//lux_mix
		hwlog_info("%s:apds9999_rgb_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
		memcpy(als_data.als_extend_data, apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para,
			sizeof(apds9999_als_para_diff_tp_color_table[als_para_table].apds9999_para)>SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE?SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE:sizeof(apds_als_para_diff_tp_color_table[als_para_table].apds251_para));
	}else if (ams_tmd3725_rgb_flag == 1) {//tmd3725
		tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para[0]= als_debug_para[1];//atime
		tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para[1]= als_debug_para[2];//again
		tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para[2]= als_debug_para[3];//dgf
		tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para[3]= als_debug_para[4];//c_coef
		tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para[4]= als_debug_para[5];//r_coef
		tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para[5]= als_debug_para[6];//g_coef
		tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para[6]= als_debug_para[7];//b_coef
		hwlog_info("%s:tmd3725_rgb_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
		memcpy(als_data.als_extend_data, tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para,
			sizeof(tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para)>SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE?SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE:sizeof(tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para));
	}else if (ams_tmd3702_rgb_flag == 1) {//tmd3702
		tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para[0]= als_debug_para[1];//atime
		tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para[1]= als_debug_para[2];//again
		tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para[2]= als_debug_para[3];//dgf
		tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para[3]= als_debug_para[4];//c_coef
		tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para[4]= als_debug_para[5];//r_coef
		tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para[5]= als_debug_para[6];//g_coef
		tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para[6]= als_debug_para[7];//b_coef
		hwlog_info("%s:tmd3725_rgb_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
		memcpy(als_data.als_extend_data, tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para,
			sizeof(tmd3702_als_para_diff_tp_color_table[als_para_table].tmd3702_para)>SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE?SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE:sizeof(tmd3725_als_para_diff_tp_color_table[als_para_table].tmd3725_para));
	}else if (ams_tcs3701_rgb_flag == 1) {//tcs3701
		tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para[0]= als_debug_para[1];//atime
		tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para[1]= als_debug_para[2];//again
		tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para[2]= als_debug_para[3];//dgf
		tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para[3]= als_debug_para[4];//c_coef
		tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para[4]= als_debug_para[5];//r_coef
		tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para[5]= als_debug_para[6];//g_coef
		tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para[6]= als_debug_para[7];//b_coef
		hwlog_info("%s:tcs3701_rgb_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
		memcpy(als_data.als_extend_data, tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para,
			sizeof(tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para)>SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE?SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE:sizeof(tcs3701_als_para_diff_tp_color_table[als_para_table].tcs3701_para));
	}else if (vishay_vcnl36658_als_flag ==1 ){
		vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para[0]= als_debug_para[1];
		vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para[1]= als_debug_para[2];
		vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para[2]= als_debug_para[3];
		vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para[3]= als_debug_para[4];
		vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para[4]= als_debug_para[5];
		vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para[5]= als_debug_para[6];
		vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para[6]= als_debug_para[7];
		hwlog_info("%s:vcnl36658_als_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
		memcpy(als_data.als_extend_data, vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para,
			sizeof(vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para)>SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE?SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE:sizeof(vcnl36658_als_para_diff_tp_color_table[als_para_table].vcnl36658_para));
	}else if (liteon_ltr582_rgb_flag == 1) {// liteon_ltr582_rgb_flag
		ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para[3]= als_debug_para[1];//ad_radio
		ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para[4]= als_debug_para[2];//dc_radio
		ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para[5]= als_debug_para[3];//a_winfac
		ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para[6]= als_debug_para[4];//d_winfac
		ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para[7]= als_debug_para[5];//c_winfac
		ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para[8]= als_debug_para[6];//slope
		ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para[9]= als_debug_para[7];//slope_offset
		hwlog_info("%s:liteon_ltr582_rgb_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
		memcpy(als_data.als_extend_data, ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para,
			sizeof(ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para)>SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE?SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE:sizeof(ltr582_als_para_diff_tp_color_table[als_para_table].ltr582_para));
	}  else if (tmd2745_flag == 1) {
		tmd2745_als_para_diff_tp_color_table[als_para_table].als_para[0] = als_debug_para[0];//D_factor
		tmd2745_als_para_diff_tp_color_table[als_para_table].als_para[1] = als_debug_para[1];//B_Coef
		tmd2745_als_para_diff_tp_color_table[als_para_table].als_para[2] = als_debug_para[2];//C_Coef
		tmd2745_als_para_diff_tp_color_table[als_para_table].als_para[3] = als_debug_para[3];//D_Coef
		tmd2745_als_para_diff_tp_color_table[als_para_table].als_para[4] = als_debug_para[4];//is_min_algo
		tmd2745_als_para_diff_tp_color_table[als_para_table].als_para[5] = als_debug_para[5];//is_auto_gain
		hwlog_info("%s:tmd2745_flag is true and als_para_table=%d.\n", __FUNCTION__,als_para_table);
		memcpy(als_data.als_extend_data, tmd2745_als_para_diff_tp_color_table[als_para_table].als_para,
			sizeof(tmd2745_als_para_diff_tp_color_table[als_para_table].als_para)>SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE?SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE:sizeof(tmd2745_als_para_diff_tp_color_table[als_para_table].als_para));
	}

	return size;

}

static ssize_t show_selftest_timeout(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);

	return snprintf(buf, MAX_STR_SIZE, "%d\n", 1000);	/*ms*/
}

static ssize_t show_calibrate_timeout(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);

	return snprintf(buf, MAX_STR_SIZE, "%d\n", 1000);	/*ms*/
}

static ssize_t show_mag_calibrate_method(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", mag_data.calibrate_method);
}

static ssize_t show_calibrate_method(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);

	return show_mag_calibrate_method(dev, attr, buf);	/*none:0, DOE:1, DOEaG:2*/
}
static ssize_t show_cap_prox_calibrate_type(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);

	return show_cap_prox_calibrate_method(dev, attr, buf);	/*non auto:0, auto:1*/
}
static ssize_t show_cap_prox_calibrate_order(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);

	return show_cap_prox_calibrate_orders(data->tag, dev, attr, buf);
}

#define SAR_SENSOR_DEDECT_LENGTH 10
static int sar_sensor_i2c_detect(struct sar_sensor_detect sar_detect)
{
	int detect_result = 0;
	int ret;
	uint8_t bus_num;
	uint32_t i2c_address;
	uint16_t reg_add;
	uint8_t buf_temp[4] = { 0 };
	uint32_t register_add_len = 0;
	uint32_t rx_len = 0;
	uint32_t chip_id_value;

	/*##(bus_num)##(i2c_addr)##(reg_addr)##(len)*/
	bus_num = sar_detect.cfg.bus_num;
	i2c_address = sar_detect.cfg.i2c_address;
	reg_add = sar_detect.chip_id;
	buf_temp[0] = 0;
	hwlog_info("In %s! bus_num = 0x%x, i2c_address = 0x%x, reg_add = 0x%8x ,\
		chip_id_value = 0x%8x 0x%8x\n", __func__, bus_num, i2c_address,
		reg_add, sar_detect.chip_id_value[0],
		sar_detect.chip_id_value[1]);

	if (reg_add & 0xFF00) {
		register_add_len = 2 ; //reg addr is 16bit
		rx_len = 4;
	} else {
		register_add_len = 1 ; //reg addr is 8bit
		rx_len = 1;
	}

	/*
	 *static int mcu_i2c_rw(uint8_t bus_num, uint8_t i2c_add,
	 *uint8_t register_add,uint8_t *rw, int len, uint8_t *buf)
	 */
	ret = mcu_i2c_rw(bus_num, i2c_address, (uint8_t *)&reg_add,
		register_add_len, &buf_temp[0], rx_len);
	if (ret < 0) {
		hwlog_err("In %s! i2c read reg fail!\n",__func__);
		return 0;
	}

	memcpy(&chip_id_value, buf_temp, sizeof(buf_temp));

	if ((chip_id_value == sar_detect.chip_id_value[0]) ||
		(chip_id_value == sar_detect.chip_id_value[1])) {
		detect_result = 1;
		hwlog_info("sar_sensor_detect_succ 0x%8x\n", chip_id_value);
	} else {
		detect_result = 0;
		hwlog_info("sar_sensor_detect_fail 0x%8x\n", chip_id_value);
	}
	return detect_result;
}
static ssize_t show_sar_sensor_detect(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int final_detect_result = 0;
	int semtech_detect_result = 0;
	int adi_detect_result = 0;
	int cypress_detect_result = 0;
	int semtech_9335_detect_result = 0;
	int abov_detect_result = 0;

	if (semtech_sar_detect.detect_flag == 1) {
		hwlog_info("semtech_sar_detect \n");
		semtech_detect_result = sar_sensor_i2c_detect(semtech_sar_detect);
	}
	if (adi_sar_detect.detect_flag == 1) {
		hwlog_info("adi_sar_detect \n");
		adi_detect_result = sar_sensor_i2c_detect(adi_sar_detect);
	}
	if (cypress_sar_detect.detect_flag == 1) {
		hwlog_info("cypress_sar_detect \n");
		cypress_detect_result = sar_sensor_i2c_detect(cypress_sar_detect);
	}
	if (semtech_9335_sar_detect.detect_flag == 1) {
		hwlog_info("semtech_9335_sar_detect\n");
		semtech_9335_detect_result = sar_sensor_i2c_detect(semtech_9335_sar_detect);
	}
	if (g_abov_sar_detect.detect_flag == 1) {
		hwlog_info("abov_sar_detect\n");
		abov_detect_result = sar_sensor_i2c_detect(g_abov_sar_detect);
	}

	final_detect_result = semtech_detect_result | adi_detect_result |
		cypress_detect_result | semtech_9335_detect_result | abov_detect_result;
	hwlog_info("In %s! final_detect_result=%d, semtech_9323=%d, adi=%d ,\
		cypress=%d semtech_9335=%d\n", __func__, final_detect_result,
		semtech_detect_result, adi_detect_result, cypress_detect_result,
		semtech_9335_detect_result);
	hwlog_info("abov_detect_result = %d!\n", abov_detect_result);
	return snprintf(buf, MAX_STR_SIZE, "%d\n", final_detect_result);
}
static ssize_t store_fingersense_enable(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret = -1;

	if (strict_strtoul(buf, 10, &val)) {
		hwlog_err("%s: finger sense enable val(%lu) invalid", __FUNCTION__, val);
		return -EINVAL;
	}

	hwlog_info("%s: finger sense enable val (%ld)\n", __FUNCTION__, val);
	if ((val != 0) && (val != 1)) {
		hwlog_err("%s:finger sense set enable fail, invalid val\n", __FUNCTION__);
		return size;
	}

	if (fingersense_enabled == val) {
		hwlog_info("%s:finger sense already at seted state\n", __FUNCTION__);
		return size;
	}

	hwlog_info("%s: finger sense set enable\n", __FUNCTION__);
	ret = fingersense_enable(val);
	if (ret) {
		hwlog_err("%s: finger sense enable fail: %d \n", __FUNCTION__, ret);
		return size;
	}
	fingersense_enabled = val;

	return size;
}

static ssize_t show_fingersense_enable(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", fingersense_enabled);
}

static ssize_t show_fingersense_data_ready(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", fingersense_data_ready);
}

static ssize_t show_fingersense_latch_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	int size;
	size = MAX_STR_SIZE>sizeof(fingersense_data)?sizeof(fingersense_data) : MAX_STR_SIZE;

	if ((!fingersense_data_ready) || (!fingersense_enabled)) {
		hwlog_err("%s:fingersense zaxix not ready(%d) or not enable(%d)\n", __FUNCTION__, fingersense_data_ready, fingersense_enabled);
		return size;
	}
	memcpy(buf, (char *)fingersense_data, size);
	return size;
}

static ssize_t store_fingersense_req_data(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int ret = -1;
	unsigned int sub_cmd = SUB_CMD_ACCEL_FINGERSENSE_REQ_DATA_REQ;

#if defined(CONFIG_HISI_VIBRATOR)
	if ((vibrator_shake == 1) || ((HALL_COVERD & hall_value) && (hall_sen_type == 0))) {
		hwlog_err("coverd, vibrator shaking, not send fingersense req data cmd to mcu\n");
		return -1;
	}
#endif

	if (!fingersense_enabled) {
		hwlog_err("%s: finger sense not enable,  dont req data\n", __FUNCTION__);
		return size;
	}

	fingersense_data_ready = false;
	ret = fingersense_commu(sub_cmd, sub_cmd, RESP, true);
	if (ret) {
		hwlog_err("%s: finger sense send requst data failed\n", __FUNCTION__);
		return size;
	}
	return size;
}

static ssize_t show_ois_ctrl(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", sensor_status.gyro_ois_status);
}

static ssize_t store_ois_ctrl(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int source = 0, ret = 0;
	unsigned int cmd = 0;
	unsigned int delay = 200;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	source = simple_strtol(buf, NULL, 10);

	if (1 == source) {
		cmd = CMD_CMN_OPEN_REQ;
		ret = ois_commu(TAG_OIS, cmd, source, NO_RESP, false);
		if (ret) {
			hwlog_err("%s: ois open gyro fail\n", __FUNCTION__);
			return size;
		}

		cmd = CMD_CMN_INTERVAL_REQ;
		ret = ois_commu(TAG_OIS, cmd, delay, NO_RESP, false);
		if (ret) {
			hwlog_err("%s: set delay fail\n", __FUNCTION__);
			return size;
		}

		cmd = SUB_CMD_GYRO_OIS_REQ;
		ret = ois_commu(TAG_GYRO, cmd, source, RESP, true);
		if (ret) {
			hwlog_err("%s: ois enable fail\n", __FUNCTION__);
			return size;
		}
		hwlog_info("%s:ois enable succsess\n", __FUNCTION__);
	} else {
		cmd = SUB_CMD_GYRO_OIS_REQ;
		ret = ois_commu(TAG_GYRO, cmd, source, RESP, true);
		if (ret) {
			hwlog_err("%s:ois close fail\n", __FUNCTION__);
			return size;
		}

		cmd = CMD_CMN_CLOSE_REQ;
		ret = ois_commu(TAG_OIS, cmd, source, NO_RESP, false);
		if (ret) {
			hwlog_err("%s: ois close gyro fail\n", __FUNCTION__);
			return size;
		}
		hwlog_info("%s:ois close succsess\n", __FUNCTION__);
	}
	return size;
}

int send_als_ud_data_to_mcu(int tag, uint32_t subcmd, const void *data, int length, bool is_recovery)
{
	int ret = 0;
	write_info_t pkg_ap;
	pkt_parameter_req_t cpkt;
	pkt_header_t *hd = (pkt_header_t *)&cpkt;

	memset(&pkg_ap, 0, sizeof(pkg_ap));

	pkg_ap.tag = tag;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	cpkt.subcmd = subcmd;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = length+SUBCMD_LEN;
	memcpy(cpkt.para, data, length);

	if(is_recovery){
		return write_customize_cmd(&pkg_ap, NULL, false);
	}

	ret = write_customize_cmd(&pkg_ap,  NULL, true);
	if (ret){
		hwlog_err("send als ud data to mcu fail,ret=%d\n", ret);
		return -1;
	}

	return 0;
}

void send_dc_status_to_sensorhub(uint32_t dc_status)
{
	uint32_t para;

	if (als_data.als_phone_type == VOGUE_PHONE_TYPE) {
		para = (uint32_t)dc_status;
		send_als_ud_data_to_mcu(TAG_ALS, SUB_CMD_CHANGE_DC_STATUS,
			(const void *)&(para), sizeof(para), false);
	}
}

void save_light_to_sensorhub(uint32_t mipi_level, uint32_t bl_level)
{
	uint64_t timestamp = 0;
	struct timespec64 ts;
	struct BrightData para;

	if (als_data.als_phone_type == ELE_PHONE_TYPE || als_data.als_phone_type == VOGUE_PHONE_TYPE ||
			als_data.is_bllevel_supported) {
		get_monotonic_boottime64(&ts);
		timestamp = ((unsigned long long)(ts.tv_sec * NSEC_PER_SEC) + (unsigned long long)ts.tv_nsec) / 1000000;
		para.mipiData = mipi_level;
		para.brightData = bl_level;
		para.timeStamp = (uint64_t)timestamp;
		send_als_ud_data_to_mcu(TAG_ALS, SUB_CMD_UPDATE_BL_LEVEL, (const void *)&(para), sizeof(para), false);
	}
}

extern void get_als_under_tp_calibrate_data(void);
static ssize_t show_als_ud_rgbl_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	get_als_under_tp_calibrate_data();
	if (als_data.als_phone_type != ELE_PHONE_TYPE)
		return snprintf(buf, MAX_STR_SIZE, "%d,%d,%d,%d\n", 0,0,0,0);
	else if (als_under_tp_cal_data.x == 0 && als_under_tp_cal_data.y == 0)
		return snprintf(buf, MAX_STR_SIZE, "%d,%d,%d,%d\n", 662,0,762,100);
	else
		return snprintf(buf, MAX_STR_SIZE, "%d,%d,%d,%d\n",
		als_under_tp_cal_data.x - HALF_LENGTH > 0 ? als_under_tp_cal_data.x - HALF_LENGTH : 0,
		als_under_tp_cal_data.y - HALF_LENGTH > 0 ? als_under_tp_cal_data.y - HALF_LENGTH : 0,
		als_under_tp_cal_data.x + HALF_LENGTH,
		als_under_tp_cal_data.y + HALF_LENGTH);
}

static int get_cmd(const char *str)
{
	int i;
	for (i = 0; i < sizeof(als_ud_cmd_map) / sizeof(als_ud_cmd_map[0]); ++i) {
		if (str_fuzzy_match(str, als_ud_cmd_map[i].str)) {
			return als_ud_cmd_map[i].cmd;
		}
	}
	return -1;
}

static ssize_t store_rgb_status(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int cmd = 0;
	int arg = -1;
	int argc = 0;
	uint64_t para[2];
	unsigned char *share_buf;

	for (; (buf = get_str_begin(buf)) != NULL; buf = get_str_end(buf)) {
		if (!cmd){
			cmd = get_cmd(buf);
		}

		if (get_arg(buf, &arg)) {
			if (argc < MAX_ALS_UD_CMD_BUF_ARGC) {
				para[argc++] = arg;
			} else {
				hwlog_err("too many args, %d will be ignored\n", arg);
			}
		}
	}

	switch (cmd) {
		case ALS_UD_CMD_SET_ADDR:
			als_rgb_pa_to_sh = para[1];
			share_buf = ((unsigned char*)__va(para[1]));
			send_als_ud_data_to_mcu(TAG_ALS, SUB_CMD_SET_ALS_PA, (const void *)&als_rgb_pa_to_sh, sizeof(als_rgb_pa_to_sh), false);
			break;
		case ALS_UD_CMD_BUFFER_UPDATE:
			send_als_ud_data_to_mcu(TAG_ALS, SUB_CMD_UPDATE_RGB_DATA, (const void *)&para, sizeof(para), false);
			break;
		default:
			hwlog_err("Unspport cmd\n");
			break;
	}

	return size;
}

void wake_up_als_ud_block()
{
	wake_up_interruptible(&sensorhub_als_block_waitq);
}

static ssize_t show_als_ud_rgbl_block_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = 0;
	/*Block node by soft irq*/
	sem_als_ud_rgbl_block_flag = 1;
	als_ud_rgbl_block = 0;
	ret = wait_event_freezable_timeout(sensorhub_als_block_waitq, (als_ud_rgbl_block != 0), msecs_to_jiffies(100000));
	if (ret > 0){
		hwlog_info("normal message transfer \n");
		return snprintf(buf, MAX_STR_SIZE, "%llu,%u,%u\n", \
			als_ud_data_upload.sample_start_time, als_ud_data_upload.sample_interval, als_ud_data_upload.integ_time);//unit:ms
	}else{
		hwlog_info("soft irq time out\n");
		return snprintf(buf, MAX_STR_SIZE, "%d,%d,%d\n", 0, 0, 0);
	}
}

/*files create for every sensor*/
DEVICE_ATTR(enable, 0660, show_enable, store_enable);
DEVICE_ATTR(set_delay, 0660, show_set_delay, store_set_delay);
DEVICE_ATTR(info, 0440, show_info, NULL);

static struct attribute *sensors_attributes[] = {
	&dev_attr_enable.attr,
	&dev_attr_set_delay.attr,
	&dev_attr_info.attr,
	NULL,
};
static const struct attribute_group sensors_attr_group = {
	.attrs = sensors_attributes,
};

static const struct attribute_group *sensors_attr_groups[] = {
	&sensors_attr_group,
	NULL,
};

/*files create for specific sensor*/
static DEVICE_ATTR(get_data, 0660, show_get_data, store_get_data);
static DEVICE_ATTR(self_test, 0660, show_selftest, store_selftest);
static DEVICE_ATTR(self_test_timeout, 0440, show_selftest_timeout, NULL);
static DEVICE_ATTR(read_airpress, 0440, show_read_airpress, NULL);	/*only for airpress*/
static DEVICE_ATTR(set_calidata, 0660, show_calibrate, store_calibrate);	/*only for airpress*/
static DEVICE_ATTR(calibrate, 0660, show_calibrate, store_calibrate);
static DEVICE_ATTR(als_debug_data, 0660, show_als_debug_data, store_als_debug_data);
static DEVICE_ATTR(calibrate_timeout, 0440, show_calibrate_timeout, NULL);
static DEVICE_ATTR(calibrate_method, 0440, show_calibrate_method, NULL);	/*only for magnetic*/
static DEVICE_ATTR(cap_prox_calibrate_type, 0440, show_cap_prox_calibrate_type, NULL);
static DEVICE_ATTR(cap_prox_calibrate_order, 0440, show_cap_prox_calibrate_order, NULL);
static DEVICE_ATTR(sar_sensor_detect, 0440, show_sar_sensor_detect, NULL);
static DEVICE_ATTR(get_sensor_id, 0440, show_get_sensor_id, NULL);

static DEVICE_ATTR(set_fingersense_enable, 0660, show_fingersense_enable, store_fingersense_enable);
static DEVICE_ATTR(fingersense_data_ready, 0440, show_fingersense_data_ready, NULL);
static DEVICE_ATTR(fingersense_latch_data, 0440, show_fingersense_latch_data, NULL);
static DEVICE_ATTR(fingersense_req_data, 0220, NULL, store_fingersense_req_data);
static DEVICE_ATTR(rpc_motion_req,0660, show_rpc_motion_req, store_rpc_motion_req);
static DEVICE_ATTR(rpc_sar_service_req,0660, NULL, store_rpc_sar_service_req);
static DEVICE_ATTR(ois_ctrl, 0660, show_ois_ctrl, store_ois_ctrl);
static DEVICE_ATTR(als_ud_rgbl,0660,show_als_ud_rgbl_status,store_rgb_status);
static DEVICE_ATTR(als_ud_rgbl_block,0440,show_als_ud_rgbl_block_data,NULL);

static ssize_t show_acc_sensorlist_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	memcpy(buf, &sensorlist_info[ACC], sizeof(struct sensorlist_info));
	return sizeof(struct sensorlist_info);
}
static DEVICE_ATTR(acc_sensorlist_info, 0664, show_acc_sensorlist_info, NULL);

static ssize_t show_mag_sensorlist_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	memcpy(buf, &sensorlist_info[MAG], sizeof(struct sensorlist_info));
	return sizeof(struct sensorlist_info);
}
static DEVICE_ATTR(mag_sensorlist_info, 0664, show_mag_sensorlist_info, NULL);

static ssize_t show_gyro_sensorlist_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	memcpy(buf, &sensorlist_info[GYRO], sizeof(struct sensorlist_info));
	return sizeof(struct sensorlist_info);
}

static ssize_t show_gyro_position_info(struct device *dev,
					  struct device_attribute *attr, char *buf)
{
	hwlog_info("%s: gyro_position is (%d)\n", __FUNCTION__, gyro_position);
	return snprintf(buf, MAX_STR_SIZE, "%d\n", gyro_position);
}

static DEVICE_ATTR(gyro_sensorlist_info, 0664, show_gyro_sensorlist_info, NULL);
static DEVICE_ATTR(gyro_position_info, 0660, show_gyro_position_info, NULL);

unsigned long ungyro_timestamp_offset = 0;
#define MAX_TIMEOFFSET_VAL 100000000
static ssize_t store_ungyro_time_offset(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	long val = 0;
	if(dev == NULL || attr == NULL || buf == NULL){
		return -EINVAL;
	}
	if (strict_strtol(buf, 10, &val)) {//change to 10 type
		hwlog_err("%s: read uni val(%lu) invalid", __FUNCTION__, val);
		return -EINVAL;
	}

	hwlog_info("%s: set ungyro timestamp offset val (%ld)\n", __FUNCTION__, val);
	if ((val < -MAX_TIMEOFFSET_VAL) || (val > MAX_TIMEOFFSET_VAL)) {
		hwlog_err("%s:set ungyro timestamp offset fail, invalid val\n", __FUNCTION__);
		return size;
	}

	ungyro_timestamp_offset = val;
	return size;
}
static ssize_t show_ungyro_time_offset(struct device *dev,
					  struct device_attribute *attr, char *buf)
{
	if(dev == NULL || attr == NULL || buf == NULL){
		return -EINVAL;
	}
	hwlog_info("%s: unigyro_time_offset is (%lu)\n", __FUNCTION__, ungyro_timestamp_offset);
	memcpy(buf, &ungyro_timestamp_offset, sizeof(ungyro_timestamp_offset));
	return sizeof(ungyro_timestamp_offset);
}
static DEVICE_ATTR(ungyro_time_offset, 0664, show_ungyro_time_offset, store_ungyro_time_offset);

unsigned long unacc_timestamp_offset = 0;
static ssize_t store_unacc_time_offset(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	long val = 0;
	if(dev == NULL || attr == NULL || buf == NULL){
		return -EINVAL;
	}
	if (strict_strtol(buf, 10, &val)) {//change to 10 type
		hwlog_err("%s: read unacc_timestamp_offset val(%lu) invalid", __FUNCTION__, val);
		return -EINVAL;
	}

	hwlog_info("%s: set acc timestamp offset val (%ld)\n", __FUNCTION__, val);
	if ((val < -MAX_TIMEOFFSET_VAL) || (val > MAX_TIMEOFFSET_VAL)) {
		hwlog_err("%s:set acc timestamp offset fail, invalid val\n", __FUNCTION__);
		return size;
	}

	unacc_timestamp_offset = val;
	return size;
}
static ssize_t show_unacc_time_offset(struct device *dev,
					  struct device_attribute *attr, char *buf)
{
	if(dev == NULL || attr == NULL || buf == NULL){
		return -EINVAL;
	}
	hwlog_info("%s: acc_time_offset is (%lu)\n", __FUNCTION__, unacc_timestamp_offset);
	memcpy(buf, &unacc_timestamp_offset, sizeof(unacc_timestamp_offset));
	return sizeof(unacc_timestamp_offset);
}
static DEVICE_ATTR(unacc_time_offset, 0664, show_unacc_time_offset, store_unacc_time_offset);

static ssize_t show_judge_sensor_in_board_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);

	return show_sensor_in_board_status_sysfs(dev, attr, buf);
}
static DEVICE_ATTR(judge_sensor_in_board_status, 0440, show_judge_sensor_in_board_status, NULL);

static ssize_t show_ps_sensorlist_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	memcpy(buf, &sensorlist_info[PS], sizeof(struct sensorlist_info));
	return sizeof(struct sensorlist_info);
}
static DEVICE_ATTR(ps_sensorlist_info, 0664, show_ps_sensorlist_info, NULL);

static ssize_t show_als_sensorlist_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	memcpy(buf, &sensorlist_info[ALS], sizeof(struct sensorlist_info));
	return sizeof(struct sensorlist_info);
}
static DEVICE_ATTR(als_sensorlist_info, 0664, show_als_sensorlist_info, NULL);

static ssize_t calibrate_threshold_from_mag_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int val = mag_threshold_for_als_calibrate;
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static DEVICE_ATTR(calibrate_threshold_from_mag, 0664, calibrate_threshold_from_mag_show, NULL);

static ssize_t show_airpress_sensorlist_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	memcpy(buf, &sensorlist_info[AIRPRESS], sizeof(struct sensorlist_info));
	return sizeof(struct sensorlist_info);
}
static DEVICE_ATTR(airpress_sensorlist_info, 0664, show_airpress_sensorlist_info, NULL);

static ssize_t show_als_offset_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "als OFFSET:%u  %u  %u  %u  %u  %u\n",
			als_offset[0], als_offset[1], als_offset[2], als_offset[3], als_offset[4], als_offset[5]);
}
static DEVICE_ATTR(als_offset_data, 0444, show_als_offset_data, NULL);

static ssize_t show_ps_offset_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "ps OFFSET:%d  %d  %d\n",
			ps_sensor_offset[0], ps_sensor_offset[1], ps_sensor_offset[2]);
}
static DEVICE_ATTR(ps_offset_data, 0444, show_ps_offset_data, NULL);

static ssize_t show_acc_offset_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "acc offset:%d  %d  %d\nsensitivity:%d  %d  %d\nxis_angle:%d  %d  %d  %d  %d  %d  %d  %d  %d\n",
			gsensor_offset[0], gsensor_offset[1], gsensor_offset[2], gsensor_offset[3], gsensor_offset[4], gsensor_offset[5],
			gsensor_offset[6], gsensor_offset[7], gsensor_offset[8], gsensor_offset[9], gsensor_offset[10], gsensor_offset[11],
			gsensor_offset[12], gsensor_offset[13], gsensor_offset[14]);
}

static DEVICE_ATTR(acc_offset_data, 0444, show_acc_offset_data, NULL);

static ssize_t show_acc1_offset_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "acc offset:%d %d %d\n",
			gsensor1_offset[0], gsensor1_offset[1], gsensor1_offset[2]);
}

static DEVICE_ATTR(acc1_offset_data, 0444, show_acc1_offset_data, NULL);

static ssize_t show_gyro_offset_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "gyro offset:%d  %d  %d\nsensitivity:%d  %d  %d\nxis_angle:%d  %d  %d  %d  %d  %d  %d  %d  %d\nuser calibrated offset:%d  %d  %d\n",
			gyro_sensor_offset[0], gyro_sensor_offset[1], gyro_sensor_offset[2], gyro_sensor_offset[3], gyro_sensor_offset[4], gyro_sensor_offset[5],
			gyro_sensor_offset[6], gyro_sensor_offset[7], gyro_sensor_offset[8], gyro_sensor_offset[9], gyro_sensor_offset[10], gyro_sensor_offset[11],
			gyro_sensor_offset[12], gyro_sensor_offset[13], gyro_sensor_offset[14], gyro_sensor_offset[15], gyro_sensor_offset[16], gyro_sensor_offset[17]);
}

static DEVICE_ATTR(gyro_offset_data, 0444, show_gyro_offset_data, NULL);

static ssize_t show_gyro1_offset_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "gyro1 offset:%d %d %d\n online calibrated offset:%d %d %d\n",
			gyro1_sensor_offset[0], gyro1_sensor_offset[1], gyro1_sensor_offset[2],
			gyro1_sensor_offset[15], gyro1_sensor_offset[16], gyro1_sensor_offset[17]);
}

static DEVICE_ATTR(gyro1_offset_data, 0444, show_gyro1_offset_data, NULL);


static int airpress_close_after_touch_calibrate = true;
static ssize_t attr_airpress_calibrate_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret = -1;
	read_info_t read_pkg;
	char temp[AIRPRESS_CALIDATA_NV_SIZE_WITH_AIRTOUCH] = {0};
	interval_param_t param;
	#ifdef SENSOR_DATA_ACQUISITION
	int airpress_touch_data[AIRPRESS_CAL_NUM] = {0};
	#endif
	RET_TYPE airpress_calibration_result = RET_INIT;

	if (buf == NULL){
		hwlog_err("attr_airpress_calibrate_write: invalid input.\n");
		return -EINVAL;
	}
	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

	/*send calibrate command*/
	read_pkg = send_airpress_calibrate_cmd(TAG_PRESSURE, val, &airpress_calibration_result);
	if (airpress_calibration_result != SUC){
		hwlog_err("attr_airpress_calibrate_write: send_airpress_calibrate_cmd fail.\n");
		return -1;
	}

	if (val%100 == 9){
		memcpy(&pressure_touch_calibrate_data, read_pkg.data, sizeof(pressure_touch_calibrate_data));
		memcpy(temp, &airpress_data.offset, sizeof(airpress_data.offset));
		memcpy(temp + sizeof(airpress_data.offset), &pressure_touch_calibrate_data, sizeof(pressure_touch_calibrate_data));
		if (write_calibrate_data_to_nv(AIRPRESS_CALIDATA_NV_NUM, sizeof(airpress_data.offset)+sizeof(pressure_touch_calibrate_data), "AIRDATA", temp)){
			hwlog_err("attr_airpress_calibrate_write: write_calibrate_data_to_nv fail.\n");
			return -1;
		}
		ret = 1;
		hwlog_info("temp[0~7] = %x %x %x %x %x %x %x %x\n", temp[0], temp[1], temp[2], temp[3], temp[4], temp[5], temp[6], temp[7]);
		hwlog_info("attr_airpress_calibrate_write: write_calibrate_data_to_nv success, cResult: %d %d %d %d %d %d %d %d, tResult: %d %d %d %d %d %d %d %d.\n",
			pressure_touch_calibrate_data.cResult.slope, pressure_touch_calibrate_data.cResult.base_press,
			pressure_touch_calibrate_data.cResult.max_press, pressure_touch_calibrate_data.cResult.raise_press,
			pressure_touch_calibrate_data.cResult.min_press, pressure_touch_calibrate_data.cResult.temp,
			pressure_touch_calibrate_data.cResult.speed, pressure_touch_calibrate_data.cResult.result_flag,
			pressure_touch_calibrate_data.tResult.slope, pressure_touch_calibrate_data.tResult.base_press,
			pressure_touch_calibrate_data.tResult.max_press, pressure_touch_calibrate_data.tResult.raise_press,
			pressure_touch_calibrate_data.tResult.min_press, pressure_touch_calibrate_data.tResult.temp,
			pressure_touch_calibrate_data.tResult.speed, pressure_touch_calibrate_data.tResult.result_flag);
		#ifdef SENSOR_DATA_ACQUISITION
		airpress_touch_data[0] = airpress_data.offset;
		airpress_touch_data[1] = pressure_touch_calibrate_data.cResult.slope;
		airpress_touch_data[2] = pressure_touch_calibrate_data.cResult.base_press;
		airpress_touch_data[3] = pressure_touch_calibrate_data.cResult.max_press;
		airpress_touch_data[4] = pressure_touch_calibrate_data.cResult.raise_press;
		airpress_touch_data[5] = pressure_touch_calibrate_data.cResult.min_press;
		airpress_touch_data[6] = pressure_touch_calibrate_data.cResult.temp;
		airpress_touch_data[7] = pressure_touch_calibrate_data.cResult.speed;
		airpress_touch_data[8] = pressure_touch_calibrate_data.cResult.result_flag;
		airpress_touch_data[9] = pressure_touch_calibrate_data.tResult.slope;
		airpress_touch_data[10] = pressure_touch_calibrate_data.tResult.base_press;
		airpress_touch_data[11] = pressure_touch_calibrate_data.tResult.max_press;
		airpress_touch_data[12] = pressure_touch_calibrate_data.tResult.raise_press;
		airpress_touch_data[13] = pressure_touch_calibrate_data.tResult.min_press;
		airpress_touch_data[14] = pressure_touch_calibrate_data.tResult.temp;
		airpress_touch_data[15] = pressure_touch_calibrate_data.tResult.speed;
		airpress_touch_data[16] = pressure_touch_calibrate_data.tResult.result_flag;
		airpress_touch_data_acquisition(airpress_touch_data, sizeof(airpress_touch_data)/sizeof(int));
		#endif
	}
	else if (val%100 == 3){
		if (sensor_status.opened[TAG_PRESSURE] == 0) { /*if airpress is not opened, open first*/
			hwlog_info("send airpress open cmd(during calibrate) to mcu.\n");
			airpress_close_after_touch_calibrate = true;
			ret = inputhub_sensor_enable(TAG_PRESSURE, true);
			if (ret) {
				hwlog_err("send airpress open cmd(during calibrate) to mcu fail,ret=%d\n", ret);
				return -1;
			}
		}
		else
			airpress_close_after_touch_calibrate = false;
		/*period must = 20 ms*/
		if ((sensor_status.delay[TAG_PRESSURE] == 0) || (sensor_status.delay[TAG_PRESSURE] > 20)) {
			hwlog_info("send airpress setdelay cmd(during calibrate) to mcu.\n");
			memset(&param, 0, sizeof(param));
			param.period = 20;
			ret = inputhub_sensor_setdelay(TAG_PRESSURE, &param);
			if (ret) {
				hwlog_err("send airpress set delay cmd(during calibrate) to mcu fail,ret=%d\n", ret);
				return -1;
			}
		}
		ret = 1;
	}
	else if (val%100 == 10){
		if (airpress_close_after_touch_calibrate == true) {
			airpress_close_after_touch_calibrate = false;
			hwlog_info("send airpress close cmd(during calibrate) to mcu.\n");
			ret = inputhub_sensor_enable(TAG_PRESSURE, false);
			if (ret) {
				hwlog_err("send airpress close cmd(during calibrate) to mcu fail,ret=%d\n", ret);
				return -1;
			}
		}
		ret = 1;
	}
	else
		memcpy(&ret, read_pkg.data, sizeof(ret));
	hwlog_info("attr_airpress_calibrate_write: ret = %d.\n", ret);
	return ret;
}

static ssize_t attr_airpress_calibrate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int val = airpress_calibration_res;
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static DEVICE_ATTR(airpress_calibrate, 0660, attr_airpress_calibrate_show, attr_airpress_calibrate_write);

static ssize_t attr_airpress_set_tp_info_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	pkt_parameter_req_t spkt;
	pkt_header_t *shd = (pkt_header_t *)&spkt;
	int ret = -1;
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	memset(&spkt, 0, sizeof(spkt));

	if (buf == NULL || count > sizeof(spkt.para)) {
	    hwlog_err("attr_airpress_set_tp_info_store: buf is NULL, or data length is too large.\n");
	    return -EINVAL;
	}

	hwlog_info("input buf_size is %lu.\n", count);
	spkt.subcmd = SUB_CMD_SET_TP_COORDINATE;
	pkg_ap.tag = TAG_PRESSURE;
	pkg_ap.cmd=CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf=&shd[1];
	pkg_ap.wr_len=count+SUBCMD_LEN;
	memcpy(spkt.para, buf, count);

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret)
	{
	    hwlog_err("send tag %d tp touch info to mcu fail,ret=%d\n", pkg_ap.tag, ret);
		ret = -1;
	}else{
	    if (pkg_mcu.errno != 0)
	    {
	        hwlog_err("send tp touch info to mcu fail\n");
	        ret = -1;
	    }else{
	        hwlog_info("send tp touch info to mcu succes\n");
	        ret = count;
	    }
	}

	return (ssize_t)ret;
}

static DEVICE_ATTR(airpress_set_tp_info, 0220, NULL, attr_airpress_set_tp_info_store);

static ssize_t attr_als_calibrate_under_tp_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	pkt_parameter_req_t spkt;
	pkt_header_t *shd = (pkt_header_t *)&spkt;
	int ret = -1;
	unsigned long val = 0;
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	memset(&spkt, 0, sizeof(spkt));

	if(buf == NULL){
	    hwlog_err("attr_als_calibrate_under_tp_store: buf is NULL.\n");
	    return -EINVAL;
	}
	if(strict_strtoul(buf, 10, &val)){
	    hwlog_err("attr_als_calibrate_under_tp_store: strtoul fail.\n");
		return -EINVAL;
	}
	if(val != 3){
	    hwlog_err("attr_als_calibrate_under_tp_store: wrong value input.\n");
		return -EINVAL;
	}

	spkt.subcmd = SUB_CMD_SELFCALI_REQ;
	pkg_ap.tag = TAG_ALS;
	pkg_ap.cmd=CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf=&shd[1];
	pkg_ap.wr_len=sizeof(val)+SUBCMD_LEN;
	memcpy(spkt.para, &val, sizeof(val));
	hwlog_info("attr_als_calibrate_under_tp_store: under_tp_als calibrated val is %lu, len is %lu.\n", val, sizeof(val));
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret)
	{
	    hwlog_err("attr_als_calibrate_under_tp_store: send pkg to mcu fail, ret=%d\n", ret);
		ret = -1;
	}else{
	    if (pkg_mcu.errno != 0)
	    {
	        hwlog_err("attr_als_calibrate_under_tp_store: mcu return fail\n");
	        ret = -1;
	    }else{
	        memcpy(als_under_tp_cal_rawdata, pkg_mcu.data, sizeof(als_under_tp_cal_rawdata));
	        hwlog_info("attr_als_calibrate_under_tp_store: mcu return als_rawdata suc. R=%d G=%d B=%d C=%d\n", als_under_tp_cal_rawdata[0],
				als_under_tp_cal_rawdata[1], als_under_tp_cal_rawdata[2], als_under_tp_cal_rawdata[3]);
	        ret = 1;
	    }
	}

	return (ssize_t)ret;
}

static ssize_t attr_als_calibrate_under_tp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if(dev == NULL || attr == NULL || buf == NULL)
		return -1;
	return snprintf(buf, MAX_STR_SIZE, "%d,%d,%d,%d", als_under_tp_cal_rawdata[0], als_under_tp_cal_rawdata[1],
		als_under_tp_cal_rawdata[2], als_under_tp_cal_rawdata[3]);
}

static DEVICE_ATTR(als_calibrate_under_tp, 0660, attr_als_calibrate_under_tp_show, attr_als_calibrate_under_tp_store);

static ssize_t attr_als_rgb_data_under_tp_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	pkt_parameter_req_t spkt;
	pkt_header_t *shd = (pkt_header_t *)&spkt;
	int ret;
	unsigned long val = 0;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	memset(&spkt, 0, sizeof(spkt));
	if (buf == NULL) {
		hwlog_err("%s: buf is NULL.\n", __func__);
		return -EINVAL;
	}
	ret = strict_strtoul(buf, 10, &val);
	if (ret) {
		hwlog_err("%s: strtoul fail.\n", __func__);
		return -EINVAL;
	}
	if (val != ALS_UNDER_TP_RGB_DATA) {
		hwlog_err("%s: wrong value input.\n", __func__);
		return -EINVAL;
	}
	spkt.subcmd = SUB_CMD_SELFCALI_REQ;
	pkg_ap.tag = TAG_ALS;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &shd[1];
	pkg_ap.wr_len = sizeof(val) + SUBCMD_LEN;
	memcpy(spkt.para, &val, sizeof(val));
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret) {
		hwlog_err("%s: send pkg to mcu fail,ret=%d\n", __func__, ret);
		ret = -1;
	} else {
		if (pkg_mcu.errno != 0) {
			hwlog_err("%s: mcu return fail\n", __func__);
			ret = -1;
		} else {
			memcpy(als_under_tp_rgb_data, pkg_mcu.data,
				sizeof(als_under_tp_rgb_data));
			ret = count;
		}
	}
	return ret;
}

static ssize_t attr_als_rgb_data_under_tp_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (dev == NULL || attr == NULL || buf == NULL)
		return -1;
	return snprintf(buf, MAX_STR_SIZE, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		als_under_tp_rgb_data[0], als_under_tp_rgb_data[1],
		als_under_tp_rgb_data[2], als_under_tp_rgb_data[3],
		als_under_tp_rgb_data[4], als_under_tp_rgb_data[5],
		als_under_tp_rgb_data[6], als_under_tp_rgb_data[7],
		als_under_tp_rgb_data[8], als_under_tp_rgb_data[9],
		als_under_tp_rgb_data[10], als_under_tp_rgb_data[11],
		als_under_tp_rgb_data[12], als_under_tp_rgb_data[13],
		als_under_tp_rgb_data[14], als_under_tp_rgb_data[15]);
}

static DEVICE_ATTR(als_rgb_data_under_tp, 0660,
	attr_als_rgb_data_under_tp_show, attr_als_rgb_data_under_tp_store);

/* save als para to nv */
int als_underscreen_calidata_save(void)
{
	if (sizeof(als_under_tp_cal_data) >
		ALS_TP_CALIDATA_NV1_SIZE + ALS_TP_CALIDATA_NV2_SIZE) {
		if (write_calibrate_data_to_nv(ALS_TP_CALIDATA_NV1_NUM,
				ALS_TP_CALIDATA_NV1_SIZE,
				"ALSTP1", ((char *)&als_under_tp_cal_data))) {
			hwlog_err("%s: write_calidata_to_nv1 fail.\n",
				__func__);
			return -1;
		}
		if (write_calibrate_data_to_nv(ALS_TP_CALIDATA_NV2_NUM,
				ALS_TP_CALIDATA_NV2_SIZE,
				"ALSTP2",
				((char *)(&als_under_tp_cal_data) +
				ALS_TP_CALIDATA_NV1_SIZE))) {
			hwlog_err("%s: write_calidata_to_nv2 fail.\n",
				__func__);
			return -1;
		}
		if (write_calibrate_data_to_nv(ALS_TP_CALIDATA_NV3_NUM,
				sizeof(als_under_tp_cal_data) -
				ALS_TP_CALIDATA_NV1_SIZE -
				ALS_TP_CALIDATA_NV2_SIZE,
				"ALSTP3",
				((char *)(&als_under_tp_cal_data) +
				ALS_TP_CALIDATA_NV1_SIZE +
				ALS_TP_CALIDATA_NV2_SIZE))) {
			hwlog_err("%s: write_calidata_to_nv3 fail.\n",
				__func__);
			return -1;
		}
	}
	hwlog_info("als_underscreen_calidata_save success!\n");
	return 0;
}

static ssize_t attr_als_under_tp_calidata_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int i = 0;
	int ret;
	int als_undertp_calidata[ALS_UNDER_TP_CALDATA_LEN] = {0};

	if (buf == NULL || count != (ALS_UNDER_TP_CALDATA_LEN * sizeof(int))) {
	    hwlog_err("attr_als_under_tp_calidata_store: buf is NULL, or count is %zu.\n", count);
	    return -EINVAL;
	}
	hwlog_info("attr_als_under_tp_calidata_store: input buf_size = %zu, als_under_tp_cal_data_len = %zu.\n",
		count, sizeof(als_under_tp_cal_data));
	memcpy(als_undertp_calidata, buf, sizeof(als_undertp_calidata));
	als_under_tp_cal_data.x = (uint16_t)als_undertp_calidata[0];
	als_under_tp_cal_data.y = (uint16_t)als_undertp_calidata[1];
	als_under_tp_cal_data.width= (uint16_t)als_undertp_calidata[2];
	als_under_tp_cal_data.length= (uint16_t)als_undertp_calidata[3];
	hwlog_info("attr_als_under_tp_calidata_store: x = %d, y = %d, width = %d, len = %d.\n", als_under_tp_cal_data.x,
		als_under_tp_cal_data.y, als_under_tp_cal_data.width, als_under_tp_cal_data.length);
	memcpy(als_under_tp_cal_data.a, als_undertp_calidata + 4, 25*sizeof(unsigned int));
	for(i = 4; i < 29; i++){
	    hwlog_info("attr_als_under_tp_calidata_store: als_under_tp_cal_data.a[%d] = %d.\n", i - 4, als_under_tp_cal_data.a[i - 4]);
	}
	for(i = 29; i < ALS_UNDER_TP_CALDATA_LEN; i++){
	    als_under_tp_cal_data.b[i - 29] = als_undertp_calidata[i];
	    hwlog_info("attr_als_under_tp_calidata_store: als_under_tp_cal_data.b[%d] = %d.\n", i - 29, als_under_tp_cal_data.b[i - 29]);
	}
	ret = als_underscreen_calidata_save();
	if (ret != 0) {
		ret = mcu_save_calidata_to_nv(TAG_ALS, als_undertp_calidata);
		if (ret != 0) {
			hwlog_err("%s: als_underscreen_calidata_save fail.\n",
				__func__);
			return -1;
		}

	}

	return count;
}

/* read underscreen als nv data */
static int als_under_tp_nv_read(void)
{
	size_t cal_data_len = sizeof(als_under_tp_cal_data);
	size_t read_len;

	if (cal_data_len > ALS_TP_CALIDATA_NV1_SIZE +
		ALS_TP_CALIDATA_NV2_SIZE) {
		if (read_calibrate_data_from_nv(ALS_TP_CALIDATA_NV1_NUM,
				ALS_TP_CALIDATA_NV1_SIZE, "ALSTP1"))
			return -1;
		memcpy(&als_under_tp_cal_data, user_info.nv_data,
				ALS_TP_CALIDATA_NV1_SIZE);
		if (read_calibrate_data_from_nv(ALS_TP_CALIDATA_NV2_NUM,
				ALS_TP_CALIDATA_NV2_SIZE, "ALSTP2"))
			return -1;
		memcpy((char *)(&als_under_tp_cal_data) +
				ALS_TP_CALIDATA_NV1_SIZE,
				user_info.nv_data,
				ALS_TP_CALIDATA_NV2_SIZE);
		read_len = cal_data_len - ALS_TP_CALIDATA_NV1_SIZE -
					ALS_TP_CALIDATA_NV2_SIZE;
		if (read_calibrate_data_from_nv(ALS_TP_CALIDATA_NV3_NUM,
				read_len, "ALSTP3"))
			return -1;
		memcpy((char *)(&als_under_tp_cal_data) +
				ALS_TP_CALIDATA_NV1_SIZE +
				ALS_TP_CALIDATA_NV2_SIZE,
				user_info.nv_data, read_len);
		hwlog_info("attr_als_under_tp_calidata_show: x = %d, y = %d, width = %d, len = %d\n",
				als_under_tp_cal_data.x,
				als_under_tp_cal_data.y,
				als_under_tp_cal_data.width,
				als_under_tp_cal_data.length);
		return 0;
	}
	return -1;
}

/* return underscreen als to node file */
static ssize_t attr_als_under_tp_calidata_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	int ret;
	int i;
	int print_len;

	if (dev == NULL || attr == NULL || buf == NULL)
		return -1;
	ret = als_under_tp_nv_read();
	if (ret != 0) {
		hwlog_info("%s: read under screen als fail\n", __FUNCTION__);
		return -1;
	}
	ret = snprintf(buf, MAX_STR_SIZE, "%d,%d,%d,%d",
				als_under_tp_cal_data.x,
				als_under_tp_cal_data.y,
				als_under_tp_cal_data.width,
				als_under_tp_cal_data.length);
	if (ret <= 0) {
		hwlog_info("%s: write data to buf fail\n", __FUNCTION__);
		return -1;
	}
	print_len = sizeof(als_under_tp_cal_data.a) / sizeof(unsigned int);
	for (i = 0; i < print_len; i++) {
		ret = snprintf(buf, MAX_STR_SIZE, "%s,%d", buf,
				als_under_tp_cal_data.a[i]);
		if (ret <= 0) {
			hwlog_info("%s: write a[%d] to buf fail\n",
				__FUNCTION__ , i);
			return -1;
		}
	}
	print_len = sizeof(als_under_tp_cal_data.b) / sizeof(int);
	for (i = 0; i < print_len; i++) {
		ret = snprintf(buf, MAX_STR_SIZE, "%s,%d", buf,
				als_under_tp_cal_data.b[i]);
		if (ret <= 0) {
			hwlog_info("%s: write b[%d] to buf fail\n",
				__FUNCTION__ , i);
			return -1;
		}
	}
	return (ssize_t)ret;
}

static DEVICE_ATTR(set_als_under_tp_calidata, 0660,
	attr_als_under_tp_calidata_show, attr_als_under_tp_calidata_store);

/* return als_support_under_screen_cali to node file */
static ssize_t attr_als_calibrate_after_sale_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	if (dev == NULL || attr == NULL || buf == NULL)
		return -1;
	return snprintf(buf, MAX_STR_SIZE, "%d\n",
				als_support_under_screen_cali);
}
static DEVICE_ATTR(als_calibrate_after_sale, 0440,
	attr_als_calibrate_after_sale_show, NULL);

/* return ps_support_under_screen_cali to node file */
static ssize_t attr_ps_calibrate_after_sale_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	if (dev == NULL || attr == NULL || buf == NULL)
		return -1;
	return snprintf(buf, MAX_STR_SIZE, "%d\n",
				ps_support_cali_after_sale);
}
static DEVICE_ATTR(ps_calibrate_after_sale, 0440,
	attr_ps_calibrate_after_sale_show, NULL);

static ssize_t attr_cap_prox_data_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	pkt_parameter_req_t spkt;
	pkt_header_t *shd = (pkt_header_t *)&spkt;
	int ret = 0;
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	memset(&spkt, 0, sizeof(spkt));

	if (buf == NULL) {
		hwlog_err("attr_cap_prox_data_mode_store: buf is NULL.\n");
		return -EINVAL;
	}
	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;
	if (val < MIN_CAP_PROX_MODE || val > MAZ_CAP_PROX_MODE) {
		hwlog_err("cap_prox_data_mode error, val=%lu\n", val);
		return -1;
	}

	spkt.subcmd = SUB_CMD_SET_DATA_MODE;
	pkg_ap.tag = TAG_CAP_PROX;
	pkg_ap.cmd=CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf=&shd[1];
	pkg_ap.wr_len=sizeof(val)+SUBCMD_LEN;
	memcpy(spkt.para, &val, sizeof(val));

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret)
	{
	    hwlog_err("send tag %d sar mode to mcu fail,ret=%d\n", pkg_ap.tag, ret);
		ret = -1;
	}else{
	    if (pkg_mcu.errno != 0)
	    {
	        hwlog_err("send sar mode to mcu fail\n");
	        ret = -1;
	    }else{
	        hwlog_info("send sar mode to mcu succes\n");
	        ret = count;
	    }
	}

	return ret;
}

static DEVICE_ATTR(cap_prox_data_mode, 0220, NULL, attr_cap_prox_data_mode_store);

static struct attribute *acc_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_acc_sensorlist_info.attr,
	&dev_attr_acc_offset_data.attr,
	&dev_attr_unacc_time_offset.attr,
	&dev_attr_judge_sensor_in_board_status.attr,
	&dev_attr_get_sensor_id.attr,
	NULL,
};

static const struct attribute_group acc_sensor_attrs_grp = {
	.attrs = acc_sensor_attrs,
};

static struct attribute *acc1_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_acc_sensorlist_info.attr,
	&dev_attr_acc1_offset_data.attr,
	&dev_attr_unacc_time_offset.attr,
	NULL,
};

static const struct attribute_group acc1_sensor_attrs_grp = {
	.attrs = acc1_sensor_attrs,
};


static int sleeve_test_ps_prepare(int ps_config)
{
	int ret = -1;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	pkg_ap.tag = TAG_PS;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &ps_config;
	pkg_ap.wr_len = sizeof(ps_config);
	ret = write_customize_cmd(&pkg_ap,  &pkg_mcu, true);
	if(ret) {
		hwlog_err("send sleeve_test ps config cmd to mcu fail,ret=%d\n", ret);
		return ret;
	}
	if(pkg_mcu.errno!=0){
		hwlog_err("sleeve_test ps config fail(%d)\n", pkg_mcu.errno);
	}
	return ret;
}

static ssize_t store_sleeve_test_prepare(struct device *dev, struct device_attribute *attr, const char *buf, size_t size){

	unsigned long val = 0;
	int ret = -1;

	if (strict_strtoul(buf, 10, &val)){
		hwlog_err("%s: sleeve_test enable val invalid", __FUNCTION__);
		return -EINVAL;
	}

	hwlog_info("%s: sleeve_test enable val (%ld)\n", __FUNCTION__, val);
	if((val !=0) && (val !=1)) {
		hwlog_err("%s:sleeve_test set enable fail, invalid val\n", __FUNCTION__);
		return -EINVAL;
	}

	if(sleeve_test_enabled == val){
		hwlog_info("%s:sleeve_test already at seted state\n", __FUNCTION__);
		return size;
	}

	ret = sleeve_test_ps_prepare(val);
	if(ret){
		hwlog_err("%s: sleeve_test enable fail: %d \n", __FUNCTION__, ret);
		return -EINVAL;
	}
	sleeve_test_enabled = val;
	hwlog_info("%s: sleeve_test set enable success\n", __FUNCTION__);
	return size;

}

static ssize_t show_sleeve_test_threshhold(struct device *dev, struct device_attribute *attr, char *buf){
	int i = 0;

	for(i = 0; i < MAX_PHONE_COLOR_NUM; i++){
		if(phone_color == sleeve_detect_paremeter[i].tp_color){
			hwlog_info("sleeve_test threshhold (%d), phone_color(%d)\n", sleeve_detect_paremeter[i].sleeve_detect_threshhold, phone_color);
			return snprintf(buf, MAX_STR_SIZE, "%d\n", sleeve_detect_paremeter[i].sleeve_detect_threshhold);
		}
	}
	hwlog_info("sleeve_test get threshhold fail, phone_color(%d)\n", phone_color);
	return -1;
}

static DEVICE_ATTR(sleeve_test_prepare, 0220, NULL, store_sleeve_test_prepare);
static DEVICE_ATTR(sleeve_test_threshhold, 0440, show_sleeve_test_threshhold, NULL);

static ssize_t show_support_absolute_threshold(struct device *dev, struct device_attribute *attr, char *buf)
{
	int val = 0;

	if(dev == NULL || attr == NULL || buf == NULL)
		return -1;

	val = ps_support_abs_threshold;
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static DEVICE_ATTR(support_absolute_threshold, 0664, show_support_absolute_threshold, NULL);

static struct attribute *ps_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_sleeve_test_prepare.attr,
	&dev_attr_ps_sensorlist_info.attr,
	&dev_attr_ps_offset_data.attr,
	&dev_attr_support_absolute_threshold.attr,
	&dev_attr_ps_calibrate_after_sale.attr,
	NULL,
};

static const struct attribute_group ps_sensor_attrs_grp = {
	.attrs = ps_sensor_attrs,
};

static struct attribute *als_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_als_debug_data.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_sleeve_test_threshhold.attr,
	&dev_attr_als_sensorlist_info.attr,
	&dev_attr_calibrate_threshold_from_mag.attr,
	&dev_attr_als_offset_data.attr,
	&dev_attr_als_calibrate_under_tp.attr,
	&dev_attr_set_als_under_tp_calidata.attr,
	&dev_attr_als_rgb_data_under_tp.attr,
	&dev_attr_als_ud_rgbl.attr,
	&dev_attr_als_ud_rgbl_block.attr,
	&dev_attr_als_calibrate_after_sale.attr,
	NULL,
};

static const struct attribute_group als_sensor_attrs_grp = {
	.attrs = als_sensor_attrs,
};

static struct attribute *mag_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate_method.attr,
	&dev_attr_mag_sensorlist_info.attr,
	&dev_attr_get_sensor_id.attr,
	NULL,
};

static const struct attribute_group mag_sensor_attrs_grp = {
	.attrs = mag_sensor_attrs,
};

static struct attribute *mag1_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate_method.attr,
	&dev_attr_mag_sensorlist_info.attr,
	NULL,
};

static const struct attribute_group mag1_sensor_attrs_grp = {
	.attrs = mag1_sensor_attrs,
};


static struct attribute *hall_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	NULL,
};

static const struct attribute_group hall_sensor_attrs_grp = {
	.attrs = hall_sensor_attrs,
};

static struct attribute *gyro_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_gyro_sensorlist_info.attr,
	&dev_attr_gyro_position_info.attr,
	&dev_attr_gyro_offset_data.attr,
	&dev_attr_ungyro_time_offset.attr,
	&dev_attr_get_sensor_id.attr,
	NULL,
};

static const struct attribute_group gyro_sensor_attrs_grp = {
	.attrs = gyro_sensor_attrs,
};

static struct attribute *gyro1_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_gyro_sensorlist_info.attr,
	&dev_attr_gyro_position_info.attr,
	&dev_attr_gyro1_offset_data.attr,
	&dev_attr_ungyro_time_offset.attr,
	NULL,
};

static const struct attribute_group gyro1_sensor_attrs_grp = {
	.attrs = gyro1_sensor_attrs,
};


static struct attribute *airpress_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_read_airpress.attr,
	&dev_attr_set_calidata.attr,
	&dev_attr_airpress_calibrate.attr,
	&dev_attr_airpress_sensorlist_info.attr,
	&dev_attr_airpress_set_tp_info.attr,
	NULL,
};

static const struct attribute_group airpress_sensor_attrs_grp = {
	.attrs = airpress_sensor_attrs,
};

static struct attribute *finger_sensor_attrs[] = {
	&dev_attr_set_fingersense_enable.attr,
	&dev_attr_fingersense_data_ready.attr,
	&dev_attr_fingersense_latch_data.attr,
	&dev_attr_fingersense_req_data.attr,
	NULL,
};

static const struct attribute_group finger_sensor_attrs_grp = {
	.attrs = finger_sensor_attrs,
};

static struct attribute *handpress_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	NULL,
};
static const struct attribute_group handpress_sensor_attrs_grp = {
	.attrs = handpress_sensor_attrs,
};

static struct attribute *ois_sensor_attrs[] = {
	&dev_attr_ois_ctrl.attr,
	NULL,
};

static const struct attribute_group ois_sensor_attrs_grp = {
	.attrs = ois_sensor_attrs,
};
static struct attribute *cap_prox_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_cap_prox_calibrate_type.attr,
	&dev_attr_cap_prox_calibrate_order.attr,
	&dev_attr_sar_sensor_detect.attr,
	&dev_attr_cap_prox_data_mode.attr,
	&dev_attr_get_sensor_id.attr,
	NULL,
};
static const struct attribute_group cap_prox_sensor_attrs_grp = {
	.attrs = cap_prox_sensor_attrs,
};

static struct attribute *cap_prox1_sensor_attrs[] = {
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_cap_prox_calibrate_type.attr,
	&dev_attr_cap_prox_calibrate_order.attr,
	&dev_attr_sar_sensor_detect.attr,
	&dev_attr_cap_prox_data_mode.attr,
	NULL,
};
static const struct attribute_group cap_prox1_sensor_attrs_grp = {
	.attrs = cap_prox1_sensor_attrs,
};


static struct attribute *orientation_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	NULL,
};

static const struct attribute_group orientation_attrs_grp = {
	.attrs = orientation_sensor_attrs,
};
static struct attribute *rpc_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_rpc_motion_req.attr,
	&dev_attr_rpc_sar_service_req.attr,
	NULL,
};
static const struct attribute_group rpc_sensor_attrs_grp = {
	.attrs = rpc_sensor_attrs,
};

static ssize_t ShowStepCounterInfo(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", g_stepcountdocm);
}

static DEVICE_ATTR(step_counter_info, 0664, ShowStepCounterInfo, NULL);

static struct attribute *g_stepcounterattrs[] = {
	&dev_attr_step_counter_info.attr,
	NULL,
};

static const struct attribute_group STEP_COUNTER_ATTRSGRP = {
	.attrs = g_stepcounterattrs,
};
static struct sensor_cookie all_sensors[] = {
	{
	 .tag = TAG_ACCEL,
	 .name = "acc_sensor",
	 .attrs_group = &acc_sensor_attrs_grp,
	 },
	{
	 .tag = TAG_PS,
	 .name = "ps_sensor",
	 .attrs_group = &ps_sensor_attrs_grp,
	 },
	{
	 .tag = TAG_ALS,
	 .name = "als_sensor",
	 .attrs_group = &als_sensor_attrs_grp,
	 },
	{
	 .tag = TAG_MAG,
	 .name = "mag_sensor",
	 .attrs_group = &mag_sensor_attrs_grp,
	 },
	{
	 .tag = TAG_HALL,
	 .name = "hall_sensor",
	 .attrs_group = &hall_sensor_attrs_grp,
	 },
	{
	 .tag = TAG_GYRO,
	 .name = "gyro_sensor",
	 .attrs_group = &gyro_sensor_attrs_grp,
	 },
	{
	 .tag = TAG_PRESSURE,
	 .name = "airpress_sensor",
	 .attrs_group = &airpress_sensor_attrs_grp,
	 },
	{
	 .tag = TAG_FINGERSENSE,
	 .name = "fingersense_sensor",
	 .attrs_group = &finger_sensor_attrs_grp,
	 },
	{
		.tag = TAG_HANDPRESS,
		.name = "handpress_sensor",
		.attrs_group = &handpress_sensor_attrs_grp,
	},
	{
		.tag = TAG_OIS,
		.name = "ois_sensor",
		.attrs_group = &ois_sensor_attrs_grp,
	},
	{
		.tag = TAG_CAP_PROX,
		.name = "cap_prox_sensor",
		.attrs_group = &cap_prox_sensor_attrs_grp,
	},
	{
		.tag = TAG_ORIENTATION,
		.name = "orientation_sensor",
		.attrs_group = &orientation_attrs_grp,
	},
	{
		.tag = TAG_RPC,
		.name = "rpc_sensor",
		.attrs_group = &rpc_sensor_attrs_grp,
	},
	{
		.tag = TAG_ACC1,
		.name = "acc1_sensor",
		.attrs_group = &acc1_sensor_attrs_grp,
	},
	{
		.tag = TAG_GYRO1,
		.name = "gyro1_sensor",
		.attrs_group = &gyro1_sensor_attrs_grp,
	},
	{
		.tag = TAG_MAG1,
		.name = "mag1_sensor",
		.attrs_group = &mag1_sensor_attrs_grp,
	},
	{
		.tag = TAG_CAP_PROX1,
		.name = "cap_prox1_sensor",
		.attrs_group = &cap_prox1_sensor_attrs_grp,
	},
	{
		.tag = TAG_STEP_COUNTER,
		.name = "step_counter",
		.attrs_group = &STEP_COUNTER_ATTRSGRP,
	 },
};

struct device *get_sensor_device_by_name(const char *name)
{
	int i;

	if (NULL == name)
		return NULL;

	for (i = 0; i < sizeof(all_sensors) / sizeof(all_sensors[0]); ++i) {
		if (all_sensors[i].name && (0 == strcmp(name, all_sensors[i].name))) {
			return all_sensors[i].dev;
		}
	}

	return NULL;
}

static void init_sensors_get_data(void)
{
	int tag;
	for (tag = TAG_SENSOR_BEGIN; tag < TAG_SENSOR_END; ++tag) {
		atomic_set(&sensor_status.get_data[tag].reading, 0);
		init_completion(&sensor_status.get_data[tag].complete);
		memcpy(sensor_status.selftest_result[tag], "1", 2); // 1 means fail , 2 set the length of buf
	}
}

/*device_create->device_register->device_add->device_add_attrs->device_add_attributes*/
static int sensors_register(void)
{
	int i;
	for (i = 0; i < sizeof(all_sensors) / sizeof(all_sensors[0]); ++i) {
		all_sensors[i].dev = device_create(sensors_class, NULL, 0, &all_sensors[i], all_sensors[i].name);
		if (NULL == all_sensors[i].dev) {
			hwlog_err("[%s] Failed", __func__);
			return -1;
		} else {
			if (all_sensors[i].attrs_group != NULL) {
				if (sysfs_create_group(&all_sensors[i].dev->kobj, all_sensors[i].attrs_group)) {
					hwlog_err("create files failed in %s\n", __func__);
				}
			}
		}
	}
	return 0;
}

static void sensors_unregister(void)
{
	device_destroy(sensors_class, 0);
}

static void GetDocomStepCounter(void)
{
	char *stepcountty = NULL;
	struct device_node *sensorhubnode = NULL;

	g_stepcountdocm = 0;
	sensorhubnode = of_find_compatible_node(NULL, NULL, "huawei,sensorhub");
	if (of_property_read_string(sensorhubnode, "docom_step_counter",
			(const char **)&stepcountty) == 0) {
		hwlog_info("get_docom_step_counter step_count_ty is %s\n",
						stepcountty);
		if (!strcmp("enabled", stepcountty))
			g_stepcountdocm = 1;
	}
	hwlog_info("%s : docom_step_counter status is %d\n", __func__,
					g_stepcountdocm);
}

static int sensors_feima_init(void)
{
	if (is_sensorhub_disabled())
		return -1;
	GetDocomStepCounter();
	sensors_class = class_create(THIS_MODULE, "sensors");
	if (IS_ERR(sensors_class))
		return PTR_ERR(sensors_class);
	sensors_class->dev_groups = sensors_attr_groups;
	sensors_register();
	init_sensors_get_data();
	create_debug_files();
	return 0;
}

static void sensors_feima_exit(void)
{
	sensors_unregister();
	class_destroy(sensors_class);
}

late_initcall_sync(sensors_feima_init);
module_exit(sensors_feima_exit);

MODULE_AUTHOR("SensorHub <smartphone@huawei.com>");
MODULE_DESCRIPTION("SensorHub feima driver");
MODULE_LICENSE("GPL");
