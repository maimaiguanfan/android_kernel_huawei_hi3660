/*
 * da9313.c
 *
 * vsys sc da9313 driver
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
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/raid/pq.h>
#include <linux/wakelock.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/vsys_switch/vsys_switch.h>
#include "da9313.h"

#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif

#ifdef CONFIG_HUAWEI_POWER_DEBUG
#include <huawei_platform/power/power_debug.h>
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG da9313
HWLOG_REGIST();

struct da9313_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct wake_lock wakelock;
	int chip_already_init;
	int get_id_time;
	int device_id;
	int gpio_nonkey_en;
	int gpio_pwron;
	int gpio_int;
	int irq_int;
	int irq_active;
	int state;
	int support_pwrdown_mode;
	bool is_pg;
};

static struct da9313_device_info *g_da9313_di;

#define MSG_LEN                      (2)

static int da9313_write_block(struct da9313_device_info *di,
	u8 reg, u8 *value, u8 len)
{
	struct i2c_msg msg[1];
	int ret = 0;
	int i = 0;

	if (di == NULL || value == NULL) {
		hwlog_err("di or value is null\n");
		return -EIO;
	}

	if (di->client->adapter == NULL) {
		hwlog_err("adapter is null\n");
		return -ENODEV;
	}

	if (di->chip_already_init == 0) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	*value = reg;

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = value;
	msg[0].len = len + 1;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(di->client->adapter, msg, 1);
		if (ret >= 0)
			break;
	}

	/* i2c_transfer returns number of messages transferred */
	if (ret != 1) {
		hwlog_err("write_block failed[%x]\n", reg);
		return -1;
	}

	return 0;
}

static int da9313_read_block(struct da9313_device_info *di,
	u8 reg, u8 *value, u8 len)
{
	struct i2c_msg msg[MSG_LEN];
	u8 buf = 0;
	int ret = 0;
	int i = 0;

	if (di == NULL || value == NULL) {
		hwlog_err("di or value is null\n");
		return -ENOMEM;
	}

	if (di->client->adapter == NULL) {
		hwlog_err("adapter is null\n");
		return -ENODEV;
	}

	if (!di->chip_already_init) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	buf = reg;

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = &buf;
	msg[0].len = DA9313_ADDR_LEN;

	msg[1].addr = di->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = value;
	msg[1].len = len;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(di->client->adapter, msg, MSG_LEN);
		if (ret >= 0)
			break;
	}

	/* i2c_transfer returns number of messages transferred */
	if (ret != MSG_LEN) {
		hwlog_err("read_block failed[%x]\n", reg);
		return -1;
	}

	return 0;
}

static int da9313_write_byte(u8 reg, u8 value)
{
	struct da9313_device_info *di = g_da9313_di;
	/* 2 bytes offset 1 contains the data offset 0 is used by i2c_write */
	u8 buff[DA9313_ADDR_LEN + BYTE_LEN] = {0};

	/* offset 1 contains the data */
	buff[DA9313_ADDR_LEN] = value;
	return da9313_write_block(di, reg, buff, BYTE_LEN);
}

static int da9313_read_byte(u8 reg, u8 *value)
{
	struct da9313_device_info *di = g_da9313_di;

	return da9313_read_block(di, reg, value, BYTE_LEN);
}

static int da9313_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret = 0;
	u8 val = 0;

	ret = da9313_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	ret = da9313_write_byte(reg, val);

	return ret;
}

#ifdef POWER_MODULE_DEBUG_FUNCTION
static int da9313_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret = 0;
	u8 val = 0;

	ret = da9313_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return 0;
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

static void da9313_wake_lock(void)
{
	struct da9313_device_info *di = g_da9313_di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return;
	}

	if (!wake_lock_active(&di->wakelock)) {
		wake_lock(&di->wakelock);
		hwlog_info("wake_lock\n");
	}
}

static void da9313_wake_unlock(void)
{
	struct da9313_device_info *di = g_da9313_di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return;
	}

	if (wake_lock_active(&di->wakelock)) {
		wake_unlock(&di->wakelock);
		hwlog_info("wake_unlock\n");
	}
}

#ifdef POWER_MODULE_DEBUG_FUNCTION
static void da9313_dump_register(void)
{
	u8 i = 0;
	int ret = 0;
	u8 val = 0;

	for (i = 0; i < DA9313_REG_TOTAL_NUM; i++) {
		ret = da9313_read_byte(i, &val);
		if (ret)
			hwlog_err("dump_register: read fail\n");

		hwlog_info("dump_register: reg[%x] = 0x%x\n", i, val);
	}
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

static int da9313_reset_cycle(void)
{
	int ret;
	u8 val;

	ret = da9313_write_mask(DA9313_REG_MODECTRL,
		DA9313_REG_RESET_CYCLE_MASK,
		DA9313_REG_RESET_CYCLE_SHIFT,
		DA9313_RESET_CYCLE_PWRDOWN);
	if (ret)
		return -1;

	ret = da9313_read_byte(DA9313_REG_MODECTRL, &val);
	if (ret)
		return -1;

	hwlog_info("reset_cycle reg[0x%x] = 0x%x\n", DA9313_REG_MODECTRL, val);

	return 0;
}

static int da9313_soft_reset(void)
{
	int ret;
	u8 val;

	ret = da9313_write_mask(DA9313_REG_MODECTRL,
		DA9313_REG_SOFTRESET_MASK,
		DA9313_REG_SOFTRESET_SHIFT,
		DA9313_SOFTRESET_ASSERTED);
	if (ret)
		return -1;

	ret = da9313_read_byte(DA9313_REG_MODECTRL, &val);
	if (ret)
		return -1;

	hwlog_info("soft_reset reg[0x%x] = 0x%x\n", DA9313_REG_MODECTRL, val);

	return 0;
}

static int da9313_set_test_regs(void)
{
	int ret;

	ret = da9313_write_byte(DA9313_REG_CTRL_ACCESS_TEST_REGS,
		DA9313_EN_ACCESS_TEST_REGS);
	if (ret)
		return -1;

	ret = da9313_write_byte(DA9313_REG_CTRL_PVC_SWITCHING,
		DA9313_STOP_PVC_SWITCHING);
	if (ret)
		return -1;

	ret = da9313_write_byte(DA9313_REG_CTRL_ACCESS_TEST_REGS,
		DA9313_DIS_ACCESS_TEST_REGS);
	if (ret)
		return -1;

	return ret;
}

static int da9313_get_device_id(int *id)
{
	u8 part_info = 0;
	int ret = 0;
	struct da9313_device_info *di = g_da9313_di;

	if (di == NULL || id == NULL) {
		hwlog_err("di or id is null\n");
		return -1;
	}

	if (di->get_id_time == DA9313_USED) {
		*id = di->device_id;
		return 0;
	}

	di->get_id_time = DA9313_USED;
	ret = da9313_read_byte(DA9313_REG_DEVICEID, &part_info);
	if (ret) {
		di->get_id_time = DA9313_NOT_USED;
		return -1;
	}

	hwlog_info("get_device_id [%x]=0x%x\n", DA9313_REG_DEVICEID, part_info);

	if (part_info == DA9313_DEVICEID_INFO) {
		di->device_id = VSYS_SC_DA9313;
	} else {
		di->device_id = -1;
		ret = -1;
		hwlog_err("get device id ERR\n");
	}

	*id = di->device_id;
	return ret;
}

static int da9313_get_state(void)
{
	struct da9313_device_info *di = g_da9313_di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	return di->state;
}

static int da9313_get_pg_state(void)
{
	int val;
	struct da9313_device_info *di = g_da9313_di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (!di->gpio_int) {
		hwlog_err("no device\n");
		return -ENODEV;
	}

	val = gpio_get_value(di->gpio_int);
	hwlog_info("get_pg_state: gpio_int val = %d\n", val);

	return val;
}

static int da9313_set_state(int enable)
{
	int ret = 0;
	struct da9313_device_info *di = g_da9313_di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (!di->gpio_pwron || (!di->gpio_nonkey_en &&
		di->support_pwrdown_mode)) {
		hwlog_err("set_state: no device\n");
		return -ENODEV;
	}

	enable = !!enable;

	if (di->state == enable) {
		hwlog_info("set_state already %d\n", enable);
		return 0;
	}

	di->state = enable;

	if (enable && di->support_pwrdown_mode) {
		gpio_set_value(di->gpio_nonkey_en, enable);
		hwlog_info("set_state set gpio_nonkey_en %d\n", enable);

		/* high level lasts 20ms, from power down to active mode */
		usleep_range(20000, 21000);

		gpio_set_value(di->gpio_nonkey_en, !enable);
		hwlog_info("set_state set gpio_nonkey_en %d\n", !enable);

		/* wait for 5ms */
		usleep_range(5000, 6000);
	}

	gpio_set_value(di->gpio_pwron, enable);
	hwlog_info("set_state set gpio_pwron %d\n", enable);

	if (!enable)
		di->is_pg = false;

	if (!enable && di->support_pwrdown_mode) {
		ret = da9313_reset_cycle();
		ret |= da9313_soft_reset();
		if (ret)
			return VSYS_SC_SET_MODE_CTRL_REG_FAIL;
	}

	di->chip_already_init = enable ? 1 : 0;
	hwlog_info("set_state set gpio_pwron %d\n", enable);

	if (enable) {
		ret = da9313_set_test_regs();
		if (ret)
			return VSYS_SC_SET_TEST_REGS_FAIL;

		/* wait for 100ms to read power good signal */
		usleep_range(100000, 101000);

		if (da9313_get_pg_state() == DA9313_PG_ABNOR) {
			hwlog_err("set_state: sc is not power good\n");
			return VSYS_SC_NOT_PG;
		}
		di->is_pg = true;
	}

	return ret;
}

static void da9313_irq_work(struct work_struct *work)
{
	struct da9313_device_info *di = container_of(work,
		struct da9313_device_info, irq_work);

	if (di == NULL) {
		hwlog_err("di is null\n");
		da9313_wake_unlock();
		return;
	}

	atomic_notifier_call_chain(&vsys_sc_event_nh,
		VSYS_SC_INT_NOT_PG, NULL);

	if (di->irq_active == 0) {
		di->irq_active = 1;
		enable_irq(di->irq_int);
	}

	da9313_wake_unlock();
}

static irqreturn_t da9313_interrupt(int irq, void *_di)
{
	struct da9313_device_info *di = _di;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return -1;
	}

	da9313_wake_lock();

	hwlog_info("da9313 int happened (%d)\n", di->irq_active);

	if (!di->is_pg || !di->chip_already_init ||
		di->state == DA9313_CHIP_DISABLE) {
		hwlog_err("ignore interrupt\n");
		da9313_wake_unlock();
		return IRQ_HANDLED;
	}

	if (di->irq_active == 1) {
		di->irq_active = 0;
		disable_irq_nosync(di->irq_int);
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("irq is not enable, do nothing\n");
		da9313_wake_unlock();
	}

	return IRQ_HANDLED;
}

static int da9313_reg_init(struct da9313_device_info *di)
{
	int ret = 0;

	if (di->support_pwrdown_mode) {
		di->chip_already_init = 1;
		ret = da9313_reset_cycle();
		ret |= da9313_soft_reset();
		if (ret)
			hwlog_err("reg init failed\n");
		di->chip_already_init = 0;
	}

	return ret;
}

static int da9313_gpio_init(struct da9313_device_info *di,
	struct device_node *np)
{
	int ret;

	if (di->support_pwrdown_mode) {
		di->gpio_nonkey_en = of_get_named_gpio(np, "gpio_nonkey_en", 0);
		hwlog_info("gpio_nonkey_en=%d\n", di->gpio_nonkey_en);

		if (!gpio_is_valid(di->gpio_nonkey_en)) {
			hwlog_err("gpio(gpio_nonkey_en) is not valid\n");
			ret = -EINVAL;
			goto gpio_init_fail_0;
		}

		ret = gpio_request(di->gpio_nonkey_en, "gpio_nonkey_en");
		if (ret) {
			hwlog_err("gpio(gpio_nonkey_en) request fail\n");
			goto gpio_init_fail_0;
		}

		/* 0: power down mode */
		ret = gpio_direction_output(di->gpio_nonkey_en, 0);
		if (ret) {
			hwlog_err("gpio(gpio_nonkey_en) set output fail\n");
			goto gpio_init_fail_1;
		}
	}

	di->gpio_pwron = of_get_named_gpio(np, "gpio_pwron", 0);
	hwlog_info("gpio_pwron=%d\n", di->gpio_pwron);

	if (!gpio_is_valid(di->gpio_pwron)) {
		hwlog_err("gpio(gpio_pwron) is not valid\n");
		ret = -EINVAL;
		goto gpio_init_fail_1;
	}

	ret = gpio_request(di->gpio_pwron, "gpio_pwron");
	if (ret) {
		hwlog_err("gpio(gpio_pwron) request fail\n");
		goto gpio_init_fail_1;
	}

	/* 1:enable 0:disable */
	ret = gpio_direction_output(di->gpio_pwron, DA9313_CHIP_DISABLE);
	if (ret) {
		hwlog_err("gpio(gpio_pwron) set output fail\n");
		goto gpio_init_fail_2;
	}

	return 0;

gpio_init_fail_2:
	gpio_free(di->gpio_pwron);
gpio_init_fail_1:
	if (di->support_pwrdown_mode)
		gpio_free(di->gpio_nonkey_en);
gpio_init_fail_0:
	return ret;
}

static int da9313_irq_init(struct da9313_device_info *di,
	struct device_node *np)
{
	int ret;

	di->gpio_int = of_get_named_gpio(np, "gpio_irq", 0);
	hwlog_info("gpio_int=%d\n", di->gpio_int);

	if (!gpio_is_valid(di->gpio_int)) {
		hwlog_err("gpio(gpio_int) is not valid\n");
		ret = -EINVAL;
		goto irq_init_fail_0;
	}

	ret = gpio_request(di->gpio_int, "gpio_irq");
	if (ret) {
		hwlog_err("gpio(gpio_int) request fail\n");
		goto irq_init_fail_0;
	}

	ret = gpio_direction_input(di->gpio_int);
	if (ret) {
		hwlog_err("gpio(gpio_int) set input fail\n");
		goto irq_init_fail_1;
	}

	di->irq_int = gpio_to_irq(di->gpio_int);
	if (di->irq_int < 0) {
		hwlog_err("gpio(gpio_int) map to irq fail\n");
		ret = -1;
		goto irq_init_fail_1;
	}

	ret = request_irq(di->irq_int, da9313_interrupt, IRQF_TRIGGER_FALLING,
		"da9313_irq", di);
	if (ret) {
		hwlog_err("gpio(gpio_int) irq request fail\n");
		di->irq_int = -1;
		goto irq_init_fail_1;
	}

	enable_irq_wake(di->irq_int);
	di->irq_active = 1;
	INIT_WORK(&di->irq_work, da9313_irq_work);
	return 0;

irq_init_fail_1:
	gpio_free(di->gpio_int);
irq_init_fail_0:
	return ret;
}

static void da9313_para_init(struct da9313_device_info *di)
{
	di->chip_already_init = 0;
	di->state = DA9313_CHIP_DISABLE;
	di->get_id_time = DA9313_NOT_USED;
	di->is_pg = false;
}

static struct vsys_sc_device_ops  da9313_ops = {
	.chip_name = "da9313",

	.get_state = da9313_get_state,
	.set_state = da9313_set_state,
	.get_id = da9313_get_device_id,
};

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static ssize_t da9313_dbg_show_reg_value(void *dev_data,
	char *buf, size_t size)
{
	u8 val = 0;
	int ret = 0;
	int i = 0;
	char rd_buf[DA9313_RD_BUF_SIZE] = {0};
	struct da9313_device_info *dev_p;

	dev_p = (struct da9313_device_info *)dev_data;
	if (dev_p == NULL) {
		hwlog_err("dev_p is null\n");
		return scnprintf(buf, size, "dev_p is null\n");
	}

	for (i = 0; i < DA9313_REG_TOTAL_NUM; i++) {
		val = 0;
		memset(rd_buf, 0, DA9313_RD_BUF_SIZE);
		ret = da9313_read_byte(i, &val);
		if (ret == 0)
			scnprintf(rd_buf, DA9313_RD_BUF_SIZE,
				"regaddr: 0x%x regval: 0x%x\n", i, val);
		else
			scnprintf(rd_buf, DA9313_RD_BUF_SIZE,
				"regaddr: 0x%x regval: invalid\n", i);

		strncat(buf, rd_buf, strlen(rd_buf));
	}

	return strlen(buf);
}

static ssize_t da9313_dbg_store_reg_value(void *dev_data,
	const char *buf, size_t size)
{
	int regaddr = 0;
	int regval = 0;
	int ret = 0;
	struct da9313_device_info *dev_p;

	dev_p = (struct da9313_device_info *)dev_data;
	if (dev_p == NULL) {
		hwlog_err("dev_p is null\n");
		return -EINVAL;
	}

	if (sscanf(buf, "%d %d", &regaddr, &regval) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	/* maximum value of 8-bit num is 255 */
	if (regaddr < 0 || regaddr >= DA9313_REG_TOTAL_NUM
		|| regval < 0 || regval > 255) {
		hwlog_err("regaddr 0x%x or regval 0x%x invalid\n",
			regaddr, regval);
		return -EINVAL;
	}

	ret = da9313_write_byte((u8)regaddr, (u8)regval);
	if (ret)
		return -EINVAL;

	hwlog_info("regaddr 0x%x regval 0x%x succeed\n", regaddr, regval);

	return size;
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static int da9313_parse_dts(struct device_node *np,
	struct da9313_device_info *di)
{
	int ret;

	ret = of_property_read_u32(np, "support_pwrdown_mode",
		&di->support_pwrdown_mode);
	if (ret) {
		hwlog_err("support_pwrdown_mode dts read failed\n");
		di->support_pwrdown_mode = 1;
	}

	hwlog_info("support_pwrdown_mode=%d\n",
		di->support_pwrdown_mode);

	return ret;
}

static int da9313_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret = 0;
	struct da9313_device_info *di = NULL;
	struct device_node *np = NULL;

	hwlog_info("probe begin\n");

	if (client == NULL || id == NULL) {
		hwlog_err("client or id is null\n");
		return -ENOMEM;
	}

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (di == NULL)
		return -ENOMEM;

	g_da9313_di = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	da9313_parse_dts(np, di);

	ret = da9313_gpio_init(di, np);
	if (ret)
		goto da9313_fail_0;

	ret = da9313_irq_init(di, np);
	if (ret)
		goto da9313_fail_1;

	da9313_reg_init(di);
	da9313_para_init(di);
	wake_lock_init(&di->wakelock, WAKE_LOCK_SUSPEND, "da9313_wakelock");

	ret = vsys_sc_ops_register(&da9313_ops);
	if (ret) {
		hwlog_err("register da9313 ops failed\n");
		goto da9313_fail_2;
	}

#ifdef CONFIG_HUAWEI_POWER_DEBUG
	power_dbg_ops_register("da9313_regval", i2c_get_clientdata(client),
		(power_dgb_show)da9313_dbg_show_reg_value,
		(power_dgb_store)da9313_dbg_store_reg_value);
#endif

	hwlog_info("probe end\n");
	return 0;

da9313_fail_2:
	wake_lock_destroy(&di->wakelock);
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_int);
da9313_fail_1:
	if (di->support_pwrdown_mode)
		gpio_free(di->gpio_nonkey_en);
	gpio_free(di->gpio_pwron);
da9313_fail_0:
	devm_kfree(&client->dev, di);
	g_da9313_di = NULL;
	return ret;
}

static int da9313_remove(struct i2c_client *client)
{
	struct da9313_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	if (di == NULL)
		return -1;

	/* reset da9313 */
	da9313_set_state(DA9313_CHIP_DISABLE);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	if (di->gpio_pwron)
		gpio_free(di->gpio_pwron);

	if (di->support_pwrdown_mode) {
		if (di->gpio_nonkey_en)
			gpio_free(di->gpio_nonkey_en);
	}

	wake_lock_destroy(&di->wakelock);
	g_da9313_di = NULL;

	hwlog_info("remove end\n");
	return 0;
}

MODULE_DEVICE_TABLE(i2c, da9313);

static const struct of_device_id da9313_of_match[] = {
	{
		.compatible = "huawei,da9313",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id da9313_i2c_id[] = {
	{"da9313", 0}, {}
};

static struct i2c_driver da9313_driver = {
	.probe = da9313_probe,
	.remove = da9313_remove,
	.id_table = da9313_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "huawei,da9313",
		.of_match_table = of_match_ptr(da9313_of_match),
	},
};

static int __init da9313_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&da9313_driver);
	if (ret)
		hwlog_err("i2c_add_driver error\n");

	return ret;
}

static void __exit da9313_exit(void)
{
	i2c_del_driver(&da9313_driver);
}

module_init(da9313_init);
module_exit(da9313_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("da9313 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
