#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <asm/page.h>

#include "modem_hkadc_temp.h"

#define DEFULT_ERROR_VALUE 100000

static struct hisi_mdm_adc_chip *gtm_dev = NULL;

struct mdm_adc_s {
	unsigned int channel;
	unsigned int mode;
};

struct hisi_mdm_get_adc {
	int (*get_value_func)(struct mdm_adc_s *mdm_adc_para);
};

static struct hisi_mdm_get_adc *g_mdm_adc = NULL;

static int hisi_mdm_adc_get_value_default(struct mdm_adc_s *mdm_adc_para)
{
	pr_info("%s return the default value\n", __func__);
	return (int)DEFULT_ERROR_VALUE;
}

int hisi_mdm_adc_get_value_register(int (*func)(struct mdm_adc_s *mdm_adc_para))
{
	if(NULL == func){
		return -EINVAL;
	}

	if(NULL == g_mdm_adc){
		g_mdm_adc = (struct hisi_mdm_get_adc *)kzalloc(sizeof(struct hisi_mdm_get_adc),GFP_KERNEL);
		if(NULL == g_mdm_adc){
			return -EINVAL;
		}
	}
	g_mdm_adc->get_value_func = func;

	return 0;
}

EXPORT_SYMBOL(hisi_mdm_adc_get_value_register);

static int hisi_get_mdm_temp(struct thermal_zone_device *thermal,int *temp)
{
	struct hisi_mdm_adc_t *hisi_mdm_sensor = thermal->devdata;
	int ret = 0;
	int volt = 0;
	struct mdm_adc_s mdm_adc;

	if (!hisi_mdm_sensor || !temp || !g_mdm_adc)
		return -EINVAL;

	mdm_adc.channel = hisi_mdm_sensor->channel;
	mdm_adc.mode = hisi_mdm_sensor->mode;
	if(mdm_adc.channel <= 0xf){ //0-0xe is adc channel; 0xf is xo channel
		volt = g_mdm_adc->get_value_func(&mdm_adc);
		if (volt < 0) {
			pr_err("AP get mdm adc volt value is fail,chan[%d]volt[%d]!\n", hisi_mdm_sensor->channel, volt);
			return -EINVAL;
		}else if(volt == DEFULT_ERROR_VALUE){ // has not init
			*temp = (int)DEFULT_ERROR_VALUE;
			return 0;
		}

		ret = hisi_mdm_ntc_2_temp(hisi_mdm_sensor, temp, volt);
		if (ret < 0) {
			pr_err("%s get temp failed ret = %d\n", __func__, ret);
			return ret;
		}

	} else if(mdm_adc.channel > 0xf && mdm_adc.channel <= 0x13){ //tsensor
		volt = g_mdm_adc->get_value_func(&mdm_adc);
		if(volt <= -30000){
			pr_err("AP get mdm tsensor value is fail,chan[%d]tsensor[%d]\n",hisi_mdm_sensor->channel, volt);
			return -EINVAL;
		}
		*temp = volt;
	} else if(mdm_adc.channel == 0x14) { //ddr
		volt = g_mdm_adc->get_value_func(&mdm_adc);
		if (volt < 0) {
			pr_err("AP get mdm ddr volt value is fail,chan[%d]volt[%d]!\n", hisi_mdm_sensor->channel, volt);
			return -EINVAL;
		}
		*temp = volt;
	}else{ //others
		pr_err("%s wrong channel chan[%d]\n",__func__,hisi_mdm_sensor->channel);
		return -EINVAL;
	}

	return 0;
}

/*lint -e785*/
static struct thermal_zone_device_ops mdm_thermal_zone_ops = {
	.get_temp = hisi_get_mdm_temp,
};
/*lint +e785*/
static int hisi_mdm_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *dev_node = dev->of_node;
	int ret = 0;
	int sensor_count;
	int i = 0;
	struct device_node *np;
	struct device_node *child;
	int mdm_channel = 0;
	int mdm_mode = 0;

	pr_info("enter %s\n", __func__);

	if (!of_device_is_available(dev_node)) {
		dev_err(dev, "HISI mdm dev not found\n");
		ret = -ENODEV;
		goto exit;
	}

	np = of_find_node_by_name(dev_node, "sensors");
	if (!np) {
		pr_err("sensors node not found\n");
		ret = -ENODEV;
		goto exit;
	}

	sensor_count = of_get_child_count(np);
	if (sensor_count <= 0) {
		ret = -EINVAL;
		pr_err("%s sensor count read err\n", __func__);
		goto node_put;
	}

	gtm_dev = devm_kzalloc(dev, sizeof(struct hisi_mdm_adc_chip) +
			sensor_count * sizeof(struct hisi_mdm_adc_t), GFP_KERNEL);
	if (gtm_dev == NULL) {
		pr_err("%s: kzalloc() failed\n", __func__);
		ret = -ENOMEM;
		goto node_put;
	}
	gtm_dev->tsens_num_sensor = sensor_count;

	for_each_child_of_node(np, child) {

		ret = of_property_read_u32(child, "channel", &mdm_channel);
		if (ret) {
			dev_err(dev, "canot get %d mdm ntc channel\n", i);
			ret = -ENODEV;
			goto read_channel_fail;
		}
		gtm_dev->hisi_mdm_sensor[i].channel = mdm_channel;

		ret = of_property_read_u32(child, "mode", &mdm_mode);
		if (ret) {
			dev_err(dev, "canot get %d mdm ntc mode\n", i);
			ret = -ENODEV;
			goto read_mode_fail;
		}
		gtm_dev->hisi_mdm_sensor[i].mode = mdm_mode;

		ret = of_property_read_string(child, "type", &(gtm_dev->hisi_mdm_sensor[i].name));
		if (ret) {
			pr_err("%s type read err\n", __func__);
			goto read_type_fail;
		}

		ret = of_property_read_string(child, "tm_ntc_table", &(gtm_dev->hisi_mdm_sensor[i].ntc_name));
		if (ret) {
			dev_err(dev, "detect %d tm_ntc_table name failed\n", i);
			ret = -EINVAL;
			goto read_ntc_table_fail;
		}

		gtm_dev->hisi_mdm_sensor[i].tz_dev =
			thermal_zone_device_register(gtm_dev->hisi_mdm_sensor[i].name,
			0, 0, &gtm_dev->hisi_mdm_sensor[i],&mdm_thermal_zone_ops, NULL, 0, 0);

		if (IS_ERR(gtm_dev->hisi_mdm_sensor[i].tz_dev)) {
			dev_err(dev, "register thermal zone for mdm failed.\n");
			ret = -ENODEV;
			goto unregister;
		}

		i++;
	}

	of_node_put(np);
	platform_set_drvdata(pdev, gtm_dev);

	if(NULL == g_mdm_adc){
		 hisi_mdm_adc_get_value_register(hisi_mdm_adc_get_value_default);
	}

	pr_info("%s ok\n", __func__);

	goto exit;

unregister:
	for(;i >= 0;i--){
		thermal_zone_device_unregister(gtm_dev->hisi_mdm_sensor[i].tz_dev);
	}

read_ntc_table_fail:
read_type_fail:
read_mode_fail:
read_channel_fail:
node_put:
	of_node_put(np);
exit:
	return ret;
}

static int hisi_mdm_remove(struct platform_device *pdev)
{
	struct hisi_mdm_adc_chip *chip = platform_get_drvdata(pdev);
	int i;
	if(NULL != g_mdm_adc){
	   kfree(g_mdm_adc);
	}
	if (chip) {
		for(i = 0; i < chip-> tsens_num_sensor; i++){
			thermal_zone_device_unregister(chip-> hisi_mdm_sensor[i].tz_dev);
		}
		platform_set_drvdata(pdev, NULL);
	}

	return 0;
}
/*lint -e785*/
static struct of_device_id hisi_mdm_of_match[] = {
	{ .compatible = "hisi,mdm_temp" },
	{},
};
/*lint +e785*/
MODULE_DEVICE_TABLE(of, hisi_mdm_of_match);

/*lint -e64 -e785 -esym(64,785,*)*/
static struct platform_driver hisi_mdm_platdrv = {
	.driver = {
		.name		= "hisi_mdm_temp",
		.owner		= THIS_MODULE,
		.of_match_table = hisi_mdm_of_match,
	},
	.probe	= hisi_mdm_probe,
	.remove	= hisi_mdm_remove,
};

static int __init hisi_mdm_init(void)
{
	return platform_driver_register(&hisi_mdm_platdrv);
}

static void __exit hisi_mdm_exit(void)
{
	platform_driver_unregister(&hisi_mdm_platdrv);
}
module_init(hisi_mdm_init);
module_exit(hisi_mdm_exit);

MODULE_LICENSE("GPL v2");
/*lint -e64 -e785 +esym(64,785,*)*/
