/*
 * dual_charger.c
 *
 * dual charger driver
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
#include <huawei_platform/power/huawei_charger.h>
#include <dual_charger.h>
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif

#define HWLOG_TAG dual_charger
HWLOG_REGIST();

static struct dual_charger_info *dci;
static struct charge_device_ops *g_main_ops;
static struct charge_device_ops *g_aux_ops;

int charge_main_ops_register(struct charge_device_ops *ops)
{
	int ret = 0;

	if (ops)
		g_main_ops = ops;
	else
		ret = -EPERM;

	return ret;
}

int charge_aux_ops_register(struct charge_device_ops *ops)
{
	int ret = 0;

	if (ops)
		g_aux_ops = ops;
	else
		ret = -EPERM;

	return ret;
}

static int dual_charger_chip_init(struct chip_init_crit *init_crit)
{
	int ret = 0;

	if (!init_crit) {
		hwlog_err("init_crit is null\n");
		return -ENOMEM;
	}

	if (g_main_ops && g_main_ops->chip_init)
		ret |= g_main_ops->chip_init(init_crit);

	if (g_aux_ops && g_aux_ops->chip_init)
		ret |= g_aux_ops->chip_init(init_crit);

	return ret;
}

static int dual_charger_device_check(void)
{
	int ret1 = CHARGE_IC_BAD;
	int ret2 = CHARGE_IC_BAD;

	if (g_main_ops && g_main_ops->dev_check)
		ret1 = g_main_ops->dev_check();

	if (g_aux_ops && g_aux_ops->dev_check)
		ret2 = g_aux_ops->dev_check();

	if ((ret1 == CHARGE_IC_GOOD) && (ret2 == CHARGE_IC_GOOD)) {
		hwlog_info("dual_charger is good\n");
		return CHARGE_IC_GOOD;
	}

	hwlog_err("dual_charger is bad\n");
	return CHARGE_IC_BAD;
}

static int dual_charger_set_adc_conv_rate(int mode)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->set_adc_conv_rate)
		ret |= g_main_ops->set_adc_conv_rate(mode);

	if (g_aux_ops && g_aux_ops->set_adc_conv_rate)
		ret |= g_aux_ops->set_adc_conv_rate(mode);

	return ret;
}

static void dual_charger_set_input_current_thermal(int main, int aux)
{
	if (!dci) {
		hwlog_err("dci is null\n");
		return;
	}

	dci->iin_thermal_main = main;
	dci->iin_thermal_aux = aux;
}

static void dual_charger_set_charge_current_thermal(int main, int aux)
{
	if (!dci) {
		hwlog_err("dci is null\n");
		return;
	}

	dci->ichg_thermal_main = main;
	dci->ichg_thermal_aux = aux;
}

static void dual_charger_select_input_current_by_thermal_limit(int iin)
{
	if (!dci) {
		hwlog_err("dci is null\n");
		return;
	}

	if ((dci->iin_max_each_charger <= dci->iin_thermal_main &&
		dci->iin_max_each_charger <= dci->iin_thermal_aux) ||
		(iin <= dci->iin_thermal_main && iin <= dci->iin_thermal_aux)) {
		dci->input_current_main = iin / 2;
		dci->input_current_aux = iin / 2;
	} else {
		if (dci->iin_thermal_main < dci->iin_thermal_aux) {
			dci->input_current_main = dci->iin_thermal_main;
			dci->input_current_aux = iin - dci->iin_thermal_main;
		} else {
			dci->input_current_main = iin - dci->iin_thermal_aux;
			dci->input_current_aux = dci->iin_thermal_aux;
		}
	}
}

static void dual_charger_select_charge_current_by_thermal_limit(int ichg)
{
	if (!dci) {
		hwlog_err("dci is null\n");
		return;
	}

	if (ichg <= dci->ichg_thermal_main && ichg <= dci->ichg_thermal_aux) {
		dci->charge_current_main = ichg / 2;
		dci->charge_current_aux = ichg / 2;
	} else {
		if (dci->ichg_thermal_main < dci->ichg_thermal_aux) {
			dci->charge_current_main = dci->ichg_thermal_main;
			dci->charge_current_aux = ichg - dci->ichg_thermal_main;
		} else {
			dci->charge_current_main = ichg - dci->ichg_thermal_aux;
			dci->charge_current_aux = dci->ichg_thermal_aux;
		}
	}
}

static void dual_charger_select_input_current(int iin)
{
	if (!dci) {
		hwlog_err("dci is null\n");
		return;
	}

	if (dci->charge_done == TRUE) {
		dci->input_current_main = iin;
		dci->input_current_aux = 0;
		return;
	}

	/* don't need aux charger */
	if (dci->ichg < dci->ichg_need_aux_charger) {
		dci->input_current_main = iin;
		dci->input_current_aux = 0;
	} else {
		dual_charger_select_input_current_by_thermal_limit(iin);
	}

	dci->input_current_main =
		dci->input_current_main < dci->iin_max_each_charger ?
		dci->input_current_main : dci->iin_max_each_charger;
	dci->input_current_aux =
		dci->input_current_aux < dci->iin_max_each_charger ?
		dci->input_current_aux : dci->iin_max_each_charger;
}

static void dual_charger_select_charge_current(int ichg)
{
	if (!dci) {
		hwlog_err("dci is null\n");
		return;
	}

	/* don't need aux charger */
	if (dci->ichg < dci->ichg_need_aux_charger) {
		dci->charge_current_main = ichg;
		dci->charge_current_aux = 0;
	} else {
		dual_charger_select_charge_current_by_thermal_limit(ichg);
	}

	if (dci->charge_done == TRUE)
		dci->charge_current_aux = 0;
}

static int dual_charger_set_input_current(int iin)
{
	int ret = 0;

	if (!dci) {
		hwlog_err("dci is null\n");
		return -1;
	}

	hwlog_debug("set input current %d\n", iin);

	dual_charger_select_input_current(iin);

	if (g_main_ops && g_main_ops->set_input_current)
		ret |= g_main_ops->set_input_current(dci->input_current_main);

	if (g_aux_ops && g_aux_ops->set_input_current)
		ret |= g_aux_ops->set_input_current(dci->input_current_aux);

	return ret;
}

static int dual_charger_set_charge_current(int ichg)
{
	int ret = 0;

	if (!dci) {
		hwlog_err("dci is null\n");
		return -1;
	}

	hwlog_debug("set charge current %d\n", ichg);

	dci->ichg = ichg;
	dual_charger_select_charge_current(ichg);

	if (dci->charge_current_main == CHARGE_CURRENT_0000_MA)
		dci->charge_enable_main = FALSE;

	if (dci->charge_current_aux == CHARGE_CURRENT_0000_MA)
		dci->charge_enable_aux = FALSE;

	if (g_main_ops && g_main_ops->set_charge_current)
		ret |= g_main_ops->set_charge_current(dci->charge_current_main);

	if (g_aux_ops && g_aux_ops->set_charge_current)
		ret |= g_aux_ops->set_charge_current(dci->charge_current_aux);

	return ret;
}

static int dual_charger_get_ichg_reg(int flag)
{
	if (flag == MAIN_CHARGER)
		return bq25892_main_get_ichg_reg();
	else if (flag == AUX_CHARGER)
		return bq25892_aux_get_ichg_reg();

	hwlog_err("get_ichg_reg para error");
	return 0;
}

static int dual_charger_get_ichg_adc(int flag)
{
	if (flag == MAIN_CHARGER)
		return bq25892_main_get_ichg_adc();
	else if (flag == AUX_CHARGER)
		return bq25892_aux_get_ichg_adc();

	hwlog_err("get_ichg_adc para error");
	return 0;
}

static int dual_charger_set_terminal_voltage(int value)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->set_terminal_voltage)
		ret |= g_main_ops->set_terminal_voltage(value);

	if (g_aux_ops && g_aux_ops->set_terminal_voltage)
		ret |= g_aux_ops->set_terminal_voltage(value);

	return ret;
}

static int dual_charger_set_dpm_voltage(int value)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->set_dpm_voltage)
		ret |= g_main_ops->set_dpm_voltage(value);

	if (g_aux_ops && g_aux_ops->set_dpm_voltage)
		ret |= g_aux_ops->set_dpm_voltage(value);

	return ret;
}

static int dual_charger_set_terminal_current(int value)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->set_terminal_current)
		ret |= g_main_ops->set_terminal_current(value);

	if (g_aux_ops && g_aux_ops->set_terminal_current)
		ret |= g_aux_ops->set_terminal_current(value);

	return ret;
}

static int dual_charger_set_charge_enable(int enable)
{
	int ret = 0;

	if (!dci) {
		hwlog_err("dci is null\n");
		return -1;
	}

	dci->charge_enable_main = enable;

	if (dci->ichg >= dci->ichg_need_aux_charger)
		dci->charge_enable_aux = enable;
	else
		dci->charge_enable_aux = FALSE;

	if (g_main_ops && g_main_ops->set_charge_enable)
		ret |= g_main_ops->set_charge_enable(
				dci->charge_enable_main &
				dci->charge_enable_sysfs_main);

	if (g_aux_ops && g_aux_ops->set_charge_enable)
		ret |= g_aux_ops->set_charge_enable(
				dci->charge_enable_aux &
				dci->charge_enable_sysfs_aux);

	return ret;
}

static int dual_charger_set_otg_enable(int enable)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->set_otg_enable)
		ret |= g_main_ops->set_otg_enable(enable);

	if (g_aux_ops && g_aux_ops->set_otg_enable)
		ret |= g_aux_ops->set_otg_enable(FALSE);

	return ret;
}

static int dual_charger_set_term_enable(int enable)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->set_term_enable)
		ret |= g_main_ops->set_term_enable(enable);

	if (g_aux_ops && g_aux_ops->set_term_enable)
		ret |= g_aux_ops->set_term_enable(enable);

	return ret;
}

static int dual_charger_get_charge_state(unsigned int *state)
{
	int ret = 0;

	if (!dci) {
		hwlog_err("dci is null\n");
		return -1;
	}

	*state &= ~CHAGRE_STATE_CHRG_DONE;

	if (g_main_ops && g_main_ops->get_charge_state)
		ret |= g_main_ops->get_charge_state(state);

	if (g_aux_ops && g_aux_ops->get_charge_state)
		ret |= g_aux_ops->get_charge_state(state);

	if (*state & CHAGRE_STATE_CHRG_DONE)
		dci->charge_done = TRUE;
	else
		dci->charge_done = FALSE;

	return ret;
}

static int dual_charger_reset_watchdog_timer(void)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->reset_watchdog_timer)
		ret |= g_main_ops->reset_watchdog_timer();

	if (g_aux_ops && g_aux_ops->reset_watchdog_timer)
		ret |= g_aux_ops->reset_watchdog_timer();

	return ret;
}

static int dual_charger_dump_register(char *reg_value)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->dump_register)
		ret |= g_main_ops->dump_register(reg_value);

	if (g_aux_ops && g_aux_ops->dump_register)
		ret |= g_aux_ops->dump_register(reg_value);

	return ret;
}

static int dual_charger_get_register_head(char *reg_head)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->get_register_head)
		ret |= g_main_ops->get_register_head(reg_head);

	if (g_aux_ops && g_aux_ops->get_register_head)
		ret |= g_aux_ops->get_register_head(reg_head);

	return ret;
}

static int dual_charger_set_watchdog_timer(int value)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->set_watchdog_timer)
		ret |= g_main_ops->set_watchdog_timer(value);

	if (g_aux_ops && g_aux_ops->set_watchdog_timer)
		ret |= g_aux_ops->set_watchdog_timer(value);

	return ret;
}

static int dual_charger_set_batfet_disable(int disable)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->set_batfet_disable)
		ret |= g_main_ops->set_batfet_disable(disable);

	if (g_aux_ops && g_aux_ops->set_batfet_disable)
		ret |= g_aux_ops->set_batfet_disable(disable);

	return ret;
}

static int dual_charger_get_ilim(void)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->get_ibus)
		ret += g_main_ops->get_ibus();

	if (g_aux_ops && g_aux_ops->get_ibus)
		ret += g_aux_ops->get_ibus();

	return ret;
}

static int dual_charger_get_vbus_mv(unsigned int *vbus_mv)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->get_vbus)
		ret = g_main_ops->get_vbus(vbus_mv);

	return ret;
}

static int dual_charger_get_vbat_sys(void)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->get_vbat_sys)
		ret = g_main_ops->get_vbat_sys();

	return ret;
}

static int dual_charger_set_covn_start(int enable)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->set_covn_start)
		ret |= g_main_ops->set_covn_start(enable);

	if (g_aux_ops && g_aux_ops->set_covn_start)
		ret |= g_aux_ops->set_covn_start(enable);

	return ret;
}

static int dual_charger_set_charger_hiz(int enable)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->set_charger_hiz)
		ret |= g_main_ops->set_charger_hiz(enable);

	if (g_aux_ops && g_aux_ops->set_charger_hiz)
		ret |= g_aux_ops->set_charger_hiz(enable);

	return ret;
}

static int dual_charger_check_input_dpm_state(void)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->check_input_dpm_state)
		ret |= g_main_ops->check_input_dpm_state();

	if (g_aux_ops && g_aux_ops->check_input_dpm_state)
		ret |= g_aux_ops->check_input_dpm_state();

	return ret;
}

static int dual_charger_set_otg_current(int value)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->set_otg_current)
		ret = g_main_ops->set_otg_current(value);

	return ret;
}

static int dual_charger_stop_charge_config(void)
{
	int ret = 0;

	if (!dci) {
		hwlog_err("dci is null\n");
		return -1;
	}

	dci->ichg = dci->ichg_max;
	dci->charge_enable_sysfs_main = TRUE;
	dci->charge_enable_sysfs_aux = TRUE;

	if (g_main_ops && g_main_ops->stop_charge_config)
		ret |= g_main_ops->stop_charge_config();

	if (g_aux_ops && g_aux_ops->stop_charge_config)
		ret |= g_aux_ops->stop_charge_config();

	return ret;
}

struct charge_device_ops dual_charger_ops = {
	.chip_init = dual_charger_chip_init,
	.dev_check = dual_charger_device_check,
	.set_adc_conv_rate = dual_charger_set_adc_conv_rate,
	.set_input_current = dual_charger_set_input_current,
	.set_charge_current = dual_charger_set_charge_current,
	.set_input_current_thermal = dual_charger_set_input_current_thermal,
	.set_charge_current_thermal = dual_charger_set_charge_current_thermal,
	.get_ichg_reg = dual_charger_get_ichg_reg,
	.get_ichg_adc = dual_charger_get_ichg_adc,
	.set_terminal_voltage = dual_charger_set_terminal_voltage,
	.set_dpm_voltage = dual_charger_set_dpm_voltage,
	.set_terminal_current = dual_charger_set_terminal_current,
	.set_charge_enable = dual_charger_set_charge_enable,
	.set_otg_enable = dual_charger_set_otg_enable,
	.set_term_enable = dual_charger_set_term_enable,
	.get_charge_state = dual_charger_get_charge_state,
	.reset_watchdog_timer = dual_charger_reset_watchdog_timer,
	.dump_register = dual_charger_dump_register,
	.get_register_head = dual_charger_get_register_head,
	.set_watchdog_timer = dual_charger_set_watchdog_timer,
	.set_batfet_disable = dual_charger_set_batfet_disable,
	.get_ibus = dual_charger_get_ilim,
	.get_vbus = dual_charger_get_vbus_mv,
	.get_vbat_sys = dual_charger_get_vbat_sys,
	.set_covn_start = dual_charger_set_covn_start,
	.set_charger_hiz = dual_charger_set_charger_hiz,
	.check_input_dpm_state = dual_charger_check_input_dpm_state,
	.set_otg_current = dual_charger_set_otg_current,
	.stop_charge_config = dual_charger_stop_charge_config,
};

#ifdef CONFIG_SYSFS
#define DUAL_CHARGER_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, dual_charger_sysfs_show, store), \
	.name = DUAL_CHARGER_SYSFS_##n, \
}

#define DUAL_CHARGER_SYSFS_FIELD_RW(_name, n) \
	DUAL_CHARGER_SYSFS_FIELD(_name, n, 0644, dual_charger_sysfs_store)

#define DUAL_CHARGER_SYSFS_FIELD_RO(_name, n) \
	DUAL_CHARGER_SYSFS_FIELD(_name, n, 0444, NULL)

static ssize_t dual_charger_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t dual_charger_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

struct dual_charger_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static struct dual_charger_sysfs_field_info dual_charger_sysfs_field_tbl[] = {
	DUAL_CHARGER_SYSFS_FIELD_RW(enable_charger_main, ENABLE_CHARGER_MAIN),
	DUAL_CHARGER_SYSFS_FIELD_RW(enable_charger_aux, ENABLE_CHARGER_AUX),
};

static struct attribute *dual_charger_sysfs_attrs[
	ARRAY_SIZE(dual_charger_sysfs_field_tbl) + 1];

static const struct attribute_group dual_charger_sysfs_attr_group = {
	.attrs = dual_charger_sysfs_attrs,
};

static void dual_charger_sysfs_init_attrs(void)
{
	int i, limit = ARRAY_SIZE(dual_charger_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		dual_charger_sysfs_attrs[i] =
			&dual_charger_sysfs_field_tbl[i].attr.attr;

	dual_charger_sysfs_attrs[limit] = NULL;
}

static struct dual_charger_sysfs_field_info *dual_charger_sysfs_field_lookup(
	const char *name)
{
	int i, limit = ARRAY_SIZE(dual_charger_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name,
			dual_charger_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= limit)
		return NULL;

	return &dual_charger_sysfs_field_tbl[i];
}

static ssize_t dual_charger_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct dual_charger_sysfs_field_info *info = NULL;
	int len = 0;

	info = dual_charger_sysfs_field_lookup(attr->attr.name);
	if (!info || !dci) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case DUAL_CHARGER_SYSFS_ENABLE_CHARGER_MAIN:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
				dci->charge_enable_main &
				dci->charge_enable_sysfs_main);
		break;

	case DUAL_CHARGER_SYSFS_ENABLE_CHARGER_AUX:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
				dci->charge_enable_aux &
				dci->charge_enable_sysfs_aux);
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		break;
	}

	return len;
}

static ssize_t dual_charger_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct dual_charger_sysfs_field_info *info = NULL;
	long val = 0;

	info = dual_charger_sysfs_field_lookup(attr->attr.name);
	if (!info || !dci) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case DUAL_CHARGER_SYSFS_ENABLE_CHARGER_MAIN:
		if ((kstrtol(buf, 10, &val) < 0) ||
			(val < 0) || (val > 1))
			return -EINVAL;

		dci->charge_enable_sysfs_main = val;
		hwlog_info("RUNNINGTEST set charge enable main = %d\n",
			dci->charge_enable_sysfs_main);

		if (g_main_ops && g_main_ops->set_charge_enable)
			g_main_ops->set_charge_enable(
				dci->charge_enable_main &
				dci->charge_enable_sysfs_main);
		break;

	case DUAL_CHARGER_SYSFS_ENABLE_CHARGER_AUX:
		if ((kstrtol(buf, 10, &val) < 0) ||
			(val < 0) || (val > 1))
			return -EINVAL;

		dci->charge_enable_sysfs_aux = val;
		hwlog_info("RUNNINGTEST set charge enable aux = %d\n",
			dci->charge_enable_sysfs_aux);

		if (g_aux_ops && g_aux_ops->set_charge_enable)
			g_aux_ops->set_charge_enable(
				dci->charge_enable_main &
				dci->charge_enable_sysfs_main);
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		break;
	}

	return count;
}

static int dual_charger_sysfs_create_group(void)
{
	dual_charger_sysfs_init_attrs();

	return sysfs_create_group(&dci->dev->kobj,
		&dual_charger_sysfs_attr_group);
}

static void dual_charger_sysfs_remove_group(void)
{
	sysfs_remove_group(&dci->dev->kobj, &dual_charger_sysfs_attr_group);
}

#else

static inline int charge_sysfs_create_group(void)
{
	return 0;
}

static inline void charge_sysfs_remove_group(void)
{
}

#endif /* CONFIG_SYSFS */

static int dual_charger_parse_dts(void)
{
	int ret;

	ret = of_property_read_u32(
		of_find_compatible_node(NULL, NULL, "huawei,dual_charger"),
		"iin_max_each_charger", &dci->iin_max_each_charger);
	if (ret) {
		hwlog_err("iin_max_each_charger dts read failed\n");
		dci->iin_max_each_charger = DEFAULT_IIN_MAX_EACH_CHARGER;
	}
	hwlog_info("iin_max_each_charger=%d\n", dci->iin_max_each_charger);

	ret = of_property_read_u32(
		of_find_compatible_node(NULL, NULL, "huawei,dual_charger"),
		"ichg_need_aux_charger", &dci->ichg_need_aux_charger);
	if (ret) {
		hwlog_err("ichg_need_aux_charger dts read failed\n");
		dci->ichg_need_aux_charger = DEFAULT_ICHG_NEED_AUX_CHARGER;
	}
	hwlog_info("ichg_need_aux_charger=%d\n", dci->ichg_need_aux_charger);

	ret = of_property_read_u32(
		of_find_compatible_node(NULL, NULL, "huawei,charging_core"),
		"ichg_fcp", &dci->ichg_max);
	if (ret) {
		hwlog_err("ichg_fcp dts read failed\n");
		dci->ichg_max = DEFAULT_ICHG_MAX;
	}
	hwlog_info("ichg_fcp=%d\n", dci->ichg_max);

	return ret;
}

static int dual_charger_probe(struct platform_device *pdev)
{
	int ret;
	struct class *power_class = NULL;

	hwlog_info("probe begin\n");

	dci = kzalloc(sizeof(*dci), GFP_KERNEL);
	if (dci)
		return -ENOMEM;

	dci->dev = &pdev->dev;

	dual_charger_parse_dts();

	dci->iin_thermal_main = dci->iin_max_each_charger;
	dci->iin_thermal_aux = dci->iin_max_each_charger;
	dci->ichg_thermal_main = dci->ichg_max;
	dci->ichg_thermal_aux = dci->ichg_max;
	dci->ichg = dci->ichg_max;

	dci->charge_enable_main = TRUE;
	dci->charge_enable_aux = TRUE;
	dci->charge_enable_sysfs_main = TRUE;
	dci->charge_enable_sysfs_aux = TRUE;
	dci->charge_done = FALSE;

	ret = charge_ops_register(&dual_charger_ops);
	if (ret) {
		hwlog_err("register dual charge ops failed\n");
		goto dual_charger_fail_0;
	}

	if ((!g_main_ops) ||
		(!g_main_ops->chip_init) ||
		(!g_main_ops->dev_check) ||
		(!g_main_ops->set_input_current) ||
		(!g_main_ops->set_charge_current) ||
		(!g_main_ops->set_terminal_voltage) ||
		(!g_main_ops->set_dpm_voltage) ||
		(!g_main_ops->set_terminal_current) ||
		(!g_main_ops->set_charge_enable) ||
		(!g_main_ops->set_otg_enable) ||
		(!g_main_ops->set_term_enable) ||
		(!g_main_ops->get_charge_state) ||
		(!g_main_ops->reset_watchdog_timer) ||
		(!g_main_ops->dump_register) ||
		(!g_main_ops->get_register_head) ||
		(!g_main_ops->set_watchdog_timer) ||
		(!g_main_ops->set_batfet_disable) ||
		(!g_main_ops->get_ibus) ||
		(!g_main_ops->get_vbus) ||
		(!g_main_ops->get_vbat_sys) ||
		(!g_main_ops->set_covn_start) ||
		(!g_main_ops->set_charger_hiz) ||
		(!g_main_ops->check_input_dpm_state) ||
		(!g_main_ops->set_otg_current) ||
		(!g_main_ops->stop_charge_config)) {
		hwlog_err("main charge ops is null\n");
		ret = -EINVAL;
		goto dual_charger_fail_1;
	}

	if ((!g_aux_ops) ||
		(!g_aux_ops->chip_init) ||
		(!g_aux_ops->dev_check) ||
		(!g_aux_ops->set_input_current) ||
		(!g_aux_ops->set_charge_current) ||
		(!g_aux_ops->set_terminal_voltage) ||
		(!g_aux_ops->set_dpm_voltage) ||
		(!g_aux_ops->set_terminal_current) ||
		(!g_aux_ops->set_charge_enable) ||
		(!g_aux_ops->set_otg_enable) ||
		(!g_aux_ops->set_term_enable) ||
		(!g_aux_ops->get_charge_state) ||
		(!g_aux_ops->reset_watchdog_timer) ||
		(!g_aux_ops->dump_register) ||
		(!g_aux_ops->get_register_head) ||
		(!g_aux_ops->set_watchdog_timer) ||
		(!g_aux_ops->set_batfet_disable) ||
		(!g_aux_ops->get_ibus) ||
		(!g_aux_ops->set_covn_start) ||
		(!g_aux_ops->set_charger_hiz) ||
		(!g_aux_ops->check_input_dpm_state) ||
		(!g_aux_ops->stop_charge_config)) {
		hwlog_err("aux charge ops is null\n");
		ret = -EINVAL;
		goto dual_charger_fail_2;
	}

	ret = dual_charger_sysfs_create_group();
	if (ret)
		goto dual_charger_fail_2;

	power_class = hw_power_get_class();
	if (power_class) {
		if (!charge_dev)
			charge_dev = device_create(power_class, NULL, 0, NULL,
				"charger");

		ret = sysfs_create_link(&charge_dev->kobj, &dci->dev->kobj,
			"dual_charger");
		if (ret) {
			hwlog_err("sysfs link create failed\n");
			goto dual_charger_fail_3;
		}
	}

	hwlog_info("probe end\n");
	return 0;

dual_charger_fail_3:
	dual_charger_sysfs_remove_group();
dual_charger_fail_2:
	g_aux_ops = NULL;
dual_charger_fail_1:
	g_main_ops = NULL;
dual_charger_fail_0:
	kfree(dci);
	dci = NULL;
	return ret;
}

static int dual_charger_remove(struct platform_device *pdev)
{
	hwlog_info("remove begin\n");

	dual_charger_sysfs_remove_group();

	g_aux_ops = NULL;
	g_main_ops = NULL;

	kfree(dci);
	dci = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id dual_charger_match_table[] = {
	{
		.compatible = "huawei,dual_charger",
		.data = NULL,
	},
	{},
};

static struct platform_driver dual_charger_driver = {
	.probe = dual_charger_probe,
	.remove = dual_charger_remove,
	.driver = {
		.name = "huawei,dual_charger",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dual_charger_match_table),
	},
};

int __init dual_charger_init(void)
{
	return platform_driver_register(&dual_charger_driver);
}

void __exit dual_charger_exit(void)
{
	platform_driver_unregister(&dual_charger_driver);
}

module_init(dual_charger_init);
module_exit(dual_charger_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("dual charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
