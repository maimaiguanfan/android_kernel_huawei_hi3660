/*
 * wireless_transmitter.h
 *
 * head file for wireless reverse charging
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

#ifndef _WIRELESS_TRANSMITTER_H_
#define _WIRELESS_TRANSMITTER_H_

#include <huawei_platform/power/wireless_charger.h>
#include <huawei_platform/power/huawei_charger.h>

#define WL_TX_FAIL                                    (-1)
#define WL_TX_SUCC                                    (0)
#define WL_TX_MONITOR_INTERVAL                        (300)

#define WL_TX_SOC_MIN                                 (20)
#define WL_TX_BATT_TEMP_MAX                           (50)
#define WL_TX_TBATT_DELTA_TH                          (15)
#define WL_TX_BATT_TEMP_MIN                           (-10)
#define WL_TX_BATT_TEMP_RESTORE                       (35)
#define WL_TX_STR_LEN_16                              (16)
#define WL_TX_STR_LEN_32                              (32)

#define WL_TX_VIN_MIN                                 (4500) /* mV */
#define WL_TX_VIN_MAX                                 (5800) /* mV */
#define WL_TX_VIN_RETRY_CNT                           (25)
#define WL_TX_VIN_RETRY_CNT1                          10
#define WL_TX_VIN_RETRY_CNT2                          (250)
#define WL_TX_VIN_SLEEP_TIME                          (20)
#define WL_TX_IIN_SAMPLE_LEN                          (10)
#define WL_TX_PING_TIMEOUT_1                          (120)
#define WL_TX_PING_TIMEOUT_2                          (119)
#define WL_TX_PING_CHECK_INTERVAL                     (20)
#define WL_TX_PING_VIN_OVP_CNT                        (3)
#define WL_TX_PING_VIN_UVP_CNT                        (10) /* 6s-10s */
#define WL_TX_I2C_ERR_CNT                             (5)
#define WL_TX_IIN_LOW                                 (300) /* mA */
#define WL_TX_IIN_LOW_CNT                             (30*60*1000) /* 30min */
#define WL_TX_MODE_ERR_CNT                            (10)
#define WL_TX_MODE_ERR_CNT1                           3
#define WL_TX_MODE_ERR_CNT2                           (20)
#define WL_TX_5VBST_MIN_FOP                           (138) /* kHz */

#define WL_TX_MONITOR_LOG_INTERVAL                    (3000)
#define WL_TX_CHARGER_TYPE_MAX                        (12)

#define WL_TX_DMDLOG_SIZE                             (512)

enum wireless_tx_pwr_sw_scene {
	PWR_SW_SCN_MIN = 0x00,
	PWR_SW_BY_VBUS_ON = PWR_SW_SCN_MIN,
	PWR_SW_BY_VBUS_OFF,
	PWR_SW_BY_OTG_ON,
	PWR_SW_BY_OTG_OFF,
	PWR_SW_SCN_MAX,
};
enum wireless_tx_power_src {
	PWR_SRC_NULL = 0x00,
	PWR_SRC_VBUS,
	PWR_SRC_OTG,
	PWR_SRC_5VBST,
	PWR_SRC_SPBST, /* Specialized boost */
	PWR_SRC_NA,
	PWR_SRC_MAX,
};
enum wireless_tx_power_type {
	WL_TX_PWR_VBUS_OTG = 0x00,
	WL_TX_PWR_5VBST_OTG,
	WL_TX_PWR_SPBST, /* Specialized boost */
	WL_TX_PWR_MAX,
};
enum wireless_tx_status_type {
	/* tx normal status */
	WL_TX_STATUS_DEFAULT = 0x00,    /* default */
	WL_TX_STATUS_PING,              /* TX ping RX */
	WL_TX_STATUS_PING_SUCC,         /* ping success */
	WL_TX_STATUS_IN_CHARGING,       /* in reverse charging status */
	WL_TX_STATUS_CHARGE_DONE,       /* RX charge done */
	/* tx error status */
	WL_TX_STATUS_FAULT_BASE = 0x10, /* base */
	WL_TX_STATUS_TX_CLOSE,          /* chip fault or user close TX mode */
	WL_TX_STATUS_RX_DISCONNECT,     /* monitor RX disconnect */
	WL_TX_STATUS_PING_TIMEOUT,      /* can not ping RX */
	WL_TX_STATUS_TBATT_LOW,         /* battery temperature too low */
	WL_TX_STATUS_TBATT_HIGH,        /* battery temperature too high */
	WL_TX_STATUS_IN_WL_CHARGING,    /* in wireless charging */
	WL_TX_STATUS_SOC_ERROR,         /* capacity is too low */
};
enum wireless_tx_event_type {
	WL_TX_EVENT_GET_CFG,
	WL_TX_EVENT_HANDSHAKE_SUCC,
	WL_TX_EVENT_CHARGEDONE,
	WL_TX_EVENT_CEP_TIMEOUT,
	WL_TX_EVENT_EPT_CMD,
	WL_TX_EVENT_OVP,
	WL_TX_EVENT_OCP,
	WL_TX_EVENT_PING_RX,
};
enum wireless_tx_stage {
	WL_TX_STAGE_DEFAULT = 0,
	WL_TX_STAGE_POWER_SUPPLY,
	WL_TX_STAGE_CHIP_INIT,
	WL_TX_STAGE_PING_RX,
	WL_TX_STAGE_REGULATION,
	WL_TX_STAGE_TOTAL,
};
enum wireless_tx_sysfs_type {
	WL_TX_SYSFS_TX_OPEN = 0,
	WL_TX_SYSFS_TX_STATUS,
	WL_TX_SYSFS_TX_IIN_AVG,
	WL_TX_SYSFS_DPING_FREQ,
	WL_TX_SYSFS_DPING_INTERVAL,
	WL_TX_SYSFS_MAX_FOP,
	WL_TX_SYSFS_MIN_FOP,
	WL_TX_SYSFS_TX_FOP,
};
struct wireless_tx_device_ops {
	int (*chip_reset)(void);
	void (*rx_enable)(int);
	void (*rx_sleep_enable)(int);
	int (*check_fwupdate)(enum wireless_mode);
	int (*kick_watchdog)(void);
	int (*tx_chip_init)(void);
	int (*tx_stop_config)(void);
	int (*enable_tx_mode)(bool);
	int (*get_tx_iin)(u16 *);
	int (*get_tx_vin)(u16 *);
	int (*get_tx_vrect)(u16 *);
	int (*get_chip_temp)(u8 *);
	int (*get_tx_fop)(u16 *);
	int (*set_tx_max_fop)(u16);
	int (*get_tx_max_fop)(u16 *);
	int (*set_tx_min_fop)(u16);
	int (*get_tx_min_fop)(u16 *);
	int (*set_tx_ping_frequency)(u16);
	int (*get_tx_ping_frequency)(u16 *);
	int (*set_tx_ping_interval)(u16);
	int (*get_tx_ping_interval)(u16 *);
	bool (*check_rx_disconnect)(void);
	bool (*in_tx_mode)(void);
	int (*set_tx_open_flag)(bool);
};

struct wireless_tx_device_info {
	struct device *dev;
	struct notifier_block tx_event_nb;
	struct work_struct wireless_tx_check_work;
	struct work_struct wireless_tx_evt_work;
	struct delayed_work wireless_tx_monitor_work;
	struct wireless_tx_device_ops *tx_ops;
	enum wireless_tx_power_type pwr_type;
	enum wireless_tx_power_src cur_pwr_src;
	enum wireless_tx_pwr_sw_scene cur_pwr_sw_scn;
	int gpio_sw;
	int gpio_sw_valid_val;
	unsigned int monitor_interval;
	unsigned int ping_timeout;
	unsigned int tx_event_type;
	unsigned int tx_event_data;
	unsigned int tx_iin_avg;
	unsigned int i2c_err_cnt;
	unsigned int tx_mode_err_cnt;
	unsigned int tx_iin_low_cnt;
	bool standard_rx;
	bool stop_reverse_charge; /* record  driver state */
};

extern struct blocking_notifier_head tx_event_nh;
extern int wireless_tx_ops_register(struct wireless_tx_device_ops *ops);
extern void wireless_tx_cancel_work(enum wireless_tx_pwr_sw_scene);
extern void wireless_tx_restart_check(enum wireless_tx_pwr_sw_scene);
extern int wireless_tx_get_tx_iin_limit(enum huawei_usb_charger_type);
extern bool wltx_need_disable_wired_dc(void);
#ifdef CONFIG_WIRELESS_CHARGER
bool wireless_is_in_tx_mode(void);
bool wireless_tx_get_tx_open_flag(void);
#else
static inline bool wireless_is_in_tx_mode(void)
{
	return false;
}

static inline bool wireless_tx_get_tx_open_flag(void)
{
	return false;
}
#endif

#endif /* _WIRELESS_TRANSMITTER_H_ */
