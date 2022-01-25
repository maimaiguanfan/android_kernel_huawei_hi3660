/*
 * Copyright (C) 2016 Richtek Technology Corp.
 * Author: TH <tsunghan_tsai@richtek.com>
 *
 * Power Delivery Policy Engine for DBGACC
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

#define LOG_TAG "[PD][pd_policy_engine_dbg.c]"

#include "include/pd_core.h"
#include "include/pd_dpm_core.h"
#include "include/tcpci.h"
#include "include/pd_policy_engine.h"

#ifdef CONFIG_USB_PD_CUSTOM_DBGACC_SUPPORT

void hisi_pe_dbg_ready_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	uint8_t state;

	if (pd_port->pe_ready)
		return;

	pd_port->pe_ready = true;
	pd_port->state_machine = PE_STATE_MACHINE_DBGACC;
	D("state_machine %s\n", pd_pe_state_machine_name(pd_port->state_machine));

	if (pd_port->data_role == PD_ROLE_UFP) {
		PE_INFO("Custom_DBGACC : UFP\r\n");
		state = HISI_PD_CONNECT_PE_READY_DBGACC_UFP;
		hisi_pd_set_rx_enable(pd_port, PD_RX_CAP_PE_READY_UFP);
	} else {
		PE_INFO("Custom_DBGACC : DFP\r\n");
		state = HISI_PD_CONNECT_PE_READY_DBGACC_UFP;
		hisi_pd_set_rx_enable(pd_port, PD_RX_CAP_PE_READY_DFP);
	}

	hisi_pd_reset_protocol_layer(pd_port);
	hisi_pd_update_connect_state(pd_port, state);
}

#endif /* CONFIG_USB_PD_CUSTOM_DBGACC_SUPPORT */
