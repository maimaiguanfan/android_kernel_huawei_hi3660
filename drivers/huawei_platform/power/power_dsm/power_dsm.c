/*
 * power_dsm.c
 *
 * dsm for power module
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
#include <linux/kernel.h>
#include <linux/kobject.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_dsm.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG power_dsm
HWLOG_REGIST();

/* #define POWER_DSM_DEBUG_TEST */

#if (defined(POWER_DSM_DEBUG_TEST) && defined(CONFIG_SYSFS))
static struct kobject *power_dsm_kobj;
#endif /* POWER_DSM_DEBUG_TEST && CONFIG_SYSFS */

/* define dsm device stract data */
static struct dsm_dev power_dsm_dev_cpu_buck = {
	.name = "dsm_cpu_buck",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev power_dsm_dev_usb = {
	.name = "dsm_usb",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_0256,
};

static struct dsm_dev power_dsm_dev_battery_detect = {
	.name = "dsm_battery_detect",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev power_dsm_dev_battery = {
	.name = "dsm_battery",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_2048,
};

static struct dsm_dev power_dsm_dev_charge_monitor = {
	.name = "dsm_charge_monitor",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev power_dsm_dev_superswitch = {
	.name = "superswitch",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev power_dsm_dev_smpl = {
	.name = "dsm_smpl",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev power_dsm_dev_pd_richtek = {
	.name = "dsm_pd_richtek",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev power_dsm_dev_pd = {
	.name = "dsm_pd",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_0256,
};

static struct dsm_dev power_dsm_dev_uscp = {
	.name = "dsm_usb_short_circuit_protect",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev power_dsm_dev_pmu_ocp = {
	.name = "dsm_pmu_ocp",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev power_dsm_dev_pmic_irq = {
	.name = "dsm_pmu_irq",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev power_dsm_dev_sc = {
	.name = "dsm_direct_charge_sc",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

/* define power dsm struct data */
struct power_dsm_data_info power_dsm_data[] = {
	{POWER_DSM_CPU_BUCK, "dsm_cpu_buck", NULL,
		&power_dsm_dev_cpu_buck},

	{POWER_DSM_USB, "dsm_usb", NULL,
		&power_dsm_dev_usb},

	{POWER_DSM_BATTERY_DETECT, "dsm_battery_detect", NULL,
		&power_dsm_dev_battery_detect},

	{POWER_DSM_BATTERY, "dsm_battery", NULL,
		&power_dsm_dev_battery},

	{POWER_DSM_CHARGE_MONITOR, "dsm_charge_monitor", NULL,
		&power_dsm_dev_charge_monitor},

	{POWER_DSM_SUPERSWITCH, "superswitch", NULL,
		&power_dsm_dev_superswitch},

	{POWER_DSM_SMPL, "dsm_smpl", NULL,
		&power_dsm_dev_smpl},

	{POWER_DSM_PD_RICHTEK, "dsm_pd_richtek", NULL,
		&power_dsm_dev_pd_richtek},

	{POWER_DSM_PD, "dsm_pd", NULL,
		&power_dsm_dev_pd},

	{POWER_DSM_USCP, "dsm_usb_short_circuit_protect", NULL,
		&power_dsm_dev_uscp},

	{POWER_DSM_PMU_OCP, "dsm_pmu_ocp", NULL,
		&power_dsm_dev_pmu_ocp},

	{POWER_DSM_PMU_IRQ, "dsm_pmu_irq", NULL,
		&power_dsm_dev_pmic_irq},

	{POWER_DSM_DIRECT_CHARGE_SC, "dsm_direct_charge_sc", NULL,
		&power_dsm_dev_sc},
};

struct dsm_client *power_dsm_get_dclient(enum power_dsm_type type)
{
	int i;
	struct power_dsm_data_info *p_data = power_dsm_data;
	int size = ARRAY_SIZE(power_dsm_data);
	struct dsm_client *client = NULL;

	if ((type < POWER_DSM_TYPE_BEGIN) || (type >= POWER_DSM_TYPE_END)) {
		hwlog_err("invalid type(%d)\n", type);
		return NULL;
	}

	hwlog_info("type is [%d]\n", type);

	for (i = 0; i < size; i++) {
		if (type == p_data[i].type) {
			client = p_data[i].client;
			break;
		}
	}

	if (!client) {
		hwlog_err("client is null\n");
		return NULL;
	}

	return client;
}
EXPORT_SYMBOL_GPL(power_dsm_get_dclient);

int power_dsm_dmd_report(enum power_dsm_type type, int err_no, void *buf)
{
	struct dsm_client *client = NULL;

	client = power_dsm_get_dclient(type);

	if (!client || !buf) {
		hwlog_err("client or buf is null\n");
		return -1;
	}

	if (!dsm_client_ocuppy(client)) {
		dsm_client_record(client, "%s", buf);
		dsm_client_notify(client, err_no);
		hwlog_info("report type:%d, err_no:%d\n", type, err_no);
		return 0;
	}

	hwlog_err("power dsm client is busy\n");
	return -1;
}
EXPORT_SYMBOL_GPL(power_dsm_dmd_report);

#ifdef CONFIG_HUAWEI_DATA_ACQUISITION
int power_dsm_bigdata_report(enum power_dsm_type type, int err_no, void *msg)
{
	struct dsm_client *client = NULL;

	client = power_dsm_get_dclient(type);

	if (!client || !msg) {
		hwlog_err("client or msg is null\n");
		return -1;
	}

	if (!dsm_client_ocuppy(client)) {
		dsm_client_copy_ext(client, (struct message *)msg,
			sizeof(struct message));
		dsm_client_notify(client, err_no);
		hwlog_info("report type:%d, err_no:%d\n", type, err_no);
		return 0;
	}

	hwlog_err("power dsm client is busy\n");
	return -1;
}
EXPORT_SYMBOL_GPL(power_dsm_bigdata_report);
#endif /* CONFIG_HUAWEI_DATA_ACQUISITION */

#if (defined(POWER_DSM_DEBUG_TEST) && defined(CONFIG_SYSFS))
static enum power_dsm_type g_power_dsm_type = POWER_DSM_TYPE_END;
static int g_power_dsm_errno;

static ssize_t power_dsm_set_errno(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int dsm_type;
	unsigned int dsm_errno;

	if (count >= POWER_DSM_BUF_SIZE_0016) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	/* 2: numbers for dsm_type and dsm_errno */
	if (sscanf(buf, "%d %d", &dsm_type, &dsm_errno) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	if ((dsm_type < POWER_DSM_TYPE_BEGIN) ||
		(dsm_type >= POWER_DSM_TYPE_END)) {
		hwlog_err("invalid type(%d)\n", dsm_type);
		return -EINVAL;
	}

	g_power_dsm_type = dsm_type;
	g_power_dsm_errno = dsm_errno;

	power_dsm_dmd_report(dsm_type, dsm_errno, buf);

	return count;
}

static ssize_t power_dsm_show_errno_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "type:%d, err_no:%d",
		g_power_dsm_type, g_power_dsm_errno);
}

static DEVICE_ATTR(set_errno, 0644,
	power_dsm_show_errno_info, power_dsm_set_errno);

static struct attribute *power_dsm_attributes[] = {
	&dev_attr_set_errno.attr,
	NULL,
};

static const struct attribute_group power_dsm_attr_group = {
	.attrs = power_dsm_attributes,
};
#endif /* POWER_DSM_DEBUG_TEST && CONFIG_SYSFS */

static int __init power_dsm_init(void)
{
#if (defined(POWER_DSM_DEBUG_TEST) && defined(CONFIG_SYSFS))
	int ret;
#endif /* POWER_DSM_DEBUG_TEST && CONFIG_SYSFS */
	int i;
	struct power_dsm_data_info *p_data = power_dsm_data;
	int size = ARRAY_SIZE(power_dsm_data);

	hwlog_info("probe begin\n");

	for (i = 0; i < size; i++) {
		if (!p_data[i].client)
			p_data[i].client = dsm_register_client(p_data[i].dev);

		if (!p_data[i].client)
			hwlog_err("(%s)dsm register fail\n", p_data[i].name);
		else
			hwlog_info("(%s)dsm register ok\n", p_data[i].name);
	}

#if (defined(POWER_DSM_DEBUG_TEST) && defined(CONFIG_SYSFS))
	power_dsm_kobj = kobject_create_and_add("power_dsm", NULL);
	if (!power_dsm_kobj) {
		hwlog_err("kobject create failed\n");
		ret = -ENOMEM;
		goto fail_create_kobj;
	}

	ret = sysfs_create_group(power_dsm_kobj, &power_dsm_attr_group);
	if (ret) {
		hwlog_err("sysfs group create failed\n");
		goto fail_create_sysfs;
	}
#endif /* POWER_DSM_DEBUG_TEST && CONFIG_SYSFS */

	hwlog_info("probe end\n");
	return 0;

#if (defined(POWER_DSM_DEBUG_TEST) && defined(CONFIG_SYSFS))
fail_create_sysfs:
	kobject_del(power_dsm_kobj);
	power_dsm_kobj == NULL;

fail_create_kobj:
	return ret;
#endif /* POWER_DSM_DEBUG_TEST && CONFIG_SYSFS */
}

static void __exit power_dsm_exit(void)
{
	int i;
	struct power_dsm_data_info *p_data = power_dsm_data;
	int size = ARRAY_SIZE(power_dsm_data);

	hwlog_info("remove begin\n");

	for (i = 0; i < size; i++) {
		p_data[i].client = NULL;
		hwlog_info("dsm %s un-register ok\n", p_data[i].name);
	}

#if (defined(POWER_DSM_DEBUG_TEST) && defined(CONFIG_SYSFS))
	sysfs_remove_group(power_dsm_kobj, &power_dsm_attr_group);
	kobject_del(power_dsm_kobj);
	power_dsm_kobj == NULL;
#endif /* POWER_DSM_DEBUG_TEST && CONFIG_SYSFS */

	hwlog_info("remove end\n");
}

subsys_initcall_sync(power_dsm_init);
module_exit(power_dsm_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("dsm for power module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
