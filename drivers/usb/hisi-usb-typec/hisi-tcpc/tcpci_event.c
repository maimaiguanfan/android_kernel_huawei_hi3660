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

#include <linux/kthread.h>
#include <linux/atomic.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <uapi/linux/sched/types.h>
#endif

#define LOG_TAG "[PD][tcpci_event.c]"

#include "securec.h"

#include "include/tcpci_event.h"
#include "include/tcpci_typec.h"
#include "include/tcpci.h"
#include "include/pd_policy_engine.h"

static void print_received_msg(pd_msg_t *pd_msg)
{
#ifdef CONFIG_HISI_TCPC_DEBUG
	uint16_t header  = pd_msg->msg_hdr;

	char *frame_type_name[] = {
		"SOP",
		"SOP'",
		"SOP''",
		"SOP_DEBUG'",
		"SOP_DEBUG''",
		"Hard Reset",
		"Cable Reset",
		"BIST Mode 2",
	};

	static const char * const pd_ctrl_msg_name[] = {
		"ctrl0",
		"good_crc",
		"goto_min",
		"accept",
		"reject",
		"ping",
		"ps_rdy",
		"get_src_cap",
		"get_snk_cap",
		"dr_swap",
		"pr_swap",
		"vs_swap",
		"wait",
		"soft_reset",
		"ctrlE",
		"ctrlF",
	};

	static const char * const pd_data_msg_name[] = {
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

	static char *vdm_cmd_type_name[] = {
		"REQ", "ACK", "NAK", "BUSY"
	};

	char *vdm_cmd_name[32] = {
		"Reserved",
		"Discover Identity",
		"Discover SVIDs",
		"Discover Modes",
		"Enter Mode",
		"Exit Mode",
		"Attention",
	};


	D("---------------------------------\n");
	if (((header >> 12) & 0x7) == 0) {
		if ((header & 0x1f) <= 0xf)
			D("%s, Control Message: %s\n",
				frame_type_name[pd_msg->frame_type],
				pd_ctrl_msg_name[header & 0x1f]);
		else
			D("%s, Control Message: 0x%x\n",
				frame_type_name[pd_msg->frame_type],
				header & 0x1f);
	} else {
		D("%s, Data Message: %s, payload: %d\n",
			frame_type_name[pd_msg->frame_type],
			pd_data_msg_name[header & /*0x1f*/ 0xf], ((header >> 12) & 0x7));

		if ((header & 0x1f) == 15) {
			/* vdm */
			uint16_t hdr = pd_msg->msg_hdr;
			int vdo_count = (hdr >> 12) & 0x7;
			uint32_t vdm_hdr = pd_msg->payload[0];
			char vdm_cmd = vdm_hdr & 0x1f;

			D("VDO count %d\n", vdo_count);
			D("Msg Hdr: 0x%x, VDM Hdr: 0x%x\n", hdr, vdm_hdr);
			D("SVID 0x%x, vdm type %s\n", vdm_hdr >> 16,
				(vdm_hdr & (1 << 15)) ? "Structed" : "Unstructed");
			if (vdm_hdr & (1 << 15)) {
				/* structed */
				D("version %d, Object Position %d, Command Type %s, Command %d(%s)\n",
					(vdm_hdr >> 13) & 0x3,
					(vdm_hdr >> 8) & 0x7,
					vdm_cmd_type_name[(int)((vdm_hdr >> 6) & 0x3)],
					vdm_cmd, (vdm_cmd < 7) ? vdm_cmd_name[(int)vdm_cmd] : "reserved");
			}
		}
	}
	D("---------------------------------\n");
#endif
}

#ifdef CONFIG_USB_PD_POSTPONE_VDM
static void postpone_vdm_event(struct tcpc_device *tcpc_dev)
{
	/*
	 * Postpone VDM retry event due to the retry reason
	 * maybe interrupt by some PD event ....
	 */

	pd_event_t *vdm_event = &tcpc_dev->pd_vdm_event;
	D("+\n");

	if (tcpc_dev->pd_pending_vdm_event && vdm_event->pd_msg) {
		tcpc_dev->pd_postpone_vdm_timeout = false;
		hisi_tcpc_restart_timer(tcpc_dev, PD_PE_VDM_POSTPONE);
	}
	D("-\n");
}
#endif	/* CONFIG_USB_PD_POSTPONE_VDM */

static pd_msg_t *__hisi_pd_alloc_msg(struct tcpc_device *tcpc_dev)
{
	int i;
	uint8_t mask;

	for (i = 0, mask = 1; i < PD_MSG_BUF_SIZE; i++, mask <<= 1) {
		if ((mask & tcpc_dev->pd_msg_buffer_allocated) == 0) {
			tcpc_dev->pd_msg_buffer_allocated |= mask;
			return tcpc_dev->pd_msg_buffer + i;
		}
	}

	PD_ERR("hisi_pd_alloc_msg failed\n");
	return (pd_msg_t *)NULL;
}

pd_msg_t *hisi_pd_alloc_msg(struct tcpc_device *tcpc_dev)
{
	pd_msg_t *pd_msg = NULL;

	mutex_lock(&tcpc_dev->access_lock);
	pd_msg = __hisi_pd_alloc_msg(tcpc_dev);
	mutex_unlock(&tcpc_dev->access_lock);

	return pd_msg;
}

static void __hisi_pd_free_msg(struct tcpc_device *tcpc_dev, pd_msg_t *pd_msg)
{
	int index = pd_msg - tcpc_dev->pd_msg_buffer;
	uint8_t mask = 1 << index;

	D("+\n");
	if ((mask & tcpc_dev->pd_msg_buffer_allocated) == 0) {
		D("pd free msg failed\n");
		dump_stack();
		return;
	}
	tcpc_dev->pd_msg_buffer_allocated &= (~mask);
	D("-\n");
}

static void __hisi_pd_free_event(struct tcpc_device *tcpc_dev, pd_event_t *pd_event)
{
	D("+\n");
	if (pd_event->pd_msg) {
		__hisi_pd_free_msg(tcpc_dev, pd_event->pd_msg);
		pd_event->pd_msg = NULL;
	}
	D("-\n");
}

void hisi_pd_free_msg(struct tcpc_device *tcpc_dev, pd_msg_t *pd_msg)
{
	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	__hisi_pd_free_msg(tcpc_dev, pd_msg);
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

void hisi_pd_free_event(struct tcpc_device *tcpc_dev, pd_event_t *pd_event)
{
	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	__hisi_pd_free_event(tcpc_dev, pd_event);
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

/*----------------------------------------------------------------------------*/

/* Get a event form ring buffer. */
static bool __hisi_pd_get_event(struct tcpc_device *tcpc_dev, pd_event_t *pd_event)
{
	int index = 0;

	if (tcpc_dev->pd_event_count <= 0)
		return false;

	tcpc_dev->pd_event_count--;

	/* These events in a ring buffer, which implament by array and managed
	 * by head and count */
	*pd_event = tcpc_dev->pd_event_ring_buffer[tcpc_dev->pd_event_head_index];

	if (tcpc_dev->pd_event_count) {
		index = tcpc_dev->pd_event_head_index + 1;
		index %= PD_EVENT_BUF_SIZE;
	}
	tcpc_dev->pd_event_head_index = index;

	return true;
}

bool hisi_pd_get_event(struct tcpc_device *tcpc_dev, pd_event_t *pd_event)
{
	bool ret;

	D("\n");
	mutex_lock(&tcpc_dev->access_lock);
	ret = __hisi_pd_get_event(tcpc_dev, pd_event);
	mutex_unlock(&tcpc_dev->access_lock);

	return ret;
}

/* FIXME: call this function should in locked context ?? */
static bool __hisi_pd_put_event(struct tcpc_device *tcpc_dev,
			const pd_event_t *pd_event, bool from_port_partner)
{
	int index;
	char * pd_event_type_name[] = {
		"PD_EVT_PD_MSG = 0",
		"PD_EVT_CTRL_MSG",
		"PD_EVT_DATA_MSG",
		"PD_EVT_DPM_MSG",
		"PD_EVT_HW_MSG",
		"PD_EVT_PE_MSG",
		"PD_EVT_TIMER_MSG",
		NULL
	};

	D("+\n");
	D("event_type %s, from partner %d\n",
		pd_event_type_name[pd_event->event_type], from_port_partner);

#ifdef CONFIG_USB_PD_POSTPONE_OTHER_VDM
	if (from_port_partner)
		postpone_vdm_event(tcpc_dev);
#endif

	if (tcpc_dev->pd_event_count >= PD_EVENT_BUF_SIZE) {
		E("hisi_pd_put_event failed\n");
		return false;
	}

	/* head +1 when get event */
	/* count -1 when get event, +1 when put event */
	index = (tcpc_dev->pd_event_head_index + tcpc_dev->pd_event_count);
	index %= PD_EVENT_BUF_SIZE;

	tcpc_dev->pd_event_count++;
	tcpc_dev->pd_event_ring_buffer[index] = *pd_event; /* struct assign */

	atomic_inc(&tcpc_dev->pending_event);
	wake_up_interruptible(&tcpc_dev->event_loop_wait_que);
	D("-\n");
	return true;
}

bool hisi_pd_put_event(struct tcpc_device *tcpc_dev, const pd_event_t *pd_event,
				bool from_port_partner)
{
	bool ret;

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	ret = __hisi_pd_put_event(tcpc_dev, pd_event, from_port_partner);
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");

	return ret;
}

/*----------------------------------------------------------------------------*/

static inline void pd_get_attention_event(struct tcpc_device *tcpc_dev, pd_event_t *pd_event)
{
	pd_event_t attention_evt = {
		.event_type = PD_EVT_PD_MSG,
		.msg = PD_DATA_VENDOR_DEF,
		.pd_msg = NULL,
	};

	D("+\n");
	*pd_event = attention_evt;
	pd_event->pd_msg = __hisi_pd_alloc_msg(tcpc_dev);
	if (pd_event->pd_msg == NULL)
		return;
	tcpc_dev->pd_pending_vdm_attention = false;
	*pd_event->pd_msg = tcpc_dev->pd_attention_vdm_msg;
	D("-\n");
}
static inline bool pd_check_vdm_state_ready(pd_port_t *pd_port)
{
	switch (pd_port->pe_vdm_state) {
	case PE_SNK_READY:
	case PE_SRC_READY:
#ifdef CONFIG_USB_PD_CUSTOM_DBGACC_SUPPORT
	case PE_DBG_READY:
#endif	/* CONFIG_USB_PD_CUSTOM_DBGACC_SUPPORT */
		D("true\n");
		return true;
	default:
		D("false\n");
		return false;
	}
}

/* called by hisi_pd_policy_engine_run.
 * vdm_event is not come from ring buffer, but create directly. */
bool hisi_pd_get_vdm_event(struct tcpc_device *tcpc_dev, pd_event_t *pd_event)
{
	pd_event_t delay_evt = {
		.event_type = PD_EVT_CTRL_MSG,
		.msg = PD_CTRL_GOOD_CRC,
		.pd_msg = NULL,
	};

	pd_event_t reset_evt = {
		.event_type = PD_EVT_PE_MSG,
		.msg = PD_PE_VDM_RESET,
		.pd_msg = NULL,
	};
	pd_event_t discard_evt = {
		.event_type = PD_EVT_HW_MSG,
		.msg = PD_HW_TX_DISCARD,
		.pd_msg = NULL,
	};

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);

	if (tcpc_dev->pd_pending_vdm_discard) {
		*pd_event = discard_evt;
		tcpc_dev->pd_pending_vdm_discard = false;
		mutex_unlock(&tcpc_dev->access_lock);
		D("discard_evt\n");
		D("-\n");
		return true;
	}

	if (tcpc_dev->pd_pending_vdm_event) {
		pd_event_t *vdm_event = &tcpc_dev->pd_vdm_event;

		if (vdm_event->pd_msg && !tcpc_dev->pd_postpone_vdm_timeout) {
			mutex_unlock(&tcpc_dev->access_lock);
			D("postpone vdm\n");
			D("-\n");
			return false;
		}

		if (tcpc_dev->pd_pending_vdm_good_crc) {
			*pd_event = delay_evt;
			tcpc_dev->pd_pending_vdm_good_crc = false;
		} else if (tcpc_dev->pd_pending_vdm_reset) {
			*pd_event = reset_evt;
			tcpc_dev->pd_pending_vdm_reset = false;
		} else {
			*pd_event = *vdm_event;
			tcpc_dev->pd_pending_vdm_event = false;
		}
		mutex_unlock(&tcpc_dev->access_lock);
		D("normal vdm\n");
		D("-\n");
		return true;
	}

	if (tcpc_dev->pd_pending_vdm_attention && pd_check_vdm_state_ready(&tcpc_dev->pd_port)) {
		pd_get_attention_event(tcpc_dev, pd_event);
		mutex_unlock(&tcpc_dev->access_lock);
		D("attention vdm\n");
		D("-\n");
		return true;
	}

	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
	return false;
}

static inline bool reset_pe_vdm_state(struct tcpc_device *tcpc_dev, uint32_t vdm_hdr)
{
	bool vdm_reset = false;
	pd_port_t *pd_port = &tcpc_dev->pd_port;

	if (PD_VDO_SVDM(vdm_hdr)) {
		if (PD_VDO_CMDT(vdm_hdr) == CMDT_INIT)
			vdm_reset = true;
	} else {
		if (pd_port->data_role == PD_ROLE_UFP)
			vdm_reset = true;
	}
	if (vdm_reset)
		tcpc_dev->pd_pending_vdm_reset = true;
	D("%s\n", vdm_reset ? "true" : "false");
	return vdm_reset;
}

static inline bool pd_is_init_attention_event(struct tcpc_device *tcpc_dev, pd_event_t *pd_event)
{
	uint32_t vdm_hdr = pd_event->pd_msg->payload[0];
	if ((PD_VDO_CMDT(vdm_hdr) == CMDT_INIT) && PD_VDO_CMD(vdm_hdr) == CMD_ATTENTION) {
		D("true\n");
		return true;
	}
	return false;
}

/**
 * Return true means successed.
 */
bool hisi_pd_put_vdm_event(struct tcpc_device *tcpc_dev,
			pd_event_t *pd_event, bool from_port_partner)
{
	pd_msg_t *pd_msg = pd_event->pd_msg;

	D("+\n");
	D("from_port_partner %d\n", from_port_partner);
	if (from_port_partner && (NULL == pd_msg )) {
		E("pd_msg NULL!\n");
		return false;
	}
	mutex_lock(&tcpc_dev->access_lock);

	if (from_port_partner && pd_is_init_attention_event(tcpc_dev, pd_event)) {
		D("AttentionEVT\n");
		tcpc_dev->pd_pending_vdm_attention = true;
		tcpc_dev->pd_attention_vdm_msg = *pd_msg; /* save the Attention Message */
		atomic_inc(&tcpc_dev->pending_event); /* do not really wake up process*/
		wake_up_interruptible(&tcpc_dev->event_loop_wait_que);

		__hisi_pd_free_event(tcpc_dev, pd_event);
		mutex_unlock(&tcpc_dev->access_lock);
		return false;
	}

	if (tcpc_dev->pd_pending_vdm_event) {
		D("pd_pending_vdm_event true\n");
		/* If message from port partner, we have to overwrite it */
		if (from_port_partner) {
			if (pd_event_msg_match(&tcpc_dev->pd_vdm_event,
					PD_EVT_CTRL_MSG, PD_CTRL_GOOD_CRC)) {
				D("PostponeVDM GoodCRC\n");
				tcpc_dev->pd_pending_vdm_good_crc = true;
			}

			__hisi_pd_free_event(tcpc_dev, &tcpc_dev->pd_vdm_event); /* free last vdm */
		} else {
			__hisi_pd_free_event(tcpc_dev, pd_event); /* free current vdm */
			mutex_unlock(&tcpc_dev->access_lock);
			return false;
		}
	}

	tcpc_dev->pd_vdm_event = *pd_event; /* save current vdm */
	tcpc_dev->pd_pending_vdm_event = true;
	tcpc_dev->pd_postpone_vdm_timeout = true;

	if (from_port_partner) {
		/* pd_msg->time_stamp = 0; */
		tcpc_dev->pd_last_vdm_msg = *pd_msg;
		reset_pe_vdm_state(tcpc_dev, pd_msg->payload[0]);
#ifdef CONFIG_USB_PD_POSTPONE_FIRST_VDM
		D("from_port_partner and postpone first VDM\n");
		postpone_vdm_event(tcpc_dev);
		mutex_unlock(&tcpc_dev->access_lock);
		D("-\n");
		return true;
#endif	/* CONFIG_USB_PD_POSTPONE_FIRST_VDM */
	}

	atomic_inc(&tcpc_dev->pending_event); /* do not really wake up process*/
	wake_up_interruptible(&tcpc_dev->event_loop_wait_que);
	mutex_unlock(&tcpc_dev->access_lock);

	D("-\n");
	return true;
}

/* This function call when send failed. Simulate last VDM, to trigger send again. */
bool hisi_pd_put_last_vdm_event(struct tcpc_device *tcpc_dev)
{
	pd_msg_t *pd_msg = &tcpc_dev->pd_last_vdm_msg;
	pd_event_t *vdm_event = &tcpc_dev->pd_vdm_event;

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);

	tcpc_dev->pd_pending_vdm_discard = true;
	atomic_inc(&tcpc_dev->pending_event); /* do not really wake up process*/
	wake_up_interruptible(&tcpc_dev->event_loop_wait_que);
	if (!reset_pe_vdm_state(tcpc_dev, pd_msg->payload[0])) {
		mutex_unlock(&tcpc_dev->access_lock);
		return true;
	}

	vdm_event->event_type = PD_EVT_HW_MSG;
	vdm_event->msg = PD_HW_RETRY_VDM;

	if (tcpc_dev->pd_pending_vdm_event)
		__hisi_pd_free_event(tcpc_dev, &tcpc_dev->pd_vdm_event);

	vdm_event->pd_msg = __hisi_pd_alloc_msg(tcpc_dev);

	if (vdm_event->pd_msg == NULL) {
		mutex_unlock(&tcpc_dev->access_lock);
		return false;
	}

	*vdm_event->pd_msg = *pd_msg;
	tcpc_dev->pd_pending_vdm_event = true;
	tcpc_dev->pd_postpone_vdm_timeout = true;

#ifdef CONFIG_USB_PD_POSTPONE_RETRY_VDM
	postpone_vdm_event(tcpc_dev);
#endif	/* CONFIG_USB_PD_POSTPONE_RETRY_VDM */

	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
	return true;
}

/*----------------------------------------------------------------------------*/

static void __hisi_pd_event_buf_reset(struct tcpc_device *tcpc_dev)
{
	pd_event_t pd_event;

	D("+\n");
	tcpc_dev->pd_hard_reset_event_pending = false;
	while (__hisi_pd_get_event(tcpc_dev, &pd_event))
		__hisi_pd_free_event(tcpc_dev, &pd_event);

	if (tcpc_dev->pd_pending_vdm_event) {
		__hisi_pd_free_event(tcpc_dev, &tcpc_dev->pd_vdm_event);
		tcpc_dev->pd_pending_vdm_event = false;
	}

	tcpc_dev->pd_pending_vdm_reset = false;
	tcpc_dev->pd_pending_vdm_good_crc = false;
	tcpc_dev->pd_pending_vdm_attention = false;
	tcpc_dev->pd_pending_vdm_discard = false;
	D("-\n");
}

void hisi_pd_event_buf_reset(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	__hisi_pd_event_buf_reset(tcpc_dev);
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

static inline bool __pd_put_hw_event(struct tcpc_device *tcpc_dev, uint8_t hw_event)
{
	pd_event_t evt = {
		.event_type = PD_EVT_HW_MSG,
		.msg = hw_event,
		.pd_msg = NULL,
	};
	D("hw_event %d\n", hw_event);

	return __hisi_pd_put_event(tcpc_dev, &evt, false);
}

static inline bool __pd_put_pe_event(struct tcpc_device *tcpc_dev, uint8_t pe_event)
{
	pd_event_t evt = {
		.event_type = PD_EVT_PE_MSG,
		.msg = pe_event,
		.pd_msg = NULL,
	};
	D("pe_event %d\n", pe_event);

	return __hisi_pd_put_event(tcpc_dev, &evt, false);
}

void hisi_pd_put_cc_detached_event(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);

	__hisi_pd_event_buf_reset(tcpc_dev);
	__pd_put_hw_event(tcpc_dev, PD_HW_CC_DETACHED);

	tcpc_dev->pd_wait_pe_idle = true;
	tcpc_dev->pd_wait_pr_swap_complete = false;
	tcpc_dev->pd_wait_hard_reset_complete = false;
	tcpc_dev->pd_hard_reset_event_pending = false;
	tcpc_dev->pd_wait_vbus_once = PD_WAIT_VBUS_DISABLE;
	tcpc_dev->pd_bist_mode = PD_BIST_MODE_DISABLE;
	tcpc_dev->pd_ping_event_pending = false;

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	tcpc_dev->pd_discard_pending = false;
#endif

	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

void hisi_pd_put_recv_hard_reset_event(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);

	tcpc_dev->pd_transmit_state = PD_TX_STATE_HARD_RESET;

	if ((!tcpc_dev->pd_hard_reset_event_pending) &&
			(!tcpc_dev->pd_wait_pe_idle) && tcpc_dev->pd_pe_running) {
		__hisi_pd_event_buf_reset(tcpc_dev);
		__pd_put_hw_event(tcpc_dev, PD_HW_RECV_HARD_RESET);
		tcpc_dev->pd_bist_mode = PD_BIST_MODE_DISABLE;
		tcpc_dev->pd_hard_reset_event_pending = true;
		tcpc_dev->pd_ping_event_pending = false;
	}

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	tcpc_dev->pd_discard_pending = false;
#endif

	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

/**
 * Send hard reset sucessed.
 */
void hisi_pd_put_sent_hard_reset_event(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	if (tcpc_dev->pd_wait_hard_reset_complete) {
		tcpc_dev->pd_transmit_state = PD_TX_STATE_GOOD_CRC;
		__hisi_pd_event_buf_reset(tcpc_dev);
		__pd_put_pe_event(tcpc_dev, PD_PE_HARD_RESET_COMPLETED);
	} else {
		D("[HardReset] Unattached\n");
	}
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

static inline bool ignore_pe_msg_event(struct tcpc_device *tcpc_dev, pd_msg_t *pd_msg)
{
	uint32_t cnt, cmd;

	cnt = PD_HEADER_CNT(pd_msg->msg_hdr); /* Number of Data Objects, reference 6.2.1.1 */
	cmd = PD_HEADER_TYPE(pd_msg->msg_hdr); /* Message Type */

#ifdef CONFIG_HISI_TCPC_QUIRK_V100
	if ((cnt == 0) && (cmd == 0)) {
		D("Unknown ctrl Message!\n");
		return true;
	}
#endif

	/* This flag was assigned when enter bist mode. */
	if (tcpc_dev->pd_bist_mode != PD_BIST_MODE_DISABLE) {
		return true;
	}

	return false;
}

/**
 * alert --> pd
 * Called by tcpci_alert_recv_msg when receive a Message.
 */
bool hisi_pd_put_pd_msg_event(struct tcpc_device *tcpc_dev, pd_msg_t *pd_msg)
{
	uint32_t cnt, cmd;
	bool ret;
#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	bool discard_pending = false;
#endif
	pd_event_t evt = {
		.event_type = PD_EVT_PD_MSG,
		.pd_msg = pd_msg,
	};

	D("+\n");
	print_received_msg(pd_msg);

	cnt = PD_HEADER_CNT(pd_msg->msg_hdr); /* Number of Data Objects, reference 6.2.1.1 */
	cmd = PD_HEADER_TYPE(pd_msg->msg_hdr); /* Message Type */

	mutex_lock(&tcpc_dev->access_lock);

	if (ignore_pe_msg_event(tcpc_dev, pd_msg)) {
		__hisi_pd_free_event(tcpc_dev, &evt);
		mutex_unlock(&tcpc_dev->access_lock);
		return 0;
	}

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	if (tcpc_dev->pd_discard_pending
			&& (pd_msg->frame_type == TCPC_TX_SOP)
			&& (tcpc_dev->tcpc_flags & TCPC_FLAGS_RETRY_CRC_DISCARD)) {

		discard_pending = true;
		tcpc_dev->pd_discard_pending = false;

		if ((cmd == PD_CTRL_GOOD_CRC) && (cnt == 0)) {
			__hisi_pd_free_event(tcpc_dev, &evt);
			mutex_unlock(&tcpc_dev->access_lock);

			/* TODO: check it later */
			hisi_tcpc_disable_timer(tcpc_dev, PD_TIMER_DISCARD);
			tcpci_retransmit(tcpc_dev);
			return 0;
		}
	}
#endif

#ifdef CONFIG_USB_PD_DROP_REPEAT_PING
	if (cnt == 0 && cmd == PD_CTRL_PING) {
		/* Ignore new ping, before the last ping finished. */
		if (tcpc_dev->pd_ping_event_pending) {
			D("Ignore PING\n");
			__hisi_pd_free_event(tcpc_dev, &evt);
			mutex_unlock(&tcpc_dev->access_lock);
			return 0;
		}

		tcpc_dev->pd_ping_event_pending = true;
	}
#endif

	if (cnt != 0 && cmd == PD_DATA_BIST) {
		D("set pd_bist_mode\n");
		tcpc_dev->pd_bist_mode = PD_BIST_MODE_EVENT_PENDING;
	}

	mutex_unlock(&tcpc_dev->access_lock);

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	if (discard_pending) {
		hisi_tcpc_disable_timer(tcpc_dev, PD_TIMER_DISCARD);
		pd_put_hw_event(tcpc_dev, PD_HW_TX_FAILED);
	}
#endif

	if (cnt != 0 && cmd == PD_DATA_VENDOR_DEF) {
		return hisi_pd_put_vdm_event(tcpc_dev, &evt, true);
	}

	ret = hisi_pd_put_event(tcpc_dev, &evt, true);

	D("-\n");
	return ret;
}

static void pd_report_vbus_present(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	tcpc_dev->pd_wait_vbus_once = PD_WAIT_VBUS_DISABLE;
	__pd_put_hw_event(tcpc_dev, PD_HW_VBUS_PRESENT);
	D("-\n");
}

/* typec --> pd */
void hisi_pd_put_vbus_changed_event(struct tcpc_device *tcpc_dev, bool from_ic)
{
	int vbus_valid;
	bool postpone_vbus_present = false;

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	vbus_valid = tcpci_check_vbus_valid(tcpc_dev);

	switch (tcpc_dev->pd_wait_vbus_once) {
	case PD_WAIT_VBUS_VALID_ONCE:
		if (vbus_valid) {
#if CONFIG_USB_PD_VBUS_PRESENT_TOUT
			/* In vbus intr route, from_ic is true, In PD Engine
			 * route, from_ic is false. */
			postpone_vbus_present = from_ic;
#endif	/* CONFIG_USB_PD_VBUS_PRESENT_TOUT */
			if (!postpone_vbus_present)
				pd_report_vbus_present(tcpc_dev);
		}
		break;

	case PD_WAIT_VBUS_INVALID_ONCE:
		if (!vbus_valid) {
			tcpc_dev->pd_wait_vbus_once = PD_WAIT_VBUS_DISABLE;
			__pd_put_hw_event(tcpc_dev, PD_HW_VBUS_ABSENT);
		}
		break;
#ifdef CONFIG_HISI_TCPC_QUIRK_V100
	/* Sometimes need wait vsafe0V, for example hard-reset, PRS .
	 * If vbus invalid, put a vsafe0V event. */
	case PD_WAIT_VBUS_SAFE0V_ONCE:
		if (!vbus_valid) {
			tcpc_dev->pd_wait_vbus_once = PD_WAIT_VBUS_DISABLE;
			__pd_put_hw_event(tcpc_dev, PD_HW_VBUS_SAFE0V);
		}
		break;
#endif
	default:
		break;
	}
	mutex_unlock(&tcpc_dev->access_lock);

#if CONFIG_USB_PD_VBUS_PRESENT_TOUT
	if (postpone_vbus_present)
		hisi_tcpc_enable_timer(tcpc_dev, PD_TIMER_VBUS_PRESENT);
#endif	/* CONFIG_USB_PD_VBUS_PRESENT_TOUT */

	D("-\n");
}

/* timer --> pd */
void hisi_pd_put_vbus_safe0v_event(struct tcpc_device *tcpc_dev)
{
	D("+\n");
#ifdef CONFIG_USB_PD_SAFE0V_TIMEOUT
	hisi_tcpc_disable_timer(tcpc_dev, PD_TIMER_VSAFE0V_TOUT);
#endif	/* CONFIG_USB_PD_SAFE0V_TIMEOUT */

	mutex_lock(&tcpc_dev->access_lock);
	if (tcpc_dev->pd_wait_vbus_once == PD_WAIT_VBUS_SAFE0V_ONCE) {
		tcpc_dev->pd_wait_vbus_once = PD_WAIT_VBUS_DISABLE;
		__pd_put_hw_event(tcpc_dev, PD_HW_VBUS_SAFE0V);
	}
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

/* typec --> pd */
void hisi_pd_put_vbus_stable_event(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	if (tcpc_dev->pd_wait_vbus_once == PD_WAIT_VBUS_STABLE_ONCE) {
		tcpc_dev->pd_wait_vbus_once = PD_WAIT_VBUS_DISABLE;
		__pd_put_hw_event(tcpc_dev, PD_HW_VBUS_STABLE);
	}
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

/* timer --> pd */
void hisi_pd_put_vbus_present_event(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	pd_report_vbus_present(tcpc_dev);
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

/*
 * The follow functions used for PD Notify TCPC
 */

void hisi_pd_try_put_pe_idle_event(pd_port_t *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	if (tcpc_dev->pd_transmit_state <= PD_TX_STATE_WAIT)
		__pd_put_pe_event(tcpc_dev, PD_PE_IDLE);
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

void hisi_pd_notify_pe_running(pd_port_t *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_pe_running = true;
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

void hisi_pd_notify_pe_idle(pd_port_t *pd_port)
{
	bool notify_pe_idle = false;
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	if (tcpc_dev->pd_wait_pe_idle) {
		notify_pe_idle = true;
		tcpc_dev->pd_pe_running = false;
		tcpc_dev->pd_wait_pe_idle = false;
	}

	tcpc_dev->pd_wait_error_recovery = false;
	mutex_unlock(&tcpc_dev->access_lock);

	/* Update PD state. */
	hisi_pd_update_connect_state(pd_port, HISI_PD_CONNECT_NONE);

	if (notify_pe_idle)
		hisi_tcpc_enable_timer(tcpc_dev, TYPEC_RT_TIMER_PE_IDLE);
	D("-\n");
}

void hisi_pd_notify_pe_wait_vbus_once(pd_port_t *pd_port, int wait_evt)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_wait_vbus_once = wait_evt;
	mutex_unlock(&tcpc_dev->access_lock);

	switch (wait_evt) {
	case PD_WAIT_VBUS_VALID_ONCE:
	case PD_WAIT_VBUS_INVALID_ONCE:
		hisi_pd_put_vbus_changed_event(tcpc_dev, false);
		break;

	case PD_WAIT_VBUS_SAFE0V_ONCE:
#ifdef CONFIG_TCPC_VSAFE0V_DETECT_SUPPORT
		if (tcpci_check_vsafe0v(tcpc_dev, true)) {
			hisi_pd_put_vbus_safe0v_event(tcpc_dev);
			break;
		}
#else
		pd_enable_timer(pd_port, PD_TIMER_VSAFE0V_DELAY);
#endif	/* CONFIG_TCPC_VSAFE0V_DETECT_SUPPORT */

#ifdef CONFIG_USB_PD_SAFE0V_TIMEOUT
		pd_enable_timer(pd_port, PD_TIMER_VSAFE0V_TOUT);
#endif	/* CONFIG_USB_PD_SAFE0V_TIMEOUT */
		break;
	}
	D("-\n");
}

void hisi_pd_notify_pe_error_recovery(pd_port_t *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_wait_hard_reset_complete = false;
	tcpc_dev->pd_wait_pr_swap_complete = false;
	tcpc_dev->pd_wait_error_recovery = true;
	mutex_unlock(&tcpc_dev->access_lock);

	tcpci_set_cc(pd_port->tcpc_dev, TYPEC_CC_OPEN);
	hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_ERROR_RECOVERY);
	D("-\n");
}

void hisi_pd_notify_pe_transit_to_default(pd_port_t *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_hard_reset_event_pending = false;
	tcpc_dev->pd_wait_hard_reset_complete = true;
	tcpc_dev->pd_wait_pr_swap_complete = false;
	tcpc_dev->pd_bist_mode = PD_BIST_MODE_DISABLE;
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

void hisi_pd_notify_pe_hard_reset_completed(pd_port_t *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	if (!tcpc_dev->pd_wait_hard_reset_complete)
		return;

	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_wait_hard_reset_complete = false;
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

void hisi_pd_notify_pe_send_hard_reset(pd_port_t *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_transmit_state = PD_TX_STATE_WAIT_HARD_RESET;
	tcpc_dev->pd_wait_hard_reset_complete = true;
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

void hisi_pd_notify_pe_execute_pr_swap(pd_port_t *pd_port, bool start_swap)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	pd_port->during_swap = start_swap;
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_wait_pr_swap_complete = true;
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

void hisi_pd_notify_pe_cancel_pr_swap(pd_port_t *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	if (!tcpc_dev->pd_wait_pr_swap_complete)
		return;

	pd_port->during_swap = false;
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_wait_pr_swap_complete = false;
	mutex_unlock(&tcpc_dev->access_lock);

	hisi_tcpc_enable_timer(tcpc_dev, TYPEC_TIMER_PDDEBOUNCE);
	D("-\n");
}

void hisi_pd_notify_pe_reset_protocol(pd_port_t *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_wait_pr_swap_complete = false;
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

void hisi_pd_noitfy_pe_bist_mode(pd_port_t *pd_port, uint8_t mode)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_bist_mode = mode;
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

void hisi_pd_notify_pe_recv_ping_event(pd_port_t *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_ping_event_pending = false;
	mutex_unlock(&tcpc_dev->access_lock);
	D("-\n");
}

void hisi_pd_notify_pe_transmit_msg(pd_port_t *pd_port, uint8_t type)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	mutex_lock(&tcpc_dev->access_lock);
	tcpc_dev->pd_transmit_state = type;
	mutex_unlock(&tcpc_dev->access_lock);
	D("+\n");
}

void hisi_pd_notify_pe_pr_changed(pd_port_t *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	/* Check mutex later, actually,
	   typec layer will igrone all cc-change during PR-SWAP */

	/* mutex_lock(&tcpc_dev->access_lock); */
	hisi_tcpc_typec_handle_pe_pr_swap(tcpc_dev);
	/* mutex_unlock(&tcpc_dev->access_lock); */
	D("-\n");
}

void hisi_pd_notify_pe_src_explicit_contract(pd_port_t *pd_port)
{
	struct tcpc_device *tcpc_dev = pd_port->tcpc_dev;

	D("+\n");
	if (pd_port->explicit_contract)
		return;

	/*mutex_lock(&tcpc_dev->access_lock); */
	hisi_tcpc_typec_advertise_explicit_contract(tcpc_dev);
	/*mutex_unlock(&tcpc_dev->access_lock); */
	D("-\n");
}

static int tcpc_event_thread(void *param)
{
	struct tcpc_device *tcpc_dev = param;
	struct sched_param sch_param = {.sched_priority = MAX_RT_PRIO - 2};

	D("+\n");

	sched_setscheduler(current, SCHED_FIFO, &sch_param);

	while (true) {
		wait_event_interruptible(tcpc_dev->event_loop_wait_que,
				atomic_read(&tcpc_dev->pending_event) ||
					tcpc_dev->event_loop_thead_stop);

		if (kthread_should_stop() || tcpc_dev->event_loop_thead_stop)
			break;

		do {
			atomic_dec_if_positive(&tcpc_dev->pending_event);
		} while (hisi_pd_policy_engine_run(tcpc_dev));
	}

	D("-\n");

	return 0;
}

int hisi_tcpci_event_init(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	tcpc_dev->event_task = kthread_create(tcpc_event_thread, tcpc_dev,
			"hisi_tcpc_event_%s.%p", dev_name(&tcpc_dev->dev), tcpc_dev);
	tcpc_dev->event_loop_thead_stop = false;

	init_waitqueue_head(&tcpc_dev->event_loop_wait_que);
	atomic_set(&tcpc_dev->pending_event, 0);
	wake_up_process(tcpc_dev->event_task);

	D("-\n");
	return 0;
}

int hisi_tcpci_event_deinit(struct tcpc_device *tcpc_dev)
{
	D("+\n");
	tcpc_dev->event_loop_thead_stop = true;
	wake_up_interruptible(&tcpc_dev->event_loop_wait_que);
	kthread_stop(tcpc_dev->event_task);
	D("-\n");
	return 0;
}
