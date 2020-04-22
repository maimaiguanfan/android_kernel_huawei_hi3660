/*
 * Huawei Touchscreen Driver
 *
 * Copyright (C) 2017 Huawei Device Co.Ltd
 * License terms: GNU General Public License (GPL) version 2
 *
 */

#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <asm/byteorder.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/ctype.h>
#include "huawei_thp.h"

#define SYSFS_PROPERTY_PATH	  "afe_properties"
#define SYSFS_TOUCH_PATH   "touchscreen"
#define SYSFS_PLAT_TOUCH_PATH	"huawei_touch"

u8 g_thp_log_cfg = 0;
EXPORT_SYMBOL(g_thp_log_cfg);

static ssize_t thp_tui_wake_up_enable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct thp_core_data *cd = thp_get_core_data();

	ret = strncmp(buf, "open", sizeof("open"));
	if (ret == 0) {
		cd->thp_ta_waitq_flag = WAITQ_WAKEUP;
		wake_up_interruptible(&(cd->thp_ta_waitq));
		THP_LOG_ERR("%s wake up thp_ta_flag\n", __func__);
	}
	return count;
}

static ssize_t thp_tui_wake_up_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t thp_hostprocessing_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "hostprocessing\n");
}

static ssize_t thp_status_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE - 1, "status=0x%x\n", thp_get_status_all());
}

/*
 * If not config ic_name in dts, it will be "unknown"
 */
static ssize_t thp_chip_info_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (cd->hide_product_info_en)
		return snprintf(buf, PAGE_SIZE, "%s\n", cd->project_id);
	else
		return snprintf(buf, PAGE_SIZE, "%s-%s-%s\n",
				cd->thp_dev->ic_name,
				cd->project_id,
				cd->vendor_name ? cd->vendor_name : "unknown");
}

static ssize_t thp_loglevel_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	u8 new_level = g_thp_log_cfg ? 0 : 1;

	int len = snprintf(buf, PAGE_SIZE, "%d -> %d\n",
				g_thp_log_cfg, new_level);

	g_thp_log_cfg = new_level;

	return len;
}

#if defined(THP_CHARGER_FB)
static ssize_t thp_host_charger_state_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	THP_LOG_DEBUG("%s called\n", __func__);

	return snprintf(buf, 32, "%d\n", thp_get_status(THP_STATUS_CHARGER));
}
static ssize_t thp_host_charger_state_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	/*
	 * get value of charger status from first byte of buf
	 */
	unsigned int value = buf[0] - '0';

	THP_LOG_INFO("%s: input value is %d\n", __func__, value);

	thp_set_status(THP_STATUS_CHARGER, value);

	return count;
}
#endif

static ssize_t thp_roi_data_show(struct device *dev,
				struct device_attribute *attr, char* buf)
{
	struct thp_core_data *cd = thp_get_core_data();
	short *roi_data = cd->roi_data;

	memcpy(buf, roi_data, ROI_DATA_LENGTH * sizeof(short));

	return ROI_DATA_LENGTH * sizeof(short);
}

static const char* move_to_next_number(const char* str_in)
{
	const char * str = str_in;
	const char *next_num;

	str = skip_spaces(str);
	next_num = strchr(str, ' ');
	if (next_num)
		return next_num;

	return str_in;
}

static ssize_t thp_roi_data_store(struct device *dev,
		struct device_attribute *attr, const char* buf, size_t count)
{
	int i = 0;
	int num;
	struct thp_core_data *cd = thp_get_core_data();
	short *roi_data = cd->roi_data;
	const char *str = buf;

	while (i < ROI_DATA_LENGTH && *str) {
		if (sscanf(str, "%7d", &num) < 0)
			break;

		str = move_to_next_number(str);
		roi_data[i++] = (short)num;
	}

	return count;
}

static ssize_t thp_roi_data_debug_show(struct device *dev,
				struct device_attribute *attr, char* buf)
{
	int count = 0;
	int i;
	struct thp_core_data *cd = thp_get_core_data();
	short *roi_data = cd->roi_data;

	for (i = 0; i < ROI_DATA_LENGTH; ++i) {
		count += snprintf(buf + count, PAGE_SIZE - count,
				"%4d ", roi_data[i]);
		/* every 7 data is a row */
		if (!((i + 1) % 7))
			count += snprintf(buf + count, PAGE_SIZE - count, "\n");
	}

	return count;
}
static ssize_t thp_roi_enable_store(struct device *dev,
		struct device_attribute *attr, const char* buf, size_t count)
{
	long status = 0;
	int ret;

	ret = strict_strtoul(buf, 10, &status);
	if (ret) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return ret;
	}

	thp_set_status(THP_STATUS_ROI, !!status);
	THP_LOG_INFO("%s: set roi enable status to %d\n", __func__, !!status);

	return count;
}

static ssize_t thp_roi_enable_show(struct device *dev,
					struct device_attribute *attr, char* buf)
{
	return snprintf(buf, PAGE_SIZE - 1, "%d\n",
		thp_get_status(THP_STATUS_ROI));
}

static ssize_t thp_holster_enable_store(struct device *dev,
		struct device_attribute *attr, const char* buf, size_t count)
{
	long status = 0;
	int ret;

	ret = strict_strtoul(buf, 10, &status);
	if (ret) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return ret;
	}

	thp_set_status(THP_STATUS_HOLSTER, !!status);
	THP_LOG_INFO("%s: set holster status to %d\n", __func__, !!status);

	return count;
}

static ssize_t thp_holster_enable_show(struct device *dev,
					struct device_attribute *attr, char* buf)
{
	return snprintf(buf, PAGE_SIZE - 1, "%d\n",
				thp_get_status(THP_STATUS_HOLSTER));
}

static ssize_t thp_glove_enable_store(struct device *dev,
		struct device_attribute *attr, const char* buf, size_t count)
{
	long status = 0;
	int ret;

	ret = strict_strtoul(buf, 10, &status);
	if (ret) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return ret;
	}

	thp_set_status(THP_STATUS_GLOVE, !!status);
	THP_LOG_INFO("%s: set glove status to %d\n", __func__, !!status);

	return count;
}

static ssize_t thp_glove_enable_show(struct device *dev,
					struct device_attribute *attr, char* buf)
{

	return snprintf(buf, PAGE_SIZE - 1, "%d\n",
			thp_get_status(THP_STATUS_GLOVE));
}


static ssize_t thp_holster_window_store(struct device *dev,
		struct device_attribute *attr, const char* buf, size_t count)
{
	struct thp_core_data *cd = thp_get_core_data();
	int ret;

	int window_enable;
	int x0 = 0;
	int y0 = 0;
	int x1 = 0;
	int y1 = 0;

	ret = sscanf(buf,"%4d %4d %4d %4d %4d", &window_enable, &x0,&y0,&x1, &y1);
	if (ret <= 0) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return -EBUSY;
	}
	cd->window.x0 = x0;
	cd->window.y0 = y0;
	cd->window.x1 = x1;
	cd->window.y1 = y1;

	thp_set_status(THP_STATUS_HOLSTER, !!window_enable);
	thp_set_status(THP_STAUTS_WINDOW_UPDATE, !thp_get_status(THP_STAUTS_WINDOW_UPDATE));
	THP_LOG_INFO("%s: update window %d %d %d %d %d\n", __func__, window_enable, x0, y0, x1, y1);

	return count;
}

static ssize_t thp_holster_window_show(struct device *dev,
					struct device_attribute *attr, char* buf)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_window_info *window = &cd->window;

	return snprintf(buf, PAGE_SIZE - 1, "%d %d %d %d %d\n",
			thp_get_status(THP_STATUS_HOLSTER),
			window->x0, window->y0, window->x1, window->y1);
}

static ssize_t thp_touch_switch_store(struct device *dev,
		struct device_attribute *attr, const char* buf, size_t count)
{
	struct thp_core_data *cd = thp_get_core_data();
	int ret;

	int type = 0;
	int status = 0;
	int parameter = 0;

	ret = sscanf(buf,"%4d,%4d,%4d", &type, &status,&parameter);
	if (ret <= 0) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return -EBUSY;
	}
	cd->scene_info.type = type;
	cd->scene_info.status = status;
	cd->scene_info.parameter = parameter;
	thp_set_status(THP_STAUTS_TOUCH_SCENE, !thp_get_status(THP_STAUTS_TOUCH_SCENE));
	THP_LOG_INFO("%s:touch scene update %d %d %d\n", __func__, type, status, parameter);

	return count;
}

static ssize_t thp_touch_switch_show(struct device *dev,
					struct device_attribute *attr, char* buf)
{
	unsigned int value = 0;

	THP_LOG_INFO("%s:value = %d\n", __func__,value);

	/* Inherit from tskit ,which will be use for ApsService to decide doze is support or not .
	 * This feature is not supported by default in THP.
	 */
	return  snprintf(buf, PAGE_SIZE - 1, "%d\n", value);
}

static ssize_t thp_udfp_enable_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE - 1, "udfp status : %d\n", thp_get_status(THP_STAUTS_UDFP));
}

static ssize_t thp_udfp_enable_store(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t count)
{
	unsigned int value = 0;

	if (sscanf(buf, "%d", &value) <= 0) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return -EINVAL;
	}

	thp_set_status(THP_STAUTS_UDFP, value);
	THP_LOG_INFO("%s: ud fp status: %d\n", __func__, !!value);

	return count;
}

static ssize_t thp_supported_func_indicater_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();

	return snprintf(buf, PAGE_SIZE - 1, "%d\n", cd->supported_func_indicater);
}

static ssize_t thp_easy_wakeup_gesture_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_easy_wakeup_info *info = &cd->easy_wakeup_info;
	ssize_t ret;

	if (!cd->support_gesture_mode) {
		THP_LOG_ERR("[%s]not support\n", __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("%s\n", __func__);

	if (!dev) {
		THP_LOG_ERR("dev is null\n");
		return -EINVAL;
	}

	ret = snprintf(buf, MAX_STR_LEN, "0x%04X\n",
		info->easy_wakeup_gesture);

	return ret;
}

static ssize_t thp_easy_wakeup_gesture_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_easy_wakeup_info *info;
	unsigned long value = 0;
	int ret;

	if (!cd->support_gesture_mode) {
		THP_LOG_ERR("[%s]not support\n",  __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("thp_easy_wakeup_gesture_store called\n");

	if (!dev) {
		THP_LOG_ERR("dev is null\n");
		return -EINVAL;
	}
	ret = sscanf(buf, "%4lu", &value);
	if (ret <= 0 || value > TS_GESTURE_INVALID_COMMAND) {
		THP_LOG_ERR("invalid parm\n");
		return -EBUSY;
	}
	info = &cd->easy_wakeup_info;
	info->easy_wakeup_gesture = (u16) value & TS_GESTURE_COMMAND;
	THP_LOG_INFO("easy_wakeup_gesture=0x%x\n", info->easy_wakeup_gesture);
	if (info->easy_wakeup_gesture == false) {
		cd->sleep_mode = TS_POWER_OFF_MODE;
		THP_LOG_INFO("poweroff mode\n");
	} else {
		cd->sleep_mode = TS_GESTURE_MODE;
		THP_LOG_INFO("gesture mode\n");
	}
	ret = size;

	THP_LOG_INFO("ts gesture wakeup done\n");
	return ret;
}
static ssize_t thp_wakeup_gesture_enable_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned long tmp = 0;
	struct thp_core_data *cd = thp_get_core_data();
	int error;

	if (!cd->support_gesture_mode) {
		THP_LOG_ERR("[%s]not support\n",  __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("%s called\n", __func__);

	error = sscanf(buf, "%4lu", &tmp);
	if (error <= 0) {
		THP_LOG_ERR("sscanf return invaild :%d\n", error);
		error = -EINVAL;
		goto out;
	}
	THP_LOG_INFO("%s:%lu\n", __func__, tmp);
	error = count;
out:
	return error;
}

static ssize_t thp_wakeup_gesture_enable_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();
	if (!cd->support_gesture_mode) {
		THP_LOG_ERR("[%s]not support\n", __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("%s called\n", __func__);
	return 0;
}

static ssize_t thp_easy_wakeup_control_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct thp_core_data *cd = thp_get_core_data();

	unsigned long value = 0;
	int ret;
	int error = NO_ERR;

	if (!cd->support_gesture_mode) {
		THP_LOG_ERR("[%s]not support\n", __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("%s called\n", __func__);

	ret = sscanf(buf, "%4lu", &value);
	if (ret <= 0 || value > TS_GESTURE_INVALID_CONTROL_NO) {
		THP_LOG_INFO("[%s] -> invalid parm\n", __func__);
		return -EINVAL;
	}

	value = (u8) value & TS_GESTURE_COMMAND;
	if (value == 1) {
		if (cd->thp_dev->ops->chip_wrong_touch) {
			error = cd->thp_dev->ops->chip_wrong_touch(cd->thp_dev);
			if (error < 0) {
				THP_LOG_INFO("chip_wrong_touch error\n");
			}
		} else {
			THP_LOG_INFO("chip_wrong_touch not init\n");
		}
	}
	THP_LOG_INFO("%s done\n", __func__);
	return size;
}

static ssize_t thp_easy_wakeup_position_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();
	if (!cd->support_gesture_mode) {
		THP_LOG_ERR("[%s]not support\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static ssize_t thp_oem_info_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int error = NO_ERR;
	struct thp_core_data *cd = thp_get_core_data();

	THP_LOG_INFO("%s: called\n", __func__);

	if ((dev == NULL) || (cd == NULL)) {
		THP_LOG_ERR("%s: dev is null\n", __func__);
		return -EINVAL;
	}
	if (cd->support_oem_info == THP_OEM_INFO_LCD_EFFECT_TYPE) {
		error = snprintf(buf, OEM_INFO_DATA_LENGTH, "%s",
			cd->oem_info_data);
		if (error < 0) {
			THP_LOG_INFO("%s:oem info data:%s\n",
				__func__, cd->oem_info_data);
			return error;
		}
		THP_LOG_INFO("%s: oem info :%s\n", __func__, buf);
	}

	THP_LOG_DEBUG("%s done\n", __func__);
	return error;
}



static DEVICE_ATTR(thp_status, S_IRUGO, thp_status_show, NULL);
static DEVICE_ATTR(touch_chip_info, (S_IRUSR | S_IWUSR | S_IRGRP),
			thp_chip_info_show, NULL);
static DEVICE_ATTR(hostprocessing, S_IRUGO, thp_hostprocessing_show, NULL);
static DEVICE_ATTR(loglevel, S_IRUGO, thp_loglevel_show, NULL);
static DEVICE_ATTR(charger_state, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_host_charger_state_show, thp_host_charger_state_store);
static DEVICE_ATTR(roi_data, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			 thp_roi_data_show, thp_roi_data_store);
static DEVICE_ATTR(roi_data_internal, S_IRUGO, thp_roi_data_debug_show, NULL);
static DEVICE_ATTR(roi_enable, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_roi_enable_show, thp_roi_enable_store);
static DEVICE_ATTR(touch_sensitivity, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_holster_enable_show, thp_holster_enable_store);
static DEVICE_ATTR(touch_glove, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_glove_enable_show, thp_glove_enable_store);
static DEVICE_ATTR(touch_window, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_holster_window_show, thp_holster_window_store);
static DEVICE_ATTR(touch_switch,(S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_touch_switch_show, thp_touch_switch_store);
static DEVICE_ATTR(udfp_enable, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_udfp_enable_show, thp_udfp_enable_store);
static DEVICE_ATTR(supported_func_indicater, (S_IRUSR),
			thp_supported_func_indicater_show, NULL);
static DEVICE_ATTR(tui_wake_up_enable, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_tui_wake_up_enable_show, thp_tui_wake_up_enable_store);
static DEVICE_ATTR(easy_wakeup_gesture, (S_IRUSR | S_IWUSR),
			thp_easy_wakeup_gesture_show, thp_easy_wakeup_gesture_store);
static DEVICE_ATTR(wakeup_gesture_enable, (S_IRUSR | S_IWUSR),
			thp_wakeup_gesture_enable_show,
			thp_wakeup_gesture_enable_store);
static DEVICE_ATTR(easy_wakeup_control, S_IWUSR, NULL,
			thp_easy_wakeup_control_store);
static DEVICE_ATTR(easy_wakeup_position, S_IRUSR, thp_easy_wakeup_position_show,
			NULL);
static DEVICE_ATTR(touch_oem_info, (S_IRUSR | S_IWUSR | S_IRGRP),
			thp_oem_info_show, NULL);

static struct attribute *thp_ts_attributes[] = {
	&dev_attr_thp_status.attr,
	&dev_attr_touch_chip_info.attr,
	&dev_attr_hostprocessing.attr,
	&dev_attr_loglevel.attr,
#if defined(THP_CHARGER_FB)
	&dev_attr_charger_state.attr,
#endif
	&dev_attr_roi_data.attr,
	&dev_attr_roi_data_internal.attr,
	&dev_attr_roi_enable.attr,
	&dev_attr_touch_sensitivity.attr,
	&dev_attr_touch_glove.attr,
	&dev_attr_touch_window.attr,
	&dev_attr_touch_switch.attr,
	&dev_attr_udfp_enable.attr,
	&dev_attr_supported_func_indicater.attr,
	&dev_attr_tui_wake_up_enable.attr,
	&dev_attr_easy_wakeup_gesture.attr,
	&dev_attr_wakeup_gesture_enable.attr,
	&dev_attr_easy_wakeup_control.attr,
	&dev_attr_easy_wakeup_position.attr,
	&dev_attr_touch_oem_info.attr,
	NULL,
};

static const struct attribute_group thp_ts_attr_group = {
	.attrs = thp_ts_attributes,
};


int thp_init_sysfs(struct thp_core_data *cd)
{
	int rc;

	if (!cd) {
		THP_LOG_ERR("%s: core data null\n", __func__);
		return -EINVAL;
	}

	cd->thp_platform_dev = platform_device_alloc("huawei_thp", -1);
	if (!cd->thp_platform_dev) {
		THP_LOG_ERR("%s: regist platform_device failed\n", __func__);
		return -ENODEV;
	}

	rc = platform_device_add(cd->thp_platform_dev);
	if (rc) {
		THP_LOG_ERR("%s: add platform_device failed\n", __func__);
		platform_device_unregister(cd->thp_platform_dev);
		return -ENODEV;
	}

	rc = sysfs_create_group(&cd->thp_platform_dev->dev.kobj, &thp_ts_attr_group);
	if (rc) {
		THP_LOG_ERR("%s:can't create ts's sysfs\n", __func__);
		goto err_create_group;
	}

	rc = sysfs_create_link(NULL, &cd->thp_platform_dev->dev.kobj, SYSFS_TOUCH_PATH);
	if (rc) {
		THP_LOG_ERR("%s: fail create link error = %d\n", __func__, rc);
		goto err_create_link;
	}

	return 0;

err_create_link:
	sysfs_remove_group(&cd->thp_platform_dev->dev.kobj, &thp_ts_attr_group);
err_create_group:
	platform_device_put(cd->thp_platform_dev);
	platform_device_unregister(cd->thp_platform_dev);
	return rc;
}


void thp_sysfs_release(struct thp_core_data *cd)
{
	if (!cd) {
		THP_LOG_ERR("%s: core data null\n", __func__);
		return;
	}
	platform_device_put(cd->thp_platform_dev);
	platform_device_unregister(cd->thp_platform_dev);
	sysfs_remove_group(&cd->sdev->dev.kobj, &thp_ts_attr_group);
}


