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
#include "huawei_thp.h"

#include <linux/time.h>
#include <linux/syscalls.h>

#define GOODIX_IC_NAME "goodix"
#define THP_GOODIX_DEV_NODE_NAME "goodix_thp"

#define MODULE_INFO_READ_RETRY       5
#define FW_INFO_READ_RETRY           3
#define SEND_COMMAND_RETRY           3

#define GOODIX_FRAME_ADDR_DEFAULT      0x8C05
#define GOODIX_CMD_ADDR_DEFAULT        0x58BF
#define GOODIX_FW_STATE_ADDR_DEFAULT   0xBFDE
#define GOODIX_FW_STATE_LEN_DEFAULT    10
#define GOODIX_MODULE_INFO_ADDR        0x452C
#define GOODIX_PROJECT_ID_ADDR         0xBDB4
#define GOODIX_AFE_INFO_ADDR           0x6D20
#define GOODIX_MAX_AFE_LEN             300
#define GOODIX_FRAME_LEN_OFFSET        20
#define GOODIX_FRAME_ADDR_OFFSET       22
#define GOODIX_FW_STATUS_LEN_OFFSET    91
#define GOODIX_FW_STATUS_ADDR_OFFSET   93
#define GOODIX_CMD_ADDR_OFFSET         102

#define CMD_ACK_BUF_OVERFLOW    0x01
#define CMD_ACK_CHECKSUM_ERROR  0x02
#define CMD_ACK_BUSY            0x04
#define CMD_ACK_OK              0x80
#define CMD_ACK_IDLE            0xFF

#define CMD_FRAME_DATA          0x90
#define CMD_HOVER               0x91
#define CMD_FORCE               0x92
#define CMD_SLEEP               0x96
#define CMD_SCREEN_ON_OFF       0x96

#define CMD_PT_MODE             0x05
#define CMD_GESTURE_MODE        0xB5

#define IC_STATUS_GESTURE       0x01
#define IC_STATUS_POWEROF       0x02
#define IC_STATUS_UDFP          0x04
#define IC_STATUS_PT_TEST       0x08

#define PT_MODE                 0

#define FEATURE_ENABLE          1
#define FEATURE_DISABLE         0

#define SPI_FLAG_WR             0xF0
#define SPI_FLAG_RD             0xF1
#define MASK_8BIT               0xFF
#define MASK_1BIT               0x01

#define GOODIX_MASK_ID           "NOR_G1"
#define GOODIX_MASK_ID_LEN       6
#define MOVE_8BIT                8
#define MOVE_16BIT               16
#define MOVE_24BIT               24
#define MAX_FW_STATUS_DATA_LEN   64

#define SEND_COMMAND_WAIT_DELAY  10
#define SEND_COMMAND_END_DELAY    2
#define SEND_COMMAND_SPI_READ_LEN 1
#define COMMAND_BUF_LEN           5
#define COMMUNI_CHECK_RETRY_DELAY 10
#define DELAY_1MS                 1
#define NO_DELAY                  0
#define GET_AFE_INFO_RETRY_DELAY 10
#define DEBUG_AFE_DATA_BUF_LEN    20
#define DEBUG_AFE_DATA_BUF_OFFSET DEBUG_AFE_DATA_BUF_LEN

#define INFO_ADDR_BUF_LEN          2
#define IRQ_EVENT_TYPE_FRAME       0x80
#define IRQ_EVENT_TYPE_GESTURE     0x81

#define GESTURE_EVENT_HEAD_LEN               6
#define GESTURE_TYPE_DOUBLE_TAP              0x01
#define GOODIX_CUSTOME_INFO_LEN              30
#define GOODIX_GET_PROJECT_ID_RETRY          3
#define GOODIX_GET_PROJECT_RETRY_DELAY_10MS  10
#define IDLE_WAKE_FLAG                       0xF0
#define IDLE_SPI_SPEED                       3500
#define ACTIVE_SPI_SPEED                     7500000
#define READ_WRITE_BYTE_OFFSET               1
#define READ_CMD_BUF_LEN                     3
#define CMD_TOUCH_REPORT                     0xAC
#define WAIT_FOR_SPI_BUS_RESUMED_DELAY       20
#define WAIT_FOR_SPI_BUS_READ_DELAY          5
#define GOODIX_SPI_READ_XFER_LEN             2
#define SPI_WAKEUP_RETRY                     8
#define GOODIX_LCD_EFFECT_FLAG_POSITION 12

#pragma pack(1)
struct goodix_module_info {
	u8 mask_id[6];
	u8 mask_vid[3];
	u8 pid[8];
	u8 vid[4];
	u8 sensor_id;
	u8 reserved[49];
	u8 checksum;
};
#pragma pack()

struct goodix_module_info module_info;
static int goodix_frame_addr = GOODIX_FRAME_ADDR_DEFAULT;
static int goodix_frame_len;
static int goodix_cmd_addr = GOODIX_CMD_ADDR_DEFAULT;
static int goodix_fw_status_addr = GOODIX_FW_STATE_ADDR_DEFAULT;
static int goodix_fw_status_len = GOODIX_FW_STATE_LEN_DEFAULT;
static unsigned int g_thp_udfp_stauts;
static int work_status;

static int thp_goodix_gesture_event(struct thp_device *tdev,
		unsigned int *gesture_wakeup_value);
static int thp_goodix_switch_cmd(struct thp_device *tdev, u8 cmd, u8 status);
int thp_goodix_spi_active(struct thp_device *tdev);
static int thp_goodix_idle_wakeup(struct thp_device *tdev);
static int thp_goodix_spi_read(struct thp_device *tdev, unsigned int addr,
			   u8 *data, unsigned int len)
{
	struct spi_device *spi = NULL;
	u8 *rx_buf = NULL;
	u8 *tx_buf = NULL;
	u8 start_cmd_buf[READ_CMD_BUF_LEN];
	struct spi_transfer xfers[GOODIX_SPI_READ_XFER_LEN];
	struct spi_message spi_msg;
	int ret;

	if ((tdev == NULL) || (data == NULL) || (tdev->tx_buff == NULL) ||
		(tdev->rx_buff == NULL) || (tdev->thp_core == NULL) ||
		(tdev->thp_core->sdev == NULL)) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	if (len + READ_WRITE_BYTE_OFFSET > THP_MAX_FRAME_SIZE) {
		THP_LOG_ERR("%s:invalid len:%d\n", __func__, len);
		return -EINVAL;
	}
	spi = tdev->thp_core->sdev;
	rx_buf = tdev->rx_buff;
	tx_buf = tdev->tx_buff;

	spi_message_init(&spi_msg);
	memset(xfers, 0, sizeof(xfers));

	start_cmd_buf[0] = SPI_FLAG_WR; // 0xF0 start write flag
	start_cmd_buf[1] = (addr >> MOVE_8BIT) & MASK_8BIT;
	start_cmd_buf[2] = addr & MASK_8BIT;

	xfers[0].tx_buf = start_cmd_buf;
	xfers[0].len = READ_CMD_BUF_LEN;
	xfers[0].cs_change = 1;
	spi_message_add_tail(&xfers[0], &spi_msg);

	tx_buf[0] = SPI_FLAG_RD; // 0xF1 start read flag
	xfers[1].tx_buf = tx_buf;
	xfers[1].rx_buf = rx_buf;
	xfers[1].len = len + READ_WRITE_BYTE_OFFSET;
	xfers[1].cs_change = 1;
	spi_message_add_tail(&xfers[1], &spi_msg);
	ret = thp_bus_lock();
	if (ret < 0) {
		THP_LOG_ERR("%s:get lock failed:%d\n", __func__, ret);
		return ret;
	}
	ret = thp_spi_sync(spi, &spi_msg);
	thp_bus_unlock();
	if (ret < 0) {
		THP_LOG_ERR("Spi transfer error:%d\n", ret);
		return ret;
	}
	memcpy(data, &rx_buf[READ_WRITE_BYTE_OFFSET], len);

	return ret;
}

static int thp_goodix_spi_write(struct thp_device *tdev,
		unsigned int addr, u8 *data, unsigned int len)
{
	struct spi_device *spi = NULL;
	u8 *tx_buf = NULL;
	struct spi_transfer xfers;
	struct spi_message spi_msg;
	int ret;

	if ((tdev == NULL) || (data == NULL) || (tdev->tx_buff == NULL) ||
		(tdev->rx_buff == NULL) || (tdev->thp_core == NULL) ||
		(tdev->thp_core->sdev == NULL)) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	spi = tdev->thp_core->sdev;
	tx_buf = tdev->tx_buff;

	spi_message_init(&spi_msg);
	memset(&xfers, 0, sizeof(xfers));

	if (addr) {
		tx_buf[0] = SPI_FLAG_WR; // 0xF1 start read flag
		tx_buf[1] = (addr >> MOVE_8BIT) & MASK_8BIT;
		tx_buf[2] = addr & MASK_8BIT;
		memcpy(&tx_buf[3], data, len);
		xfers.len = len + 3;
	} else {
		memcpy(&tx_buf[0], data, len);
		xfers.len = len;
	}

	xfers.tx_buf = tx_buf;
	xfers.cs_change = 1;
	spi_message_add_tail(&xfers, &spi_msg);
	ret = thp_bus_lock();
	if (ret < 0) {
		THP_LOG_ERR("%s:get lock failed:%d\n", __func__, ret);
		return ret;
	}
	ret = thp_spi_sync(spi, &spi_msg);
	thp_bus_unlock();
	if (ret < 0)
		THP_LOG_ERR("Spi transfer error:%d, addr 0x%x\n", ret, addr);

	return ret;
}
#define CMD_ACK_OFFSET 4
static int thp_goodix_switch_cmd(struct thp_device *tdev,
		u8 cmd, u8 status)
{
	int ret = 0;
	int retry = 0;
	u8 cmd_buf[COMMAND_BUF_LEN];
	u8 cmd_ack = 0;

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	if (thp_goodix_idle_wakeup(tdev)) {
		THP_LOG_ERR("failed wakeup idle before send command\n");
		return -EINVAL;
	}
	cmd_buf[0] = cmd;
	cmd_buf[1] = status;
	cmd_buf[2] = 0 - cmd_buf[1] - cmd_buf[0]; /* checksum */
	cmd_buf[3] = 0;
	cmd_buf[4] = 0; // use to clear cmd ack flag
	while (retry++ < SEND_COMMAND_RETRY) {
		ret = thp_goodix_spi_write(tdev, goodix_cmd_addr,
					cmd_buf, sizeof(cmd_buf));
		if (ret < 0) {
			THP_LOG_ERR("%s: failed send command, ret %d\n",
				__func__, ret);
			return -EINVAL;
		}

		ret = thp_goodix_spi_read(tdev,
					goodix_cmd_addr + CMD_ACK_OFFSET,
					&cmd_ack, SEND_COMMAND_SPI_READ_LEN);
		if (ret < 0) {
			THP_LOG_ERR("%s: failed read command ack info, ret %d\n",
				__func__, ret);
			return -EINVAL;
		}

		if (cmd_ack != CMD_ACK_OK) {
			ret = -EINVAL;
			THP_LOG_DEBUG("%s: command state ack info 0x%x\n",
					__func__, cmd_ack);
			if (cmd_ack == CMD_ACK_BUF_OVERFLOW ||
				cmd_ack == CMD_ACK_BUSY)
				mdelay(SEND_COMMAND_WAIT_DELAY);
		} else {
			ret = 0;
			mdelay(SEND_COMMAND_END_DELAY);
			break;
		}
	}
	return ret;
}

static int thp_goodix_init(struct thp_device *tdev)
{
	int rc;
	struct thp_core_data *cd = NULL;
	struct device_node *goodix_node = NULL;

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	cd = tdev->thp_core;
	goodix_node = of_get_child_by_name(cd->thp_node,
					THP_GOODIX_DEV_NODE_NAME);

	THP_LOG_INFO("%s: called\n", __func__);

	if (!goodix_node) {
		THP_LOG_INFO("%s: goodix dev not config in dts\n", __func__);
		return -ENODEV;
	}

	rc = thp_parse_spi_config(goodix_node, cd);
	if (rc)
		THP_LOG_ERR("%s: spi config parse fail\n", __func__);

	rc = thp_parse_timing_config(goodix_node, &tdev->timing_config);
	if (rc)
		THP_LOG_ERR("%s: timing config parse fail\n", __func__);

	rc = thp_parse_feature_config(goodix_node, cd);
	if (rc)
		THP_LOG_ERR("%s: feature_config fail\n", __func__);

	if (cd->support_gesture_mode) {
		cd->easy_wakeup_info.sleep_mode = TS_POWER_OFF_MODE;
		cd->easy_wakeup_info.easy_wakeup_gesture = false;
		cd->easy_wakeup_info.off_motion_on = false;
	}

	return 0;
}

static u8 checksum_u8(u8 *data, u32 size)
{
	u8 checksum = 0;
	u32 i;
	int zero_count = 0;

	for (i = 0; i < size; i++) {
		checksum += data[i];
		if (!data[i])
			zero_count++;
	}
	return zero_count == size ? MASK_8BIT : checksum;
}

static int thp_goodix_communication_check(struct thp_device *tdev)
{
	int ret = 0;
	int len;
	int retry;
	u8 temp_buf[GOODIX_MASK_ID_LEN + 1] = {0};
	u8 checksum = 0;

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}

	len = sizeof(module_info);
	memset(&module_info, 0, len);

	for (retry = 0; retry < MODULE_INFO_READ_RETRY; retry++) {
		ret = thp_goodix_spi_read(tdev, GOODIX_MODULE_INFO_ADDR,
				(u8 *)&module_info, len);
		print_hex_dump(KERN_INFO, "[I/THP] goodix module info: ",
				DUMP_PREFIX_NONE,
				32, 3, (u8 *)&module_info, len, 0);
		checksum = checksum_u8((u8 *)&module_info, len);
		if (!ret && !checksum)
			break;

		mdelay(COMMUNI_CHECK_RETRY_DELAY);
	}

	THP_LOG_INFO("hw info: ret %d, checksum 0x%x, retry %d\n", ret,
		checksum, retry);
	if (retry == MODULE_INFO_READ_RETRY) {
		THP_LOG_ERR("%s:failed read module info\n", __func__);
		return -EINVAL;
	}

	if (memcmp(module_info.mask_id, GOODIX_MASK_ID,
			sizeof(GOODIX_MASK_ID) - 1)) {
		memcpy(temp_buf, module_info.mask_id, GOODIX_MASK_ID_LEN);
		THP_LOG_ERR("%s: invalied mask id %s != %s\n",
			__func__, temp_buf, GOODIX_MASK_ID);
		return -EINVAL;
	}

	THP_LOG_INFO("%s: communication check passed\n", __func__);
	memcpy(temp_buf, module_info.mask_id, GOODIX_MASK_ID_LEN);
	temp_buf[GOODIX_MASK_ID_LEN] = '\0';
	THP_LOG_INFO("MASK_ID %s : ver %*ph\n", temp_buf,
		(u32)sizeof(module_info.mask_vid), module_info.mask_vid);
	THP_LOG_INFO("PID %s : ver %*ph\n", module_info.pid,
		(u32)sizeof(module_info.vid), module_info.vid);
	return 0;
}

static int goodix_power_init(void)
{
	int ret;

	ret = thp_power_supply_get(THP_VCC);
	ret |= thp_power_supply_get(THP_IOVDD);
	if (ret)
		THP_LOG_ERR("%s: fail to get power\n", __func__);

	return 0;
}

static void goodix_power_release(void)
{
	thp_power_supply_put(THP_VCC);
	thp_power_supply_put(THP_IOVDD);
}

static int goodix_power_on(struct thp_device *tdev)
{
	int ret;

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("%s:called\n", __func__);
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_HIGH);

	ret = thp_power_supply_ctrl(THP_VCC, THP_POWER_ON, NO_DELAY);
	ret |= thp_power_supply_ctrl(THP_IOVDD, THP_POWER_ON, DELAY_1MS);
	if (ret)
		THP_LOG_ERR("%s:power ctrl fail\n", __func__);
	THP_LOG_INFO("%s pull up tp ic reset\n", __func__);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
	return ret;
}

static int goodix_power_off(struct thp_device *tdev)
{
	int ret;

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("%s pull down tp ic reset\n", __func__);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	thp_do_time_delay(tdev->timing_config.suspend_reset_after_delay_ms);

	ret = thp_power_supply_ctrl(THP_IOVDD, THP_POWER_OFF, NO_DELAY);
	ret |= thp_power_supply_ctrl(THP_VCC, THP_POWER_OFF, NO_DELAY);
	if (ret)
		THP_LOG_ERR("%s:power ctrl fail\n", __func__);
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);

	return ret;
}

static void thp_goodix_timing_work(struct thp_device *tdev)
{
	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return;
	}
	THP_LOG_INFO("%s:called,do hard reset\n", __func__);
	gpio_direction_output(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	thp_do_time_delay(tdev->timing_config.boot_reset_after_delay_ms);
}
static u16 checksum_16(u8 *data, int size)
{
	int i;
	int non_zero_count = 0;
	u16 checksum = 0;

	if ((data == NULL) || (size <= sizeof(u16))) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return MASK_8BIT;
	}

	for (i = 0; i < (size - sizeof(u16)); i++) {
		checksum += data[i];
		if (data[i])
			non_zero_count++;
	}

	checksum += (data[i] << MOVE_8BIT) + data[i + 1];

	return non_zero_count ? checksum : MASK_8BIT;
}
static u32 checksum_32(u8 *data, int size)
{
	int i;
	int non_zero_count = 0;
	u32 checksum = 0;

	if ((data == NULL) || (size <= sizeof(u32))) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return MASK_8BIT;
	}

	for (i = 0; i < (size - sizeof(u32)); i += 2) {
		checksum += ((data[i] << MOVE_8BIT) | data[i + 1]);
		if (data[i] || data[i + 1])
			non_zero_count++;
	}

	checksum += (data[i] << MOVE_24BIT) + (data[i + 1] << MOVE_16BIT) +
		    (data[i + 2] << MOVE_8BIT) + data[i + 3];

	return non_zero_count ? checksum : MASK_8BIT;
}

static int thp_goodix_get_afe_info(struct thp_device *tdev)
{
	int ret = 0;
	int retry = 0;
	int afe_data_len = 0;
	u8 buf[INFO_ADDR_BUF_LEN] = {0};
	u8 afe_data_buf[GOODIX_MAX_AFE_LEN] = {0};

	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}

	for (retry = 0; retry < FW_INFO_READ_RETRY; retry++) {
		ret = thp_goodix_spi_read(tdev, GOODIX_AFE_INFO_ADDR,
				buf, sizeof(buf));
		if (ret) {
			THP_LOG_ERR("%s: failed read afe data length, ret %d\n",
				__func__, ret);
			goto err_out;
		}

		afe_data_len = (buf[0] << MOVE_8BIT) | buf[1];
		/* data len must be equal or less than GOODIX_MAX_AFE_LEN */
		if ((afe_data_len <= 0) || (afe_data_len > GOODIX_MAX_AFE_LEN)
			|| (afe_data_len & MASK_1BIT)) {
			THP_LOG_ERR("%s: invalied afe_data_len 0x%x retry\n",
				__func__, afe_data_len);
			mdelay(GET_AFE_INFO_RETRY_DELAY);
			continue;
		}
		THP_LOG_INFO("%s: got afe data len %d\n",
				__func__, afe_data_len);
		ret = thp_goodix_spi_read(tdev, GOODIX_AFE_INFO_ADDR + 2,
				afe_data_buf, afe_data_len);
		if (ret) {
			THP_LOG_ERR("%s: failed read afe data, ret %d\n",
				__func__, ret);
			goto err_out;
		}

		if (!checksum_32(afe_data_buf, afe_data_len)) {
			THP_LOG_INFO("%s: successfuly read afe data\n",
				__func__);
			break;
		}
		THP_LOG_ERR("%s: afe data checksum error, checksum 0x%x, retry\n",
			__func__, checksum_32(afe_data_buf, afe_data_len));
		THP_LOG_ERR("afe_data_buf[0-20] %*ph\n",
			DEBUG_AFE_DATA_BUF_LEN, afe_data_buf);
		THP_LOG_ERR("afe_data_buf[20-40] %*ph\n",
			DEBUG_AFE_DATA_BUF_LEN, afe_data_buf +
			DEBUG_AFE_DATA_BUF_OFFSET);
		THP_LOG_ERR("afe_data_buf[40-60] %*ph\n",
			DEBUG_AFE_DATA_BUF_LEN, afe_data_buf +
			DEBUG_AFE_DATA_BUF_OFFSET * 2);
		THP_LOG_ERR("afe_data_buf[60-80] %*ph\n",
			DEBUG_AFE_DATA_BUF_LEN, afe_data_buf +
			DEBUG_AFE_DATA_BUF_OFFSET * 3);
		THP_LOG_ERR("afe_data_buf[80-100] %*ph\n",
			DEBUG_AFE_DATA_BUF_LEN, afe_data_buf +
			DEBUG_AFE_DATA_BUF_OFFSET * 4);
		mdelay(GET_AFE_INFO_RETRY_DELAY);
	}

	if (retry != FW_INFO_READ_RETRY) {
		THP_LOG_INFO("%s: try get useful info from afe data\n", __func__);
		goodix_frame_addr =
			(afe_data_buf[GOODIX_FRAME_ADDR_OFFSET] << MOVE_8BIT) +
			afe_data_buf[GOODIX_FRAME_ADDR_OFFSET + 1];
		goodix_frame_len =
			(afe_data_buf[GOODIX_FRAME_LEN_OFFSET] << MOVE_8BIT) +
			afe_data_buf[GOODIX_FRAME_LEN_OFFSET + 1];
		goodix_cmd_addr =
			(afe_data_buf[GOODIX_CMD_ADDR_OFFSET] << MOVE_8BIT) +
			afe_data_buf[GOODIX_CMD_ADDR_OFFSET + 1];
		goodix_fw_status_addr =
			(afe_data_buf[GOODIX_FW_STATUS_ADDR_OFFSET] << MOVE_8BIT) +
			afe_data_buf[GOODIX_FW_STATUS_ADDR_OFFSET + 1];
		goodix_fw_status_len =
			(afe_data_buf[GOODIX_FW_STATUS_LEN_OFFSET] << MOVE_8BIT) +
			afe_data_buf[GOODIX_FW_STATUS_LEN_OFFSET + 1];
		ret = 0;
	} else {
		THP_LOG_ERR("%s: failed get afe info, use default\n", __func__);
		ret = -EINVAL;
	}

err_out:
	THP_LOG_INFO("%s: frame addr 0x%x, len %d, cmd addr 0x%x\n", __func__,
			goodix_frame_addr, goodix_frame_len, goodix_cmd_addr);
	THP_LOG_INFO("%s: fw status addr 0x%x, len %d\n", __func__,
			goodix_fw_status_addr, goodix_fw_status_len);
	return ret;
}

int thp_goodix_chip_detect(struct thp_device *tdev)
{
	int ret;

	THP_LOG_INFO("%s: called\n", __func__);
	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}

	goodix_power_init();
	ret = goodix_power_on(tdev);
	if (ret)
		THP_LOG_ERR("%s: power on failed\n", __func__);

	thp_goodix_timing_work(tdev);

	if (thp_goodix_communication_check(tdev)) {
		THP_LOG_ERR("%s:communication check fail\n", __func__);
		goodix_power_off(tdev);
		goodix_power_release();
		return -ENODEV;
	}

	if (thp_goodix_get_afe_info(tdev))
		THP_LOG_ERR("%s: failed get afe addr info\n", __func__);
	return 0;
}

static int thp_goodix_get_frame(struct thp_device *tdev,
			char *buf, unsigned int len)
{
	if ((tdev == NULL) || (buf == NULL)) {
		THP_LOG_INFO("%s: input dev null\n", __func__);
		return -ENOMEM;
	}

	if (!len) {
		THP_LOG_INFO("%s: read len illegal\n", __func__);
		return -ENOMEM;
	}

	return thp_goodix_spi_read(tdev, goodix_frame_addr, buf, len);
}

static int goodix_gesture_report(struct thp_device *tdev,
		unsigned int *gesture_wakeup_value)
{
	int retval;

	THP_LOG_INFO("[%s]\n", __func__);
	retval = thp_goodix_gesture_event(tdev, gesture_wakeup_value);
	if (retval != 0) {
		THP_LOG_INFO("[%s] ->get event failed\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static int goodix_wakeup_gesture_enable_switch(
		struct thp_device *tdev, u8 switch_value)
{
	int retval = NO_ERR;

	if (!tdev) {
		THP_LOG_ERR("%s: input dev is null\n", __func__);
		return -EINVAL;
	}

	if (switch_value) {
		retval = thp_goodix_switch_cmd(tdev, CMD_GESTURE_MODE,
				FEATURE_ENABLE);
		retval |= thp_goodix_switch_cmd(tdev, CMD_SCREEN_ON_OFF,
				FEATURE_ENABLE);
		if (retval) {
			THP_LOG_ERR("failed enable gesture mode\n");
		} else {
			work_status |= IC_STATUS_GESTURE;
			THP_LOG_INFO("enable gesture mode\n");
		}
	} else {
		retval =  thp_goodix_switch_cmd(tdev, CMD_GESTURE_MODE,
				FEATURE_DISABLE);
		if (retval) {
			THP_LOG_ERR("failed disable gesture mode\n");
		} else {
			THP_LOG_INFO("disable gesture mode\n");
			work_status &= ~IC_STATUS_GESTURE;
		}
	}

	THP_LOG_INFO("%s, write TP IC\n", __func__);
	return retval;
}

static int goodix_wrong_touch(struct thp_device *tdev)
{
	if (!tdev) {
		THP_LOG_ERR("%s: input dev is null\n", __func__);
		return -EINVAL;
	}

	if (tdev->thp_core->support_gesture_mode) {
		mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
		tdev->thp_core->easy_wakeup_info.off_motion_on = true;
		mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
		THP_LOG_INFO("[%s] ->done\n", __func__);
	}
	return 0;
}

/* call this founction when TPIC in gesture mode
*  return: if get valied gesture type 0 is returened
*/
#define GESTURE_EVENT_RETRY_TIME 10
static int thp_goodix_gesture_event(struct thp_device *tdev,
		unsigned int *gesture_wakeup_value)
{
	u8 sync_flag = 0;
	int retval = -1;
	u16 gesture_type;
	u8 gesture_event_head[GESTURE_EVENT_HEAD_LEN + 1] = {0};
	int i = 0;

	if (tdev == NULL) {
		THP_LOG_ERR("%s: input dev is null\n", __func__);
		return -EINVAL;
	}
	if (!(work_status & IC_STATUS_GESTURE)) {
		THP_LOG_INFO("%s:please enable gesture mode first\n", __func__);
		retval = -EINVAL;
		goto err_out;
	}
	msleep(WAIT_FOR_SPI_BUS_RESUMED_DELAY);
	/* wait spi bus resume */
	for (i = 0; i < GESTURE_EVENT_RETRY_TIME; i++) {
		retval = thp_goodix_spi_read(tdev, goodix_frame_addr,
				gesture_event_head, sizeof(gesture_event_head));
		if (retval == 0) {
			break;
		} else {
			THP_LOG_ERR("%s: spi not work normal, ret %d retry\n",
				__func__, retval);
			msleep(WAIT_FOR_SPI_BUS_READ_DELAY);
		}
	}
	if (retval) {
		THP_LOG_ERR("%s: failed read gesture head info, ret %d\n",
			__func__, retval);
		return -EINVAL;
	}

	THP_LOG_INFO("gesute_data:%*ph\n", (u32)sizeof(gesture_event_head),
			gesture_event_head);
	if (gesture_event_head[0] != IRQ_EVENT_TYPE_GESTURE) {
		THP_LOG_ERR("%s: not gesture irq event, event_type 0x%x\n",
			__func__, gesture_event_head[0]);
		retval = -EINVAL;
		goto err_out;
	}

	if (checksum_16(gesture_event_head + 1, GESTURE_EVENT_HEAD_LEN)) {
		THP_LOG_ERR("gesture data checksum error\n");
		retval = -EINVAL;
		goto err_out;
	}

	gesture_type = (gesture_event_head[1] << MOVE_8BIT) +
			gesture_event_head[2];
	if (gesture_type == GESTURE_TYPE_DOUBLE_TAP) {
		THP_LOG_INFO("found valid gesture type\n");
		mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
		if (tdev->thp_core->easy_wakeup_info.off_motion_on == true) {
			tdev->thp_core->easy_wakeup_info.off_motion_on = false;
			*gesture_wakeup_value = TS_DOUBLE_CLICK;
		}
		mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
		retval = 0;
	} else {
		THP_LOG_ERR("found invalid gesture type:0x%x\n", gesture_type);
		retval = -EINVAL;
	}
	/* clean sync flag */
	thp_goodix_spi_write(tdev, goodix_frame_addr, &sync_flag, 1);
	return retval;

err_out:
	/* clean sync flag */
	thp_goodix_spi_write(tdev, goodix_frame_addr, &sync_flag, 1);
	/* resend gesture command */
	retval =  thp_goodix_switch_cmd(tdev, CMD_SCREEN_ON_OFF, 1);
	retval |=  thp_goodix_switch_cmd(tdev, CMD_GESTURE_MODE,
		FEATURE_ENABLE);
	work_status |= IC_STATUS_GESTURE;
	if (retval)
		THP_LOG_ERR("resend gesture command\n");
	else
		THP_LOG_INFO("success resend gesture command\n");
	return -EINVAL;
}

static int thp_goodix_resume(struct thp_device *tdev)
{
	int ret = 0;

	THP_LOG_INFO("%s: called\n", __func__);
	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	// if udfp enable or pt mode or gesture mode ,we should reset
	if (g_thp_udfp_stauts  || is_pt_test_mode(tdev) ||
		(tdev->thp_core->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE)) {
		gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
		mdelay(tdev->timing_config.resume_reset_after_delay_ms);
		gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
		THP_LOG_INFO("%s: called end\n", __func__);
		return ret;
	}
	ret = goodix_power_on(tdev);
	return ret;
}
static int thp_goodix_after_resume(struct thp_device *tdev)
{
	int ret = 0;

	THP_LOG_INFO("%s: called\n", __func__);
	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	thp_do_time_delay(tdev->timing_config.boot_reset_after_delay_ms);
	if (!g_thp_udfp_stauts  && !is_pt_test_mode(tdev)) {
		// Turn off sensorhub report when
		// fingerprintud  isn't in work state.
		ret = thp_goodix_switch_cmd(tdev, CMD_TOUCH_REPORT, 0);
		if (ret)
			THP_LOG_ERR("failed send CMD_TOUCH_REPORT mode\n");
	}

	return ret;
}

static int thp_goodix_suspend(struct thp_device *tdev)
{
	int ret = 0;

	THP_LOG_INFO("%s: called\n", __func__);
	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	g_thp_udfp_stauts = thp_get_status(THP_STAUTS_UDFP);
	if (is_pt_test_mode(tdev)) {
		THP_LOG_INFO("%s: suspend PT mode\n", __func__);
		ret =  thp_goodix_switch_cmd(tdev, CMD_PT_MODE, PT_MODE);
		if (ret)
			THP_LOG_ERR("failed enable PT mode\n");
	} else if (g_thp_udfp_stauts ||
		(tdev->thp_core->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE)) {
		ret =  thp_goodix_switch_cmd(tdev, CMD_SCREEN_ON_OFF, 1);
		if (ret)
			THP_LOG_ERR("failed to screen_on off\n");
		if (tdev->thp_core->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE) {
			THP_LOG_INFO("%s TS_GESTURE_MODE\n", __func__);
			ret = goodix_wakeup_gesture_enable_switch(tdev, FEATURE_ENABLE);
			if (ret)
				THP_LOG_ERR("failed to wakeup gesture enable\n");
			mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
			tdev->thp_core->easy_wakeup_info.off_motion_on = true;
			mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
		}
	} else {
		ret = goodix_power_off(tdev);
		THP_LOG_INFO("enter poweroff mode: ret = %d\n", ret);
	}
	THP_LOG_INFO("%s: called end\n", __func__);
	return ret;
}

static void goodix_get_oem_info(struct thp_device *tdev, char *buff)
{
	struct thp_core_data *cd = thp_get_core_data();
	char lcd_effect_flag;
	int ret;

	if ((!tdev) || (!buff) || (!tdev->thp_core)) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return;
	}
	if (tdev->thp_core->support_oem_info == THP_OEM_INFO_LCD_EFFECT_TYPE) {
		/* 12th byte is lcd_effect_flag, and 0xaa is valid */
		lcd_effect_flag = buff[GOODIX_LCD_EFFECT_FLAG_POSITION - 1];
		memset(cd->oem_info_data, 0, sizeof(cd->oem_info_data));
		ret = snprintf(cd->oem_info_data, OEM_INFO_DATA_LENGTH,
			"0x%x", lcd_effect_flag);
		if (ret < 0)
			THP_LOG_INFO("%s:snprintf error\n", __func__);
		THP_LOG_INFO("%s:lcd effect flag :%s\n", __func__,
			cd->oem_info_data);
		return;
	}
	THP_LOG_INFO("%s:not support oem info\n", __func__);
}

static int goodix_get_project_id(struct thp_device *tdev, char *buf, unsigned int len)
{
	int retry;
	char proj_id[GOODIX_CUSTOME_INFO_LEN + 1] = {0};
	int ret = 0;

	if ((tdev == NULL) || (buf == NULL)) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}

	for (retry = 0; retry < GOODIX_GET_PROJECT_ID_RETRY; retry++) {
		ret = thp_goodix_spi_read(tdev, GOODIX_PROJECT_ID_ADDR,
			proj_id, GOODIX_CUSTOME_INFO_LEN);
		if (ret) {
			THP_LOG_ERR("Project_id Read ERR\n");
			return -EIO;
		}

		if (!checksum_u8(proj_id, GOODIX_CUSTOME_INFO_LEN)) {
			proj_id[THP_PROJECT_ID_LEN] = '\0';
			if (!is_valid_project_id(proj_id)) {
				THP_LOG_ERR("get project id fail\n");
				return -EIO;
			}
			strncpy(buf, proj_id, len);
			THP_LOG_INFO("%s:get project id:%s\n", __func__, buf);
			goodix_get_oem_info(tdev, proj_id);
			return 0;
		}
		THP_LOG_ERR("proj_id[0-30] %*ph\n",
				GOODIX_CUSTOME_INFO_LEN, proj_id);
		THP_LOG_ERR("%s:get project id fail, retry\n", __func__);
		mdelay(GOODIX_GET_PROJECT_RETRY_DELAY_10MS);
	}

	return -EIO;
}

static void thp_goodix_exit(struct thp_device *tdev)
{
	THP_LOG_INFO("%s: called\n", __func__);
	if (!tdev) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return;
	}
	kfree(tdev->tx_buff);
	kfree(tdev->rx_buff);
	kfree(tdev);
}

static int thp_goodix_afe_notify_callback(struct thp_device *tdev,
		unsigned long event)
{
	if (tdev == NULL) {
		THP_LOG_ERR("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	return thp_goodix_get_afe_info(tdev);
}

#define GOODIX_SPI_ACTIVE_DELAY 1000
int thp_goodix_spi_active(struct thp_device *tdev)
{
	int ret;
	u8 wake_flag = IDLE_WAKE_FLAG;

	ret = thp_set_spi_max_speed(IDLE_SPI_SPEED);
	if (ret) {
		THP_LOG_ERR("failed set spi speed to %dHz, ret %d\n",
			IDLE_SPI_SPEED, ret);
		return ret;
	}

	ret = thp_goodix_spi_write(tdev, 0, &wake_flag, sizeof(wake_flag));
	if (ret)
		THP_LOG_ERR("failed write wakeup flag %x, ret %d",
				wake_flag, ret);
	THP_LOG_INFO("[%s] tdev->sdev->max_speed_hz-> %d\n", __func__,
			tdev->sdev->max_speed_hz);

	ret = thp_set_spi_max_speed(ACTIVE_SPI_SPEED);
	if (ret) {
		THP_LOG_ERR("failed reset speed to %dHz, ret %d\n",
			tdev->sdev->max_speed_hz, ret);
		return ret;
	}

	udelay(GOODIX_SPI_ACTIVE_DELAY);
	return ret;
}
#define FW_STAUTE_DATA_MASK  0x04
#define FW_STAUTE_DATA_FLAG  0xAA
static int thp_goodix_idle_wakeup(struct thp_device *tdev)
{
	int ret;
	int i;
	u8 fw_status_data[MAX_FW_STATUS_DATA_LEN] = {0};

	THP_LOG_DEBUG("[%s]\n", __func__);

	if (!goodix_fw_status_addr || !goodix_fw_status_len ||
		goodix_fw_status_len > MAX_FW_STATUS_DATA_LEN) {
		THP_LOG_ERR("invalied fw status address 0x%x or length info %d\n",
		goodix_fw_status_addr, goodix_fw_status_len);
		return 0;
	}
	ret = thp_goodix_spi_read(tdev, goodix_fw_status_addr,
				fw_status_data, goodix_fw_status_len);
	if (ret) {
		THP_LOG_ERR("failed read fw status info data, ret %d\n", ret);
		return -EIO;
	}

	if (!checksum_16(fw_status_data, goodix_fw_status_len) &&
		!(fw_status_data[0] & FW_STAUTE_DATA_MASK) &&
		(fw_status_data[goodix_fw_status_len - 3] ==
			FW_STAUTE_DATA_FLAG))
		return 0;

	THP_LOG_DEBUG("fw status data:%*ph\n",
			goodix_fw_status_len, fw_status_data);
	THP_LOG_DEBUG("need do spi wakeup\n");
	for (i = 0; i < SPI_WAKEUP_RETRY; i++) {
		ret = thp_goodix_spi_active(tdev);
		if (ret) {
			THP_LOG_DEBUG("failed write spi active flag, ret %d\n",
				ret);
			continue;
		}
		// recheck spi state
		ret = thp_goodix_spi_read(tdev, goodix_fw_status_addr,
					fw_status_data, goodix_fw_status_len);
		if (ret) {
			THP_LOG_ERR("[recheck]failed read fw status info data, ret %d\n",
				ret);
			continue;
		}

		if (!checksum_16(fw_status_data, goodix_fw_status_len) &&
			!(fw_status_data[0] & FW_STAUTE_DATA_MASK) &&
			(fw_status_data[goodix_fw_status_len - 3] ==
				FW_STAUTE_DATA_FLAG))
			return 0;

		THP_LOG_DEBUG("fw status data:%*ph\n", goodix_fw_status_len,
						fw_status_data);
		THP_LOG_DEBUG("failed wakeup form idle retry %d\n", i);
	}
	return -EIO;
}

struct thp_device_ops goodix_dev_ops = {
	.init = thp_goodix_init,
	.detect = thp_goodix_chip_detect,
	.get_frame = thp_goodix_get_frame,
	.resume = thp_goodix_resume,
	.after_resume = thp_goodix_after_resume,
	.suspend = thp_goodix_suspend,
	.get_project_id = goodix_get_project_id,
	.exit = thp_goodix_exit,
	.afe_notify = thp_goodix_afe_notify_callback,
	.chip_wakeup_gesture_enable_switch = goodix_wakeup_gesture_enable_switch,
	.chip_wrong_touch = goodix_wrong_touch,
	.chip_gesture_report = goodix_gesture_report,
};

static int __init thp_goodix_module_init(void)
{
	int rc;
	struct thp_device *dev = kzalloc(sizeof(struct thp_device), GFP_KERNEL);

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

	dev->ic_name = GOODIX_IC_NAME;
	dev->dev_node_name = THP_GOODIX_DEV_NODE_NAME;
	dev->ops = &goodix_dev_ops;

	rc = thp_register_dev(dev);
	if (rc) {
		THP_LOG_ERR("%s: register fail\n", __func__);
		goto err;
	}

	return rc;
err:
	kfree(dev->tx_buff);
	dev->tx_buff = NULL;
	kfree(dev->rx_buff);
	dev->rx_buff = NULL;
	kfree(dev);
	dev = NULL;
	return rc;
}

static void __exit thp_goodix_module_exit(void)
{
	THP_LOG_INFO("%s: called\n", __func__);
};

module_init(thp_goodix_module_init);
module_exit(thp_goodix_module_exit);
MODULE_AUTHOR("goodix");
MODULE_DESCRIPTION("goodix driver");
MODULE_LICENSE("GPL");
