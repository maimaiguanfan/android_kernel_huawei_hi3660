/*
 * Copyright (C) 2018 Hisilicon Technology Corp.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __LINUX_TCPCI_CORE_H_
#define __LINUX_TCPCI_CORE_H_

#include <linux/device.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/notifier.h>
#include <linux/semaphore.h>

#include <linux/hisi/usb/hisi_tcpm.h>
#include "include/tcpci_timer.h"
#include "include/tcpci_config.h"

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
#include "include/pd_core.h"
#endif

struct tcpc_device;

struct tcpc_desc {
	uint8_t role_def;
	uint8_t rp_lvl;
	char *name;
};

/* TCPC Power Register Define */
#define TCPC_REG_POWER_STATUS_DBG_ACC_CONNECTED	(1<<7)
#define TCPC_REG_POWER_STATUS_TCPC_INITIAL	(1<<6)
#define TCPC_REG_POWER_STATUS_SRC_HV		(1<<5)
#define TCPC_REG_POWER_STATUS_SRC_VBUS		(1<<4)
#define TCPC_REG_POWER_STATUS_VBUS_PRES_DET	(1<<3)
#define TCPC_REG_POWER_STATUS_VBUS_PRES		(1<<2)
#define TCPC_REG_POWER_STATUS_VCONN_PRES	(1<<1)
#define TCPC_REG_POWER_STATUS_SINK_VBUS		(1<<0)

#define TCPC_REG_ALERT_VBUS_DISCNCT (1<<11)
#define TCPC_REG_ALERT_RX_BUF_OVF   (1<<10)
#define TCPC_REG_ALERT_FAULT        (1<<9)
#define TCPC_REG_ALERT_V_ALARM_LO   (1<<8)
#define TCPC_REG_ALERT_V_ALARM_HI   (1<<7)
#define TCPC_REG_ALERT_TX_SUCCESS   (1<<6)
#define TCPC_REG_ALERT_TX_DISCARDED (1<<5)
#define TCPC_REG_ALERT_TX_FAILED    (1<<4)
#define TCPC_REG_ALERT_RX_HARD_RST  (1<<3)
#define TCPC_REG_ALERT_RX_STATUS    (1<<2)
#define TCPC_REG_ALERT_POWER_STATUS (1<<1)
#define TCPC_REG_ALERT_CC_STATUS    (1<<0)

#define TCPC_REG_ALERT_RX_MASK		\
		(TCPC_REG_ALERT_RX_STATUS | TCPC_REG_ALERT_RX_BUF_OVF)
#define TCPC_REG_ALERT_HRESET_SUCCESS	\
		(TCPC_REG_ALERT_TX_SUCCESS | TCPC_REG_ALERT_TX_FAILED)
#define TCPC_REG_ALERT_TX_MASK		\
		(TCPC_REG_ALERT_TX_SUCCESS | TCPC_REG_ALERT_TX_FAILED	\
			| TCPC_REG_ALERT_TX_DISCARDED)
#define TCPC_REG_ALERT_TXRX_MASK	\
		(TCPC_REG_ALERT_TX_MASK | TCPC_REG_ALERT_RX_MASK)

/* TCPC Behavior Flags */
#define TCPC_FLAGS_RETRY_CRC_DISCARD		(1<<0)
#define TCPC_FLAGS_WAIT_HRESET_COMPLETE		(1<<1)
#define TCPC_FLAGS_CHECK_CC_STABLE		(1<<2)
#define TCPC_FLAGS_LPM_WAKEUP_WATCHDOG		(1<<3)

enum tcpc_cc_pull {
	TYPEC_CC_RA = 0,
	TYPEC_CC_RP = 1,
	TYPEC_CC_RD = 2,
	TYPEC_CC_OPEN = 3,
	TYPEC_CC_DRP = 4,	/* from Rd */

	TYPEC_CC_RP_DFT = 1,		/* 0x00 + 1 */
	TYPEC_CC_RP_1_5 = 9,		/* 0x08 + 1*/
	TYPEC_CC_RP_3_0 = 17,		/* 0x10 + 1 */

	TYPEC_CC_DRP_DFT = 4,		/* 0x00 + 4 */
	TYPEC_CC_DRP_1_5 = 12,		/* 0x08 + 4 */
	TYPEC_CC_DRP_3_0 = 20,		/* 0x10 + 4 */
};

#define TYPEC_CC_PULL_GET_RES(pull)		(pull & 0x07)
#define TYPEC_CC_PULL_GET_RP_LVL(pull)	((pull & 0x18) >> 3)

enum tcpm_transmit_type {
	TCPC_TX_SOP = 0,
	TCPC_TX_SOP_PRIME = 1,
	TCPC_TX_SOP_PRIME_PRIME = 2,
	TCPC_TX_SOP_DEBUG_PRIME = 3,
	TCPC_TX_SOP_DEBUG_PRIME_PRIME = 4,
	TCPC_TX_HARD_RESET = 5,
	TCPC_TX_CABLE_RESET = 6,
	TCPC_TX_BIST_MODE_2 = 7
};

enum tcpm_rx_cap_type {
	TCPC_RX_CAP_SOP = 1 << 0,
	TCPC_RX_CAP_SOP_PRIME = 1 << 1,
	TCPC_RX_CAP_SOP_PRIME_PRIME = 1 << 2,
	TCPC_RX_CAP_SOP_DEBUG_PRIME = 1 << 3,
	TCPC_RX_CAP_SOP_DEBUG_PRIME_PRIME = 1 << 4,
	TCPC_RX_CAP_HARD_RESET = 1 << 5,
	TCPC_RX_CAP_CABLE_RESET = 1 << 6,
};

struct tcpc_ops {
	int (*init)(struct tcpc_device *tcpc, bool sw_reset);
	int (*deinit)(struct tcpc_device *tcpc);
	int (*alert_status_clear)(struct tcpc_device *tcpc, uint32_t mask);
	int (*fault_status_clear)(struct tcpc_device *tcpc, uint8_t status);
	int (*get_alert_status)(struct tcpc_device *tcpc, uint32_t *alert);
	int (*get_power_status)(struct tcpc_device *tcpc, uint16_t *pwr_status);
	int (*get_fault_status)(struct tcpc_device *tcpc, uint8_t *status);
	int (*get_cc)(struct tcpc_device *tcpc, int *cc1, int *cc2);
	int (*set_cc)(struct tcpc_device *tcpc, int pull);
	int (*set_polarity)(struct tcpc_device *tcpc, int polarity);
	int (*set_vconn)(struct tcpc_device *tcpc, int enable);
	int (*source_vbus)(struct tcpc_device *tcpc, uint8_t type, int mv, int ma);
	int (*sink_vbus)(struct tcpc_device *tcpc, uint8_t type, int mv, int ma);

#ifdef CONFIG_TCPC_LOW_POWER_MODE
	int (*is_low_power_mode)(struct tcpc_device *tcpc);
	int (*set_low_power_mode)(struct tcpc_device *tcpc, bool en, int pull);
#endif /* CONFIG_TCPC_LOW_POWER_MODE */

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	int (*set_msg_header)(struct tcpc_device *tcpc,
					int power_role, int data_role);
	int (*set_rx_enable)(struct tcpc_device *tcpc, uint8_t enable);
	int (*get_message)(struct tcpc_device *tcpc, uint32_t *payload,
			uint16_t *head, enum tcpm_transmit_type *type);
	int (*set_bist_test_mode)(struct tcpc_device *tcpc, bool en);
	int (*set_bist_carrier_mode)(struct tcpc_device *tcpc, uint8_t pattern);

	int (*transmit)(struct tcpc_device *tcpc,
			enum tcpm_transmit_type type,
			uint16_t header, const uint32_t *data);
#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	int (*retransmit)(struct tcpc_device *tcpc);
#endif	/* CONFIG_USB_PD_RETRY_CRC_DISCARD */
#endif	/* CONFIG_USB_POWER_DELIVERY_SUPPORT */

	void (*set_vbus_detect)(struct tcpc_device *tcpc, bool enable);
	void (*reset_pd_fsm)(struct tcpc_device *tcpc);
	void (*tcpc_pd_fsm_state)(struct tcpc_device *tcpc);
};

#define TCPC_VBUS_SOURCE_0V		(0)
#define TCPC_VBUS_SOURCE_5V		(5000)

#define TCPC_VBUS_SINK_0V		(0)
#define TCPC_VBUS_SINK_5V		(5000)

struct tcpc_device {
	struct tcpc_ops *ops;
	void *drv_data;
	struct tcpc_desc desc;
	struct device dev;

	bool wake_lock_user;
	uint8_t wake_lock_pd;
	struct wake_lock attach_wake_lock;
	struct wake_lock dettach_temp_wake_lock;

	/* For tcpc timer & event */
	uint32_t timer_handle_index;
	struct hrtimer tcpc_timer[PD_TIMER_NR];

	ktime_t last_expire[PD_TIMER_NR];
	struct delayed_work timer_handle_work[2];
	struct mutex access_lock;
	struct mutex typec_lock;
	struct mutex timer_lock;
	struct semaphore timer_enable_mask_lock;
	struct spinlock timer_tick_lock;
	atomic_t pending_event;
	uint64_t timer_tick;
	uint64_t timer_enable_mask;
	wait_queue_head_t event_loop_wait_que;
	wait_queue_head_t  timer_wait_que;
	struct task_struct *event_task;
	struct task_struct *timer_task;
	bool timer_thead_stop;
	bool event_loop_thead_stop;

	struct delayed_work	init_work;
	struct srcu_notifier_head evt_nh;

	/* For TCPC TypeC */
	uint8_t typec_state;
	uint8_t typec_role;
	uint8_t typec_attach_old;
	uint8_t typec_attach_new;
	uint8_t typec_local_cc;
	uint8_t typec_local_rp_level;
	uint8_t typec_remote_cc[2];
	uint8_t typec_remote_rp_level;
	uint8_t typec_wait_ps_change;
	bool typec_polarity;
	bool typec_drp_try_timeout;
	bool typec_lpm;
	bool typec_cable_only;
	bool typec_power_ctrl;

	int typec_usb_sink_curr;

	uint8_t typec_lpm_pull;
	uint8_t typec_lpm_retry;

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	uint8_t typec_during_role_swap;
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

#ifdef CONFIG_DUAL_ROLE_USB_INTF
	struct dual_role_phy_instance *dr_usb;
	uint8_t dual_role_supported_modes;
	uint8_t dual_role_mode;
	uint8_t dual_role_pr;
	uint8_t dual_role_dr;
	uint8_t dual_role_vconn;
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	/* Event */
	uint8_t pd_event_count;
	uint8_t pd_event_head_index;
	uint8_t pd_msg_buffer_allocated;

	uint8_t pd_last_vdm_msg_id;
	bool pd_pending_vdm_event;
	bool pd_pending_vdm_reset;
	bool pd_pending_vdm_good_crc;
	bool pd_pending_vdm_discard;
	bool pd_pending_vdm_attention;
	bool pd_postpone_vdm_timeout;

	pd_msg_t pd_last_vdm_msg;
	pd_msg_t pd_attention_vdm_msg;
	pd_event_t pd_vdm_event;

	pd_msg_t pd_msg_buffer[PD_MSG_BUF_SIZE];
	pd_event_t pd_event_ring_buffer[PD_EVENT_BUF_SIZE];

	bool pd_pe_running;
	bool pd_wait_pe_idle;
	bool pd_hard_reset_event_pending;
	bool pd_wait_hard_reset_complete;
	bool pd_wait_pr_swap_complete;
	bool pd_wait_error_recovery;
	bool pd_ping_event_pending;
	uint8_t pd_bist_mode;
	uint8_t pd_transmit_state;
	int pd_wait_vbus_once;

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	bool pd_discard_pending;
#endif

	uint8_t tcpc_flags;

	pd_port_t pd_port;
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

	u8 vbus_level:2;
	u8 irq_enabled:1;

	bool typec_during_direct_charge;
	bool vbus_detect;
};


extern struct tcpc_device *hisi_tcpc_dev_get_by_name(const char *name);
extern struct tcpc_device *hisi_tcpc_device_register(struct device *parent,
	    struct tcpc_desc *tcpc_desc, struct tcpc_ops *ops, void *drv_data);
extern int hisi_tcpc_tcpci_init(struct tcpc_device *tcpc, bool sw_reset);
extern int hisi_tcpc_schedule_init_work(struct tcpc_device *tcpc);
extern int hisi_register_tcp_dev_notifier(struct tcpc_device *tcp_dev,
					struct notifier_block *nb);
extern int hisi_unregister_tcp_dev_notifier(struct tcpc_device *tcp_dev,
					struct notifier_block *nb);
extern void hisi_tcpc_device_unregister(
			struct device *dev, struct tcpc_device *tcpc);
static inline void *hisi_tcpc_get_dev_data(struct tcpc_device *tcpc)
{
	return tcpc->drv_data;
}
static inline void hisi_tcpci_lock_typec(struct tcpc_device *tcpc)
{
	D("+\n");
	mutex_lock(&tcpc->typec_lock);
	D("-\n");
}
static inline void hisi_tcpci_unlock_typec(struct tcpc_device *tcpc)
{
	D("+\n");
	mutex_unlock(&tcpc->typec_lock);
	D("-\n");
}

#endif /* #ifndef __LINUX_TCPCI_CORE_H_ */
