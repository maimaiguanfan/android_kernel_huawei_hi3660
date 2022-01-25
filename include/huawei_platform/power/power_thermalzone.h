/*
 * power_thermalzone.h
 *
 * thermal for power module
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

#ifndef _POWER_THERMALZONE_H_
#define _POWER_THERMALZONE_H_

#define POWER_TZ_SENSORS                      (10)
#define POWER_TZ_STR_MAX_LEN                  (16)

#define POWER_TZ_DEFAULT_TEMP                 (25000)
#define POWER_TZ_MIN_TEMP                     (-40000)
#define POWER_TZ_MAX_TEMP                     (125000)

/* in order to resolve divisor less than zero */
#define PRORATE_OF_INIT                       (1000)

/* get temp data through hisi adc */
#define POWER_TZ_DEFAULT_OPS                  "hisi_adc"
#define POWER_TZ_ADC_RETRY_TIMES              (3)

enum power_tz_ntc_index {
	NTC_BEGIN = 0,

	NTC_07050125 = NTC_BEGIN, /* Thermitor 07050125 */
	NTC_07050124_PULLUP_10K = 1, /* Thermitor 07050124, pullup 10K */
	NTC_07050124_PULLUP_12K = 2, /* Thermitor 07050124, pullup 12K */
	NTC_07050124_PULLUP_18K = 3, /* Thermitor 07050124, pullup 18K */

	NTC_END,
};

enum power_tz_sensors_para {
	THERMAL_SENSOR_NAME,
	THERMAL_DEVICE_NAME,
	THERMAL_NTC_INDEX,
	THERMAL_ZONE_PARA_TOTAL,
};

struct power_tz_sensor {
	char sensor_name[POWER_TZ_STR_MAX_LEN];
	char ops_name[POWER_TZ_STR_MAX_LEN];
	int adc_channel;
	int ntc_index;
	struct thermal_zone_device *tz_dev;
	int (*get_raw_data)(int adc_channel, long *data);
};

struct power_tz_info {
	struct platform_device *pdev;
	struct device *dev;
	int total_sensor;
	struct power_tz_sensor sensor[POWER_TZ_SENSORS];
};

struct power_tz_ops {
	int (*get_raw_data)(int adc_channel, long *data);
};

int power_tz_ops_register(struct power_tz_ops *ops, char *ops_name);

#endif /* _POWER_THERMALZONE_H_ */
