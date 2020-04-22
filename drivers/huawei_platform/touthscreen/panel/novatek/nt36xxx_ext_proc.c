/*
 * Copyright (C) 2010 - 2016 Novatek, Inc.
 *
 * $Revision: 4017 $
 * $Date: 2016-04-01 09:41:08 +0800 (星期五, 01 四月 2016) $
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

#include <linux/proc_fs.h>
#include <linux/unistd.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#include <asm/uaccess.h>
#include <linux/delay.h>
#include <../../huawei_touchscreen_chips.h>
#include <linux/regulator/consumer.h>
#include <huawei_platform/log/log_jank.h>
#include "../../huawei_touchscreen_algo.h"
#if defined (CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

#include "nt36xxx.h"


#define NVT_FW_VERSION "nvt_fw_version"
#define NVT_BASELINE "nvt_baseline"
#define NVT_RAW "nvt_raw"
#define NVT_DIFF "nvt_diff"

#define I2C_TANSFER_LENGTH  64

#define NORMAL_MODE 0x00
#define TEST_MODE_1 0x21
#define TEST_MODE_2 0x22

#define RAW_PIPE0_ADDR  0x10000
#define RAW_PIPE1_ADDR  0x12000
#define BASELINE_ADDR   0x10E70
#define DIFF_PIPE0_ADDR 0x10830
#define DIFF_PIPE1_ADDR 0x12830

#define RAW_BTN_PIPE0_ADDR  0x10E60
#define RAW_BTN_PIPE1_ADDR  0x12E60
#define BASELINE_BTN_ADDR   0x12E70
#define DIFF_BTN_PIPE0_ADDR 0x10E68
#define DIFF_BTN_PIPE1_ADDR 0x12E68

#define XDATA_SECTOR_SIZE   256

static uint8_t xdata_tmp[2048] = {0};
static int32_t xdata[2048] = {0};
uint8_t fw_ver = 0;
static uint8_t x_num = 0;
static uint8_t y_num = 0;
static uint8_t button_num = 0;

static struct proc_dir_entry *NVT_proc_fw_version_entry;
static struct proc_dir_entry *NVT_proc_baseline_entry;
static struct proc_dir_entry *NVT_proc_raw_entry;
static struct proc_dir_entry *NVT_proc_diff_entry;

extern struct nvt_ts_data *ts;
extern int32_t novatek_ts_i2c_read(struct i2c_client *client, uint16_t i2c_addr, uint8_t *buf, uint16_t len);
extern int32_t novatek_ts_i2c_dummy_read(struct i2c_client *client, uint16_t i2c_addr);
extern int32_t novatek_ts_i2c_write(struct i2c_client *client, uint16_t i2c_addr, uint8_t *buf, uint16_t len);
extern int32_t nvt_clear_fw_status(void);
extern int32_t nvt_check_fw_status(void);
extern struct ts_data g_ts_data;



/*******************************************************
  Create Device Node (Proc Entry)
*******************************************************/
#if NVT_TOUCH_PROC
static struct proc_dir_entry *NVT_proc_entry;
#define DEVICE_NAME	"NVTflash"

/*******************************************************
Description:
	Novatek touchscreen /proc/NVTflash read function.

return:
	Executive outcomes. 2---succeed. -5,-14---failed.
*******************************************************/
static ssize_t nvt_flash_read(struct file *file, char __user *buff, size_t count, loff_t *offp)
{
	uint8_t str[68] = {0};
	int32_t ret = -1;
	int32_t retries = 0;
	int8_t i2c_wr = 0;
	u16 tmp_addr = 0;

	if (count > sizeof(str))
		return -EFAULT;

	if (copy_from_user(str, buff, count))
		return -EFAULT;

	i2c_wr = str[0] >> 7;

	if (i2c_wr == 0) {	//I2C write
		TS_LOG_DEBUG("%s: i2c_addr=0x%02X, len=%d\n", __func__, (str[0] & 0x7F), str[1]);
		
		if (!ts->chip_data->bops->bus_read) {
			TS_LOG_ERR("%s: error, invalid bus_write\n", __func__);
			return -EIO;
		}
		
		mutex_lock(&ts->i2c_mutex);
		tmp_addr = g_ts_data.client->addr;
		g_ts_data.client->addr = (str[0] & 0x7F);
		
		ret = ts->chip_data->bops->bus_write(&str[2], str[1]);
		if (ret < 0)
			TS_LOG_ERR("%s: error, bus_write fail, ret=%d\n", __func__, ret);
		
		g_ts_data.client->addr = tmp_addr;
		mutex_unlock(&ts->i2c_mutex);

		return ret;
	} else if (i2c_wr == 1) {	//I2C read	
		TS_LOG_DEBUG("%s: i2c_addr=0x%02X, len=%d\n", __func__, (str[0] & 0x7F), str[1]);
	
		if (!ts->chip_data->bops->bus_read) {
			TS_LOG_ERR("%s: error, invalid bus_read\n", __func__);
			return -EIO;
		}
	
		mutex_lock(&ts->i2c_mutex);
		tmp_addr = g_ts_data.client->addr;
		g_ts_data.client->addr = (str[0] & 0x7F);
	
		ret = ts->chip_data->bops->bus_read(&str[2], 1, &str[3], (str[1] - 1));
		if (ret < 0)
			TS_LOG_ERR("%s: error, bus_read fail, ret=%d\n", __func__, ret);
	
		g_ts_data.client->addr = tmp_addr;
		mutex_unlock(&ts->i2c_mutex);
	
		// copy buff to user if i2c transfer
		if (retries < 20) {
			if (copy_to_user(buff, str, count))
				return -EFAULT;
		}

		return ret;
	} else {
		TS_LOG_ERR("%s: Call error, str[0]=%d\n", __func__, str[0]);
		return -EFAULT;
	}
}

/*******************************************************
Description:
	Novatek touchscreen /proc/NVTflash open function.

return:
	Executive outcomes. 0---succeed. -12---failed.
*******************************************************/
static int32_t nvt_flash_open(struct inode *inode, struct file *file)
{
	struct nvt_flash_data *dev;

	dev = kmalloc(sizeof(struct nvt_flash_data), GFP_KERNEL);
	if (dev == NULL) {
		TS_LOG_ERR("%s: Failed to allocate memory for nvt flash data\n", __func__);
		return -ENOMEM;
	}

	rwlock_init(&dev->lock);
	file->private_data = dev;

	return 0;
}

/*******************************************************
Description:
	Novatek touchscreen /proc/NVTflash close function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int32_t nvt_flash_close(struct inode *inode, struct file *file)
{
	struct nvt_flash_data *dev = file->private_data;

	if (dev)
		kfree(dev);

	return 0;
}

static const struct file_operations nvt_flash_fops = {
	.owner = THIS_MODULE,
	.open = nvt_flash_open,
	.release = nvt_flash_close,
	.read = nvt_flash_read,
};

/*******************************************************
Description:
	Novatek touchscreen /proc/NVTflash initial function.

return:
	Executive outcomes. 0---succeed. -12---failed.
*******************************************************/
int32_t nvt_flash_proc_init(void)
{
	NVT_proc_entry = proc_create(DEVICE_NAME, 0444, NULL,&nvt_flash_fops);
	if (NVT_proc_entry == NULL) {
		TS_LOG_ERR("%s: Failed!\n", __func__);
		return -ENOMEM;
	} else {
		TS_LOG_INFO("%s: Succeeded!\n", __func__);
	}

	TS_LOG_INFO("============================================================\n");
	TS_LOG_INFO("Create /proc/NVTflash\n");
	TS_LOG_INFO("============================================================\n");

	return 0;
}
#endif



/*******************************************************
  Create Device Node (Ext Proc Entry)
*******************************************************/
#if NVT_TOUCH_EXT_PROC
/*******************************************************
Description:
	Novatek touchscreen change mode function.

return:
	n.a.
*******************************************************/
void nvt_change_mode(uint8_t mode)
{
	uint8_t buf[8] = {0};

	//---set xdata index to 0x11E00---
	buf[0] = 0xFF;
	buf[1] = 0x01;
	buf[2] = 0x1E;
	novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);

	//---set mode---
	buf[0] = 0x50;
	buf[1] = mode;
	novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);

	if (mode == NORMAL_MODE) {
		buf[0] = 0x51;
		buf[1] = 0xBB;
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);
	}
}

/*******************************************************
Description:
	Novatek touchscreen switch no PD function.

return:
	Executive outcomes. 0---success. -1---fail.
*******************************************************/
int8_t nvt_switch_noPD(uint8_t noPD)
{
	uint8_t buf[8] = {0};
	uint8_t retry = 0;
	int8_t ret = 0;

	for (retry = 0; retry < 5; retry++) {
		//---set xdata index to 0x11E00---
		buf[0] = 0xFF;
		buf[1] = 0x01;
		buf[2] = 0x1E;
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);

		//---switch noPD---
		buf[0] = 0x50;
		buf[1] = noPD;
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);

		msleep(20);

		buf[0] = 0x50;
		buf[1] = 0xFF;
		novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 2);

		if (buf[1] == 0x00)
			break;

		msleep(10);
	}

	if (unlikely(retry == 5)) {
		TS_LOG_ERR("%s: switch noPD 0x%02X failed\n", __func__, noPD);
		ret = -1;
	}

	return ret;
}

/*******************************************************
Description:
	Novatek touchscreen get firmware related information
	function.

return:
	Executive outcomes. 0---success. -1---fail.
*******************************************************/
int8_t nvt_get_fw_info(void)
{
	uint8_t buf[64] = {0};
	uint32_t retry_count = 0;
	int8_t ret = 0;

info_retry:
	//---set xdata index to 0x11E00---
	buf[0] = 0xFF;
	buf[1] = 0x01;
	buf[2] = 0x1E;
	novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);

	//---read fw info---
	buf[0] = 0x78;
	novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 17);
	fw_ver = buf[1];
	x_num = buf[3];
	y_num = buf[4];
	button_num = buf[11];

	//---clear x_num, y_num if fw info is broken---
	if ((buf[1] + buf[2]) != 0xFF) {
		TS_LOG_ERR("%s: FW info is broken! fw_ver=0x%02X, ~fw_ver=0x%02X\n", __func__, buf[1], buf[2]);
		x_num = 0;
		y_num = 0;
		button_num = 0;

		if(retry_count < 3) {
			retry_count++;
			TS_LOG_ERR("%s: retry_count=%d\n", __func__, retry_count);
			goto info_retry;
		} else {
			ret = -1;
		}
	} else {
		ret = 0;
	}

	return ret;
}

/*******************************************************
Description:
	Novatek touchscreen get firmware pipe function.

return:
	Executive outcomes. 0---pipe 0. 1---pipe 1.
*******************************************************/
uint8_t nvt_get_fw_pipe(void)
{
	uint8_t buf[8]= {0};

	//---set xdata index to 0x11E00---
	buf[0] = 0xFF;
	buf[1] = 0x01;
	buf[2] = 0x1E;
	novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);

	//---read fw status---
	buf[0] = 0x51;
	buf[1] = 0x00;
	novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 2);

	//TS_LOG_INFO("FW pipe=%d, buf[1]=0x%02X\n", (buf[1]&0x01), buf[1]);

	return (buf[1] & 0x01);
}

/*******************************************************
Description:
	Novatek touchscreen read meta data function.

return:
	n.a.
*******************************************************/
void nvt_read_mdata(uint32_t xdata_addr, uint32_t xdata_btn_addr)
{
	int32_t i = 0;
	int32_t j = 0;
	int32_t k = 0;
	uint8_t buf[I2C_TANSFER_LENGTH + 1] = {0};
	uint32_t head_addr = 0;
	int32_t dummy_len = 0;
	int32_t data_len = 0;
	int32_t residual_len = 0;

	//---set xdata sector address & length---
	head_addr = xdata_addr - (xdata_addr % XDATA_SECTOR_SIZE);
	dummy_len = xdata_addr - head_addr;
	data_len = x_num * y_num * 2;
	residual_len = (head_addr + dummy_len + data_len) % XDATA_SECTOR_SIZE;

	//printk("head_addr=0x%05X, dummy_len=0x%05X, data_len=0x%05X, residual_len=0x%05X\n", head_addr, dummy_len, data_len, residual_len);

	//read xdata : step 1
	for (i = 0; i < ((dummy_len + data_len) / XDATA_SECTOR_SIZE); i++) {
		//---change xdata index---
		buf[0] = 0xFF;
		buf[1] = ((head_addr + XDATA_SECTOR_SIZE * i) >> 16);
		buf[2] = ((head_addr + XDATA_SECTOR_SIZE * i) >> 8) & 0xFF;
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);

		//---read xdata by I2C_TANSFER_LENGTH
		for (j = 0; j < (XDATA_SECTOR_SIZE / I2C_TANSFER_LENGTH); j++) {
			//---read data---
			buf[0] = I2C_TANSFER_LENGTH * j;
			novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, I2C_TANSFER_LENGTH + 1);

			//---copy buf to xdata_tmp---
			for (k = 0; k < I2C_TANSFER_LENGTH; k++) {
				xdata_tmp[XDATA_SECTOR_SIZE * i + I2C_TANSFER_LENGTH * j + k] = buf[k + 1];
				//printk("0x%02X, 0x%04X\n", buf[k+1], (XDATA_SECTOR_SIZE*i + I2C_TANSFER_LENGTH*j + k));
			}
		}
		//printk("addr=0x%05X\n", (head_addr+XDATA_SECTOR_SIZE*i));
	}

	//read xdata : step2
	if (residual_len != 0) {
		//---change xdata index---
		buf[0] = 0xFF;
		buf[1] = ((xdata_addr + data_len - residual_len) >> 16);
		buf[2] = ((xdata_addr + data_len - residual_len) >> 8) & 0xFF;
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);

		//---read xdata by I2C_TANSFER_LENGTH
		for (j = 0; j < (residual_len / I2C_TANSFER_LENGTH + 1); j++) {
			//---read data---
			buf[0] = I2C_TANSFER_LENGTH * j;
			novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, I2C_TANSFER_LENGTH + 1);

			//---copy buf to xdata_tmp---
			for (k = 0; k < I2C_TANSFER_LENGTH; k++) {
				xdata_tmp[(dummy_len + data_len - residual_len) + I2C_TANSFER_LENGTH * j + k] = buf[k + 1];
				//printk("0x%02X, 0x%04x\n", buf[k+1], ((dummy_len+data_len-residual_len) + I2C_TANSFER_LENGTH*j + k));
			}
		}
		//printk("addr=0x%05X\n", (xdata_addr+data_len-residual_len));
	}

	//---remove dummy data and 2bytes-to-1data---
	for (i = 0; i < (data_len / 2); i++) {
		xdata[i] = (xdata_tmp[dummy_len + i * 2] + 256 * xdata_tmp[dummy_len + i * 2 + 1]);
	}

#if TOUCH_KEY_NUM > 0
		//read button xdata : step3
		//---change xdata index---
		buf[0] = 0xFF;
		buf[1] = (xdata_btn_addr >> 16);
		buf[2] = ((xdata_btn_addr >> 8) & 0xFF);
		CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

		//---read data---
		buf[0] = (xdata_btn_addr & 0xFF);
		CTP_I2C_READ(ts->client, I2C_FW_Address, buf, (TOUCH_KEY_NUM * 2 + 1));

		//---2bytes-to-1data---
		for (i = 0; i < TOUCH_KEY_NUM; i++) {
			xdata[x_num * y_num + i] = (buf[1 + i * 2] + 256 * buf[1 + i * 2 + 1]);
		}
#endif

	//---set xdata index to 0x11E00---
	buf[0] = 0xFF;
	buf[1] = 0x01;
	buf[2] = 0x1E;
	novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);
}

/*******************************************************
Description:
    Novatek touchscreen get meta data function.

return:
    n.a.
*******************************************************/
void nvt_get_mdata(int32_t *buf, uint8_t *m_x_num, uint8_t *m_y_num)
{
    *m_x_num = x_num;
    *m_y_num = y_num;
    memcpy(buf, xdata, (x_num * y_num * 4));
}

/*******************************************************
Description:
	Novatek touchscreen firmware version show function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int32_t c_fw_version_show(struct seq_file *m, void *v)
{
	seq_printf(m, "fw_ver=%d, x_num=%d, y_num=%d, button_num=%d\n", fw_ver, x_num, y_num, button_num);
	return 0;
}

/*******************************************************
Description:
	Novatek touchscreen xdata sequence print show
	function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int32_t c_show(struct seq_file *m, void *v)
{
	int32_t i = 0;
	int32_t j = 0;

	for (i = 0; i < y_num; i++) {
		for (j = 0; j < x_num; j++) {
			seq_printf(m, "%5d, ", (int16_t)xdata[i * x_num + j]);
		}
		seq_puts(m, "\n");
	}
	seq_printf(m, "\n\n");
	return 0;
}

/*******************************************************
Description:
	Novatek touchscreen xdata sequence print start
	function.

return:
	Executive outcomes. 1---call next function.
	NULL---not call next function and sequence loop
	stop.
*******************************************************/
static void *c_start(struct seq_file *m, loff_t *pos)
{
	return *pos < 1 ? (void *)1 : NULL;
}

/*******************************************************
Description:
	Novatek touchscreen xdata sequence print next
	function.

return:
	Executive outcomes. NULL---no next and call sequence
	stop function.
*******************************************************/
static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

/*******************************************************
Description:
	Novatek touchscreen xdata sequence print stop
	function.

return:
	n.a.
*******************************************************/
static void c_stop(struct seq_file *m, void *v)
{
	return;
}

const struct seq_operations nvt_fw_version_seq_ops = {
	.start  = c_start,
	.next   = c_next,
	.stop   = c_stop,
	.show   = c_fw_version_show
};

const struct seq_operations nvt_seq_ops = {
	.start  = c_start,
	.next   = c_next,
	.stop   = c_stop,
	.show   = c_show
};

/*******************************************************
Description:
	Novatek touchscreen /proc/nvt_fw_version open
	function.

return:
	n.a.
*******************************************************/
static int32_t nvt_fw_version_open(struct inode *inode, struct file *file)
{
	if (nvt_get_fw_info())
		return -EAGAIN;

	return seq_open(file, &nvt_fw_version_seq_ops);
}

static const struct file_operations nvt_fw_version_fops = {
	.owner = THIS_MODULE,
	.open = nvt_fw_version_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*******************************************************
Description:
	Novatek touchscreen /proc/nvt_baseline open function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int32_t nvt_baseline_open(struct inode *inode, struct file *file)
{
	if (nvt_clear_fw_status())
		return -EAGAIN;

	nvt_change_mode(TEST_MODE_2);

	if (nvt_check_fw_status())
		return -EAGAIN;

	if (nvt_get_fw_info())
		return -EAGAIN;

	nvt_read_mdata(BASELINE_ADDR, BASELINE_BTN_ADDR);

	nvt_change_mode(NORMAL_MODE);

	return seq_open(file, &nvt_seq_ops);
}

static const struct file_operations nvt_baseline_fops = {
	.owner = THIS_MODULE,
	.open = nvt_baseline_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*******************************************************
Description:
	Novatek touchscreen /proc/nvt_raw open function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int32_t nvt_raw_open(struct inode *inode, struct file *file)
{
	if (nvt_clear_fw_status())
		return -EAGAIN;

	nvt_change_mode(TEST_MODE_2);

	if (nvt_check_fw_status())
		return -EAGAIN;

	if (nvt_get_fw_info())
		return -EAGAIN;

	if (nvt_get_fw_pipe() == 0)
		nvt_read_mdata(RAW_PIPE0_ADDR, RAW_BTN_PIPE0_ADDR);
	else
		nvt_read_mdata(RAW_PIPE1_ADDR, RAW_BTN_PIPE1_ADDR);

	nvt_change_mode(NORMAL_MODE);

	return seq_open(file, &nvt_seq_ops);
}

static const struct file_operations nvt_raw_fops = {
	.owner = THIS_MODULE,
	.open = nvt_raw_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*******************************************************
Description:
	Novatek touchscreen /proc/nvt_diff open function.

return:
	Executive outcomes. 0---succeed. negative---failed.
*******************************************************/
static int32_t nvt_diff_open(struct inode *inode, struct file *file)
{
	if (nvt_clear_fw_status())
		return -EAGAIN;

	nvt_change_mode(TEST_MODE_2);

	if (nvt_check_fw_status())
		return -EAGAIN;

	nvt_get_fw_info();

	if (nvt_get_fw_pipe() == 0)
		nvt_read_mdata(DIFF_PIPE0_ADDR, DIFF_BTN_PIPE0_ADDR);
	else
		nvt_read_mdata(DIFF_PIPE1_ADDR, DIFF_BTN_PIPE1_ADDR);

	nvt_change_mode(NORMAL_MODE);

	return seq_open(file, &nvt_seq_ops);
}

static const struct file_operations nvt_diff_fops = {
	.owner = THIS_MODULE,
	.open = nvt_diff_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*******************************************************
Description:
	Novatek touchscreen extra function proc. file node
	initial function.

return:
	Executive outcomes. 0---succeed. -12---failed.
*******************************************************/
int32_t nvt_extra_proc_init(void)
{
	NVT_proc_fw_version_entry = proc_create(NVT_FW_VERSION, 0444, NULL,&nvt_fw_version_fops);
	if (NVT_proc_fw_version_entry == NULL) {
		TS_LOG_ERR("%s: create proc/nvt_fw_version Failed!\n", __func__);
		return -ENOMEM;
	} else {
		TS_LOG_INFO("%s: create proc/nvt_fw_version Succeeded!\n", __func__);
	}

	NVT_proc_baseline_entry = proc_create(NVT_BASELINE, 0444, NULL,&nvt_baseline_fops);
	if (NVT_proc_baseline_entry == NULL) {
		TS_LOG_ERR("%s: create proc/nvt_baseline Failed!\n", __func__);
		return -ENOMEM;
	} else {
		TS_LOG_INFO("%s: create proc/nvt_baseline Succeeded!\n", __func__);
	}

	NVT_proc_raw_entry = proc_create(NVT_RAW, 0444, NULL,&nvt_raw_fops);
	if (NVT_proc_raw_entry == NULL) {
		TS_LOG_ERR("%s: create proc/nvt_raw Failed!\n", __func__);
		return -ENOMEM;
	} else {
		TS_LOG_INFO("%s: create proc/nvt_raw Succeeded!\n", __func__);
	}

	NVT_proc_diff_entry = proc_create(NVT_DIFF, 0444, NULL,&nvt_diff_fops);
	if (NVT_proc_diff_entry == NULL) {
		TS_LOG_ERR("%s: create proc/nvt_diff Failed!\n", __func__);
		return -ENOMEM;
	} else {
		TS_LOG_INFO("%s: create proc/nvt_diff Succeeded!\n", __func__);
	}

	return 0;
}
#endif
