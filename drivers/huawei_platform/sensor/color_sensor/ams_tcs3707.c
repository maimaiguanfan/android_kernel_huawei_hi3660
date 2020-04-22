/*
*****************************************************************************
* Copyright by ams AG                                                       *
* All rights are reserved.                                                  *
*                                                                           *
* IMPORTANT - PLEASE READ CAREFULLY BEFORE COPYING, INSTALLING OR USING     *
* THE SOFTWARE.                                                             *
*                                                                           *
* THIS SOFTWARE IS PROVIDED FOR USE ONLY IN CONJUNCTION WITH AMS PRODUCTS.  *
* USE OF THE SOFTWARE IN CONJUNCTION WITH NON-AMS-PRODUCTS IS EXPLICITLY    *
* EXCLUDED.                                                                 *
*                                                                           *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS         *
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  *
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT          *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE     *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.      *
*****************************************************************************
*/

/*
 * Input Driver Module
 */

/*
 * @@AMS_REVISION_Id:
 */

#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/unistd.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include "ams_tcs3707.h"
#include <linux/ioctl.h>
#include <linux/miscdevice.h>


static UINT32 fd_ratio = 1;
static bool fd_gain_adjusted = false;

#define REPORT_FIFO_LEN 128
#define FIFO_LEVEL_16 16
#define FIFO_LEVEL_32 32
#define FIFO_LEVEL_48 48
#define FIFO_LEVEL_64 64
#define FIFO_LEVEL_80 80
#define FIFO_LEVEL_96 96
#define FIFO_LEVEL_112 112

#define BUF_16_MAX_LEVEL5    2
#define BUF_16_MAX_LEVEL4    9
#define BUF_16_MAX_LEVEL3    36
#define BUF_16_MAX_LEVEL2    288

#define BUF_RATIO_8_16BIT 2
#define ONE_BYTE  1
#define HWLOG_TAG color_sensor
static bool fd_timer_stopped = false;

#define MS_2_US         1000
#define DOUBLE_BYTE     2
#define MAX_SATURATION  65535
#define FIRST_RGB_TIMER 4
#define POLL_RGB_TIMER 100
#define CALI_RGB_TIMER 120
#define FIRST_FLK_TIMER   6
#define MAX_BUFFER_SIZE   256
static u16 ring_buffer[MAX_BUFFER_SIZE] = {0};
static u32 read_out_buffer[MAX_BUFFER_SIZE] = {0};
static u16 *buffer_ptr = &ring_buffer[0];
static u16 *head_ptr = &ring_buffer[0];
static u16 *tail_ptr = &ring_buffer[0];

static u8 first_circle_end = 0;
#define BUFSIZ 512

static DEFINE_MUTEX(ring_buffer_mutex);
static DEFINE_MUTEX(enable_handle_mutex);

HWLOG_REGIST();

static int report_value[AMS_REPORT_DATA_LEN] = {0};
extern int color_report_val[MAX_REPORT_LEN];

static struct colorDriver_chip *p_chip = NULL;
static bool color_calibrate_result = true;
static bool report_calibrate_result = false;
static at_color_sensor_cali_para_nv color_nv_para;
static int read_nv_first_in = 0;
static int enable_status_before_calibrate = 0;
#ifdef CONFIG_HUAWEI_DSM
static bool color_devcheck_dmd_result = true;
extern struct dsm_client* shb_dclient;
#endif

struct delayed_work ams_tcs3707_dmd_work;
static UINT8 report_logcount = 0;

struct file *file_fd = NULL;

#if defined(CONFIG_AMS_OPTICAL_SENSOR_ALS)

typedef struct{
	UINT32 rawC;
	UINT32 rawR;
	UINT32 rawG;
	UINT32 rawB;
	UINT32 rawW;
}export_alsData_t;

typedef struct color_sensor_input_tcs3707 {
	UINT32 enable;
	UINT32 tar_c;
	UINT32 tar_r;
	UINT32 tar_g;
	UINT32 tar_b;
	UINT32 tar_w;
	UINT32 reserverd[3];
}color_sensor_input_para_tcs3707;

static UINT8 const tcs3707_ids[] = {
	0x18,
};

tcs3707_reg_setting default_setting[] = {
	{TCS3707_ATIME_REG,   0x3b},//default atime is 29, from 1d to 3b, max value = 36000
	{TCS3707_CFG0_REG,    0x00},
	{TCS3707_CFG1_REG,    AGAIN_4X},//default again is 4X
	{TCS3707_CFG3_REG,    0x0C},
	{TCS3707_CFG4_REG,    0x80},//use fd_gain as the adata5's gain value,, not again
	{TCS3707_CFG8_REG,    0x08},//bit3 is set to disable flicker AGC, bit2 is 0 means disable ALS AGC
	{TCS3707_CFG10_REG,   0xf2},
	{TCS3707_CFG11_REG,   0x40},
	{TCS3707_CFG12_REG,   0x00},
	{TCS3707_CFG14_REG,   0x00},
	{TCS3707_PERS_REG,    0x00},
	{TCS3707_GPIO_REG,    0x02},
	{TCS3707_ASTEPL_REG,  0xe7},//default astep is 2.78ms
	{TCS3707_ASTEPH_REG,  0x03},
	{TCS3707_AGC_GAIN_MAX_REG,0x99},
	{TCS3707_AZ_CONFIG_REG, 0x00},//close auto zero
	{TCS3707_FD_CFG0, 0x80},//enable fd_fifo_mode
	{TCS3707_FD_CFG1, 0x67},//default fd_time = 0.5ms so the sampel rate is 1000hz //from 0xb3 to 0x67
	{TCS3707_FD_CFG3, 0x31},// default fd_gain = 32x//from 0x30 to 0x31
	{TCS3707_FIFO_MAP,0x00}
};


static UINT32 const ams_tcs3707_alsGain_conversion[] = {
	GAIN_CALI_LEVEL_1 * AMS_TCS3707_GAIN_SCALE,
	GAIN_CALI_LEVEL_2 * AMS_TCS3707_GAIN_SCALE,
	GAIN_CALI_LEVEL_3 * AMS_TCS3707_GAIN_SCALE,
	GAIN_CALI_LEVEL_4 * AMS_TCS3707_GAIN_SCALE,
	GAIN_CALI_LEVEL_5 * AMS_TCS3707_GAIN_SCALE
};

static UINT32 const ams_tcs3707_als_gains[] = {
	GAIN_LEVEL_1,
	GAIN_LEVEL_2,
	GAIN_LEVEL_3,
	GAIN_LEVEL_4,
	GAIN_LEVEL_5
};

static UINT16 const ams_tcs3707_als_gain_values[] = {
	0,//actual gain is 0.5
	1,
	2,
	4,
	8,
	16,
	32,
	64,
	128,
	256,
	512
};

const ams_tcs3707_gainCaliThreshold_t  ams_tcs3707_setGainThreshold[CAL_STATE_GAIN_LAST] = {
	{0, MAX_AMS_CALI_THRESHOLD},//set threshold 1x to 0~100
	{0, MAX_AMS_CALI_THRESHOLD},//set threshold 1x to 0~100
	{(AMS_TCS3707_FLOAT_TO_FIX/AMS_TCS3707_CAL_THR), (AMS_TCS3707_FLOAT_TO_FIX*AMS_TCS3707_CAL_THR)},
	{(AMS_TCS3707_FLOAT_TO_FIX/AMS_TCS3707_CAL_THR), (AMS_TCS3707_FLOAT_TO_FIX*AMS_TCS3707_CAL_THR)},
	{(AMS_TCS3707_FLOAT_TO_FIX/AMS_TCS3707_CAL_THR), (AMS_TCS3707_FLOAT_TO_FIX*AMS_TCS3707_CAL_THR)},	
};
 

static bool ams_tcs3707_getDeviceInfo(ams_tcs3707_deviceInfo_t * info);
static bool ams_tcs3707_deviceEventHandler(ams_tcs3707_deviceCtx_t * ctx, bool inCalMode);
static ams_tcs3707_deviceIdentifier_e ams_tcs3707_testForDevice(AMS_PORT_portHndl * portHndl);
extern int ap_color_report(int value[], int length);
extern int color_register(struct colorDriver_chip *chip);
extern int read_color_calibrate_data_from_nv(int nv_number, int nv_size, char * nv_name, char * temp);
extern int write_color_calibrate_data_to_nv(int nv_number, int nv_size, char * nv_name, char * temp);
extern int (*color_default_enable)(bool enable);
extern UINT32 g_flicker_support;

int AMS_PORT_TCS3707_getByte(AMS_PORT_portHndl * handle, uint8_t reg, uint8_t * data, uint8_t len){
	int ret = 0;
	if ((handle == NULL) || (data == NULL )){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return -EPERM;
	}
	ret = i2c_smbus_read_i2c_block_data(handle, reg, len, data);

	if (ret < 0)
		hwlog_err("%s: failed at address %x (%d bytes)\n",__func__, reg, len);

	return ret;
}

int AMS_PORT_TCS3707_setByte(AMS_PORT_portHndl * handle, uint8_t reg, uint8_t* data, uint8_t len){
	int ret = 0;
	if ((handle == NULL) || (data == NULL)){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return -EPERM;
	}
	ret = i2c_smbus_write_i2c_block_data(handle, reg, len, data);

	if (ret < 0)
			hwlog_err("%s: failed at address %x (%d bytes)\n", __func__, reg, len);

	return ret;
}

static int ams_tcs3707_getByte(AMS_PORT_portHndl * portHndl, UINT8 reg, UINT8 * readData)
{
	int read_count = 0;

	if ((portHndl == NULL) || (readData == NULL)){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return read_count;
	}
	read_count = AMS_PORT_TCS3707_getByte(portHndl,reg, readData,ONE_BYTE);

	return read_count;
}

static int ams_tcs3707_setByte(AMS_PORT_portHndl * portHndl, UINT8 reg, UINT8 data)
{
	int write_count = 0;

	if (portHndl == NULL ){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return write_count;
	}

	write_count = AMS_PORT_TCS3707_setByte(portHndl,reg,&data,ONE_BYTE);
	return write_count;
}

static int ams_tcs3707_getBuf(AMS_PORT_portHndl * portHndl, UINT8 reg, UINT8 * readData, UINT8 length)
{
	int read_count = 0;

	if ((portHndl == NULL) || (readData == NULL)){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return read_count;
	}

	read_count = AMS_PORT_TCS3707_getByte(portHndl,reg,readData,length);
	return read_count;
}

static int ams_tcs3707_rgb_report_type(void){
    return AWB_SENSOR_RAW_SEQ_TYPE_C_R_G_B_W;
}

static int ams_tcs3707_report_data(int value[])
{
	hwlog_debug("ams_tcs3707_report_data\n");
	return ap_color_report(value, AMS_REPORT_DATA_LEN*sizeof(int));
}

static int ams_tcs3707_getField(AMS_PORT_portHndl * portHndl, UINT8 reg, UINT8 * readData, UINT8 mask)
{
	int read_count = 0;

	if ((portHndl == NULL) || (readData == NULL)){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return read_count;
	}

	read_count = AMS_PORT_TCS3707_getByte(portHndl,reg, readData,ONE_BYTE);
	*readData &= mask;
	return read_count;
}

static int ams_tcs3707_setField(AMS_PORT_portHndl * portHndl, UINT8 reg, UINT8 data, UINT8 mask)
{
	int write_count = 0;
	UINT8 original_data = 0, new_data = 0;

	if (portHndl == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return write_count;
	}

	ams_tcs3707_getByte(portHndl, reg, &original_data);

	new_data = original_data & (~mask);
	new_data |= (data & mask);

	if (new_data != original_data){
		write_count = ams_tcs3707_setByte(portHndl,reg,new_data);
	}

	return write_count;
}

static int ams_tcs3707_set_enable(AMS_PORT_portHndl *portHndl,
			UINT8 reg,
			UINT8 data,
			UINT8 mask)
{
	int write_count = 0;
	UINT8 original_data = 0;
	UINT8 new_data;

	mutex_lock(&enable_handle_mutex);
	if (portHndl == NULL) {
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		mutex_unlock(&enable_handle_mutex);
		return write_count;
	}
	ams_tcs3707_getByte(portHndl, TCS3707_ENABLE_REG, &original_data);
	new_data = original_data & (~mask);
	new_data |= (data & mask);
	if (new_data != original_data)
		write_count = ams_tcs3707_setByte(portHndl,
						TCS3707_ENABLE_REG, new_data);

	mutex_unlock(&enable_handle_mutex);
	return write_count;
}

static ams_tcs3707_deviceIdentifier_e ams_tcs3707_testForDevice(AMS_PORT_portHndl * portHndl)
{
	UINT8 chipId = 0,  i = 0;
	if (portHndl == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return AMS_UNKNOWN_DEVICE;
	}
	ams_tcs3707_getByte(portHndl, TCS3707_ID_REG, &chipId);

	hwlog_info("ams_tcs3707_testForDevice: chipId = 0x%02x \n", chipId);
	for(i = 0; i < ARRAY_SIZE(tcs3707_ids);i++){
		if(chipId == tcs3707_ids[i]){
			return AMS_TCS3707_REV0;
		}
	}

	return AMS_UNKNOWN_DEVICE;
}

static void ams_tcs3707_resetAllRegisters(AMS_PORT_portHndl * portHndl)
{
	UINT8 i = 0;
	if (portHndl == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}

	for(i=0; i<ARRAY_SIZE(default_setting);i++){
		ams_tcs3707_setByte(portHndl,default_setting[i].reg, default_setting[i].value);
	}	
	hwlog_info("ASTEP_LOW_BYTE = %d, ASTEP_HIGH_BYTE = %d, ASTEP_US(DEFAULT_ASTEP) = %d\n",
		ASTEP_LOW_BYTE, ASTEP_HIGH_BYTE, ASTEP_US(DEFAULT_ASTEP));
	hwlog_warn("_3707_resetAllRegisters\n");
}

static UINT8 ams_tcs3707_gainToReg(UINT32 x)
{
	UINT8 i = 0;

	for (i = sizeof(ams_tcs3707_alsGain_conversion)/sizeof(UINT32)-1; i != 0; i--) {
			if (x >= ams_tcs3707_alsGain_conversion[i]) break;
	}
	return (i);
}


static INT32 ams_tcs3707_getGain(void *ctx)
{
	UINT8 cfg1_reg_data = 0;
	INT32 gain = 0;

	if (ctx == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return true;
	}

	ams_tcs3707_getField(((ams_tcs3707_deviceCtx_t *)ctx)->portHndl, TCS3707_CFG1_REG, &cfg1_reg_data, AGAIN_MASK);

	if(cfg1_reg_data < sizeof(ams_tcs3707_als_gain_values)/sizeof(ams_tcs3707_als_gain_values[0])){
		gain = ams_tcs3707_als_gain_values[cfg1_reg_data];
	}
	hwlog_info("now the gain val = %d\n", gain);
	return gain;
}

static INT32 ams_tcs3707_setGain(void *ctx, int gain)
{
	INT32 ret = 0;
	UINT8 cfg1 = 0;

	if (ctx == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return true;
	}	

	switch (gain)
	{
	case ALS_GAIN_VALUE_1:
		cfg1 = AGAIN_0_5X;
		break;
	case ALS_GAIN_VALUE_2*AMS_TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_1X;
		break;
	case ALS_GAIN_VALUE_3*AMS_TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_2X;
		break;
	case ALS_GAIN_VALUE_4*AMS_TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_4X;
		break;
	case ALS_GAIN_VALUE_5*AMS_TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_8X;
		break;
	case ALS_GAIN_VALUE_6*AMS_TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_16X;
		break;
	case ALS_GAIN_VALUE_7*AMS_TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_32X;
		break;
	case ALS_GAIN_VALUE_8*AMS_TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_64X;
		break;
	case ALS_GAIN_VALUE_9*AMS_TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_128X;
		break;
	case ALS_GAIN_VALUE_10*AMS_TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_256X;
		break;
	case ALS_GAIN_VALUE_11*AMS_TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_512X;
		break;
	default:
		break;
	}

	ret = ams_tcs3707_setField(((ams_tcs3707_deviceCtx_t *)ctx)->portHndl, TCS3707_CFG1_REG, cfg1 , AGAIN_MASK);
	((ams_tcs3707_deviceCtx_t *)ctx)->algCtx.als_data.gain =  (uint32_t)gain;

	hwlog_warn("ams_tcs3707_setGain: gain = %d, cfg1 = %d\n", gain,cfg1); 

	return (ret);
}

static INT32 ams_tcs3707_setItime(ams_tcs3707_deviceCtx_t * ctx, int itime_ms)
{
	INT32 ret = 0;
	int itime_us = 0;
	int atime = 0;

	if (ctx == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return true;
	}

	itime_us = itime_ms *MS_2_US;
	/**
	 * defaut integration step is DEFAULT_ASTEP as set in
	 * the ams_tcs3707_resetAllRegisters() function, set atime algo
	 */
	atime = ((itime_us + DEFAULT_ASTEP / 2) / DEFAULT_ASTEP) - 1;
	if(atime < 0){
		atime = 0;
	}
	ret = ams_tcs3707_setByte(ctx->portHndl, TCS3707_ATIME_REG, atime);

	return ret;
}

#ifdef CONFIG_AMS_FLICKER_DETECT
static INT32 ams_tcs3707_set_fd_gain(ams_tcs3707_deviceCtx_t * ctx, uint32_t gain)
{
	INT32 ret = 0;
	UINT8 cfg1 = 0;

	if (ctx == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return true;
	}

	switch (gain)
	{
	case ALS_GAIN_VALUE_1:
		cfg1 = FD_GAIN_0_5X;
		break;
	case ALS_GAIN_VALUE_2:
		cfg1 = FD_GAIN_1X;
		break;
	case ALS_GAIN_VALUE_3:
		cfg1 = FD_GAIN_2X;
		break;
	case ALS_GAIN_VALUE_4:
		cfg1 = FD_GAIN_4X;
		break;
	case ALS_GAIN_VALUE_5:
		cfg1 = FD_GAIN_8X;
		break;
	case ALS_GAIN_VALUE_6:
		cfg1 = FD_GAIN_16X;
		break;
	case ALS_GAIN_VALUE_7:
		cfg1 = FD_GAIN_32X;
		break;
	case ALS_GAIN_VALUE_8:
		cfg1 = FD_GAIN_64X;
		break;
	case ALS_GAIN_VALUE_9:
		cfg1 = FD_GAIN_128X;
		break;
	case ALS_GAIN_VALUE_10:
		cfg1 = FD_GAIN_256X;
		break;
	case ALS_GAIN_VALUE_11:
		cfg1 = FD_GAIN_512X;
		break;
	default:
		break;
	}
	//close FDEN
	ams_tcs3707_set_enable(ctx->portHndl, TCS3707_ENABLE_REG, 0, FDEN);
	ams_tcs3707_setField(ctx->portHndl, TCS3707_CONTROL_REG, CLEAR_FIFO, CLEAR_FIFO);
	//open FDEN
	ret = ams_tcs3707_setField(ctx->portHndl,
		TCS3707_FD_CFG3,
		cfg1,
		FD_GAIN_MASK);
	ams_tcs3707_set_enable(ctx->portHndl, TCS3707_ENABLE_REG, FDEN, FDEN);

	ctx->fd_data.fd_gain = gain;

	hwlog_info("%s: fd_gain = %d, cfg1>>3 = 0x%02x\n",
		__func__, gain, cfg1 >> 3);
	return ret;

}

#endif
static bool ams_tcs3707_alsRegUpdate(ams_tcs3707_deviceCtx_t * ctx, bool inCalMode)
{

	if (ctx == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return true;
	}
	/* For first integration after AEN */
	ctx->first_inte = false;
	if (!inCalMode) {
		ctx->algCtx.als_data.gain = AMS_TCS3707_AGAIN_FOR_FIRST_DATA;
		ctx->algCtx.als_data.itime_ms = AMS_TCS3707_ITIME_FOR_FIRST_DATA;
		ctx->first_inte = true;
	}
	hwlog_info("%s, inCalMode = %d, update to gain = %d, Itime = %d, first_inte =%d\n",
			__func__, inCalMode, ctx->algCtx.als_data.gain,
			ctx->algCtx.als_data.itime_ms, ctx->first_inte);
	ams_tcs3707_setItime(ctx, ctx->algCtx.als_data.itime_ms);
	ams_tcs3707_setGain(ctx, ctx->algCtx.als_data.gain);

	return false;
}

static void ams_tcs3707_rgb_power_onoff(ams_tcs3707_deviceCtx_t *ctx,
			UINT32 data,
			bool inCalMode)
{
	if (ctx == NULL) {
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}

	if (data == 0) {
		ams_tcs3707_set_enable(ctx->portHndl,
			TCS3707_ENABLE_REG, 0, AEN);
		ams_tcs3707_setByte(ctx->portHndl,
			TCS3707_STATUS_REG,
			(ASAT_MASK | AINT_MASK));
	} else {
		ams_tcs3707_alsRegUpdate(ctx,
			inCalMode);
		ctx->updateAvailable = 0;
		ams_tcs3707_setByte(ctx->portHndl,
			TCS3707_STATUS_REG,
			(ASAT_MASK | AINT_MASK));
		ams_tcs3707_set_enable(ctx->portHndl,
			TCS3707_ENABLE_REG, AEN, AEN);
	}
}

static bool ams_tcs3707_saturation_check(ams_tcs3707_deviceCtx_t * ctx, ams_tcs3707_adc_data_t *current_raw)
{
	UINT32 saturation = 0;
	UINT8 atime = 0;
	UINT8 astep[DOUBLE_BYTE] = {0};
	UINT16 astep_inte = 0;

	if((ctx == NULL) || (current_raw==NULL)){
		return false;
	}
	ams_tcs3707_getByte(ctx->portHndl, TCS3707_ATIME_REG, &atime);
	ams_tcs3707_getBuf(ctx->portHndl, TCS3707_ASTEPL_REG, astep, DOUBLE_BYTE);
	astep_inte = (astep[1] << 8) | astep[0];	//get real astep_inte

	saturation = (atime +1)*(astep_inte +1) ;//calculate saturation value
	if(saturation > MAX_SATURATION){
		saturation = MAX_SATURATION;
	}
	saturation = (saturation * SATURATION_CHECK_PCT) / 10;
	//get saturation x 80%

	if ((current_raw->c > saturation) || (current_raw->w > saturation)){
		return true;
	}
	return false;
}

static bool ams_tcs3707_insufficience_check(ams_tcs3707_deviceCtx_t * ctx, ams_tcs3707_adc_data_t *current_raw)
{
	if((current_raw == NULL) || (ctx == NULL)){
		hwlog_err("%s NULL poniter\n", __func__);
		return false;
	}
	if (current_raw->c < AMS_TCS3707_LOW_LEVEL){
		return true;
	}
	return false;
}

#ifdef CONFIG_AMS_FLICKER_DETECT
static bool ams_tcs3707_fd_saturation_check(ams_tcs3707_deviceCtx_t * ctx, uint16_t value)
{
	UINT8 low_8_bit = 0;
	UINT8 high_3_bit = 0;
	UINT16 fd_atime = 0;
	UINT16 fd_time = 0;
	UINT32 saturation = 0;

	if(ctx == NULL){
		hwlog_err("%s NULL poniter\n", __func__);
		return false;
	}
	ams_tcs3707_getByte(ctx->portHndl, TCS3707_FD_CFG1, &low_8_bit);
	ams_tcs3707_getField(ctx->portHndl, TCS3707_FD_CFG3, &high_3_bit, FD_TIME_HIGH_3_BIT_MASK);
	fd_atime = (high_3_bit << 8) | low_8_bit;//get fd_atime
	fd_time = (fd_atime+1)*(0+1);//step values is 2.78us. so astep value is 0. saturation value = (ATIME+1)*(ASTEP+1)
	saturation = (fd_time*8)/10;//get real 0.8 saturation

	if(value >= saturation ){
		return true;
	}
	return false;
}
static bool ams_tcs3707_fd_insufficience_check(ams_tcs3707_deviceCtx_t * ctx, uint16_t value)
{
	if (value < AMS_TCS3707_FD_LOW_LEVEL){
		return true;
	}
	return false;
}
#endif

static bool ams_tcs3707_handleALS(ams_tcs3707_deviceCtx_t * ctx, bool inCalMode)
{

	UINT8 adc_data[AMS_TCS3707_ADC_BYTES] = {0};
	bool saturation_check = false;
	bool insufficience_check = false;
	bool re_enable = false;

	ams_tcs3707_adc_data_t current_raw = {
		.c   = 0,
		.r   = 0,
		.g   = 0,
		.b   = 0,
		.w   = 0
	};

	if (ctx == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return true;
	}

	/* read raw ADC data */
	ams_tcs3707_getBuf(ctx->portHndl, TCS3707_ADATA0L_REG, adc_data, AMS_TCS3707_ADC_BYTES);

	current_raw.c = (adc_data[0] << 0) | (adc_data[1] << ONE_BYTE_LENGTH_8_BITS);
	current_raw.r = (adc_data[2] << 0) | (adc_data[3] << ONE_BYTE_LENGTH_8_BITS);
	current_raw.g = (adc_data[4] << 0) | (adc_data[5] << ONE_BYTE_LENGTH_8_BITS);
	current_raw.b = (adc_data[6] << 0) | (adc_data[7] << ONE_BYTE_LENGTH_8_BITS);
	current_raw.w = (adc_data[8] << 0) | (adc_data[9] << ONE_BYTE_LENGTH_8_BITS);

	if (inCalMode)
	{
		ctx->updateAvailable |= AMS_TCS3707_FEATURE_ALS;
		goto handleALS_exit;
	}

	/* use a short timer to quickly fix a proper gain for first data */
	if (ctx->first_inte)
	{
		hwlog_info("before enter gain adjust, ams_tcs3707_handleALS: ITIME = %d, AGAIN = %d\n",
			(UINT16)ctx->algCtx.als_data.itime_ms,(UINT32)ctx->algCtx.als_data.gain);
		/*
		 * first integration time is 2.78ms,
		 * so the adc saturation value is 1000
		 * saturation_new = 1000*0.8 = 800,
		 * the init gain value is 4x,
		 * so if gain change to 16, also not saturated,
		 * so the threshold is 800/4 = 200;
		 * so if gain change to 64, also not saturated,
		 * so the threshold is 800/16 = 50;
		 * so if gain change to 256, also not saturated,
		 * the threshold is 800/64 = 13;
		 */
		if(current_raw.c <= GAIN_QUICKLY_FIX_LEVEL_1){
			ams_tcs3707_setGain(ctx, (GAIN_LEVEL_5 * AMS_TCS3707_GAIN_SCALE));//use 256 gain
		}else if(current_raw.c <= GAIN_QUICKLY_FIX_LEVEL_2){
			ams_tcs3707_setGain(ctx, (GAIN_LEVEL_4 * AMS_TCS3707_GAIN_SCALE));//use 64 gain
		}else if(current_raw.c <= GAIN_QUICKLY_FIX_LEVEL_3){
			ams_tcs3707_setGain(ctx, (GAIN_LEVEL_3 * AMS_TCS3707_GAIN_SCALE));//use 16 gain
		}else if(current_raw.c <= GAIN_QUICKLY_FIX_LEVEL_4){
			hwlog_info("use the init 4x gain\n");//use 4 gain
		}else{
			ams_tcs3707_setGain(ctx, (GAIN_LEVEL_1 * AMS_TCS3707_GAIN_SCALE));//use 1 gain
		}
		//note: if the init gain changes or the sensor can recieve much light, above threshold values can be changed.
		//change ITIME back to AMS_TCS3707_ITIME_DEFAUL again after changing the gain
		ctx->algCtx.als_data.itime_ms = AMS_TCS3707_ITIME_DEFAULT;
		ams_tcs3707_setItime(ctx, ctx->algCtx.als_data.itime_ms);

		re_enable = true;
		ctx->first_inte = false;
		goto handleALS_exit;
	}

	saturation_check = ams_tcs3707_saturation_check(ctx, &current_raw);
	insufficience_check = ams_tcs3707_insufficience_check(ctx, &current_raw);

	/* Adjust gain setting */
	if (saturation_check &&	ctx->algCtx.als_data.gain == (GAIN_LEVEL_2* AMS_TCS3707_GAIN_SCALE))
	{
		ams_tcs3707_setGain(ctx, (GAIN_LEVEL_1 * AMS_TCS3707_GAIN_SCALE));
		re_enable = true;
	}
	else if ((saturation_check && ctx->algCtx.als_data.gain == (GAIN_LEVEL_3 * AMS_TCS3707_GAIN_SCALE)) ||
			(insufficience_check &&	ctx->algCtx.als_data.gain == (GAIN_LEVEL_1 * AMS_TCS3707_GAIN_SCALE)))
	{
		ams_tcs3707_setGain(ctx, (GAIN_LEVEL_2 * AMS_TCS3707_GAIN_SCALE));
		re_enable = true;
	}
	else if ((saturation_check && ctx->algCtx.als_data.gain == (GAIN_LEVEL_4 * AMS_TCS3707_GAIN_SCALE)) ||
			(insufficience_check &&	ctx->algCtx.als_data.gain == (GAIN_LEVEL_2 * AMS_TCS3707_GAIN_SCALE)))
	{
		ams_tcs3707_setGain(ctx, (GAIN_LEVEL_3 * AMS_TCS3707_GAIN_SCALE));
		re_enable = true;
	}
	else if ((saturation_check && ctx->algCtx.als_data.gain == (GAIN_LEVEL_5 * AMS_TCS3707_GAIN_SCALE)) ||
			(insufficience_check && ctx->algCtx.als_data.gain == (GAIN_LEVEL_3 * AMS_TCS3707_GAIN_SCALE)))
	{
		ams_tcs3707_setGain(ctx, (GAIN_LEVEL_4 * AMS_TCS3707_GAIN_SCALE));
		re_enable = true;
	}
	else if (insufficience_check &&	ctx->algCtx.als_data.gain == (GAIN_LEVEL_4 * AMS_TCS3707_GAIN_SCALE))
	{
		ams_tcs3707_setGain(ctx, (GAIN_LEVEL_5 * AMS_TCS3707_GAIN_SCALE));
		re_enable = true;
	}
	else
	{
		ctx->updateAvailable |= AMS_TCS3707_FEATURE_ALS;
	}

handleALS_exit:
	if (!re_enable)
	{
		ctx->algCtx.als_data.datasetArray.c   = current_raw.c;
		ctx->algCtx.als_data.datasetArray.r   = current_raw.r;
		ctx->algCtx.als_data.datasetArray.g   = current_raw.g;
		ctx->algCtx.als_data.datasetArray.b   = current_raw.b;
		ctx->algCtx.als_data.datasetArray.w   = current_raw.w;
	}
	return re_enable;
}

/* --------------------------------------------------------------------*/
/* Called by the OSAL interrupt service routine                        */
/* --------------------------------------------------------------------*/
static bool ams_tcs3707_deviceEventHandler(ams_tcs3707_deviceCtx_t * ctx, bool inCalMode)
{
	bool ret = false;

	if (ctx == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return false;
	}

	ams_tcs3707_getByte(ctx->portHndl, TCS3707_STATUS_REG, &ctx->algCtx.als_data.status);
	if (ctx->algCtx.als_data.status & (AINT_MASK)){
		ret = ams_tcs3707_handleALS(ctx, inCalMode);
		/* Clear the soft interrupt */
		ams_tcs3707_setByte(ctx->portHndl,
			TCS3707_STATUS_REG,
			(ASAT_MASK | AINT_MASK));
	}
	return ret;
}

static UINT32 ams_tcs3707_deviceGetResult(ams_tcs3707_deviceCtx_t * ctx)
{
	if (ctx == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return true;
	}
	return ctx->updateAvailable;
}

static bool ams_tcs3707_deviceGetAls(ams_tcs3707_deviceCtx_t * ctx, export_alsData_t * exportData)
{
	if ((ctx == NULL) || (exportData == NULL)){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return true;
	}
	ctx->updateAvailable &= ~(AMS_TCS3707_FEATURE_ALS);

	exportData->rawC = ctx->algCtx.als_data.datasetArray.c;
	exportData->rawR = ctx->algCtx.als_data.datasetArray.r;
	exportData->rawG = ctx->algCtx.als_data.datasetArray.g;
	exportData->rawB = ctx->algCtx.als_data.datasetArray.b;
	exportData->rawW = ctx->algCtx.als_data.datasetArray.w;
	return false;
}



/* --------------------------------------------------------------------
 * Return default calibration data
 * --------------------------------------------------------------------*/
static void ams_tcs3707_getDefaultCalData(ams_tcs3707_calibrationData_t *cal_data)
{
	if (cal_data == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}

	cal_data->timeBase_us = AMS_TCS3707_USEC_PER_TICK;
	return;
}

static bool ams_tcs3707_getDeviceInfo(ams_tcs3707_deviceInfo_t * info)
{
	if (info == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return true;
	}
	memset(info, 0, sizeof(ams_tcs3707_deviceInfo_t));
	info->memorySize =  sizeof(ams_tcs3707_deviceCtx_t);
	ams_tcs3707_getDefaultCalData(&info->defaultCalibrationData);
	return false;
}

void osal_tcs3707_als_timerHndl(unsigned long data)
{
	struct colorDriver_chip *chip = (struct colorDriver_chip*) data;

	if (chip == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}
	schedule_work(&chip->als_work);
}
#ifdef CONFIG_AMS_FLICKER_DETECT
void osal_fd_timerHndl(unsigned long data)
{
	struct colorDriver_chip *chip = (struct colorDriver_chip*) data; 
	if (chip == NULL){
		hwlog_err("\nAMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}
	schedule_work(&chip->fd_work);
}
#endif

static UINT8 get_rgb_fd_enable_status(AMS_PORT_portHndl *portHndl)
{
	UINT8 enable_sta = 0;

	if (portHndl == NULL) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return 0;
	}
	AMS_PORT_TCS3707_getByte(portHndl,
		TCS3707_ENABLE_REG, &enable_sta,
		ONE_BYTE);

	return enable_sta;
}

static ssize_t osal_als_enable_set(struct colorDriver_chip *chip, uint8_t valueToSet)
{
	UINT8 rgb_enable_status;

	if (chip == NULL) {
		hwlog_err("%s: chip Pointer is NULL\n", __func__);
		return true;
	}

	hwlog_info("osal_als_enable_set = %d\n", valueToSet);

	rgb_enable_status = get_rgb_fd_enable_status(chip->client);
	ams_tcs3707_rgb_power_onoff(chip->deviceCtx,
		valueToSet, chip->inCalMode);

	if (valueToSet) { //enable set
		if ((rgb_enable_status & AEN) != AEN) {
			if (chip->inCalMode == false) {
				mod_timer(&chip->work_timer,
					jiffies +
					msecs_to_jiffies(FIRST_RGB_TIMER));
				hwlog_info("osal_als_enable_set 4ms for finding a proper gain quickly\n");
				report_logcount = AMS_REPORT_LOG_COUNT_NUM;
			} else {
				mod_timer(&chip->work_timer,
					jiffies +
					msecs_to_jiffies(CALI_RGB_TIMER));
				//calibrate enable set the timer as 120ms
				hwlog_info("in calibrate mode timer set as 120ms \n");
			}
		} else { //already enable
			hwlog_warn("osal_als_enable_set: timer already running\n");
		}
	} else {
		hwlog_info("osal_als_enable_set: del_timer\n");
	}

	return 0;
}
static int get_cal_para_from_nv(void)
{
	int i = 0, ret = 0;

	ret = read_color_calibrate_data_from_nv(RGBAP_CALI_DATA_NV_NUM, RGBAP_CALI_DATA_SIZE, "RGBAP", (char *)&color_nv_para);
	if(ret < 0){
		hwlog_err("AMS_Driver: %s: fail,use default para!!\n", __func__);
		for (i = 0; i < CAL_STATE_GAIN_LAST; i++){
			hwlog_err("AMS_Driver: get_cal_para_from_nv: gain[%d]: [%d, %d, %d, %d,%d]\n", i,
			color_nv_para.calCratio[i], color_nv_para.calRratio[i], color_nv_para.calGratio[i], 
			color_nv_para.calBratio[i], color_nv_para.calWratio[i]);
		}
		return 0;
	}

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++){
		hwlog_warn("AMS_Driver: get_cal_para_from_nv: gain[%d]: [%d, %d, %d, %d, %d]\n", i,
		color_nv_para.calCratio[i], color_nv_para.calRratio[i], color_nv_para.calGratio[i], 
		color_nv_para.calBratio[i],color_nv_para.calWratio[i]);
		if(!color_nv_para.calCratio[i]||!color_nv_para.calRratio[i]
			||!color_nv_para.calGratio[i]||!color_nv_para.calBratio[i]||!color_nv_para.calWratio[i]){
			color_nv_para.calCratio[i] = AMS_TCS3707_FLOAT_TO_FIX ;
			color_nv_para.calRratio[i] = AMS_TCS3707_FLOAT_TO_FIX ;
			color_nv_para.calGratio[i] = AMS_TCS3707_FLOAT_TO_FIX ;
			color_nv_para.calBratio[i] = AMS_TCS3707_FLOAT_TO_FIX ;
			color_nv_para.calWratio[i] = AMS_TCS3707_FLOAT_TO_FIX ;
		}
	}
	return 1;
}

static int save_cal_para_to_nv(struct colorDriver_chip *chip)
{
	int i = 0, ret = 0;
	if (chip == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return 0;
	}

	color_nv_para.nv_Ctarget = chip->calibrationCtx.calCtarget;
	color_nv_para.nv_Rtarget = chip->calibrationCtx.calRtarget;
	color_nv_para.nv_Gtarget = chip->calibrationCtx.calGtarget;
	color_nv_para.nv_Btarget = chip->calibrationCtx.calBtarget;
	color_nv_para.nv_Wtarget = chip->calibrationCtx.calWtarget;

	for(i = 0; i < CAL_STATE_GAIN_LAST; i++){
		color_nv_para.calCratio[i] = chip->calibrationCtx.calCresult[i];
		color_nv_para.calRratio[i]  = chip->calibrationCtx.calRresult[i];
		color_nv_para.calGratio[i]  = chip->calibrationCtx.calGresult[i];
		color_nv_para.calBratio[i] = chip->calibrationCtx.calBresult[i];
		color_nv_para.calWratio[i] = chip->calibrationCtx.calWresult[i];
		hwlog_info("AMS_Driver: save_cal_para_to_nv: gain[%d]: [%d, %d, %d, %d,%d]\n", i,
		color_nv_para.calCratio[i], color_nv_para.calRratio[i], color_nv_para.calGratio[i], 
		color_nv_para.calBratio[i],color_nv_para.calWratio[i]);

	}

	ret = write_color_calibrate_data_to_nv(RGBAP_CALI_DATA_NV_NUM, RGBAP_CALI_DATA_SIZE, "RGBAP", (char *)&color_nv_para);
	if(ret < 0){
		hwlog_err("AMS_Driver: %s: fail\n", __func__);
	}
	return 1;
}

static void osal_calHandl_als(struct colorDriver_chip *chip)
{
	export_alsData_t outData;
	ams_tcs3707_deviceCtx_t * ctx = NULL;
	UINT32 currentGain = 0;
	UINT32 result = 0; /* remember, this is for fixed point and can cause lower performance */

	hwlog_info( "comes into osal_calHandl_als \n");

	if (chip == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}
	if (chip->deviceCtx == NULL){
		hwlog_err("AMS_Driver: %s: deviceCtx is NULL\n", __func__);
		return;
	}

	ctx = chip->deviceCtx;
	currentGain = (ctx->algCtx.als_data.gain / AMS_TCS3707_GAIN_SCALE);

	ams_tcs3707_deviceGetAls(chip->deviceCtx, &outData);

	hwlog_info( "osal_calHandl_als: state = %d, count = %d, currentGain = %d\n", chip->calibrationCtx.calState, \
		chip->calibrationCtx.calSampleCounter, currentGain);
	ams_tcs3707_getGain(ctx);

	if (chip->calibrationCtx.calState < CAL_STATE_GAIN_LAST && chip->calibrationCtx.calState >= 0){
		chip->calibrationCtx.calSampleCounter++;
		chip->calibrationCtx.calCsample += outData.rawC;
		chip->calibrationCtx.calRsample += outData.rawR;
		chip->calibrationCtx.calGsample += outData.rawG;
		chip->calibrationCtx.calBsample += outData.rawB;
		chip->calibrationCtx.calWsample += outData.rawW;

		if((chip->calibrationCtx.calState < CAL_STATE_GAIN_LAST) && (chip->calibrationCtx.calState >= 0)){
			//judge C channel
			hwlog_info("%s, get C channel value = %d, target = %d, calState = %d\n", __func__, chip->calibrationCtx.calCsample, \
				chip->calibrationCtx.calCtarget, chip->calibrationCtx.calState);

			result = (chip->calibrationCtx.calCsample / AMS_TCS3707_CAL_AVERAGE);
			if (result){
				result = (chip->calibrationCtx.calCtarget * (currentGain *
					AMS_TCS3707_FLOAT_TO_FIX / AMS_TCS3707_GAIN_OF_GOLDEN)) / result;
				if(result > ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].high_thr||
					result < ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].low_thr){
					hwlog_err("%s: ratio is out bound[%d, %d]! calCresult[%d] = %d\n" , __func__,
						ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].high_thr,
						ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].low_thr, chip->calibrationCtx.calState, result);
					color_calibrate_result = false;
				}
			} else {
				/* cant devide by zero, maintain the default calibration scaling factor */
				result = AMS_TCS3707_FLOAT_TO_FIX;
			}
			chip->calibrationCtx.calCresult[chip->calibrationCtx.calState] = result;
			hwlog_info("%s, get R channel value = %d, target = %d, calState = %d\n", __func__, chip->calibrationCtx.calRsample, \
				chip->calibrationCtx.calRtarget, chip->calibrationCtx.calState);

			//judge R channel
			result = (chip->calibrationCtx.calRsample / AMS_TCS3707_CAL_AVERAGE);
			if (result){
				result = (chip->calibrationCtx.calRtarget * (currentGain *
					AMS_TCS3707_FLOAT_TO_FIX / AMS_TCS3707_GAIN_OF_GOLDEN)) / result;
				if(result > ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].high_thr
					|| result < ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].low_thr ){
					hwlog_err("%s: ratio is out bound[%d, %d]! calRresult[%d] = %d\n" , __func__,
						ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].high_thr,
						ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].low_thr, chip->calibrationCtx.calState, result);
					color_calibrate_result = false;
				}
			} else {
				/* cant devide by zero, maintain the default calibration scaling factor */
				result = AMS_TCS3707_FLOAT_TO_FIX;
			}
			chip->calibrationCtx.calRresult[chip->calibrationCtx.calState] = result;

			//judge G channel
			hwlog_info("%s, get G channel value = %d, target = %d, calState = %d\n", __func__, chip->calibrationCtx.calGsample, \
				chip->calibrationCtx.calGtarget, chip->calibrationCtx.calState);
			result = (chip->calibrationCtx.calGsample / AMS_TCS3707_CAL_AVERAGE);
			if (result){
				result = (chip->calibrationCtx.calGtarget * (currentGain *
					AMS_TCS3707_FLOAT_TO_FIX / AMS_TCS3707_GAIN_OF_GOLDEN)) / result;
				if(result > ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].high_thr
					|| result < ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].low_thr ){

					hwlog_err("%s: ratio is out bound[%d, %d]! calGresult[%d] = %d\n" , __func__,
						ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].high_thr,
						ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].low_thr, chip->calibrationCtx.calState, result);
					color_calibrate_result = false;
				}
			} else {
				/* cant devide by zero, maintain the default calibration scaling factor */
				result = AMS_TCS3707_FLOAT_TO_FIX;
			}
			chip->calibrationCtx.calGresult[chip->calibrationCtx.calState] = result;
			hwlog_info("%s, get B channel value = %d, target = %d, calState = %d\n", __func__, chip->calibrationCtx.calBsample, \
				chip->calibrationCtx.calBtarget, chip->calibrationCtx.calState);

			//judge B channel
			result = (chip->calibrationCtx.calBsample / AMS_TCS3707_CAL_AVERAGE);
			if (result){
				result = (chip->calibrationCtx.calBtarget * (currentGain *
					AMS_TCS3707_FLOAT_TO_FIX / AMS_TCS3707_GAIN_OF_GOLDEN)) / result;
				if(result > ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].high_thr||
					result < ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].low_thr ){

					hwlog_err("%s: ratio is out bound[%d, %d]! calBresult[%d] = %d\n" , __func__,
						ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].high_thr,
						ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].low_thr, chip->calibrationCtx.calState, result);
					color_calibrate_result = false;
				}
			} else {
				/* cant devide by zero, maintain the default calibration scaling factor */
				hwlog_err("%s: calBresult[%d] = 0!!\n" , __func__, chip->calibrationCtx.calState);
				result = AMS_TCS3707_FLOAT_TO_FIX;
			}
			chip->calibrationCtx.calBresult[chip->calibrationCtx.calState] = result;
			hwlog_info("%s, get W channel value = %d, target = %d, calState = %d\n", __func__, chip->calibrationCtx.calWsample, \
				chip->calibrationCtx.calWtarget, chip->calibrationCtx.calState);

			//judge W channel
			result = (chip->calibrationCtx.calWsample / AMS_TCS3707_CAL_AVERAGE);
			if (result){
				result = (chip->calibrationCtx.calWtarget * (currentGain *
					AMS_TCS3707_FLOAT_TO_FIX / AMS_TCS3707_GAIN_OF_GOLDEN)) / result;
				if(result > ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].high_thr||
					result < ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].low_thr ){

					hwlog_err("%s: ratio is out bound[%d, %d]! calWresult[%d] = %d\n" , __func__,	
						ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].high_thr,
						ams_tcs3707_setGainThreshold[chip->calibrationCtx.calState].low_thr, chip->calibrationCtx.calState, result);
					color_calibrate_result = false;
				}
			} else {
				/* cant devide by zero, maintain the default calibration scaling factor */
				hwlog_err("%s: calWresult[%d] = 0!!\n" , __func__, chip->calibrationCtx.calState);
				result = AMS_TCS3707_FLOAT_TO_FIX;
			}
			chip->calibrationCtx.calWresult[chip->calibrationCtx.calState] = result;

			hwlog_info( "osal_calHandl_als: calCresult  %d\n", chip->calibrationCtx.calCresult[chip->calibrationCtx.calState]);
			hwlog_info( "osal_calHandl_als: calRresult  %d\n", chip->calibrationCtx.calRresult[chip->calibrationCtx.calState]);
			hwlog_info( "osal_calHandl_als: calGresult  %d\n", chip->calibrationCtx.calGresult[chip->calibrationCtx.calState]);
			hwlog_info( "osal_calHandl_als: calBresult %d\n", chip->calibrationCtx.calBresult[chip->calibrationCtx.calState]);
			hwlog_info( "osal_calHandl_als: calWresult %d\n", chip->calibrationCtx.calWresult[chip->calibrationCtx.calState]);

			chip->calibrationCtx.calState++;
			chip->calibrationCtx.calSampleCounter = 0;
			chip->calibrationCtx.calCsample = 0;
			chip->calibrationCtx.calRsample = 0;
			chip->calibrationCtx.calGsample = 0;
			chip->calibrationCtx.calBsample = 0;
			chip->calibrationCtx.calWsample = 0;
			if(chip->calibrationCtx.calState < CAL_STATE_GAIN_LAST){
				osal_als_enable_set(chip, AMSDRIVER_ALS_DISABLE);
				ctx->algCtx.als_data.gain = ams_tcs3707_alsGain_conversion[chip->calibrationCtx.calState];
				osal_als_enable_set(chip, AMSDRIVER_ALS_ENABLE);
			}
		}
	}
	else {
		if(true == color_calibrate_result){
			save_cal_para_to_nv(chip);
			report_calibrate_result = true;
		}else {
			dev_err(&chip->client->dev, "color_calibrate_result fail\n");
			report_calibrate_result = false;
		}
		chip->inCalMode = false;
		osal_als_enable_set(chip, AMSDRIVER_ALS_DISABLE);

		if(1 == enable_status_before_calibrate){
			osal_als_enable_set(chip, AMSDRIVER_ALS_ENABLE);
		}else{
			hwlog_info( "color sensor disabled before calibrate\n");
		}
		hwlog_info( "osal_calHandl_als: done\n");
	}
	return;
}

static void  osal_report_als(struct colorDriver_chip *chip)
{
	export_alsData_t outData = {0};
	UINT8 currentGainIndex = 0;
	UINT8 currentGainRadioIndex = 0;
	UINT32 currentGain = 0;
	UINT8 j = 0;
	ams_tcs3707_deviceCtx_t * ctx = NULL;

	if (chip == NULL){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}
	ctx = chip->deviceCtx;

	ams_tcs3707_deviceGetAls(chip->deviceCtx, &outData);
	if(NULL != ctx){
		currentGain = ctx->algCtx.als_data.gain;
	}
	if (currentGain < ams_tcs3707_alsGain_conversion[CAL_STATE_GAIN_5]){
		currentGainIndex = ams_tcs3707_gainToReg(currentGain);
	} else {
		currentGainIndex = CAL_STATE_GAIN_5;
	}

	if (currentGain < ams_tcs3707_alsGain_conversion[CAL_STATE_GAIN_3]){
		currentGainRadioIndex = ams_tcs3707_gainToReg(ams_tcs3707_alsGain_conversion[CAL_STATE_GAIN_3]);
	} else if(currentGain < ams_tcs3707_alsGain_conversion[CAL_STATE_GAIN_5]){
		currentGainRadioIndex = ams_tcs3707_gainToReg(currentGain);
	}else {
		currentGainRadioIndex = CAL_STATE_GAIN_5;
	}

	/* adjust the report data when the calibrate ratio is acceptable */

	outData.rawC *= color_nv_para.calCratio[currentGainRadioIndex];
	outData.rawC /= AMS_TCS3707_FLOAT_TO_FIX;
	
	outData.rawR *= color_nv_para.calRratio[currentGainRadioIndex];
	outData.rawR /= AMS_TCS3707_FLOAT_TO_FIX;
	
	outData.rawG *= color_nv_para.calGratio[currentGainRadioIndex];
	outData.rawG /= AMS_TCS3707_FLOAT_TO_FIX;
	
	outData.rawB *= color_nv_para.calBratio[currentGainRadioIndex];
	outData.rawB /= AMS_TCS3707_FLOAT_TO_FIX;

	outData.rawW *= color_nv_para.calWratio[currentGainRadioIndex];
	outData.rawW /= AMS_TCS3707_FLOAT_TO_FIX;

	report_value[0] = (int)outData.rawC;
	report_value[1] = (int)outData.rawR;
	report_value[2] = (int)outData.rawG;
	report_value[3] = (int)outData.rawB;
	report_value[4] = (int)outData.rawW;
	color_report_val[0] = AMS_REPORT_DATA_LEN;

	for(j = 0; j < 5; j++){
		report_value[j] *= AMS_TCS3707_GAIN_OF_GOLDEN;
		report_value[j] /= ams_tcs3707_als_gains[currentGainIndex];
		color_report_val[j+1] = report_value[j];
	}

	ams_tcs3707_report_data(report_value);
	report_logcount++;
	if(report_logcount >= AMS_REPORT_LOG_COUNT_NUM){
		hwlog_info("COLOR SENSOR tcs3707 report data %d, %d, %d, %d, %d, currentGain[%d]\n",
			report_value[0], report_value[1], report_value[2], report_value[3], report_value[4], ams_tcs3707_als_gains[currentGainIndex]);
		hwlog_info("AMS_Driver: currentGain[%d],currentGainIndex=[%d], currentGainRadioIndex[%d], calCratio[%d],calRratio[%d],calGratio[%d],calBratio[%d],calWratio[%d]\n",
			ams_tcs3707_als_gains[currentGainIndex],
			currentGainIndex,
			currentGainRadioIndex,
			color_nv_para.calCratio[currentGainRadioIndex],
			color_nv_para.calRratio[currentGainRadioIndex],
			color_nv_para.calGratio[currentGainRadioIndex],
			color_nv_para.calBratio[currentGainRadioIndex],
			color_nv_para.calWratio[currentGainRadioIndex]);
		report_logcount = 0;
	}
}

int ams_tcs3707_setenable(bool enable)
{
	struct colorDriver_chip *chip = p_chip;
	if (enable)
		osal_als_enable_set(chip, AMSDRIVER_ALS_ENABLE);
	else
		osal_als_enable_set(chip, AMSDRIVER_ALS_DISABLE);
	return 1;
}
EXPORT_SYMBOL_GPL(ams_tcs3707_setenable);

void ams_tcs3707_show_calibrate(struct colorDriver_chip *chip, at_color_sensor_output_para * out_para)
{
	int i = 0;

	if ((NULL == out_para) || (NULL == chip)){
		hwlog_err("ams_show_calibrate input para NULL \n");
		return;
	}

	if (chip->inCalMode == false){
		hwlog_err("ams_show_calibrate not in calibration mode \n");
	}

	out_para->result = (UINT32)report_calibrate_result;
	hwlog_info(" color_calibrate_show result = %d\n", out_para->result);
	out_para->gain_arr = CAL_STATE_GAIN_LAST;
	out_para->color_arr = AMS_REPORT_DATA_LEN;
	memcpy(out_para->report_gain, ams_tcs3707_als_gains, sizeof(out_para->report_gain));
	memcpy(out_para->report_raw[0], chip->calibrationCtx.calCresult, sizeof(out_para->report_raw[0]));
	memcpy(out_para->report_raw[1],  chip->calibrationCtx.calRresult,  sizeof(out_para->report_raw[1]));
	memcpy(out_para->report_raw[2],  chip->calibrationCtx.calGresult,  sizeof(out_para->report_raw[2]));
	memcpy(out_para->report_raw[3],  chip->calibrationCtx.calBresult,  sizeof(out_para->report_raw[3]));
	memcpy(out_para->report_raw[4],  chip->calibrationCtx.calWresult,  sizeof(out_para->report_raw[4]));

	for(i = 0;i<CAL_STATE_GAIN_LAST; i++)
	{
		hwlog_info(" color_calibrate_show i = %d: %d,%d,%d,%d,%d.\n", i,
		out_para->report_raw[0][i],out_para->report_raw[1][i],out_para->report_raw[2][i],out_para->report_raw[3][i],out_para->report_raw[4][i]);
	}
	return;
}

void ams_tcs3707_store_calibrate(struct colorDriver_chip *chip, at_color_sensor_input_para * in_para)
{
	ams_tcs3707_deviceCtx_t * ctx = NULL;
	color_sensor_input_para_tcs3707 input_para;
	UINT8 rgb_enable_status;
	if((NULL == chip) || (NULL == in_para)){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}

	AMS_MUTEX_LOCK(&chip->lock);
	report_calibrate_result = false;
	input_para.enable = in_para->enable;
	input_para.tar_c = in_para->reserverd[0]; /* calculate targer for gain 1x (assuming its set at 64x) */
	input_para.tar_r = in_para->reserverd[1];
	input_para.tar_g = in_para->reserverd[2];
	input_para.tar_b = in_para->reserverd[3];
	input_para.tar_w = in_para->reserverd[4];

	if (input_para.enable &&  chip->inCalMode){
		AMS_MUTEX_UNLOCK(&chip->lock);
		hwlog_err("ams_store_calibrate: Already in calibration mode.\n");
		return;
	}

	if (input_para.enable){
		ctx = chip->deviceCtx;
		hwlog_info( "ams_store_calibrate: starting calibration mode\n");
		chip->calibrationCtx.calSampleCounter = 0;
		chip->calibrationCtx.calCsample = 0;
		chip->calibrationCtx.calRsample = 0;
		chip->calibrationCtx.calGsample = 0;
		chip->calibrationCtx.calBsample = 0;
		chip->calibrationCtx.calWsample = 0;
		chip->calibrationCtx.calCtarget= input_para.tar_c; /* calculate targer for gain 1x (assuming its set at 64x) */
		chip->calibrationCtx.calRtarget= input_para.tar_r;
		chip->calibrationCtx.calGtarget= input_para.tar_g;
		chip->calibrationCtx.calBtarget= input_para.tar_b;
		chip->calibrationCtx.calWtarget= input_para.tar_w;
		chip->calibrationCtx.calState = CAL_STATE_GAIN_1;
		hwlog_info( "input_para->tar_c = %d,input_para->tar_r = %d,,input_para->tar_g = %d,input_para->tar_b = %d ,input_para->tar_w = %d\n",
		input_para.tar_c,input_para.tar_r,input_para.tar_g,input_para.tar_b,input_para.tar_w);

		if(NULL == ctx){
			hwlog_err("%s ctx null\n", __func__);
			AMS_MUTEX_UNLOCK(&chip->lock);
			return;
		}
		ctx->algCtx.als_data.gain = ams_tcs3707_alsGain_conversion[CAL_STATE_GAIN_1];
		ctx->algCtx.als_data.itime_ms = AMS_TCS3707_ITIME_DEFAULT;
		rgb_enable_status = get_rgb_fd_enable_status(ctx->portHndl);
		if ((rgb_enable_status & AEN) == AEN) {
			enable_status_before_calibrate = 1;//enabled before calibrate
			hwlog_info("AMS_Driver: %s: enabled before calibrate\n", __func__);
			osal_als_enable_set(chip, AMSDRIVER_ALS_DISABLE);
			msleep(10);//sleep 10 ms to make sure disable timer
		} else {
			enable_status_before_calibrate = 0;//disabled before calibrate
			hwlog_info("AMS_Driver: %s: disabled before calibrate\n", __func__);
		}
		chip->inCalMode = true;
		color_calibrate_result = true;//make the calibrate_result true for calibrate again!!
		osal_als_enable_set(chip, AMSDRIVER_ALS_ENABLE);
	} else {
		hwlog_info( "ams_store_calibrate: stopping calibration mode\n");
		chip->inCalMode = false;
		}

	AMS_MUTEX_UNLOCK(&chip->lock);
	return;
}

void ams_tcs3707_show_enable(struct colorDriver_chip *chip, int *state)
{
	UINT8 rgb_enable_status;
	if((NULL == chip) || (NULL == state)){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}

	rgb_enable_status = get_rgb_fd_enable_status(chip->client);

	if (rgb_enable_status & AEN) {
		*state = 1;
	} else {
		*state = 0;
	}
}

void ams_tcs3707_store_enable(struct colorDriver_chip *chip, int state)
{
	if(NULL == chip){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}
	if (state)
		osal_als_enable_set(chip, AMSDRIVER_ALS_ENABLE);
	else
		osal_als_enable_set(chip, AMSDRIVER_ALS_DISABLE);
}
#endif

#ifdef CONFIG_AMS_FLICKER_DETECT
static void fd_enable_set(struct colorDriver_chip *chip, uint8_t valueToSet)
{
	ams_tcs3707_deviceCtx_t* ctx = NULL;
	UINT8 fd_enable_status;

	if (chip == NULL){
		hwlog_err("\nAMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}
	ctx = (ams_tcs3707_deviceCtx_t*)chip->deviceCtx;
	if (ctx == NULL) {
		hwlog_err("\nAMS_Driver: %s: ctx Pointer is NULL\n", __func__);
		return;
	}
	hwlog_info("fd_enable_set valueToSet = %d\n", valueToSet);

	fd_enable_status = get_rgb_fd_enable_status(ctx->portHndl);
	if (valueToSet) {
		if ((fd_enable_status & FDEN) != FDEN) {
			ams_tcs3707_set_fd_gain(ctx, DEFAULT_FD_GAIN);
			ctx->fd_data.first_fd_inte = true;
			mod_timer(&chip->fd_timer, jiffies + msecs_to_jiffies(FIRST_FLK_TIMER));//first enable flicker timer
			fd_timer_stopped = false;
			hwlog_info("fd_enable_set 6ms for finding a proper fliker detect gain quickly\n");
		} else {
			hwlog_warn("fd_enable_set fd has already been enabled\n");
		}
	} else {
		if ((fd_enable_status & FDEN) == FDEN) {
			ams_tcs3707_set_enable(ctx->portHndl,
				TCS3707_ENABLE_REG,
				0,
				FDEN);
			fd_timer_stopped = true;
			memset(ring_buffer, 0, MAX_BUFFER_SIZE*2);//clear ring_buffer when close the fd
			buffer_ptr = &ring_buffer[0];
			head_ptr = &ring_buffer[0];
			tail_ptr = &ring_buffer[0];
			hwlog_info("fd_enable_set now disable fd sensor\n");
		}else{
			hwlog_warn("fd_enable_set fd has already been disabled\n");
		}
	}
	return;
}

void ams_tcs3707_fd_show_enable(struct colorDriver_chip *chip, int *state)
{
	UINT8 fd_enable_status;
	if((NULL == chip) || (NULL == state)){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}

	fd_enable_status = get_rgb_fd_enable_status(chip->client);
	if ((fd_enable_status & FDEN) == FDEN) {
		*state = 1;
	} else {
		*state = 0;
	}
}

void ams_tcs3707_fd_store_enable(struct colorDriver_chip *chip, int state)
{

	if(NULL == chip){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}
	if (state)
		fd_enable_set(chip, AMSDRIVER_FD_ENABLE);
	else
		fd_enable_set(chip, AMSDRIVER_FD_DISABLE);
	hwlog_info("%s enable = %d success\n", __func__, state);
}
#endif

static void amsdriver_work(struct work_struct *work)
{
	int ret = 0;
	bool re_enable = false;
	UINT32 rgb_data_status;
	ams_tcs3707_deviceCtx_t *ctx = NULL;
	UINT8 rgb_enable_status;
	struct colorDriver_chip *chip = NULL;

	if(NULL == work){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}
	chip = container_of(work, struct colorDriver_chip, als_work);
	if(NULL == chip){
		hwlog_err("AMS_Driver: %s: Pointer chip is NULL\n", __func__);
		return;
	}
	ctx = (ams_tcs3707_deviceCtx_t *)chip->deviceCtx;
	if (ctx == NULL) {
		hwlog_err("%s: ctx Pointer is NULL\n", __func__);
		return;
	}
	AMS_MUTEX_LOCK(&chip->lock);
	if(0 == read_nv_first_in){
		ret = get_cal_para_from_nv();
		if(!ret){
			hwlog_err("\ams_tcs3707: get_cal_para_from_nv fail \n");
		}
		read_nv_first_in = -1;// -1: do not read again.
	}
	re_enable = ams_tcs3707_deviceEventHandler((ams_tcs3707_deviceCtx_t*)chip->deviceCtx, chip->inCalMode);

	rgb_data_status = ams_tcs3707_deviceGetResult(
		(ams_tcs3707_deviceCtx_t *)chip->deviceCtx);
	if (rgb_data_status & AMS_TCS3707_FEATURE_ALS) {
		if (chip->inCalMode == false && !re_enable) {
			osal_report_als(chip);
		} else {
			hwlog_info( "amsdriver_work: calibration mode\n");
			osal_calHandl_als(chip);
		}
	}

	rgb_enable_status = get_rgb_fd_enable_status(ctx->portHndl);
	if (((rgb_enable_status & AEN) == AEN)) {
		if (chip->inCalMode == false) {
			if (re_enable) {
				mod_timer(&chip->work_timer,
					jiffies +
					msecs_to_jiffies(POLL_RGB_TIMER + 1));
				hwlog_info("crgb re_enable timer set 101ms\n");
			} else {
				// timer set as 100ms
				mod_timer(&chip->work_timer, jiffies + HZ / 10);
			}
		} else {
			//calibrate mode set timer for 120ms
			mod_timer(&chip->work_timer,
				jiffies + msecs_to_jiffies(120));
			hwlog_info("in calib mode mod timer set as 120ms\n");
		}
	}else{
		hwlog_warn("%s: rgb already disabled, del timer\n", __func__);
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
}

#ifdef CONFIG_AMS_FLICKER_DETECT


void get_read_out_buffer(void)
{
	uint32_t i = 0,j = 0;
	uint32_t head_ptr_delta = 0;
	uint32_t k = 0;
	u16 *temp_ptr = &ring_buffer[0];
	u16 *temp_head_ptr = head_ptr;

	head_ptr_delta = (uint32_t)((uint64_t)temp_head_ptr - (uint64_t)(&ring_buffer[0]));
	head_ptr_delta = head_ptr_delta/2;
	
	if(head_ptr_delta > MAX_BUFFER_SIZE -1){
		head_ptr_delta = MAX_BUFFER_SIZE -1;
	}
	memset(read_out_buffer, 0, sizeof(read_out_buffer));//clear buffer before every new circle, 16bit value, so size multiply 2

	for(i = 0; i < (MAX_BUFFER_SIZE - head_ptr_delta); i++){
		read_out_buffer[i] = *temp_head_ptr;
		temp_head_ptr++;
	}

	for(j = (MAX_BUFFER_SIZE - head_ptr_delta); j < MAX_BUFFER_SIZE; j++){
		read_out_buffer[j] = *temp_ptr;
		temp_ptr++;
	}

	for(k = 0; k < MAX_BUFFER_SIZE; k++){
		hwlog_debug("get_read_out_buffer read_out_buffer[%d] = %d\n",k,read_out_buffer[k]);
	}
}


void ring_buffer_process(uint8_t fifo_lvl, uint16_t *buf_16)
{
	int i = 0;

	mutex_lock(&ring_buffer_mutex);

	for(i = 0; i < fifo_lvl; i++){
		*buffer_ptr = buf_16[i];
		tail_ptr = buffer_ptr;
		hwlog_debug("tcs3707_report_als *buffer_ptr = %d\n", *buffer_ptr);
		buffer_ptr++;

		if(buffer_ptr == &ring_buffer[MAX_BUFFER_SIZE]){
			buffer_ptr = &ring_buffer[0];
			first_circle_end = 1;
		}
	}
	if((tail_ptr == &ring_buffer[MAX_BUFFER_SIZE-1]) || (first_circle_end == 0)){
		head_ptr = &ring_buffer[0];
	}

	if((first_circle_end == 1)&&(tail_ptr != &ring_buffer[MAX_BUFFER_SIZE-1])){
		head_ptr = (tail_ptr + 1);
	}

	mutex_unlock(&ring_buffer_mutex);
}

static bool fd_auto_gain_control(ams_tcs3707_deviceCtx_t* ctx, uint16_t* buf_16_bit)
{
	UINT8 j = 0;
	UINT8 k = 0;
	UINT16 buf_16_bit_temp = 0;
	UINT16 buf_16_bit_max = 0;
	bool fd_saturation_check = false;
	bool fd_insufficience_check = false;
	fd_gain_adjusted = false;

	//sort the 10 buf[i] data, and find the max data
	for(j = 0; j < 9; j++){//min 9 val
		for(k = j+1; k < 10; k++){//max 10 val
			if(buf_16_bit[j] > buf_16_bit[k]){
				buf_16_bit_temp = buf_16_bit[j];
				buf_16_bit[j] = buf_16_bit[k];
				buf_16_bit[k] = buf_16_bit_temp;
			}
		}
	}
	buf_16_bit_max = buf_16_bit[9];//get max bit
	//first flicker detect data is special
	if(ctx->fd_data.first_fd_inte){
		/**
		 * init gain is 4x, init fd time is 1ms,
		 * so the saturated value is (ATIME+1)*(ASTEP+1)*80%= 288
		 * if gain changes to 32, also not saturated,
		 * so the threshold is 288/(32/4) = 36
		 * if gain changes to 128, also not saturated,
		 * so the threshold is 288/(128/4) = 9
		 * if gain changes to 512, also not saturated,
		 * so the threshold is 288/(512/4) = 2
		 */
		if (buf_16_bit_max <= BUF_16_MAX_LEVEL5) {
			//if min 9x gain auto lev5
			ams_tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_5);
		} else if (buf_16_bit_max <= BUF_16_MAX_LEVEL4) {
			//if min 18x gain auto lev4
			ams_tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_4);
		} else if (buf_16_bit_max <= BUF_16_MAX_LEVEL3) {
			//if min 36x gain auto lev3
			ams_tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_3);
		} else if (buf_16_bit_max <= BUF_16_MAX_LEVEL2) {
			hwlog_info("%s, keep the init 4x gain\n", __func__);
		} else {
			ams_tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_1);
		}

		hwlog_info("%s, come into first_fd_inte, gain = %d\n", __func__,
			ctx->fd_data.fd_gain);
		ctx->fd_data.first_fd_inte = false;
		fd_gain_adjusted = true;
		goto handle_fliker_exi;
	}

	fd_saturation_check = ams_tcs3707_fd_saturation_check(ctx, buf_16_bit_max);
	fd_insufficience_check = ams_tcs3707_fd_insufficience_check(ctx, buf_16_bit_max);

	/* Adjust gain setting for flicker detect */
	if (fd_saturation_check &&	(ctx->fd_data.fd_gain == FD_GAIN_LEVEL_2)){
		ams_tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_1);
		fd_gain_adjusted = true;
	}
	else if ((fd_saturation_check && (ctx->fd_data.fd_gain == FD_GAIN_LEVEL_3)) ||
			(fd_insufficience_check &&	(ctx->fd_data.fd_gain == FD_GAIN_LEVEL_1))){
		ams_tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_2);
		fd_gain_adjusted = true;
	}
	else if ((fd_saturation_check && (ctx->fd_data.fd_gain == FD_GAIN_LEVEL_4)) ||
			(fd_insufficience_check &&	(ctx->fd_data.fd_gain == FD_GAIN_LEVEL_2))){
		ams_tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_3);
		fd_gain_adjusted = true;
	}
	else if ((fd_saturation_check && (ctx->fd_data.fd_gain == FD_GAIN_LEVEL_5)) ||
			(fd_insufficience_check && (ctx->fd_data.fd_gain == FD_GAIN_LEVEL_3))){
		ams_tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_4);
		fd_gain_adjusted = true;
	}
	else if (fd_insufficience_check && (ctx->fd_data.fd_gain == FD_GAIN_LEVEL_4)){
		ams_tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_5);
		fd_gain_adjusted = true;
	}

handle_fliker_exi:
	return fd_gain_adjusted;
}

static void  read_fd_data(struct colorDriver_chip *chip)
{
	UINT8 fifo_level = 0;
	UINT8 ret = 0;
	UINT16 i = 0;
	UINT16 j = 0;
	UINT8 buf[MAX_BUFFER_SIZE] = {0};
	UINT16 fd_report_value[REPORT_FIFO_LEN] = {0};
	UINT16 buf_16_bit[REPORT_FIFO_LEN] = {0};
	bool ret_auto_gain = false;
	UINT8 fd_enable_status;

	UINT32 delta_time = 0;
	UINT32 delta_time_inside_sensor = 0;
	UINT8 delta_fifo_num = 0;

	static struct timeval last_time;
	static UINT8 fifo_level_last = 0;
	struct timeval curr_time;

	ams_tcs3707_deviceCtx_t* ctx = (ams_tcs3707_deviceCtx_t*)chip->deviceCtx;

	fd_enable_status = get_rgb_fd_enable_status(ctx->portHndl);

	ret = ams_tcs3707_getByte(ctx->portHndl,TCS3707_FIFO_STATUS,&fifo_level);

	if(ret > 0){
		if(!ctx->fd_data.first_fd_inte){ 
			if(fd_gain_adjusted == false){
				do_gettimeofday(&curr_time);
				delta_fifo_num = fifo_level - fifo_level_last;
				delta_time_inside_sensor =
					(uint32_t)delta_fifo_num *
					FD_TIME_RATIO;//unit is us
				delta_time =
					(curr_time.tv_sec - last_time.tv_sec) *
					FD_TIME_RATIO * FD_TIME_RATIO +
					(curr_time.tv_usec - last_time.tv_usec);

				//if upper flow should use the fd_ratio devided by 1000  
				//F-calibrated = F_fft /fd_ratio.    F_fft is the frequency calculated by the camera algorithm
				if(delta_time_inside_sensor > 0){
					fd_ratio = (delta_time * FD_RATIO_SCL) /
						delta_time_inside_sensor;
				}
			}
		}
	}

	if (fd_timer_stopped == false) {
		mod_timer(&chip->fd_timer, jiffies + msecs_to_jiffies(FD_POLLING_TIME));
	} else {
		hwlog_info("fd has been disabled, del fd work timer\n");
	}

	if(ret > 0){
		if(fifo_level > 0){
			if (fifo_level <= FIFO_LEVEL_16) {
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[0], fifo_level*FIFO_LEVEL_RATIO);
			} else if (fifo_level <= FIFO_LEVEL_32) {
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[0], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[32], fifo_level*FIFO_LEVEL_RATIO -TCS3707_GETBUF_LEN);
			} else if (fifo_level <= FIFO_LEVEL_48) {
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[0], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[32], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[64], fifo_level*FIFO_LEVEL_RATIO -64);
			} else if (fifo_level <= FIFO_LEVEL_64) {
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[0], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[32], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[64], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[96], fifo_level*FIFO_LEVEL_RATIO -96);
			} else if (fifo_level <= FIFO_LEVEL_80) {
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[0], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[32], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[64], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[96], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[128], fifo_level*FIFO_LEVEL_RATIO -128);
			} else if (fifo_level <= FIFO_LEVEL_96) {
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[0], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[32], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[64], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[96], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[128], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[160], fifo_level*FIFO_LEVEL_RATIO -160);
			} else if (fifo_level <= FIFO_LEVEL_112) {
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[0], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[32], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[64], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[96], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[128], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[160], TCS3707_GETBUF_LEN);
				ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[192], fifo_level*FIFO_LEVEL_RATIO -192);
			} else {
				ret = ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[0], TCS3707_GETBUF_LEN);
				ret = ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[32], TCS3707_GETBUF_LEN);
				ret = ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[64], TCS3707_GETBUF_LEN);
				ret = ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[96], TCS3707_GETBUF_LEN);
				ret = ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[128], TCS3707_GETBUF_LEN);
				ret = ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[160], TCS3707_GETBUF_LEN);
				ret = ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[192], TCS3707_GETBUF_LEN);
				ret = ams_tcs3707_getBuf(ctx->portHndl, TCS3707_FDADAL, &buf[224], fifo_level*FIFO_LEVEL_RATIO -224);
			}

			//add for flicker clock calibrate, get the fifo level after the fifo is read. also get the system time
			ret = ams_tcs3707_getByte(ctx->portHndl,TCS3707_FIFO_STATUS,&fifo_level_last);
			do_gettimeofday(&last_time);

			for(i = 0; i < fifo_level; i++){
				buf_16_bit[i] = (uint16_t)(buf[i*BUF_RATIO_8_16BIT])|(uint16_t)(buf[i*BUF_RATIO_8_16BIT+1] << 8);//high 8 bit for 16 bit
				//5 debug
				if(i < 5){
					hwlog_debug("buf[%d*2] = %d , buf[%d*2+1] = %d\n", i,buf[i*BUF_RATIO_8_16BIT],i,buf[i*BUF_RATIO_8_16BIT+1]);
				}
			}

			//normalize to the flicker golden gain value
			for(j = 0; j < fifo_level; j++){
				fd_report_value[j] = (buf_16_bit[j]*AMS_TCS3707_FD_GAIN_OF_GOLDEN)/ctx->fd_data.fd_gain;
				//5 debug
				if(j < 5){
					hwlog_debug("buf_16_bit[%d] = %d,fd_report_value[%d] = %d, ctx->fd_data.fd_gain = %d \n",
						j,buf_16_bit[j],j,fd_report_value[j],ctx->fd_data.fd_gain);
				}
			}
		}
		ret_auto_gain = fd_auto_gain_control(ctx,buf_16_bit);
		if(ret_auto_gain == false){
			//to forbid the case when the sensor is closed,
			//but still process the data
			if ((fd_enable_status & FDEN) == FDEN) {
				ring_buffer_process(fifo_level,fd_report_value);
			}
		}else if(ret_auto_gain == true){//if fd_gain is adjusted, clear the ringbuffer and reset the related pointer.
			mutex_lock(&ring_buffer_mutex);
			memset(ring_buffer, 0, MAX_BUFFER_SIZE*BUF_RATIO_8_16BIT);//clear ring_buffer when close the fd
			buffer_ptr = &ring_buffer[0];
			head_ptr = &ring_buffer[0];
			tail_ptr = &ring_buffer[0];
			mutex_unlock(&ring_buffer_mutex);
		}
	}
}


static void ams_fd_work(struct work_struct *work)
{
	struct colorDriver_chip *chip = NULL;
	if(NULL == work){
		hwlog_err("AMS_Driver: %s: Pointer work is NULL\n", __func__);
		return;
	}
	chip = container_of(work, struct colorDriver_chip, fd_work);
	if(NULL == chip){
		hwlog_err("AMS_Driver: %s: Pointer chip is NULL\n", __func__);
		return;
	}
	
	AMS_MUTEX_LOCK(&chip->lock);
	read_fd_data(chip); 
	AMS_MUTEX_UNLOCK(&chip->lock);
}

#endif

#ifdef CONFIG_HUAWEI_DSM
static void amsdriver_dmd_work(struct work_struct *work)
{
	if (!dsm_client_ocuppy(shb_dclient)) {
		if (color_devcheck_dmd_result == false){
			dsm_client_record(shb_dclient, "ap_color_sensor_detected fail\n");
			dsm_client_notify(shb_dclient, DSM_AP_ERR_COLORSENSOR_DETECT);
			hwlog_err("AMS_Driver: %s: DMD ap_color_sensor_detected fail\n", __func__);
		}
	}
}
#endif

static void ams_tcs3707_get_fd_data(struct colorDriver_chip *chip, char *ams_fd_data)
{
	if((NULL == chip) || (NULL == ams_fd_data)){
		hwlog_err("\nAMS_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}

	mutex_lock(&ring_buffer_mutex);	
	get_read_out_buffer();
	memcpy(ams_fd_data, read_out_buffer, sizeof(read_out_buffer));
	mutex_unlock(&ring_buffer_mutex);
}

int ams_tcs3707_probe(struct i2c_client *client,
	const struct i2c_device_id *idp)
{
	int ret = 0;
	int i = 0;
	int rc;
	struct device *dev = NULL;
	static struct colorDriver_chip *chip = NULL;
	struct driver_i2c_platform_data *pdata = NULL;
	struct device_node *dev_node = NULL;
	ams_tcs3707_deviceInfo_t amsDeviceInfo;
	ams_tcs3707_deviceIdentifier_e deviceId;

	hwlog_info("enter [%s] \n", __func__);
	if(NULL == client){
		hwlog_err("ams_tcs3707: %s: Pointer is NULL\n", __func__);
		return -1;
	}
	dev = &client->dev;
	pdata = dev->platform_data;
	dev_node = dev->of_node;

	/****************************************/
	/* Validate bus and device registration */
	/****************************************/
	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA)) {
		hwlog_err("%s: i2c smbus byte data unsupported\n", __func__);
		ret = -EOPNOTSUPP;
		goto init_failed;
	}

	chip = kzalloc(sizeof(struct colorDriver_chip), GFP_KERNEL);
	if (!chip) {
		ret = -ENOMEM;
		goto malloc_failed;
	}

	mutex_init(&chip->lock);
	chip->client = client;
	chip->pdata = pdata;
	i2c_set_clientdata(chip->client, chip);

	chip->in_suspend = 0;
	chip->inCalMode = false;
	chip->calibrationCtx.calState = 0;

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++){
		chip->calibrationCtx.calCresult[i]  = AMS_TCS3707_FLOAT_TO_FIX ;
		chip->calibrationCtx.calRresult[i]  = AMS_TCS3707_FLOAT_TO_FIX ;
		chip->calibrationCtx.calGresult[i]  = AMS_TCS3707_FLOAT_TO_FIX ;
		chip->calibrationCtx.calBresult[i] = AMS_TCS3707_FLOAT_TO_FIX ;
		chip->calibrationCtx.calWresult[i] = AMS_TCS3707_FLOAT_TO_FIX ;
		color_nv_para.calCratio[i] = AMS_TCS3707_FLOAT_TO_FIX ;
		color_nv_para.calRratio[i]  = AMS_TCS3707_FLOAT_TO_FIX ;
		color_nv_para.calGratio[i]  = AMS_TCS3707_FLOAT_TO_FIX ;
		color_nv_para.calBratio[i] = AMS_TCS3707_FLOAT_TO_FIX ;
		color_nv_para.calWratio[i] = AMS_TCS3707_FLOAT_TO_FIX ;
	}

#ifdef CONFIG_HUAWEI_DSM
	INIT_DELAYED_WORK(&ams_tcs3707_dmd_work, amsdriver_dmd_work);
#endif
	/********************************************************************/
	/* Validate the appropriate ams device is available for this driver */
	/********************************************************************/
	deviceId = ams_tcs3707_testForDevice(chip->client);

	hwlog_info("ams_tcs3707: ams_tcs3707_testForDevice() %d \n", deviceId);

	if (deviceId == AMS_UNKNOWN_DEVICE) {
		hwlog_info( "ams_tcs3707_testForDevice failed: AMS_UNKNOWN_DEVICE\n");
#ifdef CONFIG_HUAWEI_DSM
		color_devcheck_dmd_result = false;
		schedule_delayed_work(&ams_tcs3707_dmd_work, msecs_to_jiffies(AP_COLOR_DMD_DELAY_TIME_MS));
#endif
		goto id_failed;
	}

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_AP_COLOR_SENSOR);
#endif
	ams_tcs3707_getDeviceInfo(&amsDeviceInfo);
	hwlog_info( "ams_amsDeviceInfo() ok\n");
	rc = of_property_read_u32(dev_node, "flicker_support", &g_flicker_support);
	if (rc < 0) {
		hwlog_warn("%s, get g_flicker_supportfailed\n", __func__);
		g_flicker_support = 1; // default support flk
	}
	hwlog_info("%s g_flicker_support = %d\n", __func__, g_flicker_support);

	chip->deviceCtx = kzalloc(amsDeviceInfo.memorySize, GFP_KERNEL);

	if (chip->deviceCtx) {
		((ams_tcs3707_deviceCtx_t *)(chip->deviceCtx))->portHndl =
			chip->client;
		ams_tcs3707_resetAllRegisters(chip->client);
		ams_tcs3707_set_enable(chip->client,
			TCS3707_ENABLE_REG,
			PON,
			PON);
		hwlog_info("%s, init set PON\n", __func__);
		msleep(3); //delay for mode set ready
		ams_tcs3707_setField(chip->client,
			TCS3707_CONTROL_REG,
			ALS_MANUAL_AZ,
			ALS_MANUAL_AZ);
		if (ret == false){
			hwlog_info( "ams_amsDeviceInit() ok\n");
		} else {
			hwlog_info( "ams_deviceInit failed.\n");
			goto id_failed;
		}
	} else {
		hwlog_info( "ams_tcs3707 kzalloc failed.\n");
		goto id_failed;
	}

	/*********************/
	/* Initialize ALS    */
	/*********************/

#ifdef CONFIG_AMS_OPTICAL_SENSOR_ALS
	/* setup */
	hwlog_info( "Setup for ALS\n");
#endif

	init_timer(&chip->work_timer);
	setup_timer(&chip->work_timer, osal_tcs3707_als_timerHndl, (unsigned long) chip);
	INIT_WORK(&chip->als_work, amsdriver_work);

	chip->at_color_show_calibrate_state = ams_tcs3707_show_calibrate;
	chip->at_color_store_calibrate_state = ams_tcs3707_store_calibrate;
	chip->color_enable_show_state = ams_tcs3707_show_enable;
	chip->color_enable_store_state = ams_tcs3707_store_enable;
	chip->color_sensor_getGain = ams_tcs3707_getGain;
	chip->color_sensor_setGain = ams_tcs3707_setGain;
	chip->get_flicker_data = ams_tcs3707_get_fd_data;
	chip->flicker_enable_show_state = ams_tcs3707_fd_show_enable;
	chip->flicker_enable_store_state = ams_tcs3707_fd_store_enable;
    chip->color_report_type = ams_tcs3707_rgb_report_type;
 
#ifdef CONFIG_AMS_FLICKER_DETECT
	//Flicker Detect timer and work 
	init_timer(&chip->fd_timer);
	setup_timer(&chip->fd_timer, osal_fd_timerHndl, (unsigned long) chip);
	INIT_WORK(&chip->fd_work, ams_fd_work);	
#endif
	
	p_chip = chip;
	ret = color_register(chip);
	if(ret < 0){
		hwlog_err("ams_tcs3707: color_register fail \n");
	}
	color_default_enable = ams_tcs3707_setenable;

	hwlog_info("[%s]ams_tcs3707 probe ok \n", __func__);
	return 0;

	/********************************************************************************/
	/* Exit points for general device initialization failures                       */
	/********************************************************************************/

id_failed:
	if (chip->deviceCtx) kfree(chip->deviceCtx);
	i2c_set_clientdata(client, NULL);
malloc_failed:
	kfree(chip);

init_failed:
	dev_err(dev, "Probe failed.\n");
	return ret;
}

int ams_tcs3707_suspend(struct device *dev)
{
	struct colorDriver_chip  *chip = NULL;

	if(NULL == dev){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if(NULL == chip){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return -1;
	}

	hwlog_info( "%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	chip->in_suspend = 1;

	if (chip->wake_irq) {
		irq_set_irq_wake(chip->client->irq, 1);
	} else if (!chip->unpowered) {
		hwlog_info( "powering off\n");
		/* TODO
		   platform power off */
	}
	AMS_MUTEX_UNLOCK(&chip->lock);

	return 0;
}

int ams_tcs3707_resume(struct device *dev)
{
	struct colorDriver_chip *chip = NULL;

	if(NULL == dev){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if(NULL == chip){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return -1;
	}

	return 0;
	AMS_MUTEX_LOCK(&chip->lock);

	chip->in_suspend = 0;

	if (chip->wake_irq) {
		irq_set_irq_wake(chip->client->irq, 0);
		chip->wake_irq = 0;
	}

/* err_power: */
	AMS_MUTEX_UNLOCK(&chip->lock);

	return 0;
}

int ams_tcs3707_remove(struct i2c_client *client)
{
	struct colorDriver_chip *chip = NULL;

	if(NULL == client){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return -1;
	}
	chip = i2c_get_clientdata(client);
	if(NULL == chip){
		hwlog_err("AMS_Driver: %s: Pointer is NULL\n", __func__);
		return -1;
	}

	free_irq(client->irq, chip);
	/* TODO
	   platform teardown */
	i2c_set_clientdata(client, NULL);
	kfree(chip->deviceCtx);
	kfree(chip);
	return 0;
}

static struct i2c_device_id amsdriver_idtable[] = {
	{"ams_tcs3707", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, amsdriver_idtable);

static const struct dev_pm_ops ams_tcs3707_pm_ops = {
	.suspend = ams_tcs3707_suspend,
	.resume  = ams_tcs3707_resume,
};

static const struct of_device_id amsdriver_of_id_table[] = {
	{.compatible = "ams,tcs3707"},
	{},
};


static struct i2c_driver ams_tcs3707_driver = {
	.driver = {
		.name = "ams_tcs3707",
		.owner = THIS_MODULE,
		.of_match_table = amsdriver_of_id_table,
	},
	.id_table = amsdriver_idtable,
	.probe = ams_tcs3707_probe,
	.remove = ams_tcs3707_remove,
};

static int __init ams_tcs3707_init(void)
{
	int rc;
	hwlog_info("ams_tcs3707: init()\n");

	rc = i2c_add_driver(&ams_tcs3707_driver);

	printk(KERN_ERR "ams_tcs3707:  %d", rc);
	return rc;
}

static void __exit ams_tcs3707_exit(void)
{
	hwlog_info("ams_tcs3707: exit()\n");
	i2c_del_driver(&ams_tcs3707_driver);
}

module_init(ams_tcs3707_init);
module_exit(ams_tcs3707_exit);

MODULE_AUTHOR("AMS AOS Software<cs.americas@ams.com>");
MODULE_DESCRIPTION("AMS tcs3707 ALS, XYZ color sensor driver");
MODULE_LICENSE("GPL");
