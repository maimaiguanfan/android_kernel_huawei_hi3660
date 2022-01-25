/* Copyright (c) 2017-2018, Huawei terminal Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/

#ifndef __LCD_KIT_DISP_H_
#define __LCD_KIT_DISP_H_
/***********************************************************
*macro definition
***********************************************************/
#include "lcd_kit_common.h"
#include "hisi_fb.h"
#include "hisi_mipi_dsi.h"
#include "lcd_kit_utils.h"
//////////////////////////////macro////////////////////////////////
#define DTS_COMP_LCD_KIT_PANEL_TYPE     "huawei,lcd_panel_type"
#define LCD_KIT_PANEL_COMP_LENGTH       128
//app setting default backlight
#define MAX_BACKLIGHT_FROM_APP  (993)//995 from huguangyu
#define MIN_BACKLIGHT_FROM_APP  (500)
#define BACKLIGHT_HIGH_LEVEL (1)
#define BACKLIGHT_LOW_LEVEL  (2)
#define SCBAKDATA11 (0x438)
//pt test state
#define IN_POWER_TEST 2
#define LCD_POWER_LEN 3

//UD PrintFinger HBM
#define LCD_KIT_FP_HBM_ENTER 1
#define LCD_KIT_FP_HBM_EXIT  2
#define LCD_KIT_DATA_LEN_TWO 2
#define LCD_KIT_ENABLE_ELVSSDIM  0
#define LCD_KIT_DISABLE_ELVSSDIM 1
#define LCD_KIT_ELVSSDIM_NO_WAIT 0

struct lcd_kit_disp_info *lcd_kit_get_disp_info(void);
#define disp_info	lcd_kit_get_disp_info()
//////////////////////////////ENUM////////////////////////////////
enum alpm_mode {
	ALPM_DISPLAY_OFF,
	ALPM_ON_MIDDLE_LIGHT,
	ALPM_EXIT,
	ALPM_ON_LOW_LIGHT,
};

//////////////////////STRUCT///////////////////////
struct lcd_kit_disp_info {
	/********************running test****************/
	/*pcd err flag test*/
	u32 pcd_errflag_check_support;
	u32 gpio_pcd;
	u32 gpio_errflag;
	/*backlight open short test*/
	u32 bkl_open_short_support;
	struct lcd_kit_cascade_ic cascade_ic;
	/*check sum test*/
	struct lcd_kit_checksum checksum;
	/*adc sample vsp voltage*/
	struct lcd_kit_hkadc hkadc;
	/*current detect*/
	struct lcd_kit_current_detect current_det;
	/*lv detect*/
	struct lcd_kit_lv_detect lv_det;
	/*ldo check*/
	struct lcd_kit_ldo_check ldo_check;
	/*vertical line test*/
	struct lcd_kit_vertical_line vertical_line;
	/*pcd errflag*/
	struct lcd_kit_pcd_errflag pcd_errflag;
	/********************end****************/
	/********************effect****************/
	/*gamma calibration*/
	struct lcd_kit_gamma gamma_cal;
	/*oem information*/
	struct lcd_kit_oem_info oeminfo;
	/*rgbw function*/
	struct lcd_kit_rgbw rgbw;
	/********************end****************/
	/********************normal****************/
	u8 bl_is_shield_backlight;
	u8 bl_is_start_second_timer;
	/*lcd type*/
	u32 lcd_type;
	/*panel information*/
	char* compatible;
	/*board version*/
	u32 board_version;
	/*product id*/
	u32 product_id;
	/*dsi1 support*/
	u32 dsi1_cmd_support;
	/*vr support*/
	u32 vr_support;
	/*lcd kit semaphore*/
	struct semaphore lcd_kit_sem;
	/*lcd kit mipi mutex lock*/
	struct mutex mipi_lock;
	/*alpm -aod*/
	struct lcd_kit_alpm alpm;
	/*quickly sleep out*/
	struct lcd_kit_quickly_sleep_out quickly_sleep_out;
	/*fps ctrl*/
	struct lcd_kit_fps fps;
	/*project id*/
	struct lcd_kit_project_id project_id;
	/*panel version*/
	struct lcd_kit_panel_version panel_version;
	/*otp gamma*/
	struct lcd_kit_otp_gamma otp_gamma;
	/********************end****************/
};

/***********************************************************
*variable declaration
***********************************************************/
/*extern variable*/
extern int lcd_kit_power_init(struct platform_device* pdev);
extern int lcd_kit_sysfs_init(void);
extern int lcd_kit_dbg_init(void);
/***********************************************************
*function declaration
***********************************************************/
#endif
