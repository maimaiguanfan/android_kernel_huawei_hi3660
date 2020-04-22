/*
 *
 * Copyright (c) 2011-2013 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 *
 * Discription:
 *     3630 using gpio_203 realizing volume-up-key and gpio_204
 * realizing volume-down-key instead of KPC in kernel, only support simple
 * key-press at currunt version, not support combo-keys.
 *     6620 using gpio_12 realizing volume-up-key and gpio_13
 * realizing volume-down-key.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/wakelock.h>
#include <asm/irq.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include "../../hisi/tzdriver/libhwsecurec/securec.h"

#if defined (CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

#ifdef CONFIG_HW_ZEROHUNG
#include <chipset_common/hwzrhung/zrhung.h>
#include <chipset_common/hwzrhung/hung_wp_screen.h>
#endif

#define TRUE                    (1)

#define GPIO_KEY_PRESS          (1)
#define GPIO_KEY_RELEASE        (0)

#ifdef CONFIG_HISI_HI6555V300_PMU
#define  PMIC_GPIO_KEYUP_DATA     0x1ED
#define  PMIC_GPIO_KEYUP_DIR      0x1EF
#define  PMIC_GPIO_KEYUP_IS       0x1F1
#define  PMIC_GPIO_KEYUP_IBE      0x1F2
#define  PMIC_GPIO_KEYUP_PUSEL    0x1F5
#define  PMIC_GPIO_KEYUP_PDSEL    0x1F6
#define  PMIC_GPIO_KEYUP_DEBSEL   0x1F7
#define  PMIC_GPIO_KEYDOWN_DATA   0x1F8
#define  PMIC_GPIO_KEYDOWN_DIR    0x1EA
#define  PMIC_GPIO_KEYDOWN_IS     0x1FC
#define  PMIC_GPIO_KEYDOWN_IBE    0x1FD
#define  PMIC_GPIO_KEYDOWN_PUSEL  0x200
#define  PMIC_GPIO_KEYDOWN_PDSEL  0x201
#define  PMIC_GPIO_KEYDOWN_DEBSEL 0x202
#else
#define  PMIC_GPIO_KEYUP_DATA     0x264
#define  PMIC_GPIO_KEYUP_DIR      0x266
#define  PMIC_GPIO_KEYUP_IS       0x268
#define  PMIC_GPIO_KEYUP_IBE      0x269
#define  PMIC_GPIO_KEYUP_PUSEL    0x26C
#define  PMIC_GPIO_KEYUP_PDSEL    0x26D
#define  PMIC_GPIO_KEYUP_DEBSEL   0x26E
#define  PMIC_GPIO_KEYDOWN_DATA   0x26F
#define  PMIC_GPIO_KEYDOWN_DIR    0x271
#define  PMIC_GPIO_KEYDOWN_IS     0x273
#define  PMIC_GPIO_KEYDOWN_IBE    0x274
#define  PMIC_GPIO_KEYDOWN_PUSEL  0x277
#define  PMIC_GPIO_KEYDOWN_PDSEL  0x278
#define  PMIC_GPIO_KEYDOWN_DEBSEL 0x279
#endif

#define  PMIC_GPIO_KEY_REG_VALUE_ZERO   0x0
#define  PMIC_GPIO_KEY_REG_VALUE_ONE    0x1

#define GPIO_HIGH_VOLTAGE               (1)
#define GPIO_LOW_VOLTAGE                (0)
#define TIMER_DEBOUNCE                  (15)

static struct wake_lock volume_up_key_lock;
static struct wake_lock volume_down_key_lock;
#define HISI_PMIC_GPIO_KEY_IRQ_COUNTS   2

static const char *hisi_pmic_gpio_key_irq_table[HISI_PMIC_GPIO_KEY_IRQ_COUNTS] = {
	"vol-up",
	"vol-down",
};

#if defined (CONFIG_HUAWEI_DSM)
#define PRESS_KEY_INTERVAL             (80)   //the minimum press interval
#define STATISTIC_INTERVAL	       (60) 	//the statistic interval for key event
#define MAX_PRESS_KEY_COUNT            (120)   //the default press count for a normal use

static int volume_up_press_count = 0;
static int volume_down_press_count = 0;
static unsigned long volume_up_last_press_time = 0;
static unsigned long volume_down_last_press_time = 0;
static struct timer_list dsm_gpio_key_timer; //used to reset the statistic variable

static struct dsm_dev dsm_key = {
	.name = "dsm_key",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024,
};

static struct dsm_client *key_dclient = NULL;
#endif

#ifdef CONFIG_VOLUME_KEY_MASK
#define VOLUME_KEY_ENABLE_MASK        (0)
#define VOLUME_KEY_DISABLE_MASK       (1)
static int key_mask_ctl = VOLUME_KEY_ENABLE_MASK;
static int key_mask_flag = VOLUME_KEY_ENABLE_MASK;
#endif

struct hisi_pmic_gpio_key {
	struct input_dev            *input_dev;
	struct delayed_work         gpio_keyup_work;
	struct delayed_work         gpio_keydown_work;
	struct timer_list           key_up_timer;
	struct timer_list           key_down_timer;
	int             gpiokey_irq[HISI_PMIC_GPIO_KEY_IRQ_COUNTS];
};

#if defined (CONFIG_HUAWEI_DSM)
static void dsm_gpio_key_timer_func(unsigned long data)
{
	int key_press_error_found = -1;

	if (!dsm_client_ocuppy(key_dclient)) {
		key_press_error_found++;
	}

	/* judge the press count */
	if ((!key_press_error_found) && (volume_up_press_count > MAX_PRESS_KEY_COUNT)) {
		key_press_error_found++;
		dsm_client_record(key_dclient, "volume up key trigger on the abnormal style.\n");
	} else if ((!key_press_error_found) && (volume_down_press_count > MAX_PRESS_KEY_COUNT)) {
		key_press_error_found++;
		dsm_client_record(key_dclient, "volume down key trigger on the abnormal style.\n");
	}

	if (key_press_error_found > 0) {
		dsm_client_notify(key_dclient, DSM_KEY_ERROR_NO);
	} else if (!key_press_error_found) {
		dsm_client_unocuppy(key_dclient);
	} else {
		/* key_dclient preempt failed, we have nothing to do */
	}

	/* reset the statistic variable */
	volume_up_press_count = 0;
	volume_down_press_count = 0;
	mod_timer(&dsm_gpio_key_timer, jiffies + STATISTIC_INTERVAL * HZ);

	return;
}
#endif

/* read register  */
static unsigned int hisi_pmic_gpio_key_read(int gpio_address)
{
	return hisi_pmic_reg_read(gpio_address);
}

/* write register  */
static void hisi_pmic_gpio_key_write(int gpiokey_set, int gpio_address)
{
	hisi_pmic_reg_write(gpio_address, gpiokey_set);
}

static int hisi_pmic_gpio_key_open(struct input_dev *dev)
{
	return 0;
}

static void hisi_pmic_gpio_key_close(struct input_dev *dev)
{
	return;
}

static void hisi_pmic_gpio_keyup_work(struct work_struct *work)
{
	struct hisi_pmic_gpio_key *gpio_key = container_of(work,
		struct hisi_pmic_gpio_key, gpio_keyup_work.work);

	unsigned int keyup_value = 0;
	unsigned int report_action = GPIO_KEY_RELEASE;

	keyup_value = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYUP_DATA);
	/*judge key is pressed or released.*/
	if (keyup_value == GPIO_LOW_VOLTAGE) {
		report_action = GPIO_KEY_PRESS;
#if defined (CONFIG_HUAWEI_DSM)
		volume_up_press_count++;
#endif
	} else if (keyup_value == GPIO_HIGH_VOLTAGE) {
		report_action = GPIO_KEY_RELEASE;
	} else {
		printk(KERN_ERR "[gpiokey][%s]invalid gpio key_value.\n", __FUNCTION__);
		return;
	}

#ifdef CONFIG_VOLUME_KEY_MASK
    /* make sure report in pair */
	if((key_mask_ctl == VOLUME_KEY_DISABLE_MASK) && (report_action == GPIO_KEY_PRESS)) {
		key_mask_flag = VOLUME_KEY_DISABLE_MASK;
	}else if(key_mask_ctl == VOLUME_KEY_ENABLE_MASK) {
		key_mask_flag = VOLUME_KEY_ENABLE_MASK;
	}

	if(key_mask_flag == VOLUME_KEY_ENABLE_MASK) {
#endif
#ifdef CONFIG_HW_ZEROHUNG
		hung_wp_screen_vkeys_cb(WP_SCREEN_VUP_KEY, report_action);
#endif
		printk(KERN_INFO "[gpiokey] volumn key %u action %u\n", KEY_VOLUMEUP, report_action);
		input_report_key(gpio_key->input_dev, KEY_VOLUMEUP, report_action);
		input_sync(gpio_key->input_dev);
#ifdef CONFIG_VOLUME_KEY_MASK
	} else {
		printk(KERN_INFO "[gpiokey]volumn key is disabled %u\n", report_action);
	}
#endif

	if (keyup_value == GPIO_HIGH_VOLTAGE)
		wake_unlock(&volume_up_key_lock);/*lint !e455*/

	return;
}

static void hisi_pmic_gpio_keydown_work(struct work_struct *work)
{
	struct hisi_pmic_gpio_key *gpio_key = container_of(work,
		struct hisi_pmic_gpio_key, gpio_keydown_work.work);

	unsigned int keydown_value = 0;
	unsigned int report_action = GPIO_KEY_RELEASE;

	keydown_value = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYDOWN_DATA);
	/*judge key is pressed or released.*/
	if (keydown_value == GPIO_LOW_VOLTAGE) {
		report_action = GPIO_KEY_PRESS;
#if defined (CONFIG_HUAWEI_DSM)
		volume_down_press_count++;
#endif
	} else if (keydown_value == GPIO_HIGH_VOLTAGE) {
		report_action = GPIO_KEY_RELEASE;
	} else {
		printk(KERN_ERR "[gpiokey][%s]invalid gpio key_value.\n", __FUNCTION__);
		return;
	}

#ifdef CONFIG_VOLUME_KEY_MASK
    /* make sure report in pair */
	if((key_mask_ctl == VOLUME_KEY_DISABLE_MASK) && (report_action == GPIO_KEY_PRESS)) {
		key_mask_flag = VOLUME_KEY_DISABLE_MASK;
	}else if(key_mask_ctl == VOLUME_KEY_ENABLE_MASK) {
		key_mask_flag = VOLUME_KEY_ENABLE_MASK;
	}

	if(key_mask_flag == VOLUME_KEY_ENABLE_MASK) {
#endif
#ifdef CONFIG_HW_ZEROHUNG
		hung_wp_screen_vkeys_cb(WP_SCREEN_VDOWN_KEY, report_action);
#endif
		printk(KERN_INFO "[gpiokey]volumn key %u action %u\n", KEY_VOLUMEDOWN, report_action);
		input_report_key(gpio_key->input_dev, KEY_VOLUMEDOWN, report_action);
		input_sync(gpio_key->input_dev);
#ifdef CONFIG_VOLUME_KEY_MASK
	} else {
		printk(KERN_INFO "[gpiokey]volumn key is disabled %u\n", report_action);
	}
#endif

	if (keydown_value == GPIO_HIGH_VOLTAGE)
		wake_unlock(&volume_down_key_lock);/*lint !e455*/

	return;
}



static void hisi_pmic_gpio_keyup_timer(unsigned long data)
{
	unsigned int keyup_value;
	struct hisi_pmic_gpio_key *gpio_key = (struct hisi_pmic_gpio_key *)data;

	keyup_value = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYUP_DATA);
	/*judge key is pressed or released.*/
	if (keyup_value == GPIO_LOW_VOLTAGE){
		wake_lock(&volume_up_key_lock);
#if defined (CONFIG_HUAWEI_DSM)
		if ((jiffies - volume_up_last_press_time) < msecs_to_jiffies(PRESS_KEY_INTERVAL)) {
			if (!dsm_client_ocuppy(key_dclient)) {
				dsm_client_record(key_dclient, "volume up key trigger on the abnormal style.\n");
				dsm_client_notify(key_dclient, DSM_KEY_ERROR_NO);
			}
		}
		volume_up_last_press_time = jiffies;
#endif
	}

	schedule_delayed_work(&(gpio_key->gpio_keyup_work), 0);/*lint !e456*/

	return;/*lint !e456 !e454*/
}

static void hisi_pmic_gpio_keydown_timer(unsigned long data)
{
	int keydown_value;
	struct hisi_pmic_gpio_key *gpio_key = (struct hisi_pmic_gpio_key *)data;

	keydown_value = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYDOWN_DATA);
	/*judge key is pressed or released.*/
	if (keydown_value == GPIO_LOW_VOLTAGE){
		wake_lock(&volume_down_key_lock);

#if defined (CONFIG_HUAWEI_DSM)
	if ((jiffies - volume_down_last_press_time) < msecs_to_jiffies(PRESS_KEY_INTERVAL)) {
		if (!dsm_client_ocuppy(key_dclient)) {
			dsm_client_record(key_dclient, "volume down key trigger on the abnormal style.\n");
			dsm_client_notify(key_dclient, DSM_KEY_ERROR_NO);
		}
	}
	volume_down_last_press_time = jiffies;
 #endif
	}

	schedule_delayed_work(&(gpio_key->gpio_keydown_work), 0);/*lint !e456*/

	return;/*lint !e454*/
}

/*powerkey and keydown go to fastboot*/
static char s_vol_down_hold = 0;
static int vol_up_gpio = -1;
static int vol_up_active_low = -1;
static int vol_down_gpio = -1;
static int vol_down_active_low = -1;

static void gpio_key_vol_updown_press_set_bit(int bit_number)
{
	s_vol_down_hold |= (1 << bit_number);
}

void pmic_gpio_key_vol_updown_press_set_zero(void)
{
	s_vol_down_hold = 0;
}

int pmic_gpio_key_vol_updown_press_get(void)
{
	return s_vol_down_hold;
}

int is_pmic_gpio_key_vol_updown_pressed(void)
{
	int state1 = 0;
	int state2 = 0;

	if((vol_up_gpio < 0) || (vol_down_gpio < 0)
		|| (vol_up_active_low < 0) || (vol_down_active_low < 0)){
		printk(KERN_ERR "[%s]:vol_updown gpio or active_low is invalid!",__FUNCTION__);
		return 0;
	}

	mdelay(5);
	state1 = (hisi_pmic_gpio_key_read(PMIC_GPIO_KEYUP_DATA) ? 1 : 0) ^ vol_up_active_low;
	state2 = (hisi_pmic_gpio_key_read(PMIC_GPIO_KEYDOWN_DATA) ? 1 : 0) ^ vol_down_active_low;

	if(!!state1 && !!state2){
		return 1;
	}

	return 0;
}

static irqreturn_t hisi_pmic_gpio_key_irq_handler(int irq, void *dev_id)
{
	struct hisi_pmic_gpio_key *gpio_key = (struct hisi_pmic_gpio_key *)dev_id;
	int key_event = 0;

	/* handle gpio key volume up & gpio key volume down event at here */
	if (irq == gpio_key->gpiokey_irq[0]) {
		key_event = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYUP_DATA);
		if (0 == key_event) {
			gpio_key_vol_updown_press_set_bit(VOL_UP_BIT);
		} else {
			pmic_gpio_key_vol_updown_press_set_zero();
		}
		mod_timer(&(gpio_key->key_up_timer), jiffies + msecs_to_jiffies(TIMER_DEBOUNCE));
		wake_lock_timeout(&volume_up_key_lock, 50);
	} else if (irq == gpio_key->gpiokey_irq[1]) {
		key_event = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYDOWN_DATA);
		if (0 == key_event) {
			gpio_key_vol_updown_press_set_bit(VOL_DOWN_BIT);
		} else {
			pmic_gpio_key_vol_updown_press_set_zero();
		}
		mod_timer(&(gpio_key->key_down_timer), jiffies + msecs_to_jiffies(TIMER_DEBOUNCE));
		wake_lock_timeout(&volume_down_key_lock, 50);
	} else {
		printk(KERN_ERR "[gpiokey] [%s]invalid irq %d!\n", __FUNCTION__, irq);
	}
	return IRQ_HANDLED;
}

static int hisi_pmic_gpio_key_irq_init(struct platform_device* pdev, struct hisi_pmic_gpio_key *gpio_key)
{
    int i;
    int ret = 0;

	for (i = 0; i < HISI_PMIC_GPIO_KEY_IRQ_COUNTS; i++) {
		gpio_key->gpiokey_irq[i] =
				platform_get_irq_byname(pdev, hisi_pmic_gpio_key_irq_table[i]);
		if (gpio_key->gpiokey_irq[i] < 0) {
				dev_err(&pdev->dev, "Failed to get %s irq number!\n",
						hisi_pmic_gpio_key_irq_table[i]);
				return -EINVAL;
		}

		ret = devm_request_irq(&pdev->dev, gpio_key->gpiokey_irq[i],
					hisi_pmic_gpio_key_irq_handler, IRQF_NO_SUSPEND,
					hisi_pmic_gpio_key_irq_table[i], gpio_key);
		if (ret) {
			dev_err(&pdev->dev, "Failed to  request %s irq!\n",
					hisi_pmic_gpio_key_irq_table[i]);
			return -EINVAL;
		}
	}

	return 0;
}

static void hisi_pmic_gpio_key_reg_init(void)
{
	/*20ms filter*/
	hisi_pmic_gpio_key_write(PMIC_GPIO_KEY_REG_VALUE_ONE, PMIC_GPIO_KEYUP_DEBSEL);
	hisi_pmic_gpio_key_write(PMIC_GPIO_KEY_REG_VALUE_ONE, PMIC_GPIO_KEYDOWN_DEBSEL);
	/* not PD */
	hisi_pmic_gpio_key_write(PMIC_GPIO_KEY_REG_VALUE_ZERO, PMIC_GPIO_KEYUP_PDSEL);
	hisi_pmic_gpio_key_write(PMIC_GPIO_KEY_REG_VALUE_ZERO, PMIC_GPIO_KEYDOWN_PDSEL);
	/* PU */
	hisi_pmic_gpio_key_write(PMIC_GPIO_KEY_REG_VALUE_ONE, PMIC_GPIO_KEYUP_PUSEL);
	hisi_pmic_gpio_key_write(PMIC_GPIO_KEY_REG_VALUE_ONE, PMIC_GPIO_KEYDOWN_PUSEL);
	/* input */
	hisi_pmic_gpio_key_write(PMIC_GPIO_KEY_REG_VALUE_ZERO, PMIC_GPIO_KEYUP_DIR);
	hisi_pmic_gpio_key_write(PMIC_GPIO_KEY_REG_VALUE_ZERO, PMIC_GPIO_KEYDOWN_DIR);
	/* edge trigger */
	hisi_pmic_gpio_key_write(PMIC_GPIO_KEY_REG_VALUE_ZERO, PMIC_GPIO_KEYUP_IS);
	hisi_pmic_gpio_key_write(PMIC_GPIO_KEY_REG_VALUE_ZERO, PMIC_GPIO_KEYDOWN_IS);
	/* double edge trigger */
	hisi_pmic_gpio_key_write(PMIC_GPIO_KEY_REG_VALUE_ONE, PMIC_GPIO_KEYUP_IBE);
	hisi_pmic_gpio_key_write(PMIC_GPIO_KEY_REG_VALUE_ONE, PMIC_GPIO_KEYDOWN_IBE);

}

#ifdef CONFIG_VOLUME_KEY_MASK
static ssize_t hisi_pmic_key_mask_read(struct file *file, char __user *user_buf,
			size_t count, loff_t *ppos)
{
	char buf[8] = {0};

	snprintf_s(buf, sizeof(buf), sizeof(buf)-1, "%d", key_mask_flag);
	printk(KERN_INFO "[%s]key_mask read:%d\n", __FUNCTION__, key_mask_flag);

	return simple_read_from_buffer(user_buf, count, ppos, buf, strlen(buf));
}

static ssize_t hisi_pmic_key_mask_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[64] = {0};
	int size;
	int mask = 0;

	size = (count < 8) ? count : 8;

	if (NULL == user_buf) {
		printk(KERN_ERR "[gpiokey]user_buf is NULL!\n");
		return -EINVAL;
	}

	if (copy_from_user(buf, user_buf, size))
		return 0;

	if (sscanf(buf, "%d", &mask) != 1) /* [false alarm]:fortify */  /* unsafe_function_ignore: sscanf */
		return 0;

	if((mask == VOLUME_KEY_ENABLE_MASK) || (mask == VOLUME_KEY_DISABLE_MASK)) {
		key_mask_ctl = mask;
	}
	printk(KERN_INFO "[%s]key_mask write:%d\n", __FUNCTION__, mask);

	return count;
}

static const struct file_operations key_mask_fops = {
	.read = hisi_pmic_key_mask_read,
	.write = hisi_pmic_key_mask_write,
};
#endif

static int hisi_pmic_gpio_key_probe(struct platform_device* pdev)
{
	struct hisi_pmic_gpio_key *gpio_key = NULL;
	struct input_dev *input_dev = NULL;
	int err =0;

	if (NULL == pdev) {
		printk(KERN_ERR "[gpiokey]parameter error!\n");
		return -EINVAL;
	}

	dev_info(&pdev->dev, "hisi gpio key driver probes start!\n");

	gpio_key = devm_kzalloc(&pdev->dev, sizeof(struct hisi_pmic_gpio_key), GFP_KERNEL);
	if (!gpio_key) {
		dev_err(&pdev->dev, "Failed to allocate struct hisi_gpio_key!\n");
		return -ENOMEM;
	}

	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(&pdev->dev, "Failed to allocate struct input_dev!\n");
		return -ENOMEM;/*lint !e429*/
	}

	input_dev->name = pdev->name;
	input_dev->id.bustype = BUS_HOST;
	input_dev->dev.parent = &pdev->dev;
	input_set_drvdata(input_dev, gpio_key);
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_SYN, input_dev->evbit);
	set_bit(KEY_VOLUMEUP, input_dev->keybit);
	set_bit(KEY_VOLUMEDOWN, input_dev->keybit);

	input_dev->open = hisi_pmic_gpio_key_open;
	input_dev->close = hisi_pmic_gpio_key_close;

	gpio_key->input_dev = input_dev;

	/*initial work before we use it.*/
	INIT_DELAYED_WORK(&(gpio_key->gpio_keyup_work), hisi_pmic_gpio_keyup_work);
	INIT_DELAYED_WORK(&(gpio_key->gpio_keydown_work), hisi_pmic_gpio_keydown_work);
	wake_lock_init(&volume_down_key_lock, WAKE_LOCK_SUSPEND, "key_down_wake_lock");
	wake_lock_init(&volume_up_key_lock, WAKE_LOCK_SUSPEND, "key_up_wake_lock");

	vol_up_active_low = GPIO_KEY_PRESS;
	vol_down_active_low = GPIO_KEY_PRESS;

#if defined (CONFIG_HUAWEI_DSM)
	/* initialize the statistic variable */
	volume_up_press_count = 0;
	volume_down_press_count = 0;
	volume_up_last_press_time = 0;
	volume_down_last_press_time = 0;
#endif

	setup_timer(&(gpio_key->key_up_timer), hisi_pmic_gpio_keyup_timer, (unsigned long )gpio_key);
	setup_timer(&(gpio_key->key_down_timer), hisi_pmic_gpio_keydown_timer, (unsigned long )gpio_key);

#if defined (CONFIG_HUAWEI_DSM)
	setup_timer(&dsm_gpio_key_timer, dsm_gpio_key_timer_func, (uintptr_t)gpio_key);
#endif

	hisi_pmic_gpio_key_reg_init();

	err = input_register_device(gpio_key->input_dev);
	if (err) {
		dev_err(&pdev->dev, "Failed to register input device!\n");
		goto err_register_dev;
	}

	device_init_wakeup(&pdev->dev, TRUE);
	platform_set_drvdata(pdev, gpio_key);

#if defined (CONFIG_HUAWEI_DSM)
	if (!key_dclient) {
		key_dclient = dsm_register_client(&dsm_key);
	}

	mod_timer(&dsm_gpio_key_timer, jiffies + STATISTIC_INTERVAL * HZ);
#endif

#ifdef CONFIG_VOLUME_KEY_MASK
	if (!proc_create("key_mask", 0660, NULL, &key_mask_fops)) {
		printk(KERN_ERR "[%s]:failed to create key_mask proc\n",__FUNCTION__);
	}
#endif

	err = hisi_pmic_gpio_key_irq_init(pdev, gpio_key);
	if(err) {
		dev_err(&pdev->dev, "Failed to request irq!\n");
		goto err_request_irq;
	}
	dev_info(&pdev->dev, "hisi gpio key driver probes successfully!\n");
	return 0;

err_request_irq:
err_register_dev:
	wake_lock_destroy(&volume_down_key_lock);
	wake_lock_destroy(&volume_up_key_lock);

	pr_info(KERN_ERR "[gpiokey]K3v3 gpio key probe failed! ret = %d.\n", err);
	return err;/*lint !e593*/
}

static int hisi_pmic_gpio_key_remove(struct platform_device* pdev)
{
	struct hisi_pmic_gpio_key *gpio_key = platform_get_drvdata(pdev);

	if (gpio_key == NULL) {
		printk(KERN_ERR "get invalid gpio_key pointer\n");
		return -EINVAL;
	}

	cancel_delayed_work(&(gpio_key->gpio_keyup_work));
	cancel_delayed_work(&(gpio_key->gpio_keydown_work));
	wake_lock_destroy(&volume_down_key_lock);
	wake_lock_destroy(&volume_up_key_lock);

	input_unregister_device(gpio_key->input_dev);
	platform_set_drvdata(pdev, NULL);
#ifdef CONFIG_VOLUME_KEY_MASK
	remove_proc_entry("key_mask", 0);
#endif

	return 0;
}

#ifdef CONFIG_PM
static int hisi_pmic_gpio_key_suspend(struct platform_device *pdev, pm_message_t state)
{

	dev_info(&pdev->dev, "%s: suspend +\n", __func__);

	dev_info(&pdev->dev, "%s: suspend -\n", __func__);
	return 0;
}

static int hisi_pmic_gpio_key_resume(struct platform_device *pdev)
{

	dev_info(&pdev->dev, "%s: resume +\n", __func__);

	dev_info(&pdev->dev, "%s: resume -\n", __func__);
	return 0;
}
#endif

static const struct of_device_id hisi_pmic_gpio_key_match[] = {
	{ .compatible = "hisilicon,pmic-gpio-key",},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_pmic_gpio_key_match);

struct platform_driver hisi_pmic_gpio_key_driver = {
	.probe = hisi_pmic_gpio_key_probe,
	.remove = hisi_pmic_gpio_key_remove,
	.driver = {
		.name = "hisi_pmic_gpio_key",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_pmic_gpio_key_match),
	},
#ifdef CONFIG_PM
	.suspend = hisi_pmic_gpio_key_suspend,
	.resume = hisi_pmic_gpio_key_resume,
#endif
};

module_platform_driver(hisi_pmic_gpio_key_driver);

MODULE_AUTHOR("Hisilicon K3 Driver Group");
MODULE_DESCRIPTION("Hisilicon keypad platform driver");
MODULE_LICENSE("GPL");
