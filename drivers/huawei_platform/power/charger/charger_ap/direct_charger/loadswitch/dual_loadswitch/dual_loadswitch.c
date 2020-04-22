/*
 * dual_loadswitch.c
 *
 * dual loadswitch driver
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/slab.h>

#include <huawei_platform/log/hw_log.h>
#include <dual_loadswitch.h>

#define HWLOG_TAG dual_loadswitch
HWLOG_REGIST();

static struct dual_loadswitch_info *dli;
static struct loadswitch_ops *g_main_ops;
static struct loadswitch_ops *g_aux_ops;

int loadswitch_main_ops_register(struct loadswitch_ops *ops)
{
	int ret = 0;

	if (ops != NULL) {
		g_main_ops = ops;
		hwlog_info("dual loadswitch main ops register ok\n");
	} else {
		hwlog_info("dual loadswitch main ops has registered\n");
		ret = -1;
	}

	return ret;
}

int loadswitch_aux_ops_register(struct loadswitch_ops *ops)
{
	int ret = 0;

	if (ops != NULL) {
		g_aux_ops = ops;
		hwlog_info("dual loadswitch aux ops register ok\n");
	} else {
		hwlog_info("dual loadswitch aux ops has registered\n");
		ret = -1;
	}

	return ret;
}

static int dual_loadswitch_charge_status(void)
{
	int ret = 0;

	if (g_main_ops != NULL && g_main_ops->ls_status != NULL)
		ret |= g_main_ops->ls_status();
	else
		ret |= -1;

	if (g_aux_ops != NULL && g_aux_ops->ls_status != NULL)
		ret |= g_aux_ops->ls_status();
	else
		ret |= -1;

	return ret;
}

static int dual_loadswitch_charge_init(void)
{
	int ret = 0;

	if (g_main_ops != NULL && g_main_ops->ls_init != NULL)
		ret |= g_main_ops->ls_init();

	if (g_aux_ops != NULL && g_aux_ops->ls_init != NULL)
		ret |= g_aux_ops->ls_init();

	return ret;
}

static int dual_loadswitch_charge_exit(void)
{
	int ret = 0;

	if (g_main_ops != NULL && g_main_ops->ls_exit != NULL)
		ret |= g_main_ops->ls_exit();

	if (g_aux_ops != NULL && g_aux_ops->ls_exit != NULL)
		ret |= g_aux_ops->ls_exit();

	return ret;
}

static int dual_loadswitch_enable(int enable)
{
	int ret = 0;

	if (g_main_ops != NULL && g_main_ops->ls_enable != NULL)
		ret |= g_main_ops->ls_enable(enable);

	if (g_aux_ops != NULL && g_aux_ops->ls_enable != NULL)
		ret |= g_aux_ops->ls_enable(enable);

	return ret;
}

static int dual_loadswitch_discharge(int enable)
{
	int ret = 0;

	if (g_main_ops != NULL && g_main_ops->ls_discharge != NULL)
		ret |= g_main_ops->ls_discharge(enable);

	if (g_aux_ops != NULL && g_aux_ops->ls_discharge != NULL)
		ret |= g_aux_ops->ls_discharge(enable);

	return ret;
}

static int dual_loadswitch_is_ls_close(void)
{
	int ret = 0;

	if (g_main_ops != NULL && g_main_ops->is_ls_close != NULL)
		ret |= g_main_ops->is_ls_close();

	if (g_aux_ops != NULL && g_aux_ops->is_ls_close != NULL)
		ret |= g_aux_ops->is_ls_close();

	return ret;
}

static int dual_loadswitch_get_ls_id(void)
{
	int ret = -1;

	if (g_main_ops != NULL && g_main_ops->get_ls_id != NULL)
		ret = g_main_ops->get_ls_id();

	return ret;
}

static int dual_loadswitch_config_watchdog_ms(int time)
{
	int ret = 0;

	if (g_main_ops != NULL && g_main_ops->watchdog_config_ms != NULL)
		ret |= g_main_ops->watchdog_config_ms(time);

	if (g_aux_ops != NULL && g_aux_ops->watchdog_config_ms != NULL)
		ret |= g_aux_ops->watchdog_config_ms(time);

	return ret;
}

struct loadswitch_ops dual_loadswitch_ops = {
	.ls_init = dual_loadswitch_charge_init,
	.ls_exit = dual_loadswitch_charge_exit,
	.ls_enable = dual_loadswitch_enable,
	.ls_discharge = dual_loadswitch_discharge,
	.is_ls_close = dual_loadswitch_is_ls_close,
	.get_ls_id = dual_loadswitch_get_ls_id,
	.ls_status = dual_loadswitch_charge_status,
	.watchdog_config_ms = dual_loadswitch_config_watchdog_ms,
};

static int dual_loadswitch_probe(struct platform_device *pdev)
{
	struct dual_loadswitch_info *info = NULL;
	int ret = -1;

	hwlog_info("probe begin\n");

	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;

	dli = info;

	info->pdev = pdev;
	info->dev = &pdev->dev;
	if (info->pdev == NULL || info->dev == NULL ||
		info->dev->of_node == NULL) {
		hwlog_err("device_node is null\n");
		goto dual_loadswitch_fail_0;
	}

	ret = loadswitch_ops_register(&dual_loadswitch_ops);
	if (ret) {
		hwlog_err("dual loadswitch ops register failed\n");
		goto dual_loadswitch_fail_0;
	}

	if (g_main_ops == NULL ||
		g_main_ops->ls_init == NULL ||
		g_main_ops->ls_enable == NULL ||
		g_main_ops->ls_exit == NULL) {
		hwlog_err("main loadswitch ops is null\n");
		ret = -EINVAL;
		goto dual_loadswitch_fail_1;
	}

	if (g_aux_ops == NULL ||
		g_aux_ops->ls_init == NULL ||
		g_aux_ops->ls_enable == NULL ||
		g_aux_ops->ls_exit == NULL) {
		hwlog_err("aux loadswitch ops is null\n");
		ret = -EINVAL;
		goto dual_loadswitch_fail_2;
	}

	platform_set_drvdata(pdev, info);

	hwlog_info("probe end\n");
	return ret;

dual_loadswitch_fail_2:
	g_aux_ops = NULL;
dual_loadswitch_fail_1:
	g_main_ops = NULL;
dual_loadswitch_fail_0:
	devm_kfree(&pdev->dev, info);
	dli = NULL;

	return ret;
}

static int dual_loadswitch_remove(struct platform_device *pdev)
{
	struct dual_loadswitch_info *info = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, info);
	dli = NULL;

	hwlog_info("remove end\n");

	return 0;
}

static const struct of_device_id dual_loadswitch_match_table[] = {
	{
		.compatible = "dual_loadswitch",
		.data = NULL,
	},
	{},
};

static struct platform_driver dual_loadswitch_driver = {
	.probe = dual_loadswitch_probe,
	.remove = dual_loadswitch_remove,
	.driver = {
		.name = "dual_loadswitch",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dual_loadswitch_match_table),
	},
};

static int __init dual_loadswitch_init(void)
{
	return platform_driver_register(&dual_loadswitch_driver);
}

static void __exit dual_loadswitch_exit(void)
{
	platform_driver_unregister(&dual_loadswitch_driver);
}

device_initcall_sync(dual_loadswitch_init);
module_exit(dual_loadswitch_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("dual loadswitch module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
