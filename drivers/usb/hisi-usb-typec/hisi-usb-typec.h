/*
 * Copyright (C) 2018 Hisilicon
 * Author: Hisillicon <>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _LOCAL_HISI_USB_TYPEC_H_
#define _LOCAL_HISI_USB_TYPEC_H_

void hisi_usb_typec_power_status_change(int vbus_state);
void hisi_usb_typec_set_vconn(int enable);
int hisi_usb_typec_register_pd_dpm(void);
void hisi_usb_typec_register_tcpc_device(struct tcpc_device *tcpc_dev);
void hisi_usb_typec_max_power(int max_power);
void hisi_usb_typec_cc_status_change(uint8_t cc1, uint8_t cc2);
void hisi_usb_typec_cc_alert(uint8_t cc1, uint8_t cc2);
void hisi_usb_pd_dp_state_change(struct tcp_ny_ama_dp_state *ama_dp_state);
void hisi_usb_pd_dp_hpd_state_change(struct tcp_ny_ama_dp_hpd_state *ama_dp_hpd_state);
void hisi_usb_pd_ufp_update_dock_svid(uint16_t svid);

#endif /* _LOCAL_HISI_USB_TYPEC_H_ */
