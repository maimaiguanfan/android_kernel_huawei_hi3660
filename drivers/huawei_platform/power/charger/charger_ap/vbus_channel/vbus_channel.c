/*
 * vbus_channel.c
 *
 * vbus channel driver
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
#include <linux/slab.h>

#include <linux/power/hisi/hisi_bci_battery.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/vbus_channel/vbus_channel.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG vbus_ch
HWLOG_REGIST();

static struct vbus_ch_dev *g_vbus_ch_dev;

static const char * const vbus_ch_user_table[] = {
	[VBUS_CH_USER_WIRED_OTG] = "wired_otg",
	[VBUS_CH_USER_WR_TX] = "wireless_tx",
	[VBUS_CH_USER_DC] = "direct_charger",
	[VBUS_CH_USER_PD] = "pd",
	[VBUS_CH_USER_AUDIO] = "audio",
};

static const char * const vbus_ch_type_table[] = {
	[VBUS_CH_TYPE_CHARGER] = "charger",
	[VBUS_CH_TYPE_BOOST_GPIO] = "boost_gpio",
};

static int vbus_ch_check_user(unsigned int index)
{
	if ((index >= VBUS_CH_USER_BEGIN) && (index < VBUS_CH_USER_END))
		return 0;

	hwlog_err("invalid user(%d)\n", index);
	return -EINVAL;
}

static int vbus_ch_check_type(unsigned int index)
{
	if ((index >= VBUS_CH_TYPE_BEGIN) && (index < VBUS_CH_TYPE_END))
		return 0;

	hwlog_err("invalid type(%d)\n", index);
	return -EINVAL;
}

static int vbus_ch_get_type(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(vbus_ch_type_table); i++) {
		if (!strncmp(str, vbus_ch_type_table[i], strlen(str)))
			return i;
	}

	hwlog_err("invalid type_str(%s)\n", str);
	return -EINVAL;
}

static const char *vbus_ch_get_user_name(unsigned int index)
{
	if ((index >= VBUS_CH_USER_BEGIN) && (index < VBUS_CH_USER_END))
		return vbus_ch_user_table[index];

	return "illegal user";
}

static const char *vbus_ch_get_type_name(unsigned int index)
{
	if ((index >= VBUS_CH_TYPE_BEGIN) && (index < VBUS_CH_TYPE_END))
		return vbus_ch_type_table[index];

	return "illegal type";
}

static struct vbus_ch_dev *vbus_ch_get_dev(void)
{
	if (!g_vbus_ch_dev) {
		hwlog_err("g_vbus_ch_dev is null\n");
		return NULL;
	}

	return g_vbus_ch_dev;
}

static struct vbus_ch_ops *vbus_ch_get_ops(unsigned int user,
	unsigned int type)
{
	if (vbus_ch_check_user(user))
		return NULL;

	if (vbus_ch_check_type(type))
		return NULL;

	if (!g_vbus_ch_dev) {
		hwlog_err("g_vbus_ch_dev is null\n");
		return NULL;
	}

	if (!g_vbus_ch_dev->p_ops[type]) {
		hwlog_err("p_ops is null(%d)\n", type);
		return NULL;
	}

	return g_vbus_ch_dev->p_ops[type];
}

static int vbus_ch_init_dev_data(void)
{
	struct vbus_ch_dev *l_dev = NULL;

	l_dev = vbus_ch_get_dev();
	if (!l_dev)
		return -EINVAL;

	/* init support_type & total_ops */
	l_dev->support_type = 0;
	l_dev->total_ops = 0;

	return 0;
}

int vbus_ch_ops_register(struct vbus_ch_ops *ops)
{
	int type;

	if (!g_vbus_ch_dev || !ops || !ops->type_name) {
		hwlog_err("g_vbus_ch_dev or ops or type_name is null\n");
		return -EINVAL;
	}

	type = vbus_ch_get_type(ops->type_name);
	if (type < 0) {
		hwlog_err("%s ops register fail\n", ops->type_name);
		return -EINVAL;
	}

	g_vbus_ch_dev->p_ops[type] = ops;
	g_vbus_ch_dev->total_ops++;
	g_vbus_ch_dev->support_type |= (1 << type);

	hwlog_info("total_ops(%d) type(%d:%s) ops register ok\n",
		g_vbus_ch_dev->total_ops, type, ops->type_name);

	return 0;
}

int vbus_ch_open(unsigned int user, unsigned int type, int flag)
{
	struct vbus_ch_ops *l_ops = NULL;

	l_ops = vbus_ch_get_ops(user, type);
	if (!l_ops)
		return -EINVAL;

	if (!l_ops->open) {
		hwlog_err("open is null\n");
		return -EINVAL;
	}

	hwlog_info("(%s:%s)open\n",
		vbus_ch_get_user_name(user),
		vbus_ch_get_type_name(type));

	return l_ops->open(user, flag);
}

int vbus_ch_close(unsigned int user, unsigned int type, int flag, int force)
{
	struct vbus_ch_ops *l_ops = NULL;

	l_ops = vbus_ch_get_ops(user, type);
	if (!l_ops)
		return -EINVAL;

	if (!l_ops->close) {
		hwlog_err("close is null\n");
		return -EINVAL;
	}

	hwlog_info("(%s:%s)close\n",
		vbus_ch_get_user_name(user),
		vbus_ch_get_type_name(type));

	return l_ops->close(user, flag, force);
}

int vbus_ch_get_state(unsigned int user, unsigned int type, int *state)
{
	struct vbus_ch_ops *l_ops = NULL;

	l_ops = vbus_ch_get_ops(user, type);
	if (!l_ops)
		return -EINVAL;

	if (!l_ops->get_state) {
		hwlog_err("get_state is null\n");
		return -EINVAL;
	}

	hwlog_info("(%s:%s)get_state\n",
		vbus_ch_get_user_name(user),
		vbus_ch_get_type_name(type));

	return l_ops->get_state(user, state);
}

int vbus_ch_get_mode(unsigned int user, unsigned int type, int *mode)
{
	struct vbus_ch_ops *l_ops = NULL;

	l_ops = vbus_ch_get_ops(user, type);
	if (!l_ops)
		return -EINVAL;

	if (!l_ops->get_state) {
		hwlog_err("get_state is null\n");
		return -EINVAL;
	}

	hwlog_info("(%s:%s)get_mode\n",
		vbus_ch_get_user_name(user),
		vbus_ch_get_type_name(type));

	return l_ops->get_mode(user, mode);
}

int vbus_ch_set_switch_mode(unsigned int user, unsigned int type, int mode)
{
	struct vbus_ch_ops *l_ops = NULL;

	l_ops = vbus_ch_get_ops(user, type);
	if (!l_ops)
		return -EINVAL;

	if (!l_ops->set_switch_mode) {
		hwlog_err("set_switch_mode is null\n");
		return -EINVAL;
	}

	hwlog_info("(%s:%s)set_switch_mode\n",
		vbus_ch_get_user_name(user),
		vbus_ch_get_type_name(type));

	return l_ops->set_switch_mode(user, mode);
}

int vbus_ch_set_voltage(unsigned int user, unsigned int type, int volt)
{
	struct vbus_ch_ops *l_ops = NULL;

	l_ops = vbus_ch_get_ops(user, type);
	if (!l_ops)
		return -EINVAL;

	if (!l_ops->set_voltage) {
		hwlog_err("set_voltage is null\n");
		return -EINVAL;
	}

	hwlog_info("(%s:%s)set_voltage\n",
		vbus_ch_get_user_name(user),
		vbus_ch_get_type_name(type));

	return l_ops->set_voltage(user, volt);
}

int vbus_ch_get_voltage(unsigned int user, unsigned int type, int *volt)
{
	struct vbus_ch_ops *l_ops = NULL;

	l_ops = vbus_ch_get_ops(user, type);
	if (!l_ops)
		return -EINVAL;

	if (!l_ops->get_voltage) {
		hwlog_err("get_voltage is null\n");
		return -EINVAL;
	}

	hwlog_info("(%s:%s)get_voltage\n",
		vbus_ch_get_user_name(user),
		vbus_ch_get_type_name(type));

	return l_ops->get_voltage(user, volt);
}

#ifdef CONFIG_SYSFS
#define VBUS_CH_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, vbus_ch_sysfs_show, store), \
	.name = VBUS_CH_SYSFS_##n, \
}

#define VBUS_CH_SYSFS_FIELD_RO(_name, n) \
	VBUS_CH_SYSFS_FIELD(_name, n, 0444, NULL)

struct vbus_ch_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static ssize_t vbus_ch_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct vbus_ch_sysfs_field_info vbus_ch_sysfs_field_tbl[] = {
	VBUS_CH_SYSFS_FIELD_RO(support_type, SUPPORT_TYPE),
};

#define VBUS_CH_SYSFS_ATTRS_SIZE  (ARRAY_SIZE(vbus_ch_sysfs_field_tbl) + 1)

static struct attribute *vbus_ch_sysfs_attrs[VBUS_CH_SYSFS_ATTRS_SIZE];

static const struct attribute_group vbus_ch_sysfs_attr_group = {
	.attrs = vbus_ch_sysfs_attrs,
};

static void vbus_ch_sysfs_init_attrs(void)
{
	int s;
	int e = ARRAY_SIZE(vbus_ch_sysfs_field_tbl);

	for (s = 0; s < e; s++)
		vbus_ch_sysfs_attrs[s] = &vbus_ch_sysfs_field_tbl[s].attr.attr;

	vbus_ch_sysfs_attrs[e] = NULL;
}

static struct vbus_ch_sysfs_field_info *vbus_ch_sysfs_field_lookup(
	const char *name)
{
	int s;
	int e = ARRAY_SIZE(vbus_ch_sysfs_field_tbl);

	for (s = 0; s < e; s++) {
		if (!strncmp(name, vbus_ch_sysfs_field_tbl[s].attr.attr.name,
			strlen(name)))
			break;
	}

	if (s >= e)
		return NULL;

	return &vbus_ch_sysfs_field_tbl[s];
}

static ssize_t vbus_ch_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct vbus_ch_sysfs_field_info *info = NULL;
	struct vbus_ch_dev *l_dev = NULL;
	int len;

	l_dev = vbus_ch_get_dev();
	if (!l_dev)
		return -EINVAL;

	info = vbus_ch_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case VBUS_CH_SYSFS_SUPPORT_TYPE:
		len = scnprintf(buf, VBUS_CH_RD_BUF_SIZE, "%x\n",
			l_dev->support_type);
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		len = 0;
		break;
	}

	return len;
}
#endif /* CONFIG_SYSFS */

static int __init vbus_ch_init(void)
{
	int ret;
	struct vbus_ch_dev *l_dev = NULL;
	struct class *power_class = NULL;

	hwlog_info("probe begin\n");

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_vbus_ch_dev = l_dev;

#ifdef CONFIG_SYSFS
	vbus_ch_sysfs_init_attrs();

	power_class = hw_power_get_class();
	if (power_class) {
		l_dev->dev = device_create(power_class, NULL, 0, NULL,
			"vbus_channel");
		if (IS_ERR(l_dev->dev)) {
			hwlog_err("sysfs device create failed\n");
			ret = PTR_ERR(l_dev->dev);
			goto fail_free_mem;
		}

		ret = sysfs_create_group(&l_dev->dev->kobj,
			&vbus_ch_sysfs_attr_group);
		if (ret) {
			hwlog_err("sysfs group create failed\n");
			goto fail_free_mem;
		}
	}
#endif /* CONFIG_SYSFS */

	ret = vbus_ch_init_dev_data();
	if (ret) {
		hwlog_err("data init failed\n");
		goto fail_free_mem;
	}

	hwlog_info("probe end\n");
	return 0;

fail_free_mem:
	kfree(l_dev);
	g_vbus_ch_dev = NULL;

	return ret;
}

static void __exit vbus_ch_exit(void)
{
	hwlog_info("remove begin\n");

#ifdef CONFIG_SYSFS
	sysfs_remove_group(&g_vbus_ch_dev->dev->kobj,
		&vbus_ch_sysfs_attr_group);
	kobject_del(&g_vbus_ch_dev->dev->kobj);
	kobject_put(&g_vbus_ch_dev->dev->kobj);
#endif /* CONFIG_SYSFS */

	kfree(g_vbus_ch_dev);
	g_vbus_ch_dev = NULL;

	hwlog_info("remove ok\n");
}

subsys_initcall_sync(vbus_ch_init);
module_exit(vbus_ch_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("vbus channel module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
