/*
 * high_resistance_hs_det.c
 *
 * support for high_res_hs_det module
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "huawei_platform/audio/high_resistance_hs_det.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/miscdevice.h>
#include "huawei_platform/log/hw_log.h"

#define HWLOG_TAG high_resistance_hs_det
HWLOG_REGIST();

#define DEFAULT_MIN_THRESHOLD    180
#define DEFAULT_MAX_THRESHOLD    220
#define DEFAULT_FEEDBACK_RES_VAL 0x2
#define DEFAULT_OUTPUT_AMPLITUDE 0x68
#define DEFAULT_INNER_FB_RES     16000
#define DEFAULT_MIN_INNER_FB_RES 371
#define DEFAULT_MAX_INNER_FB_RES 32864

struct high_res_hs_data *g_high_res_hs_data;

bool check_high_res_hs_det_support(void)
{
	if (g_high_res_hs_data != NULL)
		return true;

	return false;
}

int high_res_dev_register(struct high_res_hs_dev *dev, void *codec_data)
{
	if ((g_high_res_hs_data == NULL) ||
		(g_high_res_hs_data->codec_ops_dev != NULL)) {
		hwlog_err("pdata is NULL or codec has been registed\n");
		return -ENODEV;
	}

	mutex_lock(&g_high_res_hs_data->mutex);
	g_high_res_hs_data->codec_ops_dev = dev;
	g_high_res_hs_data->private_data = codec_data;
	mutex_unlock(&g_high_res_hs_data->mutex);

	hwlog_info("success\n");
	return 0;
}

int get_high_res_data(const unsigned int cmd)
{
	int val;

	if (g_high_res_hs_data == NULL) {
		hwlog_err("g_high_res_hs_data is NULL\n");
		return -ENODEV;
	}

	mutex_lock(&g_high_res_hs_data->mutex);
	switch (cmd) {
	case HIGH_RES_GET_FB_VAL:
		val = g_high_res_hs_data->high_res_feedback_value;
		hwlog_info("high_res_feedback_value = %d\n", val);
		break;
	case HIGH_RES_GET_HS_STATE:
		val = g_high_res_hs_data->high_res_hs_status;
		hwlog_info("high_res_hs_status = %d\n", val);
		break;
	case HIGH_RES_GET_CALIB_STATE:
		val = g_high_res_hs_data->res_calib_status;
		hwlog_info("res_calib_status = %d\n", val);
		break;
	case HIGH_RES_GET_CALIB_VAL:
		val = g_high_res_hs_data->res_calib_val;
		hwlog_info("res_calib_val = %d\n", val);
		break;
	case HIGH_RES_GET_MIN_THRESHOLD:
		val = g_high_res_hs_data->high_res_min_threshold;
		hwlog_info("high res min threshold = %d\n", val);
		break;
	case HIGH_RES_GET_MAX_THRESHOLD:
		val = g_high_res_hs_data->high_res_max_threshold;
		hwlog_info("high res max threshold = %d\n", val);
		break;
	case HIGH_RES_GET_OUTPUT_AMP:
		val = g_high_res_hs_data->output_amplitude;
		hwlog_info("high res output_amplitude = %d\n", val);
		break;
	default:
		val = HIGH_RES_INVALLID_VAL;
		hwlog_err("invalid cmd, set val = %d\n", val);
		break;
	}
	mutex_unlock(&g_high_res_hs_data->mutex);
	return val;
}

void set_high_res_data(const unsigned int cmd, const unsigned int val)
{
	if (g_high_res_hs_data == NULL) {
		hwlog_err("g_high_res_hs_data is NULL\n");
		return;
	}

	mutex_lock(&g_high_res_hs_data->mutex);
	switch (cmd) {
	case HIGH_RES_SET_HS_STATE:
		if (val > HIGH_RES_HS) {
			hwlog_err("invalid val\n");
			mutex_unlock(&g_high_res_hs_data->mutex);
			return;
		}
		g_high_res_hs_data->high_res_hs_status = val;
		hwlog_info("update high resistance headset status = %u\n", val);
		break;
	case HIGH_RES_SET_CALIB_STATE:
		if (val > RES_CALIBRATED) {
			hwlog_err("invalid val\n");
			mutex_unlock(&g_high_res_hs_data->mutex);
			return;
		}
		g_high_res_hs_data->res_calib_status = val;
		hwlog_info("update resistance calibration status = %u\n", val);
		break;
	case HIGH_RES_SET_CALIB_VAL:
		if (val < DEFAULT_MIN_INNER_FB_RES ||
			val > DEFAULT_MAX_INNER_FB_RES) {
			hwlog_err("invalid val\n");
			mutex_unlock(&g_high_res_hs_data->mutex);
			return;
		}
		g_high_res_hs_data->res_calib_val = val;
		hwlog_info("update resistance calibration value = %u\n", val);
		break;
	default:
		hwlog_err("invalid cmd\n");
		break;
	}
	mutex_unlock(&g_high_res_hs_data->mutex);
	hwlog_info("success\n");
}

static int high_res_hs_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct high_res_hs_data *priv;
	int ret;

	hwlog_info("begin\n");
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (priv == NULL)
		return -ENOMEM;

	ret = of_property_read_u32(dev->of_node, "high_res_min_threshold",
			&(priv->high_res_min_threshold));
	if (ret < 0) {
		hwlog_info("high_res_min_threshold use default value\n");
		priv->high_res_min_threshold = DEFAULT_MIN_THRESHOLD;
	}

	ret = of_property_read_u32(dev->of_node, "high_res_max_threshold",
			&(priv->high_res_max_threshold));
	if (ret < 0) {
		hwlog_info("high_res_max_threshold use default value\n");
		priv->high_res_max_threshold = DEFAULT_MAX_THRESHOLD;
	}

	ret = of_property_read_u32(dev->of_node, "high_res_feedback_value",
			&(priv->high_res_feedback_value));
	if (ret < 0) {
		hwlog_info("high_res_feedback_value use default value\n");
		priv->high_res_feedback_value = DEFAULT_FEEDBACK_RES_VAL;
	}

	ret = of_property_read_u32(dev->of_node, "output_amplitude",
			&(priv->output_amplitude));
	if (ret < 0) {
		hwlog_info("output_amplitude use default value\n");
		priv->output_amplitude = DEFAULT_OUTPUT_AMPLITUDE;
	}
	priv->high_res_hs_status = NORMAL_HS;
	priv->res_calib_status = RES_NOT_CALIBRATED;
	priv->res_calib_val = DEFAULT_INNER_FB_RES;
	mutex_init(&(priv->mutex));
	g_high_res_hs_data = priv;
	return ret;
}

static int high_res_hs_remove(struct platform_device *pdev)
{
	hwlog_debug("begin\n");
	if (pdev == NULL || g_high_res_hs_data == NULL)
		return -ENODEV;

	mutex_destroy(&(g_high_res_hs_data->mutex));
	kfree(g_high_res_hs_data);
	g_high_res_hs_data = NULL;
	hwlog_debug("end\n");
	return 0;
}

static const struct of_device_id high_res_hs_match_table[] = {
	{
		.compatible = "huawei,high_resistance_hs",
		.data = NULL,
	},
	{},
};

static struct platform_driver high_res_hs_driver = {
	.probe  = high_res_hs_probe,
	.remove = high_res_hs_remove,
	.driver = {
		.name = "huawei,high_resistance_hs",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(high_res_hs_match_table),
	},
};

static int __init high_res_hs_init(void)
{
	return platform_driver_register(&high_res_hs_driver);
}

static void __exit high_res_hs_exit(void)
{
	platform_driver_unregister(&high_res_hs_driver);
}

subsys_initcall(high_res_hs_init);
module_exit(high_res_hs_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("high res hs detect module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

