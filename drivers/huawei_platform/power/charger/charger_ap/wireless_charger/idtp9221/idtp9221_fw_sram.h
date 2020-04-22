/*
 * idtp9221_fw_sram.h
 *
 * idtp9221 otp sram_fw header
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _IDTP9221_FW_SRAM_H_
#define _IDTP9221_FW_SRAM_H_

#include <idtp9221_fw_sram_020b.h>
#include <idtp9221_rxfw_sram_0318.h>
#include <idtp9221_txfw_sram_031a.h>

#include <stwlc33_rxfw_sram_1100.h>
#include <stwlc33_txfw_sram_1100.h>

#include <huawei_platform/power/wireless_charger.h>

struct fw_update_info {
	const enum wireless_mode fw_sram_mode; /* TX_SRAM or RX_SRAM */
	const char *name_fw_update_from; /* from which OTP firmware version */
	const char *name_fw_update_to; /* to which OTP firmware version */
	const unsigned char *fw_sram; /* SRAM */
	const unsigned int fw_sram_size;
	const u16 fw_sram_update_addr;
};

const struct fw_update_info fw_update[] = {
	{
		.fw_sram_mode        = WIRELESS_RX_MODE,
		.name_fw_update_from = IDT9221_OTP_FW_VERSION_020BH,
		.name_fw_update_to   = IDT9221_OTP_FW_VERSION_020BH,
		.fw_sram             = idt_fw_sram_020bh,
		.fw_sram_size        = ARRAY_SIZE(idt_fw_sram_020bh),
		.fw_sram_update_addr = IDT9221_RX_SRAMUPDATE_ADDR,
	},
	{
		.fw_sram_mode        = WIRELESS_RX_MODE,
		.name_fw_update_from = IDT9221_OTP_FW_VERSION_030FH,
		.name_fw_update_to   = IDT9221_OTP_FW_VERSION_030FH,
		.fw_sram             = idt_rxfw_sram_0318h,
		.fw_sram_size        = ARRAY_SIZE(idt_rxfw_sram_0318h),
		.fw_sram_update_addr = IDT9221_RX_SRAMUPDATE_ADDR,
	},
	{
		.fw_sram_mode        = WIRELESS_TX_MODE,
		.name_fw_update_from = IDT9221_OTP_FW_VERSION_030FH,
		.name_fw_update_to   = IDT9221_OTP_FW_VERSION_030FH,
		.fw_sram             = idt_txfw_sram_031ah,
		.fw_sram_size        = ARRAY_SIZE(idt_txfw_sram_031ah),
		.fw_sram_update_addr = IDT9221_TX_SRAMUPDATE_ADDR,
	},
	{
		.fw_sram_mode        = WIRELESS_RX_MODE,
		.name_fw_update_from = STWLC33_OTP_FW_VERSION_1100H,
		.name_fw_update_to   = STWLC33_OTP_FW_VERSION_1100H,
		.fw_sram             = st_rxfw_sram_1100h,
		.fw_sram_size        = ARRAY_SIZE(st_rxfw_sram_1100h),
		.fw_sram_update_addr = STWLC33_RX_SRAMUPDATE_ADDR,
	},
	{
		.fw_sram_mode        = WIRELESS_TX_MODE,
		.name_fw_update_from = STWLC33_OTP_FW_VERSION_1100H,
		.name_fw_update_to   = STWLC33_OTP_FW_VERSION_1100H,
		.fw_sram             = st_txfw_sram_1100h,
		.fw_sram_size        = ARRAY_SIZE(st_txfw_sram_1100h),
		.fw_sram_update_addr = STWLC33_TX_SRAMUPDATE_ADDR,
	},
};

#endif /* _IDTP9221_FW_SRAM_H_ */
