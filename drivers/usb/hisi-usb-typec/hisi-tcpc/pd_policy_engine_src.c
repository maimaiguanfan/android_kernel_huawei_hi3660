/*
 * Copyright (C) 2016 Richtek Technology Corp.
 * Author: TH <tsunghan_tsai@richtek.com>
 *
 * Power Delivery Policy Engine for SRC
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

#define LOG_TAG "[PD][engine_src]"


#include <linux/delay.h>
#include "include/pd_core.h"
#include "include/pd_dpm_core.h"
#include "include/tcpci.h"
#include "include/pd_policy_engine.h"

/*
 * [PD2.0] Figure 8-38 Source Port Policy Engine state diagram
 */

void hisi_pe_src_startup_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	pd_port->state_machine = PE_STATE_MACHINE_SOURCE;
	D("state_machine %s\n", pd_pe_state_machine_name(pd_port->state_machine));

	pd_port->cap_counter = 0;
	pd_port->request_i = -1;
	pd_port->request_v = TCPC_VBUS_SOURCE_5V;

	hisi_pd_reset_protocol_layer(pd_port);
	hisi_pd_set_rx_enable(pd_port, PD_RX_CAP_PE_STARTUP);

	switch (pd_event->event_type) {
	case PD_EVT_HW_MSG:	/* CC attached */
		hisi_pd_enable_vbus_valid_detection(pd_port, true);
		break;

	case PD_EVT_PE_MSG: /* From Hard-Reset */
		pd_enable_timer(pd_port, PD_TIMER_SOURCE_START);
		break;

	case PD_EVT_CTRL_MSG: /* From PR-SWAP (Received PS_RDY) */
		pd_enable_timer(pd_port, PD_TIMER_SOURCE_START);
		break;
	}
	D("-\n");
}

void hisi_pe_src_discovery_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	/* MessageID Should be 0 for First SourceCap (Ellisys)... */

	/* The SourceCapabilitiesTimer continues to run during the states
	 * defined in Source Startup Structured VDM Discover Identity State
	 * Diagram
	 */

	D("+\n");
	pd_port->msg_id_tx[TCPC_TX_SOP] = 0;
	pd_port->pd_connected = false;

	pd_enable_timer(pd_port, PD_TIMER_SOURCE_CAPABILITY);

#ifdef CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID
	if (pd_is_auto_discover_cable_id(pd_port)) {
		pd_port->msg_id_tx[TCPC_TX_SOP_PRIME] = 0;
		pd_enable_timer(pd_port, PD_TIMER_DISCOVER_ID);
	}
#endif
	D("-\n");
}

void hisi_pe_src_send_capabilities_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");

	hisi_pd_set_rx_enable(pd_port, PD_RX_CAP_PE_SEND_WAIT_CAP);

	hisi_pd_dpm_send_source_caps(pd_port);
	pd_port->cap_counter++;

	pd_free_pd_event(pd_port, pd_event);	/* soft-reset */
	D("-\n");
}

void hisi_pe_src_send_capabilities_exit(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	pd_disable_timer(pd_port, PD_TIMER_SENDER_RESPONSE);
	D("-\n");
}

void hisi_pe_src_negotiate_capabilities_entry(
				pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	pd_port->pd_connected = true;
	pd_port->pd_prev_connected = true;
	D("set pd_prev_connected %d\n", pd_port->pd_prev_connected);

	hisi_pd_dpm_src_evaluate_request(pd_port, pd_event);
	pd_free_pd_event(pd_port, pd_event);
	D("-\n");
}

void hisi_pe_src_transition_supply_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	if (pd_event->msg == PD_DPM_PD_REQUEST)	/* goto-min */ {
		pd_port->request_i_new = pd_port->request_i_op;
		hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_GOTO_MIN);
	} else
		hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_ACCEPT);

	pd_enable_timer(pd_port, PD_TIMER_SOURCE_TRANSITION);
	D("-\n");
}

void hisi_pe_src_transition_supply_exit(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	pd_disable_timer(pd_port, PD_TIMER_SOURCE_TRANSITION);
	D("-\n");
}

void hisi_pe_src_transition_supply2_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_PS_RDY);
	D("-\n");
}

void hisi_pe_src_ready_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	pd_port->state_machine = PE_STATE_MACHINE_SOURCE;
	D("state_machine %s\n", pd_pe_state_machine_name(pd_port->state_machine));
	hisi_pd_notify_pe_src_explicit_contract(pd_port);
	hisi_pe_power_ready_entry(pd_port, pd_event);
	D("-\n");
}

void hisi_pe_src_disabled_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_set_rx_enable(pd_port, PD_RX_CAP_PE_DISABLE);
	hisi_pd_update_connect_state(pd_port, HISI_PD_CONNECT_TYPEC_ONLY);
	D("-\n");
}

void hisi_pe_src_capability_response_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	switch (pd_event->msg_sec) {
	case PD_DPM_NAK_REJECT_INVALID:
		pd_port->invalid_contract = true;
		/* fall through */
	case PD_DPM_NAK_REJECT:
		hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_REJECT);
		break;

	case PD_DPM_NAK_WAIT:
		hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_WAIT);
		break;
	default:
		break;
	}
	D("-\n");
}

void hisi_pe_src_hard_reset_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_send_hard_reset(pd_port);

	pd_free_pd_event(pd_port, pd_event);
	pd_enable_timer(pd_port, PD_TIMER_PS_HARD_RESET);
	D("-\n");
}

void hisi_pe_src_hard_reset_received_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	pd_enable_timer(pd_port, PD_TIMER_PS_HARD_RESET);
	D("-\n");
}

void hisi_pe_src_transition_to_default_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_reset_local_hw(pd_port);
	hisi_pd_dpm_src_hard_reset(pd_port);
	D("-\n");
}

void hisi_pe_src_transition_to_default_exit(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_dpm_enable_vconn(pd_port, true);
	pd_enable_timer(pd_port, PD_TIMER_NO_RESPONSE);
	D("-\n");
}


void hisi_pe_src_get_sink_cap_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_GET_SINK_CAP);
	D("-\n");
}

void hisi_pe_src_get_sink_cap_exit(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	pd_disable_timer(pd_port, PD_TIMER_SENDER_RESPONSE);
	hisi_pd_dpm_dr_inform_sink_cap(pd_port, pd_event);
	D("-\n");
}

void hisi_pe_src_wait_new_capabilities_entry(
			pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+-\n");
	/* Wait for new Source Capabilities */
}

void hisi_pe_src_send_soft_reset_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_send_soft_reset(pd_port, PE_STATE_MACHINE_SOURCE);
	pd_free_pd_event(pd_port, pd_event);
	D("-\n");
}

void hisi_pe_src_soft_reset_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_handle_soft_reset(pd_port, PE_STATE_MACHINE_SOURCE);
	pd_free_pd_event(pd_port, pd_event);
	D("-\n");
}

void hisi_pe_src_ping_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+-\n");
	/* TODO: Send Ping Message */
}

/*
 * [PD2.0] Figure 8-81
 Source Startup Structured VDM Discover Identity State Diagram (TODO)
 */

#ifdef CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID

void hisi_pe_src_vdm_identity_request_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_set_rx_enable(pd_port, PD_RX_CAP_PE_DISCOVER_CABLE);

	pd_send_vdm_discover_id(pd_port, TCPC_TX_SOP_PRIME);

	pd_port->discover_id_counter++;
	pd_enable_timer(pd_port, PD_TIMER_VDM_RESPONSE);

	pd_free_pd_event(pd_port, pd_event);
	D("-\n");
}

void hisi_pe_src_vdm_identity_acked_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	pd_port->dpm_flags &= ~DPM_FLAGS_CHECK_CABLE_ID;

	pd_disable_timer(pd_port, PD_TIMER_VDM_RESPONSE);
	hisi_pd_dpm_src_inform_cable_vdo(pd_port, pd_event);

	pd_free_pd_event(pd_port, pd_event);
	D("-\n");
}

void hisi_pe_src_vdm_identity_naked_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	pd_disable_timer(pd_port, PD_TIMER_VDM_RESPONSE);
	hisi_pd_dpm_src_inform_cable_vdo(pd_port, pd_event);

	pd_free_pd_event(pd_port, pd_event);
	D("-\n");
}

#endif	/* CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID */
