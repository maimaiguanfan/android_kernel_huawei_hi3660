/*
 * irda_chiptype.c
 *
 * irda chiptype driver registe
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
 */

#include <irda_driver.h>

#define IRDA_BUFF_SIZE		50
#define IRDA_SUCCESS		0
#define IRDA_ERROR		-1
#define DECIMAL_BASE		10

#define HWLOG_TAG irda_chiptype
HWLOG_REGIST();

static int g_chip_type;
extern struct class *irda_class;

struct irda_device {
	struct platform_device *pdev;
	struct device *dev;
};

static ssize_t chip_type_get(
			struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	hwlog_info("chiptype is :%d\n", g_chip_type);

	return snprintf(buf, IRDA_BUFF_SIZE, "%d\n", g_chip_type);
}

static DEVICE_ATTR(ir_chip_type, 0440, chip_type_get, NULL);

static int irda_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *dev_node = NULL;
	struct irda_device *irda_dev = NULL;

	g_chip_type = 0;
	dev_node = pdev->dev.of_node;
	if (dev_node == NULL) {
		hwlog_err("default chip type is maxim\n");
		g_chip_type = MAXIM_616;
		return 0;
	}

	irda_dev = kzalloc(sizeof(struct irda_device), GFP_KERNEL);
	if (irda_dev == NULL) {
		ret = -ENOMEM;
		return ret;
	}

	ret = of_property_read_u32(dev_node, IRDA_CHIP_TYPE, &g_chip_type);
	if (ret) {
		hwlog_warn("Failed to get chipset type; ret:%d\n", ret);
		/* set default chiptype as maxim */
		g_chip_type = MAXIM_616;
	}

	irda_dev->dev = device_create(
		irda_class, NULL, MKDEV(0, 0), NULL, "%s", "irda_chip");
	if (IS_ERR(irda_dev->dev)) {
		ret = PTR_ERR(irda_dev->dev);
		hwlog_err("Failed to create dev; ret:%d\n", ret);
		goto free_irda_dev;
	}

	ret = device_create_file(irda_dev->dev, &dev_attr_ir_chip_type);
	if (ret) {
		hwlog_err("Failed to create file; ret:%d\n", ret);
		goto free_dev;
	}

	dev_set_drvdata(irda_dev->dev, irda_dev);
	platform_set_drvdata(pdev, irda_dev);
	hwlog_info("platform device probe success\n");
	return 0;

free_dev:
	device_destroy(irda_class, irda_dev->dev->devt);
free_irda_dev:
	kfree(irda_dev);
	return ret;
}

static int irda_remove(struct platform_device *pdev)
{
	struct irda_device *irda_dev = platform_get_drvdata(pdev);

	device_remove_file(irda_dev->dev, &dev_attr_ir_chip_type);
	device_destroy(irda_class, irda_dev->dev->devt);
	kfree(irda_dev);

	return 0;
}

static const struct of_device_id irda_match_table[] = {
	{
		.compatible = IRDA_DRIVER_COMPATIBLE_ID,
		.data = NULL,
	},
	{
	},
};

MODULE_DEVICE_TABLE(of, irda_match_table);

static struct platform_driver irda_driver = {
	.probe = irda_probe,
	.remove = irda_remove,
	.driver = {
		.name = "irda",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(irda_match_table),
	},
};

int irda_chip_type_regist(void)
{
	return platform_driver_register(&irda_driver);
}

void irda_chip_type_unregist(void)
{
	platform_driver_unregister(&irda_driver);
}
