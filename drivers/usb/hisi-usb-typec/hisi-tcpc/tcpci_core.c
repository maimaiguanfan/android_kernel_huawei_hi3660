/*
 * Copyright (C) 2018 Hisilicon.
 * Author: Hisilicon <>
 *
 * Copyright (C) 2016 Richtek Technology Corp.
 * Author: TH <tsunghan_tsai@richtek.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>

#include "securec.h"

#include "include/tcpci.h"
#include "include/tcpci_typec.h"


#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
#include "pd_dpm_prv.h"
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

#define to_tcpc_device(obj) container_of(obj, struct tcpc_device, dev)
static struct class *hisi_tcpc_class;
static struct device_type tcpc_dev_type;

#ifdef CONFIG_HISI_DEBUG_FS
static ssize_t tcpc_show_property(struct device *dev,
				  struct device_attribute *attr, char *buf);
static ssize_t tcpc_store_property(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count);

#define TCPC_DEVICE_ATTR(_name, _mode)					\
{									\
	.attr = { .name = #_name, .mode = (_mode) },			\
	.show = tcpc_show_property,					\
	.store = tcpc_store_property,					\
}


static struct device_attribute tcpc_device_attributes[] = {
	TCPC_DEVICE_ATTR(role_def, S_IRUGO | S_IWUSR),
	TCPC_DEVICE_ATTR(rp_lvl, S_IRUGO | S_IWUSR),
	TCPC_DEVICE_ATTR(pd_test, S_IRUGO | S_IWUSR),
	TCPC_DEVICE_ATTR(info, S_IRUGO),
	TCPC_DEVICE_ATTR(timer, S_IRUGO | S_IWUSR),
	TCPC_DEVICE_ATTR(caps_info, S_IRUGO),
	TCPC_DEVICE_ATTR(cc_orient_info, S_IRUGO),
	TCPC_DEVICE_ATTR(remote_rp_lvl, S_IRUGO),
	TCPC_DEVICE_ATTR(pd_state, S_IRUGO),
};

enum {
	TCPC_DESC_ROLE_DEF = 0,
	TCPC_DESC_RP_LEVEL,
	TCPC_DESC_PD_TEST,
	TCPC_DESC_INFO,
	TCPC_DESC_TIMER,
	TCPC_DESC_CAP_INFO,
	TCPC_DESC_CC_ORIENT_INFO,
	TCPC_DESC_REMOTE_RP_LEVEL,
	TCPC_DESC_PD_STATE,
};

static struct attribute *__tcpc_attrs[ARRAY_SIZE(tcpc_device_attributes) + 1];
static struct attribute_group tcpc_attr_group = {
	.attrs = __tcpc_attrs,
};

static const struct attribute_group *tcpc_attr_groups[] = {
	&tcpc_attr_group,
	NULL,
};

static const char * const role_text[] = {
	"Unknown",
	"SNK Only",
	"SRC Only",
	"DRP",
	"Try.SRC",
	"Try.SNK",
};

#define MAX_PROP_LEN (PAGE_SIZE)

static void tcpc_show_cap_info(struct tcpc_device *tcpc, char *buf)
{
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
#define CHECK_SIZE(_size) do { if (_size >= MAX_PROP_LEN) return; } while(0)
	int vmin, vmax, ioper;
	int i = 0;
	unsigned long int size = 0;

	size += snprintf_s(buf, MAX_PROP_LEN - size,
			MAX_PROP_LEN - size - 1,
			"%s = %d\n%s = %d\n",
			"local_selected_cap",
			tcpc->pd_port.local_selected_cap,
			"remote_selected_cap",
			tcpc->pd_port.remote_selected_cap);

	CHECK_SIZE(size);

	size += snprintf_s(buf + size, MAX_PROP_LEN - size,
			MAX_PROP_LEN - size - 1,
			"%s\n",
			"local_src_cap(vmin, vmax, ioper)");

	CHECK_SIZE(size);

	for (i = 0; i < tcpc->pd_port.local_src_cap.nr; i++) {
		hisi_pd_extract_pdo_power(
			tcpc->pd_port.local_src_cap.pdos[i],
			&vmin, &vmax, &ioper);
		size += snprintf_s(buf + size, MAX_PROP_LEN - size,
			MAX_PROP_LEN - size - 1,
			"%d %d %d\n",
			vmin, vmax, ioper);

		CHECK_SIZE(size);
	}

	size += snprintf_s(buf + size, MAX_PROP_LEN - size,
			MAX_PROP_LEN - size - 1,
			"%s\n",
			"local_snk_cap(vmin, vmax, ioper)");

	CHECK_SIZE(size);

	for (i = 0; i < tcpc->pd_port.local_snk_cap.nr; i++) {
		hisi_pd_extract_pdo_power(
			tcpc->pd_port.local_snk_cap.pdos[i],
			&vmin, &vmax, &ioper);
		size += snprintf_s(buf + size, MAX_PROP_LEN - size,
			MAX_PROP_LEN - size - 1,
			"%d %d %d\n",
			vmin, vmax, ioper);

		CHECK_SIZE(size);
	}

	size += snprintf_s(buf + size, MAX_PROP_LEN - size,
			MAX_PROP_LEN - size - 1,
			"%s\n",
			"remote_src_cap(vmin, vmax, ioper)");
	CHECK_SIZE(size);

	for (i = 0; i < tcpc->pd_port.remote_src_cap.nr; i++) {
		hisi_pd_extract_pdo_power(
			tcpc->pd_port.remote_src_cap.pdos[i],
			&vmin, &vmax, &ioper);
		size += snprintf_s(buf + size, MAX_PROP_LEN - size,
			MAX_PROP_LEN - size - 1,
			"%d %d %d\n",
			vmin, vmax, ioper);

		CHECK_SIZE(size);
	}
	size += snprintf_s(buf + size, MAX_PROP_LEN - size - 1,
			MAX_PROP_LEN - size,
			"%s\n",
			"remote_snk_cap(vmin, vmax, ioper)");
	CHECK_SIZE(size);

	for (i = 0; i < tcpc->pd_port.remote_snk_cap.nr; i++) {
		hisi_pd_extract_pdo_power(
			tcpc->pd_port.remote_snk_cap.pdos[i],
			&vmin, &vmax, &ioper);
		size += snprintf_s(buf + size, MAX_PROP_LEN - size - 1,
			MAX_PROP_LEN - size,
			"%d %d %d\n",
			vmin, vmax, ioper);

		CHECK_SIZE(size);
	}
#endif
}

extern char *pd_pe_state_name(uint8_t pe_state);

static void tcpc_show_pd_state(struct tcpc_device *tcpc, char *buf)
{
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	snprintf_s(buf, MAX_PROP_LEN, MAX_PROP_LEN - 1,
		"pe_pd_state %s\n" "pe_vdm_state %s\n"
		"pe_state_curr %s\n" "pe_state_next %s\n",
		pd_pe_state_name(tcpc->pd_port.pe_pd_state),
		pd_pe_state_name(tcpc->pd_port.pe_vdm_state),
		pd_pe_state_name(tcpc->pd_port.pe_state_curr),
		pd_pe_state_name(tcpc->pd_port.pe_state_next));
#endif
}

static ssize_t tcpc_show_property(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct tcpc_device *tcpc = to_tcpc_device(dev);
	const ptrdiff_t offset = attr - tcpc_device_attributes;
	int size = 0;
	uint8_t cc1 = 0, cc2 = 0;
	bool from_ic = true;

	switch (offset) {
	case TCPC_DESC_CC_ORIENT_INFO:
		snprintf_s(buf, MAX_PROP_LEN, MAX_PROP_LEN - 1,
			"%s\n", tcpc->typec_polarity ? "2" : "1");
		break;
	case TCPC_DESC_CAP_INFO:
		tcpc_show_cap_info(tcpc, buf);
		break;
	case TCPC_DESC_ROLE_DEF:
		snprintf_s(buf, MAX_PROP_LEN, MAX_PROP_LEN - 1, "%s\n"
			"1:sink, 2:source, 3:drp, 4:try source, 5:try sink\n",
			role_text[tcpc->typec_role]);
		break;
	case TCPC_DESC_RP_LEVEL:
		if (tcpc->typec_local_rp_level == TYPEC_CC_RP_DFT)
			snprintf_s(buf, MAX_PROP_LEN, MAX_PROP_LEN - 1, "%s\n", "Default");
		else if (tcpc->typec_local_rp_level == TYPEC_CC_RP_1_5)
			snprintf_s(buf, MAX_PROP_LEN, MAX_PROP_LEN - 1, "%s\n", "1.5");
		else if (tcpc->typec_local_rp_level == TYPEC_CC_RP_3_0)
			snprintf_s(buf, MAX_PROP_LEN, MAX_PROP_LEN - 1, "%s\n", "3.0");
		break;
	case TCPC_DESC_REMOTE_RP_LEVEL:
		(void)hisi_tcpm_inquire_remote_cc(tcpc, &cc1, &cc2, from_ic);
		snprintf_s(buf, MAX_PROP_LEN, MAX_PROP_LEN - 1, " cc1 %s cc2 %s\n",
				tcpm_cc_voltage_status_string(cc1),
				tcpm_cc_voltage_status_string(cc2));

		break;
	case TCPC_DESC_PD_TEST:
		snprintf_s(buf, MAX_PROP_LEN, MAX_PROP_LEN - 1, "%s\n%s\n%s\n%s\n%s\n",
				"1: Power Role Swap Test",
				"2: Data Role Swap Test",
				"3: Vconn Swap Test",
				"4: soft reset",
				"5: hard reset");
		break;
	case TCPC_DESC_INFO:
		size += snprintf_s(buf, MAX_PROP_LEN, MAX_PROP_LEN - 1,
					"role = %s\n", role_text[tcpc->desc.role_def]);
		snprintf_s(buf + size, MAX_PROP_LEN - size, MAX_PROP_LEN - size - 1,
			"rplvl = %s\n", tcpm_cc_voltage_status_string(cc2));
		break;
	case TCPC_DESC_PD_STATE:
		tcpc_show_pd_state(tcpc, buf);
		break;
	default:
		break;
	}

	return strnlen(buf, MAX_PROP_LEN);
}

static int get_parameters(char *buf, long int *param1, int num_of_par)
{
	char *token;
	int base, cnt;

	token = strsep(&buf, " ");
	for (cnt = 0; cnt < num_of_par; cnt++) {
		if (token != NULL) {
			if ((token[1] == 'x') || (token[1] == 'X'))
				base = 16;
			else
				base = 10;

			if (kstrtoul(token, base, &param1[cnt]) != 0)
				return -EINVAL;

			token = strsep(&buf, " ");
			}
		else
			return -EINVAL;
	}

	return 0;
}

static void tcpc_init_attrs(struct device_type *dev_type)
{
	unsigned int i;

	D("+\n");
	dev_type->groups = tcpc_attr_groups;
	for (i = 0; i < ARRAY_SIZE(tcpc_device_attributes); i++)
		__tcpc_attrs[i] = &tcpc_device_attributes[i].attr;
	D("-\n");
}

static void tcpc_enable_timer(struct tcpc_device *tcpc, uint32_t timer_id)
{
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
		if (timer_id > 0 && timer_id <= PD_PE_TIMER_END_ID)
			pd_enable_timer(&tcpc->pd_port, timer_id);
		else if (timer_id > PD_PE_TIMER_END_ID && timer_id < PD_TIMER_NR)
			hisi_tcpc_enable_timer(tcpc, timer_id);
#else
		if (timer_id > 0 && timer_id < PD_TIMER_NR)
			hisi_tcpc_enable_timer(tcpc, timer_id);
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */
}

static void tcpc_pd_test(struct tcpc_device *tcpc, unsigned int test_id)
{
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	struct tcpm_power_cap cap;
	switch (test_id) {
	case 1: /* Power Role Swap */
		hisi_tcpm_power_role_swap(tcpc);
		break;
	case 2: /* Data Role Swap */
		hisi_tcpm_data_role_swap(tcpc);
		break;
	case 3: /* Vconn Swap */
		hisi_tcpm_vconn_swap(tcpc);
		break;
	case 4: /* Software Reset */
		hisi_tcpm_soft_reset(tcpc);
		break;
	case 5: /* Hardware Reset */
		hisi_tcpm_hard_reset(tcpc);
		break;
	case 6:
		hisi_tcpm_get_source_cap(tcpc, &cap);
		break;
	case 7:
		hisi_tcpm_get_sink_cap(tcpc, &cap);
		break;
	case 8:
		hisi_tcpc_typec_force_unattach(tcpc);
		break;
	default:
		break;
	}
#endif
}

static void tcpc_set_local_rp_level(struct tcpc_device *tcpc, unsigned level)
{
	switch (level) {
	case 0:
		tcpc->typec_local_rp_level = TYPEC_CC_RP_DFT;
		break;
	case 1:
		tcpc->typec_local_rp_level = TYPEC_CC_RP_1_5;
		break;
	case 2:
		tcpc->typec_local_rp_level = TYPEC_CC_RP_3_0;
		break;
	default:
		break;
	}
}
static ssize_t tcpc_store_property(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct tcpc_device *tcpc = to_tcpc_device(dev);
	const ptrdiff_t offset = attr - tcpc_device_attributes;
	int ret;
	long int val = 0;

	D("+\n");
	switch (offset) {
	case TCPC_DESC_ROLE_DEF:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_err(dev, "get parameters fail\n");
			return -EINVAL;
		}

		D("to hisi_tcpm_typec_change_role\n");
		(void)hisi_tcpm_typec_change_role(tcpc, val);
		break;
	case TCPC_DESC_TIMER:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_err(dev, "get parameters fail\n");
			return -EINVAL;
		}

		tcpc_enable_timer(tcpc, (uint32_t)val);

		break;

	case TCPC_DESC_PD_TEST:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_err(dev, "get parameters fail\n");
			return -EINVAL;
		}
		tcpc_pd_test(tcpc, (unsigned int)val);
		break;

	case TCPC_DESC_RP_LEVEL:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_err(dev, "get parameters fail\n");
			return -EINVAL;
		}

		tcpc_set_local_rp_level(tcpc, (unsigned int)val);

		break;

	default:
		break;
	}
	D("-\n");
	return count;
}

#else
static inline void tcpc_init_attrs(struct device_type *dev_type){}
#endif

static int tcpc_match_device_by_name(struct device *dev, const void *data)
{
	const char *name = data;
	struct tcpc_device *tcpc = dev_get_drvdata(dev);

	return strcmp(tcpc->desc.name, name) == 0;
}

struct tcpc_device *hisi_tcpc_dev_get_by_name(const char *name)
{
	struct device *dev;

	D("+-\n");

	dev = class_find_device(hisi_tcpc_class,
			NULL, (const void *)name, tcpc_match_device_by_name);

	return dev ? dev_get_drvdata(dev) : NULL;
}


static void tcpc_device_release(struct device *dev)
{
	struct tcpc_device *tcpc_dev = to_tcpc_device(dev);

	D("+\n");
	/* Un-init pe thread */
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	hisi_tcpci_event_deinit(tcpc_dev);
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

	/* Un-init timer thread */
	hisi_tcpci_timer_deinit(tcpc_dev);
	/* Un-init Mutex */
	/* Do initialization */
	devm_kfree(dev, tcpc_dev);
	D("-\n");
}

struct tcpc_device *hisi_tcpc_device_register(struct device *parent,
	    struct tcpc_desc *tcpc_desc, struct tcpc_ops *ops, void *drv_data)
{
	struct tcpc_device *tcpc;
	int ret = 0;

	D("+\n");

	tcpc = devm_kzalloc(parent, sizeof(*tcpc), GFP_KERNEL);
	if (!tcpc) {
		pr_err("%s : allocate tcpc memeory failed\n", __func__);
		return NULL;
	}

	tcpc->dev.class = hisi_tcpc_class;
	tcpc->dev.type = &tcpc_dev_type;
	tcpc->dev.parent = parent;
	tcpc->dev.release = tcpc_device_release;
	dev_set_drvdata(&tcpc->dev, tcpc);
	tcpc->drv_data = drv_data;
	dev_set_name(&tcpc->dev, tcpc_desc->name);
	tcpc->desc = *tcpc_desc;
	tcpc->ops = ops;
	tcpc->typec_local_rp_level = tcpc_desc->rp_lvl;
	D("typec_local_rp_level %d\n", tcpc->typec_local_rp_level);

	D("to devices_register\n");
	ret = device_register(&tcpc->dev);
	if (ret) {
		devm_kfree(parent, tcpc);
		return ERR_PTR(ret);
	}

	srcu_init_notifier_head(&tcpc->evt_nh);

	mutex_init(&tcpc->access_lock);
	mutex_init(&tcpc->typec_lock);
	mutex_init(&tcpc->timer_lock);
	sema_init(&tcpc->timer_enable_mask_lock, 1);
	spin_lock_init(&tcpc->timer_tick_lock);

	/* If system support "WAKE_LOCK_IDLE",
	 * please use it instead of "WAKE_LOCK_SUSPEND" */
	wake_lock_init(&tcpc->attach_wake_lock, WAKE_LOCK_SUSPEND,
			"hisi_tcpc_attach_wakelock");
	wake_lock_init(&tcpc->dettach_temp_wake_lock, WAKE_LOCK_SUSPEND,
			"hisi_tcpc_detach_wakelock");

	D("to timer_init\n");
	hisi_tcpci_timer_init(tcpc);

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	hisi_tcpci_event_init(tcpc);
	hisi_pd_core_init(tcpc);
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

#ifdef CONFIG_DUAL_ROLE_USB_INTF
	ret = hisi_tcpc_dual_role_phy_init(tcpc);
	if (ret < 0)
		dev_err(&tcpc->dev, "dual role usb init fail\n");
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

	D("-\n");
	return tcpc;
}
EXPORT_SYMBOL(hisi_tcpc_device_register);


int hisi_tcpc_tcpci_init(struct tcpc_device *tcpc, bool sw_reset)
{
	int ret;
	uint16_t power_status = 0;

	D("+\n");
	if (tcpc->ops->init) {
		ret = tcpc->ops->init(tcpc, sw_reset);
		if (ret)
			return ret;
	} else {
		pr_err("%s Please implment tcpc ops init function\n", __func__);
		return -EINVAL;
	}

	ret = tcpci_get_power_status(tcpc, &power_status);
	if (ret)
		return ret;
	D("power_status 0x%x\n", power_status);
	hisi_tcpci_vbus_level_init(tcpc, power_status);
	D("-\n");
	return 0;
}


static int tcpc_device_irq_enable(struct tcpc_device *tcpc)
{
	int ret;

	D("+\n");

	ret = hisi_tcpc_tcpci_init(tcpc, false);
	if (ret < 0) {
		pr_err("%s tcpc init fail\n", __func__);
		return ret;
	}

	hisi_tcpci_lock_typec(tcpc);
	ret = hisi_tcpc_typec_init(tcpc, tcpc->desc.role_def);
	hisi_tcpci_unlock_typec(tcpc);

	if (ret < 0) {
		pr_err("%s : tcpc typec init fail\n", __func__);
		return ret;
	}

	D("-\n");
	return 0;
}

int hisi_tcpc_schedule_init_work(struct tcpc_device *tcpc)
{
	D("\n");
	return tcpc_device_irq_enable(tcpc);
}
EXPORT_SYMBOL(hisi_tcpc_schedule_init_work);

int hisi_register_tcp_dev_notifier(struct tcpc_device *tcp_dev,
			      struct notifier_block *nb)
{
	int ret;

	D("+\n");
	ret = srcu_notifier_chain_register(&tcp_dev->evt_nh, nb);
	if (ret != 0)
		D("srcu_notifier_chain_register ret:%d\n", ret);

	D("-\n");
	return ret;
}
EXPORT_SYMBOL(hisi_register_tcp_dev_notifier);

int hisi_unregister_tcp_dev_notifier(struct tcpc_device *tcp_dev,
				struct notifier_block *nb)
{
	return srcu_notifier_chain_unregister(&tcp_dev->evt_nh, nb);
}
EXPORT_SYMBOL(hisi_unregister_tcp_dev_notifier);


void hisi_tcpc_device_unregister(struct device *dev, struct tcpc_device *tcpc)
{
	D("+\n");
	if (!tcpc)
		return;

	hisi_tcpc_typec_deinit(tcpc);

	wake_lock_destroy(&tcpc->dettach_temp_wake_lock);
	wake_lock_destroy(&tcpc->attach_wake_lock);

#ifdef CONFIG_DUAL_ROLE_USB_INTF
	devm_dual_role_instance_unregister(&tcpc->dev, tcpc->dr_usb);
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

	device_unregister(&tcpc->dev);
	D("-\n");
}
EXPORT_SYMBOL(hisi_tcpc_device_unregister);

static int __init hisi_tcpc_class_init(void)
{
	D("+\n");
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	hisi_dpm_check_supported_modes();
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

	hisi_tcpc_class = class_create(THIS_MODULE, "hisi_tcpc_class");
	if (IS_ERR(hisi_tcpc_class)) {
		pr_info("Unable to create tcpc class; errno = %ld\n", PTR_ERR(hisi_tcpc_class));
		return PTR_ERR(hisi_tcpc_class);
	}
	tcpc_init_attrs(&tcpc_dev_type);
	hisi_tcpc_class->suspend = NULL;
	hisi_tcpc_class->resume = NULL;

	D("-\n");
	return 0;
}

static void __exit hisi_tcpc_class_exit(void)
{
	D("\n");
	class_destroy(hisi_tcpc_class);
}

subsys_initcall(hisi_tcpc_class_init);
module_exit(hisi_tcpc_class_exit);

MODULE_DESCRIPTION("hisilicon tcpc");
MODULE_AUTHOR("hisilicon");
MODULE_LICENSE("GPL");
