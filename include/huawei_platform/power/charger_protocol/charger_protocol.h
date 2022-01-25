/*
 * charger_protocol.h
 *
 * charger protocol driver
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

#ifndef _ADAPTER_PROTOCOL_H_
#define _ADAPTER_PROTOCOL_H_

#define ADAPTER_RD_BUF_SIZE (32)
#define ADAPTER_WR_BUF_SIZE (32)

enum adapter_sysfs_type {
	ADAPTER_SYSFS_BEGIN = 0,

	ADAPTER_SYSFS_SUPPORT_MODE = ADAPTER_SYSFS_BEGIN,
	ADAPTER_SYSFS_CHIP_ID,
	ADAPTER_SYSFS_VENDOR_ID,
	ADAPTER_SYSFS_MODULE_ID,
	ADAPTER_SYSFS_SERIAL_NO,
	ADAPTER_SYSFS_HWVER,
	ADAPTER_SYSFS_FWVER,
	ADAPTER_SYSFS_MIN_VOLT,
	ADAPTER_SYSFS_MAX_VOLT,
	ADAPTER_SYSFS_MIN_CUR,
	ADAPTER_SYSFS_MAX_CUR,

	ADAPTER_SYSFS_END,
};

enum adapter_protocol_type {
	ADAPTER_PROTOCOL_BEGIN = 0,

	ADAPTER_PROTOCOL_SCP = ADAPTER_PROTOCOL_BEGIN, /* huawei scp */

	ADAPTER_PROTOCOL_END,
};

enum adapter_support_mode {
	ADAPTER_SUPPORT_UNDEFINED = 0x0,
	ADAPTER_SUPPORT_SCP_B_LVC = 0x1,
	ADAPTER_SUPPORT_SCP_B_SC = 0x2,
	ADAPTER_SUPPORT_SCP_A = 0x3,
};

enum adapter_type {
	ADAPTER_TYPE_UNKNOWN = 0x0,
	ADAPTER_TYPE_5V5A = 0x1,
	ADAPTER_TYPE_10V4A = 0x2,
	ADAPTER_TYPE_10V2A = 0x3,
	ADAPTER_TYPE_20V3P25A = 0x4,
};

enum adapter_detect_error_code {
	ADAPTER_DETECT_OTHER = -1,
	ADAPTER_DETECT_SUCC = 0,
	ADAPTER_DETECT_FAIL = 1,
};

struct adapter_support_mode_data {
	int mode;
	const char *mode_name;
};

struct adapter_init_data {
	int scp_mode_enable;
	int vset_boundary;
	int iset_boundary;
	int init_adaptor_voltage;
	int watchdog_timer;
	int dp_delitch_time;
};

struct adapter_info {
	int support_mode;
	int chip_id;
	int vendor_id;
	int module_id;
	int serial_no;
	int hwver;
	int fwver;
	int min_volt;
	int max_volt;
	int min_cur;
	int max_cur;
};

struct adapter_protocol_ops {
	const char *type_name;
	int (*soft_reset_master)(void);
	int (*soft_reset_slave)(void);
	int (*detect_adapter_support_mode)(int *mode);
	int (*get_support_mode)(int *mode);
	int (*get_device_info)(struct adapter_info *info);
	int (*get_chip_vendor_id)(int *id);
	int (*set_output_enable)(int enable);
	int (*set_reset)(int enable);
	int (*set_output_voltage)(int volt);
	int (*get_output_voltage)(int *volt);
	int (*set_output_current)(int cur);
	int (*get_output_current)(int *cur);
	int (*get_output_current_set)(int *cur);
	int (*get_min_voltage)(int *volt);
	int (*get_max_voltage)(int *volt);
	int (*get_min_current)(int *cur);
	int (*get_max_current)(int *cur);
	int (*get_inside_temp)(int *temp);
	int (*get_port_temp)(int *temp);
	int (*get_port_leakage_current_flag)(int *flag);
	int (*auth_encrypt_start)(int key, unsigned char *hash, int size);
	int (*auth_encrypt_release)(void);
	int (*set_usbpd_enable)(int enable);
	int (*set_default_state)(void);
	int (*set_default_param)(void);
	int (*set_init_data)(struct adapter_init_data *data);
	int (*get_protocol_register_state)(void);
	int (*get_adp_type)(int *type);
};

struct adapter_dev {
	struct device *dev;
	unsigned int type;
	struct adapter_info info;
	unsigned int total_ops;
	struct adapter_protocol_ops *p_ops[ADAPTER_PROTOCOL_END];
};

#ifdef CONFIG_CHARGER_PROTOCOL
int adapter_protocol_ops_register(struct adapter_protocol_ops *ops);

int adapter_soft_reset_master(void);
int adapter_soft_reset_slave(void);
int adapter_detect_adapter_support_mode(int *mode);
int adapter_get_support_mode(int *mode);
int adapter_get_device_info(void);
int adapter_show_device_info(void);
int adapter_get_chip_vendor_id(int *id);
int adapter_set_output_enable(int enable);
int adapter_set_reset(int enable);
int adapter_set_output_voltage(int volt);
int adapter_get_output_voltage(int *volt);
int adapter_set_output_current(int cur);
int adapter_get_output_current(int *cur);
int adapter_get_output_current_set(int *cur);
int adapter_get_min_voltage(int *volt);
int adapter_get_max_voltage(int *volt);
int adapter_get_min_current(int *cur);
int adapter_get_max_current(int *cur);
int adapter_get_inside_temp(int *temp);
int adapter_get_port_temp(int *temp);
int adapter_get_port_leakage_current_flag(int *flag);
int adapter_auth_encrypt_start(int key, unsigned char *hash, int size);
int adapter_auth_encrypt_release(void);
int adapter_set_usbpd_enable(int enable);
int adapter_set_default_state(void);
int adapter_set_default_param(void);
int adapter_set_init_data(struct adapter_init_data *data);
int adapter_get_protocol_register_state(void);
int adapter_get_adp_type(int *type);

#else

static inline int adapter_protocol_ops_register(
	struct adapter_protocol_ops *ops)
{
	return -1;
}

static inline int adapter_soft_reset_master(void)
{
	return -1;
}

static inline int adapter_soft_reset_slave(void)
{
	return -1;
}

static inline int adapter_detect_adapter_support_mode(int *mode)
{
	return -1;
}

static inline int adapter_get_support_mode(int *mode)
{
	return -1;
}

static inline int adapter_get_device_info(void)
{
	return -1;
}

static inline int adapter_show_device_info(void)
{
	return -1;
}

static inline int adapter_get_chip_vendor_id(int *id)
{
	return -1;
}

static inline int adapter_set_output_enable(int enable)
{
	return -1;
}

static inline int adapter_set_reset(int enable)
{
	return -1;
}

static inline int adapter_set_output_voltage(int volt)
{
	return -1;
}

static inline int adapter_get_output_voltage(int *volt)
{
	return -1;
}

static inline int adapter_set_output_current(int cur)
{
	return -1;
}

static inline int adapter_get_output_current(int *cur)
{
	return -1;
}

static inline int adapter_get_output_current_set(int *cur)
{
	return -1;
}

static inline int adapter_get_min_voltage(int *volt)
{
	return -1;
}

static inline int adapter_get_max_voltage(int *volt)
{
	return -1;
}

static inline int adapter_get_min_current(int *cur)
{
	return -1;
}

static inline int adapter_get_max_current(int *cur)
{
	return -1;
}

static inline int adapter_get_inside_temp(int *temp)
{
	return -1;
}

static inline int adapter_get_port_temp(int *temp)
{
	return -1;
}

static inline int adapter_get_port_leakage_current_flag(int *flag)
{
	return -1;
}

static inline int adapter_auth_encrypt_start(int key, unsigned char *hash,
	int size)
{
	return -1;
}

static inline int adapter_auth_encrypt_release(void)
{
	return -1;
}

static inline int adapter_set_usbpd_enable(int enable)
{
	return -1;
}

static inline int adapter_set_default_state(void)
{
	return -1;
}

static inline int adapter_set_default_param(void)
{
	return -1;
}

static inline int adapter_set_init_data(struct adapter_init_data *data)
{
	return -1;
}

static inline int adapter_get_protocol_register_state(void)
{
	return -1;
}

static inline int adapter_get_adp_type(int *type)
{
	return -1;
}
#endif /* CONFIG_CHARGER_PROTOCOL */

#endif /* _ADAPTER_PROTOCOL_H_ */
