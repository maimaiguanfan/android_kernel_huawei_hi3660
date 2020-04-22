/*
 * hw_usb.h
 *
 * usb driver
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

#ifndef _HW_USB_H_
#define _HW_USB_H_

#define HW_USB_STR_MAX_LEN          (16)

/* event types notify user-space host abnormal event  */
enum hw_usb_host_abnormal_event_type {
	USB_HOST_EVENT_NORMAL,
	USB_HOST_EVENT_POWER_INSUFFICIENT,
	USB_HOST_EVENT_HUB_TOO_DEEP,
	USB_HOST_EVENT_UNKNOW_DEVICE
};

enum hw_usb_ldo_ctrl_type {
	HW_USB_LDO_CTRL_BEGIN = 0,

	HW_USB_LDO_CTRL_USB = HW_USB_LDO_CTRL_BEGIN,
	HW_USB_LDO_CTRL_COMBOPHY,
	HW_USB_LDO_CTRL_DIRECT_CHARGE,
	HW_USB_LDO_CTRL_HIFIUSB,
	HW_USB_LDO_CTRL_TYPECPD,

	HW_USB_LDO_CTRL_MAX,
};

struct hw_usb_device {
	struct platform_device *pdev;
	struct device *dev;
	struct regulator *usb_phy_ldo; /* usb phy 3.3v ldo */
	char usb_speed[HW_USB_STR_MAX_LEN];
};

#ifdef CONFIG_HUAWEI_USB
extern void hw_usb_host_abnormal_event_notify(unsigned int event);
extern void hw_usb_set_usb_speed(unsigned int usb_speed);
extern int hw_usb_ldo_supply_enable(enum hw_usb_ldo_ctrl_type type);
extern int hw_usb_ldo_supply_disable(enum hw_usb_ldo_ctrl_type type);

#else

static inline void hw_usb_host_abnormal_event_notify(unsigned int event)
{
}

static inline void hw_usb_set_usb_speed(unsigned int usb_speed)
{
}

static inline int hw_usb_ldo_supply_enable(enum hw_usb_ldo_ctrl_type type)
{
	return 0;
}

static inline int hw_usb_ldo_supply_disable(enum hw_usb_ldo_ctrl_type type)
{
	return 0;
}
#endif /* CONFIG_HUAWEI_USB */

#endif /* _HW_USB_H_ */
