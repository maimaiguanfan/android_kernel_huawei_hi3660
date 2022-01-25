/*
 * Copyright (C) huawei company
 *
 * This	program	is free	software; you can redistribute it and/or modify
 * it under	the	terms of the GNU General Public	License	version	2 as
 * published by	the	Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/completion.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/touthscreen/huawei_tp_color.h>
#define HWLOG_TAG tp_color
HWLOG_REGIST();


#if (defined CONFIG_HUAWEI_TP_KIT_COLOR_INFO) || (defined CONFIG_HUAWEI_TP_KIT_COLOR_INFO_3_0)
#define TP_COLOR_SIZE   15
#define FIRST_ELEMENT   0
extern u8 cypress_ts_kit_color[TP_COLOR_SIZE];
#endif

#define TP_COLOR_NV_DATA_SIZE 104
#define NV_READ_TIMEOUT   30

static struct work_struct nv_read_work;
static struct work_struct nv_write_work;
static char g_nv_data[TP_COLOR_NV_DATA_SIZE];
static struct completion nv_read_done;
static unsigned char tp_color_buf[TP_COLOR_BUF_SIZE];
static int read_tp_color_from_nv(void);

u8 cypress_tp_color[15] = { 0 };

static void tp_nv_read_work_fn(struct work_struct *work)
{
	int ret= 0;

	ret = read_tp_color_from_nv();
	if (ret)
	{
		strncpy(g_nv_data, "", sizeof(""));
	}
	complete_all(&nv_read_done);
}

static void tp_nv_write_work_fn(struct work_struct *work)
{
	int ret= 0;
	struct hisi_nve_info_user user_info;

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = 0;
	user_info.nv_number = 16;
	user_info.valid_size = 15;
	strncpy(user_info.nv_data, g_nv_data,  sizeof(user_info.nv_data));
	strncpy(user_info.nv_name, "TPCOLOR", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret) {
		hwlog_err("hisi_nve_direct_access write error(%d)\n", ret);
	}
	hwlog_info("[%s]:(%s)\n", __func__, user_info.nv_data);
}

/******************************************************************************
Function:	    read_tp_color
******************************************************************************/
int read_tp_color(void)
{
	int tp_color;
	hwlog_info("tp color is %s\n", tp_color_buf);

	tp_color = (int)simple_strtol(tp_color_buf, NULL, 0);
	return tp_color;
}

static int __init early_parse_tp_color_cmdline(char *arg)
{
	int len = 0;
	memset(tp_color_buf, 0, sizeof(tp_color_buf));
	if (arg) {
		len = strlen(arg);

		if (len > sizeof(tp_color_buf)) {
			len = sizeof(tp_color_buf);
		}
		memcpy(tp_color_buf, arg, len);
	} else {
		hwlog_info("%s : arg is NULL\n", __func__);
	}

	return 0;
}

early_param("TP_COLOR", early_parse_tp_color_cmdline);

int is_color_correct(u8 color)
{
	return ((color & 0x0f) == ((~(color >> 4)) & 0x0f));
}

int tp_color_provider(void)
{
	u8 lcd_id = 0;

	lcd_id = read_tp_color();
	if (lcd_id != 0xff) {
		hwlog_info("tp color is %u \n", lcd_id);
	}
	if (is_color_correct(cypress_tp_color[0]))
		return cypress_tp_color[0];
	else if (is_color_correct(lcd_id))
		return lcd_id;
	else {
		hwlog_err("LCD/TP ID both error!\n");
		return -1;
	}
}

EXPORT_SYMBOL(tp_color_provider);
static int read_tp_color_from_nv()
{
	int ret = 0;
	struct hisi_nve_info_user user_info;

	memset(&user_info, 0, sizeof(user_info));
	memset(g_nv_data, 0, sizeof(g_nv_data));
	user_info.nv_operation = 1;
	user_info.nv_number = 16;
	user_info.valid_size = 15;
	strncpy(user_info.nv_name, "TPCOLOR", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret) {
		hwlog_err("hisi_nve_direct_access read error(%d)\n", ret);
		return -1;
	}
	if ((!strncmp(user_info.nv_data, "white", strlen("white")))
		|| (!strncmp(user_info.nv_data, "black", strlen("black")))
		|| (!strncmp(user_info.nv_data, "silver", strlen("silver")))
		|| (!strncmp(user_info.nv_data, "pink", strlen("pink")))
		|| (!strncmp(user_info.nv_data, "red", strlen("red")))
		|| (!strncmp(user_info.nv_data, "yellow", strlen("yellow")))
		|| (!strncmp(user_info.nv_data, "blue", strlen("blue")))
		|| (!strncmp(user_info.nv_data, "gold", strlen("gold")))
		|| (!strncmp(user_info.nv_data, "gray", strlen("gray")))
		|| (!strncmp(user_info.nv_data, "cafe", strlen("cafe")))
		|| (!strncmp(user_info.nv_data, "pdblack", strlen("pdblack")))
		|| (!strncmp(user_info.nv_data, "green", strlen("green")))
		|| (!strncmp(user_info.nv_data, "pinkgold", strlen("pinkgold")))) {
		strncpy(g_nv_data, user_info.nv_data,
			strlen(user_info.nv_data));
		g_nv_data[strlen(user_info.nv_data)] = '\0';
		return 0;
	} else {
		hwlog_err("[%s]:read_tp_color_from_nv read unormal value!!\n",
			  __func__);
		return -1;
	}
}

static int write_tp_color_to_nv(void)
{
	int ret = 0;
	u8 lcd_id = 0;
	u8 phone_color = 0;

	memset(g_nv_data, 0, sizeof(g_nv_data));
	lcd_id = read_tp_color();
	if (lcd_id != 0xff) {
		hwlog_info("lcd id is %u from read tp color\n", lcd_id);
	}
	if (is_color_correct(cypress_tp_color[0]))
		phone_color = cypress_tp_color[0];
	else if (is_color_correct(lcd_id))
		phone_color = lcd_id;
#if (defined CONFIG_HUAWEI_TP_KIT_COLOR_INFO) || (defined CONFIG_HUAWEI_TP_KIT_COLOR_INFO_3_0)
	else if (is_color_correct(cypress_ts_kit_color[FIRST_ELEMENT])) {
		phone_color = cypress_ts_kit_color[FIRST_ELEMENT];
	}
#endif
	else {
		hwlog_err("LCD/TP ID both error!\n");
		return -1;
	}
	switch (phone_color) {
	case WHITE:
		strncpy(g_nv_data, "white", sizeof(g_nv_data));
		break;
	case BLACK:
		strncpy(g_nv_data, "black", sizeof(g_nv_data));
		break;
	case PINK:
	case PINKGOLD:
		strncpy(g_nv_data, "pink", sizeof(g_nv_data));
		break;
	case RED:
		strncpy(g_nv_data, "red", sizeof(g_nv_data));
		break;
	case YELLOW:
		strncpy(g_nv_data, "yellow", sizeof(g_nv_data));
		break;
	case BLUE:
		strncpy(g_nv_data, "blue", sizeof(g_nv_data));
		break;
	case GOLD:
		strncpy(g_nv_data, "gold", sizeof(g_nv_data));
		break;
	case SILVER:
		strncpy(g_nv_data, "silver", sizeof(g_nv_data));
		break;
	case GRAY:
		strncpy(g_nv_data, "gray", sizeof(g_nv_data));
		break;
	case CAFE:
	case CAFE2:
		strncpy(g_nv_data, "cafe", sizeof(g_nv_data));
		break;
	case BLACK2:
		strncpy(g_nv_data, "pdblack", sizeof(g_nv_data));
		break;
	case GREEN:
		strncpy(g_nv_data, "green", sizeof(g_nv_data));
		break;
	default:
		strncpy(g_nv_data, "", sizeof(g_nv_data));
		break;
	}

	schedule_work(&nv_write_work);
	return ret;
}

static ssize_t attr_get_tp_color_info(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	u8 lcd_id = 0;
	u8 phone_color = 0;
	lcd_id = read_tp_color();
	if (lcd_id != 0xff) {
		hwlog_info("lcd id is %u from read tp color\n", lcd_id);
	}
	hwlog_info("%s:tp id=%x   lcd id=%x.\n", __func__, cypress_tp_color[0],
		   lcd_id);
	reinit_completion(&nv_read_done);
	if (is_color_correct(cypress_tp_color[0]))
		phone_color = cypress_tp_color[0];
	else if (is_color_correct(lcd_id))
		phone_color = lcd_id;
#if (defined CONFIG_HUAWEI_TP_KIT_COLOR_INFO) || (defined CONFIG_HUAWEI_TP_KIT_COLOR_INFO_3_0)
	else if (is_color_correct(cypress_ts_kit_color[FIRST_ELEMENT])) {
		phone_color = cypress_ts_kit_color[FIRST_ELEMENT];
	}
#endif
	else {
		schedule_work(&nv_read_work);
		if (!wait_for_completion_interruptible_timeout(&nv_read_done, msecs_to_jiffies(NV_READ_TIMEOUT))) {
			hwlog_info("nv_read_work time out\n");
			strncpy(g_nv_data, "", sizeof(""));
			return strlen(buf);
		}
		strncpy(buf, g_nv_data, strlen(g_nv_data));
		buf[strlen(g_nv_data)] = '\0';
		hwlog_info("nv_read_color is %s\n",buf);
		return strlen(buf);
	}
	switch (phone_color) {
	case WHITE:
		strncpy(buf, "white", sizeof("white"));
		break;
	case BLACK:
		strncpy(buf, "black", sizeof("black"));
		break;
	case PINK:
	case PINKGOLD:
		strncpy(buf, "pink", sizeof("pink"));
		break;
	case RED:
		strncpy(buf, "red", sizeof("red"));
		break;
	case YELLOW:
		strncpy(buf, "yellow", sizeof("yellow"));
		break;
	case BLUE:
		strncpy(buf, "blue", sizeof("blue"));
		break;
	case GOLD:
		strncpy(buf, "gold", sizeof("gold"));
		break;
	case SILVER:
		strncpy(buf, "silver", sizeof("silver"));
		break;
	case GRAY:
		strncpy(buf, "gray", sizeof("gray"));
		break;
	case CAFE:
	case CAFE2:
		strncpy(buf, "cafe", sizeof("cafe"));
		break;
	case BLACK2:
		strncpy(buf, "pdblack", sizeof("pdblack"));
		break;
	case GREEN:
		strncpy(buf, "green", sizeof("green"));
		break;
	default:
		strncpy(buf, "", sizeof(""));
		break;
	}
	return strlen(buf);
}

static ssize_t attr_set_tp_color_info(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	unsigned long val = simple_strtoul(buf, NULL, 10);
	hwlog_info("[%s] val=%lu\n", __func__, val);
	if (val == 1) {
		write_tp_color_to_nv();
	}
	return count;
}

static struct device_attribute tp_color_file =
__ATTR(tp_color_info, 0664, attr_get_tp_color_info, attr_set_tp_color_info);

static struct platform_device huawei_tp_color = {
	.name = "huawei_tp_color",
	.id = -1,
};

static int __init tp_color_info_init(void)
{
	int ret = 0;
	hwlog_info("[%s] ++", __func__);

	ret = platform_device_register(&huawei_tp_color);
	if (ret) {
		hwlog_err("%s: platform_device_register failed, ret:%d.\n",
			  __func__, ret);
		goto REGISTER_ERR;
	}
	if (device_create_file(&huawei_tp_color.dev, &tp_color_file)) {
		hwlog_err("%s:Unable to create interface\n", __func__);
		goto SYSFS_CREATE_FILE_ERR;
	}
	INIT_WORK(&nv_read_work, tp_nv_read_work_fn);
	INIT_WORK(&nv_write_work, tp_nv_write_work_fn);
	init_completion(&nv_read_done);
	hwlog_info("[%s] --", __func__);
	return 0;

SYSFS_CREATE_FILE_ERR:
	platform_device_unregister(&huawei_tp_color);
REGISTER_ERR:
	return ret;

}

device_initcall(tp_color_info_init);
MODULE_DESCRIPTION("tp color info");
MODULE_AUTHOR("huawei driver group of k3");
MODULE_LICENSE("GPL");
