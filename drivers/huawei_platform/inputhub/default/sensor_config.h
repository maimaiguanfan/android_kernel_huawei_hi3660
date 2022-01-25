/*
 *  drivers/misc/inputhub/inputhub_route.h
 *  Sensor Hub Channel driver
 *
 *  Copyright (C) 2012 Huawei, Inc.
 *  Author: qindiwen <inputhub@huawei.com>
 *
 */
#ifndef	__SENSORS_H__
#define	__SENSORS_H__

#include "sensor_detect.h"

extern int stk3338_als_flag;
extern int ltr2568_als_flag;
extern int vishay_vcnl36832_als_flag;

#define EXTEND_DATA_TYPE_IN_DTS_BYTE        0
#define EXTEND_DATA_TYPE_IN_DTS_HALF_WORD   1
#define EXTEND_DATA_TYPE_IN_DTS_WORD        2

#define HALL_COVERD     (1)

#define SENSOR_VOLTAGE_3_2V 3200000
#define SENSOR_VOLTAGE_3V 3000000
#define SENSOR_VOLTAGE_1V8 1800000

#define NV_READ_TAG	1
#define NV_WRITE_TAG	0
#define PS_CALIDATA_NV_NUM	334
#define PS_CALIDATA_NV_SIZE  24
#define TOF_CALIDATA_NV_SIZE  47
#define ALS_CALIDATA_NV_NUM	339
#define ALS_CALIDATA_NV_SIZE  12
#define ALS_CALIDATA_NV_SIZE_WITH_DARK_NOISE_OFFSET  14
#define GYRO_CALIDATA_NV_NUM	341
#define GYRO_TEMP_CALI_NV_NUM	377
#define GYRO_CALIDATA_NV_SIZE  72
#define GYRO_TEMP_CALI_NV_SIZE  56
#define HANDPRESS_CALIDATA_NV_NUM  354
#define HANDPRESS_CALIDATA_NV_SIZE  24
#define AIRPRESS_CALIDATA_NV_NUM	332
#define AIRPRESS_CALIDATA_NV_SIZE  4
#define AIRPRESS_CALIDATA_NV_SIZE_WITH_AIRTOUCH  100
#define CAP_PROX_CALIDATA_NV_NUM  310
#define CAP_PROX_CALIDATA_NV_SIZE  28
#define ALS_TP_CALIDATA_NV1_NUM  403
#define ALS_TP_CALIDATA_NV1_SIZE  104
#define ALS_TP_CALIDATA_NV2_NUM  404
#define ALS_TP_CALIDATA_NV2_SIZE  104
#define ALS_TP_CALIDATA_NV3_NUM  405
#define ALS_TP_CALIDATA_NV3_SIZE  104
#define pinhole_para_size (10)
#define TMD2745_PARA_SIZE (10)
#define RPR531_PARA_SIZE (16)
#define APDS9999_PARA_SIZE (24)
#define TMD3702_PARA_SIZE (29)
#define TCS3701_PARA_SIZE (32)
#define VCNL36658_PARA_SIZE (31)
#define TSL2591_PARA_SIZE (15)
#define BH1726_PARA_SIZE (16)
#define VD6281_PARA_SIZE (8)
#define MAX_PARA_SIZE (33)
#define BH1749_PARA_SIZE (27)
#define ACC_OFFSET_NV_NUM 307
#define ACC_OFFSET_NV_SIZE (60)
#define MAG_CALIBRATE_DATA_NV_NUM 233
#define MAG_CALIBRATE_DATA_NV_SIZE (MAX_MAG_CALIBRATE_DATA_LENGTH)
#define MAG_AKM_CALIBRATE_DATA_NV_SIZE (MAX_MAG_AKM_CALIBRATE_DATA_LENGTH)
#define VIB_CALIDATA_NV_NUM 337
#define VIB_CALIDATA_NV_SIZE 3
#define VIB_CALIDATA_NV_NAME "VIBCAL"
#define SAR_SEMTECH_USE_PH_NUM (2)
#define SAR_ABOV_USE_PH_NUM (2)
#define ALS_UNDER_TP_CALDATA_LEN 59

#define ACC1_OFFSET_NV_NUM	410
#define ACC1_OFFSET_NV_SIZE	(60)
#define ACC1_NV_NAME  "GSENSOR1"

#define GYRO1_OFFSET_NV_NUM	411
#define GYRO1_OFFSET_NV_SIZE	(72)
#define GYRO1_NV_NAME  "GYRO1"

#define MAG1_OFFSET_NV_NUM	412
#define MAG1_OFFSET_NV_SIZE	(12)
#define MAG1_NV_NAME  "MSENSOR1"

#define CAP_PROX1_CALIDATA_NV_NUM	413
#define CAP_PROX1_CALIDATA_NV_SIZE	(28)
#define CAP_PROX1_NV_NAME  "CSENSOR1"

enum ALS_SENSNAME{
	APDS9922 = 1,
	LTR578 = 2,
};

typedef enum {
	RET_INIT = 0,
	SUC = 1,
	EXEC_FAIL,
	NV_FAIL,
	COMMU_FAIL,
	POSITION_FAIL,
	RET_TYPE_MAX
} RET_TYPE;

enum detect_state {
	DET_INIT = 0,
	DET_FAIL,
	DET_SUCC
};

struct sar_cap_proc_calibrate_data {
	uint16_t swap_flag[3];
	uint16_t cal_fact_base[3];
	uint16_t cal_offset[3];
	uint16_t digi_offset[3];
	uint16_t cap_prox_extend_data[2];
};

struct sar_cypress_calibrate_data {
	uint16_t sar_idac;
	uint16_t raw_data;
	uint16_t near_signaldata;
	uint16_t far_signaldata;
};

struct sar_semtech_calibrate_data {
	uint16_t offset[SAR_SEMTECH_USE_PH_NUM];
	uint16_t diff[SAR_SEMTECH_USE_PH_NUM];
};

struct sar_semtech_9335_calibrate_data {
	uint16_t offset[SAR_SEMTECH_USE_PH_NUM];
	uint16_t diff[SAR_SEMTECH_USE_PH_NUM];
};

struct sar_abov_calibrate_data {
	uint16_t offset[SAR_ABOV_USE_PH_NUM];
	uint16_t diff[SAR_ABOV_USE_PH_NUM];
};

union sar_calibrate_data {
	struct sar_cap_proc_calibrate_data cap_cali_data;
	struct sar_cypress_calibrate_data cypres_cali_data;
	struct sar_semtech_calibrate_data semtech_cali_data;
	struct sar_semtech_9335_calibrate_data semtech_9335_cali_data;
	struct sar_abov_calibrate_data abov_cali_data;
};

struct press_alg_result {
	int slope;                  //calibrated para
	int base_press;             //based airpress value
	short max_press;            //max airpress value in down event
	short raise_press;          //airpress value without touch
	short min_press;            //min value of airpress
	short temp;                 //temperature
	short speed;                //time of down
	char result_flag;		    //calibrated result
};
struct airpress_touch_calibrate_data {
	struct press_alg_result cResult;  //calibrated result
	struct press_alg_result tResult;  //tested result
};

struct als_under_tp_calidata
{
	uint16_t x;  //left_up x-aix
	uint16_t y;  //left_up y-aix
	uint16_t width;
	uint16_t length;
	unsigned int a[25];   //area para.
	int b[30];    //algrothm para,
};

typedef struct _BH1745_ALS_PARA_TABLE {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 bh745_para[25];	/*give to bh1745 rgb sensor use,output lux and cct will use these para*/
} BH1745_ALS_PARA_TABLE;

typedef struct _APDS9251_ALS_PARA_TABLE {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 apds251_para[21];	/*give to apds251 rgb sensor use,output lux and cct will use these par*/
} APDS9251_ALS_PARA_TABLE;	/*the apds251_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE*/

typedef struct _TMD3725_ALS_PARA_TABLE {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 tmd3725_para[33];/*give to tmd3725 rgb sensor use,output lux and cct will use these par*/
} TMD3725_ALS_PARA_TABLE;/*the tmd3725_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE*/


typedef struct _LTR582_ALS_PARA_TABLE {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 ltr582_para[26];/*give to ltr582 rgb sensor use,output lux and cct will use these par*/
} LTR582_ALS_PARA_TABLE;/*the ltr582_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE*/


typedef struct _PINHOLE_ALS_PARA_TABLE {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t sens_name;
	uint8_t tp_manufacture;
	s16 pinhole_para[pinhole_para_size];/*modify the size of the array to pass more data */
} PINHOLE_ALS_PARA_TABLE;	/*the ph_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE*/

typedef struct _TMD2745_ALS_PARA_TABLE {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 als_para[TMD2745_PARA_SIZE];/*modify the size of the array to pass more data */
} TMD2745_ALS_PARA_TABLE;           /*keep als_para size smaller than SENSOR_PLATFORM_EXTEND_DATA_SIZE*/

typedef struct _RPR531_ALS_PARA_TABLE{
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	s16 rpr531_para[RPR531_PARA_SIZE];
}RPR531_ALS_PARA_TABLE;

typedef struct _APDS9999_ALS_PARA_TABLE {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 apds9999_para[APDS9999_PARA_SIZE];	/*give to apds9999 rgb sensor use,output lux and cct will use these par*/
} APDS9999_ALS_PARA_TABLE;	/*the apds9999_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE*/

typedef struct _TMD3702_ALS_PARA_TABLE {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 tmd3702_para[TMD3702_PARA_SIZE];/*give to tmd3702 rgb sensor use,output lux and cct will use these par*/
} TMD3702_ALS_PARA_TABLE;/*the tmd3702_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE*/

typedef struct _TCS3701_ALS_PARA_TABLE {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 tcs3701_para[TCS3701_PARA_SIZE];/*give to tcs3701 rgb sensor use,output lux and cct will use these par*/
} TCS3701_ALS_PARA_TABLE;/*the tcs3701_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE*/

typedef struct _VCNL36658_ALS_PARA_TABLE {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 vcnl36658_para[31];/*give to vcnl36658 rgb sensor use,output lux and cct will use these par*/
} VCNL36658_ALS_PARA_TABLE;/*the vcnl36658_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE*/

typedef struct _TSL2591_ALS_PARA_TABLE {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 tsl2591_para[TSL2591_PARA_SIZE];
} TSL2591_ALS_PARA_TABLE;

typedef struct _BH1726_ALS_PARA_TABLE {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 bh1726_para[BH1726_PARA_SIZE];
} BH1726_ALS_PARA_TABLE;

typedef struct _bh1749_als_para_table{
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 bh1749_para[BH1749_PARA_SIZE];
} bh1749_als_para_table_t;

typedef struct {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t len;
	s16 als_para[MAX_PARA_SIZE];
} als_para_normal_table;

typedef struct _VD6281_ALS_PARA_TABLE {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 vd6281_para[VD6281_PARA_SIZE];
} VD6281_ALS_PARA_TABLE;
extern int fill_extend_data_in_dts(struct device_node *dn, const char *name, unsigned char *dest, size_t max_size, int flag);
extern int mcu_i3c_rw(uint8_t bus_num, uint8_t i2c_add, uint8_t *tx, uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len);
extern int mcu_i2c_rw(uint8_t bus_num, uint8_t i2c_add, uint8_t *tx, uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len);
extern int mcu_spi_rw(uint8_t bus_num, union SPI_CTRL ctrl, uint8_t *tx, uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len);
extern int combo_bus_trans(struct sensor_combo_cfg *p_cfg, uint8_t *tx, uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len);
extern void __dmd_log_report(int dmd_mark, const char *err_func, const char *err_msg);
extern int write_gyro_sensor_offset_to_nv(char *temp, int length);
extern int write_magsensor_calibrate_data_to_nv(char *src);

extern void reset_calibrate_data(void);
extern void reset_add_data(void);
extern int send_gsensor_calibrate_data_to_mcu(void);
extern int send_airpress_calibrate_data_to_mcu(void);
extern int send_gyro_calibrate_data_to_mcu(void);
extern int send_handpress_calibrate_data_to_mcu(void);
extern int mag_current_notify(void);
extern void read_tp_color_cmdline(void);
extern int write_calibrate_data_to_nv(int nv_number, int nv_size, char *nv_name, char *temp);
extern int write_gsensor_offset_to_nv(char *temp, int length);
extern int write_gyro_temperature_offset_to_nv(char *temp, int length);
extern int send_gsensor1_calibrate_data_to_mcu(void);
extern int send_gyro1_calibrate_data_to_mcu(void);
extern int write_gsensor1_offset_to_nv(char *temp, int length);
extern int write_gyro1_sensor_offset_to_nv(char *temp, int length);
int send_als_under_tp_calibrate_data_to_mcu(void);
int write_ps_offset_to_nv(int *temp);
int mcu_save_calidata_to_nv(int tag, int *para);
int open_send_current(int (*send) (int));
#endif /* __SENSORS_H__ */
