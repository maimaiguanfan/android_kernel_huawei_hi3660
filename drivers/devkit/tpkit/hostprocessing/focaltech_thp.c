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
#include "huawei_thp.h"

#define FOCALTECH_IC_NAME "focaltech"
#define THP_FOCALTECH_DEV_NODE_NAME "focaltech"

#define DATA_HEAD			0x3F
#define DATA_MASK			0xFF
#define COMOND_HEAD			0xC0
#define COMOND_READ_ONE			0x81
#define COMOND_WRITE_ONE		0x01
#define COMOND_ADDR_RESET		0x55
#define COMMOND_READ_DATA		0x3A
#define COMOND_ADDR_DETECT		0x90
#define CHIP_DETECT_FAILE_ONE		0x00
#define CHIP_DETECT_FAILE_TWO		0xFF
#define CHIP_DETECT_FAILE_THR		0xEF
#define CHIP_DETECT_RETRY_NUMB		3
#define DELAY_AFTER_FIRST_BYTE		30

/* spi interface */
static int thp_fts_spi_transfer(char *tx_buf,
			char *rx_buf, unsigned int len)
{
	int rc;
	struct thp_core_data *cd = thp_get_core_data();
	struct spi_device *sdev = cd->sdev;
	struct spi_message msg;
	struct spi_transfer xfer[2];

	if (cd->suspended)
		return 0;

	memset(xfer, 0, sizeof(xfer));

	spi_message_init(&msg);
	xfer[0].tx_buf = &tx_buf[0];
	if (rx_buf)
		xfer[0].rx_buf = &rx_buf[0];
	xfer[0].len = 1;
	xfer[0].delay_usecs = DELAY_AFTER_FIRST_BYTE;
	spi_message_add_tail(&xfer[0], &msg);
	if (len > 1) {
		xfer[1].tx_buf = &tx_buf[1];
		if (rx_buf)
			xfer[1].rx_buf = &rx_buf[1];
		xfer[1].len = len - 1;
		spi_message_add_tail(&xfer[1], &msg);
	}

	mutex_lock(&cd->spi_mutex);
	thp_spi_cs_set(GPIO_HIGH);
	rc = spi_sync(sdev, &msg);
	mutex_unlock(&cd->spi_mutex);

	return rc;
}

static int thp_fts_hardware_reset_to_rom(struct thp_device *tdev)
{
	int rc;
	unsigned char cmd[3]; // three byte once

	if (!tdev) {
		THP_LOG_INFO("%s: input dev null\n", __func__);
		return -ENOMEM;
	}
	/* reset */
	gpio_direction_output(tdev->gpios->rst_gpio, THP_RESET_LOW);
	thp_do_time_delay(tdev->timing_config.boot_reset_low_delay_ms);

	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	thp_do_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);

	mdelay(8); // sequential

	cmd[0] = COMOND_HEAD;
	cmd[1] = COMOND_WRITE_ONE;
	cmd[2] = COMOND_ADDR_RESET;
	rc = thp_fts_spi_transfer(cmd, cmd, ARRAY_SIZE(cmd));
	if (rc) {
		THP_LOG_ERR("%s:write 0x55 command fail\n", __func__);
		return rc;
	}

	mdelay(8); // sequential
	return 0;
}

static u8 cal_ecc_xor8(u8 *buf, u16 buflen)
{
	u16 i;
	u8 result = 0;

	for (i = 0; i < buflen; i++)
		result ^= buf[i];

	return result;
}

static int thp_fts_check_frame_valid(u8 *buf, u16 len)
{
	u16 ecc_buf;
	u16 ecc_cal;

	if (len < 2) {
		THP_LOG_ERR("thp_fts_check_frame_valid buf len < 2 %d", len);
		return -EIO;
	}
	ecc_cal = (u16)cal_ecc_xor8(buf, len - 2);
	ecc_buf = (u16)((buf[len - 2] << 8) + buf[len - 1]);
	if (ecc_cal != ecc_buf) {
		THP_LOG_ERR("ecc fail:cal(%x) != buf(%x)", ecc_cal, ecc_buf);
		return -EIO;
	}

	return 0;
}

static int thp_fts_init(struct thp_device *tdev)
{
	struct thp_core_data *cd = NULL;
	struct device_node *fts_node = NULL;

	THP_LOG_INFO("%s: called\n", __func__);
	if (!tdev) {
		THP_LOG_INFO("%s: input dev null\n", __func__);
		return -ENOMEM;
	}
	cd = tdev->thp_core;
	fts_node = of_get_child_by_name(cd->thp_node,
						THP_FOCALTECH_DEV_NODE_NAME);
	if (!fts_node) {
		THP_LOG_INFO("%s: focaltech dev not config in dts\n", __func__);
		return -ENODEV;
	}

	if (thp_parse_spi_config(fts_node, cd))
		THP_LOG_ERR("%s: spi config parse fail\n", __func__);

	if (thp_parse_timing_config(fts_node, &tdev->timing_config))
		THP_LOG_ERR("%s: timing config parse fail\n", __func__);

	if (thp_parse_feature_config(fts_node, cd))
		THP_LOG_ERR("%s: feature_config fail\n", __func__);

	return 0;
}

static int thp_fts_chip_detect(struct thp_device *tdev)
{
	int rc;
	int i;
	unsigned char cmd[3]; // three byte once

	if (!tdev) {
		THP_LOG_INFO("%s: input dev null\n", __func__);
		return -ENOMEM;
	}
	for (i = 0; i < CHIP_DETECT_RETRY_NUMB; i++) {
		rc = thp_fts_hardware_reset_to_rom(tdev);
		if (rc) {
			THP_LOG_ERR("%s:write 0x55 command fail\n", __func__);
			continue;
		}
		cmd[0] = COMOND_HEAD;
		cmd[1] = COMOND_READ_ONE;
		cmd[2] = COMOND_ADDR_DETECT;
		rc = thp_fts_spi_transfer(cmd, cmd, ARRAY_SIZE(cmd));
		if (rc) {
			THP_LOG_ERR("%s:write 0x90 command fail\n", __func__);
			return rc;
		}
		msleep(10); // sequential

		cmd[0] = DATA_HEAD;
		cmd[1] = DATA_MASK;
		cmd[2] = DATA_MASK;
		rc = thp_fts_spi_transfer(cmd, cmd, ARRAY_SIZE(cmd));
		if (rc) {
			THP_LOG_ERR("%s:read 0x90 data fail\n", __func__);
			return rc;
		}

		if ((cmd[1] == CHIP_DETECT_FAILE_ONE) ||
			(cmd[1] == CHIP_DETECT_FAILE_TWO) ||
			(cmd[1] == CHIP_DETECT_FAILE_THR)) {
			THP_LOG_ERR("%s:chip id read fail", __func__);
			THP_LOG_ERR("ret=%d, i=%d, data=%x\n", rc, i, cmd[1]);
		} else {
			THP_LOG_INFO("%s:chip id read success", __func__);
			THP_LOG_INFO("chip id:0x%X%X\n", cmd[1], cmd[2]);
			return 0;
		}
		msleep(50); // sequential
	}

	return -EIO;
}

static int thp_fts_get_frame(struct thp_device *tdev,
			char *buf, unsigned int len)
{
	int ret;
	unsigned char *w_buf = NULL;
	unsigned char *r_buf = NULL;

	if (!tdev) {
		THP_LOG_INFO("%s: input dev null\n", __func__);
		return -ENOMEM;
	}
	if (!tdev->tx_buff || !tdev->rx_buff) {
		THP_LOG_ERR("%s: out of memory\n", __func__);
		return -ENOMEM;
	}
	if (!buf) {
		THP_LOG_INFO("%s: input buf null\n", __func__);
		return -ENOMEM;
	}

	if ((!len) || (len >= THP_MAX_FRAME_SIZE - 1)) {
		THP_LOG_INFO("%s: read len: %u illegal\n", __func__, len);
		return -ENOMEM;
	}

	w_buf = tdev->tx_buff;
	r_buf = tdev->rx_buff;
	memset(tdev->tx_buff, 0xFF, THP_MAX_FRAME_SIZE);
	memset(tdev->rx_buff, 0, THP_MAX_FRAME_SIZE);
	w_buf[0] = COMMOND_READ_DATA;
	ret = thp_fts_spi_transfer(w_buf, r_buf, len + 1);
	if (ret < 0) {
		THP_LOG_ERR("spi transfer fail");
		return ret;
	}
	ret = thp_fts_check_frame_valid(r_buf + 1, len);
	if (ret < 0) {
		ret = thp_fts_spi_transfer(w_buf, r_buf, len + 1);
		if (ret < 0) {
			THP_LOG_ERR("spi transfer fail");
			return ret;
		}
	}

	memcpy(buf, r_buf + 1, len);

	return 0;
}

static int thp_fts_resume(struct thp_device *tdev)
{
	THP_LOG_INFO("%s: called\n", __func__);
	if (!tdev) {
		THP_LOG_INFO("%s: input dev null\n", __func__);
		return -ENOMEM;
	}
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_HIGH);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
	thp_do_time_delay(tdev->timing_config.resume_reset_after_delay_ms);

	return 0;
}

static int thp_fts_suspend(struct thp_device *tdev)
{
	THP_LOG_INFO("%s: called\n", __func__);
	if (!tdev) {
		THP_LOG_INFO("%s: input dev null\n", __func__);
		return -ENOMEM;
	}
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);
	thp_do_time_delay(tdev->timing_config.suspend_reset_after_delay_ms);

	return 0;
}

static void thp_fts_exit(struct thp_device *tdev)
{
	THP_LOG_INFO("%s: called\n", __func__);
	if (tdev) {
		kfree(tdev->tx_buff);
		tdev->tx_buff = NULL;
		kfree(tdev->rx_buff);
		tdev->rx_buff = NULL;
		kfree(tdev);
		tdev = NULL;
	}
}

struct thp_device_ops fts_dev_ops = {
	.init = thp_fts_init,
	.detect = thp_fts_chip_detect,
	.get_frame = thp_fts_get_frame,
	.resume = thp_fts_resume,
	.suspend = thp_fts_suspend,
	.exit = thp_fts_exit,
	.spi_transfer = thp_fts_spi_transfer,
};

static int __init thp_focaltech_module_init(void)
{
	int rc;
	struct thp_device *dev = NULL;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		THP_LOG_ERR("%s: thp device malloc fail\n", __func__);
		return -ENOMEM;
	}

	dev->tx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	dev->rx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	if (!dev->tx_buff || !dev->rx_buff) {
		THP_LOG_ERR("%s: out of memory\n", __func__);
		rc = -ENOMEM;
		goto err;
	}

	dev->ic_name = FOCALTECH_IC_NAME;
	dev->ops = &fts_dev_ops;

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

static void __exit thp_focaltech_module_exit(void)
{
	THP_LOG_INFO("%s: called\n", __func__);
};

module_init(thp_focaltech_module_init);
module_exit(thp_focaltech_module_exit);
MODULE_AUTHOR("focaltech");
MODULE_DESCRIPTION("focaltech driver");
MODULE_LICENSE("GPL");
