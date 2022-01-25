/*
 * sw_core.c
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
#include "sw_detect.h"

#define KB_DEVICE_NAME         "hwsw_kb"
#define VERSION                "1.0"
#define SCREEN_ON              (0)
#define SCREEN_OFF             (1)
#define RECV_DATA_DELAY_MS     (5)
#define KB_POWERKEY_DATA_COUNT (7)

struct hw_sw_core_data {
	struct sk_buff_head tx_data_seq;
	struct sk_buff_head rx_data_seq;
	struct work_struct events;

	void *pm_data;

	struct sw_device *device;
	u16 kb_state;
	u32 product;
};

struct hw_sw_disc_data {
	struct platform_device *pm_pdev;
	struct hw_sw_core_data *core_data;
	struct wake_lock kb_wakelock;
	struct delayed_work kb_channel_work;
	struct completion kb_comm_complete;

	int kb_online;
	int fb_state;
	int wait_fb_on;

	struct sw_detector_ops detect_ops;
};

struct platform_device *hw_sw_device;
static int g_sw_state = -1;
static struct class *hwkb_class;
static struct device *hwkb_device;

/* workqueue to process sw events */
static struct workqueue_struct *sw_wq;
static void sw_core_event(struct work_struct *work);

static struct hw_sw_disc_data *get_disc_data(void)
{
	struct platform_device *pdev;
	struct hw_sw_disc_data *pdisc_data;

	pdev = hw_sw_device;
	if (pdev == NULL) {
		SW_PRINT_ERR("pdev is null\n");
		return NULL;
	}

	pdisc_data = dev_get_drvdata(&pdev->dev);
	if (pdisc_data == NULL) {
		SW_PRINT_ERR("pdisc_data is null\n");
		return NULL;
	}

	return pdisc_data;
}

static ssize_t stateinfo_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int len = 0;
	int ret = 0;
	int buf_len = PAGE_SIZE - 1;
	char *cur = buf;
	struct hw_sw_disc_data *pdisc_data;
	struct hw_sw_core_data *pcore_data;

	SW_PRINT_FUNCTION_NAME;

	pdisc_data = get_disc_data();
	if (pdisc_data != NULL) {
		pcore_data = pdisc_data->core_data;
		if (pcore_data != NULL) {
			ret = snprintf(cur, buf_len, "keyboard is %s\n",
				(pcore_data->kb_state == KBSTATE_ONLINE) ?
				"online" : "offline");
			buf_len -= ret;
			cur += ret;
			len += ret;

			if (pcore_data->kb_state == KBSTATE_ONLINE) {
				ret = snprintf(cur, buf_len,
					"keyboard vendor id is 0x%04X\n",
					pcore_data->device->vendor);
				buf_len -= ret;
				cur += ret;
				len += ret;

				ret = snprintf(cur, buf_len,
					"keyboard version is %d.%d\n",
					pcore_data->device->mainver,
					pcore_data->device->subver);
				buf_len -= ret;
				cur += ret;
				len += ret;
			}
		}
		buf_len -= ret;
		cur += ret;
		len += ret;
	} else {
		len = snprintf(buf, PAGE_SIZE, "no support\n");
	}
	return (len >= PAGE_SIZE) ? (PAGE_SIZE - 1) : len;
}

static DEVICE_ATTR_RO(stateinfo);

static struct attribute *sw_dev_attrs[] = {
	&dev_attr_stateinfo.attr,
	NULL,
};
static const struct attribute_group sw_platform_group = {
	.attrs = sw_dev_attrs,
};

static int sw_get_core_reference(struct hw_sw_core_data **core_data)
{
	struct platform_device *pdev;
	struct hw_sw_disc_data *pdisc_data;

	pdev = hw_sw_device;
	if (pdev == NULL) {
		*core_data = NULL;
		SW_PRINT_ERR("pdev is null\n");
		return FAILURE;
	}

	pdisc_data = dev_get_drvdata(&pdev->dev);
	if (pdisc_data == NULL) {
		*core_data = NULL;
		SW_PRINT_ERR("pdisc_data is null\n");
		return FAILURE;
	}

	*core_data = pdisc_data->core_data;

	return SUCCESS;
}

static void sw_recv_data_frame(struct hw_sw_core_data *core_data,
	struct sk_buff *skb)
{
	if ((core_data == NULL) || (skb == NULL))
		return;

	if (core_data->device == NULL)
		return;

	if ((core_data->device->drv == NULL) ||
		(core_data->device->drv->recvframe == NULL))
		return;

	core_data->device->drv->recvframe(core_data->device, skb);
}

static int sw_connect_device(struct hw_sw_core_data *core_data,
	struct sk_buff *skb)
{
	int ret;
	u32 vendorid;
	struct sw_device *device;

	if ((skb == NULL) || (skb->len < KB_HANDSHAKE_LEN))
		return -1;

	vendorid = (u32)((skb->data[KB_VID_HIGH_BYTE] << BITS_NUM_PER_BYTE) |
		skb->data[KB_VID_LOW_BYTE]);

	device = sw_create_new_device(core_data->product, vendorid);
	if (device == NULL)
		return -1;

	device->mainver = (u8)skb->data[KB_MAIN_VERSION_BYTE];
	device->subver = (u8)skb->data[KB_SUB_VERSION_BYTE];
	device->dev.parent = &hw_sw_device->dev;

	ret = device_register(&device->dev);
	if (ret)
		SW_PRINT_ERR("device_add %x\n", ret);

	core_data->device = device;
	core_data->kb_state = KBSTATE_ONLINE;

	return ret;
}

static int sw_disconnect_device(struct hw_sw_core_data *core_data)
{
	struct sw_device *device;
	int err = 0;

	if (core_data == NULL)
		return 0;

	if (core_data->device != NULL) {
		device = core_data->device;
		core_data->device = NULL;
		err = sw_release_device(device);
	}

	return err;
}

static void swkb_wake_lock_timeout(struct hw_sw_disc_data *pdisc_data,
	long timeout)
{
	if (!wake_lock_active(&pdisc_data->kb_wakelock)) {
		wake_lock_timeout(&pdisc_data->kb_wakelock, timeout);
		SW_PRINT_INFO("swkb wake lock\n");
	}
}

static void swkb_wake_lock(struct hw_sw_disc_data *pdisc_data)
{
	if (!wake_lock_active(&pdisc_data->kb_wakelock)) {
		wake_lock(&pdisc_data->kb_wakelock);
		SW_PRINT_INFO("swkb wake lock\n");
	}
}

static void swkb_wake_unlock(struct hw_sw_disc_data *pdisc_data)
{
	if (wake_lock_active(&pdisc_data->kb_wakelock)) {
		wake_unlock(&pdisc_data->kb_wakelock);
		SW_PRINT_INFO("swkb wake unlock\n");
	}
}

static void sw_notify_android_uevent(int is_conn_or_disconn)
{
	char *disconnected[KB_STATE_MAX] = { "KB_STATE=DISCONNECTED", NULL };
	char *connected[KB_STATE_MAX] = { "KB_STATE=CONNECTED", NULL };

	if (IS_ERR(hwkb_device)) {
		SW_PRINT_INFO("sw_notify_android_uevent device uninit\n");
		return;
	}
	switch (is_conn_or_disconn) {
	case KBSTATE_ONLINE:
		kobject_uevent_env(&hwkb_device->kobj,
			KOBJ_CHANGE, connected);
		SW_PRINT_DBG("sw_notify_android_uevent connected\n");
		break;
	case KBSTATE_OFFLINE:
		kobject_uevent_env(&hwkb_device->kobj,
			KOBJ_CHANGE, disconnected);
		SW_PRINT_DBG("sw_notify_android_uevent disconnected\n");
		break;
	default:
		SW_PRINT_ERR("is_conn_or_disconn = %d, parameter is invalid\n",
			is_conn_or_disconn);
		break;
	}
}

static int sw_core_init(struct hw_sw_core_data **core_data)
{
	struct hw_sw_core_data *psw_core_data;
	struct device_node *np;
	u32 val;

	SW_PRINT_FUNCTION_NAME;

	psw_core_data = kzalloc(sizeof(*psw_core_data), GFP_KERNEL);
	if (psw_core_data == NULL)
		return -ENOMEM;

	skb_queue_head_init(&psw_core_data->rx_data_seq);
	skb_queue_head_init(&psw_core_data->tx_data_seq);

	psw_core_data->device = NULL;
	/* product default cmr keyboard product id */
	psw_core_data->product = CMR_KEYBOARD_PID;

	INIT_WORK(&psw_core_data->events, sw_core_event);

	np = of_find_compatible_node(NULL, NULL, "huawei,sw_kb");
	if (np == NULL) {
		SW_PRINT_ERR("unable to find %s\n", "huawei, sw_kb");
		return -ENOENT;
	}

	if (of_property_read_u32(np, "product_id", &val))
		SW_PRINT_INFO("dts:can not get product_id\n");
	else
		psw_core_data->product = val;

	SW_PRINT_INFO("kb product_id:0x%x\n", psw_core_data->product);

	*core_data = psw_core_data;
	return 0;
}

static int sw_skb_enqueue(struct hw_sw_core_data *psw_core_data,
	struct sk_buff *skb, u8 type)
{
	if (unlikely(psw_core_data == NULL)) {
		SW_PRINT_ERR("psw_core_data is null\n");
		return -EINVAL;
	}

	if (unlikely(skb == NULL)) {
		SW_PRINT_ERR("skb is null\n");
		return -EINVAL;
	}

	switch (type) {
	case TX_DATA_QUEUE:
		skb_queue_tail(&psw_core_data->tx_data_seq, skb);
		break;
	case RX_DATA_QUEUE:
		skb_queue_tail(&psw_core_data->rx_data_seq, skb);
		break;
	default:
		SW_PRINT_ERR("queue type is error, type=%d\n", type);
		break;
	}

	return 0;
}

static int sw_push_skb_queue(struct hw_sw_core_data *psw_core_data,
	u8 *buf_ptr, int pkt_len, u8 type)
{
	struct sk_buff *skb;

	if (psw_core_data == NULL) {
		SW_PRINT_ERR("psw_core_data is null\n");
		return -EINVAL;
	}

	if (buf_ptr == NULL) {
		SW_PRINT_ERR("buf_ptr is null\n");
		return -EINVAL;
	}

	skb = alloc_skb(pkt_len, GFP_ATOMIC);
	if (skb == NULL) {
		SW_PRINT_ERR("can't allocate mem for new skb,len=%d\n",
			pkt_len);
		return -EINVAL;
	}

	skb_put(skb, pkt_len);
	memcpy(skb->data, buf_ptr, pkt_len);
	sw_skb_enqueue(psw_core_data, skb, type);

	return 0;
}

static struct sk_buff *sw_skb_dequeue(struct hw_sw_core_data *psw_core_data,
	u8 type)
{
	struct sk_buff *curr_skb = NULL;

	if (psw_core_data == NULL) {
		SW_PRINT_ERR("psw_core_data is null\n");
		return NULL;
	}

	switch (type) {
	case TX_DATA_QUEUE:
		curr_skb = skb_dequeue(&psw_core_data->tx_data_seq);
		break;
	case RX_DATA_QUEUE:
		curr_skb = skb_dequeue(&psw_core_data->rx_data_seq);
		break;
	default:
		SW_PRINT_ERR("queue type is error, type=%d\n", type);
		break;
	}

	return curr_skb;
}

static void sw_kfree_skb(struct hw_sw_core_data *psw_core_data, u8 type)
{
	struct sk_buff *skb;

	SW_PRINT_FUNCTION_NAME;
	if (psw_core_data == NULL) {
		SW_PRINT_ERR("psw_core_data is null");
		return;
	}

	while ((skb = sw_skb_dequeue(psw_core_data, type)))
		kfree_skb(skb);

	switch (type) {
	case TX_DATA_QUEUE:
		skb_queue_purge(&psw_core_data->tx_data_seq);
		break;
	case RX_DATA_QUEUE:
		skb_queue_purge(&psw_core_data->rx_data_seq);
		break;
	default:
		SW_PRINT_ERR("queue type is error, type=%d\n", type);
		break;
	}
}

static int sw_core_exit(struct hw_sw_core_data *pcore_data)
{
	SW_PRINT_FUNCTION_NAME;

	if (pcore_data != NULL) {
		sw_disconnect_device(pcore_data);
		/* free sw tx and rx queue */
		sw_kfree_skb(pcore_data, TX_DATA_QUEUE);
		sw_kfree_skb(pcore_data, RX_DATA_QUEUE);
		kfree(pcore_data);
	}
	return 0;
}

static void kick_sw_wq(void)
{
	struct hw_sw_core_data *sw_core_data;
	int ret;

	ret = sw_get_core_reference(&sw_core_data);
	if ((ret != SUCCESS) || (sw_core_data == NULL))
		return;

	if (work_pending(&sw_core_data->events))
		return;

	if (queue_work(sw_wq, &sw_core_data->events))
		return;
}

static void sw_send_power_key(struct hw_sw_core_data *core_data)
{
	char powerkey_down[] = { 0x0, 0x7, 0x0, 0x0, 0x0, 0x30, 0x0 };
	char powerkey_up[] = { 0x0, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0 };
	struct sk_buff *skb;

	if (core_data == NULL)
		return;
	SW_PRINT_DBG("sw_send_power_key ++++\n");

	skb = alloc_skb(KB_POWERKEY_DATA_COUNT, GFP_ATOMIC);
	if (skb == NULL) {
		SW_PRINT_ERR("can't allocate mem for new skb\n");
		return;
	}

	skb_put(skb, KB_POWERKEY_DATA_COUNT);
	memcpy(skb->data, powerkey_down, KB_POWERKEY_DATA_COUNT);
	sw_recv_data_frame(core_data, skb);
	msleep(RECV_DATA_DELAY_MS);
	memcpy(skb->data, powerkey_up, KB_POWERKEY_DATA_COUNT);
	sw_recv_data_frame(core_data, skb);
	kfree_skb(skb);
	SW_PRINT_DBG("sw_send_power_key ----\n");
}

/*
 * Note:handshake data format
 * cmd(1 byte) + devno(1 byte) + len(1 byte) + crc(2 bytes) +
 * payload(1 byte) + vendor(2 bytes)+ mainver(1 byte) +  subver(1 byte)
 */
static int sw_process_cmd_handshake(struct hw_sw_core_data *core_data,
	struct sk_buff *skb)
{
	int ret;

	SW_PRINT_DBG("sw_core_event connect keyboard\n");

	if (core_data->device == NULL) {
		ret = sw_connect_device(core_data, skb);
		sw_notify_android_uevent(KBSTATE_ONLINE);
	} else {
		ret = 0;
	}

	return ret;
}

static int sw_process_cmd_disconnect(struct hw_sw_core_data *core_data,
	struct sk_buff *skb)
{
	int ret;
	struct hw_sw_disc_data *pdisc_data;

	pdisc_data = (struct hw_sw_disc_data *)(core_data->pm_data);
	if (pdisc_data == NULL)
		return -1;

	SW_PRINT_DBG("sw_core_event disconnect keyboard\n");

	/* disable kb device */
	ret = sw_disconnect_device(core_data);
	core_data->kb_state = KBSTATE_OFFLINE;

	if (pdisc_data->detect_ops.notify != NULL)
		pdisc_data->detect_ops.notify(pdisc_data->detect_ops.detector,
			SW_NOTIFY_EVENT_DISCONNECT, pdisc_data);

	if (ret == 0) {
		swkb_wake_lock_timeout(pdisc_data, DEFAULT_WAKE_TIMEOUT);
		kernel_send_kb_cmd(KBHB_IOCTL_STOP, 0);
		sw_notify_android_uevent(KBSTATE_OFFLINE);
	}
	return ret;
}

static int sw_process_cmd_default(struct hw_sw_core_data *core_data,
	struct sk_buff *skb)
{
	struct hw_sw_disc_data *pdisc_data;

	pdisc_data = (struct hw_sw_disc_data *)(core_data->pm_data);
	if (pdisc_data == NULL)
		return -1;

	if (pdisc_data->fb_state == SCREEN_OFF) {
		if (pdisc_data->wait_fb_on == SCREEN_ON) {
			pdisc_data->wait_fb_on = SCREEN_OFF;
			sw_send_power_key(core_data);
		}
	}

	if (pdisc_data->fb_state == SCREEN_ON) {
		if (pdisc_data->wait_fb_on == SCREEN_OFF)
			pdisc_data->wait_fb_on = SCREEN_ON;
		sw_recv_data_frame(core_data, skb);
	}

	return 0;
}

static void sw_core_event(struct work_struct *work)
{
	struct hw_sw_core_data *core_data;
	struct sk_buff *skb;
	u8 hdr;

	SW_PRINT_FUNCTION_NAME;

	core_data = container_of(work, struct hw_sw_core_data, events);
	if (core_data == NULL)
		return;

	while ((skb = sw_skb_dequeue(core_data, RX_DATA_QUEUE))) {
		skb_orphan(skb);
		if (!skb_linearize(skb)) {
			hdr = (u8)skb->data[0];
			switch (hdr) {
			case PROTO_CMD_HANDSHAKE:
				sw_process_cmd_handshake(core_data, skb);
				break;
			case PROTO_CMD_DISCONNECT:
				sw_process_cmd_disconnect(core_data, skb);
				break;
			case PROTO_CMD_WORKMODE:
				sw_recv_data_frame(core_data, skb);
				break;
			default:
				sw_process_cmd_default(core_data, skb);
				break;
			}
		}

		kfree_skb(skb);
	}
}

static int sw_core_recv(void *core_data, u8 *data, int count)
{
	struct hw_sw_core_data *sw_core_data;
	int ret;

	SW_PRINT_FUNCTION_NAME;
	sw_core_data = (struct hw_sw_core_data *)core_data;
	if (sw_core_data == NULL) {
		SW_PRINT_ERR("core_data is null\n");
		return 0;
	}

	if (unlikely(data == NULL)) {
		SW_PRINT_ERR("received null\n");
		return 0;
	}

	if (count < 1) {
		SW_PRINT_ERR("received count error\n");
		return 0;
	}

	ret = sw_push_skb_queue(sw_core_data, (u8 *)data, count, RX_DATA_QUEUE);
	if (ret == 0)
		kick_sw_wq();

	return ret;
}

static int sw_poweroff_notify_sys(struct notifier_block *this,
	unsigned long code, void *unused)
{
	return 0;
}

static struct notifier_block plat_poweroff_notifier = {
	.notifier_call = sw_poweroff_notify_sys,
};

static void sw_fb_notifier_action(int screen_on_off)
{
	struct hw_sw_disc_data *pdisc_data;

	pdisc_data = get_disc_data();
	if (pdisc_data == NULL)
		return;

	pdisc_data->fb_state = screen_on_off;
	if ((pdisc_data->fb_state == SCREEN_ON) &&
			(pdisc_data->wait_fb_on == SCREEN_OFF))
		pdisc_data->wait_fb_on = SCREEN_ON;
}

static int sw_fb_notifier(struct notifier_block *nb,
	unsigned long action, void *data)
{
	int *blank;
	struct fb_event *event = data;

	if (event == NULL) {
		SW_PRINT_ERR("event is null\n");
		return NOTIFY_OK;
	}

	blank = event->data;
	if (blank == NULL) {
		SW_PRINT_ERR("blank is null\n");
		return NOTIFY_OK;
	}

	switch (action) {
	case FB_EVENT_BLANK: /* change finished */
		switch (*blank) {
		case FB_BLANK_UNBLANK: /* screen on */
			SW_PRINT_INFO("screen on\n");
			sw_fb_notifier_action(SCREEN_ON);
			break;

		case FB_BLANK_POWERDOWN: /* screen off */
			SW_PRINT_INFO("screen off\n");
			sw_fb_notifier_action(SCREEN_OFF);
			break;

		default:
			SW_PRINT_ERR("unknown---> lcd unknown\n");
			break;
		}
		break;

	default:
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block fb_notify = {
	.notifier_call = sw_fb_notifier,
};

static void swkb_channel_com_work(struct work_struct *work)
{
	unsigned long timeout;
	struct hw_sw_disc_data *pdisc_data = container_of(work,
		struct hw_sw_disc_data, kb_channel_work.work);

	if (pdisc_data == NULL)
		return;

	SW_PRINT_INFO("start\n");
	swkb_wake_lock(pdisc_data);
	timeout = wait_for_completion_timeout(&pdisc_data->kb_comm_complete,
		msecs_to_jiffies(KB_COMM_COMPLETE_TIMEOUT_MS));
	if (timeout == 0) {
		SW_PRINT_ERR("wait_for_completion_timeout timeout\n");
		goto err_timeout;
	}

	reinit_completion(&pdisc_data->kb_comm_complete);

err_timeout:
	swkb_wake_unlock(pdisc_data);
	SW_PRINT_INFO("end\n");
}

static void sw_destroy_monitor_device(struct platform_device *pdev)
{
	if (pdev == NULL)
		return;

	if (!IS_ERR(hwkb_device))
		device_destroy(hwkb_device->class, hwkb_device->devt);

	if (!IS_ERR(hwkb_class))
		class_destroy(hwkb_class);

	hwkb_device = NULL;
	hwkb_class = NULL;
}

static void sw_init_monitor_device(struct platform_device *pdev)
{
	int ret;

	if ((hwkb_device != NULL) || (hwkb_class != NULL))
		sw_destroy_monitor_device(pdev);

	hwkb_class = class_create(THIS_MODULE, KB_DEVICE_NAME);
	if (IS_ERR(hwkb_class)) {
		ret = PTR_ERR(hwkb_class);
		goto err_init;
	}

	hwkb_device = device_create(hwkb_class, NULL, 0, NULL, "hwkb");
	if (IS_ERR(hwkb_device)) {
		ret = PTR_ERR(hwkb_device);
		goto err_init;
	}

	return;

err_init:
	SW_PRINT_ERR("sw_init_monitor_device failed %x\n", ret);
	sw_destroy_monitor_device(pdev);
}

/*
 * if dts config hwsw_kb g_sw_state = 0
 * in inputhub  kbhub_channel model will init when  g_sw_state = 0
 */
static int swkb_get_statue_callback(void)
{
	return g_sw_state;
}

static void swkb_notify_event_callback(unsigned int event)
{
	struct hw_sw_disc_data *pdisc_data;

	pdisc_data = get_disc_data();
	if (pdisc_data == NULL)
		return;
	if (pdisc_data->detect_ops.notify != NULL) {
		if (event == NOTIFY_EVENT_DETECT)
			pdisc_data->detect_ops.notify(
				pdisc_data->detect_ops.detector,
				SW_NOTIFY_EVENT_REDETECT,
				pdisc_data);
	}
}

static int swkb_process_kbdata_callback(char *data, int count)
{
	struct hw_sw_core_data *sw_core_data;
	struct sw_driver *swdriver;
	int ret;

	ret = sw_get_core_reference(&sw_core_data);
	if ((ret != SUCCESS) || (sw_core_data == NULL))
		return -1;

	if (sw_core_data->device != NULL) {
		swdriver = sw_core_data->device->drv;
		if (swdriver && swdriver->pre_recvframe) {
			/* if ret != 0 , this frame need ignore */
			ret = swdriver->pre_recvframe(sw_core_data->device,
				data, count);
			if (ret)
				return -1;
		}
	}

	ret = sw_core_recv(sw_core_data, (u8 *)data, count);
	return ret;
}

static struct kb_dev_ops sw_kbhubops = {
	.process_kbdata = swkb_process_kbdata_callback,
	.get_status = swkb_get_statue_callback,
	.notify_event = swkb_notify_event_callback,
};

static int sw_detect_notifier_call(struct notifier_block *detect_nb,
	unsigned long event, void *data)
{
	struct hw_sw_disc_data *pdisc_data = get_disc_data();

	if (pdisc_data != NULL) {
		SW_PRINT_ERR("sw_detect_notifier_call event %lx\n", event);
		if (event == DEVSTAT_KBDEV_ONLINE) {
			swkb_wake_lock_timeout(pdisc_data,
				DEFAULT_WAKE_TIMEOUT);
			/* notify sensorhub start work */
			kernel_send_kb_cmd(KBHB_IOCTL_START, 0);
		}
		pdisc_data->kb_online = event;
	}

	return NOTIFY_OK;
}

static void sw_detectops_destroy(struct sw_detector_ops *ops)
{
	if ((ops != NULL) && (ops->detector != NULL)) {
		ops->notify(ops->detector,
			SW_NOTIFY_EVENT_DESTORY, ops);
		ops->detector = NULL;
	}
}

static int sw_probe(struct platform_device *pdev)
{
	struct hw_sw_disc_data *pdisc_data;
	int ret;

	SW_PRINT_FUNCTION_NAME;

	hw_sw_device = pdev;

	pdisc_data = kzalloc(sizeof(*pdisc_data), GFP_KERNEL);
	if (pdisc_data == NULL)
		return -ENOMEM;

	dev_set_drvdata(&pdev->dev, pdisc_data);

	pdisc_data->detect_ops.detect_nb.notifier_call =
		sw_detect_notifier_call;
	pdisc_data->detect_ops.notify = NULL;

	ret = sw_gpio_detect_register(pdev, &pdisc_data->detect_ops);
	if (ret < 0) {
		SW_PRINT_ERR("sw_gpio_detect_register failed\n");
		goto err_core_init;
	}

	ret = sw_core_init(&pdisc_data->core_data);
	if (ret != 0) {
		SW_PRINT_ERR("sw core init failed\n");
		goto err_core_init;
	}
	/* refer to itself */
	pdisc_data->core_data->pm_data = pdisc_data;
	/* get reference of pdev */
	pdisc_data->pm_pdev = pdev;

	wake_lock_init(&pdisc_data->kb_wakelock,
		WAKE_LOCK_SUSPEND, "swkb_wakelock");
	INIT_DELAYED_WORK(&pdisc_data->kb_channel_work, swkb_channel_com_work);

	init_completion(&pdisc_data->kb_comm_complete);

	pdisc_data->kb_online = KBSTATE_OFFLINE;

	ret = sysfs_create_group(&pdev->dev.kobj, &sw_platform_group);
	if (ret)
		SW_PRINT_ERR("sysfs_create_group error ret = %d\n", ret);

	sw_init_monitor_device(pdev);

	pdisc_data->fb_state = SCREEN_ON;
	pdisc_data->wait_fb_on = SCREEN_ON;

	fb_register_client(&fb_notify);

	ret = register_reboot_notifier(&plat_poweroff_notifier);
	if (ret) {
		SW_PRINT_ERR("register_reboot_notifier fail (err=%d)\n", ret);
		goto err_exception;
	}

	ret = kbdev_proxy_register(&sw_kbhubops);
	if (ret) {
		SW_PRINT_ERR("kbdev_proxy_register fail (err=%d)\n", ret);
		goto err_exception;
	}

	g_sw_state = 0;
	return 0;

err_exception:
	sw_core_exit(pdisc_data->core_data);
err_core_init:
	wake_lock_destroy(&pdisc_data->kb_wakelock);
	sw_detectops_destroy(&pdisc_data->detect_ops);
	kfree(pdisc_data);
	return ret;
}

static int sw_remove(struct platform_device *pdev)
{
	struct hw_sw_disc_data *pdisc_data;

	SW_PRINT_FUNCTION_NAME;

	pdisc_data = dev_get_drvdata(&pdev->dev);
	if (pdisc_data == NULL) {
		SW_PRINT_ERR("pdisc_data is null\n");
	} else {
		pdisc_data->pm_pdev = NULL;
		sw_core_exit(pdisc_data->core_data);
		sw_detectops_destroy(&pdisc_data->detect_ops);

		kfree(pdisc_data);
		pdisc_data = NULL;
	}

	sw_destroy_monitor_device(pdev);
	return 0;
}

static int sw_suspend(struct platform_device *pdev, pm_message_t state)
{
	SW_PRINT_DBG("sw_suspend enter\n");
	return 0;
}

static int sw_resume(struct platform_device *pdev)
{
	SW_PRINT_DBG("sw_resume enter\n");
	return 0;
}

#define HW_SW_NAME "huawei,sw_kb"
static const struct of_device_id sw_match_table[] = {
	{
		.compatible = HW_SW_NAME,
		.data = NULL,
	},
	{ },
};

static struct platform_driver sw_platform_driver = {
	.probe = sw_probe,
	.remove = sw_remove,
	.suspend = sw_suspend,
	.resume = sw_resume,
	.driver = {
		.name = "hwsw_kb",
		.owner = THIS_MODULE,
		.of_match_table = sw_match_table,
	},
};

static int __init sw_init(void)
{
	int ret;

	SW_PRINT_INFO("huawei 3 popo-pin by uart to hid\n");

	ret = sw_drivers_init();
	if (ret) {
		SW_PRINT_ERR("can't register sw bus\n");
		goto err_ret;
	}

	ret = platform_driver_register(&sw_platform_driver);
	if (ret) {
		SW_PRINT_ERR("unable to register platform sw driver\n");
		goto err_bus;
	}

	sw_wq = alloc_workqueue("sw_core_wq", WQ_FREEZABLE, 0);
	if (sw_wq == NULL) {
		SW_PRINT_ERR("unable to alloc sw_core_wq workqueue\n");
		goto err_bus;
	}

	return 0;

err_bus:
	sw_drivers_exit();
err_ret:
	return ret;
}

static void __exit sw_exit(void)
{
	platform_driver_unregister(&sw_platform_driver);
	sw_drivers_exit();
}

module_init(sw_init);
module_exit(sw_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Huawei 3 Popo-pin core driver by UART");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
