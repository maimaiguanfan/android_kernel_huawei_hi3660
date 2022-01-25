#ifndef HISI_USB_OTG_ID_H
#define HISI_USB_OTG_ID_H

/* include platform head-file */
#if defined(CONFIG_DEC_USB)
#include "dwc_otg_dec.h"
#include "dwc_otg_cil.h"
#endif


#define SAMPLE_DOING	 (0)
#define SAMPLE_DONE      (1)

#define GPIO_HIGH        (1)
#define GPIO_LOW         (0)

struct otg_gpio_id_dev {
	struct platform_device *pdev;
	struct notifier_block otg_nb;
	unsigned int otg_adc_channel;
	int gpio;
	int irq;
	unsigned int fpga_flag;
	unsigned int sampling_time_optimize;
	struct work_struct  otg_intb_work;
	bool otg_irq_enabled;
};


#define hw_usb_dbg(format, arg...)    \
	do {                 \
		printk(KERN_INFO "[USB_DEBUG][%s]"format, __func__, ##arg); \
	} while (0)
#define hw_usb_err(format, arg...)    \
	do {                 \
		printk(KERN_ERR "[USB_DEBUG]"format, ##arg); \
	} while (0)

#ifdef CONFIG_OTG_GPIO_ID
extern int hw_get_otg_id_gpio_value(int *gpio_value);
#else
static inline int hw_get_otg_id_gpio_value(int *gpio_value)
{
	return 0;
}
#endif /* CONFIG_OTG_GPIO_ID */

#endif /* HISI_USB_OTG_ID_H */

