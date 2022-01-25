


#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>

#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"
#include "../pmic/hw_pmic.h"

#define I2S(i) container_of(i, sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)
#define CTL_RESET_HOLD    (0)
#define CTL_RESET_RELEASE (1)

static struct sensor_power_setting imx386hybird_power_setting [] = {
    //M0 AVDD0  2.80V  [LDO19]
    {
        .seq_type = SENSOR_AVDD0,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M1 AVDD1  2.80V  [CAM_PMIC_LDO3]
    {
        .seq_type = SENSOR_AVDD1,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M1 AVDD1  1.80V  [CAM_PMIC_LDO5]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_5,
        .config_val = PMIC_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    // DVDD BUCK_1 1.125v
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_BUCK_1,
        .config_val = PMIC_1P125V,  // 1.125v
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //DVDD2  1p05V  [LDO20]
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P05V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //DRVVDD 2.85V [PMIC_LDO4]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val  = VOUT_LDO_4,
        .config_val = PMIC_2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M0  VCM  3V  [PMIC BUCK2]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val  = VOUT_BUCK_2,
        .config_val = PMIC_2P9V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M0 + M1  IOVDD  1.8V  [CAM_PMIC_LDO1]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_1,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 0,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 2,
        .delay = 1,
    },

    //M0 RESET  [GPIO_018]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //M1 RESET  [GPIO_17]
    {
        .seq_type = SENSOR_RST2,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

static struct sensor_power_setting imx386hybird_lon_power_setting[] = {
    //M0 AVDD0  2.80V  [LDO19]
    {
        .seq_type = SENSOR_AVDD0,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M1 AVDD1  2.80V  [CAM_PMIC_LDO3]
    {
        .seq_type = SENSOR_AVDD1,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M1 AVDD1  1.80V  [CAM_PMIC_LDO5]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_5,
        .config_val = PMIC_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    // DVDD BUCK_1 1.125v
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_BUCK_1,
        .config_val = PMIC_1P125V,  // 1.125v
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M0  VCM  3V  [PMIC BUCK2]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val  = VOUT_BUCK_2,
        .config_val = PMIC_2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //DRVVDD 2.85V [PMIC_LDO4]
    {
        .seq_type = SENSOR_VCM_AVDD2,
        .config_val = LDO_VOLTAGE_V2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M0 + M1  IOVDD  1.8V  [CAM_PMIC_LDO1]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_1,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 0,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 2,
        .delay = 1,
    },

    //M0 RESET  [GPIO_018]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //M1 RESET  [GPIO_17]
    {
        .seq_type = SENSOR_RST2,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

static struct sensor_power_setting imx386hybird_udp_power_setting[] = {
    //enable gpio51 output iovdd 1.8v
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_2,
        .config_val = PMIC_2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M1 AVDD0  2.80V  [CAM_PMIC_LDO3]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_3,
        .config_val = PMIC_2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M1 AVDD1  1.80V  [CAM_PMIC_LDO3]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_5,
        .config_val = PMIC_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    // DVDD BUCK_1 1.15v
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_BUCK_1,
        .config_val = PMIC_1P15V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M0  VCM  3V  [PMIC BUCK2]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val  = VOUT_BUCK_2,
        .config_val = PMIC_3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //DRVVDD 2.85V [PMIC_LDO4]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val  = VOUT_LDO_4,
        .config_val = PMIC_2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //enable gpio51 output iovdd 1.8v
    {
        .seq_type = SENSOR_LDO_EN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 0,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 2,
        .delay = 1,
    },

    // M0 RESET  [GPIO_052]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_RST2,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

static struct sensor_power_setting imx386hybird_power_setting_ab [] = {
    //AFVDD GPIO 004 ENABLE
    // Power up vcm first, then power up AVDD1
    // XBUCK_AFVDD 2V55---| bucker |---> AVDD1 1V8
    {
        .seq_type = SENSOR_VCM_PWDN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M0 OIS DRV 2.85 [ldo25]
    {
        .seq_type = SENSOR_OIS_DRV,
        .config_val = LDO_VOLTAGE_V2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M0+M1 IOVDD 1.8V [ld021]
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M0 AVDD0 2.8v [ldo33]
    {
        .seq_type = SENSOR_AVDD0,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M1 AVDD 2.8v [ldo13]
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M1 AVDD1 gpio 008 VBUCK3(2v55) -> 1V8
    {
        .seq_type = SENSOR_AVDD1_EN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    // M0 DVDD0 [ldo19] 1v2
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    // M1 DVDD1 [ldo20] 1v1
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 0,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 2,
        .delay = 1,
    },
    //M0 RESET  [GPIO_013]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //M1 RESET  [GPIO_136]
    {
        .seq_type = SENSOR_RST2,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

static char const*
imx386hybird_get_name(
        hwsensor_intf_t* si)
{
    sensor_t* sensor = I2S(si);
    return sensor->board_info->name;
}

static int
imx386hybird_power_up(
        hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    if (hw_is_fpga_board()) {
        ret = do_sensor_power_on(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_up(sensor);
    }
    if (0 == ret )
    {
        cam_info("%s. power up sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power up sensor fail.", __func__);
    }
    return ret;
}

static int
imx386hybird_power_down(
        hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    if (hw_is_fpga_board()) {
        ret = do_sensor_power_off(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_down(sensor);
    }
    if (0 == ret )
    {
        cam_info("%s. power down sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power down sensor fail.", __func__);
    }
    return ret;
}

static int imx386hybird_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

static int imx386hybird_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int
imx386hybird_match_id(
        hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = I2S(si);
    struct sensor_cfg_data *cdata = (struct sensor_cfg_data *)data;

    cam_info("%s name:%s", __func__, sensor->board_info->name);

    strncpy(cdata->cfg.name, sensor->board_info->name, DEVICE_NAME_SIZE-1);
    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

enum camera_metadata_enum_android_hw_dual_primary_mode{
    ANDROID_HW_DUAL_PRIMARY_FIRST  = 0,
    ANDROID_HW_DUAL_PRIMARY_SECOND = 2,
    ANDROID_HW_DUAL_PRIMARY_BOTH   = 3,
};


static int imx386hybird_do_hw_reset(hwsensor_intf_t* si, int ctl, int id)
{
//lint -save -e826 -e778 -e774 -e747
    sensor_t* sensor = I2S(si);
    hwsensor_board_info_t *b_info;
    int ret;

    b_info = sensor->board_info;
    if (NULL == b_info) {
        cam_warn("%s invalid sensor board info", __func__);
        return 0;
    }
    ret = gpio_request(b_info->gpios[RESETB].gpio, "imx386reset-0");
    if (ret) {
        cam_err("%s requeset reset-0 pin failed", __func__);
        return ret;
    }
    ret = gpio_request(b_info->gpios[RESETB2].gpio, "imx386reset-1");
    if (ret) {
        gpio_free(b_info->gpios[RESETB].gpio);
        cam_err("%s requeset reset-1 pin failed", __func__);
        return ret;
    }
    if(CTL_RESET_HOLD == ctl) {
            ret |= gpio_direction_output(b_info->gpios[RESETB].gpio,CTL_RESET_HOLD);
            ret |= gpio_direction_output(b_info->gpios[RESETB2].gpio, CTL_RESET_HOLD);
            cam_info("RESETB = CTL_RESET_HOLD, RESETB2 = CTL_RESET_HOLD");
            hw_camdrv_msleep(1);
    } else if (CTL_RESET_RELEASE == ctl) {
        if(id == ANDROID_HW_DUAL_PRIMARY_FIRST) {
            ret |= gpio_direction_output(b_info->gpios[RESETB].gpio, CTL_RESET_RELEASE);
            ret |= gpio_direction_output(b_info->gpios[RESETB2].gpio, CTL_RESET_HOLD);
            cam_info("RESETB = CTL_RESET_RELEASE, RESETB2 = CTL_RESET_HOLD");
        }else if (id == ANDROID_HW_DUAL_PRIMARY_BOTH) {
            ret |= gpio_direction_output(b_info->gpios[RESETB].gpio, CTL_RESET_RELEASE);
            ret |= gpio_direction_output(b_info->gpios[RESETB2].gpio, CTL_RESET_RELEASE);
            cam_info("RESETB = CTL_RESET_RELEASE, RESETB2 = CTL_RESET_RELEASE");
        }else if (id == ANDROID_HW_DUAL_PRIMARY_SECOND) {
            ret |= gpio_direction_output(b_info->gpios[RESETB2].gpio, CTL_RESET_RELEASE);
            ret |= gpio_direction_output(b_info->gpios[RESETB].gpio, CTL_RESET_HOLD);
            cam_info("RESETB = CTL_RESET_HOLD, RESETB2 = CTL_RESET_RELEASE");
        }
        hw_camdrv_msleep(2);
    }
    gpio_free(b_info->gpios[RESETB].gpio);
    gpio_free(b_info->gpios[RESETB2].gpio);
    if (ret) {
        cam_err("%s set reset pin failed", __func__);
    } else {
        cam_info("%s: set reset state=%d, mode=%d", __func__, ctl, id);
    }
//lint -restore
    return ret;
}



static int
imx386hybird_config(
        hwsensor_intf_t* si,
        void  *argp)
{
    struct sensor_cfg_data *data;
    int ret =0;

	if (NULL == si || NULL == argp){
		cam_err("%s : si or argp is null", __func__);
		return -1;
	}

    data = (struct sensor_cfg_data *)argp;
    cam_debug("imx386hybird cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            ret = si->vtbl->power_up(si);
            break;
        case SEN_CONFIG_POWER_OFF:
            ret = si->vtbl->power_down(si);
            break;
        case SEN_CONFIG_WRITE_REG:
            break;
        case SEN_CONFIG_READ_REG:
            break;
        case SEN_CONFIG_WRITE_REG_SETTINGS:
            break;
        case SEN_CONFIG_READ_REG_SETTINGS:
            break;
        case SEN_CONFIG_ENABLE_CSI:
            break;
        case SEN_CONFIG_DISABLE_CSI:
            break;
        case SEN_CONFIG_MATCH_ID:
            ret = si->vtbl->match_id(si,argp);
            break;
        case SEN_CONFIG_RESET_HOLD:
            ret = imx386hybird_do_hw_reset(si, CTL_RESET_HOLD, data->mode);
            break;
        case SEN_CONFIG_RESET_RELEASE:
            ret = imx386hybird_do_hw_reset(si, CTL_RESET_RELEASE, data->mode);
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }
    return ret;
}

static hwsensor_vtbl_t s_imx386hybird_vtbl =
{
    .get_name = imx386hybird_get_name,
    .config = imx386hybird_config,
    .power_up = imx386hybird_power_up,
    .power_down = imx386hybird_power_down,
    .match_id = imx386hybird_match_id,
    .csi_enable = imx386hybird_csi_enable,
    .csi_disable = imx386hybird_csi_disable,
};
/* individual driver data for each device */

static sensor_t s_imx386hybird =
{
    .intf = { .vtbl = &s_imx386hybird_vtbl, },
    .power_setting_array = {
        .size = ARRAY_SIZE(imx386hybird_power_setting),
        .power_setting = imx386hybird_power_setting,
    },
};

static sensor_t s_imx386hybird_lon =
{
    .intf = { .vtbl = &s_imx386hybird_vtbl, },
    .power_setting_array = {
        .size = ARRAY_SIZE(imx386hybird_lon_power_setting),
        .power_setting = imx386hybird_lon_power_setting,
    },
};

static sensor_t s_imx386hybird_udp =
{
    .intf = { .vtbl = &s_imx386hybird_vtbl, },
    .power_setting_array = {
        .size = ARRAY_SIZE(imx386hybird_udp_power_setting),
        .power_setting = imx386hybird_udp_power_setting,
    },
};

static sensor_t s_imx386hybird_ab =
{
    .intf = { .vtbl = &s_imx386hybird_vtbl, },
    .power_setting_array = {
        .size = ARRAY_SIZE(imx386hybird_power_setting_ab),
        .power_setting = imx386hybird_power_setting_ab,
    },
};

/* support both imx386hybird & imx386legacydual */
static const struct of_device_id
s_imx386hybird_dt_match[] =
{
    {
        .compatible = "huawei,imx386hybird",
        .data = &s_imx386hybird.intf,
    },
    {
        .compatible = "huawei,imx386hybird_lon",
        .data = &s_imx386hybird_lon.intf,
    },
    {
        .compatible = "huawei,imx386hybird_udp",
        .data = &s_imx386hybird_udp.intf,
    },
    {
        .compatible = "huawei,imx386hybird_ab",
        .data = &s_imx386hybird_ab.intf,
    },
    { } /* terminate list */
};
MODULE_DEVICE_TABLE(of, s_imx386hybird_dt_match);
/* platform driver struct */
static int32_t imx386hybird_platform_probe(struct platform_device* pdev);
static int32_t imx386hybird_platform_remove(struct platform_device* pdev);
static struct platform_driver
s_imx386hybird_driver =
{
    .probe = imx386hybird_platform_probe,
    .remove = imx386hybird_platform_remove,
    .driver =
    {
        .name = "huawei,imx386hybird",
        .owner = THIS_MODULE,
        .of_match_table = s_imx386hybird_dt_match,
    },
};


static int32_t
imx386hybird_platform_probe(
        struct platform_device* pdev)
{
    int rc = 0;
    struct device_node *np = pdev->dev.of_node;
    const struct of_device_id *id;
    hwsensor_intf_t *intf;
    sensor_t *sensor;

    cam_info("enter %s gal",__func__);

    if (!np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_imx386hybird_dt_match, np);
    if (!id) {
        cam_err("%s none id matched", __func__);
        return -ENODEV;
    }

    intf = (hwsensor_intf_t*)id->data;
    sensor = I2S(intf);
    rc = hw_sensor_get_dt_data(pdev, sensor);
    if (rc < 0) {
        cam_err("%s no dt data", __func__);
        return -ENODEV;
    }
    sensor->dev = &pdev->dev;

    rc = hwsensor_register(pdev, intf);
    rc = rpmsg_sensor_register(pdev, (void*)sensor);

    return rc;
}

static int32_t
imx386hybird_platform_remove(
    struct platform_device * pdev)
{
    struct device_node *np = pdev->dev.of_node;
    const struct of_device_id *id;
    hwsensor_intf_t *intf;
    sensor_t *sensor;

    cam_info("enter %s",__func__);

    if (!np) {
        cam_info("%s of_node is NULL", __func__);
        return 0;
    }
    /* don't use dev->p->driver_data
     * we need to search again */
    id = of_match_node(s_imx386hybird_dt_match, np);
    if (!id) {
        cam_info("%s none id matched", __func__);
        return 0;
    }

    intf = (hwsensor_intf_t*)id->data;
    sensor = I2S(intf);

    rpmsg_sensor_unregister((void*)&sensor);
    hwsensor_unregister(pdev);
    return 0;
}
static int __init
imx386hybird_init_module(void)
{
    cam_info("enter %s",__func__);
    return platform_driver_probe(&s_imx386hybird_driver,
            imx386hybird_platform_probe);
}

static void __exit
imx386hybird_exit_module(void)
{
    platform_driver_unregister(&s_imx386hybird_driver);
}

module_init(imx386hybird_init_module);
module_exit(imx386hybird_exit_module);
MODULE_DESCRIPTION("imx386hybird");
MODULE_LICENSE("GPL v2");
