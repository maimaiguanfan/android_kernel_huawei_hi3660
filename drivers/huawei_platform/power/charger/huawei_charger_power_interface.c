#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#include <linux/power/hisi/hisi_bci_battery.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger_power_interface.h>

#define HWLOG_TAG power_if
HWLOG_REGIST();


struct power_if_device_info *g_power_if_info;

static DEFINE_MUTEX(g_power_if_sysfs_get_mutex);
static DEFINE_MUTEX(g_power_if_sysfs_set_mutex);


static const char * const power_if_op_user_table[] = {
	[POWER_IF_OP_USER_DEFAULT] = "default",
	[POWER_IF_OP_USER_RC] = "rc",
	[POWER_IF_OP_USER_HIDL] = "hidl",
	[POWER_IF_OP_USER_HEALTHD] = "healthd",
	[POWER_IF_OP_USER_LIMIT_CURRENT] = "limit_current",
	[POWER_IF_OP_USER_CHARGE_MONITOR] = "charge_monitor",
	[POWER_IF_OP_USER_ATCMD] = "atcmd",
	[POWER_IF_OP_USER_THERMAL] = "thermal",
	[POWER_IF_OP_USER_AI] = "ai",
	[POWER_IF_OP_USER_RUNNING] = "running",
	[POWER_IF_OP_USER_FWK] = "fwk",
	[POWER_IF_OP_USER_APP] = "app",
	[POWER_IF_OP_USER_SHELL] = "shell",
	[POWER_IF_OP_USER_KERNEL] = "kernel",
};

static const char * const power_if_op_type_table[] = {
	[POWER_IF_OP_TYPE_DCP] = "dcp",
	[POWER_IF_OP_TYPE_DCP_SH] = "dcp_sh",
	[POWER_IF_OP_TYPE_OTG] = "otg",
	[POWER_IF_OP_TYPE_FCP] = "fcp",
	[POWER_IF_OP_TYPE_FCP_AUX] = "fcp_aux",
	[POWER_IF_OP_TYPE_PD] = "pd",
	[POWER_IF_OP_TYPE_PD_AUX] = "pd_aux",
	[POWER_IF_OP_TYPE_LVC] = "lvc",
	[POWER_IF_OP_TYPE_LVC_AUX] = "lvc_aux",
	[POWER_IF_OP_TYPE_LVC_SH] = "lvc_sh",
	[POWER_IF_OP_TYPE_LVC_AUX_SH] = "lvc_aux_sh",
	[POWER_IF_OP_TYPE_SC] = "sc",
	[POWER_IF_OP_TYPE_SC_AUX] = "sc_aux",
	[POWER_IF_OP_TYPE_SC_SH] = "sc_sh",
	[POWER_IF_OP_TYPE_SC_AUX_SH] = "sc_aux_sh",
	[POWER_IF_OP_TYPE_WL] = "wl",
	[POWER_IF_OP_TYPE_WL_LVC] = "wl_lvc",
	[POWER_IF_OP_TYPE_WL_SC] = "wl_sc",
	[POWER_IF_OP_TYPE_WL_REVERSE] = "wl_reverse",
	[POWER_IF_OP_TYPE_ALL] = "all",
};

static const char * const power_if_sysfs_type_table[] = {
	[POWER_IF_SYSFS_ENABLE_CHARGER] = "enable_charger",
	[POWER_IF_SYSFS_VBUS_IIN_LIMIT] = "iin_limit",
};


static const char *power_if_get_op_user_name(unsigned int index)
{
	if ((index >= POWER_IF_OP_USER_BEGIN) && (index < POWER_IF_OP_USER_END))
		return power_if_op_user_table[index];

	return "illegal user";
}

static const char *power_if_get_op_type_name(unsigned int index)
{
	if ((index >= POWER_IF_OP_TYPE_BEGIN) && (index < POWER_IF_OP_TYPE_END))
		return power_if_op_type_table[index];

	return "illegal type";
}

static const char *power_if_get_sysfs_type_name(unsigned int index)
{
	if ((index >= POWER_IF_SYSFS_BEGIN) && (index < POWER_IF_SYSFS_END))
		return power_if_sysfs_type_table[index];

	return "illegal sysfs_type";
}

static int power_if_get_op_user(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(power_if_op_user_table); i++) {
		if (!strncmp(str, power_if_op_user_table[i], strlen(str)))
			return i;
	}

	hwlog_err("invalid user_str(%s)\n", str);
	return -1;
}

static int power_if_get_op_type(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(power_if_op_type_table); i++) {
		if (!strncmp(str, power_if_op_type_table[i], strlen(str)))
			return i;
	}

	hwlog_err("invalid type_str(%s)\n", str);
	return -1;
}

static int power_if_check_op_user(unsigned int index)
{
	if ((index >= POWER_IF_OP_USER_BEGIN) && (index < POWER_IF_OP_USER_END))
		return 0;

	hwlog_err("invalid user(%d)\n", index);
	return -1;
}

static int power_if_check_op_type(unsigned int index)
{
	if ((index >= POWER_IF_OP_TYPE_BEGIN) && (index < POWER_IF_OP_TYPE_END))
		return 0;

	hwlog_err("invalid type(%d)\n", index);
	return -1;
}

static int power_if_check_sysfs_type(unsigned int index)
{
	if ((index >= POWER_IF_SYSFS_BEGIN) && (index < POWER_IF_SYSFS_END))
		return 0;

	hwlog_err("invalid sysfs_type(%d)\n", index);
	return -1;
}

static int power_if_check_paras(unsigned int user, unsigned int type,
	unsigned int sysfs_type)
{
	if (power_if_check_op_user(user))
		return -1;

	if (power_if_check_op_type(type))
		return -1;

	if (power_if_check_sysfs_type(sysfs_type))
		return -1;

	return 0;
}

static struct power_if_ops *power_if_get_ops(unsigned int type)
{
	if (!g_power_if_info) {
		hwlog_err("g_power_if_info is null\n");
		return NULL;
	}

	if (!g_power_if_info->ops[type]) {
		hwlog_err("ops is null\n");
		return NULL;
	}

	return g_power_if_info->ops[type];
}

static int power_if_operator_get(unsigned int type, unsigned int sysfs_type,
	unsigned int *value)
{
	int ret = POWER_IF_ERRCODE_INVAID_OP;
	struct power_if_ops *l_ops = NULL;

	l_ops = power_if_get_ops(type);
	if (!l_ops)
		return ret;

	switch (sysfs_type) {
	case POWER_IF_SYSFS_ENABLE_CHARGER:
		if (l_ops->get_enable_charger) {
			ret = l_ops->get_enable_charger(value);
			hwlog_info("get enable charger=%d\n", *value);
		}
		break;

	case POWER_IF_SYSFS_VBUS_IIN_LIMIT:
		if (l_ops->get_iin_limit) {
			ret = l_ops->get_iin_limit(value);
			hwlog_info("get vbus iin_limit=%d\n", *value);
		}
		break;

	default:
		break;
	}

	return ret;
}

static int power_if_operator_set(unsigned int type, unsigned int sysfs_type,
	unsigned int value)
{
	int ret = 0;
	struct power_if_ops *l_ops = NULL;

	l_ops = power_if_get_ops(type);
	if (!l_ops)
		return ret;

	switch (sysfs_type) {
	case POWER_IF_SYSFS_ENABLE_CHARGER:
		if (l_ops->set_enable_charger) {
			ret = l_ops->set_enable_charger(value);
			hwlog_info("set enable charger=%d\n", value);
		}
		break;

	case POWER_IF_SYSFS_VBUS_IIN_LIMIT:
		if (l_ops->set_iin_limit) {
			ret = l_ops->set_iin_limit(value);
			hwlog_info("set vbus iin_limit=%d\n", value);
		}
		break;

	default:
		break;
	}

	return ret;
}

static int power_if_common_sysfs_get(unsigned int user, unsigned int type,
	unsigned int sysfs_type, char *buf)
{
	int ret;
	unsigned int value;
	unsigned int type_s;
	unsigned int type_e;
	char rd_buf[POWER_IF_RD_BUF_SIZE] = {0};

	if (power_if_check_paras(user, type, sysfs_type))
		return -1;

	hwlog_info("sysfs_get(%s): user(%s) type(%s)\n",
		power_if_get_sysfs_type_name(sysfs_type),
		power_if_get_op_user_name(user),
		power_if_get_op_type_name(type));

	mutex_lock(&g_power_if_sysfs_get_mutex);

	if (type == POWER_IF_OP_TYPE_ALL) {
		/* loop all type */
		type_s = POWER_IF_OP_TYPE_BEGIN;
		type_e = type;
	} else {
		/* specified one type */
		type_s = type;
		type_e = type + 1;
	}

	/* output all value */
	for (; type_s < type_e; type_s++) {
		value = 0;
		ret = power_if_operator_get(type_s, sysfs_type, &value);

		/* if op is invalid, must be skip output */
		if (ret == POWER_IF_ERRCODE_INVAID_OP)
			continue;

		memset(rd_buf, 0, POWER_IF_RD_BUF_SIZE);
		if (ret == 0)
			scnprintf(rd_buf, POWER_IF_RD_BUF_SIZE, "%s: %d\n",
				power_if_get_op_type_name(type_s), value);
		else
			scnprintf(rd_buf, POWER_IF_RD_BUF_SIZE, "%s: invalid\n",
				power_if_get_op_type_name(type_s));

		strncat(buf, rd_buf, strlen(rd_buf));
	}

	mutex_unlock(&g_power_if_sysfs_get_mutex);

	return strlen(buf);
}

static int power_if_common_sysfs_set(unsigned int user, unsigned int type,
	unsigned int sysfs_type, unsigned int value)
{
	int ret = 0;
	unsigned int type_s;
	unsigned int type_e;

	if (power_if_check_paras(user, type, sysfs_type))
		return -1;

	hwlog_info("sysfs_set(%s): user(%s) type(%s) value(%d)\n",
		power_if_get_sysfs_type_name(sysfs_type),
		power_if_get_op_user_name(user),
		power_if_get_op_type_name(type), value);

	mutex_lock(&g_power_if_sysfs_set_mutex);

	if (type == POWER_IF_OP_TYPE_ALL) {
		/* loop all type */
		type_s = POWER_IF_OP_TYPE_BEGIN;
		type_e = type;
	} else {
		/* specified one type */
		type_s = type;
		type_e = type + 1;
	}

	for (; type_s < type_e; type_s++)
		ret |= power_if_operator_set(type_s, sysfs_type, value);

	mutex_unlock(&g_power_if_sysfs_set_mutex);

	return ret;
}

int power_if_kernel_sysfs_get(unsigned int type, unsigned int sysfs_type,
	unsigned int *value)
{
	hwlog_info("sysfs_get(%s): user(%s) type(%s)\n",
		power_if_get_sysfs_type_name(sysfs_type),
		power_if_get_op_user_name(POWER_IF_OP_USER_KERNEL),
		power_if_get_op_type_name(type));

	return power_if_operator_get(type, sysfs_type, value);
}
EXPORT_SYMBOL_GPL(power_if_kernel_sysfs_get);

int power_if_kernel_sysfs_set(unsigned int type, unsigned int sysfs_type,
	unsigned int value)
{
	return power_if_common_sysfs_set(POWER_IF_OP_USER_KERNEL, type,
		sysfs_type, value);
}
EXPORT_SYMBOL_GPL(power_if_kernel_sysfs_set);


#ifdef CONFIG_SYSFS
#define POWER_IF_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, power_if_sysfs_show, store), \
	.name = POWER_IF_SYSFS_##n, \
}

#define POWER_IF_SYSFS_FIELD_RW(_name, n) \
	POWER_IF_SYSFS_FIELD(_name, n, 0644, power_if_sysfs_store)

#define POWER_IF_SYSFS_FIELD_RO(_name, n) \
	POWER_IF_SYSFS_FIELD(_name, n, 0444, NULL)

struct power_if_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static ssize_t power_if_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t power_if_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_if_sysfs_field_info power_if_sysfs_field_tbl[] = {
	POWER_IF_SYSFS_FIELD_RW(enable_charger, ENABLE_CHARGER),
	POWER_IF_SYSFS_FIELD_RW(iin_limit, VBUS_IIN_LIMIT),
};

#define POWER_IF_ATTRS_SIZE  (ARRAY_SIZE(power_if_sysfs_field_tbl) + 1)

static struct attribute *power_if_sysfs_attrs[POWER_IF_ATTRS_SIZE];

static const struct attribute_group power_if_sysfs_attr_group = {
	.attrs = power_if_sysfs_attrs,
};

static void power_if_sysfs_init_attrs(void)
{
	int s, e = ARRAY_SIZE(power_if_sysfs_field_tbl);

	for (s = 0; s < e; s++)
		power_if_sysfs_attrs[s] =
			&power_if_sysfs_field_tbl[s].attr.attr;

	power_if_sysfs_attrs[e] = NULL;
}

static struct power_if_sysfs_field_info *power_if_sysfs_field_lookup(
	const char *name)
{
	int s, e = ARRAY_SIZE(power_if_sysfs_field_tbl);

	for (s = 0; s < e; s++) {
		if (!strncmp(name, power_if_sysfs_field_tbl[s].attr.attr.name,
			strlen(name)))
			break;
	}

	if (s >= e)
		return NULL;

	return &power_if_sysfs_field_tbl[s];
}

static ssize_t power_if_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_if_sysfs_field_info *info = NULL;

	info = power_if_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	return power_if_common_sysfs_get(POWER_IF_OP_USER_DEFAULT,
		POWER_IF_OP_TYPE_ALL, info->name, buf);
}

static ssize_t power_if_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_if_sysfs_field_info *info = NULL;

	char user_name[POWER_IF_RD_BUF_SIZE] = {0};
	char type_name[POWER_IF_RD_BUF_SIZE] = {0};
	int user;
	int type;
	unsigned int value;

	info = power_if_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	if (count >= (POWER_IF_RD_BUF_SIZE - 2)) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	if (sscanf(buf, "%s %s %d", user_name, type_name, &value) != 3) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	user = power_if_get_op_user(user_name);
	if (user < 0) {
		hwlog_err("invalid user(%s)\n", user_name);
		return -EINVAL;
	}

	type = power_if_get_op_type(type_name);
	if (type < 0) {
		hwlog_err("invalid type(%s)\n", type_name);
		return -EINVAL;
	}

	power_if_common_sysfs_set(user, type, info->name, value);

	return count;
}
#endif /* CONFIG_SYSFS */

int power_if_ops_register(struct power_if_ops *ops)
{
	int type;

	if (!g_power_if_info || !ops || !ops->type_name) {
		hwlog_err("g_power_if_info or ops is null\n");
		return -EPERM;
	}

	type = power_if_get_op_type(ops->type_name);
	if (type < 0) {
		hwlog_err("%s ops register fail\n", ops->type_name);
		return -EPERM;
	}

	g_power_if_info->ops[type] = ops;
	g_power_if_info->total_ops++;

	hwlog_info("total_ops(%d) type(%d:%s) ops register ok\n",
		g_power_if_info->total_ops, type, ops->type_name);

	return 0;
}

static int __init power_interface_init(void)
{
#ifdef CONFIG_SYSFS
	int ret;
#endif /* CONFIG_SYSFS */
	struct power_if_device_info *info = NULL;
	struct class *power_class = NULL;

	hwlog_info("probe begin\n");

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	g_power_if_info = info;

#ifdef CONFIG_SYSFS
	power_if_sysfs_init_attrs();

	power_class = hw_power_get_class();
	if (power_class) {
		g_power_if_info->dev = device_create(power_class, NULL, 0, NULL,
			"interface");
		if (IS_ERR(g_power_if_info->dev)) {
			hwlog_err("sysfs device create failed\n");
			ret = PTR_ERR(g_power_if_info->dev);
			goto fail_create_device;
		}

		ret = sysfs_create_group(&g_power_if_info->dev->kobj,
			&power_if_sysfs_attr_group);
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
	kfree(info);
	g_power_if_info = NULL;

	return ret;
#endif /* CONFIG_SYSFS */
}

static void __exit power_interface_exit(void)
{
	hwlog_info("remove begin\n");

#ifdef CONFIG_SYSFS
	sysfs_remove_group(&g_power_if_info->dev->kobj,
		&power_if_sysfs_attr_group);
	kobject_del(&g_power_if_info->dev->kobj);
	kobject_put(&g_power_if_info->dev->kobj);
#endif /* CONFIG_SYSFS */

	kfree(g_power_if_info);
	g_power_if_info = NULL;

	hwlog_info("remove ok\n");
}

fs_initcall_sync(power_interface_init);
module_exit(power_interface_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("power interface module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
