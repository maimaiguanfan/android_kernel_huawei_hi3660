/*
 * Copyright (C) 2016 Richtek Technology Corp.
 * Author: TH <tsunghan_tsai@richtek.com>
 *
 * Power Delivery Core Driver
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

#include <linux/of.h>
#include <linux/slab.h>

#define LOG_TAG "[pd][pd_core.c]"

#include "include/tcpci.h"
#include "include/pd_core.h"
#include "include/pd_dpm_core.h"
#include "include/tcpci_typec.h"
#include "include/tcpci_event.h"
#include "include/pd_policy_engine.h"

#include "securec.h"

/* From DTS */

static int pd_parse_pdata(pd_port_t *pd_port)
{
	struct device_node *np;
	int ret = 0, i;

	D("+\n");
	np = of_find_node_by_name(pd_port->tcpc_dev->dev.parent->of_node, "pd-data-objects");

	if (!np) {
		D("can not find pd-data-objects of node\n");
		return 0;
	}

	/* Source PDO */
	ret = of_property_read_u32(np, "pd,source-pdo-size",
			(u32 *)&pd_port->local_src_cap_default.nr);
	if (ret < 0)
		pr_err("%s get source pdo size fail\n", __func__);

	ret = of_property_read_u32_array(np, "pd,source-pdo-data",
		(u32 *)pd_port->local_src_cap_default.pdos,
		pd_port->local_src_cap_default.nr);
	if (ret < 0)
		pr_err("%s get source pdo data fail\n", __func__);

	pr_info("%s src pdo data =\n", __func__);
	for (i = 0; i < pd_port->local_src_cap_default.nr; i++) {
		pr_info("%s %d: 0x%08x\n", __func__, i,
			pd_port->local_src_cap_default.pdos[i]);
	}

	/* Sink PDO */
	ret = of_property_read_u32(np, "pd,sink-pdo-size",
				(u32 *)&pd_port->local_snk_cap.nr);
	if (ret < 0)
		pr_err("%s get sink pdo size fail\n", __func__);

	ret = of_property_read_u32_array(np, "pd,sink-pdo-data",
		(u32 *)pd_port->local_snk_cap.pdos,
			pd_port->local_snk_cap.nr);
	if (ret < 0)
		pr_err("%s get sink pdo data fail\n", __func__);

	pr_info("%s snk pdo data =\n", __func__);
	for (i = 0; i < pd_port->local_snk_cap.nr; i++) {
		pr_info("%s %d: 0x%08x\n", __func__, i,
			pd_port->local_snk_cap.pdos[i]);
	}

	/* VDO */
	ret = of_property_read_u32(np, "pd,id-vdo-size",
				(u32 *)&pd_port->id_vdo_nr);
	if (ret < 0)
		pr_err("%s get id vdo size fail\n", __func__);
	ret = of_property_read_u32_array(np, "pd,id-vdo-data",
		(u32 *)pd_port->id_vdos, pd_port->id_vdo_nr);
	if (ret < 0)
		pr_err("%s get id vdo data fail\n", __func__);

	pr_info("%s id vdos data =\n", __func__);
	for (i = 0; i < pd_port->id_vdo_nr; i++)
		pr_info("%s %d: 0x%08x\n", __func__, i,
		pd_port->id_vdos[i]);

	D("-\n");
	return 0;
}

#define DEFAULT_DP_ROLE_CAP				(MODE_DP_SRC)
#define DEFAULT_DP_FIRST_CONNECTED		(DPSTS_DFP_D_CONNECTED)
#define DEFAULT_DP_SECOND_CONNECTED		(DPSTS_DFP_D_CONNECTED)

#ifdef CONFIG_USB_PD_ALT_MODE_SUPPORT
static const struct {
	const char *prop_name;
	uint32_t mode;
} supported_dp_pin_modes[] = {
	{ "pin_assignment,mode_a", MODE_DP_PIN_A },
	{ "pin_assignment,mode_b", MODE_DP_PIN_B },
	{ "pin_assignment,mode_c", MODE_DP_PIN_C },
	{ "pin_assignment,mode_d", MODE_DP_PIN_D },
	{ "pin_assignment,mode_e", MODE_DP_PIN_E },
	{ "pin_assignment,mode_f", MODE_DP_PIN_F },
};

static const struct {
	const char *conn_mode;
	uint32_t val;
} dp_connect_mode[] = {
	{"both", DPSTS_BOTH_CONNECTED},
	{"dfp_d", DPSTS_DFP_D_CONNECTED},
	{"ufp_d", DPSTS_UFP_D_CONNECTED},
};

static void __dpm_alt_mode_parse_svid_connected(pd_port_t *pd_port, struct device_node *np)
{
	const char *connection;
	unsigned int i = 0;

	pd_port->dp_first_connected = DEFAULT_DP_FIRST_CONNECTED;
	pd_port->dp_second_connected = DEFAULT_DP_SECOND_CONNECTED;

	if (of_property_read_string(np, "1st_connection", &connection) == 0) {
		for (i = 0; i < ARRAY_SIZE(dp_connect_mode); i++) {
			if (strcasecmp(connection, dp_connect_mode[i].conn_mode) == 0) {
				pd_port->dp_first_connected = dp_connect_mode[i].val;
				break;
			}
		}
	}

	if (of_property_read_string(np, "2nd_connection", &connection) == 0) {
		for (i = 0; i < ARRAY_SIZE(dp_connect_mode); i++) {
			if (strcasecmp(connection, dp_connect_mode[i].conn_mode) == 0) {
				pd_port->dp_second_connected = dp_connect_mode[i].val;
				break;
			}
		}
	}

	D("dp_first_connected %d, dp_second_connected %d\n",
		pd_port->dp_first_connected, pd_port->dp_second_connected);
}

static int dpm_alt_mode_parse_svid_data(
		pd_port_t *pd_port, svdm_svid_data_t *svid_data)
{
	struct device_node *np, *ufp_np, *dfp_np;
	uint32_t ufp_d_pin_cap = 0;
	uint32_t dfp_d_pin_cap = 0;
	uint32_t signal = MODE_DP_V13;
	uint32_t receptacle = 1;
	uint32_t usb2 = 0;
	unsigned int i = 0;

	D("+\n");
	np = of_find_node_by_name(pd_port->tcpc_dev->dev.of_node, "display-port");
	if (np == NULL) {
		pd_port->svid_data_cnt = 0;
		E("get display port data failed\n");
		return -1;
	}

	svid_data->svid = USB_SID_DISPLAYPORT;
	ufp_np = of_find_node_by_name(np, "ufp_d");
	dfp_np = of_find_node_by_name(np, "dfp_d");

	if (ufp_np) {
		for (i = 0; i < ARRAY_SIZE(supported_dp_pin_modes) - 1; i++) { /* don't support pin_f */
			if (of_property_read_bool(ufp_np, supported_dp_pin_modes[i].prop_name))
				ufp_d_pin_cap |= supported_dp_pin_modes[i].mode;
		}
		D("ufp_d_pin_cap 0x%x\n", ufp_d_pin_cap);
	}

	if (dfp_np) {
		pr_info("dp, dfp_np\n");
		for (i = 0; i < ARRAY_SIZE(supported_dp_pin_modes); i++) {
			if (of_property_read_bool(dfp_np, supported_dp_pin_modes[i].prop_name))
				dfp_d_pin_cap |= supported_dp_pin_modes[i].mode;
		}
		D("dfp_d_pin_cap 0x%x\n", dfp_d_pin_cap);
	}

	if (of_property_read_bool(np, "signal,dp_v13"))
		signal |= MODE_DP_V13;
	if (of_property_read_bool(np, "signal,dp_gen2"))
		signal |= MODE_DP_GEN2;
	if (of_property_read_bool(np, "usbr20_not_used"))
		usb2 = 1;
	if (of_property_read_bool(np, "typec,receptacle"))
		receptacle = 1;

	svid_data->local_mode.mode_cnt = 1;
	svid_data->local_mode.mode_vdo[0] = VDO_MODE_DP(ufp_d_pin_cap, dfp_d_pin_cap,
			usb2, receptacle, signal,
			(ufp_d_pin_cap ? MODE_DP_SNK : 0) | (dfp_d_pin_cap ? MODE_DP_SRC : 0));

	D("mode_cnt 1, mode_vdo[0] 0x%x\n", svid_data->local_mode.mode_vdo[0]);


	__dpm_alt_mode_parse_svid_connected(pd_port, np);

	D("-\n");
	return 0;
}

#endif /* CONFIG_USB_PD_ALT_MODE_SUPPORT*/

#ifdef CONFIG_USB_PD_RICHTEK_UVDM

static int dpm_richtek_parse_svid_data(
	pd_port_t *pd_port, svdm_svid_data_t *svid_data)
{
	D("+-\n");
	svid_data->svid = USB_SID_DIRECTCHARGE;
	return 0;
}

#endif	/* CONFIG_USB_PD_RICHTEK_UVDM */

static void pd_core_parse_svid_data(pd_port_t *pd_port)
{
	int ret, i = 0;

	/* TODO: dynamic allocate svid_data from DTS */
	D("+\n");

#ifdef CONFIG_USB_PD_ALT_MODE_SUPPORT
	ret = dpm_alt_mode_parse_svid_data(pd_port, &pd_port->svid_data[i]);
	if (ret == 0)
		i++;
#endif	/* CONFIG_USB_PD_ALT_MODE_SUPPORT */

#ifdef CONFIG_USB_PD_RICHTEK_UVDM
	ret = dpm_richtek_parse_svid_data(pd_port, &pd_port->svid_data[i]);
	if (ret == 0)
		i++;
#endif	/* CONFIG_USB_PD_RICHTEK_UVDM */

	pd_port->svid_data_cnt = i;
	D("svid_data_cnt %d\n", pd_port->svid_data_cnt);

	WARN_ON(i > VDO_MAX_SVID_SIZE);
	D("-\n");
}

struct supported_dpm_cap {
	const char *prop_name;
	uint32_t val;
};

static const struct supported_dpm_cap supported_dpm_caps[] = {
	{"local_dr_power", DPM_CAP_LOCAL_DR_POWER},
	{"local_dr_data", DPM_CAP_LOCAL_DR_DATA},
	{"local_ext_power", DPM_CAP_LOCAL_EXT_POWER},
	{"local_usb_comm", DPM_CAP_LOCAL_USB_COMM},
	{"local_usb_suspend", DPM_CAP_LOCAL_USB_SUSPEND},
	{"local_high_cap", DPM_CAP_LOCAL_HIGH_CAP},
	{"local_give_back", DPM_CAP_LOCAL_GIVE_BACK},
	{"local_no_suspend", DPM_CAP_LOCAL_NO_SUSPEND},
	{"local_vconn_supply", DPM_CAP_LOCAL_VCONN_SUPPLY},

	{"attemp_discover_cable_dfp", DPM_CAP_ATTEMP_DISCOVER_CABLE_DFP},
	{"attemp_enter_dp_mode", DPM_CAP_ATTEMP_ENTER_DP_MODE},
	{"attemp_discover_cable", DPM_CAP_ATTEMP_DISCOVER_CABLE},
	{"attemp_discover_id", DPM_CAP_ATTEMP_DISCOVER_ID},

	{"pr_reject_as_source", DPM_CAP_PR_SWAP_REJECT_AS_SRC},
	{"pr_reject_as_sink", DPM_CAP_PR_SWAP_REJECT_AS_SNK},
	{"pr_check_gp_source", DPM_CAP_PR_SWAP_CHECK_GP_SRC},
	{"pr_check_gp_sink", DPM_CAP_PR_SWAP_CHECK_GP_SNK},

	{"dr_reject_as_dfp", DPM_CAP_DR_SWAP_REJECT_AS_DFP},
	{"dr_reject_as_ufp", DPM_CAP_DR_SWAP_REJECT_AS_UFP},

	{"snk_prefer_low_voltage", DPM_CAP_SNK_PREFER_LOW_VOLTAGE},
	{"snk_ignore_mismatch_current", DPM_CAP_SNK_IGNORE_MISMATCH_CURRENT},
};

static void pd_core_power_flags_init(pd_port_t *pd_port)
{
	uint32_t src_flag, snk_flag, val = 0;
	struct device_node *np;
	unsigned int i;
	pd_port_power_caps *snk_cap = &pd_port->local_snk_cap;
	pd_port_power_caps *src_cap = &pd_port->local_src_cap_default;

	D("+\n");
	np = of_find_node_by_name(pd_port->tcpc_dev->dev.parent->of_node, "dpm-caps");

	for (i = 0; i < ARRAY_SIZE(supported_dpm_caps); i++) {
		if (of_property_read_bool(np, supported_dpm_caps[i].prop_name))
			pd_port->dpm_caps |= supported_dpm_caps[i].val;

		D("dpm_caps: %s 0x%x\n", supported_dpm_caps[i].prop_name,
					supported_dpm_caps[i].val);
	}

	if (of_property_read_u32(np, "pr_check", &val) == 0)
		pd_port->dpm_caps |= DPM_CAP_PR_CHECK_PROP(val);
	else
		pr_err("%s get pr_check data fail\n", __func__);

	if (of_property_read_u32(np, "dr_check", &val) == 0)
		pd_port->dpm_caps |= DPM_CAP_DR_CHECK_PROP(val);
	else
		pr_err("%s get dr_check data fail\n", __func__);

	I("dpm_caps = 0x%08x\n", pd_port->dpm_caps);

	src_flag = 0;
	if (pd_port->dpm_caps & DPM_CAP_LOCAL_DR_POWER)
		src_flag |= PDO_FIXED_DUAL_ROLE;

	if (pd_port->dpm_caps & DPM_CAP_LOCAL_DR_DATA)
		src_flag |= PDO_FIXED_DATA_SWAP;

	if (pd_port->dpm_caps & DPM_CAP_LOCAL_EXT_POWER)
		src_flag |= PDO_FIXED_EXTERNAL;

	if (pd_port->dpm_caps & DPM_CAP_LOCAL_USB_COMM)
		src_flag |= PDO_FIXED_COMM_CAP;

	if (pd_port->dpm_caps & DPM_CAP_LOCAL_USB_SUSPEND)
		src_flag |= PDO_FIXED_SUSPEND;

	snk_flag = src_flag;
	if (pd_port->dpm_caps & DPM_CAP_LOCAL_HIGH_CAP)
		snk_flag |= PDO_FIXED_HIGH_CAP;

	snk_cap->pdos[0] |= snk_flag;
	src_cap->pdos[0] |= src_flag;
	D("snk_flag 0x%x, src_flag 0x%x\n", snk_flag, src_flag);

	D("-\n");
}

int hisi_pd_core_init(struct tcpc_device *tcpc_dev)
{
	int ret;
	pd_port_t *pd_port = &tcpc_dev->pd_port;

	D("+\n");
	mutex_init(&pd_port->pd_lock);
	pd_port->tcpc_dev = tcpc_dev;
	D("tcpc_dev %p\n", pd_port->tcpc_dev);

	pd_port->pe_pd_state = PE_IDLE2;
	pd_port->pe_vdm_state = PE_IDLE2;
	D("pe_pd_state %d, pe_vdm_state %d\n",
			pd_port->pe_pd_state, pd_port->pe_vdm_state);

	pd_port->pd_connect_state = HISI_PD_CONNECT_NONE;
	D("pd_connect_state %d\n", pd_port->pd_connect_state);

	ret = pd_parse_pdata(pd_port);
	if (ret)
		return ret;

	pd_core_parse_svid_data(pd_port);
	pd_core_power_flags_init(pd_port);

	hisi_pd_dpm_core_init(pd_port);

	PE_INFO("hisi_pd_core_init\n");
	D("-\n");
	return 0;
}

void hisi_pd_extract_rdo_power(uint32_t rdo, uint32_t pdo,
			uint32_t *op_curr, uint32_t *max_curr)
{
	uint32_t op_power, max_power, vmin;

	D("+\n");

	switch (pdo & PDO_TYPE_MASK) {
	case PDO_TYPE_FIXED:
	case PDO_TYPE_VARIABLE:
		*op_curr = RDO_FIXED_VAR_EXTRACT_OP_CURR(rdo);
		*max_curr = RDO_FIXED_VAR_EXTRACT_MAX_CURR(rdo);
		break;

	case PDO_TYPE_BATTERY: /* TODO: check it later !! */
		vmin = PDO_BATT_EXTRACT_MIN_VOLT(pdo);
		op_power = RDO_BATT_EXTRACT_OP_POWER(rdo);
		max_power = RDO_BATT_EXTRACT_MAX_POWER(rdo);

		*op_curr = op_power / vmin;
		*max_curr = max_power / vmin;
		break;

	default:
		*op_curr = *max_curr = 0;
		break;
	}
	D("-\n");
}

uint32_t hisi_pd_reset_pdo_power(uint32_t pdo, uint32_t imax)
{
	uint32_t ioper;

	D("+\n");
	switch (pdo & PDO_TYPE_MASK) {
	case PDO_TYPE_FIXED:
		ioper = PDO_FIXED_EXTRACT_CURR(pdo);
		if (ioper > imax)
			return PDO_FIXED_RESET_CURR(pdo, imax);
		break;

	case PDO_TYPE_VARIABLE:
		ioper = PDO_VAR_EXTRACT_CURR(pdo);
		if (ioper > imax)
			return PDO_VAR_RESET_CURR(pdo, imax);
		break;

	case PDO_TYPE_BATTERY:
		/* TODO: check it later !! */
		PD_ERR("No Support\n");
		break;
	}
	D("-\n");
	return pdo;
}

void hisi_pd_extract_pdo_power(uint32_t pdo,
		uint32_t *vmin, uint32_t *vmax, uint32_t *ioper)
{
	uint32_t pwatt;

	D("+\n");
	switch (pdo & PDO_TYPE_MASK) {
	case PDO_TYPE_FIXED:
		*ioper = PDO_FIXED_EXTRACT_CURR(pdo);
		*vmin = *vmax = PDO_FIXED_EXTRACT_VOLT(pdo);
		break;

	case PDO_TYPE_VARIABLE:
		*ioper = PDO_VAR_EXTRACT_CURR(pdo);
		*vmin = PDO_VAR_EXTRACT_MIN_VOLT(pdo);
		*vmax = PDO_VAR_EXTRACT_MAX_VOLT(pdo);
		break;

	case PDO_TYPE_BATTERY:	/* TODO: check it later !! */
		*vmin = PDO_BATT_EXTRACT_MIN_VOLT(pdo);
		*vmax = PDO_BATT_EXTRACT_MAX_VOLT(pdo);
		pwatt = PDO_BATT_EXTRACT_OP_POWER(pdo);
		*ioper = pwatt / *vmin;
		break;

	default:
		*vmin = *vmax = *ioper = 0;
	}
	D("-\n");
}


uint32_t hisi_pd_extract_cable_curr(uint32_t vdo)
{
	uint32_t cable_curr;

	D("+\n");
	switch (PD_VDO_CABLE_CURR(vdo)) {
	case CABLE_CURR_1A5:
		cable_curr = 1500;
		break;
	case CABLE_CURR_5A:
		cable_curr = 5000;
		break;
	default:
	case CABLE_CURR_3A:
		cable_curr = 3000;
		break;
	}

	D("-\n");
	return cable_curr;
}

void hisi_pd_reset_svid_data(pd_port_t *pd_port)
{
	uint8_t i;
	svdm_svid_data_t *svid_data;

	D("+\n");
	for (i = 0; i < pd_port->svid_data_cnt; i++) {
		svid_data = &pd_port->svid_data[i];
		svid_data->exist = false;
		svid_data->remote_mode.mode_cnt = 0;
		svid_data->active_mode = 0;
	}

	/* clear */
	hisi_usb_pd_ufp_update_dock_svid(0);
	D("-\n");
}

int hisi_pd_reset_protocol_layer(pd_port_t *pd_port)
{
	int i = 0;

	D("+\n");
	hisi_pd_notify_pe_reset_protocol(pd_port);

	pd_port->explicit_contract = 0;
	pd_port->local_selected_cap = 0;
	pd_port->remote_selected_cap = 0;
	pd_port->during_swap = 0;
	pd_port->dpm_ack_immediately = 0;

#ifdef CONFIG_USB_PD_DFP_FLOW_DELAY
#ifdef CONFIG_USB_PD_DFP_FLOW_DELAY_RESET
	if (pd_port->pd_prev_connected)
		pd_port->dpm_dfp_flow_delay_done = 0;
#endif	/* CONFIG_USB_PD_DFP_FLOW_DELAY_RESET */
#endif	/* CONFIG_USB_PD_DFP_FLOW_DELAY */

#ifdef CONFIG_USB_PD_DFP_READY_DISCOVER_ID
	pd_port->vconn_return = false;
#endif	/* CONFIG_USB_PD_DFP_READY_DISCOVER_ID */

	for (i = 0; i < PD_SOP_NR; i++) {
		pd_port->msg_id_tx[i] = 0;
		pd_port->msg_id_rx[i] = 0;
		pd_port->msg_id_rx_init[i] = false;
	}

#ifdef CONFIG_USB_PD_IGNORE_PS_RDY_AFTER_PR_SWAP
	pd_port->msg_id_pr_swap_last = 0xff;
#endif	/* CONFIG_USB_PD_IGNORE_PS_RDY_AFTER_PR_SWAP */

	D("-\n");
	return 0;
}

int hisi_pd_set_rx_enable(pd_port_t *pd_port, uint8_t enable)
{
	D("0x%x\n", enable);
	return tcpci_set_rx_enable(pd_port->tcpc_dev, enable);
}

#ifdef CONFIG_HISI_TCPC_QUIRK_V100
void hisi_pd_set_vbus_detect(pd_port_t *pd_port, bool enable)
{
	D("+\n");
	tcpci_set_vbus_detect(pd_port->tcpc_dev, enable);
	D("-\n");
	return;
}
#endif

int hisi_pd_enable_vbus_valid_detection(pd_port_t *pd_port, bool wait_valid)
{
	D("wait_valid %d\n", wait_valid);

	if (wait_valid) {
		hisi_pd_notify_pe_wait_vbus_once(pd_port, PD_WAIT_VBUS_VALID_ONCE);
	} else {
		hisi_pd_notify_pe_wait_vbus_once(pd_port, PD_WAIT_VBUS_INVALID_ONCE);
	}

	return 0;
}

int hisi_pd_enable_vbus_safe0v_detection(pd_port_t *pd_port)
{
	D("+-\n");
	PE_DBG("WaitVSafe0V\n");
	hisi_pd_notify_pe_wait_vbus_once(pd_port, PD_WAIT_VBUS_SAFE0V_ONCE);
	return 0;
}

int hisi_pd_enable_vbus_stable_detection(pd_port_t *pd_port)
{
	D("+-\n");
	PE_DBG("WaitVStable\n");
	hisi_pd_notify_pe_wait_vbus_once(pd_port, PD_WAIT_VBUS_STABLE_ONCE);
	return 0;
}

int hisi_pd_set_data_role(pd_port_t *pd_port, uint8_t dr)
{
	int ret;

	D("+\n");
	pd_port->data_role = dr;

#ifdef CONFIG_DUAL_ROLE_USB_INTF
	/* dual role usb--> 0:ufp, 1:dfp */
	pd_port->tcpc_dev->dual_role_mode = pd_port->data_role;
	/* dual role usb --> 0: Device, 1: Host */
	pd_port->tcpc_dev->dual_role_dr = !(pd_port->data_role);
	hisi_tcpc_dual_role_instance_changed(pd_port->tcpc_dev->dr_usb);
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

	tcpci_notify_role_swap(pd_port->tcpc_dev, TCP_NOTIFY_DR_SWAP, dr);
	ret = tcpci_set_msg_header(pd_port->tcpc_dev,
			pd_port->power_role, pd_port->data_role);
	D("-\n");
	return ret;
}

int hisi_pd_set_power_role(pd_port_t *pd_port, uint8_t pr)
{
	int ret;

	D("+\n");
	pd_port->power_role = pr;
	ret = tcpci_set_msg_header(pd_port->tcpc_dev, pd_port->power_role, pd_port->data_role);
	if (ret)
		return ret;

	/* modiry power_role, and change cc setting */
	hisi_pd_notify_pe_pr_changed(pd_port);

#ifdef CONFIG_DUAL_ROLE_USB_INTF
	/* 0:sink, 1: source */
	pd_port->tcpc_dev->dual_role_pr = !(pd_port->power_role);
	hisi_tcpc_dual_role_instance_changed(pd_port->tcpc_dev->dr_usb);
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

	tcpci_notify_role_swap(pd_port->tcpc_dev, TCP_NOTIFY_PR_SWAP, pr);
	D("-\n");
	return ret;
}

int hisi_pd_init_role(pd_port_t *pd_port, uint8_t pr, uint8_t dr, bool vr)
{
	int ret;
	D("+\n");
	pd_port->power_role = pr;
	pd_port->data_role = dr;
	pd_port->vconn_source = vr;

	ret = tcpci_set_msg_header(pd_port->tcpc_dev,
			pd_port->power_role, pd_port->data_role);
	D("-\n");
	return ret;
}

int hisi_pd_set_vconn(pd_port_t *pd_port, int enable)
{
	int ret;

	D("+\n");
	pd_port->vconn_source = enable;

#ifdef CONFIG_DUAL_ROLE_USB_INTF
	pd_port->tcpc_dev->dual_role_vconn = pd_port->vconn_source;
	hisi_tcpc_dual_role_instance_changed(pd_port->tcpc_dev->dr_usb);
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

	tcpci_notify_role_swap(pd_port->tcpc_dev,
				TCP_NOTIFY_VCONN_SWAP, enable);
	ret = tcpci_set_vconn(pd_port->tcpc_dev, enable);
	D("-\n");
	return ret;
}

static inline int pd_reset_modal_operation(pd_port_t *pd_port)
{
	uint8_t i;
	svdm_svid_data_t *svid_data;

	D("+\n");
	for (i = 0; i < pd_port->svid_data_cnt; i++) {
		svid_data = &pd_port->svid_data[i];

		if (svid_data->active_mode) {
			svid_data->active_mode = 0;
			tcpci_exit_mode(pd_port->tcpc_dev, svid_data->svid);
		}
	}

	pd_port->modal_operation = false;
	D("-\n");
	return 0;
}

int hisi_pd_reset_local_hw(pd_port_t *pd_port)
{
	D("+\n");
	hisi_pd_notify_pe_transit_to_default(pd_port);
	hisi_pd_unlock_msg_output(pd_port);

	pd_reset_pe_timer(pd_port);
	hisi_pd_set_rx_enable(pd_port, PD_RX_CAP_PE_HARDRESET);

	pd_port->explicit_contract = false;
	pd_port->pd_connected  = false;
	pd_port->pe_ready = false;
	pd_port->dpm_ack_immediately = false;

#ifdef CONFIG_USB_PD_HANDLE_PRDR_SWAP
	pd_port->postpone_pr_swap = false;
	pd_port->postpone_dr_swap = false;
#endif	/* CONFIG_USB_PD_HANDLE_PRDR_SWAP */

	pd_reset_modal_operation(pd_port);

	hisi_pd_set_vconn(pd_port, false);

	if (pd_port->power_role == PD_ROLE_SINK) {
		pd_port->state_machine = PE_STATE_MACHINE_SINK;
		D("state_machine %d(%s)\n", pd_port->state_machine,
			pd_pe_state_machine_name(pd_port->state_machine));
		hisi_pd_set_data_role(pd_port, PD_ROLE_UFP);
	} else {
		pd_port->state_machine = PE_STATE_MACHINE_SOURCE;
		D("state_machine %d(%s)\n", pd_port->state_machine,
			pd_pe_state_machine_name(pd_port->state_machine));
		hisi_pd_set_data_role(pd_port, PD_ROLE_DFP);
	}

	hisi_pd_dpm_notify_pe_hardreset(pd_port);
	PE_DBG("reset_local_hw\n");

	D("-\n");
	return 0;
}

int hisi_pd_enable_bist_test_mode(pd_port_t *pd_port, bool en)
{
	D("+-\n");
	PE_DBG("bist_test_mode=%d\n", en);
	return tcpci_set_bist_test_mode(pd_port->tcpc_dev, en);
}

/* ---- Handle PD Message ----*/

int hisi_pd_handle_soft_reset(pd_port_t *pd_port, uint8_t state_machine)
{
	int ret;

	D("+\n");
	pd_port->state_machine = state_machine;
	D("state_machine %s\n", pd_pe_state_machine_name(pd_port->state_machine));
	hisi_pd_reset_protocol_layer(pd_port);
	hisi_pd_update_dpm_request_state(pd_port, DPM_REQ_ERR_RECV_SRESET);
	ret = hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_ACCEPT);
	D("-\n");
	return ret;
}

/* ---- Send PD Message ----*/

#ifdef CONFIG_HISI_TCPC_DEBUG
static void print_send_vdm_msg(uint16_t header, const uint32_t *payload)
{
	uint32_t vdo_count = header & (0x3 << 12);
	uint32_t vdm_hdr = payload[0];

	char *vdm_cmd_type[4] = {"REQ", "ACK", "NAK", "BUSY"};
	char vdm_cmd = vdm_hdr & 0x1f;
	char *vdm_cmd_name[32] = {
		"Reserved",
		"Discover Identity",
		"Discover SVIDs",
		"Discover Modes",
		"Enter Mode",
		"Exit Mode",
		"Attention",
	};

	D("VDO count %d\n", vdo_count >> 12);
	D("SVID 0x%x, vdm type %s, version %d, Object Position %d, "
		"Command Type %s, Command %d %s\n",
			vdm_hdr >> 16,
			(vdm_hdr & (1 << 15)) ? "structed" : "unstructed",
			(vdm_hdr >> 13) & 0x3,
			(vdm_hdr >> 8) & 0x7,
			vdm_cmd_type[(int)((vdm_hdr >> 6) & 0x3)],
			vdm_cmd,
			(vdm_cmd < 7) ? vdm_cmd_name[(int)vdm_cmd] : "reserved");

	return;
}

static void print_send_msg( uint8_t sop_type, uint8_t msg, uint16_t count,
		const uint32_t *data, uint16_t msg_hdr)
{
	int i;
	char * sop_type_name[] = {
		"SOP",
		"SOP'",
		"SOP''",
		"SOP_DEBUG'",
		"SOP_DEBUG''",
		"HARD_RESET",
		"CABLE_RESET",
		"BIST_MODE_2"
	};

	char * const ctrl_msg_name[32] = {
		"Reserved",
		"GoodCRC",
		"GotoMin",
		"Accept",
		"Reject",
		"Ping",
		"PS_RDY",
		"Get_Source_Cap",
		"Get_Sink_Cap",
		"DR_Swap",
		"PR_Swap",
		"VCONN_Swap",
		"Wait",
		"Soft_Reset",
		"Reserved",
		"Reserved",
		"Not_Supported",
		"Get_Source_Cap_Extended",
		"Get_Status",
		"FR_Swap",
		"Get_PPS_Status",
		"Get_Country_Codes",
	};

	char * const data_msg_name[] = {
		"data0",
		"src_cap",
		"request",
		"bist",
		"sink_cap",
		"data5",
		"data6",
		"data7",
		"data8",
		"data9",
		"dataA",
		"dataB",
		"dataC",
		"dataD",
		"dataE",
		"vdm",
	};

	D("++++++++++++++++++++++++++++++++++++++++++++\n");
	if (count == 0) {
		D("%s, " "Control Msg %d(%s), hdr 0x%x\n",
			sop_type_name[sop_type], msg, ctrl_msg_name[msg], msg_hdr);
	} else if ( msg == 15) {
		D("%s, " "VDM, hdr 0x%x, VDM hdr 0x%x\n",
			sop_type_name[sop_type], msg_hdr, data[0]);
		print_send_vdm_msg(msg_hdr, data);
	} else {
		D("%s, " "Data Msg %d(%s), hdr 0x%x\n",
			sop_type_name[sop_type], msg, data_msg_name[msg], msg_hdr);
		for (i = 0; i < count; i++) {
			D("Data PDO[%d] 0x%x\n", i, data[i]);
		}
	}
	D("++++++++++++++++++++++++++++++++++++++++++++\n");
}
#else
static inline void print_send_msg( uint8_t sop_type, uint8_t msg,
		uint16_t count,	const uint32_t *data, uint16_t msg_hdr){}
#endif

static int pd_send_message(pd_port_t *pd_port, uint8_t sop_type,
			uint8_t msg, uint16_t count, const uint32_t *data)
{
	int ret;
	uint16_t msg_hdr;
	uint8_t type = PD_TX_STATE_WAIT_CRC_PD;
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");

	/* FIXME: Should lock ? */
	if (tcpc_dev->typec_attach_old == TYPEC_UNATTACHED) {
		PE_DBG("[SendMsg] Unattached\n");
		return 0;
	}

	if (tcpc_dev->pd_hard_reset_event_pending) {
		PE_DBG("[SendMsg] HardReset Pending");
		return 0;
	}


	if (sop_type == TCPC_TX_SOP) {
		msg_hdr = PD_HEADER_SOP(msg, pd_port->power_role, pd_port->data_role,
				pd_port->msg_id_tx[sop_type], count);
	} else {
		msg_hdr = PD_HEADER_SOP_PRIME(msg, 0, pd_port->msg_id_tx[sop_type], count);
	}

	if ((count > 0) && (msg == PD_DATA_VENDOR_DEF))
		type = PD_TX_STATE_WAIT_CRC_VDM;

	pd_port->msg_id_tx[sop_type] = (pd_port->msg_id_tx[sop_type] + 1) % 8;

	print_send_msg(sop_type, msg, count, data, msg_hdr);

	hisi_pd_notify_pe_transmit_msg(pd_port, type);
	ret = tcpci_transmit(pd_port->tcpc_dev, sop_type, msg_hdr, data);
	if (ret < 0)
		PD_ERR("[SendMsg] Failed, %d\n", ret);

	D("-\n");
	return ret;
}

int hisi_pd_send_ctrl_msg(pd_port_t *pd_port, uint8_t sop_type, uint8_t msg)
{
	D("+-\n");
	return pd_send_message(pd_port, sop_type, msg, 0, NULL);
}

int hisi_pd_send_data_msg(pd_port_t *pd_port, uint8_t sop_type,
				uint8_t msg, uint8_t cnt, uint32_t *payload)
{
	D("+-\n");
	return pd_send_message(pd_port, sop_type, msg, cnt, payload);
}

int hisi_pd_send_soft_reset(pd_port_t *pd_port, uint8_t state_machine)
{
	int ret;

	D("+\n");
	pd_port->state_machine = state_machine;
	D("state_machine %s\n", pd_pe_state_machine_name(pd_port->state_machine));
	hisi_pd_reset_protocol_layer(pd_port);
	hisi_pd_update_dpm_request_state(pd_port, DPM_REQ_ERR_SEND_SRESET);
	ret = hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_SOFT_RESET);
	D("-\n");
	return ret;
}

int hisi_pd_send_hard_reset(pd_port_t *pd_port)
{
	int ret;
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	PE_DBG("Send HARD Reset\n");

	pd_port->hard_reset_counter++;
	hisi_pd_notify_pe_send_hard_reset(pd_port);

	ret = tcpci_transmit(tcpc_dev, TCPC_TX_HARD_RESET, 0, NULL);

	D("-\n");
	return ret;
}

int hisi_pd_send_bist_mode2(pd_port_t *pd_port)
{
	int ret = 0;

	D("+\n");
	hisi_pd_update_dpm_request_state(pd_port, DPM_REQ_ERR_SEND_BIST);

#ifdef CONFIG_USB_PD_TRANSMIT_BIST2
	PE_DBG("BIST_MODE_2\n");
	ret = tcpci_transmit(pd_port->tcpc_dev, TCPC_TX_BIST_MODE_2, 0, NULL);
#else
	ret = tcpci_set_bist_carrier_mode(pd_port->tcpc_dev, 1 << 2);
#endif
	D("-\n");

	return ret;
}

int hisi_pd_disable_bist_mode2(pd_port_t *pd_port)
{
	D("+-\n");
#ifdef CONFIG_USB_PD_TRANSMIT_BIST2
	return 0;
#else
	return tcpci_set_bist_carrier_mode(pd_port->tcpc_dev, 0);
#endif
}

/* ---- Send / Reply VDM Command ----*/

int hisi_pd_send_svdm_request(pd_port_t *pd_port,
		uint8_t sop_type, uint16_t svid, uint8_t vdm_cmd,
		uint8_t obj_pos, uint8_t cnt, uint32_t *data_obj,
		uint32_t timer_id)
{
#ifdef CONFIG_USB_PD_STOP_SEND_VDM_IF_RX_BUSY
	int rv;
	uint32_t alert_status;
#endif	/* CONFIG_USB_PD_STOP_SEND_VDM_IF_RX_BUSY */

	int ret;
	uint32_t payload[VDO_MAX_SIZE];

	D("+\n");

	payload[0] = VDO_S(svid, CMDT_INIT, vdm_cmd, obj_pos);

	if (data_obj && cnt > 0) {
		memcpy_s(&payload[1], sizeof(uint32_t) * (VDO_MAX_SIZE - 1),
				data_obj, sizeof(uint32_t) * cnt);
	}

#ifdef CONFIG_USB_PD_STOP_SEND_VDM_IF_RX_BUSY
	rv = tcpci_get_alert_status(pd_port->tcpc_dev, &alert_status);
	if (rv)
		return rv;

	if (alert_status & TCPC_REG_ALERT_RX_STATUS) {
		PE_INFO("RX Busy, stop send VDM\n");
		return 0;
	}
#endif	/* CONFIG_USB_PD_STOP_SEND_VDM_IF_RX_BUSY */

	ret = hisi_pd_send_data_msg(
			pd_port, sop_type, PD_DATA_VENDOR_DEF, 1+cnt, payload);

	if (ret == 0 && timer_id != 0)
		pd_enable_timer(pd_port, timer_id);

	D("-\n");
	return ret;
}

int hisi_pd_reply_svdm_request(pd_port_t *pd_port, pd_event_t *pd_event,
				uint8_t reply, uint8_t cnt, uint32_t *data_obj)
{
#ifdef CONFIG_USB_PD_STOP_REPLY_VDM_IF_RX_BUSY
	int rv;
	uint32_t alert_status = 0;
#endif	/* CONFIG_USB_PD_STOP_REPLY_VDM_IF_RX_BUSY */
	uint32_t vdo;
	uint32_t payload[VDO_MAX_SIZE] = {0};

	D("+\n");
	if (!pd_port || !pd_event) {
		PE_INFO("invalid pd_port or pd_event\n");
		return 0;
	}

	vdo = pd_event->pd_msg->payload[0];
	payload[0] = VDO_S(PD_VDO_VID(vdo), reply, PD_VDO_CMD(vdo), PD_VDO_OPOS(vdo));

	if (data_obj && cnt > 0) {
		memcpy_s(&payload[1], sizeof(uint32_t) * (VDO_MAX_SIZE - 1),
				data_obj, sizeof(uint32_t) * cnt);
	}

#ifdef CONFIG_USB_PD_STOP_REPLY_VDM_IF_RX_BUSY
	udelay(300);
	rv = tcpci_get_alert_status(pd_port->tcpc_dev, &alert_status);
	if (rv)
		return rv;

	if (alert_status & TCPC_REG_ALERT_RX_STATUS) {
		PE_INFO("RX Busy, stop reply VDM\n");
		return 0;
	}

#ifdef CONFIG_HISI_TCPC_QUIRK_V100
	/*
	 * Ellisys TD.PD.VDMU.E17
	 * Do not send VDM when Rx busy.
	 */
	mutex_lock(&pd_port->tcpc_dev->access_lock);
	if (pd_port->tcpc_dev->pd_pending_vdm_event) {
		mutex_unlock(&pd_port->tcpc_dev->access_lock);
		PE_INFO("RX Busy, stop reply VDM\n");
		return 0;
	}
	mutex_unlock(&pd_port->tcpc_dev->access_lock);
#endif
#endif	/* CONFIG_USB_PD_STOP_REPLY_VDM_IF_RX_BUSY */

	D("payload: %08x %08x %08x %08x %08x %08x %08x\n", payload[0],
			payload[1], payload[2], payload[3], payload[4],
			payload[5], payload[6]);

	rv = hisi_pd_send_data_msg(pd_port, TCPC_TX_SOP, PD_DATA_VENDOR_DEF, cnt + 1, payload);
	D("-\n");

	return rv;
}

void hisi_pd_lock_msg_output(pd_port_t *pd_port)
{
	D("+-\n");
	if (pd_port->msg_output_lock)
		return;
	pd_port->msg_output_lock = true;
}

void hisi_pd_unlock_msg_output(pd_port_t *pd_port)
{
	D("+-\n");
	if (!pd_port->msg_output_lock)
		return;
	pd_port->msg_output_lock = false;
}

int hisi_pd_update_connect_state(pd_port_t *pd_port, uint8_t state)
{
	int ret;

	D("+\n");
	if (pd_port->pd_connect_state == state)
		return 0;

	switch (state) {
	case HISI_PD_CONNECT_TYPEC_ONLY:
		if (pd_port->power_role == PD_ROLE_SOURCE)
			state = HISI_PD_CONNECT_TYPEC_ONLY_SRC;
		else {
			switch (pd_port->tcpc_dev->typec_remote_rp_level) {
			case TYPEC_CC_VOLT_SNK_DFT:
				state = HISI_PD_CONNECT_TYPEC_ONLY_SNK_DFT;
				break;

			case TYPEC_CC_VOLT_SNK_1_5:
			case TYPEC_CC_VOLT_SNK_3_0:
				state = HISI_PD_CONNECT_TYPEC_ONLY_SNK;
				break;
			}
		}
		break;

	case HISI_PD_CONNECT_PE_READY:
		state = pd_port->power_role == PD_ROLE_SOURCE ?
			HISI_PD_CONNECT_PE_READY_SRC : HISI_PD_CONNECT_PE_READY_SNK;
		break;

	case HISI_PD_CONNECT_NONE:
		break;
	}

	pd_port->pd_connect_state = state;

	/* Call tcpci function, notify other subsystem. */
	ret = tcpci_notify_pd_state(pd_port->tcpc_dev, state);
	D("-\n");
	return ret;
}

void hisi_pd_update_dpm_request_state(pd_port_t *pd_port, uint8_t state)
{
	/* TODO */
}
