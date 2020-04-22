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

#include "hisi_mipi_dsi.h"

//global definition for the cmd queue which will be send after next vactive start
static struct dsi_delayed_cmd_queue g_delayed_cmd_queue;
static bool g_delayed_cmd_queue_inited = false;

static void mipi_dsi_sread_request(struct dsi_cmd_desc *cm, char __iomem *dsi_base);

static int _mipi_dsi_delayed_cmd_queue_write(struct dsi_cmd_desc *pCmdset, int Cmdset_cnt, bool isLowPriority);

/*lint -e455 -e454 -e456*/
static inline void _mipi_dsi_cmd_send_lock(void)
{
	if (g_delayed_cmd_queue_inited) {
		spin_lock(&g_delayed_cmd_queue.CmdSend_lock);
	}
}

static inline void _mipi_dsi_cmd_send_unlock(void)
{
	if (g_delayed_cmd_queue_inited) {
		spin_unlock(&g_delayed_cmd_queue.CmdSend_lock);
	}
}
/*lint +e455 +e454 +e456 */

/*
 * mipi dsi short write with 0, 1 2 parameters
 * Write to GEN_HDR 24 bit register the value:
 * 1. 00h, MCS_command[15:8] ,VC[7:6],13h
 * 2. Data1[23:16], MCS_command[15:8] ,VC[7:6],23h
 */
int mipi_dsi_swrite(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	uint32_t hdr = 0;
	int len = 0;

	if (cm->dlen && cm->payload == 0) {
		HISI_FB_ERR("NO payload error!\n");
		return 0;
	}

	if (cm->dlen > 2) {
		HISI_FB_ERR("cm->dlen is invalid");
		return -EINVAL;
	}
	len = cm->dlen;

	//len = (cm->dlen > 2) ? 2 : cm->dlen;

	hdr |= DSI_HDR_DTYPE(cm->dtype);
	hdr |= DSI_HDR_VC(cm->vc);
	if (len == 1) {
		hdr |= DSI_HDR_DATA1(cm->payload[0]);
		hdr |= DSI_HDR_DATA2(0);
	} else if (len == 2) {
		hdr |= DSI_HDR_DATA1(cm->payload[0]);
		hdr |= DSI_HDR_DATA2(cm->payload[1]);
	} else {
		hdr |= DSI_HDR_DATA1(0);
		hdr |= DSI_HDR_DATA2(0);
	}

	set_reg(dsi_base + MIPIDSI_GEN_HDR_OFFSET, hdr, 24, 0);

	HISI_FB_DEBUG("hdr=0x%x!\n", hdr);
	return len;  /* 4 bytes */
}

/*
 * mipi dsi long write
 * Write to GEN_PLD_DATA 32 bit register the value:
 * Data3[31:24], Data2[23:16], Data1[15:8], MCS_command[7:0]
 * If need write again to GEN_PLD_DATA 32 bit register the value:
 * Data7[31:24], Data6[23:16], Data5[15:8], Data4[7:0]
 *
 * Write to GEN_HDR 24 bit register the value: WC[23:8] ,VC[7:6],29h
 */
/*lint -e574*/
int mipi_dsi_lwrite(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	uint32_t hdr = 0;
	int i = 0;
	int j = 0;
	uint32_t pld = 0;

	if (cm->dlen && cm->payload == 0) {
		HISI_FB_ERR("NO payload error!\n");
		return 0;
	}

	/* fill up payload */
	for (i = 0;  i < cm->dlen; i += 4) {
		if ((i + 4) <= cm->dlen) {
			pld = *((uint32_t *)(cm->payload + i));
		} else {
			for (j = i; j < cm->dlen; j++) {
				pld |= ((uint32_t)(cm->payload[j] & 0x0ff) << ((j - i) * 8));
			}
			HISI_FB_DEBUG("pld=0x%x!\n", pld);
		}

		set_reg(dsi_base + MIPIDSI_GEN_PLD_DATA_OFFSET, pld, 32, 0);
		pld = 0;
	}

	/* fill up header */
	hdr |= DSI_HDR_DTYPE(cm->dtype);
	hdr |= DSI_HDR_VC(cm->vc);
	hdr |= DSI_HDR_WC(cm->dlen);

	set_reg(dsi_base + MIPIDSI_GEN_HDR_OFFSET, hdr, 24, 0);

	HISI_FB_DEBUG("hdr=0x%x!\n", hdr);
	return cm->dlen;
}
/*lint +e574*/
void mipi_dsi_max_return_packet_size(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	uint32_t hdr = 0;

	/* fill up header */
	hdr |= DSI_HDR_DTYPE(cm->dtype);
	hdr |= DSI_HDR_VC(cm->vc);
	hdr |= DSI_HDR_WC(cm->dlen);
	set_reg(dsi_base + MIPIDSI_GEN_HDR_OFFSET, hdr, 24, 0);
}

uint32_t mipi_dsi_read(uint32_t *out, const char __iomem *dsi_base)
{
	uint32_t pkg_status;
	uint32_t try_times = 700;

	do {
		pkg_status = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		if (!(pkg_status & 0x10))
			break;
		udelay(50);
	} while (--try_times);

	*out = inp32(dsi_base + MIPIDSI_GEN_PLD_DATA_OFFSET);
	if (!try_times)
		HISI_FB_ERR("mipi_dsi_read timeout,CMD_PKT_STATUS[0x%x],PHY_STATUS[0x%x],INT_ST0[0x%x],INT_ST1[0x%x]. \n",
			inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET), inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base + MIPIDSI_INT_ST0_OFFSET), inp32(dsi_base + MIPIDSI_INT_ST1_OFFSET));

	return try_times;
}

void mipi_dsi_sread(uint32_t *out, const char __iomem *dsi_base)
{
	unsigned long dw_jiffies = 0;
	uint32_t tmp = 0;

	dw_jiffies = jiffies + HZ / 2;
	do {
		tmp = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		if ((tmp & 0x00000040) == 0x00000040) {
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	dw_jiffies = jiffies + HZ / 2;
	do {
		tmp = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		if ((tmp & 0x00000040) != 0x00000040) {
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	*out = inp32(dsi_base + MIPIDSI_GEN_PLD_DATA_OFFSET);
}

void mipi_dsi_lread(uint32_t *out, char __iomem *dsi_base)
{
	/* do something here*/
}

int mipi_dsi_cmd_is_read(struct dsi_cmd_desc *cm)
{
	int ret = 0;

	switch (DSI_HDR_DTYPE(cm->dtype)) {
		case DTYPE_GEN_READ:
		case DTYPE_GEN_READ1:
		case DTYPE_GEN_READ2:
		case DTYPE_DCS_READ:
			ret = 1;
			break;
		default:
			ret = 0;
			break;
	}
	return ret;
}

int mipi_dsi_lread_reg(uint32_t *out, struct dsi_cmd_desc *cm, uint32_t len, char *dsi_base)
{
	int ret = 0;
	int i = 0;
	struct dsi_cmd_desc packet_size_cmd_set;

	if (cm == NULL) {
		HISI_FB_ERR("cmds is NULL!\n");
		return -1;
	}
	if (dsi_base == NULL) {
		HISI_FB_ERR("dsi_base is NULL!\n");
		return -1;
	}

	if (mipi_dsi_cmd_is_read(cm)) {
		packet_size_cmd_set.dtype = DTYPE_MAX_PKTSIZE;
		packet_size_cmd_set.vc = 0;
		packet_size_cmd_set.dlen = len;
		mipi_dsi_max_return_packet_size(&packet_size_cmd_set, dsi_base);
		mipi_dsi_sread_request(cm, dsi_base);
		for (i = 0; i < (len + 3)/4; i++) {
			if (!mipi_dsi_read(out, dsi_base)) {
				ret = -1;
				HISI_FB_ERR("Read register 0x%X timeout\n", cm->payload[0]);
				break;
			}
			out++;
		}
	} else {
		ret = -1;
		HISI_FB_ERR("dtype=%x NOT supported!\n", cm->dtype);
	}

	return ret;
}

/*
 * prepare cmd buffer to be txed
 */
int mipi_dsi_cmd_add(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	int len = 0;

	if (NULL == cm) {
		HISI_FB_ERR("cm is NULL");
		return -EINVAL;
	}
	if (NULL == dsi_base) {
		HISI_FB_ERR("dsi_base is NULL");
		return -EINVAL;
	}

	_mipi_dsi_cmd_send_lock();

	switch (DSI_HDR_DTYPE(cm->dtype)) {
	case DTYPE_GEN_WRITE:
	case DTYPE_GEN_WRITE1:
	case DTYPE_GEN_WRITE2:

	case DTYPE_DCS_WRITE:
	case DTYPE_DCS_WRITE1:
		len = mipi_dsi_swrite(cm, dsi_base);
		break;
	case DTYPE_GEN_LWRITE:
	case DTYPE_DCS_LWRITE:
	case DTYPE_DSC_LWRITE:

		len = mipi_dsi_lwrite(cm, dsi_base);
		break;
	default:
		HISI_FB_ERR("dtype=%x NOT supported!\n", cm->dtype);
		break;
	}

	_mipi_dsi_cmd_send_unlock();

	return len;
}

int mipi_dsi_cmds_tx(struct dsi_cmd_desc *cmds, int cnt, char __iomem *dsi_base)
{
	struct dsi_cmd_desc *cm = NULL;
	int i = 0;

	if (NULL == cmds) {
		HISI_FB_ERR("cmds is NULL");
		return -EINVAL;
	}
	if (NULL == dsi_base) {
		HISI_FB_ERR("dsi_base is NULL");
		return -EINVAL;
	}

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		mipi_dsi_cmd_add(cm, dsi_base);

		if (cm->wait) {
			if (cm->waittype == WAIT_TYPE_US)
				udelay(cm->wait);
			else if (cm->waittype == WAIT_TYPE_MS) {
				if (cm->wait <= 10) {
					mdelay((unsigned long)cm->wait);
				} else {
					msleep(cm->wait);
				}
			}
			else
				msleep(cm->wait * 1000);
		}
		cm++;
	}

	return cnt;
}

void mipi_dsi_check_0lane_is_ready(const char __iomem *dsi_base)
{
	unsigned long dw_jiffies = 0;
	uint32_t tmp = 0;

	dw_jiffies = jiffies + HZ / 10;
	do {
		tmp = inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		//phy_stopstate0lane
		if ((tmp & 0x10) == 0x10) {
			HISI_FB_INFO("0 lane is stopping state");
			return;
		}
	} while (time_after(dw_jiffies, jiffies));

	HISI_FB_ERR("0 lane is not stopping state:tmp=0x%x", tmp);
}

static void mipi_dsi_sread_request(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	uint32_t hdr = 0;

	/* fill up header */
	hdr |= DSI_HDR_DTYPE(cm->dtype);
	hdr |= DSI_HDR_VC(cm->vc);
	hdr |= DSI_HDR_DATA1(cm->payload[0]);
	hdr |= DSI_HDR_DATA2(0);
	set_reg(dsi_base + MIPIDSI_GEN_HDR_OFFSET, hdr, 24, 0);
}

static int mipi_dsi_read_add(uint32_t *out, struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	unsigned long dw_jiffies = 0;
	uint32_t pkg_status = 0;
	uint32_t phy_status = 0;
	int is_timeout = 1;
	int ret = 0;

	if (NULL == cm) {
		HISI_FB_ERR("cm is NULL");
		return -EINVAL;
	}
	if (NULL == dsi_base) {
		HISI_FB_ERR("dsi_base is NULL");
		return -EINVAL;
	}



	if (DSI_HDR_DTYPE(cm->dtype) == DTYPE_DCS_READ) {
		_mipi_dsi_cmd_send_lock();

		mipi_dsi_sread_request(cm, dsi_base);

		if (!mipi_dsi_read(out, dsi_base)) {
			HISI_FB_ERR("Read register 0x%X timeout\n",cm->payload[0]);
			ret = -1;
		}

		_mipi_dsi_cmd_send_unlock();
	} else if (cm->dtype == DTYPE_GEN_READ1) {

		/*read status register*/
		dw_jiffies = jiffies + HZ;
		do {
			pkg_status = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			phy_status = inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
			if ((pkg_status & 0x1) == 0x1 && !(phy_status & 0x2)){
				is_timeout = 0;
				break;
			}
		} while (time_after(dw_jiffies, jiffies));

		if (is_timeout) {
			HISI_FB_ERR("mipi_dsi_read timeout :0x%x \n \
				MIPIDSI_CMD_PKT_STATUS = 0x%x \n \
				MIPIDSI_PHY_STATUS = 0x%x \n \
				MIPIDSI_INT_ST1_OFFSET = 0x%x \n",
				cm->payload[0],
				inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET),
				inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET),
				inp32(dsi_base + MIPIDSI_INT_ST1_OFFSET));
			return -1;
		}

		_mipi_dsi_cmd_send_lock();
		/*send read cmd to fifo*/
		set_reg(dsi_base + MIPIDSI_GEN_HDR_OFFSET, ((cm->payload[0] << 8) | cm->dtype), 24, 0);

		is_timeout = 1;
		/*wait dsi read data*/
		dw_jiffies = jiffies + HZ;
		do {
			pkg_status = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			if (!(pkg_status & 0x10)) {
				is_timeout = 0;
				break;
			}
		} while (time_after(dw_jiffies, jiffies));

		if (is_timeout) {
			HISI_FB_ERR("mipi_dsi_read timeout :0x%x \n \
				MIPIDSI_CMD_PKT_STATUS = 0x%x \n \
				MIPIDSI_PHY_STATUS = 0x%x \n",
				cm->payload[0],
				inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET),
				inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET));
			_mipi_dsi_cmd_send_unlock();
			return -1;
		}
		/*get read data*/
		*out = inp32(dsi_base + MIPIDSI_GEN_PLD_DATA_OFFSET);

		_mipi_dsi_cmd_send_unlock();
	} else {
		ret = -1;
		HISI_FB_ERR("dtype=%x NOT supported!\n", cm->dtype);
	}

	return ret;
}

int mipi_dsi_cmds_rx(uint32_t *out, struct dsi_cmd_desc *cmds, int cnt,
	char __iomem *dsi_base)
{
	struct dsi_cmd_desc *cm = NULL;
	int i = 0;
	int err_num = 0;

	if (NULL == cmds) {
		HISI_FB_ERR("cmds is NULL");
		return -EINVAL;
	}
	if (NULL == dsi_base) {
		HISI_FB_ERR("dsi_base is NULL");
		return -EINVAL;
	}

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if(mipi_dsi_read_add(&(out[i]), cm, dsi_base)){
			err_num++;
		}

		if (cm->wait) {
			if (cm->waittype == WAIT_TYPE_US)
				udelay(cm->wait);
			else if (cm->waittype == WAIT_TYPE_MS) {
				if (cm->wait <= 10) {
					mdelay((unsigned long)cm->wait);
				} else {
					msleep(cm->wait);
				}
			}
			else
				msleep(cm->wait * 1000);
		}
		cm++;
	}

	return err_num;
}

int mipi_dsi_read_compare(struct mipi_dsi_read_compare_data *data,
	char __iomem *dsi_base)
{
	uint32_t *read_value = NULL;
	uint32_t *expected_value = NULL;
	uint32_t *read_mask = NULL;
	char **reg_name = NULL;
	int log_on = 0;
	struct dsi_cmd_desc *cmds = NULL;

	int cnt = 0;
	int cnt_not_match = 0;
	int ret = 0;
	int i;

	if (NULL == data) {
		HISI_FB_ERR("data is NULL");
		return -EINVAL;
	}
	if (NULL == dsi_base) {
		HISI_FB_ERR("dsi_base is NULL");
		return -EINVAL;
	}

	read_value = data->read_value;
	expected_value = data->expected_value;
	read_mask = data->read_mask;
	reg_name = data->reg_name;
	log_on = data->log_on;

	cmds = data->cmds;
	cnt = data->cnt;

	ret = mipi_dsi_cmds_rx(read_value, cmds, cnt, dsi_base);
	if (ret) {
		HISI_FB_ERR("Read error number: %d\n", ret);
		return cnt;
	}

	for (i = 0; i < cnt; i++) {
		if (log_on) {
			HISI_FB_INFO("Read reg %s: 0x%x, value = 0x%x\n",
				reg_name[i], cmds[i].payload[0], read_value[i]);
		}

		if (expected_value[i] != (read_value[i] & read_mask[i])) {
			cnt_not_match++;
		}
	}

	return cnt_not_match;
}

static int _mipi_dsi_fifo_is_full(const char __iomem * dsi_base)
{
	uint32_t pkg_status = 0;
	uint32_t phy_status = 0;
	int is_timeout = 100;

	if (NULL == dsi_base) {
		HISI_FB_ERR("dsi_base is NULL");
		return -EINVAL;
	}

	/*read status register*/
	do {
		pkg_status = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		phy_status = inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		if ((pkg_status & 0x2) != 0x2 && !(phy_status & 0x2)) {
			break;
		}
		udelay(100);
	} while (is_timeout-- > 0);

	if (!is_timeout) {
		HISI_FB_ERR("mipi check full fail: \n \
						MIPIDSI_CMD_PKT_STATUS = 0x%x \n \
						MIPIDSI_PHY_STATUS = 0x%x \n \
						MIPIDSI_INT_ST1_OFFSET = 0x%x \n",
					inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET),
					inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET),
					inp32(dsi_base + MIPIDSI_INT_ST1_OFFSET));
		return -1;
	}
	return 0;
}

int mipi_dsi_cmds_tx_with_check_fifo(struct dsi_cmd_desc *cmds, int cnt, char __iomem *dsi_base)
{
	struct dsi_cmd_desc *cm = NULL;
	int i = 0;

	if (NULL == cmds) {
		HISI_FB_ERR("cmds is NULL");
		return -EINVAL;
	}
	if (NULL == dsi_base) {
		HISI_FB_ERR("dsi_base is NULL");
		return -EINVAL;
	}

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if (!_mipi_dsi_fifo_is_full(dsi_base)) {
			mipi_dsi_cmd_add(cm, dsi_base);
		} else {
			HISI_FB_ERR("dsi fifo full, write (%d) cmds, left (%d) cmds!!", i, cnt-i);
			break;
		}

		if (cm->wait) {
			if (cm->waittype == WAIT_TYPE_US)
				udelay(cm->wait);
			else if (cm->waittype == WAIT_TYPE_MS) {
				if (cm->wait <= 10) {
					mdelay((unsigned long)cm->wait);
				} else {
					msleep(cm->wait);
				}
			}
			else
				msleep(cm->wait * 1000);
		}
		cm++;
	}

	return cnt;
}

int mipi_dsi_cmds_rx_with_check_fifo(uint32_t *out, struct dsi_cmd_desc *cmds, int cnt, char __iomem *dsi_base)
{
	struct dsi_cmd_desc *cm = NULL;
	int i = 0;
	int err_num = 0;

	if (NULL == cmds) {
		HISI_FB_ERR("cmds is NULL");
		return -EINVAL;
	}
	if (NULL == dsi_base) {
		HISI_FB_ERR("dsi_base is NULL");
		return -EINVAL;
	}

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if (!_mipi_dsi_fifo_is_full(dsi_base)) {
			if(mipi_dsi_read_add(&(out[i]), cm, dsi_base)){
				err_num++;
			}
		} else {
			err_num += cnt-i;
			HISI_FB_ERR("dsi fifo full, read (%d) cmds, left (%d) cmds!!", i, cnt-i);
			break;
		}

		if (cm->wait) {
			if (cm->waittype == WAIT_TYPE_US)
				udelay(cm->wait);
			else if (cm->waittype == WAIT_TYPE_MS) {
				if (cm->wait <= 10) {
					mdelay((unsigned long)cm->wait);
				} else {
					msleep(cm->wait);
				}
			}
			else
				msleep(cm->wait * 1000);
		}
		cm++;
	}

	return err_num;
}

static void _delay_for_next_cmd_by_sleep(uint32_t wait, uint32_t waittype)
{
	if (wait) {
		HISI_FB_DEBUG("waittype=%d, wait=%d.", waittype, wait);
		if (waittype == WAIT_TYPE_US) {
			udelay(wait);
		} else if (waittype == WAIT_TYPE_MS) {
			if (wait <= 10) {
				mdelay((unsigned long)wait);
			} else {
				msleep(wait);
			}
		} else {
			msleep(wait * 1000);
		}
	}
}

static void _delay_by_msleep(uint32_t usec)
{
	uint32_t msec;

	if (usec > 1000) {
		msec = usec / 1000;
		usec = usec - msec * 1000;
		//mdelay(msec);
		if (msec <= 10)
			mdelay((unsigned long)msec);
		else
			msleep(msec);
		if (usec)
			udelay(usec);
	} else if (usec) {
		udelay(usec);
	}
}

static uint32_t _calc_next_wait_time(uint32_t wait, uint32_t waittype)
{
	if (wait) {
		if (waittype == WAIT_TYPE_US)
			return wait;
		else if (waittype == WAIT_TYPE_MS)
			return wait *1000;
		else
			return wait * 1000 * 1000;
	}
	return 0;
}

static int _mipi_dual_dsi_read(uint32_t *ValueOut_0, uint32_t *ValueOut_1, const char __iomem * dsi_base_0, char __iomem * dsi_base_1)
{
	uint32_t pkg_status_0, pkg_status_1;
	uint32_t try_times = 700;
	bool read_done_0 = false, read_done_1 = false;
	int ret = 2;

	if ((NULL == dsi_base_0) || (NULL == dsi_base_1)) {
		HISI_FB_ERR("dsi_base is NULL");
		return 0;
	}
	if ((NULL == ValueOut_0) || (NULL == ValueOut_1)) {
		HISI_FB_ERR("out buffer is NULL");
		return 0;
	}

	do {
		if(!read_done_0) {
			pkg_status_0 = inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			if (!(pkg_status_0 & 0x10)) {
				read_done_0 = true;
				*ValueOut_0 = inp32(dsi_base_0 + MIPIDSI_GEN_PLD_DATA_OFFSET);
			}
		}
		if(!read_done_1) {
			pkg_status_1 = inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			if (!(pkg_status_1 & 0x10)) {
				read_done_1 = true;
				*ValueOut_1 = inp32(dsi_base_1 + MIPIDSI_GEN_PLD_DATA_OFFSET);
			}
		}

		if (read_done_0 && read_done_1)
			break;
		udelay(50);
	} while (--try_times);

	if (!read_done_0) {
		ret -= 1;
		HISI_FB_ERR("_mipi_dual_dsi_read dsi0 timeout,CMD_PKT_STATUS[0x%x],PHY_STATUS[0x%x],INT_ST0[0x%x],INT_ST1[0x%x]. \n",
			inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET), inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base_0 + MIPIDSI_INT_ST0_OFFSET), inp32(dsi_base_0 + MIPIDSI_INT_ST1_OFFSET));
	}
	if (!read_done_1) {
		ret -= 1;
		HISI_FB_ERR("_mipi_dual_dsi_read dsi1 timeout,CMD_PKT_STATUS[0x%x],PHY_STATUS[0x%x],INT_ST0[0x%x],INT_ST1[0x%x]. \n",
			inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET), inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base_1 + MIPIDSI_INT_ST0_OFFSET), inp32(dsi_base_1 + MIPIDSI_INT_ST1_OFFSET));
	}

	return ret;
}

//just for Cyclomatic Complexity, no need to check input param
static inline int _mipi_dual_dsi_read_add_send(struct dsi_cmd_desc *pCmd, char __iomem * dsi_base_0, char __iomem * dsi_base_1, int *is_timeout_0, int *is_timeout_1)
{
	unsigned long dw_jiffies = 0;
	uint32_t pkg_status_0 = 0, pkg_status_1 = 0;
	uint32_t phy_status_0 = 0, phy_status_1 = 0;
	int ret = 2;

	/*read status register*/
	dw_jiffies = jiffies + HZ;
	do {
		if (*is_timeout_0) {
			pkg_status_0 = inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			phy_status_0 = inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET);
			if ((pkg_status_0 & 0x1) == 0x1 && !(phy_status_0 & 0x2)){
				*is_timeout_0 = 0;
				/*send read cmd to fifo*/
				set_reg(dsi_base_0 + MIPIDSI_GEN_HDR_OFFSET, ((pCmd->payload[0] << 8) | pCmd->dtype), 24, 0);
			}
		}

		if (*is_timeout_1) {
			pkg_status_1 = inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			phy_status_1 = inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET);
			if ((pkg_status_1 & 0x1) == 0x1 && !(phy_status_1 & 0x2)){
				*is_timeout_1 = 0;
				/*send read cmd to fifo*/
				set_reg(dsi_base_1 + MIPIDSI_GEN_HDR_OFFSET, ((pCmd->payload[0] << 8) | pCmd->dtype), 24, 0);
			}
		}

		if ( (!(*is_timeout_0)) && (!(*is_timeout_1)) )	//if both done, break
			break;
	} while (time_after(dw_jiffies, jiffies));

	if (*is_timeout_0) {
		HISI_FB_ERR("_mipi_dual_dsi_read_add dsi0 tx timeout :0x%x \n \
			MIPIDSI_CMD_PKT_STATUS = 0x%x \n \
			MIPIDSI_PHY_STATUS = 0x%x \n \
			MIPIDSI_INT_ST1_OFFSET = 0x%x \n",
			pCmd->payload[0],
			inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base_0 + MIPIDSI_INT_ST1_OFFSET));
		ret -= 1;
	}
	if (*is_timeout_1) {
		HISI_FB_ERR("_mipi_dual_dsi_read_add dsi1 tx timeout :0x%x \n \
			MIPIDSI_CMD_PKT_STATUS = 0x%x \n \
			MIPIDSI_PHY_STATUS = 0x%x \n \
			MIPIDSI_INT_ST1_OFFSET = 0x%x \n",
			pCmd->payload[0],
			inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base_1 + MIPIDSI_INT_ST1_OFFSET));

		ret -= 1;
	}

	return ret;
}

//just for Cyclomatic Complexity, no need to check input param
static inline int _mipi_dsi_read_add_receive(uint32_t *ValueOut, struct dsi_cmd_desc *pCmd, const char __iomem * dsi_base)
{
	unsigned long dw_jiffies = 0;
	uint32_t pkg_status = 0;
	bool read_done = false;

	/*wait dsi read data*/
	dw_jiffies = jiffies + HZ;
	do {
		pkg_status = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		if (!(pkg_status & 0x10)) {
			read_done = true;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	if (!read_done) {
		HISI_FB_ERR("mipi_dsi_read timeout :0x%x \n \
			MIPIDSI_CMD_PKT_STATUS = 0x%x \n \
			MIPIDSI_PHY_STATUS = 0x%x \n",
			pCmd->payload[0],
			inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET));
		return 0;
	}
	/*get read data*/
	*ValueOut = inp32(dsi_base + MIPIDSI_GEN_PLD_DATA_OFFSET);
	return 1;
}
static inline int _mipi_dual_dsi_read_add_receive(uint32_t *ValueOut_0, uint32_t *ValueOut_1, struct dsi_cmd_desc *pCmd,
													  char __iomem * dsi_base_0, char __iomem * dsi_base_1, int is_timeout_0, int is_timeout_1)
{
	unsigned long dw_jiffies = 0;
	uint32_t pkg_status_0 = 0, pkg_status_1 = 0;
	bool read_done_0 = false, read_done_1 = false;
	int ret = 2;

	if ((is_timeout_0==0) && (is_timeout_1==0)) {
		/*wait dsi read data*/
		dw_jiffies = jiffies + HZ;
		do {
			if(!read_done_0) {
				pkg_status_0 = inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
				if (!(pkg_status_0 & 0x10)) {
					read_done_0 = true;
					*ValueOut_0 = inp32(dsi_base_0 + MIPIDSI_GEN_PLD_DATA_OFFSET);;
				}
			}
			if(!read_done_1) {
				pkg_status_1 = inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
				if (!(pkg_status_1 & 0x10)) {
					read_done_1 = true;
					*ValueOut_1 = inp32(dsi_base_1 + MIPIDSI_GEN_PLD_DATA_OFFSET);;
				}
			}

			if (read_done_0 && read_done_1 )
				break;
		} while (time_after(dw_jiffies, jiffies));

		if (!read_done_0) {
			HISI_FB_ERR("_mipi_dual_dsi_read_add dsi0 rx timeout :0x%x \n \
				MIPIDSI_CMD_PKT_STATUS = 0x%x \n \
				MIPIDSI_PHY_STATUS = 0x%x \n",
				pCmd->payload[0],
				inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
				inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET));
			ret -= 1;
		}
		if (!read_done_1) {
			HISI_FB_ERR("_mipi_dual_dsi_read_add dsi1 rx timeout :0x%x \n \
				MIPIDSI_CMD_PKT_STATUS = 0x%x \n \
				MIPIDSI_PHY_STATUS = 0x%x \n",
				pCmd->payload[0],
				inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
				inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET));
			ret -= 1;
		}
	} else if (is_timeout_0==0) {
		ret = _mipi_dsi_read_add_receive(ValueOut_0, pCmd, dsi_base_0);
	} else if (is_timeout_1==0) {
		ret = _mipi_dsi_read_add_receive(ValueOut_1, pCmd, dsi_base_1);
	} else {
		ret = 0;
	}

	return ret;
}

//only support same cmd for two dsi
static int _mipi_dual_dsi_read_add(uint32_t *ValueOut_0, uint32_t *ValueOut_1, struct dsi_cmd_desc *pCmd, char __iomem * dsi_base_0, char __iomem * dsi_base_1)
{
	int is_timeout_0 = 1, is_timeout_1 = 1;
	int ret = 0;

	if (NULL == pCmd) {
		HISI_FB_ERR("Cmd is NULL");
		return -EINVAL;
	}
	if ((NULL == dsi_base_0) || (NULL == dsi_base_1)) {
		HISI_FB_ERR("dsi_base is NULL");
		return -EINVAL;
	}
	if ((NULL == ValueOut_0) || (NULL == ValueOut_1)) {
		HISI_FB_ERR("out buffer is NULL");
		return -EINVAL;
	}

	if (DSI_HDR_DTYPE(pCmd->dtype) == DTYPE_DCS_READ) {
		_mipi_dsi_cmd_send_lock();

		mipi_dsi_sread_request(pCmd, dsi_base_0);
		mipi_dsi_sread_request(pCmd, dsi_base_1);
		ret = _mipi_dual_dsi_read(ValueOut_0, ValueOut_1, dsi_base_0, dsi_base_1);

		if (ret <= 0) {
			HISI_FB_ERR("Read register 0x%X error\n",pCmd->payload[0]);
			ret = -1;
		}

		_mipi_dsi_cmd_send_unlock();
	} else if (pCmd->dtype == DTYPE_GEN_READ1) {
		_mipi_dsi_cmd_send_lock();

		ret = _mipi_dual_dsi_read_add_send(pCmd, dsi_base_0, dsi_base_1, &is_timeout_0, &is_timeout_1);
		if (ret > 0 ) {
			ret = _mipi_dual_dsi_read_add_receive(ValueOut_0, ValueOut_1, pCmd, dsi_base_0, dsi_base_1, is_timeout_0, is_timeout_1);
		}

		_mipi_dsi_cmd_send_unlock();
	} else {
		ret = -1;
		HISI_FB_ERR("dtype=%x NOT supported!\n", pCmd->dtype);
	}

	return ret;
}

//just for Cyclomatic Complexity, no need to check input param
static inline int _mipi_dual_dsi_cmds_tx_normal(struct dsi_cmd_desc *Cmd_0, int Cmdset_cnt_0, char __iomem * dsi_base_0,
						    							struct dsi_cmd_desc *Cmd_1, int Cmdset_cnt_1, char __iomem * dsi_base_1)
{
	int i = 0, j=0;
	uint32_t nextwait, nextwait_0=0, nextwait_1=0;

	while ( (i < Cmdset_cnt_0) && (j < Cmdset_cnt_1) ) {
		if (nextwait_0 == 0) {
			mipi_dsi_cmd_add(Cmd_0, dsi_base_0);

			nextwait_0 = _calc_next_wait_time(Cmd_0->wait, Cmd_0->waittype);
			Cmd_0++;
			i++;
		}
		if (nextwait_1 == 0) {
			mipi_dsi_cmd_add(Cmd_1, dsi_base_1);

			nextwait_1 = _calc_next_wait_time(Cmd_1->wait, Cmd_1->waittype);
			Cmd_1++;
			j++;
		}

		nextwait = (nextwait_0 > nextwait_1)? nextwait_1 : nextwait_0;
		_delay_by_msleep(nextwait);
		nextwait_0 -= nextwait;
		nextwait_1 -= nextwait;
	}


	//send the left Cmds
	if (i < Cmdset_cnt_0) { //means j==Cmdset_cnt_1
		//delay the left time
		_delay_by_msleep(nextwait_0);

		for ( ; i < Cmdset_cnt_0; i++) {
			mipi_dsi_cmd_add(Cmd_0, dsi_base_0);

			_delay_for_next_cmd_by_sleep(Cmd_0->wait, Cmd_0->waittype);
			Cmd_0++;
		}
	} else if (j < Cmdset_cnt_1) { //means i==Cmdset_cnt_0
		//delay the left time
		_delay_by_msleep(nextwait_0);

		for ( ; j < Cmdset_cnt_1; j++) {
			mipi_dsi_cmd_add(Cmd_1, dsi_base_1);

			_delay_for_next_cmd_by_sleep(Cmd_1->wait, Cmd_1->waittype);
			Cmd_1++;
		}
	}
	return Cmdset_cnt_0 + Cmdset_cnt_1;
}

//just for Cyclomatic Complexity, no need to check input param
static inline int _mipi_dual_dsi_cmds_rx_normal(uint32_t *ValueOut_0, struct dsi_cmd_desc *Cmd_0, int Cmdset_cnt_0, char __iomem * dsi_base_0,
						    							uint32_t *ValueOut_1, struct dsi_cmd_desc *Cmd_1, int Cmdset_cnt_1, char __iomem * dsi_base_1)
{
	int i = 0, j=0;
	uint32_t nextwait, nextwait_0=0, nextwait_1=0;
	int err_num = 0;

	while ( (i < Cmdset_cnt_0) && (j < Cmdset_cnt_1) ) {
		if (nextwait_0 == 0) {
			if(mipi_dsi_read_add(&(ValueOut_0[i]), Cmd_0, dsi_base_0)){
				err_num++;
			}

			nextwait_0 = _calc_next_wait_time(Cmd_0->wait, Cmd_0->waittype);
			Cmd_0++;
			i++;
		}
		if (nextwait_1 == 0) {
			if(mipi_dsi_read_add(&(ValueOut_1[j]), Cmd_1, dsi_base_1)){
				err_num++;
			}

			nextwait_1 = _calc_next_wait_time(Cmd_1->wait, Cmd_1->waittype);
			Cmd_1++;
			j++;
		}

		nextwait = (nextwait_0 > nextwait_1)? nextwait_1 : nextwait_0;
		_delay_by_msleep(nextwait);
		nextwait_0 -= nextwait;
		nextwait_1 -= nextwait;
	}

	//send the left Cmds
	if (i < Cmdset_cnt_0) { //means j==Cmdset_cnt_1
		//delay the left time
		_delay_by_msleep(nextwait_0);

		for ( ; i < Cmdset_cnt_0; i++) {
			if(mipi_dsi_read_add(&(ValueOut_0[i]), Cmd_0, dsi_base_0)){
				err_num++;
			}

			_delay_for_next_cmd_by_sleep(Cmd_0->wait, Cmd_0->waittype);
			Cmd_0++;
		}
	} else if (j < Cmdset_cnt_1) { //means i==Cmdset_cnt_0
		//delay the left time
		_delay_by_msleep(nextwait_0);

		for ( ; j < Cmdset_cnt_1; j++) {
			if(mipi_dsi_read_add(&(ValueOut_1[j]), Cmd_1, dsi_base_1)){
				err_num++;
			}

			_delay_for_next_cmd_by_sleep(Cmd_1->wait, Cmd_1->waittype);
			Cmd_1++;
		}
	}

	return err_num;
}

//wait two dsi both ready to ensure send cmd at the same time
static int _mipi_dual_dsi_fifo_is_full(const char __iomem * dsi_base_0, const char __iomem * dsi_base_1)
{
	uint32_t pkg_status_0, pkg_status_1;
	uint32_t phy_status_0, phy_status_1;
	int is_timeout = 100;

	if ((NULL == dsi_base_0) || (NULL == dsi_base_1)) {
		HISI_FB_ERR("dsi_base is NULL");
		return -EINVAL;
	}

	/*read status register*/
	do {
		pkg_status_0 = inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		phy_status_0 = inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET);
		pkg_status_1 = inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		phy_status_1 = inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET);
		if ((pkg_status_0 & 0x2) != 0x2 && !(phy_status_0 & 0x2) && (pkg_status_1 & 0x2) != 0x2 && !(phy_status_1 & 0x2)) {
			break;
		}
		udelay(100);
	} while (is_timeout-- > 0);

	if (!is_timeout) {
		HISI_FB_ERR("mipi check full fail: dsi0-\n \
						MIPIDSI_CMD_PKT_STATUS = 0x%x \n \
						MIPIDSI_PHY_STATUS = 0x%x \n \
						MIPIDSI_INT_ST1_OFFSET = 0x%x \n",
					inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
					inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET),
					inp32(dsi_base_0 + MIPIDSI_INT_ST1_OFFSET));
		HISI_FB_ERR("mipi check full fail: dsi1-\n \
						MIPIDSI_CMD_PKT_STATUS = 0x%x \n \
						MIPIDSI_PHY_STATUS = 0x%x \n \
						MIPIDSI_INT_ST1_OFFSET = 0x%x \n",
					inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
					inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET),
					inp32(dsi_base_1 + MIPIDSI_INT_ST1_OFFSET));
		return -1;
	}
	return 0;
}

//just for Cyclomatic Complexity, no need to check input param
static inline int _mipi_dual_dsi_tx_normal_same_delay(struct dsi_cmd_desc *Cmd0, struct dsi_cmd_desc *Cmd1, int Cmdset_cnt, char __iomem * dsi_base_0, char __iomem * dsi_base_1)
{
	int i = 0;
	int send_cnt=0;

	for (i = 0; i < Cmdset_cnt; i++) {
		if (!_mipi_dual_dsi_fifo_is_full(dsi_base_0, dsi_base_1)) {
			mipi_dsi_cmd_add(Cmd0, dsi_base_0);
			mipi_dsi_cmd_add(Cmd1, dsi_base_1);
		} else {
			HISI_FB_ERR("dsi fifo full, send (%d) cmds, left (%d) cmds!!", send_cnt, Cmdset_cnt*2-send_cnt);
			break;
		}

		_delay_for_next_cmd_by_sleep(Cmd0->wait, Cmd0->waittype);
		send_cnt += 2;

		Cmd0++;
		Cmd1++;
	}

	return send_cnt;
}

//just for Cyclomatic Complexity, no need to check input param
static inline int _mipi_dual_dsi_tx_auto_mode(struct dsi_cmd_desc *Cmd0, struct dsi_cmd_desc *Cmd1, int Cmdset_cnt, char __iomem * dsi_base_0, char __iomem * dsi_base_1)
{
	int send_cnt=0;
	struct timespec64 ts;
	s64 timestamp, delta_time;
	s64 oneframe_time;

	HISI_FB_DEBUG("+.\n");

	if (Cmd0 != Cmd1) {
		HISI_FB_DEBUG("different cmd for two dsi, using normal mode.\n");
		goto NormalMode;
	}

	if (!g_delayed_cmd_queue_inited) {
		HISI_FB_ERR("delayed cmd queue is not inited yet!\n");
		goto NormalMode;
	}

	ktime_get_ts64(&ts);
	timestamp = ts.tv_sec;
	timestamp *= NSEC_PER_SEC;
	timestamp += ts.tv_nsec;
	delta_time = timestamp -g_delayed_cmd_queue.timestamp_frame_start;
	oneframe_time = g_delayed_cmd_queue.oneframe_time;

	//the CMD_AUTO_MODE_THRESHOLD is not accurate value, can be ajusted later
	if ( (delta_time < oneframe_time*CMD_AUTO_MODE_THRESHOLD) || (delta_time > oneframe_time) ) {
		HISI_FB_DEBUG("enough time(%lld) to send in this frame, using normal mode.\n", delta_time);
		goto NormalMode;
	}

	HISI_FB_DEBUG("NOT enough time(%lld) to send in this frame, using low priority delayed mode.\n", delta_time);
	send_cnt = _mipi_dsi_delayed_cmd_queue_write(Cmd0, Cmdset_cnt, true) * 2;

	HISI_FB_DEBUG("-.\n");
	return send_cnt;

NormalMode:
	send_cnt = _mipi_dual_dsi_tx_normal_same_delay(Cmd0, Cmd1, Cmdset_cnt, dsi_base_0, dsi_base_1);
	HISI_FB_DEBUG("--.\n");
	return send_cnt;
}

int  mipi_dual_dsi_cmds_tx( struct dsi_cmd_desc *pCmdset_0, int Cmdset_cnt_0, char __iomem * dsi_base_0,
						    		struct dsi_cmd_desc *pCmdset_1, int Cmdset_cnt_1, char __iomem * dsi_base_1, uint8_t tx_mode )
{
	struct dsi_cmd_desc *Cmd0 = pCmdset_0;
	struct dsi_cmd_desc *Cmd1 = pCmdset_1;
	int send_cnt=0;
	bool sameCmdDelay = false;

	if ( (NULL == pCmdset_0) || (0 == Cmdset_cnt_0) ) {
		HISI_FB_ERR("Cmdset is NULL!\n");
		return 0;
	}
	if (NULL == dsi_base_0) {
		HISI_FB_ERR("dsi_base is NULL");
		return -0;
	}

	//if only one dsi_base exist, fallback to mipi_dsi_cmds_tx() interface
	if (NULL == dsi_base_1) {
		return mipi_dsi_cmds_tx_with_check_fifo(pCmdset_0, Cmdset_cnt_0, dsi_base_0);
	}

	if ( (NULL == pCmdset_1) || (0 == Cmdset_cnt_1) ) { //only one valid Cmdset, use same Cmd_set for two dsi
		Cmd1 = pCmdset_0;
		sameCmdDelay = true;
	} else if (Cmdset_cnt_0 == Cmdset_cnt_1) {	//support different cmd value by same step
		sameCmdDelay = true;
	}

	if (sameCmdDelay) {
		if (tx_mode == EN_DSI_TX_NORMAL_MODE) {
			//only support same cmdset length and wait, just cmd value different
			send_cnt = _mipi_dual_dsi_tx_normal_same_delay(Cmd0, Cmd1, Cmdset_cnt_0, dsi_base_0, dsi_base_1);
		} else if (tx_mode == EN_DSI_TX_LOW_PRIORITY_DELAY_MODE) {
			//only support same cmd set yet
			send_cnt = _mipi_dsi_delayed_cmd_queue_write(Cmd0, Cmdset_cnt_0, true) * 2;
		} else if (tx_mode == EN_DSI_TX_HIGH_PRIORITY_DELAY_MODE) {
			//only support same cmd set yet
			send_cnt = _mipi_dsi_delayed_cmd_queue_write(Cmd0, Cmdset_cnt_0, false) * 2;
		} else if (tx_mode == EN_DSI_TX_AUTO_MODE) {
			//only support same cmdset length and wait, just cmd value different
			send_cnt = _mipi_dual_dsi_tx_auto_mode(Cmd0, Cmd1, Cmdset_cnt_0, dsi_base_0, dsi_base_1);
		}
	} else { //two different Cmdsets case
		//just use normal mode, expect no need use
		send_cnt = _mipi_dual_dsi_cmds_tx_normal(pCmdset_0, Cmdset_cnt_0, dsi_base_0, pCmdset_1, Cmdset_cnt_1, dsi_base_1);
	}

	return send_cnt;
}

int mipi_dual_dsi_cmds_rx( uint32_t *ValueOut_0, struct dsi_cmd_desc *pCmdset_0, int Cmdset_cnt_0, char __iomem * dsi_base_0,
						   		uint32_t *ValueOut_1, struct dsi_cmd_desc *pCmdset_1, int Cmdset_cnt_1, char __iomem * dsi_base_1 )
{
	struct dsi_cmd_desc *Cmd = NULL;
	int i = 0;
	int err_num = 0;
	int cnt;

	if ( (NULL == pCmdset_0) || (0 == Cmdset_cnt_0) ) {
		HISI_FB_ERR("Cmdset is NULL!\n");
		return -EINVAL;
	}
	if (NULL == dsi_base_0) {
		HISI_FB_ERR("dsi_base is NULL");
		return -EINVAL;
	}
	if (NULL == ValueOut_0) {
		HISI_FB_ERR("out buffer 0 is NULL");
		return -EINVAL;
	}

	//if only one dsi_base exist, fallback to mipi_dsi_cmds_rx() interface
	if (NULL == dsi_base_1) {
		return mipi_dsi_cmds_rx_with_check_fifo(ValueOut_0, pCmdset_0, Cmdset_cnt_0, dsi_base_0);
	}
	if (NULL == ValueOut_1) {
		HISI_FB_ERR("out buffer 1 is NULL");
		return 0;
	}
	Cmd = pCmdset_0;

	if ((NULL == pCmdset_1) || (0 == Cmdset_cnt_1) ) { //only one valid Cmdset, use same Cmd_set for two dsi
		for (i = 0; i < Cmdset_cnt_0; i++) {
			if (!_mipi_dual_dsi_fifo_is_full(dsi_base_0, dsi_base_1)) {
				cnt = _mipi_dual_dsi_read_add(&(ValueOut_0[i]), &(ValueOut_1[i]), Cmd, dsi_base_0, dsi_base_1);
				if (cnt <= 0) {
					err_num += 2;
				} else if (cnt == 1) {
					err_num += 1;
				}
			} else {
				err_num += (Cmdset_cnt_0-i)*2;
				HISI_FB_ERR("dsi fifo full, read (%d) cmds, left (%d) cmds!!", i, Cmdset_cnt_0-i);
				break;
			}

			_delay_for_next_cmd_by_sleep(Cmd->wait, Cmd->waittype);
			Cmd++;
		}
	} else { //two different Cmdsets case, expect no need use
		err_num = _mipi_dual_dsi_cmds_rx_normal(ValueOut_0, pCmdset_0, Cmdset_cnt_0, dsi_base_0, ValueOut_1, pCmdset_1, Cmdset_cnt_1, dsi_base_1);
	}

	return err_num;
}

//only support same cmd for two dsi
int mipi_dual_dsi_lread_reg( uint32_t *ValueOut_0, uint32_t *ValueOut_1, struct dsi_cmd_desc *pCmd, uint32_t dlen, char __iomem * dsi_base_0, char __iomem * dsi_base_1)
{
	int ret = 0;
	uint32_t i = 0;
	struct dsi_cmd_desc packet_size_cmd_set;

	if ( (NULL == pCmd) ||  (0 == dlen) ) {
		HISI_FB_ERR("Cmd is NULL!\n");
		return -1;
	}
	if ((NULL == dsi_base_0) || (NULL == dsi_base_1)) {
		HISI_FB_ERR("dsi_base is NULL");
		return -1;
	}
	if ((NULL == ValueOut_0) || (NULL == ValueOut_1)) {
		HISI_FB_ERR("out buffer is NULL");
		return -1;
	}

	if (mipi_dsi_cmd_is_read(pCmd)) {
		if (!_mipi_dual_dsi_fifo_is_full(dsi_base_0, dsi_base_1)) {
			packet_size_cmd_set.dtype = DTYPE_MAX_PKTSIZE;
			packet_size_cmd_set.vc = 0;
			packet_size_cmd_set.dlen = dlen;

			_mipi_dsi_cmd_send_lock();

			mipi_dsi_max_return_packet_size(&packet_size_cmd_set, dsi_base_0);
			mipi_dsi_max_return_packet_size(&packet_size_cmd_set, dsi_base_1);
			mipi_dsi_sread_request(pCmd, dsi_base_0);
			mipi_dsi_sread_request(pCmd, dsi_base_1);
			for (i = 0; i < (dlen + 3)/4; i++) {
				if (_mipi_dual_dsi_read(ValueOut_0, ValueOut_1, dsi_base_0, dsi_base_1) < 2) {
					ret = -1;
					HISI_FB_ERR("Read register 0x%X timeout\n", pCmd->payload[0]);
					break;
				}
				ValueOut_0++;
				ValueOut_1++;
			}

			_mipi_dsi_cmd_send_unlock();
		} else {
			ret = -1;
			HISI_FB_ERR("dsi fifo full, read fail!!");
		}
	} else {
		ret = -1;
		HISI_FB_ERR("dtype=%x NOT supported!\n", pCmd->dtype);
	}

	return ret;
}



static int _mipi_dsi_delayed_cmd_queue_write(struct dsi_cmd_desc *pCmdset, int Cmdset_cnt, bool isLowPriority)
{
	spinlock_t *pSpinlock;
	uint32_t *w_ptr;
	uint32_t *r_ptr;
	struct dsi_cmd_desc* CmdQueue;
	bool *isQueueFull;
	bool *isQueueWorking;
	uint32_t QueueLen;
	int i;
	u32 j;

	struct dsi_cmd_desc *Cmd = pCmdset;

	if (!pCmdset) {
		HISI_FB_ERR("Cmd is NULL!\n");
		return 0;
	}

	if (!g_delayed_cmd_queue_inited) {
		HISI_FB_ERR("delayed cmd queue is not inited yet!\n");
		return 0;
	}

	if (isLowPriority)
	{
		CmdQueue = g_delayed_cmd_queue.CmdQueue_LowPriority;
		pSpinlock = &g_delayed_cmd_queue.CmdQueue_LowPriority_lock;
		QueueLen = MAX_CMD_QUEUE_LOW_PRIORITY_SIZE;
	} else {
		CmdQueue = g_delayed_cmd_queue.CmdQueue_HighPriority;
		pSpinlock = &g_delayed_cmd_queue.CmdQueue_HighPriority_lock;
		QueueLen = MAX_CMD_QUEUE_HIGH_PRIORITY_SIZE;
	}

	spin_lock(pSpinlock);

	if (isLowPriority)
	{
		w_ptr = &g_delayed_cmd_queue.CmdQueue_LowPriority_wr;
		r_ptr = &g_delayed_cmd_queue.CmdQueue_LowPriority_rd;
		isQueueFull = &g_delayed_cmd_queue.isCmdQueue_LowPriority_full;
		isQueueWorking = &g_delayed_cmd_queue.isCmdQueue_LowPriority_working;
	} else {
		w_ptr = &g_delayed_cmd_queue.CmdQueue_HighPriority_wr;
		r_ptr = &g_delayed_cmd_queue.CmdQueue_HighPriority_rd;
		isQueueFull = &g_delayed_cmd_queue.isCmdQueue_HighPriority_full;
		isQueueWorking = &g_delayed_cmd_queue.isCmdQueue_HighPriority_working;
	}

	if (*isQueueFull) {
		HISI_FB_ERR("Fail, delayed cmd queue (%d) is full!\n", isLowPriority);
		spin_unlock(pSpinlock);
		return 0;
	}

	for (i=0; i<Cmdset_cnt; i++) {
		CmdQueue[(*w_ptr)].dtype = Cmd->dtype;
		CmdQueue[(*w_ptr)].vc = Cmd->vc;
		CmdQueue[(*w_ptr)].wait = Cmd->wait;
		CmdQueue[(*w_ptr)].waittype = Cmd->waittype;
		CmdQueue[(*w_ptr)].dlen = Cmd->dlen;
		if (Cmd->dlen > 0) {
			CmdQueue[(*w_ptr)].payload =(char*)kmalloc(Cmd->dlen * sizeof(char), GFP_ATOMIC);
			if (CmdQueue[(*w_ptr)].payload) {
				for (j= 0; j < Cmd->dlen; j++) {
					CmdQueue[(*w_ptr)].payload[j] = Cmd->payload[j];
				}
			} else {
				HISI_FB_ERR("Cmd(%d/%d) payload malloc mem (%d) fail!\n", Cmd->dtype, Cmd->vc, Cmd->dlen);
				continue;	//skip this cmd
			}
		}

		(*w_ptr) = (*w_ptr) + 1;
		if ((*w_ptr) >= QueueLen) {
			(*w_ptr) = 0;
		}

		(*isQueueWorking) = true;

		if ((*w_ptr) == (*r_ptr)) {
			(*isQueueFull) = true;
			HISI_FB_ERR("Fail, delayed cmd queue (%d) become full, %d cmds are not added in queue!\n", isLowPriority, (Cmdset_cnt-i));
			break;
		}

		Cmd++;
	}

	spin_unlock(pSpinlock);

	HISI_FB_DEBUG("%d cmds are added to delayed cmd queue (%d).\n", i, isLowPriority);
	return i;

}

static int _mipi_dsi_delayed_cmd_queue_read(struct dsi_cmd_desc* Cmd, bool isLowPriority)
{
	spinlock_t *pSpinlock;
	uint32_t *w_ptr;
	uint32_t *r_ptr;
	struct dsi_cmd_desc* CmdQueue;
	bool *isQueueFull;
	bool *isQueueWorking;
	uint32_t QueueLen;

	if (!Cmd) {
		HISI_FB_ERR("Cmd is NULL!\n");
		return -1;
	}

	if (!g_delayed_cmd_queue_inited) {
		HISI_FB_ERR("delayed cmd queue is not inited yet!\n");
		return -1;
	}

	if (isLowPriority)
	{
		CmdQueue = g_delayed_cmd_queue.CmdQueue_LowPriority;
		pSpinlock = &g_delayed_cmd_queue.CmdQueue_LowPriority_lock;
		QueueLen = MAX_CMD_QUEUE_LOW_PRIORITY_SIZE;
	} else {
		CmdQueue = g_delayed_cmd_queue.CmdQueue_HighPriority;
		pSpinlock = &g_delayed_cmd_queue.CmdQueue_HighPriority_lock;
		QueueLen = MAX_CMD_QUEUE_HIGH_PRIORITY_SIZE;
	}

	spin_lock(pSpinlock);

	if (isLowPriority)
	{
		w_ptr = &g_delayed_cmd_queue.CmdQueue_LowPriority_wr;
		r_ptr = &g_delayed_cmd_queue.CmdQueue_LowPriority_rd;
		isQueueFull = &g_delayed_cmd_queue.isCmdQueue_LowPriority_full;
		isQueueWorking = &g_delayed_cmd_queue.isCmdQueue_LowPriority_working;
	} else {
		w_ptr = &g_delayed_cmd_queue.CmdQueue_HighPriority_wr;
		r_ptr = &g_delayed_cmd_queue.CmdQueue_HighPriority_rd;
		isQueueFull = &g_delayed_cmd_queue.isCmdQueue_HighPriority_full;
		isQueueWorking = &g_delayed_cmd_queue.isCmdQueue_HighPriority_working;
	}

	if ( (!(*isQueueFull)) && ((*w_ptr) == (*r_ptr)) ) {
		//HISI_FB_DEBUG("Note, delayed cmd queue (%d) is empty, no cmd need to send.\n", isLowPriority);
		spin_unlock(pSpinlock);
		return -1;
	}

	Cmd->dtype = CmdQueue[(*r_ptr)].dtype;
	Cmd->vc = CmdQueue[(*r_ptr)].vc;
	Cmd->wait = CmdQueue[(*r_ptr)].wait;
	Cmd->waittype = CmdQueue[(*r_ptr)].waittype;
	Cmd->dlen = CmdQueue[(*r_ptr)].dlen;
	Cmd->payload = CmdQueue[(*r_ptr)].payload;		//Note: just copy the pointer. The malloc mem will be free by the caller after useless
	memset(&(CmdQueue[(*r_ptr)]), 0, sizeof(struct dsi_cmd_desc));

	(*r_ptr) = (*r_ptr) + 1;
	if ((*r_ptr) >= QueueLen) {
		(*r_ptr) = 0;
	}

	(*isQueueFull) = false;

	if ((*r_ptr) == (*w_ptr)) {
		(*isQueueWorking) = false;
		HISI_FB_DEBUG("The last cmd in delayed queue (%d) is sent!\n", isLowPriority);
	}

	spin_unlock(pSpinlock);

	HISI_FB_DEBUG("Acquire one cmd from delayed cmd queue (%d) successl!\n", isLowPriority);
	return 0;
}

static uint32_t _mipi_dsi_get_delayed_cmd_queue_send_count(bool isLowPriority)
{
	spinlock_t *pSpinlock;
	uint32_t w_ptr;
	uint32_t r_ptr;
	bool isQueueFull;
	uint32_t QueueLen;
	uint32_t send_count = 0;

	if (!g_delayed_cmd_queue_inited) {
		HISI_FB_ERR("delayed cmd queue is not inited yet!\n");
		return 0;
	}

	if (isLowPriority)
	{
		pSpinlock = &g_delayed_cmd_queue.CmdQueue_LowPriority_lock;
		QueueLen = MAX_CMD_QUEUE_LOW_PRIORITY_SIZE;
	} else {
		pSpinlock = &g_delayed_cmd_queue.CmdQueue_HighPriority_lock;
		QueueLen = MAX_CMD_QUEUE_HIGH_PRIORITY_SIZE;
	}

	spin_lock(pSpinlock);

	if (isLowPriority)
	{
		w_ptr = g_delayed_cmd_queue.CmdQueue_LowPriority_wr;
		r_ptr = g_delayed_cmd_queue.CmdQueue_LowPriority_rd;
		isQueueFull = g_delayed_cmd_queue.isCmdQueue_LowPriority_full;
	} else {
		w_ptr = g_delayed_cmd_queue.CmdQueue_HighPriority_wr;
		r_ptr = g_delayed_cmd_queue.CmdQueue_HighPriority_rd;
		isQueueFull = g_delayed_cmd_queue.isCmdQueue_HighPriority_full;
	}

	spin_unlock(pSpinlock);		//no need lock any more

	if (w_ptr > r_ptr) {
		send_count = w_ptr - r_ptr;
	} else if (w_ptr < r_ptr) {
		send_count = w_ptr + QueueLen - r_ptr;
	} else if (isQueueFull) {
		send_count = QueueLen;
	} //else: means queue empty, send_count is 0

	if (send_count > 0) {
		HISI_FB_DEBUG("delay cmd queue (%d): %d cmds need to be sent.\n", isLowPriority, send_count);
	}

	return send_count;
}


void mipi_dsi_init_delayed_cmd_queue(void)
{
	HISI_FB_INFO("+.\n");

	memset(&g_delayed_cmd_queue, 0, sizeof(struct dsi_delayed_cmd_queue));
	sema_init(&g_delayed_cmd_queue.work_queue_sem, 1);
	spin_lock_init(&g_delayed_cmd_queue.CmdSend_lock);
	spin_lock_init(&g_delayed_cmd_queue.CmdQueue_LowPriority_lock);
	spin_lock_init(&g_delayed_cmd_queue.CmdQueue_HighPriority_lock);
	g_delayed_cmd_queue_inited = true;

	return;
}

void mipi_dsi_delayed_cmd_queue_handle_func(struct work_struct *work)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct dsi_cmd_desc Cmd={0};
	uint32_t send_count_high_priority, send_count_low_priority;
	s64 oneframe_time;

	if (NULL == work) {
		HISI_FB_ERR("work is NULL\n");
		return;
	}

	if (!g_delayed_cmd_queue_inited) {
		HISI_FB_ERR("delayed cmd queue is not inited yet!\n");
		return;
	}

	hisifd = container_of(work, struct hisi_fb_data_type, delayed_cmd_queue_work);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL\n");
		return;
	}

	HISI_FB_INFO("+.\n");

	down(&g_delayed_cmd_queue.work_queue_sem);

	send_count_low_priority = _mipi_dsi_get_delayed_cmd_queue_send_count(true);
	send_count_high_priority = _mipi_dsi_get_delayed_cmd_queue_send_count(false);

	while ( send_count_high_priority > 0 ) {
		if (_mipi_dsi_delayed_cmd_queue_read(&Cmd, false)) {	//get the next cmd in high priority queue
			//send the cmd with normal mode
			_mipi_dual_dsi_tx_normal_same_delay(&Cmd, &Cmd, 1, hisifd->mipi_dsi0_base, hisifd->mipi_dsi1_base);
			if ( Cmd.payload ) {
				kfree(Cmd.payload);
				Cmd.payload = NULL;
			}
		}
		send_count_high_priority--;
	}

	while ( send_count_low_priority > 0 ) {
		if (_mipi_dsi_delayed_cmd_queue_read(&Cmd, true)) {	//get the next cmd in low priority queue
			//send the cmd with normal mode
			_mipi_dual_dsi_tx_normal_same_delay(&Cmd, &Cmd, 1, hisifd->mipi_dsi0_base, hisifd->mipi_dsi1_base);
			if ( Cmd.payload ) {
				kfree(Cmd.payload);
				Cmd.payload = NULL;
			}
		}
		send_count_low_priority--;
	}

	up(&g_delayed_cmd_queue.work_queue_sem);

	//re-fresh one frame time
	oneframe_time = (s64)(hisifd->panel_info.xres + hisifd->panel_info.ldi.h_back_porch +
			hisifd->panel_info.ldi.h_front_porch + hisifd->panel_info.ldi.h_pulse_width) * (hisifd->panel_info.yres +
			hisifd->panel_info.ldi.v_back_porch + hisifd->panel_info.ldi.v_front_porch + hisifd->panel_info.ldi.v_pulse_width) *
			1000000000UL / hisifd->panel_info.pxl_clk_rate;
	if (oneframe_time != g_delayed_cmd_queue.oneframe_time) {
		g_delayed_cmd_queue.oneframe_time = oneframe_time;
		HISI_FB_INFO("update one frame time to %lld.\n", oneframe_time);
	}

	HISI_FB_INFO("-.\n");
	return;
}

void mipi_dsi_set_timestamp(void)
{
	struct timespec64 ts;
	s64 timestamp;

	if (g_delayed_cmd_queue_inited) {
		ktime_get_ts64(&ts);
		timestamp = ts.tv_sec;
		timestamp *= NSEC_PER_SEC;
		timestamp += ts.tv_nsec;
		g_delayed_cmd_queue.timestamp_frame_start = timestamp;
	}
	return;
}

bool mipi_dsi_check_delayed_cmd_queue_working(void)
{
	bool ret = false;

	if (g_delayed_cmd_queue.isCmdQueue_HighPriority_working || g_delayed_cmd_queue.isCmdQueue_LowPriority_working) {
		ret = true;
	}

	return ret;
}

