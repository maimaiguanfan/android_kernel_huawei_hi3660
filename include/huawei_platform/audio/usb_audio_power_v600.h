/*
 * usb_audio_power_v600.h -- usb audio power control driver
 *
 * Copyright (c) 2015 Huawei Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __USB_AUDIO_POWER_V600_H__
#define __USB_AUDIO_POWER_V600_H__

#define IOCTL_USB_AUDIO_POWER_BUCKBOOST_NO_HEADSET_CMD     _IO('Q', 0x01)
#define IOCTL_USB_AUDIO_POWER_SCHARGER_CMD                 _IO('Q', 0x02)
#ifdef CONFIG_USB_AUDIO_POWER_V600
void set_otg_switch_enable_v600(void);
#else
static inline void set_otg_switch_enable_v600(void)
{
	return ;
}
#endif
#endif //__USB_AUDIO_POWER_V600_H_
