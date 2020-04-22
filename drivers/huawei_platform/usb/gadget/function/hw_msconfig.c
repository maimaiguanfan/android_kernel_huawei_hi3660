/*
 * hw_msconfig.c
 *
 *   mass storage autorun and lun config driver
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
 *
 */

#define MS_STG_SET_LEN         (32)
#define FSG_MAX_LUNS_HUAWEI    (2)

/* enable or disable autorun function "enable" & "disable" */
static char autorun[MS_STG_SET_LEN] = "enable";
/* "sdcard" & "cdrom,sdcard" & "cdrom" & "sdcard,cdrom" can be used */
static char luns[MS_STG_SET_LEN] = "sdcard";

static ssize_t autorun_store(struct device *dev,
	struct device_attribute *attr, const char *buff, size_t size)
{
	if (size > MS_STG_SET_LEN || !buff) {
		pr_err("mass_storage: autorun_store buff error\n");
		return -EINVAL;
	}

	if ((strcmp(buff, "enable") != 0) && (strcmp(buff, "disable") != 0)) {
		pr_err("mass_storage: autorun_store para error '%s'\n", buff);
		return -EINVAL;
	}

	strlcpy(autorun, buff, sizeof(autorun));

	return size;
}

static ssize_t autorun_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", autorun);
}

static DEVICE_ATTR(autorun, 0600, autorun_show, autorun_store);

static ssize_t luns_store(struct device *dev,
	struct device_attribute *attr, const char *buff, size_t size)
{
	if (size > MS_STG_SET_LEN || !buff) {
		pr_err("mass_storage: luns_store buff error\n");
		return -EINVAL;
	}

	strlcpy(luns, buff, sizeof(luns));

	return size;
}

static ssize_t luns_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", luns);
}

static DEVICE_ATTR(luns, 0600, luns_show, luns_store);
