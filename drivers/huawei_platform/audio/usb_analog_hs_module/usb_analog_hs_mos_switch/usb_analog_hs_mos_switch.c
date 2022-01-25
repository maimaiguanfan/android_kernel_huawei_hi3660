/*
 * usb_analog_hs_mos_switch.c
 *
 * usb analog headset mos switch driver
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
 */

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/of_gpio.h>
#include <linux/hisi/hi64xx/hi64xx_mbhc.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/sensor/hw_comm_pmic.h>
#include "huawei_platform/audio/usb_analog_hs_mos_switch.h"

#define SD_GPIO_SCTRL_REG    0xfff0a314
#define HWLOG_TAG usb_analog_hs_mos_switch
HWLOG_REGIST();

#define IN_FUNCTION   hwlog_info("%s function comein\n", __func__)
#define OUT_FUNCTION  hwlog_info("%s function comeout\n", __func__)
#define MAX_BUF_SIZE                    32
#define DEFLAUT_SLEEP_TIME_20MS         20
#define DEFAULT_MIC_SWITCH_DELAY_TIME   0
#define HIGH                            1
#define LOW                             0
#define PLUG_IN_DELAY_800MS             800
#define WAKE_LOCK_TIMEOUT               1000
#define LR_CHANNEL_VOLTAGE              2950000
#define USB_PMIC_VOLTAGE                5000000

struct usb_ana_hs_mos_data {
	int gpio_type;
	int gpio_audio_auxpand;   /* H:sub1->aux padn, sub2->aux padp */
	int gpio_mic_gnd;         /* H:audio sbu1->mic, audio sbu2->gnd */
	int gpio_auxpand_switch;  /* H:sub1->aux padp, sub2->aux padn */
	int gpio_fb;              /* H:connect to fb */
	int gpio_mos_switch;      /* H:mos to hs, L:mos to usb */
	int mic_switch_delay_time;
	int registed;             /* usb analog headset dev register flag */
	struct wake_lock wake_lock;
	struct mutex mutex;
	struct workqueue_struct *analog_hs_plugin_delay_wq;
	struct delayed_work analog_hs_plugin_delay_work;
	struct workqueue_struct *analog_hs_plugout_delay_wq;
	struct delayed_work analog_hs_plugout_delay_work;
	struct usb_analog_hs_dev *codec_ops_dev;
	void *private_data;      /* store codec description data */
	int usb_analog_hs_in;
	bool connect_linein_r;
	struct platform_device *pdev;
	struct device *dev;
	struct regulator *mos_switch_phy_ldo;
};

static struct usb_ana_hs_mos_data *g_pdata_mos_switch;

static int usb_analog_hs_gpio_get_value(int gpio)
{
	if (gpio <= 0)
		return -ENODEV;

	if (g_pdata_mos_switch->gpio_type == USB_ANALOG_HS_GPIO_CODEC)
		return gpio_get_value_cansleep(gpio);

	return gpio_get_value(gpio);
}

static void usb_analog_hs_gpio_set_value(int gpio, int value)
{
	if (gpio <= 0)
		return;

	if (g_pdata_mos_switch->gpio_type == USB_ANALOG_HS_GPIO_CODEC)
		gpio_set_value_cansleep(gpio, value);
	else
		gpio_set_value(gpio, value);
}

static void usb_analog_hs_mos_set_gpio_state(int audio_auxpand, int mic_gnd,
	int auxpand_switch, int fb)
{
	usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_audio_auxpand,
		audio_auxpand);
	usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_mic_gnd, mic_gnd);
	usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_auxpand_switch,
		auxpand_switch);
	usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_fb, fb);
}

static void usb_analog_hs_plugin_work(struct work_struct *work)
{
	struct hw_comm_pmic_cfg_t audio_pmic_ldo_set;

	IN_FUNCTION;
	wake_lock(&g_pdata_mos_switch->wake_lock);

	/*
	 * change codec hs resistence from 70ohm to 3Kohm,
	 * to reduce the pop sound in hs when usb analog hs plug in.
	 */
	g_pdata_mos_switch->codec_ops_dev->ops.hs_high_resistence_enable(
		g_pdata_mos_switch->private_data, true);

	if (g_pdata_mos_switch->gpio_mos_switch > 0) {
		usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_mos_switch, HIGH);
	} else {
		audio_pmic_ldo_set.pmic_num = 1;
		audio_pmic_ldo_set.pmic_power_type = VOUT_BOOST_EN;
		audio_pmic_ldo_set.pmic_power_voltage = USB_PMIC_VOLTAGE;
		audio_pmic_ldo_set.pmic_power_state = LOW;
		hw_pmic_power_cfg(AUDIO_PMIC_REQ, &audio_pmic_ldo_set);
		if (!g_pdata_mos_switch->mos_switch_phy_ldo) {
			hwlog_err("g_pdata_mos_switch is null or ldo is null\n");
			audio_pmic_ldo_set.pmic_power_state = HIGH;
			hw_pmic_power_cfg(AUDIO_PMIC_REQ, &audio_pmic_ldo_set);
			return;
		}

		if (regulator_enable(g_pdata_mos_switch->mos_switch_phy_ldo)) {
			hwlog_err("error: regulator enable failed\n");
			audio_pmic_ldo_set.pmic_power_state = HIGH;
			hw_pmic_power_cfg(AUDIO_PMIC_REQ, &audio_pmic_ldo_set);
			return;
		}
	}

	usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_audio_auxpand, 0);
	usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_fb, 1);
	usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_auxpand_switch, 0);

	msleep(DEFLAUT_SLEEP_TIME_20MS);
	g_pdata_mos_switch->codec_ops_dev->ops.plug_in_detect(
		g_pdata_mos_switch->private_data);
	mutex_lock(&g_pdata_mos_switch->mutex);
	g_pdata_mos_switch->usb_analog_hs_in = USB_ANA_HS_PLUG_IN;
	mutex_unlock(&g_pdata_mos_switch->mutex);
	/* recovery codec hs resistence to 70ohm. */
	g_pdata_mos_switch->codec_ops_dev->ops.hs_high_resistence_enable(
		g_pdata_mos_switch->private_data, false);
	wake_unlock(&g_pdata_mos_switch->wake_lock);

	OUT_FUNCTION;
}

static void usb_analog_hs_plugout_work(struct work_struct *work)
{
	struct hw_comm_pmic_cfg_t audio_pmic_ldo_set;

	IN_FUNCTION;

	wake_lock(&g_pdata_mos_switch->wake_lock);
	if (g_pdata_mos_switch->analog_hs_plugin_delay_wq) {
		cancel_delayed_work(
			&g_pdata_mos_switch->analog_hs_plugin_delay_work);
		flush_workqueue(g_pdata_mos_switch->analog_hs_plugin_delay_wq);
	}
	if (g_pdata_mos_switch->usb_analog_hs_in == USB_ANA_HS_PLUG_IN) {
		hwlog_info("%s usb analog hs plug out act\n", __func__);
		g_pdata_mos_switch->codec_ops_dev->ops.plug_out_detect(
			g_pdata_mos_switch->private_data);
		mutex_lock(&g_pdata_mos_switch->mutex);
		g_pdata_mos_switch->usb_analog_hs_in = USB_ANA_HS_PLUG_OUT;
		mutex_unlock(&g_pdata_mos_switch->mutex);

		if (g_pdata_mos_switch->gpio_mos_switch > 0) {
			usb_analog_hs_gpio_set_value(
				g_pdata_mos_switch->gpio_mos_switch, LOW);
		} else {
			if (!g_pdata_mos_switch->mos_switch_phy_ldo) {
				hwlog_err("%s g_pdata_mos is null or ldo is null\n",
					__func__);
				return;
			}

			if (regulator_disable(g_pdata_mos_switch->mos_switch_phy_ldo)) {
				hwlog_err("error: regulator enable failed\n");
				return;
			}
			audio_pmic_ldo_set.pmic_num = 1;
			audio_pmic_ldo_set.pmic_power_type = VOUT_BOOST_EN;
			audio_pmic_ldo_set.pmic_power_voltage = USB_PMIC_VOLTAGE;
			audio_pmic_ldo_set.pmic_power_state = HIGH;
			hw_pmic_power_cfg(AUDIO_PMIC_REQ, &audio_pmic_ldo_set);
		}

		usb_analog_hs_gpio_set_value(
			g_pdata_mos_switch->gpio_audio_auxpand, 1);
		usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_fb, 0);
		usb_analog_hs_gpio_set_value(
			g_pdata_mos_switch->gpio_auxpand_switch, 0);
		if (usb_analog_hs_gpio_get_value(g_pdata_mos_switch->gpio_mic_gnd) == HIGH)
			usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_mic_gnd, LOW);
	}
	wake_unlock(&g_pdata_mos_switch->wake_lock);

	OUT_FUNCTION;
}

int usb_ana_hs_mos_dev_register(struct usb_analog_hs_dev *dev, void *codec_data)
{
	/* usb analog headset driver not be probed, just return */
	if (!g_pdata_mos_switch) {
		hwlog_err("%s pdata is NULL\n", __func__);
		return -ENODEV;
	}

	/* only support one codec to be registed */
	if (g_pdata_mos_switch->registed == USB_ANALOG_HS_ALREADY_REGISTER) {
		hwlog_err("%s one codec has registed, no more permit\n",
			__func__);
		return -EEXIST;
	}

	if (!dev->ops.plug_in_detect || !dev->ops.plug_out_detect) {
		hwlog_err("%s codec ops function must be all registed\n",
			__func__);
		return -EINVAL;
	}

	mutex_lock(&g_pdata_mos_switch->mutex);
	g_pdata_mos_switch->codec_ops_dev = dev;
	g_pdata_mos_switch->private_data = codec_data;
	g_pdata_mos_switch->registed = USB_ANALOG_HS_ALREADY_REGISTER;
	mutex_unlock(&g_pdata_mos_switch->mutex);

	hwlog_info("%s usb analog hs has been register sucessful\n", __func__);

	return 0;
}

int usb_ana_hs_mos_check_hs_pluged_in(void)
{
	int analog_hs_state = pd_dpm_get_analog_hs_state();

	hwlog_info("%s analog_hs_state =%d\n", __func__, analog_hs_state);

	if (analog_hs_state)
		return USB_ANA_HS_PLUG_IN;

	return USB_ANA_HS_PLUG_OUT;
}

void usb_ana_hs_mos_mic_switch_change_state(void)
{
	int gpio_mic_gnd;

	if (!g_pdata_mos_switch) {
		hwlog_err("%s pdata is NULL\n", __func__);
		return;
	}

	if (g_pdata_mos_switch->registed == USB_ANALOG_HS_NOT_REGISTER) {
		hwlog_err("%s codec_ops_dev is not registed\n", __func__);
		return;
	}

	IN_FUNCTION;

	gpio_mic_gnd = usb_analog_hs_gpio_get_value(
		g_pdata_mos_switch->gpio_mic_gnd);
	gpio_mic_gnd = (gpio_mic_gnd == HIGH) ? LOW : HIGH;
	usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_mic_gnd,
		gpio_mic_gnd);
	if (g_pdata_mos_switch->mic_switch_delay_time > 0)
		msleep(g_pdata_mos_switch->mic_switch_delay_time);
	gpio_mic_gnd = usb_analog_hs_gpio_get_value(
		g_pdata_mos_switch->gpio_mic_gnd);
	hwlog_info("%s gpio mic sel change to %d\n", __func__, gpio_mic_gnd);

	OUT_FUNCTION;
}

void usb_ana_hs_mos_plug_in_out_handle(int hs_state)
{
	if (!g_pdata_mos_switch) {
		hwlog_err("%s pdata is NULL\n", __func__);
		return;
	}

	if (g_pdata_mos_switch->registed == USB_ANALOG_HS_NOT_REGISTER) {
		hwlog_err("%s codec_ops_dev is not registed\n", __func__);
		return;
	}

	IN_FUNCTION;

	wake_lock_timeout(&g_pdata_mos_switch->wake_lock,
		msecs_to_jiffies(WAKE_LOCK_TIMEOUT));

	switch (hs_state) {
	case USB_ANA_HS_PLUG_IN:
		queue_delayed_work(g_pdata_mos_switch->analog_hs_plugin_delay_wq,
			&g_pdata_mos_switch->analog_hs_plugin_delay_work,
			msecs_to_jiffies(PLUG_IN_DELAY_800MS));
		break;
	case USB_ANA_HS_PLUG_OUT:
		queue_delayed_work(g_pdata_mos_switch->analog_hs_plugout_delay_wq,
			&g_pdata_mos_switch->analog_hs_plugout_delay_work, 0);
		break;
	case DP_PLUG_IN:
		usb_analog_hs_mos_set_gpio_state(1, 0, 1, 0);
		break;
	case DP_PLUG_IN_CROSS:
		usb_analog_hs_mos_set_gpio_state(1, 0, 0, 0);
		break;
	case DP_PLUG_OUT:
		usb_analog_hs_mos_set_gpio_state(1, 0, 1, 0);
		break;
	default:
		break;
	}
	hwlog_info("%s hs_state is %d\n", __func__, hs_state);

	OUT_FUNCTION;
}

bool check_usb_analog_hs_mos_support(void)
{
	if (!g_pdata_mos_switch ||
		g_pdata_mos_switch->registed == USB_ANALOG_HS_NOT_REGISTER)
		return false;

	return true;
}

static const struct of_device_id usb_ana_hs_mos_of_match[] = {
	{ .compatible = "huawei,usb_ana_hs_mos_switch", },
	{},
};
MODULE_DEVICE_TABLE(of, usb_ana_hs_mos_of_match);

/* load dts config for board difference */
static void load_gpio_type_config(struct device_node *node)
{
	unsigned int temp = USB_ANALOG_HS_GPIO_SOC;

	if (!of_property_read_u32(node, "gpio_type", &temp))
		g_pdata_mos_switch->gpio_type = temp;
	else
		g_pdata_mos_switch->gpio_type = USB_ANALOG_HS_GPIO_SOC;
}

static int hs_load_gpio(struct device *dev, int *gpio_index,
	int out_value, const char *gpio_name)
{
	int gpio;
	int ret;

	gpio = of_get_named_gpio(dev->of_node, gpio_name, 0);
	*gpio_index = gpio;
	if (gpio < 0) {
		hwlog_err("%s:Looking up %s property in node %s failed %d\n",
			__func__, dev->of_node->full_name, gpio_name, gpio);
		return 0; // maybe no need gpio, do not return error
	}
	if (!gpio_is_valid(gpio)) {
		hwlog_err("%s gpio %s is unvalid\n", __func__, gpio_name);
		ret = -ENOENT;
	} else {
		ret = gpio_request(gpio, gpio_name);
		if (ret < 0) {
			hwlog_err("%s request GPIO for %s fail %d\n",
				__func__, gpio_name, ret);
		} else {
			ret = gpio_direction_output(gpio, out_value);
			if (ret < 0) {
				hwlog_err("%s set output for %s fail %d\n",
					__func__, gpio_name, ret);
			}
		}
	}
	return ret;
}

static int usb_ana_hs_mos_load_gpio_pdata(struct device *dev,
	struct usb_ana_hs_mos_data *data)
{
	if (hs_load_gpio(dev, &(data->gpio_audio_auxpand), LOW,
		"switch_audio_auxpand") < 0)
		goto mos_switch_get_gpio_audio_auxpand_err;

	if (hs_load_gpio(dev, &(data->gpio_mic_gnd), LOW,
		"switch_mic_gnd") < 0)
		goto mos_switch_get_gpio_mic_gnd_err;

	if (hs_load_gpio(dev, &(data->gpio_auxpand_switch), LOW,
		"switch_auxpand") < 0)
		goto mos_switch_get_gpio_auxpand_switch_err;

	if (hs_load_gpio(dev, &(data->gpio_fb), LOW, "switch_fb") < 0)
		goto mos_switch_get_gpio_fb_err;

	if (hs_load_gpio(dev, &(data->gpio_mos_switch), LOW,
		"switch_mos_hs") < 0)
		goto mos_switch_get_gpio_mos_err;
	return 0;

mos_switch_get_gpio_mos_err:
	if (data->gpio_fb > 0)
		gpio_free(data->gpio_fb);
mos_switch_get_gpio_fb_err:
	if (data->gpio_auxpand_switch > 0)
		gpio_free(data->gpio_auxpand_switch);
mos_switch_get_gpio_auxpand_switch_err:
	if (data->gpio_mic_gnd > 0)
		gpio_free(data->gpio_mic_gnd);
mos_switch_get_gpio_mic_gnd_err:
	if (data->gpio_audio_auxpand > 0)
		gpio_free(data->gpio_audio_auxpand);
mos_switch_get_gpio_audio_auxpand_err:
	return -ENOENT;
}

static void usb_analog_hs_free_gpio(struct usb_ana_hs_mos_data *data)
{
	IN_FUNCTION;

	if (data->gpio_audio_auxpand > 0)
		gpio_free(data->gpio_audio_auxpand);

	if (data->gpio_mic_gnd > 0)
		gpio_free(data->gpio_mic_gnd);

	if (data->gpio_auxpand_switch > 0)
		gpio_free(data->gpio_auxpand_switch);

	if (data->gpio_fb > 0)
		gpio_free(data->gpio_fb);

	if (data->gpio_mos_switch > 0)
		gpio_free(data->gpio_mos_switch);
}

#ifdef USB_ANALOG_HEADSET_DEBUG
static ssize_t usb_ana_hs_mos_mic_switch_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int value;

	if (!g_pdata_mos_switch) {
		hwlog_err("%s pdata is NULL\n", __func__);
		return 0;
	}

	if (g_pdata_mos_switch->registed == USB_ANALOG_HS_NOT_REGISTER) {
		hwlog_err("%s pdata is not registed\n", __func__);
		return 0;
	}

	value = usb_analog_hs_gpio_get_value(g_pdata_mos_switch->gpio_mic_gnd);

	return scnprintf(buf, MAX_BUF_SIZE, "%d\n", value);
}

static ssize_t usb_ana_hs_mos_mic_switch_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	long val;

	if (!g_pdata_mos_switch) {
		hwlog_err("%s:pdata is NULL\n", __func__);
		return count;
	}

	if (g_pdata_mos_switch->registed == USB_ANALOG_HS_NOT_REGISTER) {
		hwlog_err("%s pdata is not registed\n", __func__);
		return count;
	}

	ret = kstrtol(buf, 10, &val);
	if (ret < 0) {
		hwlog_err("%s:input error\n", __func__);
		return count;
	}

	if (val)
		usb_analog_hs_gpio_set_value(
			g_pdata_mos_switch->gpio_mic_gnd, 1);
	else
		usb_analog_hs_gpio_set_value(
			g_pdata_mos_switch->gpio_mic_gnd, 0);

	return count;
}

static DEVICE_ATTR(mic_switch, 0660, usb_ana_hs_mos_mic_switch_show,
	usb_ana_hs_mos_mic_switch_store);

static struct attribute *usb_ana_hs_mos_attributes[] = {
	&dev_attr_mic_switch.attr,
	NULL
};

static const struct attribute_group usb_ana_hs_mos_attr_group = {
	.attrs = usb_ana_hs_mos_attributes,
};
#endif

static long usb_ana_hs_mos_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	int ret = 0;
	int gpio_mic_sel_val = 0;
	unsigned int __user *p_user = (unsigned int __user *) arg;

	if (!g_pdata_mos_switch)
		return -EBUSY;

	if (g_pdata_mos_switch->registed == USB_ANALOG_HS_NOT_REGISTER)
		return -EBUSY;

	switch (cmd) {
	case IOCTL_USB_ANA_HS_GET_MIC_SWITCH_STATE:
		gpio_mic_sel_val = usb_analog_hs_gpio_get_value(
			g_pdata_mos_switch->gpio_mic_gnd);
		hwlog_info("%s gpio_mic_sel_val = %d\n", __func__,
			gpio_mic_sel_val);
		ret = put_user((__u32)(gpio_mic_sel_val), p_user);
		break;
	case IOCTL_USB_ANA_HS_GET_CONNECT_LINEIN_R_STATE:
		hwlog_info("%s connect_linein_r = %d\n", __func__,
			g_pdata_mos_switch->connect_linein_r);
		ret = put_user((__u32)(g_pdata_mos_switch->connect_linein_r),
			p_user);
		break;
	case IOCTL_USB_ANA_HS_GND_FB_CONNECT:
		hwlog_info("usb analog hs mos switch ioctl gnd fb connect\n");
		usb_analog_hs_gpio_set_value(
			g_pdata_mos_switch->gpio_audio_auxpand, 0);
		usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_fb, 1);
		break;
	case IOCTL_USB_ANA_HS_GND_FB_DISCONNECT:
		hwlog_info("usb analog hs mos switch ioctl fb disconnect\n");
		usb_analog_hs_gpio_set_value(
			g_pdata_mos_switch->gpio_audio_auxpand, 1);
		usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_fb, 0);
		break;
	default:
		hwlog_err("%s unsupport cmd\n", __func__);
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static const struct file_operations usb_ana_hs_mos_fops = {
	.owner               = THIS_MODULE,
	.open                = simple_open,
	.unlocked_ioctl      = usb_ana_hs_mos_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl        = usb_ana_hs_mos_ioctl,
#endif
};

static struct miscdevice usb_ana_hs_mos_device = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = "usb_analog_hs",
	.fops   = &usb_ana_hs_mos_fops,
};

struct usb_analog_hs_ops usb_ana_hs_mos_ops = {
	.usb_ana_hs_check_headset_pluged_in = usb_ana_hs_mos_check_hs_pluged_in,
	.usb_ana_hs_dev_register = usb_ana_hs_mos_dev_register,
	.check_usb_ana_hs_support = check_usb_analog_hs_mos_support,
	.usb_ana_hs_plug_in_out_handle = usb_ana_hs_mos_plug_in_out_handle,
	.usb_ana_hs_mic_swtich_change_state =
		usb_ana_hs_mos_mic_switch_change_state,
};

static int hw_mos_switch_regulator_get(
	struct usb_ana_hs_mos_data *g_pdata_mos_switch)
{
	int ret;

	if (!g_pdata_mos_switch) {
		hwlog_err("%s error: g_pdata_mos_switch is null\n", __func__);
		return -1;
	}
	if (g_pdata_mos_switch->gpio_mos_switch > 0) {
		hwlog_info("%s use gpio to switch mos\n", __func__);
		usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_mos_switch, LOW);
		return 0;
	}

	g_pdata_mos_switch->mos_switch_phy_ldo =
		devm_regulator_get(g_pdata_mos_switch->dev,
			"switch_lr_channel");
	if (IS_ERR(g_pdata_mos_switch->mos_switch_phy_ldo)) {
		hwlog_err("mos_switch_phy_ldo regulator dts read failed\n");
		return -1;
	}

	ret = regulator_set_voltage(g_pdata_mos_switch->mos_switch_phy_ldo,
		LR_CHANNEL_VOLTAGE, LR_CHANNEL_VOLTAGE);
	if(ret != 0) {
		hwlog_err("mos_switch_phy_ldo regulator set voltage failed\n");
		return -1;
	}

	ret = regulator_get_voltage(g_pdata_mos_switch->mos_switch_phy_ldo);
	hwlog_info("%s mos_switch_phy_ldo_295v=%d\n", __func__, ret);

	return 0;
}

static int read_property_value(struct device *dev,
	const char *property_name, int default_value)
{
	int ret;
	int value;

	ret = of_property_read_u32(dev->of_node, property_name, &value);
	if (ret)
		value = default_value;
	hwlog_info("%s %s =%d\n", __func__, property_name, value);
	return value;
}

static bool read_property_setting(struct device *dev,
	const char *property_name, bool default_setting)
{
	bool setting = false;
	int val;

	if (!of_property_read_u32(dev->of_node, property_name, &val)) {
		if (val == 0)
			setting = false;
		else
			setting = true;
	} else {
		setting = default_setting;
		hwlog_info("%s: missing %s, set default value %s\n", __func__,
			property_name, default_setting ? "true" : "false");
	}
	return setting;
}

static int usb_ana_hs_create_plug_workqueue(void)
{
	int ret = 0;

	g_pdata_mos_switch->analog_hs_plugin_delay_wq =
		create_singlethread_workqueue("usb_analog_hs_plugin_delay_wq");
	if (!(g_pdata_mos_switch->analog_hs_plugin_delay_wq)) {
		hwlog_err("plugin wq create failed\n");
		ret = -ENOMEM;
		return ret;
	}
	INIT_DELAYED_WORK(&g_pdata_mos_switch->analog_hs_plugin_delay_work,
		usb_analog_hs_plugin_work);

	g_pdata_mos_switch->analog_hs_plugout_delay_wq =
		create_singlethread_workqueue("usb_analog_hs_plugout_delay_wq");
	if (!(g_pdata_mos_switch->analog_hs_plugout_delay_wq)) {
		hwlog_err("plugout wq create failed\n");
		ret = ENOMEM;
		return ret;
	}
	INIT_DELAYED_WORK(&g_pdata_mos_switch->analog_hs_plugout_delay_work,
		usb_analog_hs_plugout_work);

	return ret;

}

static int usb_ana_hs_mos_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	int ret;

	IN_FUNCTION;
	g_pdata_mos_switch = kzalloc(sizeof(*g_pdata_mos_switch), GFP_KERNEL);
	if (!g_pdata_mos_switch)
		return -ENOMEM;
	ret = usb_ana_hs_mos_load_gpio_pdata(dev, g_pdata_mos_switch);
	if (ret < 0) {
		hwlog_err("get gpios failed, ret =%d\n", ret);
		goto mos_switch_err_out;
	}
	g_pdata_mos_switch->mic_switch_delay_time = read_property_value(dev,
		"mic_switch_delay", DEFAULT_MIC_SWITCH_DELAY_TIME);
	g_pdata_mos_switch->connect_linein_r = read_property_setting(dev,
		"connect_linein_r", true);
	wake_lock_init(&g_pdata_mos_switch->wake_lock, WAKE_LOCK_SUSPEND,
		"usb_analog_hs");
	mutex_init(&g_pdata_mos_switch->mutex);
	/* load dts config for board difference */
	load_gpio_type_config(node);
	g_pdata_mos_switch->registed = USB_ANALOG_HS_NOT_REGISTER;
	g_pdata_mos_switch->usb_analog_hs_in = USB_ANA_HS_PLUG_OUT;
	usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_audio_auxpand, 1);
	usb_analog_hs_gpio_set_value(g_pdata_mos_switch->gpio_fb, 0);
	g_pdata_mos_switch->pdev = pdev;
	g_pdata_mos_switch->dev = &pdev->dev;
	if (!g_pdata_mos_switch->pdev || !g_pdata_mos_switch->dev ||
		!g_pdata_mos_switch->dev->of_node) {
		hwlog_err("error: device_node is null\n");
		goto mos_switch_err_regulator_get;
	}
	if (hw_mos_switch_regulator_get(g_pdata_mos_switch)) {
		hwlog_err("error: parse dts failed\n");
		goto mos_switch_err_regulator_get;
	}
	ret = usb_ana_hs_create_plug_workqueue();
	if (ret < 0)
		goto mos_switch_err_regulator_get;
	else if (ret > 0)
		goto mos_switch_err_plugin_delay_wq;
	ret = usb_analog_hs_ops_register(&usb_ana_hs_mos_ops);
	if (ret) {
		hwlog_err("register usb_ana_hs_mos_ops ops failed\n");
		goto mos_switch_err_misc_register;
	}
	/* register misc device for userspace */
	ret = misc_register(&usb_ana_hs_mos_device);
	if (ret) {
		hwlog_err("usb_analog_hs_device misc device register failed\n");
		goto mos_switch_err_misc_register;
	}
#ifdef USB_ANALOG_HEADSET_DEBUG
	ret = sysfs_create_group(&dev->kobj, &usb_ana_hs_mos_attr_group);
	if (ret < 0)
		hwlog_err("failed to register sysfs\n");
#endif
	hwlog_err("usb_analog_hs probe success\n");
	return 0;
mos_switch_err_misc_register:
	if (g_pdata_mos_switch->analog_hs_plugout_delay_wq) {
		cancel_delayed_work(
			&g_pdata_mos_switch->analog_hs_plugout_delay_work);
		flush_workqueue(g_pdata_mos_switch->analog_hs_plugout_delay_wq);
		destroy_workqueue(
			g_pdata_mos_switch->analog_hs_plugout_delay_wq);
	}
mos_switch_err_plugin_delay_wq:
	if (g_pdata_mos_switch->analog_hs_plugin_delay_wq) {
		cancel_delayed_work(
			&g_pdata_mos_switch->analog_hs_plugin_delay_work);
		flush_workqueue(g_pdata_mos_switch->analog_hs_plugin_delay_wq);
		destroy_workqueue(
			g_pdata_mos_switch->analog_hs_plugin_delay_wq);
	}
mos_switch_err_regulator_get:
	wake_lock_destroy(&g_pdata_mos_switch->wake_lock);
	usb_analog_hs_free_gpio(g_pdata_mos_switch);
mos_switch_err_out:
	kfree(g_pdata_mos_switch);
	g_pdata_mos_switch = NULL;
	return ret;
}

static int usb_ana_hs_mos_remove(struct platform_device *pdev)
{
	if (!g_pdata_mos_switch)
		return 0;

	if (g_pdata_mos_switch->analog_hs_plugin_delay_wq) {
		cancel_delayed_work(
			&g_pdata_mos_switch->analog_hs_plugin_delay_work);
		flush_workqueue(g_pdata_mos_switch->analog_hs_plugin_delay_wq);
		destroy_workqueue(
			g_pdata_mos_switch->analog_hs_plugin_delay_wq);
	}

	if (g_pdata_mos_switch->analog_hs_plugout_delay_wq) {
		cancel_delayed_work(
			&g_pdata_mos_switch->analog_hs_plugout_delay_work);
		flush_workqueue(g_pdata_mos_switch->analog_hs_plugout_delay_wq);
		destroy_workqueue(
			g_pdata_mos_switch->analog_hs_plugout_delay_wq);
	}

	usb_analog_hs_free_gpio(g_pdata_mos_switch);
	misc_deregister(&usb_ana_hs_mos_device);

#ifdef USB_ANALOG_HEADSET_DEBUG
	sysfs_remove_group(&pdev->dev.kobj, &usb_ana_hs_mos_attr_group);
#endif

	kfree(g_pdata_mos_switch);
	g_pdata_mos_switch = NULL;

	return 0;
}

static struct platform_driver usb_ana_hs_mos_driver = {
	.driver = {
		.name   = "usb_ana_hs_mos",
		.owner  = THIS_MODULE,
		.of_match_table = usb_ana_hs_mos_of_match,
	},
	.probe  = usb_ana_hs_mos_probe,
	.remove = usb_ana_hs_mos_remove,
};

static int __init usb_ana_hs_mos_init(void)
{
	return platform_driver_register(&usb_ana_hs_mos_driver);
}

static void __exit usb_ana_hs_mos_exit(void)
{
	platform_driver_unregister(&usb_ana_hs_mos_driver);
}

fs_initcall(usb_ana_hs_mos_init);
module_exit(usb_ana_hs_mos_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("usb analog headset mos switch driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
