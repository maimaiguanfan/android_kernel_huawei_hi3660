#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/seq_file.h>
#include "elan_mmi.h"

static short *g_test_adc_databuf = NULL;
static short *g_test_adc_databuf2 = NULL;
static uint8_t *g_rawdatabuf = NULL;
static short *g_totlebuf = NULL;
extern struct elan_ktf_ts_data *g_elan_ts;

static int elan_tx_open_test(void);
static int elan_rx_open_test(void);

/****************HID over I2C send cmd format****************
  hid over i2c cmd 0-4 is head 0x04,0x00,0x23,0x00,0x03;
  6th send valid data len
  after 6th is valid cmd data sent to IC
 ************************************************************/

static int elan_poll_int(void)
{
	int i = 0;
	int ret = 0;
	if (!g_elan_ts) {
		TS_LOG_ERR("[elan]:%s, g_elan_ts is NULL!\n", __func__);
		return -EINVAL;
	}
	for (i = 0; i < POLL_INT_COUNT; i++) {
		ret = gpio_get_value(g_elan_ts->int_gpio);
		if (ret == 0) {
			TS_LOG_INFO("[elan]:int is low!count=%d\n", i);
			return NO_ERR;
		} else {
			msleep(5); // IC need
		}
	}
	TS_LOG_INFO("[elan]:int is high!\n");
	return -EINVAL;
}

int alloc_data_buf(void)
{
	int rx = 0;
	int tx = 0;
	if ((!g_elan_ts) || (g_elan_ts->rx_num <= 0) || (g_elan_ts->tx_num <= 0)) {
		TS_LOG_ERR("[elan]:%s,g_elan_ts is NULL or tx rx num is invalid!\n", __func__);
		return -EINVAL;
	}
	rx = g_elan_ts->rx_num;
	tx = g_elan_ts->tx_num;
	g_rawdatabuf = (uint8_t *)kzalloc(sizeof(uint8_t) * (tx * rx * 2), GFP_KERNEL);// rawdata be saved by two bytes
	if (g_rawdatabuf == NULL) {
		TS_LOG_ERR("[elan]:%s,alloc mem for rawdata buf fail!\n", __func__);
		return -ENOMEM;
	}
	g_totlebuf = (short *)kzalloc(sizeof(short) * (tx * rx), GFP_KERNEL);
	if (g_totlebuf == NULL) {
		TS_LOG_ERR("[elan]:%s,alloc mem for g_totlebuf fail!\n", __func__);
		return -ENOMEM;
	}

	return NO_ERR;
}

void free_data_buf(void)
{
	if (g_rawdatabuf) {
		kfree(g_rawdatabuf);
		g_rawdatabuf = NULL;
	}
	if (g_totlebuf) {
		kfree(g_totlebuf);
		g_totlebuf = NULL;
	}
	if (g_test_adc_databuf) {
		kfree(g_test_adc_databuf);
		g_test_adc_databuf = NULL;
	}
	if (g_test_adc_databuf2) {
		kfree(g_test_adc_databuf2);
		g_test_adc_databuf2 = NULL;
	}
	return;
}

static int elan_ts_get_data_by_cmd(uint8_t *cmd, uint8_t *buf, size_t w_size,  size_t r_size)
{
	int ret = 0;
	if (buf == NULL || cmd == NULL) {
		TS_LOG_ERR("[elan]:write and read cmd buf NULL!\n");
		return -EINVAL;
	}
	ret = elan_i2c_write(cmd, w_size);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,write cmd fail!ret=%d\n", __func__, ret);
		return -EINVAL;
	}

	ret = elan_poll_int();
	if (ret) {
		TS_LOG_ERR("[elan]:%s-->elan wait Int low timeout!<--\n", __func__);
		return -EINVAL;
	}
	ret = elan_i2c_read(NULL, 0, buf, r_size);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,read data fail!ret=%d\n", __func__, ret);
		return -EINVAL;
	}
	return NO_ERR;
}

int disable_finger_report(void)
{
	int ret = 0;
	uint8_t disable_finger_cmd[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x54, 0xCA, 0x00, 0x01 };
	ret = elan_i2c_write(disable_finger_cmd, sizeof(disable_finger_cmd));
	if (ret) {
		TS_LOG_ERR("[elan]:i2c send Disable Algorithm1 cmd fail ret=%d!\n", ret);
	}
	return ret;
}

static void copy_this_line(char *dest, char *src)
{
	char *copy_from = NULL;
	char *copy_to = NULL;

	copy_from = src;
	copy_to = dest;
	do {
		*copy_to = *copy_from;
		copy_from++;
		copy_to++;
	} while ((*copy_from != '\n') && (*copy_from != '\r') && (*copy_from != '\0'));
	*copy_to = '\0';
}

static void goto_next_line(char **ptr)
{
	do {
		*ptr = *ptr + 1;
	} while (**ptr != '\n' && **ptr != '\0');
	if (**ptr == '\0') {
		return;
	}
	*ptr = *ptr + 1;
}

static void parse_valid_data(char *buf_start, loff_t buf_size,
		char *ptr, int32_t *data, int rows)
{
	int i = 0;
	int j = 0;
	char *token = NULL;
	char *tok_ptr = NULL;
	char row_data[512] = {0}; // ensure have enough space to save one line data

	if (!ptr) {
		TS_LOG_ERR("%s, ptr is NULL\n", __func__);
		return;
	}
	if (!data) {
		TS_LOG_ERR("%s, data is NULL\n", __func__);
		return;
	}

	for (i = 0; i < rows; i++) {
		// copy this line to row_data buffer
		memset(row_data, 0, sizeof(row_data));
		copy_this_line(row_data, ptr);
		tok_ptr = row_data;
		while ((token = strsep(&tok_ptr, ", \t\n\r\0"))) {
			if (strlen(token) == 0) {
				continue;
			}

			data[j] = (int32_t)simple_strtol(token, NULL, STRTOL_LEN);
			j ++;
		}
		goto_next_line(&ptr);                // next row
		if (!ptr || (strlen(ptr) == 0) || (ptr >= (buf_start + buf_size))) {
			TS_LOG_INFO("invalid ptr, return\n");
			break;
		}
	}

	return;
}

static void print_data(char *target_name, int32_t *data, int rows, int columns)
{
	int i = 0;
	int j = 0;

	if (data == NULL) {
		TS_LOG_ERR("rawdata is NULL\n");
		return;
	}

	for (i = 0; i < rows; i++) {
		for (j = 0; j < columns; j++) {
			TS_LOG_DEBUG("\t%d", data[i * columns + j]);
		}
		TS_LOG_DEBUG("\n");
	}

	return;
}

/* lint -save -e* */
int elan_parse_csvfile(char *file_path, char *target_name, int32_t  *data, int rows, int columns)
{
	struct file *fp = NULL;
	struct kstat stat;
	int ret = 0;
	int32_t read_ret = 0;
	char *buf = NULL;
	char *ptr = NULL;
	mm_segment_t org_fs;
	loff_t pos = 0;
	org_fs = get_fs();
	set_fs(KERNEL_DS);

	if (file_path == NULL) {
		TS_LOG_ERR("file path pointer is NULL\n");
		ret = -EPERM;
		goto exit_free;
	}

	if (target_name == NULL) {
		TS_LOG_ERR("target path pointer is NULL\n");
		ret = -EPERM;
		goto exit_free;
	}

	TS_LOG_INFO("%s, file name is %s, target is %s.\n", __func__, file_path, target_name);

	fp = filp_open(file_path, O_RDONLY, 0);
	if (IS_ERR_OR_NULL(fp)) {
		TS_LOG_ERR("%s, filp_open error, file name is %s.\n", __func__, file_path);
		ret = -EPERM;
		goto exit_free;
	}

	ret = vfs_stat(file_path, &stat);
	if (ret) {
		TS_LOG_ERR("%s, failed to get file stat.\n", __func__);
		ret = -ENOENT;
		goto exit_free;
	}

	buf = (char *)vmalloc(stat.size + 1);
	if (buf == NULL) {
		TS_LOG_ERR("%s: vmalloc %lld bytes failed.\n", __func__, stat.size);
		ret = -ESRCH;
		goto exit_free;
	}
	memset(buf, 0, stat.size + 1);
	read_ret = vfs_read(fp, buf, stat.size, &pos);
	if (read_ret > 0) {
		buf[stat.size] = '\0';
		ptr = buf;
		ptr = strstr(ptr, target_name);
		if (ptr == NULL) {
			TS_LOG_ERR("%s: load %s failed 1!\n", __func__, target_name);
			ret = -EINTR;
			goto exit_free;
		}

		// walk thru this line
		goto_next_line(&ptr);
		if ((ptr == NULL) || (strlen(ptr) == 0)) {
			TS_LOG_ERR("%s: load %s failed 2!\n", __func__, target_name);
			ret = -EIO;
			goto exit_free;
		}

		// analyze the data
		if (data) {
			parse_valid_data(buf, stat.size, ptr, data, rows);
			print_data(target_name, data, rows, columns);
		} else {
			TS_LOG_ERR("%s: load %s failed 3!\n", __func__, target_name);
			ret = -EINTR;
			goto exit_free;
		}
	} else {
		TS_LOG_ERR("%s: ret=%d,read_ret=%d, stat.size=%lld\n", __func__,
			ret, read_ret, stat.size);
		ret = -ENXIO;
		goto exit_free;
	}
	ret = 0;
exit_free:
	TS_LOG_INFO("%s exit free\n", __func__);
	set_fs(org_fs);
	if (buf) {
		TS_LOG_INFO("vfree buf\n");
		vfree(buf);
		buf = NULL;
	}

	if (!IS_ERR_OR_NULL(fp)) {        // fp open fail not means fp is NULL, so free fp may cause Uncertainty
		TS_LOG_INFO("filp close\n");
		filp_close(fp, NULL);
		fp = NULL;
	}
	return ret;
}
/* lint -restore */


/* columns means column number in csv file,rows means row number in csv file */
int elan_get_threshold_from_csvfile(int columns, int rows, char *target_name, int32_t *data)
{
	char file_path[FILE_PATH_MAX_LEN] = {0};
	char file_name[FILE_NAME_MAX_LEN] = {0};
	int ret = 0;
	int result = 0;
	TS_LOG_INFO("[elan]:%s called\n", __func__);

	if ((!data) || (!target_name) || (!g_elan_ts) ||
		(!g_elan_ts->elan_chip_data) ||
		(!g_elan_ts->elan_chip_data->ts_platform_data)) {
		TS_LOG_ERR("parse csvfile failed, data or target_name or g_elan_ts or elan_chip_data is NULL\n");
		return TEST_FAIL;
	}
	snprintf(file_name, sizeof(file_name), "%s_%s_%s_raw.csv",
			g_elan_ts->elan_chip_data->ts_platform_data->product_name,
			g_elan_ts->elan_chip_data->chip_name,
			g_elan_ts->elan_chip_data->module_name);

	snprintf(file_path, sizeof(file_path), "/odm/etc/firmware/ts/%s", file_name);
	TS_LOG_INFO("[elan]:threshold file name:%s, rows_size=%d, columns_size=%d\n", file_path, rows, columns);

	result =  elan_parse_csvfile(file_path, target_name, data, rows, columns);
	if (result == 0) {
		ret = TEST_PASS;
		TS_LOG_INFO("[elan]:Get threshold successed form csvfile\n");
	} else {
		TS_LOG_ERR("[elan]:csv file parse fail:%s\n", file_path);
		ret = TEST_FAIL;
	}
	return ret;
}


/* Disable Algorithm Func */
int elan_get_set_opcmd(void)
{
	int len = 0;
	uint8_t buf_recv[RECV_DATALEN] = {0};
	uint8_t get_option_cmd[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x53, 0xB1, 0x00, 0x01 };
	uint8_t get_option_cmdt[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x53, 0xC1, 0x00, 0x01 };

	len = elan_ts_get_data_by_cmd(get_option_cmd, buf_recv, CMD_DATALEN, RECV_DATALEN);
	if (len) {
		TS_LOG_ERR("[elan]:i2c send or send op cmd fail!\n");
		return -EINVAL;
	} else {
		TS_LOG_DEBUG("[elan]%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", \
				buf_recv[0], buf_recv[1], buf_recv[2], buf_recv[3], buf_recv[4], buf_recv[5], \
				buf_recv[6], buf_recv[7], buf_recv[8], buf_recv[9]); // operation databuf
	}

	get_option_cmd[SEND_CMD_BYTE] = WRITE_CMD;
	get_option_cmd[SEND_OPTION_HBYTE] = (((buf_recv[OPTION_HIGHBYTE] << 8) |
				buf_recv[OPTION_LOWBYTE]) & ALG2_SET_OPTION_BIT) >> 8;
	get_option_cmd[SEND_OPTION_LBYTE] = (((buf_recv[OPTION_HIGHBYTE] << 8) |
				buf_recv[OPTION_LOWBYTE]) & ALG2_SET_OPTION_BIT) & 0xff;

	len = elan_i2c_write(get_option_cmd, sizeof(get_option_cmd));
	if (len) {
		TS_LOG_ERR("[elan]:i2c send Disable Algorithm2 cmd fail,len=%d!\n", len);
		return -EINVAL;
	} else {
		TS_LOG_DEBUG("[elan]i2c send Disable Algorithm2 cmd suceed!\n");
	}

	len = elan_ts_get_data_by_cmd(get_option_cmdt, buf_recv, CMD_DATALEN, RECV_DATALEN);
	if (len) {
		TS_LOG_ERR("[elan]:i2c send or send op cmd fail!\n");
		return -EINVAL;
	} else {
		TS_LOG_DEBUG("[elan]:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", \
				buf_recv[0], buf_recv[1], buf_recv[2], buf_recv[3], buf_recv[4], buf_recv[5], \
				buf_recv[6], buf_recv[7], buf_recv[8], buf_recv[9]);
	}

	get_option_cmdt[SEND_CMD_BYTE] = WRITE_CMD;
	get_option_cmdt[SEND_OPTION_HBYTE] = (((buf_recv[OPTION_HIGHBYTE] << 8) |
				buf_recv[OPTION_LOWBYTE])& ALG3_SET_OPTION_BIT) >> 8;
	get_option_cmdt[SEND_OPTION_LBYTE] = (((buf_recv[OPTION_HIGHBYTE] << 8) |
				buf_recv[OPTION_LOWBYTE])& ALG3_SET_OPTION_BIT) & 0xff;

	len = elan_i2c_write(get_option_cmdt, sizeof(get_option_cmdt));
	if (len) {
		TS_LOG_ERR("[elan]:i2c send Disable Algorithm3 cmd fail,len=%d!\n", len);
		return -EINVAL;
	} else {
		TS_LOG_DEBUG("[elan]:i2c send Disable Algorithm3 cmd suceed!\n");
	}
	msleep(20); // mmi test need
	return NO_ERR;
}

/* set capacitance charger time */
int elan_set_read_ph3(void)
{
	int len = 0;
	uint8_t set_tp_parameter_cmd[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x54, 0x2d, 0x00, 0x01 };
	uint8_t ph3_cmd_buf[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x54, 0xC7, 0x00, PH3 };
	/* 0th-5th is head,6th send byte len,7th read or write cmd */
	uint8_t res_buf[RECV_DATALEN] = {0};
	len = elan_i2c_write(ph3_cmd_buf, sizeof(ph3_cmd_buf));
	if (len) {
		TS_LOG_ERR("[elan]:i2c send ph3_cmd_buf cmd fail!,len=%d\n", len);
		return -EINVAL;
	}
	len = elan_i2c_write(set_tp_parameter_cmd, sizeof(set_tp_parameter_cmd));
	if (len) {
		TS_LOG_ERR("[elan]:i2c send setTPParameter cmd fail!len=%d\n", len);
		return -EINVAL;
	}
	msleep(DELAY_TIME); // mmi test need
	ph3_cmd_buf[7] = READ_CMD; // this 7th is write or read byte
	len = elan_i2c_read(ph3_cmd_buf, sizeof(ph3_cmd_buf), res_buf, sizeof(res_buf));
	if (len || (res_buf[7] != PH3)) {                // 7th is read ph3 byte
		TS_LOG_ERR("[elan]:read ph3 fail!len=%d\n", len);
		return -EINVAL;
	}
	return NO_ERR;
}

void print_rawdata(const int rx, const int tx, int mode, void *data)
{
	int i = 0;
	int j = 0;
	short (*databuf)[rx] = NULL;
	if (!g_totlebuf) {
		TS_LOG_ERR("%s, g_totlebuf is NULL\n", __func__);
		return;
	}
	switch (mode) {
		case NOISE_ADC_PRINT:    // noise adc data print
			for (i = 0; i < tx; i++) {
				printk("[elan_mmi]%02d:", i + 1);
				for (j = 0; j < rx; j++) {
					printk("%04d,", g_totlebuf[j + i * rx]);
					if (j == rx - 1) {
						printk("\n");
					}
				}
			}
			break;
		case SHORT_DATA_PRINT:    // short data print
			printk("[elan_mmi]:Rx:");
			for (i = 0; i < (rx + tx); i++) {
				printk("%04d,", g_totlebuf[i]);
				if (i == rx - 1) {
					printk("\n");
					printk("[elan_mmi]:Tx:");
				}
			}
			printk("\n");
			break;
		case RXTX_DIFF_PRINT:    // tx rx open diff data print
			databuf = (short (*)[rx])data;
			if (databuf == NULL) {
				TS_LOG_ERR("[elan]:%s,databuf is null\n", __func__);
				return;
			}
			for (i = 0; i < tx; i++) {
				printk("[elan_mmi]%02d:", i + 1);
				for (j = 0; j < rx; j++) {
					for (j = 0; j < rx; j++) {
						printk("%04d,", abs(databuf[i][j]));
						if (j == rx - 1) {
							printk("\n");
						}
					}
				}
			}
			break;
		default:
			TS_LOG_DEBUG("[elan]:unknown print rwadata mode!\n");
			break;
	}
	return;
}

/* read capacitance charger arg */
int elan_read_ph(void)
{
	int len = 0;
	uint8_t buf_recv[RECV_DATALEN] = {0};
	uint8_t ph1_cmd_buf[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x53, 0xC5, 0x00, 0x01 };
	uint8_t ph2_cmd_buf[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x53, 0xC6, 0x00, 0x01 };
	uint8_t ph3_cmd_buf[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x53, 0xC7, 0x00, 0x01 };

	len = elan_ts_get_data_by_cmd(ph1_cmd_buf, buf_recv, CMD_DATALEN, RECV_DATALEN);
	if (len) {
		TS_LOG_ERR("[elan]:i2c send read ph1 cmd fail!\n");
		return -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:ph1:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", \
				buf_recv[0], buf_recv[1], buf_recv[2], buf_recv[3], buf_recv[4], \
				buf_recv[5], buf_recv[6], buf_recv[7]); // 0-3th hid head data 4-7 ph1 data
	}

	len = elan_ts_get_data_by_cmd(ph2_cmd_buf, buf_recv, CMD_DATALEN, RECV_DATALEN);
	if (len) {
		TS_LOG_ERR("[elan]:i2c send read ph2 cmd fail!\n");
		return -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:ph2:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", \
				buf_recv[0], buf_recv[1], buf_recv[2], buf_recv[3], buf_recv[4], \
				buf_recv[5], buf_recv[6], buf_recv[7]); // 0-3th hid head data 4-7 ph2 data
	}

	len = elan_ts_get_data_by_cmd(ph3_cmd_buf, buf_recv, CMD_DATALEN, RECV_DATALEN);
	if (len) {
		TS_LOG_ERR("[elan]:i2c send read ph3 cmd fail!\n");
		return -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:ph3:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", \
				buf_recv[0], buf_recv[1], buf_recv[2], buf_recv[3], buf_recv[4], \
				buf_recv[5], buf_recv[6], buf_recv[7]); // 0-3th hid head data 4-7 ph3 data
	}
	return NO_ERR;
}

int elan_calibration_base(void)
{
	int ret = 0;
	uint8_t buff[RECV_DATALEN] = {0};
	uint8_t rsp_buf[4] = { CMD_HEADER_REK, CMD_HEADER_REK, CMD_HEADER_REK, CMD_HEADER_REK };
	uint8_t cmd_buf[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x54, 0xC0, 0xE1, 0x5A }; // flash_key
	uint8_t rek_cmd_buf[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x54, 0x29, 0x00, 0x01 };

	ret = elan_i2c_write(cmd_buf, sizeof(cmd_buf));
	if (ret) {
		TS_LOG_ERR("elanRekTp send cmd fail!ret=%d\n", ret);
		return -EINVAL;
	}

	ret = elan_ts_get_data_by_cmd(rek_cmd_buf, buff, CMD_DATALEN, RECV_DATALEN);
	if (ret) {
		TS_LOG_ERR("elanRekTp elan_ts_get_data_by_cmd fail!\n");
		return -EINVAL;
	}
	ret = memcmp(buff + 4, rsp_buf, sizeof(rsp_buf)); // rsp_buf 4th-7th is rek data(right 0x66)
	if (ret) {
		TS_LOG_ERR("[elan]:unexpected calibration response\n");
		return -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:calibration succeed!\n");
	}
	msleep(DELAY_TIME);    // mmi test need
	return NO_ERR;
}

static int enter_test_mode(void) {
	int ret = 0;
	uint8_t enter_test_mode_cmd[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x55, 0x55, 0x55, 0x55 };
	ret = elan_i2c_write(enter_test_mode_cmd, sizeof(enter_test_mode_cmd));
	if (ret) {
		TS_LOG_ERR("[elan]:i2c send enter test mode cmd fail!ret=%d\n", ret);
	} else {
		TS_LOG_INFO("[elan]:enter test mode succeed!\n");
	}
	return ret;
}

static int exit_test_mode(void) {
	int ret = 0;
	uint8_t exit_test_mode_cmd[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0xa5, 0xa5, 0xa5, 0xa5 };
	ret = elan_i2c_write(exit_test_mode_cmd, sizeof(exit_test_mode_cmd));
	if (ret) {
		TS_LOG_ERR("[elan]:i2c send exit test mode cmd fail!ret=%d\n", ret);
	} else {
		TS_LOG_INFO("[elan]:exit test mode succeed!\n");
	}
	return ret;
}

int elan_noise_test(int rx, int tx, struct noise_limit limit)
{
	int i = 0;
	int j = 0;
	int over_hb = 0;
	int over_lb = 0;
	short (*dv)[rx] = (short (*)[rx])g_totlebuf;
	if (g_totlebuf == NULL) {
		TS_LOG_ERR("[elan]:%s,g_totlebuf or info is null\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < tx; i++) {
		for (j = 0; j < rx; j++) {
			if (dv[i][j] > limit.noise_limit_hb[j + rx * i] || dv[i][j] < limit.noise_limit_lb[j + rx * i]) {
				TS_LOG_ERR("[elan]:noise %d,%d,%d\n", i, j, dv[i][j]);
				return -EINVAL;
			} else if (dv[i][j] > limit.noise_test_hb[j + rx * i]) {
				over_hb++;
				TS_LOG_INFO("[elan]:noise %d,%d,%d\n", i, j, dv[i][j]);
			} else if (dv[i][j] < limit.noise_test_lb[j + rx * i]) {
				over_lb++;
				TS_LOG_INFO("[elan]:noise %d,%d,%d\n", i, j, dv[i][j]);
			}
		}
	}

	if (over_hb > limit.noise_test_hb_fail_point || over_lb > limit.noise_test_lb_fail_point) {
		TS_LOG_ERR("[elan]:Noise Over HB or LB Fail,Fail HBnum=%d,LBnum=%d\n", over_hb, over_lb);
		return 1; // test fail
	}

	return NO_ERR;
}

int elan_get_rawdata(int datalen, bool checkData)
{
	int i = 0;
	int j = 0;
	int ret = 0;
	int recv_num = 0;
	uint8_t databuf[RECV_DATALEN] = {0};
	if (!g_totlebuf || !g_rawdatabuf) {
		TS_LOG_ERR("[elan]:%s,g_totlebuf or g_rawdatabuf is null\n", __func__);
		return -EINVAL;
	}
	recv_num = datalen / VALID_DATA_LEN + ((datalen % VALID_DATA_LEN) != 0); // receive data times
	for (i = 0; i < recv_num; i++) {
		ret = elan_i2c_read(NULL, 0, databuf, sizeof(databuf));
		if (ret) {
			TS_LOG_ERR("[elan]:%s,i2c_read fail!ret=%d\n", __func__, ret);
			return -EINVAL;
		}
		if (checkData) {
			if ((i == (recv_num - 1)) && ((datalen % VALID_DATA_LEN) != 0)) {
				memcpy(g_rawdatabuf + VALID_DATA_LEN * i, databuf + 7, datalen % VALID_DATA_LEN); // 0-6 head data
			} else {
				memcpy(g_rawdatabuf + VALID_DATA_LEN * i, databuf + 7, VALID_DATA_LEN); // 0-6 head data
			}
		}
	}
	if (checkData) {
		for (i = 0; i < datalen; i = i + 2) {
			g_totlebuf[j] = (g_rawdatabuf[i] << 8) | g_rawdatabuf[i + 1];
			j++;
		}
	}
	return NO_ERR;
}

int get_noise_test_data(struct ts_rawdata_info *info)
{
	int ret = 0;
	int i = 0;
	int tx = 0;
	int rx = 0;
	int data_length = 0;
	u32 data_buf[2] = {0};
	struct noise_limit noise;
	uint8_t get_dvdata_cmd[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x06, 0x58, 0x04, 0x00, 0x00, 0x00, 0x00 };
	if (!info || !g_elan_ts || g_elan_ts->tx_num <= 0 || g_elan_ts->rx_num <= 0) {
		TS_LOG_ERR("[elan]:%s,info or g_elan_ts is null\n", __func__);
		return -EINVAL;
	}
	memset(&noise, 0, sizeof(struct noise_limit));
	tx = g_elan_ts->tx_num;
	rx = g_elan_ts->rx_num;
	data_length = tx * rx * 2;
	noise.noise_limit_hb = kzalloc(sizeof(int) * tx * rx, GFP_KERNEL);
	if (noise.noise_limit_hb == NULL) {
		TS_LOG_ERR("[elan]:noise.noise_limit_hb is NULL\n");
		return -ENOMEM;
	}
	noise.noise_limit_lb = kzalloc(sizeof(int) * tx * rx, GFP_KERNEL);
	if (noise.noise_limit_lb == NULL) {
		TS_LOG_ERR("[elan]:noise.noise_limit_lb is NULL\n");
		ret = -ENOMEM;
		goto free_test_space;
	}
	noise.noise_test_hb = kzalloc(sizeof(int) * tx * rx, GFP_KERNEL);
	if (noise.noise_test_hb == NULL) {
		TS_LOG_ERR("[elan]:noise.noise_test_hb is NULL\n");
		ret = -ENOMEM;
		goto free_test_space;
	}
	noise.noise_test_lb = kzalloc(sizeof(int) * tx * rx, GFP_KERNEL);
	if (noise.noise_test_lb == NULL) {
		TS_LOG_ERR("[elan]:noise.noise_test_lb is NULL\n");
		ret = -ENOMEM;
		goto free_test_space;
	}
	ret = elan_get_threshold_from_csvfile(rx, tx, NOISE_TEST_LIMIT_HB, noise.noise_limit_hb);
	if (ret == TEST_FAIL) {
		TS_LOG_ERR("get NoiseTestLimitHB err\n");
		ret = -EINVAL;
		goto free_test_space;
	}
	ret = elan_get_threshold_from_csvfile(rx, tx, NOISE_TEST_LIMIT_LB, noise.noise_limit_lb);
	if (ret == TEST_FAIL) {
		TS_LOG_ERR("get NoiseTestLimitLB err\n");
		ret = -EINVAL;
		goto free_test_space;
	}
	ret = elan_get_threshold_from_csvfile(rx, tx, NOISE_TEST_HB, noise.noise_test_hb);
	if (ret == TEST_FAIL) {
		TS_LOG_ERR("get NoiseTestHB err\n");
		ret = -EINVAL;
		goto free_test_space;
	}
	ret = elan_get_threshold_from_csvfile(rx, tx, NOISE_TEST_LB, noise.noise_test_lb);
	if (ret == TEST_FAIL) {
		TS_LOG_ERR("get NoiseTestLB err\n");
		ret = -EINVAL;
		goto free_test_space;
	}

	ret = elan_get_threshold_from_csvfile(2, 1, NOISE_TEST_FAIL_POINT, data_buf); // 2 col, 1 row
	if (ret == TEST_FAIL) {
		TS_LOG_ERR("get NoiseTestFailPoint err\n");
		ret = -EINVAL;
		goto free_test_space;
	}
	noise.noise_test_hb_fail_point = data_buf[0];
	noise.noise_test_lb_fail_point = data_buf[1];
	TS_LOG_INFO("[elan]:noise_test_hb_fail_point = %d,noise_test_lb_fail_point = %d\n",
			noise.noise_test_hb_fail_point, noise.noise_test_lb_fail_point);

	get_dvdata_cmd[RAW_DATALEN_HIGHBYTE] = (data_length & 0xff00) >> 8;
	get_dvdata_cmd[RAW_DATALEN_LOWBYTE] = data_length & 0x00ff;

	for (i = 0; i < NOISE_SKIP_FRAME + NOISE_TEST_FRAME; i++) {
		ret = enter_test_mode();
		if (ret) {
			TS_LOG_ERR("[elan]:%s,can't enter test mode!\n", __func__);
			ret = -EINVAL;
			goto free_test_space;
		}
		ret = elan_i2c_write(get_dvdata_cmd, sizeof(get_dvdata_cmd));
		if (ret) {
			TS_LOG_ERR("[elan]:%s,send get dv data cmd fail!ret=%d\n", __func__, ret);
			ret = -EINVAL;
			goto free_test_space;
		}
		ret = elan_poll_int();
		if (ret) {
			TS_LOG_ERR("[elan]:%s,poll int fail\n", __func__);
			ret = -EINVAL;
			goto free_test_space;
		}

		if (i >= NOISE_SKIP_FRAME) {
			ret = elan_get_rawdata(data_length, true);
			if (ret) {
				TS_LOG_ERR("[elan]:%s,get check data  fail\n", __func__);
				ret = -EINVAL;
				goto free_test_space;
			}
		} else {
			ret = elan_get_rawdata(data_length, false);
			if (ret) {
				TS_LOG_ERR("[elan]:%s,get skip data  fail\n", __func__);
				ret = -EINVAL;
				goto free_test_space;
			}
		}

		ret = exit_test_mode();
		if (ret) {
			TS_LOG_ERR("[elan]:%s,can't exit test mode!\n", __func__);
			ret = -EINVAL;
			goto free_test_space;
		}
		if (i >= NOISE_SKIP_FRAME) {
			ret = elan_noise_test(rx, tx, noise);
			if (ret) {
				TS_LOG_ERR("[elan]:noise test fail and exit!\n");
				goto noise_test_out;
			}
		}
	}

noise_test_out:
	show_rawdata_data(info);
#ifdef PRINT_RAWDATA
	TS_LOG_INFO("------->>elan_mmi Get Noise Data Start<<-------\n");
	print_rawdata(rx, tx, NOISE_ADC_PRINT, NULL);
#endif

free_test_space:
	if (noise.noise_limit_hb) {
		kfree(noise.noise_limit_hb);
		noise.noise_limit_hb = NULL;
	}
	if (noise.noise_limit_lb) {
		kfree(noise.noise_limit_lb);
		noise.noise_limit_lb = NULL;
	}
	if (noise.noise_test_hb) {
		kfree(noise.noise_test_hb);
		noise.noise_test_hb = NULL;
	}
	if (noise.noise_test_lb) {
		kfree(noise.noise_test_lb);
		noise.noise_test_lb = NULL;
	}
	return ret;
}

int adc_mean_low_boundary_test(int datalen)
{
	int i = 0;
	int meau_adc = 0;
	int total_adc = 0;
	int adc_lb = 0;
	int ret = 0;
	u32 data_buf[2] = {0};

	if (!g_totlebuf) {
		TS_LOG_ERR("%s, g_totlebuf is null\n", __func__);
		return -EINVAL;
	}
	ret = elan_get_threshold_from_csvfile(1, 1, ADCLB, data_buf);
	if (ret == TEST_FAIL) {
		TS_LOG_ERR("get ADCLB err\n");
		return -EINVAL;
	}
	adc_lb = data_buf[0];
	TS_LOG_INFO("[elan]:adc_lb = %d\n", adc_lb);
	for (i = 0; i < datalen; i++) {
		total_adc = total_adc + g_totlebuf[i];
	}
	if (datalen > 0) {
		meau_adc = total_adc / datalen;
		TS_LOG_INFO("[elan]:meau adc = %d\n", meau_adc);
		if (meau_adc < adc_lb) {
			TS_LOG_ERR("[elan]:meau_adc < %d\n", adc_lb);
			return -EINVAL;
		}
	} else {
		TS_LOG_ERR("[elan]:data len is no right!\n");
		return -EINVAL;
	}
	return NO_ERR;
}

int enter_get_data_mode(int adc, int frameNum)
{
	int ret = 0;
	uint8_t buf_recv[RECV_DATALEN] = {0};
	uint8_t enter_getdata_mode_cmd[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x54, 0xcd,
											adc, frameNum };
	uint8_t ready_ack[4] = { 0x42, 0x41, 0x53, 0x45 };
	ret = elan_ts_get_data_by_cmd(enter_getdata_mode_cmd, buf_recv, CMD_DATALEN, RECV_DATALEN);
	if (ret) {
		TS_LOG_ERR("[elan]:send get data mode cmd fail!\n");
		return -EINVAL;
	}
	ret = memcmp(buf_recv + 4, ready_ack, sizeof(ready_ack)); // buf_recv 4th-7th is ack data
	if (ret) {
		TS_LOG_ERR("[elan]:%s,get ACK fail!\n", __func__);
		return -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:%s,get ACK succeed\n", __func__);
	}
	return NO_ERR;
}

/* get normal ADC rawdata */
int get_normal_adc_data(bool checkData)  // checkData true is test ,false is skipframe
{
	int ret = 0;
	int tx = 0;
	int rx = 0;
	int data_length = 0;
	uint8_t data_mode_cmd[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x06, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00 };

	if (!g_elan_ts) {
		TS_LOG_ERR("[elan]:%s,g_elan_ts is null\n", __func__);
		return -EINVAL;
	}

	tx = g_elan_ts->tx_num;
	rx = g_elan_ts->rx_num;
	data_length = tx * rx * 2;
	data_mode_cmd[RAW_DATALEN_HIGHBYTE] = (data_length & 0xff00) >> 8;
	data_mode_cmd[RAW_DATALEN_LOWBYTE] = data_length & 0x00ff;

	ret = enter_test_mode();
	if (ret) {
		TS_LOG_ERR("[elan]:%s,can't enter test mode!\n", __func__);
		return -EINVAL;
	}
	ret = elan_i2c_write(data_mode_cmd, sizeof(data_mode_cmd));
	if (ret) {
		TS_LOG_ERR("[elan]:send data mode cmd fail!ret=%d\n", ret);
		return -EINVAL;
	}
	ret = elan_poll_int();
	if (ret) {
		TS_LOG_ERR("[elan]:%s,poll int fail\n", __func__);
		return -EINVAL;
	}
	ret = elan_get_rawdata(data_length, checkData);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,get normal adc rawdata fail!\n", __func__);
		return -EINVAL;
	}
	ret = exit_test_mode();
	if (ret) {
		TS_LOG_ERR("[elan]:%s,can't exit test mode!\n", __func__);
		return -EINVAL;
	}
#ifdef PRINT_RAWDATA
	if (checkData) {
		TS_LOG_INFO("---->elan_mmi: Get Normal ADC Data_[2] Start<----\n");
		print_rawdata(rx, tx, NOISE_ADC_PRINT, NULL);
	}
#endif
	return NO_ERR;
}

int enter_short_test_mode(void)
{
	int ret = 0;
	uint8_t short_test_cmd[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x54, 0x33, 0x10, 0x00 };

	ret = elan_i2c_write(short_test_cmd, sizeof(short_test_cmd));
	if (ret) {
		TS_LOG_ERR("[elan]:send open short cmd fail!ret=%d\n", ret);
		return ret;
	} else {
		TS_LOG_INFO("[elan]:send open short cmd succeed!\n");
	}
	return ret;
}

/* get txrx short data */
int get_txrx_short_data(int data, int frame, bool checkData)
{
	int ret = 0;
	int tx = 0;
	int rx = 0;
	int data_length = 0;
	uint8_t enter_getdata_mode_cmd[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x54, 0x34, 0x80,
							(uint8_t)data | (uint8_t)frame };
	uint8_t get_short_cmd[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x06, 0x58, 0x01, 0x00, 0x00, 0x00, 0x00 };
	uint8_t dummy_cmd[CMD_DATALEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x06, 0x58, 0xc2, 0x00, 0x00, 0x00, 0x00 };

	if (!g_elan_ts || g_elan_ts->tx_num <= 0 || g_elan_ts->rx_num <= 0) {
		TS_LOG_ERR("[elan]:%s,g_elan_ts is null\n", __func__);
		return -EINVAL;
	}

	tx = g_elan_ts->tx_num;
	rx = g_elan_ts->rx_num;
	data_length = tx * rx * 2;

	get_short_cmd[RAW_DATALEN_HIGHBYTE] = ((unsigned int)data_length & 0xff00) >> 8;
	get_short_cmd[RAW_DATALEN_LOWBYTE] = (unsigned int)data_length & 0x00ff;

	ret = elan_i2c_write(enter_getdata_mode_cmd, sizeof(enter_getdata_mode_cmd));
	if (ret) {
		TS_LOG_ERR("[elan]:send enter get data mode Cmd fail!ret=%d\n", ret);
		return -EINVAL;
	} else {
		TS_LOG_DEBUG("[elan]:send get data cmd:%02x,%02x,%02x,%02x succeed\n", enter_getdata_mode_cmd[7], \
				enter_getdata_mode_cmd[8], enter_getdata_mode_cmd[9], enter_getdata_mode_cmd[10]);
	}

	ret = enter_test_mode();
	if (ret) {
		TS_LOG_ERR("[elan]:%s,can't enter test mode!\n", __func__);
		return -EINVAL;
	}

	ret = elan_i2c_write(get_short_cmd, sizeof(get_short_cmd));
	if (ret) {
		TS_LOG_ERR("[elan]:send get open short cmd fail!ret=%d\n", ret);
		return -EINVAL;
	}
	ret = elan_poll_int();
	if (ret) {
		TS_LOG_ERR("[elan]:%s poll int fail!\n", __func__);
		return -EINVAL;
	}
	ret = elan_get_rawdata(data_length, checkData);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,get tx rx short rawdata fail!\n", __func__);
		return -EINVAL;
	}
	ret = elan_i2c_write(dummy_cmd, sizeof(dummy_cmd));
	if (ret) {
		TS_LOG_ERR("[elan]:send dummy cmd fail!ret=%d\n", ret);
		return -EINVAL;
	}
	ret = exit_test_mode();
	if (ret) {
		TS_LOG_ERR("[elan]:%s,can't exit test mode!\n", __func__);
		return -EINVAL;
	}

	if (checkData) {
		if (data == TEST_ADC_DATA) {
			g_test_adc_databuf = (short *)kzalloc(sizeof(short) * (tx + rx), GFP_KERNEL);
			if (g_test_adc_databuf == NULL)
			{
				TS_LOG_ERR("[elan]:g_test_adc_databuf is NULL\n");
				return -ENOMEM;
			}
			memcpy(g_test_adc_databuf, g_totlebuf, sizeof(short) * (tx + rx));
		} else if (data == TEST_ADC_DATA_2) {
			g_test_adc_databuf2 = (short *)kzalloc(sizeof(short) * (tx + rx), GFP_KERNEL);
			if (g_test_adc_databuf2 == NULL)
			{
				TS_LOG_ERR("[elan]:g_test_adc_databuf2 is NULL\n");
				return -ENOMEM;
			}
			memcpy(g_test_adc_databuf2, g_totlebuf, sizeof(short) * (tx + rx));
		}
#ifdef PRINT_RAWDATA
		TS_LOG_INFO("--->elan_mmi: Check Open Short Data[2] Start<---\n");
		print_rawdata(rx, tx, SHORT_DATA_PRINT, NULL);
#endif
	}
	return NO_ERR;
}

int elan_tx_open_check(void)
{
	int ret = NO_ERR;

	if (!g_elan_ts || (g_elan_ts->tx_num < 1) || (g_elan_ts->rx_num < 1)) {
		TS_LOG_ERR("%s, g_elan_ts invalid\n", __func__);
		return -EINVAL;
	}
	ret = elan_tx_open_test();
	if (ret != NO_ERR) {
		TS_LOG_ERR("elan_tx_open_test failed\n");
	}

	return ret;
}
static int elan_tx_open_test(void)
{
	int i = 0;
	int j = 0;
	int over_hb = 0;
	int edge_over_hb = 0;
	int csv_tx_over_hbpt_ratio = 0;
	int csv_tx_edge_trace_count = 0;
	u32 data_buf[2] = {0};
	u32 *csv_tx_diff_hb = NULL;
	int ret = NO_ERR;
	int res = NO_ERR;
	const int rx = g_elan_ts->rx_num;
	const int tx = g_elan_ts->tx_num;
	short (*databuf)[rx] = (short (*)[rx])g_totlebuf;
	short (*tx_diff_data)[rx] = NULL;
	short *tx_diff_buff = (short *)kzalloc(sizeof(short) * (tx - 1) * rx, GFP_KERNEL);
	if (!tx_diff_buff) {
		TS_LOG_ERR("[elan]:%s tx_diff_buff is null!\n", __func__);
		return -EINVAL;
	}

	tx_diff_data = (short (*)[rx])tx_diff_buff;
	csv_tx_diff_hb = kzalloc(sizeof(u32) * (tx - 1) * rx, GFP_KERNEL);
	if (!csv_tx_diff_hb) {
		TS_LOG_ERR("[elan]:%s csv_tx_diff_hb buf is null!\n", __func__);
		kfree(tx_diff_data);
		tx_diff_data = NULL;
		return -EINVAL;
	}
	if (!g_totlebuf) {
		TS_LOG_ERR("[elan]:%s data buf is null!\n", __func__);
		kfree(tx_diff_data);
		tx_diff_data = NULL;
		kfree(csv_tx_diff_hb);
		csv_tx_diff_hb = NULL;
		return -EINVAL;
	}

	res = elan_get_threshold_from_csvfile(rx, tx - 1, TX_DIFFER_HB, csv_tx_diff_hb);
	if (res == TEST_FAIL) {
		TS_LOG_ERR("get TX_DIFFER_HB err\n");
		ret = -EINVAL;
		goto tx_open_test_exit;
	}

	res = elan_get_threshold_from_csvfile(1, 1, TX_OVER_HBPT_RATIO, data_buf);
	if (res == TEST_FAIL) {
		TS_LOG_ERR("get TxOverHBPTRatio err\n");
		ret = -EINVAL;
		goto tx_open_test_exit;
	}
	csv_tx_over_hbpt_ratio = data_buf[0];
	TS_LOG_INFO("[elan]:csv_tx_over_hbpt_ratio = %d\n", csv_tx_over_hbpt_ratio);

	res = elan_get_threshold_from_csvfile(1, 1, TX_EDGE_TRACE_COUNT, data_buf);
	if (res == TEST_FAIL) {
		TS_LOG_ERR("get TX_EDGE_TRACE_COUNT err\n");
		ret = -EINVAL;
		goto tx_open_test_exit;
	}
	csv_tx_edge_trace_count = data_buf[0];
	TS_LOG_INFO("[elan]:csv_tx_edge_trace_count = %d\n", csv_tx_edge_trace_count);

	for (i = 0; i < tx - 1; i++) {
		for (j = 0; j < rx; j++) {
			if (databuf[i][j] < 0 || databuf[i + 1][j] < 0) {
				TS_LOG_ERR("[elan]:%s,databuf[%d][%d]=%d,databuf[%d][%d]=%d\n", \
						__func__, i, j, databuf[i][j], i + 1, j, databuf[i + 1][j]);
				ret = -EINVAL;
				goto tx_open_test_exit;
			}
			tx_diff_data[i][j] = databuf[i][j] - databuf[i + 1][j];
			if (!((j >= csv_tx_edge_trace_count) && (j <= rx - 1 - csv_tx_edge_trace_count) &&
						(i >= csv_tx_edge_trace_count) && (i <= tx - 2 - csv_tx_edge_trace_count))) { // tx-2 is edge
				if (abs(tx_diff_data[i][j]) > (databuf[i][j]*csv_tx_diff_hb[j + rx * i] / 100)) {
					TS_LOG_INFO("[over_hbelan]:Edge====i=%d,j=%d,tx_diff_data=%d\n", i, j, tx_diff_data[i][j]);
					edge_over_hb++;
				}
			} else {
				if (abs(tx_diff_data[i][j]) > (databuf[i][j]*csv_tx_diff_hb[j + rx * i] / 100)) {
					TS_LOG_INFO("[over_hbelan]:====i=%d,j=%d,tx_diff_data=%d\n", i, j, tx_diff_data[i][j]);
					over_hb++;
				}
			}
		}
	}
#ifdef PRINT_RAWDATA
	TS_LOG_INFO("---->[elan_mmi] TX Diff Data<----\n");
	print_rawdata(rx, tx - 1, RXTX_DIFF_PRINT, tx_diff_data);
#endif

	TS_LOG_INFO("[elan]:TxDiffData edge_over_hb=%d,over_hb=%d\n", edge_over_hb, over_hb);
	if ((edge_over_hb > (rx * csv_tx_over_hbpt_ratio / 100)) ||
			(over_hb > rx * csv_tx_over_hbpt_ratio / 100)) {
		TS_LOG_ERR("[elan]:edge_over_hb is over spec!\n");
		ret = -EINVAL;
		goto tx_open_test_exit;
	}

tx_open_test_exit:
	if (tx_diff_data) {
		kfree(tx_diff_data);
		tx_diff_data = NULL;
	}
	if (csv_tx_diff_hb) {
		kfree(csv_tx_diff_hb);
		csv_tx_diff_hb = NULL;
	}
	return ret;
}

void show_rawdata_data(struct ts_rawdata_info *info)
{
	int i = 0;
	int tx = 0;
	int rx = 0;
	if ((!info) || (!g_elan_ts) || !g_totlebuf) {
		TS_LOG_ERR("[elan]:%s,info is null\n", __func__);
		return;
	}
	tx = g_elan_ts->tx_num;
	rx = g_elan_ts->rx_num;
	for (i = 0; i < tx * rx; i++) {
		info->buff[info->used_size + i] = (int)g_totlebuf[i];
	}
	info->used_size += tx * rx;
	return;
}

void show_txrx_short_data(struct ts_rawdata_info *info)
{
	int tx = 0;
	int rx = 0;
	int i = 0;
	if ((!info) || (!g_elan_ts) || !g_totlebuf) {
		TS_LOG_ERR("[elan]:%s,info is null\n", __func__);
		return;
	}
	tx = g_elan_ts->tx_num;
	rx = g_elan_ts->rx_num;
	for (i = 0; i < (tx + rx); i++) {
		info->buff[i + info->used_size] = g_totlebuf[i];
	}
	info->used_size += (tx + rx);
	return;
}

static int elan_rx_open_test(void)
{
	int i = 0;
	int j = 0;
	int over_hb = 0;
	int edge_over_hb = 0;
	int csv_rx_over_hbpt_ratio = 0;
	int csv_rx_edge_trace_ratio = 0;
	int ret = NO_ERR;
	int res = NO_ERR;
	int rx = g_elan_ts->rx_num;
	int tx = g_elan_ts->tx_num;
	u32 data_buf[2] = {0};
	u32 *csv_rx_differ_hb = NULL;
	short (*databuf)[rx] = (short (*)[rx])g_totlebuf;
	short (*rx_diff_data)[rx - 1] = NULL;
	short *rx_diff_buff = (short *)kzalloc(sizeof(short) * tx * (rx - 1), GFP_KERNEL);
	if (!rx_diff_buff) {
		TS_LOG_ERR("[elan]:alloc rx_diff_buff fail\n");
		return -EINVAL;
	}
	rx_diff_data = (short (*)[rx - 1])rx_diff_buff;
	csv_rx_differ_hb = kzalloc(sizeof(u32) * (rx - 1) * tx, GFP_KERNEL);
	if (!csv_rx_differ_hb) {
		TS_LOG_ERR("[elan]:%s csv_rx_differ_hb buf is null!\n", __func__);
		kfree(rx_diff_data);
		rx_diff_data = NULL;
		return -EINVAL;
	}
	if (!g_totlebuf) {
		TS_LOG_ERR("[elan]:%s,data buf is null\n", __func__);
		kfree(rx_diff_data);
		rx_diff_data = NULL;
		kfree(csv_rx_differ_hb);
		csv_rx_differ_hb = NULL;
		return -EINVAL;
	}
	res = elan_get_threshold_from_csvfile(rx - 1, tx, RX_DIFFER_HB, csv_rx_differ_hb);
	if (res == TEST_FAIL) {
		TS_LOG_ERR("get RxDifferHB err\n");
		ret = -EINVAL;
		goto rx_open_test_exit;
	}

	res = elan_get_threshold_from_csvfile(1, 1, RX_OVER_HBPT_RATIO, data_buf);
	if (res == TEST_FAIL) {
		TS_LOG_ERR("get RxOverHBPTRatio err\n");
		ret = -EINVAL;
		goto rx_open_test_exit;
	}
	csv_rx_over_hbpt_ratio = data_buf[0];
	TS_LOG_INFO("[elan]:csv_rx_over_hbpt_ratio = %d\n", csv_rx_over_hbpt_ratio);

	res = elan_get_threshold_from_csvfile(1, 1, RX_EDGE_TRACE_COUNT, data_buf);
	if (res == TEST_FAIL) {
		TS_LOG_ERR("get RxEdgeTraceCount err\n");
		ret = -EINVAL;
		goto rx_open_test_exit;
	}
	csv_rx_edge_trace_ratio = data_buf[0];
	TS_LOG_INFO("[elan]:csv_rx_edge_trace_ratio = %d\n", csv_rx_edge_trace_ratio);

	for (i = 0; i < tx; i++) {
		for (j = 0; j < rx - 1; j++) {
			if (databuf[i][j] < 0 || databuf[i][j + 1] < 0) {
				TS_LOG_ERR("[elan]:%s,databuf[%d][%d]=%d,databuf[%d][%d]=%d\n", \
						__func__, i, j, databuf[i][j], i + 1, j, databuf[i + 1][j]);
				ret = -EINVAL;
				goto rx_open_test_exit;
			}
			rx_diff_data[i][j] = abs(databuf[i][j] - databuf[i][j + 1]);
			if (!((j >= csv_rx_edge_trace_ratio) && (j <= rx - 2 - csv_rx_edge_trace_ratio) &&
						(i >= csv_rx_edge_trace_ratio) && (i <= tx - 1 - csv_rx_edge_trace_ratio))) { // rx-2 is edge
				if (rx_diff_data[i][j] > (databuf[i][j] * csv_rx_differ_hb[j + (rx - 1) * i] / 100)) {
					TS_LOG_INFO("[elan over_hb]:i=%d,j=%d\n", i, j);
					edge_over_hb++;
				}
			} else {
				if (rx_diff_data[i][j] > (databuf[i][j] * csv_rx_differ_hb[j + (rx - 1) * i] / 100)) {
					TS_LOG_INFO("[elan over_hb]:i=%d,j=%d\n", i, j);
					over_hb++;
				}
			}
		}
	}
#ifdef PRINT_RAWDATA
	TS_LOG_INFO("----->[elan_mmi] RX Diff Data<----\n");
	print_rawdata(rx - 1, tx, RXTX_DIFF_PRINT, rx_diff_data);
#endif

	TS_LOG_INFO("[elan]:RxDiffData edge_over_hb=%d,over_hb=%d\n", edge_over_hb, over_hb);
	if ((edge_over_hb > tx * csv_rx_over_hbpt_ratio / 100) ||
			(over_hb > tx * csv_rx_over_hbpt_ratio / 100)) {
		TS_LOG_ERR("[elan]:edge_over_hb is over spec!\n");
		ret = -EINVAL;
	}

rx_open_test_exit:
	if (rx_diff_data) {
		kfree(rx_diff_data);
		rx_diff_data = NULL;
	}
	if (csv_rx_differ_hb) {
		kfree(csv_rx_differ_hb);
		csv_rx_differ_hb = NULL;
	}
	return ret;
}

int elan_txrx_short_test(void)
{
	int i = 0;
	int ret = 0;
	int over_hb = 0;
	bool txrx_short_test_result = true;
	int rx = 0;
	int tx = 0;
	u32 csv_tx_trace_adjust_differ_hb_cond1[MAX_TRACE_LEN] = {0};
	u32 csv_rx_trace_adjust_differ_hb_cond1[MAX_TRACE_LEN] = {0};
	short *differ_data = g_test_adc_databuf;
	if (!g_test_adc_databuf || !g_test_adc_databuf2 || !g_elan_ts || g_elan_ts->rx_num <= 1 ||
			g_elan_ts->tx_num <= 1) { // avoid tx -1 < 0
		TS_LOG_ERR("[elan]:%s,data buf is null\n", __func__);
		return -EINVAL;
	}
	rx = g_elan_ts->rx_num;
	tx = g_elan_ts->tx_num;
	ret = elan_get_threshold_from_csvfile(tx - 1, 1, TX_TRACE_ADJ_DIFFER_HB_COND1, csv_tx_trace_adjust_differ_hb_cond1);
	if (ret == TEST_FAIL) {
		TS_LOG_ERR("get TX_TRACE_ADJ_DIFFER_HB_COND1 err\n");
		return -EINVAL;
	}
	ret = elan_get_threshold_from_csvfile(rx - 1, 1, RX_TRACE_ADJ_DIFFER_HB_COND1, csv_rx_trace_adjust_differ_hb_cond1);
	if (ret == TEST_FAIL) {
		TS_LOG_ERR("get RX_TRACE_ADJ_DIFFER_HB_COND1 err\n");
		return -EINVAL;
	}
	TS_LOG_INFO("[elan_mmi]:Rx diff:");
	for (i = 0; i < (rx + tx); i++) {
		if (g_test_adc_databuf[i] == 0 || g_test_adc_databuf2[i] == 0) {
			txrx_short_test_result = false;
			TS_LOG_ERR("[elan]:g_test_adc_databuf[%d]=%d,g_test_adc_databuf2[%d]=%d",
					i, g_test_adc_databuf[i], i, g_test_adc_databuf2[i]);
		}
		differ_data[i] = abs(g_test_adc_databuf[i] - g_test_adc_databuf2[i]);
		printk("%03d,", differ_data[i]);
		if (i == rx - 1) {
			TS_LOG_INFO("\n");
			TS_LOG_INFO("[elan_mmi]:Tx diff:");
		}
	}
	TS_LOG_INFO("\n");
	TS_LOG_INFO("[elan_mmi]:RX Short Test Adjacent Differ Data:");
	for (i = 0; i < (rx - 1); i++) {
		differ_data[i] = abs(differ_data[i] - differ_data[i + 1]);
		printk("%d,", differ_data[i]);
		if ((i > 0) && (i < rx - 2)) {
			if (differ_data[i] > (RX_TRACE_TARGET_VALUE * csv_rx_trace_adjust_differ_hb_cond1[i] / 100)) { // 100 is percent
				over_hb++;
				TS_LOG_ERR("[elan]:%s,over_hb=%d\n", __func__, over_hb);
			}
		} else {
			if (differ_data[i] > (RX_TRACE_TARGET_VALUE * csv_rx_trace_adjust_differ_hb_cond1[i] / 100)) { // 100 is percent
				over_hb++;
				TS_LOG_ERR("[elan]:%s,over_hb=%d\n", __func__, over_hb);
			}
		}
	}

	if (over_hb > 0) {
		TS_LOG_ERR("[elan]:RX Short Test Fail!\n");
		txrx_short_test_result = false;
	} else {
		TS_LOG_INFO("[elan]:RX Short Test Pass!\n");
	}

	over_hb = 0;
	TS_LOG_INFO("[elan_mmi]:TX Short Test Adjacent Differ Data:");
	for (i = rx; i < (rx + tx - 1); i++) {
		differ_data[i] = abs(differ_data[i] - differ_data[i + 1]);
		printk("%d,", differ_data[i]);
		if ((i > rx) && (i < rx + tx - 2)) {
			if ((differ_data[i] > TX_TRACE_TARGET_VALUE * csv_tx_trace_adjust_differ_hb_cond1[i - rx] / 100)) {
				over_hb++;
			}
		} else {
			if (differ_data[i] > TX_TRACE_TARGET_VALUE * csv_tx_trace_adjust_differ_hb_cond1[i - rx] / 100) {
				over_hb++;
			}
		}
	}

	if (over_hb > 0) {
		txrx_short_test_result = false;
	}

	if (!txrx_short_test_result) {
		TS_LOG_ERR("[elan]:TX Short Test Fail!\n");
		ret = -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:TX Short Test Pass!\n");
		ret = NO_ERR;
	}
	return ret;
}

/* get tx rx short data */
static int elan_txrx_short(struct ts_rawdata_info *info) {
	int ret = 0;
	if (!info) {
		TS_LOG_ERR("[elan]:%s,info is null\n", __func__);
		return -EINVAL;
	}
	ret = get_txrx_short_data(TEST_ADC_DATA, NORMAL_ADC_SKIP_FRAME, false);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,get short txrx skip frame data1 fail!\n", __func__);
		return -EINVAL;
	}
	ret = get_txrx_short_data(TEST_ADC_DATA, NORMAL_ADC_TEST_FRAME, true);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,get short txrx test frame data1 fail!\n", __func__);
		return -EINVAL;
	}
	show_txrx_short_data(info);
	ret = get_txrx_short_data(TEST_ADC_DATA_2, NORMAL_ADC_SKIP_FRAME, false);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,get short txrx skip frame data2 fail!\n", __func__);
		return -EINVAL;
	}
	ret = get_txrx_short_data(TEST_ADC_DATA_2, NORMAL_ADC_TEST_FRAME, true);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,get short txrx test frame data2 fail!\n", __func__);
		return -EINVAL;
	}
	show_txrx_short_data(info);
	return NO_ERR;
}

static int normal_adc_get_data(void)
{
	int ret = 0;
	ret = enter_get_data_mode(GET_NORMAL_ADC, NORMAL_ADC_SKIP_FRAME);
	if (ret) {
		TS_LOG_ERR("[elan]:enter skip frame adc get data mode fail\n");
		return -EINVAL;
	}
	ret = get_normal_adc_data(false);
	if (ret) {
		TS_LOG_ERR("[elan]:get ship frame normal adc fail\n");
		return -EINVAL;
	}
	ret = enter_get_data_mode(GET_NORMAL_ADC, NORMAL_ADC_TEST_FRAME);
	if (ret) {
		TS_LOG_ERR("[elan]:enter test frame adc get data mode fail\n");
		return -EINVAL;
	}
	ret = get_normal_adc_data(true);
	if (ret) {
		TS_LOG_ERR("[elan]:get test frame normal adc fail\n");
		return -EINVAL;
	}
	return NO_ERR;
}

int elan_rx_open_check(struct ts_rawdata_info *info)
{
	int ret = 0;
	if (!info || !g_elan_ts || (g_elan_ts->tx_num < 1) || (g_elan_ts->rx_num < 1)) {
		TS_LOG_ERR("[elan]:%s,info is null\n", __func__);
		return -EINVAL;
	}
	ret = normal_adc_get_data();
	if (ret) {
		TS_LOG_ERR("[elan]:%s,normal adc get data fail!\n", __func__);
		return -EINVAL;
	}
	if (info) {
		show_rawdata_data(info);
	}
	ret = elan_rx_open_test();
	if (ret) {
		TS_LOG_ERR("[elan]:rx open test fail!\n");
		return 1;
	} else {
		TS_LOG_INFO("[elan]:rx open test pass!\n");
	}
	return NO_ERR;
}

int elan_mean_value_check(char *result, struct ts_rawdata_info *info) {
	int ret = 0;

	if (!g_elan_ts) {
		TS_LOG_ERR("%s, [elan] g_elan_ts is null\n", __func__);
		return -EINVAL;
	}
	ret = elan_set_read_ph3();
	if (ret) {
		TS_LOG_ERR("[elan]:set ph3 fail\n");
		return -EINVAL;
	}

	ret = normal_adc_get_data();
	if (ret) {
		TS_LOG_ERR("[elan]:get check data fail\n");
		return -EINVAL;
	}
	if (info) {
		show_rawdata_data(info);
	}
	ret = adc_mean_low_boundary_test(g_elan_ts->tx_num * g_elan_ts->rx_num);
	if (ret) {
		TS_LOG_ERR("[elan]:ADC Mean Low Boundary Test Fail\n");
		strncat(result, "1F-", strlen("1F-"));
	} else {
		TS_LOG_INFO("[elan]:ADC Mean Low Boundary Test Pass\n");
		strncat(result, "1P-", strlen("1P-"));
	}
	return NO_ERR;
}

int elan_txrx_short_check(char *result, struct ts_rawdata_info *info) {
	int ret = 0;
	ret = enter_short_test_mode();
	if (ret) {
		TS_LOG_ERR("[elan]:%s,enter short txrx mode fail!\n", __func__);
		return -EINVAL;
	}
	ret = elan_txrx_short(info);
	if (ret) {
		TS_LOG_ERR("[elan]:tx rx short test get data fail!\n");
		return -EINVAL;
	}
	ret = elan_txrx_short_test();
	if (ret) {
		TS_LOG_ERR("[elan]:tx rx short test Fail!\n");
		strncat(result, "4F;", strlen("4F;"));
	} else {
		TS_LOG_INFO("[elan]:tx rx short test Pass!\n");
		strncat(result, "4P;", strlen("4P;"));
	}
	return NO_ERR;
}
