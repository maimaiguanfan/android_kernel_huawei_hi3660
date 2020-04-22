/*
 * Copyright (C) 2018 Hisilicon Technology Corp.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <linux/semaphore.h>
#include <linux/pm_runtime.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/cpu.h>
#include <linux/sched/rt.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <uapi/linux/sched/types.h>
#endif

#include "securec.h"

#include "include/pd_dbg_info.h"
#include "include/tcpci.h"
#include "hi6526.h"
#include "hisi_usb_vbus.h"


#define TCPC_IRQ_POLL_COUNT 20

struct hisi_tcpc_chip {
	struct i2c_client *client;
	struct device *dev;
	struct semaphore suspend_lock;
	struct tcpc_desc *tcpc_desc;
	struct tcpc_device *tcpc;
	struct kthread_worker irq_worker;
	struct kthread_work irq_work;
	struct task_struct *irq_worker_task;
	struct wake_lock irq_wakelock;

	atomic_t poll_count;
	struct delayed_work	poll_work;

	int irq_gpio;
	int irq;
	int low_power_mode;

	uint8_t pd_dbg_rdata_cfg;	/* for pd fsm state */
	uint8_t vbus_detect;		/* record vbus detect state */
#ifdef CONFIG_HISI_TCPC_VBUS_IRQ_ASSIST
	bool vbus_irq;
	int vbus_status;
#endif
	int poll_cc_status;
};

static void hisi_tcpc_command(struct tcpc_device *tcpc, uint8_t cmd);
static int hisi_tcpc_set_cc(struct tcpc_device *tcpc, int pull);

static int hisi_tcpc_read8(struct tcpc_device *tcpc, u8 reg)
{
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc);
	return hisi_tcpc_i2c_read8(chip->client, reg);
}

static int hisi_tcpc_write8(struct tcpc_device *tcpc, u8 reg, u8 data)
{
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc);
	return hisi_tcpc_i2c_write8(chip->client, reg, data);
}

#ifdef CONFIG_HISI_TCPC_DEBUG

static struct hisi_tcpc_chip *hisi_chip;

#define DUMP(name) pr_err("reg:%s \t\t = 0x%x\n", #name,	\
		hisi_tcpc_i2c_read8(chip->client, name))

#define DUMPWORD(name) do {					\
		uint16_t val = 0;					\
		hisi_tcpc_block_read(name, 2, &val);	\
		pr_err("reg:%s \t\t = 0x%x\n", #name, val);	\
	} while (0)

int hisi_tcpc_debug_dump_all_regs(void)
{
	struct hisi_tcpc_chip *chip = hisi_chip;

	if (!chip)
		return -ENODEV;

	DUMPWORD(TCPCI_REG_VENDOR_ID);
	DUMPWORD(TCPCI_REG_PRODUCT_ID);
	DUMPWORD(TCPCI_REG_DEVICE_ID);
	DUMPWORD(TCPCI_REG_USBTYPEC_REV);
	DUMPWORD(TCPCI_REG_USBPD_REV_VER);
	DUMPWORD(TCPCI_REG_PD_INTERFACE_REV);
	DUMPWORD(TCPCI_REG_ALERT);
	DUMPWORD(TCPCI_REG_ALERT_MASK);

	DUMP(TCPCI_REG_POWER_STATUS_MASK);
	DUMP(TCPCI_REG_FAULT_STATUS_MASK);
	DUMP(TCPCI_REG_CONFIG_STANDARD_OUTPUT);
	DUMP(TCPCI_REG_TCPC_CONTROL);
	DUMP(TCPCI_REG_ROLE_CONTROL);
	DUMP(TCPCI_REG_FAULT_CONTROL);
	DUMP(TCPCI_REG_POWER_CONTROL);
	DUMP(TCPCI_REG_CC_STATUS);
	DUMP(TCPCI_REG_POWER_STATUS);
	DUMP(TCPCI_REG_FAULT_STATUS);
	DUMP(TCPCI_REG_COMMAND);

	DUMPWORD(TCPCI_REG_DEVICE_CAPABILITIES_1);
	DUMPWORD(TCPCI_REG_DEVICE_CAPABILITIES_2);

	DUMP(TCPCI_REG_STANDARD_INPUT_CAPABILITIES);
	DUMP(TCPCI_REG_STANDARD_OUTPUT_CAPABILITIES);

	DUMP(TCPCI_REG_MESSAGE_HEADER_INFO);
	DUMP(TCPCI_REG_RECEIVE_DETECT);
	DUMP(TCPCI_REG_RECEIVE_BYTE_COUNT);
	DUMP(TCPCI_REG_RX_BUF_FRAME_TYPE);
	DUMP(TCPCI_REG_RX_BUF_HEADER_BYTE_0);
	DUMP(TCPCI_REG_RX_BUF_HEADER_BYTE_1);

	DUMP(REG_PD_VDM_CFG_0);
	DUMP(REG_PD_VDM_ENABLE);
	DUMP(REG_PD_VDM_CFG_1);
	DUMP(REG_PD_DBG_RDATA_CFG);
	DUMP(REG_PD_DBG_RDATA);
	DUMP(REG_VDM_PAGE_SELECT);

	DUMP(REG_IRQ_MASK);
	DUMP(REG_IRQ_MASK_0);
	DUMP(REG_IRQ_MASK_1);
	DUMP(REG_IRQ_MASK_2);
	DUMP(REG_IRQ_MASK_3);
	DUMP(REG_IRQ_MASK_4);
	DUMP(REG_IRQ_MASK_5);
	DUMP(REG_IRQ_MASK_6);
	DUMP(REG_IRQ_MASK_7);

	DUMP(REG_IRQ_FLAG);
	DUMP(REG_IRQ_FLAG_0);
	DUMP(REG_IRQ_FLAG_1);
	DUMP(REG_IRQ_FLAG_2);
	DUMP(REG_IRQ_FLAG_3);
	DUMP(REG_IRQ_FLAG_4);
	DUMP(REG_IRQ_FLAG_5);
	DUMP(REG_IRQ_FLAG_6);
	DUMP(REG_IRQ_FLAG_7);

	pr_err("gpio value %d\n", gpio_get_value(chip->irq_gpio));

	return 0;
}
static inline void hisi_tcpc_debug_init(struct hisi_tcpc_chip *chip)
{
	hisi_chip = chip;
}
#else
static inline void hisi_tcpc_debug_init(struct hisi_tcpc_chip *chip){}
#endif

#ifdef CONFIG_WORKAROUND_FF_CONER_CHIP_BIST_MODE_ERROR
#define BIST_MODE_2_STOP_TIME_MS 45
static void bist_mode_2_stop_work_fn(struct work_struct *work)
{
	struct hisi_tcpc_chip *chip = hisi_chip;
	D("+\n");
	(void)hisi_tcpc_i2c_write8(chip->client, REG_PD_VDM_ENABLE, 0);
	(void)hisi_tcpc_i2c_write8(chip->client, REG_PD_VDM_ENABLE, 1);
	D("-\n");
}
static DECLARE_DELAYED_WORK(bist_mode_2_stop_work, bist_mode_2_stop_work_fn);
#endif

void hisi_tcpc_enable_vbus(struct i2c_client *client)
{
#ifdef CONFIG_HISI_TCPC_QUIRK_V100
	uint8_t reg;
	int ret;

	D("+\n");

	/* disable ForceDischargeDisconnect */
	ret = hisi_tcpc_i2c_read8(client, TCPC_V10_REG_POWER_CTRL);
	if (ret < 0) {
		E("hisi_tcpc_i2c_read8 error return %d\n", ret);
		return;
	}

	reg = (uint8_t)ret;
	reg &= ~(TCPC_V10_REG_POWER_CTRL_FORCE_DISCHARGE_DISCONNECT);
	(void)hisi_tcpc_i2c_write8(client, TCPC_V10_REG_POWER_CTRL, reg);

	D("-\n");
#endif
}

void hisi_tcpc_disable_vbus(struct i2c_client *client)
{
#ifdef CONFIG_HISI_TCPC_QUIRK_V100
	uint8_t reg;
	int ret;

	D("+\n");

	/* disable ForceDischargeDisconnect */
	ret = hisi_tcpc_i2c_read8(client, TCPC_V10_REG_POWER_CTRL);
	if (ret < 0) {
		E("hisi_tcpc_i2c_read8 error return %d\n", ret);
		return;
	}

	reg = (uint8_t)ret;
	reg |= (TCPC_V10_REG_POWER_CTRL_FORCE_DISCHARGE_DISCONNECT);
	(void)hisi_tcpc_i2c_write8(client, TCPC_V10_REG_POWER_CTRL, reg);
	mdelay(50);

	D("-\n");
#endif
}

static int hisi_tcpc_source_vbus(struct tcpc_device *tcpc, uint8_t type, int mv, int ma)
{
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc);

	D("type: 0x%x\n", type);

	if (type == TCP_VBUS_CTRL_REMOVE) {
		hisi_tcpc_disable_vbus(chip->client);
		return 0;
	}

	if (type == TCP_VBUS_CTRL_TYPEC) {
		if (mv != TCPC_VBUS_SOURCE_0V)
			hisi_tcpc_enable_vbus(chip->client);
		else
			hisi_tcpc_disable_vbus(chip->client);

		return 0;
	}

	switch (type) {
	case TCP_VBUS_CTRL_HRESET:
	case TCP_VBUS_CTRL_PR_SWAP:
	case TCP_VBUS_CTRL_REQUEST:
	case TCP_VBUS_CTRL_STANDBY:
	case TCP_VBUS_CTRL_STANDBY_UP:
	case TCP_VBUS_CTRL_STANDBY_DOWN:
		if (mv != TCPC_VBUS_SOURCE_0V) {
			D("PD source vbus %dmV %dmA\n", mv, ma);
			hisi_tcpc_enable_vbus(chip->client);
		} else {
			hisi_tcpc_disable_vbus(chip->client);
		}
		break;
	case TCP_VBUS_CTRL_PD_HRESET:
	case TCP_VBUS_CTRL_PD_PR_SWAP:
	case TCP_VBUS_CTRL_PD_REQUEST:
	case TCP_VBUS_CTRL_PD_STANDBY:
	case TCP_VBUS_CTRL_PD_STANDBY_UP:
	case TCP_VBUS_CTRL_PD_STANDBY_DOWN:
		if (mv != TCPC_VBUS_SOURCE_0V) {
			D("PD_DETECT source vbus %dmV %dmA\n", mv, ma);
			hisi_tcpc_enable_vbus(chip->client);
		} else {
			hisi_tcpc_disable_vbus(chip->client);
		}
		break;
	default:
		break;
	}

	D("-\n");
	return 0;
}

static int hisi_tcpc_sink_vbus(struct tcpc_device *tcpc, uint8_t type, int mv, int ma)
{
	return 0;
}

static void hisi_tcpc_set_vbus_detect(struct tcpc_device *tcpc, bool enable)
{
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc);
	uint8_t reg;
	int ret;

	D("enable %d\n", enable);

	if (chip->vbus_detect == enable) {
		D("vbus_detect already %s\n", enable ? "enabled" : "disabled");
		return;
	}

	chip->vbus_detect = enable;
	if (enable) {
		hisi_tcpc_command(tcpc, TCPM_CMD_ENABLE_VBUS_DETECT);

		/* enable AutoDischargeDisconnect */
		ret = hisi_tcpc_i2c_read8(chip->client, TCPC_V10_REG_POWER_CTRL);
		if (ret < 0) {
			E("hisi_tcpc_i2c_read8 error return %d\n", ret);
			return;
		}

		reg = (uint8_t)ret;
		reg |= TCPC_V10_REG_POWER_CTRL_AUTO_DISCHARGE_DISCONNECT;
		(void)hisi_tcpc_i2c_write8(chip->client, TCPC_V10_REG_POWER_CTRL, reg);
	} else {
		hisi_tcpc_command(tcpc, TCPM_CMD_DISABLE_VBUS_DETECT);

		/* disable AutoDischargeDisconnect */
		ret = hisi_tcpc_i2c_read8(chip->client, TCPC_V10_REG_POWER_CTRL);
		if (ret < 0) {
			E("hisi_tcpc_i2c_read8 error return %d\n", ret);
			return;
		}

		reg = (uint8_t)ret;
		reg &= ~TCPC_V10_REG_POWER_CTRL_AUTO_DISCHARGE_DISCONNECT;
		(void)hisi_tcpc_i2c_write8(chip->client, TCPC_V10_REG_POWER_CTRL, reg);
	}
}

static int __hisi_tcpc_pd_fsm_reset(struct i2c_client *client)
{
	uint8_t reg;
	int ret;

	D("+\n");
	ret = hisi_tcpc_i2c_read8(client, REG_PD_VDM_CFG_1);
	if (ret < 0) {
		E("hisi_tcpc_i2c_read8 error return %d\n", ret);
		return ret;
	}

	reg = (uint8_t)ret;
	reg |= PD_TC_ALL_RESET;
	(void)hisi_tcpc_i2c_write8(client, REG_PD_VDM_CFG_1, reg);

	ret = hisi_tcpc_i2c_read8(client, REG_PD_VDM_CFG_1);
	if (ret < 0) {
		E("hisi_tcpc_i2c_read8 error return %d\n", ret);
		return ret;
	}

	reg = (uint8_t)ret;
	reg &= ~PD_TC_ALL_RESET;
	(void)hisi_tcpc_i2c_write8(client, REG_PD_VDM_CFG_1, reg);
	D("-\n");

	return 0;
}

static void hisi_tcpc_pd_fsm_reset(struct tcpc_device *tcpc)
{
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc);
	D("+\n");
#ifdef CONFIG_WORKAROUND_FF_CONER_CHIP_BIST_MODE_ERROR
	D("cancel bist_mode_2_stop_work\n");
	cancel_delayed_work_sync(&bist_mode_2_stop_work);
#endif
	(void)__hisi_tcpc_pd_fsm_reset(chip->client);
	D("-\n");
}

static void __get_tcpc_fsm_state(struct tcpc_device *tcpc)
{
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc);
	u8 rdata;
	s32 ret;
	char *tc_cu_st[16] = {
		"IDLE",
		"Unattached.SNK",
		"Attachwait.SNK",
		"Attached.SNK",
		"Unattached.SRC",
		"Attachwait.SRC",
		"Attached.SRC",
		"DegbuAccessory.SNK",
		"Orientation.Snk",
		"Attached.AudioAcc",
		"Try.SNK",
		"TryWait.SRC",
	};

	if (chip->pd_dbg_rdata_cfg != PD_DBG_RDATA_EN) {
		(void)hisi_tcpc_i2c_write8(chip->client, REG_PD_DBG_RDATA_CFG, PD_DBG_RDATA_EN);
		chip->pd_dbg_rdata_cfg = PD_DBG_RDATA_EN;
	}

	ret = hisi_tcpc_i2c_read8(chip->client, REG_PD_DBG_RDATA);
	if (ret < 0) {
		E("i2c read8 error ret %d\n", ret);
		return;
	}

	rdata = (u8)(u32)ret;
	D("pd_dbg_rdata 0x%02x\n", rdata);

	if (rdata & (1 << 6))
		D("DebugAccessory.Snk\n");

	if (rdata & (1 << 5))
		D("PD Attached\n");
	else
		D("PD Unattached or AttachWait\n");

	if (rdata & (1 << 4))
		D("TypeC Attached\n");
	else
		D("TypeC Unattached or AttachWait\n");

	D("tc_cu_st %s\n", tc_cu_st[rdata & 0xf]);
}

static void hisi_tcpc_pd_fsm_state(struct tcpc_device *tcpc)
{
	__get_tcpc_fsm_state(tcpc);
}

static void hisi_tcpc_init_alert_mask(struct tcpc_device *tcpc)
{
	uint16_t mask;
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc);

	mask = TCPC_V10_REG_ALERT_CC_STATUS | TCPC_V10_REG_ALERT_POWER_STATUS;

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	/* Need to handle RX overflow */
	mask |= TCPC_V10_REG_ALERT_TX_SUCCESS | TCPC_V10_REG_ALERT_TX_DISCARDED
			| TCPC_V10_REG_ALERT_TX_FAILED
			| TCPC_V10_REG_ALERT_RX_HARD_RST
			| TCPC_V10_REG_ALERT_RX_STATUS
			| TCPC_V10_REG_RX_OVERFLOW;
#endif

	mask |= TCPC_REG_ALERT_FAULT;

	(void)hisi_tcpc_i2c_write16(chip->client, TCPC_V10_REG_ALERT_MASK, mask);
}

static void hisi_tcpc_init_power_status_mask(struct tcpc_device *tcpc)
{
	const uint8_t mask = TCPC_V10_REG_POWER_STATUS_VBUS_PRES;
	(void)hisi_tcpc_write8(tcpc,
			TCPC_V10_REG_POWER_STATUS_MASK, mask);
}

static void hisi_tcpc_init_fault_mask(struct tcpc_device *tcpc)
{
	const uint8_t mask = TCPC_V10_REG_FAULT_STATUS_VCONN_OV |
			TCPC_V10_REG_FAULT_STATUS_VCONN_OC;

	(void)hisi_tcpc_write8(tcpc,
			TCPC_V10_REG_FAULT_STATUS_MASK, mask);
}

static void hisi_tcpc_command(struct tcpc_device *tcpc, uint8_t cmd)
{
	D("cmd 0x%x\n", cmd);
	(void)hisi_tcpc_write8(tcpc, TCPC_V10_REG_COMMAND, cmd);
}

static int hisi_tcpc_tcpc_init(struct tcpc_device *tcpc, bool sw_reset)
{
	D("+\n");
	if (sw_reset) {
		D("sw_reset true!!!\n");
	}

	/* UFP Both RD setting
	 * DRP = 0, RpVal = 0 (Default), Rd, Rd */
	(void)hisi_tcpc_write8(tcpc, TCPC_V10_REG_ROLE_CTRL,
			TCPC_V10_REG_ROLE_CTRL_RES_SET(0, 0, CC_RD, CC_RD));


	tcpci_alert_status_clear(tcpc, 0xffffffff);

	hisi_tcpc_init_power_status_mask(tcpc);
	hisi_tcpc_init_alert_mask(tcpc);
	hisi_tcpc_init_fault_mask(tcpc);

	D("-\n");
	return 0;
}

static int hisi_tcpc_tcpc_deinit(struct tcpc_device *tcpc_dev)
{
	D("+\n");

#ifdef CONFIG_TCPC_SHUTDOWN_CC_DETACH
	hisi_tcpc_set_cc(tcpc_dev, TYPEC_CC_DRP);
	hisi_tcpc_set_cc(tcpc_dev, TYPEC_CC_OPEN);
#endif	/* CONFIG_TCPC_SHUTDOWN_CC_DETACH */

	D("-\n");

	return 0;
}

static int hisi_tcpc_alert_status_clear(struct tcpc_device *tcpc, uint32_t mask)
{
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc);
	uint16_t mask_t1;

	/* Write 1 clear */
	mask_t1 = (uint16_t) mask;
	if (mask_t1) {
		D("clear alert 0x%x\n", mask_t1);
		(void)hisi_tcpc_i2c_write16(chip->client, TCPC_V10_REG_ALERT, mask_t1);
	}

	return 0;
}

static int hisi_tcpc_fault_status_clear(struct tcpc_device *tcpc, uint8_t status)
{
	/* Write 1 clear (Check it later )*/

	D("+\n");
	(void)hisi_tcpc_write8(tcpc, TCPC_V10_REG_FAULT_STATUS, status);
	D("-\n");
	return 0;
}

static int hisi_tcpc_get_alert_status(struct tcpc_device *tcpc, uint32_t *alert)
{
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc);
	int ret;

	ret = hisi_tcpc_i2c_read16(chip->client, TCPC_V10_REG_ALERT);
	if (ret < 0)
		return ret;

	*alert = (uint16_t) ret;

	if (chip->poll_cc_status) {
		D("poll_cc_status, cc_status_change\n");
		*alert |= TCPC_REG_ALERT_CC_STATUS;
		chip->poll_cc_status = 0;
	}

#ifdef CONFIG_HISI_TCPC_VBUS_IRQ_ASSIST
	if (chip->vbus_irq) {
		D("vbus irq issue power status irq\n");
		chip->vbus_irq = false;
		*alert |= TCPC_REG_ALERT_POWER_STATUS;
	}
#endif
	D("raw alert status: 0x%x\n", *alert);

	return 0;
}

static void enable_auto_discharge_disconnect(struct hisi_tcpc_chip *chip)
{
	uint8_t reg;
	int ret;

	D("enable auto discharge disconnect\n");
	ret = hisi_tcpc_i2c_read8(chip->client, TCPC_V10_REG_POWER_CTRL);
	if (ret < 0) {
		E("hisi_tcpc_i2c_read8 error ret %d\n", ret);
		return;
	}

	reg = (uint8_t)(unsigned)ret;
	reg |= TCPC_V10_REG_POWER_CTRL_AUTO_DISCHARGE_DISCONNECT;
	(void)hisi_tcpc_i2c_write8(chip->client, TCPC_V10_REG_POWER_CTRL, reg);
}

static int hisi_tcpc_get_power_status(struct tcpc_device *tcpc, uint16_t *pwr_status)
{
	uint8_t ps_reg;
	int ret;
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc);

	D("+\n");
	ret = hisi_tcpc_read8(tcpc, TCPC_V10_REG_POWER_STATUS);
	if (ret < 0)
		return ret;

	ps_reg = (uint8_t)(unsigned int)ret;
	D("raw power_status 0x%x\n", ps_reg);

	*pwr_status = 0;

	if (ps_reg & TCPC_V10_REG_POWER_STATUS_SINK_VBUS)
		*pwr_status |= TCPC_REG_POWER_STATUS_SINK_VBUS;

	if (ps_reg & TCPC_V10_REG_POWER_STATUS_VCONN_PRES)
		*pwr_status |= TCPC_REG_POWER_STATUS_VCONN_PRES;

	if (ps_reg & TCPC_V10_REG_POWER_STATUS_VBUS_PRES_DET)
		*pwr_status |= TCPC_REG_POWER_STATUS_VBUS_PRES_DET;

	if (ps_reg & TCPC_V10_REG_POWER_STATUS_SRC_VBUS)
		*pwr_status |= TCPC_REG_POWER_STATUS_SRC_VBUS;

	if (chip->vbus_detect) {
		if (ps_reg & TCPC_V10_REG_POWER_STATUS_VBUS_PRES) {
			*pwr_status |= TCPC_REG_POWER_STATUS_VBUS_PRES;
			enable_auto_discharge_disconnect(chip);
		}
	} else {
#ifdef CONFIG_HISI_TCPC_VBUS_IRQ_ASSIST
		D("vbus_status %d\n", chip->vbus_status);
		if (chip->vbus_status == 0) {
			*pwr_status &= ~TCPC_REG_POWER_STATUS_VBUS_PRES;
		} else {
			*pwr_status |= TCPC_REG_POWER_STATUS_VBUS_PRES;
		}
#endif
	}

	D("pwr_status 0x%x\n", *pwr_status);
	D("-\n");
	return 0;
}

static int hisi_tcpc_get_fault_status(struct tcpc_device *tcpc, uint8_t *status)
{
	int ret;

	D("+\n");
	ret = hisi_tcpc_read8(tcpc, TCPC_V10_REG_FAULT_STATUS);
	if (ret < 0)
		return ret;
	D("raw fault status 0x%x\n", ret);
	*status = (uint8_t) ret;
	D("-\n");
	return 0;
}

static int hisi_tcpc_get_cc(struct tcpc_device *tcpc, int *cc1, int *cc2)
{
	int status, role_ctrl, cc_role;
	bool act_as_sink, act_as_drp;

	status = hisi_tcpc_read8(tcpc, TCPC_V10_REG_CC_STATUS);
	if (status < 0)
		return status;
	D("cc status reg: 0x%x\n", status);

	role_ctrl = hisi_tcpc_read8(tcpc, TCPC_V10_REG_ROLE_CTRL);
	if (role_ctrl < 0)
		return role_ctrl;
	D("role ctrl reg: 0x%x\n", role_ctrl);

	if (status & TCPC_V10_REG_CC_STATUS_DRP_TOGGLING) {
		*cc1 = TYPEC_CC_DRP_TOGGLING;
		*cc2 = TYPEC_CC_DRP_TOGGLING;
		return 0;
	}

	*cc1 = TCPC_V10_REG_CC_STATUS_CC1(status);
	*cc2 = TCPC_V10_REG_CC_STATUS_CC2(status);

	act_as_drp = TCPC_V10_REG_ROLE_CTRL_DRP & role_ctrl;
	D("act_as_drp %d\n", act_as_drp);

	if (act_as_drp) {
		act_as_sink = TCPC_V10_REG_CC_STATUS_DRP_RESULT(status);
	} else {
		cc_role =  TCPC_V10_REG_CC_STATUS_CC1(role_ctrl);
		if (cc_role == TYPEC_CC_RP)
			act_as_sink = false;
		else
			act_as_sink = true;
	}

	/*
	 * If status is not open, then OR in termination to convert to
	 * enum tcpm_cc_voltage_status.
	 */

	if ((*cc1 != TYPEC_CC_VOLT_OPEN) && act_as_sink)
		*cc1 |= TYPEC_CC_VOLT_ACT_AS_SINK;

	if ((*cc2 != TYPEC_CC_VOLT_OPEN) && act_as_sink)
		*cc2 |= TYPEC_CC_VOLT_ACT_AS_SINK;

	return 0;
}

static int hisi_tcpc_set_cc(struct tcpc_device *tcpc, int pull)
{
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc);
	uint8_t data;
	int rp_lvl;

	rp_lvl = TYPEC_CC_PULL_GET_RP_LVL(pull);
	pull = TYPEC_CC_PULL_GET_RES(pull);

	D("rp_lvl %x, pull 0x%x\n", rp_lvl, pull);

	if (pull == TYPEC_CC_DRP) {
		data = TCPC_V10_REG_ROLE_CTRL_RES_SET(1, rp_lvl, TYPEC_CC_RD, TYPEC_CC_RD);
		D("DRP set role ctrl 0x%x\n", data);
		(void)hisi_tcpc_i2c_write8(chip->client, TCPC_V10_REG_ROLE_CTRL, data);

		hisi_tcpc_command(tcpc, TCPM_CMD_LOOK_CONNECTION);
	} else {
		data = TCPC_V10_REG_ROLE_CTRL_RES_SET(0, rp_lvl, pull, pull);
		D("set role ctrl 0x%x\n", data);
		(void)hisi_tcpc_write8(tcpc, TCPC_V10_REG_ROLE_CTRL, data);
	}

	return 0;
}

static int hisi_tcpc_set_polarity(struct tcpc_device *tcpc, int polarity)
{
	int data;

	data = hisi_tcpc_read8(tcpc, TCPC_V10_REG_TCPC_CTRL);
	if (data < 0)
		return data;

	data &= ~TCPC_V10_REG_TCPC_CTRL_PLUG_ORIENT;
	data |= polarity ? TCPC_V10_REG_TCPC_CTRL_PLUG_ORIENT : 0;

	D("set TCPC_CTRL 0x%x\n", data);
	(void)hisi_tcpc_write8(tcpc, TCPC_V10_REG_TCPC_CTRL, data);

	return 0;
}

static int hisi_tcpc_set_vconn(struct tcpc_device *tcpc, int enable)
{
	int data;

	D("%d\n", enable);

	data = hisi_tcpc_read8(tcpc, TCPC_V10_REG_POWER_CTRL);
	if (data < 0)
		return data;

	data &= ~TCPC_V10_REG_POWER_CTRL_VCONN;
	data |= enable ? TCPC_V10_REG_POWER_CTRL_VCONN : 0;
	(void)hisi_tcpc_write8(tcpc, TCPC_V10_REG_POWER_CTRL, data);

	hisi_usb_typec_set_vconn(enable);

	return 0;
}


#ifdef CONFIG_TCPC_LOW_POWER_MODE
static int hisi_tcpc_is_low_power_mode(struct tcpc_device *tcpc_dev)
{
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc_dev);
	return chip->low_power_mode;
}

static int hisi_tcpc_set_low_power_mode(
		struct tcpc_device *tcpc_dev, bool en, int pull)
{
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc_dev);

	chip->low_power_mode = en;
	return 0;
}
#endif	/* CONFIG_TCPC_LOW_POWER_MODE */


#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
static int hisi_tcpc_set_msg_header(struct tcpc_device *tcpc,
		int power_role, int data_role)
{
	uint8_t msg_header = TCPC_V10_REG_MSG_HDR_INFO_SET(data_role, power_role);
	D("msg_header 0x%02x\n", msg_header);
	(void)hisi_tcpc_write8(tcpc, TCPC_V10_REG_MSG_HDR_INFO, msg_header);
	return 0;
}

static int hisi_tcpc_set_rx_enable(struct tcpc_device *tcpc, uint8_t enable)
{
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc);
	uint8_t reg = 0;
	int ret = 0;

	ret = hisi_tcpc_i2c_read8(chip->client, TCPC_V10_REG_RX_DETECT);
	if (ret < 0) {
		E("hisi_tcpc_i2c_read8 error ret %d\n", ret);
		return ret;
	}

	reg = (uint8_t)(unsigned)ret;
	D("RX_DETECT 0x%x, enable 0x%x\n", reg, enable);

	reg = enable;
	ret = hisi_tcpc_i2c_write8(chip->client, TCPC_V10_REG_RX_DETECT, reg);

	return ret;
}

static int hisi_tcpc_get_message(struct tcpc_device *tcpc, uint32_t *payload,
			uint16_t *msg_head, enum tcpm_transmit_type *frame_type)
{
	int rv;
	uint8_t type, cnt = 0;
	uint8_t buf[8] = {0};
	const uint16_t alert_rx = TCPC_V10_REG_ALERT_RX_STATUS|TCPC_V10_REG_RX_OVERFLOW;

	D("+\n");
	rv = hisi_tcpc_block_read(TCPC_V10_REG_RX_BYTE_CNT, 8, buf);
	if (rv) {
		E("hisi_tcpc_block_read error rv %d\n", rv);
		return rv;
	}

	cnt = buf[0];	/* length, refer to tcpci spec */
	type = buf[1];	/* bit0~2: SOP, SOP', SOP'', refer to tcpci spec */
	*msg_head = *(uint16_t *)&buf[2];
	payload[0] = *(uint32_t *)&buf[4];

	/* TCPC 1.0 ==> no need to subtract the size of msg_head */
	if (cnt > 7) {
		cnt -= 7; /* MSG_HDR */
		rv = hisi_tcpc_block_read(TCPC_V10_REG_RX_DATA + 4, cnt, (uint8_t *)(payload + 1));
		if (rv) {
			E("hisi_tcpc_block_read error rv %d\n", rv);
			return rv;
		}
	}

	*frame_type = (enum tcpm_transmit_type) type;

	/*
	 * FIXME:
	 * TD.PD.VDMU.E17
	 * do not clear rx_alert hera, but after put_msg
	 * because it will check rx_status when send msg
	 */

	/* Read complete, clear RX status alert bit */
	tcpci_alert_status_clear(tcpc, alert_rx);

	D("-\n");
	return rv;
}

static int hisi_tcpc_set_bist_test_mode(struct tcpc_device *tcpc, bool en)
{
	int data;

	D("+\n");

	/* receive mode */
	data = hisi_tcpc_read8(tcpc, TCPC_V10_REG_TCPC_CTRL);
	if (data < 0)
		return data;

	data &= ~TCPC_V10_REG_TCPC_CTRL_BIST_TEST_MODE;
	data |= en ? TCPC_V10_REG_TCPC_CTRL_BIST_TEST_MODE : 0;
	(void)hisi_tcpc_write8(tcpc, TCPC_V10_REG_TCPC_CTRL, data);

	D("-\n");
	return 0;
}

static int hisi_tcpc_set_bist_carrier_mode(struct tcpc_device *tcpc, uint8_t pattern)
{
	D("\n");
	/* Don't support this function */
	return 0;
}

/* total length(1byte) + message header (2byte) + data object (7*4) */
#define TRANSMIT_MAX_SIZE	(1 + sizeof(uint16_t) + sizeof(uint32_t)*7)

static int hisi_tcpc_transmit(struct tcpc_device *tcpc,
	enum tcpm_transmit_type type, uint16_t header, const uint32_t *data)
{
	int rv;
	int data_cnt, packet_cnt;
	uint8_t temp[TRANSMIT_MAX_SIZE];

	D("type %d\n", type);
	if (type < TCPC_TX_HARD_RESET) {
		data_cnt = sizeof(uint32_t) * PD_HEADER_CNT(header);
		packet_cnt = data_cnt + sizeof(uint16_t);

		temp[0] = packet_cnt;
		memcpy_s(temp + 1, 2, (uint8_t *)&header, 2);
		if (data_cnt > 0)
			memcpy_s(temp + 3, TRANSMIT_MAX_SIZE - 3,
				(uint8_t *)data, data_cnt);

		D("to block_write\n");
		rv = hisi_tcpc_block_write(TCPC_V10_REG_TX_BYTE_CNT,
				packet_cnt + 1, (uint8_t *)temp);
		if (rv) {
			E("hisi_tcpc_block_write error rv %d\n", rv);
			return rv;
		}

#ifdef CONFIG_WORKAROUND_FF_CONER_CHIP_BIST_MODE_ERROR
	} else if (type == TCPC_TX_BIST_MODE_2) {
		D("transmit BIST Mode 2\n");
		schedule_delayed_work(&bist_mode_2_stop_work,
				msecs_to_jiffies(BIST_MODE_2_STOP_TIME_MS));
#endif
	}

	(void)hisi_tcpc_write8(tcpc, TCPC_V10_REG_TRANSMIT,
				TCPC_V10_REG_TRANSMIT_SET(type));

	return 0;
}

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
static int hisi_tcpc_retransmit(struct tcpc_device *tcpc)
{
	D("+-\n");
	(void)hisi_tcpc_write8(tcpc, TCPC_V10_REG_TRANSMIT,
			TCPC_V10_REG_TRANSMIT_SET(TCPC_TX_SOP));
	return 0;
}
#endif /* CONFIG_USB_PD_RETRY_CRC_DISCARD */

#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

static struct tcpc_ops hisi_tcpc_ops = {
	.init = hisi_tcpc_tcpc_init,
	.alert_status_clear = hisi_tcpc_alert_status_clear,
	.fault_status_clear = hisi_tcpc_fault_status_clear,
	.get_alert_status = hisi_tcpc_get_alert_status,
	.get_power_status = hisi_tcpc_get_power_status,
	.get_fault_status = hisi_tcpc_get_fault_status,
	.get_cc = hisi_tcpc_get_cc,
	.set_cc = hisi_tcpc_set_cc,
	.set_polarity = hisi_tcpc_set_polarity,
	.set_vconn = hisi_tcpc_set_vconn,
	.source_vbus = hisi_tcpc_source_vbus,
	.sink_vbus = hisi_tcpc_sink_vbus,
	.deinit = hisi_tcpc_tcpc_deinit,

#ifdef CONFIG_TCPC_LOW_POWER_MODE
	.is_low_power_mode = hisi_tcpc_is_low_power_mode,
	.set_low_power_mode = hisi_tcpc_set_low_power_mode,
#endif /* CONFIG_TCPC_LOW_POWER_MODE */


#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	.set_msg_header = hisi_tcpc_set_msg_header,
	.set_rx_enable = hisi_tcpc_set_rx_enable,
	.get_message = hisi_tcpc_get_message,
	.set_bist_test_mode = hisi_tcpc_set_bist_test_mode,
	.set_bist_carrier_mode = hisi_tcpc_set_bist_carrier_mode,
	.transmit = hisi_tcpc_transmit,

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	.retransmit = hisi_tcpc_retransmit,
#endif /* CONFIG_USB_PD_RETRY_CRC_DISCARD */
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */

	.set_vbus_detect = hisi_tcpc_set_vbus_detect,
	.reset_pd_fsm = hisi_tcpc_pd_fsm_reset,
	.tcpc_pd_fsm_state = hisi_tcpc_pd_fsm_state,
};

static inline void hisi_tcpc_poll_ctrl(struct hisi_tcpc_chip *chip)
{
	cancel_delayed_work_sync(&chip->poll_work);

	if (atomic_read(&chip->poll_count) == 0) {
		atomic_inc(&chip->poll_count);
		cpu_idle_poll_ctrl(true);
	}

	schedule_delayed_work(&chip->poll_work, msecs_to_jiffies(40));
}

static void hisi_tcpc_poll_work(struct work_struct *work)
{
	struct hisi_tcpc_chip *chip = container_of(
			work, struct hisi_tcpc_chip, poll_work.work);

	if (atomic_dec_and_test(&chip->poll_count))
		cpu_idle_poll_ctrl(false);
}

void hisi_tcpc_poll_cc_status(struct tcpc_device *tcpc)
{
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc);
	D("+-\n");
	chip->poll_cc_status = 1;
	kthread_queue_work(&chip->irq_worker, &chip->irq_work);
}

static void hisi_tcpc_irq_work_handler(struct kthread_work *work)
{
	struct hisi_tcpc_chip *chip =
			container_of(work, struct hisi_tcpc_chip, irq_work);
	int ret = 0;
	int gpio_val;

	D("+\n");

	hisi_tcpc_poll_ctrl(chip);

	/* make sure I2C bus had resumed */
	down(&chip->suspend_lock);
	hisi_tcpci_lock_typec(chip->tcpc); /* Coution this lock !!! */

	do {
		ret = hisi_tcpci_alert(chip->tcpc);
		if (ret)
			break;

		gpio_val = gpio_get_value(chip->irq_gpio);
	} while (gpio_val == 0);

	hisi_tcpci_unlock_typec(chip->tcpc);
	up(&chip->suspend_lock);

	if (wake_lock_active(&chip->irq_wakelock))
		wake_unlock(&chip->irq_wakelock); /*lint !e455*/

	D("-\n");
}

static irqreturn_t hisi_tcpc_intr_handler(int irq, void *data)
{
	struct hisi_tcpc_chip *chip = data;

	D("\n");

	if (!wake_lock_active(&chip->irq_wakelock))
		wake_lock(&chip->irq_wakelock);

	kthread_queue_work(&chip->irq_worker, &chip->irq_work); /*lint !e456*/

	return IRQ_HANDLED; /*lint !e454*/
}

/*lint -e454 -e456*/
void hisi_tcpc_vbus_irq_handler(void *data, int vbus_status)
{
	struct hisi_tcpc_chip *chip = data;

	if (!chip) {
		E("Invalid tcpc chip!\n");
		return;
	}

	D("vbus_status %d\n", vbus_status);

#ifdef CONFIG_HISI_TCPC_VBUS_IRQ_ASSIST
	chip->vbus_irq = true;

	if (vbus_status != 0)
		chip->vbus_status = 1;
	else
		chip->vbus_status = 0;

	if (!chip->vbus_detect) {
		if (!wake_lock_active(&chip->irq_wakelock))
			wake_lock(&chip->irq_wakelock);

		D("vbus detect disabled, queue tcpc irq work!\n");
		kthread_queue_work(&chip->irq_worker, &chip->irq_work);
	}
#endif
}
/*lint +e454 +e456*/

static void __hisi_tcpc_init_alert(struct hisi_tcpc_chip *chip)
{
	uint8_t reg_u8;
	int ret;

	D("+\n");
	/* Clear Alert Mask & Status */
	(void)hisi_tcpc_i2c_write16(chip->client, TCPC_V10_REG_ALERT_MASK, 0);
	(void)hisi_tcpc_i2c_write16(chip->client, TCPC_V10_REG_ALERT, 0xffff);

	ret = hisi_tcpc_i2c_read8(chip->client, REG_IRQ_MASK);
	if (ret < 0) {
		E("hisi_tcpc_i2c_read8 error ret %d\n", ret);
		return;
	}

	reg_u8 = (uint8_t)(unsigned)ret;
	D("REG_IRQ_MASK 0x%x\n", reg_u8);
	if (reg_u8 & BIT_IRQ_MASK_GLB) {
		D("global irq unmask\n");
		reg_u8 &= ~BIT_IRQ_MASK_GLB;
		(void)hisi_tcpc_i2c_write8(chip->client, REG_IRQ_MASK, reg_u8);
	}
	if (reg_u8 & BIT_IRQ_MASK_SRC) {
		D("global irq source unmask\n");
		reg_u8 &= ~BIT_IRQ_MASK_SRC;
		(void)hisi_tcpc_i2c_write8(chip->client, REG_IRQ_MASK, reg_u8);
	}
	if (reg_u8 & BIT_IRQ_MASK_PD) {
		reg_u8 &= ~BIT_IRQ_MASK_PD;
		(void)hisi_tcpc_i2c_write8(chip->client, REG_IRQ_MASK, reg_u8);
	}

	D("-\n");
}

static int hisi_tcpc_init_alert(struct tcpc_device *tcpc)
{
	struct hisi_tcpc_chip *chip = hisi_tcpc_get_dev_data(tcpc);
	struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };
	int ret = 0;

	D("+\n");

	__hisi_tcpc_init_alert(chip);

	pr_info("%s : name = %s\n", __func__, chip->tcpc_desc->name);

	chip->irq = gpio_to_irq(chip->irq_gpio);
	pr_info("%s : gpio # = %d\n", __func__, chip->irq_gpio);
	pr_info("%s : IRQ number = %d\n", __func__, chip->irq);

	kthread_init_worker(&chip->irq_worker);
	chip->irq_worker_task = kthread_run(kthread_worker_fn,
			&chip->irq_worker, chip->tcpc_desc->name);
	if (IS_ERR(chip->irq_worker_task)) {
		pr_err("%s : Could not create tcpc task\n", __func__);
		ret = -EINVAL;
		goto out;
	}

	sched_setscheduler(chip->irq_worker_task, SCHED_FIFO, &param);
	kthread_init_work(&chip->irq_work, hisi_tcpc_irq_work_handler);
	wake_lock_init(&chip->irq_wakelock, WAKE_LOCK_SUSPEND, "hisi_tcpc_irq_wakelock");

	/*
	 * Request irq.
	 */
#ifdef CONFIG_HISI_TCPC_DEBUG
	ret = gpio_get_value(chip->irq_gpio);
	D("gpio %d value %d\n", chip->irq_gpio, ret);
#endif

	ret = request_irq(chip->irq, hisi_tcpc_intr_handler,
			IRQF_SHARED | IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND
			| IRQF_NO_THREAD,
			chip->tcpc_desc->name, chip);
	if (ret) {
		pr_err("%s : request_irq error ret %d\n", __func__, ret);
		goto out;
	}

	/*
	 * need this statement ??
	 */
	ret = enable_irq_wake(chip->irq);
	if (ret) {
		pr_err("%s : enable_irq_wake error ret %d\n", __func__, ret);
	}
out:
	D("-\n");
	return ret;
}

static int hisi_tcpc_parse_dt(struct hisi_tcpc_chip *chip, struct device *dev)
{
	chip->irq_gpio = hi6526_irq_gpio;
	D("irq_gpio %d\n", chip->irq_gpio);
	return 0;
}

static int hisi_tcpc_tcpcdev_init(struct hisi_tcpc_chip *chip, struct device *dev)
{
	struct tcpc_desc *desc;
	struct device_node *np = dev->of_node;
	u32 val = 0;

	D("+\n");
	desc = devm_kzalloc(dev, sizeof(*desc), GFP_KERNEL);
	if (!desc)
		return -ENOMEM;

	if (of_property_read_u32(np, "hisi-tcpc,role_def", &val) >= 0) {
		if (val >= TYPEC_ROLE_NR)
			desc->role_def = TYPEC_ROLE_DRP;
		else
			desc->role_def = val;
	} else {
		dev_info(dev, "use default Role DRP\n");
		desc->role_def = TYPEC_ROLE_DRP;
	}

	D("role_def %d\n", desc->role_def);


	if (of_property_read_u32(np, "hisi-tcpc,rp_level", &val) >= 0) {
		switch (val) {
		case 0: /* RP Default */
			desc->rp_lvl = TYPEC_CC_RP_DFT;
			break;
		case 1: /* RP 1.5V */
			desc->rp_lvl = TYPEC_CC_RP_1_5;
			break;
		case 2: /* RP 3.0V */
			desc->rp_lvl = TYPEC_CC_RP_3_0;
			break;
		default:
			break;
		}
	}

	D("rp_level %d\n", desc->rp_lvl);

	desc->name = "hisi-tcpc";
	chip->tcpc_desc = desc;

	chip->tcpc = hisi_tcpc_device_register(dev, desc, &hisi_tcpc_ops, chip);
	if (IS_ERR(chip->tcpc))
		return -EINVAL;

	hisi_usb_typec_register_tcpc_device(chip->tcpc);

	D("-\n");

	return 0;
}

static inline int hisi_tcpc_check_revision(struct i2c_client *client)
{
	u16 vid, pid, did;
	u16 data;
	int ret;

	D("+\n");

	ret = vid = hisi_tcpc_i2c_read16(client, TCPC_V10_REG_VID);
	if (ret < 0) {
		dev_err(&client->dev, "read chip ID fail\n");
		return -EIO;
	}

	D("vid 0x%04x\n", vid);

	ret = pid = hisi_tcpc_i2c_read16(client, TCPC_V10_REG_PID);
	if (ret < 0) {
		dev_err(&client->dev, "read product ID fail\n");
		return -EIO;
	}
	D("pid 0x%04x\n", pid);

	ret = did = hisi_tcpc_i2c_read16(client, TCPC_V10_REG_DID);
	if (ret < 0) {
		dev_err(&client->dev, "read device ID fail\n");
		return -EIO;
	}
	D("did 0x%04x\n", did);


	ret = data = hisi_tcpc_i2c_read16(client, TCPC_V10_REG_TYPEC_REV);
	if (ret < 0) {
		dev_err(&client->dev, "read typec revision fail\n");
		return -EIO;
	}
	D("typec rev 0x%04x\n", data);

	ret = data = hisi_tcpc_i2c_read16(client, TCPC_V10_REG_PD_REV);
	if (ret < 0) {
		dev_err(&client->dev, "read pd rev ver fail\n");
		return -EIO;
	}
	D("pd rev ver 0x%04x\n", data);

	ret = data = hisi_tcpc_i2c_read16(client, TCPC_V10_REG_PDIF_REV);
	if (ret < 0) {
		dev_err(&client->dev, "read pd interface revision fail\n");
		return -EIO;
	}
	D("pd interface rev 0x%04x\n", data);

	D("-\n");

	return ret;
}

static int hisi_tcpc_probe(struct platform_device *pdev)
{
	struct hisi_tcpc_chip *chip;
	int ret = 0;
	struct i2c_client *client;

	D("+\n");

	ret = hisi_usb_typec_register_pd_dpm();
	if (ret == -EBUSY) {
		return 0;
	}

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip) {
		E("alloc chip failed\n");
		return -ENOMEM;
	}

	hisi_tcpc_debug_init(chip);

	client = hi6526_i2c_client;
	if (!client) {
		E("no client\n");
		devm_kfree(&pdev->dev, chip);
		return -ENODEV;
	}

	chip->client = client;

	ret = hisi_tcpc_check_revision(client);
	if (ret < 0) {
		pr_err("[%s]hisi_tcpc_check_revision ret %d\n", __func__, ret);
		return ret;
	}

	ret = hisi_tcpc_parse_dt(chip, &pdev->dev);
	if (ret)
		return ret;

	chip->dev = &pdev->dev;
	platform_set_drvdata(pdev, chip);

	sema_init(&chip->suspend_lock, 1);
	INIT_DELAYED_WORK(&chip->poll_work, hisi_tcpc_poll_work);


	ret = hisi_tcpc_tcpcdev_init(chip, &pdev->dev);
	if (ret < 0) {
		dev_err(&client->dev, "hisi tcpc dev init fail\n");
		return ret;
	}


	ret = hisi_tcpc_init_alert(chip->tcpc);
	if (ret < 0) {
		pr_err("hisi tcpc init alert fail\n");
		goto err_irq_init;
	}

	hisi_usb_vbus_init(chip);
	chip->vbus_detect = true;
	chip->vbus_status = hisi_usb_vbus_status();

	/* start typec state machine! */
	hisi_tcpc_schedule_init_work(chip->tcpc);

	D("-\n");

	return 0;

err_irq_init:
	hisi_tcpc_device_unregister(chip->dev, chip->tcpc);
	return ret;
}

static int hisi_tcpc_remove(struct platform_device *pdev)
{
	struct hisi_tcpc_chip *chip = platform_get_drvdata(pdev);

	if (chip) {
		cancel_delayed_work_sync(&chip->poll_work);
		hisi_tcpc_device_unregister(chip->dev, chip->tcpc);
	}

	return 0;
}

static void hisi_tcpc_shutdown(struct platform_device *pdev)
{
	struct hisi_tcpc_chip *chip = platform_get_drvdata(pdev);
	struct tcpc_device *tcpc;

	if (!chip)
		return;

	tcpc = chip->tcpc;
	if (chip->irq)
		disable_irq(chip->irq);

	hisi_tcpm_shutdown(tcpc);
}

#ifdef CONFIG_PM
static int hisi_tcpc_suspend(struct device *dev)
{
	struct hisi_tcpc_chip *chip;
	struct i2c_client *client = to_i2c_client(dev);

	chip = i2c_get_clientdata(client);
	if (!chip)
		return 0;

	down(&chip->suspend_lock);
	enable_irq_wake(chip->irq);

	return 0;
}

static int hisi_tcpc_resume(struct device *dev)
{
	struct hisi_tcpc_chip *chip;
	struct i2c_client *client = to_i2c_client(dev);

	chip = i2c_get_clientdata(client);
	if (!chip)
		return 0;

	up(&chip->suspend_lock);
	disable_irq_wake(chip->irq);

	return 0;
}

static const struct dev_pm_ops hisi_tcpc_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(
			hisi_tcpc_suspend,
			hisi_tcpc_resume)
};
#define HISI_TCPC_PM_OPS	(&hisi_tcpc_pm_ops)
#else
#define HISI_TCPC_PM_OPS	(NULL)
#endif /* CONFIG_PM */

static const struct of_device_id hisi_tcpc_match_table[] = {
	{.compatible = "hisilicon,hisi_tcpc",},
	{},
};

static struct platform_driver hisi_tcpc_driver = {
	.driver = {
		.name = "hisi_tcpc",
		.owner = THIS_MODULE,
		.of_match_table = hisi_tcpc_match_table,
		.pm = HISI_TCPC_PM_OPS,
	},
	.probe = hisi_tcpc_probe,
	.remove = hisi_tcpc_remove,
	.shutdown = hisi_tcpc_shutdown,
};

static int __init hisi_tcpc_init(void)
{
	D("hisi_tcpc module_init\n");
	return platform_driver_register(&hisi_tcpc_driver);
}

static void __exit hisi_tcpc_exit(void)
{
	D("hisi_tcpc module_exit\n");
	platform_driver_unregister(&hisi_tcpc_driver);
}

late_initcall_sync(hisi_tcpc_init);
module_exit(hisi_tcpc_exit);

MODULE_DESCRIPTION("Hisilicon Type-C Port Controller Driver");
MODULE_LICENSE("GPL");
