/*
 * hw_ycable.h
 *
 * huawei ycable driver
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

#ifndef _HUAWEI_YCABLE_H_
#define _HUAWEI_YCABLE_H_

#include <linux/notifier.h>

#define YCABLE_WORK_TIMEOUT                 (50)
#define YCABLE_OTG_ENABLE_WORK_TIMEOUT      (500)
#define YCABLE_DETECT_TIMEOUT               (30000)
#define YCABLE_WAIT_COMPLETE_TIMEOUT        (1000)

#define YCABLE_OTG_THRESHOLD_VOLTAGE_MIN    (0) /* mv */
#define YCABLE_OTG_THRESHOLD_VOLTAGE_MAX    (150) /* mv */
#define YCABLE_CHG_THRESHOLD_VOLTAGE_MIN    (270) /* mv */
#define YCABLE_CHG_THRESHOLD_VOLTAGE_MAX    (450) /* mv */
#define YCABLE_INVALID_THRESHOLD_VOLTAGE    (-1)

#define YCABLE_OTG_BOOSTV                   (5000)
#define YCABLE_CURR_DEFAULT                 (1000)
#define YCABLE_CURR_0MA                     (0)

#define YCABLE_NEED_WAIT                    (1)
#define YCABLE_ADC_INVALID                  (0)

struct ycable_info {
	struct platform_device *pdev;
	struct device *dev;
	bool ycable_support;
	int ycable_status;
	unsigned int otg_adc_channel;
	unsigned int ycable_iin_curr;
	unsigned int ycable_ichg_curr;
	bool ycable_otg_enable_flag;
	bool ycable_charger_enable_flag;
	struct delayed_work ycable_work;
	struct blocking_notifier_head ycable_evt_nh;
	struct notifier_block usb_nb;
	struct completion dev_off_completion;
};

enum ycable_status {
	YCABLE_STATUS_BEGIN = 0,

	YCABLE_UNKNOW = YCABLE_STATUS_BEGIN,
	YCABLE_CHARGER,
	YCABLE_OTG,

	YCABLE_STATUS_END,
};

enum ycable_id_status {
	YCABLE_ID_BEGIN = 0,

	YCABLE_ID_HIGH = YCABLE_ID_BEGIN,
	YCABLE_ID_LOW_OTG,
	YCABLE_ID_LOW_WITH_CHARGE,
	YCABLE_ID_LOW_INVALID,

	YCABLE_ID_END,
};

#ifdef CONFIG_HUAWEI_YCABLE
extern enum ycable_status ycable_get_status(void);
extern int ycable_register_event_notifier(struct notifier_block *nb);
extern int ycable_unregister_event_notifier(struct notifier_block *nb);
extern int ycable_is_support(void);
extern int ycable_handle_otg_event(enum otg_dev_event_type event_type,
	bool need_wait);
extern int ycable_get_input_current(void);
extern int ycable_get_charge_current(void);
extern void ycable_init_devoff_completion(void);
extern bool ycable_is_charge_connect(int vol_value);
extern int ycable_get_gpio_adc_min(void);
extern bool ycable_is_with_charger(void);

#else

static inline enum ycable_status ycable_get_status(void)
{
	return 0;
}

static inline int ycable_register_event_notifier(struct notifier_block *nb)
{
	return 0;
}

static inline int ycable_unregister_event_notifier(struct notifier_block *nb)
{
	return 0;
}

static inline int ycable_is_support(void)
{
	return 0;
}

static inline int ycable_handle_otg_event(enum otg_dev_event_type event_type,
	bool need_wait)
{
	return 0;
}

static inline int ycable_get_input_current(void)
{
	return 0;
}

static inline int ycable_get_charge_current(void)
{
	return 0;
}

static inline void ycable_init_devoff_completion(void)
{
}

static inline bool ycable_is_charge_connect(int vol_value)
{
	return 0;
}

static inline int ycable_get_gpio_adc_min(void)
{
	return 0;
}

static inline bool ycable_is_with_charger(void)
{
	return 0;
}
#endif /* CONFIG_HUAWEI_YCABLE */

#endif /* _HUAWEI_YCABLE_H_ */
