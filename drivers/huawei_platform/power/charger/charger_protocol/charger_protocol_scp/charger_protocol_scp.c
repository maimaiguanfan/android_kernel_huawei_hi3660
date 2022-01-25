/*
 * charger_protocol_scp.c
 *
 * scp protocol driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/delay.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/charger_protocol/charger_protocol.h>
#include <huawei_platform/power/charger_protocol/charger_protocol_scp.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG scp_protocol
HWLOG_REGIST();

static struct scp_protocol_dev *g_scp_protocol_dev;

static const char * const scp_protocol_device_id_table[] = {
	[DEVICE_ID_FSA9685] = "fsa9685",
	[DEVICE_ID_SCHARGER_V300] = "scharger_v300",
	[DEVICE_ID_SCHARGER_V600] = "scharger_v600",
	[DEVICE_ID_FUSB3601] = "fusb3601",
};

/* power: 10^n */
static int scp_protocol_ten_power[] = {
	[0] = 1,
	[1] = 10,
	[2] = 100,
	[3] = 1000,
};

static int scp_protocol_get_device_id(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(scp_protocol_device_id_table); i++) {
		if (!strncmp(str, scp_protocol_device_id_table[i], strlen(str)))
			return i;
	}

	return -1;
}

static struct scp_protocol_dev *scp_protocol_get_dev(void)
{
	if (!g_scp_protocol_dev) {
		hwlog_err("g_scp_protocol_dev is null\n");
		return NULL;
	}

	return g_scp_protocol_dev;
}

static struct scp_protocol_ops *scp_protocol_get_ops(void)
{
	if (!g_scp_protocol_dev || !g_scp_protocol_dev->p_ops) {
		hwlog_err("g_scp_protocol_dev or p_ops is null\n");
		return NULL;
	}

	return g_scp_protocol_dev->p_ops;
}

int scp_protocol_ops_register(struct scp_protocol_ops *ops)
{
	int dev_id;

	if (!g_scp_protocol_dev || !ops || !ops->chip_name) {
		hwlog_err("g_scp_protocol_dev or ops or chip_name is null\n");
		return -1;
	}

	dev_id = scp_protocol_get_device_id(ops->chip_name);
	if (dev_id < 0) {
		hwlog_err("%s ops register fail\n", ops->chip_name);
		return -1;
	}

	g_scp_protocol_dev->p_ops = ops;
	g_scp_protocol_dev->dev_id = dev_id;

	hwlog_info("(%d:%s) ops register ok\n",
		dev_id, ops->chip_name);
	return 0;
}

static int scp_protocol_check_trans_num(int num)
{
	/* num must be less equal than 16 */
	if ((num >= BYTE_ONE) && (num <= BYTE_SIXTEEN)) {
		/* num must be 1 or even numbers */
		if ((num > 1) && (num % 2 == 1))
			return -1;

		return 0;
	} else {
		return -1;
	}
}

static int scp_protocol_get_rw_error_flag(void)
{
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	return l_dev->info.rw_error_flag;
}

static void scp_protocol_set_rw_error_flag(int flag)
{
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return;

	hwlog_info("set_rw_error_flag: %d\n", flag);
	l_dev->info.rw_error_flag = flag;
}

static int scp_protocol_reg_read(int reg, int *val, int num)
{
	int ret;
	int i;
	u8 value[BYTE_SIXTEEN] = {0}; /* 16-byte buffer */
	struct scp_protocol_ops *l_ops = NULL;

	if (scp_protocol_get_rw_error_flag())
		return -1;

	l_ops = scp_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->reg_read) {
		hwlog_err("reg_read is null\n");
		return -1;
	}

	if (scp_protocol_check_trans_num(num)) {
		hwlog_err("num(%d) is invalid\n", num);
		return -1;
	}

	/* multi-byte read reg 0xa8 & 0xa9 */
	if ((l_ops->reg_multi_read) && (num == BYTE_TWO) &&
		(reg == SCP_PROTOCOL_READ_VOUT_HBYTE)) {
		ret = l_ops->reg_multi_read((u8)reg, value, (u8)num);
		if (ret) {
			if (reg != SCP_PROTOCOL_ADP_TYPE0)
				scp_protocol_set_rw_error_flag(RW_ERROR_FLAG);

			hwlog_err("reg_multi_read fail(reg=0x%x)\n", reg);
			return -1;
		}

		for (i = 0; i < num; i++)
			val[i] = value[i];

		return 0;
	}

	ret = l_ops->reg_read(reg, val, num);
	if (ret < 0) {
		if (reg != SCP_PROTOCOL_ADP_TYPE0)
			scp_protocol_set_rw_error_flag(RW_ERROR_FLAG);

		hwlog_err("reg_read fail(reg=0x%x)\n", reg);
		return -1;
	}

	return 0;
}

static int scp_protocol_reg_write(int reg, int *val, int num)
{
	int ret;
	struct scp_protocol_ops *l_ops = NULL;

	if (scp_protocol_get_rw_error_flag())
		return -1;

	l_ops = scp_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->reg_write) {
		hwlog_err("reg_write is null\n");
		return -1;
	}

	if (scp_protocol_check_trans_num(num)) {
		hwlog_err("num(%d) is invalid\n", num);
		return -1;
	}

	ret = l_ops->reg_write(reg, val, num);
	if (ret < 0) {
		if (reg != SCP_PROTOCOL_ADP_TYPE0)
			scp_protocol_set_rw_error_flag(RW_ERROR_FLAG);

		hwlog_err("reg_write fail(reg=0x%x)\n", reg);
		return -1;
	}

	return 0;
}

static int scp_protocol_detect_adapter(void)
{
	struct scp_protocol_ops *l_ops = NULL;

	l_ops = scp_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->detect_adapter) {
		hwlog_err("detect_adapter is null\n");
		return -1;
	}

	hwlog_info("detect_adapter\n");

	return l_ops->detect_adapter();
}

static int scp_protocol_soft_reset_master(void)
{
	struct scp_protocol_ops *l_ops = NULL;

	l_ops = scp_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->soft_reset_master) {
		hwlog_err("soft_reset_master is null\n");
		return -1;
	}

	hwlog_info("soft_reset_master\n");

	return l_ops->soft_reset_master();
}

static int scp_protocol_soft_reset_slave(void)
{
	struct scp_protocol_ops *l_ops = NULL;

	l_ops = scp_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->soft_reset_slave) {
		hwlog_err("soft_reset_slave is null\n");
		return -1;
	}

	hwlog_info("soft_reset_slave\n");

	return l_ops->soft_reset_slave();
}

static int scp_protocol_process_pre_init(void)
{
	struct scp_protocol_ops *l_ops = NULL;

	l_ops = scp_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->pre_init)
		return 0;

	hwlog_info("process_pre_init\n");

	return l_ops->pre_init();
}

static int scp_protocol_process_post_init(void)
{
	struct scp_protocol_ops *l_ops = NULL;

	l_ops = scp_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->post_init)
		return 0;

	hwlog_info("process_post_init\n");

	return l_ops->post_init();
}

static int scp_protocol_process_pre_exit(void)
{
	struct scp_protocol_ops *l_ops = NULL;

	l_ops = scp_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->pre_exit)
		return 0;

	hwlog_info("process_pre_exit\n");

	return l_ops->pre_exit();
}

static int scp_protocol_process_post_exit(void)
{
	struct scp_protocol_ops *l_ops = NULL;

	l_ops = scp_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->post_exit)
		return 0;

	hwlog_info("process_post_exit\n");

	return l_ops->post_exit();
}

static int scp_protocol_set_default_param(void)
{
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	memset(&l_dev->info, 0, sizeof(l_dev->info));
	memset(l_dev->encrypt_random_host, 0,
		sizeof(l_dev->encrypt_random_host));
	memset(l_dev->encrypt_random_slave, 0,
		sizeof(l_dev->encrypt_random_slave));
	memset(l_dev->encrypt_hash_slave, 0,
		sizeof(l_dev->encrypt_hash_slave));

	return 0;
}

static int scp_protocol_get_vendor_id(int *id)
{
	int value[BYTE_TWO] = {0};
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (l_dev->info.vid_rd_flag == HAS_READ_FLAG) {
		*id = ((l_dev->info.vid_h << SCP_PROTOCOL_BYTE_BITS) |
			l_dev->info.vid_l);
		hwlog_info("get_vendor_id_a: 0x%x\n", *id);
		return 0;
	}

	/* reg: 0x82 & 0x83 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_VENDOR_ID_HBYTE,
		value, BYTE_TWO))
		return -1;

	*id = ((value[0] << SCP_PROTOCOL_BYTE_BITS) | value[1]);
	l_dev->info.vid_rd_flag = HAS_READ_FLAG;
	l_dev->info.vid_h = value[0];
	l_dev->info.vid_l = value[1];

	hwlog_info("get_vendor_id_f: 0x%x\n", *id);
	return 0;
}

static int scp_protocol_get_module_id(int *id)
{
	int value[BYTE_TWO] = {0};
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (l_dev->info.mid_rd_flag == HAS_READ_FLAG) {
		*id = ((l_dev->info.mid_h << SCP_PROTOCOL_BYTE_BITS) |
			l_dev->info.mid_l);
		hwlog_info("get_module_id_a: 0x%x\n", *id);
		return 0;
	}

	/* reg: 0x84 & 0x85 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_MODULE_ID_HBYTE,
		value, BYTE_TWO))
		return -1;

	*id = ((value[0] << SCP_PROTOCOL_BYTE_BITS) | value[1]);
	l_dev->info.mid_rd_flag = HAS_READ_FLAG;
	l_dev->info.mid_h = value[0];
	l_dev->info.mid_l = value[1];

	hwlog_info("get_module_id_f: 0x%x\n", *id);
	return 0;
}

static int scp_protocol_get_serial_no_id(int *id)
{
	int value[BYTE_TWO] = {0};
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (l_dev->info.serial_rd_flag == HAS_READ_FLAG) {
		*id = ((l_dev->info.serial_h << SCP_PROTOCOL_BYTE_BITS) |
			l_dev->info.serial_l);
		hwlog_info("get_serial_no_id_a: 0x%x\n", *id);
		return 0;
	}

	/* reg: 0x86 & 0x87 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_SERIAL_NO_HBYTE,
		value, BYTE_TWO))
		return -1;

	*id = ((value[0] << SCP_PROTOCOL_BYTE_BITS) | value[1]);
	l_dev->info.serial_rd_flag = HAS_READ_FLAG;
	l_dev->info.serial_h = value[0];
	l_dev->info.serial_l = value[1];

	hwlog_info("get_serial_no_id_f: 0x%x\n", *id);
	return 0;
}

static int scp_protocol_get_chip_id(int *id)
{
	int value[BYTE_ONE] = {0};
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (l_dev->info.chip_id_rd_flag == HAS_READ_FLAG) {
		*id = l_dev->info.chip_id;
		hwlog_info("get_chip_id_a: 0x%x\n", *id);
		return 0;
	}

	/* reg: 0x88 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_CHIP_ID, value, BYTE_ONE))
		return -1;

	*id = value[0];
	l_dev->info.chip_id_rd_flag = HAS_READ_FLAG;
	l_dev->info.chip_id = value[0];

	hwlog_info("get_chip_id_f: 0x%x\n", *id);
	return 0;
}

static int scp_protocol_get_hw_version_id(int *id)
{
	int value[BYTE_ONE] = {0};
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (l_dev->info.hwver_rd_flag == HAS_READ_FLAG) {
		*id = l_dev->info.hwver;
		hwlog_info("get_hw_version_id_a: %x\n", *id);
		return 0;
	}

	/* reg: 0x89 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_HWVER, value, BYTE_ONE))
		return -1;

	*id = value[0];
	l_dev->info.hwver_rd_flag = HAS_READ_FLAG;
	l_dev->info.hwver = value[0];

	hwlog_info("get_hw_version_id_f: 0x%x\n", *id);
	return 0;
}

static int scp_protocol_get_fw_version_id(int *id)
{
	int value[BYTE_TWO] = {0};
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (l_dev->info.fwver_rd_flag == HAS_READ_FLAG) {
		*id = ((l_dev->info.fwver_h << SCP_PROTOCOL_BYTE_BITS) |
			l_dev->info.fwver_l);
		hwlog_info("get_fw_version_id_a: 0x%x\n", *id);
		return 0;
	}

	/* reg: 0x8a & 0x8b */
	if (scp_protocol_reg_read(SCP_PROTOCOL_FWVER_HBYTE, value, BYTE_TWO))
		return -1;

	*id = ((value[0] << SCP_PROTOCOL_BYTE_BITS) | value[1]);
	l_dev->info.fwver_rd_flag = HAS_READ_FLAG;
	l_dev->info.fwver_h = value[0];
	l_dev->info.fwver_l = value[1];

	hwlog_info("get_fw_version_id_f: 0x%x\n", *id);
	return 0;
}

static int scp_protocol_get_adp_type(int *type)
{
	int value[BYTE_ONE] = {0};
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev || !type)
		return -1;

	if (l_dev->info.adp_b_type1_rd_falg == HAS_READ_FLAG) {
		*type = l_dev->info.adp_b_type1;
		hwlog_info("get_adp_type_a: %d\n", *type);
		return 0;
	}

	/* reg: 0x8d */
	if (scp_protocol_reg_read(SCP_PROTOCOL_ADP_B_TYPE1, value, BYTE_ONE))
		return -1;

	*type = value[0];
	l_dev->info.adp_b_type1 = value[0];
	l_dev->info.adp_b_type1_rd_falg = HAS_READ_FLAG;

	hwlog_info("get_adp_type_f: %d\n", *type);
	return 0;
}

static int scp_protocol_get_power_drop_info(int *drop_flag, int *drop_ratio)
{
	int value[BYTE_ONE] = {0};
	int value_a;
	int value_b;

	/* reg: 0xa5 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_SSTS, value, BYTE_ONE))
		return -1;

	hwlog_info("ssts[%x]=%x\n", SCP_PROTOCOL_SSTS, value[0]);

	value_a = ((value[0] & SCP_PROTOCOL_SSTS_DPARTO_MASK) >>
		SCP_PROTOCOL_SSTS_DPARTO_SHIFT);
	value_b = ((value[0] & SCP_PROTOCOL_SSTS_DROP_MASK) >>
		SCP_PROTOCOL_SSTS_DROP_SHIFT);

	*drop_flag = value_b;
	*drop_ratio = value_a;

	hwlog_info("get_power_drop_info: %d,%d\n", *drop_flag, *drop_ratio);
	return 0;
}

static int scp_protocol_get_min_voltage(int *volt)
{
	int value[BYTE_ONE] = {0};
	int value_a;
	int value_b;
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (l_dev->info.min_volt_rd_flag == HAS_READ_FLAG) {
		*volt = l_dev->info.min_volt;
		hwlog_info("get_min_voltage_a: %d\n", *volt);
		return 0;
	}

	/* reg: 0x92 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_MIN_VOUT, value, BYTE_ONE))
		return -1;

	hwlog_info("min_vout[%x]=%x\n", SCP_PROTOCOL_MIN_VOUT, value[0]);

	value_a = ((value[0] & SCP_PROTOCOL_VOUT_A_MASK) >>
		SCP_PROTOCOL_VOUT_A_SHIFT);
	value_b = ((value[0] & SCP_PROTOCOL_VOUT_B_MASK) >>
		SCP_PROTOCOL_VOUT_B_SHIFT);

	if (value_a < SCP_PROTOCOL_VOUT_A_0 ||
		value_a > SCP_PROTOCOL_VOUT_A_3) {
		hwlog_err("invalid value_a(%d)\n", value_a);
		return -1;
	}

	*volt = (scp_protocol_ten_power[value_a] * value_b);
	l_dev->info.min_volt_rd_flag = HAS_READ_FLAG;
	l_dev->info.min_volt = *volt;

	hwlog_info("get_min_voltage_f: %d\n", *volt);
	return 0;
}

static int scp_protocol_get_max_voltage(int *volt)
{
	int value[BYTE_ONE] = {0};
	int value_a;
	int value_b;
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (l_dev->info.max_volt_rd_flag == HAS_READ_FLAG) {
		*volt = l_dev->info.max_volt;
		hwlog_info("get_max_voltage_a: %d\n", *volt);
		return 0;
	}

	/* reg: 0x93 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_MAX_VOUT, value, BYTE_ONE))
		return -1;

	hwlog_info("max_vout[%x]=%x\n", SCP_PROTOCOL_MAX_VOUT, value[0]);

	value_a = ((value[0] & SCP_PROTOCOL_VOUT_A_MASK) >>
		SCP_PROTOCOL_VOUT_A_SHIFT);
	value_b = ((value[0] & SCP_PROTOCOL_VOUT_B_MASK) >>
		SCP_PROTOCOL_VOUT_B_SHIFT);

	if (value_a < SCP_PROTOCOL_VOUT_A_0 ||
		value_a > SCP_PROTOCOL_VOUT_A_3) {
		hwlog_err("invalid value_a(%d)\n", value_a);
		return -1;
	}

	*volt = (scp_protocol_ten_power[value_a] * value_b);
	l_dev->info.max_volt_rd_flag = HAS_READ_FLAG;
	l_dev->info.max_volt = *volt;

	hwlog_info("get_max_voltage_f: %d\n", *volt);
	return 0;
}

static int scp_protocol_get_min_current(int *cur)
{
	int value[BYTE_ONE] = {0};
	int value_a;
	int value_b;
	int drop_flag;
	int drop_ratio;
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (l_dev->info.min_cur_rd_flag == HAS_READ_FLAG) {
		*cur = l_dev->info.min_cur;
		hwlog_info("get_min_current_a: %d\n", *cur);
		return 0;
	}

	if (scp_protocol_get_power_drop_info(&drop_flag, &drop_ratio))
		return -1;

	/* reg: 0x94 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_MIN_IOUT, value, BYTE_ONE))
		return -1;

	hwlog_info("min_iout[%x]=%x\n", SCP_PROTOCOL_MIN_IOUT, value[0]);

	value_a = ((value[0] & SCP_PROTOCOL_IOUT_A_MASK) >>
		SCP_PROTOCOL_IOUT_A_SHIFT);
	value_b = ((value[0] & SCP_PROTOCOL_IOUT_B_MASK) >>
		SCP_PROTOCOL_IOUT_B_SHIFT);

	if (value_a < SCP_PROTOCOL_IOUT_A_0 ||
		value_a > SCP_PROTOCOL_IOUT_A_3) {
		hwlog_err("invalid value_a(%d)\n", value_a);
		return -1;
	}

	if (drop_flag == SCP_PROTOCOL_DROP_ENABLE)
		*cur = (scp_protocol_ten_power[value_a] * value_b) *
			drop_ratio / SCP_PROTOCOL_DROP_FACTOR;
	else
		*cur = (scp_protocol_ten_power[value_a] * value_b);

	l_dev->info.min_cur_rd_flag = HAS_READ_FLAG;
	l_dev->info.min_cur = *cur;

	hwlog_info("get_min_current_f: %d\n", *cur);
	return 0;
}

static int scp_protocol_get_max_current(int *cur)
{
	int value[BYTE_ONE] = {0};
	int value_a;
	int value_b;
	int drop_flag;
	int drop_ratio;
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (l_dev->info.max_cur_rd_flag == HAS_READ_FLAG) {
		*cur = l_dev->info.max_cur;
		hwlog_info("get_max_current_a: %d\n", *cur);
		return 0;
	}

	if (scp_protocol_get_power_drop_info(&drop_flag, &drop_ratio))
		return -1;

	/* reg: 0x95 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_MAX_IOUT, value, BYTE_ONE))
		return -1;

	hwlog_info("max_iout[%x]=%x\n", SCP_PROTOCOL_MAX_IOUT, value[0]);

	value_a = ((value[0] & SCP_PROTOCOL_IOUT_A_MASK) >>
		SCP_PROTOCOL_IOUT_A_SHIFT);
	value_b = ((value[0] & SCP_PROTOCOL_IOUT_B_MASK) >>
		SCP_PROTOCOL_IOUT_B_SHIFT);

	if (value_a < SCP_PROTOCOL_IOUT_A_0 ||
		value_a > SCP_PROTOCOL_IOUT_A_3) {
		hwlog_err("invalid value_a(%d)\n", value_a);
		return -1;
	}

	if (drop_flag == SCP_PROTOCOL_DROP_ENABLE)
		*cur = (scp_protocol_ten_power[value_a] * value_b) *
			drop_ratio / SCP_PROTOCOL_DROP_FACTOR;
	else
		*cur = (scp_protocol_ten_power[value_a] * value_b);

	l_dev->info.max_cur_rd_flag = HAS_READ_FLAG;
	l_dev->info.max_cur = *cur;

	hwlog_info("get_max_current_f: %d\n", *cur);
	return 0;
}

static int scp_protocol_detect_adapter_support_mode_by_0x7e(int *mode)
{
	int value[BYTE_ONE] = {0};
	int ret = SCP_DETECT_FAIL;

	/* reg: 0x7e */
	if (scp_protocol_reg_read(SCP_PROTOCOL_ADP_TYPE0, value, BYTE_ONE)) {
		hwlog_err("read adp_type0(0x7e) fail\n");
		return ret;
	}

	hwlog_info("adp_type0[%x]=%x\n", SCP_PROTOCOL_ADP_TYPE0, value[0]);

	if (value[0] & SCP_PROTOCOL_ADP_TYPE0_AB_MASK) {
		/* b type: regulable high voltage adapter */
		if (value[0] & SCP_PROTOCOL_ADP_TYPE0_B_SC_MASK) {
			*mode |= ADAPTER_SUPPORT_SCP_B_SC;
			ret = SCP_DETECT_SUCC;
		}

		/* b type: regulable low voltage adapter */
		if (!(value[0] & SCP_PROTOCOL_ADP_TYPE0_B_LVC_MASK)) {
			*mode |= ADAPTER_SUPPORT_SCP_B_LVC;
			ret = SCP_DETECT_SUCC;
		}

		hwlog_info("scp type_b detected(0x7e), support mode: 0x%x\n",
			*mode);
	}

	return ret;
}

static int scp_protocol_detect_adapter_support_mode_by_0x80(int *mode)
{
	int value_a[BYTE_ONE] = {0};
	int value_b[BYTE_ONE] = {0};
	/* int ret = SCP_DETECT_FAIL; */

	/* reg: 0x80 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_ADP_TYPE1, value_a, BYTE_ONE)) {
		hwlog_err("read adp_type1(0x80) fail\n");
		return SCP_DETECT_OTHER;
	}

	hwlog_info("adp_type1[%x]=%x\n", SCP_PROTOCOL_ADP_TYPE1, value_a[0]);

	if (value_a[0] & SCP_PROTOCOL_ADP_TYPE1_B_MASK) {
		hwlog_info("scp type_b detected(0x80)\n");

		/* reg: 0x81 */
		if (scp_protocol_reg_read(SCP_PROTOCOL_B_ADP_TYPE,
			value_b, BYTE_ONE)) {
			hwlog_err("read b_adp_type(0x81) fail\n");
			return SCP_DETECT_OTHER;
		}

		hwlog_info("b_adp_type[%x]=%x\n",
			SCP_PROTOCOL_B_ADP_TYPE, value_b[0]);

		if (value_b[0] == SCP_PROTOCOL_B_ADP_TYPE_B_MASK) {
			*mode |= ADAPTER_SUPPORT_SCP_B_LVC;
			hwlog_info("scp type_b detected(0x81), support mode: 0x%x\n",
				*mode);

			return SCP_DETECT_SUCC;
		}
	}

	return SCP_DETECT_OTHER;
}

static int scp_protocol_detect_adapter_support_mode(int *mode)
{
	int ret;
	int support_mode = 0;
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return ADAPTER_DETECT_OTHER;

	/*  set all parameter to default state */
	scp_protocol_set_default_param();

	l_dev->info.support_mode = ADAPTER_SUPPORT_UNDEFINED;

	/* protocol handshark: detect scp adapter */
	ret = scp_protocol_detect_adapter();
	if (ret == SCP_DETECT_OTHER) {
		hwlog_err("scp adapter detect other\n");
		return ADAPTER_DETECT_OTHER;
	}
	if (ret == SCP_DETECT_FAIL) {
		hwlog_err("scp adapter detect fail\n");
		return ADAPTER_DETECT_FAIL;
	}

	/* adapter type detect: 0x7e */
	ret = scp_protocol_detect_adapter_support_mode_by_0x7e(&support_mode);
	if (ret == SCP_DETECT_SUCC) {
		*mode = support_mode;
		l_dev->info.support_mode = support_mode;

		hwlog_info("scp adapter type_b detect sucess(judge by 0x7e)\n");
		return ADAPTER_DETECT_SUCC;
	}

	/* adapter type detect: 0x80 */
	ret = scp_protocol_detect_adapter_support_mode_by_0x80(&support_mode);
	if (ret == SCP_DETECT_SUCC) {
		*mode = support_mode;
		l_dev->info.support_mode = support_mode;

		hwlog_info("scp adapter type_b detect sucess(judge by 0x80)\n");
		return ADAPTER_DETECT_SUCC;
	}

	hwlog_err("detect_adapter_type fail\n");
	return ADAPTER_DETECT_OTHER;
}

static int scp_protocol_get_support_mode(int *mode)
{
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	*mode = l_dev->info.support_mode;

	hwlog_info("get_support_mode\n");
	return 0;
}

static int scp_protocol_get_device_info(struct adapter_info *info)
{
	if (scp_protocol_get_vendor_id(&info->vendor_id))
		return -1;

	if (scp_protocol_get_module_id(&info->module_id))
		return -1;

	if (scp_protocol_get_serial_no_id(&info->serial_no))
		return -1;

	if (scp_protocol_get_chip_id(&info->chip_id))
		return -1;

	if (scp_protocol_get_hw_version_id(&info->hwver))
		return -1;

	if (scp_protocol_get_fw_version_id(&info->fwver))
		return -1;

	if (scp_protocol_get_min_voltage(&info->min_volt))
		return -1;

	if (scp_protocol_get_max_voltage(&info->max_volt))
		return -1;

	if (scp_protocol_get_min_current(&info->min_cur))
		return -1;

	if (scp_protocol_get_max_current(&info->max_cur))
		return -1;

	hwlog_info("get_device_info\n");
	return 0;
}

static int scp_protocol_get_chip_vendor_id(int *id)
{
	int chip_id = 0;
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (scp_protocol_get_chip_id(&chip_id))
		return -1;

	switch (chip_id) {
	case SCP_PROTOCOL_CHIP_ID_RICHTEK:
		hwlog_info("adapter chip is richtek\n");
		*id = ADAPTER_CHIP_RICHTEK;
		break;

	case SCP_PROTOCOL_CHIP_ID_WELTREND:
		hwlog_info("adapter chip is weltrend\n");
		*id = ADAPTER_CHIP_WELTREND;
		break;

	case SCP_PROTOCOL_CHIP_ID_IWATT:
		hwlog_info("adapter chip is iwatt\n");
		*id = ADAPTER_CHIP_IWATT;
		break;

	case SCP_PROTOCOL_CHIP_ID_0X32:
		hwlog_info("adapter chip is 0x32\n");
		*id = ADAPTER_CHIP_ID0X32;
		break;

	default:
		hwlog_err("invalid adaptor chip id\n");
		*id = chip_id;
		break;
	}

	l_dev->info.chip_vid = *id;

	hwlog_info("get_chip_vendor_id: %d\n", *id);
	return 0;
}

static int scp_protocol_set_output_mode(int enable)
{
	int value[BYTE_ONE] = {0};
	int tmp_value;

	/* reg: 0xa0 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_CTRL_BYTE0, value, BYTE_ONE))
		return -1;

	tmp_value = (enable ? SCP_PROTOCOL_OUTPUT_MODE_ENABLE :
		SCP_PROTOCOL_OUTPUT_MODE_DISABLE);
	value[0] &= ~(SCP_PROTOCOL_OUTPUT_MODE_MASK);
	value[0] |= ((tmp_value << SCP_PROTOCOL_OUTPUT_MODE_SHIFT) &
		SCP_PROTOCOL_OUTPUT_MODE_MASK);

	/* reg: 0xa0 */
	if (scp_protocol_reg_write(SCP_PROTOCOL_CTRL_BYTE0, value, BYTE_ONE))
		return -1;

	hwlog_info("set_output_mode: %d,%x\n", enable, value[0]);
	return 0;
}

static int scp_protocol_set_reset(int enable)
{
	int value[BYTE_ONE] = {0};
	int tmp_value;

	/* reg: 0xa0 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_CTRL_BYTE0, value, BYTE_ONE))
		return -1;

	tmp_value = (enable ? SCP_PROTOCOL_RESET_ENABLE :
		SCP_PROTOCOL_RESET_DISABLE);
	value[0] &= ~(SCP_PROTOCOL_RESET_MASK);
	value[0] |= ((tmp_value << SCP_PROTOCOL_RESET_SHIFT) &
		SCP_PROTOCOL_RESET_MASK);

	/* reg: 0xa0 */
	if (scp_protocol_reg_write(SCP_PROTOCOL_CTRL_BYTE0, value, BYTE_ONE))
		return -1;

	hwlog_info("set_reset: %d,%x\n", enable, value[0]);
	return 0;
}

static int scp_protocol_set_output_enable(int enable)
{
	int value[BYTE_ONE] = {0};
	int tmp_value;

	if (scp_protocol_set_output_mode(SCP_PROTOCOL_OUTPUT_MODE_ENABLE))
		return -1;

	/* reg: 0xa0 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_CTRL_BYTE0, value, BYTE_ONE))
		return -1;

	tmp_value = (enable ? SCP_PROTOCOL_OUTPUT_ENABLE :
		SCP_PROTOCOL_OUTPUT_DISABLE);
	value[0] &= ~(SCP_PROTOCOL_OUTPUT_EN_MASK);
	value[0] |= ((tmp_value << SCP_PROTOCOL_OUTPUT_EN_SHIFT) &
		SCP_PROTOCOL_OUTPUT_EN_MASK);

	/* reg: 0xa0 */
	if (scp_protocol_reg_write(SCP_PROTOCOL_CTRL_BYTE0, value, BYTE_ONE))
		return -1;

	hwlog_info("set_output_enable: %d,%x\n", enable, value[0]);
	return 0;
}

static int scp_protocol_set_dp_delitch(int time)
{
	int value[BYTE_ONE] = {0};
	int tmp_value;

	/* reg: 0xa1 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_CTRL_BYTE1, value, BYTE_ONE))
		return -1;

	tmp_value = (time);
	value[0] &= ~(SCP_PROTOCOL_DP_DELITCH_MASK);
	value[0] |= ((tmp_value << SCP_PROTOCOL_DP_DELITCH_SHIFT) &
		SCP_PROTOCOL_DP_DELITCH_MASK);

	/* reg: 0xa1 */
	if (scp_protocol_reg_write(SCP_PROTOCOL_CTRL_BYTE1, value, BYTE_ONE))
		return -1;

	hwlog_info("set_dp_delitch: %d,%x\n", time, value[0]);
	return 0;
}

static int scp_protocol_set_watchdog_timer(int second)
{
	int value[BYTE_ONE] = {0};
	int tmp_value;

	/* reg: 0xa1 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_CTRL_BYTE1, value, BYTE_ONE))
		return -1;

	tmp_value = (second * SCP_PROTOCOL_WATCHDOG_BITS_UNIT);
	value[0] &= ~(SCP_PROTOCOL_WATCHDOG_MASK);
	value[0] |= ((tmp_value << SCP_PROTOCOL_WATCHDOG_SHIFT) &
		SCP_PROTOCOL_WATCHDOG_MASK);

	/* reg: 0xa1 */
	if (scp_protocol_reg_write(SCP_PROTOCOL_CTRL_BYTE1, value, BYTE_ONE))
		return -1;

	hwlog_info("set_watchdog_timer: %d,%x\n", second, value[0]);
	return 0;
}

static int scp_protocol_config_vset_boundary(int vboundary)
{
	int value[BYTE_TWO] = {0};
	int tmp_value;

	tmp_value = (vboundary / SCP_PROTOCOL_VSET_BOUNDARY_STEP);
	value[0] = ((tmp_value >> SCP_PROTOCOL_BYTE_BITS) &
		SCP_PROTOCOL_BYTE_MASK);
	value[1] = (tmp_value & SCP_PROTOCOL_BYTE_MASK);

	/* reg: 0xb0 & 0xb1 */
	if (scp_protocol_reg_write(SCP_PROTOCOL_VSET_BOUNDARY_HBYTE,
		value, BYTE_TWO))
		return -1;

	hwlog_info("config_vset_boundary: %d\n", vboundary);
	return 0;
}

static int scp_protocol_config_iset_boundary(int iboundary)
{
	int value[BYTE_TWO] = {0};
	int tmp_value;

	tmp_value = (iboundary / SCP_PROTOCOL_ISET_BOUNDARYSTEP);
	value[0] = ((tmp_value >> SCP_PROTOCOL_BYTE_BITS) &
		SCP_PROTOCOL_BYTE_MASK);
	value[1] = (tmp_value & SCP_PROTOCOL_BYTE_MASK);

	/* reg: 0xb2 & 0xb3 */
	if (scp_protocol_reg_write(SCP_PROTOCOL_ISET_BOUNDARY_HBYTE,
		value, BYTE_TWO))
		return -1;

	hwlog_info("config_iset_boundary: %d\n", iboundary);
	return 0;
}

/* single byte */
static int scp_protocol_set_output_voltage_s(int volt)
{
	int value[BYTE_ONE] = {0};
	int tmp_value;

	tmp_value = (volt - SCP_PROTOCOL_VSSET_OFFSET);
	value[0] = (tmp_value / SCP_PROTOCOL_VSSET_STEP);

	/* reg: 0xca */
	if (scp_protocol_reg_write(SCP_PROTOCOL_VSSET, value, BYTE_ONE))
		return -1;

	hwlog_info("set_output_voltage_s: %d\n", volt);
	return 0;
}

/* double byte */
static int scp_protocol_set_output_voltage_d(int volt)
{
	int value[BYTE_TWO] = {0};
	int tmp_value;

	tmp_value = (volt / SCP_PROTOCOL_VSET_STEP);
	value[0] = ((tmp_value >> SCP_PROTOCOL_BYTE_BITS) &
		SCP_PROTOCOL_BYTE_MASK);
	value[1] = (tmp_value & SCP_PROTOCOL_BYTE_MASK);

	/* reg: 0xb8 & 0xb9 */
	if (scp_protocol_reg_write(SCP_PROTOCOL_VSET_HBYTE, value, BYTE_TWO))
		return -1;

	hwlog_info("set_output_voltage_d: %d\n", volt);
	return 0;
}

static int scp_protocol_set_output_voltage(int volt)
{
	if (volt > SCP_PROTOCOL_VSSET_MAX_VOLT)
		return scp_protocol_set_output_voltage_d(volt);
	else
		return scp_protocol_set_output_voltage_s(volt);
}

/* single byte */
static int scp_protocol_get_output_voltage_s(int *volt)
{
	int value[BYTE_ONE] = {0};
	int tmp_value;

	/* reg: 0xc8 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_SREAD_VOUT, value, BYTE_ONE))
		return -1;

	tmp_value = (value[0] * SCP_PROTOCOL_SREAD_VOUT_STEP);
	*volt = (tmp_value + SCP_PROTOCOL_SREAD_VOUT_OFFSET);

	hwlog_info("get_output_voltage_s: %d\n", *volt);
	return 0;
}

/* double byte */
static int scp_protocol_get_output_voltage_d(int *volt)
{
	int value[BYTE_TWO] = {0};
	int tmp_value;

	/* reg: 0xa8 & 0xa9 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_READ_VOUT_HBYTE,
		value, BYTE_TWO))
		return -1;

	tmp_value = ((value[0] << SCP_PROTOCOL_BYTE_BITS) | value[1]);
	*volt = (tmp_value * SCP_PROTOCOL_READ_VOUT_STEP);

	hwlog_info("get_output_voltage_d: %d\n", *volt);
	return 0;
}

static int scp_protocol_get_output_voltage(int *volt)
{
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (l_dev->info.support_mode & ADAPTER_SUPPORT_SCP_B_SC)
		return scp_protocol_get_output_voltage_d(volt);
	else
		return scp_protocol_get_output_voltage_s(volt);
}

/* single byte */
static int scp_protocol_set_output_current_s(int cur)
{
	int value[BYTE_ONE] = {0};

	value[0] = (cur / SCP_PROTOCOL_ISSET_STEP);

	/* reg: 0xcb */
	if (scp_protocol_reg_write(SCP_PROTOCOL_ISSET, value, BYTE_ONE))
		return -1;

	hwlog_info("set_output_current_s: %d\n", cur);
	return 0;
}

/* double byte */
#ifdef POWER_MODULE_DEBUG_FUNCTION
static int scp_protocol_set_output_current_d(int cur)
{
	int value[BYTE_TWO] = {0};
	int tmp_value;

	tmp_value = (cur / SCP_PROTOCOL_ISET_STEP);
	value[0] = ((tmp_value >> SCP_PROTOCOL_BYTE_BITS) &
		SCP_PROTOCOL_BYTE_MASK);
	value[1] = (tmp_value & SCP_PROTOCOL_BYTE_MASK);

	/* reg: 0xba & 0xbb */
	if (scp_protocol_reg_write(SCP_PROTOCOL_ISET_HBYTE, value, BYTE_TWO))
		return -1;

	hwlog_info("set_output_current_d: %d\n", cur);
	return 0;
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

static int scp_protocol_set_output_current(int cur)
{
	return scp_protocol_set_output_current_s(cur);
}

/* single byte */
static int scp_protocol_get_output_current_s(int *cur)
{
	int value[BYTE_ONE] = {0};

	/* reg: 0xc9 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_SREAD_IOUT, value, BYTE_ONE))
		return -1;

	*cur = value[0] * SCP_PROTOCOL_SREAD_IOUT_STEP;

	hwlog_info("get_output_current_s: %d\n", *cur);
	return 0;
}

/* double byte */
#ifdef POWER_MODULE_DEBUG_FUNCTION
static int scp_protocol_get_output_current_d(int *cur)
{
	int value[BYTE_TWO] = {0};
	int tmp_value;

	/* reg: 0xaa & 0xab */
	if (scp_protocol_reg_read(SCP_PROTOCOL_READ_IOUT_HBYTE,
		value, BYTE_TWO))
		return -1;

	tmp_value = ((value[0] << SCP_PROTOCOL_BYTE_BITS) | value[1]);
	*cur = (tmp_value * SCP_PROTOCOL_READ_IOUT_STEP);

	hwlog_info("get_output_current_d: %d\n", *cur);
	return 0;
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

static int scp_protocol_get_output_current(int *cur)
{
	return scp_protocol_get_output_current_s(cur);
}

/* single byte */
static int scp_protocol_get_output_current_set_s(int *cur)
{
	int value[BYTE_ONE] = {0};

	/* reg: 0xcb */
	if (scp_protocol_reg_read(SCP_PROTOCOL_ISSET, value, BYTE_ONE))
		return -1;

	*cur = value[0] * SCP_PROTOCOL_ISSET_STEP;

	hwlog_info("get_output_current_set_s: %d\n", *cur);
	return 0;
}

/* double byte */
#ifdef POWER_MODULE_DEBUG_FUNCTION
static int scp_protocol_get_output_current_set_d(int *cur)
{
	int value[BYTE_TWO] = {0};
	int tmp_value;

	/* reg: 0xba & 0xbb */
	if (scp_protocol_reg_read(SCP_PROTOCOL_ISET_HBYTE, value, BYTE_TWO))
		return -1;

	tmp_value = ((value[0] << SCP_PROTOCOL_BYTE_BITS) | value[1]);
	*cur = (tmp_value * SCP_PROTOCOL_ISET_STEP);

	hwlog_info("get_output_current_set_d: %d\n", *cur);
	return 0;
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

static int scp_protocol_get_output_current_set(int *cur)
{
	return scp_protocol_get_output_current_set_s(cur);
}

static int scp_protocol_get_inside_temp(int *temp)
{
	int value[BYTE_ONE] = {0};

	/* reg: 0xa6 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_INSIDE_TMP, value, BYTE_ONE))
		return -1;

	*temp = value[0] * SCP_PROTOCOL_INSIDE_TMP_UNIT;

	hwlog_info("get_inside_temp: %d\n", *temp);
	return 0;
}

static int scp_protocol_get_port_temp(int *temp)
{
	int value[BYTE_ONE] = {0};

	/* reg: 0xa7 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_PORT_TMP, value, BYTE_ONE))
		return -1;

	*temp = value[0] * SCP_PROTOCOL_PORT_TMP_UNIT;

	hwlog_info("get_port_temp: %d\n", *temp);
	return 0;
}

static int scp_protocol_get_port_leakage_cur_flag(int *flag)
{
	int value[BYTE_ONE] = {0};

	/* reg: 0xa2 */
	if (scp_protocol_reg_read(SCP_PROTOCOL_STATUS_BYTE0, value, BYTE_ONE))
		return -1;

	if ((value[0] & SCP_PROTOCOL_LEAKAGE_FLAG_MASK) >>
		SCP_PROTOCOL_LEAKAGE_FLAG_SHIFT)
		*flag = SCP_PROTOCOL_PORT_LEAKAGE;
	else
		*flag = SCP_PROTOCOL_PORT_NOT_LEAKAGE;

	hwlog_info("get_port_leakage_current_flag: %d\n", *flag);
	return 0;
}

static int scp_protocol_set_encrypt_index(int retrys, int index)
{
	int i;
	int value[BYTE_ONE] = {0};

	value[0] = index;

	/* reg: 0xce */
	for (i = 0; i < retrys; i++) {
		if (scp_protocol_reg_write(SCP_PROTOCOL_KEY_INDEX,
			value, BYTE_ONE) == 0)
			break;
	}

	if (i >= retrys) {
		hwlog_err("set_encrypt_index failed\n");
		return -1;
	}

	hwlog_info("set_encrypt_index: %d\n", index);
	return 0;
}

static int scp_protocol_get_encrypt_enable(int retrys, int *flag)
{
	int i;
	int value[BYTE_ONE] = {0};

	/* reg: 0xcf */
	for (i = 0; i < retrys; i++) {
		if (scp_protocol_reg_read(SCP_PROTOCOL_ENCRYPT_INFO,
			value, BYTE_ONE) == 0)
			break;
	}

	if (i >= retrys) {
		hwlog_err("get_encrypt_enable failed\n");
		return -1;
	}

	if ((value[0] & SCP_PROTOCOL_ENCRYPT_ENABLE_MASK) >>
		SCP_PROTOCOL_ENCRYPT_ENABLE_SHIFT)
		*flag = SCP_PROTOCOL_ENCRYPT_ENABLE;
	else
		*flag = SCP_PROTOCOL_ENCRYPT_DISABLE;

	hwlog_info("get_encrypt_enable: %d\n", *flag);
	return 0;
}

static int scp_protocol_get_encrypt_completed(int retrys, int *flag)
{
	int i;
	int value[BYTE_ONE] = {0};

	/* reg: 0xcf */
	for (i = 0; i < retrys; i++) {
		if (scp_protocol_reg_read(SCP_PROTOCOL_ENCRYPT_INFO,
			value, BYTE_ONE) == 0)
			break;
	}

	if (i >= retrys) {
		hwlog_err("get_encrypt_completed failed\n");
		return -1;
	}

	if ((value[0] & SCP_PROTOCOL_ENCRYPT_COMPLETED_MASK) >>
		SCP_PROTOCOL_ENCRYPT_COMPLETED_SHIFT)
		*flag = SCP_PROTOCOL_ENCRYPT_COMPLETED;
	else
		*flag = SCP_PROTOCOL_ENCRYPT_NOT_COMPLETED;

	hwlog_info("get_encrypt_completed: %d\n", *flag);
	return 0;
}

static int scp_protocol_set_random_num(int retrys)
{
	int i;
	int value[BYTE_EIGHT] = {0};
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	for (i = 0; i < BYTE_EIGHT; i++) {
		get_random_bytes(&value[i], sizeof(unsigned char));
		l_dev->encrypt_random_host[i] = value[i];
	}

	/* reg: 0xa0 */
	for (i = 0; i < retrys; i++) {
		if (scp_protocol_reg_write(SCP_PROTOCOL_ENCRYPT_RANDOM_WR_BASE,
			value, BYTE_EIGHT) == 0)
			break;
	}

	if (i >= retrys) {
		hwlog_err("get_encrypt_completed failed\n");
		return -1;
	}

	return 0;
}

static int scp_protocol_get_random_num(int retrys)
{
	int i;
	int value[BYTE_EIGHT] = {0};
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	/* reg: 0xa8 */
	for (i = 0; i < retrys; i++) {
		if (scp_protocol_reg_read(SCP_PROTOCOL_ENCRYPT_RANDOM_RD_BASE,
			value, BYTE_EIGHT) == 0)
			break;
	}

	if (i >= retrys) {
		hwlog_err("get_encrypt_completed failed\n");
		return -1;
	}

	for (i = 0; i < BYTE_EIGHT; i++)
		l_dev->encrypt_random_slave[i] = value[i];

	return 0;
}

static int scp_protocol_get_encrypted_value(int retrys)
{
	int i;
	int value[BYTE_SIXTEEN] = {0};
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	/* reg: 0xb0 */
	for (i = 0; i < retrys; i++) {
		if (scp_protocol_reg_read(SCP_PROTOCOL_ENCRYPT_HASH_RD_BASE,
			value, BYTE_SIXTEEN) == 0)
			break;
	}

	if (i >= retrys) {
		hwlog_err("get_encrypt_completed failed\n");
		return -1;
	}

	for (i = 0; i < BYTE_SIXTEEN; i++)
		l_dev->encrypt_hash_slave[i] = value[i];

	return 0;
}

static int scp_protocol_copy_hash_value(int key, unsigned char *hash, int size)
{
	int i = 0;
	int j;
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (size != (BYTE_EIGHT + BYTE_EIGHT + BYTE_SIXTEEN + 1)) {
		hwlog_err("hash_size(%d) is invalid\n", size);
		return -1;
	}

	for (j = 0; i < BYTE_EIGHT; i++, j++)
		hash[i] = l_dev->encrypt_random_host[j];

	for (j = 0; i < BYTE_EIGHT + BYTE_EIGHT; i++, j++)
		hash[i] = l_dev->encrypt_random_slave[j];

	for (j = 0; i < BYTE_EIGHT + BYTE_EIGHT + BYTE_SIXTEEN; i++, j++)
		hash[i] = l_dev->encrypt_hash_slave[j];

	hash[i] = (unsigned char)key;

	return 0;
}

static int scp_protocol_auth_encrypt_start(int key, unsigned char *hash,
	int size)
{
	int ret;
	int encrypted_flag = 0;
	int completed_flag = 0;
	int retry = RETRY_ONE;

	/* first: set key index */
	if (scp_protocol_set_encrypt_index(retry, key)) {
		ret = -1;
		goto fail_encrypt;
	}

	/* second: get encrypt enable flag */
	if (scp_protocol_get_encrypt_enable(retry, &encrypted_flag)) {
		ret = -1;
		goto fail_encrypt;
	}

	if (encrypted_flag == SCP_PROTOCOL_ENCRYPT_DISABLE) {
		ret = -1;
		goto fail_encrypt;
	}

	/* third: host set random num to slave */
	if (scp_protocol_set_random_num(retry)) {
		ret = -1;
		goto fail_encrypt;
	}

	/* fouth: get encrypt completed flag */
	if (scp_protocol_get_encrypt_completed(retry, &completed_flag)) {
		ret = -1;
		goto fail_encrypt;
	}

	if (completed_flag == SCP_PROTOCOL_ENCRYPT_NOT_COMPLETED) {
		ret = -1;
		goto fail_encrypt;
	}

	/* fifth: host get random num from slave */
	if (scp_protocol_get_random_num(retry)) {
		ret = -1;
		goto fail_encrypt;
	}

	/* sixth: host get hash num from slave */
	if (scp_protocol_get_encrypted_value(retry)) {
		ret = -1;
		goto fail_encrypt;
	}

	/* seventh: copy hash value */
	if (scp_protocol_copy_hash_value(key, hash, size)) {
		ret = -1;
		goto fail_encrypt;
	}

	hwlog_info("auth_encrypt_start\n");
	ret = 0;

fail_encrypt:
	if (scp_protocol_set_encrypt_index(retry,
		SCP_PROTOCOL_KEY_INDEX_RELEASE))
		return -1;

	return ret;
}

static int scp_protocol_auth_encrypt_release(void)
{
	hwlog_info("auth_encrypt_release\n");
	return 0;
}

static int scp_protocol_set_usbpd_enable(int enable)
{
	int value[BYTE_ONE] = {0};
	int tmp_value = 0;

	/* reg: 0xcf */
	if (scp_protocol_reg_read(SCP_PROTOCOL_USBPD_INFO, value, BYTE_ONE))
		return -1;

	tmp_value = (enable ? SCP_PROTOCOL_USBPD_ENABLE :
		SCP_PROTOCOL_USBPD_DISABLE);
	value[0] &= ~(SCP_PROTOCOL_USBPD_ENABLE_MASK);
	value[0] |= ((tmp_value << SCP_PROTOCOL_USBPD_ENABLE_SHIFT) &
		SCP_PROTOCOL_USBPD_ENABLE_MASK);

	/* reg: 0xcf */
	if (scp_protocol_reg_write(SCP_PROTOCOL_USBPD_INFO, value, BYTE_ONE))
		return -1;

	hwlog_info("set_usbpd_enable: %d,%x\n", enable, value[0]);
	return 0;
}

static int scp_protocol_set_default_state(void)
{
	int ret;
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	/* process non protocol flow */
	if (scp_protocol_process_pre_exit())
		return -1;

	ret = scp_protocol_set_output_mode(SCP_PROTOCOL_OUTPUT_MODE_DISABLE);

	switch (l_dev->info.chip_vid) {
	case ADAPTER_CHIP_IWATT:
		ret |= scp_protocol_set_reset(SCP_PROTOCOL_RESET_ENABLE);
		break;

	default:
		hwlog_info("adapter non iwatt chip\n");
		break;
	}

	msleep(RESET_TIME_50MS);

	/* process non protocol flow */
	if (scp_protocol_process_post_exit())
		return -1;

	if (ret != 0)
		hwlog_info("set_default_state fail\n");
	else
		hwlog_info("set_default_state ok\n");

	return ret;
}

static int scp_protocol_set_init_data(struct adapter_init_data *data)
{
	int value[BYTE_FOUR] = {0};
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	/* process non protocol flow */
	if (scp_protocol_process_pre_init())
		return -1;

	if (scp_protocol_set_output_mode(data->scp_mode_enable))
		return -1;

	if (scp_protocol_config_vset_boundary(data->vset_boundary))
		return -1;

	if (scp_protocol_config_iset_boundary(data->iset_boundary))
		return -1;

	if (scp_protocol_set_output_voltage(data->init_adaptor_voltage))
		return -1;

	/* fix a superswitch bug, set dp_delitch to 5ms for richtek chip */
	if ((l_dev->dev_id == DEVICE_ID_FUSB3601) &&
		(l_dev->info.chip_vid == ADAPTER_CHIP_RICHTEK)) {
		if (scp_protocol_set_dp_delitch(SCP_PROTOCOL_DP_DELITCH_5MS))
			return -1;
	}

	if (scp_protocol_set_watchdog_timer(data->watchdog_timer))
		return -1;

	/* process non protocol flow */
	if (scp_protocol_process_post_init())
		return -1;

	if (scp_protocol_reg_read(SCP_PROTOCOL_CTRL_BYTE0, value, BYTE_FOUR))
		return -1;

	hwlog_info("ctrl_byte0[%x]=%x\n",
		SCP_PROTOCOL_CTRL_BYTE0, value[0]);
	hwlog_info("ctrl_byte1[%x]=%x\n",
		SCP_PROTOCOL_CTRL_BYTE1, value[1]);
	hwlog_info("status_byte0[%x]=%x\n",
		SCP_PROTOCOL_STATUS_BYTE0, value[2]);
	hwlog_info("status_byte1[%x]=%x\n",
		SCP_PROTOCOL_STATUS_BYTE1, value[3]);

	if (scp_protocol_reg_read(SCP_PROTOCOL_VSET_BOUNDARY_HBYTE,
		value, BYTE_FOUR))
		return -1;

	hwlog_info("vset_boundary[%x]=%x\n",
		SCP_PROTOCOL_VSET_BOUNDARY_HBYTE, value[0]);
	hwlog_info("vset_boundary[%x]=%x\n",
		SCP_PROTOCOL_VSET_BOUNDARY_LBYTE, value[1]);
	hwlog_info("iset_boundary[%x]=%x\n",
		SCP_PROTOCOL_ISET_BOUNDARY_HBYTE, value[2]);
	hwlog_info("iset_boundary[%x]=%x\n",
		SCP_PROTOCOL_ISET_BOUNDARY_LBYTE, value[3]);

	hwlog_info("set_init_data\n");
	return 0;
}

static int scp_protocol_get_protocol_register_state(void)
{
	struct scp_protocol_dev *l_dev = NULL;

	l_dev = scp_protocol_get_dev();
	if (!l_dev)
		return -1;

	if ((l_dev->dev_id >= DEVICE_ID_BEGIN) &&
		(l_dev->dev_id < DEVICE_ID_END))
		return 0;

	hwlog_info("get_protocol_register_state fail\n");
	return -1;
}

struct adapter_protocol_ops adapter_protocol_scp_ops = {
	.type_name = "hw_scp",

	.soft_reset_master = scp_protocol_soft_reset_master,
	.soft_reset_slave = scp_protocol_soft_reset_slave,
	.detect_adapter_support_mode = scp_protocol_detect_adapter_support_mode,
	.get_support_mode = scp_protocol_get_support_mode,
	.get_device_info = scp_protocol_get_device_info,
	.get_chip_vendor_id = scp_protocol_get_chip_vendor_id,
	.set_output_enable = scp_protocol_set_output_enable,
	.set_reset = scp_protocol_set_reset,
	.set_output_voltage = scp_protocol_set_output_voltage,
	.get_output_voltage = scp_protocol_get_output_voltage,
	.set_output_current = scp_protocol_set_output_current,
	.get_output_current = scp_protocol_get_output_current,
	.get_output_current_set = scp_protocol_get_output_current_set,
	.get_min_voltage = scp_protocol_get_min_voltage,
	.get_max_voltage = scp_protocol_get_max_voltage,
	.get_min_current = scp_protocol_get_min_current,
	.get_max_current = scp_protocol_get_max_current,
	.get_inside_temp = scp_protocol_get_inside_temp,
	.get_port_temp = scp_protocol_get_port_temp,
	.get_port_leakage_current_flag = scp_protocol_get_port_leakage_cur_flag,
	.auth_encrypt_start = scp_protocol_auth_encrypt_start,
	.auth_encrypt_release = scp_protocol_auth_encrypt_release,
	.set_usbpd_enable = scp_protocol_set_usbpd_enable,
	.set_default_state = scp_protocol_set_default_state,
	.set_default_param = scp_protocol_set_default_param,
	.set_init_data = scp_protocol_set_init_data,
	.get_protocol_register_state = scp_protocol_get_protocol_register_state,
	.get_adp_type = scp_protocol_get_adp_type,
};

static int __init scp_protocol_init(void)
{
	int ret;
	struct scp_protocol_dev *l_dev = NULL;

	hwlog_info("probe begin\n");

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_scp_protocol_dev = l_dev;

	ret = adapter_protocol_ops_register(&adapter_protocol_scp_ops);
	if (ret)
		goto fail_register_ops;

	hwlog_info("probe end\n");
	return 0;

fail_register_ops:
	kfree(l_dev);
	g_scp_protocol_dev = NULL;

	return ret;
}

static void __exit scp_protocol_exit(void)
{
	hwlog_info("remove begin\n");

	kfree(g_scp_protocol_dev);
	g_scp_protocol_dev = NULL;

	hwlog_info("remove end\n");
}

subsys_initcall_sync(scp_protocol_init);
module_exit(scp_protocol_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("scp protocol driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
