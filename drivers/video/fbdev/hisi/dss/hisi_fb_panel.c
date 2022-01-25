/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#include "hisi_fb_panel.h"
#include "panel/mipi_lcd_utils.h"

/*lint -e574 -e647 -e568 -e685 -e578*/
DEFINE_SEMAPHORE(hisi_fb_dts_resource_sem);


// none, orise2x, orise3x, himax2x, rsp2x, rsp3x, vesa2x, vesa3x
mipi_ifbc_division_t g_mipi_ifbc_division[MIPI_DPHY_NUM][IFBC_TYPE_MAX] =
{
	//single mipi
	{
		//none
		{XRES_DIV_1, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_0, PXL0_DSI_GT_EN_1},
		//orise2x
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_1, PXL0_DSI_GT_EN_3},
		//orise3x
		{XRES_DIV_3, YRES_DIV_1, IFBC_COMP_MODE_1, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_2, PXL0_DSI_GT_EN_3},
		//himax2x
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_2, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_1, PXL0_DSI_GT_EN_3},
		//rsp2x
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_3, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_OPEN, PXL0_DIVCFG_1, PXL0_DSI_GT_EN_3},
		//rsp3x  [NOTE]reality: xres_div = 1.5, yres_div = 2, amended in "mipi_ifbc_get_rect" function
		{XRES_DIV_3, YRES_DIV_1, IFBC_COMP_MODE_4, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_OPEN, PXL0_DIVCFG_2, PXL0_DSI_GT_EN_3},
		//vesa2x_1pipe
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_5, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_1, PXL0_DSI_GT_EN_3},
		//vesa3x_1pipe
		{XRES_DIV_3, YRES_DIV_1, IFBC_COMP_MODE_5, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_2, PXL0_DSI_GT_EN_3},
		//vesa2x_2pipe
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_6, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_1, PXL0_DSI_GT_EN_3},
		//vesa3x_2pipe
		{XRES_DIV_3, YRES_DIV_1, IFBC_COMP_MODE_6, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_2, PXL0_DSI_GT_EN_3},
		//vesa3.75x_2pipe
		{XRES_DIV_3, YRES_DIV_1, IFBC_COMP_MODE_6, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_2, PXL0_DSI_GT_EN_3}
	},

	//dual mipi
	{
		//none
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_1, PXL0_DSI_GT_EN_3},
		//orise2x
		{XRES_DIV_4, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_3, PXL0_DSI_GT_EN_3},
		//orise3x
		{XRES_DIV_6, YRES_DIV_1, IFBC_COMP_MODE_1, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_5, PXL0_DSI_GT_EN_3},
		//himax2x
		{XRES_DIV_4, YRES_DIV_1, IFBC_COMP_MODE_2, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_3, PXL0_DSI_GT_EN_3},
		//rsp2x
		{XRES_DIV_4, YRES_DIV_1, IFBC_COMP_MODE_3, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_OPEN, PXL0_DIVCFG_3, PXL0_DSI_GT_EN_3},
		//rsp3x
		{XRES_DIV_3, YRES_DIV_2, IFBC_COMP_MODE_4, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_OPEN, PXL0_DIVCFG_5, PXL0_DSI_GT_EN_3},
		//vesa2x_1pipe
		{XRES_DIV_4, YRES_DIV_1, IFBC_COMP_MODE_5, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_3, PXL0_DSI_GT_EN_3},
		//vesa3x_1pipe
		{XRES_DIV_6, YRES_DIV_1, IFBC_COMP_MODE_5, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_5, PXL0_DSI_GT_EN_3},
		//vesa2x_2pipe
		{XRES_DIV_4, YRES_DIV_1, IFBC_COMP_MODE_6, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_3, PXL0_DSI_GT_EN_3},
		//vesa3x_2pipe
		{XRES_DIV_6, YRES_DIV_1, IFBC_COMP_MODE_6, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_5, 3},
		//vesa3.75x_2pipe
		{XRES_DIV_6, YRES_DIV_1, IFBC_COMP_MODE_6, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_5, 3}
	}
};

int hisi_lcd_backlight_on(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	int ret = 0;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +!\n", hisifd->index);

	if (hisifd->panel_info.bl_set_type & BL_SET_BY_PWM) {
		ret = hisi_pwm_on(pdev);
	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_BLPWM) {
		ret = hisi_blpwm_on(pdev);
	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_SH_BLPWM) {
		ret = hisi_sh_blpwm_on(pdev);
	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_MIPI) {
		;
	} else {
		HISI_FB_ERR("No such bl_set_type(%d)!\n", hisifd->panel_info.bl_set_type);
	}

	HISI_FB_DEBUG("fb%d, -!\n", hisifd->index);

	return ret;
}

int hisi_lcd_backlight_off(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	int ret = 0;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +!\n", hisifd->index);

	if (hisifd->panel_info.bl_set_type & BL_SET_BY_PWM) {
		ret = hisi_pwm_off(pdev);
	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_BLPWM) {
		ret = hisi_blpwm_off(pdev);
	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_SH_BLPWM) {
		ret = hisi_sh_blpwm_off(pdev);
	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_MIPI) {
		;
	} else {
		HISI_FB_ERR("No such bl_set_type(%d)!\n", hisifd->panel_info.bl_set_type);
	}

	HISI_FB_DEBUG("fb%d, -!\n", hisifd->index);

	return ret;
}

int gpio_cmds_tx(struct gpio_desc *cmds, int cnt)
{
	int ret = 0;
	struct gpio_desc *cm = NULL;
	int i = 0;

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if ((cm == NULL) || (cm->label == NULL)) {
			HISI_FB_ERR("cm or cm->label is null! index=%d\n", i);
			ret = -1;
			goto error;
		}

		if (!gpio_is_valid(*(cm->gpio))) {
			HISI_FB_ERR("gpio invalid, dtype=%d, lable=%s, gpio=%d!\n",
				cm->dtype, cm->label, *(cm->gpio));
			ret = -1;
			goto error;
		}

		if (cm->dtype == DTYPE_GPIO_INPUT) {
			if (gpio_direction_input(*(cm->gpio)) != 0) {
				HISI_FB_ERR("failed to gpio_direction_input, lable=%s, gpio=%d!\n",
					cm->label, *(cm->gpio));
				ret = -1;
				goto error;
			}
		} else if (cm->dtype == DTYPE_GPIO_OUTPUT) {
			if (gpio_direction_output(*(cm->gpio), cm->value) != 0) {
				HISI_FB_ERR("failed to gpio_direction_output, label%s, gpio=%d!\n",
					cm->label, *(cm->gpio));
				ret = -1;
				goto error;
			}
		} else if (cm->dtype == DTYPE_GPIO_REQUEST) {
			if (gpio_request(*(cm->gpio), cm->label) != 0) {
				HISI_FB_ERR("failed to gpio_request, lable=%s, gpio=%d!\n",
					cm->label, *(cm->gpio));
				ret = -1;
				goto error;
			}
		} else if (cm->dtype == DTYPE_GPIO_FREE) {
			gpio_free(*(cm->gpio));
		} else {
			HISI_FB_ERR("dtype=%x NOT supported\n", cm->dtype);
			ret = -1;
			goto error;
		}

		if (cm->wait) {
			if (cm->waittype == WAIT_TYPE_US)
				udelay(cm->wait);
			else if (cm->waittype == WAIT_TYPE_MS)
				mdelay(cm->wait);
			else
				mdelay(cm->wait * 1000);
		}

		cm++;
	}

	return 0;

error:
	return ret;
}

int resource_cmds_tx(struct platform_device *pdev,
	struct resource_desc *cmds, int cnt)
{
	int ret = 0;
	struct resource *res = NULL;
	struct resource_desc *cm = NULL;
	int i = 0;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if ((cm == NULL) || (cm->name == NULL)) {
			HISI_FB_ERR("cm or cm->name is null! index=%d\n", i);
			ret = -1;
			goto error;
		}

		res = platform_get_resource_byname(pdev, cm->flag, cm->name);
		if (!res) {
			HISI_FB_ERR("failed to get %s resource!\n", cm->name);
			ret = -1;
			goto error;
		}

		*(cm->value) = res->start;

		cm++;
	}

error:
	return ret;
}

int spi_cmds_tx(struct spi_device *spi, struct spi_cmd_desc *cmds, int cnt)
{
	return 0;
}

int vcc_cmds_tx(struct platform_device *pdev, struct vcc_desc *cmds, int cnt)
{
	int ret = 0;
	struct vcc_desc *cm = NULL;
	int i = 0;

	if (g_fpga_flag == 1) {
		return 0;
	}

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if ((cm == NULL) || (cm->id == NULL)) {
			HISI_FB_ERR("cm or cm->id is null! index=%d\n", i);
			ret = -1;
			goto error;
		}

		if (cm->dtype == DTYPE_VCC_GET) {
			if (NULL == pdev) {
				HISI_FB_ERR("pdev is NULL");
				return -EINVAL;
			}

			*(cm->regulator) = devm_regulator_get(&pdev->dev, cm->id);
			if (IS_ERR(*(cm->regulator))) {
				HISI_FB_ERR("failed to get %s regulator!\n", cm->id);
				ret = -1;
				goto error;
			}
		} else if (cm->dtype == DTYPE_VCC_PUT) {
			if (!IS_ERR(*(cm->regulator))) {
				devm_regulator_put(*(cm->regulator));
			}
		} else if (cm->dtype == DTYPE_VCC_ENABLE) {
			if (!IS_ERR(*(cm->regulator))) {
				if (regulator_enable(*(cm->regulator)) != 0) {
					HISI_FB_ERR("failed to enable %s regulator!\n", cm->id);
					ret = -1;
					goto error;
				}
			}
		} else if (cm->dtype == DTYPE_VCC_DISABLE) {
			if (!IS_ERR(*(cm->regulator))) {
				if (regulator_disable(*(cm->regulator)) != 0) {
					HISI_FB_ERR("failed to disable %s regulator!\n", cm->id);
					ret = -1;
					goto error;
				}
			}
		} else if (cm->dtype == DTYPE_VCC_SET_VOLTAGE) {
			if (!IS_ERR(*(cm->regulator))) {
				if (regulator_set_voltage(*(cm->regulator), cm->min_uV, cm->max_uV) != 0) {
					HISI_FB_ERR("failed to set %s regulator voltage!\n", cm->id);
					ret = -1;
					goto error;
				}
			}
		} else {
			HISI_FB_ERR("dtype=%x NOT supported\n", cm->dtype);
			ret = -1;
			goto error;
		}

		if (cm->wait) {
			if (cm->waittype == WAIT_TYPE_US)
				udelay(cm->wait);
			else if (cm->waittype == WAIT_TYPE_MS)
				mdelay(cm->wait);
			else
				mdelay(cm->wait * 1000);
		}

		cm++;
	}

	return 0;

error:
	return ret;
}

int pinctrl_cmds_tx(struct platform_device *pdev, struct pinctrl_cmd_desc *cmds, int cnt)
{
	int ret = 0;

	int i = 0;
	struct pinctrl_cmd_desc *cm = NULL;

	if (g_fpga_flag == 1) {
		return 0;
	}

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if (cm == NULL) {
			HISI_FB_ERR("cm is null! index=%d\n", i);
			continue;
		}

		if (cm->dtype == DTYPE_PINCTRL_GET) {
			if (NULL == pdev) {
				HISI_FB_ERR("pdev is NULL");
				return -EINVAL;
			}
			cm->pctrl_data->p = devm_pinctrl_get(&pdev->dev);
			if (IS_ERR(cm->pctrl_data->p)) {
				ret = -1;
				HISI_FB_ERR("failed to get p, index=%d!\n", i);
				goto err;
			}
		} else if (cm->dtype == DTYPE_PINCTRL_STATE_GET) {
			if (cm ->mode == DTYPE_PINCTRL_STATE_DEFAULT) {
				cm->pctrl_data->pinctrl_def = pinctrl_lookup_state(cm->pctrl_data->p, PINCTRL_STATE_DEFAULT);
				if (IS_ERR(cm->pctrl_data->pinctrl_def)) {
					ret = -1;
					HISI_FB_ERR("failed to get pinctrl_def, index=%d!\n", i);
					goto err;
				}
			} else if (cm ->mode == DTYPE_PINCTRL_STATE_IDLE) {
				cm->pctrl_data->pinctrl_idle = pinctrl_lookup_state(cm->pctrl_data->p, PINCTRL_STATE_IDLE);
				if (IS_ERR(cm->pctrl_data->pinctrl_idle)) {
					ret = -1;
					HISI_FB_ERR("failed to get pinctrl_idle, index=%d!\n", i);
					goto err;
				}
			} else {
				ret = -1;
				HISI_FB_ERR("unknown pinctrl type to get!\n");
				goto err;
			}
		} else if (cm->dtype == DTYPE_PINCTRL_SET) {
			if (cm ->mode == DTYPE_PINCTRL_STATE_DEFAULT) {
				if (cm->pctrl_data->p && cm->pctrl_data->pinctrl_def) {
					ret = pinctrl_select_state(cm->pctrl_data->p, cm->pctrl_data->pinctrl_def);
					if (ret) {
						HISI_FB_ERR("could not set this pin to default state!\n");
						ret = -1;
						goto err;
					}
				}
			} else if (cm ->mode == DTYPE_PINCTRL_STATE_IDLE) {
				if (cm->pctrl_data->p && cm->pctrl_data->pinctrl_idle) {
					ret = pinctrl_select_state(cm->pctrl_data->p, cm->pctrl_data->pinctrl_idle);
					if (ret) {
						HISI_FB_ERR("could not set this pin to idle state!\n");
						ret = -1;
						goto err;
					}
				}
			} else {
				ret = -1;
				HISI_FB_ERR("unknown pinctrl type to set!\n");
				goto err;
			}
		} else if (cm->dtype == DTYPE_PINCTRL_PUT) {
			if (cm->pctrl_data->p)
				pinctrl_put(cm->pctrl_data->p);
		} else {
			HISI_FB_ERR("not supported command type!\n");
			ret = -1;
			goto err;
		}

		cm++;
	}

	return 0;

err:
	return ret;
}

int panel_next_set_fastboot(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}

	pdata = (struct hisi_fb_panel_data *)pdev->dev.platform_data;
	if (pdata) {
		next_pdev = pdata->next;
		if (next_pdev) {
			next_pdata = (struct hisi_fb_panel_data *)next_pdev->dev.platform_data;
			if ((next_pdata) && (next_pdata->set_fastboot))
				ret = next_pdata->set_fastboot(next_pdev);
		}
	}

	return ret;
}

int panel_next_on(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->on))
			ret = next_pdata->on(next_pdev);
	}

	return ret;
}

int panel_next_off(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->off))
			ret = next_pdata->off(next_pdev);
	}

	return ret;
}

int panel_next_lp_ctrl(struct platform_device *pdev, bool lp_enter)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lp_ctrl))
			ret = next_pdata->lp_ctrl(next_pdev, lp_enter);
	}

	return ret;
}

int panel_next_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->remove))
			ret = next_pdata->remove(next_pdev);
	}

	return ret;
}

int panel_next_set_backlight(struct platform_device *pdev, uint32_t bl_level)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->set_backlight))
			ret = next_pdata->set_backlight(next_pdev, bl_level);
	}

	return ret;
}

int panel_next_sbl_ctrl(struct platform_device *pdev, int enable)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->sbl_ctrl))
			ret = next_pdata->sbl_ctrl(next_pdev, enable);
	}

	return ret;
}

int panel_next_vsync_ctrl(struct platform_device *pdev, int enable)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->vsync_ctrl))
			ret = next_pdata->vsync_ctrl(next_pdev, enable);
	}

	return ret;
}

int panel_next_lcd_fps_scence_handle(struct platform_device *pdev, uint32_t scence)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_fps_scence_handle))
			ret = next_pdata->lcd_fps_scence_handle(next_pdev, scence);
	}

	return ret;
}

int panel_next_lcd_fps_updt_handle(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_fps_updt_handle))
			ret = next_pdata->lcd_fps_updt_handle(next_pdev);
	}

	return ret;
}

int panel_next_esd_handle(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->esd_handle))
			ret = next_pdata->esd_handle(next_pdev);
	}

	return ret;
}

int panel_next_set_display_region(struct platform_device *pdev, struct dss_rect *dirty)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if((pdev == NULL) || (dirty == NULL)){
		HISI_FB_ERR("pedv or dirty is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->set_display_region))
			ret = next_pdata->set_display_region(next_pdev, dirty);
	}

	return ret;
}

int panel_next_get_lcd_id(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->get_lcd_id))
			ret = next_pdata->get_lcd_id(next_pdev);
	}
	return ret;
}

int panel_next_bypass_powerdown_ulps_support(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return 0;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->panel_bypass_powerdown_ulps_support))
			ret = next_pdata->panel_bypass_powerdown_ulps_support(next_pdev);
	}
	return ret;
}

ssize_t panel_next_snd_mipi_clk_cmd_store(struct platform_device *pdev, uint32_t clk_val)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->snd_mipi_clk_cmd_store))
			ret = next_pdata->snd_mipi_clk_cmd_store(next_pdev, clk_val);
	}

	return ret;
}


ssize_t panel_next_lcd_model_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_model_show))
			ret = next_pdata->lcd_model_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_cabc_mode_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_cabc_mode_show))
			ret = next_pdata->lcd_cabc_mode_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_cabc_mode_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_cabc_mode_store))
			ret = next_pdata->lcd_cabc_mode_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_check_reg(struct platform_device *pdev, char *buf)
{
	ssize_t ret = -1;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_check_reg))
			ret = next_pdata->lcd_check_reg(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_mipi_detect(struct platform_device *pdev, char *buf)
{
	ssize_t ret = -1;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_mipi_detect))
			ret = next_pdata->lcd_mipi_detect(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_mipi_dsi_bit_clk_upt_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->mipi_dsi_bit_clk_upt_store))
			ret = next_pdata->mipi_dsi_bit_clk_upt_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_support_checkmode_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_support_checkmode_show))
			ret = next_pdata->lcd_support_checkmode_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_mipi_dsi_bit_clk_upt_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->mipi_dsi_bit_clk_upt_show))
			ret = next_pdata->mipi_dsi_bit_clk_upt_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_hkadc_debug_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_hkadc_debug_show))
			ret = next_pdata->lcd_hkadc_debug_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_hkadc_debug_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_hkadc_debug_store))
			ret = next_pdata->lcd_hkadc_debug_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_gram_check_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_gram_check_show))
			ret = next_pdata->lcd_gram_check_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_gram_check_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_gram_check_store))
			ret = next_pdata->lcd_gram_check_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_dynamic_sram_checksum_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_dynamic_sram_checksum_show))
			ret = next_pdata->lcd_dynamic_sram_checksum_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_dynamic_sram_checksum_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_dynamic_sram_checksum_store))
			ret = next_pdata->lcd_dynamic_sram_checksum_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_ic_color_enhancement_mode_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_ic_color_enhancement_mode_show))
			ret = next_pdata->lcd_ic_color_enhancement_mode_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_ic_color_enhancement_mode_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_ic_color_enhancement_mode_store))
			ret = next_pdata->lcd_ic_color_enhancement_mode_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_voltage_enable_store(struct platform_device *pdev, const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_voltage_enable_store))
			ret = next_pdata->lcd_voltage_enable_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_bist_check(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_bist_check))
			ret = next_pdata->lcd_bist_check(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_sleep_ctrl_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_sleep_ctrl_show))
			ret = next_pdata->lcd_sleep_ctrl_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_sleep_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_sleep_ctrl_store))
			ret = next_pdata->lcd_sleep_ctrl_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_test_config_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_test_config_show))
			ret = next_pdata->lcd_test_config_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_test_config_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_test_config_store))
			ret = next_pdata->lcd_test_config_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_reg_read_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_reg_read_show))
			ret = next_pdata->lcd_reg_read_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_reg_read_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_reg_read_store))
			ret = next_pdata->lcd_reg_read_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_support_mode_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_support_mode_show))
			ret = next_pdata->lcd_support_mode_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_support_mode_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_support_mode_store))
			ret = next_pdata->lcd_support_mode_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_lp2hs_mipi_check_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_lp2hs_mipi_check_show))
			ret = next_pdata->lcd_lp2hs_mipi_check_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_lp2hs_mipi_check_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_lp2hs_mipi_check_store))
			ret = next_pdata->lcd_lp2hs_mipi_check_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_amoled_pcd_errflag_check(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
	next_pdata = dev_get_platdata(&next_pdev->dev);
	 if ((next_pdata) && (next_pdata->amoled_pcd_errflag_check))
		 ret = next_pdata->amoled_pcd_errflag_check(next_pdev, buf);
	}

	return ret;
}
ssize_t panel_next_lcd_hbm_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_hbm_ctrl_store))
			ret = next_pdata->lcd_hbm_ctrl_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_hbm_ctrl_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_hbm_ctrl_show))
			ret = next_pdata->lcd_hbm_ctrl_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_amoled_vr_mode_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	if (NULL == buf) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_amoled_vr_mode_store))
			ret = next_pdata->lcd_amoled_vr_mode_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_amoled_vr_mode_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	if (NULL == buf) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_amoled_vr_mode_show))
			ret = next_pdata->lcd_amoled_vr_mode_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_acl_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	if (NULL == buf) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_acl_ctrl_store))
			ret = next_pdata->lcd_acl_ctrl_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_alpm_setting_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	if (NULL == buf) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->amoled_alpm_setting_store))
			ret = next_pdata->amoled_alpm_setting_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_acl_ctrl_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	if (NULL == buf) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_acl_ctrl_show))
			ret = next_pdata->lcd_acl_ctrl_show(next_pdev, buf);
	}

	return ret;
}


ssize_t panel_next_sharpness2d_table_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->sharpness2d_table_store))
			ret = next_pdata->sharpness2d_table_store(next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_sharpness2d_table_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->sharpness2d_table_show))
			ret = next_pdata->sharpness2d_table_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_panel_info_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->panel_info_show))
			ret = next_pdata->panel_info_show(next_pdev, buf);
	}

	return ret;
}

bool is_ldi_panel(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return false;
	}
	if (hisifd->panel_info.type & PANEL_LCDC)
		return true;

	return false;
}

bool is_mipi_cmd_panel(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return false;
	}

	if (hisifd->panel_info.type & (PANEL_MIPI_CMD | PANEL_DUAL_MIPI_CMD))
		return true;

	return false;
}

bool is_mipi_cmd_panel_ext(struct hisi_panel_info *pinfo)
{
	if (NULL == pinfo) {
		HISI_FB_ERR("pinfo is NULL");
		return false;
	}

	if (pinfo->type & (PANEL_MIPI_CMD | PANEL_DUAL_MIPI_CMD))
		return true;

	return false;
}

bool is_mipi_video_panel(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return false;
	}

	if (hisifd->panel_info.type & (PANEL_MIPI_VIDEO | PANEL_DUAL_MIPI_VIDEO | PANEL_RGB2MIPI))
		return true;

	return false;
}

bool is_dp_panel(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return false;
	}

	if (hisifd->panel_info.type & PANEL_DP)
		return true;

	return false;
}

bool is_mipi_panel(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return false;
	}

	if (hisifd->panel_info.type & (PANEL_MIPI_VIDEO | PANEL_MIPI_CMD |
		PANEL_DUAL_MIPI_VIDEO | PANEL_DUAL_MIPI_CMD))
		return true;

	return false;
}

bool is_hisync_mode(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return false;
	}

	if (is_mipi_video_panel(hisifd) && hisifd->panel_info.hisync_mode) {
		return true;
	}

	return false;
}

bool is_video_idle_ctrl_mode(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return false;
	}

	if (is_mipi_video_panel(hisifd) && hisifd->panel_info.video_idle_mode) {
		return true;
	}

	return false;
}

bool is_dual_mipi_panel(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return false;
	}

	if (hisifd->panel_info.type & (PANEL_DUAL_MIPI_VIDEO | PANEL_DUAL_MIPI_CMD))
		return true;

	return false;
}

bool is_dual_mipi_panel_ext(struct hisi_panel_info *pinfo)
{
	if (NULL == pinfo) {
		HISI_FB_ERR("pinfo is NULL");
		return false;
	}

	if (pinfo->type & (PANEL_DUAL_MIPI_VIDEO | PANEL_DUAL_MIPI_CMD))
		return true;

	return false;
}

bool is_hisi_writeback_panel(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return false;
	}

	if (hisifd->panel_info.type & PANEL_WRITEBACK)
		return true;

	return false;
}

bool is_ifbc_panel(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return false;
	}

	if (hisifd->panel_info.ifbc_type != IFBC_TYPE_NONE)
		return true;

	return false;
}

bool is_ifbc_vesa_panel(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return false;
	}

	if ((hisifd->panel_info.ifbc_type == IFBC_TYPE_VESA2X_SINGLE) ||
		(hisifd->panel_info.ifbc_type == IFBC_TYPE_VESA3X_SINGLE) ||
		(hisifd->panel_info.ifbc_type == IFBC_TYPE_VESA2X_DUAL) ||
		(hisifd->panel_info.ifbc_type == IFBC_TYPE_VESA3X_DUAL) ||
		(hisifd->panel_info.ifbc_type == IFBC_TYPE_VESA3_75X_DUAL))
		return true;

	return false;
}

ssize_t panel_mode_switch_store(struct hisi_fb_data_type *hisifd,
	const char *buf, size_t count)
{
	struct hisi_panel_info *pinfo = NULL;
	uint8_t mode_switch_to_tmp = MODE_8BIT;
	int str_len = 0;
	int i = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	for (i = 0; buf[i] != '\0' && buf[i] != '\n'; i++) {
		str_len++;
		if (str_len >= 9) {
			HISI_FB_ERR("invalid input parameter: n_str = %d, count = %ld\n", str_len, count);
			break;
		}
	}

	if (str_len != 8) {
		HISI_FB_ERR("invalid input parameter: n_str = %d, count = %ld\n", str_len, count);
		return count;
	}

	if (!hisifd->panel_info.panel_mode_swtich_support) {
		HISI_FB_INFO("fb%d, not support!\n", hisifd->index);
		return count;
	}

	if (pinfo->current_mode != pinfo->mode_switch_to) {
		HISI_FB_ERR("last switch action is not over.\n");
		return count;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (g_debug_panel_mode_switch == 1) {
		HISI_FB_ERR("panel_mode_switch is closed.\n");
		return count;
	}

	if (!strncmp(buf, PANEL_10BIT_VIDEO_MODE_STR, str_len)) {
		mode_switch_to_tmp = MODE_10BIT_VIDEO_3X;
	} else if (!strncmp(buf, PANEL_8BIT_CMD_MODE_STR, str_len)) {
		mode_switch_to_tmp = MODE_8BIT;
	} else {
		HISI_FB_ERR("fb%d, unknown panel mode!\n", hisifd->index);
		return count;
	}

	if (mode_switch_to_tmp != pinfo->mode_switch_to) {
		pinfo->mode_switch_to = mode_switch_to_tmp;
	} else {
		HISI_FB_ERR("current mode or mode_switch_to is already %d !\n", mode_switch_to_tmp);
		return count;
	}

	HISI_FB_INFO("switch panel mode to %s.\n", buf);
	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return count;
}

static uint32_t mode_switch_wait_vfp(struct hisi_fb_data_type *hisifd, uint8_t mode_switch_to)
{
	uint32_t wait_time = 0;
	struct hisi_panel_info *pinfo = NULL;

	pinfo = &(hisifd->panel_info);

	if (mode_switch_to == MODE_8BIT) {
		wait_time = (pinfo->ldi.h_back_porch + pinfo->ldi.h_front_porch + pinfo->ldi.h_pulse_width) / 3;
		wait_time += pinfo->xres *30 /24 / 3;
		wait_time *= pinfo->ldi.v_front_porch;
		wait_time = wait_time / (uint32_t)(pinfo->pxl_clk_rate /1000000 /3);
	}

	HISI_FB_DEBUG("wait_time:%d us.\n", wait_time);
	return wait_time;
}

void panel_mode_switch_isr_handler(struct hisi_fb_data_type *hisifd, uint8_t mode_switch_to)
{
	struct hisi_panel_info *pinfo = NULL;
	int ret = -1;
	uint32_t wait_time_us = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL .\n");
		return;
	}
	pinfo = &(hisifd->panel_info);

	HISI_FB_DEBUG("+ .\n");
	HISI_FB_DEBUG(" LDI_CTRL = 0x%x .\n", inp32(hisifd->dss_base + DSS_LDI0_OFFSET + LDI_CTRL));

	wait_time_us = mode_switch_wait_vfp(hisifd, mode_switch_to);
	udelay(wait_time_us);

	ret = switch_panel_mode(hisifd, mode_switch_to);

	single_frame_update(hisifd);

	if (ret == 0) {
		HISI_FB_INFO("panel mode successfully switched from %d to %d.\n", pinfo->current_mode, mode_switch_to);
		pinfo->current_mode = pinfo->mode_switch_to;
	}

	HISI_FB_DEBUG("- .\n");
	return;
}

bool mipi_panel_check_reg (struct hisi_fb_data_type *hisifd,
	uint32_t *read_value)
{
	int ret = 0;
	char lcd_reg_05[] = {0x05};
	char lcd_reg_0a[] = {0x0a};
	char lcd_reg_0e[] = {0x0e};
	char lcd_reg_0f[] = {0x0f};

	struct dsi_cmd_desc lcd_check_reg[] = {
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_05), lcd_reg_05},
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_0a), lcd_reg_0a},
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_0e), lcd_reg_0e},
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_0f), lcd_reg_0f},
	};

	ret = mipi_dsi_cmds_rx(read_value, lcd_check_reg, \
		ARRAY_SIZE(lcd_check_reg), hisifd->mipi_dsi0_base);
	if (ret) {
		HISI_FB_ERR("Read error number: %d\n", ret);
		return false;
	}

	return true;
}

/*lint -save -e573*/
int mipi_ifbc_get_rect(struct hisi_fb_data_type *hisifd, struct dss_rect *rect)
{
	uint32_t ifbc_type;
	uint32_t mipi_idx;
	uint32_t xres_div;
	uint32_t yres_div;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	if (NULL == rect) {
		HISI_FB_ERR("rect is NULL");
		return -EINVAL;
	}

	ifbc_type = hisifd->panel_info.ifbc_type;
	if ((ifbc_type < IFBC_TYPE_NONE) || (ifbc_type >= IFBC_TYPE_MAX)) {
		HISI_FB_ERR("ifbc_type is invalid");
		return -EINVAL;
	}

	mipi_idx = is_dual_mipi_panel(hisifd) ? 1 : 0;

	xres_div = g_mipi_ifbc_division[mipi_idx][ifbc_type].xres_div;
	yres_div = g_mipi_ifbc_division[mipi_idx][ifbc_type].yres_div;

	if ((rect->w % xres_div) > 0) {
		HISI_FB_ERR("fb%d, xres(%d) is not division_h(%d) pixel aligned!\n", hisifd->index, rect->w, xres_div);
	}

	if ((rect->h % yres_div) > 0) {
		HISI_FB_ERR("fb%d, yres(%d) is not division_v(%d) pixel aligned!\n", hisifd->index, rect->h, yres_div);
	}

	//[NOTE] rsp3x && single_mipi CMD mode amended xres_div = 1.5, yres_div = 2 ,VIDEO mode amended xres_div = 3, yres_div = 1
	if ((mipi_idx == 0) && (ifbc_type == IFBC_TYPE_RSP3X) && (hisifd->panel_info.type == PANEL_MIPI_CMD)) {
		rect->w *= 2;
		rect->h /= 2;
	}

	if ((hisifd->panel_info.mode_switch_to == MODE_10BIT_VIDEO_3X)
		&& (hisifd->panel_info.ifbc_type == IFBC_TYPE_VESA3X_DUAL)) {
		rect->w = rect->w * 30 / 24 / xres_div;
	} else {
		rect->w /= xres_div;
	}

	rect->h /= yres_div;

	return 0;
}
/*lint -restore*/

void hisifb_snd_cmd_before_frame(struct hisi_fb_data_type *hisifd)
{
	struct hisi_fb_panel_data *pdata = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null\n");
		return ;
	}
	if (!hisifd->panel_info.snd_cmd_before_frame_support) {
		return ;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (pdata == NULL) {
		HISI_FB_ERR("pdata is null\n");
		return ;
	}

	if (pdata->snd_cmd_before_frame) {
		pdata->snd_cmd_before_frame(hisifd->pdev);
	}
	return;
}

bool hisi_fb_device_probe_defer(uint32_t panel_type, uint32_t bl_type)
{
	bool flag = true;

	down(&hisi_fb_dts_resource_sem);

	switch (panel_type) {
	case PANEL_NO:
		if(g_dts_resouce_ready & DTS_FB_RESOURCE_INIT_READY) {
			flag = false;
		}
		break;
	case PANEL_LCDC:
	case PANEL_MIPI2RGB:
	case PANEL_RGB2MIPI:
		if ((g_dts_resouce_ready & DTS_FB_RESOURCE_INIT_READY) &&
			(g_dts_resouce_ready & DTS_SPI_READY)) {
			if (bl_type & (BL_SET_BY_PWM | BL_SET_BY_BLPWM)) {
				if (g_dts_resouce_ready & DTS_PWM_READY)
					flag = false;
			} else {
				flag = false;
			}
		}
		break;
	case PANEL_MIPI_VIDEO:
	case PANEL_MIPI_CMD:
	case PANEL_DUAL_MIPI_VIDEO:
	case PANEL_DUAL_MIPI_CMD:
		if (g_dts_resouce_ready & DTS_FB_RESOURCE_INIT_READY) {
			if (bl_type & (BL_SET_BY_PWM | BL_SET_BY_BLPWM)) {
				if (g_dts_resouce_ready & DTS_PWM_READY)
					flag = false;
			} else {
				flag = false;
			}
		}
		break;
	case PANEL_HDMI:
	case PANEL_DP:
		if (g_dts_resouce_ready & DTS_PANEL_PRIMARY_READY)
			flag = false;
		break;
	case PANEL_OFFLINECOMPOSER:
		if (g_dts_resouce_ready & DTS_PANEL_EXTERNAL_READY)
			flag = false;
		break;
	case PANEL_WRITEBACK:
		if (g_dts_resouce_ready & DTS_PANEL_OFFLINECOMPOSER_READY)
			flag = false;
		break;
	case PANEL_MEDIACOMMON:
		if (g_dts_resouce_ready & DTS_PANEL_OFFLINECOMPOSER_READY)
			flag = false;
		break;
	default:
		HISI_FB_ERR("not support this panel type(%d).\n", panel_type);
		break;
	}

	up(&hisi_fb_dts_resource_sem);

	return flag;
}

void hisi_fb_device_set_status0(uint32_t status)
{
	down(&hisi_fb_dts_resource_sem);
	g_dts_resouce_ready |= status;
	up(&hisi_fb_dts_resource_sem);
}

int hisi_fb_device_set_status1(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	down(&hisi_fb_dts_resource_sem);

	switch (hisifd->panel_info.type) {
	case PANEL_LCDC:
	case PANEL_MIPI_VIDEO:
	case PANEL_MIPI_CMD:
	case PANEL_DUAL_MIPI_VIDEO:
	case PANEL_DUAL_MIPI_CMD:
	case PANEL_DP:
	case PANEL_MIPI2RGB:
	case PANEL_RGB2MIPI:
	case PANEL_HDMI:
		if (hisifd->index == PRIMARY_PANEL_IDX) {
			g_dts_resouce_ready |= DTS_PANEL_PRIMARY_READY;
		} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
			g_dts_resouce_ready |= DTS_PANEL_EXTERNAL_READY;
		} else {
			HISI_FB_ERR("not support fb(%d).\n", hisifd->index);
		}
		break;
	case PANEL_OFFLINECOMPOSER:
		g_dts_resouce_ready |= DTS_PANEL_OFFLINECOMPOSER_READY;
		break;
	case PANEL_WRITEBACK:
		g_dts_resouce_ready |= DTS_PANEL_WRITEBACK_READY;
		break;
	case PANEL_MEDIACOMMON:
		g_dts_resouce_ready |= DTS_PANEL_MEDIACOMMON_READY;
		break;
	default:
		HISI_FB_ERR("not support this panel type(%d).\n", hisifd->panel_info.type);
		ret = -1;
		break;
	}

	up(&hisi_fb_dts_resource_sem);

	return ret;
}

struct platform_device *hisi_fb_device_alloc(struct hisi_fb_panel_data *pdata,
	uint32_t type, uint32_t id)
{
	struct platform_device *this_dev = NULL;
	char dev_name[32] = {0};

	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return NULL;
	}

	switch (type) {
	case PANEL_MIPI_VIDEO:
	case PANEL_MIPI_CMD:
	case PANEL_DUAL_MIPI_VIDEO:
	case PANEL_DUAL_MIPI_CMD:
		snprintf(dev_name, sizeof(dev_name), DEV_NAME_MIPIDSI);
		break;
	case PANEL_DP:
		snprintf(dev_name, sizeof(dev_name), DEV_NAME_DP);
		break;
	case PANEL_NO:
	case PANEL_LCDC:
	case PANEL_HDMI:
	case PANEL_OFFLINECOMPOSER:
	case PANEL_WRITEBACK:
	case PANEL_MEDIACOMMON:
		snprintf(dev_name, sizeof(dev_name), DEV_NAME_DSS_DPE);
		break;
	case PANEL_RGB2MIPI:
		snprintf(dev_name, sizeof(dev_name), DEV_NAME_RGB2MIPI);
		break;
	default:
		HISI_FB_ERR("invalid panel type = %d!\n", type);
		return NULL;
	}

	if (pdata != NULL) {
		pdata->next = NULL;
	}

	this_dev = platform_device_alloc(dev_name, (((uint32_t)type << 16) | (uint32_t)id));
	if (this_dev) {
		if (platform_device_add_data(this_dev, pdata, sizeof(struct hisi_fb_panel_data))) {
			HISI_FB_ERR("failed to platform_device_add_data!\n");
			platform_device_put(this_dev);
			return NULL;
		}
	}

	return this_dev;
}

void panel_check_status_and_report_by_dsm(struct lcd_reg_read_t *lcd_status_reg, int cnt, char __iomem *mipi_dsi0_base)
{
	u32 read_value = 0, expected_value = 0, read_mask = 0;
	u8 reg_addr = 0;
	char *reg_name = NULL;
	int dsm_error_found = 0, dsm_client_ready = 0;
	u32 pkg_status = 0, try_times = 100, i = 0;
	struct dsi_cmd_desc packet_size_set_cmd = {DTYPE_MAX_PKTSIZE, 0, 10, WAIT_TYPE_US, 1, NULL};
	bool for_debug = false;

	if (lcd_dclient && !dsm_client_ocuppy(lcd_dclient)) {
		dsm_client_ready = 1;
	}

	lcd_status_reg->recovery = 0;
	mipi_dsi_check_0lane_is_ready(mipi_dsi0_base);
	mipi_dsi_max_return_packet_size(&packet_size_set_cmd, mipi_dsi0_base);

	for (i = 0; i < cnt; i++) {
		/*Each iteration read a registers*/
		reg_addr = lcd_status_reg[i].reg_addr;
		reg_name = lcd_status_reg[i].reg_name;
		expected_value = lcd_status_reg[i].expected_value;
		read_mask = lcd_status_reg[i].read_mask;
		for_debug = lcd_status_reg[i].for_debug;

		/* Do not read debug register to avoid lcd light delay */
		if (for_debug) {
			continue;
		}

		/*Send MIPI read command, and wait the return value no longer than 100*30us*/
		try_times = 100;
		outp32(mipi_dsi0_base + MIPIDSI_GEN_HDR_OFFSET, reg_addr << 8 | 0x06);
		udelay(20);
		do {
			pkg_status = inp32(mipi_dsi0_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			if (!(pkg_status & 0x10)) break;
			udelay(30);
		} while (--try_times);

		/*Read the return value, report error if read timeout or read value is not expected*/
		read_value = inp32(mipi_dsi0_base + MIPIDSI_GEN_PLD_DATA_OFFSET);
		if (read_mask != 0)
			read_value &= read_mask;
		if (!try_times) {
			HISI_FB_ERR("Read %s timeout!\n", reg_name);
			if (dsm_client_ready) {
				dsm_client_record(lcd_dclient, "Read %s timeout!\n", reg_name);
				dsm_error_found++;
			}
			break;
		} else if (read_value != expected_value && read_mask != 0) {
			HISI_FB_ERR("ERROR: read %s = 0x%X, but expect: 0x%X\n", reg_name, read_value, expected_value);
			if (dsm_client_ready) {
				dsm_client_record(lcd_dclient, "ERROR: read %s = 0x%X, but expect: 0x%X\n", reg_name, read_value, expected_value);
				dsm_error_found++;
			}
			break;
		} else if (read_value == expected_value && read_mask == 0) {
			/*In this case read_mask == 0, read value equal to expected_value is a LCD error*/
			HISI_FB_ERR("ERROR: read %s = 0x%X, but expect NOT equal to: 0x%X\n", reg_name, read_value, expected_value);
			if (dsm_client_ready) {
				dsm_client_record(lcd_dclient, "ERROR: read %s = 0x%X, but expect NOT equal to: 0x%X\n", reg_name, read_value, expected_value);
				dsm_error_found++;
			}
			break;
		} else {
			HISI_FB_INFO("Read %s = 0x%x\n", reg_name, read_value);
		}
	}

	/*Report error to Device Status Monitor*/
	if (dsm_error_found > 0) {
		for (i = 0; i < cnt; i++) {
			reg_addr = lcd_status_reg[i].reg_addr;
			for_debug = lcd_status_reg[i].for_debug;
			expected_value = lcd_status_reg[i].expected_value;

			/* Do not read status registers because already read them just now */
			if (!for_debug) {
				continue;
			}

			/*Send MIPI read command, and wait the return value no longer than 100*30us*/
			try_times = 100;
			outp32(mipi_dsi0_base + MIPIDSI_GEN_HDR_OFFSET, reg_addr << 8 | 0x06);
			udelay(20);
			do {
				pkg_status = inp32(mipi_dsi0_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
				if (!(pkg_status & 0x10)) {
					break;
				}
				udelay(30);
			} while (--try_times);

			/* dump the register which you want to read for debug */
			read_value = inp32(mipi_dsi0_base + MIPIDSI_GEN_PLD_DATA_OFFSET);

			if (dsm_client_ready) {
				dsm_client_record(lcd_dclient,"Read 0x%X = 0x%x expect_value = 0x%x\n",
												reg_addr,read_value,expected_value);
			}
		}
		dsm_client_notify(lcd_dclient, DSM_LCD_STATUS_ERROR_NO);

		lcd_status_reg->recovery = 1;
	}

	if (dsm_client_ready) {
		dsm_client_unocuppy(lcd_dclient);
	} else {
		HISI_FB_INFO("dsm lcd_dclient ocuppy failed!\n");
	}
}
void panel_status_report_by_dsm(struct lcd_reg_read_t *lcd_status_reg, int cnt, char __iomem *mipi_dsi0_base, int report_cnt)
{
	u32 read_value = 0, expected_value = 0, read_mask = 0;
	u8 reg_addr = 0;
	char *reg_name = NULL;
	int dsm_error_found = 0, dsm_client_ready = 0;
	u32 pkg_status = 0, try_times = 100, i = 0;
	struct dsi_cmd_desc packet_size_set_cmd = {DTYPE_MAX_PKTSIZE, 0, 10, WAIT_TYPE_US, 1, NULL};
	static int dmd_report_count = 0;

	if (!dsm_client_ocuppy(lcd_dclient)) {
		dsm_client_ready = 1;
	}

	mipi_dsi_check_0lane_is_ready(mipi_dsi0_base);
	mipi_dsi_max_return_packet_size(&packet_size_set_cmd, mipi_dsi0_base);

	for (i = 0; i < cnt; i++) {
		/*Each iteration read a registers*/
		reg_addr = lcd_status_reg[i].reg_addr;
		reg_name = lcd_status_reg[i].reg_name;
		expected_value = lcd_status_reg[i].expected_value;
		read_mask = lcd_status_reg[i].read_mask;

		/*Send MIPI read command, and wait the return value no longer than 100*30us*/
		try_times = 100;
		outp32(mipi_dsi0_base + MIPIDSI_GEN_HDR_OFFSET, reg_addr << 8 | 0x06);
		udelay(20);
		do {
			pkg_status = inp32(mipi_dsi0_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			if (!(pkg_status & 0x10))
				break;
			udelay(30);
		} while (--try_times);

		/*Read the return value, report error if read timeout or read value is not expected*/
		read_value = inp32(mipi_dsi0_base + MIPIDSI_GEN_PLD_DATA_OFFSET);
		if (read_mask != 0)
			read_value &= read_mask;
		if (!try_times) {
			HISI_FB_ERR("Read %s timeout!\n", reg_name);
			if (dsm_client_ready) {
				dsm_client_record(lcd_dclient, "Read %s timeout!\n", reg_name);
				dsm_error_found++;
			}
			break;
		} else if (read_value != expected_value && read_mask != 0) {
			HISI_FB_ERR("ERROR: read %s = 0x%X, but expect: 0x%X\n", reg_name, read_value, expected_value);
			if (dsm_client_ready) {
				dsm_client_record(lcd_dclient, "ERROR: read %s = 0x%X, but expect: 0x%X\n", reg_name, read_value, expected_value);
				dsm_error_found++;
			}
			break;
		} else if (read_value == expected_value && read_mask == 0) {
			/*In this case read_mask == 0, read value equal to expected_value is a LCD error*/
			HISI_FB_ERR("ERROR: read %s = 0x%X, but expect NOT equal to: 0x%X\n", reg_name, read_value, expected_value);
			if (dsm_client_ready) {
				dsm_client_record(lcd_dclient, "ERROR: read %s = 0x%X, but expect NOT equal to: 0x%X\n", reg_name, read_value, expected_value);
				dsm_error_found++;
			}
			break;
		} else {
			HISI_FB_INFO("Read %s = 0x%x\n", reg_name, read_value);
		}
	}

	/*Report error to Device Status Monitor*/
	if (dsm_error_found > 0) {
		dmd_report_count ++;
		if (dmd_report_count == report_cnt) {
			dsm_client_notify(lcd_dclient, DSM_LCD_STATUS_ERROR_NO);
			dmd_report_count = 0;
		}
	}
	if (dsm_client_ready) {
		dsm_client_unocuppy(lcd_dclient);
	} else {
		HISI_FB_INFO("dsm lcd_dclient ocuppy failed!\n");
	}
}
/*lint +e574 +e647 +e568 +e685 +e578*/

int panel_next_tcon_mode(struct platform_device *pdev, struct hisi_panel_info *pinfo)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("platform_device is NULL");
		return -EINVAL;
	}

	pdata = dev_get_platdata(&pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("hisi_fb_panel_data is NULL");
		return -EINVAL;
	}

	if (NULL == pinfo) {
		HISI_FB_ERR("panel_info is NULL");
		return -EINVAL;
	}

	if (!pinfo->cascadeic_support) {
		HISI_FB_DEBUG("Nomal IC, do nothing.");
		return 0;
	}

	if (pinfo->current_display_region == EN_DISPLAY_REGION_NONE) {
		pinfo->current_display_region = EN_DISPLAY_REGION_AB;
		HISI_FB_INFO("first power on, set default region.");
		return 0;
	} else if ((pinfo->current_display_region == EN_DISPLAY_REGION_A) ||
			 (pinfo->current_display_region == EN_DISPLAY_REGION_B) ||
			 (pinfo->current_display_region == EN_DISPLAY_REGION_AB) ||
			 (pinfo->current_display_region == EN_DISPLAY_REGION_AB_FOLDED)) {
		HISI_FB_INFO("change DDIC display region to (%d).", pinfo->current_display_region);
	} else {
		pinfo->current_display_region = EN_DISPLAY_REGION_AB;
		HISI_FB_ERR("wrong display region, should not occur (%d).", pinfo->current_display_region);
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->set_tcon_mode)) {
			HISI_FB_INFO("send DCS cmd to DDIC.");
			ret = next_pdata->set_tcon_mode(next_pdev, pinfo->current_display_region);
		}
	}

	return ret;
}


//just for Cyclomatic Complexity, no need to check input param
static inline int _set_display_region(struct hisi_fb_data_type *hisifd, struct hisi_panel_info *pinfo, struct _panel_region_notify* region_notify)
{
	int ret = 0;

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on ) {
	    pinfo->current_display_region = region_notify->panel_display_region;
	    if (region_notify->notify_mode == EN_MODE_POWER_OFF_SWITCH_NOTIFY) {
	        HISI_FB_INFO("receive display region change (%d) when powe off, set when next power on.", pinfo->current_display_region);
	    } else {
	        HISI_FB_WARNING("receive display region change (%d) when powe off by %d notify_mode.", pinfo->current_display_region, region_notify->notify_mode);
	    }
	    up(&hisifd->blank_sem);
	    return ret;
	}

	hisifb_activate_vsync(hisifd);

	if (region_notify->notify_mode == EN_MODE_POWER_OFF_SWITCH_NOTIFY) {
		HISI_FB_WARNING("panel already power on, set DDIC display region(%d) right now, check the timing sequene.\n", pinfo->current_display_region);
		ret = panel_next_tcon_mode(hisifd->pdev, pinfo);
	} else if (region_notify->notify_mode == EN_MODE_PRE_NOTIFY) {
		//set DDIC right now
		if ( (region_notify->panel_display_region == EN_DISPLAY_REGION_AB) ||
		   (region_notify->panel_display_region == EN_DISPLAY_REGION_AB_FOLDED) ) {
			if (pinfo->current_display_region != region_notify->panel_display_region) {
				pinfo->current_display_region = region_notify->panel_display_region;
				HISI_FB_INFO("notify_mode:%d, change CurrentDisplayRegion to %d.", region_notify->notify_mode, pinfo->current_display_region);
				ret = panel_next_tcon_mode(hisifd->pdev, pinfo);
			} else {
				HISI_FB_INFO("same DDIC display region (%d), no need pre change.", pinfo->current_display_region);
			}
		} else {
			HISI_FB_ERR("wrong pre notify which region is %d.\n", region_notify->panel_display_region);
			ret = -EFAULT;
		}
	} else if (region_notify->notify_mode == EN_MODE_REAL_SWITCH_NOTIFY) {
		//need check with TUI & AOD, select the biggest one
		//TBD...
		if (pinfo->current_display_region != region_notify->panel_display_region) {
			pinfo->current_display_region = region_notify->panel_display_region;
			HISI_FB_INFO("notify_mode:%d, change CurrentDisplayRegion to %d.", region_notify->notify_mode, pinfo->current_display_region);
			ret = panel_next_tcon_mode(hisifd->pdev, pinfo);
		} else {
			HISI_FB_INFO("same DDIC display region (%d), no need real change.", pinfo->current_display_region);
		}
	}

	hisifb_deactivate_vsync(hisifd);
	up(&hisifd->blank_sem);

	return ret;
}

int panel_set_display_region(struct hisi_fb_data_type *hisifd, void __user *argp)
{
	int ret = 0;
	struct _panel_region_notify region_notify;
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("argp is NULL\n");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	if (NULL == pinfo) {
		HISI_FB_ERR("panel_info is NULL");
		return -EINVAL;
	}

	ret = (int)copy_from_user(&region_notify, argp, sizeof(struct _panel_region_notify));
	if (ret) {
		HISI_FB_ERR("copy_from_user(param) failed! ret=%d.\n", ret);
		return -EFAULT;
	}

	ret = _set_display_region(hisifd, pinfo, &region_notify);

	//set partial update max area
	//TODO...

	return ret;
}

