/*
 * buck_boost.c
 *
 * buck_boost driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/power/buck_boost.h>

#define HWLOG_TAG buck_boost
HWLOG_REGIST();

static struct bbst_device_ops *g_bbst_ops;
static struct bbst_device_info *g_bbst_di;

int buck_boost_ops_register(struct bbst_device_ops *ops)
{
	if (!ops) {
		hwlog_err("%s: fail\n", __func__);
		return -EPERM;
	}

	g_bbst_ops = ops;
	hwlog_info("[%s] succ\n", __func__);
	return 0;
}

int buck_boost_set_vol(unsigned int vol)
{
	struct bbst_device_info *di = g_bbst_di;

	if (!di || !di->ops || !di->ops->set_vout) {
		hwlog_err("%s: di or ops null\n", __func__);
		return -EPERM;
	}

	return di->ops->set_vout(vol);
}

bool buck_boost_pwr_good(void)
{
	struct bbst_device_info *di = g_bbst_di;

	if (!di || !di->ops || !di->ops->pwr_good) {
		hwlog_err("%s: di or ops null\n", __func__);
		return false;
	}

	return di->ops->pwr_good();
}

static int buck_boost_probe(struct platform_device *pdev)
{
	struct bbst_device_info *di = NULL;

	if (!pdev)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_bbst_di = di;
	di->dev = &pdev->dev;
	di->ops = g_bbst_ops;
	platform_set_drvdata(pdev, di);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	if (di->ops && di->ops->device_check) {
		if (di->ops->device_check())
			set_hw_dev_flag(DEV_I2C_BUCKBOOST);
		else
			hwlog_err("%s: device check fail\n", __func__);
	}
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

	buck_boost_set_vol(BBST_DEFAULT_VOUT);

	hwlog_info("[%s] ok\n", __func__);
	return 0;
}

static const struct of_device_id buck_boost_match_table[] = {
	{
		.compatible = "huawei, buck_boost",
		.data = NULL,
	},
	{},
};

static struct platform_driver buck_boost_driver = {
	.probe = buck_boost_probe,
	.remove = NULL,
	.shutdown = NULL,
	.driver = {
		   .name = "huawei, buck_boost",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(buck_boost_match_table),
	},
};

static int __init buck_boost_init(void)
{
	return platform_driver_register(&buck_boost_driver);
}

static void __exit buck_boost_exit(void)
{
	platform_driver_unregister(&buck_boost_driver);
}

device_initcall_sync(buck_boost_init);
module_exit(buck_boost_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("buck_boost module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
