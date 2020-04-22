/*
 * vsys_switch.h
 *
 * vsys switch driver
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

#ifndef _VSYS_SWITCH_H_
#define _VSYS_SWITCH_H_

#include <linux/wakelock.h>

#define BUCK_CHNL_ENABLE                (1)
#define BUCK_CHNL_DISABLE               (0)
#define BUCK_VOUT_LOW                   (0)
#define BUCK_VOUT_HIGH                  (1)

#define SC_CHNL_ENABLE                  (1)
#define SC_CHNL_DISABLE                 (0)

#define VSYS_BUCK_OPEN                  (0)
#define VSYS_SC_OPEN                    (1)

#define CONTROL_INTERVAL                (2000) /* ms */

/* unit: mV */
#define VBATT_TH_DEFAULT                (7500)
#define VBATT_TH_MAX                    (8800)
#define VBATT_TH_MIN                    (6400)
#define VBATT_BASE_DEFAULT              (7000)
#define VBATT_GAP_DEFAULT               (200)

#define DISCHG_CURR_MAX                 (2000) /* mA */
#define COMP_RES_DEFAULT                (140) /* mohm */

/* length of moving average filter */
#define MAF_BUFFER_LEN                  (10)

/* mA positive: discharging  negative: charging */
#define IBATT_VAL_MAX                   (-6000) /* mA */
#define RBATT_VAL_MAX                   (10000) /* mohm * 10 * 2 */
#define VBATT_VAL_MIN                   (6400) /* mV */

#define SWITCH_VSYS_BUCK                (0)
#define SWITCH_VSYS_SC                  (1)
#define SWITCH_VSYS_AUTO                (2)

#define RBATT_SF_ROWS_MAX               (20)
#define RBATT_SF_COLS_MAX               (20)
#define RBATT_AGING_PARA_LEVEL          (8)

#define TEN                             (10)
#define HUNDRED                         (100)
#define THOUSAND                        (1000)

#define SWITCH_SC_RETRY_MAX             (8)

enum vsys_sc_id {
	VSYS_SC_BEGIN = 0,

	VSYS_SC_DA9313 = VSYS_SC_BEGIN,

	VSYS_SC_END,
};

enum vsys_ovp_switch_id {
	VSYS_OVP_SWITCH_BEGIN = 0,

	VSYS_OVP_SWITCH_FPF2283 = VSYS_OVP_SWITCH_BEGIN,

	VSYS_OVP_SWITCH_END,
};

enum vsys_sc_int_type {
	VSYS_SC_INT_BEGIN = 0,

	VSYS_SC_INT_NOT_PG = VSYS_SC_INT_BEGIN,

	VSYS_SC_INT_END,
};

enum vsys_sc_set_state_result {
	VSYS_SC_SET_STATE_BEGIN = 0,

	VSYS_SC_SET_STATE_SUCC = VSYS_SC_SET_STATE_BEGIN,
	VSYS_SC_SET_GPIO_NONKEY_FAIL,
	VSYS_SC_SET_GPIO_EN_FAIL,
	VSYS_SC_SET_TEST_REGS_FAIL,
	VSYS_SC_SET_MODE_CTRL_REG_FAIL,
	VSYS_SC_NOT_PG,

	VSYS_SC_SET_STATE_END,
};

enum vsys_switch_sysfs_type {
	VSYS_SWITCH_SYSFS_BEGIN = 0,

	VSYS_SWITCH_SYSFS_SWITCH_VSYS = VSYS_SWITCH_SYSFS_BEGIN,
	VSYS_SWITCH_SYSFS_ENABLE_SC,
	VSYS_SWITCH_SYSFS_ENABLE_BUCK,
	VSYS_SWITCH_SYSFS_VBATT_TH,
	VSYS_SWITCH_SYSFS_VSYS_VOLT,
	VSYS_SWITCH_SYSFS_VSYS_CHNL,

	VSYS_SWITCH_SYSFS_END,
};

enum vsys_switch_rbatt_aging_para_info {
	RBATT_AGING_PARA_INFO_BEGIN = 0,

	RBATT_AGING_PARA_INFO_CYCLE = RBATT_AGING_PARA_INFO_BEGIN,
	RBATT_AGING_PARA_INFO_COEFFICIENT,

	RBATT_AGING_PARA_INFO_TOTAL,
};

struct vsys_switch_batt_para {
	int vbatt_avg;
	int vbatt_sum;
	int vbatt_maf_count;
	int ibatt_avg;
	int ibatt_sum;
	int ibatt_maf_count;
	int rbatt_avg;
	int rbatt_sum;
	int rbatt_maf_count;
	int comp_res;
};

struct vsys_switch_ctrl_para {
	int vbatt_gap; /* hysteresis voltage */
	int vbatt_base; /* threshold that just satisfies to enable SC */
	int vbatt_th; /* voltage threshold to switch SC channel */
	int dischg_curr_max; /* maximum discharging current */
	int enable_sc;
	int enable_buck;
	int switch_vsys_chnl;
};

struct vsys_switch_state_info {
	int vsys_volt; /* output voltage of vsys */
	int curr_vsys_chnl; /* current power supply channel */
	int sc_not_pg;
};

struct vsys_buck_device_ops {
	const char *chip_name;

	int (*get_state)(void);
	int (*set_state)(int enable);
	int (*get_vout)(void);
	int (*set_vout)(int vout);
};

struct vsys_sc_device_ops {
	const char *chip_name;

	int (*get_state)(void);
	int (*set_state)(int enable);
	int (*get_id)(int *id);
};

struct vsys_ovp_switch_device_ops {
	const char *chip_name;

	int (*set_state)(int enable);
	int (*get_state)(void);
	int (*get_id)(int *id);
};

/*
 * struct rbatt_lut -
 * @rows:   number of soc entries should be <= RBATT_SF_COLS_MAX
 * @cols:   number of temperature entries should be <= RBATT_SF_ROWS_MAX
 * @soc:    the state of charge at which sf data is available in the tbl
 * @temp:   the temperature at which sf data is available in the tbl
 * @sf:     the scaling factor data
 */
struct vsys_switch_rbatt_lut {
	int rows;
	int cols;
	int soc[RBATT_SF_COLS_MAX];
	int temp[RBATT_SF_ROWS_MAX];
	int sf[RBATT_SF_ROWS_MAX][RBATT_SF_COLS_MAX];
};

struct vsys_switch_rbatt_aging_para {
	int cycle;
	int coefficient;
};

struct vsys_switch_rbatt_aging_data {
	int total_level;
	struct vsys_switch_rbatt_aging_para *rbatt_aging_para;
};

struct vsys_switch_device_info {
	struct device *dev;
	struct notifier_block sc_event_nb;
	struct work_struct sc_event_work;
	struct delayed_work ctrl_work;
	struct wake_lock wakelock;
	struct vsys_buck_device_ops *buck_ops;
	struct vsys_sc_device_ops *sc_ops;
	struct vsys_ovp_switch_device_ops *ovp_ops;
	struct vsys_switch_batt_para batt_para;
	struct vsys_switch_ctrl_para ctrl_para;
	struct vsys_switch_state_info state_info;
	struct vsys_switch_rbatt_lut *rbatt_sf_lut;
	struct vsys_switch_rbatt_aging_data rbatt_aging_data;
	int ctrl_work_interval;
	int sc_event_type;
	int switch_sc_retry_num;
	bool cancel_ctrl_work_flag;
};

extern struct atomic_notifier_head vsys_sc_event_nh;
extern struct device *vsys_switch_dev;

int vsys_buck_ops_register(struct vsys_buck_device_ops *ops);
int vsys_sc_ops_register(struct vsys_sc_device_ops *ops);
int vsys_ovp_switch_ops_register(struct vsys_ovp_switch_device_ops *ops);

#endif /* _VSYS_SWITCH_H_ */
