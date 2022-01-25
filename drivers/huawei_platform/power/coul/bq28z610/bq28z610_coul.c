/*
 * bq28z610_coul.c
 *
 * coul with bq28z610 driver
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

#include <linux/module.h>
#include <linux/param.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/idr.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <asm/unaligned.h>
#include <linux/of.h>

#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif

#include <bq28z610_coul.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG bq28z610_coul
HWLOG_REGIST();

static struct bq28z610_device_info *g_bq28z610_dev;
static struct mutex bq28z610_mutex;
static struct chrg_para_lut *p_batt_data;
static struct device *coul_dev;

#define MSG_LEN                      (2)

static int bq28z610_read_block(struct bq28z610_device_info *di,
	u8 reg, u8 *data, u8 len)
{
	struct i2c_msg msg[MSG_LEN];
	int ret = 0;
	int i = 0;

	if (di == NULL || data == NULL) {
		hwlog_err("di or data is null\n");
		return -ENOMEM;
	}

	if (di->client->adapter == NULL) {
		hwlog_err("adapter is null\n");
		return -ENODEV;
	}

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = &reg;
	msg[0].len = 1;

	msg[1].addr = di->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = data;
	msg[1].len = len;

	mutex_lock(&bq28z610_mutex);
	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(di->client->adapter, msg, MSG_LEN);
		if (ret >= 0)
			break;

		usleep_range(5000, 5100); /* sleep 5ms */
	}
	mutex_unlock(&bq28z610_mutex);

	if (ret < 0) {
		hwlog_err("read_block failed[%x]\n", reg);
		return -1;
	}

	return 0;
}

static int bq28z610_read_byte(u8 reg, u8 *data)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;

	return bq28z610_read_block(di, reg, data, 1); /* 2: one byte */
}

static int bq28z610_read_word(u8 reg, u16 *data)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;
	u8 buff[MSG_LEN] = {0};
	int ret = 0;

	ret = bq28z610_read_block(di, reg, buff, 2); /* 2: one word */
	if (ret)
		return -1;

	*data = get_unaligned_le16(buff);
	return 0;
}

static int bq28z610_write_block(struct bq28z610_device_info *di,
	u8 reg, u8 *buff, u8 len)
{
	struct i2c_msg msg;
	int ret;
	int i = 0;

	if (di == NULL || buff == NULL) {
		hwlog_err("di or buff is null\n");
		return -EIO;
	}

	if (di->client->adapter == NULL) {
		hwlog_err("adapter is null\n");
		return -ENODEV;
	}

	buff[0] = reg;

	msg.buf = buff;
	msg.addr = di->client->addr;
	msg.flags = 0;
	msg.len = len + 1;

	mutex_lock(&bq28z610_mutex);
	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(di->client->adapter, &msg, 1);
		if (ret >= 0)
			break;

		usleep_range(5000, 5100); /* sleep 5ms */
	}
	mutex_unlock(&bq28z610_mutex);

	if (ret < 0) {
		hwlog_err("write_block failed[%x]\n", reg);
		return -1;
	}

	return 0;
}

static int bq28z610_write_word(u8 reg, u16 data)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;
	/* 4 bytes offset 2 contains the data offset 0 is used by i2c_write */
	u8 buff[4];

	put_unaligned_le16(data, &buff[1]);

	return bq28z610_write_block(di, reg, buff, 2); /* 2: one word */
}

static u8 checksum(u8 *data, u8 len)
{
	u16 sum = 0;
	int i;

	for (i = 0; i < len; i++)
		sum += data[i];

	sum &= 0xFF;

	return 0xFF - sum;
}

static int bq28z610_read_mac_data(u16 cmd, u8 *dat)
{
	u8 buf[36] = {0}; /* max buf size is 36 bytes */
	u8 cksum_calc, cksum;
	u8 len;
	int ret;
	int i;
	struct bq28z610_device_info *di = g_bq28z610_dev;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return 0;
	}

	/* read length to be read */
	ret = bq28z610_read_byte(BQ28Z610_MAC_DATA_LEN, &len);
	if (ret)
		return 0;

	if (len > 36)
		len = 36;

	/* here length includes checksum byte and length byte itself */
	len = len - 2;

	if (len < 0) {
		hwlog_err("length is not correct %04x\n", len);
		return 0;
	}

	mdelay(2); /* delay 2ms */

	/* read data */
	ret = bq28z610_read_block(di, BQ28Z610_MAC_CMD, buf, len);
	if (ret)
		hwlog_err("bq28z610 read mac data fail\n");

	mdelay(2); /* delay 2ms */

	/* calculate checksum */
	cksum_calc = checksum(buf, len);

	/*read gauge calculated checksum */
	ret = bq28z610_read_byte(BQ28Z610_MAC_DATA_CHECKSUM, &cksum);
	if (ret)
		return 0;

	/* compare checksum */
	if (cksum != cksum_calc) {
		hwlog_err("checksum error\n");
		return 0;
	}

	if (cmd != get_unaligned_le16(buf)) { /* command code not match */
		hwlog_err("command code not match, input: %04x output: %04x\n",
			cmd, get_unaligned_le16(buf));
		return 0;
	}

	/* ignore command code, return data field */
	len -= 2;
	for (i = 0; i < len; i++)
		dat[i] = buf[i+2];

	return (int)len;
}

static int bq28z610_is_ready(void)
{
	if (g_bq28z610_dev != NULL)
		return 1;
	else
		return 0;
}

static int bq28z610_get_battery_id_vol(void)
{
	return 0;
}

static int bq28z610_get_battery_temp(void)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;
	int temp_c = ABNORMAL_BATT_TEMPERATURE_LOW - 1;
	u16 temp_k = temp_c + 273; /* default offset:273 */
	int ret = 0;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq28z610_read_word(BQ28Z610_REG_TEMP, &temp_k);
	if (ret)
		temp_c = di->cache.temp;
	else
		temp_c = (int)temp_k / 10 - 273; /* step:10, offset:273 */

	di->cache.temp = temp_c;

	return temp_c;
}

static int bq28z610_is_battery_exist(void)
{
	int temp;

#ifdef CONFIG_HLTHERM_RUNTEST
	return 0;
#endif

	temp = bq28z610_get_battery_temp();

	if ((temp <= ABNORMAL_BATT_TEMPERATURE_LOW) ||
		(temp >= ABNORMAL_BATT_TEMPERATURE_HIGH))
		return 0;
	else
		return 1;
}

static int bq28z610_get_battery_soc(void)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;
	u16 soc = 0;
	int ret = 0;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return 0;
	}

	if (di->charge_status == CHARGE_STATE_CHRG_DONE) {
		hwlog_info("charge done, force soc to full\n");
		return BATTERY_FULL_CAPACITY;
	}

	ret = bq28z610_read_word(BQ28Z610_REG_SOC, &soc);
	if (ret)
		soc = di->cache.soc;

	di->cache.soc = soc;

	hwlog_info("soc = %d\n", (int)soc);
	return (int)soc;
}

int bq28z610_is_battery_reach_threshold(void)
{
	int soc = 0;

	if (!bq28z610_is_battery_exist())
		return 0;

	soc = bq28z610_get_battery_soc();

	if (soc > BATTERY_CC_WARNING_LEV)
		return 0;
	else if (soc > BATTERY_CC_LOW_LEV)
		return BQ_FLAG_SOC1;
	else
		return BQ_FLAG_LOCK;
}

char *bq28z610_get_battery_brand(void)
{
	u8 data[40]; /* max buf size is 40 bytes */
	static char brand[BQ28Z610_BATT_BRAND_LEN];
	int ret;
	int len;
	int i;

	ret = bq28z610_write_word(BQ28Z610_MAC_CMD, BQ28Z610_MANUINFO);
	if (ret)
		return "error";

	mdelay(5); /* delay 5ms */

	len = bq28z610_read_mac_data(BQ28Z610_MANUINFO, data);
	if (len != BQ28Z610_MANUINFO_LEN)
		return "error";

	memset(brand, 0, BQ28Z610_BATT_BRAND_LEN);
	switch (data[BQ28Z610_PACK_NAME]) {
	case 'D':
		strncat(brand, "desay", strlen("desay"));
		break;

	case 'I':
		strncat(brand, "sunwoda", strlen("sunwoda"));
		break;

	case 'C':
		strncat(brand, "coslight", strlen("coslight"));
		break;

	default:
		strncat(brand, "error", strlen("error"));
		break;
	}

	switch (data[BQ28Z610_CELL_NAME]) {
	case 'A':
		strncat(brand, "atl", strlen("atl"));
		break;

	case 'L':
		strncat(brand, "lg", strlen("lg"));
		break;

	case 'C':
		strncat(brand, "coslight", strlen("coslight"));
		break;

	default:
		strncat(brand, "error", strlen("error"));
		break;
	}

	len = strlen(brand);
	for (i = 0; i < BQ28Z610_PACK_DATE_LEN; i++)
		if (len + i < BQ28Z610_BATT_BRAND_LEN - 1)
			brand[len + i] = data[BQ28Z610_PACK_DATE + i] + 0x30;

	return brand;
}

static int bq28z610_get_battery_vol(void)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;
	u16 vol = 0;
	int ret = 0;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq28z610_read_word(BQ28Z610_REG_VOLT, &vol);
	if (ret)
		vol = di->cache.vol;

	di->cache.vol = vol;

	return (int)vol;
}

static int bq28z610_get_battery_vol_uv(void)
{
	/* mv to uv unit: 1000 */
	return 1000 * bq28z610_get_battery_vol();
}

static int bq28z610_get_battery_curr(void)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;
	u16 curr = 0;
	int ret = 0;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq28z610_read_word(BQ28Z610_REG_CURR, &curr);
	if (ret)
		curr = di->cache.curr;

	di->cache.curr = curr;

	return -(int)((s16)curr);
}

static int bq28z610_get_battery_avgcurr(void)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;
	u16 avg_curr = 0;
	int ret = 0;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq28z610_read_word(BQ28Z610_REG_AVRGCURR, &avg_curr);
	if (ret)
		avg_curr = di->cache.avg_curr;

	di->cache.avg_curr = avg_curr;

	return (int)((s16)avg_curr);
}

static int bq28z610_get_battery_rm(void)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;
	u16 rm = 0;
	int ret = 0;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq28z610_read_word(BQ28Z610_REG_RM, &rm);
	if (ret)
		rm = di->cache.rm;

	di->cache.rm = rm;

	return (int)rm;
}

static int bq28z610_get_battery_dc(void)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;
	u16 dc = 0;
	int ret = 0;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq28z610_read_word(BQ28Z610_REG_DC, &dc);
	if (ret)
		dc = di->cache.dc;

	di->cache.dc = dc;

	return (int)dc;
}

static int bq28z610_get_battery_fcc(void)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;
	u16 fcc = 0;
	int ret = 0;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq28z610_read_word(BQ28Z610_REG_FCC, &fcc);
	if (ret)
		fcc = di->cache.fcc;

	di->cache.fcc = fcc;

	return (int)fcc;
}

static int bq28z610_get_battery_tte(void)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;
	u16 tte = 0;
	int ret = 0;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq28z610_read_word(BQ28Z610_REG_TTE, &tte);
	if (ret)
		tte = di->cache.tte;

	di->cache.tte = tte;

	/* 65535: max value for a word */
	if (tte == 65535) {
		hwlog_info("the battery is not being discharged\n");
		return -1;
	}

	return (int)tte;
}

static int bq28z610_get_battery_ttf(void)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;
	u16 ttf = 0;
	int ret = 0;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq28z610_read_word(BQ28Z610_REG_TTF, &ttf);
	if (ret)
		ttf = di->cache.ttf;

	di->cache.ttf = ttf;

	/* 65535: max value for a word */
	if (ttf == 65535) {
		hwlog_info("the battery is not being charged\n");
		return -1;
	}

	return (int)ttf;
}

static int bq28z610_get_battery_cycle(void)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;
	u16 cycle = 0;
	int ret = 0;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq28z610_read_word(BQ28Z610_REG_CYCLE, &cycle);
	if (ret)
		cycle = di->cache.ttf;

	di->cache.cycle = cycle;

	return cycle;
}

static int bq28z610_battery_unfiltered_soc(void)
{
	return bq28z610_get_battery_soc();
}

static int bq28z610_get_battery_health(void)
{
	int status = POWER_SUPPLY_HEALTH_GOOD;
	int temp = 0;

	if (!bq28z610_is_battery_exist())
		return 0;

	temp = bq28z610_get_battery_temp();
	if (temp < TEMP_TOO_COLD)
		status = POWER_SUPPLY_HEALTH_COLD;
	else if (temp > TEMP_TOO_HOT)
		status = POWER_SUPPLY_HEALTH_OVERHEAT;

	return status;
}

static int bq28z610_get_battery_capacity_level(void)
{
	int capacity = 0;
	int status = 0;

	if (!bq28z610_is_battery_exist())
		return 0;

	capacity = bq28z610_get_battery_soc();

	/*
	 * soc level: 0%, 5%, 15%, 95%, 100%
	 */
	if (capacity > 100 || capacity < 0)
		status = POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
	else if ((capacity >= 0) && (capacity <= 5))
		status = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
	else if ((capacity > 5) && (capacity <= 15))
		status = POWER_SUPPLY_CAPACITY_LEVEL_LOW;
	else if ((capacity >= 95) && (capacity < 100))
		status = POWER_SUPPLY_CAPACITY_LEVEL_HIGH;
	else if (capacity == 100)
		status = POWER_SUPPLY_CAPACITY_LEVEL_FULL;
	else
		status = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;

	return status;
}

static int bq28z610_get_battery_technology(void)
{
	/* default technology is "Li-poly" */
	return POWER_SUPPLY_TECHNOLOGY_LIPO;
}

static int bq28z610_get_battery_vbat_max(void)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;

	if (di == NULL) {
		hwlog_err("di is null\n");
		return 0;
	}

	return di->vbat_max;
}

static int bq28z610_is_fcc_debounce(void)
{
	return 0;
}

static int bq28z610_device_check(void)
{
	u8 version;
	u8 manu_info[40]; /* max buf size is 40 bytes */
	int ret;
	int len;

	ret = bq28z610_write_word(BQ28Z610_MAC_CMD, BQ28Z610_MANUINFO);
	if (ret)
		return -1;

	mdelay(5); /* delay 5ms */

	len = bq28z610_read_mac_data(BQ28Z610_MANUINFO, manu_info);
	if (len != BQ28Z610_MANUINFO_LEN)
		return -1;

	version = manu_info[BQ28Z610_VERSION_INDEX];
	if (version == BQ28Z610_DEFAULT_VERSION)
		return -1;

	return 0;
}

static void charger_event_process(struct bq28z610_device_info *di,
	unsigned int event)
{
	if (di == NULL) {
		hwlog_err("di is null\n");
		return;
	}

	switch (event) {
	/* fall through: usb & ac event */
	case VCHRG_START_USB_CHARGING_EVENT:
	case VCHRG_START_AC_CHARGING_EVENT:
	case VCHRG_START_CHARGING_EVENT:
		hwlog_info("receive charge start event = %d\n", event);
		di->charge_status = CHARGE_STATE_START_CHARGING;
		break;

	case VCHRG_STOP_CHARGING_EVENT:
		hwlog_info("receive charge stop event = %d\n", event);
		di->charge_status = CHARGE_STATE_STOP_CHARGING;
		break;

	case VCHRG_CHARGE_DONE_EVENT:
		hwlog_info("receive charge done event = %d\n", event);
		di->charge_status = CHARGE_STATE_CHRG_DONE;
		break;

	case VCHRG_NOT_CHARGING_EVENT:
		di->charge_status = CHARGE_STATE_NOT_CHARGING;
		hwlog_err("charging is stop by fault\n");
		break;

	case VCHRG_POWER_SUPPLY_OVERVOLTAGE:
		di->charge_status = CHARGE_STATE_NOT_CHARGING;
		hwlog_err("charging is stop by overvoltage\n");
		break;

	case VCHRG_POWER_SUPPLY_WEAKSOURCE:
		di->charge_status = CHARGE_STATE_NOT_CHARGING;
		hwlog_err("charging is stop by weaksource\n");
		break;

	default:
		di->charge_status = CHARGE_STATE_NOT_CHARGING;
		hwlog_err("unknow event %d\n", event);
		break;
	}
}

static int bq28z610_battery_charger_event_rcv(unsigned int evt)
{
	struct bq28z610_device_info *di = g_bq28z610_dev;

	if (di == NULL || !bq28z610_is_battery_exist())
		return 0;

	charger_event_process(di, evt);
	return 0;
}

static int get_batt_para(void)
{
	int ret = 0;
	int array_len = 0;
	int i = 0;
	int row = 0;
	int col = 0;
	const char *string = NULL;
	struct bq28z610_device_info *di = g_bq28z610_dev;
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "Watt_2900_4400_battery");
	if (np == NULL) {
		hwlog_err("Watt_2900_4400_battery dts read failed\n");
		return -EINVAL;
	}

	/* vbat_max */
	ret = of_property_read_u32(np, "vbat_max", &di->vbat_max);
	if (ret) {
		hwlog_err("vbat_max dts read failed\n");
		di->vbat_max = BATTERY_DEFAULT_MAX_VOLTAGE;
	}
	hwlog_info("vbat_max=%d\n", di->vbat_max);

	/* temp_para */
	array_len = of_property_count_strings(np, "temp_para");
	if ((array_len <= 0) || (array_len % TEMP_PARA_TOTAL != 0) ||
		(array_len > TEMP_PARA_LEVEL * TEMP_PARA_TOTAL)) {
		hwlog_err("temp_para dts is invaild\n");
		return -EINVAL;
	}
	p_batt_data->temp_len = array_len;

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "temp_para",
			i, &string);
		if (ret) {
			hwlog_err("temp_para dts read failed\n");
			return -EINVAL;
		}

		row = i / TEMP_PARA_TOTAL;
		col = i % TEMP_PARA_TOTAL;
		if (kstrtol(string, 10,
			(long *)&p_batt_data->temp_data[row][col])) {
			hwlog_err("kstrtol fail\n");
			return -EINVAL;
		}

		hwlog_info("temp_data[%d][%d]=%d\n",
			row, col, p_batt_data->temp_data[row][col]);
	}

	/* vbat_para */
	array_len = of_property_count_strings(np, "vbat_para");
	if ((array_len <= 0) || (array_len % VOLT_PARA_TOTAL != 0) ||
		(array_len > VOLT_PARA_LEVEL * VOLT_PARA_TOTAL)) {
		hwlog_err("vbat_para dts is invaild\n");
		return -EINVAL;
	}
	p_batt_data->volt_len = array_len;

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "vbat_para",
			i, &string);
		if (ret) {
			hwlog_err("vbat_para dts read failed\n");
			return -EINVAL;
		}

		row = i / VOLT_PARA_TOTAL;
		col = i % VOLT_PARA_TOTAL;
		if (kstrtol(string, 10,
			(long *)&p_batt_data->volt_data[row][col])) {
			hwlog_err("kstrtol fail\n");
			return -EINVAL;
		}

		hwlog_info("volt_data[%d][%d]=%d\n",
			row, col, p_batt_data->volt_data[row][col]);
	}

	/* segment_para */
	array_len = of_property_count_strings(np, "segment_para");
	if ((array_len <= 0) || (array_len % SEGMENT_PARA_TOTAL != 0) ||
		(array_len > SEGMENT_PARA_LEVEL * SEGMENT_PARA_TOTAL)) {
		hwlog_err("segment_para dts is invaild\n");
		return -EINVAL;
	}
	p_batt_data->segment_len = array_len;

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "segment_para",
			i, &string);
		if (ret) {
			hwlog_err("segment_para dts read failed\n");
			return -EINVAL;
		}

		row = i / SEGMENT_PARA_TOTAL;
		col = i % SEGMENT_PARA_TOTAL;
		if (kstrtol(string, 10,
			(long *)&p_batt_data->segment_data[row][col])) {
			hwlog_err("kstrtol fail\n");
			return -EINVAL;
		}

		hwlog_info("segment_data[%d][%d]=%d\n",
			row, col, p_batt_data->segment_data[row][col]);
	}

	return 0;
}

struct chrg_para_lut *bq28z610_get_battery_charge_params(void)
{
	return p_batt_data;
}

struct hisi_coul_ops bq28z610_ops = {
	.battery_id_voltage = bq28z610_get_battery_id_vol,
	.is_coul_ready = bq28z610_is_ready,
	.is_battery_exist = bq28z610_is_battery_exist,
	.is_battery_reach_threshold = bq28z610_is_battery_reach_threshold,
	.battery_brand = bq28z610_get_battery_brand,
	.battery_voltage = bq28z610_get_battery_vol,
	.battery_voltage_uv = bq28z610_get_battery_vol_uv,
	.battery_current = bq28z610_get_battery_curr,
	.battery_current_avg = bq28z610_get_battery_avgcurr,
	.battery_unfiltered_capacity = bq28z610_battery_unfiltered_soc,
	.battery_capacity = bq28z610_get_battery_soc,
	.battery_temperature = bq28z610_get_battery_temp,
	.battery_rm = bq28z610_get_battery_rm,
	.battery_fcc = bq28z610_get_battery_fcc,
	.battery_tte = bq28z610_get_battery_tte,
	.battery_ttf = bq28z610_get_battery_ttf,
	.battery_health = bq28z610_get_battery_health,
	.battery_capacity_level = bq28z610_get_battery_capacity_level,
	.battery_technology = bq28z610_get_battery_technology,
	.battery_charge_params = bq28z610_get_battery_charge_params,
	.battery_vbat_max = bq28z610_get_battery_vbat_max,
	.charger_event_rcv = bq28z610_battery_charger_event_rcv,
	.coul_is_fcc_debounce = bq28z610_is_fcc_debounce,
	.battery_cycle_count = bq28z610_get_battery_cycle,
	.battery_fcc_design = bq28z610_get_battery_dc,
	.dev_check = bq28z610_device_check,
};

#if CONFIG_SYSFS
#define BQ28Z610_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, bq28z610_sysfs_show, store), \
	.name = BQ28Z610_SYSFS_##n, \
}

#define BQ28Z610_SYSFS_FIELD_RO(_name, n) \
	BQ28Z610_SYSFS_FIELD(_name, n, 0444, NULL)

static ssize_t bq28z610_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

struct bq28z610_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static struct bq28z610_sysfs_field_info bq28z610_sysfs_field_tbl[] = {
	BQ28Z610_SYSFS_FIELD_RO(gaugelog,       GAUGELOG),
	BQ28Z610_SYSFS_FIELD_RO(gaugelog_head,  GAUGELOG_HEAD),
};

#define BQ28Z610_SYSFS_ATTRS_SIZE  (ARRAY_SIZE(bq28z610_sysfs_field_tbl) + 1)

static struct attribute *bq28z610_sysfs_attrs[BQ28Z610_SYSFS_ATTRS_SIZE];

static const struct attribute_group bq28z610_sysfs_attr_group = {
	.attrs = bq28z610_sysfs_attrs,
};

static void bq28z610_sysfs_init_attrs(void)
{
	int i, limit = ARRAY_SIZE(bq28z610_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		bq28z610_sysfs_attrs[i] =
		&bq28z610_sysfs_field_tbl[i].attr.attr;

	bq28z610_sysfs_attrs[limit] = NULL;
}

static struct bq28z610_sysfs_field_info *bq28z610_sysfs_field_lookup(
	const char *name)
{
	int i, limit = ARRAY_SIZE(bq28z610_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name, bq28z610_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= limit)
		return NULL;

	return &bq28z610_sysfs_field_tbl[i];
}

static ssize_t bq28z610_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bq28z610_sysfs_field_info *info = NULL;
	int temp = 0;
	int voltage = 0;
	int curr = 0;
	int avg_curr = 0;
	int soc = 100; /* full capacity */
	int afsoc = 0;
	int rm = 0;
	int fcc = 0;
	int len = 0;

	info = bq28z610_sysfs_field_lookup(attr->attr.name);
	if (info == NULL) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case BQ28Z610_SYSFS_GAUGELOG_HEAD:
		len = snprintf(buf, PAGE_SIZE,
			"ss_VOL  ss_CUR  ss_AVGCUR  ss_SOC  SOC  ss_RM  ss_FCC  Temp  ");
		break;

	case BQ28Z610_SYSFS_GAUGELOG:
		temp = bq28z610_get_battery_temp();
		voltage = bq28z610_get_battery_vol();
		curr = -bq28z610_get_battery_curr();
		avg_curr = bq28z610_get_battery_avgcurr();
		soc = bq28z610_get_battery_soc();
		afsoc = hisi_bci_show_capacity();
		rm = bq28z610_get_battery_rm();
		fcc = bq28z610_get_battery_fcc();

		len = snprintf(buf, PAGE_SIZE,
			"%-6d  %-6d  %-9d  %-6d  %-3d  %-5d  %-6d  %-4d  ",
			voltage, curr, avg_curr, soc, afsoc, rm, fcc, temp);
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		break;
	}

	return len;
}

static int bq28z610_sysfs_create_group(struct bq28z610_device_info *di)
{
	bq28z610_sysfs_init_attrs();
	return sysfs_create_group(&di->dev->kobj, &bq28z610_sysfs_attr_group);
}

static void bq28z610_sysfs_remove_group(struct bq28z610_device_info *di)
{
	sysfs_remove_group(&di->dev->kobj, &bq28z610_sysfs_attr_group);
}

#else

static inline int bq28z610_sysfs_create_group(struct bq28z610_device_info *di)
{
	return 0;
}

static inline void bq28z610_sysfs_remove_group(struct bq28z610_device_info *di)
{
}
#endif /* CONFIG_SYSFS */

static int bq28z610_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct bq28z610_device_info *di = NULL;
	struct device_node *np = NULL;
	struct class *power_class = NULL;
	int ret = 0;

	hwlog_info("probe begin\n");

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (di == NULL)
		return -ENOMEM;

	di->cache.vol = BATTERY_DEFAULT_VOLTAGE;
	di->cache.temp = ABNORMAL_BATT_TEMPERATURE_LOW - 1;
	di->cache.soc = BATTERY_DEFAULT_CAPACITY;
	di->charge_status = CHARGE_STATE_NOT_CHARGING;
	g_bq28z610_dev = di;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	mutex_init(&bq28z610_mutex);

	p_batt_data = kzalloc(sizeof(*p_batt_data), GFP_KERNEL);
	if (p_batt_data == NULL) {
		ret = -ENOMEM;
		goto bq28z610_fail_0;
	}

	ret = get_batt_para();
	if (ret)
		goto bq28z610_fail_1;

	ret = hisi_coul_ops_register(&bq28z610_ops, COUL_BQ28Z610);
	if (ret) {
		hwlog_err("register bq28z610 coul ops failed\n");
		goto bq28z610_fail_2;
	}

	ret = bq28z610_sysfs_create_group(di);
	if (ret) {
		hwlog_err("sysfs group create failed\n");
		goto bq28z610_fail_2;
	}

	power_class = hw_power_get_class();
	if (power_class != NULL) {
		if (coul_dev == NULL)
			coul_dev = device_create(power_class, NULL, 0, NULL,
				"coul");

		ret = sysfs_create_link(&coul_dev->kobj, &di->dev->kobj,
			"coul_data");
		if (ret) {
			hwlog_err("sysfs link create failed\n");
			goto bq28z610_fail_3;
		}
	}

	hwlog_info("probe end\n");
	return 0;

bq28z610_fail_3:
	bq28z610_sysfs_remove_group(di);
bq28z610_fail_2:
bq28z610_fail_1:
	kfree(p_batt_data);
	p_batt_data = NULL;
bq28z610_fail_0:
	kfree(di);
	di = NULL;
	return ret;
}

static int bq28z610_remove(struct i2c_client *client)
{
	struct bq28z610_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	bq28z610_sysfs_remove_group(di);

	hwlog_info("remove end\n");
	return 0;
}

MODULE_DEVICE_TABLE(i2c, bq28z610);
static const struct of_device_id bq28z610_of_match[] = {
	{
		.compatible = "huawei,bq28z610_coul",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id bq28z610_i2c_id[] = {
	{"bq28z610_coul", 0}, {}
};

static struct i2c_driver bq28z610_driver = {
	.probe = bq28z610_probe,
	.remove = bq28z610_remove,
	.id_table = bq28z610_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bq28z610_coul",
		.of_match_table = of_match_ptr(bq28z610_of_match),
	},
};

static int __init bq28z610_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&bq28z610_driver);
	if (ret)
		hwlog_err("i2c_add_driver error\n");

	return ret;
}

static void __exit bq28z610_exit(void)
{
	i2c_del_driver(&bq28z610_driver);
}

rootfs_initcall(bq28z610_init);
module_exit(bq28z610_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bq28z610 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
