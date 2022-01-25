/*
 * drivers/power/smpl/smpl.c
 * Copyright (C) 2012-2015 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/wakelock.h>
#include <linux/mfd/hisi_pmic_mntn.h>
#include <linux/hisi/usb/hisi_usb.h>
#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#include <huawei_platform/power/huawei_charger.h>

#define HWLOG_TAG smpl
HWLOG_REGIST();

struct smpl_device_info {
    struct device   *dev;
    struct delayed_work smpl_delayed_work;

    //for vsys_pwroff_abs_pd disable/enable when insert or pull out charger
    struct notifier_block usb_nb;
    struct work_struct config_abs_pd_work;
    atomic_t in_charging;
    struct wake_lock abs_pd_wakelock;
};

static struct smpl_device_info* g_smpl_devinfo = NULL;

static unsigned int smpl_happened = 0;
static int __init early_parse_normal_reset_type_cmdline(char * p)
{
	if (!p) {
		hwlog_err("cmdline is null\n");
		return 0;
	}

	hwlog_info("normal_reset_type=%s\n", p);

	/* AP_S_SMPL = 0x51  BR_POWERON_BY_SMPL = 0x26 */
	if (strstr(p, "SMPL"))
		smpl_happened = 1;

	hwlog_info("smpl happened=%d\n", smpl_happened);
	return 0;
}
early_param("normal_reset_type", early_parse_normal_reset_type_cmdline);

static void smpl_work(struct work_struct *work)
{
    int batt_vol = 0;
    int batt_temp = INVALID_TEMP_VAL;
    int batt_ufsoc = -1;
    int batt_afsoc = -1;
    int batt_soc = -1;

    if(smpl_happened)
    {
        if (!dsm_client_ocuppy(power_dsm_get_dclient(POWER_DSM_SMPL)))
        {
            batt_vol = hisi_battery_voltage();
            batt_temp = hisi_battery_temperature();
            batt_ufsoc = hisi_battery_unfiltered_capacity();
            batt_afsoc = hisi_battery_capacity();
            batt_soc = hisi_bci_show_capacity();

            hwlog_info("smpl record and notify\n");
            dsm_client_record(power_dsm_get_dclient(POWER_DSM_SMPL), "smpl happened,vbatt = %d,temp = %d,ufsoc = %d,afsoc = %d,soc = %d\n",
                    batt_vol,batt_temp,batt_ufsoc,batt_afsoc,batt_soc);
            dsm_client_notify(power_dsm_get_dclient(POWER_DSM_SMPL), ERROR_NO_SMPL);
        }
    }
}

static int lock_abs_pd_wakelock(void)
{
    if( g_smpl_devinfo )
    {
        wake_lock(&g_smpl_devinfo->abs_pd_wakelock);
    }
    return 0;
}

static int unlock_abs_pd_wakelock(void)
{
    if( g_smpl_devinfo )
    {
        wake_unlock(&g_smpl_devinfo->abs_pd_wakelock);
    }
    return 0;
}

static void config_pwroff_abs_pd_work(struct work_struct *work)
{
    int ret = 0;
    bool enable = false;
    struct smpl_device_info* smpl_devinfo = container_of(work, struct smpl_device_info, config_abs_pd_work);
    if( atomic_read(&smpl_devinfo->in_charging) )
    {
        //in charge, disable pwroff_abs_pd
        enable = false;
        hwlog_info("%s %d charger insert, enable pwroff_abs_pd, config enable:%d\n", __func__, __LINE__, enable);
    }
    else
    {
        //not in charge, enable pwroff_abs_pd
        enable = true;
        hwlog_info("%s %d charger pull out, enable pwroff_abs_pd, config enable:%d\n", __func__, __LINE__, enable);
    }

    ret = hisi_pmic_mntn_config_vsys_pwroff_abs_pd(enable);
    if(ret < 0 )
    {
        hwlog_info("%s %d hisi_pmic_mntn_config_vsys_pwroff_abs_pd fail, ret:%d\n", __func__, __LINE__, ret);
    }
    else
    {
        ret = hisi_pmic_mntn_vsys_pwroff_abs_pd_state();
        hwlog_info("%s %d hisi_pmic_mntn_config_vsys_pwroff_abs_pd success, read state:%d, config state:%d\n", __func__, __LINE__, ret, enable);
    }
    unlock_abs_pd_wakelock();
}

static int charge_usb_notifier_call(struct notifier_block *usb_nb, unsigned long event, void *data)
{
    bool valid_event = false;

    struct smpl_device_info* smpl_devinfo = container_of(usb_nb, struct smpl_device_info, usb_nb);

    switch ((enum hisi_charger_type)event)
    {
        case CHARGER_TYPE_SDP:
        case CHARGER_TYPE_CDP:
        case CHARGER_TYPE_DCP:
        case CHARGER_TYPE_UNKNOWN:
            atomic_set(&smpl_devinfo->in_charging, 1);
            valid_event = true;
            break;
        case CHARGER_TYPE_NONE:
            atomic_set(&smpl_devinfo->in_charging, 0);
            valid_event = true;
            break;
        default:
            valid_event = false;
            break;
    }

    hwlog_info("%s %d receive usb event:%lu, in_charging:%d, valid_event:%d\n", __func__, __LINE__, event, atomic_read(&smpl_devinfo->in_charging), valid_event);

    if( true == valid_event )
    {
        //1. get wake lock
        lock_abs_pd_wakelock();

        hwlog_info("%s %d schedule_work config_pwroff_abs_pd_work to config vsys_pwroff_abs_pd\n", __func__, __LINE__);

        //2. queue the work to check changer status and charging state
        schedule_work(&smpl_devinfo->config_abs_pd_work);
    }

    return NOTIFY_OK;
}

static int smpl_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct device_node* np;
    struct smpl_device_info* di;
    enum hisi_charger_type charger = CHARGER_TYPE_NONE;

    np = pdev->dev.of_node;
    if(NULL == np)
    {
        hwlog_err("np is NULL\n");
        return -1;
    }
    di = kzalloc(sizeof(*di), GFP_KERNEL);
    if (!di)
    {
        hwlog_err("di is NULL\n");
        return -ENOMEM;
    }

    INIT_DELAYED_WORK(&di->smpl_delayed_work, smpl_work);
    schedule_delayed_work(&di->smpl_delayed_work, 10);

    //add for sys_pwroff_abs_pd enable/disable
    INIT_WORK(&di->config_abs_pd_work, config_pwroff_abs_pd_work);
    g_smpl_devinfo = di;

    wake_lock_init(&di->abs_pd_wakelock, WAKE_LOCK_SUSPEND, "abs_pd_wakelock");

    di->usb_nb.notifier_call = charge_usb_notifier_call;
    ret = hisi_charger_type_notifier_register(&di->usb_nb);
    if (ret < 0)
    {
		wake_lock_destroy(&di->abs_pd_wakelock);
        g_smpl_devinfo = NULL;
        hwlog_err("%s %d hisi_charger_type_notifier_register failed.\n", __func__, __LINE__);
        kfree(di);
        di = NULL;
        return ret;
    }

    charger = hisi_get_charger_type();
    hwlog_info("%s %d hisi_get_charger_type return:%d\n", __func__, __LINE__, charger);
    if( CHARGER_TYPE_NONE != charger )
    {
        charge_usb_notifier_call(&di->usb_nb, charger, g_smpl_devinfo);
    }

    hwlog_info("smpl probe ok!\n");

    return 0;
}

static int smpl_remove(struct platform_device *pdev)
{
    int ret = 0;

    hwlog_info("%s %d enter++\n", __func__, __LINE__);

    if( NULL== g_smpl_devinfo )
    {
        hwlog_info("g_smpl_devinfo is NULL\n");
        goto out;
    }

	wake_lock_destroy(&g_smpl_devinfo->abs_pd_wakelock);

    ret = hisi_charger_type_notifier_unregister(&g_smpl_devinfo->usb_nb);
    if( ret )
    {
        hwlog_err("%s %d hisi_charger_type_notifier_unregister fail, ret:%d\n", __func__, __LINE__, ret);
        goto out;
    }

    flush_work(&g_smpl_devinfo->config_abs_pd_work);

    kfree(g_smpl_devinfo);
    g_smpl_devinfo = NULL;
out:
    hwlog_info("%s %d enter--\n", __func__, __LINE__);
    return ret;
}


static struct of_device_id smpl_match_table[] =
{
    {
        .compatible = "huawei,smpl",
        .data = NULL,
    },
    {
    },
};
static struct platform_driver smpl_driver = {
    .probe = smpl_probe,
    .remove = smpl_remove,
    .driver = {
        .name = "huawei,smpl",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(smpl_match_table),
    },
};

static int __init smpl_init(void)
{
    return platform_driver_register(&smpl_driver);
}

module_init(smpl_init);

static void __exit smpl_exit(void)
{
    platform_driver_unregister(&smpl_driver);
}

module_exit(smpl_exit);

MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:smpl");
MODULE_AUTHOR("HUAWEI Inc");
