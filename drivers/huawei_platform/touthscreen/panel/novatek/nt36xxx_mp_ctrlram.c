/* drivers/input/touchscreen/nt36772/nt36772_mp_ctrlram.c
 *
 * Copyright (C) 2010 - 2016 Novatek, Inc.
 *
 * $Revision: 5629 $
 * $Date: 2016-07-15 11:24:48 +0800 (星期五, 15 七月 2016) $
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
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/syscalls.h>

#include <../../huawei_touchscreen_chips.h>
#include <linux/regulator/consumer.h>
#include <huawei_platform/log/log_jank.h>
#include "../../huawei_touchscreen_algo.h"
#if defined (CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

#include "nt36xxx.h"
#include "nt36xxx_mp_ctrlram.h"

//---For Debug : Test Time, Mallon 20160907---
#include <linux/jiffies.h>

#define NORMAL_MODE 0x00
#define TEST_MODE_1 0x21
#define TEST_MODE_2 0x22
#define MP_MODE_CC 0x41
#define FREQ_HOP_DISABLE 0x66
#define FREQ_HOP_ENABLE 0x65

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

static const int32_t TOTAL_AFE_CNT = 32;
static const int32_t TOTAL_COL_CNT = 18;

static uint8_t AIN_X[IC_X_CFG_SIZE] = {17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
static uint8_t AIN_Y[IC_Y_CFG_SIZE] = {0xFF, 0xFF, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

static uint8_t RecordResult_Short[40 * 40] = {0};
static uint8_t RecordResult_Open[40 * 40] = {0};
static uint8_t RecordResult_FWMutual[40 * 40] = {0};
static uint8_t RecordResult_FW_CC[40 * 40] = {0};
static uint8_t RecordResult_FW_Diff[40 * 40] = {0};
static int32_t RecordResult_FWMutual_X_Delta[40 * 40] = {0};
static int32_t RecordResult_FWMutual_Y_Delta[40 * 40] = {0};

static int32_t TestResult_Short = 0;
static int32_t TestResult_Open = 0;
static int32_t TestResult_FWMutual = 0;
static int32_t TestResult_FW_CC = 0;
static int32_t TestResult_Noise = 0;
static int32_t TestResult_FW_Diff = 0;
static int32_t TestResult_FWMutual_Delta = 0;

static int32_t RawData_Short[40 * 40] = {0};
static int32_t RawData_Open[40 * 40] = {0};
static int32_t RawData_Diff[40 * 40] = {0};
static int32_t RawData_FWMutual[40 * 40] = {0};
static int32_t RawData_FW_CC[40 * 40] = {0};

#define TP_MMI_RESULT_LEN 80
static char mmitest_result[80] = {0};/*store mmi test result*/

struct test_cmd {
	uint32_t addr;
	uint8_t len;
	uint8_t data[64];
};

struct test_cmd *CtrlRAM_test_cmd = NULL;
int32_t CtrlRAM_test_cmd_num = 0;
struct test_cmd *SignalGen_test_cmd = NULL;
int32_t SignalGen_test_cmd_num = 0;
struct test_cmd *SignalGen2_test_cmd = NULL;
int32_t SignalGen2_test_cmd_num = 0;

// control ram parameter from short_open.ini
static uint32_t System_Init_CTRLRAMTableStartAddress = 0;
static uint32_t TableType0_GLOBAL0_RAW_BASE_ADDR = 0;
static uint32_t TableType1_GLOBAL0_RAW_BASE_ADDR = 0;
static uint32_t TableType0_GLOBAL_Addr = 0;
static uint32_t TableType1_GLOBAL_Addr = 0;

static int8_t nvt_mp_isInitialed = 0;

extern struct nvt_ts_data *ts;
extern int32_t novatek_ts_i2c_read(struct i2c_client *client, uint16_t i2c_addr, uint8_t *buf, uint16_t len);
extern int32_t novatek_ts_i2c_dummy_read(struct i2c_client *client, uint16_t i2c_addr);
extern int32_t novatek_ts_i2c_write(struct i2c_client *client, uint16_t i2c_addr, uint8_t *buf, uint16_t len);
extern void nvt_sw_reset_idle(void);
extern void nvt_bootloader_reset(void);
extern int32_t nvt_check_fw_reset_state(RST_COMPLETE_STATE check_reset_state);
extern int8_t nvt_switch_noPD(uint8_t noPD);
extern int32_t nvt_clear_fw_status(void);
extern int32_t nvt_check_fw_status(void);
extern void nvt_change_mode(uint8_t mode);
extern int8_t nvt_get_fw_info(void);
extern uint8_t nvt_get_fw_pipe(void);
extern void nvt_read_mdata(uint32_t xdata_addr, uint32_t xdata_btn_addr);
extern void nvt_get_mdata(int32_t *buf, uint8_t *m_x_num, uint8_t *m_y_num);
extern int32_t novatek_read_projectid(void);
extern char novatek_project_id[PROJECT_ID_LEN+1];

typedef enum mp_criteria_item {
	E_Raw_Lim_Open_Pos,
	E_Raw_Lim_Open_Neg,
	E_Raw_Lim_Short_Pos,
	E_Raw_Lim_Short_Neg,
	E_Lim_FW_Raw_Pos,
	E_Lim_FW_Raw_Neg,
	E_Lim_FW_CC_Pos,
	E_Lim_FW_CC_Neg,
	E_Lim_FW_Diff_Pos,
	E_Lim_FW_Diff_Neg,
	E_FW_Raw_Delta_Pos,
	E_FW_Raw_Delta_Neg,
	E_Open_rawdata,
	E_ADCOper_Cnt,
	E_MP_Cri_Item_Last
} mp_criteria_item_e;

typedef enum ctrl_ram_item {
	E_System_Init_CTRLRAMTableStartAddress,
	E_TableType0_GLOBAL_Addr,
	E_TableType1_GLOBAL_Addr,
	E_TableType0_GLOBAL0_RAW_BASE_ADDR,
	E_TableType1_GLOBAL0_RAW_BASE_ADDR,
	E_CtrlRam_Item_Last
} ctrl_ram_item_e;

typedef enum signal_gen_item {
	E_SignalGen,
	E_SignalGen2,
	E_SignalGen_Item_Last
} signal_gen_item_e;

typedef enum signal_gen_cmd_item {
	E_SignalGen_Cmd_No,
	E_SignalGen_Cmd_Name,
	E_SignalGen_Cmd_Addr,
	E_SignalGen_Cmd_Val,
	E_SignalGen_Cmd_Last
} signal_gen_cmd_item_e;

typedef enum rawdata_type {
	E_RawdataType_Short,
	E_RawdataType_Open,
	E_RawdataType_Last
} rawdata_type_e;

static void goto_next_line(char **ptr)
{
	do {
		*ptr = *ptr + 1;
	} while (**ptr != '\n');
	*ptr = *ptr + 1;
}

static void copy_this_line(char *dest, char *src)
{
	char *copy_from;
	char *copy_to;

	copy_from = src;
	copy_to = dest;
	do {
		*copy_to = *copy_from;
		copy_from++;
		copy_to++;
	} while((*copy_from != '\n') && (*copy_from != '\r'));
	*copy_to = '\0';
}

static void str_low(char *str)
{
	int i;

	for (i = 0; i < strlen(str); i++)
		if ((str[i] >= 65) && (str[i] <= 90))
			str[i] += 32;
}

static unsigned long str_to_hex(char *p)
{
	unsigned long hex = 0;
	unsigned long length = strlen(p), shift = 0;
	unsigned char dig = 0;

	str_low(p);
	length = strlen(p);

	if (length == 0)
		return 0;

	do {
		dig = p[--length];
		dig = dig < 'a' ? (dig - '0') : (dig - 'a' + 0xa);
		hex |= (dig << shift);
		shift += 4;
	} while (length);
	return hex;
}

/*******************************************************
Description:
	Novatek touchscreen check ASR error function.

return:
	Executive outcomes. 0---succeed. negative---failed.
*******************************************************/
static int32_t nvt_check_asr_error(void)
{
	uint8_t buf[4] = {0};

	//---write i2c cmds to ASR error flag---
	buf[0] = 0xFF;
	buf[1] = 0x01;
	buf[2] = 0xF6;
	novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);

	//---read ASR error flag---
	buf[0] = 0x96;
	novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 3);

	if((buf[1] & 0x01) || (buf[2] & 0x01)) {
		TS_LOG_ERR("%s: Error!, buf[1]=0x%02X, buf[2]=0x%02X\n", __func__, buf[1], buf[2]);
		return -1;
	} else {
		return 0;
	}
}

/*******************************************************
Description:
	Novatek touchscreen clear ASR error flag function.

return:
	n.a
*******************************************************/
static void nvt_clean_asr_error_flag(void)
{
	uint8_t buf[4] = {0};

	//---write i2c cmds to ASR error flag---
	buf[0] = 0xFF;
	buf[1] = 0x01;
	buf[2] = 0xF6;
	novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);
	
	//---clean ASR error flag---
	buf[0] = 0x96;
	buf[1] = 0x00;
	buf[2] = 0x00;
	novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);
}

static int32_t nvt_load_mp_ctrlram_ini(void)
{
	int32_t retval = 0;
	struct file *fp = NULL;
	char *fbufp = NULL; // buffer for content of file
	mm_segment_t org_fs;
	char file_path[64];
	struct kstat stat;
	loff_t pos = 0;
	int32_t read_ret = 0;
	char *ptr = NULL;
	ctrl_ram_item_e ctrl_ram_item = E_System_Init_CTRLRAMTableStartAddress;
	char ctrlram_data_buf[128] = {0};
	char ctrlram_item_str[128] = {0};

	TS_LOG_INFO("%s:++\n", __func__);
#ifdef BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE
	snprintf(file_path, sizeof(file_path), "/product/etc/firmware/ts/%s_short_open.ini", novatek_project_id);
#else
	snprintf(file_path, sizeof(file_path), "/vendor/firmware/ts/%s_short_open.ini", novatek_project_id);
//	snprintf(file_path, sizeof(file_path), "/data/nvt_data/%s_short_open.ini", novatek_project_id);
#endif
		
	org_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(file_path, O_RDONLY, 0);
	if (fp == NULL || IS_ERR(fp)) {
		TS_LOG_ERR("%s: open %s failed\n", __func__, file_path);
		set_fs(org_fs);
		retval = -1;
		return retval;
	}

	retval = vfs_stat(file_path, &stat);
	if (!retval) {
		fbufp = (char *)vmalloc(stat.size + 1);
		if (!fbufp) {
			TS_LOG_ERR("%s: vmalloc %lld bytes failed.\n", __func__, stat.size);
			retval = -2;
			set_fs(org_fs);
			filp_close(fp, NULL);
			return retval;
		}else{
			memset(fbufp, 0, stat.size + 1);
		}

		read_ret = vfs_read(fp, (char __user *)fbufp, stat.size, &pos);
		if (read_ret > 0) {
			//pr_info("%s: File Size:%lld\n", __func__, stat.size);
			//pr_info("---------------------------------------------------\n");
			//printk("fbufp:\n");
			//for(i = 0; i < stat.size; i++) {
			//  printk("%c", fbufp[i]);
			//}
			//pr_info("---------------------------------------------------\n");

			fbufp[stat.size] = 0;
			ptr = fbufp;

			while ( ptr && (ptr < (fbufp + stat.size))) {
				if (ctrl_ram_item == E_System_Init_CTRLRAMTableStartAddress) {
					ptr = strstr(ptr, "[System_Init]");
					if (ptr == NULL) {
						TS_LOG_ERR("%s: [System_Init] not found!\n", __func__);
						retval = -5;
						goto exit_free;
					}
					ptr = strstr(ptr, "CTRLRAMTableStartAddress");
					if (ptr == NULL) {
						TS_LOG_ERR("%s: CTRLRAMTableStartAddress not found!\n", __func__);
						retval = -6;
						goto exit_free;
					}
					copy_this_line(ctrlram_data_buf, ptr);
					sscanf(ctrlram_data_buf, "CTRLRAMTableStartAddress=%s", ctrlram_item_str);
					System_Init_CTRLRAMTableStartAddress = str_to_hex(ctrlram_item_str + 2); // skip "0x"
					TS_LOG_INFO("System_Init_CTRLRAMTableStartAddress = 0x%08X\n", System_Init_CTRLRAMTableStartAddress);
				} else if (ctrl_ram_item == E_TableType0_GLOBAL_Addr) {
					ptr = strstr(ptr, "[TableType0]");
					if (ptr == NULL) {
						TS_LOG_ERR("%s: [TableType0] not found!\n", __func__);
						retval = -7;
						goto exit_free;
					}
					ptr = strstr(ptr, "GLOBAL_Addr=");
					if (ptr == NULL) {
						TS_LOG_ERR("%s: \"[TableType0] GLOBAL_Addr\" not found!\n", __func__);
						retval = -8;
						goto exit_free;
					}
					copy_this_line(ctrlram_data_buf, ptr);
					sscanf(ctrlram_data_buf, "GLOBAL_Addr=%s", ctrlram_item_str);
					TableType0_GLOBAL_Addr = str_to_hex(ctrlram_item_str + 2); // skip "0x"
					TS_LOG_INFO("TableType0_GLOBAL_Addr = 0x%08X\n", TableType0_GLOBAL_Addr);
					TableType0_GLOBAL_Addr = TableType0_GLOBAL_Addr - 0x10000;
				} else if (ctrl_ram_item == E_TableType1_GLOBAL_Addr) {
					ptr = strstr(ptr, "[TableType1]");
					if (ptr == NULL) {
						TS_LOG_ERR("%s: [TableType1] not found!\n", __func__);
						retval = -9;
						goto exit_free;
					}
					ptr = strstr(ptr, "GLOBAL_Addr=");
					if (ptr == NULL) {
						TS_LOG_ERR("%s: \"[TableType1] GLOBAL_Addr\" not found!\n", __func__);
						retval = -10;
						goto exit_free;
					}
					copy_this_line(ctrlram_data_buf, ptr);
					sscanf(ctrlram_data_buf, "GLOBAL_Addr=%s", ctrlram_item_str);
					TableType1_GLOBAL_Addr = str_to_hex(ctrlram_item_str + 2); // skip "0x"
					TS_LOG_INFO("TableType1_GLOBAL_Addr = 0x%08X\n", TableType1_GLOBAL_Addr);
					TableType1_GLOBAL_Addr = TableType1_GLOBAL_Addr - 0x10000;
				} else if (ctrl_ram_item == E_TableType0_GLOBAL0_RAW_BASE_ADDR) {
					ptr = strstr(ptr, "[TableType0_GLOBAL0]");
					if (ptr == NULL) {
						TS_LOG_ERR("%s: [TableType0_GLOBAL0] not found!\n", __func__);
						retval = -11;
						goto exit_free;
					}
					ptr = strstr(ptr, "\nRAW_BASE_ADDR=");
					ptr++; // skip first byte '\n'
					if (ptr == NULL) {
						TS_LOG_ERR("%s: \"[TableType0_GLOBAL0] RAW_BASE_ADDR\" not found!\n", __func__);
						retval = -12;
						goto exit_free;
					}
					copy_this_line(ctrlram_data_buf, ptr);
					sscanf(ctrlram_data_buf, "RAW_BASE_ADDR=%s", ctrlram_item_str);
					TableType0_GLOBAL0_RAW_BASE_ADDR = str_to_hex(ctrlram_item_str + 2); // skip "0x"
					TS_LOG_INFO("TableType0_GLOBAL0_RAW_BASE_ADDR = 0x%08X\n", TableType0_GLOBAL0_RAW_BASE_ADDR);
					TableType0_GLOBAL0_RAW_BASE_ADDR = TableType0_GLOBAL0_RAW_BASE_ADDR | 0x10000;
				} else if (ctrl_ram_item == E_TableType1_GLOBAL0_RAW_BASE_ADDR) {
					ptr = strstr(ptr, "[TableType1_GLOBAL0]");
					if (ptr == NULL) {
						TS_LOG_ERR("%s: [TableType1_GLOBAL0] not found!\n", __func__);
						retval = -13;
						goto exit_free;
					}
					ptr = strstr(ptr, "\nRAW_BASE_ADDR=");
					ptr++; // skip first byte '\n'
					if (ptr == NULL) {
						TS_LOG_ERR("%s: \"[TableType1_GLOBAL0] RAW_BASE_ADDR\" not found!\n", __func__);
						retval = -14;
						goto exit_free;
					}
					copy_this_line(ctrlram_data_buf, ptr);
					sscanf(ctrlram_data_buf, "RAW_BASE_ADDR=%s", ctrlram_item_str);
					TableType1_GLOBAL0_RAW_BASE_ADDR = str_to_hex(ctrlram_item_str + 2); // skip "0x"
					TS_LOG_INFO("TableType1_GLOBAL0_RAW_BASE_ADDR = 0x%08X\n", TableType1_GLOBAL0_RAW_BASE_ADDR);
					TableType1_GLOBAL0_RAW_BASE_ADDR = TableType1_GLOBAL0_RAW_BASE_ADDR | 0x10000;
				}

				ctrl_ram_item++;
				if (ctrl_ram_item == E_CtrlRam_Item_Last) {
					TS_LOG_INFO("%s: Load control ram items finished\n", __func__);
					retval = 0;
					break;
				}
			}

        } else {
            TS_LOG_ERR("%s: retval=%d, read_ret=%d, fbufp=%p, stat.size=%lld\n", __func__, retval, read_ret, fbufp, stat.size);
            retval = -3;
            goto exit_free;
        }
    } else {
        TS_LOG_ERR("%s: failed to get file stat, retval = %d\n", __func__, retval);
        retval = -4;
        goto exit_free;
    }

exit_free:
	set_fs(org_fs);
	if (fbufp) {
		vfree(fbufp);
		fbufp = NULL;
	}
	if (fp) {
		filp_close(fp, NULL);
		fp = NULL;
	}

	TS_LOG_INFO("%s:--, retval=%d\n", __func__, retval);
	return retval;
}

static int32_t nvt_load_mp_ctrlram_bin(void)
{
	int32_t retval = 0;
	struct file *fp = NULL;
	char *fbufp = NULL; // buffer for content of file
	mm_segment_t org_fs;
	char file_path[64];
	struct kstat stat;
	loff_t pos = 0;
	int32_t read_ret = 0;
	char *ptr = NULL;
	uint32_t i = 0;
	uint32_t ctrlram_cur_addr = 0;

	TS_LOG_INFO("%s:++\n", __func__);
#ifdef BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE
	snprintf(file_path, sizeof(file_path), "/product/etc/firmware/ts/%s_short_open.bin", novatek_project_id);	
#else
	snprintf(file_path, sizeof(file_path), "/vendor/firmware/ts/%s_short_open.bin", novatek_project_id);
//	snprintf(file_path, sizeof(file_path), "/data/nvt_data/%s_short_open.bin", novatek_project_id);
#endif
		
	org_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(file_path, O_RDONLY, 0);
	if (fp == NULL || IS_ERR(fp)) {
		TS_LOG_ERR("%s: open %s failed\n", __func__, file_path);
		set_fs(org_fs);
		retval = -1;
		return retval;
	}

	retval = vfs_stat(file_path, &stat);
	if (!retval) {
		fbufp = (char *)vmalloc(stat.size + 1);
		if (!fbufp) {
			TS_LOG_ERR("%s: vmalloc %lld bytes failed.\n", __func__, stat.size);
			retval = -2;
			set_fs(org_fs);
			filp_close(fp, NULL);
			return retval;
		}else{
			memset(fbufp, 0, stat.size + 1);
		}

		read_ret = vfs_read(fp, (char __user *)fbufp, stat.size, &pos);
		if (read_ret > 0) {
			//pr_info("%s: File Size:%lld\n", __func__, stat.size);
			//pr_info("---------------------------------------------------\n");
			//printk("fbufp:\n");
			//for(i = 0; i < stat.size; i++) {
			//  printk("%c", fbufp[i]);
			//}
			//pr_info("---------------------------------------------------\n");
			fbufp[stat.size] = 0;
			printk("ctrlram bin file size is %lld\n", stat.size);
			ptr = fbufp;

			CtrlRAM_test_cmd_num = (int32_t)((int32_t)stat.size / 4);
			if (System_Init_CTRLRAMTableStartAddress != 0) {
				ctrlram_cur_addr = System_Init_CTRLRAMTableStartAddress;
			} else {
				TS_LOG_ERR("%s: System_Init_CTRLRAMTableStartAddress is not initialized!\n", __func__);
				retval = -5;
				goto exit_free;
			}
			if (CtrlRAM_test_cmd != NULL) {
				vfree(CtrlRAM_test_cmd);
				CtrlRAM_test_cmd = NULL;
			}
			CtrlRAM_test_cmd = (struct test_cmd *)vmalloc(CtrlRAM_test_cmd_num * sizeof(struct test_cmd));
			if (!CtrlRAM_test_cmd) {
				TS_LOG_ERR("%s: vmalloc for CtrlRAM_test_cmd failed.\n", __func__);
				retval = -ENOMEM;
				goto exit_free;
			}

			for (i = 0; i < CtrlRAM_test_cmd_num; i++) {
				CtrlRAM_test_cmd[i].addr = ctrlram_cur_addr;
				CtrlRAM_test_cmd[i].len = 4;
				CtrlRAM_test_cmd[i].data[0] = *(ptr + (4 * i));
				CtrlRAM_test_cmd[i].data[1] = *(ptr + (4 * i) + 1); 
				CtrlRAM_test_cmd[i].data[2] = *(ptr + (4 * i) + 2);
				CtrlRAM_test_cmd[i].data[3] = *(ptr + (4 * i) + 3);
				//if (i < 10) {
				//	printk("%02X %02X %02X %02X\n", CtrlRAM_test_cmd[i].data[0], CtrlRAM_test_cmd[i].data[1], CtrlRAM_test_cmd[i].data[2], CtrlRAM_test_cmd[i].data[3]);
				//}
				ctrlram_cur_addr = ctrlram_cur_addr + 4;
			}

		} else {
			TS_LOG_ERR("%s: retval=%d, read_ret=%d, fbufp=%p, stat.size=%lld\n", __func__, retval, read_ret, fbufp, stat.size);
			retval = -3;
			goto exit_free;
		}
	} else {
		TS_LOG_ERR("%s: failed to get file stat, retval = %d\n", __func__, retval);
		retval = -4;
		goto exit_free;
	}

exit_free:
	set_fs(org_fs);
	if (fbufp) {
		vfree(fbufp);
		fbufp = NULL;
	}
	if (fp) {
		filp_close(fp, NULL);
		fp = NULL;
	}

	TS_LOG_INFO("%s:--, retval=%d\n", __func__, retval);

	return retval;
}

static int32_t nvt_load_mp_signal_gen_setting(void)
{
	int32_t retval = 0;
	struct file *fp = NULL;
	char *fbufp = NULL; // buffer for content of file
	mm_segment_t org_fs;
	char file_path[64];
	struct kstat stat;
	loff_t pos = 0;
	int32_t read_ret = 0;
	char *ptr = NULL;
	uint32_t i = 0;
	signal_gen_item_e signal_gen_item = E_SignalGen;
	char signal_gen_buf[128] = {0};
	signal_gen_cmd_item_e signal_gen_cmd_item = E_SignalGen_Cmd_Name;
	char *token = NULL;
	char *tok_ptr = NULL;

	TS_LOG_INFO("%s:++\n", __func__);
#ifdef BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE
	snprintf(file_path, sizeof(file_path), "/product/etc/firmware/ts/%s_default.ini", novatek_project_id);
#else
	snprintf(file_path, sizeof(file_path), "/vendor/firmware/ts/%s_default.ini", novatek_project_id);
//	snprintf(file_path, sizeof(file_path), "/data/nvt_data/%s_default.ini", novatek_project_id);
#endif
	org_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(file_path, O_RDONLY, 0);
	if (fp == NULL || IS_ERR(fp)) {
		TS_LOG_ERR("%s: open %s failed\n", __func__, file_path);
		set_fs(org_fs);
		retval = -1;
		return retval;
	}

	retval = vfs_stat(file_path, &stat);
	if (!retval) {
		fbufp = (char *)vmalloc(stat.size + 1);
		if (!fbufp) {
			TS_LOG_ERR("%s: vmalloc %lld bytes failed.\n", __func__, stat.size);
			retval = -2;
			set_fs(org_fs);
			filp_close(fp, NULL);
			return retval;
		}else{
			memset(fbufp, 0, stat.size + 1);
		}

		read_ret = vfs_read(fp, (char __user *)fbufp, stat.size, &pos);
		if (read_ret > 0) {
			//pr_info("%s: File Size:%lld\n", __func__, stat.size);
			//pr_info("---------------------------------------------------\n");
			//printk("fbufp:\n");
			//for(i = 0; i < stat.size; i++) {
			//  printk("%c", fbufp[i]);
			//}
			//pr_info("---------------------------------------------------\n");

			fbufp[stat.size] = 0;
			ptr = fbufp;

			while ( ptr && (ptr < (fbufp + stat.size))) {
				if (signal_gen_item == E_SignalGen) {
					ptr = strstr(ptr, "[SignalGen]");
					if (ptr == NULL) {
						TS_LOG_ERR("%s: [SignalGen] not found!\n", __func__);
						retval = -5;
						goto exit_free;
					}
					ptr = strstr(ptr, "SignalNum=");
					if (ptr == NULL) {
						TS_LOG_ERR("%s: [SignalGen] SignalNum  not found!\n", __func__);
						retval = -6;
						goto exit_free;
					}
					copy_this_line(signal_gen_buf, ptr);
					sscanf(signal_gen_buf, "SignalNum=%d", &SignalGen_test_cmd_num);
					TS_LOG_INFO("SignalGen_test_cmd_num = %d\n", SignalGen_test_cmd_num);
					if (SignalGen_test_cmd) {
						vfree(SignalGen_test_cmd);
						SignalGen_test_cmd = NULL;
					}
					SignalGen_test_cmd = vmalloc(SignalGen_test_cmd_num * sizeof(struct test_cmd));
					if (!SignalGen_test_cmd) {
						TS_LOG_ERR("%s: vmalloc for SignalGen_test_cmd failed!\n", __func__);
						retval = -ENOMEM;
						goto exit_free;
					}
					for (i = 0; i < SignalGen_test_cmd_num; i++) {
						goto_next_line(&ptr);
						copy_this_line(signal_gen_buf, ptr);
						signal_gen_cmd_item = E_SignalGen_Cmd_No;
						tok_ptr = signal_gen_buf;
						while((token = strsep(&tok_ptr,", =\t\0"))) {
							if (strlen(token) == 0) {
								continue;
							}
							if (signal_gen_cmd_item == E_SignalGen_Cmd_No) {
								signal_gen_cmd_item++;
							} else if (signal_gen_cmd_item == E_SignalGen_Cmd_Name) {
								signal_gen_cmd_item++;
							} else if (signal_gen_cmd_item == E_SignalGen_Cmd_Addr) {
								SignalGen_test_cmd[i].addr = str_to_hex(token + 2); // skip "0x"
								SignalGen_test_cmd[i].len = 1;
								signal_gen_cmd_item++;
							} else if (signal_gen_cmd_item == E_SignalGen_Cmd_Val) {
								SignalGen_test_cmd[i].data[0] = (uint8_t)str_to_hex(token + 2); // skip "0x"
								signal_gen_cmd_item++;
							} else if (signal_gen_cmd_item == E_SignalGen_Cmd_Last) {
								break;
							}
						}
					}
				} else if (signal_gen_item == E_SignalGen2) {
					ptr = strstr(ptr, "[SignalGen2]");
					if (ptr == NULL) {
						TS_LOG_ERR("%s: [SignalGen2] not found!\n", __func__);
						retval = -7;
						goto exit_free;
					}
					ptr = strstr(ptr, "SignalNum=");
					if (ptr == NULL) {
						TS_LOG_ERR("%s: [SignalGen2] SignalNum not found!\n", __func__);
						retval = -8;
						goto exit_free;
					}
					copy_this_line(signal_gen_buf, ptr);
					sscanf(signal_gen_buf, "SignalNum=%d", &SignalGen2_test_cmd_num);
					TS_LOG_INFO("SignalGen2_test_cmd_num = %d\n", SignalGen2_test_cmd_num);
					if (SignalGen2_test_cmd) {
						vfree(SignalGen2_test_cmd);
						SignalGen2_test_cmd = NULL;
					}
					SignalGen2_test_cmd = vmalloc(SignalGen2_test_cmd_num * sizeof(struct test_cmd));
					if (!SignalGen2_test_cmd) {
						TS_LOG_ERR("%s: vmalloc for SignalGen2_test_cmd failed!\n", __func__);
						retval = -ENOMEM;
						goto exit_free;
					}
					for (i = 0; i < SignalGen2_test_cmd_num; i++) {
						goto_next_line(&ptr);
						copy_this_line(signal_gen_buf, ptr);
						signal_gen_cmd_item = E_SignalGen_Cmd_No;
						tok_ptr = signal_gen_buf;
						while((token = strsep(&tok_ptr,", =\t\0"))) {
							if (strlen(token) == 0) {
								continue;
							}
							if (signal_gen_cmd_item == E_SignalGen_Cmd_No) {
								signal_gen_cmd_item++;
							} else if (signal_gen_cmd_item == E_SignalGen_Cmd_Name) {
								signal_gen_cmd_item++;
							} else if (signal_gen_cmd_item == E_SignalGen_Cmd_Addr) {
								SignalGen2_test_cmd[i].addr = str_to_hex(token + 2); // skip "0x"
								SignalGen2_test_cmd[i].len = 1;
								signal_gen_cmd_item++;
							} else if (signal_gen_cmd_item == E_SignalGen_Cmd_Val) {
								SignalGen2_test_cmd[i].data[0] = (uint8_t)str_to_hex(token + 2); // skip "0x"
								signal_gen_cmd_item++;
							} else if (signal_gen_cmd_item == E_SignalGen_Cmd_Last) {
								break;
							}
						}
					}
				}

				signal_gen_item++;
				if (signal_gen_item == E_SignalGen_Item_Last) {
					TS_LOG_INFO("%s: Load signal gen items finished\n", __func__);
					retval = 0;
					break;
				}
			}

		} else {
			TS_LOG_ERR("%s: retval=%d, read_ret=%d, fbufp=%p, stat.size=%lld\n", __func__, retval, read_ret, fbufp, stat.size);
			retval = -3;
			goto exit_free;
		}
	} else {
		TS_LOG_ERR("%s: failed to get file stat, retval = %d\n", __func__, retval);
		retval = -4;
		goto exit_free;
	}

exit_free:
	set_fs(org_fs);
	if (fbufp) {
		vfree(fbufp);
		fbufp = NULL;
	}
	if (fp) {
		filp_close(fp, NULL);
		fp = NULL;
	}

	TS_LOG_INFO("%s:--, retval=%d\n", __func__, retval);
	return retval;
}

/*******************************************************
Description:
	Novatek touchscreen set ADC operation function.

return:
	Executive outcomes. 0---succeed. negative---failed.
*******************************************************/
static int32_t nvt_set_adc_oper(void)
{
	uint8_t buf[4] = {0};
	int32_t i, j;
	const int32_t retry_adc_oper = 10;
	const int32_t retry_adc_status = 10;

	for (i = 0; i < retry_adc_oper; i++) {
		//---write i2c cmds to set ADC operation---
		buf[0] = 0xFF;
		buf[1] = 0x01;
		buf[2] = 0xF4;
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);

		//---write i2c cmds to clear ADC operation---
		buf[0] = 0x4C;
		buf[1] = 0x00;
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);

		msleep(10);
		//---write i2c cmds to set ADC operation---
		buf[0] = 0x4C;
		buf[1] = 0x01;
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);

		for (j = 0; j < retry_adc_status; j++) {
			//---read ADC status---
			buf[0] = 0x4C;
			buf[1] = 0x00;
			novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 2);

			if (buf[1] == 0x00)
				break;

			msleep(10);
		}

		if ((j >= retry_adc_status) || (nvt_check_asr_error() != 0)) {
			TS_LOG_ERR("%s: Failed!, buf[1]=0x%02X, i=%d\n", __func__, buf[1], i);
			nvt_clean_asr_error_flag();
		} else {
			break;
		}
	}

	if (i >= retry_adc_oper) {
		TS_LOG_ERR("%s: Failed!\n", __func__);
		return -1;
	} else {
		return 0;
	}
}

/*******************************************************
Description:
	Novatek touchscreen write test commands function.

return:
	n.a.
*******************************************************/
static void nvt_write_test_cmd(struct test_cmd *cmds, int32_t cmd_num)
{
	int32_t i = 0;
	int32_t j = 0;
	uint8_t buf[64];

	for (i = 0; i < cmd_num; i++) {
		//---set xdata index---
		buf[0] = 0xFF;
		buf[1] = ((cmds[i].addr >> 16) & 0xFF);
		buf[2] = ((cmds[i].addr >> 8) & 0xFF);
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);

		//---write test cmds---
		buf[0] = (cmds[i].addr & 0xFF);
		for (j = 0; j < cmds[i].len; j++) {
			buf[1 + j] = cmds[i].data[j];
		}
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 1 + cmds[i].len);

/*
		//---read test cmds (debug)---
		buf[0] = (cmds[i].addr & 0xFF);
		novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 1 + cmds[i].len);
		printk("0x%08X, ", cmds[i].addr);
		for (j = 0; j < cmds[i].len; j++) {
			printk("0x%02X, ", buf[j + 1]);
		}
		printk("\n");
*/
	}
}

static int32_t nvt_set_memory(uint32_t addr, uint8_t data)
{
	int32_t ret = 0;
	uint8_t buf[64] = {0};

	//---set xdata index---
	buf[0] = 0xFF;
	buf[1] = (addr >> 16) & 0xFF;
	buf[2] = (addr >> 8) & 0xFF;
	ret = novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);
	if (ret < 0) {
		TS_LOG_ERR("%s: write xdata index failed!(%d)\n", __func__, ret);
		return ret;
	}

	//---write data---
	buf[0] = addr & 0xFF;
	buf[1] = data;
	ret = novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 2);
	if (ret < 0) {
		TS_LOG_ERR("%s: write data failed!(%d)\n", __func__, ret);
		return ret;
	}

/*
	//---read data (debug)---
	buf[0] = addr & 0xFF;
	buf[1] = 0x00;
	ret = novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 2);
	printk("0x%08X, 0x%02X\n", addr, buf[1]);
*/

	return 0;
}

static int32_t nvt_get_memory(uint32_t addr, uint8_t *data)
{
	int32_t ret = 0;
	uint8_t buf[64] = {0};

	//---set xdata index---
	buf[0] = 0xFF;
	buf[1] = (addr >> 16) & 0xFF;
	buf[2] = (addr >> 8) & 0xFF;
	ret = novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);
	if (ret < 0) {
		TS_LOG_ERR("%s: write xdata index failed!(%d)\n", __func__, ret);
		return ret;
	}

	//---read data---
	buf[0] = addr & 0xFF;
	buf[1] = 0;
	ret = novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 2);
	if (ret < 0) {
		TS_LOG_ERR("%s: read data failed!(%d)\n", __func__, ret);
		return ret;
	}
	*data = buf[1];

	return 0;
}

static void nvt_ASR_PowerOnMode(void)
{
	nvt_set_memory(0x1F690, 0x00);
	nvt_set_memory(0x1F693, 0x00);
	nvt_set_memory(0x1F68C, 0x01);
	nvt_set_memory(0x1F691, 0x01);
}

static void nvt_ASR_FrameMode(void)
{
	nvt_set_memory(0x1F690, 0x01);
	nvt_set_memory(0x1F693, 0x01);
	nvt_set_memory(0x1F68C, 0x01);
	nvt_set_memory(0x1F691, 0x01);
}

#if 0
static void nvt_TCON_PowerOnInit(void)
{
	uint8_t tmp_val = 0;

	// Step1
	nvt_set_memory(0x1F4BD, 0x01);
	nvt_set_memory(0x1F60F, 0x03);
	// ASR(Power On Mode)
	nvt_ASR_PowerOnMode();
	// Step2
	// 0x1F5EC,bit1 = 1 bit5 = 1, (| 0x22)
	tmp_val = 0;
	nvt_get_memory(0x1F5EC, &tmp_val);
	tmp_val |= 0x22;
	nvt_set_memory(0x1F5EC, tmp_val);
	// 0x1F5EE,bit1 = 1 bit5 = 1, (| 0x22)
	tmp_val = 0;
	nvt_get_memory(0x1F5EE, &tmp_val);
	tmp_val |= 0x22;
	nvt_set_memory(0x1F5EE, tmp_val);
	// ASR(Power On Mode)
	nvt_ASR_PowerOnMode();
	// Delay 2ms
	msleep(2);
	// Step3
	// 0x1F5F0, bit0 = 1, (| 0x01)
	tmp_val = 0;
	nvt_get_memory(0x1F5F0, &tmp_val);
	tmp_val |= 0x01;
	nvt_set_memory(0x1F5F0, tmp_val);
	// 0x1F5F2, bit0 = 1, (| 0x01)
	tmp_val = 0;
	nvt_get_memory(0x1F5F2, &tmp_val);
	tmp_val |= 0x01;
	nvt_set_memory(0x1F5F2, tmp_val);
	// ASR(Power On mode)
	nvt_ASR_PowerOnMode();
	// Step4
	// 0x1F5EC, bit2 = 1 bit3 = 1, (| 0x0C)
	tmp_val = 0;
	nvt_get_memory(0x1F5EC, &tmp_val);
	tmp_val |= 0x0C;
	nvt_set_memory(0x1F5EC, tmp_val);
	// 0x1F5EE, bit2 = 1 bit3 = 1, (| 0x0C)
	tmp_val = 0;
	nvt_get_memory(0x1F5EE, &tmp_val);
	tmp_val |= 0x0C;
	nvt_set_memory(0x1F5EE, tmp_val);
	// ASR(Frame Mode)
	nvt_ASR_FrameMode();
	msleep(1);
}
#endif
//---fix high current issue, Taylor 20160908---
static void nvt_TCON_PowerOnInit(void)
{
	uint8_t tmp_val = 0;

	// Step1
	nvt_set_memory(0x1F4BD, 0x01);
	nvt_set_memory(0x1F60F, 0x03);
	// ASR(Power On Mode)
	nvt_ASR_PowerOnMode();

	// Step2
	// 0x1F5EC,bit1 = 1 bit5 = 1, (| 0x22)
	nvt_get_memory(0x1F5EC, &tmp_val);
	tmp_val |= 0x22;
	nvt_set_memory(0x1F5EC, tmp_val);
	// 0x1F5EE,bit1 = 1 bit5 = 1, (| 0x22)
	tmp_val = 0;
	nvt_get_memory(0x1F5EE, &tmp_val);
	tmp_val |= 0x22;
	nvt_set_memory(0x1F5EE, tmp_val);
	// ASR(Frame On Mode)
	nvt_ASR_FrameMode();
	msleep(2);

	// Step3
	// 0x1F5F0, bit0 = 1, (| 0x01)
	tmp_val = 0;
	nvt_get_memory(0x1F5F0, &tmp_val);
	tmp_val |= 0x01;
	nvt_set_memory(0x1F5F0, tmp_val);
	// 20160908 0x1F5F1 = 0
	nvt_set_memory(0x1F5F1, 0x00);
	// 0x1F5F2, bit0 = 1, (| 0x01)
	tmp_val = 0;
	nvt_get_memory(0x1F5F2, &tmp_val);
	tmp_val |= 0x01;
	nvt_set_memory(0x1F5F2, tmp_val);

	// 20160908 sync to tuning tool
	// 0x1F5F3 = 0
	nvt_set_memory(0x1F5F3, 0x00);
	//0x1F5F4 = 0
	nvt_set_memory(0x1F5F4, 0x00);
	//0x1F5F5 = 0
	nvt_set_memory(0x1F5F5, 0x00);
	//0x1F5F6 = 0
	nvt_set_memory(0x1F5F6, 0x00);
	//0x1F5F7 = 0
	nvt_set_memory(0x1F5F7, 0x00);
	//0x1F5F8 = 0x24
	nvt_set_memory(0x1F5F8, 0x24);
	//0x1F5F9 = 0x1b
	nvt_set_memory(0x1F5F9, 0x1B);
	//0x1F5FA = 0
	nvt_set_memory(0x1F5FA, 0x00);
	//0x1F5FB = 0
	nvt_set_memory(0x1F5FB, 0x00);
	//0x1F5FC = 1
	nvt_set_memory(0x1F5FC, 0x01);
	//0x1F5FD = 0
	nvt_set_memory(0x1F5FD, 0x00);
	//0x1F5FE = 0
	nvt_set_memory(0x1F5FE, 0x00);
	//0x1F478 = 1
	nvt_set_memory(0x1F478, 0x01);
	// 20160908 sync to tuning tool (end)

	// ASR(Power On Mode)
	nvt_ASR_PowerOnMode();

	// Step4
	// 0x1F5EC, bit2 = 1 bit3 = 1, (| 0x0C)
	tmp_val = 0;
	nvt_get_memory(0x1F5EC, &tmp_val);
	tmp_val |= 0x0C;
	nvt_set_memory(0x1F5EC, tmp_val);
	// 0x1F5EE, bit2 = 1 bit3 = 1, (| 0x0C)
	tmp_val = 0;
	nvt_get_memory(0x1F5EE, &tmp_val);
	tmp_val |= 0x0C;
	nvt_set_memory(0x1F5EE, tmp_val);
	// ASR(Frame On Mode)
	nvt_ASR_FrameMode();
	msleep(1);
}

static void nvt_TCON_PowerOn(void)
{
	uint8_t tmp_val = 0;

	// Step1
	nvt_set_memory(0x1F4BD, 0x01);
	nvt_set_memory(0x1F60F, 0x03);
	// ASR(Power On Mode)
	nvt_ASR_PowerOnMode();
	// Step2
	// 0x1F5EC,bit4 = 0, (& 0xEF)
	nvt_get_memory(0x1F5EC, &tmp_val);
	tmp_val &= 0xEF;
	nvt_set_memory(0x1F5EC, tmp_val);
	// 0x1F5ED,bit0 = 0, (& 0xFE)
	tmp_val = 0;
	nvt_get_memory(0x1F5ED, &tmp_val);
	tmp_val &= 0xFE;
	nvt_set_memory(0x1F5ED, tmp_val);
	// 0x1F5EE,bit4 = 0, (& 0xEF)
	tmp_val = 0;
	nvt_get_memory(0x1F5EE, &tmp_val);
	tmp_val &= 0xEF;
	nvt_set_memory(0x1F5EE, tmp_val);
	// 0x1F5EF,bit0 = 0, (& 0xFE)
	tmp_val = 0;
	nvt_get_memory(0x1F5EF, &tmp_val);
	tmp_val &= 0xFE;
	nvt_set_memory(0x1F5EF, tmp_val);
	// ASR(Frame Mode)
	nvt_ASR_FrameMode();
	// Step3
	// 0x1F5EC, bit1 = 1 bit2 = 1 bit3 = 1 bit5 = 1, (| 0x2E)
	tmp_val = 0;
	nvt_get_memory(0x1F5EC, &tmp_val);
	tmp_val |= 0x2E;
	nvt_set_memory(0x1F5EC, tmp_val);
	// 0x1F5EE, bit1 = 1 bit2 = 1 bit3 = 1 bit5 = 1, (| 0x2E)
	tmp_val = 0;
	nvt_get_memory(0x1F5EE, &tmp_val);
	tmp_val |= 0x2E;
	nvt_set_memory(0x1F5EE, tmp_val);
	// ASR(Frame Mode)
	nvt_ASR_FrameMode();
	// Step4
	// 0x1F5F0,bit0 = 1, (| 0x01)
	tmp_val = 0;
	nvt_get_memory(0x1F5F0, &tmp_val);
	tmp_val |= 0x01;
	nvt_set_memory(0x1F5F0, tmp_val);
	// 0x1F5F2,bit0 = 1, (| 0x01)
	tmp_val = 0;
	nvt_get_memory(0x1F5F2, &tmp_val);
	tmp_val |= 0x01;
	nvt_set_memory(0x1F5F2, tmp_val);
	// ASR(Power On Mode)
	nvt_ASR_PowerOnMode();
	//Delay 100us
	msleep(1);
}

static void nvt_Active(void)
{
	uint8_t tmp_val = 0;

	// RESET2 toggle high
	// 0x1F066, bit2 = 1, (| 0x04)
	nvt_get_memory(0x1F066, &tmp_val);
	tmp_val |= 0x04;
	nvt_set_memory(0x1F066, tmp_val);
	// DSV_EN toggle high
	// 0x1F06E, 0x1, (| 0x01)
	tmp_val = 0;
	nvt_get_memory(0x1F06E, &tmp_val);
	tmp_val |= 0x01;
	nvt_set_memory(0x1F06E, tmp_val);
	// Wait MTP download ready IRQ
	msleep(100);
	// Wait TSSTB high
	// RO, 0x1F067, bit2 == 0 & bit7 == 1, (& 0xFB | 0x80)
	tmp_val = 0;
	nvt_get_memory(0x1F067, &tmp_val);
	tmp_val &= 0xFB;
	tmp_val |= 0x80;
	nvt_set_memory(0x1F067, tmp_val);
	msleep(17);
	// TCON Power on
	nvt_TCON_PowerOn();
}

static void nvt_Set_SignalGen(void)
{
	// [SignalGen]
	nvt_write_test_cmd(SignalGen_test_cmd, SignalGen_test_cmd_num);

	// [SignalGen2]
	nvt_write_test_cmd(SignalGen2_test_cmd, SignalGen2_test_cmd_num);
}

void nvt_SwitchGlobalTable(rawdata_type_e rawdata_type)
{
	if (rawdata_type == E_RawdataType_Short) {
		nvt_set_memory(System_Init_CTRLRAMTableStartAddress, (uint8_t)(TableType0_GLOBAL_Addr & 0xFF));
		nvt_set_memory(System_Init_CTRLRAMTableStartAddress + 1, (uint8_t)((TableType0_GLOBAL_Addr & 0xFF00) >> 8));
	} else if (rawdata_type == E_RawdataType_Open) {
		nvt_set_memory(System_Init_CTRLRAMTableStartAddress, (uint8_t)(TableType1_GLOBAL_Addr & 0xFF));
		nvt_set_memory(System_Init_CTRLRAMTableStartAddress + 1, (uint8_t)((TableType1_GLOBAL_Addr & 0xFF00) >> 8));
	} else {
		// do nothing
	}
}

static int32_t nvt_mp_Initial(rawdata_type_e rawdata_type)
{
	TableType0_GLOBAL_Addr = 0;
	TableType1_GLOBAL_Addr = 0;
	TableType0_GLOBAL0_RAW_BASE_ADDR = 0;
	TableType1_GLOBAL0_RAW_BASE_ADDR = 0;

	// 0. SW reset + idle
	nvt_sw_reset_idle();

	// 1. Stop WDT
	nvt_set_memory(0x1F050, 0x07);
	nvt_set_memory(0x1F051, 0x55);

	// 2. Switch Long HV
	// TSVD_pol Positive (register)
	//nvt_set_memory(0x1F44D, 0x01);
	// TSHD_pol Positive (register)
	nvt_set_memory(0x1F44E, 0x01);
	// TSVD_en (register)
	nvt_set_memory(0x1F44F, 0x01);
	// TSVD_pol Positive (register)
	nvt_set_memory(0x1F44D, 0x01);

	// 3. Set CtrlRAM from INI and BIN
	if (nvt_load_mp_ctrlram_ini()) {
		TS_LOG_ERR("%s: load MP CtrlRAM ini failed!\n", __func__);
		return -EAGAIN;
	}
	if (nvt_load_mp_ctrlram_bin()) {
		TS_LOG_ERR("%s: load MP CtrlRAM bin failed!\n", __func__);
		return -EAGAIN;
	}
	nvt_write_test_cmd(CtrlRAM_test_cmd, CtrlRAM_test_cmd_num);

	// 4. TCON Power on initial class AB flow
	nvt_TCON_PowerOnInit();

	// 5. TCON Power on class AB flow
	nvt_TCON_PowerOn();

	// 6. Active
	nvt_Active();

	// ADC Oper
	// Step1
	// write [CTRLRAMTableStartAddress] at 0x1f448, 0x1f449 (0x00, 0x00)
	nvt_set_memory(0x1F448, (uint8_t)((System_Init_CTRLRAMTableStartAddress - 0x10000) & 0xFF));
	nvt_set_memory(0x1F449, (uint8_t)(((System_Init_CTRLRAMTableStartAddress - 0x10000) >> 8) & 0xFF));
	// write GlobalTableAddress at &CTRLRAMTableStartAddress (0x08, 0x00)
	
	// Step2
	// write 1 at 0x1f447
	nvt_set_memory(0x1F447, 0x01);
	// Step3, 4 - Signal Gen
	nvt_set_memory(0x1F690, 0x00);
	nvt_set_memory(0x1F693, 0x00);
	// write all registry of [SignalGen]&[SignalGen2] from "default.ini" to register
	if (nvt_load_mp_signal_gen_setting()) {
		TS_LOG_ERR("%s: load MP signal gen setting failed!\n", __func__);
		return -EAGAIN;
	}
	nvt_Set_SignalGen();
	nvt_set_memory(0x1F68C, 0x01);
	nvt_set_memory(0x1F691, 0x01);
	// Step5 - TADC (not used)
	// Step6 - ADC oper
	// nvt_set_memory(0x1F44C, 0x01);
	// nvt_set_memory(0x1F44D, 0x00);

	//RAW_RDY_SH_NUM[5:0]
	nvt_set_memory(0x1F450, 0x04);
	//for CUT2
	nvt_set_memory(0x1F50B, 0x00);
	nvt_set_memory(0x1F6D6, 0x01);
	nvt_set_memory(0x1F6DF, 0x01);

	nvt_SwitchGlobalTable(rawdata_type);

	nvt_mp_isInitialed = 1;

	return 0;
}

int32_t OffsetToReg(uint32_t addr, uint32_t *offset_data, uint32_t afe_cnt)
{
	const uint32_t OFFSET_TABLE_SIZE = 88;
	uint8_t *reg_data = NULL;
	int32_t col = 0;
	int32_t i = 0;
	struct test_cmd RegData_test_cmd;

	reg_data = (uint8_t *)vmalloc(OFFSET_TABLE_SIZE * 9);
	if (!reg_data) {
		TS_LOG_ERR("%s: vmalloc for reg_data failed.\n", __func__);
		return -ENOMEM;
	}

	for (col = 0; col < 9; col++) {
		int32_t rawdata_cnt = col * 66;
		for (i = 0; i < (OFFSET_TABLE_SIZE / 4); i++) {
			reg_data[col * OFFSET_TABLE_SIZE + i * 4 + 0] = (uint8_t)((offset_data[rawdata_cnt + 0] >> 0) & 0xFF);
			reg_data[col * OFFSET_TABLE_SIZE + i * 4 + 1] = (uint8_t)(((offset_data[rawdata_cnt + 0] >> 8) & 0x03) | ((offset_data[rawdata_cnt + 1] << 2) & 0xFC));
			reg_data[col * OFFSET_TABLE_SIZE + i * 4 + 2] = (uint8_t)(((offset_data[rawdata_cnt + 1] >> 6) & 0x0F) | ((offset_data[rawdata_cnt + 2] << 4) & 0xF0));
			reg_data[col * OFFSET_TABLE_SIZE + i * 4 + 3] = (uint8_t)(((offset_data[rawdata_cnt + 2] >> 4) & 0x3F));
			rawdata_cnt += 3;
		}

		// write (OFFSET_TABLE_SIZE / 2) each time
		RegData_test_cmd.addr = addr + col * OFFSET_TABLE_SIZE;
		RegData_test_cmd.len = OFFSET_TABLE_SIZE / 2;
		memcpy(RegData_test_cmd.data, reg_data + col * OFFSET_TABLE_SIZE, OFFSET_TABLE_SIZE / 2);
		nvt_write_test_cmd(&RegData_test_cmd, 1);

		RegData_test_cmd.addr = addr + col * OFFSET_TABLE_SIZE + (OFFSET_TABLE_SIZE / 2);
		RegData_test_cmd.len = OFFSET_TABLE_SIZE / 2;
		memcpy(RegData_test_cmd.data, reg_data + col * OFFSET_TABLE_SIZE + (OFFSET_TABLE_SIZE / 2), OFFSET_TABLE_SIZE / 2);
		nvt_write_test_cmd(&RegData_test_cmd, 1);
	}

	if (reg_data) {
		vfree(reg_data);
		reg_data = NULL;
	}

	return 0;
}

int32_t OpenRawToCS_pF(int32_t rawdata)
{
	int64_t CS = 0;
	int64_t Raw = 0;

	Raw = (int64_t)rawdata;
	Raw = Raw * 1000;
	//CS = ((Raw / 1228) - 5000) * 16;
	CS = ((Raw / 1208) - 5083) * 32;

	return (int32_t)CS;
}

/*******************************************************
Description:
	Novatek touchscreen read open test raw data function.

return:
	Executive outcomes. 0---succeed. negative---failed.
*******************************************************/
static int32_t nvt_read_open(void)
{
	int32_t i = 0;
	int32_t x = 0;
	int32_t y = 0;
	uint8_t buf[128] = {0};
	struct file *fp = NULL;
	char *fbufp = NULL;
	mm_segment_t org_fs;
	char file_path[64];
	loff_t pos = 0;
	int32_t write_ret = 0;
	uint32_t output_len = 0;
	uint8_t *rawdata_buf = NULL;

	TS_LOG_INFO("%s:++\n", __func__);

	snprintf(file_path, sizeof(file_path), "/data/nvt_data/%s_OpenTest.csv", novatek_project_id);

	rawdata_buf = (uint8_t *) vmalloc(IC_X_CFG_SIZE * IC_Y_CFG_SIZE * 2);
	if (!rawdata_buf) {
		TS_LOG_ERR("%s: vmalloc for rawdata_buf failed!\n", __func__);
		return -ENOMEM;
	}

	if (nvt_mp_isInitialed == 0) {
		if (nvt_mp_Initial(E_RawdataType_Open)) {
			TS_LOG_ERR("%s: MP Initial failed!\n", __func__);
			return -EAGAIN;
		}
	}

	nvt_SwitchGlobalTable(E_RawdataType_Open);

	for (i = 0; i < mADCOper_Cnt; i++) {
		if (nvt_set_adc_oper() < 0) {
			if (rawdata_buf) {
				vfree(rawdata_buf);
				rawdata_buf = NULL;
			}
			return -EAGAIN;
		}
	}

	for (y = 0; y < IC_Y_CFG_SIZE; y++) {
		//---change xdata index---
		buf[0] = 0xFF;
		buf[1] = 0x01;
		buf[2] = (uint8_t)(((TableType1_GLOBAL0_RAW_BASE_ADDR + y * IC_X_CFG_SIZE * 2) & 0xFF00) >> 8);
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);
		//---read data---
		buf[0] = (uint8_t)((TableType1_GLOBAL0_RAW_BASE_ADDR + y * IC_X_CFG_SIZE * 2) & 0xFF);
		novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, IC_X_CFG_SIZE * 2 + 1);
		memcpy(rawdata_buf + y * IC_X_CFG_SIZE * 2, buf + 1, IC_X_CFG_SIZE * 2);
	}

	for (y = 0; y < IC_Y_CFG_SIZE; y++) {
		for (x = 0; x < IC_X_CFG_SIZE; x++) {
			if ((AIN_Y[y] != 0xFF) && (AIN_X[x] != 0xFF)) {
				RawData_Open[AIN_Y[y] * X_Channel + AIN_X[x]] = ((rawdata_buf[(y * IC_X_CFG_SIZE + x) * 2] + 256 * rawdata_buf[(y * IC_X_CFG_SIZE + x) * 2 + 1]));
			}
		}
	}

	if (rawdata_buf) {
		vfree(rawdata_buf);
		rawdata_buf = NULL;
	}

	fbufp = (char *)vmalloc(8192);
	if (!fbufp) {
		TS_LOG_ERR("%s: vmalloc for fbufp failed!\n", __func__);
		return -ENOMEM;
	}

	// convert rawdata to CS
	for (y = 0; y < Y_Channel; y++) {
		for (x = 0; x < X_Channel; x++) {
			printk("%5d, ", RawData_Open[y * X_Channel + x]);
			RawData_Open[y * X_Channel + x] = OpenRawToCS_pF(RawData_Open[y * X_Channel + x]);
		}
		printk("\n");
	}
	printk("\n");

	printk("%s:\n", __func__);
	for (y = 0; y < Y_Channel; y++) {
		for (x = 0; x < X_Channel; x++) {
			printk("%5d, ", RawData_Open[y * X_Channel + x]);
			sprintf(fbufp + 7 * (y * X_Channel + x) + y * 2, "%5d, ", RawData_Open[y * X_Channel + x]) ;
		}
		printk("\n");
		sprintf(fbufp + 7 * (y * X_Channel + x) + y * 2,"\r\n");
	}
	printk("\n");

	org_fs = get_fs();
	set_fs(KERNEL_DS);
	fp = filp_open(file_path, O_RDWR | O_CREAT, 0644);
	if (fp == NULL || IS_ERR(fp)) {
		TS_LOG_ERR("%s: open %s failed\n", __func__, file_path);
		set_fs(org_fs);
		if (fbufp) {
			vfree(fbufp);
			fbufp = NULL;
		}
		return -1;
	}

	output_len = Y_Channel * X_Channel * 7 + Y_Channel * 2;
	write_ret = vfs_write(fp, (char __user *)fbufp, output_len, &pos);
	if (write_ret <= 0) {
		TS_LOG_ERR("%s: write %s failed\n", __func__, file_path);
		set_fs(org_fs);
		if (fp) {
			filp_close(fp, NULL);
			fp = NULL;
		}
		if (fbufp) {
			vfree(fbufp);
			fbufp = NULL;
		}
		return -1;
	}

	set_fs(org_fs);
	if (fp) {
		filp_close(fp, NULL);
		fp = NULL;
	}
	if (fbufp) {
		vfree(fbufp);
		fbufp = NULL;
	}

	TS_LOG_INFO("%s:--\n", __func__);

	return 0;
}

static int8_t nvt_switch_FreqHopEnDis(uint8_t FreqHopEnDis)
{
	uint8_t buf[8] = {0};
	uint8_t retry = 0;
	int8_t ret = 0;

	TS_LOG_INFO("%s:++\n", __func__);

	for (retry = 0; retry < 5; retry++) {
		//---set xdata index to 0x11E00---
		buf[0] = 0xFF;
		buf[1] = 0x01;
		buf[2] = 0x1E;
		novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);

		//---switch FreqHopEnDis---
		buf[0] = 0x50;
		buf[1] = FreqHopEnDis;
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
		TS_LOG_ERR("%s: switch FreqHopEnDis 0x%02X failed, buf[1]=0x%02X\n", __func__, FreqHopEnDis, buf[1]);
		ret = -1;
	}

	TS_LOG_INFO("%s:--\n", __func__);

	return ret;
}

static int32_t nvt_read_raw(int32_t *xdata)
{
	uint8_t x_num = 0;
	uint8_t y_num = 0;
	uint32_t x = 0;
	uint32_t y = 0;
	int32_t iArrayIndex = 0;
	char *fbufp = NULL;
	struct file *fp = NULL;
	mm_segment_t org_fs;
	char file_path[64];
	loff_t pos = 0;
	int32_t write_ret = 0;
	uint32_t output_len = 0;

	TS_LOG_INFO("%s:++\n", __func__);

	snprintf(file_path, sizeof(file_path), "/data/nvt_data/%s_FWMutualTest.csv", novatek_project_id);
/*
	if (nvt_clear_fw_status()) {
		return -EAGAIN;
	}

	nvt_change_mode(TEST_MODE_2);

	if (nvt_check_fw_status()) {
		return -EAGAIN;
	}

	if (nvt_get_fw_info()) {
	return -EAGAIN;
		}
*/
	if (nvt_get_fw_pipe() == 0)
		nvt_read_mdata(RAW_PIPE0_ADDR, RAW_BTN_PIPE0_ADDR);
	else
		nvt_read_mdata(RAW_PIPE1_ADDR, RAW_BTN_PIPE1_ADDR);

	nvt_get_mdata(xdata, &x_num, &y_num);

	for (y = 0; y < y_num; y++) {
		for (x = 0; x < x_num; x++) {
			xdata[y * x_num + x] =  (int16_t)xdata[y * x_num + x];
		}
	}

//	nvt_change_mode(NORMAL_MODE);

	fbufp = (char *)vmalloc(8192);
	if (!fbufp) {
		TS_LOG_ERR("%s: vmalloc for fbufp failed!\n", __func__);
		return -ENOMEM;
	}

	printk("%s:\n", __func__);
	for (y = 0; y < Y_Channel; y++) {
		for (x = 0; x < X_Channel; x++) {
			iArrayIndex = y * X_Channel + x;
			printk("%5d ", xdata[iArrayIndex]);
			sprintf(fbufp + 7 * (y * X_Channel + x) + y * 2, "%5d, ", xdata[y * X_Channel + x]) ;
		}
		printk("\n");
		sprintf(fbufp + 7 * (y * X_Channel + x) + y * 2,"\r\n");
	}
	printk("\n");

	org_fs = get_fs();
	set_fs(KERNEL_DS);
	fp = filp_open(file_path, O_RDWR | O_CREAT, 0644);
	if (fp == NULL || IS_ERR(fp)) {
		TS_LOG_ERR("%s: open %s failed\n", __func__, file_path);
		set_fs(org_fs);
		if (fbufp) {
			vfree(fbufp);
			fbufp = NULL;
		}
		return -1;
	}

	output_len = (Y_Channel * X_Channel * 7 + Y_Channel * 2);
	write_ret = vfs_write(fp, (char __user *)fbufp, output_len, &pos);
	if (write_ret <= 0) {
		TS_LOG_ERR("%s: write %s failed\n", __func__, file_path);
		set_fs(org_fs);
		if (fp) {
			filp_close(fp, NULL);
			fp = NULL;
		}
		if (fbufp) {
			vfree(fbufp);
			fbufp = NULL;
		}
		return -1;
	}

	set_fs(org_fs);
	if (fp) {
		filp_close(fp, NULL);
		fp = NULL;
	}
	if (fbufp) {
		vfree(fbufp);
		fbufp = NULL;
	}

	TS_LOG_INFO("%s:--\n", __func__);
	return 0;
}

static int32_t nvt_read_CC(int32_t *xdata)
{
	uint8_t x_num = 0;
	uint8_t y_num = 0;
	uint32_t x = 0;
	uint32_t y = 0;
	int32_t iArrayIndex = 0;
	char *fbufp = NULL;
	struct file *fp = NULL;
	mm_segment_t org_fs;
	char file_path[64];
	loff_t pos = 0;
	int32_t write_ret = 0;
	uint32_t output_len = 0;

	TS_LOG_INFO("%s:++\n", __func__);

	snprintf(file_path, sizeof(file_path), "/data/nvt_data/%s_FWCCTest.csv", novatek_project_id);
/*
	if (nvt_clear_fw_status()) {
		return -EAGAIN;
	}

	nvt_change_mode(MP_MODE_CC);

	if (nvt_check_fw_status()) {
		return -EAGAIN;
	}

	nvt_get_fw_info();
*/
	if (nvt_get_fw_pipe() == 0)
		nvt_read_mdata(DIFF_PIPE1_ADDR, DIFF_BTN_PIPE1_ADDR);
	else
		nvt_read_mdata(DIFF_PIPE0_ADDR, DIFF_BTN_PIPE0_ADDR);

	nvt_get_mdata(xdata, &x_num, &y_num);

	for (y = 0; y < y_num; y++) {
		for (x = 0; x < x_num; x++) {
			xdata[y * x_num + x] =  (int16_t)xdata[y * x_num + x];
		}
	}

//	nvt_change_mode(NORMAL_MODE);

	fbufp = (char *)vmalloc(8192);
	if (!fbufp) {
		TS_LOG_ERR("%s: vmalloc for fbufp failed!\n", __func__);
		return -ENOMEM;
	}

	printk("%s:\n", __func__);
	for (y = 0; y < Y_Channel; y++) {
		for (x = 0; x < X_Channel; x++) {
			iArrayIndex = y * X_Channel + x;
			printk("%5d ", xdata[iArrayIndex]);
			sprintf(fbufp + 7 * (y * X_Channel + x) + y * 2, "%5d, ", xdata[y * X_Channel + x]) ;
		}
		printk("\n");
		sprintf(fbufp + 7 * (y * X_Channel + x) + y * 2,"\r\n");
	}
	printk("\n");

	org_fs = get_fs();
	set_fs(KERNEL_DS);
	fp = filp_open(file_path, O_RDWR | O_CREAT, 0644);
	if (fp == NULL || IS_ERR(fp)) {
		TS_LOG_ERR("%s: open %s failed\n", __func__, file_path);
		set_fs(org_fs);
		if (fbufp) {
			vfree(fbufp);
			fbufp = NULL;
		}
		return -1;
	}

	output_len = (Y_Channel * X_Channel * 7 + Y_Channel * 2);
	write_ret = vfs_write(fp, (char __user *)fbufp, output_len, &pos);
	if (write_ret <= 0) {
		TS_LOG_ERR("%s: write %s failed\n", __func__, file_path);
		set_fs(org_fs);
		if (fp) {
			filp_close(fp, NULL);
			fp = NULL;
		}
		if (fbufp) {
			vfree(fbufp);
			fbufp = NULL;
		}
		return -1;
	}

	set_fs(org_fs);
	if (fp) {
		filp_close(fp, NULL);
		fp = NULL;
	}
	if (fbufp) {
		vfree(fbufp);
		fbufp = NULL;
	}

	TS_LOG_INFO("%s:--\n", __func__);
	return 0;
}

static int32_t nvt_read_diff(int32_t *xdata)
{
	uint8_t x_num = 0;
	uint8_t y_num = 0;
	uint32_t x = 0;
	uint32_t y = 0;

	TS_LOG_INFO("%s:++\n", __func__);
/*
	if (nvt_clear_fw_status()) {
		return -EAGAIN;
	}

	nvt_change_mode(TEST_MODE_2);

	if (nvt_check_fw_status()) {
		return -EAGAIN;
	}

	nvt_get_fw_info();
*/
	if (nvt_get_fw_pipe() == 0)
		nvt_read_mdata(DIFF_PIPE0_ADDR, DIFF_BTN_PIPE0_ADDR);
	else
		nvt_read_mdata(DIFF_PIPE1_ADDR, DIFF_BTN_PIPE1_ADDR);

	nvt_get_mdata(xdata, &x_num, &y_num);

	for (y = 0; y < y_num; y++) {
		for (x = 0; x < x_num; x++) {
			xdata[y * x_num + x] =  (int16_t)xdata[y * x_num + x];
		}
	}

//	nvt_change_mode(NORMAL_MODE);

	TS_LOG_INFO("%s:--\n", __func__);

	return 0;
}

static int32_t nvt_read_noise(void)
{
	int32_t x = 0;
	int32_t y = 0;
	int32_t iArrayIndex = 0;
	char *fbufp = NULL;
	struct file *fp = NULL;
	mm_segment_t org_fs;
	char file_path[64];
	loff_t pos = 0;
	int32_t write_ret = 0;
	uint32_t output_len = 0;

	snprintf(file_path, sizeof(file_path), "/data/nvt_data/%s_NoiseTest.csv", novatek_project_id);

	if (nvt_read_diff(RawData_Diff)) {
		return 1; // read data failed
	}

	fbufp = (char *)vmalloc(8192);
	if (!fbufp) {
		TS_LOG_ERR("%s: vmalloc for fbufp failed!\n", __func__);
		return -ENOMEM;
	}

	printk("%s:RawData_Diff:\n", __func__);
	for (y = 0; y < Y_Channel; y++) {
		for (x = 0; x < X_Channel; x++) {
			iArrayIndex = y * X_Channel + x;
			printk("%5d ", RawData_Diff[iArrayIndex]);
			sprintf(fbufp + 7 * (y * X_Channel + x) + y * 2, "%5d, ", RawData_Diff[y * X_Channel + x]) ;
		}
		printk("\n");
		sprintf(fbufp + 7 * (y * X_Channel + x) + y * 2,"\r\n");
	}
	printk("\n");

	org_fs = get_fs();
	set_fs(KERNEL_DS);
	fp = filp_open(file_path, O_RDWR | O_CREAT, 0644);
	if (fp == NULL || IS_ERR(fp)) {
		TS_LOG_ERR("%s: open %s failed\n", __func__, file_path);
		set_fs(org_fs);
		if (fbufp) {
			vfree(fbufp);
			fbufp = NULL;
		}
		return -1;
	}

	output_len = (Y_Channel * X_Channel * 7 + Y_Channel * 2) * 2;
	write_ret = vfs_write(fp, (char __user *)fbufp, output_len, &pos);
	if (write_ret <= 0) {
		TS_LOG_ERR("%s: write %s failed\n", __func__, file_path);
		set_fs(org_fs);
		if (fp) {
			filp_close(fp, NULL);
			fp = NULL;
		}
		if (fbufp) {
			vfree(fbufp);
			fbufp = NULL;
		}
		return -1;
	}

	set_fs(org_fs);
	if (fp) {
		filp_close(fp, NULL);
		fp = NULL;
	}
	if (fbufp) {
		vfree(fbufp);
		fbufp = NULL;
	}

	return 0;
}

static void nvt_enable_short_test(void)
{
	uint8_t buf[8] = {0};

	//---set xdata index to 0x11E00---
	buf[0] = 0xFF;
	buf[1] = 0x01;
	buf[2] = 0x1E;
	novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 3);

	//---enable short test---
	buf[0] = 0x50;
	buf[1] = 0x43;
	buf[2] = 0xAA;
	buf[3] = 0x02;
	buf[4] = 0x00;
	novatek_ts_i2c_write(ts->client, I2C_FW_Address, buf, 5);
}

static int32_t nvt_polling_hand_shake_status(void)
{
	uint8_t buf[8] = {0};
	int32_t i = 0;
	const int32_t retry = 200;

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

		if ((buf[1] == 0xA0) || (buf[1] == 0xA1))
			break;

		msleep(10);
	}

	if (i >= retry) {
		TS_LOG_ERR("%s: polling hand shake status failed, buf[1]=0x%02X\n", __func__, buf[1]);
		return -1;
	} else {
		return 0;
	}
}

static int32_t nvt_read_fw_short(int32_t *xdata)
{
	uint8_t x_num = 0;
	uint8_t y_num = 0;
	uint32_t x = 0;
	uint32_t y = 0;
	int32_t iArrayIndex = 0;
	char *fbufp = NULL;
	struct file *fp = NULL;
	mm_segment_t org_fs;
	char file_path[64];
	loff_t pos = 0;
	int32_t write_ret = 0;
	uint32_t output_len = 0;

	TS_LOG_INFO("%s:++\n", __func__);

	snprintf(file_path, sizeof(file_path), "/data/nvt_data/%s_ShortTest.csv", novatek_project_id);

	if (nvt_clear_fw_status()) {
		return -EAGAIN;
	}

	nvt_enable_short_test();

	if (nvt_polling_hand_shake_status()) {
		return -EAGAIN;
	}

	if (nvt_get_fw_info()) {
		return -EAGAIN;
	}

	if (nvt_get_fw_pipe() == 0)
		nvt_read_mdata(RAW_PIPE0_ADDR, RAW_BTN_PIPE0_ADDR);
	else
		nvt_read_mdata(RAW_PIPE1_ADDR, RAW_BTN_PIPE1_ADDR);

	nvt_get_mdata(xdata, &x_num, &y_num);

	for (y = 0; y < y_num; y++) {
		for (x = 0; x < x_num; x++) {
			xdata[y * x_num + x] =  (int16_t)xdata[y * x_num + x];
		}
	}

	nvt_change_mode(NORMAL_MODE);

	fbufp = (char *)vmalloc(8192);
	if (!fbufp) {
		TS_LOG_ERR("%s: vmalloc for fbufp failed!\n", __func__);
		return -ENOMEM;
	}

	printk("%s:\n", __func__);
	for (y = 0; y < Y_Channel; y++) {
		for (x = 0; x < X_Channel; x++) {
			iArrayIndex = y * X_Channel + x;
			printk("%5d ", xdata[iArrayIndex]);
			sprintf(fbufp + 7 * (y * X_Channel + x) + y * 2, "%5d, ", xdata[y * X_Channel + x]) ;
		}
		printk("\n");
		sprintf(fbufp + 7 * (y * X_Channel + x) + y * 2,"\r\n");
	}
	printk("\n");

	org_fs = get_fs();
	set_fs(KERNEL_DS);
	fp = filp_open(file_path, O_RDWR | O_CREAT, 0644);
	if (fp == NULL || IS_ERR(fp)) {
		TS_LOG_ERR("%s: open %s failed\n", __func__, file_path);
		set_fs(org_fs);
		if (fbufp) {
			vfree(fbufp);
			fbufp = NULL;
		}
		return -1;
	}

	output_len = (Y_Channel * X_Channel * 7 + Y_Channel * 2);
	write_ret = vfs_write(fp, (char __user *)fbufp, output_len, &pos);
	if (write_ret <= 0) {
		TS_LOG_ERR("%s: write %s failed\n", __func__, file_path);
		set_fs(org_fs);
		if (fp) {
			filp_close(fp, NULL);
			fp = NULL;
		}
		if (fbufp) {
			vfree(fbufp);
			fbufp = NULL;
		}
		return -1;
	}

	set_fs(org_fs);
	if (fp) {
		filp_close(fp, NULL);
		fp = NULL;
	}
	if (fbufp) {
		vfree(fbufp);
		fbufp = NULL;
	}

	TS_LOG_INFO("%s:--\n", __func__);

	return 0;
}

/*******************************************************
Description:
	Novatek touchscreen raw data delta test function.

return:
	Executive outcomes. 0---passed. negative---failed.
*******************************************************/
static int32_t nvt_rawdata_up_low(int32_t rawdata[], uint8_t RecordResult[], uint8_t x_len, uint8_t y_len, int32_t Upper_Lmt[], int32_t Lower_Lmt[])
{
	int32_t retval = 0;
	int32_t i = 0;
	int32_t j = 0;

	//---Check Lower & Upper Limit---
	for (j = 0; j < y_len; j++) {
		for (i = 0; i < x_len; i++) {		
			if(rawdata[j * x_len + i] > Upper_Lmt[j * x_len + i]) {
				RecordResult[j * x_len + i] |= 0x01;
				retval = -1;
			}

			if(rawdata[j * x_len + i] < Lower_Lmt[j * x_len + i]) {
				RecordResult[j * x_len + i] |= 0x02;
				retval = -1;
			}
		}
	}

	TS_LOG_INFO("%s:--\n", __func__);

	//---Return Result---
	return retval;
}

/*******************************************************
Description:
	Novatek touchscreen calculate G Ratio and Normal
	function.

return:
	Executive outcomes. 0---succeed. 1---failed.
*******************************************************/
static int32_t nvt_rawdata_delta(int32_t rawdata[], uint8_t x_len, uint8_t y_len)
{
	int32_t retval = 0;
	int32_t i = 0;
	int32_t j = 0;
	int32_t delta = 0;

	TS_LOG_INFO("%s:++\n", __func__);

	//---Check X Delta---
	for (j = 0; j < y_len; j++) {
		for (i = 0; i < (x_len-1); i++) {
			RecordResult_FWMutual_X_Delta[j * (x_len-1) + i] = (rawdata[j * x_len + i] - rawdata[j * x_len + (i+1)]);
		}
	}
	
	printk("%s:RawData_X_Delta:\n", __func__);
	for (j = 0; j < y_len; j++) {
		for (i = 0; i < (x_len-1); i++) {
			if(RecordResult_FWMutual_X_Delta[j * (x_len-1) + i] > 0) {
				delta = RecordResult_FWMutual_X_Delta[j * (x_len-1) + i];
			} else {
				delta = (0 - RecordResult_FWMutual_X_Delta[j * (x_len-1) + i]);
			}
			
			if (delta > PS_Config_Lmt_FW_Rawdata_X_Delta[j * (x_len-1) + i]) {
				retval = -1;
			}
			printk("%5d, ", delta);
		}
		printk("\n");
	}
	printk("\n");
	

	//---Check Y Delta---
	for (j = 0; j < x_len; j++) {
		for (i = 0; i < (y_len-1); i++) {
			RecordResult_FWMutual_Y_Delta[i * x_len + j] = (rawdata[i * x_len + j] - rawdata[(i+1) * x_len + j]);
		}
	}
	
	printk("%s:RawData_Y_Delta:\n", __func__);
	for (j = 0; j < (y_len-1); j++) {
		for (i = 0; i < x_len; i++) {
			if(RecordResult_FWMutual_Y_Delta[j * x_len + i] > 0) {
				delta = RecordResult_FWMutual_Y_Delta[j * x_len + i];
			} else {
				delta = (0 - RecordResult_FWMutual_Y_Delta[j * x_len + i]);
			}

			if (delta > PS_Config_Lmt_FW_Rawdata_Y_Delta[j * x_len + i]) {
				retval = -1;
			}
			printk("%5d, ", delta);
		}
		printk("\n");
	}
	printk("\n");	

	TS_LOG_INFO("%s:--\n", __func__);

	//---Return Result---
	return retval;
}

/*******************************************************
Description:
	Novatek touchscreen raw data test function.

return:
	Executive outcomes. 0---passed. negative---failed.
*******************************************************/
static int32_t RawDataTest_Sub(int32_t rawdata[], uint8_t RecordResult[], uint8_t x_ch, uint8_t y_ch, int32_t Rawdata_Limit_Postive, int32_t Rawdata_Limit_Negative)
{
	int32_t i = 0;
	int32_t j = 0;
	int32_t iArrayIndex = 0;
	int32_t iTolLowBound = 0;
	int32_t iTolHighBound = 0;
	bool isAbsCriteria = false;
	bool isPass = true;

	if ((Rawdata_Limit_Postive != 0) || (Rawdata_Limit_Negative != 0))
		isAbsCriteria = true;

	for (j = 0; j < y_ch; j++) {
		for (i = 0; i < x_ch; i++) {
			iArrayIndex = j * x_ch + i;

			RecordResult[iArrayIndex] = 0x00; // default value for PASS

			if (isAbsCriteria) {
				iTolLowBound = Rawdata_Limit_Negative;
				iTolHighBound = Rawdata_Limit_Postive;
			}

			if(rawdata[iArrayIndex] > iTolHighBound)
				RecordResult[iArrayIndex] |= 0x01;

			if(rawdata[iArrayIndex] < iTolLowBound)
				RecordResult[iArrayIndex] |= 0x02;
		}
	}

	//---Check RecordResult---
	for (j = 0; j < y_ch; j++) {
		for (i = 0; i < x_ch; i++) {
			if (RecordResult[j * x_ch + i] != 0) {
				isPass = false;
				break;
			}
		}
	}

	if (isPass == false) {
		return -1; // FAIL
	} else {
		return 0; // PASS
	}
}

static int16_t nvt_get_avg(int32_t *p)
{
	int32_t sum=0;
	int i;

	for(i=0; i < (X_Channel*Y_Channel); i++){
	sum +=p[i];
	}

	return (int16_t) (sum / (X_Channel*Y_Channel));
}

static int16_t nvt_get_max(int32_t *p)
{
	int32_t max=INT_MIN;
	int i;

	for (i = 0; i <  (X_Channel*Y_Channel); i++) {
		max = max < p[i] ? p[i] : max;
	}

	return  (int16_t) max;
}

static int16_t nvt_get_min(int32_t *p)
{
	int32_t min=INT_MAX;
	int i;
    
	for (i = 0; i <  (X_Channel*Y_Channel); i++) {
		min = min >  p[i] ? p[i] : min;
	}
	
	return  (int16_t) min;
}
static int mmi_add_static_data(void)
{
 	int i;

	i=  strlen (mmitest_result);
	if  (i >=TP_MMI_RESULT_LEN) {
		return -EINVAL;
	}
	snprintf((mmitest_result+i),TP_MMI_RESULT_LEN - i,"[%5d,%5d,%5d]",
   	 nvt_get_avg(RawData_FWMutual), nvt_get_max(RawData_FWMutual),nvt_get_min(RawData_FWMutual));

	i= strlen(mmitest_result);
	if  (i >=TP_MMI_RESULT_LEN) {
		return -EINVAL;
	}
	snprintf((mmitest_result+i),TP_MMI_RESULT_LEN - i,"[%5d,%5d,%5d]",
   	 nvt_get_avg(RawData_Diff), nvt_get_max(RawData_Diff),nvt_get_min(RawData_Diff));

	return 0;
	
}
/*******************************************************
Description:
	Novatek touchscreen self-test criteria print
	function.

return:
	n.a.
*******************************************************/
void nvt_print_criteria(void)
{
	uint32_t x = 0;
	uint32_t y = 0;
	int32_t iArrayIndex = 0;

	TS_LOG_INFO("%s:++\n", __func__);

	//---PS_Config_Lmt_FW_Rawdata---
	printk("PS_Config_Lmt_FW_Rawdata_P:\n");
	for (y = 0; y < Y_Channel; y++) {
		for (x = 0; x < X_Channel; x++) {
			iArrayIndex = y * X_Channel + x;
			printk("%5d ", PS_Config_Lmt_FW_Rawdata_P[iArrayIndex]);
		}
		printk("\n");
	}
	printk("PS_Config_Lmt_FW_Rawdata_N:\n");
	for (y = 0; y < Y_Channel; y++) {
		for (x = 0; x < X_Channel; x++) {
			iArrayIndex = y * X_Channel + x;
			printk("%5d ", PS_Config_Lmt_FW_Rawdata_N[iArrayIndex]);
		}
		printk("\n");
	}

	//---PS_Config_Lmt_FW_CC---
	printk("PS_Config_Lmt_FW_CC_P: %5d\n", PS_Config_Lmt_FW_CC_P);
	printk("PS_Config_Lmt_FW_CC_N: %5d\n", PS_Config_Lmt_FW_CC_N);

	//---PS_Config_Lmt_FW_Diff---
	printk("PS_Config_Lmt_FW_Diff_P: %5d\n", PS_Config_Lmt_FW_Diff_P);
	printk("PS_Config_Lmt_FW_Diff_N: %5d\n", PS_Config_Lmt_FW_Diff_N);

	//---PS_Config_Lmt_FW_Rawdata_Delta---
	printk("PS_Config_Lmt_FW_Rawdata_X_Delta:\n");
	for (y = 0; y < Y_Channel; y++) {
		for (x = 0; x < (X_Channel-1); x++) {
			iArrayIndex = y * (X_Channel-1) + x;
			printk("%5d ", PS_Config_Lmt_FW_Rawdata_X_Delta[iArrayIndex]);
		}
		printk("\n");
	}
	printk("PS_Config_Lmt_FW_Rawdata_Y_Delta:\n");
	for (y = 0; y < (Y_Channel-1); y++) {
		for (x = 0; x < X_Channel; x++) {
			iArrayIndex = y * X_Channel + x;
			printk("%5d ", PS_Config_Lmt_FW_Rawdata_Y_Delta[iArrayIndex]);
		}
		printk("\n");
	}

	//---PS_Config_Lmt_Short_Rawdata---
	printk("PS_Config_Lmt_Short_Rawdata_P: %5d\n", PS_Config_Lmt_Short_Rawdata_P);
	printk("PS_Config_Lmt_Short_Rawdata_N: %5d\n", PS_Config_Lmt_Short_Rawdata_N);	

	//---PS_Config_Lmt_Open_Rawdata---
	printk("PS_Config_Lmt_Open_Rawdata_P: %5d\n", PS_Config_Lmt_Open_Rawdata_P);
	printk("PS_Config_Lmt_Open_Rawdata_N: %5d\n", PS_Config_Lmt_Open_Rawdata_N);

	//---mADCOper_Cnt---
	printk("mADCOper_Cnt: %5d\n", mADCOper_Cnt);	

	TS_LOG_INFO("%s:--\n", __func__);
}
/*******************************************************
Description:
	Novatek touchscreen selftest function for huawei_touchscreen.

return:
	Executive outcomes. 0---succeed. negative---failed.
*******************************************************/
#define TP_TEST_FAILED_REASON_LEN 20

static char selftest_failed_reason[TP_TEST_FAILED_REASON_LEN] = { "-software_reason" };
int32_t nvt_selftest(struct ts_rawdata_info *info)
{
	uint8_t buf[8] = {0};
	char test_0_result[4]={0};
	char test_1_result[4]={0};
	char test_2_result[4]={0};
	char test_3_result[4]={0};
	char test_4_result[4]={0};
	
	//---For Debug : Test Time, Mallon 20160907---
	unsigned long timer_start=0,timer_end=0;
	timer_start=jiffies;

	//---print criteria ,mallon 20161012-----
	nvt_print_criteria();
	
	nvt_mp_isInitialed = 0;
	TestResult_Short = 0;
	TestResult_Open = 0;

	//---Create Folder for Sample Data Output---
	sys_mkdir("/data/nvt_data", 0664);

	//---Get ProjectID to match input & output files---
	//novatek_read_projectid();

	//---Test I2C Transfer---
	buf[0] = 0x00;
	if (novatek_ts_i2c_read(ts->client, I2C_FW_Address, buf, 2) < 0) {
		TS_LOG_ERR("%s: I2C READ FAIL!", __func__);
		strcpy(test_0_result, "0F-");
		strcpy(test_1_result, "1F-");
		strcpy(test_2_result, "2F-");
		strcpy(test_3_result, "3F-");
		strcpy(test_4_result, "4F");
		goto err_nvt_i2c_read;
	} else {
		strcpy(test_0_result, "0P-");
	}

	//--Mutex Lock---
	if (mutex_lock_interruptible(&ts->mp_mutex) != 0) {
		TS_LOG_ERR("%s: mutex_lock_interruptible FAIL!", __func__);
		strcpy(test_1_result, "1F-");
		strcpy(test_2_result, "2F-");
		strcpy(test_3_result, "3F-");
		strcpy(test_4_result, "4F");
		goto err_mutex_lock_interruptible;
	}

	//---Reset IC---
	//nvt_bootloader_reset();
	//nvt_check_fw_reset_state(RESET_STATE_REK_FINISH);

	//---Disable FW Frequence Hopping---
	nvt_switch_FreqHopEnDis(FREQ_HOP_DISABLE);
	if (nvt_check_fw_reset_state(RESET_STATE_NORMAL_RUN)) {
		TS_LOG_ERR("%s: nvt_check_fw_reset_state  FAIL!", __func__);
		goto err_nvt_check_fw_reset_state;
	}

	msleep(100);

	//---Enter test mode  ,Mallon 20160928---
        if (nvt_clear_fw_status()) {
		TS_LOG_ERR("%s: nvt_clear_fw_status  FAIL!", __func__);
		goto err_nvt_clear_fw_status;
	}

	nvt_change_mode(MP_MODE_CC);

	if (nvt_check_fw_status()) {
		TS_LOG_ERR("%s: nvt_check_fw_status  FAIL!", __func__);
		goto err_nvt_check_fw_status;
	}

	if (nvt_get_fw_info()) {
		TS_LOG_ERR("%s: nvt_get_fw_info  FAIL!", __func__);
		goto err_nvt_get_fw_info;
	}

	
	//---FW Rawdata Test---
	if (nvt_read_raw(RawData_FWMutual) != 0) {
		TestResult_FWMutual = 1;
		TS_LOG_INFO("%s: nvt_read_raw ERROR! TestResult_FWMutual=%d\n", __func__, TestResult_FWMutual);
		info->buff[0] = 0;
		info->buff[1] = 0;
		info->used_size = 0;
	} else {
		TestResult_FWMutual = nvt_rawdata_up_low(RawData_FWMutual, RecordResult_FWMutual, X_Channel, Y_Channel, 
													PS_Config_Lmt_FW_Rawdata_P, PS_Config_Lmt_FW_Rawdata_N);
		if (TestResult_FWMutual == -1){
			TS_LOG_INFO("%s: FW RAWDATA TEST FAIL! TestResult_FWMutual=%d\n", __func__, TestResult_FWMutual);
		} else {
			TS_LOG_INFO("%s: FW RAWDATA TEST PASS! TestResult_FWMutual=%d\n", __func__, TestResult_FWMutual);
		}
	}

	//---FW CC Test---	
	if (nvt_read_CC(RawData_FW_CC) != 0) {
		TestResult_FW_CC = 1;
		TS_LOG_INFO("%s: nvt_read_CC ERROR! TestResult_FW_CC=%d\n", __func__, TestResult_FW_CC);
		info->buff[0] = 0;
		info->buff[1] = 0;
		info->used_size = 0;
	} else {
		TestResult_FW_CC = RawDataTest_Sub(RawData_FW_CC, RecordResult_FW_CC, X_Channel, Y_Channel,
											PS_Config_Lmt_FW_CC_P, PS_Config_Lmt_FW_CC_N);
		if (TestResult_FW_CC == -1){
			TS_LOG_INFO("%s: FW CC TEST FAIL! TestResult_FW_CC=%d\n", __func__, TestResult_FW_CC);
		} else {
			TS_LOG_INFO("%s: FW CC TEST PASS! TestResult_FW_CC=%d\n", __func__, TestResult_FW_CC);
		}
	}

	//--- Result for FW Rawdata & CC Test---
	if ((TestResult_FWMutual != 0) || (TestResult_FW_CC != 0)) {
		strcpy(test_1_result, "1F-");
	} else {
		strcpy(test_1_result, "1P-");
	}

	//---Offset between Columns and Raws---
	//---XY Nearby Delta Test---
	if (nvt_rawdata_delta(RawData_FWMutual, X_Channel, Y_Channel) != 0) {
		TestResult_FWMutual_Delta = -1;	// -1: FAIL
		TS_LOG_INFO("%s: XY Nearby Delta Test FAIL! TestResult_FWMutual_Delta=%d\n", __func__, TestResult_FWMutual_Delta);
		info->buff[0] = 0;
		info->buff[1] = 0;
		info->used_size = 0;
	} else {
		TestResult_FWMutual_Delta = 0;
		TS_LOG_INFO("%s: XY Nearby Delta Test PASS! TestResult_FWMutual_Delta=%d\n", __func__, TestResult_FWMutual_Delta);
	}

	//--- Result for XY Nearby Delta Test---
	if (TestResult_FWMutual_Delta != 0) {
		strcpy(test_2_result, "2F-");
	} else {
		strcpy(test_2_result, "2P-");
	}
	
	//---Noise Test---
	if (nvt_read_noise() != 0) {
		TestResult_Noise = 1;	// 1: ERROR
		TestResult_FW_Diff = 1;
		TS_LOG_INFO("%s: nvt_read_noise ERROR! TestResult_Noise=%d\n", __func__, TestResult_Noise);
		info->buff[0] = 0;
		info->buff[1] = 0;
		info->used_size = 0;
	} else {
		TestResult_FW_Diff = RawDataTest_Sub(RawData_Diff, RecordResult_FW_Diff, X_Channel, Y_Channel,
											PS_Config_Lmt_FW_Diff_P, PS_Config_Lmt_FW_Diff_N);
		if (TestResult_FW_Diff == -1) {
			TestResult_Noise = -1;
			TS_LOG_INFO("%s: NOISE TEST FAIL! TestResult_Noise=%d\n", __func__, TestResult_Noise);
		} else {
			TestResult_Noise = 0;
			TS_LOG_INFO("%s: NOISE TEST PASS! TestResult_Noise=%d\n", __func__, TestResult_Noise);
		}
	}

	//--- Result for Noise Test---
	if (TestResult_Noise != 0) {
		strcpy(test_3_result, "3F-");
	} else {
		strcpy(test_3_result, "3P-");
	}
	
	//---set FW to Normal Mode for short Test, 20160928----
	nvt_change_mode(NORMAL_MODE);
	
	//---Short Test (in FW)---
	if (nvt_read_fw_short(RawData_Short) != 0) {
		TestResult_Short = 1; // 1:ERROR
		TS_LOG_INFO("%s: nvt_read_short ERROR! TestResult_Short=%d\n", __func__, TestResult_Short);
		info->buff[0] = 0;
		info->buff[1] = 0;
		info->used_size = 0;
	} else {
		//---Self Test Check --- // 0:PASS, -1:FAIL
		TestResult_Short = RawDataTest_Sub(RawData_Short, RecordResult_Short, X_Channel, Y_Channel,
											PS_Config_Lmt_Short_Rawdata_P, PS_Config_Lmt_Short_Rawdata_N);
		if (TestResult_Short == -1){
			TS_LOG_INFO("%s: SHORT TEST FAIL! TestResult_Short=%d\n", __func__, TestResult_Short);
		} else {
			TS_LOG_INFO("%s: SHORT TEST PASS! TestResult_Short=%d\n", __func__, TestResult_Short);
		}
	}

	//---Enable FW Frequence Hopping---
	//invt_switch_FreqHopEnDis(FREQ_HOP_ENABLE);

	//---Open Test---
	if (nvt_read_open() != 0) {
		TestResult_Open = 1;	// 1:ERROR
		TS_LOG_INFO("%s: nvt_read_open ERROR! TestResult_Open=%d\n", __func__, TestResult_Open);
		info->buff[0] = 0;
		info->buff[1] = 0;
		info->used_size = 0;
	} else {
		//---Self Test Check --- // 0:PASS, -1:FAIL
		TestResult_Open = RawDataTest_Sub(RawData_Open, RecordResult_Open, X_Channel, Y_Channel,
											PS_Config_Lmt_Open_Rawdata_P, PS_Config_Lmt_Open_Rawdata_N);
		if (TestResult_Open == -1){
			TS_LOG_INFO("%s: OPEN TEST FAIL! TestResult_Open=%d\n", __func__, TestResult_Open);
		} else {
			TS_LOG_INFO("%s: OPEN TEST PASS! TestResult_Open=%d\n", __func__, TestResult_Open);
		}
	}

	//--- Result for Open & Short Test---
	if ((TestResult_Short != 0) || (TestResult_Open != 0)) {
		strcpy(test_4_result, "4F");
	} else {
		strcpy(test_4_result, "4P");
	}

	//---Reset IC---
	nvt_bootloader_reset();

	//---Mutex Unlock---
	mutex_unlock(&ts->mp_mutex);

	//---Copy Data to info->buff---
	info->buff[0] = X_Channel;
	info->buff[1] = Y_Channel;
	info->used_size = X_Channel * Y_Channel * 2  + 2;
	memcpy(&info->buff[2], RawData_FWMutual, (X_Channel*Y_Channel*4));
	memcpy(&info->buff[X_Channel * Y_Channel + 2], RawData_Diff, (X_Channel*Y_Channel*4));
	
err_nvt_get_fw_info:
err_nvt_check_fw_status:	
err_nvt_clear_fw_status:
err_nvt_check_fw_reset_state:	
err_mutex_lock_interruptible:
err_nvt_i2c_read:
	//---Check Fail Reason---
	if((TestResult_Short == -1) || (TestResult_Open == -1) ||
		(TestResult_FWMutual_Delta == -1) || (TestResult_Noise ==-1)||
		TestResult_FW_CC==-1||TestResult_FWMutual==-1)
		strncpy(selftest_failed_reason, "-panel_reason", TP_TEST_FAILED_REASON_LEN);

	//---String Copy---
	memset(mmitest_result, 0, sizeof(mmitest_result));
	strncat(mmitest_result, test_0_result, strlen(test_0_result));
	strncat(mmitest_result, test_1_result, strlen(test_1_result));
	strncat(mmitest_result, test_2_result, strlen(test_2_result));
	strncat(mmitest_result, test_3_result, strlen(test_3_result));
	strncat(mmitest_result, test_4_result, strlen(test_4_result));
	//----add static data in SH ,Mallon 20160928---
	mmi_add_static_data();
	strncat(mmitest_result, ";", strlen(";"));
	if (0 == strlen(mmitest_result) || strstr(mmitest_result, "F")) {
		strncat(mmitest_result, selftest_failed_reason, strlen(selftest_failed_reason));
	}

	 //------change project_id in SH, Mallon 20160928---
	//strncat(mmitest_result, "-novatek-nt36772", strlen("-novatek-nt36772"));
	strncat(mmitest_result, "-novatek_", strlen("-novatek_"));
	strncat(mmitest_result, novatek_project_id, PROJECT_ID_LEN); 
	  
	//---Copy String to Result---
	memcpy(info->result, mmitest_result, strlen(mmitest_result));
	
	//---For Debug : Test Time, Mallon 20160907---
	timer_end = jiffies;
	TS_LOG_INFO("%s: self test time:%d\n", __func__, jiffies_to_msecs(timer_end-timer_start));

	return NO_ERR;
}
