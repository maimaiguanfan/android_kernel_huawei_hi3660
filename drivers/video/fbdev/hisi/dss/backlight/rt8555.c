/*
 * rt8555.c
 *
 * rt8555 driver for backlight
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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

#include "rt8555.h"
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/regmap.h>
#include <linux/semaphore.h>

static bool g_rt8555_used;
static bool g_init_status;
static struct class *g_rt_class;
static struct rt8555_chip_data *g_rtdev;
static struct rt8555_info g_bl_info;

static struct gpio_desc g_gpio_on_cmds[] = {
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_US,
		DELAY_0_US, GPIO_RT8555_EN_NAME,
		&g_bl_info.rt8555_hw_en_gpio, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS,
		DELAY_5_MS, GPIO_RT8555_EN_NAME,
		&g_bl_info.rt8555_hw_en_gpio, 1 },
};
static struct gpio_desc g_gpio_disable_cmds[] = {
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_US,
		DELAY_0_US, GPIO_RT8555_EN_NAME,
		&g_bl_info.rt8555_hw_en_gpio, 0 },
	{ DTYPE_GPIO_INPUT, WAIT_TYPE_US,
		DELAY_10_US, GPIO_RT8555_EN_NAME,
		&g_bl_info.rt8555_hw_en_gpio, 0 },
};
static struct gpio_desc g_gpio_free_cmds[] = {
	{ DTYPE_GPIO_FREE, WAIT_TYPE_US,
		DELAY_50_US, GPIO_RT8555_EN_NAME,
		&g_bl_info.rt8555_hw_en_gpio, 0 },
};
static char *g_dts_str[RT8555_RW_REG_MAX] = {
	"rt8555_control_mode",       /* register 0x00 */
	"rt8555_current_protection", /* register 0x01 */
	"rt8555_current_setting",    /* register 0x02 */
	"rt8555_voltage_setting",    /* register 0x03 */
	"rt8555_brightness_setting", /* register 0x08 */
	"rt8555_time_control",       /* register 0x09 */
	"rt8555_mode_devision",      /* register 0x0A */
	"rt8555_compensation_duty",  /* register 0x0B */
	"rt8555_clk_pfm_enable",     /* register 0x0D */
	"rt8555_led_protection",     /* register 0x0E */
};
static unsigned int g_reg_addr[RT8555_RW_REG_MAX] = {
	RT8555_CONTROL_MODE_ADDR,
	RT8555_CURRENT_PROTECTION_ADDR,
	RT8555_CURRENT_SETTING_ADDR,
	RT8555_VOLTAGE_SETTING_ADDR,
	RT8555_BRIGHTNESS_SETTING_ADDR,
	RT8555_TIME_CONTROL_ADDR,
	RT8555_MODE_DEVISION_ADDR,
	RT8555_COMPENSATION_DUTY_ADDR,
	RT8555_CLK_PFM_ENABLE_ADDR,
	RT8555_LED_PROTECTION_ADDR,
};

unsigned int g_rt8555_msg_level = DEFAULT_MSG_LEVEL;
module_param_named(debug_rt8555_msg_level, g_rt8555_msg_level, int, 0640);
MODULE_PARM_DESC(debug_rt8555_msg_level, "backlight rt8555 msg level");

static int rt8555_parse_dts(struct device_node *np)
{
	int i;
	int ret;

	for (i = 0; i < RT8555_RW_REG_MAX; i++) {
		ret = of_property_read_u32(np, g_dts_str[i],
			&g_bl_info.reg[i]);
		if (ret < 0) {
			g_bl_info.reg[i] = PARSE_FAILED;
			RT8555_INFO(g_rtdev->dev, "Not find :%s\n",
				g_dts_str[i]);
		}
	}
	ret = of_property_read_u32(np, "rt8555_hw_en_gpio",
		&g_bl_info.rt8555_hw_en_gpio);
	if (ret < 0) {
		RT8555_ERR(g_rtdev->dev,
			"get rt8555_hw_en_gpio dts config failed\n");
		return ret;
	}
	ret = of_property_read_u32(np, "bl_on_kernel_mdelay",
		&g_bl_info.bl_on_kernel_mdelay);
	if (ret < 0) {
		RT8555_ERR(g_rtdev->dev,
			"get bl_on_kernel_mdelay dts config failed\n");
		return ret;
	}
	ret = of_property_read_u32(np, "bl_led_num",
		&g_bl_info.bl_led_num);
	if (ret < 0) {
		RT8555_ERR(g_rtdev->dev, "get bl_led_num dts config failed\n");
		return ret;
	}
	return ret;
}

static int rt8555_config_write(struct rt8555_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	int i;
	int ret = 0;

	for (i = 0; i < size; i++) {
		if (val[i] != PARSE_FAILED) {
			ret = regmap_write(pchip->regmap, reg[i], val[i]);
			if (ret < 0) {
				RT8555_ERR(g_rtdev->dev,
					"write reg 0x%x failed\n", reg[i]);
				goto exit;
			}
		}
	}
exit:
	return ret;
}

static int rt8555_config_read(struct rt8555_chip_data *pchip,
	unsigned int reg[], unsigned int val[], unsigned int size)
{
	int ret = 0;
	unsigned int i;

	for (i = 0; i < size; i++) {
		ret = regmap_read(pchip->regmap, reg[i], &val[i]);
		if (ret < 0) {
			RT8555_ERR(g_rtdev->dev,
				"read rt8555 register 0x%x failed\n", reg[i]);
			goto exit;
		} else {
			RT8555_INFO(g_rtdev->dev, "read 0x%x value = 0x%x\n",
				reg[i], val[i]);
		}
	}
exit:
	return ret;
}

static int rt8555_chip_init(struct rt8555_chip_data *pchip)
{
	int ret;

	RT8555_INFO(g_rtdev->dev, "in\n");
	ret = rt8555_config_write(pchip, g_reg_addr,
		g_bl_info.reg, RT8555_RW_REG_MAX);
	if (ret < 0) {
		RT8555_ERR(g_rtdev->dev, "rt8555 register failed\n");
		goto out;
	}
	RT8555_INFO(g_rtdev->dev, "OK\n");
	return ret;
out:
	dev_err(pchip->dev, "i2c failed to access register\n");
	return ret;
}

ssize_t rt8555_set_backlight_init(uint32_t bl_level)
{
	int ret = 0;

	if (g_fake_lcd_flag) {
		RT8555_INFO(g_rtdev->dev, "is fake lcd\n");
		return ret;
	}
	if (down_trylock(&(g_rtdev->test_sem))) {
		RT8555_INFO(g_rtdev->dev, "Now in test mode\n");
		return 0;
	}
	if ((g_init_status == false) && (bl_level > 0)) {
		mdelay(g_bl_info.bl_on_kernel_mdelay);
		gpio_cmds_tx(g_gpio_on_cmds,
			ARRAY_SIZE(g_gpio_on_cmds));
		/* chip initialize */
		ret = rt8555_chip_init(g_rtdev);
		if (ret < 0) {
			RT8555_ERR(g_rtdev->dev, "rt8555 init fail\n");
			goto out;
		}
		g_init_status = true;
	} else {
		RT8555_DEBUG(g_rtdev->dev,
			"rt8555 %u, 0: off; else inited\n", bl_level);
	}
out:
	up(&(g_rtdev->test_sem));
	return ret;
}

bool is_rt8555_used(void)
{
	return g_rt8555_used;
}

static ssize_t rt8555_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct i2c_client *client = NULL;
	struct rt8555_chip_data *pchip = NULL;

	if (!buf) {
		RT8555_ERR(g_rtdev->dev, "buf is null\n");
		return ret;
	}
	if (!dev) {
		ret =  snprintf(buf, PAGE_SIZE, "dev is null\n");
		return ret;
	}
	pchip = dev_get_drvdata(dev);
	if (!pchip) {
		ret = snprintf(buf, PAGE_SIZE, "data is null\n");
		return ret;
	}
	client = pchip->client;
	if (!client) {
		ret = snprintf(buf, PAGE_SIZE, "client is null\n");
		return ret;
	}
	ret = rt8555_config_read(pchip,
		g_reg_addr, g_bl_info.reg, RT8555_RW_REG_MAX);
	if (ret < 0) {
		RT8555_ERR(g_rtdev->dev, "rt8555 config read failed\n");
		goto i2c_error;
	}
	/* transfer rt8555 regiesters config information to Application layer */
	ret = snprintf(buf, PAGE_SIZE,
		"Control_mode_0x00= 0x%x\n"
		"Current_protection_0x01 = 0x%x\n"
		"\rCurrent_setting_0x02 = 0x%x\n"
		"Voltage_setting_0x03 = 0x%x\n"
		"\rBrightness_setting_0x08 = 0x%x\n"
		"Time_control_0x09 = 0x%x\n"
		"\rMode_devision_0x0A = 0x%x\n"
		"Compensation_duty_0x0B = 0x%x\n"
		"\rClk_pfm_enable_0x0D = 0x%x\n"
		"Led_protection_0x0E = 0x%x\n",
		g_bl_info.reg[0], g_bl_info.reg[1],
		g_bl_info.reg[2], g_bl_info.reg[3],
		g_bl_info.reg[4], g_bl_info.reg[5],
		g_bl_info.reg[6], g_bl_info.reg[7],
		g_bl_info.reg[8], g_bl_info.reg[9]);
	return ret;
i2c_error:
	ret = snprintf(buf, PAGE_SIZE, "%s: i2c access failed\n", __func__);
	return ret;
}

static ssize_t rt8555_reg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = -1;
	unsigned int val;
	unsigned int reg;
	unsigned int mask;
	struct rt8555_chip_data *pchip = NULL;

	if (!buf) {
		RT8555_ERR(g_rtdev->dev, "buf is null\n");
		return ret;
	}
	if (!dev) {
		RT8555_ERR(g_rtdev->dev, "dev is null\n");
		return ret;
	}
	pchip = dev_get_drvdata(dev);
	ret = sscanf(buf, "reg=0x%x, mask=0x%x, val=0x%x", &reg, &mask, &val);
	if (ret < 0) {
		RT8555_INFO(g_rtdev->dev, "check your input\n");
		goto out_input;
	}
	RT8555_INFO(g_rtdev->dev,
		"reg=0x%x,mask=0x%x,val=0x%x\n", reg, mask, val);
	ret = regmap_update_bits(pchip->regmap, reg, mask, val);
	if (ret < 0)
		goto i2c_error;
	return size;
i2c_error:
	dev_err(pchip->dev, "%s:i2c access fail to register\n", __func__);
	ret = snprintf((char *)buf, PAGE_SIZE, "%s: Access fail\n", __func__);
	return ret;
out_input:
	dev_err(pchip->dev, "%s:input conversion fail\n", __func__);
	ret = snprintf((char *)buf, PAGE_SIZE, "%s: input fail\n", __func__);
	return ret;
}

static DEVICE_ATTR(reg, 0644, rt8555_reg_show, rt8555_reg_store);

static struct attribute *g_rt8555_attributes[] = {
	&dev_attr_reg.attr,
	NULL,
};

static const struct attribute_group g_rt8555_group = {
	.attrs = g_rt8555_attributes,
};

static const struct regmap_config rt8555_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};


static int rt8555_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct device_node *np = NULL;
	struct rt8555_chip_data *pchip = NULL;
	struct i2c_adapter *adapter = client->adapter;

	RT8555_INFO(&client->dev, "in\n");
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c functionality check fail\n");
		return -EOPNOTSUPP;
	}
	pchip = devm_kzalloc(&client->dev,
		sizeof(struct rt8555_chip_data), GFP_KERNEL);
	if (!pchip)
		return -ENOMEM;
	pchip->regmap = devm_regmap_init_i2c(client, &rt8555_regmap);
	if (IS_ERR(pchip->regmap)) {
		ret = PTR_ERR(pchip->regmap);
		dev_err(&client->dev, "allocate regmap failed: %d\n", ret);
		goto err_out;
	}
	g_rtdev = pchip;
	pchip->client = client;
	i2c_set_clientdata(client, pchip);
	sema_init(&(pchip->test_sem), 1);
	g_rt_class = class_create(THIS_MODULE, "rt8555");
	if (IS_ERR(g_rt_class)) {
		RT8555_ERR(g_rtdev->dev,
			"Create rt8555 class failed; errno = %ld\n",
			PTR_ERR(g_rt_class));
		g_rt_class = NULL;
	}
	pchip->dev = device_create(g_rt_class, NULL, 0, "%s", client->name);
	if (IS_ERR(pchip->dev)) {
		/* Not fatal */
		RT8555_ERR(g_rtdev->dev,
			"Create device failed; errno = %ld\n",
			PTR_ERR(pchip->dev));
		pchip->dev = NULL;
	} else {
		dev_set_drvdata(pchip->dev, pchip);
		ret = sysfs_create_group(&pchip->dev->kobj, &g_rt8555_group);
		if (ret)
			goto err_sysfs;
	}
	memset(&g_bl_info, 0, sizeof(struct rt8555_info));
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_RT8555);
	if (!np) {
		RT8555_ERR(g_rtdev->dev,
			"NOT FOUND device node %s\n", DTS_COMP_RT8555);
		goto err_sysfs;
	}
	ret = rt8555_parse_dts(np);
	if (ret < 0) {
		RT8555_ERR(g_rtdev->dev, "parse rt8555 dts failed\n");
		goto err_sysfs;
	}
	return ret;
err_sysfs:
	RT8555_ERR(g_rtdev->dev, "sysfs error\n");
	device_destroy(g_rt_class, 0);
err_out:
	devm_kfree(&client->dev, pchip);
	return ret;
}

static int rt8555_remove(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &g_rt8555_group);
	return 0;
}

static const struct i2c_device_id rt8555_id[] = {
	{ RT8555_NAME, 0 },
	{},
};

static const struct of_device_id rt8555_of_id_table[] = {
	{ .compatible = "realtek,rt8555" },
	{},
};

MODULE_DEVICE_TABLE(i2c, rt8555_id);
static struct i2c_driver rt8555_i2c_driver = {
	.driver = {
		.name = "rt8555",
		.owner = THIS_MODULE,
		.of_match_table = rt8555_of_id_table,
	},
	.probe = rt8555_probe,
	.remove = rt8555_remove,
	.id_table = rt8555_id,
};

static int __init rt8555_module_init(void)
{
	return i2c_add_driver(&rt8555_i2c_driver);
}

late_initcall(rt8555_module_init);

MODULE_DESCRIPTION("HW rt8555 LED driver");
MODULE_LICENSE("GPL v2");
