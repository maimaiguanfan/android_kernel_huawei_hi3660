/*
 * kbhub.h
 *
 * kbhub driver interface
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

#ifndef _KBHUB_H_
#define _KBHUB_H_

#include <linux/ioctl.h>

/* ioctl cmd define */
#define KBHBIO                      (0xB1)

#define KBHB_IOCTL_START            (_IOW(KBHBIO, 0x01, short))
#define KBHB_IOCTL_STOP             (_IOW(KBHBIO, 0x02, short))
#define KBHB_IOCTL_ATTR_START       (_IOW(KBHBIO, 0x03, short))
#define KBHB_IOCTL_ATTR_STOP        (_IOW(KBHBIO, 0x04, short))
#define KBHB_IOCTL_INTERVAL_SET     (_IOW(KBHBIO, 0x05, short))
#define KBHB_IOCTL_CMD              (_IOW(KBHBIO, 0x06, short))

#define NOTIFY_EVENT_NONE           (0)
#define NOTIFY_EVENT_DETECT         (1)

struct kb_dev_ops {
	int  (*process_kbdata)(char *data, int count);
	int  (*get_status)(void);
	void (*notify_event)(unsigned int event);
};

/* function from kbhub */
extern int kernel_send_kb_cmd(unsigned int cmd, int val);
extern int kernel_send_kb_report_event(unsigned int cmd,
	void *buffer, int size);
extern int kbhb_get_hall_value(void);
extern int kbdev_proxy_register(struct kb_dev_ops *ops);

#endif /* _KBHUB_H_ */
