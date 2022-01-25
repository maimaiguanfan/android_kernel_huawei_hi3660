/*
 * kbhub_channel.h
 *
 * huawei kbhub driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#ifndef _KBHUB_CHANNEL_H_
#define _KBHUB_CHANNEL_H_

#define KBHB_MCUREADY            (0xFD)
#define KBHUB_REPORT_DATA_SIZE   (16)

enum kb_type_t {
	KB_TYPE_START = 0x0,
	KB_TYPE_UART_RUN,
	KB_TYPE_UART_STOP,
	KB_TYPE_UART_RESTART,
	KB_TYPE_END,
};

struct pkt_kb_data_req_t {
	pkt_header_t hd;
	enum kb_type_t cmd;
};

struct kb_outreport_t {
	unsigned int sub_cmd;
	int report_len;
	uint8_t report_data[KBHUB_REPORT_DATA_SIZE];
};

struct kb_cmd_map_t {
	int fhb_ioctl_app_cmd;
	int ca_type;
	int tag;
	obj_cmd_t cmd;
	obj_sub_cmd_t subcmd;
};

struct kbdev_proxy {
	struct kb_dev_ops *ops;
	int notify_event;
};

extern bool really_do_enable_disable(int *ref_cnt, bool enable, int bit);
extern int send_app_config_cmd(int tag, void *app_config, bool use_lock);
extern void disable_kb_when_sysreboot(void);

#endif /* _KBHUB_CHANNEL_H_ */
