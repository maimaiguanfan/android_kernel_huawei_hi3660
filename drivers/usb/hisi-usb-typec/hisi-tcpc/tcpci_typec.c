/*
 * Copyright (C) 2018 Hisilicon.
 * Author: Hisilicon <>
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

#include <linux/delay.h>
#include <linux/cpu.h>
#include <linux/hisi/usb/hisi_tcpm.h>

#include "include/tcpci.h"
#include "include/tcpci_typec.h"
#include "include/tcpci_timer.h"


#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
#define CONFIG_TYPEC_CAP_TRY_STATE
#endif

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
#undef	CONFIG_TYPEC_CAP_TRY_STATE
#define CONFIG_TYPEC_CAP_TRY_STATE
#endif

/* #define TYPEC_EXIT_ATTACHED_SRC_NO_DEBOUNCE */
#define TYPEC_EXIT_ATTACHED_SNK_VIA_VBUS

enum TYPEC_WAIT_PS_STATE {
	TYPEC_WAIT_PS_DISABLE = 0,
	TYPEC_WAIT_PS_SNK_VSAFE5V,
	TYPEC_WAIT_PS_SRC_VSAFE0V,
	TYPEC_WAIT_PS_SRC_VSAFE5V,
};

static const char *const typec_wait_ps_name[] = {
	"Disable",
	"SNK_VSafe5V",
	"SRC_VSafe0V",
	"SRC_VSafe5V",
};

enum TYPEC_ROLE_SWAP_STATE {
	TYPEC_ROLE_SWAP_NONE = 0,
	TYPEC_ROLE_SWAP_TO_SNK,
	TYPEC_ROLE_SWAP_TO_SRC,
};

static inline void typec_wait_ps_change(struct tcpc_device *tcpc_dev,
					enum TYPEC_WAIT_PS_STATE state)
{
	D("wait state %d\n", state);

#ifdef CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT
	/* Wait for vsafe0V, enable timer. */
	if (state == TYPEC_WAIT_PS_SRC_VSAFE0V)
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_RT_TIMER_SAFE0V_TOUT);
#endif	/* CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT */

	/* Wait for oter state, cancel old timer. */
	if (tcpc_dev->typec_wait_ps_change == TYPEC_WAIT_PS_SRC_VSAFE0V
			&& state != TYPEC_WAIT_PS_SRC_VSAFE0V) {
		hisi_tcpc_disable_timer(tcpc_dev, TYPEC_RT_TIMER_SAFE0V_DELAY);

#ifdef CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT
		hisi_tcpc_disable_timer(tcpc_dev, TYPEC_RT_TIMER_SAFE0V_TOUT);
#endif	/* CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT */
	}

	tcpc_dev->typec_wait_ps_change = (uint8_t) state;
}

#define typec_get_cc1()		tcpc_dev->typec_remote_cc[0]
#define typec_get_cc2()		tcpc_dev->typec_remote_cc[1]
#define typec_get_cc_res()	(tcpc_dev->typec_polarity ? typec_get_cc2() : typec_get_cc1())

#define typec_check_cc1(cc)		(typec_get_cc1() == cc)
#define typec_check_cc2(cc)		(typec_get_cc2() == cc)
#define typec_check_cc(cc1, cc2)	(typec_check_cc1(cc1) && typec_check_cc2(cc2))
#define typec_check_cc_both(res)	(typec_check_cc(res, res))
#define typec_check_cc_any(res)		(typec_check_cc1(res) || typec_check_cc2(res))
#define typec_is_drp_toggling()		(typec_get_cc1() == TYPEC_CC_DRP_TOGGLING)
#define typec_is_cc_open()		typec_check_cc_both(TYPEC_CC_VOLT_OPEN)

#define typec_is_sink_with_emark()	\
	(typec_get_cc1() + typec_get_cc2() == TYPEC_CC_VOLT_RA + TYPEC_CC_VOLT_RD)

static inline int typec_enable_vconn(struct tcpc_device *tcpc_dev)
{
	D("\n");
	if (typec_is_sink_with_emark())
		return tcpci_set_vconn(tcpc_dev, true);
	return 0;
}

/*
 * TYPEC Connection State Definition
 */

enum TYPEC_CONNECTION_STATE {
	typec_disabled = 0,
	typec_errorrecovery,

	typec_unattached_snk,
	typec_unattached_src,

	typec_attachwait_snk,
	typec_attachwait_src,

	typec_attached_snk,
	typec_attached_src,

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	/* Require : Assert Rp
	 * Exit(-> Attached.SRC) : Detect Rd (tPDDebounce).
	 * Exit(-> TryWait.SNK) : Not detect Rd after tDRPTry
	 * */
	typec_try_src,

	/* Require : Assert Rd
	 * Exit(-> Attached.SNK) : Detect Rp (tCCDebounce) and Vbus present.
	 * Exit(-> Unattached.SNK) : Not detect Rp (tPDDebounce)
	 * */

	typec_trywait_snk,
	typec_trywait_snk_pe,
#endif

#ifdef CONFIG_TYPEC_CAP_TRY_SINK

	/* Require : Assert Rd
	 * Wait for tDRPTry and only then begin monitoring CC.
	 * Exit (-> Attached.SNK) : Detect Rp (tPDDebounce) and Vbus present.
	 * Exit (-> TryWait.SRC) : Not detect Rp for tPDDebounce.
	 */
	typec_try_snk,

	/*
	 * Require : Assert Rp
	 * Exit (-> Attached.SRC) : Detect Rd (tCCDebounce)
	 * Exit (-> Unattached.SNK) : Not detect Rd after tDRPTry
	 */

	typec_trywait_src,
	typec_trywait_src_pe,
#endif	/* CONFIG_TYPEC_CAP_TRY_SINK */

	typec_audioaccessory,
	typec_debugaccessory,

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	typec_attached_dbgacc_snk,
#endif	/* CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT */

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	typec_attached_custom_src,
#endif	/* CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT */

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	typec_role_swap,
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

	typec_unattachwait_pe,	/* Wait Policy Engine go to Idle */
};

static const char *const typec_state_name[] = {
	"Disabled",
	"ErrorRecovery",

	"Unattached.SNK",
	"Unattached.SRC",

	"AttachWait.SNK",
	"AttachWait.SRC",

	"Attached.SNK",
	"Attached.SRC",

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	"Try.SRC",
	"TryWait.SNK",
	"TryWait.SNK.PE",
#endif	/* CONFIG_TYPEC_CAP_TRY_SOURCE */

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	"Try.SNK",
	"TryWait.SRC",
	"TryWait.SRC.PE",
#endif	/* CONFIG_TYPEC_CAP_TRY_SINK */

	"AudioAccessory",
	"DebugAccessory",

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	"DBGACC.SNK",
#endif	/* CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT */

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	"Custom.SRC",
#endif	/* CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT */

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	"RoleSwap",
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

	"UnattachWait.PE",
};

static inline void typec_transfer_state(struct tcpc_device *tcpc_dev,
					enum TYPEC_CONNECTION_STATE state)
{
	TYPEC_INFO("** %s\n", typec_state_name[state]);
	tcpc_dev->typec_state = (uint8_t) state;
}

#define TYPEC_NEW_STATE(state)	(typec_transfer_state(tcpc_dev, state))


#ifdef CONFIG_HISI_TCPC_DEBUG

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
static void dump_pd_port(pd_port_t *pd_port)
{
	I("+\n");
#define PR(format, arg...)	pr_err("[%s] " format, __func__, ##arg)
#define PR_UINT(_name)	PR("%s %u\n", #_name, pd_port->_name)
#define PR_INT(_name)	PR("%s %d\n", #_name, pd_port->_name)
#define PR_BOOL(_name)	PR("%s %s\n", #_name, pd_port->_name ? "true" : "false");

	PR("pd_lock %s\n", mutex_is_locked(&pd_port->pd_lock) ? "locked" : "unlocked");
	PR_BOOL(explicit_contract);
	PR_BOOL(invalid_contract);
	PR_BOOL(vconn_source);
#ifdef CONFIG_USB_PD_DFP_READY_DISCOVER_ID
	PR_BOOL(vconn_return);
#endif
	PR_BOOL(pe_ready);
	PR_BOOL(pd_connected);
	PR_BOOL(pd_prev_connected);
	PR_BOOL(msg_output_lock);

	PR_UINT(state_machine);
	PR_UINT(pd_connect_state);

	PR_BOOL(reset_vdm_state);
	PR_INT(pe_pd_state);
	PR_INT(pe_vdm_state);
	PR_INT(pe_state_next);
	PR_INT(pe_state_curr);
	PR_INT(data_role);
	PR_INT(power_role);
	PR_INT(cap_counter);
	PR_INT(discover_id_counter);
	PR_INT(hard_reset_counter);
	PR_INT(snk_cap_count);
	PR_INT(src_cap_count);
	PR_INT(get_snk_cap_count);
	PR_INT(get_src_cap_count);

	PR_INT(vdm_discard_retry_flag);
	PR_INT(vdm_discard_retry_count);
#ifdef CONFIG_USB_PD_IGNORE_PS_RDY_AFTER_PR_SWAP
	PR_INT(msg_id_pr_swap_last);
#endif
	PR_INT(last_rdo);

	PR_INT(mode_svid);
	PR_INT(mode_obj_pos);
	PR_INT(modal_operation);
	PR_INT(dpm_ack_immediately);

#ifdef CONFIG_USB_PD_DFP_FLOW_DELAY
	PR_INT(dpm_dfp_flow_delay_done);
#endif

#ifdef CONFIG_USB_PD_UFP_FLOW_DELAY
	PR_INT(dpm_ufp_flow_delay_done);
#endif

	PR_INT(dpm_flags);
	PR_INT(dpm_init_flags);
	PR_INT(dpm_caps);
	PR_INT(dpm_dfp_retry_cnt);

#ifdef CONFIG_USB_PD_ALT_MODE_DFP_SUPPORT
	PR_INT(local_dp_config);
	PR_INT(remote_dp_config);
	PR_INT(dp_ufp_u_attention);
	PR_INT(dp_dfp_u_state);
#endif

	PR_INT(dp_status);
	PR_INT(dp_ufp_u_state);
	PR_INT(dp_first_connected);
	PR_INT(dp_second_connected);
#ifdef CONFIG_USB_PD_UVDM_SUPPORT
	PR_INT(uvdm_wait_resp);
	PR_INT(uvdm_cnt);
	PR_INT(uvdm_svid);
#endif
	PR_INT(postpone_pr_swap);
	PR_INT(postpone_dr_swap);

#undef PR_UINT
#undef PR_INT
#undef PR_BOOL
#undef PR
	I("-\n");
}
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

int dump_tcpc_dev(void)
{
	struct tcpc_device *tcpc_dev;

	I("+\n");
	tcpc_dev = hisi_tcpc_dev_get_by_name("hisi-tcpc");
	if (!tcpc_dev) {
		E("can not find hisi-tcpc\n");
		return -ENODEV;
	}
#define PR(format, arg...)	pr_err("[%s] " format, __func__, ##arg)
#define PR_UINT(_name)	PR("%s %u\n", #_name, tcpc_dev->_name)
#define PR_INT(_name)	PR("%s %d\n", #_name, tcpc_dev->_name)
#define PR_BOOL(_name)	PR("%s %s\n", #_name, tcpc_dev->_name ? "true" : "false");

	PR("desc: role_def %d, rp_lvl %d, name %s\n",
			tcpc_dev->desc.role_def,
			tcpc_dev->desc.rp_lvl,
			tcpc_dev->desc.name);
	PR_BOOL(wake_lock_user);
	PR_BOOL(wake_lock_pd);
	PR("attach_wake_lock active %d\n",
			wake_lock_active(&tcpc_dev->attach_wake_lock));
	PR("dettach_temp_wake_lock active %d\n",
			wake_lock_active(&tcpc_dev->dettach_temp_wake_lock));

	PR("access_lock %s\n", mutex_is_locked(&tcpc_dev->access_lock) ? "locked" : "unlocked");
	PR("typec_lock %s\n", mutex_is_locked(&tcpc_dev->typec_lock) ? "locked" : "unlocked");
	PR("timer_lock %s\n", mutex_is_locked(&tcpc_dev->timer_lock) ? "locked" : "unlocked");
	PR("timer_enable_mask_lock %d\n", tcpc_dev->timer_enable_mask_lock.count);

	PR("pending_event %d\n", atomic_read(&tcpc_dev->pending_event));
	PR("timer_tick %llu\n", tcpc_dev->timer_tick);
	PR("timer_enable_mask 0x%llx\n", tcpc_dev->timer_enable_mask);

	PR_BOOL(timer_thead_stop);
	PR_BOOL(event_loop_thead_stop);


	PR("typec_state %d(%s)\n", tcpc_dev->typec_state,
			typec_state_name[tcpc_dev->typec_state]);
	PR("typec_role %d(%s)\n", tcpc_dev->typec_role,
			typec_role_name[tcpc_dev->typec_role]);
	PR("typec_attach_old %d(%s)\n", tcpc_dev->typec_attach_old,
			typec_attach_type_name(tcpc_dev->typec_attach_old));
	PR("typec_attach_new %d(%s)\n", tcpc_dev->typec_attach_new,
			typec_attach_type_name(tcpc_dev->typec_attach_new));
	PR("typec_local_cc 0x%x\n", tcpc_dev->typec_local_cc);
	PR("typec_local_rp_level 0x%x\n", tcpc_dev->typec_local_rp_level);
	PR("typec_remote_cc 0x%x 0x%x\n", tcpc_dev->typec_remote_cc[0],
			tcpc_dev->typec_remote_cc[1]);
	PR("typec_remote_rp_level 0x%x\n", tcpc_dev->typec_remote_rp_level);
	PR("typec_wait_ps_change %d(%s)\n", tcpc_dev->typec_wait_ps_change,
			typec_wait_ps_name[tcpc_dev->typec_wait_ps_change]);

	PR_INT(typec_polarity);
	PR_BOOL(typec_drp_try_timeout);
	PR_BOOL(typec_lpm);
	PR_BOOL(typec_cable_only);
	PR_BOOL(typec_power_ctrl);

	PR_INT(typec_usb_sink_curr);
	PR_UINT(typec_lpm_pull);
	PR_UINT(typec_lpm_retry);

	PR_UINT(typec_during_role_swap);

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	PR_UINT(pd_event_count);
	PR_UINT(pd_event_head_index);
	PR_UINT(pd_msg_buffer_allocated);

	PR_UINT(pd_last_vdm_msg_id);
	PR_BOOL(pd_pending_vdm_event);
	PR_BOOL(pd_pending_vdm_reset);
	PR_BOOL(pd_pending_vdm_good_crc);
	PR_BOOL(pd_pending_vdm_discard);
	PR_BOOL(pd_pending_vdm_attention);
	PR_BOOL(pd_postpone_vdm_timeout);

	PR("pd_last_vdm_msg: ...\n");
	PR("pd_attention_vdm_msg: ...\n");
	PR("pd_vdm_event: ...\n");

	PR_BOOL(pd_pe_running);
	PR_BOOL(pd_wait_pe_idle);
	PR_BOOL(pd_hard_reset_event_pending);
	PR_BOOL(pd_wait_hard_reset_complete);
	PR_BOOL(pd_wait_pr_swap_complete);
	PR_BOOL(pd_wait_error_recovery);
	PR_BOOL(pd_ping_event_pending);

	PR_UINT(pd_bist_mode);
	PR_UINT(pd_transmit_state);
	PR_INT(pd_wait_vbus_once);

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	PR_BOOL(pd_discard_pending);
#endif

	PR("tcpc_flags 0x%x\n", tcpc_dev->tcpc_flags);
	dump_pd_port(&tcpc_dev->pd_port);
#endif

	PR("vbus_level %u(%s)\n", tcpc_dev->vbus_level,
			tcpm_vbus_level_name(tcpc_dev->vbus_level));
	PR("irq_enabled %u\n", tcpc_dev->irq_enabled);
	PR_BOOL(typec_during_direct_charge);
	PR_BOOL(vbus_detect);

#undef PR_UINT
#undef PR_INT
#undef PR_BOOL
#undef PR

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	dump_pd_port(&tcpc_dev->pd_port);
#endif

	I("-\n");
	return 0;
}
#endif /* CONFIG_HISI_TCPC_DEBUG */


static inline int typec_set_polarity(struct tcpc_device *tcpc_dev, bool polarity)
{
	D("+-\n");
	tcpc_dev->typec_polarity = polarity;
	return tcpci_set_polarity(tcpc_dev, polarity);
}


static inline int typec_set_plug_orient(struct tcpc_device *tcpc_dev,
				uint8_t res, bool polarity)
{
	int rv;

	D("+\n");
	rv = typec_set_polarity(tcpc_dev, polarity);
	if (rv)
		return rv;

	tcpci_tcpc_print_pd_fsm_state(tcpc_dev);

	tcpci_set_cc(tcpc_dev, res);
	D("-\n");
	return 0;
}

/**
 * Handle state change.
 */
static int typec_alert_attach_state_change(struct tcpc_device *tcpc_dev)
{
	int ret = 0;

	D("+\n");
	if (tcpc_dev->typec_attach_old == tcpc_dev->typec_attach_new) {
		TYPEC_INFO("Attached-> %s(repeat)\n",
			typec_attach_type_name(tcpc_dev->typec_attach_new));
		return 0;
	}

	TYPEC_INFO("Attached-> %s\n",
			typec_attach_type_name(tcpc_dev->typec_attach_new));

	/* Report function: report port change to pd engine. */
	ret = hisi_tcpci_report_usb_port_changed(tcpc_dev);

	/* The typec_attach_old have three assignment: 1.here; 2.prs; 3.init */
	tcpc_dev->typec_attach_old = tcpc_dev->typec_attach_new;
	D("set typec_attach_old %d\n", tcpc_dev->typec_attach_old);
	D("-\n");
	return ret;
}

static inline int typec_try_low_power_mode(struct tcpc_device *tcpc_dev)
{
	int ret;

	D("+\n");

	ret = tcpci_set_low_power_mode(tcpc_dev, true, tcpc_dev->typec_lpm_pull);
	if (ret < 0)
		return ret;

	ret = tcpci_is_low_power_mode(tcpc_dev);
	if (ret < 0)
		return ret;

	if (ret == 1)
		return 0;

	if (tcpc_dev->typec_lpm_retry == 0) {
		TCPC_INFO("Enter low power mode failed\n");
		return 0;
	}

	tcpc_dev->typec_lpm_retry--;
	hisi_tcpc_enable_timer(tcpc_dev, TYPEC_RT_TIMER_LOW_POWER_MODE);

	D("-\n");
	return 0;
}

static inline int typec_enter_low_power_mode(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	hisi_tcpc_enable_timer(tcpc_dev, TYPEC_RT_TIMER_LOW_POWER_MODE);
	D("-\n");
	return 0;
}

static inline int typec_enable_low_power_mode(
			struct tcpc_device *tcpc_dev, int pull)
{
	int ret = 0;

	D("+\n");
	if (tcpc_dev->typec_cable_only) {
		TYPEC_DBG("LPM_RaOnly\n");
		return 0;
	}

	if (tcpc_dev->typec_lpm != true){
		tcpc_dev->typec_lpm = true;
		tcpc_dev->typec_lpm_retry = 5;
		tcpc_dev->typec_lpm_pull = pull;
		ret = typec_enter_low_power_mode(tcpc_dev);
	}
	D("-\n");
	return ret;
}

static inline int typec_disable_low_power_mode(
	struct tcpc_device *tcpc_dev)
{
	int ret = 0;
	D("+\n");

	if (tcpc_dev->typec_lpm != false){
		tcpc_dev->typec_lpm = false;
		hisi_tcpc_disable_timer(tcpc_dev, TYPEC_RT_TIMER_LOW_POWER_MODE);
		ret = tcpci_set_low_power_mode(tcpc_dev, false, TYPEC_CC_DRP);
	}
	D("-\n");
	return ret;
}

static void typec_unattached_power_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

	if (tcpc_dev->typec_power_ctrl) {
		tcpci_set_vconn(tcpc_dev, false);
		tcpci_disable_vbus_control(tcpc_dev);
		hisi_tcpci_report_power_control(tcpc_dev, false);
	}
	D("-\n");
}

extern void hisi_tcpc_poll_cc_status(struct tcpc_device *tcpc);

static inline void typec_unattached_cc_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");

	tcpc_dev->typec_during_direct_charge = false;

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc_dev->typec_during_role_swap) {
		TYPEC_NEW_STATE(typec_role_swap);
		return;
	}
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

	switch (tcpc_dev->typec_role) {
	case TYPEC_ROLE_SNK:
		D("TYPEC_ROLE_SNK\n");
		TYPEC_NEW_STATE(typec_unattached_snk);
		tcpci_set_cc(tcpc_dev, TYPEC_CC_RD);
		typec_enable_low_power_mode(tcpc_dev, TYPEC_CC_RD);
		break;
	case TYPEC_ROLE_SRC:
		D("TYPEC_ROLE_SRC\n");
		TYPEC_NEW_STATE(typec_unattached_src);
		tcpci_set_cc(tcpc_dev, TYPEC_CC_RP);
		typec_enable_low_power_mode(tcpc_dev, TYPEC_CC_RP);
		break;
	default:
		/* DRP */
		D("typec_state: %s\n", typec_state_name[tcpc_dev->typec_state]);
		switch (tcpc_dev->typec_state) {
		case typec_attachwait_snk:
		case typec_audioaccessory:
			TYPEC_NEW_STATE(typec_unattached_src);
#ifdef CONFIG_HISI_TCPC_QUIRK_V100
			tcpci_set_cc(tcpc_dev, TYPEC_CC_RP_DFT);
#else
			tcpci_set_cc(tcpc_dev, TYPEC_CC_RP_1_5); /* Why Rp 1.5A */
#endif
			/* Don't DRP now, but wait 20ms. */
			hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_DRP_SRC_TOGGLE);
			break;
		default:
			TYPEC_NEW_STATE(typec_unattached_snk);

#ifdef CONFIG_HISI_TCPC_QUIRK_V100
			if (!tcpc_dev->vbus_detect) {
				D("enable vbus_detect\n");
				tcpci_set_vbus_detect(tcpc_dev, true);
			}

			tcpci_tcpc_print_pd_fsm_state(tcpc_dev);
			D("typec_local_cc %d\n", tcpc_dev->typec_local_cc);
			tcpci_reset_pd_fsm(tcpc_dev);
			tcpci_tcpc_print_pd_fsm_state(tcpc_dev);
#endif
			tcpci_set_cc(tcpc_dev, TYPEC_CC_DRP);
			typec_enable_low_power_mode(tcpc_dev, TYPEC_CC_DRP);
			hisi_tcpc_poll_cc_status(tcpc_dev);
			break;
		}
		break;
	}

	D("-\n");
}

static void typec_unattached_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	typec_unattached_cc_entry(tcpc_dev);
	typec_unattached_power_entry(tcpc_dev);
	D("-\n");
}

/**
 * Wait for PD Engine enter Idle state.
 * If PD is enabled, a timer will call typec_unattached_entry later.
 */
static void typec_unattach_wait_pe_idle_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	tcpc_dev->typec_attach_new = TYPEC_UNATTACHED;
	D("set typec_attach_new %d\n", tcpc_dev->typec_attach_new);

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (tcpc_dev->typec_attach_old) {
		TYPEC_NEW_STATE(typec_unattachwait_pe);
		return;
	}
#endif

	typec_unattached_entry(tcpc_dev);
	D("-\n");
}

/**
 * This function called when vbus present 5V.
 */
static void typec_source_attached_with_vbus_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	tcpc_dev->typec_attach_new = TYPEC_ATTACHED_SRC;
	D("set typec_attach_new %d\n", tcpc_dev->typec_attach_new);
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);
	D("-\n");
}

static inline void typec_source_attached_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	TYPEC_NEW_STATE(typec_attached_src);
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_SRC_VSAFE5V);

	/* Assign typec_attach_new ? No !
	 * Assign typec_attach_new after vbus valid. */
	hisi_tcpc_disable_timer(tcpc_dev, TYPEC_TRY_TIMER_DRP_TRY);

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc_dev->typec_during_role_swap) {
		tcpc_dev->typec_during_role_swap = TYPEC_ROLE_SWAP_NONE;
		hisi_tcpc_disable_timer(tcpc_dev, TYPEC_RT_TIMER_ROLE_SWAP_STOP);
	}
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

	typec_set_plug_orient(tcpc_dev, tcpc_dev->typec_local_rp_level,
			typec_check_cc2(TYPEC_CC_VOLT_RD));

	hisi_tcpci_report_power_control(tcpc_dev, true);
	typec_enable_vconn(tcpc_dev);
	tcpci_source_vbus(tcpc_dev, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SOURCE_5V, -1);
	D("-\n");
}

static inline void typec_sink_attached_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	TYPEC_NEW_STATE(typec_attached_snk);
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

	tcpc_dev->typec_attach_new = TYPEC_ATTACHED_SNK;
	D("set typec_attach_new %d\n", tcpc_dev->typec_attach_new);

#ifdef CONFIG_TYPEC_CAP_TRY_STATE
	if (tcpc_dev->typec_role >= TYPEC_ROLE_DRP)
		hisi_tcpc_reset_typec_try_timer(tcpc_dev);
#endif	/* CONFIG_TYPEC_CAP_TRY_STATE */

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc_dev->typec_during_role_swap) {
		tcpc_dev->typec_during_role_swap = TYPEC_ROLE_SWAP_NONE;
		hisi_tcpc_disable_timer(tcpc_dev, TYPEC_RT_TIMER_ROLE_SWAP_STOP);
	}
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

	typec_set_plug_orient(tcpc_dev, TYPEC_CC_RD, !typec_check_cc2(TYPEC_CC_VOLT_OPEN));
	tcpc_dev->typec_remote_rp_level = typec_get_cc_res();

	hisi_tcpci_report_power_control(tcpc_dev, true);
	tcpci_sink_vbus(tcpc_dev, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, -1);
	D("-\n");
}

static inline void typec_custom_src_attached_entry(struct tcpc_device *tcpc_dev)
{
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	int cc1 = typec_get_cc1();
	int cc2 = typec_get_cc2();

	if (cc1 == TYPEC_CC_VOLT_SNK_DFT && cc2 == TYPEC_CC_VOLT_SNK_DFT) {
		TYPEC_NEW_STATE(typec_attached_custom_src);
		tcpc_dev->typec_attach_new = TYPEC_ATTACHED_CUSTOM_SRC;
		D("set typec_attach_new %d\n", tcpc_dev->typec_attach_new);
		return;
	}
#endif	/* CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT */

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	D("[Warning] Same Rp (%d)\n", typec_get_cc1());
#else
	D("[Warning] CC Both Rp\n");
#endif
}

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT

static inline uint8_t typec_get_sink_dbg_acc_rp_level(int cc1, int cc2)
{
	D("\n");
	if (cc2 == TYPEC_CC_VOLT_SNK_DFT)
		return (uint8_t)cc1;
	else
		return TYPEC_CC_VOLT_SNK_DFT;
}

static inline void typec_sink_dbg_acc_attached_entry(struct tcpc_device *tcpc_dev)
{
	bool polarity;
	uint8_t rp_level;
	int cc1 = typec_get_cc1();
	int cc2 = typec_get_cc2();

	D("+\n");
	if (cc1 == cc2) {
		typec_custom_src_attached_entry(tcpc_dev);
		return;
	}

	TYPEC_NEW_STATE(typec_attached_dbgacc_snk);

	tcpc_dev->typec_attach_new = TYPEC_ATTACHED_DBGACC_SNK;
	D("set typec_attach_new %d\n", tcpc_dev->typec_attach_new);

	polarity = cc2 > cc1;

	if (polarity)
		rp_level = typec_get_sink_dbg_acc_rp_level(cc2, cc1);
	else
		rp_level = typec_get_sink_dbg_acc_rp_level(cc1, cc2);

	typec_set_plug_orient(tcpc_dev, TYPEC_CC_RD, polarity);
	tcpc_dev->typec_remote_rp_level = rp_level;

	hisi_tcpci_report_power_control(tcpc_dev, true);
	tcpci_sink_vbus(tcpc_dev, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, -1);
	D("-\n");
}
#else
static inline void typec_sink_dbg_acc_attached_entry(struct tcpc_device *tcpc_dev)
{
	typec_custom_src_attached_entry(tcpc_dev);
}
#endif	/* CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT */

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE

static inline bool typec_role_is_try_src(struct tcpc_device *tcpc_dev)
{
	D("typec_tole %d, typec_during_role_swap %d\n", tcpc_dev->typec_role,
				tcpc_dev->typec_during_role_swap);
	if (tcpc_dev->typec_role != TYPEC_ROLE_TRY_SRC)
		return false;

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc_dev->typec_during_role_swap)
		return false;
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

	return true;
}

static inline void typec_try_src_entry(struct tcpc_device *tcpc_dev)
{
	TYPEC_NEW_STATE(typec_try_src);
	tcpc_dev->typec_drp_try_timeout = false;

	tcpci_set_cc(tcpc_dev, TYPEC_CC_RP);
	hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TRY_TIMER_DRP_TRY);
}

static inline void typec_trywait_snk_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	TYPEC_NEW_STATE(typec_trywait_snk);
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

	tcpci_set_vconn(tcpc_dev, false);
	tcpci_set_cc(tcpc_dev, TYPEC_CC_RD);
	tcpci_source_vbus(tcpc_dev, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SOURCE_0V, 0);
	hisi_tcpc_disable_timer(tcpc_dev, TYPEC_TRY_TIMER_DRP_TRY);

	hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
	D("-\n");
}

static inline void typec_trywait_snk_pe_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	tcpc_dev->typec_attach_new = TYPEC_UNATTACHED;
	D("set typec_attach_new %d\n", tcpc_dev->typec_attach_new);

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (tcpc_dev->typec_attach_old) {
		TYPEC_NEW_STATE(typec_trywait_snk_pe);
		return;
	}
#endif

	typec_trywait_snk_entry(tcpc_dev);
	D("-\n");
}

#endif /* CONFIG_TYPEC_CAP_TRY_SOURCE */

#ifdef CONFIG_TYPEC_CAP_TRY_SINK

static inline bool typec_role_is_try_sink(struct tcpc_device *tcpc_dev)
{
	D("typec_role %d, typec_during_role_swap %d\n", tcpc_dev->typec_role,
				tcpc_dev->typec_during_role_swap);
	if (tcpc_dev->typec_role != TYPEC_ROLE_TRY_SNK)
		return false;

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc_dev->typec_during_role_swap)
		return false;
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

	return true;
}

static inline void typec_try_snk_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	TYPEC_NEW_STATE(typec_try_snk);
	tcpc_dev->typec_drp_try_timeout = false;

	tcpci_set_cc(tcpc_dev, TYPEC_CC_RD);
	hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TRY_TIMER_DRP_TRY);
	D("-\n");
}

static inline void typec_trywait_src_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	TYPEC_NEW_STATE(typec_trywait_src);
	tcpc_dev->typec_drp_try_timeout = false;

	tcpci_set_cc(tcpc_dev, TYPEC_CC_RP_DFT);
	tcpci_sink_vbus(tcpc_dev, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_0V, 0); // stop sink vbus
	hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TRY_TIMER_DRP_TRY);
	D("-\n");
}

#endif /* CONFIG_TYPEC_CAP_TRY_SINK */

static inline void typec_cc_snk_detect_vsafe5v_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

	if (!typec_check_cc_any(TYPEC_CC_VOLT_OPEN)) {	/* Both Rp */
		typec_sink_dbg_acc_attached_entry(tcpc_dev);
		return;
	}

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	if (typec_role_is_try_src(tcpc_dev)) {
		if (tcpc_dev->typec_state == typec_attachwait_snk) {
			typec_try_src_entry(tcpc_dev);
			return;
		}
	}
#endif /* CONFIG_TYPEC_CAP_TRY_SOURCE */

	typec_sink_attached_entry(tcpc_dev);
	D("-\n");
}

/**
 * If Port Partner act as Source without VBUS, wait vSafe5V
 */
static inline void typec_cc_snk_detect_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	if (tcpci_check_vbus_valid(tcpc_dev))
		typec_cc_snk_detect_vsafe5v_entry(tcpc_dev);
	else{
		typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_SNK_VSAFE5V);
	}
	D("-\n");
}

static inline void typec_cc_src_detect_vsafe0v_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	if (typec_role_is_try_sink(tcpc_dev)) {
		if (tcpc_dev->typec_state == typec_attachwait_src) {
			D("to try SNK\n");
			typec_try_snk_entry(tcpc_dev);
			return;
		}
	}
#endif /* CONFIG_TYPEC_CAP_TRY_SINK */

	typec_source_attached_entry(tcpc_dev);
	D("-\n");
}

/**
 * If Port Partner act as Sink with low VBUS, wait vSafe0v
 */
static inline void typec_cc_src_detect_entry(struct tcpc_device *tcpc_dev)
{
	bool vbus_absent;

	D("+\n");
	vbus_absent = tcpci_check_vsafe0v(tcpc_dev, true);
	if (vbus_absent)
		typec_cc_src_detect_vsafe0v_entry(tcpc_dev);
	else
		typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_SRC_VSAFE0V);
	D("-\n");
}

static inline void typec_cc_src_remove_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	if (typec_role_is_try_src(tcpc_dev)) {
		switch (tcpc_dev->typec_state) {
		case typec_attached_src:
			typec_trywait_snk_pe_entry(tcpc_dev);
			return;
		case typec_try_src:
			typec_trywait_snk_entry(tcpc_dev);
			return;
		}
	}
#endif	/* CONFIG_TYPEC_CAP_TRY_SOURCE */

	typec_unattach_wait_pe_idle_entry(tcpc_dev);
	D("-\n");
}

static inline void typec_cc_snk_remove_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	if (tcpc_dev->typec_state == typec_try_snk) {
		typec_trywait_src_entry(tcpc_dev);
		return;
	}
#endif	/* CONFIG_TYPEC_CAP_TRY_SINK */

	typec_unattach_wait_pe_idle_entry(tcpc_dev);
	D("-\n");
}

static inline bool typec_debug_acc_attached_entry(struct tcpc_device *tcpc_dev)
{
	TYPEC_NEW_STATE(typec_debugaccessory);
	TYPEC_DBG("[Debug] CC1&2 Both Rd\n");
	tcpc_dev->typec_attach_new = TYPEC_ATTACHED_DEBUG;
	D("set typec_attach_new %d\n", tcpc_dev->typec_attach_new);
	return true;
}

static inline bool typec_audio_acc_attached_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");

	TYPEC_NEW_STATE(typec_audioaccessory);
	TYPEC_DBG("[Audio] CC1&2 Both Ra\n");
	tcpc_dev->typec_attach_new = TYPEC_ATTACHED_AUDIO;
	D("set typec_attach_new %d\n", tcpc_dev->typec_attach_new);

	D("-\n");
	return true;
}

static inline bool typec_cc_change_source_entry(struct tcpc_device *tcpc_dev)
{
	bool src_remove = false;
	D("+\n");

	switch (tcpc_dev->typec_state) {
	case typec_attached_src:
		if (typec_get_cc_res() != TYPEC_CC_VOLT_RD)
			src_remove = true;
		break;
	case typec_audioaccessory:
		if (!typec_check_cc_both(TYPEC_CC_VOLT_RA))
			src_remove = true;
		break;
	case typec_debugaccessory:
		if (!typec_check_cc_both(TYPEC_CC_VOLT_RD))
			src_remove = true;
		break;
	default:
		if (typec_check_cc_both(TYPEC_CC_VOLT_RD))
			typec_debug_acc_attached_entry(tcpc_dev);
		else if (typec_check_cc_both(TYPEC_CC_VOLT_RA))
			typec_audio_acc_attached_entry(tcpc_dev);
		else if (typec_check_cc_any(TYPEC_CC_VOLT_RD))
			typec_cc_src_detect_entry(tcpc_dev);
		else
			src_remove = true;
		break;
	}

	if (src_remove)
		typec_cc_src_remove_entry(tcpc_dev);

	D("-\n");
	return true;
}

static inline bool typec_attached_snk_cc_change(struct tcpc_device *tcpc_dev)
{
	uint8_t cc_res = typec_get_cc_res();

	D("+\n");
	if (cc_res != tcpc_dev->typec_remote_rp_level) {
		TYPEC_INFO("RpLvl Change\n");
		tcpc_dev->typec_remote_rp_level = cc_res;

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
		if (tcpc_dev->pd_port.pd_prev_connected)
			return true;
#endif

		tcpci_sink_vbus(tcpc_dev, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, -1);
	}

	D("-\n");
	return true;
}

static inline bool typec_cc_change_sink_entry(struct tcpc_device *tcpc_dev)
{
	bool snk_remove = false;

	D("+\n");
	D("typec_state %d(%s)\n", tcpc_dev->typec_state, typec_state_name[tcpc_dev->typec_state]);
	switch (tcpc_dev->typec_state) {
	case typec_attached_snk:
		if (typec_get_cc_res() == TYPEC_CC_VOLT_OPEN)
			snk_remove = true;
		else
			typec_attached_snk_cc_change(tcpc_dev);
		break;

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	case typec_attached_dbgacc_snk:
		if (typec_get_cc_res() == TYPEC_CC_VOLT_OPEN)
			snk_remove = true;
		break;
#endif	/* CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT */

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	case typec_attached_custom_src:
		if (typec_check_cc_any(TYPEC_CC_VOLT_OPEN))
			snk_remove = true;
		break;
#endif	/* CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT */

	default:
		if (!typec_is_cc_open())
			typec_cc_snk_detect_entry(tcpc_dev);
		else
			snk_remove = true;
	}

	if (snk_remove) {
		D("sink removed\n");
		typec_cc_snk_remove_entry(tcpc_dev);
	}

	D("-\n");
	return true;
}

static inline bool typec_is_act_as_sink_role(struct tcpc_device *tcpc_dev)
{
	bool as_sink = true;
	uint8_t cc_sum;

	switch (tcpc_dev->typec_local_cc & 0x07) {
	case TYPEC_CC_RP:
		as_sink = false;
		break;
	case TYPEC_CC_RD:
		as_sink = true;
		break;
	case TYPEC_CC_DRP:
		cc_sum = typec_get_cc1() + typec_get_cc2();
		as_sink = (cc_sum >= TYPEC_CC_VOLT_SNK_DFT);
		break;
	}

	D("as_sink %d\n", as_sink);

	return as_sink;
}

static inline bool typec_handle_cc_changed_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	TYPEC_INFO("[CC_Change] %d/%d\n", typec_get_cc1(), typec_get_cc2());

	tcpc_dev->typec_attach_new = tcpc_dev->typec_attach_old;
	D("set typec_attach_new %d\n", tcpc_dev->typec_attach_new);

	if (typec_is_act_as_sink_role(tcpc_dev))
		typec_cc_change_sink_entry(tcpc_dev);
	else
		typec_cc_change_source_entry(tcpc_dev);

	hisi_usb_typec_cc_status_change(typec_get_cc1(), typec_get_cc2());

	typec_alert_attach_state_change(tcpc_dev);
	D("-\n");
	return true;
}

static inline void typec_attach_wait_entry(struct tcpc_device *tcpc_dev)
{
	bool as_sink;

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	bool pd_en = tcpc_dev->pd_port.pd_prev_connected;
#else
	bool pd_en = false;
#endif	/* CONFIG_USB_POWER_DELIVERY_SUPPORT */

	D("+\n");
	if (tcpc_dev->typec_attach_old == TYPEC_ATTACHED_SNK && !pd_en) {
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
		D("RpLvl Alert\n");
		return;
	}

	if ((tcpc_dev->typec_attach_old != TYPEC_UNATTACHED)
			|| (tcpc_dev->typec_state == typec_attached_src)) {
		hisi_tcpc_reset_typec_debounce_timer(tcpc_dev);
		D("Attached, Ignore cc_attach\n");
		return;
	}

	D("typec_state %d(%s)\n", tcpc_dev->typec_state,
			typec_state_name[tcpc_dev->typec_state]);
	switch (tcpc_dev->typec_state) {
#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	case typec_try_src:
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
		return;

	case typec_trywait_snk:
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_CCDEBOUNCE);
		return;
#endif

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	case typec_try_snk:
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
		return;

	case typec_trywait_src:
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
		return;
#endif

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	case typec_unattachwait_pe:
		/* Connected during wait for PE Idle. Quick pull out an plug ?? */
		TYPEC_INFO("Force PE Idle\n");
		tcpc_dev->pd_wait_pe_idle = false;
		hisi_tcpc_disable_timer(tcpc_dev, TYPEC_RT_TIMER_PE_IDLE);
		typec_unattached_power_entry(tcpc_dev);
		break;
#endif
	default:
		break;
	}

	as_sink = typec_is_act_as_sink_role(tcpc_dev);
	if (as_sink)
		TYPEC_NEW_STATE(typec_attachwait_snk);
	else
		TYPEC_NEW_STATE(typec_attachwait_src);

	hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_CCDEBOUNCE);
	D("-\n");
}

#ifdef TYPEC_EXIT_ATTACHED_SNK_VIA_VBUS
static inline int typec_attached_snk_cc_detach(struct tcpc_device *tcpc_dev)
{
	int vbus_valid = tcpci_check_vbus_valid(tcpc_dev);
	bool detach_by_cc = false;

	D("+\n");
	if (tcpc_dev->typec_during_direct_charge) {
		D("direct charging, detach by cc\n");
		detach_by_cc = true;
	}
#ifdef CONFIG_HISI_TCPC_QUIRK_V100
	if (!tcpc_dev->vbus_detect) {
		D("vbus detect disabled, detach by cc\n");
		detach_by_cc = true;
		tcpci_set_vbus_detect(tcpc_dev, true);
	}
#endif

	D("vbus_valid %d\n", vbus_valid);
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	/* For Source detach during HardReset */
	if ((!vbus_valid) && tcpc_dev->pd_wait_hard_reset_complete) {
		detach_by_cc = true;
		D("Detach_CC (HardReset)\n");
	}
#endif

	if (detach_by_cc)
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);

	D("-\n");
	return 0;
}
#endif	/* TYPEC_EXIT_ATTACHED_SNK_VIA_VBUS */

static inline void typec_detach_wait_entry(struct tcpc_device *tcpc_dev)
{
	D("+\n");

	switch (tcpc_dev->typec_state) {
#ifdef TYPEC_EXIT_ATTACHED_SNK_VIA_VBUS
	case typec_attached_snk:
		typec_attached_snk_cc_detach(tcpc_dev);
		break;
#endif /* TYPEC_EXIT_ATTACHED_SNK_VIA_VBUS */

	case typec_audioaccessory:
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_CCDEBOUNCE);
		break;

#ifdef TYPEC_EXIT_ATTACHED_SRC_NO_DEBOUNCE
	case typec_attached_src:
		TYPEC_INFO("Exit Attached.SRC immediately\n");
		hisi_tcpc_reset_typec_debounce_timer(tcpc_dev);

		/* force to terminate TX */
		hisi_tcpc_tcpci_init(tcpc_dev, true);

		typec_cc_src_remove_entry(tcpc_dev);
		typec_alert_attach_state_change(tcpc_dev);
		break;
#endif /* TYPEC_EXIT_ATTACHED_SRC_NO_DEBOUNCE */

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	case typec_try_src:
		if (tcpc_dev->typec_drp_try_timeout)
			hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
		else {
			hisi_tcpc_reset_typec_debounce_timer(tcpc_dev);
			D("[Try] Igrone cc_detach\n");
		}
		break;
#endif	/* CONFIG_TYPEC_CAP_TRY_SOURCE */

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	case typec_trywait_src:
		if (tcpc_dev->typec_drp_try_timeout)
			hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
		else {
			hisi_tcpc_reset_typec_debounce_timer(tcpc_dev);
			D("[Try] Igrone cc_detach\n");
		}
		break;
#endif	/* CONFIG_TYPEC_CAP_TRY_SINK */
	default:
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
		break;
	}

	D("-\n");
}

static inline bool typec_is_cc_attach(struct tcpc_device *tcpc_dev)
{
	bool cc_attach = false;
	int cc1 = typec_get_cc1();
	int cc2 = typec_get_cc2();
	int cc_res = typec_get_cc_res();

	tcpc_dev->typec_cable_only = false;

	switch (tcpc_dev->typec_attach_old) {
	case TYPEC_ATTACHED_SNK:
	case TYPEC_ATTACHED_SRC:

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	case TYPEC_ATTACHED_CUSTOM_SRC:
#endif	/* CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT */

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	case TYPEC_ATTACHED_DBGACC_SNK:
#endif	/* CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT */
		if ((cc_res != TYPEC_CC_VOLT_OPEN) && (cc_res != TYPEC_CC_VOLT_RA))
			cc_attach = true;
		break;

	case TYPEC_ATTACHED_AUDIO:
		if (typec_check_cc_both(TYPEC_CC_VOLT_RA))
			cc_attach = true;
		break;

	case TYPEC_ATTACHED_DEBUG:
		if (typec_check_cc_both(TYPEC_CC_VOLT_RD))
			cc_attach = true;
		break;

	default:	/* TYPEC_UNATTACHED */
		if (cc1 != TYPEC_CC_VOLT_OPEN)
			cc_attach = true;

		if (cc2 != TYPEC_CC_VOLT_OPEN)
			cc_attach = true;

		/* Cable Only, no device */
		if ((cc1 + cc2) == TYPEC_CC_VOLT_RA) {
			cc_attach = false;
			tcpc_dev->typec_cable_only = true;
			D("[Cable] Ra Only\n");
		}
		break;
	}

	D("%d\n", cc_attach);
	return cc_attach;
}

int hisi_tcpc_typec_handle_cc_change(struct tcpc_device *tcpc_dev)
{
	int ret;

	D("+\n");
	ret = tcpci_get_cc(tcpc_dev);
	if (ret < 0)
		return ret;

	if (typec_is_drp_toggling()) {
		D("DRP Toggling\n");
		if (tcpc_dev->typec_lpm)
			typec_enter_low_power_mode(tcpc_dev);
		return 0;
	}

	D("[CC_Alert] cc1:%d, cc2:%d\n", typec_get_cc1(), typec_get_cc2());

	typec_disable_low_power_mode(tcpc_dev);

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (tcpc_dev->pd_wait_pr_swap_complete) {
		D("[PR.Swap] Ignore CC_Alert\n");
		return 0;
	}

	if (tcpc_dev->pd_wait_error_recovery) {
		D("[Error Recovery] Ignore CC_Alert\n");
		return 0;
	}
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	if ((tcpc_dev->typec_state == typec_try_snk)
			&& (!tcpc_dev->typec_drp_try_timeout)) {
		D("[Try.SNK] Ignore CC_Alert\n");
		return 0;
	}

	if (tcpc_dev->typec_state == typec_trywait_src_pe) {
		D("[Try.PE] Ignore CC_Alert\n");
		return 0;
	}
#endif	/* CONFIG_TYPEC_CAP_TRY_SINK */

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	if (tcpc_dev->typec_state == typec_trywait_snk_pe) {
		D("[Try.PE] Ignore CC_Alert\n");
		return 0;
	}
#endif	/* CONFIG_TYPEC_CAP_TRY_SOURCE */

	/* for cc alert monitoring */
	hisi_usb_typec_cc_alert(typec_get_cc1(), typec_get_cc2());

	if (tcpc_dev->typec_state == typec_attachwait_snk
			|| tcpc_dev->typec_state == typec_attachwait_src)
		typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

	if (typec_is_cc_attach(tcpc_dev)) {
		typec_attach_wait_entry(tcpc_dev);
	} else {
		typec_detach_wait_entry(tcpc_dev);
	}

	D("-\n");
	return 0;
}

#ifdef CONFIG_TYPEC_CAP_TRY_STATE
static inline int typec_handle_drp_try_timeout(struct tcpc_device *tcpc_dev)
{
	bool src_detect = false, en_timer;

	D("+\n");
	tcpc_dev->typec_drp_try_timeout = true;
	hisi_tcpc_disable_timer(tcpc_dev, TYPEC_TRY_TIMER_DRP_TRY);

	if (typec_is_drp_toggling()) {
		D("DRP Toggling, ignore debounce timeout.\n");
		return 0;
	}

	if (typec_check_cc1(TYPEC_CC_VOLT_RD) || typec_check_cc2(TYPEC_CC_VOLT_RD)) {
		src_detect = true;
	}

	switch (tcpc_dev->typec_state) {
#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	case typec_try_src:
		en_timer = !src_detect;
		break;
#endif /* CONFIG_TYPEC_CAP_TRY_SOURCE */

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	case typec_trywait_src:
		en_timer = !src_detect;
		break;

	case typec_try_snk:
		en_timer = true;
		break;
#endif /* CONFIG_TYPEC_CAP_TRY_SINK */

	default:
		en_timer = false;
		break;
	}

	if (en_timer)
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);

	D("-\n");
	return 0;
}
#endif	/* CONFIG_TYPEC_CAP_TRY_STATE */

static inline int typec_handle_debounce_timeout(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	if (typec_is_drp_toggling()) {
		D("DRP Toggling, ignore debounce timeout.\n");
		return 0;
	}

	typec_handle_cc_changed_entry(tcpc_dev);
	D("-\n");
	return 0;
}

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT

static inline int typec_handle_error_recovery_timeout(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	/* TODO: Check it later */
	tcpc_dev->typec_attach_new = TYPEC_UNATTACHED;
	D("set typec_attach_new %d\n", tcpc_dev->typec_attach_new);

	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_wait_error_recovery = false;
	mutex_unlock(&tcpc_dev->access_lock);

	typec_unattach_wait_pe_idle_entry(tcpc_dev);
	typec_alert_attach_state_change(tcpc_dev);

	D("-\n");
	return 0;
}

static inline int typec_handle_pe_idle(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	switch (tcpc_dev->typec_state) {

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	case typec_trywait_snk_pe:
		typec_trywait_snk_entry(tcpc_dev);
		break;
#endif

	case typec_unattachwait_pe:
		typec_unattached_entry(tcpc_dev);
		break;

	default:
		D("Dummy pe_idle\n");
		break;
	}

	D("-\n");
	return 0;
}
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

static inline int typec_handle_src_reach_vsafe0v(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	if (typec_is_drp_toggling()) {
		D("DRP Toggling, ignore vsafe0V\n");
		return 0;
	}

	typec_cc_src_detect_vsafe0v_entry(tcpc_dev);
	D("-\n");
	return 0;
}

static inline int typec_handle_src_toggle_timeout(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	if (tcpc_dev->typec_state == typec_unattached_src) {
		TYPEC_NEW_STATE(typec_unattached_snk);
		tcpci_set_cc(tcpc_dev, TYPEC_CC_DRP);
		typec_enable_low_power_mode(tcpc_dev, TYPEC_CC_DRP);
	}

	D("-\n");
	return 0;
}

static int hisi_tcpc_typec_handle_vsafe0v(struct tcpc_device *tcpc_dev)
{
	if (tcpc_dev->typec_wait_ps_change == TYPEC_WAIT_PS_SRC_VSAFE0V) {
#ifdef CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_DELAY
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_RT_TIMER_SAFE0V_DELAY);
#else
		typec_handle_src_reach_vsafe0v(tcpc_dev);
#endif
	}

	return 0;
}

static inline int typec_handle_safe0v_tout(struct tcpc_device *tcpc_dev)
{
        int ret = 0;
        uint16_t power_status = 0;

	D("+\n");
        if (!tcpci_check_vbus_valid(tcpc_dev))
                ret = hisi_tcpc_typec_handle_vsafe0v(tcpc_dev);
        else {
                TYPEC_INFO("VBUS still Valid!!\n");

                ret = tcpci_get_power_status(tcpc_dev, &power_status);
                if(ret) {
			TYPEC_INFO("TCPCI get power status: %d!!\n", ret);
		}
                hisi_tcpci_vbus_level_init(tcpc_dev, power_status);

                TCPC_INFO("Safe0V TOUT: ps=%d\n", tcpc_dev->vbus_level);

                if (!tcpci_check_vbus_valid(tcpc_dev))
                        ret = hisi_tcpc_typec_handle_vsafe0v(tcpc_dev);
		else {
			D("wait safe0v timeout\n");
		}
        }

	D("-\n");
        return ret;
}

/**
 * typec_handle_timeout() - called by hisi_tcpc_typec_handle_timeout.
 */
static int tcpc_typec_handle_timeout(struct tcpc_device *tcpc_dev, uint32_t timer_id)
{
	int ret = 0;

	switch (timer_id) {
	case TYPEC_TIMER_CCDEBOUNCE:
	case TYPEC_TIMER_PDDEBOUNCE:
		ret = typec_handle_debounce_timeout(tcpc_dev);
		break;

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	case TYPEC_TIMER_ERROR_RECOVERY:
		ret = typec_handle_error_recovery_timeout(tcpc_dev);
		break;

	case TYPEC_RT_TIMER_PE_IDLE:
		ret = typec_handle_pe_idle(tcpc_dev);
		break;
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

	case TYPEC_RT_TIMER_SAFE0V_DELAY:
		ret = typec_handle_src_reach_vsafe0v(tcpc_dev);
		break;

	case TYPEC_RT_TIMER_LOW_POWER_MODE:
		if (tcpc_dev->typec_lpm)
			typec_try_low_power_mode(tcpc_dev);
		break;

#ifdef CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT
	case TYPEC_RT_TIMER_SAFE0V_TOUT:
		ret = typec_handle_safe0v_tout(tcpc_dev);
                break;
#endif	/* CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT */

	case TYPEC_TIMER_DRP_SRC_TOGGLE:
		ret = typec_handle_src_toggle_timeout(tcpc_dev);
		break;

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	case TYPEC_RT_TIMER_ROLE_SWAP_START:
		if (tcpc_dev->typec_during_role_swap == TYPEC_ROLE_SWAP_TO_SNK) {
			D("Role Swap to Sink\n");
			tcpci_set_cc(tcpc_dev, TYPEC_CC_RD);
			hisi_tcpc_enable_timer(tcpc_dev, TYPEC_RT_TIMER_ROLE_SWAP_STOP);
		} else if (tcpc_dev->typec_during_role_swap == TYPEC_ROLE_SWAP_TO_SRC) {
			D("Role Swap to Source\n");
			tcpci_set_cc(tcpc_dev, TYPEC_CC_RP);
			hisi_tcpc_enable_timer(tcpc_dev, TYPEC_RT_TIMER_ROLE_SWAP_STOP);
		}
		break;

	case TYPEC_RT_TIMER_ROLE_SWAP_STOP:
		if (tcpc_dev->typec_during_role_swap) {
			D("TypeC Role Swap Failed\n");
			tcpc_dev->typec_during_role_swap = false;
			hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
		}
		break;
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

	default:
		break;

	}

	return ret;
}

int hisi_tcpc_typec_handle_timeout(struct tcpc_device *tcpc_dev, uint32_t timer_id)
{
	int ret = 0;

	D("+\n");
	D("timer_id %d\n", timer_id);

#ifdef CONFIG_TYPEC_CAP_TRY_STATE
	if (timer_id == TYPEC_TRY_TIMER_DRP_TRY)
		return typec_handle_drp_try_timeout(tcpc_dev);
#endif	/* CONFIG_TYPEC_CAP_TRY_STATE */

	if (timer_id >= TYPEC_TIMER_START_ID)
		hisi_tcpc_reset_typec_debounce_timer(tcpc_dev);
	else if (timer_id >= TYPEC_RT_TIMER_START_ID)
		hisi_tcpc_disable_timer(tcpc_dev, timer_id);

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (tcpc_dev->pd_wait_pr_swap_complete) {
		D("[PR.Swap] Igrone timer_evt\n");
		return 0;
	}

	if (tcpc_dev->pd_wait_error_recovery && (timer_id != TYPEC_TIMER_ERROR_RECOVERY)) {
		D("[Error Recovery] Igrone timer_evt\n");
		return 0;
	}
#endif

	ret = tcpc_typec_handle_timeout(tcpc_dev, timer_id);

	D("-\n");
	return ret;
}

static inline int typec_handle_vbus_present(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	switch (tcpc_dev->typec_wait_ps_change) {
	case TYPEC_WAIT_PS_SNK_VSAFE5V:
		typec_cc_snk_detect_vsafe5v_entry(tcpc_dev);
		typec_alert_attach_state_change(tcpc_dev);
		break;
	case TYPEC_WAIT_PS_SRC_VSAFE5V:
		typec_source_attached_with_vbus_entry(tcpc_dev);
		typec_alert_attach_state_change(tcpc_dev);
		break;
	}

	D("-\n");
	return 0;
}

static inline int typec_attached_snk_vbus_absent(struct tcpc_device *tcpc_dev)
{
	D("+\n");
#ifdef TYPEC_EXIT_ATTACHED_SNK_VIA_VBUS
	if (tcpc_dev->typec_during_direct_charge &&
			!tcpci_check_vsafe0v(tcpc_dev, true)) {
		TYPEC_DBG("Ignore vbus_absent(snk), DirectCharge\n");
		return 0;
	}
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (tcpc_dev->pd_wait_hard_reset_complete ||
			tcpc_dev->pd_hard_reset_event_pending) {
		if (typec_get_cc_res() != TYPEC_CC_VOLT_OPEN) {
			TYPEC_DBG("Ignore vbus_absent(snk), HReset & CC!=0\n");
			return 0;
		}
	}
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

	typec_unattach_wait_pe_idle_entry(tcpc_dev);
	typec_alert_attach_state_change(tcpc_dev);
#endif /* TYPEC_EXIT_ATTACHED_SNK_VIA_VBUS */
	D("-\n");
	return 0;
}


static inline int typec_handle_vbus_absent(struct tcpc_device *tcpc_dev)
{
	D("+\n");

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	if (tcpc_dev->pd_wait_pr_swap_complete) {
		D("[PR.Swap] Igrone vbus_absent\n");
		return 0;
	}

	if (tcpc_dev->pd_wait_error_recovery) {
		D("[Error Recovery] Igrone vbus_absent\n");
		return 0;
	}
#endif

	switch (tcpc_dev->typec_state) {
	case typec_attached_snk:
#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT
	case typec_attached_dbgacc_snk:
#endif     /* CONFIG_TYPEC_CAP_DBGACC_SNK_SUPPORT */

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT
	case typec_attached_custom_src:
#endif     /* CONFIG_TYPEC_CAP_CUSTOM_SRC_SUPPORT */
		typec_attached_snk_vbus_absent(tcpc_dev);
		break;
        }

#ifndef CONFIG_TCPC_VSAFE0V_DETECT_SUPPORT
	hisi_tcpc_typec_handle_vsafe0v(tcpc_dev);
#endif /* #ifdef CONFIG_TCPC_VSAFE0V_DETECT_SUPPORT */

	D("-\n");
	return 0;
}

/**
 * Power status change interrupt service route.
 */
int hisi_tcpc_typec_handle_ps_change(struct tcpc_device *tcpc_dev, int vbus_level)
{
	D("\n");
	if (typec_is_drp_toggling()) {
		D("DRP Toggling, ignore power status change.\n");
		return 0;
	}

	if (vbus_level >= TCPC_VBUS_VALID)
		return typec_handle_vbus_present(tcpc_dev);
	else
		return typec_handle_vbus_absent(tcpc_dev);
}

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
/**
 * According to PD Spec ?
 */
int hisi_tcpc_typec_advertise_explicit_contract(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	if (tcpc_dev->typec_local_rp_level == TYPEC_CC_RP_DFT)
		tcpci_set_cc(tcpc_dev, TYPEC_CC_RP_1_5);
	D("-\n");
	return 0;
}

int hisi_tcpc_typec_handle_pe_pr_swap(struct tcpc_device *tcpc_dev)
{
	int ret = 0;

	D("+\n");
	mutex_lock(&tcpc_dev->typec_lock);
	switch (tcpc_dev->typec_state) {
	case typec_attached_snk:
		TYPEC_NEW_STATE(typec_attached_src);
		tcpc_dev->typec_attach_old = TYPEC_ATTACHED_SRC;
		D("set typec_attach_old %d\n", tcpc_dev->typec_attach_old);
		tcpci_set_cc(tcpc_dev, tcpc_dev->typec_local_rp_level);
		break;
	case typec_attached_src:
		tcpci_tcpc_print_pd_fsm_state(tcpc_dev);
		TYPEC_NEW_STATE(typec_attached_snk);
		tcpc_dev->typec_attach_old = TYPEC_ATTACHED_SNK;
		D("set typec_attach_old %d\n", tcpc_dev->typec_attach_old);
		tcpci_set_cc(tcpc_dev, TYPEC_CC_RD);
		tcpci_tcpc_print_pd_fsm_state(tcpc_dev);
		break;
	default:
		break;
	}
	mutex_unlock(&tcpc_dev->typec_lock);
	D("-\n");
	return ret;
}
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
int hisi_tcpc_typec_swap_role(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	if (tcpc_dev->typec_role < TYPEC_ROLE_DRP)
		return -1;

	if (tcpc_dev->typec_during_role_swap)
		return -1;

	switch (tcpc_dev->typec_attach_old) {
	case TYPEC_ATTACHED_SNK:
		tcpc_dev->typec_during_role_swap = TYPEC_ROLE_SWAP_TO_SRC;
		break;
	case TYPEC_ATTACHED_SRC:
		tcpc_dev->typec_during_role_swap = TYPEC_ROLE_SWAP_TO_SNK;
		break;
	}

	if (tcpc_dev->typec_during_role_swap) {
		TYPEC_INFO("TypeC Role Swap Start\n");
		tcpci_set_cc(tcpc_dev, TYPEC_CC_OPEN);
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_RT_TIMER_ROLE_SWAP_START);
		return 0;
	}

	D("-\n");
	return -1;
}
#endif /* CONFIG_TYPEC_CAP_ROLE_SWAP */

int hisi_tcpc_typec_set_direct_charge(struct tcpc_device *tcpc_dev, bool direct_charge)
{
	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->typec_during_direct_charge = direct_charge;
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
	return 0;
}

/**
 * For tcpm.
 */
int hisi_tcpc_typec_set_rp_level(struct tcpc_device *tcpc_dev, uint8_t res)
{
	D("+\n");
	switch (res) {
	case TYPEC_CC_RP_DFT:
	case TYPEC_CC_RP_1_5:
	case TYPEC_CC_RP_3_0:
		TYPEC_INFO("TypeC-Rp: %d\n", res);
		tcpc_dev->typec_local_rp_level = res;
		break;
	default:
		TYPEC_INFO("TypeC-Unknown-Rp (%d)\n", res);
		return -1;
	}

	if ((tcpc_dev->typec_attach_old != TYPEC_UNATTACHED) &&
			(tcpc_dev->typec_attach_new != TYPEC_UNATTACHED)) {
		tcpci_set_cc(tcpc_dev, res);
	}
	D("-\n");
	return 0;
}

/**
 * tcpm call this function, for force role swap!!!
 */
int hisi_tcpc_typec_change_role(struct tcpc_device *tcpc_dev, uint8_t typec_role)
{
	bool force_unattach = false;

	D("+\n");
	if (typec_role == TYPEC_ROLE_UNKNOWN || typec_role >= TYPEC_ROLE_NR) {
		TYPEC_INFO("Wrong TypeC-Role: %d\n", typec_role);
		return -1;
	}

	mutex_lock(&tcpc_dev->access_lock);

	/* The tcpc_dev->typec_role come from desc! */
	tcpc_dev->typec_role = typec_role;
	TYPEC_INFO("typec_new_role: %s\n", typec_role_name[typec_role]);

	if (tcpc_dev->typec_attach_new != TYPEC_UNATTACHED) {
		force_unattach = true;
	} else {
		switch (tcpc_dev->typec_role) {
		case TYPEC_ROLE_SNK:
			tcpci_set_cc(tcpc_dev, TYPEC_CC_RD);
			break;
		case TYPEC_ROLE_SRC:
			tcpci_set_cc(tcpc_dev, TYPEC_CC_RP);
			break;
		default:
			tcpci_set_cc(tcpc_dev, TYPEC_CC_DRP);
			break;
		}
	}

	if (force_unattach) {
		TYPEC_DBG("force_unattach\n");
		tcpci_set_cc(tcpc_dev, TYPEC_CC_OPEN);
		mutex_unlock(&tcpc_dev->access_lock);
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
		return 0;
	}

	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
	return 0;
}

void hisi_tcpc_typec_force_unattach(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	tcpci_set_cc(tcpc_dev, TYPEC_CC_OPEN);
	mutex_unlock(&tcpc_dev->access_lock);
	hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
	D("-\n");
}

#ifdef CONFIG_TYPEC_CAP_POWER_OFF_CHARGE
static int typec_init_power_off_charge(struct tcpc_device *tcpc_dev)
{
	int ret;

	D("+\n");
	ret = tcpci_get_cc(tcpc_dev);
	if (ret < 0)
		return ret;

	if (tcpc_dev->typec_role == TYPEC_ROLE_SRC)
		return 0;

	if (typec_is_cc_open()) {
		D("cc both open\n");
		return 0;
	}

	if (!tcpci_check_vbus_valid(tcpc_dev)) {
		D("vbus not valid\n");
		return 0;
	}

	/*
	 * System startup with a charger connected ??
	 */
	TYPEC_NEW_STATE(typec_unattached_snk);
	typec_wait_ps_change(tcpc_dev, TYPEC_WAIT_PS_DISABLE);

	tcpci_set_cc(tcpc_dev, TYPEC_CC_OPEN);
	tcpci_set_cc(tcpc_dev, TYPEC_CC_RD);
	D("-\n");

	return 1;
}
#endif	/* CONFIG_TYPEC_CAP_POWER_OFF_CHARGE */

int hisi_tcpc_typec_init(struct tcpc_device *tcpc_dev, uint8_t typec_role)
{
	int ret = 0;

	D("+\n");
	if (typec_role >= TYPEC_ROLE_NR) {
		TYPEC_INFO("Wrong TypeC-Role: %d\n", typec_role);
		return -EINVAL;
	}

	D("typec_role: %s\n", typec_role_name[typec_role]);

	tcpc_dev->typec_role = typec_role;
	tcpc_dev->typec_attach_new = TYPEC_UNATTACHED;
	tcpc_dev->typec_attach_old = TYPEC_UNATTACHED;
	D("init typec_attach_new and typec_attach_old\n");

	tcpc_dev->typec_remote_cc[0] = TYPEC_CC_VOLT_OPEN;
	tcpc_dev->typec_remote_cc[1] = TYPEC_CC_VOLT_OPEN;

	tcpc_dev->wake_lock_pd = 0;
	tcpc_dev->wake_lock_user = true;
	tcpc_dev->typec_usb_sink_curr = CONFIG_TYPEC_SNK_CURR_DFT;

#ifdef CONFIG_TYPEC_CAP_POWER_OFF_CHARGE
	ret = typec_init_power_off_charge(tcpc_dev);
	if (ret != 0) {
		pr_err("typec_init_power_off_charge error ret %d", ret);
		return ret;
	}
#endif	/* CONFIG_TYPEC_CAP_POWER_OFF_CHARGE */

#ifdef CONFIG_TYPEC_POWER_CTRL_INIT
	tcpc_dev->typec_power_ctrl = true;
#endif	/* CONFIG_TYPEC_POWER_CTRL_INIT */

	typec_unattached_entry(tcpc_dev);

	D("-\n");
	return ret;
}

void  hisi_tcpc_typec_deinit(struct tcpc_device *tcpc_dev)
{
	D("\n");
}
