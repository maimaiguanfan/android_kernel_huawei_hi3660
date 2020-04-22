#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/ioctl.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include "huawei_platform/audio/usb_audio_power_v600.h"

#define HWLOG_TAG usb_audio_power_v600
HWLOG_REGIST();

#define USB_DISCONNECT_TIME (4000)   // DBC station: wait for usb disconnect when accept the AT command.

extern int set_otg_switch_mode_enable(void);

struct usb_audio_power_v600_data {
    struct workqueue_struct* v600_delay_wq;
    struct delayed_work v600_delay_work;
};

static struct usb_audio_power_v600_data *pdata;

static void v600_switch_work(struct work_struct* work)
{
    pd_dpm_vbus_ctrl(PLEASE_PROVIDE_POWER);
    set_otg_switch_mode_enable();
}

void set_otg_switch_enable_v600(void)
{
	if (!pdata) {
		hwlog_warn("%s pdata is NULL!\n", __func__);
		return ;
	}
	set_otg_switch_mode_enable();
}
/**
 * usb_audio_power_v600_ioctl - ioctl interface for userspeace
 *
 * @file: file description
 * @cmd: control commond
 * @arg: arguments
 *
 * userspeace can get charge status and force control
 * charge status.
 **/
static long usb_audio_power_v600_ioctl(struct file *file, unsigned int cmd,
                                                unsigned long arg)
{
    int ret = 0;

    switch (cmd) {
        case IOCTL_USB_AUDIO_POWER_BUCKBOOST_NO_HEADSET_CMD:
            queue_delayed_work(pdata->v600_delay_wq,
                                &pdata->v600_delay_work,
                                msecs_to_jiffies(USB_DISCONNECT_TIME));
            ret = 0;
            break;
        case IOCTL_USB_AUDIO_POWER_SCHARGER_CMD:
            pd_dpm_vbus_ctrl(CHARGER_TYPE_NONE);
            ret = 0;
            break;
        default:
            hwlog_err("unsupport cmd\n");
            ret = -EINVAL;
            break;
    }

    return (long)ret;
}

static const struct of_device_id usb_audio_power_v600_of_match[] = {
    {
        .compatible = "huawei,usb_audio_power_v600",
    },
    { },
};

MODULE_DEVICE_TABLE(of, usb_audio_power_v600_of_match);

static const struct file_operations usb_audio_power_v600_fops = {
    .owner            = THIS_MODULE,
    .open             = simple_open,
    .unlocked_ioctl   = usb_audio_power_v600_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl     = usb_audio_power_v600_ioctl,
#endif
};

static struct miscdevice usb_audio_power_v600_miscdev = {
    .minor =    MISC_DYNAMIC_MINOR,
    .name =     "usb_audio_power",
    .fops =     &usb_audio_power_v600_fops,
};

static int usb_audio_power_v600_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    int ret = 0;

    pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
    if (NULL == pdata) {
        hwlog_err("cannot allocate usb_audio_power_v600 data\n");
        return -ENOMEM;
    }

    pdata->v600_delay_wq = create_singlethread_workqueue("v600_delay_wq");
    if (!(pdata->v600_delay_wq)) {
        hwlog_err("%s : v600 delay work queue create failed\n", __func__);
        ret = -ENOMEM;
        goto err_out;
    }
    INIT_DELAYED_WORK(&pdata->v600_delay_work, v600_switch_work);

    ret = misc_register(&usb_audio_power_v600_miscdev);
    if (0 != ret) {
        hwlog_err("%s: can't register usb audio power miscdev, ret:%d.\n", __func__, ret);
        goto v600_delay_wq_err;
    }

    hwlog_info("usb_audio_power_v600 probe success!\n");

    return 0;

v600_delay_wq_err:
    if (pdata->v600_delay_wq) {
        cancel_delayed_work(&pdata->v600_delay_work);
        flush_workqueue(pdata->v600_delay_wq);
        destroy_workqueue(pdata->v600_delay_wq);
    }
err_out:
    devm_kfree(dev, pdata);
    pdata = NULL;
    return ret;
}

static int usb_audio_power_v600_remove(struct platform_device *pdev)
{
    if (pdata) {
        if (pdata->v600_delay_wq) {
            cancel_delayed_work(&pdata->v600_delay_work);
            flush_workqueue(pdata->v600_delay_wq);
            destroy_workqueue(pdata->v600_delay_wq);
        }

        devm_kfree(&pdev->dev, pdata);
        pdata = NULL;
    }

    misc_deregister(&usb_audio_power_v600_miscdev);
    hwlog_info("%s: exit\n", __func__);

    return 0;
}

static struct platform_driver usb_audio_power_v600_driver = {
    .driver = {
        .name   = "usb_audio_power_v600",
        .owner  = THIS_MODULE,
        .of_match_table = usb_audio_power_v600_of_match,
    },
    .probe  = usb_audio_power_v600_probe,
    .remove = usb_audio_power_v600_remove,
};

static int __init usb_audio_power_v600_init(void)
{
    return platform_driver_register(&usb_audio_power_v600_driver);
}

static void __exit usb_audio_power_v600_exit(void)
{
    platform_driver_unregister(&usb_audio_power_v600_driver);
}

fs_initcall(usb_audio_power_v600_init);
module_exit(usb_audio_power_v600_exit);

MODULE_DESCRIPTION("usb audio power v600 control driver");
MODULE_LICENSE("GPL");
