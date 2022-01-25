/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/
#ifndef HISI_MIPI_DSI_H
#define HISI_MIPI_DSI_H

#include "hisi_fb.h"

/* mipi dsi panel */
enum {
    DSI_VIDEO_MODE,
    DSI_CMD_MODE,
};

enum {
    DSI_1_1_VERSION = 0,
    DSI_1_2_VERSION,
};

enum {
    DSI_1_LANES = 0,
    DSI_2_LANES,
    DSI_3_LANES,
    DSI_4_LANES,
};

enum {
    DSI_LANE_NUMS_DEFAULT = 0,
    DSI_1_LANES_SUPPORT = BIT(0),
    DSI_2_LANES_SUPPORT = BIT(1),
    DSI_3_LANES_SUPPORT = BIT(2),
    DSI_4_LANES_SUPPORT = BIT(3),
};

enum {
    DSI_16BITS_1 = 0,
    DSI_16BITS_2,
    DSI_16BITS_3,
    DSI_18BITS_1,
    DSI_18BITS_2,
    DSI_24BITS_1,
    DSI_24BITS_2,
    DSI_24BITS_3,
    DSI_DSC24_COMPRESSED_DATA = 0xF,
};

enum {
    DSI_NON_BURST_SYNC_PULSES = 0,
    DSI_NON_BURST_SYNC_EVENTS,
    DSI_BURST_SYNC_PULSES_1,
    DSI_BURST_SYNC_PULSES_2,
};

enum {
    EN_DSI_TX_NORMAL_MODE = 0x0,
    EN_DSI_TX_LOW_PRIORITY_DELAY_MODE = 0x1,
    EN_DSI_TX_HIGH_PRIORITY_DELAY_MODE = 0x2,
    EN_DSI_TX_AUTO_MODE = 0xF,
};


#define DSI_VIDEO_DST_FORMAT_RGB565			0
#define DSI_VIDEO_DST_FORMAT_RGB666			1
#define DSI_VIDEO_DST_FORMAT_RGB666_LOOSE	2
#define DSI_VIDEO_DST_FORMAT_RGB888			3

#define DSI_CMD_DST_FORMAT_RGB565	0
#define DSI_CMD_DST_FORMAT_RGB666	1
#define DSI_CMD_DST_FORMAT_RGB888	2

#define GEN_VID_LP_CMD		BIT(24)	/* vid lowpwr cmd write*/
/* dcs read/write */
#define DTYPE_DCS_WRITE		0x05	/* short write, 0 parameter */
#define DTYPE_DCS_WRITE1	0x15	/* short write, 1 parameter */
#define DTYPE_DCS_READ		0x06	/* read */
#define DTYPE_DCS_LWRITE	0x39	/* long write */
#define DTYPE_DSC_LWRITE	0x0A	/* dsc dsi1.2 vase3x long write */

/* generic read/write */
#define DTYPE_GEN_WRITE		0x03	/* short write, 0 parameter */
#define DTYPE_GEN_WRITE1	0x13    /* short write, 1 parameter */
#define DTYPE_GEN_WRITE2	0x23	/* short write, 2 parameter */
#define DTYPE_GEN_LWRITE	0x29	/* long write */
#define DTYPE_GEN_READ		0x04	/* long read, 0 parameter */
#define DTYPE_GEN_READ1		0x14	/* long read, 1 parameter */
#define DTYPE_GEN_READ2		0x24	/* long read, 2 parameter */

#define DTYPE_TEAR_ON		0x35	/* set tear on */
#define DTYPE_MAX_PKTSIZE	0x37	/* set max packet size */
#define DTYPE_NULL_PKT		0x09	/* null packet, no data */
#define DTYPE_BLANK_PKT		0x19	/* blankiing packet, no data */

#define DTYPE_CM_ON				0x02	/* color mode off */
#define DTYPE_CM_OFF			0x12	/* color mode on */
#define DTYPE_PERIPHERAL_OFF	0x22
#define DTYPE_PERIPHERAL_ON		0x32

#define DSI_HDR_DTYPE(dtype)	((dtype) & 0x03f)
#define DSI_HDR_VC(vc)			(((vc) & 0x03) << 6)
#define DSI_HDR_DATA1(data)		(((data) & 0x0ff) << 8)
#define DSI_HDR_DATA2(data)		(((data) & 0x0ff) << 16)
#define DSI_HDR_WC(wc)			(((wc) & 0x0ffff) << 8)

#define DSI_PLD_DATA1(data)		((data) & 0x0ff)
#define DSI_PLD_DATA2(data)		(((data) & 0x0ff) << 8)
#define DSI_PLD_DATA3(data)		(((data) & 0x0ff) << 16)
#define DSI_PLD_DATA4(data)		(((data) & 0x0ff) << 24)

struct dsi_cmd_desc {
	uint32_t dtype;
	uint32_t vc;
	uint32_t wait;
	uint32_t waittype;
	uint32_t dlen;
	char *payload;
};

struct mipi_dsi_read_compare_data {
	uint32_t *read_value;
	uint32_t *expected_value;
	uint32_t *read_mask;
	char **reg_name;
	int log_on;
	struct dsi_cmd_desc *cmds;
	int cnt;
};

#define DEFAULT_MAX_TX_ESC_CLK	(10 * 1000000UL)
//#define DEFAULT_DSI_BURST_MODE	DSI_BURST_SYNC_PULSES_1

#define DEFAULT_MIPI_CLK_RATE	(192 * 100000L)
#define DEFAULT_PCLK_DSI_RATE	(120 * 1000000L)

struct dsi_phy_seq_info {
	uint32_t min_range;
	uint32_t max_range;
	uint32_t rg_pll_vco_750M;
	uint32_t rg_hstx_ckg_sel;
};

#define MAX_CMD_QUEUE_LOW_PRIORITY_SIZE    256
#define MAX_CMD_QUEUE_HIGH_PRIORITY_SIZE   128
#define CMD_AUTO_MODE_THRESHOLD                (2/3)
struct dsi_delayed_cmd_queue {
	struct semaphore work_queue_sem;
	spinlock_t CmdSend_lock;
	struct dsi_cmd_desc CmdQueue_LowPriority[MAX_CMD_QUEUE_LOW_PRIORITY_SIZE];
	uint32_t CmdQueue_LowPriority_rd;
	uint32_t CmdQueue_LowPriority_wr;
	bool isCmdQueue_LowPriority_full;
	bool isCmdQueue_LowPriority_working;
	spinlock_t CmdQueue_LowPriority_lock;

	struct dsi_cmd_desc CmdQueue_HighPriority[MAX_CMD_QUEUE_HIGH_PRIORITY_SIZE];
	uint32_t CmdQueue_HighPriority_rd;
	uint32_t CmdQueue_HighPriority_wr;
	bool isCmdQueue_HighPriority_full;
	bool isCmdQueue_HighPriority_working;
	spinlock_t CmdQueue_HighPriority_lock;

	s64 timestamp_frame_start;
	s64 oneframe_time;
};

/******************************************************************************
** FUNCTIONS PROTOTYPES
*/
int mipi_dsi_on(struct platform_device *pdev);
int mipi_dsi_set_fastboot(struct platform_device *pdev);
int mipi_dsi_off(struct platform_device *pdev);
void mipi_dsi_max_return_packet_size(struct dsi_cmd_desc *cm,
	char __iomem *dsi_base);
void mipi_dsi_sread(uint32_t *out, const char __iomem *dsi_base);
void mipi_dsi_lread(uint32_t *out, char __iomem *dsi_base);
uint32_t mipi_dsi_read(uint32_t *out, const char __iomem *dsi_base);
int mipi_dsi_swrite(struct dsi_cmd_desc *cm, char __iomem *dsi_base);
int mipi_dsi_lwrite(struct dsi_cmd_desc *cm, char __iomem *dsi_base);
void mipi_dsi_check_0lane_is_ready(const char __iomem *dsi_base);
int mipi_dsi_cmds_tx(struct dsi_cmd_desc *cmds, int cnt,
	char __iomem *dsi_base);
int mipi_dsi_cmds_rx(uint32_t *out, struct dsi_cmd_desc *cmds, int cnt,
	char __iomem *dsi_base);

int mipi_dsi_read_compare(struct mipi_dsi_read_compare_data *data,
	char __iomem *dsi_base);
int mipi_dsi_lread_reg(uint32_t *out, struct dsi_cmd_desc *cm,
	uint32_t len, char *dsi_base);
int mipi_dsi_cmd_is_read(struct dsi_cmd_desc *cm);

struct hisi_fb_data_type;
int mipi_dsi_clk_enable(struct hisi_fb_data_type *hisifd);
int mipi_dsi_clk_disable(struct hisi_fb_data_type *hisifd);
int mipi_dsi_bit_clk_upt_isr_handler(struct hisi_fb_data_type *hisifd);
void mipi_dsi_reset(struct hisi_fb_data_type *hisifd);
void mipi_init(struct hisi_fb_data_type *hisifd, char __iomem *mipi_dsi_base);

int mipi_dsi_cmds_tx_with_check_fifo(struct dsi_cmd_desc *cmds, int cnt, char __iomem *dsi_base);
int mipi_dsi_cmds_rx_with_check_fifo(uint32_t *out, struct dsi_cmd_desc *cmds, int cnt, char __iomem *dsi_base);

//dual-dsi parallel interface
/*
tx_mode: select different tx motheds.
    -EN_DSI_TX_NORMAL_MODE: send cmds right now, then return.
    -EN_DSI_TX_LOW_PRIORITY_DELAY_MODE: store cmds to a queue, then return.
          The cmds in queue will be sent after next frame start(vsync)
    -EN_DSI_TX_HIGH_PRIORITY_DELAY_MODE: store cmds to a queue, then return.
          The cmds in queue will be sent after next frame start(vsync) which is prior to be sent than EN_DSI_TX_LOW_PRIORITY_DELAY_MODE
    -EN_DSI_TX_AUTO_MODE: check the timestamp first,
           if the left time to the next frame start(vsync)  is less than a threshold, use EN_DSI_TX_LOW_PRIORITY_DELAY_MODE,
           otherwise use EN_DSI_TX_NORMAL_MODE
*/
int  mipi_dual_dsi_cmds_tx( struct dsi_cmd_desc *pCmdset_0, int Cmdset_cnt_0, char __iomem * dsi_base_0,
						   struct dsi_cmd_desc *pCmdset_1, int Cmdset_cnt_1, char __iomem * dsi_base_1, uint8_t tx_mode );
int mipi_dual_dsi_cmds_rx( uint32_t *ValueOut_0, struct dsi_cmd_desc *pCmdset_0, int Cmdset_cnt_0, char __iomem * dsi_base_0,
					    uint32_t *ValueOut_1, struct dsi_cmd_desc *pCmdset_1, int Cmdset_cnt_1, char __iomem * dsi_base_1 );
int mipi_dual_dsi_lread_reg( uint32_t *ValueOut_0, uint32_t *ValueOut_1, struct dsi_cmd_desc *pCmd, uint32_t dlen, char __iomem * dsi_base_0, char __iomem * dsi_base_1);

void mipi_dsi_init_delayed_cmd_queue(void);
void mipi_dsi_delayed_cmd_queue_handle_func(struct work_struct *work);
void mipi_dsi_set_timestamp(void);
bool mipi_dsi_check_delayed_cmd_queue_working(void);
#endif  /* HISI_MIPI_DSI_H */
