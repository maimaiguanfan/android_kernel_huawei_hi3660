/*
 * irda_driver.c
 *
 * irda module init
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
 */

#include <irda_driver.h>

#define HWLOG_TAG irda
HWLOG_REGIST();

struct class *irda_class;

static int get_chip_type(void)
{
	struct device_node *chip_np = NULL;
	int chip_type;
	int ret;

	chip_np = of_find_compatible_node(
				NULL, NULL, IRDA_DRIVER_COMPATIBLE_ID);
	if (!chip_np) {
		hwlog_err("not find node %s\n", IRDA_DRIVER_COMPATIBLE_ID);
		/* default type is maxim, for compatible with old products */
		return MAXIM_616;
	}

	ret = of_property_read_u32(chip_np, IRDA_CHIP_TYPE, &chip_type);
	if (ret) {
		hwlog_err("fail to get irda chip type\n");
		/* default type is maxim, for compatible with old products */
		return MAXIM_616;
	}
	return chip_type;
}

static int irda_remote_init(void)
{
	int ret;
	int chip_type;

	irda_class = class_create(THIS_MODULE, "irda");
	if (IS_ERR(irda_class)) {
		ret = PTR_ERR(irda_class);
		hwlog_err("Failed to create irda class; ret:%d\n", ret);
		return ret;
	}

	chip_type = get_chip_type();
	if ((chip_type == HI11xx) || (chip_type == HI64XX))
		ret = irda_chip_type_regist();
	else
		ret = irda_maxim_power_config_regist();

	if (ret < 0)
		goto free_class;

	return 0;

free_class:
	hwlog_err("Failed to init irda driver");
	class_destroy(irda_class);
	irda_class = NULL;
	return ret;
}

static void irda_remote_exit(void)
{
	irda_maxim_power_config_unregist();
	irda_chip_type_unregist();
	class_destroy(irda_class);
	irda_class = NULL;
}

module_init(irda_remote_init);
module_exit(irda_remote_exit);

MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("debug for irda module driver");
