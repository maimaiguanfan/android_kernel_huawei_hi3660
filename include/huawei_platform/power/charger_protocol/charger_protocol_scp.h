/*
 * charger_protocol_scp.h
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

#ifndef _CHARGER_PROTOCOL_SCP_H_
#define _CHARGER_PROTOCOL_SCP_H_

#ifndef BIT
#define BIT(x)                               (1 << (x))
#endif

#define SCP_PROTOCOL_BYTE_BITS               (8)
#define SCP_PROTOCOL_BYTE_MASK               (0xff)

#define SCP_PROTOCOL_WORD_BITS               (16)
#define SCP_PROTOCOL_WORD_MASK               (0xffff)

/* adapter type information register */
#define SCP_PROTOCOL_ADP_TYPE0               (0x7e)
#define SCP_PROTOCOL_ADP_TYPE0_AB_MASK       (BIT(7) | BIT(5) | BIT(4))
#define SCP_PROTOCOL_ADP_TYPE0_B_MASK        (BIT(4))
#define SCP_PROTOCOL_ADP_TYPE0_B_SC_MASK     (BIT(3))
#define SCP_PROTOCOL_ADP_TYPE0_B_LVC_MASK    (BIT(2))

#define SCP_PROTOCOL_ADP_TYPE1               (0x80)
#define SCP_PROTOCOL_ADP_TYPE1_B_MASK        (BIT(4))

#define SCP_PROTOCOL_COMPILEDVER_HBYTE       (0x7c)
#define SCP_PROTOCOL_COMPILEDVER_LBYTE       (0x7d)

/* adapter information register */
#define SCP_PROTOCOL_B_ADP_TYPE              (0x81)
#define SCP_PROTOCOL_B_ADP_TYPE_B_MASK       (BIT(4))

#define SCP_PROTOCOL_VENDOR_ID_HBYTE         (0x82)
#define SCP_PROTOCOL_VENDOR_ID_LBYTE         (0x83)

#define SCP_PROTOCOL_MODULE_ID_HBYTE         (0x84)
#define SCP_PROTOCOL_MODULE_ID_LBYTE         (0x85)

#define SCP_PROTOCOL_SERIAL_NO_HBYTE         (0x86) /* years: start 2015 */
#define SCP_PROTOCOL_SERIAL_NO_LBYTE         (0x87) /* weeks: start 0 */
#define SCP_PROTOCOL_START_YEARS             (2015)
#define SCP_PROTOCOL_START_WEEKS             (0)

#define SCP_PROTOCOL_CHIP_ID                 (0x88)
#define SCP_PROTOCOL_CHIP_ID_RICHTEK         (0x01)
#define SCP_PROTOCOL_CHIP_ID_WELTREND        (0x02)
#define SCP_PROTOCOL_CHIP_ID_IWATT           (0x03)
#define SCP_PROTOCOL_CHIP_ID_0X32            (0x32)

#define SCP_PROTOCOL_HWVER                   (0x89)

#define SCP_PROTOCOL_FWVER_HBYTE             (0x8a) /* xx */
#define SCP_PROTOCOL_FWVER_LBYTE             (0x8b) /* yy.zz */
#define SCP_PROTOCOL_FWVER_XX_MASK           (BIT(7) | BIT(6) | BIT(5) | \
	BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SCP_PROTOCOL_FWVER_XX_SHIFT          (0)
#define SCP_PROTOCOL_FWVER_YY_MASK           (BIT(7) | BIT(6) | BIT(5) | BIT(4))
#define SCP_PROTOCOL_FWVER_YY_SHIFT          (4)
#define SCP_PROTOCOL_FWVER_ZZ_MASK           (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SCP_PROTOCOL_FWVER_ZZ_SHIFT          (0)

#define SCP_PROTOCOL_RESERVED_0X8C           (0x8c)

#define SCP_PROTOCOL_ADP_B_TYPE1             (0x8d)

#define SCP_PROTOCOL_FACTORY_ID              (0x8e)

#define SCP_PROTOCOL_RESERVED_0X8F           (0x8f)

/* adapter index specification information register */
#define SCP_PROTOCOL_MAX_POWER               (0x90)
#define SCP_PROTOCOL_CNT_POWER               (0x91)

#define SCP_PROTOCOL_MIN_VOUT                (0x92)
#define SCP_PROTOCOL_MAX_VOUT                (0x93)
#define SCP_PROTOCOL_VOUT_A_MASK             (BIT(7) | BIT(6))
#define SCP_PROTOCOL_VOUT_A_SHIFT            (6)
#define SCP_PROTOCOL_VOUT_B_MASK             (BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define SCP_PROTOCOL_VOUT_B_SHIFT            (0)

#define SCP_PROTOCOL_VOUT_A_0                (0)
#define SCP_PROTOCOL_VOUT_A_1                (1)
#define SCP_PROTOCOL_VOUT_A_2                (2)
#define SCP_PROTOCOL_VOUT_A_3                (3)

#define SCP_PROTOCOL_MIN_IOUT                (0x94)
#define SCP_PROTOCOL_MAX_IOUT                (0x95)
#define SCP_PROTOCOL_IOUT_A_MASK             (BIT(7) | BIT(6))
#define SCP_PROTOCOL_IOUT_A_SHIFT            (6)
#define SCP_PROTOCOL_IOUT_B_MASK             (BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define SCP_PROTOCOL_IOUT_B_SHIFT            (0)

#define SCP_PROTOCOL_IOUT_A_0                (0)
#define SCP_PROTOCOL_IOUT_A_1                (1)
#define SCP_PROTOCOL_IOUT_A_2                (2)
#define SCP_PROTOCOL_IOUT_A_3                (3)

#define SCP_PROTOCOL_VSTEP                   (0x96)
#define SCP_PROTOCOL_ISTEP                   (0x97)

#define SCP_PROTOCOL_MAX_VERR                (0x98)
#define SCP_PROTOCOL_MAX_IEER                (0x99)

#define SCP_PROTOCOL_MAX_STTIME              (0x9a)
#define SCP_PROTOCOL_MAX_RSPTIME             (0x9b)

#define SCP_PROTOCOL_RESERVED_0X9C           (0x9c)
#define SCP_PROTOCOL_RESERVED_0X9D           (0x9d)
#define SCP_PROTOCOL_RESERVED_0X9E           (0x9e)
#define SCP_PROTOCOL_RESERVED_0X9F           (0x9f)

/* adapter control information register */
#define SCP_PROTOCOL_CTRL_BYTE0              (0xa0)
#define SCP_PROTOCOL_OUTPUT_EN_MASK          (BIT(7))
#define SCP_PROTOCOL_OUTPUT_EN_SHIFT         (7)
#define SCP_PROTOCOL_OUTPUT_MODE_MASK        (BIT(6))
#define SCP_PROTOCOL_OUTPUT_MODE_SHIFT       (6)
#define SCP_PROTOCOL_RESET_MASK              (BIT(5))
#define SCP_PROTOCOL_RESET_SHIFT             (5)

#define SCP_PROTOCOL_OUTPUT_ENABLE           (1)
#define SCP_PROTOCOL_OUTPUT_DISABLE          (0)

#define SCP_PROTOCOL_OUTPUT_MODE_ENABLE      (1)
#define SCP_PROTOCOL_OUTPUT_MODE_DISABLE     (0)

#define SCP_PROTOCOL_RESET_ENABLE            (1)
#define SCP_PROTOCOL_RESET_DISABLE           (0)

#define SCP_PROTOCOL_CTRL_BYTE1              (0xa1)
#define SCP_PROTOCOL_DP_DELITCH_MASK         (BIT(4) | BIT(3))
#define SCP_PROTOCOL_DP_DELITCH_SHIFT        (3)
#define SCP_PROTOCOL_WATCHDOG_MASK           (BIT(2) | BIT(1) | BIT(0))
#define SCP_PROTOCOL_WATCHDOG_SHIFT          (0)

#define SCP_PROTOCOL_DP_DELITCH_1MS          (0x0)
#define SCP_PROTOCOL_DP_DELITCH_2MS          (0x1)
#define SCP_PROTOCOL_DP_DELITCH_3MS          (0x2)
#define SCP_PROTOCOL_DP_DELITCH_5MS          (0x3)

#define SCP_PROTOCOL_WATCHDOG_BITS_UNIT      (2) /* 1 bit means 0.5 second */

/* adapter status information register */
#define SCP_PROTOCOL_STATUS_BYTE0            (0xa2)
#define SCP_PROTOCOL_LEAKAGE_FLAG_MASK       (BIT(4))
#define SCP_PROTOCOL_LEAKAGE_FLAG_SHIFT      (4)
#define SCP_PROTOCOL_CCCV_STS_MASK           (BIT(4))
#define SCP_PROTOCOL_CCCV_STS_SHIFT          (4)

#define SCP_PROTOCOL_PORT_LEAKAGE            (1)
#define SCP_PROTOCOL_PORT_NOT_LEAKAGE        (0)

#define SCP_PROTOCOL_CC_STATUS               (1)
#define SCP_PROTOCOL_CV_STATUS               (0)

#define SCP_PROTOCOL_STATUS_BYTE1            (0xa3)
#define SCP_PROTOCOL_STATUS_BYTE2            (0xa4)

#define SCP_PROTOCOL_SSTS                    (0xa5)
#define SCP_PROTOCOL_SSTS_DPARTO_MASK        (BIT(3) | BIT(2) | BIT(1))
#define SCP_PROTOCOL_SSTS_DPARTO_SHIFT       (1)
#define SCP_PROTOCOL_SSTS_DROP_MASK          (BIT(0))
#define SCP_PROTOCOL_SSTS_DROP_SHIFT         (0)

#define SCP_PROTOCOL_DROP_ENABLE             (1)
#define SCP_PROTOCOL_DROP_FACTOR             (8)

#define SCP_PROTOCOL_INSIDE_TMP              (0xa6)
#define SCP_PROTOCOL_INSIDE_TMP_UNIT         (1) /* step: 1centigrade */

#define SCP_PROTOCOL_PORT_TMP                (0xa7)
#define SCP_PROTOCOL_PORT_TMP_UNIT           (1) /* step: 1centigrade */

#define SCP_PROTOCOL_READ_VOUT_HBYTE         (0xa8)
#define SCP_PROTOCOL_READ_VOUT_LBYTE         (0xa9)
#define SCP_PROTOCOL_READ_VOUT_STEP          (1) /* step: 1mv */

#define SCP_PROTOCOL_READ_IOUT_HBYTE         (0xaa)
#define SCP_PROTOCOL_READ_IOUT_LBYTE         (0xab)
#define SCP_PROTOCOL_READ_IOUT_STEP          (1) /* step: 1ma */

#define SCP_PROTOCOL_DAC_VSET_HBYTE          (0xac)
#define SCP_PROTOCOL_DAC_VSET_LBYTE          (0xad)

#define SCP_PROTOCOL_DAC_ISET_HBYTE          (0xae)
#define SCP_PROTOCOL_DAC_ISET_LBYTE          (0xaf)

#define SCP_PROTOCOL_SREAD_VOUT              (0xc8)
#define SCP_PROTOCOL_SREAD_VOUT_OFFSET       (3000)
#define SCP_PROTOCOL_SREAD_VOUT_STEP         (10) /* step: 10mv */

#define SCP_PROTOCOL_SREAD_IOUT              (0xc9)
#define SCP_PROTOCOL_SREAD_IOUT_STEP         (50) /* step: 50ma */

/* control adapter information response register */
#define SCP_PROTOCOL_VSET_BOUNDARY_HBYTE     (0xb0)
#define SCP_PROTOCOL_VSET_BOUNDARY_LBYTE     (0xb1)
#define SCP_PROTOCOL_VSET_BOUNDARY_STEP      (1) /* step: 1mv */

#define SCP_PROTOCOL_ISET_BOUNDARY_HBYTE     (0xb2)
#define SCP_PROTOCOL_ISET_BOUNDARY_LBYTE     (0xb3)
#define SCP_PROTOCOL_ISET_BOUNDARYSTEP       (1) /* step: 1ma */

#define SCP_PROTOCOL_MAX_VSET_OFFSET         (0xb4)
#define SCP_PROTOCOL_MAX_ISET_OFFSET         (0xb5)

#define SCP_PROTOCOL_RESERVED_0XB6           (0xb6)
#define SCP_PROTOCOL_RESERVED_0XB7           (0xb7)

#define SCP_PROTOCOL_VSET_HBYTE              (0xb8)
#define SCP_PROTOCOL_VSET_LBYTE              (0xb9)
#define SCP_PROTOCOL_VSET_STEP               (1) /* step: 1mv */

#define SCP_PROTOCOL_ISET_HBYTE              (0xba)
#define SCP_PROTOCOL_ISET_LBYTE              (0xbb)
#define SCP_PROTOCOL_ISET_STEP               (1) /* step: 1ma */

#define SCP_PROTOCOL_VSET_OFFSET_HBYTE       (0xbc)
#define SCP_PROTOCOL_VSET_OFFSET_LBYTE       (0xbd)

#define SCP_PROTOCOL_ISET_OFFSET_HBYTE       (0xbe)
#define SCP_PROTOCOL_ISET_OFFSET_LBYTE       (0xbf)

#define SCP_PROTOCOL_RESERVED_0XC0           (0xc0)
#define SCP_PROTOCOL_RESERVED_0XC1           (0xc1)
#define SCP_PROTOCOL_RESERVED_0XC2           (0xc2)
#define SCP_PROTOCOL_RESERVED_0XC3           (0xc3)
#define SCP_PROTOCOL_RESERVED_0XC4           (0xc4)
#define SCP_PROTOCOL_RESERVED_0XC5           (0xc5)

#define SCP_PROTOCOL_VROFFSET                (0xc6)
#define SCP_PROTOCOL_VSOFFSET                (0xc7)

#define SCP_PROTOCOL_VSSET                   (0xca)
#define SCP_PROTOCOL_VSSET_OFFSET            (3000)
#define SCP_PROTOCOL_VSSET_STEP              (10) /* step: 10mv */
#define SCP_PROTOCOL_VSSET_MAX_VOLT          (5500) /* max: 5500mv */

#define SCP_PROTOCOL_ISSET                   (0xcb)
#define SCP_PROTOCOL_ISSET_STEP              (50) /* step: 50ma */

#define SCP_PROTOCOL_STEP_VSET_OFFSET        (0xcc)
#define SCP_PROTOCOL_STEP_ISET_OFFSET        (0xcd)

/* adapter encrypt register */
#define SCP_PROTOCOL_KEY_INDEX               (0xce)
#define SCP_PROTOCOL_KEY_INDEX_BASE          (0x00)
#define SCP_PROTOCOL_KEY_INDEX_PUBLIC     (SCP_PROTOCOL_KEY_INDEX_BASE + 0x01)
#define SCP_PROTOCOL_KEY_INDEX_1          (SCP_PROTOCOL_KEY_INDEX_BASE + 0x02)
#define SCP_PROTOCOL_KEY_INDEX_2          (SCP_PROTOCOL_KEY_INDEX_BASE + 0x03)
#define SCP_PROTOCOL_KEY_INDEX_3          (SCP_PROTOCOL_KEY_INDEX_BASE + 0x04)
#define SCP_PROTOCOL_KEY_INDEX_4          (SCP_PROTOCOL_KEY_INDEX_BASE + 0x05)
#define SCP_PROTOCOL_KEY_INDEX_5          (SCP_PROTOCOL_KEY_INDEX_BASE + 0x06)
#define SCP_PROTOCOL_KEY_INDEX_6          (SCP_PROTOCOL_KEY_INDEX_BASE + 0x07)
#define SCP_PROTOCOL_KEY_INDEX_7          (SCP_PROTOCOL_KEY_INDEX_BASE + 0x08)
#define SCP_PROTOCOL_KEY_INDEX_8          (SCP_PROTOCOL_KEY_INDEX_BASE + 0x09)
#define SCP_PROTOCOL_KEY_INDEX_9          (SCP_PROTOCOL_KEY_INDEX_BASE + 0x0a)
#define SCP_PROTOCOL_KEY_INDEX_10         (SCP_PROTOCOL_KEY_INDEX_BASE + 0x0b)
#define SCP_PROTOCOL_KEY_INDEX_RELEASE    (SCP_PROTOCOL_KEY_INDEX_BASE + 0xff)

#define SCP_PROTOCOL_ENCRYPT_INFO            (0xcf)
#define SCP_PROTOCOL_ENCRYPT_ENABLE_MASK     (BIT(7))
#define SCP_PROTOCOL_ENCRYPT_ENABLE_SHIFT    (7)
#define SCP_PROTOCOL_ENCRYPT_COMPLETED_MASK  (BIT(6))
#define SCP_PROTOCOL_ENCRYPT_COMPLETED_SHIFT (6)

#define SCP_PROTOCOL_ENCRYPT_DISABLE         (0)
#define SCP_PROTOCOL_ENCRYPT_ENABLE          (1)

#define SCP_PROTOCOL_ENCRYPT_NOT_COMPLETED   (0)
#define SCP_PROTOCOL_ENCRYPT_COMPLETED       (1)

#define SCP_PROTOCOL_ENCRYPT_RANDOM_WR_BASE  (0xa0)
#define SCP_PROTOCOL_ENCRYPT_RANDOM_WR_SIZE  (8)

#define SCP_PROTOCOL_ENCRYPT_RANDOM_RD_BASE  (0xa8)
#define SCP_PROTOCOL_ENCRYPT_RANDOM_RD_SIZE  (8)

#define SCP_PROTOCOL_ENCRYPT_HASH_RD_BASE    (0xb0)
#define SCP_PROTOCOL_ENCRYPT_HASH_RD_SIZE    (16)

#define SCP_PROTOCOL_USBPD_INFO              (0xcf)
#define SCP_PROTOCOL_USBPD_ENABLE_MASK       (BIT(0))
#define SCP_PROTOCOL_USBPD_ENABLE_SHIFT      (0)

#define SCP_PROTOCOL_USBPD_DISABLE           (1)
#define SCP_PROTOCOL_USBPD_ENABLE            (0)

enum scp_protocol_rw_error_flag {
	NO_RW_ERROR_FLAG = 0,
	RW_ERROR_FLAG = 1,
};

enum scp_protocol_read_flag {
	NOT_READ_FLAG = 0,
	HAS_READ_FLAG = 2,
};

enum scp_protocol_error_code {
	SCP_DETECT_OTHER = -1,
	SCP_DETECT_SUCC = 0,
	SCP_DETECT_FAIL = 1,
};

enum scp_protocol_reset_time {
	RESET_TIME_10MS = 10,
	RESET_TIME_20MS = 20,
	RESET_TIME_30MS = 30,
	RESET_TIME_40MS = 40,
	RESET_TIME_50MS = 50,
	RESET_TIME_51MS = 51,
};

enum scp_protocol_retry_num {
	RETRY_ONE = 1,
	RETRY_TWO = 2,
	RETRY_THREE = 3,
	RETRY_FOUR = 4,
	RETRY_FIVE = 5,
	RETRY_SIX = 6,
};

enum scp_protocol_byte_num {
	BYTE_ONE = 1,
	BYTE_TWO = 2,
	BYTE_FOUR = 4,
	BYTE_EIGHT = 8,
	BYTE_SIXTEEN = 16,
};

enum scp_protocol_chip_vendor {
	ADAPTER_CHIP_RICHTEK,
	ADAPTER_CHIP_IWATT,
	ADAPTER_CHIP_WELTREND,
	ADAPTER_CHIP_ID0X32,
};

enum scp_protocol_device_id {
	DEVICE_ID_BEGIN = 0,

	DEVICE_ID_FSA9685 = DEVICE_ID_BEGIN, /* for fsa9685 */
	DEVICE_ID_SCHARGER_V300, /* for scharger_v300 */
	DEVICE_ID_SCHARGER_V600, /* for scharger_v600 */
	DEVICE_ID_FUSB3601, /* for fusb3601 */

	DEVICE_ID_END,
};

struct scp_protocol_device_info {
	int support_mode; /* adapter support mode */
	int vid_h; /* vendor id */
	int vid_l;
	int mid_h; /* module id */
	int mid_l;
	int serial_h; /* serial no */
	int serial_l;
	int chip_id; /* chip id */
	int hwver; /* hardware version */
	int fwver_h; /* firmware version */
	int fwver_l;
	int chip_vid; /* chip vendor id */
	int min_volt; /* minimum voltage */
	int max_volt; /* maximum voltage */
	int min_cur; /* minimum current */
	int max_cur; /* maximum current */
	int vid_rd_flag;
	int mid_rd_flag;
	int serial_rd_flag;
	int chip_id_rd_flag;
	int hwver_rd_flag;
	int fwver_rd_flag;
	int min_volt_rd_flag;
	int max_volt_rd_flag;
	int min_cur_rd_flag;
	int max_cur_rd_flag;
	int rw_error_flag;
	int adp_b_type1;
	int adp_b_type1_rd_falg;
};

struct scp_protocol_ops {
	const char *chip_name;
	int (*reg_read)(int reg, int *val, int num);
	int (*reg_write)(int reg, int *val, int num);
	int (*reg_multi_read)(u8 reg, u8 *val, u8 num);
	int (*detect_adapter)(void);
	int (*soft_reset_master)(void);
	int (*soft_reset_slave)(void);
	int (*pre_init)(void); /* process non protocol flow */
	int (*post_init)(void);  /* process non protocol flow */
	int (*pre_exit)(void); /* process non protocol flow */
	int (*post_exit)(void);  /* process non protocol flow */
};

struct scp_protocol_dev {
	struct device *dev;
	unsigned char encrypt_random_host[BYTE_EIGHT];
	unsigned char encrypt_random_slave[BYTE_EIGHT];
	unsigned char encrypt_hash_slave[BYTE_SIXTEEN];
	struct scp_protocol_device_info info;

	int dev_id;
	struct scp_protocol_ops *p_ops;
};

#ifdef CONFIG_CHARGER_PROTOCOL_SCP
int scp_protocol_ops_register(struct scp_protocol_ops *ops);

#else

static inline int scp_protocol_ops_register(struct scp_protocol_ops *ops)
{
	return -1;
}
#endif /* CONFIG_CHARGER_PROTOCOL_SCP */

#endif /* _CHARGER_PROTOCOL_SCP_H_ */
