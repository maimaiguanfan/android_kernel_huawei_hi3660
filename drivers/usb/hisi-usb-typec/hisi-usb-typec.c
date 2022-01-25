/*
 * drivers/usb/hisi-usb-typec/hisi-usb-typec.c - Hisilicon USB Type-C framework.
 *
 * Copyright (C) 2018 Hisilicon
 * Author: Hisillicon <>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/fb.h>
#include <linux/debugfs.h>

#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/audio/usb_analog_hs_interface.h>
#include <huawei_platform/power/boost_5v.h>
#include <huawei_platform/audio/usb_audio_power.h>
#include <huawei_platform/dp_aux_switch/dp_aux_switch.h>

#include <linux/hisi/usb/hisi_hifi_usb.h>
#include <linux/hisi/contexthub/tca.h>
#include <linux/hisi/usb/hisi_tcpm.h>
#include "hisi-usb-typec.h"

#include "securec.h"

#define USB_EVENT_INTERVAL_MS 20
#define HIGH_POWER_VALUE (9000 * 2000)
#define HIGH_VOLTAGE_VALUE 9000

#ifdef CONFIG_HISI_USB_TYPEC_DBG
#define D(format,arg...) pr_info("[hisitypec][DBG][%s]" format, __func__, ##arg)
#else
#define D(format,arg...) do {} while(0)
#endif
#define I(format,arg...) pr_info("[hisitypec][INF][%s]" format, __func__, ##arg)
#define E(format,arg...) pr_err("[hisitypec][ERR][%s]" format, __func__, ##arg)

struct usb_typec {
	struct platform_device *pdev;
	struct tcpc_device *tcpc_dev;

	struct mutex lock;
	struct tcp_ny_vbus_state vbus_state;
	struct tcp_ny_typec_state typec_state;
	struct tcp_ny_pd_state pd_state;
	struct tcp_ny_mode_ctrl mode_ctrl;
	struct tcp_ny_ama_dp_state ama_dp_state;
	struct tcp_ny_ama_dp_attention ama_dp_attention;
	struct tcp_ny_ama_dp_hpd_state ama_dp_hpd_state;
	struct tcp_ny_uvdm uvdm_msg;

	uint16_t dock_svid;
	int rt_vbus_state;
	int rt_vconn_state;

	int power_role;		/* PD_ROLE_SINK or PD_ROLE_SOURCE*/
	int data_role;		/* PD_ROLE_UFP or PD_ROLE_DFP */
	int vconn;			/* PD_ROLE_VCONN_OFF or PD_ROLE_VCONN_ON */
	int audio_accessory;	/* 0 or 1 */

	bool direct_charge_cable;
	bool direct_charging;
	int pd_adapter_voltage;	/* Record adapter voltage setting. */
	unsigned int bc12_type;

	uint8_t rt_cc1;
	uint8_t rt_cc2;
	unsigned long time_stamp_cc_alert;
	unsigned long time_stamp_typec_attached;
	unsigned long time_stamp_pd_attached;

	/* for handling tcpc notify */
	struct notifier_block tcpc_nb;
	struct list_head tcpc_notify_list;
	spinlock_t tcpc_notify_list_lock;
	unsigned tcpc_notify_count;
	struct work_struct tcpc_notify_work;
	struct workqueue_struct *hisi_typec_wq;

	/* ? */
	TCPC_MUX_CTRL_TYPE dp_mux_type;

	struct dentry *debugfs_root;

	struct notifier_block wakelock_control_nb;
	struct notifier_block bc12_nb;

	unsigned suspend_count;
	unsigned resume_count;
};

static struct usb_typec *_typec;

#ifdef CONFIG_HISI_DEBUG_FS
static inline char *charger_type_string(int charger_type)
{
	switch(charger_type) {
	case CHARGER_TYPE_NONE: return "none";
	case CHARGER_TYPE_SDP: return "sdp";
	case CHARGER_TYPE_DCP: return "dcp";
	case CHARGER_TYPE_CDP: return "cdp";
	case CHARGER_TYPE_UNKNOWN: return "unknown";
	case PLEASE_PROVIDE_POWER: return "sourcing vbus";
	default: return "unknown value";
	}
}
static inline char *remote_rp_level_string(unsigned char rp_level)
{
	switch(rp_level) {
	case TYPEC_CC_VOLT_OPEN: return "open";
	case TYPEC_CC_VOLT_RA: return "Ra";
	case TYPEC_CC_VOLT_RD: return "Rd";
	case TYPEC_CC_VOLT_SNK_DFT: return "Rp_Default";
	case TYPEC_CC_VOLT_SNK_1_5: return "Rp_1.5A";
	case TYPEC_CC_VOLT_SNK_3_0: return "Rp_3.0A";
	default: return "illegal value";
	}
}

static int usb_typec_status_show(struct seq_file *s, void *data)
{
	struct usb_typec *typec = s->private;

	if (!typec)
		return -ENOENT;


	seq_printf(s, "power_role             %s\n",
			(typec->power_role == PD_ROLE_SOURCE) ? "SOURCE" :
			(typec->power_role == PD_ROLE_SINK) ? "PD_SINK" : "SINK");
	seq_printf(s, "data_role              %s\n",
			(typec->data_role == PD_ROLE_UFP) ? "UFP" : "DFP");
	seq_printf(s, "vconn                  %s\n", typec->vconn ? "Y" : "N");
	seq_printf(s, "audio_accessory        %s\n", typec->audio_accessory ? "Y" : "N");



	seq_printf(s, "orient                 %s\n", typec->typec_state.polarity ? "fliped" : "normal");
	seq_printf(s, "chargerType            %s\n", charger_type_string(typec->bc12_type));



	seq_printf(s, "tcpc_notify_count      %u\n", typec->tcpc_notify_count);
	seq_printf(s, "dp_mux_type            %d\n", typec->dp_mux_type);

	seq_printf(s, "direct_charge_cable    %s\n", typec->direct_charge_cable ? "Y" : "N");
	seq_printf(s, "direct_charging        %s\n", typec->direct_charging ? "Y" : "N");
	seq_printf(s, "suspend_count          %u\n", typec->suspend_count);
	seq_printf(s, "resume_count           %u\n", typec->resume_count);

	seq_printf(s, "remote rp level        %s\n",
			remote_rp_level_string(typec->typec_state.rp_level));
	seq_printf(s, "dock_svid              0x%04x\n", typec->dock_svid);
	seq_printf(s, "pd_adapter_voltage     %d\n", typec->pd_adapter_voltage);

	seq_printf(s, "vbus.ma                %d\n", typec->vbus_state.ma);
	seq_printf(s, "vbus.mv                %d\n", typec->vbus_state.mv);
	seq_printf(s, "vbus.type              0x%x\n", typec->vbus_state.type);

	return 0;
}

static int usb_typec_status_open(struct inode *inode, struct file *file)
{
	return single_open(file, usb_typec_status_show, inode->i_private);
}

static const struct file_operations usb_typec_status_fops = {
	.open			= usb_typec_status_open,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static int usb_typec_perf_show(struct seq_file *s, void *data)
{
	struct usb_typec *typec = s->private;

	if (!typec)
		return -ENOENT;

	if (typec->data_role == PD_ROLE_DFP) {
		seq_printf(s, "attached.source     %d ms\n",
			jiffies_to_msecs(typec->time_stamp_typec_attached
					- typec->time_stamp_cc_alert));
	} else if (typec->data_role == PD_ROLE_UFP) {
		seq_printf(s, "attached.sink       %d ms\n",
			jiffies_to_msecs(typec->time_stamp_typec_attached
					- typec->time_stamp_cc_alert));
	}

	if (typec->pd_state.connected != HISI_PD_CONNECT_NONE) {
		seq_printf(s, "pd connect(%d)      %d ms\n",
			typec->pd_state.connected,
			jiffies_to_msecs(typec->time_stamp_pd_attached
					- typec->time_stamp_cc_alert));
	}

	return 0;
}

static int usb_typec_perf_open(struct inode *inode, struct file *file)
{
	return single_open(file, usb_typec_perf_show, inode->i_private);
}

static const struct file_operations usb_typec_perf_fops = {
	.open			= usb_typec_perf_open,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static int usb_typec_cable_show(struct seq_file *s, void *data)
{
#define PD_VDO_MAX_SIZE 7

	struct usb_typec *typec = s->private;
	uint32_t vdos[PD_VDO_MAX_SIZE] = {0};
	int i;
	int ret;

	if (!typec)
		return -ENOENT;

	ret = hisi_tcpm_discover_cable(typec->tcpc_dev, vdos, PD_VDO_MAX_SIZE);
	if (ret != TCPM_SUCCESS) {
		E("hisi_tcpm_discover_cable error ret %d\n", ret);
		return 0;
	}

	for (i = 0; i < PD_VDO_MAX_SIZE; i++) {
		seq_printf(s, "vdo[%d] 0x%08x\n", i, vdos[i]);
	}

	return 0;
}

static int usb_typec_cable_open(struct inode *inode, struct file *file)
{
	return single_open(file, usb_typec_cable_show, inode->i_private);
}

static const struct file_operations usb_typec_cable_fops = {
	.open			= usb_typec_cable_open,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static int usb_typec_debugfs_init(struct usb_typec *typec)
{
	struct dentry		*root;
	struct dentry		*file;

	root = debugfs_create_dir("hisi_usb_typec", usb_debug_root);
	if (IS_ERR_OR_NULL(root))
		return -ENOMEM;

	file = debugfs_create_file("status", S_IRUGO, root,
				typec, &usb_typec_status_fops);
	if (!file)
		goto err;

	file = debugfs_create_file("perf", S_IRUGO, root,
				typec, &usb_typec_perf_fops);
	if (!file)
		goto err;

	file = debugfs_create_file("cable", S_IRUGO, root,
				typec, &usb_typec_cable_fops);
	if (!file)
		goto err;

	typec->debugfs_root = root;
	return 0;

err:
	E("usb_typec_debugfs_init failed\n");
	debugfs_remove_recursive(root);
	return -ENOMEM;
}

static void usb_typec_debugfs_exit(struct usb_typec *typec)
{
	if (!typec->debugfs_root)
		return;

	debugfs_remove_recursive(typec->debugfs_root);
}
#else
static inline int usb_typec_debugfs_init(struct usb_typec *typec){return 0;}
static inline void usb_typec_debugfs_exit(struct usb_typec *typec){}
#endif

static void handle_typec_unattached(struct usb_typec *typec,
		struct tcp_ny_typec_state *typec_state)
{
	D("+\n");

	if (typec->audio_accessory) {
		typec->audio_accessory = 0;
	}

	if (typec->data_role == PD_ROLE_UFP) {
		typec->data_role = PD_ROLE_UNATTACHED;
	} else if (typec->data_role == PD_ROLE_DFP) {
		typec->data_role = PD_ROLE_UNATTACHED;
	}

	typec->power_role = PD_ROLE_UNATTACHED;

	typec->direct_charge_cable = false;
	typec->direct_charging = false;

	D("-\n");
}

static void handle_typec_attached_sink(struct usb_typec *typec,
		struct tcp_ny_typec_state *typec_state)
{
	D("+\n");

	if (typec->data_role == PD_ROLE_UFP) {
		D("Already UFP\n");
		return;
	}

	typec->data_role = PD_ROLE_UFP;

	D("-\n");
}

static void handle_typec_attached_source(struct usb_typec *typec,
		struct tcp_ny_typec_state *typec_state)
{
	D("+\n");

	if (typec->data_role == PD_ROLE_DFP) {
		D("Already DFP\n");
		return;
	}

	typec->data_role = PD_ROLE_DFP;

	D("-\n");
}

static void handle_typec_attached_audio_accessory(struct usb_typec *typec,
		struct tcp_ny_typec_state *typec_state)
{
	D("+\n");
	typec->audio_accessory = true;
	D("-\n");
}

static void handle_typec_attached_debug_accessory(struct usb_typec *typec,
		struct tcp_ny_typec_state *typec_state)
{
	D("+\n");
	D("-\n");
}

static void handle_typec_attached_debug_accessory_sink(struct usb_typec *typec,
		struct tcp_ny_typec_state *typec_state)
{
	D("+\n");
	typec->data_role = PD_ROLE_UFP;
	D("-\n");
}

/**
 * hisi_usb_typec_state_change() - Handle typec state change event - [for debug].
 */
void hisi_usb_typec_state_change(struct tcp_ny_typec_state *typec_state)
{
	struct usb_typec *typec = _typec;

	D("+\n");

	mutex_lock(&typec->lock);
	I("tyec_state: %s --> %s / %s / %s\n",
		typec_attach_type_name(typec_state->old_state),
		typec_attach_type_name(typec_state->new_state),
		typec_state->polarity ? "fliped" : "normal",
		tcpm_cc_voltage_status_string(typec_state->rp_level));

	/* Save typec_state for futher use. */
	memcpy_s(&typec->typec_state, sizeof(struct tcp_ny_typec_state),
		typec_state, sizeof(typec->typec_state));

	switch (typec_state->new_state) {
	case TYPEC_UNATTACHED:
		handle_typec_unattached(typec, typec_state);
		break;
	case TYPEC_ATTACHED_SNK:
		handle_typec_attached_sink(typec, typec_state);
		break;
	case TYPEC_ATTACHED_SRC:
		handle_typec_attached_source(typec, typec_state);
		break;
	case TYPEC_ATTACHED_AUDIO:
		handle_typec_attached_audio_accessory(typec, typec_state);
		break;
	case TYPEC_ATTACHED_DEBUG:
		handle_typec_attached_debug_accessory(typec, typec_state);
		break;
	case TYPEC_ATTACHED_DBGACC_SNK:
	case TYPEC_ATTACHED_CUSTOM_SRC:
		handle_typec_attached_debug_accessory_sink(typec, typec_state);
		break;
	default:
		E("unknown new_sate %d\n", typec_state->new_state);
		break;
	}

	D("update time_stamp_typec_attached\n");
	typec->time_stamp_typec_attached = jiffies;

	mutex_unlock(&typec->lock);
	D("-\n");
}

/**
 * hisi_usb_typec_pd_state_change() - Handle PD state change, save PD state for debug.
 *
 * pd_state:          Should be one of hisi_pd_connect_result.
 */
void hisi_usb_typec_pd_state_change(struct tcp_ny_pd_state *pd_state)
{
	struct usb_typec *typec = _typec;

	D("+\n");
	mutex_lock(&typec->lock);
	I("pd connect state: %d\n", pd_state->connected);
	typec->pd_state.connected = pd_state->connected;

	typec->time_stamp_pd_attached = jiffies;

	mutex_unlock(&typec->lock);
	D("update time_stamp_pd_attached\n");
	D("-\n");
}

/**
 * hisi_usb_typec_data_role_swap() - Handle data role swap event - [for debug].
 */
void hisi_usb_typec_data_role_swap(u8 role)
{
	struct usb_typec *typec = _typec;

	D("+\n");
	I("\n");
	mutex_lock(&typec->lock);

	if (typec->data_role == PD_ROLE_UNATTACHED) {
		E("Unattached!\n");
		goto done;
	}

	if (typec->data_role == role) {
		D("Data role not chage!\n");
		goto done;
	}

	I("new role: %s", role == PD_ROLE_DFP ? "PD_ROLE_DFP" : "PD_ROLE_UFP");
	if (role == PD_ROLE_DFP) {
		typec->data_role = PD_ROLE_DFP;
	} else {
		typec->data_role = PD_ROLE_UFP;
	}
done:
	mutex_unlock(&typec->lock);
	D("-\n");
}

/**
 * hisi_usb_typec_set_vconn() - Turn on/off vconn power.
 * enable:          0 - disable, 1 - enable
 */
void hisi_usb_typec_set_vconn(int enable)
{
	struct usb_typec *typec = _typec;

	D("+\n");
	typec->rt_vconn_state = enable;
#ifdef CONFIG_TCPC_CLASS
	pd_dpm_handle_pe_event(PD_DPM_PE_EVT_SOURCE_VCONN, &enable);
#endif
	D("-\n");
}

/**
 * hisi_usb_typec_source_vbus() - Handle source vbus operation for debug
 */
void hisi_usb_typec_source_vbus(struct tcp_ny_vbus_state *vbus_state)
{
	struct usb_typec *typec = _typec;

	D("+\n");
	mutex_lock(&typec->lock);
	I("vbus_state: %d %d 0x%02x\n", vbus_state->ma, vbus_state->mv, vbus_state->type);

	/*
	 * Must save vbus_state first.
	 */
	memcpy_s(&typec->vbus_state, sizeof(struct tcp_ny_vbus_state),
		vbus_state, sizeof(typec->vbus_state));

	D("power role: %d, data role: %d\n", typec->power_role, typec->data_role);

	if (vbus_state->mv != 0) {
		typec->power_role = PD_ROLE_SOURCE;
	}

	mutex_unlock(&typec->lock);
	D("-\n");
}

/**
 * hisi_usb_typec_sink_vbus() - Handle sink vbus operation [for debug].
 */
void hisi_usb_typec_sink_vbus(struct tcp_ny_vbus_state *vbus_state)
{
	struct usb_typec *typec = _typec;

	D("+\n");
	mutex_lock(&typec->lock);
	I("vbus_state: %d %d 0x%02x\n", vbus_state->ma, vbus_state->mv, vbus_state->type);

	/* save vbus_state. */
	typec->vbus_state.ma = vbus_state->ma;
	typec->vbus_state.mv = vbus_state->mv;
	typec->vbus_state.type = vbus_state->type;
	I("vbus_state: %d %d 0x%02x\n", vbus_state->ma, vbus_state->mv, vbus_state->type);
	D("power role: %d, data role: %d\n", typec->power_role, typec->data_role);

	if (vbus_state->mv != 0) {
		typec->power_role = PD_ROLE_SINK;
	}

	mutex_unlock(&typec->lock);
	D("-\n");
}

/**
 * hisi_usb_typec_disable_vbus_control() - [for debug].
 */
void hisi_usb_typec_disable_vbus_control(struct tcp_ny_vbus_state *vbus_state)
{
	struct usb_typec *typec = _typec;

	D("+\n");
	mutex_lock(&typec->lock);

	typec->vbus_state.ma = vbus_state->ma;
	typec->vbus_state.mv = vbus_state->mv;
	typec->vbus_state.type = vbus_state->type;
	I("vbus_state: %d %d 0x%02x\n", vbus_state->ma, vbus_state->mv, vbus_state->type);

	typec->power_role = PD_ROLE_UNATTACHED;
	mutex_unlock(&typec->lock);
	D("-\n");
}

void hisi_usb_pd_dp_state_change(struct tcp_ny_ama_dp_state *ama_dp_state)
{
	struct usb_typec *typec = _typec;
	int ret;
#ifdef CONFIG_TCPC_CLASS
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
#endif
#endif

	D("+\n");
	D("%u %u 0x%x %u %u\n", ama_dp_state->sel_config, ama_dp_state->signal,
			ama_dp_state->pin_assignment, ama_dp_state->polarity,
			ama_dp_state->active);

	/* add aux switch */
	dp_aux_switch_op(ama_dp_state->polarity);

	/* add aux uart switch*/
	dp_aux_uart_switch_enable();

#ifndef MODE_DP_PIN_A
#define MODE_DP_PIN_A 0x01
#define MODE_DP_PIN_B 0x02
#define MODE_DP_PIN_C 0x04
#define MODE_DP_PIN_D 0x08
#define MODE_DP_PIN_E 0x10
#define MODE_DP_PIN_F 0x20
#else
#error
#endif

	mutex_lock(&typec->lock);

	if((MODE_DP_PIN_C == ama_dp_state->pin_assignment)
			|| (MODE_DP_PIN_E == ama_dp_state->pin_assignment))
		typec->dp_mux_type = TCPC_DP;
	else if((MODE_DP_PIN_D == ama_dp_state->pin_assignment)
			|| (MODE_DP_PIN_F == ama_dp_state->pin_assignment))
		typec->dp_mux_type = TCPC_USB31_AND_DP_2LINE;

	mutex_unlock(&typec->lock);

#ifdef CONFIG_TCPC_CLASS
#ifdef CONFIG_CONTEXTHUB_PD
	event.dev_type = TCA_ID_RISE_EVENT;
	event.irq_type = TCA_IRQ_HPD_OUT;
	event.mode_type = TCPC_NC;
	event.typec_orien = ama_dp_state->polarity;

	ret = pd_dpm_handle_combphy_event(event);
	if (ret) {
		E("pd_dpm_handle_combphy_event RISE EVENT ret %d\n", ret);
	}
	pd_dpm_set_combphy_status(typec->dp_mux_type);

	event.dev_type = TCA_ID_FALL_EVENT;
	event.irq_type = TCA_IRQ_HPD_IN;
	event.mode_type = typec->dp_mux_type;
	event.typec_orien = ama_dp_state->polarity;
	ret = pd_dpm_handle_combphy_event(event);
	if (ret) {
		E("pd_dpm_handle_combphy_event FALL_EVENT ret %d\n", ret);
	}
#endif
#endif

	D("-\n");
}

void hisi_usb_pd_dp_hpd_state_change(struct tcp_ny_ama_dp_hpd_state *ama_dp_hpd_state)
{
	struct usb_typec *typec = _typec;
	int ret;
#ifdef CONFIG_TCPC_CLASS
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
#endif
#endif

	D("+\n");
	I("irq %d, state %d\n", ama_dp_hpd_state->irq, ama_dp_hpd_state->state);
	mutex_lock(&typec->lock);

	typec->ama_dp_hpd_state.irq = ama_dp_hpd_state->irq;
	typec->ama_dp_hpd_state.state = ama_dp_hpd_state->state;

	mutex_unlock(&typec->lock);

#ifdef CONFIG_TCPC_CLASS
#ifdef CONFIG_CONTEXTHUB_PD
	event.dev_type = TCA_DP_IN;
	event.irq_type = TCA_IRQ_HPD_IN;
	event.mode_type = typec->dp_mux_type;
	event.typec_orien = typec->typec_state.polarity;


	if (!ama_dp_hpd_state->state) {
		event.dev_type = TCA_DP_OUT;
		event.irq_type = TCA_IRQ_HPD_OUT;
		ret = pd_dpm_handle_combphy_event(event);
		if (ret) {
			E("pd_dpm_handle_combphy_event DP_OUT ret %d\n", ret);
		}
		pd_dpm_set_last_hpd_status(false);

		pd_dpm_send_event(DP_CABLE_OUT_EVENT);
	} else {
		event.dev_type = TCA_DP_IN;
		ret = pd_dpm_handle_combphy_event(event);
		if (ret) {
			E("pd_dpm_handle_combphy_event  TCA_DP_IN ret %d\n", ret);
		}

		pd_dpm_set_last_hpd_status(true);
		pd_dpm_send_event(DP_CABLE_IN_EVENT);
	}

	if (ama_dp_hpd_state->irq) {
		event.irq_type = TCA_IRQ_SHORT;
		ret = pd_dpm_handle_combphy_event(event);
		if (ret) {
			E("pd_dpm_handle_combphy_event IRQ_SHORT ret %d\n", ret);
		}
	}
#endif
#endif

	D("-\n");
}

void hisi_usb_pd_ufp_update_dock_svid(uint16_t svid)
{
	struct usb_typec *typec = _typec;
	D("+\n");
	D("0x%04x\n", svid);
	typec->dock_svid = svid;
	D("-\n");
}

/**
 * hisi_usb_typec_power_status_change() - Recore vbus state, 0 means vbus invalid,
 * 1 mean vbus valid.
 */
void hisi_usb_typec_power_status_change(int vbus_state)
{
	struct usb_typec *typec = _typec;
	D("+\n");
	I("vbus_state %d\n", vbus_state);
	typec->rt_vbus_state = vbus_state;
	D("-\n");
}


/**
 * hisi_usb_typec_cc_status_change() - Monitor cc status which pass through CCDebounce.
 * Called by typec_handle_cc_changed_entry.
 */
void hisi_usb_typec_cc_status_change(uint8_t cc1, uint8_t cc2)
{
	D("cc1 %u, cc2 %u\n", cc1, cc2);
#ifdef CONFIG_TCPC_CLASS
	pd_dpm_handle_pe_event(PD_DPM_PE_ABNORMAL_CC_CHANGE_HANDLER, NULL);
#endif
}

/**
 * hisi_usb_typec_cc_alert() - Monitor the raw cc status.
 * Called by hisi_tcpc_typec_handle_cc_change.
 */
void hisi_usb_typec_cc_alert(uint8_t cc1, uint8_t cc2)
{
	struct usb_typec *typec = _typec;

	/*
	 * for huawei type-c headset.
	 */
	D("cc1 %d, cc2 %d\n", cc1, cc2);
	hisi_usb_check_hifi_usb_status(HIFI_USB_TCPC);

	/*
	 * only record the time of first cc connection.
	 */
	if ((typec->rt_cc1 == TYPEC_CC_VOLT_OPEN) &&
			(typec->rt_cc2 == TYPEC_CC_VOLT_OPEN)) {
		if ((cc1 != TYPEC_CC_VOLT_OPEN) || (cc2 != TYPEC_CC_VOLT_OPEN)) {
			D("update time_stamp_cc_alert\n");
			typec->time_stamp_cc_alert = jiffies;
		}
	}

	/* record real cc status */
	typec->rt_cc1 = cc1;
	typec->rt_cc2 = cc2;
}


struct tcpc_notify {
	struct list_head node;
	struct tcp_notify notify;
	unsigned long tcpc_notify_type;
};

#define TCPC_NOTIFY_MAX_COUNT 4096

static int queue_notify(struct usb_typec *typec, unsigned long action, void *data)
{
	struct tcpc_notify *noti;

	if (typec->tcpc_notify_count > TCPC_NOTIFY_MAX_COUNT) {
		E("tcpc_notify_list too long, %u\n", typec->tcpc_notify_count);
		return -EBUSY;
	}

	noti = kzalloc(sizeof(*noti), GFP_KERNEL);
	if (!noti) {
		E("No memory!\n");
		return -ENOMEM;
	}

	noti->tcpc_notify_type = action;
	memcpy_s(&noti->notify, sizeof(struct tcp_notify),
			data, sizeof(noti->notify));

	spin_lock(&typec->tcpc_notify_list_lock);
	list_add_tail(&noti->node, &typec->tcpc_notify_list);
	typec->tcpc_notify_count++;
	spin_unlock(&typec->tcpc_notify_list_lock);

	return 0; /*lint !e429 */
}

struct tcpc_notify *get_notify(struct usb_typec *typec)
{
	struct tcpc_notify *noti;

	spin_lock(&typec->tcpc_notify_list_lock);
	noti = list_first_entry_or_null(&typec->tcpc_notify_list,
				struct tcpc_notify, node);

	if (noti) {
		list_del_init(&noti->node);
		typec->tcpc_notify_count--;
	}
	spin_unlock(&typec->tcpc_notify_list_lock);

	return noti;
}

static void free_notify(struct tcpc_notify *noti)
{
	kfree(noti);
}

static inline int __tcpc_notifier_work(struct tcpc_notify *noti)
{
#ifdef CONFIG_TCPC_CLASS
	struct pd_dpm_typec_state tc_state = {0};
	struct pd_dpm_vbus_state vbus_state = {0};
	struct pd_dpm_swap_state swap_state = {0};
	struct pd_dpm_pd_state pd_state = {0};
#endif

	D("tcpc_notify_type %lu\n", noti->tcpc_notify_type);

	switch (noti->tcpc_notify_type) {
	case TCP_NOTIFY_DIS_VBUS_CTRL:
		hisi_usb_typec_disable_vbus_control(&noti->notify.vbus_state);

#ifdef CONFIG_TCPC_CLASS
		vbus_state.mv = noti->notify.vbus_state.mv;
		vbus_state.ma = noti->notify.vbus_state.ma;
		vbus_state.vbus_type = noti->notify.vbus_state.type;

		pd_dpm_handle_pe_event(PD_DPM_PE_EVT_DIS_VBUS_CTRL, (void *)&vbus_state);
#endif
		break;
	case TCP_NOTIFY_SOURCE_VBUS:
		hisi_usb_typec_source_vbus(&noti->notify.vbus_state);

#ifdef CONFIG_TCPC_CLASS
		vbus_state.mv = noti->notify.vbus_state.mv;
		vbus_state.ma = noti->notify.vbus_state.ma;
		vbus_state.vbus_type = noti->notify.vbus_state.type;

		pd_dpm_handle_pe_event(PD_DPM_PE_EVT_SOURCE_VBUS, (void *)&vbus_state);
#endif

		break;
	case TCP_NOTIFY_SINK_VBUS:
		hisi_usb_typec_sink_vbus(&noti->notify.vbus_state);

#ifdef CONFIG_TCPC_CLASS
		vbus_state.mv = noti->notify.vbus_state.mv;
		vbus_state.ma = noti->notify.vbus_state.ma;
		vbus_state.vbus_type = noti->notify.vbus_state.type;

		pd_dpm_handle_pe_event(PD_DPM_PE_EVT_SINK_VBUS, (void *)&vbus_state);
#endif

		break;
	case TCP_NOTIFY_PR_SWAP:
#ifdef CONFIG_TCPC_CLASS
		swap_state.new_role = noti->notify.swap_state.new_role;
		pd_dpm_handle_pe_event(PD_DPM_PE_EVT_PR_SWAP, &swap_state);
#endif
		break;
	case TCP_NOTIFY_DR_SWAP:
		hisi_usb_typec_data_role_swap(noti->notify.swap_state.new_role);

#ifdef CONFIG_TCPC_CLASS
		swap_state.new_role = noti->notify.swap_state.new_role;
		pd_dpm_handle_pe_event(PD_DPM_PE_EVT_DR_SWAP, &swap_state);
#endif
		break;

	case TCP_NOTIFY_TYPEC_STATE:
		hisi_usb_typec_state_change(&noti->notify.typec_state);

#ifdef CONFIG_TCPC_CLASS
		tc_state.polarity = noti->notify.typec_state.polarity;
		tc_state.new_state = noti->notify.typec_state.new_state;

		pd_dpm_handle_pe_event(PD_DPM_PE_EVT_TYPEC_STATE, (void*)&tc_state);
#endif
		break;
	case TCP_NOTIFY_PD_STATE:
		hisi_usb_typec_pd_state_change(&noti->notify.pd_state);

#ifdef CONFIG_TCPC_CLASS
		pd_state.connected = noti->notify.pd_state.connected;
		pd_dpm_handle_pe_event(PD_DPM_PE_EVT_PD_STATE, (void*)&pd_state);
#endif
		break;
	default:
		break;
	}

	return 0;
}

static void tcpc_notifier_work(struct work_struct *work)
{
	struct usb_typec *typec = container_of(work, struct usb_typec, tcpc_notify_work);
	struct tcpc_notify *noti;
	int ret;

	D("+\n");
	while (1) {
		noti = get_notify(typec);
		if (!noti)
			break;

		ret = __tcpc_notifier_work(noti);
		if (ret)
			D("__tcpc_notifier_work ret %d\n", ret);

		free_notify(noti);
	}
	D("-\n");
}

static int tcpc_notifier_call(struct notifier_block *nb,
			unsigned long action, void *data)
{
	struct usb_typec *typec = container_of(nb, struct usb_typec, tcpc_nb);
	int ret;

	D("+\n");
	I("action %lu\n", action);
	ret = queue_notify(typec, action, data);
	if (ret) {
		E("queue_notify failed!\n");
		return NOTIFY_DONE;
	}

	/* Returns %false if @work was already on a queue, %true otherwise. */
	ret = queue_work(typec->hisi_typec_wq, &typec->tcpc_notify_work);
	D("queue_work ret %d\n", ret);

	D("-\n");
	return NOTIFY_OK;
}

/**
 * hisi_usb_typec_max_power() - Record optional_max_power by max_power value
 *                              from pdo.
 * max_power:                   max power value from pdo.
 */
void hisi_usb_typec_max_power(int max_power)
{
	pd_dpm_set_optional_max_power_status(max_power >= HIGH_POWER_VALUE);
}

/* ************************************************************************ */

int hisi_usb_typec_bc12_notify(struct notifier_block *nb, unsigned long action, void *data)
{
	struct usb_typec *typec = _typec;

	D("+\n");

	if (!typec)
		return -ENODEV;

	typec->bc12_type = (unsigned int)action;
	D("-\n");

	return 0;
}

void hisi_usb_typec_wakelock_control(bool lock)
{
	struct usb_typec *typec = _typec;
	int ret;

	if (!typec || !typec->tcpc_dev) {
		E("_typec NULL!\n");
		return;
	}

	I("%s\n", lock ? "lock" : "unlock");
	if (lock) {
		ret = hisi_tcpm_typec_set_wake_lock(typec->tcpc_dev, true);
	} else {
		ret = hisi_tcpm_typec_set_wake_lock(typec->tcpc_dev, false);
	}

	if (ret)
		E("%s failed ret %d\n", lock ? "lock" : "unlock", ret);
}

#ifdef CONFIG_TCPC_CLASS
static int wakelock_notify_call(struct notifier_block *nb, unsigned long event, void *data)
{
	if (event == PD_WAKE_LOCK) {
		hisi_usb_typec_wakelock_control(true);
	} else if (event == PD_WAKE_UNLOCK) {
		hisi_usb_typec_wakelock_control(false);
	} else {
		E("unknown wakelock control event %lu\n", event);
	}
	return NOTIFY_OK;
}
#endif

#ifdef CONFIG_TCPC_CLASS
static void hisi_usb_typec_issue_hardreset(void *data)
{
	struct usb_typec *typec = _typec;
	int ret;

	if (!typec || !typec->tcpc_dev) {
		E("hisi-tcpc not ready\n");
		return;
	}
	I("\n");
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	ret = hisi_tcpm_hard_reset(typec->tcpc_dev);
	if (ret != TCPM_SUCCESS) {
		E("hisi_tcpm_hard_reset ret %d\n", ret);
	}
#endif
}

static bool hisi_usb_pd_get_hw_dock_svid_exist(void *client)
{
	struct usb_typec *typec = _typec;

	D("\n");
	return (typec->dock_svid == PD_DPM_HW_DOCK_SVID);

}

static int hisi_usb_typec_mark_direct_charging(void *data, bool direct_charging)
{
	struct usb_typec *typec = _typec;

	if (!typec || !typec->tcpc_dev) {
		E("hisi usb typec is not ready\n");
		return -ENODEV;
	}

	I("%s\n", direct_charging ? "true" : "false");
	(void)hisi_tcpm_typec_notify_direct_charge(typec->tcpc_dev, direct_charging);
	typec->direct_charging = direct_charging ;

	return 1;
}

static void hisi_usb_typec_set_pd_adapter_voltage(void *data, int voltage_mv)
{
	struct usb_typec *typec = _typec;

	if (!typec || !typec->tcpc_dev)
		return ;

	I("%d mV\n", voltage_mv);
	typec->pd_adapter_voltage = voltage_mv;

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	hisi_tcpm_request_voltage(typec->tcpc_dev, voltage_mv);
#endif
}

static int hisi_usb_get_cc_state(void)
{
	struct usb_typec *typec = _typec;
	return typec->rt_cc1 | (typec->rt_cc2 << 2);
}

static struct pd_dpm_ops hisi_device_pd_dpm_ops = {
	.pd_dpm_hard_reset = hisi_usb_typec_issue_hardreset,
	.pd_dpm_get_hw_dock_svid_exist = hisi_usb_pd_get_hw_dock_svid_exist,
	.pd_dpm_notify_direct_charge_status = hisi_usb_typec_mark_direct_charging,
	.pd_dpm_set_voltage = hisi_usb_typec_set_pd_adapter_voltage,
	.pd_dpm_get_cc_state = hisi_usb_get_cc_state,
	.pd_dpm_set_cc_mode = NULL, // todo:
	.pd_dpm_disable_pd = NULL,
};
#endif


/**
 * hisi_usb_typec_direct_charge_cable_detect() - Check the cable for direct charge or not.
 * return 0 means cable support direct charge, others for not.
 * This function may used by ecall.
 */
int hisi_usb_typec_direct_charge_cable_detect(void)
{
	struct usb_typec *typec = _typec;
	uint8_t cc1, cc2;
	int ret;

	I("\n");
	if (!typec || !typec->tcpc_dev) {
		E("hisi usb typec is not ready\n");
		return -1;
	}

	ret = hisi_tcpm_inquire_remote_cc(typec->tcpc_dev, &cc1, &cc2, true);
	if (ret) {
		E("inquire remote cc failed\n");
		return -1;
	}

	if ((cc1 == PD_DPM_CC_VOLT_SNK_DFT) && (cc2 == PD_DPM_CC_VOLT_SNK_DFT)) {
		I("using \"direct charge cable\" !\n");
		typec->direct_charge_cable = true;
		return 0;
	} else {
		I("not \"direct charge cable\" !\n");
		typec->direct_charge_cable = false;
		return -1;
	}
}

#ifdef CONFIG_TCPC_CLASS
static struct cc_check_ops direct_charge_cable_check_ops = {
        .is_cable_for_direct_charge = hisi_usb_typec_direct_charge_cable_detect,
};
#endif

/**
 * hisi_usb_typec_pd_dpm_register() - Register pd dpm ops for hw_pd driver.
 */
int hisi_usb_typec_register_pd_dpm(void)
{
	int ret = 0;
	struct usb_typec *typec = _typec;

	if (!_typec) {
		return -EPERM;
	}

#ifdef CONFIG_TCPC_CLASS
	ret = pd_dpm_ops_register(&hisi_device_pd_dpm_ops, typec);
	if (ret) {
		I("Need not hisi pd\n");
		return -EBUSY;
	}

	ret = cc_check_ops_register(&direct_charge_cable_check_ops );
	if (ret) {
		E("cc_check_ops register failed!\n");
		/* There is no interface like that "pd_dpm_ops_unregister" */
		return -EBUSY;
	}

	typec->wakelock_control_nb.notifier_call = wakelock_notify_call;
	ret = register_pd_wake_unlock_notifier(&typec->wakelock_control_nb);
	if (ret) {
		E("register_pd_wake_unlock_notifier ret %d\n", ret);
	}
#endif

	return ret;
}

/**
 * hisi_usb_typec_register_tcpc_device() - Save tcpc_device pointer for futher use.
 * tcpc_dev:          Pointer of tcpc_device structure.
 */
void hisi_usb_typec_register_tcpc_device(struct tcpc_device *tcpc_dev)
{
	struct usb_typec *typec = _typec;
	int ret;

	D("+\n");
	if (!_typec)
		return;

	typec->bc12_nb.notifier_call = hisi_usb_typec_bc12_notify;
	(void)hisi_charger_type_notifier_register(&typec->bc12_nb);

	/* save the tcpc handler */
	typec->tcpc_dev = tcpc_dev;

	INIT_LIST_HEAD(&typec->tcpc_notify_list);
	spin_lock_init(&typec->tcpc_notify_list_lock);
	typec->tcpc_notify_count = 0;
	INIT_WORK(&typec->tcpc_notify_work, tcpc_notifier_work);

	typec->tcpc_nb.notifier_call = tcpc_notifier_call;
	ret = hisi_tcpm_register_tcpc_dev_notifier(tcpc_dev, &typec->tcpc_nb);
	if (ret)
		E("register tcpc notifier failed ret %d\n", ret);

	D("-\n");
}

static int typec_probe(struct platform_device *pdev)
{
	struct usb_typec *typec;
	int ret = 0;

	I("+\n");
	typec = devm_kzalloc(&pdev->dev, sizeof(*typec), GFP_KERNEL);
	if (!typec)
		return -ENOMEM;

	typec->power_role = PD_ROLE_UNATTACHED;
	typec->data_role = PD_ROLE_UNATTACHED;
	typec->vconn = PD_ROLE_VCONN_OFF;
	typec->audio_accessory = 0;

	mutex_init(&typec->lock);

	typec->hisi_typec_wq = create_singlethread_workqueue("hisi_usb_typec");

	ret = usb_typec_debugfs_init(typec);
	if (ret) {
		E("debugfs init failed ret %d\n", ret);
		return ret;
	}

	_typec = typec;

	ret = of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);
	if (ret) {
		E("populate child failed ret %d\n", ret);
		usb_typec_debugfs_exit(typec);
		_typec = NULL;
		return ret;
	}

	I("-\n");
	return 0;
}

static int typec_remove(struct platform_device *pdev)
{
	struct usb_typec *typec = _typec;

	I("+\n");
	of_platform_depopulate(&pdev->dev);
	_typec = NULL;
	usb_typec_debugfs_exit(typec);
	I("-\n");
	return 0;
}

#ifdef CONFIG_PM
static int typec_suspend(struct device *dev)
{
	struct usb_typec *typec = _typec;
	D("+\n");
	typec->suspend_count++;
	D("-\n");
	return 0;
}

static int typec_resume(struct device *dev)
{
	struct usb_typec *typec = _typec;
	D("+\n");
	typec->resume_count++;
	D("-\n");
	return 0;
}

static const struct dev_pm_ops typec_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(typec_suspend, typec_resume)
};
#define TYPEC_PM_OPS	(&typec_pm_ops)
#else
#define TYPEC_PM_OPS	(NULL)
#endif /* CONFIG_PM */

static const struct of_device_id typec_match_table[] = {
	{.compatible = "hisilicon,hisi-usb-typec",},
	{},
};

static struct platform_driver typec_driver = {
	.driver = {
		.name = "hisi-usb-typec",
		.owner = THIS_MODULE,
		.of_match_table = typec_match_table,
		.pm = TYPEC_PM_OPS,
	},
	.probe = typec_probe,
	.remove = typec_remove,
};

static int __init hisi_typec_init(void)
{
	I("\n");
	return platform_driver_register(&typec_driver);
}

static void __exit hisi_typec_exit(void)
{
	I("\n");
	platform_driver_unregister(&typec_driver);
}

subsys_initcall(hisi_typec_init);
module_exit(hisi_typec_exit);

MODULE_AUTHOR("Hisilicon");
MODULE_DESCRIPTION("Hisilicon USB Type-C Driver");
MODULE_LICENSE("GPL");
