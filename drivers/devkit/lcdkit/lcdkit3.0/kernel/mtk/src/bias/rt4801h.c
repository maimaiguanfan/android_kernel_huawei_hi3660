/*
 * rt4801h.c
 *
 * rt4801h bias driver
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

#include "rt4801h.h"
#include "lcd_kit_common.h"
#include "lcd_kit_core.h"
#include "lcd_kit_bias.h"
#ifdef CONFIG_HUAWEI_DEV_SELFCHECK
#include <huawei_platform/dev_detect/hw_dev_detect.h>
#endif

#define DTS_COMP_RT4801H "rt,rt4801h"
#define BIAS_TRUE 1
#define BIAS_FALSE 0

static struct rt4801h_device_info *dev_info;
static int g_vol_mapped = BIAS_FALSE;
static unsigned char vpos_cmd;
static unsigned char vneg_cmd;
static struct rt4801h_voltage voltage_table[] = {
	{ LCD_BIAS_VOL_40, RT4801H_VOL_40 },
	{ LCD_BIAS_VOL_41, RT4801H_VOL_41 },
	{ LCD_BIAS_VOL_42, RT4801H_VOL_42 },
	{ LCD_BIAS_VOL_43, RT4801H_VOL_43 },
	{ LCD_BIAS_VOL_44, RT4801H_VOL_44 },
	{ LCD_BIAS_VOL_45, RT4801H_VOL_45 },
	{ LCD_BIAS_VOL_46, RT4801H_VOL_46 },
	{ LCD_BIAS_VOL_47, RT4801H_VOL_47 },
	{ LCD_BIAS_VOL_48, RT4801H_VOL_48 },
	{ LCD_BIAS_VOL_49, RT4801H_VOL_49 },
	{ LCD_BIAS_VOL_50, RT4801H_VOL_50 },
	{ LCD_BIAS_VOL_51, RT4801H_VOL_51 },
	{ LCD_BIAS_VOL_52, RT4801H_VOL_52 },
	{ LCD_BIAS_VOL_53, RT4801H_VOL_53 },
	{ LCD_BIAS_VOL_54, RT4801H_VOL_54 },
	{ LCD_BIAS_VOL_55, RT4801H_VOL_55 },
	{ LCD_BIAS_VOL_56, RT4801H_VOL_56 },
	{ LCD_BIAS_VOL_57, RT4801H_VOL_57 },
	{ LCD_BIAS_VOL_58, RT4801H_VOL_58 },
	{ LCD_BIAS_VOL_59, RT4801H_VOL_59 },
	{ LCD_BIAS_VOL_60, RT4801H_VOL_60 }
};

static void rt4801h_get_target_voltage(int vpos_target, int vneg_target)
{
	int i;
	int vol_size = ARRAY_SIZE(voltage_table);

	for (i = 0; i < vol_size; i++) {
		if (voltage_table[i].voltage == vpos_target) {
			LCD_KIT_INFO("rt4801h vsp voltage:0x%x\n",
				voltage_table[i].value);
			vpos_cmd = voltage_table[i].value;
			break;
		}
	}
	if (i >= vol_size) {
		LCD_KIT_ERR("not found vsp voltage, use default voltage\n");
		vpos_cmd = RT4801H_VOL_55;
	}
	for (i = 0; i < vol_size; i++) {
		if (voltage_table[i].voltage == vneg_target) {
			LCD_KIT_INFO("rt4801h vsn voltage:0x%x\n",
				voltage_table[i].value);
			vneg_cmd = voltage_table[i].value;
			break;
		}
	}
	if (i >= vol_size) {
		LCD_KIT_ERR("not found vsn voltage, use default voltage\n");
		vneg_cmd = RT4801H_VOL_55;
	}
	LCD_KIT_INFO("vpos = 0x%x, vneg = 0x%x\n", vpos_cmd, vneg_cmd);
}

static int rt4801h_reg_init(struct i2c_client *client, unsigned char vpos_cmd,
	unsigned char vneg_cmd)
{
	unsigned char app_dis;
	int ret = LCD_KIT_FAIL;

	if (client == NULL) {
		LCD_KIT_ERR("%s: NULL point for client\n", __func__);
		return ret;
	}

	ret = i2c_smbus_read_byte_data(client, RT4801H_REG_APP_DIS);
	if (ret < 0) {
		LCD_KIT_ERR("%s: read app_dis failed\n", __func__);
		return ret;
	}
	app_dis = (unsigned char)ret;
	app_dis = app_dis | RT4801H_DISP_BIT | RT4801H_DISN_BIT |
		RT4801H_APPS_BIT;

	ret = i2c_smbus_write_byte_data(client, RT4801H_REG_VPOS, vpos_cmd);
	if (ret < 0) {
		LCD_KIT_ERR("%s: write vpos failed\n", __func__);
		return ret;
	}

	ret = i2c_smbus_write_byte_data(client, RT4801H_REG_VNEG, vneg_cmd);
	if (ret < 0) {
		LCD_KIT_ERR("%s: write vneg failed\n", __func__);
		return ret;
	}

	ret = i2c_smbus_write_byte_data(client, RT4801H_REG_APP_DIS,
		app_dis);
	if (ret < 0)
		LCD_KIT_ERR("%s: write app_dis failed\n", __func__);

	return ret;
}

void rt4801h_set_voltage(unsigned char vpos, unsigned char vneg)
{
	int ret;

	if (vpos >= RT4801H_VOL_MAX) {
		LCD_KIT_ERR("set vpos error, vpos = %d is out of range\n", vpos);
		return;
	}

	if (vneg >= RT4801H_VOL_MAX) {
		LCD_KIT_ERR("set vneg error, vneg = %d is out of range\n", vneg);
		return;
	}

	if (dev_info == NULL)
		return;

	ret = rt4801h_reg_init(dev_info->client, vpos, vneg);
	if (ret)
		LCD_KIT_ERR("rt4801h reg init not success\n");
	LCD_KIT_INFO("rt4801h set voltage succeed\n");
}

static int rt4801h_set_bias(int vpos, int vneg)
{
	if (g_vol_mapped == BIAS_FALSE) {
		rt4801h_get_target_voltage(vpos, vneg);
		g_vol_mapped = BIAS_TRUE;
	}

	rt4801h_set_voltage(vpos_cmd, vneg_cmd);
	return LCD_KIT_OK;
}

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = rt4801h_set_bias,
	.dbg_set_bias_voltage = NULL,
};

static int rt4801h_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int retval = 0;

	if (client == NULL) {
		pr_err("%s: NULL point for client\n", __func__);
		retval = -ENODEV;
		goto failed_1;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		LCD_KIT_ERR("%s: need I2C_FUNC_I2C\n", __func__);
		retval = -ENODEV;
		goto failed_1;
	}
	dev_info = kzalloc(sizeof(*dev_info), GFP_KERNEL);
	if (!dev_info) {
		dev_err(&client->dev, "failed to allocate device info\n");
		retval = -ENOMEM;
		goto failed_1;
	}
	i2c_set_clientdata(client, dev_info);
	dev_info->dev = &client->dev;
	dev_info->client = client;

#ifdef CONFIG_HUAWEI_DEV_SELFCHECK
	set_hw_dev_detect_result(DEV_DETECT_DC_DC);
#endif
	lcd_kit_bias_register(&bias_ops);
	return retval;

failed_1:
	if (dev_info) {
		kfree(dev_info);
		dev_info = NULL;
	}
	return retval;
}

static const struct of_device_id rt4801h_match_table[] = {
	{
		.compatible = DTS_COMP_RT4801H,
		.data = NULL,
	},
	{},
};


static const struct i2c_device_id rt4801h_i2c_id[] = {
	{ "rt4801h", 0 },
	{}
};

MODULE_DEVICE_TABLE(of, rt4801h_match_table);

static struct i2c_driver rt4801h_driver = {
	.id_table = rt4801h_i2c_id,
	.probe = rt4801h_probe,
	.driver = {
		.name = "rt4801h",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(rt4801h_match_table),
	},
};

static int __init rt4801h_module_init(void)
{
	int ret;

	ret = i2c_add_driver(&rt4801h_driver);
	if (ret)
		LCD_KIT_ERR("unable to register rt4801h driver\n");
	return ret;
}

static void __exit rt4801h_exit(void)
{
	i2c_del_driver(&rt4801h_driver);
}

module_init(rt4801h_module_init);
module_exit(rt4801h_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("driver for rt4801h");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
