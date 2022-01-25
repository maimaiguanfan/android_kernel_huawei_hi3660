/*
 * hw_comm_pmic.h
 *
 * debug for pmic sensor
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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
#ifndef _HW_COMM_PMIC_H_
#define _HW_COMM_PMIC_H_

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <linux/irq.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>

/******************** pmic extern common func cfg declare ********************/
/** not allowed to add pmic types below without the permission of the owner **/
enum pmic_seq_index {
	VOUT_LDO_1 = 0,
	VOUT_LDO_2,
	VOUT_LDO_3,
	VOUT_LDO_4,
	VOUT_LDO_5,
	VOUT_BUCK_1,
	VOUT_BUCK_2,
	VOUT_MAX,
	VOUT_BOOST,
	VOUT_BOOST_EN,
};

// typedef pmic_seq_index_t for compatible history code
// exposure of external interfaces
typedef enum pmic_seq_index pmic_seq_index_t;

enum pmic_power_req_src_t {
	TP_PMIC_REQ = 1 << 0,
	FP_PMIC_REQ = 1 << 1,
	VIB_PMIC_REQ = 1 << 2,
	MAIN_CAM_PMIC_REQ = 1 << 3,
	AUDIO_PMIC_REQ = 1 << 4,
	DF_PMIC_REQ = 1 << 5,
	DOT_PMIC_REQ = 1 << 6,
	VCM_PMIC_REQ = 1 << 7,
};

struct hw_comm_pmic_cfg_t {
	u8 pmic_num;
	pmic_seq_index_t pmic_power_type;
	u32 pmic_power_voltage;
	bool pmic_power_state;
};

#ifdef CONFIG_HUAWEI_SENSORS_INPUT_INFO
int hw_pmic_power_cfg(enum pmic_power_req_src_t pmic_power_src,
	struct hw_comm_pmic_cfg_t *comm_pmic_cfg);
#else
static inline int hw_pmic_power_cfg(enum pmic_power_req_src_t pmic_power_src,
	struct hw_comm_pmic_cfg_t *comm_pmic_cfg)
{
	return 0;
}
#endif
/** not allowed to add pmic types above without the permission of the owner **/
/********************** pmic controler struct define **********************/
struct hw_pmic_info {
	const char *name;
	int index;
	unsigned int slave_address;
	unsigned int intr;
	unsigned int irq;
	unsigned int irq_flag;
	unsigned int boost_en_pin;
	unsigned int power_vote[VOUT_BOOST];
};

struct hw_pmic_i2c_client {
	struct hw_pmic_i2c_fn_t *i2c_func_tbl;
	struct i2c_client *client;
};

struct hw_pmic_i2c_fn_t {
	int (*i2c_read)(struct hw_pmic_i2c_client *, u8, u8 *);
	int (*i2c_write)(struct hw_pmic_i2c_client *, u8, u8);
};

/* pmic function table */
struct hw_pmic_ctrl_t {
	struct platform_device *pdev;
	struct mutex *hisi_pmic_mutex;
	struct device *dev;
	struct hw_pmic_fn_t *func_tbl;
	struct hw_pmic_i2c_client *pmic_i2c_client;
	struct hw_pmic_info pmic_info;
	void *pdata;
};

struct hw_pmic_fn_t {
	int (*pmic_config)(struct hw_pmic_ctrl_t *, void *);
	int (*pmic_on)(struct hw_pmic_ctrl_t *, void *);
	int (*pmic_off)(struct hw_pmic_ctrl_t *);
	int (*pmic_init)(struct hw_pmic_ctrl_t *);
	int (*pmic_exit)(struct hw_pmic_ctrl_t *);
	int (*pmic_match)(struct hw_pmic_ctrl_t *);
	int (*pmic_get_dt_data)(struct hw_pmic_ctrl_t *);
	int (*pmic_seq_config)(struct hw_pmic_ctrl_t *, enum pmic_seq_index,
		u32, int);
	int (*pmic_check_exception)(struct hw_pmic_ctrl_t *);
};

/***************define pmic globle var******************/
#define PMIC_POWER_OFF 0x00
#define PMIC_POWER_ON  0x01

/***************pmic comm func declare******************/
#define DEFINE_HW_PMIC_MUTEX(name) \
	static struct mutex pmic_mut_##name = __MUTEX_INITIALIZER(pmic_mut_##name)

#ifdef CONFIG_HUAWEI_DSM
#define REPORT_PMIC_DSM(client_pmic, reg, value, pmureg, pmuvalue, dmd_no, dmd_detail) \
	do { \
		if (!dsm_client_ocuppy(client_pmic)) { \
			dsm_client_record(client_pmic,\
			"" dmd_detail " 0x%x=0x%x, pmu: 0x%x=0x%x\n",\
			reg, value, pmureg, pmuvalue); \
			dsm_client_notify(client_pmic, dmd_no); \
		} \
	} while (0)
#endif

/***************extern function declare******************/
int hw_pmic_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id);
struct hw_pmic_ctrl_t *hw_get_pmic_ctrl(void);
void hw_pmic_release_intr(struct hw_pmic_ctrl_t *pmic_ctrl);
int hw_pmic_setup_intr(struct hw_pmic_ctrl_t *pmic_ctrl);
int hw_pmic_gpio_boost_enable(struct hw_pmic_ctrl_t *pmic_ctrl, int state);
#endif
