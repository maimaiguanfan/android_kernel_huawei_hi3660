/*
 * hw_u_serial.h
 *
 *  interface to USB gadget "serial port"/TTY utilities
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

#ifndef _HW_U_SERIAL_H_
#define _HW_U_SERIAL_H_

#include <linux/usb/composite.h>
#include <linux/usb/cdc.h>
#include "usb_vendor.h"

#define HW_MAX_U_SERIAL_PORTS       (4)
#define HW_ACM_IS_SINGLE            (1)
#define HW_ACM_SUPPORT_NOTIFY       (0)

#define USB_CDC_SET_COMM_FEATURE    (0x02)
#define USB_CDC_SEND_PORT_STATUS    (0xa3)

struct f_serial_opts {
	struct usb_function_instance func_inst;
	u8 port_num;
};

/*
 * One non-multiplexed "serial" I/O port ... there can be several of these
 * on any given USB peripheral device, if it provides enough endpoints.
 *
 * The "u_serial" utility component exists to do one thing:  manage TTY
 * style I/O using the USB peripheral endpoints listed here, including
 * hookups to sysfs and /dev for each logical "tty" device.
 *
 * REVISIT at least ACM could support tiocmget() if needed.
 *
 * REVISIT someday, allow multiplexing several TTYs over these endpoints.
 */
struct gserial {
	struct usb_function func;

	/* port is managed by gserial_{connect,disconnect} */
	struct gs_port *ioport;

	struct usb_ep *in;
	struct usb_ep *out;

	/* REVISIT avoid this CDC-ACM support harder ... */
	struct usb_cdc_line_coding port_line_coding; /* 9600-8-N-1 etc */

	/* notification callbacks */
	void (*connect)(struct gserial *p);
	void (*disconnect)(struct gserial *p);

	void (*notify_state)(struct gserial *p, u16 state);
	void (*flow_control)(struct gserial *p, u32 rx_is_on, u32 tx_is_on);

	int (*send_break)(struct gserial *p, int duration);
};

/* table for acm interface name and corresponding protocol */
struct acm_name_type_tbl {
	char *name;
	USB_PID_UNIFY_IF_PROT_T type;
};

/* utilities to allocate/free request and buffer */
struct usb_request *hw_gs_alloc_req(struct usb_ep *ep, unsigned int len,
	gfp_t kmalloc_flags);
void hw_gs_free_req(struct usb_ep *ep, struct usb_request *req);

/* management of individual TTY ports */
int hw_gserial_alloc_line(unsigned char *line_num);
void hw_gserial_free_line(unsigned char port_num);

/* connect/disconnect is handled by individual functions */
int hw_gserial_connect(struct gserial *gser, u8 port_num);
void hw_gserial_disconnect(struct gserial *gser);

/* functions are bound to configurations by a config or gadget driver */
int gser_bind_config(struct usb_configuration *c, u8 port_num);
int obex_bind_config(struct usb_configuration *c, u8 port_num);

#endif /* _HW_U_SERIAL_H_ */
