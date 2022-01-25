/*
 * sw_device.c
 *
 * single-wire driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <linux/kref.h>
#include <linux/module.h>
#include <linux/file.h>
#include <linux/kthread.h>
#include <linux/hidraw.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/tty.h>
#include <linux/crc16.h>
#include <linux/crc-ccitt.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/hisi/hisi_adc.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <huawei_platform/inputhub/kbhub.h>
#include "sw_core.h"
#include "sw_debug.h"

static int sw_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	return 0;
}

static bool sw_match_one_id(struct sw_device *hdev,
	const struct sw_device_id *id)
{
	SW_PRINT_FUNCTION_NAME;

	if ((hdev == NULL) || (id == NULL))
		return false;

	return (id->type == SW_TYPE_ANY || id->type == hdev->type) &&
		(id->bus == SW_BUS_ANY || id->bus == hdev->bus) &&
		(id->group == SW_GROUP_ANY || id->group == hdev->group) &&
		(id->vendor == SW_ANY_ID || id->vendor == hdev->vendor) &&
		(id->product == SW_ANY_ID || id->product == hdev->product);
}

static int sw_match_device(
	const struct sw_device_id *ids, struct sw_device *sdev)
{
	SW_PRINT_FUNCTION_NAME;

	if ((ids == NULL) || (sdev == NULL))
		return 0;

	for (; ids->bus; ids++) {
		if (sw_match_one_id(sdev, ids))
			return 1;
	}

	return 0;
}

static int sw_bus_match(struct device *dev, struct device_driver *drv)
{
	struct sw_driver *sdrv = container_of(drv, struct sw_driver, driver);
	struct sw_device *sdev = container_of(dev, struct sw_device, dev);

	if (sdrv == NULL)
		return -1;
	return sw_match_device(sdrv->id_table, sdev);
}

static int sw_connect_driver(struct sw_device *dev, struct sw_driver *drv)
{
	int retval;

	if ((dev == NULL) || (drv == NULL))
		return FAILURE;

	mutex_lock(&dev->drv_mutex);
	dev->drv = drv;
	retval = drv->probe(dev);
	if (retval)
		dev->drv = NULL;
	mutex_unlock(&dev->drv_mutex);

	return retval;
}

static void sw_disconnect_driver(struct sw_device *dev)
{
	if (dev == NULL)
		return;

	mutex_lock(&dev->drv_mutex);
	if (dev->drv != NULL)
		dev->drv->disconnect(dev);
	mutex_unlock(&dev->drv_mutex);
}

static int sw_device_probe(struct device *dev)
{
	struct sw_driver *sdrv = container_of(dev->driver,
		struct sw_driver, driver);
	struct sw_device *sdev = container_of(dev, struct sw_device, dev);

	SW_PRINT_FUNCTION_NAME;

	return sw_connect_driver(sdev, sdrv);
}

static int sw_device_remove(struct device *dev)
{
	struct sw_device *sdev = container_of(dev, struct sw_device, dev);

	if (sdev == NULL)
		return FAILURE;

	sw_disconnect_driver(sdev);
	return 0;
}

static struct bus_type sw_bus_type = {
	.name = "sw",
	.match = sw_bus_match,
	.probe = sw_device_probe,
	.remove = sw_device_remove,
	.uevent = sw_uevent,
};

/*
 * Free a device structure, all reports, and all fields.
 */
static void sw_device_release(struct device *dev)
{
	struct sw_device *sdev = container_of(dev, struct sw_device, dev);

	SW_PRINT_FUNCTION_NAME;

	kfree(sdev);
}

static struct sw_device *sw_allocate_device(void)
{
	struct sw_device *sdev;

	sdev = kzalloc(sizeof(*sdev), GFP_KERNEL);
	if (sdev == NULL)
		return NULL;

	sdev->dev.release = sw_device_release;
	sdev->dev.bus = &sw_bus_type;

	mutex_init(&sdev->drv_mutex);

	return sdev;
}

struct sw_device *sw_create_new_device(u32 pid, u32 vid)
{
	struct sw_device *newdev = sw_allocate_device();

	if (newdev == NULL)
		return NULL;

	newdev->type = SW_DEV_TYPE;
	newdev->bus = SW_BUS_ANY;
	newdev->group = SW_GROUP_ANY;
	newdev->vendor = vid;
	newdev->product = pid;
	dev_set_name(&newdev->dev, "SW%04X:%04X:%04X.%04X00", newdev->bus,
		newdev->vendor, newdev->product, newdev->type);
	return newdev;
}

struct sw_device *sw_register_new_hiddevice(int devno, struct sw_device *dev,
	u8 *rd_data, u32 rd_size)
{
	struct sw_device *newdev;
	int ret;

	if (dev == NULL)
		return NULL;

	newdev = sw_allocate_device();
	if (newdev == NULL)
		return NULL;

	newdev->type = SW_HID_TYPE;
	newdev->bus = SW_BUS_ANY;
	newdev->group = SW_GROUP_ANY;
	newdev->vendor = dev->vendor;
	newdev->product = dev->product;
	newdev->rd_data = rd_data;
	newdev->rd_size = rd_size;
	newdev->dev.parent = dev->dev.parent;

	dev_set_name(&newdev->dev, "SW%04X:%04X:%04X.%04X%02X", newdev->bus,
		newdev->vendor, newdev->product, newdev->type, devno);

	ret = device_register(&newdev->dev);
	if (ret)
		SW_PRINT_ERR("device_add %x\n", ret);

	return newdev;
}

int sw_release_device(struct sw_device *device)
{
	struct kobject *kobj;
	int err = 0;

	if (device == NULL)
		return 0;

	kobj = &device->dev.kobj;
	if (kobj == NULL) {
		SW_PRINT_ERR("kobj is null\n");
		err = -1;
	}

	if (kobj->sd == NULL) {
		SW_PRINT_ERR("kobj->sd is null\n");
		err = -1;
	}

	if (err) {
		SW_PRINT_ERR("sw_relese_device error\n");
		return -1;
	}
	SW_PRINT_INFO("sw_relese_device device_del begin\n");
	device_unregister(&device->dev);
	return 0;
}
EXPORT_SYMBOL_GPL(sw_release_device);

int sw_register_driver(struct sw_driver *drv, struct module *owner,
	const char *mod_name)
{
	int error;

	SW_PRINT_FUNCTION_NAME;

	drv->driver.bus = &sw_bus_type;
	drv->driver.owner = owner;
	drv->driver.mod_name = mod_name;
	error = driver_register(&drv->driver);
	if (error) {
		SW_PRINT_ERR("driver_register() failed for %s, error: %d\n",
			drv->driver.name, error);
		return error;
	}
	return error;
}

void sw_deregister(struct sw_driver *driver)
{
	SW_PRINT_INFO("sw deregistering interface driver %s\n", driver->name);
}

int sw_bus_init(void)
{
	int ret;

	SW_PRINT_INFO("sw_bus_init in\n");
	ret = bus_register(&sw_bus_type);
	if (ret)
		SW_PRINT_ERR("can't register sw bus\n");

	return ret;
}

void sw_bus_exit(void)
{
	SW_PRINT_INFO("sw_bus_exit in\n");
	bus_unregister(&sw_bus_type);
}
