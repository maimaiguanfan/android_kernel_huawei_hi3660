
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/string.h>
#include <linux/spi/spi.h>
#include "huawei_thp.h"
#include <linux/time.h>
#include <linux/syscalls.h>

#ifdef CONFIG_HUAWEI_DUBAI
#include <huawei_platform/log/hwlog_kernel.h>
#endif

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#define SSL_IC_NAME "ssl_thp"
#define THP_SSL_DEV_NODE_NAME "ssl_thp"

#define MXT680U2_FAMILY_ID            166
#define MXT680U2_VARIANT_ID           22

#define MXT_WAKEUP_TIME               10
#define BIT_SHIFT_8                    8
#define T144_ACTIVE_TIMER_OFFSET       8
#define T144_ACTIVE_DOZING_READ_LEN    8
// opcodes
#define SPI_WRITE_REQ            0x01
#define SPI_WRITE_OK             0x81
#define SPI_WRITE_FAIL           0x41
#define SPI_READ_REQ             0x02
#define SPI_READ_OK              0x82
#define SPI_READ_FAIL            0x42
#define SPI_INVALID_REQ          0x04
#define SPI_INVALID_CRC          0x08
#define SPI_APP_HEADER_LEN       6
#define SPI_BOOTL_HEADER_LEN     2
#define T117_BYTES_READ_LIMIT    1505 // 7 x 215 (T117 size)
#define WRITE_DUMMY_BYTE         400
#define READ_DUMMY_BYTE          400
#define FRAME_READ_DUMMY_BYTE    400
#define NUMBER_OF_HEADER         2
#define SPI_APP_DATA_MAX_LEN     64
#define SPI_APP_BUF_SIZE_WRITE   (SPI_APP_HEADER_LEN + SPI_APP_DATA_MAX_LEN +\
				WRITE_DUMMY_BYTE)
#define SPI_APP_BUF_SIZE_READ    (NUMBER_OF_HEADER * SPI_APP_HEADER_LEN +\
				T117_BYTES_READ_LIMIT + FRAME_READ_DUMMY_BYTE)
#define MXT_OBJECT_START         0x07 // after struct mxt_info
#define MXT_INFO_CHECKSUM_SIZE   3 // after list of struct mxt_object
#define GETPDSDATA_COMMAND       0x81
#define MXT_T6_DIAGNOSTIC_OFFSET 0x05
#define READ_ID_RETRY_TIMES      3

#define SSL_T117_ADDR            117
#define SSL_T7_ADDR              7
#define SSL_T6_ADDR              6
#define SSL_T37_ADDR             37
#define SSL_T118_ADDR            118
#define SSL_T24_ADDR             24
#define SSL_T144_ADDR            144
#define SSL_T8_ADDR              8
#define SSL_T145_ADDR            145

#define SSL_SYNC_DATA_REG_DEFALUT_ADDR  238
#define THP_BOOTLOADER_SPI_FREQ_HZ      100000U
#define BOOTLOADER_READ_CNT             2
#define BOOTLOADER_READ_LEN             1
#define BOOTLOADER_CRC_ERROR_CODE       0x60
#define BOOTLOADER_WAITING_CMD_MODE_0   0xC0
#define BOOTLOADER_WAITING_CMD_MODE_1   0xE0

#define PDS_HEADER_OFFSET               4
#define PROJECTID_ARR_LEN               32
#define SSL_T7_COMMAMD_LEN              2
#define T24_GESTURE_ON_MASK             0x03
#define T117_HEADER_STATUS_OFFSET       0x05
#define T117_GESTURE_EVENT              0x40
#define SEND_COMMAND_RETRY              3
#define SSL_WAIT_FOR_SPI_BUS_RESUMED_DELAY 20
#define SSL_WAIT_FOR_SPI_BUS_READ_DELAY 5

#define MXT_T145_SCREEN_OFF_CMD         1
#define SCREEN_OFF_MODE_SCAN_RATE       50
#define SCREEN_OFF_SCAN_MODE            2
#define SCREEN_OFF_MCT_SCAN_MODE        3
#define T8_MEASIDLEDEF_OFFSET           11
#define MXT_T145_CMD_OFFSET             9

#define MOVE_8BIT                       8
#define MOVE_16BIT                      16
#define MOVE_24BIT                      24

struct mxt_info {
	u8 family_id;
	u8 variant_id;
	u8 version;
	u8 build;
	u8 matrix_xsize;
	u8 matrix_ysize;
	u8 object_num;
};

struct mxt_object {
	u8 type;
	u16 start_address;
	u8 size_minus_one;
	u8 instances_minus_one;
	u8 num_report_ids;
} __packed;

static uint16_t t117_address;
static uint16_t t7_address;
static uint16_t t6_address;
static uint16_t t37_address;
static uint16_t t118_address;
static uint16_t t24_address;
static uint16_t t144_address;
static uint16_t t8_address;
static uint16_t t145_address;

static u8 is_bootloader_mode;
static unsigned int g_thp_udfp_stauts;

static int ssl_wakeup_gesture_enable_switch(struct thp_device *tdev,
				u8 switch_value);

static u8 get_crc8_iter(u8 crc, u8 data)
{
	static const u8 crc_inter_check = 0x8c;
	u8 index = 8;
	u8 fb = 0;

	do {
		fb = (crc ^ data) & 0x01;
		data >>= 1;
		crc >>= 1;
		if (fb)
			crc ^= crc_inter_check;
	} while (--index);
	return crc;
}

static u8 get_header_crc(u8 *p_msg)
{
	u8 calc_crc = 0;
	int i = 0;

	if (p_msg == NULL) {
		THP_LOG_ERR("%s: point null\n", __func__);
		return -EINVAL;
	}
	for (; i < (SPI_APP_HEADER_LEN - 1); i++)
		calc_crc = get_crc8_iter(calc_crc, p_msg[i]);

	return calc_crc;
}

static void spi_prepare_header(u8 *header, u8 opcode,
		u16 start_register, u16 count)
{
	if (header == NULL) {
		THP_LOG_ERR("%s: point null\n", __func__);
		return;
	}

	header[0] = opcode;
	header[1] = start_register & 0xff;
	header[2] = start_register >> MOVE_8BIT;
	header[3] = count & 0xff;
	header[4] = count >> MOVE_8BIT;
	header[5] = get_header_crc(header);
}

static int mxt_bootloader_read(struct thp_device *tdev,
		struct spi_device *client, unsigned char *buf, int count)
{
	/* Require SPI LSB 0x01 for read op */
	static const char op_header[] = { 0x01, 0x00 };
	int ret_val;
	struct spi_message spi_msg;
	struct spi_transfer transfer;

	if ((tdev == NULL) || (client == NULL) || (buf == NULL) ||
		(tdev->tx_buff == NULL) || (tdev->rx_buff == NULL)) {
		THP_LOG_ERR("%s: point null\n", __func__);
		return -EINVAL;
	}
	spi_message_init(&spi_msg);
	memset(&transfer, 0, sizeof(struct spi_transfer));

	transfer.tx_buf = tdev->tx_buff;
	transfer.rx_buf = tdev->rx_buff;
	transfer.len = sizeof(op_header) + count;
	memcpy(tdev->tx_buff, op_header, sizeof(op_header));
	spi_message_add_tail(&transfer, &spi_msg);
	ret_val = spi_sync(client, &spi_msg);
	if (ret_val < 0) {
		THP_LOG_ERR("%s: Error reading from spi ret = %d\n", __func__, ret_val);
		return ret_val;
	}
	memcpy(buf, tdev->rx_buff + sizeof(op_header), count);
	return ret_val;

}

static int mxt_wait_for_chg_is_low(struct thp_device *tdev)
{
	int ret_val;
	int count_timeout = 100; // 100 msec

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	while ((gpio_get_value(tdev->gpios->irq_gpio) != 0) &&
		(count_timeout > 0)) {
		count_timeout--;
		mdelay(1); // 1 msec
	}
	ret_val = count_timeout > 0 ? 0 : -1;
	return ret_val;
}


static int mxt_check_is_bootloader(struct thp_device *tdev)
{
	u8 status_byte = 0;
	int ret_value = 0;
	int bootloader_read_cnt = BOOTLOADER_READ_CNT;
	int max_thp_spi_clock;
	int ret;

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	max_thp_spi_clock = tdev->thp_core->sdev->max_speed_hz;
	THP_LOG_INFO("max thp spi clock= %d\n", max_thp_spi_clock);
	tdev->thp_core->sdev->max_speed_hz = THP_BOOTLOADER_SPI_FREQ_HZ;
	THP_LOG_INFO("set max thp spi clock= %d\n",
		tdev->thp_core->sdev->max_speed_hz);

	do {
		if (mxt_wait_for_chg_is_low(tdev) < 0) {
			THP_LOG_ERR("%s:CHG doesn't change to LOW\n", __func__);
		} else {
			ret = thp_bus_lock();
			if (ret) {
				THP_LOG_ERR("%s: get lock failed\n", __func__);
				continue;
			}
			ret = mxt_bootloader_read(tdev, tdev->thp_core->sdev,
					&status_byte, BOOTLOADER_READ_LEN);
			thp_bus_unlock();
			if (ret)
				THP_LOG_ERR("%s: mxt_bootloader_readfailed\n",
						__func__);
			THP_LOG_INFO("the bootloader status byte is %d\n",
					status_byte);
			if ((status_byte == BOOTLOADER_CRC_ERROR_CODE) ||
				(status_byte == BOOTLOADER_WAITING_CMD_MODE_0) ||
				(status_byte == BOOTLOADER_WAITING_CMD_MODE_1)) {
				// 0x60: CRC error  0xE0: waiting cmd mode
				THP_LOG_INFO("bootloader mode found -status is 0x%x\n",
						status_byte);
				ret_value = 1;
				break;
			}
		}
	} while (bootloader_read_cnt-- > 0);
	tdev->thp_core->sdev->max_speed_hz = max_thp_spi_clock;

	return ret_value;
}


static int __mxt_read_reg(struct thp_device *tdev,
		struct spi_device *client, u16 start_register, u16 len, u8 *val)
{
	u8 attempt = 0;
	int ret_val;
	int i;
	int dummy_byte;
	int dummy_offset = 0;
	u8 *rx_buf = NULL;
	u8 *tx_buf = NULL;

	struct spi_message  spi_msg;
	struct spi_transfer transfer;

	if ((tdev == NULL) || (client == NULL) || (val == NULL) ||
		(tdev->rx_buff == NULL) || (tdev->tx_buff == NULL)) {
		THP_LOG_ERR("%s: tdev or client or val null\n", __func__);
		return -EINVAL;
	}

	rx_buf = tdev->rx_buff;
	tx_buf = tdev->tx_buff;

	if (len == T117_BYTES_READ_LIMIT)
		dummy_byte = FRAME_READ_DUMMY_BYTE;
	else
		dummy_byte = READ_DUMMY_BYTE;

	do {
		attempt++;
		if (attempt > 1) {
			if (attempt > 5) {
				THP_LOG_ERR("%s:Too many Retries\n", __func__);
				return -EIO;
			}
			if (len < T117_BYTES_READ_LIMIT)
				// SPI2 need some delay time
				mdelay(MXT_WAKEUP_TIME);
			else
				return  -EINVAL;
		}

		memset(tx_buf, 0xFF, (NUMBER_OF_HEADER * SPI_APP_HEADER_LEN +
			dummy_byte));
		spi_prepare_header(tx_buf, SPI_READ_REQ, start_register, len);

		spi_message_init(&spi_msg);
		memset(&transfer, 0,  sizeof(struct spi_transfer));
		transfer.tx_buf = tx_buf;
		transfer.rx_buf = rx_buf;
		transfer.len = NUMBER_OF_HEADER * SPI_APP_HEADER_LEN +
				dummy_byte + len;
		spi_message_add_tail(&transfer, &spi_msg);

		ret_val = spi_sync(client, &spi_msg);

		if (ret_val < 0) {
			THP_LOG_ERR("%s: Error reading from spi ret = %d\n",
				__func__, ret_val);
			return ret_val;
		}

		for (i = 0; i < dummy_byte; i++) {
			if (rx_buf[SPI_APP_HEADER_LEN + i] == SPI_READ_OK) {
				dummy_offset = i + SPI_APP_HEADER_LEN;
				if (dummy_offset > READ_DUMMY_BYTE / 2)
					THP_LOG_INFO("Found read dummy offset %d\n",
						dummy_offset);
				break;
			}
		}
		if (dummy_offset == 0) {
			THP_LOG_ERR(" cannot find dummy byte offset- read address =%d\n",
				start_register);
			if (len == T117_BYTES_READ_LIMIT)
				return -EINVAL;
		} else {
			// tx[1] or rx[1] :LSB of start register address
			// tx[2] or rx[2] :MSB of start register address
			// tx[3] or rx[3] :LSB of len
			// tx[4] or rx[4] :MSB of len
			if ((tx_buf[1] != rx_buf[1 + dummy_offset]) ||
				(tx_buf[2] != rx_buf[2 + dummy_offset])) {
				THP_LOG_ERR("%s:Unexpected address %d != %d reading from spi\n",
					__func__,
					rx_buf[1 + dummy_offset] | (rx_buf[2 + dummy_offset] << 8),
					start_register);
				if (len < T117_BYTES_READ_LIMIT) // normal register read retry
					dummy_offset = 0;
				else   // T117 should not retry
					return -EINVAL;
			} else if ((tx_buf[3] != rx_buf[3 + dummy_offset]) ||
					(tx_buf[4] != rx_buf[4 + dummy_offset])) {
				THP_LOG_ERR("%s: Unexpected count %d != %d reading from spi\n",
					__func__,
					rx_buf[3 + dummy_offset] | (rx_buf[4 + dummy_offset] << 8), len);
				if (len < T117_BYTES_READ_LIMIT) // normal register read retry
					dummy_offset = 0;
				else   // T117 should not retry
					return -EINVAL;
			}
		}
	} while ((get_header_crc(rx_buf + dummy_offset) !=
		rx_buf[SPI_APP_HEADER_LEN - 1 + dummy_offset]) ||
		(dummy_offset == 0));
	memcpy(val, rx_buf + SPI_APP_HEADER_LEN + dummy_offset, len);
	return 0;
}

static int __mxt_write_reg(struct thp_device *tdev,
		struct spi_device *client, u16 start_register,
		u16 len, const u8 *val)
{
	int i;
	int ret_val = -EINVAL;
	int attempt = 0;
	struct spi_message  spi_msg;
	struct spi_transfer transfer;

	u8 *rx_buf = NULL;
	u8 *tx_buf = NULL;
	int dummy_byte = WRITE_DUMMY_BYTE;
	int dummy_offset = 0;

	if ((tdev == NULL) || (client == NULL) || (val == NULL) ||
		(tdev->rx_buff == NULL) || (tdev->tx_buff == NULL)) {
		THP_LOG_ERR("%s:point is  null\n", __func__);
		return -EINVAL;
	}
	rx_buf = tdev->rx_buff;
	tx_buf = tdev->tx_buff;

	do {
		attempt++;
		if (attempt > 1) {
			if (attempt > 5) {
				THP_LOG_ERR("Too many spi write Retries\n");
				return -EIO;
			}
			THP_LOG_INFO("__mxt_write_reg retry %d after write fail\n",
				attempt - 1);
			mdelay(MXT_WAKEUP_TIME);
		}

		/* WRITE SPI_WRITE_REQ */

		memset(tx_buf, 0xFF, NUMBER_OF_HEADER * SPI_APP_HEADER_LEN
			+ dummy_byte);
		spi_prepare_header(tx_buf, SPI_WRITE_REQ, start_register, len);
		memcpy(tx_buf + SPI_APP_HEADER_LEN, val, len);
		spi_message_init(&spi_msg);
		memset(&transfer, 0,  sizeof(struct spi_transfer));
		transfer.tx_buf = tx_buf;
		transfer.rx_buf = rx_buf;
		transfer.len = NUMBER_OF_HEADER * SPI_APP_HEADER_LEN +
			dummy_byte + len;
		spi_message_add_tail(&transfer, &spi_msg);

		ret_val = thp_bus_lock();
		if (ret_val < 0) {
			THP_LOG_ERR("%s:get lock failed\n", __func__);
			continue;
		}
		ret_val = spi_sync(client, &spi_msg);
		thp_bus_unlock();
		if (ret_val < 0) {
			THP_LOG_ERR("%s:Error writing to spi\n", __func__);
			continue;
		}

		for (i = 0; i < dummy_byte; i++) {
			if (rx_buf[SPI_APP_HEADER_LEN + i] == SPI_WRITE_OK) {
				dummy_offset = i + SPI_APP_HEADER_LEN;
				if (dummy_offset > WRITE_DUMMY_BYTE / 2)
					THP_LOG_INFO("Found big write dummy offset %d\n",
							dummy_offset);
				break;
			}
		}

		if (dummy_offset) {
			// tx[1] or rx[1] :LSB of start register address
			// tx[2] or rx[2] :MSB of start register address
			// tx[3] or rx[3] :LSB of len
			// tx[4] or rx[4] :MSB of len
			if ((tx_buf[1] != rx_buf[1 + dummy_offset]) ||
				(tx_buf[2] != rx_buf[2 + dummy_offset])) {
				THP_LOG_ERR("Unexpected address %d != %d reading from spi\n",
						(rx_buf[1 + dummy_offset] |
						(rx_buf[2 + dummy_offset] << MOVE_8BIT)),
						start_register);
				dummy_offset = 0;
			} else if ((tx_buf[3] != rx_buf[3 + dummy_offset]) ||
					(tx_buf[4] != rx_buf[4 + dummy_offset])) {
				THP_LOG_ERR("Unexpected count %d != %d reading from spi\n",
						(rx_buf[3 + dummy_offset] |
						(rx_buf[4 + dummy_offset] << MOVE_8BIT)),
						len);
				dummy_offset = 0;
			}
		} else {
			THP_LOG_ERR("mxt write-Cannot found write dummy offset address = %d\n",
				start_register);
		}
	} while ((get_header_crc(rx_buf + dummy_offset) !=
		rx_buf[SPI_APP_HEADER_LEN - 1 + dummy_offset]) ||
		(dummy_offset == 0));
	return 0;
}

static int mxt_read_blks(struct thp_device *tdev,
		struct spi_device *client, u16 start, u16 count,
		u8 *buf, u16 override_limit)
{
	u16 offset = 0;
	int ret_val;
	u16 size;

	if ((tdev == NULL) || (client == NULL) || (buf == NULL)) {
		THP_LOG_ERR("%s:point is  null\n", __func__);
		return -EINVAL;
	}

	ret_val = thp_bus_lock();
	if (ret_val < 0) {
		THP_LOG_ERR("%s:get lock failed\n", __func__);
		return -EINVAL;
	}
	while (offset < count) {
		if (override_limit == 0)
			size = min(SPI_APP_DATA_MAX_LEN, count - offset);
		else
			size = min(override_limit, (u16)(count - offset));
		ret_val = __mxt_read_reg(tdev, client, (start + offset),
					size, (buf + offset));
		if (ret_val)
			break;
		offset += size;
	}
	thp_bus_unlock();

	return ret_val;
}


static int mxt_write_blks(struct thp_device *tdev,
		struct spi_device *client,
		u16 start, u16 count, u8 *buf)
{
	u16 offset = 0;
	int ret_val;
	u16 size;

	if ((tdev == NULL) || (client == NULL) || (buf == NULL)) {
		THP_LOG_ERR("%s:point is  null\n", __func__);
		return -EINVAL;
	}

	while (offset < count) {
		size = min(SPI_APP_DATA_MAX_LEN, count - offset);

		ret_val = __mxt_write_reg(tdev, client, start + offset, size,
			buf + offset);
		if (ret_val)
			break;
		offset += size;
	}

	return ret_val;
}

static int thp_ssl_init(struct thp_device *tdev)
{
	int rc;
	struct thp_core_data *cd = NULL;
	struct device_node *ssl_node = NULL;

	THP_LOG_INFO("%s: called\n", __func__);

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev is  null\n", __func__);
		return -EINVAL;
	}
	cd = tdev->thp_core;
	ssl_node = of_get_child_by_name(cd->thp_node, THP_SSL_DEV_NODE_NAME);
	if (ssl_node == NULL) {
		THP_LOG_INFO("%s: syna dev not config in dts\n", __func__);
		return -ENODEV;
	}

	THP_LOG_INFO("%s >>>\n", __func__);

	rc = thp_parse_spi_config(ssl_node, cd);
	if (rc)
		THP_LOG_ERR("%s: spi config parse fail\n", __func__);

	rc = thp_parse_timing_config(ssl_node, &tdev->timing_config);
	if (rc)
		THP_LOG_ERR("%s: timing config parse fail\n", __func__);

	rc = thp_parse_feature_config(ssl_node, cd);
	if (rc)
		THP_LOG_ERR("%s: feature_config fail\n", __func__);

	rc = thp_parse_trigger_config(ssl_node, cd);
	if (rc)
		THP_LOG_ERR("%s: trigger_config fail\n", __func__);
	if (cd->support_gesture_mode) {
		cd->easy_wakeup_info.sleep_mode = TS_POWER_OFF_MODE;
		cd->easy_wakeup_info.easy_wakeup_gesture = false;
		cd->easy_wakeup_info.off_motion_on = false;
	}
	return 0;
}

static int thp_ssl_get_project_id(struct thp_device *tdev, char *buf,
		unsigned int len)
{
	int ret = -EINVAL;
	int i;
	char buff[PROJECTID_ARR_LEN] = {0};
	int8_t retry = READ_ID_RETRY_TIMES;
	unsigned char write_value = GETPDSDATA_COMMAND;
	struct thp_core_data *cd = NULL;
	char *project_id = NULL;

	memset(buff, 0, sizeof(buff));

	if ((tdev == NULL) || (buf == NULL)) {
		THP_LOG_ERR("%s:tdev or buf is  null\n", __func__);
		return -EINVAL;
	}
	cd = tdev->thp_core;
	if (is_bootloader_mode) {
		project_id = cd->project_id_dummy;
		memcpy(buf, project_id, len);
		THP_LOG_INFO("The project id is set to %s in bootloader mode\n", project_id);
		return 0;
	}

	if ((t37_address == 0) || (t6_address == 0) ||
		(len > (sizeof(buff) - PDS_HEADER_OFFSET - 1))) {
		THP_LOG_ERR("project ID address or size check fail,read len=%d\n", len);
		return ret;
	}
	do {
		ret = mxt_write_blks(tdev, tdev->thp_core->sdev,
			t6_address + MXT_T6_DIAGNOSTIC_OFFSET, 1, &write_value);
		if (ret != 0)
			THP_LOG_ERR("Failed to send T6 diagnositic command\n");
		mdelay(3 + READ_ID_RETRY_TIMES - retry);

		ret = mxt_read_blks(tdev, tdev->thp_core->sdev, t37_address,
			len + PDS_HEADER_OFFSET, buff, 0);
		for (i = 0; i < len; i++)
			THP_LOG_INFO(" [%2x] ", buff[i]);

		if ((ret != 0) || (buff[0] != 0x81) || (buff[1] != 0) ||
			(buff[2] != 0x24) || (buff[PDS_HEADER_OFFSET] == 0)) {
			THP_LOG_ERR("Failed to read T37 data for project ID\n");
		} else {
			THP_LOG_INFO("read T37 data for project ID\n");
			break;
		}
	} while (retry-- > 0);

	if (retry < 0) {
		THP_LOG_ERR("read T37 data for projecd ID timeout\n");
		return -1;
	}
	memcpy(buf, buff + PDS_HEADER_OFFSET, len);
	THP_LOG_INFO("the project id is %s\n", buff + PDS_HEADER_OFFSET);
	return 0;
}

static void thp_ssl_update_addr(struct mxt_object *object)
{
	if (object == NULL) {
		THP_LOG_ERR("[%s] object is  NULL\n", __func__);
		return;
	}
	switch (object->type) {
	case SSL_T117_ADDR:
		t117_address = object->start_address;
		break;
	case SSL_T7_ADDR:
		t7_address = object->start_address;
		break;
	case SSL_T6_ADDR:
		t6_address = object->start_address;
		break;
	case SSL_T37_ADDR:
		t37_address = object->start_address;
		break;
	case SSL_T118_ADDR:
		t118_address = object->start_address;
		break;
	case SSL_T24_ADDR:
		t24_address = object->start_address;
		break;
	case SSL_T8_ADDR:
		t8_address = object->start_address;
		break;
	case SSL_T145_ADDR:
		t145_address = object->start_address;
		break;
	case SSL_T144_ADDR:
		t144_address = object->start_address;
		break;
	default:
		THP_LOG_DEBUG("%s: unused reg address\n", __func__);
		break;
	}
}

static int thp_ssl_update_obj_addr(struct thp_device *tdev)
{
	int ret_val;
	int i;
	struct mxt_info mxtinfo;
	u8 *buff = NULL;
	size_t curr_size;
	struct mxt_object *object_table = NULL;
	struct mxt_object *object = NULL;

	THP_LOG_INFO("%s: called\n", __func__);
	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev is  null\n", __func__);
		return -EINVAL;
	}

	curr_size = MXT_OBJECT_START;

	ret_val = mxt_read_blks(tdev, tdev->thp_core->sdev, 0, curr_size,
				(u8 *)&mxtinfo, 0);
	if (ret_val) {
		THP_LOG_ERR("mxt_read_blks--info block reading error\n");
		return -EINVAL;
	}

	curr_size += mxtinfo.object_num * sizeof(struct mxt_object) +
			MXT_INFO_CHECKSUM_SIZE;
	buff = (u8 *)kzalloc(curr_size, GFP_KERNEL);
	if (buff == NULL) {
		THP_LOG_ERR("%s:buff alloc faild\n", __func__);
		return -ENOMEM;
	}

	ret_val = mxt_read_blks(tdev, tdev->thp_core->sdev,
				MXT_OBJECT_START, curr_size, buff, 0);

	if (ret_val) {
		THP_LOG_ERR("mxt_read_blks--info block reading error\n");
		goto error_free;
	}

	t117_address = 0;
	object_table = (struct mxt_object *)(buff);
	for (i = 0; i < mxtinfo.object_num; i++) {
		object = object_table + i;
		le16_to_cpus(&object->start_address);
		thp_ssl_update_addr(object);
	}
	THP_LOG_INFO("t117_address updated [%d]\n", t117_address);
	tdev->thp_core->frame_data_addr = t117_address;
	THP_LOG_INFO("%s:frame_data_addr %d\n", __func__,
			tdev->thp_core->frame_data_addr);
error_free:
	kfree(buff);
	return ret_val;
}

static int mxt_power_init(struct thp_device *tdev)
{
	int ret;

	THP_LOG_INFO("%s: called\n", __func__);

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev is  null\n", __func__);
		return -EINVAL;
	}
	ret = thp_power_supply_get(THP_VCC);
	ret |= thp_power_supply_get(THP_IOVDD);
	if (ret)
		THP_LOG_ERR("%s: fail to get power\n", __func__);

	return 0;
}

static int mxt_power_on(struct thp_device *tdev)
{
	int ret;

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev is  null\n", __func__);
		return -EINVAL;
	}
	gpio_direction_output(tdev->gpios->rst_gpio, GPIO_LOW);
	mdelay(1);
	ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_ON, 1);
	ret |= thp_power_supply_ctrl(THP_VCC, THP_POWER_ON, 1);
	if (ret)
		THP_LOG_ERR("%s:power on ctrl fail\n", __func__);
	THP_LOG_INFO("%s pull up tp ic reset\n", __func__);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
	return ret;
}

static int mxt_power_off(struct thp_device *tdev)
{
	int ret;

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev is  null\n", __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("%s pull down tp ic reset\n", __func__);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_OFF, 0);
	ret |= thp_power_supply_ctrl(THP_VCC, THP_POWER_OFF, 1);
	if (ret)
		THP_LOG_ERR("%s:power off ctrl fail\n", __func__);
	return ret;
}

static int thp_ssl_chip_detect(struct thp_device *tdev)
{
	int ret_val;
	int i;
	struct mxt_info mxtinfo;
	u8 *buff = NULL;
	size_t curr_size;
	struct mxt_object *object_table = NULL;
	int retry_time = READ_ID_RETRY_TIMES;
	struct mxt_object *object = NULL;

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev is  null\n", __func__);
		return -EINVAL;
	}
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	THP_LOG_INFO("%s: called\n", __func__);
	ret_val = mxt_power_init(tdev);
	if (ret_val)
		THP_LOG_ERR("mxt_power_init fails\n");
	curr_size = MXT_OBJECT_START;
	ret_val = mxt_power_on(tdev);
	if (ret_val)
		THP_LOG_ERR("mxt_power_on fails\n");

	thp_do_time_delay(tdev->timing_config.boot_reset_after_delay_ms);

	do {
		ret_val = mxt_read_blks(tdev, tdev->thp_core->sdev, 0,
					sizeof(mxtinfo), (u8 *)&mxtinfo, 0);

		if (ret_val) {
			THP_LOG_ERR("mxt_read_blks info- read device ID fails\n");
			continue;
		}

		if ((mxtinfo.family_id != MXT680U2_FAMILY_ID) ||
			(mxtinfo.variant_id != MXT680U2_VARIANT_ID)) {
			THP_LOG_ERR("%s: chip is not identified %d, %d\n",
				__func__, mxtinfo.family_id, mxtinfo.variant_id);
			THP_LOG_INFO("retry time is %d\n", retry_time);
		} else {
			THP_LOG_INFO("%s: Chip is identified OK!!! %d, %d\n",
				__func__, mxtinfo.family_id, mxtinfo.variant_id);
		}

	} while ((retry_time-- > 0) &&
		(mxtinfo.family_id != MXT680U2_FAMILY_ID));


	if ((mxtinfo.family_id != MXT680U2_FAMILY_ID) ||
		(mxtinfo.variant_id != MXT680U2_VARIANT_ID)) {
		if (mxt_check_is_bootloader(tdev)) {
			is_bootloader_mode = 1;
			t117_address = SSL_SYNC_DATA_REG_DEFALUT_ADDR; // set default value
			return 0;
		}
		THP_LOG_ERR("chip is not identified, try to check bootloader mode\n");
		return -ENODEV;
	}

	curr_size += mxtinfo.object_num * sizeof(struct mxt_object) +
			MXT_INFO_CHECKSUM_SIZE;
	buff = (u8 *)kzalloc(curr_size, GFP_KERNEL);

	if (buff == NULL) {
		THP_LOG_ERR("%s:buff  alloc faild\n", __func__);
		ret_val = -ENOMEM;
		goto error_free;
	}
	ret_val = mxt_read_blks(tdev, tdev->thp_core->sdev,
				MXT_OBJECT_START, curr_size, buff, 0);
	if (ret_val) {
		THP_LOG_ERR("mxt_read_blks--info block reading error\n");
		goto error_free;
	}

	object_table = (struct mxt_object *)(buff);
	for (i = 0; i < mxtinfo.object_num; i++) {
		object = object_table + i;
		// object start_address from little endian to local CPU
		// endianness **IN PLACE**
		// IMPORTANT: this is only for the first loop through
		// the object table
		le16_to_cpus(&object->start_address);
		thp_ssl_update_addr(object);
	}

	if (t117_address == 0) {
		t117_address = SSL_SYNC_DATA_REG_DEFALUT_ADDR;
		THP_LOG_ERR("Got t117_address fails, set to default value %d\n",
			t117_address);
	}

	tdev->thp_core->frame_data_addr = t117_address;
	THP_LOG_INFO("%s tui_tp_addr %d\n", __func__,
		tdev->thp_core->frame_data_addr);

error_free:
	kfree(buff);
	return ret_val;
}

static int thp_ssl_get_frame(struct thp_device *tdev, char *buf, unsigned int len)
{
	int ret_val;

	if ((tdev == NULL) || (buf == NULL)) {
		THP_LOG_INFO("%s: input dev or buf null\n", __func__);
		return -ENOMEM;
	}

	if (!len) {
		THP_LOG_INFO("%s: read len illegal\n", __func__);
		return -ENOMEM;
	}
	ret_val = mxt_read_blks(tdev, tdev->thp_core->sdev, t117_address, len,
				buf, T117_BYTES_READ_LIMIT);
	return ret_val;
}
static int thp_ssl_set_screen_off_mode(struct thp_device *tdev)
{
	int ret_val;

	u8 t7_idle_scan_rate = SCREEN_OFF_MODE_SCAN_RATE;
	u8 t8_scan_mode[2] = { SCREEN_OFF_SCAN_MODE, SCREEN_OFF_MCT_SCAN_MODE };
	u8 t145_cmd = MXT_T145_SCREEN_OFF_CMD;

	if (tdev == NULL) {
		THP_LOG_ERR("%s: input dev null\n", __func__);
		return -ENOMEM;
	}

	if ((t7_address == 0) || (t8_address == 0) || (t145_address == 0)) {
		THP_LOG_ERR("Object address for screen off is not correct\n");
		return -ENOMEM;
	}

	ret_val = mxt_write_blks(tdev, tdev->thp_core->sdev, t7_address,
				1, &t7_idle_scan_rate);
	ret_val |= mxt_write_blks(tdev, tdev->thp_core->sdev,
				t8_address + T8_MEASIDLEDEF_OFFSET,
				2, t8_scan_mode);
	ret_val |= mxt_write_blks(tdev, tdev->thp_core->sdev,
				t145_address + MXT_T145_CMD_OFFSET,
				1, &t145_cmd);

	THP_LOG_INFO("mxt set screen off mode done\n");
	return ret_val;
}

static int thp_ssl_get_active_idle_timer(struct thp_device *tdev,
		uint32_t *active_time, uint32_t *dozing_time)
{
	int ret_val;
	u8 buf[T144_ACTIVE_DOZING_READ_LEN] = {0};

	if (tdev == NULL) {
		THP_LOG_ERR("%s: input dev null\n", __func__);
		return -ENOMEM;
	}

	if (t144_address == 0) {
		THP_LOG_ERR("T144 address is not correct\n");
		return -ENOMEM;
	}
	ret_val = mxt_read_blks(tdev, tdev->thp_core->sdev, t144_address +
				T144_ACTIVE_TIMER_OFFSET,
				T144_ACTIVE_DOZING_READ_LEN, buf, 0);
	if (ret_val) {
		THP_LOG_ERR("mxt_read_blks--info block reading error\n");
		ret_val = -ENOMEM;
	}
	// use buf[0],buf[1],buf[2],buf[3] to concatenate active time
	// use buf[4],buf[5],buf[6],buf[7] to concatenate dozing time
	*active_time = (buf[0] << MOVE_24BIT) | (buf[1] << MOVE_16BIT) |
			(buf[2] << MOVE_8BIT) | buf[3];
	*dozing_time = (buf[4] << MOVE_24BIT) | (buf[5] << MOVE_16BIT) |
			(buf[6] << MOVE_8BIT) | buf[7];

	THP_LOG_INFO("%s: Active time[%d], Dozing time[%d]\n", __func__,
		*active_time, *dozing_time);
	return ret_val;
}

static int thp_ssl_resume(struct thp_device *tdev)
{
	int ret = 0;
	uint32_t active_time = 0;
	uint32_t dozing_time = 0;

	THP_LOG_INFO("%s: called\n", __func__);
	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev is  null\n", __func__);
		return -EINVAL;
	}
	if (is_pt_test_mode(tdev)) {
		gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
		mdelay(tdev->timing_config.resume_reset_after_delay_ms);
		gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
	} else if (g_thp_udfp_stauts ||
		(tdev->thp_core->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE)) {
		THP_LOG_INFO("get_active_idle_timer\n");
		ret = thp_ssl_get_active_idle_timer(tdev, &active_time,
							&dozing_time);
		if (ret)
			THP_LOG_ERR("%s: thp_ssl_get_active_idle_timer failed\n",
				__func__);
#ifdef CONFIG_HUAWEI_DUBAI
		HWDUBAI_LOGE("DUBAI_TAG_TP_DURATION", "active=%d dozing=%d",
					active_time, dozing_time);
#endif
		THP_LOG_INFO("%s TS_GESTURE_MODE or tp_ud enable ,so reset\n",
			__func__);
		gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
		mdelay(tdev->timing_config.resume_reset_after_delay_ms);
		gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
	} else {
		ret = mxt_power_on(tdev);
	}
	THP_LOG_INFO("%s: called end\n", __func__);
	return ret;
}

static int thp_ssl_after_resume(struct thp_device *tdev)
{
	int ret = 0;

	THP_LOG_INFO("%s: called\n", __func__);
	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	if (!g_thp_udfp_stauts)
		thp_do_time_delay(tdev->timing_config.boot_reset_after_delay_ms);
	return ret;
}

static int pt_mode_set(struct thp_device *tdev)
{
	int ret;
	u8 t7_active[SSL_T7_COMMAMD_LEN] = { 8, 8 }; // pt station cmd

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	if (t7_address == 0) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	ret = mxt_write_blks(tdev, tdev->thp_core->sdev, t7_address,
			SSL_T7_COMMAMD_LEN, t7_active);
	if (ret != 0) {
		THP_LOG_ERR("Failed to send T7 always active command\n");
		return -EINVAL;
	}
	return ret;
}

static int thp_ssl_suspend(struct thp_device *tdev)
{
	int ret = -EINVAL;

	THP_LOG_INFO("%s: called\n", __func__);

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev is  null\n", __func__);
		return -EINVAL;
	}

	g_thp_udfp_stauts = thp_get_status(THP_STAUTS_UDFP);

	if (is_pt_test_mode(tdev)) {
		THP_LOG_INFO("%s: suspend PT mode\n", __func__);
		ret = pt_mode_set(tdev);
		if (ret != 0)
			THP_LOG_ERR("Failed to send T7 always active command\n");
	} else if (g_thp_udfp_stauts ||
		(tdev->thp_core->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE)) {
		if (tdev->thp_core->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE) {
			THP_LOG_INFO("%s TS_GESTURE_MODE\n", __func__);
			ret = ssl_wakeup_gesture_enable_switch(tdev, 1);
			if (ret != 0)
				THP_LOG_ERR("Failed to send wakeup gesture enable command\n");
			mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
			tdev->thp_core->easy_wakeup_info.off_motion_on = true;
			mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
		}
		ret = thp_ssl_set_screen_off_mode(tdev);
		if (ret != 0)
			THP_LOG_ERR("Failed to set_screen_off_mode command\n");
	} else {
		ret = mxt_power_off(tdev);
		THP_LOG_INFO("enter poweroff mode\n");
	}
	THP_LOG_INFO("%s: called end\n", __func__);
	return ret;
}



static void thp_ssl_exit(struct thp_device *tdev)
{
	THP_LOG_INFO("%s: called\n", __func__);
	if (tdev != NULL) {
		if (tdev->tx_buff != NULL) {
			kfree(tdev->tx_buff);
			tdev->tx_buff = NULL;
		}
		if (tdev->rx_buff != NULL) {
			kfree(tdev->rx_buff);
			tdev->rx_buff = NULL;
		}
		kfree(tdev);
		tdev = NULL;
	}
}

static int thp_ssl_afe_notify_callback(struct thp_device *tdev, unsigned long event)
{
	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	return thp_ssl_update_obj_addr(tdev);
}

static int thp_ssl_set_fw_update_mode(struct thp_device *tdev,
		struct thp_ioctl_set_afe_status set_afe_status)
{
	int rc = -EINVAL;

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	if (set_afe_status.status == THP_AFE_FW_UPDATE_SET_SPI_COM_MODE)
		rc = thp_set_spi_com_mode(tdev->thp_core, set_afe_status.parameter);

	return rc;
}

#define SSL_SCREEN_OFF_MODE 0x04
static void thp_ssl_check_screen_off_mode(struct thp_device *tdev)
{
	int ret_val;
	u8 buffer = 0;

	if (!tdev) {
		THP_LOG_INFO("%s: input dev null\n", __func__);
		return;
	}

	if (!t118_address) {
		THP_LOG_INFO("%s: invalid T118 frame data address\n", __func__);
		return;
	}

	ret_val = mxt_read_blks(tdev, tdev->thp_core->sdev,
		t118_address, 0, &buffer, 0);

	if (!ret_val) {
		if ((buffer & SSL_SCREEN_OFF_MODE) != SSL_SCREEN_OFF_MODE) {
			THP_LOG_ERR("%s: TPIC is NOT in screen off mode [%d], try to set screen off mode\n",
				__func__, buffer);

			ret_val = ssl_wakeup_gesture_enable_switch(tdev, 1);
			if (ret_val != 0)
				THP_LOG_ERR("Failed to send wakeup gesture enable command\n");

			ret_val = thp_ssl_set_screen_off_mode(tdev);
			if (ret_val != 0)
				THP_LOG_ERR("Failed to set_screen_off_mode command\n");
		} else {
			THP_LOG_INFO("%s: TPIC is in screen off mode [%d]\n",
				__func__, buffer);
		}
	}
}

// add this function in ISR to get the double tap gesture event
// buffer=0 -> no event   buffer = 0x40 -> double tap gesture event
#define GESTRUE_EVENT_RETRY_TIME 10
int thp_ssl_check_gesture_event(struct thp_device *tdev,
		unsigned int *gesture_wakeup_value)
{
	int ret_val;
	u8 buffer[T117_HEADER_STATUS_OFFSET + 2] = {0};
	int i;

	if (tdev == NULL) {
		THP_LOG_INFO("%s: input dev null\n", __func__);
		return -ENOMEM;
	}

	if (!t117_address) {
		THP_LOG_INFO("%s: invalid T117 frame data address\n", __func__);
		return -ENOMEM;
	}

	THP_LOG_INFO("[%s]\n", __func__);
	/* wait spi bus resume */
	msleep(SSL_WAIT_FOR_SPI_BUS_RESUMED_DELAY);
	for (i = 0; i < GESTRUE_EVENT_RETRY_TIME; i++) {
		ret_val = mxt_read_blks(tdev, tdev->thp_core->sdev,
					t117_address,
					T117_HEADER_STATUS_OFFSET + 1,
					buffer, 0);
		if (ret_val == 0) {
			break;
		} else {
			THP_LOG_ERR("%s: spi not work normal, ret %d\n",
				__func__, ret_val);
			msleep(SSL_WAIT_FOR_SPI_BUS_READ_DELAY);
		}
	}
	if (!ret_val) {
		if ((buffer[T117_HEADER_STATUS_OFFSET] & T117_GESTURE_EVENT) ==
			T117_GESTURE_EVENT) {
			THP_LOG_INFO("THP found double tap gesture [%x]\n",
					buffer[T117_HEADER_STATUS_OFFSET]);
			mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
			if (tdev->thp_core->easy_wakeup_info.off_motion_on == true) {
				tdev->thp_core->easy_wakeup_info.off_motion_on = false;
				*gesture_wakeup_value = TS_DOUBLE_CLICK;
			}
			mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
		} else {
			THP_LOG_ERR("NO gesture event found status[%x, %x]\n",
				buffer[T117_HEADER_STATUS_OFFSET],
				buffer[T117_HEADER_STATUS_OFFSET + 1]);
			thp_ssl_check_screen_off_mode(tdev);
		}
	}
	return ret_val;
}

/* enable=1: to enable gesture function, enable=0: to disable gesture function */
static int ssl_wakeup_gesture_enable_switch(struct thp_device *tdev,
				u8 switch_value)
{
	int ret_val;
	u8 t24_ctrl = 0;
	u16 retry = 0;

	if (tdev == NULL) {
		THP_LOG_INFO("%s: input dev null\n", __func__);
		return -EINVAL;
	}
	if (!t24_address) {
		THP_LOG_INFO("%s: invalid T24 frame data address\n", __func__);
		return -ENOMEM;
	}
	while (retry++ < SEND_COMMAND_RETRY) {
		ret_val = mxt_read_blks(tdev, tdev->thp_core->sdev, t24_address,
					1, &t24_ctrl, 0);
		if (!ret_val)
			t24_ctrl = switch_value ?
			(t24_ctrl | T24_GESTURE_ON_MASK) :
			(t24_ctrl & (~T24_GESTURE_ON_MASK));
		ret_val |= mxt_write_blks(tdev, tdev->thp_core->sdev,
					t24_address, 1, &t24_ctrl);
		THP_LOG_INFO("[%s] t24_ctrl-> 0x%x|switch_value = %d\n",
				__func__, t24_ctrl, switch_value);
		if (ret_val == 0)
			break;
	}
	return ret_val;
}

static int ssl_wrong_touch(struct thp_device *tdev)
{
	if (!tdev) {
		THP_LOG_ERR("%s: input dev null\n", __func__);
		return -EINVAL;
	}
	if (tdev->thp_core->support_gesture_mode) {
		mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
		tdev->thp_core->easy_wakeup_info.off_motion_on = true;
		mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
		THP_LOG_INFO("%s: done\n", __func__);
	}
	return 0;
}

static int ssl_gesture_report(struct thp_device *tdev,
					 unsigned int *gesture_wakeup_value)
{
	int retval;

	retval = thp_ssl_check_gesture_event(tdev, gesture_wakeup_value);
	if (retval != 0) {
		THP_LOG_ERR("[%s] retval-> %d\n", __func__, retval);
		return -EINVAL;
	}
	return 0;
}


struct thp_device_ops ssl_dev_ops = {
	.init = thp_ssl_init,
	.detect = thp_ssl_chip_detect,
	.get_frame = thp_ssl_get_frame,
	.resume = thp_ssl_resume,
	.after_resume = thp_ssl_after_resume,
	.suspend = thp_ssl_suspend,
	.get_project_id = thp_ssl_get_project_id,
	.exit = thp_ssl_exit,
	.afe_notify = thp_ssl_afe_notify_callback,
	.set_fw_update_mode = thp_ssl_set_fw_update_mode,
	.chip_wakeup_gesture_enable_switch = ssl_wakeup_gesture_enable_switch,
	.chip_wrong_touch = ssl_wrong_touch,
	.chip_gesture_report = ssl_gesture_report,
};

static int __init thp_ssl_module_init(void)
{
	int rc;
	struct thp_device *dev = NULL;

	THP_LOG_INFO("%s: called\n", __func__);
	dev = kzalloc(sizeof(struct thp_device), GFP_KERNEL);
	if (dev == NULL) {
		THP_LOG_ERR("%s: thp device malloc fail\n", __func__);
		return -ENOMEM;
	}

	dev->tx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	dev->rx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	if ((dev->tx_buff == NULL) || (dev->rx_buff == NULL)) {
		THP_LOG_ERR("%s: out of memory\n", __func__);
		rc = -ENOMEM;
		goto err;
	}

	dev->ic_name = SSL_IC_NAME;
	dev->ops = &ssl_dev_ops;

	rc = thp_register_dev(dev);
	if (rc) {
		THP_LOG_ERR("%s: register fail\n", __func__);
		goto err;
	} else {
		THP_LOG_INFO("%s: register success\n", __func__);
	}

	return rc;
err:
	thp_ssl_exit(dev);
	return rc;
}

static void __exit thp_ssl_module_exit(void)
{
	THP_LOG_ERR("%s: called\n", __func__);
};

module_init(thp_ssl_module_init);
module_exit(thp_ssl_module_exit);
