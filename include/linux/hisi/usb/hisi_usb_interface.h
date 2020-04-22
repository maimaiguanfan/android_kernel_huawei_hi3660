#ifndef HISI_USB_INTERFACE_H
#define HISI_USB_INTERFACE_H
enum phy_change_type {
	PHY_MODE_CHANGE_BEGIN,
	PHY_MODE_CHANGE_END,
};

enum misc_ctrl_type {
	MICS_CTRL_USB= 0,
	MICS_CTRL_COMBOPHY = 1,
};

int hisi_usb_combophy_notify(enum phy_change_type type);
int dwc3_misc_ctrl_get(enum misc_ctrl_type type);
void dwc3_misc_ctrl_put(enum misc_ctrl_type type);
u32 usb31phy_cr_read(u32 addr);
int usb31phy_cr_write(u32 addr, u32 value);
#endif /* hisi_usb_interface.h */
