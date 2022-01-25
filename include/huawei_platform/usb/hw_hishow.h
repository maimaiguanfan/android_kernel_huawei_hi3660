/*
 * hw_hishow.h
 *
 * hishow driver
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

#ifndef _HW_HISHOW_H_
#define _HW_HISHOW_H_

#define HISHOW_DEVICE_OFFLINE      (0x1)
#define HISHOW_DEVICE_ONLINE       (0x10)

#define HISHOW_DEV_DATA_MAX        (32)
#define HISHOW_STATE_MAX           (3)

enum hishow_devno {
	HISHOW_DEVICE_BEGIN = 0,

	HISHOW_UNKNOWN_DEVICE = HISHOW_DEVICE_BEGIN, /* unknown hishow device */
	HISHOW_USB_DEVICE, /* usb hishow device */
	HISHOW_HALL_DEVICE, /* hall hishow device */

	HISHOW_DEVICE_END
};

enum hishow_state {
	HISHOW_UNKNOWN,
	HISHOW_DISCONNECTED,
	HISHOW_CONNECTED
};

struct hishow_info {
	struct platform_device *pdev;
	struct device *dev;
	int dev_state;
	int dev_no;
};

#ifdef CONFIG_HUAWEI_HISHOW
extern void hishow_notify_android_uevent(int dev_state, int dev_no);

#else

static inline void hishow_notify_android_uevent(int dev_state, int dev_no)
{
}
#endif /* CONFIG_HUAWEI_HISHOW */

#endif /* _HW_HISHOW_H_ */
