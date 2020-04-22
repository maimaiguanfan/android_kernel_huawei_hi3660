/*
 * rt5112.c
 *
 * debug for pmic sensor
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
#include <linux/delay.h>
#include <linux/mutex.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/sensor/hw_comm_pmic.h>

#define HWLOG_TAG huawei_pmic
HWLOG_REGIST();

// pmic chip id
#define REVERSION_ID 0x2A

// LDO
#define LDO1_CTL  0x01
#define LDO2_CTL  0x02
#define LDO3_CTL  0x03
#define LDO4_CTL  0x04
#define LDO1_VOUT 0x0D // 0.6V-3.775V 25mV/step
#define LDO2_VOUT 0x0E // 0.6V-3.775V 25mV/step
#define LDO3_VOUT 0x0F // 0.6V-3.775V 25mV/step
#define LDO4_VOUT 0x10 // 0.6V-3.775V 25mV/step

// 600000 -> 3775000
#define RT5112_LDO_VOL_MIN 600000
#define RT5112_LDO_VOL_MAX 3775000

// step 25000 (0.025)
#define RT5112_LDO_VOL_STEP 25000

// Buck
#define BUCK_CTL   0x00
#define BUCK2_CTL  0x06
#define BUCK_VOUT  0x0C // 0.6V-3.3V 12.5mV/step
#define BUCK2_VOUT 0x12 // 0.6V-3.3V 12.5mV/step
#define RT5112_BUCK_VOL_MIN  600000
#define RT5112_BUCK_VOL_MAX  3300000
#define RT5112_BUCK_VOL_STEP 12500

// Boost
#define BOOST_CTL    0x05
#define BOOST_VOUT   0x11 //4.5V-5.5V 25mV/step
#define RT5112_BOOST_ENABLE    0x08
#define RT5112_BOOST_POWER_ON  0x00
#define RT5112_BOOST_POWER_OFF 0x01
#define RT5112_BOOST_VOL_MIN   4500000
#define RT5112_BOOST_VOL_MAX   5500000
#define RT5112_BOOST_VOL_STEP  25000

// OVP && OCP
#define RT5112_PMU_STATUS_REG 0x43
#define RT5112_UVP_STATUS_REG 0x15
#define RT5112_OCP_STATUS_REG 0x17
#define RT5112_OVP_STATUS_REG 0x16

// interrupt
#define RT5112_INT_STATUS_REG 0x14
#define RT5112_MASK_INTR_REG  0x33
#define RT5112_MASK_BOOST_5V  0x04
#define RT5112_MASK_UV_EVT    0x04
#define RT5112_MASK_FAULT_EVT 0x01
#define RT5112_MASK_OC_EVT    0x04

// PIN
#define RT5112_PIN_ENABLE    1
#define RT5112_PIN_DISABLE   0
#define RT5112_POWERON_MASK  0x80
#define RT5112_POWEROFF_MASK 0x7F

// BUCK_PWM_MODE
#define RT5112_BUCK_POWERON_MASK  0x81
#define RT5112_BUCK_POWEROFF_MASK 0x7E
#define RT5112_BUCK_PWM_MODE      1
#define RT5112_LDO2_CTRL          1

// define mutex for avoid competition
DEFINE_HW_PMIC_MUTEX(rt5112);

// rt5112 private pmic struct data
struct rt5112_pmic_data_t {
	unsigned int enable_pin;
	unsigned int intr_pin;
	unsigned int buck_pwm_mode;
};

struct voltage_map_t {
	int chx_enable;
	int vout_reg;
};

static struct voltage_map_t voltage_map[VOUT_MAX] = {
	{ LDO1_CTL, LDO1_VOUT },
	{ LDO2_CTL, LDO2_VOUT },
	{ LDO3_CTL, LDO3_VOUT },
	{ LDO4_CTL, LDO4_VOUT },
	{ 0, 0, }, // stub not support LDO5
	{ BUCK_CTL, BUCK_VOUT },
};

// static var
static struct rt5112_pmic_data_t rt5112_pdata;
static struct hw_pmic_i2c_client rt5112_i2c_client;

static u32 calc_voltage_level(u32 voltage, u32 max, u32 min, u32 step)
{
	if ((voltage > max) || (voltage < min) || (step == 0))
		return min;

	return (voltage - min) / step;
}

static int rt5122_check_null(struct hw_pmic_ctrl_t *pmic_ctrl)
{
	if (!pmic_ctrl || !pmic_ctrl->pmic_i2c_client || !pmic_ctrl->pdata) {
		hwlog_err("%s pmic_ctrl is NULL\n", __func__);
		return -EFAULT;
	}
	if (!pmic_ctrl->pmic_i2c_client->i2c_func_tbl ||
		!pmic_ctrl->pmic_i2c_client->i2c_func_tbl->i2c_read ||
		!pmic_ctrl->pmic_i2c_client->i2c_func_tbl->i2c_write) {
		hwlog_err("%s i2c read write func is NULL\n", __func__);
		return -EFAULT;
	}
	return 0;
}

static int rt5112_boost_seq_config(struct hw_pmic_ctrl_t *pmic_ctrl,
	enum pmic_seq_index seq_index, u32 voltage, int state)
{
	struct hw_pmic_i2c_client *i2c_client = NULL;
	struct hw_pmic_i2c_fn_t *i2c_func = NULL;
	u8 chx_enable = 0;
	int ret;

	hwlog_info("%s enter\n", __func__);
	if (seq_index != VOUT_BOOST) {
		hwlog_err("%s error, seq_index-%u\n", __func__, seq_index);
		return -EPERM;
	}

	if ((state != PMIC_POWER_OFF) && (state != PMIC_POWER_ON)) {
		hwlog_err("%s state %u error\n", __func__, state);
		return -EPERM;
	}

	i2c_client = pmic_ctrl->pmic_i2c_client;
	i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;

	ret = i2c_func->i2c_read(i2c_client, RT5112_BOOST_ENABLE, &chx_enable);
	if (ret < 0)
		return ret;

	if (state == PMIC_POWER_ON)
		chx_enable = chx_enable | RT5112_BOOST_POWER_ON;
	else
		chx_enable = chx_enable & RT5112_BOOST_POWER_OFF;

	ret = i2c_func->i2c_write(i2c_client, RT5112_BOOST_ENABLE, chx_enable);
	hwlog_info("%s, bst onoff-%d, reg-%u\n", __func__, state, chx_enable);

	return ret;
}

static int rt5112_buck_seq_config(struct hw_pmic_ctrl_t *pmic_ctrl,
	enum pmic_seq_index seq_index, u32 volt, int state)
{
	struct rt5112_pmic_data_t *pdata = NULL;
	struct hw_pmic_i2c_client *i2c_client = NULL;
	struct hw_pmic_i2c_fn_t *i2c_func = NULL;
	u8 chx_enable = 0;
	u8 chx_enable_reg;
	u8 volt_reg;
	u32 volt_level = 0;
	int ret;

	if (seq_index != VOUT_BUCK_1) {
		hwlog_err("%s index err, seq_index-%u\n", __func__, seq_index);
		return -EFAULT;
	}

	if ((volt > RT5112_BUCK_VOL_MAX) || (volt < RT5112_BUCK_VOL_MIN)) {
		hwlog_err("%s voltage error, vol-%u\n", __func__, volt);
		return -EFAULT;
	}

	if ((state != PMIC_POWER_OFF) && (state != PMIC_POWER_ON)) {
		hwlog_err("%s state-%u error\n", __func__, state);
		return -1;
	}

	pdata = pmic_ctrl->pdata;
	i2c_client = pmic_ctrl->pmic_i2c_client;
	i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;

	chx_enable_reg = voltage_map[seq_index].chx_enable;
	volt_reg = voltage_map[seq_index].vout_reg;

	ret = i2c_func->i2c_read(i2c_client, chx_enable_reg, &chx_enable);
	if (ret < 0)
		return ret;

	if (state == PMIC_POWER_ON) {
		volt_level = calc_voltage_level(volt,
			RT5112_BUCK_VOL_MAX,
			RT5112_BUCK_VOL_MIN,
			RT5112_BUCK_VOL_STEP);
		// set voltage
		ret = i2c_func->i2c_write(i2c_client, volt_reg, volt_level);
		if (ret < 0)
			return ret;

		if (pdata->buck_pwm_mode == RT5112_BUCK_PWM_MODE)
			chx_enable = chx_enable | RT5112_BUCK_POWERON_MASK;
		else
			chx_enable = chx_enable | RT5112_POWERON_MASK;
	} else {
		if (pdata->buck_pwm_mode == RT5112_BUCK_PWM_MODE)
			chx_enable = chx_enable & RT5112_BUCK_POWEROFF_MASK;
		else
			chx_enable = chx_enable & RT5112_POWEROFF_MASK;
	}
	ret = i2c_func->i2c_write(i2c_client, chx_enable_reg, chx_enable);
	hwlog_info("%s power state-%d, chx_reg-%u, reg-%u, vol-%u,vol_level-%u",
		__func__, state, chx_enable_reg, volt_reg, volt, volt_level);

	return ret;
}

// RT5112 ldo set
static int rt5112_ldo_seq_config(struct hw_pmic_ctrl_t *pmic_ctrl,
	enum pmic_seq_index seq_index, u32 voltage, int state)
{
	struct hw_pmic_i2c_client *i2c_client = NULL;
	struct hw_pmic_i2c_fn_t *i2c_func = NULL;
	u8 chx_enable = 0;
	u8 chx_enable_reg;
	u8 voltage_reg;
	u32 volt_level;
	int ret;

	if ((seq_index < VOUT_LDO_1) || (seq_index > VOUT_LDO_4)) {
		hwlog_err("%s seq_index err, dex-%u\n", __func__, seq_index);
		return -EFAULT;
	}

	if ((voltage > RT5112_LDO_VOL_MAX) || (voltage < RT5112_LDO_VOL_MIN)) {
		hwlog_err("%s voltage error, vol-%u\n", __func__, voltage);
		return -EFAULT;
	}

	if ((state != PMIC_POWER_OFF) && (state != PMIC_POWER_ON)) {
		hwlog_err("%s state -%u error", __func__, state);
		return -EFAULT;
	}

	i2c_client = pmic_ctrl->pmic_i2c_client;
	i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;

	chx_enable_reg = voltage_map[seq_index].chx_enable;
	voltage_reg = voltage_map[seq_index].vout_reg;

	ret = i2c_func->i2c_read(i2c_client, chx_enable_reg, &chx_enable);
	if (ret < 0)
		return ret;

	if (state == PMIC_POWER_ON) {
		// set voltage level
		volt_level = calc_voltage_level(voltage,
			RT5112_LDO_VOL_MAX,
			RT5112_LDO_VOL_MIN,
			RT5112_LDO_VOL_STEP);
		volt_level = volt_level << 1;
		ret = i2c_func->i2c_write(i2c_client, voltage_reg, volt_level);
		if (ret < 0)
			return ret;

		chx_enable = chx_enable | RT5112_POWERON_MASK;
	} else {
		chx_enable = chx_enable & RT5112_POWEROFF_MASK;
	}
	ret = i2c_func->i2c_write(i2c_client, chx_enable_reg, chx_enable);
	hwlog_info("set ldo-%d, enable-%d, voltage-%d\n",
		seq_index, state, voltage);
	return ret;
}

static int rt5112_seq_config(struct hw_pmic_ctrl_t *pmic_ctrl,
	enum pmic_seq_index index, u32 volt, int state)
{
	int ret;

	if (rt5122_check_null(pmic_ctrl) < 0)
		return -EFAULT;

	mutex_lock(&pmic_mut_rt5112);
	if (index == VOUT_BOOST)
		ret = rt5112_boost_seq_config(pmic_ctrl, index, volt, state);
	else if (index == VOUT_BOOST_EN)
		ret = hw_pmic_gpio_boost_enable(pmic_ctrl, state);
	else if (index < VOUT_LDO_5)
		ret = rt5112_ldo_seq_config(pmic_ctrl, index, volt, state);
	else
		ret = rt5112_buck_seq_config(pmic_ctrl, index, volt, state);
	hwlog_info("%s, set power-%d, volt-%d, state to %d, ret = %d\n",
		__func__, index, volt, state, ret);
	mutex_unlock(&pmic_mut_rt5112);
	return ret;
}

static int rt5112_match(struct hw_pmic_ctrl_t *pmic_ctrl)
{
	return 0;
}

static int rt5112_get_dt_data(struct hw_pmic_ctrl_t *pmic_ctrl)
{
	struct rt5112_pmic_data_t *pdata = NULL;
	struct device_node *dev_node = NULL;
	int rc;

	if (!pmic_ctrl || !pmic_ctrl->pdata || !pmic_ctrl->dev) {
		hwlog_err("%s pmic ctrl is NULL\n", __func__);
		return -EFAULT;
	}

	pdata = pmic_ctrl->pdata;
	dev_node = pmic_ctrl->dev->of_node;

	// get pmic Enable gpio
	rc = of_property_read_u32(dev_node, "hw,pmic-pin", &pdata->enable_pin);
	hwlog_info("%s get enable pmic-pin %d\n", __func__, pdata->enable_pin);
	if (rc < 0)
		hwlog_err("%s, failed %d\n", __func__, __LINE__);
	return rc;
}

static int rt5112_on(struct hw_pmic_ctrl_t *pmic_ctrl, void *data)
{
	// check Error registor
	return 0;
}

static int rt5112_off(struct hw_pmic_ctrl_t *pmic_ctrl)
{
	// Never power off pmic when SOC is running;
	return 0;
}

static void rt5112_clear_interrupt(struct hw_pmic_ctrl_t *pmic_ctrl)
{
	struct hw_pmic_i2c_client *i2c_client = NULL;
	struct hw_pmic_i2c_fn_t *i2c_func = NULL;
	u8 reg_value = 0;

	i2c_client = pmic_ctrl->pmic_i2c_client;
	i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;

	// clean status
	i2c_func->i2c_read(i2c_client, RT5112_PMU_STATUS_REG, &reg_value);
	i2c_func->i2c_read(i2c_client, RT5112_OVP_STATUS_REG, &reg_value);
	i2c_func->i2c_read(i2c_client, RT5112_OCP_STATUS_REG, &reg_value);
	i2c_func->i2c_read(i2c_client, RT5112_UVP_STATUS_REG, &reg_value);

	// clear interrupt
	i2c_func->i2c_read(i2c_client, RT5112_INT_STATUS_REG, &reg_value);
	// clear intr by write bit0 to 1
	reg_value = reg_value | 0x01;
	i2c_func->i2c_write(i2c_client, RT5112_INT_STATUS_REG, reg_value);
}

static int pmic_check_state_exception(struct hw_pmic_ctrl_t *pmic_ctrl)
{
	struct hw_pmic_i2c_client *i2c_client = NULL;
	struct hw_pmic_i2c_fn_t *i2c_func = NULL;
	struct rt5112_pmic_data_t *pdata = NULL;
	u8 pmu_status = 0;

	if (rt5122_check_null(pmic_ctrl) < 0)
		return -EFAULT;

	i2c_client = pmic_ctrl->pmic_i2c_client;
	i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;
	pdata = pmic_ctrl->pdata;

	// PMU_STATUS
	i2c_func->i2c_read(i2c_client, RT5112_PMU_STATUS_REG, &pmu_status);

	rt5112_clear_interrupt(pmic_ctrl);
	// reset rt5112_ENABLE need delay for 1000us
	gpio_set_value(pdata->enable_pin, RT5112_PIN_DISABLE);
	udelay(1000);
	gpio_set_value(pdata->enable_pin, RT5112_PIN_ENABLE);
	// mask boost interrupt
	i2c_func->i2c_write(i2c_client, RT5112_MASK_INTR_REG,
		RT5112_MASK_BOOST_5V);

	return 0;
}

static int rt5112_exit(struct hw_pmic_ctrl_t *pmic_ctrl)
{
	struct rt5112_pmic_data_t *pdata = NULL;

	if (!pmic_ctrl || !pmic_ctrl->pdata) {
		hwlog_err("%s pmic_ctrl is NULL.", __func__);
		return -EFAULT;
	}
	pdata = pmic_ctrl->pdata;
	gpio_free(pdata->enable_pin);
	hw_pmic_release_intr(pmic_ctrl);
	return 0;
}

static int rt5112_init(struct hw_pmic_ctrl_t *pmic_ctrl)
{
	struct hw_pmic_i2c_client *i2c_client = NULL;
	struct hw_pmic_i2c_fn_t *i2c_func = NULL;
	struct rt5112_pmic_data_t *pdata = NULL;
	u8 chip_id = 0;
	int ret;

	if (rt5122_check_null(pmic_ctrl) < 0)
		return -EFAULT;

	pdata = pmic_ctrl->pdata;
	ret = hw_pmic_setup_intr(pmic_ctrl);
	if (ret < 0)
		hwlog_err("%s setup interrupt failed", __func__);

	ret = gpio_request(pdata->enable_pin, "pmic-enable-ctrl");
	if (ret < 0) {
		hwlog_err("%s fail request enable pin = %d\n", __func__, ret);
		goto req_failed;
	}

	gpio_direction_output(pdata->enable_pin, RT5112_PIN_ENABLE);
	msleep(1);
	i2c_client = pmic_ctrl->pmic_i2c_client;
	i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;

	ret = i2c_func->i2c_read(i2c_client, REVERSION_ID, &chip_id);
	if (ret < 0) {
		hwlog_err("%s: read CHIP ID failed, ret = %d ", __func__, ret);
		goto id_fail;
	}
	hwlog_info("%s chip id = %d\n", __func__, chip_id);

	// mask boost interrupt
	i2c_func->i2c_write(i2c_client, RT5112_MASK_INTR_REG,
		RT5112_MASK_BOOST_5V);
	// clean interrupt
	rt5112_clear_interrupt(pmic_ctrl);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_CAMERA_PMIC);
#endif
	hwlog_info("%s success\n", __func__);
	return 0;

id_fail:
	gpio_free(pdata->enable_pin);

req_failed:
	hw_pmic_release_intr(pmic_ctrl);
	return ret;
}

static int rt5112_remove(struct i2c_client *client)
{
	hwlog_warn("%s enter\n", __func__);
	if (!client || !client->adapter)
		return 0;
	client->adapter = NULL;
	return 0;
}

static struct hw_pmic_fn_t rt5112_func_tbl = {
	.pmic_init = rt5112_init,
	.pmic_exit = rt5112_exit,
	.pmic_on = rt5112_on,
	.pmic_off = rt5112_off,
	.pmic_match = rt5112_match,
	.pmic_get_dt_data = rt5112_get_dt_data,
	.pmic_seq_config = rt5112_seq_config,
	.pmic_check_exception = pmic_check_state_exception,
};

static struct hw_pmic_ctrl_t rt5112_ctrl = {
	.pmic_i2c_client = &rt5112_i2c_client,
	.func_tbl = &rt5112_func_tbl,
	.hisi_pmic_mutex = &pmic_mut_rt5112,
	.pdata = (void *)&rt5112_pdata,
};

static const struct i2c_device_id rt5112_id[] = {
	{ "hw_rt5112", (unsigned long)&rt5112_ctrl },
	{}
};

static const struct of_device_id rt5112_dt_match[] = {
	{ .compatible = "hw,hw_rt5112" },
	{}
};
MODULE_DEVICE_TABLE(of, rt5112_dt_match);

static struct i2c_driver rt5112_i2c_driver = {
	.probe = hw_pmic_i2c_probe,
	.remove = rt5112_remove,
	.id_table = rt5112_id,
	.driver = {
		.name = "hw_rt5112",
		.of_match_table = rt5112_dt_match,
	},
};

static int __init rt5112_module_init(void)
{
	hwlog_info("%s enter\n", __func__);
	return i2c_add_driver(&rt5112_i2c_driver);
}

static void __exit rt5112_module_exit(void)
{
	hwlog_info("%s enter\n", __func__);
	i2c_del_driver(&rt5112_i2c_driver);
}

arch_initcall(rt5112_module_init);
module_exit(rt5112_module_exit);

MODULE_DESCRIPTION("RT5112 PMIC");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

