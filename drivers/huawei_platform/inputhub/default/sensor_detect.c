/*
 * Copyright (C) huawei company
 *
 * This	program	is free	software; you can redistribute it and/or modify
 * it under	the	terms of the GNU General Public	License	version	2 as
 * published by	the	Free Software Foundation.
 * Filename:  sensor_detect.c
 *
 * Discription: some functions of sensorhub power
 *
 * Owner:DIVS_SENSORHUB
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/wakelock.h>
#include "sensor_feima.h"
#include "contexthub_route.h"
#include "contexthub_boot.h"
#include "protocol.h"
#include <sensor_sysfs.h>
#include <sensor_config.h>
#include <sensor_detect.h>
#include "contexthub_debug.h"
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/switch.h>
#include <linux/hisi/hw_cmdline_parse.h>

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#ifdef CONFIG_CONTEXTHUB_SHMEM
#include "shmem.h"
#endif
#define HANDPRESS_DEFAULT_STATE		"huawei,default-state"
#define ADAPT_SENSOR_LIST_NUM           20

#define CURRENT_FAC_AKM_INIT  0
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

static struct sensor_redetect_state s_redetect_state ;
static struct wake_lock sensor_rd;
static struct work_struct redetect_work;
static const char *str_soft_para = "softiron_parameter";
static char buf[MAX_PKT_LENGTH] = { 0 };
static char aux_buf[MAX_PKT_LENGTH] = { 0 };
static pkt_sys_dynload_req_t *dyn_req = (pkt_sys_dynload_req_t *) buf;
static pkt_sys_dynload_req_t *aux_req = (pkt_sys_dynload_req_t *) aux_buf;
struct sleeve_detect_pare sleeve_detect_paremeter[MAX_PHONE_COLOR_NUM] = {{0,0},};
struct sensorlist_info sensorlist_info[SENSOR_MAX];
static u32 tof_replace_ps_flag = 0;
uint8_t gyro_cali_way = 0;
uint8_t acc_cali_way = 0;
int mag_threshold_for_als_calibrate = 0;
int ps_support_abs_threshold=0;
uint8_t als_support_under_screen_cali = 0;
uint8_t ps_support_cali_after_sale = 0;
uint8_t tof_register_value = 0;
extern int ltr578_flag;
extern int apds9922_flag;
int akm_cal_algo;
extern int tmd2745_flag;
extern int rpr531_flag;
extern int tsl2591_flag;
extern int bh1726_flag;
int g_bh1749_flag = 0;
uint8_t gyro_position;
extern u8 phone_color;
extern uint16_t sensorlist[SENSOR_LIST_NUM] ;
extern char sensor_chip_info[SENSOR_MAX][MAX_CHIP_INFO_LEN];
extern int rohm_rgb_flag;
extern int avago_rgb_flag;
extern int  ams_tmd3725_rgb_flag;
extern int  ams_tmd3725_ps_flag;
extern int liteon_ltr582_ps_flag;
extern int liteon_ltr582_rgb_flag;
extern int txc_ps_flag;
extern int ams_tmd2620_ps_flag;
extern int avago_apds9110_ps_flag;
extern int g_iom3_state;
extern int iom3_power_state;
extern char *sar_calibrate_order;
extern char *acc_sensor_id;
extern char *gyro_sensor_id;
extern char *mag_sensor_id;
extern char *cap_sensor_id;
extern char *sensor_in_board_status;
extern int is_cali_supported;
extern int send_para_flag;
extern int gyro_range;
extern struct charge_device_ops *g_ops;
extern uint8_t sens_name;
extern int hifi_supported;
extern int apds9999_rgb_flag;
extern int apds9999_ps_flag;
extern int  ams_tmd3702_rgb_flag;
extern int  ams_tmd3702_ps_flag;
extern int apds9253_rgb_flag;
extern int vishay_vcnl36658_als_flag;
extern int vishay_vcnl36658_ps_flag;
extern int vd6281_als_flag;
extern int ams_tof_flag;
extern int sharp_tof_flag;
extern int apds9253_006_ps_flag;
extern int ams_tcs3701_rgb_flag;
extern int ams_tcs3701_ps_flag;

extern struct CONFIG_ON_DDR* pConfigOnDDr;
extern void select_als_para(struct device_node *dn);
extern int iom3_need_recovery(int modid, exp_source_t f);
extern t_ap_sensor_ops_record all_ap_sensor_operations[TAG_SENSOR_END];
uint8_t is_close = 0;
static int _device_detect(struct device_node *dn, int index, struct sensor_combo_cfg *p_succ_ret);
int get_combo_bus_tag(const char *bus, uint8_t *tag);
int hall_number = 1 ;
int hall_sen_type = 0;
int mag_opend = 0;
int akm_need_charger_current = 0;
int akm_current_x_fac = 0;
int akm_current_y_fac = 0;
int akm_current_z_fac = 0;

#define DEF_SENSOR_COM_SETTING \
{\
	.bus_type = TAG_I2C,\
	.bus_num = 0,\
	.disable_sample_thread = 0,\
	{.data = 0}\
}

/*lint -e785*/
struct g_sensor_platform_data gsensor1_data;
struct g_sensor_platform_data gsensor2_data;
struct gyro_platform_data gyro1_data;
struct gyro_platform_data gyro2_data;
struct compass_platform_data mag1_data;
struct sar_platform_data sar1_pdata;
struct gyro_platform_data gyro_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.poll_interval = 10,
	.position = 1,//gyro position for OIS
	.axis_map_x = 1,
	.axis_map_y = 0,
	.axis_map_z = 2,
	.negate_x = 1,
	.negate_y = 0,
	.negate_z = 1,
	.gpio_int1 = 210,
	.gpio_int2 = 0,
	.gpio_int2_sh = 0,
	.fac_fix_offset_Y = 100,//100 times than real value
	.still_calibrate_threshold = 5,
	.calibrate_way = 0,
	.calibrate_thredhold = 572,// 40 dps
	.gyro_range = 2000,// 2000 dps
};

struct g_sensor_platform_data gsensor_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.poll_interval = 10,
	.axis_map_x = 1,
	.axis_map_y = 0,
	.axis_map_z = 2,
	.negate_x = 0,
	.negate_y = 1,
	.negate_z = 0,
	.used_int_pin = 2,
	.gpio_int1 = 208,
	.gpio_int2 = 0,
	.gpio_int2_sh = 0,
	.device_type = 0,
	.calibrate_style = 0,
	.calibrate_way = 0,
	.x_calibrate_thredhold = 250,// 250 mg
	.y_calibrate_thredhold = 250,// 250 mg
	.z_calibrate_thredhold = 320,// 320 mg
	.wakeup_duration = 0x60,// /*default set up 3 duration*/
};

struct compass_platform_data mag_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.poll_interval = 10,
	.outbit = 0,
	.axis_map_x = 0,
	.axis_map_y = 1,
	.axis_map_z = 2,
	.soft_filter = 0,
	.calibrate_method=1,
	.charger_trigger=0,
};

struct als_platform_data als_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.threshold_value = 1,
	.GA1 = 4166,
	.GA2 = 3000,
	.GA3 = 3750,
	.COE_B = 0,
	.COE_C = 0,
	.COE_D = 0,
	.gpio_int1 = 206,
	.atime = 0xdb,
	.again = 1,
	.poll_interval = 1000,
	.init_time = 150,
	.als_phone_type = 0,
	.als_phone_version = 0,
	.is_close = 0,
	.tp_info = 0,
};

struct ps_platform_data ps_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.min_proximity_value = 750,
	.pwindows_value = 100,
	.pwave_value = 60,
	.threshold_value = 70,
	.rdata_under_sun = 5500,
	.ps_pulse_count = 5,
	.gpio_int1 = 206,
	.persistent = 0x33,
	.ptime = 0xFF,
	.poll_interval = 250,
	.init_time = 100,
	.ps_oily_threshold = 2,
	.use_oily_judge = 0,
	.ps_calib_20cm_threshold =120,
	.ps_calib_5cm_threshold =200,
	.ps_calib_3cm_threshold =250,
	.wtime = 100,//ms
	.pulse_len = 8 ,//us
	.pgain = 4,
	.led_current = 102,//mA
	.prox_avg = 2, //ps average contrl
	.offset_max = 200 ,
	.offset_min = 50,
	.oily_max_near_pdata = 230,
	.max_oily_add_pdata = 50,
	.max_near_pdata_loop = 4,
	.oily_count_size = 12,
	.ps_tp_threshold = 40,
	.pwindows_screenon_value = 0,
	.pwave_screenon_value = 0,
	.threshold_screenon_value = 0,
	.digital_offset_max = 0,
	.is_always_on = 0,
};

struct airpress_platform_data airpress_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.poll_interval = 1000,
};

struct tof_platform_data tof_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.tof_calib_zero_threshold = 20000,
	.tof_calib_6cm_threshold = 10000,
	.tof_calib_10cm_threshold = 10000,
	.tof_calib_60cm_threshold = 10000,
};

static struct handpress_platform_data handpress_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.bootloader_type = 0,
	.poll_interval = 500,
};

struct cap_prox_platform_data cap_prox_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.init_reg_val = {
			 0x00010005, 0x00020529, 0x000300cc, 0x00040001,
			 0x00050F55,
			 0x00069905, 0x000700e8, 0x00080200, 0x00090000,
			 0x000a000C, 0x00798000,
			 0x000b9905, 0x000c00e8, 0x000d0200, 0x000e0000,
			 0x000f000C, 0x007a8000},
	.poll_interval = 200,
};

struct sar_platform_data sar_pdata = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.poll_interval = 200,
	.calibrate_type = 5,
	.gpio_int = 0,
};

struct adux_sar_add_data_t adux_sar_add_data;

static struct connectivity_platform_data connectivity_data = {
	.cfg = {
		.bus_type = TAG_I3C,
		.bus_num = 1,
		.disable_sample_thread = 1,
		{.i2c_address = 0x6E},
	},
	.poll_interval = 50,
	.gpio1_gps_cmd_ap = 200,
	.gpio1_gps_cmd_sh = 230,
	.gpio2_gps_ready_ap = 213,
	.gpio2_gps_ready_sh = 242,
	.gpio3_wakeup_gps_ap = 214,
	.gpio3_wakeup_gps_sh = 243,
};

static struct fingerprint_platform_data fingerprint_data =
{
	.cfg = {
		.bus_type = TAG_SPI,
		.bus_num = 2,
		.disable_sample_thread = 1,
		{.ctrl = {.data = 218}},
	},
	.reg = 0xFC,
	.chip_id =0x021b,
	.product_id = 9,
	.gpio_irq =207,
	.gpio_irq_sh =236,
	.gpio_reset = 149,
	.gpio_reset_sh = 1013,
	.gpio_cs = 218,
	.poll_interval =50,
};

static struct fingerprint_platform_data fingerprint_ud_data =
{
	.cfg = {
		.bus_type = TAG_SPI,
		.bus_num = 2,
		.disable_sample_thread = 1,
		{.ctrl = {.data = 218}},
	},
	.reg = 0xF1,
	.chip_id =0x1204,
	.product_id = 35,
	.gpio_irq =147,
	.gpio_irq_sh =1020,
	.gpio_reset = 211,
	.gpio_cs = 216,
	.poll_interval = 50,
	.tp_hover_support = 0,
};

static struct tp_ud_platform_data tp_ud_data = {
	.cfg = {
		.bus_type = TAG_I3C,
		.bus_num = 0,
		.disable_sample_thread = 1,
		{.i2c_address = 0x70},
	},
	.gpio_irq = 226,
	.gpio_irq_sh = 1000,
	.gpio_cs  = 236,
	.pressure_support = 1,
	.anti_forgery_support = 0,
	.spi_max_speed_hz = 10000000,
	.spi_mode = 0,
	.ic_type = 0,
	.hover_enable = 1,
	.i2c_max_speed_hz = 0,
};

static struct key_platform_data key_data = {
	.cfg = {
		.bus_type = TAG_I2C,
		.bus_num = 0,
		.disable_sample_thread = 0,
		{.i2c_address = 0x27},
	},
	.i2c_address_bootloader = 0x28,
	.poll_interval = 30,
};
struct vibrator_paltform_data vibrator_data = {
	.cfg = {
		.bus_type = TAG_I2C,
		.bus_num = 0,
		.disable_sample_thread = 0,
		{.i2c_address = 0x5A},
	},
	.max_timeout_ms = 10000,
	.reduce_timeout_ms = 0,
	.skip_lra_autocal =1,
};
struct magn_bracket_platform_data magn_bracket_data = {
	.cfg = DEF_SENSOR_COM_SETTING, /*donot use, just give it a value*/
	.mag_x_change_lower = 0,
	.mag_x_change_upper = 0,
	.mag_y_change_lower = 0,
	.mag_y_change_upper = 0,
	.mag_z_change_lower = 0,
	.mag_z_change_upper = 0,
};
struct aod_platform_data aod_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.phone_type = 0,
};
struct rpc_platform_data rpc_data={
	.table = {0,},
	.mask = {0,},
	.default_value = 0,
	.mask_enable = 0,
};

struct kb_platform_data g_kb_data = {
	.uart_num = KB_DEFAULT_UART_NUM,
	.kb_detect_adc_num = KB_DEFAULT_DETECT_ADC_NUM,
	.kb_disconnect_adc_min = KB_DEFAULT_DISCONNECT_ADC_VOL,
	.kb_disable_angle = 0,
};

struct sar_sensor_detect semtech_sar_detect = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.detect_flag = 0,
	.chip_id = 0,
};

struct sar_sensor_detect semtech_9335_sar_detect = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.detect_flag = 0,
	.chip_id = 0,
};

struct sar_sensor_detect adi_sar_detect = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.detect_flag = 0,
	.chip_id = 0,
};

struct sar_sensor_detect cypress_sar_detect = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.detect_flag = 0,
	.chip_id = 0,
};

struct sar_sensor_detect g_abov_sar_detect = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.detect_flag = 0,
	.chip_id = 0,
};
/*lint +e785*/
struct sensor_detect_manager sensor_manager[SENSOR_MAX] = {
    {"acc", ACC,DET_INIT,TAG_ACCEL, &gsensor_data, sizeof(gsensor_data)},
    {"mag", MAG,DET_INIT,TAG_MAG, &mag_data, sizeof(mag_data)},
    {"gyro", GYRO,DET_INIT,TAG_GYRO, &gyro_data, sizeof(gyro_data)},
    {"als", ALS,DET_INIT,TAG_ALS, &als_data, sizeof(als_data)},
    {"ps", PS,DET_INIT,TAG_PS, &ps_data, sizeof(ps_data)},
    {"airpress", AIRPRESS,DET_INIT,TAG_PRESSURE, &airpress_data, sizeof(airpress_data)},
    {"handpress", HANDPRESS,DET_INIT,TAG_HANDPRESS, &handpress_data, sizeof(handpress_data)},
    {"cap_prox", CAP_PROX,DET_INIT,TAG_CAP_PROX, &sar_pdata, sizeof(sar_pdata)},
    {"connectivity", CONNECTIVITY,DET_INIT,TAG_CONNECTIVITY,&connectivity_data,sizeof(connectivity_data)},
    {"fingerprint", FINGERPRINT, DET_INIT, TAG_FP, &fingerprint_data, sizeof(fingerprint_data)},
    {"key", KEY, DET_INIT, TAG_KEY, &key_data, sizeof(key_data)},
    {"hw_magn_bracket", MAGN_BRACKET, DET_INIT,TAG_MAGN_BRACKET,&magn_bracket_data,sizeof(magn_bracket_data)},
    {"rpc", RPC, DET_INIT,TAG_RPC, &rpc_data, sizeof(rpc_data)},
    {"vibrator", VIBRATOR, DET_INIT,TAG_VIBRATOR, &vibrator_data, sizeof(vibrator_data)},
    {"fingerprint_ud", FINGERPRINT_UD, DET_INIT, TAG_FP_UD, &fingerprint_ud_data, sizeof(fingerprint_ud_data)},
    {"tof", TOF, DET_INIT,TAG_TOF, &tof_data, sizeof(tof_data)},
    {"tp_ud", TP_UD, DET_INIT, TAG_TP, &tp_ud_data, sizeof(tp_ud_data)},
	{"sh_aod", SH_AOD, DET_INIT, TAG_AOD, &aod_data, sizeof(aod_data)},
    {"acc1", ACC1,DET_INIT,TAG_ACC1, &gsensor1_data, sizeof(gsensor1_data)},
    {"gyro1", GYRO1,DET_INIT,TAG_GYRO1, &gyro1_data, sizeof(gyro1_data)},
    {"acc2", ACC2,DET_INIT,TAG_ACC2, &gsensor2_data, sizeof(gsensor2_data)},
    {"gyro2", GYRO2,DET_INIT,TAG_GYRO2, &gyro2_data, sizeof(gyro2_data)},
    {"mag1", MAG1,DET_INIT,TAG_MAG1, &mag1_data, sizeof(mag1_data)},
    {"cap_prox1", CAP_PROX1,DET_INIT,TAG_CAP_PROX1, &sar1_pdata, sizeof(sar1_pdata)},
	{ "kb", KB, DET_INIT, TAG_KB, &g_kb_data, sizeof(g_kb_data) },
};

SENSOR_DETECT_LIST get_id_by_sensor_tag(int tag)
{
	int i = 0;
	for(i = 0; i < SENSOR_MAX; i++)
	{
		if(sensor_manager[i].tag == tag)
			break;
	}
	return i;
}

void read_sensorlist_info(struct device_node *dn, int sensor)
{
	int temp = 0;
	char *chip_info = NULL;

	if(0 <= of_property_read_string(dn, "sensorlist_name", (const char **)&chip_info))
	{
		strncpy(sensorlist_info[sensor].name, chip_info, MAX_CHIP_INFO_LEN-1);
		sensorlist_info[sensor].name[MAX_CHIP_INFO_LEN-1] = '\0';
		hwlog_info("sensor chip info name %s\n", chip_info);
		hwlog_info("sensor SENSOR_DETECT_LIST %d get name %s\n", sensor, sensorlist_info[sensor].name);
	}
	else
		sensorlist_info[sensor].name[0] = '\0';

	if(0 == of_property_read_string(dn, "vendor", (const char **)&chip_info))
	{
		strncpy(sensorlist_info[sensor].vendor, chip_info, MAX_CHIP_INFO_LEN-1);
		sensorlist_info[sensor].vendor[MAX_CHIP_INFO_LEN-1] = '\0';
		hwlog_info("sensor SENSOR_DETECT_LIST %d get vendor %s\n", sensor, sensorlist_info[sensor].vendor);
	}
	else
		sensorlist_info[sensor].vendor[0] = '\0';

	if (0 == of_property_read_u32(dn, "version", &temp))
	{
		sensorlist_info[sensor].version= temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get version %d\n", sensor, temp);
	}
	else
		sensorlist_info[sensor].version = -1;
	if (0 == of_property_read_u32(dn, "maxRange", &temp))
	{
		sensorlist_info[sensor].maxRange= temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get maxRange %d\n", sensor, temp);
	}
	else
		sensorlist_info[sensor].maxRange = -1;
	if (0 == of_property_read_u32(dn, "resolution", &temp))
	{
		sensorlist_info[sensor].resolution= temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get resolution %d\n", sensor, temp);
	}
	else
		sensorlist_info[sensor].resolution = -1;
	if (0 == of_property_read_u32(dn, "power", &temp))
	{
		sensorlist_info[sensor].power= temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get power %d\n", sensor, temp);
	}
	else
		sensorlist_info[sensor].power = -1;
	if (0 == of_property_read_u32(dn, "minDelay", &temp))
	{
		sensorlist_info[sensor].minDelay= temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get minDelay %d\n", sensor, temp);
	}
	else
		sensorlist_info[sensor].minDelay = -1;
	if (0 == of_property_read_u32(dn, "fifoReservedEventCount", &temp))
	{
		sensorlist_info[sensor].fifoReservedEventCount= temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get fifoReservedEventCount %d\n", sensor, temp);
	}
	else
		sensorlist_info[sensor].fifoReservedEventCount = -1;
	if (0 == of_property_read_u32(dn, "fifoMaxEventCount", &temp))
	{
		sensorlist_info[sensor].fifoMaxEventCount= temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get fifoMaxEventCount %d\n", sensor, temp);
	}
	else
		sensorlist_info[sensor].fifoMaxEventCount = -1;
	if (0 == of_property_read_u32(dn, "maxDelay", &temp))
	{
		sensorlist_info[sensor].maxDelay= temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get maxDelay %d\n", sensor, temp);
	}
	else
		sensorlist_info[sensor].maxDelay = -1;
	if (0 == of_property_read_u32(dn, "flags", &temp))
	{
		sensorlist_info[sensor].flags= temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get flags %d\n", sensor, temp);
	}
	else
		sensorlist_info[sensor].flags = -1;
	return;
}

void read_chip_info(struct device_node *dn, SENSOR_DETECT_LIST sname)
{
	char *chip_info = NULL;
	int ret = 0;

	ret = of_property_read_string(dn, "compatible", (const char **)&chip_info);
	if (ret)
		hwlog_err("%s:read name_id:%d info fail\n", __func__, sname);
	else
		strncpy(sensor_chip_info[sname], chip_info, MAX_CHIP_INFO_LEN - 1);
	hwlog_info("get chip info from dts success. sensor name=%s\n", sensor_chip_info[sname]);
}

static void read_aux_file_list(uint16_t fileid, uint16_t tag)
{
	aux_req->file_list[aux_req->file_count * 2] = fileid;
	aux_req->file_list[aux_req->file_count * 2 + 1] = tag;
	aux_req->file_count++;
}

static void read_acc_data_from_dts(struct device_node *dn, struct sensor_detect_manager *sm)
{
	int temp = 0, aux = 0;
	const char *bus_type;
	struct g_sensor_platform_data *gsensor_data = (struct g_sensor_platform_data *)sm->spara;

	read_chip_info(dn, sm->sensor_id);

	if(of_property_read_u32(dn, "used_int_pin", &temp))
		hwlog_err("%s:read %s used_int_pin fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->used_int_pin = (uint8_t) temp;

	temp = of_get_named_gpio(dn, "gpio_int1", 0);
	if (temp < 0)
		hwlog_err("%s:read %s gpio_int1 fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->gpio_int1 = (GPIO_NUM_TYPE) temp;

	temp = of_get_named_gpio(dn, "gpio_int2", 0);
	if (temp < 0)
		hwlog_err("%s:read %s gpio_int2 fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->gpio_int2 = (GPIO_NUM_TYPE) temp;

	if (of_property_read_u32(dn, "gpio_int2_sh", &temp))
		hwlog_err("%s:read %s gpio_int2_sh fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->gpio_int2_sh = (GPIO_NUM_TYPE) temp;

	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read %s poll_interval fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->poll_interval = (uint16_t) temp;

	if (of_property_read_u32(dn, "calibrate_style", &temp))
		hwlog_err("%s:read %s calibrate_style fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->calibrate_style = (uint8_t) temp;

	if (of_property_read_u32(dn, "axis_map_x", &temp))
		hwlog_err("%s:read %s axis_map_x fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->axis_map_x = (uint8_t) temp;

	if (of_property_read_u32(dn, "axis_map_y", &temp))
		hwlog_err("%s:read %s axis_map_y fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->axis_map_y = (uint8_t) temp;

	if (of_property_read_u32(dn, "axis_map_z", &temp))
		hwlog_err("%s:read %s axis_map_z fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->axis_map_z = (uint8_t) temp;

	if (of_property_read_u32(dn, "negate_x", &temp))
		hwlog_err("%s:read %s negate_x fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->negate_x = (uint8_t) temp;

	if (of_property_read_u32(dn, "negate_y", &temp))
		hwlog_err("%s:read %s negate_y fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->negate_y = (uint8_t) temp;

	if (of_property_read_u32(dn, "negate_z", &temp))
		hwlog_err("%s:read %s negate_z fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->negate_z = (uint8_t) temp;

        if (of_property_read_u32(dn, "device_type", &temp))
                hwlog_err("%s:read %s device_type fail\n", __func__, sm->sensor_name_str);
        else
                gsensor_data->device_type = (uint8_t) temp;

	if (of_property_read_u32(dn, "x_calibrate_thredhold", &temp))
		hwlog_err("%s:read %s x_calibrate_thredhold fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->x_calibrate_thredhold = (uint16_t) temp;

	if (of_property_read_u32(dn, "y_calibrate_thredhold", &temp))
		hwlog_err("%s:read %s y_calibrate_thredhold fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->y_calibrate_thredhold = (uint16_t) temp;

	if (of_property_read_u32(dn, "z_calibrate_thredhold", &temp))
		hwlog_err("%s:read %s z_calibrate_thredhold fail\n", __func__, sm->sensor_name_str);
	else
		gsensor_data->z_calibrate_thredhold = (uint16_t) temp;


/* i2c_address should be set when detect success! not here
	if (of_property_read_u32(dn, "reg", &temp))
		hwlog_warn("%s:read acc reg fail\n", __func__);
	else
		gsensor_data.cfg.i2c_address = (uint8_t) temp;
*/
	if (of_property_read_u32(dn, "aux_prop", &aux))
		hwlog_err("%s:read %s aux_prop fail\n", __func__, sm->sensor_name_str);
	else
		hwlog_info("%s:read %s aux_prop %d\n", __func__, sm->sensor_name_str, aux);

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read %s file_id fail\n", __func__, sm->sensor_name_str);
	else if (aux)
		read_aux_file_list(temp, sm->tag);
	else if (!aux) {
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;
		dyn_req->file_count++;
	}

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read %s sensor_list_info_id fail\n", __func__, sm->sensor_name_str);
	else
		sensorlist[++sensorlist[0]] = (uint16_t) temp;
	if (of_property_read_u32(dn, "calibrate_way", &temp))
		hwlog_err("%s:read %s calibrate_way fail\n", __func__, sm->sensor_name_str);
	else {
		gsensor_data->calibrate_way = (uint8_t) temp;
		acc_cali_way = ((ACC == sm->sensor_id) ? (uint8_t) temp : acc_cali_way);
	}

	if (of_property_read_u32(dn, "wakeup_duration", &temp))
		hwlog_err("%s:read %s wakeup_duration fail\n", __func__, sm->sensor_name_str);
	else {
		gsensor_data->wakeup_duration = (uint8_t) temp;
	}

	if (of_property_read_string(dn, "bus_type", &bus_type)) {
		hwlog_warn("%s:acc %s bus_type not configured\n", __func__, sm->sensor_name_str);
	} else {
		if (get_combo_bus_tag(bus_type, (uint8_t *)&temp)) {
			hwlog_warn("%s:acc %s bus_type invalid\n.", __func__, sm->sensor_name_str);
		} else {
			gsensor_data->cfg.bus_type = temp;
		}
	}

	if (of_property_read_u32(dn, "bus_number", &temp)) {
		hwlog_warn("%s:acc %s bus_number not configured\n", __func__, sm->sensor_name_str);
	} else {
		gsensor_data->cfg.bus_num = temp;
	}

	if (gsensor_data->cfg.bus_type == TAG_I2C || gsensor_data->cfg.bus_type == TAG_I3C) {
		if (of_property_read_u32(dn, "i2c_address", &temp)) {
			hwlog_warn("%s:acc %s i2c_address not configured\n", __func__, sm->sensor_name_str);
		} else {
			gsensor_data->cfg.i2c_address = temp;
		}
	}

	if (of_property_read_string(dn, "sensor_in_board_status", (const char**)&sensor_in_board_status)) {
		hwlog_err("read sensor_in_board_status order err.\n");
	}
	hwlog_info("sensor_in_board_status order:%s\n", sensor_in_board_status);

	read_sensorlist_info(dn, sm->sensor_id);
}

static void read_mag_data_from_dts(struct device_node *dn, struct sensor_detect_manager *sm)
{
	int temp = 0, aux = 0;
	struct compass_platform_data *mag_data = (struct compass_platform_data *)sm->spara;

	read_chip_info(dn, sm->sensor_id);

	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read %s mag poll_interval fail\n", __func__, sm->sensor_name_str);
	else
		mag_data->poll_interval = (uint16_t) temp;

	if (of_property_read_u32(dn, "axis_map_x", &temp))
		hwlog_err("%s:read %s mag axis_map_x fail\n", __func__, sm->sensor_name_str);
	else
		mag_data->axis_map_x = (uint8_t) temp;

	if (of_property_read_u32(dn, "axis_map_y", &temp))
		hwlog_err("%s:read %s mag axis_map_y fail\n", __func__, sm->sensor_name_str);
	else
		mag_data->axis_map_y = (uint8_t) temp;

	if (of_property_read_u32(dn, "axis_map_z", &temp))
		hwlog_err("%s:read %s mag axis_map_z fail\n", __func__, sm->sensor_name_str);
	else
		mag_data->axis_map_z = (uint8_t) temp;

	if (of_property_read_u32(dn, "negate_x", &temp))
		hwlog_err("%s:read %s mag negate_x fail\n", __func__, sm->sensor_name_str);
	else
		mag_data->negate_x = (uint8_t) temp;

	if (of_property_read_u32(dn, "negate_y", &temp))
		hwlog_err("%s:read %s mag negate_y fail\n", __func__, sm->sensor_name_str);
	else
		mag_data->negate_y = (uint8_t) temp;

	if (of_property_read_u32(dn, "negate_z", &temp))
		hwlog_err("%s:read %s mag negate_z fail\n", __func__, sm->sensor_name_str);
	else
		mag_data->negate_z = (uint8_t) temp;

	if (of_property_read_u32(dn, "outbit", &temp))
		hwlog_err("%s:read %s mag outbit fail\n", __func__, sm->sensor_name_str);
	else
		mag_data->outbit = (uint8_t) temp;

	if (of_property_read_u32(dn, "softfilter", &temp))
		hwlog_err("%s:read %s mag softfilter fail; use default value:%d\n", __func__, sm->sensor_name_str, mag_data->soft_filter);
	else
		mag_data->soft_filter = (uint8_t) temp;

	if (of_property_read_u32(dn, "calibrate_method", &temp))
		hwlog_err("%s:read %s mag calibrate_method fail\n", __func__, sm->sensor_name_str);
	else
		mag_data->calibrate_method = (uint8_t) temp;

	if (of_property_read_u32(dn, "charger_trigger", &temp))
		hwlog_err("%s:read %s mag charger_trigger fail; use default value:%d\n", __func__, sm->sensor_name_str, mag_data->charger_trigger);
	else
		mag_data->charger_trigger = (uint8_t)temp;

	if (of_property_read_u32(dn, "threshold_for_als_calibrate", &temp))
		hwlog_err("%s:read %s mag threshold_for_als_calibrate fail\n", __func__, sm->sensor_name_str);
	else
		mag_threshold_for_als_calibrate =  temp;

	if (of_property_read_u32(dn, "akm_cal_algo", &temp)) {
		hwlog_err("%s:read %s mag akm_cal_algo fail\n", __func__, sm->sensor_name_str);
		akm_cal_algo = 0;
	} else {
		if (1 == temp) {
			akm_cal_algo = 1;
		} else {
			akm_cal_algo = 0;
		}
		hwlog_info("%s: mag %s akm_cal_algo=%d.\n", __func__, sm->sensor_name_str, akm_cal_algo);
	}

/* i2c_address should be set when detect success! not here
	if (of_property_read_u32(dn, "reg", &temp))
		hwlog_err("%s:read mag reg fail\n", __func__);
	else
		mag_data.cfg.i2c_address = (uint8_t) temp;
*/
	if (of_property_read_u32(dn, "aux_prop", &aux))
		hwlog_err("%s:read %s aux_prop fail\n", __func__, sm->sensor_name_str);
	else
		hwlog_info("%s:read %s aux_prop %d\n", __func__, sm->sensor_name_str, aux);

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read %s file_id fail\n", __func__, sm->sensor_name_str);
	else if (aux)
		read_aux_file_list(temp, sm->tag);
	else if (!aux) {
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;
		dyn_req->file_count++;
	}

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read %s mag sensor_list_info_id fail\n", __func__, sm->sensor_name_str);
	else
		sensorlist[++sensorlist[0]] = (uint16_t) temp;

	temp = of_get_named_gpio(dn, "gpio_reset", 0);
	if (temp < 0)
		hwlog_err("%s:read %s gpio_rst fail\n", __func__, sm->sensor_name_str);
	else
		mag_data->gpio_rst = (GPIO_NUM_TYPE) temp;

	if (of_property_read_u32(dn, "akm_need_charger_current", &temp)) {
		hwlog_info("%s:read mag akm_need_charger_current fail\n", __func__);
		akm_need_charger_current = FALSE;
	} else {
		if (TRUE== temp) {
			akm_need_charger_current = TRUE;
		} else {
			akm_need_charger_current = FALSE;
		}
		hwlog_info("%s: mag akm_need_charger_current=%d.\n", __func__,
			   akm_need_charger_current);
	}
	if (of_property_read_u32(dn, "akm_current_x_fac", &temp)) {
		hwlog_info("%s:read mag akm_current_x_fac fail\n", __func__);
		akm_current_x_fac = CURRENT_FAC_AKM_INIT;
	} else {
		akm_current_x_fac = temp;
		hwlog_info("%s: mag akm_current_x_fac=%d.\n", __func__,
			   akm_current_x_fac);
	}
	if (of_property_read_u32(dn, "akm_current_y_fac", &temp)) {
		hwlog_info("%s:read mag akm_current_y_fac fail\n", __func__);
		akm_current_y_fac = CURRENT_FAC_AKM_INIT;
	} else {
		akm_current_y_fac = temp;
		hwlog_info("%s: mag akm_current_y_fac=%d.\n", __func__,
			   akm_current_y_fac);
	}
	if (of_property_read_u32(dn, "akm_current_z_fac", &temp)) {
		hwlog_info("%s:read mag akm_current_z_fac fail\n", __func__);
		akm_current_z_fac = CURRENT_FAC_AKM_INIT;
	} else {
		akm_current_z_fac = temp;
		hwlog_info("%s: mag akm_current_z_fac=%d.\n", __func__,
			   akm_current_z_fac);
	}
	read_sensorlist_info(dn, MAG);
}

static void read_gyro_data_from_dts(struct device_node *dn, struct sensor_detect_manager *sm)
{
	const char *bus_type;
	int temp = 0, aux = 0;
	struct gyro_platform_data *gyro_data = (struct gyro_platform_data *)sm->spara;

	read_chip_info(dn, sm->sensor_id);

	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read %s mag poll_interval fail\n", __func__, sm->sensor_name_str);
	else
		gyro_data->poll_interval = (uint16_t) temp;

	if (of_property_read_u32(dn, "position", &temp)){
		gyro_position = gyro_data->position;
		hwlog_err("%s:read %s gyro position fail,use default position =%d\n", __func__, sm->sensor_name_str,gyro_position);
	}
	else{
		gyro_data->position = (uint8_t) temp;
		gyro_position = gyro_data->position;
		hwlog_info("%s:read %s gyro position suc position=%d\n", __func__, sm->sensor_name_str,gyro_position);
	}

	if (of_property_read_u32(dn, "axis_map_x", &temp))
		hwlog_err("%s:read %s gyro axis_map_x fail\n", __func__, sm->sensor_name_str);
	else
		gyro_data->axis_map_x = (uint8_t) temp;

	if (of_property_read_u32(dn, "axis_map_y", &temp))
		hwlog_err("%s:read %s gyro axis_map_y fail\n", __func__, sm->sensor_name_str);
	else
		gyro_data->axis_map_y = (uint8_t) temp;

	if (of_property_read_u32(dn, "axis_map_z", &temp))
		hwlog_err("%s:read %s gyro axis_map_z fail\n", __func__, sm->sensor_name_str);
	else
		gyro_data->axis_map_z = (uint8_t) temp;

	if (of_property_read_u32(dn, "negate_x", &temp))
		hwlog_err("%s:read %s gyro negate_x fail\n", __func__, sm->sensor_name_str);
	else
		gyro_data->negate_x = (uint8_t) temp;

	if (of_property_read_u32(dn, "negate_y", &temp))
		hwlog_err("%s:read %s gyro negate_y fail\n", __func__, sm->sensor_name_str);
	else
		gyro_data->negate_y = (uint8_t) temp;

	if (of_property_read_u32(dn, "negate_z", &temp))
		hwlog_err("%s:read %s gyro negate_z fail\n", __func__, sm->sensor_name_str);
	else
		gyro_data->negate_z = (uint8_t) temp;

	if (of_property_read_u32(dn, "still_calibrate_threshold", &temp))
		hwlog_err("%s:read %s gyro still_calibrate_threshold fail\n", __func__, sm->sensor_name_str);
	else
		gyro_data->still_calibrate_threshold = (uint8_t) temp;

	if (of_property_read_u32(dn, "aux_prop", &aux))
		hwlog_err("%s:read %s aux_prop fail\n", __func__, sm->sensor_name_str);
	else
		hwlog_info("%s:read %s aux_prop %d\n", __func__, sm->sensor_name_str, aux);

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read %s file_id fail\n", __func__, sm->sensor_name_str);
	else if (aux)
		read_aux_file_list(temp, sm->tag);
	else if (!aux) {
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;
		dyn_req->file_count++;
	}

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read %s gyro sensor_list_info_id fail\n", __func__, sm->sensor_name_str);
	else
		sensorlist[++sensorlist[0]] = (uint16_t) temp;

	temp = of_get_named_gpio(dn, "gpio_int1", 0);
	if (temp < 0)
		hwlog_err("%s:read %s gpio_int1 fail\n", __func__, sm->sensor_name_str);
	else
		gyro_data->gpio_int1 = (GPIO_NUM_TYPE) temp;

	if (of_property_read_u32(dn, "fac_fix_offset_Y", &temp))
		hwlog_info("%s:read %s fac_fix_offset_Y fail\n", __func__, sm->sensor_name_str);
	else{
		gyro_data->fac_fix_offset_Y = (uint8_t) temp;
		hwlog_debug("%s:read %s acc fac_fix_offset_Y %d\n", __func__, sm->sensor_name_str, temp);
	}
	if (of_property_read_u32(dn, "calibrate_way", &temp))
		hwlog_err("%s:read %s gyro calibrate_way fail\n", __func__, sm->sensor_name_str);
	else {
		gyro_data->calibrate_way = (uint8_t) temp;
		gyro_cali_way = (uint8_t) temp;
	}

	if (of_property_read_u32(dn, "calibrate_thredhold", &temp))
		hwlog_info("%s:read %s calibrate_thredhold fail\n", __func__, sm->sensor_name_str);
	else{
		gyro_data->calibrate_thredhold = (uint16_t) temp;
		hwlog_debug("%s:read %s gyro calibrate_thredhold %d\n", __func__, sm->sensor_name_str, temp);
	}

	if (of_property_read_u32(dn, "gyro_range", &temp)){
		hwlog_debug("%s:read %s gyro_range fail\n", __func__, sm->sensor_name_str);
	}
	else{
		gyro_data->gyro_range = (uint16_t) temp;
		gyro_range = gyro_data->gyro_range;
		hwlog_info("%s:read %s gyro gyro_range %d\n", __func__, sm->sensor_name_str, temp);
	}

	if (of_property_read_string(dn, "bus_type", &bus_type)) {
		hwlog_warn("%s:gyro %s bus_type not configured\n", __func__, sm->sensor_name_str);
	} else {
		if (get_combo_bus_tag(bus_type, (uint8_t *)&temp)) {
			hwlog_warn("%s:gyro %s bus_type invalid\n.", __func__, sm->sensor_name_str);
		} else {
			gyro_data->cfg.bus_type = temp;
		}
	}

	if (of_property_read_u32(dn, "bus_number", &temp)) {
		hwlog_warn("%s:gyro %s bus_number not configured\n", __func__, sm->sensor_name_str);
	} else {
		gyro_data->cfg.bus_num = temp;
	}

	if (gyro_data->cfg.bus_type == TAG_I2C || gyro_data->cfg.bus_type == TAG_I3C) {
		if (of_property_read_u32(dn, "i2c_address", &temp)) {
			hwlog_warn("%s:gyro %s i2c_address not configured\n", __func__, sm->sensor_name_str);
		} else {
			gyro_data->cfg.i2c_address = temp;
		}
	}

	read_sensorlist_info(dn, sm->sensor_id);
}

void read_als_data_from_dts(struct device_node *dn)
{
	int temp = 0;
	int als_phone_type = 0;
	int als_phone_version = 0;
	int ret = 0;
	char *chip_info = NULL;
	read_chip_info(dn, ALS);
	temp = of_property_read_string(dn, "compatible", (const char **)&chip_info);
	if (temp < 0)
		hwlog_err("%s:read als poll_interval fail\n", __func__);
	else
		strncpy(sensor_chip_info[ALS], chip_info, MAX_CHIP_INFO_LEN - 1);

	if (!strncmp(chip_info, "huawei,rohm_bh1745", sizeof("huawei,rohm_bh1745"))) {
		rohm_rgb_flag = 1;
		hwlog_err("%s:rohm_bh1745 i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(chip_info, "huawei,rohm_bh1749", sizeof("huawei,rohm_bh1749"))) {
		g_bh1749_flag = 1;
		hwlog_err("%s:rohm_bh1749 i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(chip_info, "huawei,avago_apds9251", sizeof("huawei,avago_apds9251"))) {
		avago_rgb_flag = 1;
		hwlog_err("%s:avago_apds9251 i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(chip_info, "huawei,ams_tmd3725",sizeof("huawei,ams_tmd3725"))) {
		ams_tmd3725_rgb_flag = 1;
		hwlog_err("%s:ams_tmd3725 i2c_address suc,%d \n", __func__,temp);
	}

	if (!strncmp(chip_info, "huawei,ams_tcs3701",sizeof("huawei,ams_tcs3701"))) {
		ams_tcs3701_rgb_flag = 1;
		hwlog_err("%s:ams_tcs3701 i2c_address suc,%d \n", __func__,temp);
	}

	if (!strncmp(chip_info, "huawei,liteon_ltr582",sizeof("huawei,liteon_ltr582"))) {
		liteon_ltr582_rgb_flag = 1;
		hwlog_err("%s:liteon_ltr582_als i2c_address suc,%d \n", __func__,temp);
	}


	if (!strncmp(chip_info, "huawei,ltr578_als", sizeof("huawei,ltr578_als"))) {
		sens_name = LTR578;
		ltr578_flag = 1;
		hwlog_err("%s:ltr578_als i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(chip_info, "huawei,apds9922_als", sizeof("huawei,apds9922_als"))) {
		sens_name = APDS9922;
		apds9922_flag = 1;
		hwlog_err("%s:apds9922_als i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(chip_info, "huawei,tmd2745_als", sizeof("huawei,tmd2745_als"))) {
		tmd2745_flag = 1;
		hwlog_err("%s:tmd2745_als i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(chip_info, "huawei,rohm_rpr531_als", sizeof("huawei,rohm_rpr531_als"))) {
		rpr531_flag= true;
		hwlog_info("%s:rpr531_als i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(chip_info, "huawei,avago_apds9999", sizeof("huawei,avago_apds9999"))) {
		apds9999_rgb_flag= 1;
		hwlog_info("%s:avago_apds9999 i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(chip_info, "huawei,ams_tmd3702", sizeof("huawei,ams_tmd3702"))) {
		ams_tmd3702_rgb_flag= 1;
		hwlog_info("%s:huawei,ams_tmd3702 i2c_address suc,%d \n", __func__, temp);
	}
	if (!strncmp(chip_info, "huawei,avago_apds9253", sizeof("huawei,avago_apds9253"))) {
		apds9253_rgb_flag= 1;
		hwlog_info("%s:huawei,avago_apds9253 i2c_address suc,%d \n", __func__, temp);
	}
	if (!strncmp(chip_info, "huawei,vishay_vcnl36658", sizeof("huawei,vishay_vcnl36658"))) {
		vishay_vcnl36658_als_flag= 1;
		hwlog_info("%s:vishay_vcnl36658 i2c_address suc,%d \n", __func__, temp);
	}
	if (!strncmp(chip_info, "huawei,vishay_vcnl36832", sizeof("huawei,vishay_vcnl36832"))) {
		vishay_vcnl36832_als_flag = 1;
	}
	if (!strncmp(chip_info, "huawei,arrow_stk3338", sizeof("huawei,arrow_stk3338"))) {
		stk3338_als_flag = 1;
	}
	if (!strncmp(chip_info, "huawei,liteon_ltr2568", sizeof("huawei,liteon_ltr2568"))) {
		ltr2568_als_flag = 1;
	}
	if (!strncmp(chip_info, "huawei,st_vd6281", sizeof("huawei,st_vd6281"))) {
		vd6281_als_flag= 1;
		hwlog_info("%s:st_vd6281 i2c_address suc,%d \n", __func__, temp);
	}
	if (!strncmp(chip_info, "huawei,A", sizeof("huawei,A"))) {
		tsl2591_flag= 1;
		hwlog_info("%s:A i2c_address suc,%d \n", __func__, temp);
	}
	if (!strncmp(chip_info, "huawei,B", sizeof("huawei,B"))) {
		bh1726_flag= 1;
		hwlog_info("%s:B i2c_address suc,%d \n", __func__, temp);
	}
	temp = of_get_named_gpio(dn, "gpio_int1", 0);
	if (temp < 0)
		hwlog_err("%s:read gpio_int1 fail\n", __func__);
	else
		als_data.gpio_int1 = (GPIO_NUM_TYPE) temp;

	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read als poll_interval fail\n", __func__);
	else
		als_data.poll_interval = (uint16_t) temp;

	if (of_property_read_u32(dn, "reg", &temp))
		hwlog_err("%s:read als reg fail\n",__func__);
	else
		als_data.cfg.i2c_address = (uint8_t)temp;

	if (of_property_read_u32(dn, "init_time", &temp))
		hwlog_err("%s:read als init time fail\n", __func__);
	else
		als_data.init_time = (uint16_t) temp;

	if (of_property_read_u32(dn, "GA1", &temp))
		hwlog_err("%s:read als ga1 fail\n", __func__);
	else
		als_data.GA1 = temp;

	if (of_property_read_u32(dn, "GA2", &temp))
		hwlog_err("%s:read als ga2 fail\n", __func__);
	else
		als_data.GA2 = temp;

	if (of_property_read_u32(dn, "GA3", &temp))
		hwlog_err("%s:read als ga3 fail\n", __func__);
	else
		als_data.GA3 = temp;

	if (of_property_read_u32(dn, "als_phone_type", &als_phone_type))
		hwlog_err("%s:read als_phone_type fail\n", __func__);
	else
		als_data.als_phone_type = (uint8_t) als_phone_type;

	if (of_property_read_u32(dn, "als_phone_version", &als_phone_version))
		hwlog_err("%s:read als_phone_version fail\n", __func__);
	else
		als_data.als_phone_version = (uint8_t) als_phone_version;
	als_data.als_phone_tp_colour = phone_color;

	if (of_property_read_u32(dn, "atime", &temp))
		hwlog_err("%s:read als atime fail\n", __func__);
	else
		als_data.atime = (uint8_t) temp;

	if (of_property_read_u32(dn, "again", &temp))
		hwlog_err("%s:read als again fail\n", __func__);
	else
		als_data.again = (uint8_t) temp;

	if (of_property_read_u32(dn, "is_cali_supported", &temp))
		hwlog_err("%s:read als is_cali_supported fail\n", __func__);
	else
		is_cali_supported = (int)temp;

	if (of_property_read_u32(dn, "is_close", &temp))
		hwlog_err("%s:read als is_close fail\n", __func__);
	else
		als_data.is_close = (int)temp;

	if (of_property_read_u32(dn, "als_cali_after_sale", &temp))
		hwlog_err("%s:read als_cali_after_sale fail\n", __func__);
	else
		als_support_under_screen_cali = (uint8_t)temp;

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read als file_id fail\n", __func__);
	else
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;
	if (of_property_read_u32(dn, "is_bllevel_supported", &temp))
		hwlog_err("%s:read als is_bllevel_supported fail\n", __func__);
	else
		als_data.is_bllevel_supported = (int)temp;
	dyn_req->file_count++;
	read_sensorlist_info(dn, ALS);
	ret = of_property_read_u32(dn, "phone_color_num", &temp);
	if (0 == ret){
		if(temp < MAX_PHONE_COLOR_NUM){
			ret = of_property_read_u32_array(dn, "sleeve_detect_threshhold", (uint32_t *)sleeve_detect_paremeter, temp * 2);
			if (ret) {
				hwlog_err("%s: read sleeve_detect_threshhold failed!\n", __func__);
			}
		}
	}else{
		hwlog_err("%s:read phone_color_num fail(%d)\n", __func__, ret);
	}

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read als sensor_list_info_id fail\n", __func__);
	else
		sensorlist[++sensorlist[0]] = (uint16_t) temp;

	select_als_para(dn);
}

static void read_ps_data_from_dts(struct device_node *dn)
{
	int temp = 0;

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read ps sensor_list_info_id fail\n", __func__);
	else
		sensorlist[++sensorlist[0]] = (uint16_t) temp;

	if(tof_replace_ps_flag){
		hwlog_info("tof_replace_ps_flag is true skip read ps dtsconfig %d\n", tof_replace_ps_flag);
		return;
	}

	read_chip_info(dn, PS);
	if (!strncmp(sensor_chip_info[PS], "huawei,txc-pa224", sizeof("huawei,txc-pa224"))) {
		txc_ps_flag = 1;
		hwlog_err("%s:txc_ps i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(sensor_chip_info[PS], "huawei,ams-tmd2620", sizeof("huawei,ams-tmd2620"))) {
		ams_tmd2620_ps_flag = 1;
		hwlog_err("%s:ams_tmd2620_ps i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(sensor_chip_info[PS], "huawei,avago-apds9110", sizeof("huawei,avago-apds9110"))) {
		avago_apds9110_ps_flag = 1;
		hwlog_err("%s:avago_apds9110_ps i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(sensor_chip_info[PS], "huawei,ams-tmd3725",sizeof("huawei,ams-tmd3725"))) {
		ams_tmd3725_ps_flag = 1;
		hwlog_err("%s:ams-tmd3725_ps i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(sensor_chip_info[PS], "huawei,ams_tcs3701",sizeof("huawei,ams_tcs3701"))) {
		ams_tcs3701_ps_flag = 1;
		hwlog_err("%s:ams_tcs3701_ps i2c_address suc,%d \n", __func__, temp);
	}

       if (!strncmp(sensor_chip_info[PS], "huawei,liteon-ltr582",sizeof("huawei,liteon-ltr582"))) {
		liteon_ltr582_ps_flag = 1;
		hwlog_err("%s:liteon-ltr582_ps i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(sensor_chip_info[PS], "huawei,avago_apds9999", sizeof("huawei,avago_apds9999"))) {
		apds9999_ps_flag = 1;
		hwlog_err("%s:avago_apds9999 i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(sensor_chip_info[PS], "huawei,ams_tmd3702", sizeof("huawei,ams_tmd3702"))) {
		ams_tmd3702_ps_flag = 1;
		hwlog_err("%s:huawei,tmd3702 i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(sensor_chip_info[PS], "huawei,vishay-vcnl36658",sizeof("huawei,vishay-vcnl36658"))) {
		vishay_vcnl36658_ps_flag = 1;
		hwlog_err("%s:vishay-vcnl36658_ps i2c_address suc,%d \n", __func__, temp);
	}

	if (!strncmp(sensor_chip_info[PS], "huawei,avago_apds9253_006",sizeof("huawei,avago_apds9253_006"))) {
		apds9253_006_ps_flag = 1;
		hwlog_info("%s:apds9253_006_ps i2c_address suc,%d \n", __func__, temp);
	}

	temp = of_get_named_gpio(dn, "gpio_int1", 0);
	if (temp < 0)
		hwlog_err("%s:read gpio_int1 fail\n", __func__);
	else
		ps_data.gpio_int1 = (GPIO_NUM_TYPE) temp;

	if (of_property_read_u32(dn, "min_proximity_value", &temp))
		hwlog_err("%s:read mag min_proximity_value fail\n", __func__);
	else
		ps_data.min_proximity_value = temp;

	if (of_property_read_u32(dn, "pwindows_value", &temp))
		hwlog_err("%s:read pwindows_value fail\n", __func__);
	else
		ps_data.pwindows_value = temp;

	if (of_property_read_u32(dn, "pwave_value", &temp))
		hwlog_err("%s:read pwave_value fail\n", __func__);
	else
		ps_data.pwave_value = temp;

	if (of_property_read_u32(dn, "threshold_value", &temp))
		hwlog_err("%s:read threshold_value fail\n", __func__);
	else
		ps_data.threshold_value = temp;

	if (of_property_read_u32(dn, "pwindows_screenon_value", &temp))
		hwlog_err("%s:read pwindows_screenon_value fail\n", __func__);
	else
		ps_data.pwindows_screenon_value = temp;

	if (of_property_read_u32(dn, "pwave_screenon_value", &temp))
		hwlog_err("%s:read pwave_screenon_value fail\n", __func__);
	else
		ps_data.pwave_screenon_value = temp;

	if (of_property_read_u32(dn, "threshold_screenon_value", &temp))
		hwlog_err("%s:read threshold_screenon_value fail\n", __func__);
	else
		ps_data.threshold_screenon_value = temp;

	if (of_property_read_u32(dn, "rdata_under_sun", &temp))
		hwlog_err("%s:read rdata_under_sun fail\n", __func__);
	else
		ps_data.rdata_under_sun = temp;

	if (of_property_read_u32(dn, "ps_pulse_count", &temp))
		hwlog_err("%s:read ps_pulse_count fail\n", __func__);
	else
		ps_data.ps_pulse_count = (uint8_t) temp;

	if (of_property_read_u32(dn, "persistent", &temp))
		hwlog_err("%s:read persistent fail\n", __func__);
	else
		ps_data.persistent = (uint8_t) temp;

	if (of_property_read_u32(dn, "ptime", &temp))
		hwlog_err("%s:read ptime fail\n", __func__);
	else
		ps_data.ptime = (uint8_t) temp;

	if (of_property_read_u32(dn, "p_on", &temp))
		hwlog_err("%s:read p_on fail\n", __func__);
	else
		ps_data.p_on = (uint8_t) temp;

	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read poll_interval fail\n", __func__);
	else
		ps_data.poll_interval = (uint16_t) temp;

	if (of_property_read_u32(dn, "use_oily_judge", &temp))
		hwlog_err("%s:read use_oily_judge fail\n", __func__);
	else
		ps_data.use_oily_judge = (uint16_t) temp;

	if (of_property_read_u32(dn, "init_time", &temp))
		hwlog_err("%s:read init_time fail\n", __func__);
	else
		ps_data.init_time = (uint16_t) temp;

	if (of_property_read_u32(dn, "ps_oily_threshold", &temp))
		hwlog_err("%s:read ps_oily_threshold fail\n", __func__);
	else
		ps_data.ps_oily_threshold = (uint8_t) temp;

	if (of_property_read_u32(dn, "ps_calib_20cm_threshold", &temp))
		hwlog_err("%s:read ps_calib_20cm_threshold fail\n", __func__);
	else
		ps_data.ps_calib_20cm_threshold = (uint16_t) temp;

	if (of_property_read_u32(dn, "ps_calib_5cm_threshold", &temp))
		hwlog_err("%s:read ps_calib_5cm_threshold fail\n", __func__);
	else
		ps_data.ps_calib_5cm_threshold = (uint16_t) temp;

	if (of_property_read_u32(dn, "ps_calib_3cm_threshold", &temp))
		hwlog_err("%s:read ps_calib_3cm_threshold fail\n", __func__);
	else
		ps_data.ps_calib_3cm_threshold = (uint16_t) temp;


	if (of_property_read_u32(dn, "wtime", &temp))
		hwlog_err("%s:read wtime fail\n", __func__);
	else
		ps_data.wtime = (uint8_t) temp;

	if (of_property_read_u32(dn, "led_current", &temp))
		hwlog_err("%s:read led_current fail\n", __func__);
	else
		ps_data.led_current = (uint8_t) temp;

	if (of_property_read_u32(dn, "pulse_len", &temp))
		hwlog_err("%s:read pulse_len fail\n", __func__);
	else
		ps_data.pulse_len = (uint8_t) temp;

	if (of_property_read_u32(dn, "pgain", &temp))
		hwlog_err("%s:read pgain fail\n", __func__);
	else
		ps_data.pgain = (uint8_t) temp;

	if (of_property_read_u32(dn, "prox_avg", &temp))
		hwlog_err("%s:read prox_avg fail\n", __func__);
	else
		ps_data.prox_avg= (uint8_t) temp;

	if (of_property_read_u32(dn, "offset_max", &temp))
		hwlog_err("%s:read offset_max fail\n", __func__);
	else
		ps_data.offset_max = (uint8_t) temp;

	if (of_property_read_u32(dn, "digital_offset_max", &temp))
		hwlog_err("%s:read digital_offset_max fail\n", __func__);
	else
		ps_data.digital_offset_max = (uint8_t)temp;

	if (of_property_read_u32(dn, "offset_min", &temp))
		hwlog_err("%s:read offset_min fail\n", __func__);
	else
		ps_data.offset_min = (uint8_t) temp;

	if (of_property_read_u32(dn, "ps_cali_after_sale", &temp))
		hwlog_err("%s:read ps_cali_after_sale fail\n", __func__);
	else
		ps_support_cali_after_sale = (uint8_t)temp;

	if (of_property_read_u32(dn, "oily_max_near_pdata", &temp))
		hwlog_err("%s:read oily_max_near_pdata fail\n", __func__);
	else
		ps_data.oily_max_near_pdata = (uint16_t) temp;

	if (of_property_read_u32(dn, "max_oily_add_pdata", &temp))
		hwlog_err("%s:read max_oily_add_pdata fail\n", __func__);
	else
		ps_data.max_oily_add_pdata = (uint16_t) temp;

	if (of_property_read_u32(dn, "max_near_pdata_loop", &temp))
		hwlog_err("%s:read max_near_pdata_loop fail\n", __func__);
	else
		ps_data.max_near_pdata_loop = (uint8_t) temp;

	if (of_property_read_u32(dn, "oily_count_size", &temp))
		hwlog_err("%s:read oily_count_size fail\n", __func__);
	else
		ps_data.oily_count_size = (uint8_t) temp;

	if (of_property_read_u32(dn, "ps_tp_threshold", &temp)){
	    hwlog_err("%s:read uint16_t fail\n", __func__);
	}
	else{
	    ps_data.ps_tp_threshold = (uint16_t) temp;
	}

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read ps file_id fail\n", __func__);
	else
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;
	dyn_req->file_count++;

	if (of_property_read_u32(dn, "support_absolute_threshold", &temp))
              hwlog_err("%s:read ps support_absolute_threshold fail\n", __func__);
	else
	      ps_support_abs_threshold=temp;

	if (of_property_read_u32(dn, "is_always_on", &temp))
		hwlog_err("%s:read is_always_on fail\n", __func__);
	else
		ps_data.is_always_on = (uint8_t) temp;
	read_sensorlist_info(dn, PS);
}

static void read_tof_data_from_dts(struct device_node *dn)
{
	int temp = 0;
	int register_add = 0;
	int ret = 0;
	int i2c_address = 0;
	int i2c_bus_num = 0;
	uint8_t detected_device_id = 0;

	read_chip_info(dn, TOF);
	if (!strncmp(sensor_chip_info[TOF], "huawei,ams_tmf8701", sizeof("huawei,ams_tmf8701"))) {
		ams_tof_flag = 1;
		strncpy(sensor_chip_info[PS], "huawei,ams_tmf8701", sizeof("huawei,ams_tmf8701"));
		hwlog_info("%s:ams_tmf8701 i2c_address suc,%d \n", __func__, temp);
	}
	if (!strncmp(sensor_chip_info[TOF], "huawei,sharp_gp2ap02", sizeof("huawei,sharp_gp2ap02"))) {
		sharp_tof_flag = 1;
		strncpy(sensor_chip_info[PS], "huawei,sharp_gp2ap02", sizeof("huawei,sharp_gp2ap02"));
		hwlog_info("%s:sharp_gp2ap02 i2c_address suc,%d \n", __func__, temp);
	}

    	if (of_property_read_u32(dn, "bus_number", &i2c_bus_num)
	    || of_property_read_u32(dn, "reg", &i2c_address)
	    || of_property_read_u32(dn, "chip_id_register", &register_add)) {
		hwlog_err("%s:read i2c bus_number (%d)or bus address(%x) or chip_id_register(%x) from dts fail\n",
		     __func__, i2c_bus_num, i2c_address, register_add);
	}

	if (of_property_read_u32(dn, "tof_calib_zero_threshold", &temp)){
		hwlog_info("%s:read tof_calib_zero_threshold fail\n", __func__);
	}
	else{
		tof_data.tof_calib_zero_threshold = temp;
		hwlog_info("%s:read tof_calib_zero_threshold=%d\n", __func__,temp);
	}

	if (of_property_read_u32(dn, "tof_calib_6cm_threshold", &temp)){
		hwlog_info("%s:read tof_calib_6cm_threshold fail\n", __func__);
	}
	else{
		tof_data.tof_calib_6cm_threshold = temp;
		hwlog_info("%s:read tof_calib_6cm_threshold=%d\n", __func__,temp);
	}

	if (of_property_read_u32(dn, "tof_calib_10cm_threshold", &temp)){
		hwlog_info("%s:read tof_calib_10cm_threshold fail\n", __func__);
	}
	else{
		tof_data.tof_calib_10cm_threshold = temp;
		hwlog_info("%s:read tof_calib_10cm_threshold=%d\n", __func__,temp);
	}

	if (of_property_read_u32(dn, "tof_calib_60cm_threshold", &temp)){
		hwlog_info("%s:read tof_calib_60cm_threshold fail\n", __func__);
	}
	else{
		tof_data.tof_calib_60cm_threshold = temp;
		hwlog_info("%s:read tof_calib_60cm_threshold=%d\n", __func__,temp);
	}

	ret = mcu_i2c_rw((uint8_t)i2c_bus_num, (uint8_t)i2c_address, (uint8_t*)&register_add, 1, &detected_device_id, 1);
	if (ret) {
		hwlog_err("%s:detect_i2c_device:send i2c read cmd to mcu fail,ret=%d\n", __func__, ret);
	}
	tof_register_value = detected_device_id;
	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read tof file_id fail\n", __func__);
	else{
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;
	}
	dyn_req->file_count++;

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read tof sensor_list_info_id fail\n", __func__);
	else
		sensorlist[++sensorlist[0]] = (uint16_t) temp;
	read_sensorlist_info(dn, TOF);
}
static void read_airpress_data_from_dts(struct device_node *dn)
{
	int temp = 0;

	read_chip_info(dn, AIRPRESS);

	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read poll_interval fail\n", __func__);
	else
		airpress_data.poll_interval = (uint16_t) temp;

	if (of_property_read_u32(dn, "isSupportTouch", &temp))
	{
		hwlog_err("%s:read isSupportTouch fail\n", __func__);
		airpress_data.IsSupportTouch = 0;
	}
	else
		airpress_data.IsSupportTouch = temp;

	if (of_property_read_u32(dn, "airpress_touch_calibrate_order", &temp))
	{
		hwlog_err("%s:read airpress_touch_calibrate_order fail\n", __func__);
		airpress_data.touch_fac_order = 0;
	}
	else
		airpress_data.touch_fac_order = (uint16_t)temp;

	if (of_property_read_u32(dn, "touch_fac_wait_time", &temp))
	{
		hwlog_err("%s:read touch_fac_wait_time fail\n", __func__);
		airpress_data.touch_fac_wait_time = 0;
	}
	else
		airpress_data.touch_fac_wait_time = (uint16_t)temp;

	if (of_property_read_u16_array(dn, "tp_touch_coordinate_threshold", airpress_data.tp_touch_coordinate_threshold, TP_COORDINATE_THRESHOLD))
	{
		hwlog_err("%s:read tp_touch_coordinate_threshold fail\n", __func__);
		for (temp = 0; temp < TP_COORDINATE_THRESHOLD; temp++)
			airpress_data.tp_touch_coordinate_threshold[temp] = 0;
	}

	if (of_property_read_u32(dn, "reg", &temp))
		hwlog_err("%s:read airpress reg fail\n",__func__);
	else
		airpress_data.cfg.i2c_address = (uint8_t)temp;

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read airpress file_id fail\n", __func__);
	else
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;
	dyn_req->file_count++;

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read ps sensor_list_info_id fail\n", __func__);
	else
		sensorlist[++sensorlist[0]] = (uint16_t) temp;
	read_sensorlist_info(dn, AIRPRESS);
}

static void read_handpress_data_from_dts(struct device_node *dn)
{
	int temp = 0;

	read_chip_info(dn, HANDPRESS);

	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read poll_interval fail\n", __func__);
	else
		handpress_data.poll_interval = (uint16_t) temp;
	hwlog_info("get handpress dev from dts.sensor name=%d\n", handpress_data.poll_interval);
	if (of_property_read_u32(dn, "bootloader_type", &temp))
		hwlog_err("%s:read handpress file_id_fw fail\n", __func__);
	else
		handpress_data.bootloader_type = (uint8_t)temp;

	hwlog_info("bootloader_type:%d\n", handpress_data.bootloader_type);
	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read handpress file_id fail\n", __func__);
	else
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;
	dyn_req->file_count++;
	hwlog_info("get handpress dev from dyn_req->file_count=%d\n", dyn_req->file_count);
	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read ps sensor_list_info_id fail\n", __func__);
	else
		sensorlist[++sensorlist[0]] = (uint16_t) temp;
	hwlog_info("get handpress dev from temp=%d; ++sensorlist[0]:%d\n", temp, sensorlist[0] - 1);
}

static void read_abov_sar_data_from_dts(struct device_node *dn)
{
	uint16_t abov_phone_type = 0;
	uint32_t ph = 0;
	int temp = 0;
	int i = 0;
	uint16_t *threshold_to_modem = NULL;
	uint16_t *calibrate_thred = NULL;

	read_chip_info(dn, CAP_PROX);
	threshold_to_modem = sar_pdata.sar_datas.abov_data.threshold_to_modem;
	if (of_property_read_u16_array(dn, "to_modem_threshold", threshold_to_modem, CAP_MODEM_THRESHOLE_LEN)) {
		*threshold_to_modem = 0xC8; // 0xC8: default modem_threshold
		*(threshold_to_modem + 1) = 0;
		hwlog_info("read threshold_to_modem fail\n");
	}
	hwlog_info("read threshold_to_modem %u %u %u\n", *threshold_to_modem, *(threshold_to_modem + 1),
	           *(threshold_to_modem + (CAP_MODEM_THRESHOLE_LEN - 1)));

	if (of_property_read_u16(dn, "phone_type", &abov_phone_type)) {
		sar_pdata.sar_datas.abov_data.phone_type = 0;
		hwlog_err("%s:read phone_type fail.\n", __func__);
	} else {
		sar_pdata.sar_datas.abov_data.phone_type = abov_phone_type;
		hwlog_info("%s:read phone_type:0x%x\n", __func__, sar_pdata.sar_datas.abov_data.phone_type);
	}

	if (of_property_read_u32(dn, "ph", &ph)) {
		sar_pdata.sar_datas.abov_data.ph = 0x5; // 0x5: enable ph0
		hwlog_err("%s:read ph fail\n", __func__);
	} else {
		sar_pdata.sar_datas.abov_data.ph = (uint8_t)ph;
		hwlog_info("%s:read ph:0x%x\n", __func__, sar_pdata.sar_datas.abov_data.ph);
	}
	calibrate_thred = sar_pdata.sar_datas.abov_data.calibrate_thred;
	if (of_property_read_u16_array(dn, "calibrate_thred", calibrate_thred, CAP_CALIBRATE_THRESHOLE_LEN)) {
		hwlog_err("%s:read calibrate_thred fail\n", __func__);
		*calibrate_thred = 0;
		*(calibrate_thred + 1) = 0;
		*(calibrate_thred + 2) = 0;
		*(calibrate_thred + 3) = 0;
	}
	hwlog_info("calibrate_thred:%u %u %u %u\n", *calibrate_thred, *(calibrate_thred + 1), *(calibrate_thred + 2),
	           *(calibrate_thred + 3));
}

static void read_capprox_data_from_dts(struct device_node *dn, struct sensor_detect_manager *sm)
{
	uint16_t threshold_to_ap = 0;
	uint16_t semtech_phone_type = 0;
	uint32_t ph = 0;
	int temp = 0, aux = 0;
	struct sar_platform_data *sar_pdata = (struct sar_platform_data *)sm->spara;
	int i = 0;
	uint16_t *threshold_to_modem = NULL;
	uint16_t *calibrate_thred = NULL;
	uint32_t *init_reg_val = NULL;
	uint32_t init_reg_val_default[17] = {
			 0x00010005, 0x00020529, 0x000300cc, 0x00040001,
			 0x00050F55,
			 0x00069905, 0x000700e8, 0x00080200, 0x00090000,
			 0x000a000C, 0x00798000,
			 0x000b9905, 0x000c00e8, 0x000d0200, 0x000e0000,
			 0x000f000C, 0x007a8000};
	read_chip_info(dn, sm->sensor_id);

	temp = of_get_named_gpio(dn, "gpio_int", 0);
	if (temp < 0)
		hwlog_err("%s:read %s gpio_int fail\n", __func__, sm->sensor_name_str);
	else
		sar_pdata->gpio_int = (GPIO_NUM_TYPE) temp;

	if (of_property_read_u32(dn, "gpio_int_sh", &temp))
		hwlog_err("%s:read %s gpio_int_sh fail\n", __func__,
		sm->sensor_name_str);
	else
		sar_pdata->gpio_int_sh = (GPIO_NUM_TYPE)temp;
	if (of_property_read_u32(dn, "reg", &temp))
		hwlog_err("%s:read cap_prox reg fail\n", __func__);
	else
		sar_pdata->cfg.i2c_address = (uint8_t)temp;

	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read %s poll_interval fail\n", __func__, sm->sensor_name_str);
	else
		sar_pdata->poll_interval = (uint16_t) temp;
	hwlog_info("sar.poll_interval: %d\n", sar_pdata->poll_interval);

	if (of_property_read_u32(dn, "aux_prop", &aux))
		hwlog_err("%s:read %s aux_prop fail\n", __func__, sm->sensor_name_str);
	else
		hwlog_info("%s:read %s aux_prop %d\n", __func__, sm->sensor_name_str, aux);

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read %s file_id fail\n", __func__, sm->sensor_name_str);
	else if (aux)
		read_aux_file_list(temp, sm->tag);
	else if (!aux) {
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;
		dyn_req->file_count++;
	}

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read %s sar sensor_list_info_id fail\n", __func__, sm->sensor_name_str);
	else
		sensorlist[++sensorlist[0]] = (uint16_t) temp;
	if (of_property_read_u32(dn, "calibrate_type", &temp))
		hwlog_err("%s:read %s sar calibrate_type fail\n", __func__, sm->sensor_name_str);
	else
		sar_pdata->calibrate_type = (uint16_t)temp;

	if (of_property_read_string(dn, "calibrate_order", (const char**)&sar_calibrate_order)) {
		hwlog_err("read calibrate order err.\n");
	}
	hwlog_info("calibrate order:%s\n", sar_calibrate_order);

	if (!strncmp(sensor_chip_info[sm->sensor_id], "huawei,cypress_sar_psoc4000", strlen("huawei,cypress_sar_psoc4000"))) {
		if (of_property_read_u16(dn, "to_ap_threshold", &threshold_to_ap))
			sar_pdata->sar_datas.cypress_data.threshold_to_ap = 0xC8;
		else
			sar_pdata->sar_datas.cypress_data.threshold_to_ap = (uint16_t)threshold_to_ap;

		threshold_to_modem = sar_pdata->sar_datas.cypress_data.threshold_to_modem;
		if (of_property_read_u16_array(dn, "to_modem_threshold", threshold_to_modem, 8)) {
			*threshold_to_modem =  0xC8;
			*(threshold_to_modem+1) = 0;
		}
		hwlog_info("ap:%u, modem:%u %u %u\n", threshold_to_ap, *threshold_to_modem, *(threshold_to_modem+1), *(threshold_to_modem+7));
	} else if (!strncmp(sensor_chip_info[sm->sensor_id], "huawei,adi-adux1050", strlen("huawei,adi-adux1050"))) {

		memset(&adux_sar_add_data, 0x00, sizeof(adux_sar_add_data));

		if (of_property_read_u16_array(dn, "to_ap_threshold", adux_sar_add_data.threshold_to_ap_stg, STG_SUPPORTED_NUM)){
			adux_sar_add_data.threshold_to_ap_stg[0] = DEFAULT_THRESHOLD;
			adux_sar_add_data.threshold_to_ap_stg[1] = DEFAULT_THRESHOLD;
			adux_sar_add_data.threshold_to_ap_stg[2] = DEFAULT_THRESHOLD;
			hwlog_info("read threshold_to_ap fail\n");
		}

		if (of_property_read_u16_array(dn, "to_modem_threshold", adux_sar_add_data.threshold_to_modem_stg, TO_MODEM_SUPPORTED_LEVEL_NUM*STG_SUPPORTED_NUM)) {
			adux_sar_add_data.threshold_to_modem_stg[0] = DEFAULT_THRESHOLD;
			adux_sar_add_data.threshold_to_modem_stg[1*TO_MODEM_SUPPORTED_LEVEL_NUM] = DEFAULT_THRESHOLD;
			adux_sar_add_data.threshold_to_modem_stg[2*TO_MODEM_SUPPORTED_LEVEL_NUM] = DEFAULT_THRESHOLD;
			hwlog_info("read threshold_to_modem fail\n");
		}

		if (of_property_read_u32(dn, "update_offset", &temp))
			hwlog_err("%s:read %s sar updata_offset fail\n", __func__, sm->sensor_name_str);
		else{
			adux_sar_add_data.updata_offset=temp;
		}

		if (of_property_read_u32(dn, "cdc_calue_threshold", &temp))
			hwlog_err("%s:read %s sar cdc_calue_threshold fail\n", __func__, sm->sensor_name_str);
		else{
			adux_sar_add_data.cdc_calue_threshold=temp;
		}

		hwlog_info( "ap0=0x%x, ap1=0x%x, ap2=0x%x\n", adux_sar_add_data.threshold_to_ap_stg[0],
				adux_sar_add_data.threshold_to_ap_stg[1],
				adux_sar_add_data.threshold_to_ap_stg[2] );

		for( i=0; i<TO_MODEM_SUPPORTED_LEVEL_NUM*STG_SUPPORTED_NUM; i++ ){
			hwlog_info( "modem%d = 0x%x\n", i, adux_sar_add_data.threshold_to_modem_stg[i]);
		}

		calibrate_thred = adux_sar_add_data.calibrate_thred;
		if (of_property_read_u16_array(dn, "calibrate_thred", calibrate_thred, 4)) {
			hwlog_err("%s:read %s calibrate_thred fail\n", __func__, sm->sensor_name_str);
			*calibrate_thred = 0;
			*(calibrate_thred+1) = 0;
			*(calibrate_thred+2) = 0;
			*(calibrate_thred+3) = 0;
		}
		hwlog_info("calibrate_thred:%u %u %u %u\n", *calibrate_thred, *(calibrate_thred+1), *(calibrate_thred+2), *(calibrate_thred+3));

		init_reg_val = sar_pdata->sar_datas.adux_data.init_reg_val;
		if (of_property_read_u32_array(dn, "init_reg_val", init_reg_val, ADUX_REGS_NEED_INITIATED_NUM)) {
			hwlog_err("%s:read %s init_reg_val fail\n", __func__, sm->sensor_name_str);
			memcpy(init_reg_val, init_reg_val_default, sizeof(init_reg_val_default));
		}
		hwlog_info("init_reg_val[0]:%8x init_reg_val[%d]%8x\n", *init_reg_val, ADUX_REGS_NEED_INITIATED_NUM-1, *(init_reg_val+ADUX_REGS_NEED_INITIATED_NUM-1));

	}else if (!strncmp(sensor_chip_info[sm->sensor_id], "huawei,semtech-sx9323", strlen("huawei,semtech-sx9323"))){
		if (of_property_read_u16(dn, "to_ap_threshold", &threshold_to_ap))
			sar_pdata->sar_datas.semteck_data.threshold_to_ap = 0xC8;
		else
			sar_pdata->sar_datas.semteck_data.threshold_to_ap = (uint16_t)threshold_to_ap;
		threshold_to_modem = sar_pdata->sar_datas.semteck_data.threshold_to_modem;
		if (of_property_read_u16_array(dn, "to_modem_threshold", threshold_to_modem, 8)) {
			*threshold_to_modem =  0xC8;
			*(threshold_to_modem+1) = 0;
			hwlog_info("read threshold_to_modem fail\n");
		}
		hwlog_info("read threshold_to_modem %u %u %u\n", *threshold_to_modem, *(threshold_to_modem+1), *(threshold_to_modem+7));

		init_reg_val = sar_pdata->sar_datas.semteck_data.init_reg_val;
		if (of_property_read_u32_array(dn, "init_reg_val", init_reg_val, SEMTECH_REGS_NEED_INITIATED_NUM)) {
			hwlog_err("%s:read %s init_reg_val fail\n", __func__, sm->sensor_name_str);
			memcpy(init_reg_val, init_reg_val_default, sizeof(init_reg_val_default));
		}
		hwlog_info("init_reg_val[0]:%8x init_reg_val[%d]%8x\n", *init_reg_val, SEMTECH_REGS_NEED_INITIATED_NUM-1,*(init_reg_val+SEMTECH_REGS_NEED_INITIATED_NUM-1));

		if (of_property_read_u16(dn, "phone_type", &semtech_phone_type)) {
			sar_pdata->sar_datas.semteck_data.phone_type = 0;
			hwlog_err("%s:read %s phone_type fail.\n", __func__, sm->sensor_name_str);
		}
		else {
			sar_pdata->sar_datas.semteck_data.phone_type = semtech_phone_type;
			hwlog_info("%s:read %s phone_type:0x%x\n",__func__, sm->sensor_name_str,sar_pdata->sar_datas.semteck_data.phone_type);
		}

		if (of_property_read_u32(dn, "ph", &ph))
		{
			sar_pdata->sar_datas.semteck_data.ph= 0x2f;
			hwlog_err("%s:read %s ph fail\n", __func__, sm->sensor_name_str);
		}
		else
		{
			sar_pdata->sar_datas.semteck_data.ph= (uint8_t)ph;
			hwlog_info("%s:read %s ph:0x%x\n",__func__, sm->sensor_name_str,sar_pdata->sar_datas.semteck_data.ph);
		}
		calibrate_thred = sar_pdata->sar_datas.semteck_data.calibrate_thred;
		if (of_property_read_u16_array(dn, "calibrate_thred", calibrate_thred, 4)) {
			hwlog_err("%s:read %s calibrate_thred fail\n", __func__, sm->sensor_name_str);
			*calibrate_thred = 0;
			*(calibrate_thred+1) = 0;
			*(calibrate_thred+2) = 0;
			*(calibrate_thred+3) = 0;
		}
		hwlog_info("calibrate_thred:%u %u %u %u\n", *calibrate_thred, *(calibrate_thred+1), *(calibrate_thred+2), *(calibrate_thred+3));
	} else if (!strncmp(sensor_chip_info[CAP_PROX], "huawei,abov-a96t3x6", strlen("huawei,abov-a96t3x6"))) {
		read_abov_sar_data_from_dts(dn);
	}
	read_sensorlist_info(dn, sm->sensor_id);
}

static void read_connectivity_data_from_dts(struct device_node *dn)
{
	int temp = 0;

	read_chip_info(dn, CONNECTIVITY);

	if (of_property_read_u32(dn, "gpio1_gps_cmd_ap", &temp))
		hwlog_err("%s:read gpio1_gps_cmd_ap fail\n", __func__);
	else
		connectivity_data.gpio1_gps_cmd_ap = (GPIO_NUM_TYPE) temp;

	if (of_property_read_u32(dn, "gpio1_gps_cmd_sh", &temp))
		hwlog_err("%s:read gpio1_gps_cmd_sh fail\n", __func__);
	else
		connectivity_data.gpio1_gps_cmd_sh = (GPIO_NUM_TYPE) temp;

	if (of_property_read_u32(dn, "gpio2_gps_ready_ap", &temp))
		hwlog_err("%s:read gpio2_gps_ready_ap fail\n", __func__);
	else
		connectivity_data.gpio2_gps_ready_ap = (GPIO_NUM_TYPE) temp;

	if (of_property_read_u32(dn, "gpio2_gps_ready_sh", &temp))
		hwlog_err("%s:read gpio2_gps_ready_sh fail\n", __func__);
	else
		connectivity_data.gpio2_gps_ready_sh = (GPIO_NUM_TYPE) temp;

	if (of_property_read_u32(dn, "gpio3_wakeup_gps_ap", &temp))
		hwlog_err("%s:read gpio3_wakeup_gps_ap fail\n", __func__);
	else
		connectivity_data.gpio3_wakeup_gps_ap = (GPIO_NUM_TYPE) temp;

	if (of_property_read_u32(dn, "gpio3_wakeup_gps_sh", &temp))
		hwlog_err("%s:read gpio3_wakeup_gps_sh fail\n", __func__);
	else
		connectivity_data.gpio3_wakeup_gps_sh = (GPIO_NUM_TYPE) temp;

	if (of_property_read_u32(dn, "bus_type", &temp))
		hwlog_err("%s:read bus_type fail\n", __func__);
	else
		connectivity_data.cfg.bus_type = (uint8_t) temp;

	if (of_property_read_u32(dn, "bus_number", &temp))
		hwlog_err("%s:read bus_number fail\n", __func__);
	else
		connectivity_data.cfg.bus_num = (uint8_t) temp;

	if (of_property_read_u32(dn, "i2c_address", &temp))
		hwlog_err("%s:read i2c_address fail\n", __func__);
	else
		connectivity_data.cfg.i2c_address = (uint32_t) temp;

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read connectivity file_id fail\n", __func__);
	else
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;

	dyn_req->file_count++;

	hwlog_err("connectivity file id is %d\n", temp);
	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read connectivity sensor_list_info_id fail\n", __func__);
	else
		sensorlist[++sensorlist[0]] = (uint16_t) temp;
}
//read gpio reg chip id;

static void read_vibrator_from_dts(struct device_node* dn)
{
	int temp = 0, rc = 0;
	read_chip_info(dn, VIBRATOR);
	if (of_property_read_u32(dn, "file_id", &temp)){
		hwlog_err("%s:read read_vibrator_from_dts file_id fail\n", __func__);
	}
	else
	{
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;
	}
	dyn_req->file_count++;
	hwlog_err("read_vibrator_from_dts  file id is %d\n", temp);

	rc = of_property_read_u32(dn, "ti,max-timeout-ms", &temp);
	/* configure minimum idle timeout */
	if (rc < 0) {
		vibrator_data.max_timeout_ms = HUB_MAX_TIMEOUT;
	} else {
		vibrator_data.max_timeout_ms = (int)temp;
	}
	hwlog_err("vibrator max_timeout_ms:%d.\n", vibrator_data.max_timeout_ms);

	rc = of_property_read_u32(dn, "ti,reduce-timeout-ms", &temp);
	/* configure reduce timeout */
	if (rc < 0) {
		vibrator_data.reduce_timeout_ms = 0;
	} else {
		vibrator_data.reduce_timeout_ms = (int )temp;
	}
	hwlog_err("vibrator reduce timedout_ms:%d.\n", vibrator_data.reduce_timeout_ms);

	rc = of_property_read_u32(dn, "lra_rated_voltage", &temp);
	if (rc < 0) {
		vibrator_data.lra_rated_voltage =HUB_LRA_RATED_VOLTAGE;
	} else {
		vibrator_data.lra_rated_voltage = (char)temp;
	}
	hwlog_err("vibrator lra_rated_voltage:0x%x.\n", vibrator_data.lra_rated_voltage);

	rc = of_property_read_u32(dn, "lra_overdriver_voltage", &temp);
	if (rc < 0) {
		vibrator_data.lra_overdriver_voltage = HUB_LRA_OVERDRIVE_CLAMP_VOLTAGE;
	} else {
		vibrator_data.lra_overdriver_voltage = (char)temp;
	}
	hwlog_err("vibrator lra_overdriver_voltage:0x%x.\n", vibrator_data.lra_overdriver_voltage);
	if(runmode_is_factory()){
		vibrator_data.lra_overdriver_voltage = VIB_FAC_LRALVILTAGE;
	}

	rc = of_property_read_u32(dn, "lra_rtp_strength", &temp);
	if (rc < 0) {
		vibrator_data.lra_rtp_strength = HUB_REAL_TIME_PLAYBACK_STRENGTH;
	} else {
		vibrator_data.lra_rtp_strength = (char)temp;
	}
	hwlog_err("vibrator lra_rtp_strength:0x%x.\n", vibrator_data.lra_rtp_strength);

	rc = of_property_read_u32(dn, "support_amplitude_control", &temp);
	if(rc < 0) {
		vibrator_data.support_amplitude_control = 0;
	}else {
		vibrator_data.support_amplitude_control = (char)temp;
	}
	hwlog_err("vibrator support_amplitude_control:%d.\n", vibrator_data.support_amplitude_control);

	vibrator_data.gpio_enable = of_get_named_gpio(dn, "gpio-enable", 0);
	hwlog_err("vibrator gpio_enable:%d.\n", vibrator_data.gpio_enable);
	gpio_direction_output(vibrator_data.gpio_enable, 1);
	udelay(30);
}

static void read_fingerprint_from_dts(struct device_node* dn)
{
    int temp = 0;
    read_chip_info(dn, FINGERPRINT);

    if (of_property_read_u32(dn, "file_id", &temp))
    { hwlog_err("%s:read fingerprint file_id fail\n", __func__); }
    else
    { dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp; }

    dyn_req->file_count++;
    hwlog_err("fingerprint  file id is %d\n", temp);

    if (of_property_read_u32(dn, "chip_id_register", &temp))
    { hwlog_err("%s:read chip_id_register fail\n", __func__); }
    else
    { fingerprint_data.reg = (uint16_t) temp; }

    if (of_property_read_u32(dn, "chip_id_value", &temp))
    { hwlog_err("%s:read chip_id_value fail\n", __func__); }
    else
    { fingerprint_data.chip_id= (uint16_t) temp; }

    if (of_property_read_u32(dn, "product_id_value", &temp))
    { hwlog_err("%s:read product_id_value fail\n", __func__); }
    else
    { fingerprint_data.product_id= (uint16_t) temp; }

    if (of_property_read_u32(dn, "gpio_irq", &temp))
    { hwlog_err("%s:read gpio_irq fail\n", __func__); }
    else
    { fingerprint_data.gpio_irq = (GPIO_NUM_TYPE) temp; }

    if (of_property_read_u32(dn, "gpio_irq_sh", &temp))
    { hwlog_err("%s:read gpio_irq_sh fail\n", __func__); }
    else
    { fingerprint_data.gpio_irq_sh = (GPIO_NUM_TYPE) temp; }

    if (of_property_read_u32(dn, "gpio_reset", &temp))
    { hwlog_err("%s:read gpio_reset fail\n", __func__); }
    else
    { fingerprint_data.gpio_reset = (GPIO_NUM_TYPE) temp; }

    if (of_property_read_u32(dn, "gpio_reset_sh", &temp))
    { hwlog_err("%s:read gpio_reset_sh fail\n", __func__); }
    else
    { fingerprint_data.gpio_reset_sh = (GPIO_NUM_TYPE) temp;}

    if (of_property_read_u32(dn, "gpio_cs", &temp))
    { hwlog_err("%s:read gpio_cs fail\n", __func__); }
    else
    { fingerprint_data.gpio_cs = (GPIO_NUM_TYPE) temp; }

}

static void read_fingerprint_ud_from_dts(struct device_node* dn)
{
    int temp = 0;
    read_chip_info(dn, FINGERPRINT_UD);

    if (of_property_read_u32(dn, "file_id", &temp))
    { hwlog_err("%s:read fingerprint file_id fail\n", __func__); }
    else
    { dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp; }

    dyn_req->file_count++;
    hwlog_err("fingerprint  file id is %d\n", temp);

    if (of_property_read_u32(dn, "chip_id_register", &temp))
    { hwlog_err("%s:read chip_id_register fail\n", __func__); }
    else
    { fingerprint_ud_data.reg = (uint16_t) temp; }

    if (of_property_read_u32(dn, "chip_id_value", &temp))
    { hwlog_err("%s:read chip_id_value fail\n", __func__); }
    else
    { fingerprint_ud_data.chip_id= (uint16_t) temp; }

    if (of_property_read_u32(dn, "product_id_value", &temp))
    { hwlog_err("%s:read product_id_value fail\n", __func__); }
    else
    { fingerprint_ud_data.product_id= (uint16_t) temp; }

    if (of_property_read_u32(dn, "gpio_irq", &temp))
    { hwlog_err("%s:read gpio_irq fail\n", __func__); }
    else
    { fingerprint_ud_data.gpio_irq = (GPIO_NUM_TYPE) temp; }

    if (of_property_read_u32(dn, "gpio_irq_sh", &temp))
    { hwlog_err("%s:read gpio_irq_sh fail\n", __func__); }
    else
    { fingerprint_ud_data.gpio_irq_sh = (GPIO_NUM_TYPE) temp; }

    if (of_property_read_u32(dn, "gpio_reset", &temp))
    { hwlog_err("%s:read gpio_reset fail\n", __func__); }
    else
    { fingerprint_ud_data.gpio_reset = (GPIO_NUM_TYPE) temp; }

    if (of_property_read_u32(dn, "gpio_cs", &temp))
    { hwlog_err("%s:read gpio_cs fail\n", __func__); }
    else
    { fingerprint_ud_data.gpio_cs = (GPIO_NUM_TYPE) temp; }

    if (of_property_read_u32(dn, "tp_hover_support", &temp))
    { hwlog_warn("%s:read tp_hover_support fail\n", __func__); }
    else
    { fingerprint_ud_data.tp_hover_support= (uint16_t) temp; }

}


static void read_key_i2c_data_from_dts(struct device_node *dn)
{
	read_chip_info(dn, KEY);
	key_data.cfg.i2c_address = 0x27;
	dyn_req->file_list[dyn_req->file_count] = 59;
	dyn_req->file_count++;

	hwlog_info("key read dts\n");
}

static void read_aod_data_from_dts(struct device_node *dn)
{
	int temp = 0;
	read_chip_info(dn, SH_AOD);
	if (of_property_read_u32(dn, "phone_type", &temp))
    { hwlog_err("%s:read chip_id_value fail\n", __func__); }
    else
    {
		aod_data.phone_type = (uint32_t) temp;
		hwlog_info("aod_data.phone_type is %u\n", aod_data.phone_type);
	}
}

static void read_magn_bracket_data_from_dts(struct device_node *dn)
{
	int temp = 0;
	u32 wia[6]={ 0, };
	struct property *prop = NULL;
	unsigned int len = 0;

	read_chip_info(dn, MAGN_BRACKET);

	prop = of_find_property(dn, "mag_axis_change", NULL);
	if (!prop) {
		hwlog_err("%s! prop is NULL!\n", __func__);
		return;
	}
	if (!prop->value) {
		hwlog_err("%s! prop->value is NULL!\n", __func__);
		return;
	}
	len = prop->length / 4;
	if (of_property_read_u32_array(dn, "mag_axis_change", wia, len)) {
		hwlog_err("%s:read mag_axis_change from dts fail!\n",  __func__);
		return;
	}

	magn_bracket_data.mag_x_change_lower = (int)wia[0];
	magn_bracket_data.mag_x_change_upper = (int)wia[1];
	magn_bracket_data.mag_y_change_lower = (int)wia[2];
	magn_bracket_data.mag_y_change_upper = (int)wia[3];
	magn_bracket_data.mag_z_change_lower = (int)wia[4];
	magn_bracket_data.mag_z_change_upper = (int)wia[5];

	hwlog_info("read_magn_bracket_data_from_dts %d, %d, %d, %d, %d, %d\n", magn_bracket_data.mag_x_change_lower, magn_bracket_data.mag_x_change_upper,
		magn_bracket_data.mag_y_change_lower, magn_bracket_data.mag_y_change_upper, magn_bracket_data.mag_z_change_lower, magn_bracket_data.mag_z_change_upper);

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read magn_bracket file_id fail\n", __func__);
	else
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;
	dyn_req->file_count++;

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read magn_bracket sensor_list_info_id fail\n", __func__);
	else
		sensorlist[++sensorlist[0]] = (uint16_t) temp;

	read_sensorlist_info(dn, MAGN_BRACKET);
}
static int read_rpc_data_from_dts(struct device_node* dn)
{
    int t = 0;
    int *temp = &t;
    unsigned int i;
    u32 wia[32]={ 0, };
    struct property* prop = NULL;
    unsigned int len = 0;
    memset(&rpc_data, 0, sizeof(rpc_data));
    memset(wia, 0, sizeof(wia));

    read_chip_info(dn, RPC);

    prop = of_find_property(dn, "table", NULL);

    if (!prop)
    {
        hwlog_err("%s! prop is NULL!\n", __func__);
        return -1;
    }

    len = (u32)(prop->length) / sizeof(u32);
    if (of_property_read_u32_array(dn, "table", wia, len))
    {
        hwlog_err("%s:read rpc_table from dts fail!\n",  __func__);
        return -1;
    }
    for (i = 0; i < len; i++)
    {
        rpc_data.table[i] = (u16)wia[i];
    }
    memset(wia, 0, sizeof(wia));
    prop = of_find_property(dn, "mask", NULL);

    if (!prop)
    {
        hwlog_err("%s! prop is NULL!\n", __func__);
        return -1;
    }

    len = (u32)(prop->length) / sizeof(u32);

    if (of_property_read_u32_array(dn, "mask", wia, len))
    {
        hwlog_err("%s:read rpc_mask from dts fail!\n",  __func__);
        return -1;
    }
        for (i = 0; i < len; i++)
    {
        rpc_data.mask[i] = (u16)wia[i];
    }
    if (of_property_read_u32(dn, "file_id", (u32 *)temp))
    { hwlog_err("%s:read rpc file_id fail\n", __func__); }
    else
    { dyn_req->file_list[dyn_req->file_count] = (uint16_t) t; }

    dyn_req->file_count++;

    if (of_property_read_u32(dn, "sensor_list_info_id", (u32 *)temp))
    { hwlog_err("%s:read rpc sensor_list_info_id fail\n", __func__); }
    else
    { sensorlist[++sensorlist[0]] = (uint16_t) t; }

    if (of_property_read_u32(dn, "default_value", (u32 *)temp))
    { hwlog_err("%s:read default_value fail\n", __func__); }
    else
    {rpc_data.default_value = (uint16_t) t; }

    if (of_property_read_u32(dn, "mask_enable", (u32 *)temp))
    { hwlog_err("%s:read mask_enable fail\n", __func__); }
    else
    {rpc_data.mask_enable = (uint16_t) t; }

    read_sensorlist_info(dn, RPC);
    return 0;
}

static void read_kb_data_from_dts(struct device_node *dn)
{
	int temp = 0;

	if (dn == NULL)
		return;

	read_chip_info(dn, KB);

	if (of_property_read_u32(dn, "uart_num", &temp))
		hwlog_err("%s:read uart_num fail\n", __func__);
	else
		g_kb_data.uart_num = (uint16_t) temp;

	if (of_property_read_u32(dn, "kb_detect_adc_num", &temp))
		hwlog_err("%s:read kb_detect_adc_num fail\n", __func__);
	else
		g_kb_data.kb_detect_adc_num = (uint16_t) temp;

	if (of_property_read_u32(dn, "kb_disconnect_adc_min", &temp))
		hwlog_err("%s:read kb_disconnect_adc_min fail\n", __func__);
	else
		g_kb_data.kb_disconnect_adc_min = (uint16_t) temp;

	if (of_property_read_u32(dn, "kb_disable_angle", &temp))
		hwlog_err("%s:read kb_disable_angle fail\n", __func__);
	else
		g_kb_data.kb_disable_angle = (uint16_t) temp;

	hwlog_info("%s: uart_num:%d, adc_num:%d, disconnect_adc_min:%d\n",
		__func__, g_kb_data.uart_num, g_kb_data.kb_detect_adc_num,
		g_kb_data.kb_disconnect_adc_min);
}

static int get_adapt_file_id_for_dyn_load(void)
{
	u32 wia[ADAPT_SENSOR_LIST_NUM]={ 0 };
	struct property *prop = NULL;
	unsigned int i = 0;
	unsigned int len = 0;
	struct device_node *sensorhub_node = NULL;
	const char *name = "adapt_file_id";
	char *temp = NULL;
	sensorhub_node = of_find_compatible_node(NULL, NULL, "huawei,sensorhub");
	if (!sensorhub_node) {
		hwlog_err("%s, can't find node sensorhub\n", __func__);
		return -1;
	}
	prop = of_find_property(sensorhub_node, name, NULL);
	if (!prop) {
		hwlog_err("%s! prop is NULL!\n", __func__);
		return -EINVAL;
	}
	if (!prop->value) {
		hwlog_err("%s! prop->value is NULL!\n", __func__);
		return -ENODATA;
	}
	len = prop->length /4;

	if(of_property_read_u32_array(sensorhub_node, name, wia, len))
	{
		hwlog_err("%s:read adapt_file_id from dts fail!\n", name);
		return -1;
	}
	for (i = 0; i < len; i++) {
		dyn_req->file_list[dyn_req->file_count] = wia[i];
		dyn_req->file_count++;
	}
	//find hifi supported or not
	if (0 == of_property_read_u32(sensorhub_node, "hifi_support", &i))
	{
		if(1 == i)
		{
			hifi_supported = 1;
			hwlog_info("sensor get hifi support %d\n", i);
		}
	}
    //find number of the hall sensor
    if (0 == of_property_read_u32(sensorhub_node, "hall_number", &i))
    {
        if(2 == i)
        {
            hall_number = 2 ;
            hwlog_info("sensor get hall number %d\n" , i);
        }
    }

	if (0 == of_property_read_u32(sensorhub_node, "hall_sen_type", &i))
	{
		hall_sen_type = i;
		hwlog_info("sensor get hall sensor type %d\n", i);
		if (hall_sen_type == HUB_FOLD_HALL_TYPE)
			all_ap_sensor_operations[TAG_EXT_HALL].work_on_ap = false;
	} else {
		hall_sen_type = 0;
	}

	if (0 == of_property_read_string(sensorhub_node, "docom_step_counter", (const char **)&temp))
	{
		if (!strcmp("enabled",temp)) {
			pConfigOnDDr->reserved |= 1 << 0;
			hwlog_info("%s : docom_step_counter status is %s \n",__func__,temp);
		}
	}

	if (0 == of_property_read_string(sensorhub_node, "homo_activity", (const char **)&temp))
	{
		if (!strcmp("enabled",temp)) {
			pConfigOnDDr->reserved |= 1 << 1;
			hwlog_info("%s : homo_activity status is %s \n",__func__,temp);
		}
	}

	return 0;
}

static void swap1(uint16_t *left, uint16_t *right)
{
	uint16_t temp;
	temp = *left;
	*left = *right;
	*right = temp;
}

/* delete the repeated file id by map table*/
static uint8_t check_file_list(uint8_t file_count, uint16_t *file_list)
{
	int i, j, k;

	if ((file_count == 0) || (NULL == file_list)) {
		hwlog_err("%s, val invalid\n", __func__);
		return 0;
	}

	for (i = 0; i < file_count; i++) {
		for (j=i+1; j<file_count; j++) {
			if (file_list[i] == file_list[j]) {
				file_count-=1;
				for (k=j; k<file_count; k++)
					file_list[k] = file_list[k+1];
				j-=1;
			}
		}
	}

	for (i = 0; i < file_count; i++) {
	    for (j = 1; j < file_count - i; j++) {
	        if (file_list[j - 1] > file_list[j])
	            swap1(&file_list[j - 1], &file_list[j]);
	    }
	}
	return file_count;
}

static int get_adapt_sensor_list_id(void)
{
	u32 wia[ADAPT_SENSOR_LIST_NUM]={ 0 };
	struct property *prop = NULL;
	unsigned int i = 0, len = 0;
	struct device_node *sensorhub_node = NULL;
	const char *name = "adapt_sensor_list_id";
	sensorhub_node = of_find_compatible_node(NULL, NULL, "huawei,sensorhub");
	if (!sensorhub_node) {
		hwlog_err("%s, can't find node sensorhub\n", __func__);
		return -1;
	}
	prop = of_find_property(sensorhub_node, name, NULL);
	if (!prop) {
		hwlog_err("%s! prop is NULL!\n", __func__);
		return -EINVAL;
	}
	if (!prop->value) {
		hwlog_err("%s! prop->value is NULL!\n", __func__);
		return -ENODATA;
	}
	len = prop->length / 4;
	if (of_property_read_u32_array(sensorhub_node, name, wia, len)) {
		hwlog_err("%s:read adapt_sensor_list_id from dts fail!\n", name);
		return -1;
	}
	for (i = 0; i < len; i++) {
		sensorlist[sensorlist[0] + 1] = (uint16_t) wia[i];
		sensorlist[0]++;
	}
	return 0;
}

static int get_sensors_id_from_dts(void)
{
	struct device_node *sensorhub_node = NULL;

	sensorhub_node = of_find_compatible_node(NULL, NULL, "huawei,sensorhub");
	if (!sensorhub_node) {
		hwlog_err("%s, can't find node sensorhub\n", __func__);
		return -1;
	}

	if (of_property_read_string(sensorhub_node, "acc_sensor_id", (const char**)&acc_sensor_id)) {
		hwlog_err("acc_sensor_id err.\n");
	}
	hwlog_info("acc_sensor_id:%s\n",acc_sensor_id);

	if (of_property_read_string(sensorhub_node, "gyro_sensor_id", (const char**)&gyro_sensor_id)) {
		hwlog_err("gyro_sensor_id err.\n");
	}
	hwlog_info("gyro_sensor_id:%s\n",gyro_sensor_id);

	if (of_property_read_string(sensorhub_node, "mag_sensor_id", (const char**)&mag_sensor_id)) {
		hwlog_err("mag_sensor_id err.\n");
	}
	hwlog_info("mag_sensor_id:%s\n",mag_sensor_id);

	if (of_property_read_string(sensorhub_node, "cap_sensor_id", (const char**)&cap_sensor_id)) {
		hwlog_err("cap_sensor_id err.\n");
	}
	hwlog_info("cap_sensor_id:%s\n",cap_sensor_id);

	return 0;
}


static int send_dyn_to_mcu(void *buf, int len)
{
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	int ret = 0;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	pkg_ap.tag = TAG_SYS;
	pkg_ap.cmd = CMD_SYS_DYNLOAD_REQ;
	pkg_ap.wr_buf = buf;
	pkg_ap.wr_len = len;

	if (g_iom3_state == IOM3_ST_RECOVERY || iom3_power_state == ST_SLEEP) {
		ret = write_customize_cmd(&pkg_ap, NULL, false);
	} else {
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	}

	if (ret) {
		hwlog_err("send file id to mcu fail,ret=%d\n", ret);
		return -1;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("file id set fail\n");
		return -1;
	}

	return 0;
}

int send_fileid_to_mcu(void)
{
	int i = 0;
	pkt_sys_dynload_req_t dynload_req;

	if (dyn_req->file_count) {
		hwlog_info("sensorhub after check, get dynload file id number = %d, fild id", dyn_req->file_count);
		for (i = 0; i < dyn_req->file_count; i++) {
			hwlog_info("--%d", dyn_req->file_list[i]);
		}
		hwlog_info("\n");
		dyn_req->file_flg = 0;
		if (send_dyn_to_mcu(&(dyn_req->file_flg),
			dyn_req->file_count * sizeof(dyn_req->file_list[0]) + sizeof(dyn_req->file_flg) + sizeof(dyn_req->file_count)))
			hwlog_err("%s send file_id to mcu failed\n", __func__);
	} else {
		hwlog_err("%s file_count = 0, not send file_id to mcu\n", __func__);
		return -EINVAL;
	}

	if (aux_req->file_count) {
		hwlog_info("sensorhub after check, get aux file id number = %d, aux file id and tag ", aux_req->file_count);
		for (i = 0; i < aux_req->file_count; i++) {
			hwlog_info("--%d, %d", aux_req->file_list[2*i], aux_req->file_list[2*i + 1]);
		}
		hwlog_info("\n");
		aux_req->file_flg = 1;
		if (send_dyn_to_mcu(&(aux_req->file_flg),
			aux_req->file_count * sizeof(aux_req->file_list[0]) * 2 + sizeof(aux_req->file_flg) + sizeof(aux_req->file_count)))
			hwlog_err("%s send aux file_id to mcu failed\n", __func__);
	} else {
		hwlog_err("%s aux count = 0, not send file_id to mcu\n", __func__);
	}
	memset(&dynload_req, 0, sizeof(pkt_sys_dynload_req_t));
	dynload_req.file_flg = 2;
	return send_dyn_to_mcu(&(dynload_req.file_flg), sizeof(pkt_sys_dynload_req_t) - sizeof(pkt_header_t));
}

static int get_adapt_id_and_send(void)
{
	int ret = 0, i = 0;

	ret = get_adapt_file_id_for_dyn_load();
	if (ret < 0) {
		hwlog_err("get_adapt_file_id_for_dyn_load() failed!\n");
	}
	hwlog_info("get file id number = %d\n", dyn_req->file_count);

	ret = get_adapt_sensor_list_id();
	if (ret < 0) {
		hwlog_err("get_adapt_sensor_list_id() failed!\n");
	}

	sensorlist[0] = check_file_list(sensorlist[0], &sensorlist[1]);
	if (sensorlist[0] > 0) {
		hwlog_info("sensorhub after check, get sensor list id number = %d, list id: ", sensorlist[0]);
		for (i = 0; i < sensorlist[0]; i++) {
			hwlog_info("--%d", sensorlist[i + 1]);
		}
		hwlog_info("\n");
	} else {
		hwlog_err("%s list num = 0, not send file_id to muc\n", __func__);
		return -EINVAL;
	}
	dyn_req->file_count = check_file_list(dyn_req->file_count, dyn_req->file_list);

	//transfer_for_akm(dyn_req->file_count, dyn_req->file_list);
	return send_fileid_to_mcu();
}

static int handpress_sensor_detect(struct device_node *dn, struct handpress_platform_data *handpress_data)
{
	int ret = 0;
	int chip_check_result = 0;
	struct device_node *temp = NULL;
	int num_handpress = 0;
	const char *state = NULL;
	int chips = 0;
	u32 bus_num = 0;
	u32 id = 0, points = 0, reg = 0;
	char chip_value[12] = { 0xff, 0xff, 0x00 };

	ret = of_property_read_u32(dn, "bus_number", &bus_num);
	if (ret) {
		hwlog_err("get bus_num err ret:%d\n", ret);
		goto out;
	}
	while ((temp = of_get_next_child(dn, temp)))
		num_handpress++;

	if (!num_handpress)
		return -ECHILD;

	for_each_child_of_node(dn, temp) {
		ret = of_property_read_string(temp, HANDPRESS_DEFAULT_STATE, &state);
		if (!ret) {
			if (strncmp(state, "on", sizeof("on")) == 0) {
				if (chips >= CYPRESS_CHIPS) {
					hwlog_err("The number of chips overflow\n");
					break;
				}
				ret = of_property_read_u32(temp, "huawei,id", &id);
				if (ret) {
					hwlog_err("huawei,id ret:%d\n", ret);
					break;
				}
				handpress_data->id[chips] = id;
				ret = of_property_read_u32(temp, "huawei,points", &points);
				if (ret) {
					hwlog_err("huawei, points ret:%d\n", ret);
					break;
				}
				handpress_data->t_pionts[chips] = points;

				ret = of_property_read_u32(temp, "reg", &reg);
				if (ret) {
					hwlog_err("reg ret:%d\n", ret);
					break;
				}
				handpress_data->i2c_address[chips] = reg;
				ret = mcu_i2c_rw((uint8_t)bus_num, (uint8_t)reg, NULL, 0, (uint8_t*)chip_value,
						(uint32_t)(sizeof(chip_value) /sizeof(chip_value[0])));
				if (ret < 0) {
					hwlog_err("mcu_i2c_rw read ret:%d\n", ret);
					chip_check_result |= 1 << id;
					continue;
				}
				hwlog_err("chip_value: %2x %2x %2x %2x\n",chip_value[0],chip_value[1],chip_value[2],chip_value[3]);
				chips++;
			}
		}
	}

	if (!chip_check_result)
		ret = 0;
	else
		ret = -1;

	hwlog_info("handpress detect result:%d\n", ret);
out:
	return ret;
}

static int is_cap_prox_shared_with_sar(struct device_node *dn)
{
	int sar_shared_flag = 0, ret = 0, i2c_address = 0, i2c_bus_num = 0;

	ret = of_property_read_u32(dn, "shared_flag", &sar_shared_flag);
	if (!ret && sar_shared_flag) {
		hwlog_info("sar chip shared with key.\n");
		if (of_property_read_u32(dn, "bus_number", &i2c_bus_num) || of_property_read_u32(dn, "reg", &i2c_address)) {
			hwlog_err("read sar sensor bus/reg err.\n");
		}
		sar_pdata.cfg.bus_num = (uint8_t)i2c_bus_num;
		sar_pdata.cfg.i2c_address = (uint8_t)i2c_address;
		return 0;
	}
	return -1;
}

static int get_key_chip_type(struct device_node *dn)
{
	int ret = 0, ctype = 0;

	ret = of_property_read_u32(dn, "chip_type", &ctype);
	if (ret < 0) {
		hwlog_err("read chip type err. ret:%d\n", ret);
	}
	return ctype;
}

static int key_sensor_detect(struct device_node *dn)
{
	int ret = 0;
	int reg = 0;
	int bootloader_reg = 0;
	uint8_t values[12] = {0};
	int chip_id_register = 0;

	ret = of_property_read_u32(dn, "reg", &reg);
	if (ret < 0) {
		hwlog_err("read reg err. ret:%d\n", ret);
		return -1;
	}
	ret = of_property_read_u32(dn, "reg_bootloader", &bootloader_reg);
	if (ret < 0){
		hwlog_err("read reg_bootloader err. ret:%d\n", ret);
		return -1;
	}
	hwlog_info("[%s] debug key reg:%d, btld reg:%d\n", __func__, reg, bootloader_reg);
	msleep(50);
	ret = mcu_i2c_rw(0, (uint8_t)bootloader_reg, NULL, 0, values, (uint32_t)(sizeof(values)/sizeof(values[0])));
	if (ret < 0) {
		hwlog_info("[%s][28] %d %d %d %d %d %d %d %d\n", __func__,
			values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7]);
		msleep(10);
		chip_id_register = 0x46;
		ret = mcu_i2c_rw(0, (uint8_t)reg, (uint8_t*)&chip_id_register, 1, values, 2);
		if (ret < 0) {
			hwlog_err("i2c 27 read err\n");
			return -1;
		}
	}

	hwlog_info("[%s][28] %d %d %d %d %d %d %d %d\n", __func__,
			values[0], values[1], values[2], values[3],	values[4], values[5], values[6], values[7]);
	return 0;
}

#define FINGERPRINT_SENSOR_DETECT_SUCCESS 10
#define FINGERPRINT_WRITE_CMD_LEN 7
#define FINGERPRINT_READ_CONTENT_LEN 2
static int fingerprint_sensor_detect(struct device_node *dn, int index, struct sensor_combo_cfg *cfg)
{
	int ret = 0;
	int temp = 0;
	char *sensor_vendor = NULL;
	uint8_t tx[FINGERPRINT_WRITE_CMD_LEN] = {0, 0, 0, 0, 0, 0, 0};
	uint8_t rx[FINGERPRINT_READ_CONTENT_LEN] = {0, 0};
	uint32_t tx_len = 0;
	uint32_t rx_len = 0;
	union SPI_CTRL ctrl;
	GPIO_NUM_TYPE gpio_reset = 0;
	GPIO_NUM_TYPE gpio_cs = 0;
	GPIO_NUM_TYPE gpio_irq = 0;

	int irq_value = 0;
	int reset_value = 0;

	if (of_property_read_u32(dn, "gpio_reset", &temp))
		hwlog_err("%s:read gpio_reset fail\n", __func__);
	else
		gpio_reset = (GPIO_NUM_TYPE)temp;
	if (of_property_read_u32(dn, "gpio_cs", &temp))
		hwlog_err("%s:read gpio_cs fail\n", __func__);
	else
		gpio_cs = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_irq", &temp))
	{ hwlog_err("%s:read gpio_irq fail\n", __func__); }
	else
	{ gpio_irq = (GPIO_NUM_TYPE)temp; }

	if (sensor_manager[index].detect_result == DET_FAIL)
	{
		reset_value = gpio_get_value(gpio_reset);
		irq_value   = gpio_get_value(gpio_irq);
	}

	ret = of_property_read_string(dn, "compatible", (const char **)&sensor_vendor);
	if (!ret) {
		if (!strncmp(sensor_vendor, "fpc", 3)) {
			gpio_direction_output(gpio_reset, 1);
			msleep(10);
		}else if(!strncmp(sensor_vendor, "syna", 4)){
			gpio_direction_output(gpio_reset, 1);
			msleep(10);
			gpio_direction_output(gpio_reset, 0);
			msleep(10);
			gpio_direction_output(gpio_reset, 1);

			ctrl.data = gpio_cs;

			tx[0] = 0xF0;
			tx[1] = 0xF0;
			tx_len = 2;
			ret = mcu_spi_rw(2, ctrl, tx, tx_len, NULL, 0); //set sensor reset by software
			msleep(100);
		} else if(!strncmp(sensor_vendor, "goodix", 6)) {
			gpio_direction_output(gpio_reset, 1);
			msleep(10);
			gpio_direction_output(gpio_reset, 0);
			msleep(10);
			gpio_direction_output(gpio_reset, 1);

			ctrl.data = gpio_cs;

			tx[0] = 0xC0;
			tx_len = 1;
			ret = mcu_spi_rw(2, ctrl, tx, tx_len, NULL, 0); //set sensor to idle mode

			tx[0] = 0xF0;
			tx[1] = 0x01;
			tx[2] = 0x26;
			tx_len = 3;
			ret = mcu_spi_rw(2, ctrl, tx, tx_len, NULL, 0); //write cmd 0xF0 & address 0x0126

			tx[0] = 0xF1;
			tx_len = 1;
			rx_len = 2;
			ret = mcu_spi_rw(2, ctrl, tx, tx_len, rx, rx_len); //read cmd 0xF1
			if ((0x00 != rx[0]) || (0x00 != rx[1])) {
				tx[0] = 0xF0;
				tx[1] = 0x01;
				tx[2] = 0x24;
				tx[3] = 0x00;
				tx[4] = 0x01;
				tx[5] = rx[0];  //after reset, bit10 is 1, such as 0x04;
				tx[6] = rx[1];
				tx_len = 7;
				ret = mcu_spi_rw(2, ctrl, tx, tx_len, NULL, 0); //write cmd 0xF0 & address 0x0124, clear irq
			}

			tx[0] = 0xC0;
			tx_len = 1;
			ret = mcu_spi_rw(2, ctrl, tx, tx_len, NULL, 0); //set sensor to idle mode

			tx[0] = 0xF0;
			tx[1] = 0x00;
			tx[2] = 0x00;
			tx_len = 3;
			ret = mcu_spi_rw(2, ctrl, tx, tx_len, NULL, 0); //write cmd & address
		} else if(!strncmp(sensor_vendor, "silead", 6)) {
			gpio_direction_output(gpio_reset, 1);
			msleep(10);
			gpio_direction_output(gpio_reset, 0);
			msleep(10);
			gpio_direction_output(gpio_reset, 1);
			msleep(5);
		} else if(!strncmp(sensor_vendor, "qfp", 3)) {
			_device_detect(dn, index, cfg);
			hwlog_info("%s: fingerprint device %s detect bypass\n", __func__, sensor_vendor);
			return FINGERPRINT_SENSOR_DETECT_SUCCESS;
		}
		hwlog_info("%s: fingerprint device %s\n", __func__, sensor_vendor);
		ret = 0;
	} else {
		hwlog_err("%s: get sensor_vendor err!\n", __func__);
		ret = -1;
	}

	if (sensor_manager[index].detect_result == DET_FAIL)
	{
		if (1 == irq_value)
		{
			gpio_direction_output(gpio_reset, reset_value);
			gpio_direction_output(gpio_irq, irq_value);
		}

		hwlog_info("%s: fingerprint device after irq_value = %d, reset_value = %d\n", __func__, irq_value, reset_value);
	}

	return ret;
}

static int fingerprint_ud_sensor_detect(struct device_node *dn, int index, struct sensor_combo_cfg *cfg)
{
	int ret = 0;
	int temp = 0;
	char *sensor_vendor = NULL;
	uint8_t tx[3] = {0, 0, 0};
	uint32_t tx_len = 1;
	union SPI_CTRL ctrl;
	GPIO_NUM_TYPE gpio_reset = 0;
	GPIO_NUM_TYPE gpio_cs = 0;
	GPIO_NUM_TYPE gpio_irq = 0;

	int irq_value = 0;
	int reset_value = 0;

	if (of_property_read_u32(dn, "gpio_reset", &temp))
		hwlog_err("%s:read gpio_reset fail\n", __func__);
	else
		gpio_reset = (GPIO_NUM_TYPE)temp;
	if (of_property_read_u32(dn, "gpio_cs", &temp))
		hwlog_err("%s:read gpio_cs fail\n", __func__);
	else
		gpio_cs = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_irq", &temp))
	{ hwlog_err("%s:read gpio_irq fail\n", __func__); }
	else
	{ gpio_irq = (GPIO_NUM_TYPE)temp; }

	if (sensor_manager[index].detect_result == DET_FAIL)
	{
		reset_value = gpio_get_value(gpio_reset);
		irq_value   = gpio_get_value(gpio_irq);
	}

	ret = of_property_read_string(dn, "compatible", (const char **)&sensor_vendor);
	if (!ret) {
		/* compare the string, and len is 15 */
		if(!strncmp(sensor_vendor, "goodix,goodixG2", 15)) {
			gpio_direction_output(gpio_reset, 1);
			msleep(10);
			gpio_direction_output(gpio_reset, 0);
			msleep(10);
			gpio_direction_output(gpio_reset, 1);
			msleep(10);
			ctrl.data = gpio_cs;

			tx[0] = 0xC0; /* cmd */
			tx_len = 1;
			ret = mcu_spi_rw(2, ctrl, tx, tx_len, NULL, 0); /*set sensor to idle mode */

			msleep(100);
			tx[0] = 0xF0; /* addr of the cmd */
			tx[1] = 0x43;
			tx[2] = 0x04;
			tx_len = 3;
			ret = mcu_spi_rw(2, ctrl, tx, tx_len, NULL, 0); /* write cmd & address,2 is bus num */
		}
		else if(!strncmp(sensor_vendor, "goodix", 6)) {
			gpio_direction_output(gpio_reset, 1);
			msleep(10);
			gpio_direction_output(gpio_reset, 0);
			msleep(10);
			gpio_direction_output(gpio_reset, 1);

			ctrl.data = gpio_cs;

			tx[0] = 0xC0;
			tx_len = 1;
			ret = mcu_spi_rw(2, ctrl, tx, tx_len, NULL, 0); //set sensor to idle mode

			msleep(100);
			tx[0] = 0xF0;
			tx[1] = 0x00;
			tx[2] = 0x04;
			tx_len = 3;
			ret = mcu_spi_rw(2, ctrl, tx, tx_len, NULL, 0); //write cmd & address
		} else if (!strncmp(sensor_vendor, "syna", 4)) { // 4 byte len
			gpio_direction_output(gpio_reset, 1);
			msleep(10); //10ms
			gpio_direction_output(gpio_reset, 0);
			msleep(10); //10ms
			gpio_direction_output(gpio_reset, 1);

			ctrl.data = gpio_cs;

			/* send 2 byte cmd to software reset sensor */
			tx[0] = 0xF0;
			tx[1] = 0xF0;
			tx_len = 2;
			ret = mcu_spi_rw(2, ctrl, tx, tx_len, NULL, 0); //set sensor reset by software
			msleep(100); //100ms
			hwlog_info("%s: fingerprint device %s Finish!\n", __func__, sensor_vendor);
		} else if(!strncmp(sensor_vendor, "qfp", 3)) {
			_device_detect(dn, index, cfg);
			hwlog_info("%s: fingerprint device %s detect bypass\n", __func__, sensor_vendor);
			return FINGERPRINT_SENSOR_DETECT_SUCCESS;
		}else if(!strncmp(sensor_vendor, "silead", 6)) {
			gpio_direction_output(gpio_reset, 1);
			msleep(10);
			gpio_direction_output(gpio_reset, 0);
			msleep(10);
			gpio_direction_output(gpio_reset, 1);
			msleep(5);
		}
		hwlog_info("%s: fingerprint device %s\n", __func__, sensor_vendor);
		ret = 0;
	} else {
		hwlog_err("%s: get sensor_vendor err!\n", __func__);
		ret = -1;
	}

	if (sensor_manager[index].detect_result == DET_FAIL)
	{
		if (1 == irq_value)
		{
			gpio_direction_output(gpio_reset, reset_value);
			gpio_direction_output(gpio_irq, irq_value);
		}

		hwlog_info("%s: fingerprint device after irq_value = %d, reset_value = %d\n", __func__, irq_value, reset_value);
	}

	return ret;
}

__weak const char* ts_kit_get_vendor_name(void) { return NULL; }
int get_combo_bus_tag(const char *bus, uint8_t *tag);

static int tp_ud_sensor_detect(struct device_node *dn)
{
	int ret;
	const char *configed_vendor_name;
	const char *attached_vendor_name;
	const char *bus_type;
	int temp = 0;

	ret = of_property_read_string(dn, "vendor_name", &configed_vendor_name);
	if (ret) {
		hwlog_err("%s:ic name not configed\n", __func__);
		return ret;
	}

	ret = of_property_read_string(dn, "bus_type", &bus_type);
	if (ret) {
		hwlog_err("%s:ic name not configed\n", __func__);
		return ret;
	}
	ret = get_combo_bus_tag(bus_type,(uint8_t *)&temp);
	if (ret) {
		hwlog_err("%s:bus_typeis invalid\n.", __func__);
		return ret;
	}
	tp_ud_data.cfg.bus_type = temp;
	ret = of_property_read_u32(dn, "bus_number", &temp);
	if (ret) {
		hwlog_err("%s:bus_number not configed\n", __func__);
		return ret;
	}
	tp_ud_data.cfg.bus_num = temp;

	if (tp_ud_data.cfg.bus_type == TAG_I2C  || tp_ud_data.cfg.bus_type == TAG_I3C){
		ret = of_property_read_u32(dn, "i2c_address", &temp);
		if (ret) {
			hwlog_err("%s:i2c_address not configed\n", __func__);
			return ret;
		}
		tp_ud_data.cfg.i2c_address = temp;
	}

	attached_vendor_name = ts_kit_get_vendor_name();
	if (!attached_vendor_name) {
		hwlog_err("%s:no attached ic\n", __func__);
		return -ENODEV;
	}

	if (!strcmp(configed_vendor_name, attached_vendor_name)) {
		hwlog_info("%s:tp detect succ, ic:%s\n", __func__, configed_vendor_name);
		return 0;
	}

	return -ENODEV;
}

static void parse_tp_ud_algo_conf(void)
{
	int temp = 0;
	struct device_node *dn;

	dn = of_find_compatible_node(NULL, NULL, "up_tp,algo_config");
	if (!dn) {
		hwlog_err("%s:no config, skip\n", __func__);
		memset(&tp_ud_data.algo_conf, 0, sizeof(struct tp_ud_algo_config));
		return;
	}

	if (of_property_read_u32(dn, "move_area_x_min", &temp)){
		hwlog_err("%s:read move_area_x_min fail\n", __func__);
	} else {
		tp_ud_data.algo_conf.move_area_x_min = temp;
	}
	if (of_property_read_u32(dn, "move_area_x_max", &temp)){
		hwlog_err("%s:read move_area_x_max fail\n", __func__);
	} else {
		tp_ud_data.algo_conf.move_area_x_max = (GPIO_NUM_TYPE) temp;
	}
	if (of_property_read_u32(dn, "move_area_y_min", &temp)){
		hwlog_err("%s:read move_area_y_min fail\n", __func__);
	} else {
		tp_ud_data.algo_conf.move_area_y_min = (GPIO_NUM_TYPE) temp;
	}
	if (of_property_read_u32(dn, "move_area_y_max", &temp)){
		hwlog_err("%s:read move_area_y_max fail\n", __func__);
	} else {
		tp_ud_data.algo_conf.move_area_y_max = (GPIO_NUM_TYPE) temp;
	}
	if (of_property_read_u32(dn, "finger_area_x_min", &temp)){
		hwlog_err("%s:read finger_area_x_min fail\n", __func__);
	} else {
		tp_ud_data.algo_conf.finger_area_x_min = (GPIO_NUM_TYPE) temp;
	}
	if (of_property_read_u32(dn, "finger_area_x_max", &temp)){
		hwlog_err("%s:read finger_area_x_max fail\n", __func__);
	} else {
		tp_ud_data.algo_conf.finger_area_x_max = (GPIO_NUM_TYPE) temp;
	}
	if (of_property_read_u32(dn, "finger_area_y_min", &temp)){
		hwlog_err("%s:read finger_area_y_min fail\n", __func__);
	} else {
		tp_ud_data.algo_conf.finger_area_y_min = (GPIO_NUM_TYPE) temp;
	}
	if (of_property_read_u32(dn, "finger_area_y_max", &temp)){
		hwlog_err("%s:read finger_area_y_max fail\n", __func__);
	} else {
		tp_ud_data.algo_conf.finger_area_y_max = (GPIO_NUM_TYPE) temp;
	}
	if (of_property_read_u32(dn, "coor_scale", &temp)){
		hwlog_err("%s:read coor_scale fail\n", __func__);
	} else {
		tp_ud_data.algo_conf.coor_scale = (GPIO_NUM_TYPE) temp;
	}
}

static void read_tp_ud_from_dts(struct device_node *dn)
{
	int temp = 0;

	read_chip_info(dn, TP_UD);
	parse_tp_ud_algo_conf();

	if (of_property_read_u32(dn, "file_id", &temp)){
		hwlog_err("%s:read tp ud file_id fail\n", __func__);
	} else {
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;
	}
	dyn_req->file_count++;
	hwlog_info("tp ud file id is %d\n", temp);

	if (of_property_read_u32(dn, "spi_max_speed_hz", &temp)){
		hwlog_err("%s:read spi_max_speed_hz fail\n", __func__);
	} else {
		tp_ud_data.spi_max_speed_hz = temp;
	}

	if (of_property_read_u32(dn, "spi_mode", &temp)){
		hwlog_err("%s:read spi_mode fail\n", __func__);
	} else {
		tp_ud_data.spi_mode = temp;
	}

	if (of_property_read_u32(dn, "gpio_irq", &temp)){
		hwlog_err("%s:read gpio_irq fail\n", __func__);
	} else {
		tp_ud_data.gpio_irq = (GPIO_NUM_TYPE) temp;
	}

	if (of_property_read_u32(dn, "gpio_irq_sh", &temp)){
		hwlog_err("%s:read gpio_irq_sh fail\n", __func__);
	} else {
		tp_ud_data.gpio_irq_sh = (GPIO_NUM_TYPE) temp;
	}

	if (of_property_read_u32(dn, "gpio_cs", &temp)){
		hwlog_err("%s:read gpio_cs fail\n", __func__);
	} else {
		tp_ud_data.gpio_cs = (GPIO_NUM_TYPE) temp;
	}

	if (of_property_read_u32(dn, "pressure_support", &temp)){
		hwlog_err("%s:read pressure_support fail\n", __func__);
	} else {
		tp_ud_data.pressure_support = (GPIO_NUM_TYPE) temp;
	}

	if (of_property_read_u32(dn, "anti_forgery_support", &temp)){
		hwlog_err("%s:read anti_forgery_support fail\n", __func__);
	} else {
		tp_ud_data.anti_forgery_support = (GPIO_NUM_TYPE) temp;
	}

	if (of_property_read_u32(dn, "ic_type", &temp)){
		hwlog_err("%s:read low_power_addr fail\n", __func__);
	} else {
		tp_ud_data.ic_type = (GPIO_NUM_TYPE) temp;
	}

	if (of_property_read_u32(dn, "hover_enable", &temp)){
		hwlog_err("%s:read event_info_addr fail  use default\n", __func__);
	} else {
		tp_ud_data.hover_enable = (GPIO_NUM_TYPE) temp;
	}

	if (of_property_read_u32(dn, "i2c_max_speed_hz", &temp)){
		hwlog_err("%s:read i2c_max_speed_hz fail\n", __func__);
	} else {
		tp_ud_data.i2c_max_speed_hz = (GPIO_NUM_TYPE) temp;
	}
	if (of_property_read_u32(dn, "fw_power_config_reg", &temp)){
		hwlog_err("%s:read fw_power_config_reg not config\n", __func__);
		tp_ud_data.fw_power_config_reg = 0;
	} else {
		tp_ud_data.fw_power_config_reg = (GPIO_NUM_TYPE) temp;
		hwlog_err("%s:read fw_power_config_reg = %d\n", __func__,tp_ud_data.fw_power_config_reg );
	}

	if (of_property_read_u32(dn, "fw_touch_data_reg", &temp)){
		hwlog_err("%s:read fw_touch_data_reg not config\n", __func__);
		tp_ud_data.fw_touch_data_reg = 0;
	} else {
		tp_ud_data.fw_touch_data_reg = (GPIO_NUM_TYPE) temp;
		hwlog_err("%s:read fw_touch_data_reg = %d\n", __func__,tp_ud_data.fw_touch_data_reg );
	}

	if (of_property_read_u32(dn, "fw_touch_command_reg", &temp)){
		hwlog_err("%s:read fw_touch_command_reg not config\n", __func__);
		tp_ud_data.fw_touch_command_reg = 0;
	} else {
		tp_ud_data.fw_touch_command_reg = (GPIO_NUM_TYPE) temp;
		hwlog_err("%s:read fw_touch_command_reg = %d\n", __func__,tp_ud_data.fw_touch_command_reg );
	}

	if (of_property_read_u32(dn, "fw_addr_3", &temp)){
		hwlog_err("%s:read fw_addr_3 not config\n", __func__);
		tp_ud_data.fw_addr_3 = 0;
	} else {
		tp_ud_data.fw_addr_3 = (GPIO_NUM_TYPE) temp;
		hwlog_err("%s:read fw_addr_3 = %d\n", __func__,tp_ud_data.fw_addr_3 );
	}

	if (of_property_read_u32(dn, "fw_addr_4", &temp)){
		hwlog_err("%s:read fw_addr_4 not config\n", __func__);
		tp_ud_data.fw_addr_4 = 0;
	} else {
		tp_ud_data.fw_addr_4 = (GPIO_NUM_TYPE) temp;
		hwlog_err("%s:read fw_addr_4 = %d\n", __func__,tp_ud_data.fw_addr_4 );
	}

	if (of_property_read_u32(dn, "fw_addr_5", &temp)){
		hwlog_err("%s:read fw_addr_5 not config\n", __func__);
		tp_ud_data.fw_addr_5 = 0;
	} else {
		tp_ud_data.fw_addr_5 = (GPIO_NUM_TYPE) temp;
		hwlog_err("%s:read fw_addr_5 = %d\n", __func__,tp_ud_data.fw_addr_5 );
	}

	if (of_property_read_u32(dn, "fw_addr_6", &temp)){
		hwlog_err("%s:read fw_addr_6 not config\n", __func__);
		tp_ud_data.fw_addr_6 = 0;
	} else {
		tp_ud_data.fw_addr_6 = (GPIO_NUM_TYPE) temp;
		hwlog_err("%s:read fw_addr_6 = %d\n", __func__,tp_ud_data.fw_addr_6 );
	}

	if (of_property_read_u32(dn, "fw_addr_7", &temp)){
		hwlog_err("%s:read fw_addr_7 not config\n", __func__);
		tp_ud_data.fw_addr_7 = 0;
	} else {
		tp_ud_data.fw_addr_7 = (GPIO_NUM_TYPE) temp;
		hwlog_err("%s:read fw_addr_7 = %d\n", __func__,tp_ud_data.fw_addr_7 );
	}
	if (of_property_read_u32(dn, "tp_sensorhub_irq_flag", &temp)){
		hwlog_err("%s:read tp_sensorhub_irq_flag not config\n", __func__);
		tp_ud_data.tp_sensorhub_irq_flag = 0;
	} else {
		tp_ud_data.tp_sensorhub_irq_flag = (GPIO_NUM_TYPE) temp;
		hwlog_err("%s:read tp_sensorhub_irq_flag = %d\n",
			__func__,tp_ud_data.tp_sensorhub_irq_flag );
	}
	if (of_property_read_u32(dn,
		"tp_sensor_spi_sync_cs_low_delay_us", &temp)) {
		hwlog_err("%s:read tp_sensor_spi_sync_cs_low_delay_us not config\n",
			__func__);
		tp_ud_data.tp_sensor_spi_sync_cs_low_delay_us = 0;
	} else {
		tp_ud_data.tp_sensor_spi_sync_cs_low_delay_us =
							(GPIO_NUM_TYPE) temp;
		hwlog_err("%s:read tp_sensor_spi_sync_cs_low_delay_us = %d\n",
			__func__, tp_ud_data.tp_sensor_spi_sync_cs_low_delay_us);
	}
	return;
}

int detect_disable_sample_task_prop(struct device_node *dn, uint32_t *value)
{
	int ret;

	ret = of_property_read_u32(dn, "disable_sample_task", value);
	if (ret)
		return -1;

	return 0;
}

int get_combo_bus_tag(const char *bus, uint8_t *tag)
{
	obj_tag_t tag_tmp = TAG_END;
	if (!strcmp(bus, "i2c")) {
		tag_tmp = TAG_I2C;
	} else if(!strcmp(bus, "spi")) {
		tag_tmp = TAG_SPI;
	}	else if(!strcmp(bus, "i3c")) {
		tag_tmp = TAG_I3C;
	}

	if (tag_tmp == TAG_END) {
		return -1;
	}
	*tag = (uint8_t)tag_tmp;
	return 0;
}

static int get_combo_prop(struct device_node *dn, struct detect_word *p_det_wd)
{
	int ret;
	struct property *prop;
	const char *bus_type;
	uint32_t u32_temp;

	/*combo_bus_type*/
	ret = of_property_read_string(dn, "combo_bus_type", &bus_type);
	if (ret) {
		hwlog_err("%s: get bus_type err!\n", __func__);
		return ret;
	}
	if (get_combo_bus_tag(bus_type, &p_det_wd->cfg.bus_type)) {
		hwlog_err("%s: bus_type(%s) err!\n", __func__, bus_type);
		return -1;
	}

	/*combo_bus_num*/
	ret = of_property_read_u32(dn, "combo_bus_num", &u32_temp);
	if (ret) {
		hwlog_err("%s: get combo_data err!\n", __func__);
		return ret;
	}
	p_det_wd->cfg.bus_num = (uint8_t)u32_temp;

	/*combo_data*/
	ret = of_property_read_u32(dn, "combo_data", &p_det_wd->cfg.data);
	if (ret) {
		hwlog_err("%s: get combo_data err!\n", __func__);
		return ret;
	}

	/*combo_tx*/
	prop = of_find_property(dn, "combo_tx", NULL);
	if (!prop) {
		hwlog_err("%s: get combo_tx err!\n", __func__);
		return -1;
	}
	p_det_wd->tx_len = (uint32_t)prop->length;
	if (p_det_wd->tx_len > sizeof(p_det_wd->tx)) {
		hwlog_err("%s: get combo_tx_len(%d) too big!\n", __func__, p_det_wd->tx_len);
		return -1;
	}
	of_property_read_u8_array(dn, "combo_tx", p_det_wd->tx, (size_t)prop->length);

	/*combo_rx_mask*/
	prop = of_find_property(dn, "combo_rx_mask", NULL);
	if (!prop) {
		hwlog_err("%s: get combo_rx_mask err!\n", __func__);
		return -1;
	}
	p_det_wd->rx_len = (uint32_t)prop->length;
	if (p_det_wd->rx_len > sizeof(p_det_wd->rx_msk)) {
		hwlog_err("%s: get rx_len(%d) too big!\n", __func__, p_det_wd->rx_len);
		return -1;
	}
	of_property_read_u8_array(dn, "combo_rx_mask", p_det_wd->rx_msk, (size_t)prop->length);

	/*combo_rx_exp*/
	prop = of_find_property(dn, "combo_rx_exp", NULL);
	if (!prop) {
		hwlog_err("%s: get combo_rx_exp err!\n", __func__);
		return -1;
	}
	prop->length = (uint32_t)prop->length;
	if ((ssize_t)prop->length > sizeof(p_det_wd->rx_exp) || ((uint32_t)prop->length % p_det_wd->rx_len)) {
		hwlog_err("%s: rx_exp_len(%d) not available!\n", __func__, prop->length);
		return -1;
	}
	p_det_wd->exp_n = (uint32_t)prop->length / p_det_wd->rx_len;
	of_property_read_u8_array(dn, "combo_rx_exp", p_det_wd->rx_exp, (size_t)prop->length);

	return 0;
}

static int detect_i2c_device(struct device_node *dn, char *device_name)
{
	int i = 0, ret = 0, i2c_address = 0, i2c_bus_num = 0;
	int register_add = 0, len = 0;
	u32 wia[10] = { 0 };
	uint8_t detected_device_id[4] ={0};
	uint8_t tag = TAG_I2C;
	uint32_t register_add_len = 0;
	uint32_t rx_len = 0;
	uint32_t device_id ;
	struct property *prop = NULL;
	const char *bus_type;


	if (of_property_read_u32(dn, "bus_number", &i2c_bus_num)
	    || of_property_read_u32(dn, "reg", &i2c_address)
	    || of_property_read_u32(dn, "chip_id_register", &register_add)) {
		hwlog_err("%s:read i2c bus_number (%d)or bus address(%x) or chip_id_register(%x) from dts fail\n",
		     device_name, i2c_bus_num, i2c_address, register_add);
		return -1;
	}

	prop = of_find_property(dn, "chip_id_value", NULL);
	if (!prop)
		return -EINVAL;
	if (!prop->value)
		return -ENODATA;
	len = prop->length / 4;

	if (of_property_read_u32_array(dn, "chip_id_value", wia, len)) {
		hwlog_err("%s:read chip_id_value (id0=%x) from dts fail len=%d\n", device_name, wia[0], len);
		return -1;
	}

	if (!of_property_read_string(dn, "bus_type", &bus_type)) {
		get_combo_bus_tag(bus_type, (uint8_t *)&tag);
	}

	hwlog_info("%s:read i2c bus_type(%d) bus_number (%d) slave address(0x%x) chip_id_register(0x%x) chipid value 0x%x 0x%x 0x%x 0x%x\n",
		device_name, tag, i2c_bus_num, i2c_address, register_add, wia[0], wia[1], wia[2], wia[3]);

	if (TAG_I3C == tag) {
		ret = mcu_i3c_rw((uint8_t)i2c_bus_num, (uint8_t)i2c_address, (uint8_t*)&register_add, 1, detected_device_id, 1);
	} else {
		if(register_add & 0xFF00){
			register_add_len = 2 ;
			rx_len = 4;
		}else{
			register_add_len = 1 ;
			rx_len = 1;
		}
		ret = mcu_i2c_rw((uint8_t)i2c_bus_num, (uint8_t)i2c_address, (uint8_t*)&register_add, register_add_len, detected_device_id, rx_len);
	}

	if (ret) {
		hwlog_err("%s:detect_i2c_device:send i2c read cmd to mcu fail,ret=%d\n", device_name, ret);
		return -1;
	}
	memcpy(&device_id,detected_device_id,4);
	if(!strncmp(device_name, "vibrator", strlen("vibrator"))){
		hwlog_info("virbator temp i2c detect success,chip_value:0x%x,len:%d!\n", device_id, len);
		return 0;
	}
	for (i = 0; i < len; i++) {
		if (device_id == wia[i]) {
			hwlog_info("%s:i2c detect  suc!chip_value:0x%x\n", device_name, device_id);
			return 0;
		}
	}
	hwlog_info("%s:i2c detect fail,chip_value:0x%x,len:%d!\n", device_name, device_id, len);
	return -1;
}

static int _device_detect(struct device_node *dn, int index, struct sensor_combo_cfg *p_succ_ret)
{
	int ret = -1;
	struct detect_word det_wd;
	struct property *prop = of_find_property(dn, "combo_bus_type", NULL);
	uint8_t r_buf[max_tx_rx_len];

	memset(&det_wd, 0, sizeof(det_wd));

	if (prop) {
		uint32_t i, n;

		ret = get_combo_prop(dn, &det_wd);
		if (ret) {
			hwlog_err("%s:get_combo_prop fail\n", __func__);
			return ret;
		}

		hwlog_info("%s: combo detect bus type %d; num %d; data %d; txlen %d; tx[0] 0x%x; rxLen %d; rxmsk[0] 0x%x; n %d; rxexp[0] 0x%x",
			__func__,
			det_wd.cfg.bus_type,
			det_wd.cfg.bus_num,
			det_wd.cfg.data,
			det_wd.tx_len,
			det_wd.tx[0],
			det_wd.rx_len,
			det_wd.rx_msk[0],
			det_wd.exp_n,
			det_wd.rx_exp[0]);

		ret = combo_bus_trans(&det_wd.cfg, det_wd.tx, det_wd.tx_len, r_buf, det_wd.rx_len);
		hwlog_info("combo_bus_trans ret is %d; rx 0x%x;\n", ret, r_buf[0]);

		if (ret >= 0) { /* success */
			int rx_exp_p;
			ret = -1;                  /* fail first */
			/* check expect value */
			for (n = 0; n < det_wd.exp_n; n++) {
				for (i = 0; i < det_wd.rx_len;) {
					rx_exp_p = n * det_wd.rx_len + i;
					if ( (r_buf[i] & det_wd.rx_msk[i]) != det_wd.rx_exp[rx_exp_p]) { /* check value */
						break;
					}
					i++;
				}
				if (i == det_wd.rx_len) { /* get the success device */
					ret = 0;
					hwlog_info("%s: %s detect succ;\n", __func__, sensor_manager[index].sensor_name_str);
					break;
				}
			}
		}
	} else {
		hwlog_info("%s: [%s] donot find combo prop;\n", __func__, sensor_manager[index].sensor_name_str);
		ret = detect_i2c_device(dn, sensor_manager[index].sensor_name_str);
		if (!ret) {
			uint32_t i2c_bus_num = 0, i2c_address = 0, register_add = 0;
			if (of_property_read_u32(dn, "bus_number", &i2c_bus_num) || of_property_read_u32(dn, "reg", &i2c_address)
				|| of_property_read_u32(dn, "chip_id_register", &register_add)) {
				hwlog_err("%s:read i2c bus_number (%d)or bus address(%x) or chip_id_register(%x) from dts fail\n",
					sensor_manager[index].sensor_name_str, i2c_bus_num, i2c_address, register_add);
				return -1;
			}
			det_wd.cfg.bus_type = TAG_I2C;
			det_wd.cfg.bus_num = (uint8_t)i2c_bus_num;
			det_wd.cfg.i2c_address = (uint8_t)i2c_address;
		}
	}

	if (!ret) {
		*p_succ_ret = det_wd.cfg;
	}
	return ret;
}

static int device_detect(struct device_node *dn, int index)
{
	int ret = 0, chip_type=0;
	struct sensor_combo_cfg cfg;
	struct sensor_combo_cfg *p_cfg;
	uint32_t disable;

	if (sensor_manager[index].detect_result== DET_SUCC)
		return -1;

	if (HANDPRESS == sensor_manager[index].sensor_id) {
		ret = handpress_sensor_detect(dn, &handpress_data);
		goto out;
	} else if (FINGERPRINT == sensor_manager[index].sensor_id){
		ret = fingerprint_sensor_detect(dn, index, &cfg);
		if (FINGERPRINT_SENSOR_DETECT_SUCCESS == ret) {
			ret = 0;
			memcpy((void*)sensor_manager[index].spara, (void*)&cfg, sizeof(cfg));
			goto out;
		} else if(ret) {
			goto out;
		}
	} else if (FINGERPRINT_UD == sensor_manager[index].sensor_id){
		ret = fingerprint_ud_sensor_detect(dn, index, &cfg);
		if (FINGERPRINT_SENSOR_DETECT_SUCCESS == ret) {
			ret = 0;
			memcpy((void*)sensor_manager[index].spara, (void*)&cfg, sizeof(cfg));
			goto out;
		} else if(ret) {
			goto out;
		}
	} else if (CAP_PROX == sensor_manager[index].sensor_id){
		ret = is_cap_prox_shared_with_sar(dn);
		if(!ret)
			goto out;
	} else if (KEY == sensor_manager[index].sensor_id) {
		chip_type = get_key_chip_type(dn);
		if(chip_type) {
			ret =  key_sensor_detect(dn);
			goto out;
		}
	}else if (PS == sensor_manager[index].sensor_id) {
		ret = of_property_read_u32(dn, "replace_by_tof", &tof_replace_ps_flag);
		if (ret) {
				hwlog_info("no replace_by_tof config ret:%d\n", ret);
		}
		if(tof_replace_ps_flag){
			hwlog_info("get replace_by_tof flag %d, skip detect\n", tof_replace_ps_flag);
			goto out;
		}
	} else if (TP_UD == sensor_manager[index].sensor_id) {
		ret = tp_ud_sensor_detect(dn);
		if (ret) {
			hwlog_err("tp ud sensor detect fail\n");
		}
		goto out;
	}
	ret = _device_detect(dn, index, &cfg);
	if (!ret) {
		memcpy((void*)sensor_manager[index].spara, (void*)&cfg, sizeof(cfg));
	}
out:
	if (ret) {
		sensor_manager[index].detect_result = DET_FAIL;
		return DET_FAIL;
	} else {
		/* check disable sensor task */
		p_cfg = (struct sensor_combo_cfg *)sensor_manager[index].spara;

		ret = detect_disable_sample_task_prop(dn, &disable);
		if (!ret) {
			/* get disbale_sample_task property value */
			p_cfg->disable_sample_thread = (uint8_t)disable;
		}
		sensor_manager[index].detect_result = DET_SUCC;
		return DET_SUCC;
	}
}

static int get_sensor_index(const char* name_buf, int len)
{
	int i = 0;
	for(i = 0; i < SENSOR_MAX; i++){
		if(len != strlen(sensor_manager[i].sensor_name_str))
			continue;
		if(!strncmp(name_buf, sensor_manager[i].sensor_name_str, len))
			break;
	}
	if(i < SENSOR_MAX)
		return i;
	else{
		hwlog_err("get_sensor_detect_index fail \n");
		return -1;
	}
}

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
static void __set_hw_dev_flag(SENSOR_DETECT_LIST s_id)
{
/* detect current device successful, set the flag as present */
	switch(s_id)
	{
		case ACC:
		case ACC1:
		case ACC2:
			set_hw_dev_flag(DEV_I2C_G_SENSOR);
			break;
		case MAG:
		case MAG1:
			set_hw_dev_flag(DEV_I2C_COMPASS);
			break;
		case GYRO:
		case GYRO1:
		case GYRO2:
			set_hw_dev_flag(DEV_I2C_GYROSCOPE);
			break;
		case ALS:
		case PS:
			set_hw_dev_flag(DEV_I2C_L_SENSOR);
			break;
		case AIRPRESS:
			set_hw_dev_flag(DEV_I2C_AIRPRESS);
			break;
		case HANDPRESS:
			set_hw_dev_flag(DEV_I2C_HANDPRESS);
			break;
		case VIBRATOR:
			set_hw_dev_flag(DEV_I2C_VIBRATOR_LRA);
			break;
		case CAP_PROX:
		case CAP_PROX1:
		case FINGERPRINT:
		case KEY:
		case MAGN_BRACKET:
		case CONNECTIVITY:
		case TP_UD:
			break;
		default:
			hwlog_err("__set_hw_dev_flag:err id =%d\n", s_id);
			break;
	}
}
#endif

static int extend_config_before_sensor_detect(struct device_node *dn, int index)
{
	int ret = 0;
	SENSOR_DETECT_LIST s_id;
	s_id = sensor_manager[index].sensor_id;

	switch(s_id)
	{
		case CONNECTIVITY:
			sensor_manager[index].detect_result = DET_SUCC;
			read_connectivity_data_from_dts(dn);
			break;
		case MAGN_BRACKET:
			sensor_manager[index].detect_result = DET_SUCC;
			read_magn_bracket_data_from_dts(dn);
			break;
		case RPC:
			sensor_manager[index].detect_result = DET_SUCC;
			read_rpc_data_from_dts(dn);
			break;
		case SH_AOD:
			sensor_manager[index].detect_result = DET_SUCC;
			read_aod_data_from_dts(dn);
			break;
		case KB:
			sensor_manager[index].detect_result = DET_SUCC;
			read_kb_data_from_dts(dn);
			break;
		default:
			ret = -1;
			break;
	}
	return ret;
}

static void extend_config_after_sensor_detect(struct device_node *dn, int index)
{
	int ret = 0;
	SENSOR_DETECT_LIST s_id;
	s_id = sensor_manager[index].sensor_id;

	switch(s_id)
	{
		case ACC:
		case ACC1:
		case ACC2:
			read_acc_data_from_dts(dn, &sensor_manager[index]);
			break;
		case MAG1:
			read_mag_data_from_dts(dn, &sensor_manager[index]);
			break;
		case MAG:
			read_mag_data_from_dts(dn, &sensor_manager[index]);
			ret = fill_extend_data_in_dts(dn, str_soft_para,	mag_data.pdc_data, PDC_SIZE, EXTEND_DATA_TYPE_IN_DTS_BYTE);
			if (ret) {
				hwlog_err("%s:fill_extend_data_in_dts failed!\n", str_soft_para);
			}
			break;
		case GYRO:
		case GYRO1:
		case GYRO2:
			read_gyro_data_from_dts(dn, &sensor_manager[index]);
			break;
		case ALS:
			read_als_data_from_dts(dn);
			break;
		case PS:
			read_ps_data_from_dts(dn);
			break;
		case AIRPRESS:
			read_airpress_data_from_dts(dn);
			break;
		case HANDPRESS:
			read_handpress_data_from_dts(dn);
			break;
		case CAP_PROX:
		case CAP_PROX1:
			read_capprox_data_from_dts(dn, &sensor_manager[index]);
			break;
		case KEY:
			read_key_i2c_data_from_dts(dn);
			break;
		case FINGERPRINT:
			read_fingerprint_from_dts(dn);
			break;
		case VIBRATOR:
			read_vibrator_from_dts(dn);
			break;
		case FINGERPRINT_UD:
			read_fingerprint_ud_from_dts(dn);
			break;
		case TOF:
			read_tof_data_from_dts(dn);
			break;
		case TP_UD:
			read_tp_ud_from_dts(dn);
			break;
		default:
			hwlog_err("%s:err id =%d\n",__func__, s_id);
			break;
	}
	return;
}

#ifdef CONFIG_HUAWEI_DSM
extern struct dsm_dev dsm_sensorhub;
extern struct dsm_client* shb_dclient;
static void update_detectic_client_info(void)
{
	char sensor_name[DSM_MAX_IC_NAME_LEN] = { 0 };
	uint8_t i;
	int total_len = 0;

	for(i = 0; i < SENSOR_MAX; i++) {
		if (sensor_manager[i].detect_result == DET_FAIL) {
			total_len += strlen(sensor_manager[i].sensor_name_str);
			if(total_len < DSM_MAX_IC_NAME_LEN) {
				strcat(sensor_name, sensor_manager[i].sensor_name_str);
			}
		}
	}
	sensor_name[DSM_MAX_IC_NAME_LEN - 1] = '\0';
	hwlog_info("update_detectic_client_info %s.\n", sensor_name);
	dsm_sensorhub.ic_name = sensor_name;
	if (dsm_update_client_vendor_info(&dsm_sensorhub)) {
		hwlog_info("dsm_update_client_vendor_info failed\n");
	}
}
#endif
static uint8_t check_detect_result(DETECT_MODE mode)
{
	int i = 0;
	uint8_t detect_fail_num = 0;
	uint8_t  result = 0;
	int total_len = 0;
	char detect_result[MAX_STR_SIZE] = { 0 };
	const char *sf = " detect fail!";

	for(i = 0; i < SENSOR_MAX; i++)
	{
		result = sensor_manager[i].detect_result;
		if(result == DET_FAIL){
			detect_fail_num ++;
			total_len += strlen(sensor_manager[i].sensor_name_str);
			total_len += 2;
			if(total_len < MAX_STR_SIZE){
				strcat(detect_result,sensor_manager[i].sensor_name_str);
				strcat(detect_result,"  ");
			}
			hwlog_info("%s :  %s detect fail \n",__func__,sensor_manager[i].sensor_name_str);
		}else if(result == DET_SUCC){
			hwlog_info("%s :  %s detect success \n",__func__,sensor_manager[i].sensor_name_str);
		}
	}

	if(detect_fail_num > 0){
		s_redetect_state.need_redetect_sensor = 1;
		total_len += strlen(sf);
		if(total_len < MAX_STR_SIZE)
			strcat(detect_result,sf);

	#ifdef CONFIG_HUAWEI_DSM
		if(BOOT_DETECT_END == mode) {
			if (!dsm_client_ocuppy(shb_dclient)) {
				update_detectic_client_info();
				dsm_client_record(shb_dclient, "[%s]%s", __func__, detect_result);
				dsm_client_notify(shb_dclient, DSM_SHB_ERR_IOM7_DETECT_FAIL);
			}
		}
	#endif

	}else{
		s_redetect_state.need_redetect_sensor = 0;
	}

	if((detect_fail_num < s_redetect_state.detect_fail_num) && (REDETECT_LATER == mode)){
		s_redetect_state.need_recovery = 1;
		hwlog_info("%s : %u sensors detect success after redetect \n",__func__,s_redetect_state.detect_fail_num - detect_fail_num );
	}
	s_redetect_state.detect_fail_num = detect_fail_num;
	return detect_fail_num;
}

static void show_last_detect_fail_sensor(void)
{
	int i = 0;
	uint8_t  result = 0;
	for(i = 0; i < SENSOR_MAX; i++)
	{
		result = sensor_manager[i].detect_result;
		if(result == DET_FAIL){
			hwlog_err("last detect fail sensor:  %s \n",sensor_manager[i].sensor_name_str);
		}
	}
}

static void read_cap_prox1_info(struct device_node *dn)
{
	int register_add = 0;
	int i2c_address = 0;
	int i2c_bus_num = 0;
	u32 wia[2] = { 0 };
	char *chip_info = NULL;

	if (of_property_read_u32(dn, "bus_number", &i2c_bus_num) ||
		of_property_read_u32(dn, "reg", &i2c_address) ||
		of_property_read_u32(dn, "chip_id_register", &register_add)) {
		hwlog_err("sar sensor :read i2c bus_number (%d)or bus\
		address(%x) orchip_id_register(%8x)\from dts fail\n",
		i2c_bus_num, i2c_address, register_add);
	}

	if (of_property_read_u32_array(dn, "chip_id_value", wia, 2))
		hwlog_err("sar1 sensor:read chip_id_value (id0=0x%8x id1=0x%8x)\
		from dts fail\n", wia[0], wia[1]);

	hwlog_info("sar sensor:bus_number (%d) slave address(0x%x)\
		chip_id_register (0x%8x)chipid value0x%8x 0x%8x\n",
		i2c_bus_num, i2c_address, register_add, wia[0], wia[1]);

	if (of_property_read_string(dn, "compatible", (const char **)&chip_info))
		hwlog_err("%s:read name_id:CAP_PROX info fail.\n", __func__);

	if (!strncmp(chip_info, "huawei,semtech-sx9335",
		strlen("huawei,semtech-sx9335"))) {
		hwlog_info("sar sensor from dts is semtech-sx9335\n");
		semtech_9335_sar_detect.detect_flag = 1;
		semtech_9335_sar_detect.cfg.bus_num = (uint8_t)i2c_bus_num;
		semtech_9335_sar_detect.cfg.i2c_address = (uint8_t)i2c_address;
		semtech_9335_sar_detect.chip_id = (uint16_t)register_add;
		semtech_9335_sar_detect.chip_id_value[0] = (uint32_t)wia[0];
		semtech_9335_sar_detect.chip_id_value[1] = (uint32_t)wia[1];
	}
}

static void read_cap_prox_info(struct device_node *dn)
{
	int register_add = 0, i2c_address = 0, i2c_bus_num = 0;
	u32 wia[2] = { 0 };
	char *chip_info = NULL;

	if (of_property_read_u32(dn, "bus_number", &i2c_bus_num) || of_property_read_u32(dn, "reg", &i2c_address)
	    		|| of_property_read_u32(dn, "chip_id_register", &register_add)) {
		hwlog_err("sar sensor :read i2c bus_number (%d)or bus address(%x) or chip_id_register(%x) from dts fail\n",
		   	 i2c_bus_num, i2c_address, register_add);
	}

	if (of_property_read_u32_array(dn, "chip_id_value", wia, 2)) {
		hwlog_err("sar sensor:read chip_id_value (id0=0x%x id1=0x%x) from dts fail.\n",wia[0],wia[1]);
	}

	hwlog_info("sar sensor:bus_number (%d) slave address(0x%x) chip_id_register(0x%x) chipid value 0x%x 0x%x\n",
	    	i2c_bus_num, i2c_address, register_add, wia[0],wia[1]);

	if (of_property_read_string(dn, "compatible", (const char **)&chip_info))
		hwlog_err("%s:read name_id:CAP_PROX info fail.\n", __func__);

	if (!strncmp(chip_info, "huawei,semtech-sx9323", strlen("huawei,semtech-sx9323"))) {
		hwlog_info("sar sensor from dts is semtech-sx9323\n");
		semtech_sar_detect.detect_flag= 1;
		semtech_sar_detect.cfg.bus_num = (uint8_t)i2c_bus_num;
		semtech_sar_detect.cfg.i2c_address = (uint8_t)i2c_address;
		semtech_sar_detect.chip_id = (uint16_t)register_add;
		semtech_sar_detect.chip_id_value[0] = (uint32_t)wia[0];
		semtech_sar_detect.chip_id_value[1] = (uint32_t)wia[1];
	} else if (!strncmp(chip_info, "huawei,adi-adux1050", strlen("huawei,adi-adux1050"))) {
		hwlog_info("sar sensor from dts is adi-adux1050\n");
		adi_sar_detect.detect_flag= 1;
		adi_sar_detect.cfg.bus_num = (uint8_t)i2c_bus_num;
		adi_sar_detect.cfg.i2c_address = (uint8_t)i2c_address;
		adi_sar_detect.chip_id = (uint16_t)register_add;
		adi_sar_detect.chip_id_value[0] = (uint32_t)wia[0];
		adi_sar_detect.chip_id_value[1] = (uint32_t)wia[1];
	}else if (!strncmp(chip_info, "huawei,cypress_sar_psoc4000", strlen("huawei,cypress_sar_psoc4000"))){
		hwlog_info("sar sensor from dts is cypress_sar_psoc4000\n");
		cypress_sar_detect.detect_flag= 1;
		cypress_sar_detect.cfg.bus_num = (uint8_t)i2c_bus_num;
		cypress_sar_detect.cfg.i2c_address = (uint8_t)i2c_address;
		cypress_sar_detect.chip_id = (uint16_t)register_add;
		cypress_sar_detect.chip_id_value[0] = (uint32_t)wia[0];
		cypress_sar_detect.chip_id_value[1] = (uint32_t)wia[1];
	} else if (!strncmp(chip_info, "huawei,abov-a96t3x6", strlen("huawei,abov-a96t3x6"))) {
		hwlog_info("sar sensor from dts is abov-a96t3x6\n");
		g_abov_sar_detect.detect_flag = 1;
		g_abov_sar_detect.cfg.bus_num = (uint8_t)i2c_bus_num;
		g_abov_sar_detect.cfg.i2c_address = (uint8_t)i2c_address;
		g_abov_sar_detect.chip_id = (uint16_t)register_add;
		g_abov_sar_detect.chip_id_value[0] = (uint32_t)wia[0];
		g_abov_sar_detect.chip_id_value[1] = (uint32_t)wia[1];
	}
}

static void redetect_failed_sensors(DETECT_MODE mode)
{
	int index = 0;
	char *sensor_ty = NULL;
	char *sensor_st = NULL;
	struct device_node *dn = NULL;
	const char *st = "disabled";

	for_each_node_with_property(dn, "sensor_type") {
		//sensor type
		if (of_property_read_string(dn, "sensor_type", (const char **)&sensor_ty)) {
			hwlog_err("redetect get sensor type fail.\n");
			continue;
		}
		index = get_sensor_index(sensor_ty, strlen(sensor_ty));
		if(index < 0) {
			hwlog_err("redetect get sensor index fail.\n");
			continue;
		}
		//sensor status:ok or disabled
		if (of_property_read_string(dn, "status", (const char **)&sensor_st)) {
			hwlog_err("redetect get sensor status fail.\n");
			continue;
		}
		if (!strcmp(st,sensor_st)) {
			hwlog_info("%s : sensor %s status is %s \n",__func__,sensor_ty,sensor_st);
			continue;
		}
		if (device_detect(dn,index) != DET_SUCC)
			continue;

	#ifdef CONFIG_HUAWEI_HW_DEV_DCT
		__set_hw_dev_flag(sensor_manager[index].sensor_id);
	#endif

		extend_config_after_sensor_detect(dn,index);
	}
	check_detect_result(mode);
}

static void sensor_detect_exception_process(uint8_t result)
{
	int i = 0;
	if(result > 0)
	{
		for(i = 0; i < SENSOR_DETECT_RETRY; i++ )
		{
			hwlog_info("%s :  %d redect sensor after detect all sensor, fail sensor num  %d \n",__func__,i,s_redetect_state.detect_fail_num);
			if(s_redetect_state.detect_fail_num > 0)
				redetect_failed_sensors(DETECT_RETRY+i);
		}
	}
}

int init_sensors_cfg_data_from_dts(void)
{
	int ret = 0;
	int index = 0;
	char *sensor_ty = NULL;
	char *sensor_st = NULL;
	struct device_node *dn = NULL;
	const char *st = "disabled";
	int i = 0;
	uint8_t sensor_detect_result = 0;

	memset(&sensorlist_info, 0, SENSOR_MAX * sizeof(struct sensorlist_info));
	for (i=0; i<SENSOR_MAX; i++)  //init sensorlist_info struct array
	{
		sensorlist_info[i].version = -1;
		sensorlist_info[i].maxRange = -1;
		sensorlist_info[i].resolution = -1;
		sensorlist_info[i].power = -1;
		sensorlist_info[i].minDelay = -1;
		sensorlist_info[i].maxDelay = -1;
		sensorlist_info[i].fifoReservedEventCount = 0xFFFFFFFF;
		sensorlist_info[i].fifoMaxEventCount = 0xFFFFFFFF;
		sensorlist_info[i].flags = 0xFFFFFFFF;
	}

	for_each_node_with_property(dn, "sensor_type") {
		//sensor type
		ret = of_property_read_string(dn, "sensor_type", (const char **)&sensor_ty);
		if (ret) {
			hwlog_err("get sensor type fail ret=%d\n", ret);
			continue;
		}
		hwlog_info("%s : get sensor type %s \n",__func__,sensor_ty);
		index = get_sensor_index(sensor_ty, strlen(sensor_ty));
		if (index < 0) {
			hwlog_err("get sensor index fail ret=%d\n", ret);
			continue;
		}
		if (sensor_manager[index].sensor_id == CAP_PROX)
			read_cap_prox_info(dn);//for factory sar

		if (sensor_manager[index].sensor_id == CAP_PROX1)
			read_cap_prox1_info(dn);//for factory sar

		//sensor status:ok or disabled
		ret = of_property_read_string(dn, "status", (const char **)&sensor_st);
		if (ret) {
			hwlog_err("get sensor status fail ret=%d\n", ret);
			continue;
		}

		ret = strcmp(st,sensor_st);
		if (!ret) {
			hwlog_info("%s : sensor %s status is %s\n", __func__,
				sensor_ty, sensor_st);
			continue;
		}
		if (!extend_config_before_sensor_detect(dn, index))
			continue;

		ret = device_detect(dn,index);
		if(ret != DET_SUCC)
			continue;

	#ifdef CONFIG_HUAWEI_HW_DEV_DCT
		__set_hw_dev_flag(sensor_manager[index].sensor_id);
	#endif

		extend_config_after_sensor_detect(dn,index);
	}

	if (sensor_manager[FINGERPRINT].detect_result != DET_SUCC)
	{
		hwlog_warn("fingerprint detect fail, bypass.\n");
		sensor_manager[FINGERPRINT].detect_result = DET_SUCC;
	}
	if (sensor_manager[FINGERPRINT_UD].detect_result != DET_SUCC)
	{
		hwlog_warn("fingerprint_ud detect fail, bypass.\n");
		sensor_manager[FINGERPRINT_UD].detect_result = DET_SUCC;
	}

	sensor_detect_result = check_detect_result(BOOT_DETECT);
	sensor_detect_exception_process(sensor_detect_result);

	get_sensors_id_from_dts();
	if (get_adapt_id_and_send())
		return -EINVAL;

	return 0;
}

static void send_parameter_to_mcu(SENSOR_DETECT_LIST s_id, int cmd)
{
	int ret = 0;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	pkt_parameter_req_t cpkt;
	pkt_header_t *hd = (pkt_header_t *)&cpkt;
	char buf[50] = {0};

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

	pkg_ap.tag = sensor_manager[s_id].tag;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	cpkt.subcmd = cmd;
	pkg_ap.wr_buf=&hd[1];
	pkg_ap.wr_len=sensor_manager[s_id].cfg_data_length+SUBCMD_LEN;
	memcpy(cpkt.para, sensor_manager[s_id].spara, sensor_manager[s_id].cfg_data_length);

	hwlog_info("%s g_iom3_state = %d,tag =%d ,cmd =%d\n",__func__, g_iom3_state, sensor_manager[s_id].tag, cmd);

	if (g_iom3_state == IOM3_ST_RECOVERY || iom3_power_state == ST_SLEEP)
	{
		ret = write_customize_cmd(&pkg_ap, NULL, false);
	}else {
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	}

	if (ret)
	{
	    hwlog_err("send tag %d cfg data to mcu fail,ret=%d\n", pkg_ap.tag, ret);
	}else{
	    if (pkg_mcu.errno != 0)
	    {
	        snprintf(buf, 50, "set %s cfg fail\n", sensor_manager[s_id].sensor_name_str);
	        hwlog_err("%s\n",buf);
	    }else{
	        snprintf(buf, 50, "set %s cfg to mcu success\n", sensor_manager[s_id].sensor_name_str);
	        hwlog_info("%s\n",buf);
	        if (g_iom3_state != IOM3_ST_RECOVERY)
	        {
	        #ifdef CONFIG_HUAWEI_HW_DEV_DCT
	            __set_hw_dev_flag(s_id);
	    #endif
	        }
	    }
	}
}

void resend_als_parameters_to_mcu(void){
	send_parameter_to_mcu(ALS, SUB_CMD_SET_RESET_PARAM_REQ);
	hwlog_info("%s\n", __func__);
}

static int mag1_data_from_mcu(const pkt_header_t *head)
{
	switch(((pkt_mag_calibrate_data_req_t *)head)->subcmd) {
	case SUB_CMD_CALIBRATE_DATA_REQ:
			return write_magsensor_calibrate_data_to_nv(((pkt_mag1_calibrate_data_req_t *)head)->calibrate_data);
	default:
		hwlog_err("uncorrect subcmd 0x%x.\n", ((pkt_mag1_calibrate_data_req_t *)head)->subcmd);
	}
	return 0;
}


static int mag_data_from_mcu(const pkt_header_t *head)
{
	switch(((pkt_mag_calibrate_data_req_t *)head)->subcmd) {
	case SUB_CMD_CALIBRATE_DATA_REQ:
		if(akm_cal_algo == 1)
			return write_magsensor_calibrate_data_to_nv(((pkt_akm_mag_calibrate_data_req_t *)head)->calibrate_data);
		else
			return write_magsensor_calibrate_data_to_nv(((pkt_mag_calibrate_data_req_t *)head)->calibrate_data);
	default:
		hwlog_err("uncorrect subcmd 0x%x.\n", ((pkt_mag_calibrate_data_req_t *)head)->subcmd);
	}
	return 0;
}
static int gyro1_data_from_mcu(const pkt_header_t *head)
{
	switch(((pkt_gyro1_calibrate_data_req_t *)head)->subcmd) {
	case SUB_CMD_SELFCALI_REQ:
		return write_gyro1_sensor_offset_to_nv(((pkt_gyro1_calibrate_data_req_t *)head)->calibrate_data, head->length-SUBCMD_LEN);
	default:
		hwlog_err("uncorrect subcmd 0x%x.\n", ((pkt_gyro1_calibrate_data_req_t *)head)->subcmd);
	}
	return 0;
}

static int gyro_data_from_mcu(const pkt_header_t *head)
{
	switch(((pkt_gyro_calibrate_data_req_t *)head)->subcmd) {
	case SUB_CMD_SELFCALI_REQ:
		return write_gyro_sensor_offset_to_nv(((pkt_gyro_calibrate_data_req_t *)head)->calibrate_data, head->length-SUBCMD_LEN);
	case SUB_CMD_GYRO_TMP_OFFSET_REQ:
		return write_gyro_temperature_offset_to_nv(((pkt_gyro_temp_offset_req_t *)head)->calibrate_data, GYRO_TEMP_CALI_NV_SIZE);
	default:
		hwlog_err("uncorrect subcmd 0x%x.\n", ((pkt_gyro_calibrate_data_req_t *)head)->subcmd);
	}
	return 0;
}

static int tof_data_from_mcu(const pkt_header_t *head)
{
	switch(((pkt_tof_calibrate_data_req_t *)head)->subcmd) {
	case SUB_CMD_SELFCALI_REQ:
		return write_tof_offset_to_nv(((pkt_tof_calibrate_data_req_t *)head)->calibrate_data);
	default:
		hwlog_err("uncorrect subcmd 0x%x.\n", ((pkt_tof_calibrate_data_req_t *)head)->subcmd);
	}
	return 0;
}

/* recieve ps offset data from mcu and write to nv */
static int ps_data_from_mcu(const pkt_header_t *head)
{
	int ret = -1;
	switch (((pkt_ps_calibrate_data_req_t *)head)->subcmd) {
	case SUB_CMD_SET_WRITE_NV_ATTER_SALE:
		ret = write_ps_offset_to_nv((int *)(((pkt_ps_calibrate_data_req_t *)head)->calibrate_data));
		break;
	default:
		hwlog_err("uncorrect subcmd 0x%x\n", ((pkt_ps_calibrate_data_req_t *)head)->subcmd);
		break;
	}
	return ret;
}

/* recieve als offset data from mcu and write to nv */
static int als_data_from_mcu(const pkt_header_t *head)
{
	int ret = -1;
	switch (((pkt_als_calibrate_data_req_t *)head)->subcmd) {
	case SUB_CMD_SET_WRITE_NV_ATTER_SALE:
		ret = als_underscreen_calidata_save();
		break;
	default:
		hwlog_err("uncorrect subcmd 0x%x\n", ((pkt_als_calibrate_data_req_t *)head)->subcmd);
		break;
	}
	return ret;
}

static void register_priv_notifier(SENSOR_DETECT_LIST s_id)
{
	switch (s_id){
		case GYRO:
			register_mcu_event_notifier(TAG_GYRO, CMD_CMN_CONFIG_REQ, gyro_data_from_mcu);
			break;
		case MAG:
			register_mcu_event_notifier(TAG_MAG, CMD_CMN_CONFIG_REQ, mag_data_from_mcu);
			break;
		case TOF:
			register_mcu_event_notifier(TAG_TOF, CMD_CMN_CONFIG_REQ,tof_data_from_mcu);
			break;
		case ALS:
			register_mcu_event_notifier(TAG_ALS, CMD_CMN_CONFIG_REQ, als_data_from_mcu);
			break;
		case PS:
			register_mcu_event_notifier(TAG_PS, CMD_CMN_CONFIG_REQ, ps_data_from_mcu);
			break;
		case GYRO1:
			register_mcu_event_notifier(TAG_GYRO1, CMD_CMN_CONFIG_REQ, gyro1_data_from_mcu);
			break;
		case MAG1:
			register_mcu_event_notifier(TAG_MAG1, CMD_CMN_CONFIG_REQ, mag1_data_from_mcu);
			break;

		default:
			break;
	}
	return;
}

/*******************************************************************************************
Function:	sensor_set_cfg_data
Description: �����ò�������mcu ��
Data Accessed:  ��
Data Updated:   ��
Input:        ��
Output:         ��
Return:         �ɹ�����ʧ����Ϣ: 0->�ɹ�, -1->ʧ��
*******************************************************************************************/
int sensor_set_cfg_data(void)
{
	int ret = 0;
	SENSOR_DETECT_LIST s_id;

	for (s_id = ACC; s_id < SENSOR_MAX; s_id ++) {
		if (strlen(sensor_chip_info[s_id]) != 0) {
#ifdef CONFIG_CONTEXTHUB_SHMEM

			if (s_id != RPC) {
#endif
				send_parameter_to_mcu(s_id, SUB_CMD_SET_PARAMET_REQ);

				if (s_id == ALS) {
					send_para_flag = 1;
				}

				if (g_iom3_state != IOM3_ST_RECOVERY)
				{ register_priv_notifier(s_id); }
#ifdef CONFIG_CONTEXTHUB_SHMEM
			} else {
				ret = shmem_send(TAG_RPC, &rpc_data, sizeof(rpc_data));

				if (ret)
				{hwlog_err("%s shmem_send error \n", __func__); }
			}

#endif
		}

	}


    return ret;
}

static bool need_download_fw(uint8_t tag)
{
	return ((tag == TAG_KEY) || (tag == TAG_TOF) || (tag == TAG_CAP_PROX));
}

int sensor_set_fw_load(void)
{
	int val = 1;
	int ret = 0;
	write_info_t pkg_ap;
	pkt_parameter_req_t cpkt;
	pkt_header_t* hd = (pkt_header_t*)&cpkt;
	SENSOR_DETECT_LIST s_id;
	hwlog_info("write  fw dload.\n");
	for (s_id = ACC; s_id < SENSOR_MAX; s_id ++) {
		if (strlen(sensor_chip_info[s_id]) != 0) {
			if (need_download_fw(sensor_manager[s_id].tag )) {
				pkg_ap.tag = sensor_manager[s_id].tag;
				pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
				cpkt.subcmd = SUB_CMD_FW_DLOAD_REQ;
				pkg_ap.wr_buf = &hd[1];
				pkg_ap.wr_len = sizeof(val) + SUBCMD_LEN;
				memcpy(cpkt.para, &val, sizeof(val));
				ret = write_customize_cmd(&pkg_ap, NULL, false);
				hwlog_info("write %d fw dload.\n",sensor_manager[s_id].tag);
			}
		}
	}
	return 0;
}
static void redetect_sensor_work_handler(struct work_struct *wk)
{
	wake_lock(&sensor_rd);
	redetect_failed_sensors(REDETECT_LATER);

	if(s_redetect_state.need_recovery == 1){
		s_redetect_state.need_recovery = 0;
		hwlog_info("%s: some sensor detect success after %d redetect, begin recovery \n",__func__,s_redetect_state.redetect_num);
		iom3_need_recovery(SENSORHUB_USER_MODID, SH_FAULT_REDETECT);
	}else{
		hwlog_info("%s: no sensor redetect success\n",__func__);
	}
	wake_unlock(&sensor_rd);
}

void sensor_redetect_enter(void)
{
	if(g_iom3_state == IOM3_ST_NORMAL)
	{
		if(s_redetect_state.need_redetect_sensor == 1){
			if(s_redetect_state.redetect_num < MAX_REDETECT_NUM){
				queue_work(system_power_efficient_wq, &redetect_work);
				s_redetect_state.redetect_num ++;
			}else{
				hwlog_info("%s: some sensors detect fail  ,but the max redetect num is over flow\n",__func__);
				show_last_detect_fail_sensor();
			}
		}
	}
}

void sensor_redetect_init(void)
{
	memset(&s_redetect_state,0,sizeof(s_redetect_state));
	memcpy(&gsensor1_data, &gsensor_data, sizeof(gsensor1_data));
	memcpy(&gsensor2_data, &gsensor_data, sizeof(gsensor2_data));
	memcpy(&gyro1_data, &gyro_data, sizeof(gyro1_data));
	memcpy(&gyro2_data, &gyro_data, sizeof(gyro2_data));
	memcpy(&mag1_data, &mag_data, sizeof(mag1_data));
	memcpy(&sar1_pdata, &sar_pdata, sizeof(sar1_pdata));
	wake_lock_init(&sensor_rd, WAKE_LOCK_SUSPEND, "sensorhub_redetect");
	INIT_WORK(&redetect_work, redetect_sensor_work_handler);
}

int sensor_get_als_bh1749_flag(void)
{
	return g_bh1749_flag;
}
