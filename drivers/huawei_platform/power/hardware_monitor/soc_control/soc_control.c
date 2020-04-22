/*
 * soc_control.c
 *
 * battery capacity(soc: state of charge) control driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include <linux/power/hisi/hisi_bci_battery.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>
#include "soc_control.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG soc_control
HWLOG_REGIST();

static struct soc_ctl_dev *g_soc_ctl_dev;

static const char * const soc_ctl_op_user_table[SOC_CTL_OP_USER_END] = {
	[SOC_CTL_OP_USER_DEFAULT] = "default",
	[SOC_CTL_OP_USER_RC] = "rc",
	[SOC_CTL_OP_USER_HIDL] = "hidl",
	[SOC_CTL_OP_USER_CHARGE_MONITOR] = "charge_monitor",
	[SOC_CTL_OP_USER_SHELL] = "shell",
	[SOC_CTL_OP_USER_CUST] = "cust",
};

static int soc_ctl_get_op_user(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(soc_ctl_op_user_table); i++) {
		if (!strcmp(str, soc_ctl_op_user_table[i]))
			return i;
	}

	hwlog_err("invalid user_str(%s)\n", str);
	return -1;
}

static const char *soc_ctl_get_op_user_name(unsigned int index)
{
	if ((index >= SOC_CTL_OP_USER_BEGIN) && (index < SOC_CTL_OP_USER_END))
		return soc_ctl_op_user_table[index];

	return "illegal user";
}

static int soc_ctl_check_sysfs_type(unsigned int index)
{
	if ((index >= SOC_CTL_SYSFS_BEGIN) && (index < SOC_CTL_SYSFS_END))
		return 0;

	hwlog_err("invalid sysfs_type(%d)\n", index);
	return -1;
}

static struct soc_ctl_dev *soc_ctl_get_dev(void)
{
	if (!g_soc_ctl_dev) {
		hwlog_err("g_soc_ctl_dev is null\n");
		return NULL;
	}

	return g_soc_ctl_dev;
}

static void soc_ctl_startup_capacity_control(struct soc_ctl_dev *l_dev)
{
	int cur_soc = hisi_battery_capacity();

	hwlog_info("startup(%d): cur_soc=%d, min_soc=%d, max_soc=%d\n",
		l_dev->work_mode, cur_soc, l_dev->min_soc, l_dev->max_soc);

	/* disable charging and set input current to 100ma */
	if ((cur_soc > l_dev->max_soc) &&
		(l_dev->work_mode != WORK_IN_DISABLE_CHG_MODE)) {
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ALL,
			POWER_IF_SYSFS_ENABLE_CHARGER, SOC_CTL_CHG_DISABLE);
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, SOC_CTL_IIN_LIMIT);

		l_dev->work_mode = WORK_IN_DISABLE_CHG_MODE;
	}

	/* enable charging and recovery input current */
	if ((cur_soc < l_dev->min_soc) &&
		(l_dev->work_mode != WORK_IN_ENABLE_CHG_MODE)) {
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ALL,
			POWER_IF_SYSFS_ENABLE_CHARGER, SOC_CTL_CHG_ENABLE);
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, SOC_CTL_IIN_UNLIMIT);

		l_dev->work_mode = WORK_IN_ENABLE_CHG_MODE;
	}
}

static void soc_ctl_recovery_capacity_control(struct soc_ctl_dev *l_dev)
{
	if (l_dev->work_mode == WORK_IN_DEFAULT_MODE)
		return;

	hwlog_info("recovery(%d): cur_soc=%d, min_soc=%d, max_soc=%d\n",
		l_dev->work_mode,
		hisi_battery_capacity(), l_dev->min_soc, l_dev->max_soc);

	l_dev->work_mode = WORK_IN_DEFAULT_MODE;

	/* enable charging and recovery input current */
	power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ALL,
		POWER_IF_SYSFS_ENABLE_CHARGER, SOC_CTL_CHG_ENABLE);
	power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
		POWER_IF_SYSFS_VBUS_IIN_LIMIT, SOC_CTL_IIN_UNLIMIT);
}

static void soc_ctl_event_work(struct work_struct *work)
{
	struct soc_ctl_dev *l_dev = NULL;

	l_dev = soc_ctl_get_dev();
	if (!l_dev)
		return;

	soc_ctl_startup_capacity_control(l_dev);

	schedule_delayed_work(&l_dev->work,
		msecs_to_jiffies(SOC_CTL_LOOP_TIME));
}

static void soc_ctl_event_control(int event)
{
	struct soc_ctl_dev *l_dev = soc_ctl_get_dev();

	if (!l_dev)
		return;

	/*
	 * enable flag is set 0
	 * case1: bypass if not start soc control
	 * case2: stop soc control if soc control has started
	 */
	if (l_dev->enable == 0) {
		hwlog_info("disable: event=%d\n", event);

		cancel_delayed_work(&l_dev->work);
		soc_ctl_recovery_capacity_control(l_dev);

		return;
	}

	/*
	 * enable flag is set 1
	 * case1: start soc control with 30s delay when usb insert
	 * case2: stop soc control when usb remove
	 */
	if (event == SOC_CTL_START_EVENT) {
		hwlog_info("enable: start soc control\n");

		cancel_delayed_work(&l_dev->work);
		schedule_delayed_work(&l_dev->work,
			msecs_to_jiffies(SOC_CTL_START_TIME));
	} else {
		hwlog_info("enable: stop soc control\n");

		l_dev->event = SOC_CTL_DEFAULT_EVENT;
		cancel_delayed_work(&l_dev->work);
		soc_ctl_recovery_capacity_control(l_dev);
	}
}

static int soc_ctl_event_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct soc_ctl_dev *l_dev = soc_ctl_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	/*
	 * after receiving the message of starting charging,
	 * we set the event to start, otherwise set the event to stop
	 */
	if (event == CHARGER_START_CHARGING_EVENT) {
		/* ignore repeat event */
		if (l_dev->event == SOC_CTL_START_EVENT)
			return NOTIFY_OK;

		l_dev->event = SOC_CTL_START_EVENT;
	} else {
		/* ignore repeat event */
		if (l_dev->event == SOC_CTL_STOP_EVENT)
			return NOTIFY_OK;

		l_dev->event = SOC_CTL_STOP_EVENT;
	}

	/* process soc control */
	soc_ctl_event_control(l_dev->event);

	return NOTIFY_OK;
}

#ifdef CONFIG_SYSFS
#define SOC_CTL_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, soc_ctl_sysfs_show, store), \
	.name = SOC_CTL_SYSFS_##n, \
}

#define SOC_CTL_SYSFS_FIELD_RW(_name, n) \
	SOC_CTL_SYSFS_FIELD(_name, n, 0644, soc_ctl_sysfs_store)

#define SOC_CTL_SYSFS_FIELD_RO(_name, n) \
	SOC_CTL_SYSFS_FIELD(_name, n, 0444, NULL)

struct soc_ctl_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static ssize_t soc_ctl_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t soc_ctl_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct soc_ctl_sysfs_field_info soc_ctl_sysfs_field_tbl[] = {
	SOC_CTL_SYSFS_FIELD_RW(control, CONTROL),
};

#define SOC_CTL_SYSFS_ATTRS_SIZE (ARRAY_SIZE(soc_ctl_sysfs_field_tbl) + 1)

static struct attribute *soc_ctl_sysfs_attrs[SOC_CTL_SYSFS_ATTRS_SIZE];

static const struct attribute_group soc_ctl_sysfs_attr_group = {
	.attrs = soc_ctl_sysfs_attrs,
};

static void soc_ctl_sysfs_init_attrs(void)
{
	int s;
	int e = ARRAY_SIZE(soc_ctl_sysfs_field_tbl);

	for (s = 0; s < e; s++)
		soc_ctl_sysfs_attrs[s] = &soc_ctl_sysfs_field_tbl[s].attr.attr;

	soc_ctl_sysfs_attrs[e] = NULL;
}

static struct soc_ctl_sysfs_field_info *soc_ctl_sysfs_field_lookup(
	const char *name)
{
	int s;
	int e = ARRAY_SIZE(soc_ctl_sysfs_field_tbl);

	for (s = 0; s < e; s++) {
		if (!strncmp(name, soc_ctl_sysfs_field_tbl[s].attr.attr.name,
			strlen(name)))
			break;
	}

	if (s >= e)
		return NULL;

	return &soc_ctl_sysfs_field_tbl[s];
}

static ssize_t soc_ctl_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct soc_ctl_sysfs_field_info *info = NULL;
	struct soc_ctl_dev *l_dev = soc_ctl_get_dev();
	int len;

	if (!l_dev)
		return -EINVAL;

	info = soc_ctl_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case SOC_CTL_SYSFS_CONTROL:
		len = scnprintf(buf, PAGE_SIZE,
			"user=%s, enable=%d, min_soc=%d, max_soc=%d\n",
			soc_ctl_get_op_user_name(l_dev->user),
			l_dev->enable,
			l_dev->min_soc, l_dev->max_soc);
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		len = 0;
		break;
	}

	return len;
}

static ssize_t soc_ctl_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct soc_ctl_sysfs_field_info *info = NULL;
	struct soc_ctl_dev *l_dev = soc_ctl_get_dev();
	char user_name[SOC_CTL_RW_BUF_SIZE] = { 0 };
	int user;
	int enable;
	unsigned int min_soc;
	unsigned int max_soc;

	if (!l_dev)
		return -EINVAL;

	info = soc_ctl_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	if (soc_ctl_check_sysfs_type(info->name))
		return -EINVAL;

	if (count >= (SOC_CTL_RW_BUF_SIZE - 2)) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	/* 4: the fields of "user enable min_soc max_soc" */
	if (sscanf(buf, "%s %d %d %d",
		user_name, &enable, &min_soc, &max_soc) != 4) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	user = soc_ctl_get_op_user(user_name);
	if (user < 0)
		return -EINVAL;

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((enable < 0) || (enable > 1)) {
		hwlog_err("invalid enable(%d)\n", enable);
		return -EINVAL;
	}

	/* soc must be (0, 100) and (max_soc - min_soc) >= 10 */
	if ((min_soc < 0 || min_soc > 100) ||
		(max_soc < 0 || max_soc > 100) ||
		(min_soc + 10 > max_soc)) {
		hwlog_err("invalid min_soc(%d) or max_soc(%d)\n",
			min_soc, max_soc);
		return -EINVAL;
	}

	hwlog_info("set: user=%s, enable=%d, min_soc=%d, max_soc=%d\n",
		soc_ctl_get_op_user_name(user), enable, min_soc, max_soc);

	l_dev->user = user;
	l_dev->min_soc = min_soc;
	l_dev->max_soc = max_soc;

	/* ignore the same control event */
	if (l_dev->enable == enable) {
		hwlog_info("ignore the same control event\n");
		return count;
	}
	l_dev->enable = enable;

	/* process soc control */
	soc_ctl_event_control(l_dev->event);

	return count;
}
#endif /* CONFIG_SYSFS */

static int __init soc_ctl_init(void)
{
	int ret;
	struct soc_ctl_dev *l_dev = NULL;
	struct class *power_class = NULL;

	hwlog_info("probe begin\n");

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_soc_ctl_dev = l_dev;

	INIT_DELAYED_WORK(&l_dev->work, soc_ctl_event_work);

	l_dev->nb.notifier_call = soc_ctl_event_call;
	ret = blocking_notifier_chain_register(&charger_event_notify_head,
		&l_dev->nb);
	if (ret) {
		hwlog_err("register charger_event notifier failed\n");
		goto fail_free_mem;
	}

#ifdef CONFIG_SYSFS
	soc_ctl_sysfs_init_attrs();

	power_class = hw_power_get_class();
	if (power_class) {
		l_dev->dev = device_create(power_class, NULL, 0, NULL,
			"soc_control");
		if (IS_ERR(l_dev->dev)) {
			hwlog_err("sysfs device create failed\n");
			ret = PTR_ERR(l_dev->dev);
			goto fail_create_device;
		}

		ret = sysfs_create_group(&l_dev->dev->kobj,
			&soc_ctl_sysfs_attr_group);
		if (ret) {
			hwlog_err("sysfs group create failed\n");
			goto fail_create_sysfs;
		}
	}
#endif /* CONFIG_SYSFS */

	hwlog_info("probe end\n");
	return 0;

#ifdef CONFIG_SYSFS
fail_create_sysfs:
fail_create_device:
#endif /* CONFIG_SYSFS */
fail_free_mem:
	kfree(l_dev);
	g_soc_ctl_dev = NULL;

	return ret;
}

static void __exit soc_ctl_exit(void)
{
	struct soc_ctl_dev *l_dev = g_soc_ctl_dev;

	hwlog_info("remove begin\n");

	if (!l_dev)
		return;

	cancel_delayed_work(&l_dev->work);
	blocking_notifier_chain_unregister(&charger_event_notify_head,
		&l_dev->nb);

#ifdef CONFIG_SYSFS
	sysfs_remove_group(&l_dev->dev->kobj, &soc_ctl_sysfs_attr_group);
	kobject_del(&l_dev->dev->kobj);
	kobject_put(&l_dev->dev->kobj);
#endif /* CONFIG_SYSFS */

	kfree(l_dev);
	g_soc_ctl_dev = NULL;

	hwlog_info("remove end\n");
}

fs_initcall_sync(soc_ctl_init);
module_exit(soc_ctl_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery capacity control driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
