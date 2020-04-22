/*
 * kbhub_channel.c
 *
 * Single wire UART Keyboard Channel driver
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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <huawei_platform/inputhub/kbhub.h>
#include <huawei_platform/inputhub/sensorhub.h>
#include <huawei_platform/log/log_exception.h>
#include <huawei_platform/log/hw_log.h>
#include "protocol.h"
#include "contexthub_route.h"
#include "contexthub_boot.h"
#include "kbhub_channel.h"
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif /* CONFIG_HUAWEI_DSM */

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define KB_APP_CONFIG_LEN (16)

#define HWLOG_TAG kbhub_channel
HWLOG_REGIST();

static bool g_kbchannel_status;
static int g_kb_ref_cnt;

static const struct kb_cmd_map_t kb_cmd_map_tab[] = {
	{KBHB_IOCTL_START, -1, TAG_KB, CMD_CMN_OPEN_REQ, SUB_CMD_NULL_REQ},
	{KBHB_IOCTL_STOP, -1, TAG_KB, CMD_CMN_CLOSE_REQ, SUB_CMD_NULL_REQ},
	{KBHB_IOCTL_CMD, -1, TAG_KB, CMD_CMN_CONFIG_REQ, SUB_CMD_KB_EVENT_REQ},
};

static struct kbdev_proxy kbdev_proxy = {
	.ops = NULL,
	.notify_event = NOTIFY_EVENT_NONE,
};

/*
 * provide sw module register to kbhub module method
 * when load sw module, sw module will register kbhub module
 */
int kbdev_proxy_register(struct kb_dev_ops *ops)
{
	int ret = -1;

	if (ops != NULL) {
		kbdev_proxy.ops = ops;
		if (kbdev_proxy.notify_event != NOTIFY_EVENT_NONE &&
			ops->notify_event) {
			ops->notify_event(kbdev_proxy.notify_event);
			kbdev_proxy.notify_event = NOTIFY_EVENT_NONE;
		}
		ret = 0;
	}

	return ret;
}
EXPORT_SYMBOL(kbdev_proxy_register);

static int send_kb_cmd_internal(int tag, obj_cmd_t cmd,
	obj_sub_cmd_t subcmd, enum kb_type_t type, bool use_lock)
{
	uint8_t app_config[KB_APP_CONFIG_LEN] = {0};
	interval_param_t interval_param;

	if (type >= KB_TYPE_END)
		return -EINVAL;

	app_config[0] = type;
	app_config[1] = cmd;

	memset(&interval_param, 0, sizeof(interval_param));

	if (cmd == CMD_CMN_OPEN_REQ) {
		g_kbchannel_status = true;
		if (really_do_enable_disable(&g_kb_ref_cnt, true, type)) {
			if (use_lock) {
				inputhub_sensor_enable(tag, true);
				inputhub_sensor_setdelay(tag, &interval_param);
			} else {
				inputhub_sensor_enable_nolock(tag, true);
				inputhub_sensor_setdelay_nolock(tag,
					&interval_param);
			}
		}
		send_app_config_cmd(TAG_KB, app_config, use_lock);
	} else if (cmd == CMD_CMN_CLOSE_REQ) {
		send_app_config_cmd(TAG_KB, app_config, use_lock);
		if (really_do_enable_disable(&g_kb_ref_cnt, false, type)) {
			if (use_lock)
				inputhub_sensor_enable(tag, false);
			else
				inputhub_sensor_enable_nolock(tag, false);
		}
	} else {
		hwlog_err("%s unknown cmd\n", __func__);
		return -EINVAL;
	}
	return 0;
}

void enable_kb_when_recovery_iom3(void)
{
	g_kb_ref_cnt = 0;
	hwlog_info("%s enter\n", __func__);
	/* notify sensorhub start work */
	if (g_kbchannel_status) {
		hwlog_info("%s enable kbchannel\n", __func__);
		send_kb_cmd_internal(TAG_KB, CMD_CMN_OPEN_REQ,
			SUB_CMD_NULL_REQ, 0, false);
	}
}

void disable_kb_when_sysreboot(void)
{
	hwlog_info("%s enter\n", __func__);
}

/*
 * Provide function to send command to sensorhub kb app
 */
int kernel_send_kb_cmd(unsigned int cmd, int val)
{
	int i;

	hwlog_info("%s enter\n", __func__);
	for (i = 0; i < ARRAY_SIZE(kb_cmd_map_tab); i++) {
		if (kb_cmd_map_tab[i].fhb_ioctl_app_cmd == cmd)
			break;
	}
	if (i == ARRAY_SIZE(kb_cmd_map_tab)) {
		hwlog_err("%s unknown cmd %d in parse_ca_cmd\n",
			__func__, cmd);
		return -EFAULT;
	}

	if (!(val >= KB_TYPE_START && val < KB_TYPE_END)) {
		hwlog_err("%s error, kb type %d\n", __func__, val);
		return -EINVAL;
	}
	return send_kb_cmd_internal(kb_cmd_map_tab[i].tag,
		kb_cmd_map_tab[i].cmd, kb_cmd_map_tab[i].subcmd, val, true);
}
EXPORT_SYMBOL(kernel_send_kb_cmd);

/*
 * Provide function to send report event to sensorhub kb app
 */
int kernel_send_kb_report_event(unsigned int cmd, void *buffer, int size)
{
	int ret;
	int i;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	struct kb_outreport_t outreport;

	hwlog_info("%s enter\n", __func__);

	if ((size <= 0) || (size > KBHUB_REPORT_DATA_SIZE) || (buffer == NULL))
		return -EFAULT;

	for (i = 0; i < ARRAY_SIZE(kb_cmd_map_tab); i++) {
		if (kb_cmd_map_tab[i].fhb_ioctl_app_cmd == cmd)
			break;
	}
	if (i == ARRAY_SIZE(kb_cmd_map_tab)) {
		hwlog_err("%s unknown cmd %d in parse_ca_cmd\n",
			__func__, cmd);
		return -EFAULT;
	}

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	memset(&outreport, 0, sizeof(outreport));

	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.tag = kb_cmd_map_tab[i].tag;
	outreport.sub_cmd = kb_cmd_map_tab[i].subcmd;
	outreport.report_len = size;
	memcpy(outreport.report_data, buffer, size);
	pkg_ap.wr_buf = &outreport;
	pkg_ap.wr_len = sizeof(outreport);
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret) {
		hwlog_err("%s write cmd err\n", __func__);
		return -1;
	}

	if (pkg_mcu.errno != 0) {
		hwlog_info("%s mcu err\n", __func__);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(kernel_send_kb_report_event);

void kbhb_notify_mcu_state(sys_status_t status)
{
	if (status == ST_MCUREADY) {
		if (kbdev_proxy.ops && kbdev_proxy.ops->notify_event) {
			hwlog_info("%s notify_event NOTIFY_EVENT_DETECT\n",
				__func__);
			kbdev_proxy.ops->notify_event(NOTIFY_EVENT_DETECT);
		} else {
			hwlog_info("%s kbdev_proxy.ops is null, not notify\n",
				__func__);
			kbdev_proxy.notify_event = NOTIFY_EVENT_DETECT;
		}
	}
}

int kbhb_get_hall_value(void)
{
	/* scm keyboard disable hall, default no coverd */
	return 0;
}
EXPORT_SYMBOL(kbhb_get_hall_value);

static int kb_recovery_notifier(struct notifier_block *nb,
	unsigned long foo, void *bar)
{
	/* prevent access the emmc now: */
	hwlog_info("%s (%lu) +\n", __func__, foo);
	switch (foo) {
	case IOM3_RECOVERY_IDLE:
	case IOM3_RECOVERY_START:
	case IOM3_RECOVERY_MINISYS:
	case IOM3_RECOVERY_3RD_DOING:
	case IOM3_RECOVERY_FAILED:
		break;
	case IOM3_RECOVERY_DOING:
		enable_kb_when_recovery_iom3();
		break;
	default:
		hwlog_err("%s -unknow state %ld\n", __func__, foo);
		break;
	}
	hwlog_info("%s\n", __func__);
	return 0;
}

static struct notifier_block kb_recovery_notify = {
	.notifier_call = kb_recovery_notifier,
	.priority = -1,
};

#ifdef USE_KBHB_DEVICE
static ssize_t kbhb_read(struct file *file, char __user *buf,
	size_t count, loff_t *pos)
{
	hwlog_info("%s enter\n", __func__);
	return 0;
}

static ssize_t kbhb_write(struct file *file, const char __user *data,
	size_t len, loff_t *ppos)
{
	hwlog_info("%s\n", __func__);
	return len;
}

static long kbhb_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	hwlog_info("%s cmd: [%d]\n", __func__, cmd);
	switch (cmd) {
	case KBHB_IOCTL_START:
		break;
	case KBHB_IOCTL_STOP:
		break;
	case KBHB_IOCTL_CMD:
		break;
	default:
		hwlog_err("%s unknown cmd : %d\n", __func__, cmd);
		return -ENOTTY;
	}

	return 0;
}

static int kbhb_open(struct inode *inode, struct file *file)
{
	hwlog_info("%s enter\n", __func__);
	return 0;
}

static int kbhb_release(struct inode *inode, struct file *file)
{
	hwlog_info("%s enter\n", __func__);
	return 0;
}

static const struct file_operations kbhb_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.read = kbhb_read,
	.write = kbhb_write,
	.unlocked_ioctl = kbhb_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = kbhb_ioctl,
#endif
	.open = kbhb_open,
	.release = kbhb_release,
};

static struct miscdevice kbhub_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "kbhub",
	.fops = &kbhb_fops,
};
#endif /* USE_KBHB_DEVICE */

/*
 * according to register_mcu_event_notifier , receive kb report data
 * if receive KBHB_MCUREADY event , notify sw module for detect devices
 * and report kb data to sw module
 */
static int kb_report_callback(const pkt_header_t *head)
{
	int ret = -1;
	int count;
	char *kb_data;

	if (head == NULL)
		return -1;

	kb_data = (char *)head + sizeof(pkt_header_t);

	count = kb_data[1];

	if (kb_data[0] == KBHB_MCUREADY) {
		hwlog_info("%s mcu ready\n", __func__);
		kbhb_notify_mcu_state(ST_MCUREADY);
	} else {
		if ((kbdev_proxy.ops != NULL) &&
			(kbdev_proxy.ops->process_kbdata != NULL))
			ret = kbdev_proxy.ops->process_kbdata(kb_data, count);
	}
	return ret;
}

static int is_kbhub_disabled(void)
{
	int len;
	struct device_node *sh_node;
	const char *sh_status;
	int ret;

	sh_node = of_find_compatible_node(NULL, NULL, "huawei,sw_kb");
	if (sh_node == NULL) {
		hwlog_err("%s can not find node kbhub_status\n", __func__);
		return -1;
	}

	sh_status = of_get_property(sh_node, "status", &len);
	if (sh_status == NULL) {
		hwlog_err("%s can't find property status\n", __func__);
		return -1;
	}

	if (strstr(sh_status, "ok")) {
		hwlog_info("%s kbhub enabled\n", __func__);
		ret = 0;
	} else {
		hwlog_info("%s kbhub disabled\n", __func__);
		ret = -1;
	}

	return ret;
}

static int __init kbhub_init(void)
{
	int ret;

	if (is_sensorhub_disabled()) {
		hwlog_err("%s sensorhub disabled\n", __func__);
		return -1;
	}

	if (is_kbhub_disabled()) {
		hwlog_err("%s kbhub disabled\n", __func__);
		return -1;
	}

	hwlog_info("%s enter\n", __func__);
	ret = inputhub_route_open(ROUTE_KB_PORT);
	if (ret != 0) {
		hwlog_err("%s cannot open inputhub route err=%d\n",
			__func__, ret);
		return ret;
	}
#ifdef USE_KBHB_DEVICE
	ret = misc_register(&kbhub_miscdev);
	if (ret != 0) {
		hwlog_err("%s cannot register miscdev err=%d\n", __func__, ret);
		inputhub_route_close(ROUTE_KB_PORT);
		return ret;
	}
#endif /* USE_KBHB_DEVICE */
	register_mcu_event_notifier(TAG_KB, CMD_DATA_REQ, kb_report_callback);
	register_iom3_recovery_notifier(&kb_recovery_notify);
	hwlog_info("%s ok\n", __func__);
	return 0;
}

static void __exit kbhub_exit(void)
{
	hwlog_info("%s enter\n", __func__);
	inputhub_route_close(ROUTE_KB_PORT);
#ifdef USE_KBHB_DEVICE
	misc_deregister(&kbhub_miscdev);
#endif
	hwlog_info("%s exit\n", __func__);
}

late_initcall_sync(kbhub_init);
module_exit(kbhub_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("KBHub driver 20");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
