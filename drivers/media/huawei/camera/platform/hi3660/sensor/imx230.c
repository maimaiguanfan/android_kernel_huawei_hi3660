


#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>

#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"
//lint -save -e31

#define I2S(i) container_of(i, sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)

static hwsensor_vtbl_t s_imx230_vtbl;

int imx230_config(hwsensor_intf_t* si, void  *argp);

struct sensor_power_setting hw_imx230_power_setting[] = {

	//disable slave camera reset
	{
		.seq_type = SENSOR_SUSPEND,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},

	//enable gpio12 output iovdd 1.8v
	{
		.seq_type = SENSOR_LDO_EN,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},

	//SCAM AVDD 2.8V
	{
		.seq_type = SENSOR_AVDD,
		.data = (void*)"slave-sensor-avdd",
		.config_val = LDO_VOLTAGE_V2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},

	//SCAM DVDD1.2V
	{
		.seq_type = SENSOR_DVDD,
		.config_val = LDO_VOLTAGE_1P2V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},

    //MCAM1 AFVDD 2.85V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

	//MCAM1 AVDD 2.6V
	{
		.seq_type = SENSOR_AVDD2,
		.data = (void*)"main-sensor-avdd",
		.config_val = LDO_VOLTAGE_V2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},

	//MCAM1 DVDD 1.1V
	{
		.seq_type = SENSOR_DVDD2,
		.config_val = LDO_VOLTAGE_V1P1V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},

	{
		.seq_type = SENSOR_MCLK,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},

	{
		.seq_type = SENSOR_RST,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
};

static sensor_t s_imx230 =
{
    .intf = { .vtbl = &s_imx230_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(hw_imx230_power_setting),
            .power_setting = hw_imx230_power_setting,
     },
};

static const struct of_device_id
s_imx230_dt_match[] =
{
	{
        .compatible = "huawei,imx230",
        .data = &s_imx230.intf,
    },
	{
    },
};

MODULE_DEVICE_TABLE(of, s_imx230_dt_match);

static struct platform_driver
s_imx230_driver =
{
	.driver =
    {
		.name = "huawei,imx230",
		.owner = THIS_MODULE,
		.of_match_table = s_imx230_dt_match,
	},
};

char const*
imx230_get_name(
        hwsensor_intf_t* si)
{
    sensor_t* sensor = I2S(si);
    return sensor->board_info->name;
}

int
imx230_power_up(
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

int
imx230_power_down(
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

int imx230_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

int imx230_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int
imx230_match_id(
        hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = I2S(si);
    struct sensor_cfg_data *cdata = (struct sensor_cfg_data *)data;

    cam_info("%s TODO.", __func__);
    memset(cdata->cfg.name, 0, (unsigned long)DEVICE_NAME_SIZE);
    strncpy(cdata->cfg.name, sensor->board_info->name, DEVICE_NAME_SIZE - 1);
    cdata->data = sensor->board_info->sensor_index;

    hwsensor_writefile(sensor->board_info->sensor_index,
		sensor->board_info->name);
	return 0;

}


static hwsensor_vtbl_t
s_imx230_vtbl =
{
	.get_name = imx230_get_name,
	.config = imx230_config,
	.power_up = imx230_power_up,
	.power_down = imx230_power_down,
	.match_id = imx230_match_id,
	.csi_enable = imx230_csi_enable,
	.csi_disable = imx230_csi_disable,
	/* .sensor_register_attribute = imx230_register_attribute, */
};

int
imx230_config(
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
	cam_debug("imx230_austin cfgtype = %d",data->cfgtype);
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
		default:
			cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
			break;
	}
	return ret;
}

static int32_t
imx230_platform_probe(
        struct platform_device* pdev)
{
	int rc = 0;
	cam_notice("enter %s",__func__);
	if (pdev->dev.of_node) {
		rc = hw_sensor_get_dt_data(pdev, &s_imx230);
		if (rc < 0) {
			cam_err("%s failed line %d\n", __func__, __LINE__);
			goto imx230_sensor_probe_fail;
		}
	} else {
		cam_err("%s imx230 of_node is NULL.\n", __func__);
		goto imx230_sensor_probe_fail;
	}

    s_imx230.dev = &pdev->dev;

	rc = hwsensor_register(pdev, &s_imx230.intf);
	rc = rpmsg_sensor_register(pdev, (void *)&s_imx230);

imx230_sensor_probe_fail:
	return rc;
}

static int __init
imx230_init_module(void)
{
    cam_notice("enter %s",__func__);
    return platform_driver_probe(&s_imx230_driver,
            imx230_platform_probe);
}

static void __exit
imx230_exit_module(void)
{
    hwsensor_unregister(Sensor2Pdev(s_imx230));
    platform_driver_unregister(&s_imx230_driver);
}

module_init(imx230_init_module);
module_exit(imx230_exit_module);
MODULE_DESCRIPTION("imx230");
MODULE_LICENSE("GPL v2");
//lint -restore

