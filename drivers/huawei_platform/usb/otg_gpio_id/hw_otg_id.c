#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/delay.h>
#include <linux/of_address.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/of_gpio.h>
#include <linux/hisi/usb/hisi_usb.h>

#include "hw_otg_id.h"

#include <linux/mfd/hisi_pmic.h>
#include <pmic_interface.h>
#include <linux/hisi/hisi_adc.h>

#ifdef CONFIG_HUAWEI_YCABLE
#include <huawei_platform/usb/hw_ycable.h>
#endif

static struct otg_gpio_id_dev *otg_gpio_id_dev_p = NULL;
static int startup_otg_sample = SAMPLE_DOING;

#define VBUS_IS_CONNECTED           0
#define DISABLE_USB_IRQ             0
#define FAIL                        (-1)
#define SAMPLING_TIME_OPTIMIZE_FLAG 1
#define SAMPLING_TIME_INTERVAL 10
#define SMAPLING_TIME_OPTIMIZE 5
#define VBATT_AVR_MAX_COUNT    10
#define ADC_VOLTAGE_LIMIT      150    //mV
#define ADC_VOLTAGE_MAX        1250   //mV
#define ADC_VOLTAGE_NEGATIVE   2000   //mV
#define USB_CHARGER_INSERTED   1
#define USB_CHARGER_REMOVE     0

int hw_get_otg_id_gpio_value(int *gpio_value)
{
	int ret = -1;

	if (otg_gpio_id_dev_p == NULL) {
		hw_usb_err("error:otg_id null\n");
		return ret;
	}

	if(gpio_value == NULL) {
		hw_usb_err("error:gpio value is null\n");
		return ret;
	}

	*gpio_value = gpio_get_value(otg_gpio_id_dev_p->gpio);

	return 0;
}

/*************************************************************************************************
*  Function:       hw_is_usb_cable_connected
*  Discription:    Check whether Vbus has been exist.
*  Parameters:   void
*  return value:  zero:     Vbus isn't exist.
*                      no zero:  Vbus is exist.
**************************************************************************************************/
static int hw_is_usb_cable_connected(void)
{
    struct otg_gpio_id_dev *dev = otg_gpio_id_dev_p;
    int ret = 0;

    if (0 == dev->fpga_flag) {
#if defined(CONFIG_DEC_USB)
        ret = (hisi_pmic_reg_read(PMIC_STATUS0_ADDR(0)) & (VBUS4P3_D10 | VBUS_COMP_VBAT_D20));
#else
        ret = hisi_pmic_get_vbus_status();
#endif
    }
	hw_usb_err("%s ret is %d !\n", __func__, ret);
    return ret;
}

/*************************************************************************************************
*  Function:       hw_otg_id_notifier_call
*  Description:    respond the charger_type events from USB PHY
*  Parameters:   usb_nb:usb notifier_block
*                      event:charger type event name
*                      data:unused
*  return value:  NOTIFY_OK-success or others
**************************************************************************************************/
static int hw_otg_id_notifier_call(struct notifier_block *usb_nb,
				    unsigned long event, void *data)
{
#if defined(CONFIG_DEC_USB)
    if (DISABLE_USB_IRQ == event && otg_gpio_id_dev_p->otg_irq_enabled) {
        hw_usb_err("%s disable the otg irq!\n", __func__);
        disable_irq_nosync(otg_gpio_id_dev_p->irq);
        otg_gpio_id_dev_p->otg_irq_enabled = false;
    } else if (DISABLE_USB_IRQ == !event && !otg_gpio_id_dev_p->otg_irq_enabled){
        hw_usb_err("%s enable the otg irq!\n", __func__);
        enable_irq(otg_gpio_id_dev_p->irq);
        otg_gpio_id_dev_p->otg_irq_enabled = true;
    } else {
        hw_usb_err("%s event %u when irq state %d\n", __func__, event, otg_gpio_id_dev_p->otg_irq_enabled);
    }
#else
    switch(event){
        case CHARGER_TYPE_NONE:
            if (!otg_gpio_id_dev_p->otg_irq_enabled) {
                hw_usb_err("%s enable the otg irq %ld!\n", __func__, event);
                enable_irq(otg_gpio_id_dev_p->irq);
                otg_gpio_id_dev_p->otg_irq_enabled = true;
            }
            break;
        case CHARGER_TYPE_SDP:
        case CHARGER_TYPE_CDP:
        case CHARGER_TYPE_DCP:
        case CHARGER_TYPE_UNKNOWN:
            if (otg_gpio_id_dev_p->otg_irq_enabled) {
                hw_usb_err("%s disable the otg irq %ld!\n", __func__, event);
                disable_irq_nosync(otg_gpio_id_dev_p->irq);
                otg_gpio_id_dev_p->otg_irq_enabled = false;
            }
            break;
        default:
            hw_usb_err("%s ignore other types %ld when irq state %d\n",
                __func__, event, otg_gpio_id_dev_p->otg_irq_enabled);
    }
#endif
    return NOTIFY_OK;
}

/*************************************************************************************************
*  Function:       hw_otg_id_adc_sampling
*  Description:    get the adc sampling value.
*  Parameters:   otg_gpio_id_dev_p: otg device ptr
*  return value:  -1: get adc channel fail
*                      other value: the adc value.
**************************************************************************************************/
static int hw_otg_id_adc_sampling(struct otg_gpio_id_dev *otg_gpio_id_dev_p)
{
    int avgvalue = 0;
	int vol_value = 0;
	int sum = 0;
    int i = 0;
    int sample_cnt = 0;

	/*get adc channel*/
	if (0 == otg_gpio_id_dev_p->otg_adc_channel){
		hw_usb_err("%s Get otg_adc_channel is fail!\n", __func__);
		return FAIL;
	}

	for(i = 0; i < VBATT_AVR_MAX_COUNT; i++){
		vol_value = hisi_adc_get_value(otg_gpio_id_dev_p->otg_adc_channel);
		/*add some interval*/
		if (SAMPLING_TIME_OPTIMIZE_FLAG == otg_gpio_id_dev_p->sampling_time_optimize && SAMPLE_DOING == startup_otg_sample) {
			udelay(SMAPLING_TIME_OPTIMIZE);
		} else {
			msleep(SAMPLING_TIME_INTERVAL);
		}
		if(vol_value < 0){
			hw_usb_err("%s The value from ADC is error", __func__);
			continue;
		}
		sum += vol_value;
		sample_cnt++;
	}

	if (0 == sample_cnt) {
		/* ESD cause the sample is always negative */
		avgvalue = ADC_VOLTAGE_NEGATIVE;
	}else {
	    avgvalue = sum/sample_cnt;
	}
	hw_usb_err("%s The average voltage of ADC is %d\n", __func__, avgvalue);

	return avgvalue;
}


/*************************************************************************************************
*  Function:       hw_otg_id_intb_work
*  Discription:    Handle GPIO about OTG.
*  Parameters:   work: The work struct of otg.
*  return value:  void
**************************************************************************************************/
static void hw_otg_id_intb_work(struct work_struct *work)
{
	int gpio_value = -1;
	int avgvalue = 0;
	static bool is_otg_has_inserted = false;

	/* Fix the different of schager V200 and V300 */
	if (!is_otg_has_inserted) {
		if (VBUS_IS_CONNECTED == !hw_is_usb_cable_connected()) {
			hw_usb_err("%s Vbus is inerted!\n", __func__);
#ifdef CONFIG_HUAWEI_YCABLE
			avgvalue = hw_otg_id_adc_sampling(otg_gpio_id_dev_p);
			if (!ycable_is_charge_connect(avgvalue)) {
				return;
			}
#else
			return;
#endif
		}
	}

	gpio_value = gpio_get_value(otg_gpio_id_dev_p->gpio);
	if (0 == gpio_value){
		hw_usb_err("%s Send ID_FALL_EVENT\n", __func__);
		avgvalue = hw_otg_id_adc_sampling(otg_gpio_id_dev_p);

		if((avgvalue >= 0) && (avgvalue <= ADC_VOLTAGE_LIMIT)){
			is_otg_has_inserted = true;
			hisi_usb_otg_event(ID_FALL_EVENT);
#ifdef CONFIG_HUAWEI_YCABLE
			ycable_handle_otg_event(ID_FALL_EVENT, !YCABLE_NEED_WAIT);
#endif
		}
		else {
			hw_usb_err("%s avgvalue is %d.\n", __func__, avgvalue);
			is_otg_has_inserted = true;
#ifdef CONFIG_HUAWEI_YCABLE
			if (ycable_is_charge_connect(avgvalue)) {
				/* last cable plug out will call completion, so here reinit it */
				ycable_init_devoff_completion();
				hisi_usb_otg_event(CHARGER_DISCONNECT_EVENT);
				hisi_usb_otg_event(ID_FALL_EVENT);
				ycable_handle_otg_event(ID_FALL_EVENT, YCABLE_NEED_WAIT);
			}
#endif
		}
	} else {
		hw_usb_err("%s Send ID_RISE_EVENT\n", __func__);
		is_otg_has_inserted = false;
		hisi_usb_otg_event(ID_RISE_EVENT);
	}

	return;
}

/*************************************************************************************************
*  Function:       hw_otg_id_irq_handle
*  Discription:    Handle the GPIO200 irq.
*  Parameters:   irq: The number of irq for GPIO200
*                      dev_id: The id of devices.
*  return value:  IRQ_HANDLED
**************************************************************************************************/
static irqreturn_t hw_otg_id_irq_handle(int irq, void *dev_id)
{
	int gpio_value = -1;

	disable_irq_nosync(otg_gpio_id_dev_p->irq);
    gpio_value = gpio_get_value(otg_gpio_id_dev_p->gpio);

    if (0 == gpio_value){
    	irq_set_irq_type(otg_gpio_id_dev_p->irq, IRQF_TRIGGER_HIGH | IRQF_NO_SUSPEND);
    }
	else {
		irq_set_irq_type(otg_gpio_id_dev_p->irq, IRQF_TRIGGER_LOW | IRQF_NO_SUSPEND);
	}
	enable_irq(otg_gpio_id_dev_p->irq);

	schedule_work(&otg_gpio_id_dev_p->otg_intb_work);
    return IRQ_HANDLED;
}

/*************************************************************************************************
*  Function:       hw_otg_id_probe
*  Discription:    otg probe function.
*  Parameters:   pdev: The porinter of pdev
*  return value:  IRQ_HANDLED
**************************************************************************************************/
static int hw_otg_id_probe(struct platform_device *pdev)
{
    int ret = 0;
    int avgvalue = 0;
    int adc_voltage_min = ADC_VOLTAGE_LIMIT;
    struct device_node *np = NULL;
    struct device* dev = NULL;

    hw_usb_err("Enter %s funtion\n", __func__);

    if (NULL == pdev) {
        hw_usb_err("%s The pdev is NULL !\n", __func__);
        return ret;
    }

    otg_gpio_id_dev_p = (struct otg_gpio_id_dev *)devm_kzalloc(&pdev->dev, sizeof(*otg_gpio_id_dev_p), GFP_KERNEL);
    if (NULL == otg_gpio_id_dev_p) {
        hw_usb_err("%s alloc otg_dev failed! not enough memory\n", __func__);
        return ret;
    }
    platform_set_drvdata(pdev, otg_gpio_id_dev_p);
    otg_gpio_id_dev_p->pdev = pdev;
    otg_gpio_id_dev_p->otg_irq_enabled = true;
    otg_gpio_id_dev_p->otg_nb.notifier_call = hw_otg_id_notifier_call;
#if defined(CONFIG_DEC_USB)
    ret = hisi_usb_otg_irq_notifier_register(&otg_gpio_id_dev_p->otg_nb);
#else
    ret = hisi_charger_type_notifier_register(&otg_gpio_id_dev_p->otg_nb);
#endif
    if (ret < 0) {
        hw_usb_err("%s hisi_usb_otg_irq_notifier_register failed\n", __func__);
        goto err_notifier_register;
    }

    dev = &pdev->dev;
    np = dev->of_node;

    ret = of_property_read_u32(np, "sampling_time_optimize", &(otg_gpio_id_dev_p->sampling_time_optimize));
    if (0 != ret) {
        hw_usb_err("%s Get sampling_time_optimize failed !!! \n", __func__);
    }

    ret = of_property_read_u32(np, "otg_adc_channel", &(otg_gpio_id_dev_p->otg_adc_channel));
    if (0 != ret) {
        hw_usb_err("%s Get otg_adc_channel failed !!! \n", __func__);
        goto err_property_read_u32;
    }

	ret = of_property_read_u32(np, "fpga_flag", &(otg_gpio_id_dev_p->fpga_flag));
    if (0 != ret) {
        otg_gpio_id_dev_p->fpga_flag = 0;
        hw_usb_err("%s in asic mode!\n", __func__);
    }

    otg_gpio_id_dev_p->gpio = of_get_named_gpio(np, "otg-gpio", 0);
    if (otg_gpio_id_dev_p->gpio < 0) {
        hw_usb_err("%s of_get_named_gpio error!!! gpio=%d.\n", __func__, otg_gpio_id_dev_p->gpio);
        goto err_of_get_named_gpio;
    }

    /*init otg intr handle work funtion*/
    INIT_WORK(&otg_gpio_id_dev_p->otg_intb_work, hw_otg_id_intb_work);

    /*
     * init otg gpio process
     */
    ret = gpio_request(otg_gpio_id_dev_p->gpio, "otg_gpio_irq");
    if (ret < 0) {
        hw_usb_err("%s gpio_request error!!! ret=%d. gpio=%d.\n", __func__, ret, otg_gpio_id_dev_p->gpio);
        goto err_gpio_request;
    }

    avgvalue = hw_otg_id_adc_sampling(otg_gpio_id_dev_p);
    startup_otg_sample = SAMPLE_DONE;
#ifdef CONFIG_HUAWEI_YCABLE
    if (ycable_is_support()) {
        adc_voltage_min = ycable_get_gpio_adc_min();
    }
#endif
    if ((avgvalue > adc_voltage_min) && (avgvalue <= ADC_VOLTAGE_MAX)) {
        hw_usb_err("%s Set gpio_direction_output, avgvalue is %d.\n", __func__, avgvalue);
        ret = gpio_direction_output(otg_gpio_id_dev_p->gpio,1);
    } else {
        ret = gpio_direction_input(otg_gpio_id_dev_p->gpio);
        if (ret < 0) {
            hw_usb_err("%s gpio_direction_input error!!! ret=%d. gpio=%d.\n", __func__, ret, otg_gpio_id_dev_p->gpio);
            goto err_detect_otg_id;
        }
    }

    otg_gpio_id_dev_p->irq = gpio_to_irq(otg_gpio_id_dev_p->gpio);
    if (otg_gpio_id_dev_p->irq < 0) {
        hw_usb_err("%s gpio_to_irq error!!! dev_p->gpio=%d, dev_p->irq=%d.\n", __func__, otg_gpio_id_dev_p->gpio, otg_gpio_id_dev_p->irq);
        goto err_gpio_to_irq;
    } else {
        hw_usb_err("%s otg irq is %d.\n", __func__, otg_gpio_id_dev_p->irq);
        if (0 == !hw_is_usb_cable_connected()) {
            hw_otg_id_notifier_call(NULL, !USB_CHARGER_INSERTED, NULL);
        }
        else {
            hw_otg_id_notifier_call(NULL, !USB_CHARGER_REMOVE, NULL);
        }
    }

    ret = request_irq(otg_gpio_id_dev_p->irq, hw_otg_id_irq_handle,
	    	IRQF_TRIGGER_LOW | IRQF_NO_SUSPEND | IRQF_ONESHOT, "otg_gpio_irq", NULL);
    if (ret < 0) {
        hw_usb_err("%s request otg irq handle funtion fail!! ret:%d\n", __func__, ret);
        goto err_request_irq;
    } else {
        if (VBUS_IS_CONNECTED == !hw_is_usb_cable_connected()) {
#if defined(CONFIG_DEC_USB)
            hw_otg_id_notifier_call(NULL, !USB_CHARGER_INSERTED, NULL);
#else
            hw_otg_id_notifier_call(NULL, CHARGER_TYPE_SDP, NULL);
#endif
        }
    }

    /* check the otg status when the phone poweron*/
    ret = gpio_get_value(otg_gpio_id_dev_p->gpio);
    if(0 == ret){
      /*call work function to handle irq*/
      schedule_work(&otg_gpio_id_dev_p->otg_intb_work);
    }

    hw_usb_err("Exit [%s]-\n", __func__);

    return 0;

	err_notifier_register:
		devm_kfree(&pdev->dev, otg_gpio_id_dev_p);
		otg_gpio_id_dev_p = NULL;
		return 0;
	err_property_read_u32:
	err_of_get_named_gpio:
	err_gpio_request:
#if defined(CONFIG_DEC_USB)
		hisi_usb_otg_irq_notifier_unregister(&otg_gpio_id_dev_p->otg_nb);
#else
		hisi_charger_type_notifier_unregister(&otg_gpio_id_dev_p->otg_nb);
#endif
		devm_kfree(&pdev->dev, otg_gpio_id_dev_p);
		otg_gpio_id_dev_p = NULL;
		return 0;
	err_detect_otg_id:
	err_gpio_to_irq:
	err_request_irq:
#if defined(CONFIG_DEC_USB)
		hisi_usb_otg_irq_notifier_unregister(&otg_gpio_id_dev_p->otg_nb);
#else
		hisi_charger_type_notifier_unregister(&otg_gpio_id_dev_p->otg_nb);
#endif
		gpio_free(otg_gpio_id_dev_p->gpio);
		devm_kfree(&pdev->dev, otg_gpio_id_dev_p);
		otg_gpio_id_dev_p = NULL;
		return 0;
}

/*************************************************************************************************
*  Function:       hw_otg_id_remove
*  Description:   otg remove
*  Parameters:   pdev:platform_device
*  return value:  0-sucess or others-fail
**************************************************************************************************/
static int hw_otg_id_remove(struct platform_device *pdev)
{
	if ((NULL == otg_gpio_id_dev_p) || (NULL == pdev)) {
		hw_usb_err("%s otg_gpio_id_dev_p or pdev is NULL!\n", __func__);
		return -ENODEV;
	}

#if defined(CONFIG_DEC_USB)
	hisi_usb_otg_irq_notifier_unregister(&otg_gpio_id_dev_p->otg_nb);
#else
	hisi_charger_type_notifier_unregister(&otg_gpio_id_dev_p->otg_nb);
#endif
	free_irq(otg_gpio_id_dev_p->irq, pdev);
    gpio_free(otg_gpio_id_dev_p->gpio);
    devm_kfree(&pdev->dev, otg_gpio_id_dev_p);
	otg_gpio_id_dev_p = NULL;
	return 0;
}


static struct of_device_id hw_otg_id_of_match[] = {
    { .compatible = "huawei,usbotg-by-id", },
    { },
};

static struct platform_driver hw_otg_id_drv = {
	.probe		= hw_otg_id_probe,
	.remove     = hw_otg_id_remove,
	.driver		= {
		.owner		= THIS_MODULE,
		.name		= "hw_otg_id",
		.of_match_table	= hw_otg_id_of_match,
	},
};

static int __init hw_otg_id_init(void)
{
    int ret = 0;

    ret = platform_driver_register(&hw_otg_id_drv);
    hw_usb_err("Enter [%s] function, ret is %d\n", __func__, ret);
    return ret;
}

static void __exit hw_otg_id_exit(void)
{
    platform_driver_unregister(&hw_otg_id_drv);
    return ;
}

device_initcall_sync(hw_otg_id_init);
module_exit(hw_otg_id_exit);

MODULE_AUTHOR("huawei");
MODULE_DESCRIPTION("This module detect USB OTG connection/disconnection");
MODULE_LICENSE("GPL v2");

