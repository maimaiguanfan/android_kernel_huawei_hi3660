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

#include "hisi_fb.h"
#include "lcd_kit_common.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_dbg.h"
#include <huawei_platform/log/log_jank.h>
#include "global_ddr_map.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_adapt.h"
#include "lcd_kit_power.h"
#ifdef LV_GET_LCDBK_ON
#include <huawei_platform/inputhub/sensor_feima_ext.h>
#endif

static int lcd_kit_set_fastboot(struct platform_device* pdev);
static int lcd_kit_on(struct platform_device* pdev);
static int lcd_kit_off(struct platform_device* pdev);
static int lcd_kit_remove(struct platform_device* pdev);
static int lcd_kit_set_backlight(struct platform_device* pdev, uint32_t bl_level);
static int lcd_kit_set_backlight_by_type(struct platform_device* pdev, int backlight_type);
static int lcd_kit_set_hbm_for_mmi(struct platform_device* pdev, int level);
static int lcd_kit_esd_check(struct platform_device* pdev);
static int lcd_kit_set_display_region(struct platform_device* pdev, struct dss_rect* dirty);
static int lcd_kit_fps_scence_handle(struct platform_device* pdev, uint32_t scence);
static int lcd_kit_fps_updt_handle(struct platform_device* pdev);
static int lcd_kit_set_tcon_mode(struct platform_device *pdev, uint8_t mode);
static ssize_t lcd_kit_ce_mode_store(struct platform_device* pdev, const char* buf, size_t count);
static ssize_t lcd_kit_rgbw_set_func(struct hisi_fb_data_type* hisifd);
static ssize_t lcd_kit_hbm_set_func(struct hisi_fb_data_type* hisifd);
static ssize_t lcd_kit_cabc_store(struct platform_device* pdev, const char* buf, size_t count);
static ssize_t lcd_kit_color_param_get_func(struct hisi_fb_data_type* hisifd);
static int lcd_kit_get_pt_ulps_support(struct platform_device* pdev);
int g_max_backlight_from_app = MAX_BACKLIGHT_FROM_APP;
int g_min_backlight_from_app = MIN_BACKLIGHT_FROM_APP;
/*variable declare*/
static struct timer_list backlight_second_timer;
static struct lcd_kit_disp_info g_lcd_kit_disp_info;
/*******************************************************************************
**hisi panel data & pinfo
*/
static struct hisi_panel_info lcd_kit_pinfo = {0};
static struct hisi_fb_panel_data lcd_kit_data = {
	.panel_info = &lcd_kit_pinfo,
	.set_fastboot = lcd_kit_set_fastboot,
	.on = lcd_kit_on,
	.off = lcd_kit_off,
	.remove = lcd_kit_remove,
	.set_backlight = lcd_kit_set_backlight,
	.lcd_set_backlight_by_type_func = lcd_kit_set_backlight_by_type,
	.lcd_set_hbm_for_mmi_func = lcd_kit_set_hbm_for_mmi,
	.esd_handle = lcd_kit_esd_check,
	.set_display_region = lcd_kit_set_display_region,
	.lcd_fps_scence_handle = lcd_kit_fps_scence_handle,
	.lcd_fps_updt_handle = lcd_kit_fps_updt_handle,
	.lcd_ce_mode_store = lcd_kit_ce_mode_store,
	.lcd_rgbw_set_func = lcd_kit_rgbw_set_func,
	.lcd_hbm_set_func  = lcd_kit_hbm_set_func,
	.lcd_color_param_get_func = lcd_kit_color_param_get_func,
	.lcd_cabc_mode_store = lcd_kit_cabc_store,
	.panel_bypass_powerdown_ulps_support = lcd_kit_get_pt_ulps_support,
	.set_tcon_mode = lcd_kit_set_tcon_mode,
};

struct lcd_kit_disp_info *lcd_kit_get_disp_info(void)
{
	return &g_lcd_kit_disp_info;
}
static int uc_panel_is_power_on = 0;
static int lcd_kit_panel_is_power_on(struct hisi_fb_data_type* hisifd)
{
	uint32_t temp = 0;
	int ret       = 0;
	char __iomem* sctrl_base = NULL;
	/*bit[8] = 1 : lcd power on
	*bit[8] = 0 : lcd power off*/
	sctrl_base = hisifd->sctrl_base;
	temp = inp32(sctrl_base + SCBAKDATA11);
	ret  = (temp & 0x100) >> 8;

	LCD_KIT_INFO("inp32(SOC_SCTRL_SCBAKDATA11_ADDR(SOC_ACPU_SCTRL_BASE_ADDR))= 0x%x bit[8] = %d!\n",
				temp, ret);
	return ret;
}
static void lcd_kit_clear_sctrl_reg(struct hisi_fb_data_type* hisifd)
{
	uint32_t temp = 0;
	char __iomem* sctrl_base = NULL;

	/*bit[8] = 1 : lcd power on
	*bit[8] = 0 : lcd power off*/
	sctrl_base = hisifd->sctrl_base;
	temp = inp32(sctrl_base + SCBAKDATA11);
	temp &= ~(0x100);
	outp32(sctrl_base + SCBAKDATA11, temp);
	LCD_KIT_INFO("outp32(SOC_SCTRL_SCBAKDATA11_ADDR(SOC_ACPU_SCTRL_BASE_ADDR), 0x%x)\n",temp);
}

void lcd_kit_set_power_status(int status)
{
	uc_panel_is_power_on = status;
}

int lcd_kit_get_power_status(void)
{
	return uc_panel_is_power_on;
}

static int lcd_kit_set_elvss_dim_lp(struct hisi_fb_data_type* hisifd)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops* adapt_ops = NULL;

	if (hisifd == NULL) {
		LCD_KIT_ERR("param is NULL!\n");
		return LCD_KIT_FAIL;
	}

	adapt_ops = lcd_kit_get_adapt_ops();
	if (adapt_ops == NULL) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}

	if(common_info->hbm.hbm_set_elvss_dim_lp) {
		if (common_info->hbm.elvss_prepare_cmds.cmds != NULL){
			if (adapt_ops->mipi_tx) {
				ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.elvss_prepare_cmds);
			}
		}
		if (common_info->hbm.elvss_write_cmds.cmds != NULL){
			if (adapt_ops->mipi_tx) {
				common_info->hbm.elvss_write_cmds.cmds[0].wait = LCD_KIT_ELVSSDIM_NO_WAIT;
				common_info->hbm.elvss_write_cmds.cmds[0].payload[1] = common_info->hbm.ori_elvss_val | LCD_KIT_ENABLE_ELVSSDIM_MASK;
				ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.elvss_write_cmds);
				LCD_KIT_INFO("[power on set elvss dim] send:0x%x\n", common_info->hbm.elvss_write_cmds.cmds[0].payload[1]);
			}
		}
		if (common_info->hbm.elvss_post_cmds.cmds != NULL){
			if (adapt_ops->mipi_tx) {
				ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.elvss_post_cmds);
			}
		}
	} else {
		LCD_KIT_INFO("Do not support enable ELVSS Dim in power on, just return\n");
	}

	return ret;
}

static int lcd_kit_update_gamma_from_tpic(struct hisi_fb_data_type* hisifd)
{
	#define GAMMA_HEAD_LEN	2
	#define GAMMA_HEAD	0x47
	#define GAMMA_LEN	0x0a
	int ret = 0;

	if (disp_info->otp_gamma.support) {
		static bool has_read = false;
		struct ts_kit_ops *ts_ops = NULL;
		int i = 0;

		ts_ops = ts_kit_get_ops();
		if (!ts_ops) {
			LCD_KIT_ERR("ts_ops is null\n");
			return LCD_KIT_FAIL;
		}
		/*read gamma from tpic*/
		if (!has_read) {
			if (ts_ops->read_otp_gamma) {
				/*ret:0--invalide gamma, 1--valide gamma*/
				ret = ts_ops->read_otp_gamma(disp_info->otp_gamma.gamma, GAMMA_MAX);
			}
			has_read = true;
		}
		/*print gamma head and len*/
		LCD_KIT_INFO("HEAD:0x%x, LEN:0x%x\n", disp_info->otp_gamma.gamma[0], disp_info->otp_gamma.gamma[1]);
		/*verify gamma*/
		if ((disp_info->otp_gamma.gamma[0] != GAMMA_HEAD) || (disp_info->otp_gamma.gamma[1] != GAMMA_LEN)) {
			LCD_KIT_INFO("not otp gamma\n");
			return 0;
		}
		/*set up gamma cmds*/
		for (i = 0; i < (GAMMA_MAX - GAMMA_HEAD_LEN); i++) {
			disp_info->otp_gamma.gamma_cmds.cmds->payload[i+1] = disp_info->otp_gamma.gamma[i + GAMMA_HEAD_LEN];
		}
		/*adjust elvss*/
		ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->otp_gamma.elvss_cmds);
		if (ret) {
			LCD_KIT_ERR("send adjust elvss cmd error\n");
			return ret;
		}
		/*send otp gamma*/
		ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->otp_gamma.gamma_cmds);
		if (ret) {
			LCD_KIT_ERR("send otp gamma cmd error\n");
			return ret;
		}
	}
	return ret;
}

static void check_pcd_errflag_report(struct hisi_fb_data_type *hisifd)
{
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is NULL.\n");
		return;
	}
	(void)lcd_kit_check_pcd_errflag_check(hisifd);
}

static int lcd_kit_on(struct platform_device* pdev)
{
	struct hisi_fb_data_type* hisifd = NULL;
	struct hisi_panel_info* pinfo = NULL;
	struct lcd_kit_ops *lcd_ops = NULL;
	int ret = LCD_KIT_OK;
	char *panel_name = NULL;

	if (NULL == pdev) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	if (hisifd->aod_function) {
		LCD_KIT_INFO("AOD mode, bypass disp_kit_panel_on! \n");
		return LCD_KIT_OK;
	}

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null!\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("fb%d, +!\n", hisifd->index);

	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("panel_info is NULL!\n");
		return LCD_KIT_FAIL;
	}
	switch (pinfo->lcd_init_step) {
		case LCD_INIT_POWER_ON:
			lcd_kit_set_power_status(lcd_kit_panel_is_power_on(hisifd));
			if (common_ops->panel_power_on) {
				ret = common_ops->panel_power_on((void*)hisifd);
			}
			pinfo->lcd_init_step = LCD_INIT_MIPI_LP_SEND_SEQUENCE;
			panel_name = common_info->panel_model != NULL ? common_info->panel_model : disp_info->compatible;
			LCD_KIT_INFO("lcd_name is %s\n", panel_name);
			LOG_JANK_D(JLID_KERNEL_LCD_POWER_ON, "%s", "LCD_POWER_ON");
			break;
		case LCD_INIT_MIPI_LP_SEND_SEQUENCE:
			/*send mipi command by low power*/
			if (common_ops->panel_on_lp) {
				ret = common_ops->panel_on_lp((void*)hisifd);
			}
			/*update gamma*/
			lcd_kit_update_gamma_from_tpic(hisifd);
			lcd_kit_set_elvss_dim_lp(hisifd);
			pinfo->lcd_init_step = LCD_INIT_MIPI_HS_SEND_SEQUENCE;
			break;
		case LCD_INIT_MIPI_HS_SEND_SEQUENCE:
			/*send mipi command by high speed*/
			if (common_ops->panel_on_hs) {
				ret = common_ops->panel_on_hs((void*)hisifd);
			}
			/*record panel on time*/
			lcd_kit_disp_on_record_time();
			if(lcd_kit_get_power_status()) {
				lcd_kit_clear_sctrl_reg(hisifd);
				lcd_kit_set_power_status(0);
			}
			if (lcd_ops->power_monitor_on) {
				lcd_ops->power_monitor_on();
			}
			break;
		case LCD_INIT_NONE:
			break;
		case LCD_INIT_LDI_SEND_SEQUENCE:
			break;
		default:
			break;
	}
	// backlight on
	hisi_lcd_backlight_on(pdev);
	LCD_KIT_INFO("fb%d, -!\n", hisifd->index);
	return ret;
}

static void lcd_kit_remove_shield_backlight(void)
{
	if(disp_info->bl_is_shield_backlight == true) {
		disp_info->bl_is_shield_backlight = false;
	}
	if(false != disp_info->bl_is_start_second_timer) {
		del_timer(&backlight_second_timer);
		disp_info->bl_is_start_second_timer = false;
		LCD_KIT_INFO("panel powerOff, clear backlight shield timer.\n");
	}
}

/*
*name:lcd_kit_off
*function:power off panel
*@pdev:platform device
*/
static int lcd_kit_off(struct platform_device* pdev)
{
	struct hisi_fb_data_type* hisifd = NULL;
	struct hisi_panel_info* pinfo = NULL;
	struct lcd_kit_ops *lcd_ops = NULL;
	char *panel_name = common_info->panel_model != NULL ? common_info->panel_model : disp_info->compatible;

	if (NULL == pdev) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	if (hisifd->aod_function) {
		LCD_KIT_INFO("AOD mode, bypass disp_kit_panel_off! \n");
		return LCD_KIT_OK;
	}
	if (common_info->hbm.support) {
		common_info->hbm.hbm_level_current = 0;
	}
	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null!\n");
		return LCD_KIT_FAIL;
	}

	lcd_kit_remove_shield_backlight();
	LCD_KIT_INFO("fb%d, +!\n", hisifd->index);
	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("panel_info is NULL!\n");
		return LCD_KIT_FAIL;
	}
	switch (pinfo->lcd_uninit_step) {
		case LCD_UNINIT_MIPI_HS_SEND_SEQUENCE:
			/*check pcd errflag and report dmd*/
			check_pcd_errflag_report(hisifd);
			// check mipi errors
			common_ops->mipi_check(hisifd, panel_name, disp_info->quickly_sleep_out.panel_on_record_tv.tv_sec);
			// backlight off
			hisi_lcd_backlight_off(pdev);
			if (common_ops->panel_off_hs) {
				common_ops->panel_off_hs(hisifd);
			}
			if (lcd_ops->power_monitor_off) {
				lcd_ops->power_monitor_off();
			}
			pinfo->lcd_uninit_step = LCD_UNINIT_MIPI_LP_SEND_SEQUENCE;
			LOG_JANK_D(JLID_KERNEL_LCD_POWER_OFF, "%s", "LCD_POWER_OFF");
			break;
		case LCD_UNINIT_MIPI_LP_SEND_SEQUENCE:
			if (common_ops->panel_off_lp) {
				common_ops->panel_off_lp(hisifd);
			}
			pinfo->lcd_uninit_step = LCD_UNINIT_POWER_OFF;
			break;
		case LCD_UNINIT_POWER_OFF:
			if (common_ops->panel_power_off) {
				common_ops->panel_power_off(hisifd);
			}
			break;
		default:
			break;
	}
	LCD_KIT_INFO("fb%d, -!\n", hisifd->index);
	return LCD_KIT_OK;
}

/*
*name:lcd_kit_remove
*function:panel remove
*@pdev:platform device
*/
static int lcd_kit_remove(struct platform_device* pdev)
{
	struct hisi_fb_data_type* hisifd = NULL;

	if (NULL == pdev) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is NULL Point!\n");
		return LCD_KIT_OK;
	}
	lcd_kit_power_finit(pdev);
	return LCD_KIT_OK;
}

static void lcd_kit_second_timerout_function(unsigned long arg)
{
	unsigned long temp;
	temp = arg;
	if(disp_info->bl_is_shield_backlight == true) {
		disp_info->bl_is_shield_backlight = false;
	}
	del_timer(&backlight_second_timer);
	disp_info->bl_is_start_second_timer = false;
	LCD_KIT_INFO("Sheild backlight 1.2s timeout, remove the backlight sheild.\n");
}

static int lcd_kit_set_elvss_dim_fp(struct hisi_fb_data_type* hisifd, struct lcd_kit_adapt_ops* adapt_ops, int disable_elvss_dim)
{
	int ret = LCD_KIT_OK;

	if (hisifd == NULL || adapt_ops == NULL) {
		LCD_KIT_ERR("param is NULL!\n");
		return LCD_KIT_FAIL;
	}
	if(common_info->hbm.hbm_fp_elvss_support) {
		if (common_info->hbm.elvss_prepare_cmds.cmds != NULL){
			if (adapt_ops->mipi_tx) {
				ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.elvss_prepare_cmds);
			}
		}
		if (common_info->hbm.elvss_write_cmds.cmds != NULL){
			if (adapt_ops->mipi_tx) {
				if(disable_elvss_dim) {
					common_info->hbm.elvss_write_cmds.cmds[0].wait = common_info->hbm.hbm_fp_elvss_cmd_delay;
					common_info->hbm.elvss_write_cmds.cmds[0].payload[1] = common_info->hbm.ori_elvss_val & LCD_KIT_DISABLE_ELVSSDIM_MASK;
				} else {
					common_info->hbm.elvss_write_cmds.cmds[0].wait = LCD_KIT_ELVSSDIM_NO_WAIT;
					common_info->hbm.elvss_write_cmds.cmds[0].payload[1] = common_info->hbm.ori_elvss_val | LCD_KIT_ENABLE_ELVSSDIM_MASK;
				}
				ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.elvss_write_cmds);
				LCD_KIT_INFO("[fp set elvss dim] send:0x%x\n", common_info->hbm.elvss_write_cmds.cmds[0].payload[1]);
			}
		}
		if (common_info->hbm.elvss_post_cmds.cmds != NULL){
			if (adapt_ops->mipi_tx) {
				ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.elvss_post_cmds);
			}
		}
	} else {
		LCD_KIT_INFO("Do not support ELVSS Dim, just return\n");
	}

	return ret;
}

static int lcd_kit_enter_hbm_fb(struct hisi_fb_data_type* hisifd, struct lcd_kit_adapt_ops* adapt_ops)
{
	int ret = LCD_KIT_OK;

	if (hisifd == NULL || adapt_ops == NULL) {
		LCD_KIT_ERR("param is NULL!\n");
		return LCD_KIT_FAIL;
	}

	/*enable hbm*/
	if (common_info->hbm.fp_enter_cmds.cmds != NULL){
		if (adapt_ops->mipi_tx) {
			ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.fp_enter_cmds);
		}
	}

	return ret;
}

static int lcd_kit_set_hbm_level_fp(struct hisi_fb_data_type* hisifd, struct lcd_kit_adapt_ops* adapt_ops, unsigned int level)
{
	int ret = LCD_KIT_OK;

	if (hisifd == NULL || adapt_ops == NULL) {
		LCD_KIT_ERR("param is NULL!\n");
		return LCD_KIT_FAIL;
	}

	/*prepare*/
	if (common_info->hbm.hbm_prepare_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx) {
			ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.hbm_prepare_cmds);
		}
	}
	/*set hbm level*/
	if (common_info->hbm.hbm_cmds.cmds != NULL) {
		if (common_info->hbm.hbm_special_bit_ctrl == LCD_KIT_HIGH_12BIT_CTL_HBM_SUPPORT) {
			/*Set high 12bit hbm level, low 4bit set 0*/
			common_info->hbm.hbm_cmds.cmds[0].payload[1] = (level >> LCD_KIT_SHIFT_FOUR_BIT) & 0xff;
			common_info->hbm.hbm_cmds.cmds[0].payload[2] = (level << LCD_KIT_SHIFT_FOUR_BIT) & 0xf0;
		} else {
			common_info->hbm.hbm_cmds.cmds[0].payload[1] = (level >> 8) & 0xf;
			common_info->hbm.hbm_cmds.cmds[0].payload[2] = level & 0xff;
		}
		if (adapt_ops->mipi_tx) {
			ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.hbm_cmds);
		}
	}
	/*post*/
	if (common_info->hbm.hbm_post_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx) {
			ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.hbm_post_cmds);
		}
	}

	return ret;
}

static int lcd_kit_disable_hbm_fp(struct hisi_fb_data_type* hisifd, struct lcd_kit_adapt_ops* adapt_ops)
{
	int ret = LCD_KIT_OK;

	if (hisifd == NULL || adapt_ops == NULL) {
		LCD_KIT_ERR("param is NULL!\n");
		return LCD_KIT_FAIL;
	}

	if (common_info->hbm.exit_dim_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx) {
			ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.exit_dim_cmds);
		}
	}
	if (common_info->hbm.exit_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx) {
			ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.exit_cmds);
		}
	}

	return ret;
}

static ssize_t lcd_kit_hbm_set_func_by_level(struct hisi_fb_data_type* hisifd, uint32_t level ,int type)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops* adapt_ops = NULL;
	struct hisi_panel_info* pinfo = NULL;

	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is NULL!\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (adapt_ops == NULL) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is NULL\n");
		return LCD_KIT_FAIL;
	}

	switch(type) {
		case LCD_KIT_FP_HBM_ENTER:
			if(lcd_kit_set_elvss_dim_fp(hisifd, adapt_ops, LCD_KIT_DISABLE_ELVSSDIM) < 0) {
				LCD_KIT_ERR("lcd_kit_set_elvss_dim_fp: disable failed!\n");
			}
			if(lcd_kit_enter_hbm_fb(hisifd, adapt_ops) < 0) {
				LCD_KIT_ERR("lcd_kit_enter_hbm_fb: enable hbm failed!\n");
			}
			if(lcd_kit_set_hbm_level_fp(hisifd, adapt_ops, level) < 0) {
				LCD_KIT_ERR("lcd_kit_set_hbm_level_fp: set level failed!\n");
			}
			break;
		case LCD_KIT_FP_HBM_EXIT:
			if(level > 0) {
				if(lcd_kit_set_hbm_level_fp(hisifd, adapt_ops, level) < 0) {
					LCD_KIT_ERR("lcd_kit_set_hbm_level_fp: set level failed!\n");
				}
			} else {
				if(lcd_kit_disable_hbm_fp(hisifd, adapt_ops) < 0) {
					LCD_KIT_ERR("lcd_kit_disable_hbm_fp: disable hbm failed!\n");
				}
			}
			if(lcd_kit_set_elvss_dim_fp(hisifd, adapt_ops, LCD_KIT_ENABLE_ELVSSDIM) < 0) {
				LCD_KIT_ERR("lcd_kit_set_elvss_dim_fp: enable failed!\n");
			}
			break;
		default:
			LCD_KIT_ERR("unknown case!\n");
			break;
	}

	return ret;
}

static int lcd_kit_restore_hbm_level(struct hisi_fb_data_type* hisifd)
{
	int ret = LCD_KIT_OK;

	if (NULL == hisifd) {
		LCD_KIT_ERR("hisifd is NULL!\n");
		return ret;
	}

	mutex_lock(&common_info->hbm.hbm_lock);
	ret = lcd_kit_hbm_set_func_by_level(hisifd, common_info->hbm.hbm_level_current, LCD_KIT_FP_HBM_EXIT);
	mutex_unlock(&common_info->hbm.hbm_lock);
	return ret;
}

static void  lcd_kit_fphbm_entry_delay(struct hisi_fb_data_type* hisifd) {
	struct hisi_panel_info* pinfo = NULL;
	ktime_t current_timestamp;
	int diff = -1;

	if (NULL == hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is NULL\n");
		return;
	}
	LCD_KIT_INFO("hbm_entry_delay = %d", pinfo->hbm_entry_delay);
	if (pinfo->hbm_entry_delay <= 0) {
		return;
	}

	current_timestamp = ktime_get();
	diff = ktime_to_ms(current_timestamp)
		- ktime_to_ms(pinfo->hbm_blcode_ts);

	LCD_KIT_INFO("diff = %d ms", diff);

	if (diff >= 0 && diff < (int)(pinfo->hbm_entry_delay / 1000)) {
		LCD_KIT_INFO("delay = %d", pinfo->hbm_entry_delay / 1000 - diff);
		mdelay((unsigned long)(pinfo->hbm_entry_delay / 1000 - diff));
	}
}

static int lcd_kit_set_backlight_by_type(struct platform_device* pdev, int backlight_type)
{
	int ret = 0;
	int max_backlight = 0;
	int min_backlight = 0;
	struct hisi_fb_data_type* hisifd = NULL;
	struct lcd_kit_panel_ops * panel_ops = NULL;

	panel_ops = lcd_kit_panel_get_ops();
	if (panel_ops && panel_ops->lcd_kit_set_backlight_by_type) {
		ret = panel_ops->lcd_kit_set_backlight_by_type(pdev,
			backlight_type, &backlight_second_timer);
		return ret;
	}

	if (NULL == pdev) {
		LCD_KIT_ERR("NULL Pointer\n");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);

	if (NULL == hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return -EINVAL;
	}
	LCD_KIT_INFO("backlight_type is %d\n", backlight_type);

	max_backlight = g_max_backlight_from_app;
	min_backlight = g_min_backlight_from_app;

	switch (backlight_type) {
	case BACKLIGHT_HIGH_LEVEL:
		if(common_info->hbm.hbm_fp_support) {
			mutex_lock(&common_info->hbm.hbm_lock);
			lcd_kit_fphbm_entry_delay(hisifd);
			common_info->hbm.hbm_if_fp_is_using = 1;
			lcd_kit_hbm_set_func_by_level(hisifd, common_info->hbm.hbm_level_max, LCD_KIT_FP_HBM_ENTER);
			mutex_unlock(&common_info->hbm.hbm_lock);
		} else {
			hisifd->panel_info.need_skip_delta = 1;
			(void)lcd_kit_set_backlight(pdev, max_backlight);
			hisifd->panel_info.need_skip_delta = 0;
		}
		disp_info->bl_is_shield_backlight = true;
		if(disp_info->bl_is_start_second_timer == false) {
			init_timer(&backlight_second_timer);
			backlight_second_timer.expires = jiffies + 12*HZ/10;// 1.2s
			backlight_second_timer.data = 0;
			backlight_second_timer.function = lcd_kit_second_timerout_function;
			add_timer(&backlight_second_timer);
			disp_info->bl_is_start_second_timer = true;
		} else {
			//if timer is not timeout, restart timer
			mod_timer(&backlight_second_timer, (jiffies + 12*HZ/10));// 1.2s
		}
		LCD_KIT_INFO("backlight_type is (%d), set_backlight is (max_%d) : (pre_%d)\n", backlight_type, max_backlight,hisifd->bl_level);
		break;
	case BACKLIGHT_LOW_LEVEL:
		if(disp_info->bl_is_start_second_timer == true) {
			del_timer(&backlight_second_timer);
			disp_info->bl_is_start_second_timer = false;
		}
		disp_info->bl_is_shield_backlight = false;

		if(common_info->hbm.hbm_fp_support) {
			lcd_kit_restore_hbm_level(hisifd);
			common_info->hbm.hbm_if_fp_is_using = 0;
		}
		lcd_kit_set_backlight(pdev, hisifd->bl_level);
		LCD_KIT_INFO("backlight_type is (%d), set_backlight is (pre_%d) : (cur_%d)\n", backlight_type, min_backlight,hisifd->bl_level);
		break;
	default:
		LCD_KIT_ERR("backlight_type is not define(%d).\n", backlight_type);
		break;
	}
	return ret;
}

static int lcd_kit_set_hbm_for_mmi(struct platform_device* pdev, int level)
{
	int ret = 0;
	int mmi_level = 0;
	int backlight_type = 0;
	struct hisi_fb_data_type* hisifd = NULL;
	if (NULL == pdev) {
		LCD_KIT_ERR("NULL Pointer\n");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return -EINVAL;
	}
	if (level == 0) {
		backlight_type = BACKLIGHT_LOW_LEVEL;
		mmi_level = hisifd->bl_level;
	} else {
		backlight_type = BACKLIGHT_HIGH_LEVEL;
		mmi_level = level;
	}
	switch (backlight_type) {
		case BACKLIGHT_HIGH_LEVEL:
			LCD_KIT_INFO("set hbm for mmi: BACKLIGHT_HIGH_LEVEL backlight_type=%d  level=%d \n", backlight_type, level);
			hisifd->panel_info.need_skip_delta = 1;
			if (common_info->hbm.hbm_fp_support) {
				lcd_kit_set_backlight(pdev, hisifd->panel_info.bl_max);
				mutex_lock(&common_info->hbm.hbm_lock);
				common_info->hbm.hbm_if_fp_is_using = 1;
				lcd_kit_hbm_set_func_by_level(hisifd, mmi_level, LCD_KIT_FP_HBM_ENTER);
				mutex_unlock(&common_info->hbm.hbm_lock);
			} else {
				lcd_kit_set_backlight(pdev, mmi_level);
			}
			disp_info->bl_is_shield_backlight = true;
#if defined(CONFIG_HISI_FB_V501)
			// To avoid XCC influencing the lightness , call the interface of mask layer matching
			hisifd->mask_layer_xcc_flag = 1;
			clear_xcc_table(hisifd);
#endif
			break;
		case BACKLIGHT_LOW_LEVEL:
			LCD_KIT_INFO("set hbm for mmi: MMI BACKLIGHT_LOW_LEVEL backlight_type=%d  level=%d \n", backlight_type, level);
			disp_info->bl_is_shield_backlight = false;
			if (common_info->hbm.hbm_fp_support) {
				lcd_kit_restore_hbm_level(hisifd);
			}
			lcd_kit_set_backlight(pdev, mmi_level);
#if defined(CONFIG_HISI_FB_V501)
			// restore XCC config(include XCC enable state) while mmi test exit.
			restore_xcc_table(hisifd);
			hisifd->mask_layer_xcc_flag = 0;
#endif
			break;
		default:
			LCD_KIT_ERR("backlight_type is not define(%d).\n", backlight_type);
			break;
	}
	return ret;
}

static int lcd_kit_set_backlight(struct platform_device* pdev, uint32_t bl_level)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type* hisifd = NULL;
	static uint32_t jank_last_bl_level = 0;
	static uint32_t bl_type;
	struct hisi_panel_info* pinfo = NULL;

	if (NULL == pdev) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	if (hisifd->aod_mode && hisifd->aod_function) {
		LCD_KIT_INFO("It is in AOD mode and should bypass lcd_kit_set_backlight! \n");
		return LCD_KIT_OK;
	}
	if(true == disp_info->bl_is_shield_backlight) {
		LCD_KIT_ERR("It is in finger down status, Not running lcd_kit_set_backlight! \n");
		return 0;
	}
	pinfo = &(hisifd->panel_info);
	if (NULL == pinfo) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->quickly_sleep_out.support) {
		if (disp_info->quickly_sleep_out.panel_on_tag) {
			lcd_kit_disp_on_check_delay();
		}
	}
	if (jank_last_bl_level == 0 && bl_level != 0) {
		LOG_JANK_D(JLID_KERNEL_LCD_BACKLIGHT_ON, "LCD_BACKLIGHT_ON,%u", bl_level);
		jank_last_bl_level = bl_level;
	} else if (bl_level == 0 && jank_last_bl_level != 0) {
		LOG_JANK_D(JLID_KERNEL_LCD_BACKLIGHT_OFF, "LCD_BACKLIGHT_OFF");
		jank_last_bl_level = bl_level;
	}
	bl_flicker_detector_collect_upper_bl(bl_level);
	bl_flicker_detector_collect_algo_delta_bl(hisifd->de_info.blc_delta);

	bl_type = lcd_kit_get_bl_set_type(pinfo);
	switch (bl_type) {
		case BL_SET_BY_PWM:
			ret = hisi_pwm_set_backlight(hisifd, bl_level);
			break;
		case BL_SET_BY_BLPWM:
			ret = lcd_kit_blpwm_set_backlight(hisifd, bl_level);
			break;
		case BL_SET_BY_MIPI:
			ret = lcd_kit_mipi_set_backlight(hisifd, bl_level);
			break;
		default:
			LCD_KIT_ERR("not support bl_type\n");
			ret = -1;
			break;
	}
	LCD_KIT_INFO("bl_type = %d, bl_level = %d\n", bl_type, bl_level);
#ifdef LV_GET_LCDBK_ON
	save_light_to_sensorhub(mipi_level, bl_level);
#endif
	return ret;
}

static int lcd_kit_esd_check(struct platform_device* pdev)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type* hisifd = NULL;

	if (NULL == pdev) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&disp_info->mipi_lock);
	if (common_ops->esd_handle) {
		ret = common_ops->esd_handle(hisifd);
	}
	mutex_unlock(&disp_info->mipi_lock);
	return ret;
}

static void fastboot_check_dsm_ocuppy(void)
{
	int value = 0;

#if defined(CONFIG_HUAWEI_DSM)
	#define REC_LIMIT_TIMES -1
	int cur_rec_time = 0;
#endif

	lcd_kit_get_value_from_dts("huawei,lcd_panel_type",
		"fastboot_record_bit", &value);
	if (value & BIT(0)) {
		/*happen short in fastboot, notify dmd*/
#if defined(CONFIG_HUAWEI_DSM)
		(void)lcd_dsm_client_record(lcd_dclient,
			"lp8556 happen short in fastboot\n",
			DSM_LCD_OVP_ERROR_NO,
			REC_LIMIT_TIMES, &cur_rec_time);
#endif
		LCD_KIT_INFO("lp8556 happen short\n");
	}
	if (value & BIT(1)) {
		/*logo checksum fail in fastboot, notify dmd*/
#if defined(CONFIG_HUAWEI_DSM)
		(void)lcd_dsm_client_record(lcd_dclient,
			"checksum fail in fastboott\n",
			DSM_LCD_CHECKSUM_ERROR_NO,
			REC_LIMIT_TIMES, &cur_rec_time);
#endif
		LCD_KIT_INFO("logo checksum fail\n");
	}
	LCD_KIT_INFO("value = 0x%x", value);
}

static int lcd_kit_set_fastboot(struct platform_device* pdev)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type* hisifd = NULL;

	if (NULL == pdev) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	// backlight on
	hisi_lcd_backlight_on(pdev);
	//get blmaxnit
	if(common_info->blmaxnit.get_blmaxnit_type == GET_BLMAXNIT_FROM_DDIC){
		ret = lcd_kit_dsi_cmds_rx((void *)hisifd,
			(uint8_t *)&common_info->blmaxnit.lcd_kit_brightness_ddic_info,
			&common_info->blmaxnit.bl_maxnit_cmds);
		if(ret){
			LCD_KIT_ERR("read blmaxnit_reg error \n");
		}
		LCD_KIT_INFO("lcd_kit_brightness_ddic_info = %d\n", common_info->blmaxnit.lcd_kit_brightness_ddic_info);
	}

	//lcd panel version
	if (disp_info->panel_version.support) {
		if (!lcd_kit_panel_version_init(hisifd)) {
			LCD_KIT_INFO("read panel version successful.\n");
		} else {
			LCD_KIT_INFO("read panel version fail.\n");
		}
	}

	fastboot_check_dsm_ocuppy();

	/*update gamma*/
	lcd_kit_update_gamma_from_tpic(hisifd);
	return LCD_KIT_OK;
}

static int lcd_kit_set_display_region(struct platform_device* pdev, struct dss_rect* dirty)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type* hisifd = NULL;

	if (NULL == pdev) {
		LCD_KIT_ERR("pdev is null\n");
		return LCD_KIT_FAIL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}

	if (common_ops->dirty_region_handle) {
		mutex_lock(&disp_info->mipi_lock);
		ret = common_ops->dirty_region_handle(hisifd, (struct region_rect*)dirty);
		mutex_unlock(&disp_info->mipi_lock);
	}
	return ret;
}

static int lcd_kit_set_tcon_mode(struct platform_device *pdev, uint8_t mode)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (pdev == NULL) {
		LCD_KIT_ERR("pdev is null\n");
		return LCD_KIT_FAIL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}

	switch (mode) {
	case EN_DISPLAY_REGION_A:
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&disp_info->cascade_ic.region_a_cmds);
		break;
	case EN_DISPLAY_REGION_B:
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&disp_info->cascade_ic.region_b_cmds);
		break;
	case EN_DISPLAY_REGION_AB:
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&disp_info->cascade_ic.region_ab_cmds);
		break;
	case EN_DISPLAY_REGION_AB_FOLDED:
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&disp_info->cascade_ic.region_ab_fold_cmds);
		break;
	default:
		LCD_KIT_ERR("mode %d is not support\n", mode);
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_fps_scence_handle(struct platform_device* pdev, uint32_t scence)
{
	int ret = LCD_KIT_OK;

	if (disp_info->fps.support) {
		ret = lcd_kit_updt_fps_scence(pdev, scence);
	}
	return ret;
}

static int lcd_kit_fps_updt_handle(struct platform_device* pdev)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type* hisifd = NULL;

	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->fps.support) {
		if (is_mipi_cmd_panel(hisifd)) {
			ret = lcd_kit_updt_fps(pdev);
		}
	}
	return ret;
}

static ssize_t lcd_kit_ce_mode_store(struct platform_device* pdev, const char* buf, size_t count)
{
	int ret = LCD_KIT_OK;
	unsigned long mode = 0;
	struct hisi_fb_data_type* hisifd = NULL;

	if (pdev == NULL) {
		LCD_KIT_ERR("pdev is null\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	ret = strict_strtoul(buf, 0, &mode);
	if (ret) {
		LCD_KIT_ERR("strict_strtoul error\n");
		return ret;
	}

	hisifd->user_scene_mode = (int)mode;
	if (common_ops->set_ce_mode) {
		ret = common_ops->set_ce_mode(hisifd, mode);
	}
	return count;
}

static ssize_t lcd_kit_rgbw_set_func(struct hisi_fb_data_type* hisifd)
{
	int ret = LCD_KIT_OK;

	if (disp_info->rgbw.support) {
		ret = lcd_kit_rgbw_set_handle(hisifd);
	}
	return ret;
}

static ssize_t lcd_kit_hbm_set_func(struct hisi_fb_data_type* hisifd)
{
	int ret = LCD_KIT_OK;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return LCD_KIT_FAIL;
	}
	if (common_ops->hbm_set_handle) {
		ret = common_ops->hbm_set_handle(hisifd, hisifd->de_info.last_hbm_level, hisifd->de_info.hbm_dimming, hisifd->de_info.hbm_level);
	}
	return ret;
}

static ssize_t lcd_kit_cabc_store(struct platform_device* pdev, const char* buf, size_t count)
{
	ssize_t ret = LCD_KIT_OK;
	struct hisi_fb_data_type* hisifd = NULL;
	unsigned long mode = 0;

	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	ret = strict_strtoul(buf, 0, &mode);
	if (ret) {
		LCD_KIT_ERR("invalid data!\n");
		return ret;
	}
	if (common_ops->set_cabc_mode) {
		mutex_lock(&disp_info->mipi_lock);
		common_ops->set_cabc_mode(hisifd, mode);
		mutex_unlock(&disp_info->mipi_lock);
	}
	return count;
}

static ssize_t lcd_kit_color_param_get_func(struct hisi_fb_data_type* hisifd)
{
	struct hisi_panel_info* pinfo = NULL;
	struct lcd_kit_brightness_color_oeminfo *oeminfo = NULL;

	if (NULL == hisifd) {
		LCD_KIT_ERR("NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}

	pinfo = &(hisifd->panel_info);
	if(NULL == pinfo) {
		LCD_KIT_ERR("pinfo is NULL!\n");
		return LCD_KIT_FAIL;
	}
	oeminfo = lcd_kit_get_brightness_color_oeminfo();
	if (disp_info->oeminfo.support && disp_info->oeminfo.brightness_color_uniform.support) {
		hisifd->de_info.lcd_color_oeminfo.id_flag = oeminfo->id_flag;
		hisifd->de_info.lcd_color_oeminfo.tc_flag = oeminfo->tc_flag;

		hisifd->de_info.lcd_color_oeminfo.panel_id.modulesn = oeminfo->panel_id.modulesn;
		hisifd->de_info.lcd_color_oeminfo.panel_id.equipid = oeminfo->panel_id.equipid;
		hisifd->de_info.lcd_color_oeminfo.panel_id.modulemanufactdate = oeminfo->panel_id.modulemanufactdate;
		hisifd->de_info.lcd_color_oeminfo.panel_id.vendorid = oeminfo->panel_id.vendorid;

		hisifd->de_info.lcd_color_oeminfo.color_params.c_lmt[0] = oeminfo->color_params.c_lmt[0];
		hisifd->de_info.lcd_color_oeminfo.color_params.c_lmt[1] = oeminfo->color_params.c_lmt[1];
		hisifd->de_info.lcd_color_oeminfo.color_params.c_lmt[2] = oeminfo->color_params.c_lmt[2];
		hisifd->de_info.lcd_color_oeminfo.color_params.mxcc_matrix[0][0] = oeminfo->color_params.mxcc_matrix[0][0];
		hisifd->de_info.lcd_color_oeminfo.color_params.mxcc_matrix[0][1] = oeminfo->color_params.mxcc_matrix[0][1];
		hisifd->de_info.lcd_color_oeminfo.color_params.mxcc_matrix[0][2] = oeminfo->color_params.mxcc_matrix[0][2];
		hisifd->de_info.lcd_color_oeminfo.color_params.mxcc_matrix[1][0] = oeminfo->color_params.mxcc_matrix[1][0];
		hisifd->de_info.lcd_color_oeminfo.color_params.mxcc_matrix[1][1] = oeminfo->color_params.mxcc_matrix[1][1];
		hisifd->de_info.lcd_color_oeminfo.color_params.mxcc_matrix[1][2] = oeminfo->color_params.mxcc_matrix[1][2];
		hisifd->de_info.lcd_color_oeminfo.color_params.mxcc_matrix[2][0] = oeminfo->color_params.mxcc_matrix[2][0];
		hisifd->de_info.lcd_color_oeminfo.color_params.mxcc_matrix[2][1] = oeminfo->color_params.mxcc_matrix[2][1];
		hisifd->de_info.lcd_color_oeminfo.color_params.mxcc_matrix[2][2] = oeminfo->color_params.mxcc_matrix[2][2];
		hisifd->de_info.lcd_color_oeminfo.color_params.white_decay_luminace = oeminfo->color_params.white_decay_luminace;

		hisifd->de_info.lcd_color_oeminfo.color_mdata.chroma_coordinates[0][0] = oeminfo->color_mdata.chroma_coordinates[0][0];
		hisifd->de_info.lcd_color_oeminfo.color_mdata.chroma_coordinates[0][1] = oeminfo->color_mdata.chroma_coordinates[0][1];
		hisifd->de_info.lcd_color_oeminfo.color_mdata.chroma_coordinates[1][0] = oeminfo->color_mdata.chroma_coordinates[1][0];
		hisifd->de_info.lcd_color_oeminfo.color_mdata.chroma_coordinates[1][1] = oeminfo->color_mdata.chroma_coordinates[1][1];
		hisifd->de_info.lcd_color_oeminfo.color_mdata.chroma_coordinates[2][0] = oeminfo->color_mdata.chroma_coordinates[2][0];
		hisifd->de_info.lcd_color_oeminfo.color_mdata.chroma_coordinates[2][1] = oeminfo->color_mdata.chroma_coordinates[2][1];
		hisifd->de_info.lcd_color_oeminfo.color_mdata.chroma_coordinates[3][0] = oeminfo->color_mdata.chroma_coordinates[3][0];
		hisifd->de_info.lcd_color_oeminfo.color_mdata.chroma_coordinates[3][1] = oeminfo->color_mdata.chroma_coordinates[3][1];
		hisifd->de_info.lcd_color_oeminfo.color_mdata.white_luminance = oeminfo->color_mdata.white_luminance;
	}
	return LCD_KIT_OK;
}

/*
*name:lcd_kit_get_pt_ulps_support
*function:enter and exit ulps when pt test
*/
static int lcd_kit_get_pt_ulps_support(struct platform_device* pdev)
{
	int support_flag = 0;
	if ((common_info->pt.support) && (common_info->pt.mode == IN_POWER_TEST)) {
		support_flag = 1;
		LCD_KIT_INFO("It is need to send ulps for PT station current test! Support_flag=%d !\n",support_flag);
	}
	else {
		support_flag = 0;
		LCD_KIT_DEBUG("It is need to send ulps for PT station current test! Support_flag=%d !\n",support_flag);
	}
	return support_flag;
}

/*
*name:lcd_kit_probe
*function:panel driver probe
*@pdev:platform device
*/
static int lcd_kit_probe(struct platform_device* pdev)
{
	struct hisi_panel_info* pinfo = NULL;
	struct device_node* np = NULL;
	int ret = LCD_KIT_OK;

	np = pdev->dev.of_node;
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("enter probe!\n");
	pinfo = lcd_kit_data.panel_info;
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}
	memset(pinfo, 0, sizeof(struct hisi_panel_info));
	/*1.adapt init*/
	lcd_kit_adapt_init();
	/*2.common init*/
	if (common_ops->common_init) {
		common_ops->common_init(np);
	}
	/*3.utils init*/
	lcd_kit_utils_init(np, pinfo);
	/*4.init fnode*/
	lcd_kit_sysfs_init();
	/*5.init factory mode*/
	lcd_kit_factory_init(pinfo);
	/*6.power init*/
	lcd_kit_power_init(pdev);
	/*7.init panel ops*/
	lcd_kit_panel_init();
	/*8.init debug*/
#ifdef LCD_KIT_DEBUG_ENABLE
	lcd_kit_dbg_init();
#endif
	/*9.probe driver*/
	if (hisi_fb_device_probe_defer(pinfo->type, pinfo->bl_set_type)) {
		goto err_probe_defer;
	}
	pdev->id = 1;
	ret = platform_device_add_data(pdev, &lcd_kit_data, sizeof(struct hisi_fb_panel_data));
	if (ret) {
		LCD_KIT_ERR("platform_device_add_data failed!\n");
		goto err_device_put;
	}
	hisi_fb_add_device(pdev);
	/*read project id*/
	if (lcd_kit_read_project_id()) {
		LCD_KIT_ERR("read project id error\n");
	}
	LCD_KIT_INFO("exit probe!\n");
	return LCD_KIT_OK;

err_device_put:
	platform_device_put(pdev);
err_probe_defer:
	return -EPROBE_DEFER;
}

/***********************************************************
*platform driver definition
***********************************************************/
/*
*probe match table
*/
static struct of_device_id lcd_kit_match_table[] = {
	{
		.compatible = "auo_otm1901a_5p2_1080p_video",
		.data = NULL,
	},
	{},
};

/*
*panel platform driver
*/
static struct platform_driver lcd_kit_driver = {
	.probe = lcd_kit_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = "lcd_kit_mipi_panel",
		.of_match_table = lcd_kit_match_table,
	},
};

static void transfer_power_config(uint32_t *in, struct lcd_kit_array_data *out)
{
	uint32_t *buf = NULL;
	uint8_t i;

	if ((in == NULL) || (out == NULL)) {
		LCD_KIT_ERR("param invalid!\n");
		return;
	}
	buf = (uint32_t *)kzalloc(LCD_POWER_LEN * sizeof(uint32_t), GFP_KERNEL);
	if (!buf) {
		LCD_KIT_ERR("alloc buf fail\n");
		return;
	}
	for (i = 0; i < LCD_POWER_LEN; i++) {
		buf[i] = in[i];
		LCD_KIT_INFO("POWER : 0x%x\n", buf[i]);
	}
	out->buf = buf;
	out->cnt = LCD_POWER_LEN;
}

static void lcd_get_power_seq_from_dts(struct device_node* np)
{
	uint32_t power[LCD_POWER_LEN] = {0};

	if (np == NULL) {
		LCD_KIT_ERR("invalid input param!\n");
		return;
	}
	if (!of_property_read_u32_array(np, "lcd_vci", power, LCD_POWER_LEN))
		transfer_power_config(power, &power_hdl->lcd_vci);
	if (!of_property_read_u32_array(np, "lcd_iovcc", power, LCD_POWER_LEN))
		transfer_power_config(power, &power_hdl->lcd_iovcc);
	if (!of_property_read_u32_array(np, "lcd_vdd", power, LCD_POWER_LEN))
		transfer_power_config(power, &power_hdl->lcd_vdd);
	if (!of_property_read_u32_array(np, "lcd_vsp", power, LCD_POWER_LEN))
		transfer_power_config(power, &power_hdl->lcd_vsp);
	if (!of_property_read_u32_array(np, "lcd_vsn", power, LCD_POWER_LEN))
		transfer_power_config(power, &power_hdl->lcd_vsn);
	if (!of_property_read_u32_array(np, "lcd_rst", power, LCD_POWER_LEN))
		transfer_power_config(power, &power_hdl->lcd_rst);
	if (!of_property_read_u32_array(np, "lcd_te0", power, LCD_POWER_LEN))
		transfer_power_config(power, &power_hdl->lcd_te0);
	if (!of_property_read_u32_array(np, "lcd_aod", power, LCD_POWER_LEN))
		transfer_power_config(power, &power_hdl->lcd_aod);
}

static int __init lcd_kit_init(void)
{
	int ret = LCD_KIT_OK;
	int len = 0;
	struct device_node* np = NULL;

	if (!lcd_kit_support()) {
		LCD_KIT_INFO("not lcd_kit driver and return\n");
		return ret;
	}
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LCD_KIT_PANEL_TYPE);
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node %s!\n", DTS_COMP_LCD_KIT_PANEL_TYPE);
		ret = -1;
		return ret;
	}
	lcd_get_power_seq_from_dts(np);
	if(of_property_read_u32(np, "board_version", &disp_info->board_version)){
		disp_info->board_version = 0;
	}
	LCD_KIT_INFO("disp_info->board_version = 0x%x\n", disp_info->board_version);
	OF_PROPERTY_READ_U32_RETURN(np, "product_id", &disp_info->product_id);
	LCD_KIT_INFO("disp_info->product_id = %d", disp_info->product_id);
	disp_info->compatible = (char*)of_get_property(np, "lcd_panel_type", NULL);
	if (!disp_info->compatible) {
		LCD_KIT_ERR("can not get lcd kit compatible\n");
		return ret;
	}
	LCD_KIT_DEBUG("disp_info->compatible = %s\n", disp_info->compatible);
	len = strlen(disp_info->compatible);
	memset( (char*)lcd_kit_driver.driver.of_match_table->compatible, 0, LCD_KIT_PANEL_COMP_LENGTH);
	strncpy( (char*)lcd_kit_driver.driver.of_match_table->compatible, disp_info->compatible,
			 len > (LCD_KIT_PANEL_COMP_LENGTH - 1) ? (LCD_KIT_PANEL_COMP_LENGTH - 1) : len);
	/*register driver*/
	ret = platform_driver_register(&lcd_kit_driver);
	if (ret) {
		LCD_KIT_ERR("platform_driver_register failed, error=%d!\n", ret);
	}
	return ret;
}
module_init(lcd_kit_init);
