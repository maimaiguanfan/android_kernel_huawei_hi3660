/*
 * Copyright (C) 2016 Richtek Technology Corp.
 * Author: TH <tsunghan_tsai@richtek.com>
 *
 * Power Delivery Policy Engine for PRS
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

#define LOG_TAG "[PD][pd_policy_engine_prs.c]"

#include <linux/delay.h>

#include "include/pd_core.h"
#include "include/pd_dpm_core.h"
#include "include/tcpci.h"
#include "include/pd_policy_engine.h"

/*
 * [PD2.0] Figure 8-51:
 *      Dual-Role Port in Source to Sink Power Role Swap State Diagram
 */

void hisi_pe_prs_src_snk_evaluate_pr_swap_entry(
				pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_dpm_prs_evaluate_swap(pd_port, PD_ROLE_SINK);
	pd_free_pd_event(pd_port, pd_event);
	D("-\n");
}

void hisi_pe_prs_src_snk_accept_pr_swap_entry(
				pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_notify_pe_execute_pr_swap(pd_port, true);
	hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_ACCEPT);
	D("-\n");
}

void hisi_pe_prs_src_snk_transition_to_off_entry(
				pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_lock_msg_output(pd_port);	/* for tSRCTransition */
	hisi_pd_notify_pe_execute_pr_swap(pd_port, true);

#ifdef CONFIG_HISI_TCPC_QUIRK_V100
	hisi_pd_set_vbus_detect(pd_port, false);
#endif
	pd_enable_timer(pd_port, PD_TIMER_SOURCE_TRANSITION);	/* shutdown vbus later */
	pd_free_pd_event(pd_port, pd_event);
	D("-\n");
}

void hisi_pe_prs_src_snk_assert_rd_entry(
				pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_dpm_prs_change_role(pd_port, PD_ROLE_SINK);
	D("-\n");
}

void hisi_pe_prs_src_snk_wait_source_on_entry(
				pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_set_rx_enable(pd_port, PD_RX_CAP_PE_READY_UFP);
	hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_PS_RDY);
	D("-\n");
}

/*
 * Wait for vbus valid ?
 * Called after PS_RDY received.
 */
void hisi_pe_prs_src_snk_wait_source_on_exit(
			pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	pd_disable_timer(pd_port, PD_TIMER_PS_SOURCE_ON);
	D("-\n");
}

void hisi_pe_prs_src_snk_send_swap_entry(
				pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_PR_SWAP);
	D("-\n");
}

void hisi_pe_prs_src_snk_reject_pr_swap_entry(
				pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	if (pd_event->msg_sec == PD_DPM_NAK_REJECT)
		hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_REJECT);
	else
		hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_WAIT);
	D("-\n");
}

/*
 * [PD2.0] Figure 8-52:
 *      Dual-role Port in Sink to Source Power Role Swap State Diagram
 */

void hisi_pe_prs_snk_src_evaluate_pr_swap_entry(
				pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_dpm_prs_evaluate_swap(pd_port, PD_ROLE_SOURCE);
	pd_free_pd_event(pd_port, pd_event);
	D("-\n");
}

void hisi_pe_prs_snk_src_accept_pr_swap_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_notify_pe_execute_pr_swap(pd_port, true);
#ifdef CONFIG_HISI_TCPC_QUIRK_V100
	/* disable vbus detect before send accept message */
	hisi_pd_set_vbus_detect(pd_port, false);
#endif
	hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_ACCEPT);
	D("-\n");
}

void hisi_pe_prs_snk_src_transition_to_off_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	/*
	 * Sink should call hisi_pd_notify_pe_execute_pr_swap before this state,
	 * because source may turn off power & change CC before we got
	 * GoodCRC or Accept.
	 */

	D("+\n");
	pd_port->during_swap = true;
	pd_enable_timer(pd_port, PD_TIMER_PS_SOURCE_OFF);
	hisi_pd_dpm_prs_turn_off_power_sink(pd_port);
	pd_free_pd_event(pd_port, pd_event);

	D("-\n");
}

/* called after PD_RDY received from source */
void hisi_pe_prs_snk_src_transition_to_off_exit(
				pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	pd_disable_timer(pd_port, PD_TIMER_PS_SOURCE_OFF);
	D("-\n");
}

/* called after PD_RDY received from source */
void hisi_pe_prs_snk_src_assert_rp_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
#ifdef CONFIG_HISI_TCPC_SINK_PRS_QUIRK
	/* hisi tcpc quirk: must enable vbus first and wait for vbus valid,
	 * then enable Rp */
	pd_put_dpm_ack_event(pd_port);
#else
#ifdef CONFIG_HISI_TCPC_QUIRK_V100
	msleep(100);
#endif
	hisi_pd_dpm_prs_change_role(pd_port, PD_ROLE_SOURCE);
#endif
	pd_free_pd_event(pd_port, pd_event);
	D("-\n");
}

void hisi_pe_prs_snk_src_source_on_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{

	D("+\n");

#ifdef CONFIG_HISI_TCPC_SINK_PRS_QUIRK
	tcpci_source_vbus(pd_port->tcpc_dev, TCP_VBUS_CTRL_PR_SWAP, TCPC_VBUS_SOURCE_5V, -1);
	tcpci_wait_source_vbus(pd_port->tcpc_dev, true);

	/* hisi tcpc quirk: must enable vbus first and wait for vbus valid,
	 * then enable Rp */
	tcpci_tcpc_print_pd_fsm_state(pd_port->tcpc_dev);
	hisi_pd_set_power_role(pd_port, PD_ROLE_SOURCE);
	tcpci_tcpc_print_pd_fsm_state(pd_port->tcpc_dev);

	hisi_pd_enable_vbus_valid_detection(pd_port, true);
#else
	tcpci_tcpc_print_pd_fsm_state(pd_port->tcpc_dev);
	hisi_pd_dpm_prs_enable_power_source(pd_port, true);
#endif
	D("-\n");
}

/* called after PS_RDY send sucessfully */
void hisi_pe_prs_snk_src_source_on_exit(pd_port_t *pd_port, pd_event_t *pd_event)
{
	/*
	 * Do it in process_event after source_on
	 * hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_PS_RDY);
	 */
	/*
	 * send PS_RDY after vbus valid. actualy send PS_RDY in
	 * pd_process_hw_msg_vbus_present
	 */

#ifdef CONFIG_HISI_TCPC_QUIRK_V100
	hisi_pd_set_vbus_detect(pd_port, true);
	tcpci_tcpc_print_pd_fsm_state(pd_port->tcpc_dev);
#endif
}

void hisi_pe_prs_snk_src_send_swap_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	hisi_pd_notify_pe_execute_pr_swap(pd_port, false);
	hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_PR_SWAP);
	D("-\n");
}

void hisi_pe_prs_snk_src_reject_swap_entry(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	if (pd_event->msg_sec == PD_DPM_NAK_REJECT)
		hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_REJECT);
	else
		hisi_pd_send_ctrl_msg(pd_port, TCPC_TX_SOP, PD_CTRL_WAIT);
	D("-\n");
}
