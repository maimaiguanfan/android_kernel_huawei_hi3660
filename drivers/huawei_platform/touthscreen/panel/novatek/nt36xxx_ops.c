/*
 * Copyright (C) 2010 - 2016 Novatek, Inc.
 *
 * $Revision: 4301 $
 * $Date: 2016-04-22 17:28:06 +0800 (星期五, 22 四月 2016) $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */
	 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/proc_fs.h>
#include <linux/unistd.h>

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <../../huawei_touchscreen_chips.h>
#include <linux/regulator/consumer.h>
#include <huawei_platform/log/log_jank.h>
#include "../../huawei_touchscreen_algo.h"
#if defined (CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>

extern struct dsm_client *tp_dclient;
#endif

#include "nt36xxx.h"

struct nvt_ts_data *ts;
static DEFINE_MUTEX(ts_power_gpio_sem);
extern struct ts_data g_ts_data;

char novatek_project_id[PROJECT_ID_LEN+1]={"999999999"};
char novatek_product_id[PROJECT_ID_LEN+1]={"999999999"};
struct nvt_lcd_data {
	size_t size;
	u8 *data;
};
struct nvt_lcd_data nvt_lcd_data_entry;
struct nvt_lcd_data *lcd_data_entry = &nvt_lcd_data_entry;

#if NVT_TOUCH_PROC
extern int32_t nvt_flash_proc_init(void);
#endif

#if NVT_TOUCH_EXT_PROC
extern int32_t nvt_extra_proc_init(void);
#endif

extern int32_t Init_BootLoader(void);
extern int32_t Resume_PD(void);
extern int32_t nvt_fw_update_boot(char *file_name);
extern int32_t nvt_fw_update_sd(void);
extern int32_t nvt_get_rawdata(struct ts_rawdata_info *info);
extern int8_t nvt_get_fw_info(void);
extern int32_t novatek_read_projectid(void);
extern int32_t nvt_selftest(struct ts_rawdata_info * info);

extern int8_t fw_ver;
extern int g_sd_force_update;

#if TOUCH_KEY_NUM > 0
const uint16_t touch_key_array[TOUCH_KEY_NUM] = {
    KEY_BACK,
    KEY_HOME,
    KEY_MENU
};
#endif

#ifdef ROI
static u8 roi_switch = 0;
static u8 pre_finger_status = 0;
static unsigned char roi_data[ROI_DATA_READ_LENGTH+1] = {0};
#endif


/*******************************************************
Description:
	Novatek touchscreen i2c read function.

return:
	Executive outcomes. 2---succeed. -5---I/O error
*******************************************************/
int32_t novatek_ts_i2c_read(struct i2c_client *client, uint16_t i2c_addr, uint8_t *buf, uint16_t len)
{
	int ret = -1;
	u16 tmp_addr = 0;

	TS_LOG_DEBUG("%s: i2c_addr=0x%02X, len=%d\n", __func__, (uint8_t)i2c_addr, (uint8_t)len);

	if (!ts->chip_data->bops->bus_read) {
		TS_LOG_ERR("%s: error, invalid bus_read\n", __func__);
		ret = -EIO;
		goto i2c_err;
	}

	mutex_lock(&ts->i2c_mutex);
	tmp_addr = g_ts_data.client->addr;
	g_ts_data.client->addr = i2c_addr;

	ret = ts->chip_data->bops->bus_read(&buf[0], 1, &buf[1], (len - 1));
	if (ret < 0)
		TS_LOG_ERR("%s: error, bus_read fail, ret=%d\n", __func__, ret);

	g_ts_data.client->addr = tmp_addr;
	mutex_unlock(&ts->i2c_mutex);

i2c_err:
	return ret;
}

/*******************************************************
Description:
	Novatek touchscreen i2c dummy read function.

return:
	Executive outcomes. 1---succeed. -5---I/O error
*******************************************************/
int32_t novatek_ts_i2c_dummy_read(struct i2c_client *client, uint16_t i2c_addr)
{
	int ret = -1;
	u16 tmp_addr = 0;
	uint8_t buf[8] = {0};

	TS_LOG_DEBUG("%s: i2c_addr=0x%02X\n", __func__, (uint8_t)i2c_addr);

	if (!ts->chip_data->bops->bus_read) {
		TS_LOG_ERR("%s: error, invalid bus_read\n", __func__);
		ret = -EIO;
		goto i2c_err;
	}

	mutex_lock(&ts->i2c_mutex);
	tmp_addr = g_ts_data.client->addr;
	g_ts_data.client->addr = i2c_addr;

	ret = ts->chip_data->bops->bus_read(&buf[0], 1, &buf[1], 1);
	if (ret < 0)
		TS_LOG_ERR("%s: error, bus_read fail, ret=%d\n", __func__, ret);

	g_ts_data.client->addr = tmp_addr;
	mutex_unlock(&ts->i2c_mutex);

i2c_err:
	return ret;
}

/*******************************************************
Description:
	Novatek touchscreen i2c write function.

return:
	Executive outcomes. 1---succeed. -5---I/O error
*******************************************************/
int32_t novatek_ts_i2c_write(struct i2c_client *client, uint16_t i2c_addr, uint8_t *buf, uint16_t len)
{
	int ret = -1;
	u16 tmp_addr = 0;

	TS_LOG_DEBUG("%s: i2c_addr=0x%02X, len=%d\n", __func__, (uint8_t)i2c_addr, (uint8_t)len);

	if (!ts->chip_data->bops->bus_read) {
		TS_LOG_ERR("%s: error, invalid bus_write\n", __func__);
		ret = -EIO;
		goto i2c_err;
	}

	mutex_lock(&ts->i2c_mutex);
	tmp_addr = g_ts_data.client->addr;
	g_ts_data.client->addr = i2c_addr;

	ret = ts->chip_data->bops->bus_write(&buf[0], len);
	if (ret < 0)
		TS_LOG_ERR("%s: error, bus_write fail, ret=%d\n", __func__, ret);

	g_ts_data.client->addr = tmp_addr;
	mutex_unlock(&ts->i2c_mutex);

i2c_err:
	return ret;
}

/*******************************************************
Description:
	Novatek touchscreen set i2c debounce function.

return:
	n.a.
*******************************************************/
void nvt_set_i2c_debounce(void)
{
	uint8_t buf[8] = {0};
	uint8_t reg1_val = 0;
	uint8_t reg2_val = 0;
	uint32_t retry = 0;

	do {
		msleep(10);

		//---dummy read to resume TP before writing command---
		novatek_ts_i2c_dummy_read(ts->client, I2C_BLDR_Address);

		// set xdata index to 0x1F000
		buf[0] = 0xFF;
		buf[1] = 0x01;
		buf[2] = 0xF0;
		novatek_ts_i2c_write(ts->client, I2C_BLDR_Address, buf, 3);

		// REGW 0x36 @0x1F020
		buf[0] = 0x20;
		buf[1] = 0x36;
		novatek_ts_i2c_write(ts->client, I2C_BLDR_Address, buf, 2);

		buf[0] = 0x20;
		buf[1] = 0x00;
		novatek_ts_i2c_read(ts->client, I2C_BLDR_Address, buf, 2);
		reg1_val = buf[1];
	} while ((reg1_val != 0x36) && (retry++ < 20));

	if(retry == 20) {
		TS_LOG_ERR("%s: set i2c debounce failed, reg1_val=0x%02X, reg2_val=0x%02X\n", __func__, reg1_val, reg2_val);
	}
}

/*******************************************************
Description:
	Novatek touchscreen reset MCU then into idle mode
    function.

return:
	n.a.
*******************************************************/
void nvt_sw_reset_idle(void)
{
	uint8_t buf[4]={0};

	//---write i2c cmds to reset idle---
	buf[0]=0x00;
	buf[1]=0xA5;
	novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 2);

	msleep(5);

	buf[0]=0x00;
	buf[1]=0xA5;
	novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 2);

	msleep(10);

	nvt_set_i2c_debounce();

	//---for debug---
	// Initiate Flash Block
	buf[0] = 0x00;
	buf[1] = 0x00;
	buf[2] = I2C_FW_Address;
	novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 2);
	msleep(5);

	// Check 0xAA (Initiate Flash Block)
	buf[0] = 0x00;
	buf[1] = 0x00;
	novatek_ts_i2c_read(ts->client, I2C_HW_Address, buf, 2);
	TS_LOG_INFO("%s: status=0x%02X\n", __func__, buf[1]);

	//---write i2c cmds to Mode flag---
	buf[0] = 0xFF;
	buf[1] = 0x01;
	buf[2] = 0xF6;
	novatek_ts_i2c_write(ts->client, I2C_BLDR_Address, buf, 3);

	//---read Mode flag---
	buf[0] = 0x18;
	novatek_ts_i2c_read(ts->client, I2C_BLDR_Address, buf, 2);
	TS_LOG_INFO("%s: mode_fg=0x%02X\n", __func__, buf[1]);

}

/*******************************************************
Description:
	Novatek touchscreen reset MCU (no boot) function.

return:
	n.a.
*******************************************************/
void nvt_sw_reset(void)
{
	uint8_t buf[8] = {0};

	//---write i2c cmds to reset---
	buf[0] = 0x00;
	buf[1] = 0x5A;
	novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 2);

	// need 5ms delay after sw reset
	msleep(5);
}

/*******************************************************
Description:
	Novatek touchscreen reset MCU (boot) function.

return:
	n.a.
*******************************************************/
void nvt_bootloader_reset(void)
{
	uint8_t buf[8] = {0};

	//---write i2c cmds to reset---
	buf[0] = 0x00;
	buf[1] = 0x69;
	novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 2);

	// need 5ms delay after bootloader reset
	msleep(5);
}

/*******************************************************
Description:
	Novatek touchscreen IC hardware reset function.

return:
	n.a.
*******************************************************/
void nvt_hw_reset(void)
{
	TS_LOG_INFO("%s enter\n", __func__);

	//---trigger rst-pin to reset (pull low for 10ms)---
	//gpio_direction_output(ts->chip_data->reset_gpio, 1);
	//mdelay(5);
	gpio_direction_output(ts->chip_data->reset_gpio, 0);
	mdelay(10);
	gpio_direction_output(ts->chip_data->reset_gpio, 1);
	mdelay(5);
}

void nvt_hw_reset_down(void)
{
	TS_LOG_INFO("%s enter\n", __func__);

	mdelay(5);
	gpio_direction_output(ts->chip_data->reset_gpio, 0);
}

/*******************************************************
Description:
	Novatek touchscreen clear FW status function.

return:
	Executive outcomes. 0---succeed. -1---fail.
*******************************************************/
int32_t nvt_clear_fw_status(void)
{
	uint8_t buf[8] = {0};
	int32_t i = 0;
	const int32_t retry = 10;

	for (i = 0; i < retry; i++) {
		//---set xdata index to 0x11E00---
		buf[0] = 0xFF;
		buf[1] = 0x01;
		buf[2] = 0x1E;
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);

		//---clear fw status---
		buf[0] = 0x51;
		buf[1] = 0x00;
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);

		//---read fw status---
		buf[0] = 0x51;
		buf[1] = 0xFF;
		novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 2);

		if (buf[1] == 0x00)
			break;

		msleep(10);
	}

	if (i >= retry)
		return -1;
	else
		return 0;
}

/*******************************************************
Description:
	Novatek touchscreen check FW status function.

return:
	Executive outcomes. 0---succeed. -1---failed.
*******************************************************/
int32_t nvt_check_fw_status(void)
{
	uint8_t buf[8] = {0};
	int32_t i = 0;
	const int32_t retry = 10;

	for (i = 0; i < retry; i++) {
		//---set xdata index to 0x11E00---
		buf[0] = 0xFF;
		buf[1] = 0x01;
		buf[2] = 0x1E;
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);

		//---read fw status---
		buf[0] = 0x51;
		buf[1] = 0x00;
		novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 2);

		if ((buf[1] & 0xF0) == 0xA0)
			break;

		msleep(10);
	}

	if (i >= retry)
		return -1;
	else
		return 0;
}

/*******************************************************
Description:
	Novatek touchscreen check FW reset state function.

return:
	Executive outcomes. 0---succeed. -1---failed.
*******************************************************/
int32_t nvt_check_fw_reset_state(RST_COMPLETE_STATE check_reset_state)
{
	uint8_t buf[8] = {0};
	int32_t ret = 0;
	int32_t retry = 0;

	while (1) {
		msleep(10);

		//---read reset state---
		buf[0] = 0x60;
		buf[1] = 0x00;
		novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 2);

		if ((buf[1] >= check_reset_state) && (buf[1] < 0xFF)) {
			ret = 0;
			break;
		}

		retry++;
		if(unlikely(retry > 200)) {
			ret = -1;
			TS_LOG_ERR("%s: error, retry=%d, buf[1]=0x%02X\n", __func__, retry, buf[1]);
			break;
		}
	}

	return ret;
}

#if 0
/*******************************************************
Description:
	Novatek touchscreen check chip version trim function.

return:
	Executive outcomes. 0---NVT IC. -1---not NVT IC.
*******************************************************/
static int8_t nvt_ts_check_chip_ver_trim(void)
{
	uint8_t buf[8] = {0};
	int32_t retry = 0;
	int32_t ret = -1;

	//---Check for 5 times---
	for (retry = 5; retry > 0; retry--) {
		nvt_sw_reset_idle();

		buf[0] = 0x00;
		buf[1] = 0x35;
		novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 2);
		msleep(10);

		buf[0] = 0xFF;
		buf[1] = 0x01;
		buf[2] = 0xF6;
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);

		buf[0] = 0x4E;
		buf[1] = 0x00;
		buf[2] = 0x00;
		buf[3] = 0x00;
		buf[4] = 0x00;
		buf[5] = 0x00;
		buf[6] = 0x00;
		novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 7);

		if ((buf[1] == 0x55) && (buf[2] == 0x00) && (buf[4] == 0x00) && (buf[5] == 0x00) && (buf[6] == 0x00)) {
			TS_LOG_INFO("%s: this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else if ((buf[1] == 0x55) && (buf[2] == 0x72) && (buf[4] == 0x00) && (buf[5] == 0x00) && (buf[6] == 0x00)) {
			TS_LOG_INFO("%s: this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else if ((buf[1] == 0xAA) && (buf[2] == 0x00) && (buf[4] == 0x00) && (buf[5] == 0x00) && (buf[6] == 0x00)) {
			TS_LOG_INFO("%s: this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else if ((buf[1] == 0xAA) && (buf[2] == 0x72) && (buf[4] == 0x00) && (buf[5] == 0x00) && (buf[6] == 0x00)) {
			TS_LOG_INFO("%s: this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else if ((buf[4] == 0x72) && (buf[5] == 0x67) && (buf[6] == 0x03)) {
			TS_LOG_INFO("%s: this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else if ((buf[4] == 0x70) && (buf[5] == 0x66) && (buf[6] == 0x03)) {
			TS_LOG_INFO("%s: this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else if ((buf[4] == 0x70) && (buf[5] == 0x67) && (buf[6] == 0x03)) {
			TS_LOG_INFO("%s: this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else if ((buf[4] == 0x72) && (buf[5] == 0x66) && (buf[6] == 0x03)) {
			TS_LOG_INFO("%s: this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else {
			TS_LOG_INFO("%s: buf[1]=0x%02X, buf[2]=0x%02X, buf[3]=0x%02X, buf[4]=0x%02X, buf[5]=0x%02X, buf[6]=0x%02X\n",
				__func__, buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
			ret = -1;
		}

		msleep(10);
	}

	nvt_sw_reset();
	return ret;
}
#endif
static int novatek_get_capacitance_test_type(struct ts_test_type_info *info)
{
	TS_LOG_INFO("%s enter\n", __func__);
	if (!info){
		TS_LOG_ERR("%s\n", __func__);
		return -1;
	}

	strncpy(info->tp_test_type,
		"Normalize_type:judge_different_reslut",
		TS_CAP_TEST_TYPE_LEN);
	return NO_ERR;
}
#if 0
static int novatek_wakeup_gesture_enable_switch(struct
						  ts_wakeup_gesture_enable_info
						  *info)
{
	TS_LOG_INFO("%s enter\n", __func__);
	
//---Taylor add : for debug begin---
return NO_ERR;
//---Taylor add : for debug end---
}

static int novatek_holster_switch(struct ts_holster_info *info)
{
	TS_LOG_INFO("%s enter\n", __func__);

//---Taylor add : for debug begin---
return NO_ERR;
//---Taylor add : for debug end---
}
#endif

#define GLOVE_SWITCH_ON 1
#define GLOVE_SWITCH_OFF 0
static int novatek_glove_switch(struct ts_glove_info *info)
{
	uint8_t buf[4] = {0};
	int retval = NO_ERR;
	u8 sw = 0;

	TS_LOG_INFO("%s enter\n", __func__);
	
	if (!info) {
		TS_LOG_ERR("%s: info is Null\n", __func__);
		retval = -ENOMEM;
		return retval;
	}
	
	switch (info->op_action) {
		case TS_ACTION_READ:			
			buf[0] = 0x5A;
			retval = novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 2);
			if (retval < 0) {
				TS_LOG_ERR("%s: get glove_switch(%d), failed : %d", __func__, info->glove_switch, retval);
				break;
			}
			info->glove_switch =  ((buf[1]>>1) & 0x01);//buf[1] & 0x02;			
			TS_LOG_INFO("%s: read glove_switch=%d, 1:on 0:off\n", __func__, info->glove_switch);
			break;
			
		case TS_ACTION_WRITE:
			TS_LOG_INFO("%s: write glove_switch=%d\n", __func__, info->glove_switch);
		
			sw = info->glove_switch;			
			if ((GLOVE_SWITCH_ON != sw)
			    && (GLOVE_SWITCH_OFF != sw)) {
				TS_LOG_ERR("%s: write wrong state: switch = %d\n", __func__, sw);
				retval = -EFAULT;
				break;
			}

			if(GLOVE_SWITCH_ON == sw)	{		
				//---enable glove mode---
				buf[0] = 0x50;
				buf[1] = 0x71;
				retval = novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);
				if (retval < 0) {
					TS_LOG_ERR("%s: set glove switch(%d), failed : %d", __func__, sw, retval);
				}
			}
			else {
				//---disable glove mode---
				buf[0] = 0x50;
				buf[1] = 0x72;
				retval = novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);
				if (retval < 0) {
					TS_LOG_ERR("%s: set glove switch(%d), failed : %d", __func__, sw, retval);
				}
			}			
			break;
			
		default:
			TS_LOG_ERR("%s: invalid switch status: %d", __func__, info->glove_switch);
			retval = -EINVAL;
			break;
	}

	return retval;
}

#define PALM_SWITCH_ON 1
#define PALM_SWITCH_OFF 0
static int novatek_palm_switch(struct ts_palm_info *info)
{
	uint8_t buf[4] = {0};
	int retval = NO_ERR;
	u8 sw = 0;

	TS_LOG_INFO("%s enter\n", __func__);
	
	if (!info) {
		TS_LOG_ERR("%s: info is Null\n", __func__);
		retval = -ENOMEM;
		return retval;
	}
	
	switch (info->op_action) {
		case TS_ACTION_READ:			
			buf[0] = 0x5A;
			retval = novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 2);
			if (retval < 0) {
				TS_LOG_ERR("%s: get palm_switch(%d), failed : %d", __func__, info->palm_switch, retval);
				break;
			}
			info->palm_switch =  (buf[1] & 0x01);//buf[1] & 0x01;			
			TS_LOG_INFO("%s: read palm_switch=%d, 1:on 0:off\n", __func__, info->palm_switch);
			break;
			
		case TS_ACTION_WRITE:
			TS_LOG_INFO("%s: write palm_switch=%d\n", __func__, info->palm_switch);
			
			sw = info->palm_switch;			
			if ((PALM_SWITCH_ON != sw)
			    && (PALM_SWITCH_OFF != sw)) {
				TS_LOG_ERR("%s: write wrong state: switch = %d\n", __func__, sw);
				retval = -EFAULT;
				break;
			}

			if(PALM_SWITCH_ON == sw)	{		
				//---enable palm mode---
				buf[0] = 0x50;
				buf[1] = 0x73;
				retval = novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);
				if (retval < 0) {
					TS_LOG_ERR("%s: set palm switch(%d), failed : %d", __func__, sw, retval);
				}
			}
			else {
				//---disable palm mode---
				buf[0] = 0x50;
				buf[1] = 0x74;
				retval = novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);
				if (retval < 0) {
					TS_LOG_ERR("%s: set palm switch(%d), failed : %d", __func__, sw, retval);
				}
			}			
			break;
			
		default:
			TS_LOG_ERR("%s: invalid switch status: %d", __func__, info->palm_switch);
			retval = -EINVAL;
			break;
	}

	return retval;
}

#define HOLSTER_SWITCH_ON 1
#define HOLSTER_SWITCH_OFF 0
static int novatek_holster_switch(struct ts_holster_info *info)
{
	uint8_t buf[12] = {0};
	int retval = NO_ERR;
	u8 sw = 0;
	int x0 = g_ts_data.feature_info.window_info.top_left_x0;
	int y0 = g_ts_data.feature_info.window_info.top_left_y0;
	int x1 = g_ts_data.feature_info.window_info.bottom_right_x1;
	int y1 = g_ts_data.feature_info.window_info.bottom_right_y1;

	TS_LOG_INFO("%s enter\n", __func__);
	
	if (!info) {
		TS_LOG_ERR("%s: info is Null\n", __func__);
		retval = -ENOMEM;
		return retval;
	}
	
	switch (info->op_action) {
		case TS_ACTION_READ:			
			buf[0] = 0x5A;
			retval = novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 2);
			if (retval < 0) {
				TS_LOG_ERR("%s: get holster_switch(%d), failed : %d", __func__, info->holster_switch, retval);
				break;
			}
			info->holster_switch =  ((buf[1]>>2) & 0x01);//buf[1] & 0x04;			
			TS_LOG_INFO("%s: read holster_switch=%d, 1:on 0:off\n", __func__, info->holster_switch);
			break;
			
		case TS_ACTION_WRITE:
			TS_LOG_INFO("%s: write holster_switch=%d, x0=%d, y0=%d, x1=%d, y1=%d\n",
				__func__, info->holster_switch, x0, y0, x1, y1);			

			sw = info->holster_switch;		
			if ((HOLSTER_SWITCH_ON != sw)
			    && (HOLSTER_SWITCH_OFF != sw)) {
				TS_LOG_ERR("%s: write wrong state: switch = %d\n", __func__, sw);
				retval = -EFAULT;
				break;
			}
			
			if(HOLSTER_SWITCH_ON == sw)	{		
				//---enable holster mode & set window---
				buf[0] = 0x50;
				buf[1] = 0x75;
				buf[2] = (uint8_t)(x0 & 0xFF);
				buf[3] = (uint8_t)((x0 >> 8) & 0xFF);
				buf[4] = (uint8_t)(y0 & 0xFF);
				buf[5] = (uint8_t)((y0 >> 8) & 0xFF);
				buf[6] = (uint8_t)(x1 & 0xFF);
				buf[7] = (uint8_t)((x1 >> 8) & 0xFF);
				buf[8] = (uint8_t)(y1 & 0xFF);
				buf[9] = (uint8_t)((y1 >> 8) & 0xFF);
				retval = novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 10);
				if (retval < 0) {
					TS_LOG_ERR("%s: set holster switch(%d), failed : %d", __func__, sw, retval);
				}			
			}
			else {
				//---disable holster mode---
				buf[0] = 0x50;
				buf[1] = 0x76;
				retval = novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);
				if (retval < 0) {
					TS_LOG_ERR("%s: set holster switch(%d), failed : %d", __func__, sw, retval);
				}
			}			
			break;
			
		default:
			TS_LOG_ERR("%s: invalid switch status: %d", __func__, info->holster_switch);
			retval = -EINVAL;
			break;
	}

	return retval;
}

#define ROI_SWITCH_ON 1
#define ROI_SWITCH_OFF 0
static int novatek_roi_switch(struct ts_roi_info *info)
{
#ifdef ROI
	uint8_t buf[4] = {0};
	int retval = NO_ERR;
	u8 sw = 0;

	TS_LOG_INFO("%s enter\n", __func__);
	
	if (!info) {
		TS_LOG_ERR("%s: info is Null\n", __func__);
		retval = -ENOMEM;
		return retval;
	}
	
	switch (info->op_action) {
		case TS_ACTION_READ:			
			buf[0] = 0x5A;
			retval = novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 2);
			if (retval < 0) {
				TS_LOG_ERR("%s: get roi_switch(%d), failed : %d", __func__, info->roi_switch, retval);
				break;
			}
			info->roi_switch = ((buf[1]>>3) & 0x01);//buf[1] & 0x08;
			TS_LOG_INFO("%s: read roi_switch=%d, 1:on 0:off\n", __func__, info->roi_switch);
			roi_switch = info->roi_switch;
			break;
			
		case TS_ACTION_WRITE:
			TS_LOG_INFO("%s: write roi_switch=%d\n", __func__, info->roi_switch);
			
			sw = info->roi_switch;			
			if ((ROI_SWITCH_ON != sw)
			    && (ROI_SWITCH_OFF != sw)) {
				TS_LOG_ERR("%s: write wrong state: switch = %d\n", __func__, sw);
				retval = -EFAULT;
				break;
			}

			if(ROI_SWITCH_ON == sw)	{		
				//---enable roi mode---
				buf[0] = 0x50;
				buf[1] = 0x77;
				retval = novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);
				if (retval < 0) {
					TS_LOG_ERR("%s: set roi switch(%d), failed : %d", __func__, sw, retval);
				}
			}
			else {
				//---disable roi mode---
				buf[0] = 0x50;
				buf[1] = 0x78;
				retval = novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);
				if (retval < 0) {
					TS_LOG_ERR("%s: set roi switch(%d), failed : %d", __func__, sw, retval);
				}
			}
			roi_switch = info->roi_switch;
			break;
			
		default:
			TS_LOG_ERR("%s: invalid switch status: %d", __func__, info->roi_switch);
			retval = -EINVAL;
			break;
	}

	return retval;
#else
	return NO_ERR;
#endif
}

static unsigned char *novatek_roi_rawdata(void)
{
#ifdef ROI
	TS_LOG_INFO("%s enter\n", __func__);
	return (unsigned char *)roi_data;
#else
	return NULL;
#endif
}

/*  query the configure from dts and store in prv_data */
static int novatek_parse_dts(struct device_node *device,
			       struct ts_device_data *chip_data)
{
	int retval = NO_ERR;
	int read_val = 0;

	TS_LOG_INFO("%s enter\n", __func__);

	chip_data->irq_gpio = of_get_named_gpio(device, "attn_gpio", 0);
	if (!gpio_is_valid(chip_data->irq_gpio)) {
		TS_LOG_ERR("irq gpio is not valid, value is %d\n",
			   chip_data->irq_gpio);
	}
	chip_data->reset_gpio = of_get_named_gpio(device, "reset_gpio", 0);
	if (!gpio_is_valid(chip_data->reset_gpio)) {
		TS_LOG_ERR("reset gpio is not valid\n");
	}
	retval =
	    of_property_read_u32(device, "irq_config",
				 &chip_data->irq_config);
	if (retval) {
		TS_LOG_ERR("get irq config failed\n");
	}
	retval =
	    of_property_read_u32(device, "algo_id",
				 &chip_data->algo_id);
	if (retval) {
		TS_LOG_ERR("get algo id failed\n");
	}
	retval =
	    of_property_read_u32(device, "ic_type",
				 &chip_data->ic_type);
	if (retval) {
		TS_LOG_ERR("get device ic_type failed\n");
	}
	
	retval =
	    of_property_read_u32(device, "x_max", &chip_data->x_max);
	if (retval) {
		TS_LOG_ERR("get device x_max failed\n");
	}
	retval =
	    of_property_read_u32(device, "y_max", &chip_data->y_max);
	if (retval) {
		TS_LOG_ERR("get device y_max failed\n");
	}
	retval =
	    of_property_read_u32(device, "x_max_mt",
				 &chip_data->x_max_mt);
	if (retval) {
		TS_LOG_ERR("get device x_max_mt failed\n");
	}
	retval =
	    of_property_read_u32(device, "y_max_mt",
				 &chip_data->y_max_mt);
	if (retval) {
		TS_LOG_ERR("get device y_max_mt failed\n");
	}

	retval =
	    of_property_read_u32(device, "vci_gpio_type",
				 &chip_data->vci_gpio_type);
	if (retval) {
		TS_LOG_ERR("get device vci_gpio_type failed\n");
	}
	retval =
	    of_property_read_u32(device, "vci_regulator_type",
				 &chip_data->vci_regulator_type);
	if (retval) {
		TS_LOG_ERR("get device vci_regulator_type failed\n");
	}
	retval =
	    of_property_read_u32(device, "vddio_gpio_type",
				 &chip_data->vddio_gpio_type);
	if (retval) {
		TS_LOG_ERR("get device vddio_gpio_type failed\n");
	}

	retval =
	    of_property_read_u32(device, "vddio_regulator_type",
				 &chip_data->vddio_regulator_type);
	if (retval) {
		TS_LOG_ERR
		    ("get device vddio_regulator_type failed\n");
	}

	/*0 is power supplied by gpio, 1 is power supplied by ldo */
	if (1 == chip_data->vci_gpio_type) {
		chip_data->vci_gpio_ctrl =
		    of_get_named_gpio(device, "vci_ctrl_gpio", 0);
		if (!gpio_is_valid(chip_data->vci_gpio_ctrl)) {
			TS_LOG_ERR
			    ("SFT: ok; ASIC: Real err----power gpio is not valid\n");
		}
	}
	if (1 == chip_data->vddio_gpio_type) {
		chip_data->vddio_gpio_ctrl =
		    of_get_named_gpio(device, "vddio_ctrl_gpio", 0);
		if (!gpio_is_valid(chip_data->vddio_gpio_ctrl)) {
			TS_LOG_ERR
			    ("SFT: ok; ASIC: Real err----power gpio is not valid\n");
		}
	}

	retval = of_property_read_u32(device, "roi_supported", &read_val);
	if (!retval) {
		TS_LOG_INFO("get chip specific roi_supported = %d\n", read_val);
		g_ts_data.feature_info.roi_info.roi_supported = (u8) read_val;
	} else {
		TS_LOG_INFO("can not get roi_supported value\n");
		g_ts_data.feature_info.roi_info.roi_supported = 0;
	}

	retval = of_property_read_u32(device, "rawdata_get_timeout", &read_val);
	if (!retval) {
		TS_LOG_INFO("get chip rawdata limit time = %d\n", read_val);
		chip_data->rawdata_get_timeout = read_val;
	} else {
		TS_LOG_INFO("can not get chip rawdata limit time, use default\n");
		chip_data->rawdata_get_timeout = 0;
	}
	TS_LOG_INFO
	    ("reset_gpio = %d, irq_gpio = %d, irq_config = %d, algo_id = %d, ic_type = %d, x_max = %d, y_max = %d, x_mt = %d,y_mt = %d\n",
	     chip_data->reset_gpio, chip_data->irq_gpio, chip_data->irq_config,
	     chip_data->algo_id, chip_data->ic_type, chip_data->x_max,
	     chip_data->y_max, chip_data->x_max_mt, chip_data->y_max_mt);

	return NO_ERR;
}

void novatek_parse_specific_dts(struct ts_device_data *chip_data)
{
	struct device_node *device = NULL;
	int retval = 0;
	char *producer=NULL;
	int read_val = 0;

	TS_LOG_INFO("try to get chip specific dts: %s\n", novatek_project_id);

	device = of_find_compatible_node(NULL, NULL, novatek_project_id);
	if (!device) {
		TS_LOG_INFO("No chip specific dts: %s, need to parse\n",
			    novatek_project_id);
		return;
	}

	retval = of_property_read_u32(device, "roi_supported", &read_val);
	if (!retval) {
		TS_LOG_INFO("get chip specific roi_supported = %d\n", read_val);
		g_ts_data.feature_info.roi_info.roi_supported = (u8) read_val;
	} else {
		TS_LOG_INFO("can not get roi_supported value\n");
		g_ts_data.feature_info.roi_info.roi_supported = 0;
	}

	retval = of_property_read_string(device, "producer", &producer);
	if (!retval && NULL != producer) {
		strcpy(chip_data->module_name, producer);
	}
	TS_LOG_INFO("module_name: %s\n", chip_data->module_name);

	return ;
}

/*******************************************************************************
** TP VCC
* TP VCC/VDD  power control by GPIO in V8Rx,
* if controled by ldo in other products, open "return -EINVAL"
rmi4_data->tp_vci is 3.1V ,rmi4_data->tp_vddio is 1.8V
*/
static int novatek_regulator_get(void)
{
	if (1 == ts->chip_data->vci_regulator_type) {
		ts->tp_vci =
		    regulator_get(&ts->ts_dev->dev, "novatek-vdd");
		if (IS_ERR(ts->tp_vci)) {
			TS_LOG_ERR("regulator tp vci not used\n");
			return -EINVAL;
		}
	}

	if (1 == ts->chip_data->vddio_regulator_type) {
		ts->tp_vddio =
		    regulator_get(&ts->ts_dev->dev, "novatek-io");
		if (IS_ERR(ts->tp_vddio)) {
			TS_LOG_ERR("regulator tp vddio not used\n");
			regulator_put(ts->tp_vci);
			return -EINVAL;
		}
	}

	return 0;
}

static void novatek_regulator_put(void)
{
	if (1 == ts->chip_data->vci_regulator_type) {
		if (!IS_ERR(ts->tp_vci)) {
			regulator_put(ts->tp_vci);
		}
	}
	if (1 == ts->chip_data->vddio_regulator_type) {
		if (!IS_ERR(ts->tp_vddio)) {
			regulator_put(ts->tp_vddio);
		}
	}
}

static int novatek_vci_enable(void)
{
	int retval;
	int vol_vlaue;

	if (IS_ERR(ts->tp_vci)) {
		TS_LOG_ERR("tp_vci is err\n");
		return -EINVAL;
	}

	vol_vlaue = ts->chip_data->regulator_ctr.vci_value;
	if (!IS_ERR(ts->tp_vci)) {
		TS_LOG_INFO("set vci voltage to %d\n", vol_vlaue);
		retval =
		    regulator_set_voltage(ts->tp_vci, vol_vlaue,
					  vol_vlaue);
		if (retval < 0) {
			TS_LOG_ERR
			    ("failed to set voltage regulator tp_vci error: %d\n",
			     retval);
			return -EINVAL;
		}

		retval = regulator_enable(ts->tp_vci);
		if (retval < 0) {
			TS_LOG_ERR("failed to enable regulator tp_vci\n");
			return -EINVAL;
		}
	}
	return 0;
}

static int novatek_vci_disable(void)
{
	int retval;

	if (IS_ERR(ts->tp_vci)) {
		TS_LOG_ERR("tp_vci is err\n");
		return -EINVAL;
	}
	retval = regulator_disable(ts->tp_vci);
	if (retval < 0) {
		TS_LOG_ERR("failed to disable regulator tp_vci\n");
		return -EINVAL;
	}

	return 0;
}

static int novatek_vddio_enable(void)
{
	int retval;
	int vddio_value;

	if (IS_ERR(ts->tp_vddio)) {
		TS_LOG_ERR("tp_vddio is err\n");
		return -EINVAL;
	}

	vddio_value = ts->chip_data->regulator_ctr.vddio_value;
	if (ts->chip_data->regulator_ctr.need_set_vddio_value) {
		TS_LOG_INFO("set tp_vddio voltage to %d\n", vddio_value);
		retval =
		    regulator_set_voltage(ts->tp_vddio, vddio_value,
					  vddio_value);
		if (retval < 0) {
			TS_LOG_ERR
			    ("failed to set voltage regulator tp_vddio error: %d\n",
			     retval);
			return -EINVAL;
		}
	}

	retval = regulator_enable(ts->tp_vddio);
	if (retval < 0) {
		TS_LOG_ERR("failed to enable regulator tp_vddio\n");
		return -EINVAL;
	}

	return 0;
}

static int novatek_vddio_disable(void)
{
	int retval;

	if (IS_ERR(ts->tp_vddio)) {
		TS_LOG_ERR("tp_vddio is err\n");
		return -EINVAL;
	}

	retval = regulator_disable(ts->tp_vddio);
	if (retval < 0) {
		TS_LOG_ERR("failed to disable regulator tp_vddio\n");
		return -EINVAL;
	}

	return 0;
}


/* dts */
static int novatek_pinctrl_get_init(void)
{
	int ret = 0;
	
	ts->pctrl = devm_pinctrl_get(&ts->ts_dev->dev);
	if (IS_ERR(ts->pctrl)) {
		TS_LOG_ERR("failed to devm pinctrl get\n");
		ret = -EINVAL;
		return ret;
	}

	ts->pins_default =
	    pinctrl_lookup_state(ts->pctrl, "default");
	if (IS_ERR(ts->pins_default)) {
		TS_LOG_ERR("failed to pinctrl lookup state default\n");
		ret = -EINVAL;
		goto err_pinctrl_put;
	}

	ts->pins_idle = pinctrl_lookup_state(ts->pctrl, "idle");
	if (IS_ERR(ts->pins_idle)) {
		TS_LOG_ERR("failed to pinctrl lookup state idle\n");
		ret = -EINVAL;
		goto err_pinctrl_put;
	}
	return 0;

err_pinctrl_put:
	devm_pinctrl_put(ts->pctrl);
	return ret;
}

static int novatek_pinctrl_select_normal(void)
{
	int retval = NO_ERR;

	retval =
	    pinctrl_select_state(ts->pctrl, ts->pins_default);
	if (retval < 0) {
		TS_LOG_ERR("set iomux normal error, %d\n", retval);
	}
	return retval;
}

static int novatek_pinctrl_select_lowpower(void)
{
	int retval = NO_ERR;

	retval = pinctrl_select_state(ts->pctrl, ts->pins_idle);
	if (retval < 0) {
		TS_LOG_ERR("set iomux lowpower error, %d\n", retval);
	}
	return retval;
}

static void novatek_power_on_gpio_set(void)
{
	novatek_pinctrl_select_normal();
	gpio_direction_input(ts->chip_data->irq_gpio);
}

static void novatek_vci_on(void)
{
	TS_LOG_INFO("%s vci enable\n", __func__);
	if (1 == ts->chip_data->vci_regulator_type) {
		if (!IS_ERR(ts->tp_vci)) {
			TS_LOG_INFO("vci enable is called\n");
			novatek_vci_enable();
		}
	}

	if (ts->chip_data->vci_gpio_type) {
		TS_LOG_INFO("%s vci switch gpio on\n", __func__);
		gpio_direction_output(ts->chip_data->
				      vci_gpio_ctrl, 1);
	}
}

static void novatek_vddio_on(void)
{
	TS_LOG_INFO("%s vddio enable\n", __func__);
	if (1 == ts->chip_data->vddio_regulator_type) {
		if (!IS_ERR(ts->tp_vddio)) {
			TS_LOG_INFO("vddio enable is called\n");
			novatek_vddio_enable();
		}
	}

	if (ts->chip_data->vddio_gpio_type) {
		TS_LOG_INFO("%s vddio switch gpio on\n", __func__);
		gpio_direction_output(ts->chip_data->
				      vddio_gpio_ctrl, 1);
	}
}

static void novatek_power_on(void)
{
	TS_LOG_INFO("%s enter\n", __func__);

	novatek_vci_on();
	mdelay(5);
	novatek_vddio_on();
	mdelay(5);
	novatek_power_on_gpio_set();
}

static void novatek_power_off_gpio_set(void)
{
	TS_LOG_INFO("%s enter\n", __func__);
	
	//gpio_direction_input(ts->chip_data->reset_gpio);
	novatek_pinctrl_select_lowpower();

	mdelay(1);
}

static void novatek_vddio_off(void)
{
	if (1 == ts->chip_data->vddio_regulator_type) {
		if (!IS_ERR(ts->tp_vddio)) {
			novatek_vddio_disable();
		}
	}

	if (ts->chip_data->vddio_gpio_type) {
		TS_LOG_INFO("%s vddio switch gpio off\n", __func__);
		gpio_direction_output(ts->chip_data->
				      vddio_gpio_ctrl, 0);
	}
}

static void novatek_vci_off(void)
{
	if (1 == ts->chip_data->vci_regulator_type) {
		if (!IS_ERR(ts->tp_vci)) {
			novatek_vci_disable();
		}
	}

	if (ts->chip_data->vci_gpio_type) {
		TS_LOG_INFO("%s vci switch gpio off\n", __func__);
		gpio_direction_output(ts->chip_data->
				      vci_gpio_ctrl, 0);
	}
}

static void novatek_power_off(void)
{
	uint8_t buf[4] = {0};
	
	TS_LOG_INFO("%s enter\n", __func__);

	//---Prevrnt current leakage between sleep-in to power-off, Taylor 20160721---
	//---write i2c command to enter "deep sleep mode"---
	buf[0] = 0x50;
	buf[1] = 0x12;
	novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);
	//------------------------------------------------------------------
	
	novatek_power_off_gpio_set();
	novatek_vddio_off();
	mdelay(12);
	novatek_vci_off();
	mdelay(30);
}

static int novatek_gpio_request(void)
{
	int retval = NO_ERR;
	
	TS_LOG_INFO("%s enter\n", __func__);

	retval = gpio_request(ts->chip_data->reset_gpio, "ts_reset_gpio");
	if (retval < 0) {
		TS_LOG_ERR("Fail request gpio:%d\n",
			   ts->chip_data->reset_gpio);
		goto ts_reset_out;
	}
	
	retval = gpio_request(ts->chip_data->irq_gpio, "ts_irq_gpio");
	if (retval) {
		TS_LOG_ERR("unable to request gpio:%d\n",
			   ts->chip_data->irq_gpio);
		goto ts_irq_out;
	}
	
	if ((1 == ts->chip_data->vci_gpio_type)
	    && (1 == ts->chip_data->vddio_gpio_type)) {
		if (ts->chip_data->vci_gpio_ctrl ==
		    ts->chip_data->vddio_gpio_ctrl) {
			retval = gpio_request(ts->chip_data->vci_gpio_ctrl, "ts_vci_gpio");
			if (retval) {
				TS_LOG_ERR
				    ("SFT:Ok;  ASIC: Real ERR----unable to request vci_gpio_ctrl firset:%d\n",
				     ts->chip_data->vci_gpio_ctrl);
				goto ts_vci_out;
			}
		} else {
			retval = gpio_request(ts->chip_data->vci_gpio_ctrl, "ts_vci_gpio");
			if (retval) {
				TS_LOG_ERR
				    ("SFT:Ok;  ASIC: Real ERR----unable to request vci_gpio_ctrl2:%d\n",
				     ts->chip_data->vci_gpio_ctrl);
				goto ts_vci_out;
			}
			retval = gpio_request(ts->chip_data->vddio_gpio_ctrl, "ts_vddio_gpio");
			if (retval) {
				TS_LOG_ERR
				    ("SFT:Ok;  ASIC: Real ERR----unable to request vddio_gpio_ctrl:%d\n",
				     ts->chip_data->vddio_gpio_ctrl);
				goto ts_vddio_out;
			}
		}
	} else {
		if (1 == ts->chip_data->vci_gpio_type) {
			retval = gpio_request(ts->chip_data->vci_gpio_ctrl, "ts_vci_gpio");
			if (retval) {
				TS_LOG_ERR
				    ("SFT:Ok;  ASIC: Real ERR----unable to request vci_gpio_ctrl2:%d\n",
				     ts->chip_data->vci_gpio_ctrl);
				goto ts_vci_out;
			}
		}
		if (1 == ts->chip_data->vddio_gpio_type) {
			retval = gpio_request(ts->chip_data->vddio_gpio_ctrl, "ts_vddio_gpio");
			if (retval) {
				TS_LOG_ERR
				    ("SFT:Ok;  ASIC: Real ERR----unable to request vddio_gpio_ctrl:%d\n",
				     ts->chip_data->vddio_gpio_ctrl);
				goto ts_vddio_out;
			}
		}
	}

	TS_LOG_INFO("reset:%d, irq:%d,\n",
		    ts->chip_data->reset_gpio,
		    ts->chip_data->irq_gpio);

	goto ts_reset_out;

ts_vddio_out:
	gpio_free(ts->chip_data->vci_gpio_ctrl);
ts_vci_out:
	gpio_free(ts->chip_data->irq_gpio);
ts_irq_out:
	gpio_free(ts->chip_data->reset_gpio);
ts_reset_out:
	return retval;
}

static void novatek_gpio_free(void)
{
	TS_LOG_INFO("%s enter\n", __func__);

	gpio_free(ts->chip_data->irq_gpio);
	gpio_free(ts->chip_data->reset_gpio);
	/*0 is power supplied by gpio, 1 is power supplied by ldo */
	if (1 == ts->chip_data->vci_gpio_type) {
		if (ts->chip_data->vci_gpio_ctrl)
			gpio_free(ts->chip_data->vci_gpio_ctrl);
	}
	if (1 == ts->chip_data->vddio_gpio_type) {
		if (ts->chip_data->vddio_gpio_ctrl)
			gpio_free(ts->chip_data->vddio_gpio_ctrl);
	}
}

/*******************************************************
Description:
	Novatek touchscreen check chip version trim function.

return:
	Executive outcomes. 0---NVT IC. -1---not NVT IC.
*******************************************************/
static int8_t nvt_ts_check_chip_ver_trim(void)
{
	uint8_t buf[8] = {0};
	int32_t retry = 0;
	int32_t ret = -1;

	//---Check for 5 times---
	for (retry = 5; retry > 0; retry--) {
		nvt_bootloader_reset();
		nvt_sw_reset_idle();

		buf[0] = 0x00;
		buf[1] = 0x35;
		novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 2);
		msleep(10);

		buf[0] = 0xFF;
		buf[1] = 0x01;
		buf[2] = 0xF6;
		novatek_ts_i2c_write(ts->client, I2C_BLDR_Address, buf, 3);

		buf[0] = 0x4E;
		buf[1] = 0x00;
		buf[2] = 0x00;
		buf[3] = 0x00;
		buf[4] = 0x00;
		buf[5] = 0x00;
		buf[6] = 0x00;
		novatek_ts_i2c_read(ts->client, I2C_BLDR_Address, buf, 7);

		if ((buf[1] == 0x55) && (buf[2] == 0x00) && (buf[4] == 0x00) && (buf[5] == 0x00) && (buf[6] == 0x00)) {
			TS_LOG_INFO("%s this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else if ((buf[1] == 0x55) && (buf[2] == 0x72) && (buf[4] == 0x00) && (buf[5] == 0x00) && (buf[6] == 0x00)) {
			TS_LOG_INFO("%s this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else if ((buf[1] == 0xAA) && (buf[2] == 0x00) && (buf[4] == 0x00) && (buf[5] == 0x00) && (buf[6] == 0x00)) {
			TS_LOG_INFO("%s this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else if ((buf[1] == 0xAA) && (buf[2] == 0x72) && (buf[4] == 0x00) && (buf[5] == 0x00) && (buf[6] == 0x00)) {
			TS_LOG_INFO("%s this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else if ((buf[4] == 0x72) && (buf[5] == 0x67) && (buf[6] == 0x03)) {
			TS_LOG_INFO("%s this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else if ((buf[4] == 0x70) && (buf[5] == 0x66) && (buf[6] == 0x03)) {
			TS_LOG_INFO("%s this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else if ((buf[4] == 0x70) && (buf[5] == 0x67) && (buf[6] == 0x03)) {
			TS_LOG_INFO("%s this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else if ((buf[4] == 0x72) && (buf[5] == 0x66) && (buf[6] == 0x03)) {
			TS_LOG_INFO("%s this is NVT touch IC\n", __func__);
			ret = 0;
			break;
		} else {
			TS_LOG_ERR("%s: buf[1]=0x%02X, buf[2]=0x%02X, buf[3]=0x%02X, buf[4]=0x%02X, buf[5]=0x%02X, buf[6]=0x%02X\n",
				__func__, buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
			ret = -1;
		}

		msleep(10);
	}

	return ret;
}

static int novatek_chip_detect(struct device_node *device,
				 struct ts_device_data *chip_data,
				 struct platform_device *ts_dev)
{
	int retval = NO_ERR;
      	uint8_t buf[8] = {0};

	TS_LOG_INFO("%s enter\n", __func__);

	if (!device || !chip_data || !ts_dev) {
		TS_LOG_ERR("device, chip_data or ts_dev is NULL \n");
		return -ENOMEM;
	}

	ts = kmalloc(sizeof(struct nvt_ts_data), GFP_KERNEL);
	if (ts == NULL) {
		TS_LOG_ERR("failed to allocated memory for nvt ts data\n");
		return -ENOMEM;
	}

	mutex_init(&ts->i2c_mutex);
	mutex_init(&ts->mp_mutex);

	ts->chip_data = chip_data;
	ts->ts_dev = ts_dev;
	ts->ts_dev->dev.of_node = device;

	ts->client = chip_data->client;

	ts->abs_x_max = TOUCH_MAX_WIDTH;
	ts->abs_y_max = TOUCH_MAX_HEIGHT;
	ts->max_touch_num = TOUCH_MAX_FINGER_NUM;

#if TOUCH_KEY_NUM > 0
	ts->max_button_num = TOUCH_KEY_NUM;
#endif

	//ts->int_trigger_type = INT_TRIGGER_TYPE;

	retval = novatek_regulator_get();
	if (retval < 0) {
		goto regulator_err;
	}

	retval = novatek_gpio_request();
	if (retval < 0) {
		goto gpio_err;
	}

	retval = novatek_pinctrl_get_init();
	if (retval < 0) {
		goto pinctrl_get_err;
	}

	/*power up the chip */
	novatek_power_on();

	/*reset the chip */
	if (!chip_data->disable_reset) {
		nvt_hw_reset();
		TS_LOG_INFO("chip has been reset\n");
	} else {
		TS_LOG_INFO("chip not do reset\n");
	}
	
	//---check i2c read befor checking chipid in SH, Mallon 20160928
	if(novatek_ts_i2c_read(ts-> client,  I2C_HW_Address,  buf, 2)<0){
		nvt_hw_reset();
		if(novatek_ts_i2c_read(ts-> client,  I2C_HW_Address,  buf, 2)<0){
			TS_LOG_INFO("not find novatek devices\n");
                     goto check_err;
		}
	}

	//---not tested yet in sz, Taylor 20160506---
	//---revise and test in sh, Taylor 20160629---
	retval = nvt_ts_check_chip_ver_trim();
	if (retval) {
		TS_LOG_ERR("not find novatek device\n");
		goto check_err;
	} else {
		TS_LOG_INFO("find novatek device\n");
	}
	//------------------------------------------- 

	TS_LOG_INFO("%s done\n", __func__);
	return NO_ERR;

check_err:
	novatek_power_off();
pinctrl_get_err:
	novatek_gpio_free();
gpio_err:
	novatek_regulator_put();
regulator_err:
	if (ts)
		kfree(ts);
	ts = NULL;
	TS_LOG_ERR("no power\n");
	return retval;
}

static int novatek_init(void)
{
	int retval = NO_ERR;
	
	TS_LOG_INFO("%s enter\n", __func__);

#if NVT_TOUCH_PROC
	retval = nvt_flash_proc_init();
	if (retval != 0) {
		TS_LOG_ERR("nvt flash proc init failed. retval=%d\n", retval);
		goto init_NVT_ts_err;
	}
#endif

#if NVT_TOUCH_EXT_PROC
	retval = nvt_extra_proc_init();
	if (retval != 0) {
		TS_LOG_ERR("nvt extra proc init failed. retval=%d\n", retval);
		goto init_NVT_ts_err;
	}
#endif

	//---Prevrnt boot load flash failed while power-on, set this bootloader for safety, Taylor 20160721---
	// please make sure display reset(RESX) sequence and mipi dsi cmds sent before this
	nvt_bootloader_reset();
	nvt_check_fw_reset_state(RESET_STATE_INIT);
	//----------------------------------------------------------------------------------

	//get project id and fw version
	novatek_read_projectid();
	nvt_get_fw_info();

	//get specific dts config
	novatek_parse_specific_dts(ts->chip_data);

	TS_LOG_INFO("%s done\n", __func__);
	return NO_ERR;
	
init_NVT_ts_err:
	return retval;
}

static int novatek_input_config(struct input_dev *input_dev)
{
	TS_LOG_INFO("%s enter\n", __func__);

	//---set input device info.---
	input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) ;
	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	input_dev->propbit[0] = BIT(INPUT_PROP_DIRECT);

#if TOUCH_MAX_FINGER_NUM > 1
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);	  //area major = 255
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR, 0, 255, 0, 0);	  //area minor = 255

	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, (ts->abs_x_max - 1), 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, (ts->abs_y_max - 1), 0, 0);
#endif

	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, ts->max_touch_num, 0, 0);

#if TOUCH_KEY_NUM > 0
	for (retry = 0; retry < TOUCH_KEY_NUM; retry++) {
		input_set_capability(input_dev, EV_KEY, touch_key_array[retry]);
	}
#endif

	return NO_ERR;
}

static int novatek_irq_top_half(struct ts_cmd_node *cmd)
{
	cmd->command = TS_INT_PROCESS;

	TS_LOG_DEBUG("%s enter\n", __func__);
	
	return NO_ERR;
}

static int novatek_irq_bottom_half(struct ts_cmd_node *in_cmd,
				     struct ts_cmd_node *out_cmd)
{
	struct ts_fingers *info =
	    &out_cmd->cmd_param.pub_params.algo_param.info;

	int32_t ret = -1;
	uint8_t point_data[66] = {0};
	uint32_t position = 0;
	uint32_t input_x = 0;
	uint32_t input_y = 0;
	uint32_t input_w_major = 0;
	uint32_t input_w_minor = 0;
	uint8_t input_id = 0;
	uint8_t press_id[TOUCH_MAX_FINGER_NUM] = {0};

	int32_t i = 0;
	int32_t finger_cnt = 0;

	uint8_t roi_diff[ROI_DATA_READ_LENGTH+1] = {0};
	int32_t temp_finger_status = 0;


	out_cmd->command = TS_INPUT_ALGO;
	out_cmd->cmd_param.pub_params.algo_param.algo_order = ts->chip_data->algo_id;
	TS_LOG_DEBUG("order: %d\n",
		     out_cmd->cmd_param.pub_params.algo_param.algo_order);

	ret = novatek_ts_i2c_read(ts->client, I2C_FW_Address, point_data, 64 + 1);
	if (ret < 0) {
		TS_LOG_ERR("%s: novatek_ts_i2c_read failed. ret=%d\n", __func__, ret);
		goto XFER_ERROR;
	}

	//--- dump I2C buf ---
	//for (i = 0; i < 10; i++) {
	//	printk("%02X %02X %02X %02X %02X %02X  ", point_data[1+i*6], point_data[2+i*6], point_data[3+i*6], point_data[4+i*6], point_data[5+i*6], point_data[6+i*6]);
	//}
	//printk("\n");

	finger_cnt = 0;
	input_id = (uint8_t)(point_data[1] >> 3);

	for (i = 0; i < ts->max_touch_num; i++) {
		/*
		 * Each 2-bit finger status field represents the following:
		 * 00 = finger not present
		 * 01 = finger present and data accurate
		 * 10 = finger present but data may be inaccurate
		 * 11 = reserved
		 */
		info->fingers[i].status = 0;
	}

	for (i = 0; i < ts->max_touch_num; i++) {
		position = 1 + 6 * i;
		input_id = (uint8_t)(point_data[position + 0] >> 3);
		if (input_id > TOUCH_MAX_FINGER_NUM)
			continue;

		if ((point_data[position] & 0x07) == 0x03) {	// finger up (break)
			continue;
		} else if (((point_data[position] & 0x07) == 0x01) || ((point_data[position] & 0x07) == 0x02)) {	//finger down (enter & moving)
			input_x = (uint32_t)(point_data[position + 1] << 4) + (uint32_t) (point_data[position + 3] >> 4);
			input_y = (uint32_t)(point_data[position + 2] << 4) + (uint32_t) (point_data[position + 3] & 0x0F);
			input_w_major = (uint32_t)(point_data[position + 4]);
			if (input_w_major > 255)
				input_w_major = 255;
			input_w_minor = (uint32_t)(point_data[position + 5]);
			if (input_w_minor > 255)
				input_w_minor = 255;
			
			if ((input_x < 0) || (input_y < 0))
				continue;
			if ((input_x > ts->abs_x_max)||(input_y > ts->abs_y_max))
				continue;

			press_id[input_id - 1] = 1;
			info->fingers[input_id - 1].status = 1;
			info->fingers[input_id - 1].x = input_x;
			info->fingers[input_id - 1].y = input_y;
			info->fingers[input_id - 1].major = input_w_major;
			info->fingers[input_id - 1].minor = input_w_minor;
			finger_cnt++;
			//temp_finger_status = 1;
			temp_finger_status ++;
		}
	}
	info->cur_finger_number = finger_cnt;

#if TOUCH_KEY_NUM > 0
	if (point_data[61] == 0xF8) {
		info->special_button_flag = 1;
		for (i = 0; i < TOUCH_KEY_NUM; i++) {
				info->special_button_key = touch_key_array[i];
		}
	} else {
		info->special_button_flag = 0;
	}
#endif


#ifdef ROI
	if (roi_switch) {
		if (temp_finger_status
			&& temp_finger_status != pre_finger_status) {
			roi_diff[0] = 0x99;
			ret = novatek_ts_i2c_read(ts->client, I2C_FW_Address, roi_diff, ROI_DATA_READ_LENGTH + 1);
			if (ret < 0) {
				TS_LOG_ERR("%s: novatek_ts_i2c_read failed(ROI). ret=%d\n", __func__, ret);
				goto XFER_ERROR;
			}

			for(i=0;i<ROI_DATA_READ_LENGTH;i++)
				roi_data[i] = roi_diff[i+1];
		}
	}
	pre_finger_status = temp_finger_status;
#endif

	
XFER_ERROR:
	return NO_ERR;
}

/*  do some things before power off.
*/
static int novatek_before_suspend(void)
{
	//uint8_t buf[4] = {0};
	int retval = NO_ERR;

	TS_LOG_INFO("before_suspend +\n");

	nvt_hw_reset_down();
	TS_LOG_INFO("before_suspend -\n");
	return retval;
}

static int novatek_suspend(void)
{
	int retval = NO_ERR;

	TS_LOG_INFO("%s enter\n", __func__);

	return retval;
}

/*    do not add time-costly function here.
*/
static int novatek_resume(void)
{
	int retval = NO_ERR;

	TS_LOG_INFO("%s enter\n", __func__);
	nvt_hw_reset();
	return retval;
}

/*  do some things after power on. */
static int novatek_after_resume(void *feature_info)
{
	int retval = NO_ERR;
	
	TS_LOG_INFO("after_resume +\n");
	
	//---Prevrnt boot load flash failed while power-on, set this bootloader for safety, Taylor 20160721---
	// please make sure display reset(RESX) sequence and mipi dsi cmds sent before this
	nvt_bootloader_reset();
	nvt_check_fw_reset_state(RESET_STATE_REK);
	//----------------------------------------------------------------------------------
	
	TS_LOG_INFO("after_resume -\n");
	return retval;
}

int32_t novatek_read_projectid(void)
{
	char wrong_id[20]={"999999999"};
	uint8_t buf[64] = {0};
	int retval = NO_ERR;

	//---stop fw, Taylor 20160705---
	nvt_sw_reset_idle();
	//---------------------------

	// Step 1 : initial bootloader
	retval = Init_BootLoader();
	if (retval) {
		return retval;
	}

	// Step 2 : Resume PD
	retval = Resume_PD();
	if (retval) {
		return retval;
	}

	// Step 3 : unlock
	buf[0] = 0x00;
	buf[1] = 0x35;
	retval = novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 2);
	if (retval < 0) {
		TS_LOG_ERR("%s: write unlock error!!(%d)\n", __func__, retval);
		return retval;
	}
	msleep(10);

	//Step 4 : Flash Read Command
	buf[0] = 0x00;
	buf[1] = 0x03;
	buf[2] = 0x02;
	buf[3] = 0x00;
	buf[4] = 0x00;
	buf[5] = 0x00;
	buf[6] = 0x20;
	retval = novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 7);
	if (retval < 0) {
		TS_LOG_ERR("%s: write Read Command error!!(%d)\n", __func__, retval);
		return retval;
	}
	msleep(10);

	// Check 0xAA (Read Command)
	buf[0] = 0x00;
	buf[1] = 0x00;
	retval = novatek_ts_i2c_read(ts->client, I2C_HW_Address, buf, 2);
	if (retval < 0) {
		TS_LOG_ERR("%s: Check 0xAA (Read Command) error!!(%d)\n", __func__, retval);
		return retval;
	}
	if (buf[1] != 0xAA) {
		TS_LOG_ERR("%s: Check 0xAA (Read Command) error!! status=0x%02X\n", __func__, buf[1]);
		return -1;
	}
	msleep(10);

	//Step 5 : Read Flash Data
	buf[0] = 0xFF;
	buf[1] = 0x01;
	buf[2] = 0x40;
	retval = novatek_ts_i2c_write(ts->client, I2C_BLDR_Address, buf, 3);
	if (retval < 0) {
		TS_LOG_ERR("%s: change index error!!(%d)\n", __func__, retval);
		return retval;
	}
	msleep(10);

	// Read Back
	buf[0] = 0x00;
	retval = novatek_ts_i2c_read(ts->client, I2C_BLDR_Address, buf, 33);
	if (retval < 0) {
		TS_LOG_ERR("%s: Check 0xAA (Read Command) error!!(%d)\n", __func__, retval);
		return retval;
	}
	msleep(10);

	//buf[3:12]	=> novatek_product id
	//buf[13:22]	=> novatek_project id
	strncpy(novatek_product_id, &buf[3], PROJECT_ID_LEN);
	strncpy(novatek_project_id, &buf[14], PROJECT_ID_LEN);

	//---workaround : lgd puts wrong id "VCT033100", Taylor 20160715---
	strncpy(wrong_id, "VCT033100", 9);
	if(strcmp(novatek_project_id, wrong_id) == 0){
		TS_LOG_INFO("novatek_project_id=%s, replace to VCTO33100\n", novatek_project_id);
		strncpy(novatek_product_id, "VCTO33100", PROJECT_ID_LEN);
		strncpy(novatek_project_id, "VCTO33100", PROJECT_ID_LEN);
	}
	//----------------------------------------------------------

	TS_LOG_INFO("novatek_product_id=%s, novatek_project_id=%s\n", novatek_product_id, novatek_project_id);

	//---reset ic to run fw, Taylor 20160705---	
	nvt_bootloader_reset();
	//-----------------------------------

	return retval;
}

static int novatek_get_info(struct ts_chip_info_param *info)
{
	int retval = NO_ERR;
	nvt_get_fw_info();

	TS_LOG_INFO("%s enter\n", __func__);
	snprintf(info->ic_vendor, PAGE_SIZE, "nt36772-%s", novatek_project_id);
	snprintf(info->mod_vendor, PAGE_SIZE, "lgd");
	snprintf(info->fw_vendor, PAGE_SIZE, "%02x", fw_ver);
	return retval;
}

static int novatek_fw_update_boot(char *file_name)
{
	int retval = NO_ERR;
	char fw_file_name[128]={0};

	TS_LOG_INFO("%s enter\n", __func__);

	snprintf(fw_file_name, sizeof(fw_file_name), "ts/%s_%s_%s_%s.bin",
		g_ts_data.product_name,
		g_ts_data.chip_data->chip_name,
		novatek_project_id,
		ts->chip_data->module_name);
	TS_LOG_INFO("fw file_name is :%s\n", fw_file_name);

	retval = nvt_fw_update_boot(fw_file_name);

	TS_LOG_INFO("%s exit\n", __func__);
	return retval;		
}

static int novatek_fw_update_sd(void)
{
	int retval = NO_ERR;
	char fw_file_name[128]={0};

	TS_LOG_INFO("%s enter\n", __func__);

	snprintf(fw_file_name, sizeof(fw_file_name), "ts/%s_%s_%s_%s.bin",
		g_ts_data.product_name,
		g_ts_data.chip_data->chip_name,
		novatek_project_id,
		ts->chip_data->module_name);
	TS_LOG_INFO("fw file_name is :%s\n", fw_file_name);

	g_sd_force_update = 1;

	retval = nvt_fw_update_boot(fw_file_name);
	TS_LOG_INFO("%s exit\n", __func__);
	return retval;
}

static void novatek_shutdown(void)
{
	TS_LOG_INFO("%s enter\n", __func__);

	novatek_power_off();
	novatek_gpio_free();
	novatek_regulator_put();
	return;
}

static int novatek_get_rawdata(struct ts_rawdata_info *info, struct ts_cmd_node *out_cmd)
{
	int retval = NO_ERR;
	TS_LOG_INFO("%s enter\n", __func__);

	retval = nvt_selftest(info);
	if (retval < 0) 
		TS_LOG_ERR("failed to get rawdata\n");

	return retval;
}

void novatek_ghost_detect(int value){
	if (GHOST_OPERATE_TOO_FAST == value){
		TS_LOG_INFO("%s operate too fast\n", __func__);
	}else if (GHOST_OPERATE_TWO_POINT_OPER_TOO_FAST == value){
		TS_LOG_INFO("%s operate 2 point too fast\n", __func__);
	}
	//DMD report
#if defined (CONFIG_HUAWEI_DSM)
	if (!dsm_client_ocuppy(tp_dclient)) {
		dsm_client_record(tp_dclient, "novatek ghost touch detect.\n");
		dsm_client_notify(tp_dclient, DSM_TP_GHOST_TOUCH_ERROR_NO);
	}
#endif
}

int novatek_chip_check_status(void){
	TS_LOG_DEBUG("%s +\n", __func__);
	TS_LOG_DEBUG("%s -\n", __func__);
	return 0;
}

static u8 tp_result_info[TS_CHIP_TYPE_MAX_SIZE] = {0};
static u8 tp_type_cmd[TS_CHIP_TYPE_MAX_SIZE] = {0};

static unsigned short novatek_get_oem_data_info(void){
	return 256;
}

#define	LCD_DATA_LEN 512
int32_t test_size=LCD_DATA_LEN;
uint8_t test_data[LCD_DATA_LEN]={0};

static int novatek_get_oem_data(uint8_t *data, int32_t size)
{
	uint8_t buf[64] = {0};
	uint8_t tmp_data[1024] = {0};
	int32_t count_256 = 0;
	int32_t Flash_Address = 0x1F000;
	int32_t i = 0, j = 0, k = 0;
	int retval = 0;

	TS_LOG_INFO("%s:++\n", __func__);
	if (size % 256)
		count_256 = size / 256 + 1;
	else
		count_256 = size / 256;


	nvt_sw_reset_idle();

	// Step 1 : initial bootloader
	retval = Init_BootLoader();
	if (retval) {
		return retval;
	}

	// Step 2 : Resume PD
	retval = Resume_PD();
	if (retval) {
		return retval;
	}

	// Step 3 : unlock
	buf[0] = 0x00;
	buf[1] = 0x35;
	retval = novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 2);
	if (retval < 0) {
		TS_LOG_ERR("%s: write unlock error!!(%d)\n", __func__, retval);
		return retval;
	}
	msleep(10);


	for (i = 0; i < count_256; i++) {
		Flash_Address += i * 256;

		//Step 4 : Flash Read Command
		buf[0] = 0x00;
		buf[1] = 0x03;
		buf[2] = ((Flash_Address >> 16) & 0xFF);
		buf[3] = ((Flash_Address >> 8) & 0xFF);
		buf[4] = (Flash_Address & 0xFF);
		buf[5] = 0x01;
		buf[6] = 0x00;
		retval = novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 7);
		if (retval < 0) {
			TS_LOG_ERR("%s: write Read Command error!!(%d)\n", __func__, retval);
			return retval;
		}
		msleep(10);

		// Check 0xAA (Read Command)
		buf[0] = 0x00;
		buf[1] = 0x00;
		retval = novatek_ts_i2c_read(ts->client, I2C_HW_Address, buf, 2);
		if (retval < 0) {
			TS_LOG_ERR("%s: Check 0xAA (Read Command) error!!(%d)\n", __func__, retval);
			return retval;
		}
		if (buf[1] != 0xAA) {
			TS_LOG_ERR("%s: Check 0xAA (Read Command) error!! status=0x%02X\n", __func__, buf[1]);
			return -1;
		}
		msleep(10);

		//Step 5 : Read Flash Data
		for (j = 0; j < (256 / 32) + 1; j++) {
			buf[0] = 0xFF;
			buf[1] = 0x01;
			buf[2] = (j == (256 / 32) ? 0x41 : 0x40);
			retval = novatek_ts_i2c_write(ts->client, I2C_BLDR_Address, buf, 3);
			if (retval < 0) {
				TS_LOG_ERR("%s: change index error!!(%d)\n", __func__, retval);
				return retval;
			}

			buf[0] = j * 32;
			retval = novatek_ts_i2c_read(ts->client, I2C_BLDR_Address, buf, 33);
			if (retval < 0) {
				TS_LOG_ERR("%s: read data error!!(%d)\n", __func__, retval);
				return retval;
			}

			for (k = 0; k < 32; k++)  {
				tmp_data[j * 32 + k] = buf[k + 1];
			}
		}

		//Step 6 : Remapping (Remove 2Bytes Checksum)
		for (j = 0; j < 256; j++) {
			if((i * 256 + j) < size) {
				data[i * 256 + j] = tmp_data[j + 2];
			}
		}
	}

	nvt_bootloader_reset();

	TS_LOG_INFO("%s:--\n", __func__);

	return retval;
}

static int novatek_set_oem_data(uint8_t *data, int32_t size)
{
	uint8_t buf[64] = {0};
	uint32_t XDATA_Addr = 0x14002;
	uint32_t Flash_Address = 0x1F000;
	int32_t i = 0, j = 0, k = 0;
	uint8_t tmpvalue = 0;
	int32_t count_256 = 0;
	int32_t retry = 0;
	int retval = 0;

	TS_LOG_INFO("%s:++\n", __func__);
	if (size % 256)
		count_256 = size / 256 + 1;
	else
		count_256 = size / 256;


	nvt_sw_reset_idle();

	// Step 1 : initial bootloader
	retval = Init_BootLoader();
	if (retval) {
		return retval;
	}

	// Step 2 : Resume PD
	retval = Resume_PD();
	if (retval) {
		return retval;
	}

	// Step 3 : Write Enable
	buf[0] = 0x00;
	buf[1] = 0x06;
	retval = novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 2);
	if (retval < 0) {
		TS_LOG_ERR("%s: Write Enable error!!(%d,%d)\n", __func__, retval, i);
		return retval;
	}
	mdelay(10);

	// Check 0xAA (Write Enable)
	buf[0] = 0x00;
	buf[1] = 0x00;
	retval = novatek_ts_i2c_read(ts->client, I2C_HW_Address, buf, 2);
	if (retval < 0) {
		TS_LOG_ERR("%s: Check 0xAA (Write Enable) error!!(%d,%d)\n", __func__, retval, i);
		return retval;
	}
	if (buf[1] != 0xAA) {
		TS_LOG_ERR("%s: Check 0xAA (Write Enable) error!! status=0x%02X\n", __func__, buf[1]);
		return -1;
	}
	mdelay(10);

	// Sector Erase
	buf[0] = 0x00;
	buf[1] = 0x20;	  // Command : Sector Erase
	buf[2] = ((Flash_Address >> 16) & 0xFF);
	buf[3] = ((Flash_Address >> 8) & 0xFF);
	buf[4] = (Flash_Address & 0xFF);
	retval = novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 5);
	if (retval < 0) {
		TS_LOG_ERR("%s: Sector Erase error!!(%d,%d)\n", __func__, retval, i);
		return retval;
	}
	mdelay(20);

	retry = 0;
	while (1) {
		// Check 0xAA (Sector Erase)
		buf[0] = 0x00;
		buf[1] = 0x00;
		retval = novatek_ts_i2c_read(ts->client, I2C_HW_Address, buf, 2);
		if (retval < 0) {
			TS_LOG_ERR("%s: Check 0xAA (Sector Erase) error!!(%d,%d)\n", __func__, retval, i);
			return retval;
		}
		if (buf[1] == 0xAA) {
			break;
		}
		retry++;
		if (unlikely(retry > 5)) {
			TS_LOG_ERR("%s: Check 0xAA (Sector Erase) failed, buf[1]=0x%02X, retry=%d\n", __func__, buf[1], retry);
			return -1;
		}
		mdelay(10);
	}

	// Read Status
	retry = 0;
	while (1) {
		mdelay(30);
		buf[0] = 0x00;
		buf[1] = 0x05;
		retval = novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 2);
		if (retval < 0) {
			TS_LOG_ERR("%s: Read Status error!!(%d,%d)\n", __func__, retval, i);
			return retval;
		}

		// Check 0xAA (Read Status)
		buf[0] = 0x00;
		buf[1] = 0x00;
		buf[2] = 0x00;
		retval = novatek_ts_i2c_read(ts->client, I2C_HW_Address, buf, 3);
		if (retval < 0) {
			TS_LOG_ERR("%s: Check 0xAA (Read Status) error!!(%d,%d)\n", __func__, retval, i);
			return retval;
		}
		if ((buf[1] == 0xAA) && (buf[2] == 0x00)) {
			break;
		}
		retry++;
		if (unlikely(retry > 5)) {
			TS_LOG_ERR("%s:Check 0xAA (Read Status) failed, buf[1]=0x%02X, buf[2]=0x%02X, retry=%d\n", __func__, buf[1], buf[2], retry);
			return -1;
		}
	}


	// change I2C buffer index
	buf[0] = 0xFF;
	buf[1] = XDATA_Addr >> 16;
	buf[2] = (XDATA_Addr >> 8) & 0xFF;
	retval = novatek_ts_i2c_write(ts->client, I2C_BLDR_Address, buf, 3);
	if (retval < 0) {
		TS_LOG_ERR("%s: change I2C buffer index error!!(%d)\n", __func__, retval);
		return retval;
	}

	for (i = 0; i < count_256; i++) {
		Flash_Address += i * 256;

		// Write Enable
		buf[0] = 0x00;
		buf[1] = 0x06;
		retval = novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 2);
		if (retval < 0) {
			TS_LOG_ERR("%s: Write Enable error!!(%d)\n", __func__, retval);
			return retval;
		}
		udelay(100);

		// Write Page : 256 bytes
		for (j = 0; j < min((size_t)size - (i * 256), (size_t)256); j += 32) {
			buf[0] = (XDATA_Addr + j) & 0xFF;
			for (k = 0; k < 32; k++) {
				buf[1 + k] = data[(i * 256) + j + k];
			}
			retval = novatek_ts_i2c_write(ts->client, I2C_BLDR_Address, buf, 33);
			if (retval < 0) {
				TS_LOG_ERR("%s: Write Page error!!(%d), j=%d\n", __func__, retval, j);
				return retval;
			}
		}
		if (size - (i * 256) >= 256)
			tmpvalue=(Flash_Address >> 16) + ((Flash_Address >> 8) & 0xFF) + (Flash_Address & 0xFF) + 0x00 + (255);
		else
			tmpvalue=(Flash_Address >> 16) + ((Flash_Address >> 8) & 0xFF) + (Flash_Address & 0xFF) + 0x00 + (size - (i * 256) - 1);

		for (k = 0; k < min((size_t)size - (i * 256),(size_t)256); k++)
			tmpvalue += data[(i * 256) + k];

		tmpvalue = 255 - tmpvalue + 1;

		// Page Program
		buf[0] = 0x00;
		buf[1] = 0x02;
		buf[2] = ((Flash_Address >> 16) & 0xFF);
		buf[3] = ((Flash_Address >> 8) & 0xFF);
		buf[4] = (Flash_Address & 0xFF);
		buf[5] = 0x00;
		buf[6] = min((size_t)size - (i * 256),(size_t)256) - 1;
		buf[7] = tmpvalue;
		retval = novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 8);
		if (retval < 0) {
			TS_LOG_ERR("%s: Page Program error!!(%d), i=%d\n", __func__, retval, i);
			return retval;
		}

		// Check 0xAA (Page Program)
		retry = 0;
		while (1) {
			mdelay(3);
			buf[0] = 0x00;
			buf[1] = 0x00;
			retval = novatek_ts_i2c_read(ts->client, I2C_HW_Address, buf, 2);
			if (retval < 0) {
				TS_LOG_ERR("%s: Page Program error!!(%d)\n", __func__, retval);
				return retval;
			}
			if (buf[1] == 0xAA || buf[1] == 0xEA) {
				break;
			}
			retry++;
			if (unlikely(retry > 5)) {
				TS_LOG_ERR("%s: Check 0xAA (Page Program) failed, buf[1]=0x%02X, retry=%d\n", __func__, buf[1], retry);
				return -1;
			}
		}
		if (buf[1] == 0xEA) {
			TS_LOG_ERR("%s: Page Program error!! i=%d\n", __func__, i);
			return -3;
		}

		// Read Status
		retry = 0;
		while (1) {
			mdelay(2);
			buf[0] = 0x00;
			buf[1] = 0x05;
			retval = novatek_ts_i2c_write(ts->client, I2C_HW_Address, buf, 2);
			if (retval < 0) {
				TS_LOG_ERR("%s: Read Status error!!(%d)\n", __func__, retval);
				return retval;
			}

			// Check 0xAA (Read Status)
			buf[0] = 0x00;
			buf[1] = 0x00;
			buf[2] = 0x00;
			retval = novatek_ts_i2c_read(ts->client, I2C_HW_Address, buf, 3);
			if (retval < 0) {
				TS_LOG_ERR("%s: Check 0xAA (Read Status) error!!(%d)\n", __func__, retval);
				return retval;
			}
			if (((buf[1] == 0xAA) && (buf[2] == 0x00)) || (buf[1] == 0xEA)) {
				break;
			}
			retry++;
			if (unlikely(retry > 5)) {
				TS_LOG_ERR("%s: Check 0xAA (Read Status) failed, buf[1]=0x%02X, buf[2]=0x%02X, retry=%d\n", __func__, buf[1], buf[2], retry);
				return -1;
			}
		}
		if (buf[1] == 0xEA) {
			TS_LOG_ERR("%s: Page Program error!! i=%d\n", __func__, i);
			return -4;
		}
	}

	nvt_bootloader_reset();

	TS_LOG_INFO("%s:--\n", __func__);

	return retval;
}

static int novatek_reconstruct_barcode(struct ts_oem_info_param *info)
{
	 int retval = NO_ERR;
	 int offset1 = TS_NV_STRUCTURE_BAR_CODE_OFFSET1;
	 int offset2 = TS_NV_STRUCTURE_BAR_CODE_OFFSET2;
	 u8 type = 0;
	 u8 len = 0;

	 TS_LOG_INFO("%s enter\n", __func__);

	type = info->buff[offset1*16 + 0];
	len = info->buff[offset1*16 + 1];
	if ((type == 0x00 && len == 0x00) ||(type == 0xFF && len == 0xFF)) {
		 memcpy(&(info->buff[offset1*16]), tp_type_cmd, tp_type_cmd[1]*16);
		 TS_LOG_INFO("Will write the data to info_buff, offset is %d", offset1);
		 return retval;
	 }

	type = info->buff[offset2*16 + 0];
	len = info->buff[offset2*16 + 1];
	if ((type == 0x00 && len == 0x00) ||(type == 0xFF && len == 0xFF)) {
		 memcpy(&(info->buff[offset2*16]), tp_type_cmd, tp_type_cmd[1]*16);
		 TS_LOG_INFO("Will write the data to info_buff, offset is %d", offset2);
		 return retval;
	 }

	TS_LOG_INFO("%s barcode data is full, could not write into the data\n", __func__);
	tp_result_info[0] = TS_CHIP_NO_FLASH_ERROR;
	retval = -EINVAL;
	 return retval;
 }

static int novatek_reconstruct_brightness(struct ts_oem_info_param *info)
{
	int retval = NO_ERR;
	int offset1 = TS_NV_STRUCTURE_BRIGHTNESS_OFFSET1;
	int offset2 = TS_NV_STRUCTURE_BRIGHTNESS_OFFSET2;
	u8 type = 0;
	u8 len = 0;

	TS_LOG_INFO("%s enter\n", __func__);

	type = info->buff[offset1*16 + 0];
	len = info->buff[offset1*16 + 1];
	if ((type == 0x00 && len == 0x00) ||(type == 0xFF && len == 0xFF)) {
		 memcpy(&(info->buff[offset1*16]), tp_type_cmd, tp_type_cmd[1]*16);
		 TS_LOG_INFO("Will write the data to info_buff, offset is %d", offset1);
		 return retval;
	}

	type = info->buff[offset2*16 + 0];
	len = info->buff[offset2*16 + 1];
	if ((type == 0x00 && len == 0x00) ||(type == 0xFF && len == 0xFF)) {
		 memcpy(&(info->buff[offset2*16]), tp_type_cmd, tp_type_cmd[1]*16);
		 TS_LOG_INFO("Will write the data to info_buff, offset is %d", offset2);
		 return retval;
	 }

	 TS_LOG_INFO("%s brightness data is full, could not write into the data\n", __func__);
	 tp_result_info[0] = TS_CHIP_NO_FLASH_ERROR;
	 retval = -EINVAL;
	 return retval;
}

static int novatek_reconstruct_whitepoint(struct ts_oem_info_param *info)
{
	int retval = NO_ERR;
	int offset1 = TS_NV_STRUCTURE_WHITE_POINT_OFFSET1;
	int offset2 = TS_NV_STRUCTURE_WHITE_POINT_OFFSET2;
	u8 type = 0;
	u8 len = 0;

	TS_LOG_INFO("%s enter\n", __func__);

	type = info->buff[offset1*16 + 0];
	len = info->buff[offset1*16 + 1];
	if ((type == 0x00 && len == 0x00) ||(type == 0xFF && len == 0xFF)) {
		memcpy(&(info->buff[offset1*16]), tp_type_cmd, tp_type_cmd[1]*16);
		TS_LOG_INFO("Will write the data to info_buff, offset is %d", offset1);
		return retval;
	}

	type = info->buff[offset2*16 + 0];
	len = info->buff[offset2*16 + 1];
	if ((type == 0x00 && len == 0x00) ||(type == 0xFF && len == 0xFF)) {
		memcpy(&(info->buff[offset2*16]), tp_type_cmd, tp_type_cmd[1]*16);
		TS_LOG_INFO("Will write the data to info_buff, offset is %d", offset2);
		return retval;
	}

	TS_LOG_INFO("%s white point is full, could not write into the data\n", __func__);
	tp_result_info[0] = TS_CHIP_NO_FLASH_ERROR;
	retval = -EINVAL;
	return retval;
 }

static int novatek_reconstruct_brightness_whitepoint(struct ts_oem_info_param *info)
{
	 int retval = NO_ERR;
	 TS_LOG_INFO("%s No Flash defined in NV structure\n", __func__);
	 tp_result_info[0] = TS_CHIP_NO_FLASH_ERROR;
	 return retval;
}

static int novatek_reconstruct_repair_recode(struct ts_oem_info_param *info)
{
	int retval = NO_ERR;
	int offset = TS_NV_STRUCTURE_REPAIR_OFFSET1;
	u8 type = 0;
	u8 len = 0;

	TS_LOG_INFO("%s enter\n", __func__);

	for(;offset <= TS_NV_STRUCTURE_REPAIR_OFFSET5; ++offset) {
		type = info->buff[offset*16 + 0];
		len = info->buff[offset*16 + 1];

		if ((type == 0x00 && len == 0x00) ||(type == 0xFF && len == 0xFF)) {
			memcpy(&(info->buff[offset*16]), tp_type_cmd, tp_type_cmd[1]*16);
			printk("Will write the data to info_buff, offset is %d", offset);
			break;
		} else if( offset == TS_NV_STRUCTURE_REPAIR_OFFSET5 ) {
			TS_LOG_INFO("%s repaire recode is full, could not write into the data\n", __func__);
			tp_result_info[0] = TS_CHIP_NO_FLASH_ERROR;
			retval = -EINVAL;
		}
	}
	return retval;
}

static int novatek_reconstruct_NVstructure(struct ts_oem_info_param *info)
{
	int retval = NO_ERR;

	TS_LOG_INFO("%s called\n", __func__);
	TS_LOG_INFO("%s  info->data[0]:%2x\n", __func__, info->data[0]);

	TS_LOG_INFO("%s  itp_type_cmd[0]:%2x\n", __func__, tp_type_cmd[0]);
	switch (tp_type_cmd[0]) {
	case TS_NV_STRUCTURE_BAR_CODE:
		retval = novatek_reconstruct_barcode(info);
		if (retval != 0) {
			TS_LOG_ERR("%s, parade_reconstruct_barcode faild\n",
				   __func__);
		}
		break;
	case TS_NV_STRUCTURE_BRIGHTNESS:
		retval = novatek_reconstruct_brightness(info);
		if (retval != 0) {
			TS_LOG_ERR("%s, parade_reconstruct_barcode faild\n",
				   __func__);
		}
		break;
	case TS_NV_STRUCTURE_WHITE_POINT:
		retval = novatek_reconstruct_whitepoint(info);
		if (retval != 0) {
			TS_LOG_ERR("%s, parade_reconstruct_barcode faild\n",
				   __func__);
		}
		break;
	case TS_NV_STRUCTURE_BRI_WHITE:
		retval = novatek_reconstruct_brightness_whitepoint(info);
		if (retval != 0) {
			TS_LOG_ERR("%s, parade_reconstruct_barcode faild\n",
				   __func__);
		}
		break;
	case TS_NV_STRUCTURE_REPAIR:
		retval = novatek_reconstruct_repair_recode(info);
		if (retval != 0) {
			TS_LOG_ERR("%s, parade_reconstruct_barcode faild\n",
				   __func__);
		}
		break;
	default:
		TS_LOG_INFO("invalid NV structure type=%d\n",
				info->data[0]);
		retval = -EINVAL;
		break;
	}

	TS_LOG_INFO("%s end", __func__);
	return retval;
 }

static int novatek_get_NVstructure_cur_index(struct ts_oem_info_param *info, u8 type)
{
	int index = 0;
	int latest_index = 0;
	int flash_size = novatek_get_oem_data_info();

	for ((0x5 == type)? (index = TS_NV_STRUCTURE_REPAIR_OFFSET1) : (index = 1); index < flash_size/16; ++index) {
		u8 tmp_type = info->buff[index*16];
		if (tmp_type == type) {
			latest_index = index;
		}
	}

	return latest_index;
}

static int novatek_get_NVstructure_index(struct ts_oem_info_param *info, u8 type)
{
	int index = 0;
	int latest_index = 0;
	int flash_size = novatek_get_oem_data_info();
	int count = 0;

	for ((TS_NV_STRUCTURE_REPAIR == type)? (index = TS_NV_STRUCTURE_REPAIR_OFFSET1) : (index = 1);
		index < flash_size/16; ++index) {
		u8 tmp_type = info->buff[index*16];
		if (tmp_type == type) {
			latest_index = index;
			count += 1;
		}
	}

	if(type == TS_NV_STRUCTURE_REPAIR) {
		info->length = count;
		if(info->length) {
			latest_index = TS_NV_STRUCTURE_REPAIR_OFFSET1;
		}
	}else{
		info->length = info->buff[latest_index*16+1];
	}

	return latest_index;
}

static int novatek_set_oem_info(struct ts_oem_info_param *info)
{
	u8 type_reserved = TS_CHIP_TYPE_RESERVED;
	u8 len_reserved = TS_CHIP_TYPE_LEN_RESERVED;
	int flash_size = 0;
	int used_size = 16;
	int error = NO_ERR;
	u8 type = 0;
	u8 len = 0;
	int i = 0;

	TS_LOG_INFO("%s called\n", __func__);
	flash_size = novatek_get_oem_data_info();
	type = info->data[0];
	len  = info->data[1];
	used_size += len * 16;

	memset(tp_result_info, 0x0, TS_CHIP_TYPE_MAX_SIZE);

	//check type and len below
	TS_LOG_ERR("%s write Type=0x%2x , type data len=%d\n", __func__, type, len);
	if (type == 0x0 || type > type_reserved ) {
		TS_LOG_ERR("%s write Type=0x%2x is RESERVED\n", __func__, type);
		error = EINVAL;
		goto out;
	}

	if ( len > len_reserved ) {
		TS_LOG_ERR("%s TPIC write RESERVED NV STRUCT len\n", __func__);
		error = EINVAL;
		goto out;
	}
	//just store the data in tp_type_cmd buff
	if (len == 0x0) {
		tp_type_cmd[0] = info->data[0];
		TS_LOG_INFO("%s Just store type:%2x and then finished\n", __func__, info->data[0]);
		return error;
	}

	if (strlen(info->data) <= TS_CHIP_TYPE_MAX_SIZE) {
		memset(tp_type_cmd, 0x0, TS_CHIP_TYPE_MAX_SIZE);
		memcpy(tp_type_cmd, info->data, len*16);
	} else {
		error = EINVAL;
		TS_LOG_INFO("%s: invalid test cmd:%s\n", __func__, info->data);
		return error;
	}

	error = novatek_get_oem_data(info->buff, flash_size);
	if (error < 0) {
		TS_LOG_ERR("%s: get oem data failed,fail line=%d\n", __func__,
			   __LINE__);
		tp_result_info[0] = TS_CHIP_READ_ERROR;
		goto out;
	}

	TS_LOG_INFO("%s: Read data from TPIC flash is below\n", __func__);
	for(i = 0; i< flash_size/16; ++i ){
		TS_LOG_INFO("%s: %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x \n", __func__,
			info->buff[0+i*16],info->buff[1+i*16],info->buff[2+i*16],info->buff[3+i*16],
			info->buff[4+i*16],info->buff[5+i*16],info->buff[6+i*16],info->buff[7+i*16],
			info->buff[8+i*16],info->buff[9+i*16],info->buff[10+i*16],info->buff[11+i*16],
			info->buff[12+i*16],info->buff[13+i*16],info->buff[14+i*16],info->buff[15+i*16]
		);
	}

	TS_LOG_INFO("%s: use new oem structure\n", __func__);
	if (NO_ERR != novatek_reconstruct_NVstructure(info)){
		TS_LOG_ERR("%s: novatek_reconstruct_NVstructure fail\n", __func__ );
		goto out;
	}

	TS_LOG_INFO("%s: Add write type data into buff below\n", __func__);
	for(i = 0; i< flash_size/16; ++i ){
		TS_LOG_INFO("%s: %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x \n", __func__,
			info->buff[0+i*16],info->buff[1+i*16],info->buff[2+i*16],info->buff[3+i*16],
			info->buff[4+i*16],info->buff[5+i*16],info->buff[6+i*16],info->buff[7+i*16],
			info->buff[8+i*16],info->buff[9+i*16],info->buff[10+i*16],info->buff[11+i*16],
			info->buff[12+i*16],info->buff[13+i*16],info->buff[14+i*16],info->buff[15+i*16]
		);
	}

	//Write buffer into TPIC flash
	error = novatek_set_oem_data(info->buff, flash_size);
	if (error < 0) {
		TS_LOG_ERR("%s: get oem data failed\n", __func__);
		tp_result_info[0] = TS_CHIP_WRITE_ERROR;
		goto out;
	}

	//check the write data.
	memset(info->buff, 0, TS_CHIP_BUFF_MAX_SIZE);
	error = novatek_get_oem_data(info->buff, flash_size);
	if (error < 0) {
		TS_LOG_ERR("%s: get oem data failed,fail line=%d\n", __func__,
			   __LINE__);
		tp_result_info[0] = TS_CHIP_READ_ERROR;
		goto out;
	}

	TS_LOG_INFO("%s: Read back buff data below\n", __func__);
	for(i = 0; i< flash_size/16; ++i ){
		TS_LOG_INFO("%s: %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x \n", __func__,
			info->buff[0+i*16],info->buff[1+i*16],info->buff[2+i*16],info->buff[3+i*16],
			info->buff[4+i*16],info->buff[5+i*16],info->buff[6+i*16],info->buff[7+i*16],
			info->buff[8+i*16],info->buff[9+i*16],info->buff[10+i*16],info->buff[11+i*16],
			info->buff[12+i*16],info->buff[13+i*16],info->buff[14+i*16],info->buff[15+i*16]
		);
	}

	int latest_index = 0;
	latest_index = novatek_get_NVstructure_cur_index(info, type);
	if (!latest_index){
		TS_LOG_ERR("%s: set oem data find current line fail line=%d\n", __func__,
			   __LINE__);
		tp_result_info[0] = TS_CHIP_WRITE_ERROR;
		goto out;
	}
	used_size =  latest_index * 16;

	TS_LOG_INFO("%s: CHECK:buff from TPIC\n", __func__);
	for(i = latest_index; i<(latest_index + len); ++i ){
		TS_LOG_INFO("%s: %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x \n", __func__,
			info->buff[0+i*16],info->buff[1+i*16],info->buff[2+i*16],info->buff[3+i*16],
			info->buff[4+i*16],info->buff[5+i*16],info->buff[6+i*16],info->buff[7+i*16],
			info->buff[8+i*16],info->buff[9+i*16],info->buff[10+i*16],info->buff[11+i*16],
			info->buff[12+i*16],info->buff[13+i*16],info->buff[14+i*16],info->buff[15+i*16]
		);
	}
	TS_LOG_INFO("%s: CHECK:tp_type_cmd from PC\n", __func__);
	for(i = 0; i< len; ++i ){
		TS_LOG_INFO("%s: %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x \n", __func__,
			tp_type_cmd[0+i*16],tp_type_cmd[1+i*16],tp_type_cmd[2+i*16],tp_type_cmd[3+i*16],
			tp_type_cmd[4+i*16],tp_type_cmd[5+i*16],tp_type_cmd[6+i*16],tp_type_cmd[7+i*16],
			tp_type_cmd[8+i*16],tp_type_cmd[9+i*16],tp_type_cmd[10+i*16],tp_type_cmd[11+i*16],
			tp_type_cmd[12+i*16],tp_type_cmd[13+i*16],tp_type_cmd[14+i*16],tp_type_cmd[15+i*16]
		);
	}

	error = strncmp(&(info->buff[used_size]), tp_type_cmd, len*16);
	if( error ) {
		tp_result_info[0] = TS_CHIP_WRITE_ERROR;
		TS_LOG_ERR("%s: Write type data has some error\n", __func__);
	}

out:
	memset(tp_type_cmd, 0x0, TS_CHIP_TYPE_MAX_SIZE);
	TS_LOG_INFO("%s End\n", __func__);
	return error;

}

static int novatek_get_oem_info(struct ts_oem_info_param *info)
{
	u8 type_reserved = TS_CHIP_TYPE_RESERVED;
	u8 type = tp_type_cmd[0];
	unsigned short flash_size = 0;
	int error = NO_ERR;
	int latest_index = 0;
	int i;

	TS_LOG_INFO("%s called\n", __func__);

	memset(info->data, 0x0, TS_CHIP_TYPE_MAX_SIZE);
	flash_size = novatek_get_oem_data_info();
	if(flash_size <= 0) {
		TS_LOG_ERR("%s: Could not get TPIC flash size,fail line=%d\n", __func__,
			   __LINE__);
		error = EINVAL;
		goto out;
	}

	//return the result info if type is 0x0
	if(type == 0x0) {
		memcpy(info->data, tp_result_info, TS_CHIP_TYPE_MAX_SIZE);
		TS_LOG_INFO("%s:Reurn the write result=%2x to sys node.\n", __func__, info->data[0]);
		goto out;
	}

	//check type
	TS_LOG_INFO("%s: store type=%2x\n", __func__, type);
	if (type > type_reserved) {
		TS_LOG_ERR("%s Read Type=0x%2x is RESERVED\n", __func__, type);
		error = EINVAL;
		goto out;
	}

	error = novatek_get_oem_data(info->buff, flash_size);
	if (error < 0) {
		TS_LOG_ERR("%s: memory not enough,fail line=%d\n", __func__,
			   __LINE__);
		error = EINVAL;
		goto out;
	}

	TS_LOG_INFO("%s:Get buff data below\n", __func__);
	for(i = 0; i< flash_size/16; ++i ){
		TS_LOG_INFO("%s: %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x \n", __func__,
			info->buff[0+i*16],info->buff[1+i*16],info->buff[2+i*16],info->buff[3+i*16],
			info->buff[4+i*16],info->buff[5+i*16],info->buff[6+i*16],info->buff[7+i*16],
			info->buff[8+i*16],info->buff[9+i*16],info->buff[10+i*16],info->buff[11+i*16],
			info->buff[12+i*16],info->buff[13+i*16],info->buff[14+i*16],info->buff[15+i*16]
		);
	}

	TS_LOG_INFO("%s: use new oem structure\n", __func__);
	//scan each NV struct length
	latest_index = novatek_get_NVstructure_index(info, type);
	TS_LOG_INFO("%s get type:0x%2x  index = %d\n", __func__, type, latest_index);

	if (latest_index) {
		TS_LOG_INFO("%s type data find. len = %d\n", __func__, info->length);
		memcpy(info->data, &(info->buff[latest_index*16]), info->length*16 );
	} else {
		info->data[0] = 0x1;
		TS_LOG_INFO("%s No type data find. info->data[0] = %2x\n", __func__, info->data[0]);
	}

out:
	TS_LOG_INFO("%s End\n", __func__);
	return error;
}

static int novatek_oem_info_switch(struct ts_oem_info_param *info)
{
	int retval = NO_ERR;

	if (!info) {
		TS_LOG_ERR("novatek_oem_info_switch: info is Null\n");
		retval = -ENOMEM;
		return retval;
	}

	switch (info->op_action) {
	case TS_ACTION_WRITE:
		retval = novatek_set_oem_info(info);
		if (retval != 0) {
			TS_LOG_ERR("%s, novatek_oem_info_switch faild\n",
				   __func__);
		}
		break;
	case TS_ACTION_READ:
		retval = novatek_get_oem_info(info);
		if (retval != 0) {
			TS_LOG_ERR("%s, novatek_get_oem_info faild\n",
				   __func__);
		}
		break;
	default:
		TS_LOG_INFO("invalid oem info switch(%d) action: %d\n",
				info->data_switch, info->op_action);
		retval = -EINVAL;
		break;
	}
	return retval;
}

static int novatek_get_brightness_info(void)
{
	int error = NO_ERR;
	int bl_max_nit = 0;
	struct ts_oem_info_param *info =NULL;
	TS_LOG_INFO("%s: Enter\n", __func__);

	//setting the read brightness type
	memset(tp_type_cmd, 0x0, TS_CHIP_TYPE_MAX_SIZE);
	tp_type_cmd[0] = TS_CHIP_BRIGHTNESS_TYPE;

	if (TS_UNINIT == atomic_read(&g_ts_data.state)) {
		TS_LOG_INFO("%s:ts module not initialize\n", __func__);
		bl_max_nit = 0;
		return	bl_max_nit;
	}

	info =
	(struct ts_oem_info_param *)
		kzalloc(sizeof(struct ts_oem_info_param), GFP_KERNEL);
	if (!info) {
		TS_LOG_ERR("%s: malloc failed\n", __func__);
		error = -ENOMEM;
	goto out;
	}

	novatek_get_oem_info( info);

	if(info->data[0] == 0x1) {
		TS_LOG_INFO("%s:brightness info is not find in TPIC FLASH\n", __func__);
		bl_max_nit = 0;
	} else {
		TS_LOG_INFO("%s:brightness info find in TPIC FLASH\n", __func__);
		bl_max_nit = info->data[3] << 8 | info->data[2];
	}

out:
	return bl_max_nit;
}

struct ts_device_ops ts_novatek_ops = {
	.chip_parse_config = novatek_parse_dts,
	.chip_detect = novatek_chip_detect,
	.chip_init = novatek_init,

	.chip_input_config = novatek_input_config,

	.chip_shutdown = novatek_shutdown,

	.chip_irq_top_half = novatek_irq_top_half,
	.chip_irq_bottom_half = novatek_irq_bottom_half,

	.chip_before_suspend = novatek_before_suspend,
	.chip_suspend = novatek_suspend,
	.chip_resume = novatek_resume,
	.chip_after_resume = novatek_after_resume,

	//---return fixed strings, Taylor 20160627---	
	.chip_get_info = novatek_get_info,
	//-------------------------------------------	

	//---not tested yet in sz, Taylor 20160506---
	//---revise and test in sh, Taylor 20160623---
	.chip_fw_update_boot = novatek_fw_update_boot,
	.chip_fw_update_sd = novatek_fw_update_sd,
	//-------------------------------------------

	//---not tested yet in sz, Taylor 20160506---
	.chip_get_rawdata = novatek_get_rawdata,
	.chip_get_capacitance_test_type = novatek_get_capacitance_test_type,
	//-------------------------------------------	


	//---not finished yet in sz, Taylor 20160506---	
	//---revise and test in sh, Taylor 20160624---
//	.chip_wakeup_gesture_enable_switch = novatek_wakeup_gesture_enable_switch,
	.chip_glove_switch = novatek_glove_switch,
	.chip_palm_switch =novatek_palm_switch,
	.chip_holster_switch = novatek_holster_switch,
	.chip_roi_switch = novatek_roi_switch,
	.chip_roi_rawdata = novatek_roi_rawdata,	
	//---------------------------------------------
	.chip_ghost_detect = novatek_ghost_detect,
	.chip_check_status = novatek_chip_check_status,
	.chip_get_brightness_info = novatek_get_brightness_info,
	.oem_info_switch = novatek_oem_info_switch,
};

