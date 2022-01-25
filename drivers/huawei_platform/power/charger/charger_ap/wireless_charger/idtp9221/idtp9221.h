/*
 * idtp9221.h
 *
 * idtp9221 macro, addr etc.
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

#ifndef _IDTP9221_H_
#define _IDTP9221_H_

#define IDTP9221_CHIP_ID                      (0x9220)
#define STWLC33_CHIP_ID                       (0x21)
#define ERR_CHIP_ID                           (0)

/*
 * idtp9221 RX mode
 */
#define IDT9221_ADDR_LEN                      (2)
/* send message register */
#define IDT9221_RX_TO_TX_HEADER_ADDR          (0x0050)
#define IDT9221_RX_TO_TX_CMD_ADDR             (0x0051)
#define IDT9221_RX_TO_TX_DATA_ADDR            (0x0052)
#define IDT9221_RX_TO_TX_DATA_LEN             (4)
/* RX2TX: bit(0): header;  bit(1) : command;  bit(2,3,4,5) : data */
#define IDT9221_RX_TO_TX_PACKET_LEN           (6)
#define IDT9221_RX_TO_TX_PACKET_BUFF_LEN      (8)
#define IDT9221_RX_TO_TX_PACKET_STR_LEN       (64)
/* received message register */
#define IDT9221_TX_TO_RX_HEADER_ADDR          (0x0058)
#define IDT9221_TX_TO_RX_CMD_ADDR             (0x0059)
#define IDT9221_TX_TO_RX_DATA_ADDR            (0x005a)
#define IDT9221_TX_TO_RX_MESSAGE_LEN          (5)
#define IDT9221_TX_TO_RX_DATA_LEN             (4)
/* system operating mode register */
#define IDT9221_SYS_MODE_ADDR                 (0x004d)
#define IDT9221_OTPONLY_MODE                  (0x10)
#define IDT9221_WPC_MODE                      (0x01)
#define IDT9221_PMA_MODE                      (0x02)
#define IDT9221_TX_WPCMODE                    (0x04)
#define IDT9221_BACKPOWERED                   (0x08)
#define IDT9221_RAMPROGRAM_MODE               (0x40)
/* command register */
#define IDT9221_CMD_ADDR                      (0x004e)
#define IDT9221_CMD_SEND_RX_DATA              (0x01)
#define IDT9221_CMD_SEND_EPT                  (0x08)
#define IDT9221_CMD_CLEAR_INTERRUPT           (0x20)
#define IDT9221_CMD_SWITCH_TO_SRAM            (0x40)
#define IDT9221_CMD_SEND_FAST_CHRG            (0x80)
#define IDT9221_CMD1_ADDR                     (0x004f)
#define IDT9221_CMD1_UNLOCK_SWITCH            (0x5a)
/* interrupt status register */
#define IDT9221_RX_INT_STATUS_ADDR            (0x0034)
#define IDT9221_RX_STATUS_OCP                 (0x0001)
#define IDT9221_RX_STATUS_OVP                 (0x0002)
#define IDT9221_RX_STATUS_OTP                 (0x0004)
#define IDT9221_RX_STATUS_READY               (0x0008)
#define IDT9221_RX_STATUS_TXDATA_RECEIVED     (0x0010)
#define IDT9221_RX_STATUS_MODE_CHANGE         (0x0020)
#define IDT9221_RX_STATUS_LDO_ON              (0x0040)
#define IDT9221_RX_STATUS_LDO_OFF             (0x0080)
#define IDT9221_RX_STATUS_TX2RX_ACK           (0x0800)
#define IDT9221_RX_STATUS_POWER_ON            (0x8000)
#define IDT9221_RX_INT_ENABLE_ADDR            (0x0038)
#define IDT9221_RX_INT_ENABLE_LEN             (2)
#define IDT9221_RX_INT_CLEAR_ADDR             (0x0056)
#define IDT9221_RX_INT_CLEAR_LEN              (2)
/* end of power transfer register */
#define IDT9221_RX_EPT_ADDR                   (0x003b)
#define IDT9221_RX_EPT_UNKOWN                 (0x00)
#define IDT9221_RX_EPT_CHRG_COMPLETE          (0x01)
#define IDT9221_RX_EPT_INTERNAL_FAULT         (0x02)
#define IDT9221_RX_EPT_OTP                    (0x03)
#define IDT9221_RX_EPT_OVP                    (0x04)
#define IDT9221_RX_EPT_OCP                    (0x05)
#define IDT9221_RX_EPT_BATT_FAILURE           (0x06)
#define IDT9221_RX_EPT_RESERVED               (0x07)
#define IDT9221_RX_EPT_NO_RESPONSE            (0x08)
#define IDT9221_RX_ERR_VRECT                  (0xA0)
#define IDT9221_RX_ERR_VOUT                   (0xA1)
/* vout register */
#define IDT9221_RX_GET_VOUT_ADDR              (0x003c)
#define IDT9221_RX_VOUT_VALUE_MAX             (12600)
#define IDT9221_RX_VOUT_REG_MAX               (4095)
#define IDT9221_RX_SET_VOUT_ADDR              (0x003e)
#define IDT9221_RX_VOUT_LEN                   (2)
#define IDT9221_RX_VOUT_STEP                  (100)
#define IDT9221_RX_VOUT_OFFSET                (3500)
#define IDT9221_RX_VOUT_MAX                   (12500)
#define IDT9221_RX_VOUT_MIN                   (3500)
/* vrect register */
#define IDT9221_RX_SET_VRECT_ADDR             (0x003f)
#define IDT9221_RX_VRECT_LEN                  (2)
#define IDT9221_RX_GET_VRECT_ADDR             (0x0040)
#define IDT9221_RX_VRECT_VALUE_MAX            (21000)
#define IDT9221_RX_VRECT_REG_MAX              (4095)
#define IDT9221_RX_VRECT_MAX                  (18000)
/* iout register */
#define IDT9221_RX_GET_IOUT_ADDR              (0x0044)
#define IDT9221_RX_IOUT_LEN                   (2)
#define IDT9221_RX_IOUT_MAX                   (1300)
#define IDT9221_RX_IOUT_MIN                   (100)
/* Signal Strength Register */
#define IDT9221_RX_SS_ADDR                    (0x004b)
#define IDT9221_RX_SS_MIN                     (0)
#define IDT9221_RX_SS_MAX                     (255)

/* fop register */
#define IDT9221_RX_GET_FOP_ADDR               (0x0048)
#define IDT9221_RX_FOP_LEN                    (2)
#define IDT9221_RX_FOP_COEF                   (64*6000)

/* boost register */
#define IDT9221_SET_TX_VOUT_ADDR              (0x0078)
#define IDT9221_SET_TX_VOUT_LEN               (2)
#define IDT9221_SET_TX_VOUT_SLEEP_TIME        (50)
#define IDT9221_SET_TX_VOUT_TIMEOUT           (5000)
#define IDT9221_TX_VOUT_ERR_LTH               (500) /* lower threhold */
#define IDT9221_TX_VOUT_ERR_UTH               (1000) /* upper threhold */

/* chip id register */
#define IDT9221_RX_CHIP_ID_ADDR               (0x0000)
/* firmware version register */
#define IDT9221_RX_OTP_FW_VERSION_ADDR        (0x001C)
#define IDT9221_RX_OTP_FW_VERSION_LEN         (4)
#define IDT9221_RX_OTP_FW_VERSION_STRING_LEN  (32)
/* user flags */
#define IDT9221_RX_USER_FLAGS_ADDR            (0x0063)
#define IDT9221_RX_USER_FLAGS                 (0x30)
/* fast_charge_Timer */
#define IDT9221_RX_FC_TIMER_ADDR              (0x0064)
#define IDT9221_RX_FC_TIMEOUT_ADDR            (0x0066)
#define IDT9221_RX_FC_TIMEOUT                 (1000)
/* FOD COEF register */
#define IDT9221_RX_FOD_COEF_STSRT_ADDR        (0x0068)
#define IDT9221_RX_FOD_COEF_LEN               (8)
#define IDT9221_RX_FOD_COEF_STRING_LEN        (64)
/* LDO register */
#define IDT9221_LDO_ADDR                      (0x3404)
/* vrect coef register */
#define IDT9221_RX_PWR_KNEE_ADDR              (0xb2)
#define IDT9221_RX_VRCORR_FACTOR_ADDR         (0xb3)
#define IDT9221_RX_VRMAX_COR_ADDR             (0xb4)
#define IDT9221_RX_VRMIN_COR_ADDR             (0xb6)

/* otp and sram */
#define IDT9221_BOOTLOADER_ADDR               (0x1c00)
#define IDT9221_KEY_ADDR                      (0x3000)
#define IDT9221_KEY_VALUE                     (0x5a)
#define IDT9221_M0_ADDR                       (0x3040)
#define IDT9221_M0_UNDO                       (0x00)
#define IDT9221_M0_EXE                        (0x10)
#define IDT9221_M0_HOLD                       (0x11)
#define IDT9221_M0_RESET                      (0x80)
#define IDT9221_MAP_ADDR                      (0x3048)
#define IDT9221_MAP_RAM2OTP                   (0x80)
#define IDT9221_MAP_OTP2RAM                   (0xDA)
#define IDT9221_MAP_UNMAPING                  (0x00)
#define IDT9221_OTP_VRR_ADDR                  (0x5c04)
#define IDT9221_OTP_VRR_3V                    (0x05)
#define IDT9221_OTP_VRR_EN                    (0x11)
#define IDT9221_OTP_CTRL_ADDR                 (0x5c00)
#define IDT9221_PWM_CTRL_ADDR                 (0x3c00)
#define IDT9221_PWM_CTRL_DISABLE              (0x80)
#define IDT9221_OTP_START_ADDR                (0x8000)

#define IDT9221_OTP_STARTADDR_OFFSET          (4)
#define IDT9221_OTP_STARTADDR_SIZE            (2)
#define IDT9221_OTP_DATALEN_OFFSET            (6)
#define IDT9221_OTP_DATALEN_SIZE              (2)
#define IDT9221_OTP_CHECKSUM_OFFSET           (8)
#define IDT9221_OTP_CHECKSUM_SIZE             (2)
#define IDT9221_OTP_DATA_OFFSET               (10)
#define IDT9221_OTP_SRAM_ADDR                 (0x0400)
#define IDT9221_OTP_START_WRITE               (1)

#define IDT9221_OTP_RESET_VALUE               (00)
#define IDT9221_OTP_BUFFER_VALID              (01)
#define IDT9221_OTP_FINISH_OK                 (02)
#define IDT9221_OTP_PROGRAM_ERR               (04)
#define IDT9221_OTP_CHECKSUM_ERR              (08)
#define IDT9221_OTP_NOT_POSSIBLE              (16)
#define IDT9221_OTP_SLEEP_TIME                (100)
#define IDT9221_OTP_SIZE_CHECK                (128)
#define IDT9221_OTP_PROGRAMED                 (1)
#define IDT9221_OTP_NON_PROGRAMED             (0)
#define IDT9221_OTP_ERR_PROGRAMED             (2)

#define IDT9221_RX_SRAMUPDATE_ADDR            (0x0600)
#define IDT9221_TX_SRAMUPDATE_ADDR            (0x0800)
#define IDT9221_PAGE_SIZE                     (128)

/* firmware version register */
#define STWLC33_RX_OTP_FW_VERSION_ADDR        (0x0008)
#define STWLC33_RX_OTP_FW_VERSION_LEN         (2)
/* FOD COEF register */
#define STWLC33_RX_FOD_COEF_LEN               (8)
/* nvm data register */
#define STWLC33_NVM_SEC_NO_MAX                (15)
#define STWLC33_SEC_NO_SIZE                   (4)
#define STWLC33_NVM_REG_SIZE                  (6)
#define STWLC33_NVM_VALUE_SIZE                (128)
#define STWLC33_NVM_WR_TIME                   (40)
#define STWLC33_NVM_SEC_VAL_SIZE              (0x20)
#define STWLC33_NVM_RD_CMD                    (0x20)
#define STWLC33_NVM_WR_CMD                    (0x40)
#define STWLC33_NVM_RD_ADDR                   (0x0100)
#define STWLC33_NVM_WR_ADDR                   (0x0100)
#define STWLC33_NVM_PROGRAMED                 (1)
#define STWLC33_NVM_NON_PROGRAMED             (0)
#define STWLC33_NVM_ERR_PROGRAMED             (2)
/* sram update register */
#define STWLC33_ACT_CMD_ADDR                  (0x0186)
#define STWLC33_SRAM_SIZE_ADDR                (0x0184)
#define STWLC33_OFFSET_REG_ADDR               (0x0180)
#define STWLC33_OFFSET_VALUE_SIZE             (4)
#define STWLC33_OFFSET_REG_SIZE               (6)
#define STWLC33_CMD_STATUS_ADDR               (0x0185)
#define STWLC33_RX_SRAMUPDATE_ADDR            (0x0100)
#define STWLC33_TX_SRAMUPDATE_ADDR            (0x0100)
#define STWLC33_PAGE_SIZE                     (128)
#define STWLC33_ADDR_LEN                      (2)
#define STWLC33_READ_CMD_VALUE                (0x01)
#define STWLC33_WRITE_CMD_VALUE               (0x02)
#define STWLC33_EXEC_CMD_VALUE                (0x04)
#define STWLC33_SRAM_EXEC_TIME                (100)
/* ldo drop register */
#define STWLC33_LDO_DROP0_VAL                 0x13
#define STWLC33_LDO_DROP1_VAL                 (0x32)
#define STWLC33_LDO_DROP2_VAL                 (0xd)
#define STWLC33_LDO_DROP3_VAL                 (0x5)
#define STWLC33_LDO_CUR_TH1_VAL               (0x19)
#define STWLC33_LDO_CUR_TH2_VAL               (0x32)
#define STWLC33_LDO_CUR_TH3_VAL               (0x3f)
#define STWLC33_LDO_DROP0_SC_VAL              0x2f
#define STWLC33_LDO_DROP1_SC_VAL              (0x9)
#define STWLC33_LDO_DROP2_SC_VAL              (0x5)
#define STWLC33_LDO_DROP3_SC_VAL              (0x5)
#define STWLC33_LDO_CUR_TH1_SC_VAL            (0xd)
#define STWLC33_LDO_CUR_TH2_SC_VAL            (0x14)
#define STWLC33_LDO_CUR_TH3_SC_VAL            (0x3f)
#define STWLC33_LDO_DROP0_ADDR                (0x00b2)
#define STWLC33_LDO_DROP1_ADDR                (0x00b3)
#define STWLC33_LDO_DROP2_ADDR                (0x00b4)
#define STWLC33_LDO_DROP3_ADDR                (0x00b5)
#define STWLC33_LDO_CUR_TH1_ADDR              (0x00b6)
#define STWLC33_LDO_CUR_TH2_ADDR              (0x00b7)
#define STWLC33_LDO_CUR_TH3_ADDR              (0x00b8)
#define STWLC33_VRECT_IGNORE_CNT              30
#define STWLC33_VRECT_SLEEP_TIME              50
#define STWLC33_RX_VRECT_READ_CNT             50
#define STWLC33_VRECT_INVALID_VAL             2000
#define STWLC33_VRECT_THRES_VAL               5112

#define STWLC33_TX_ENABLE_VAL                 (0x01)
#define STWLC33_TX_ENABLE_ADDR                (0x0076)
#define STWLC33_RAM_VER_ADDR                  (0x000e)
#define STWLC33_RX_CHIP_TEMP_ADDR             (0x0046)
#define STWLC33_GET_RX_CHIP_TEMP_RETRY_CNT    (3)
#define STWLC33_CHIP_RST_VAL                  0x02
#define STWLC33_CHIP_RST_ADDR                 0x0188
#define STWLC33_IOUT_CALI_VAL                 0x0e
#define STWLC33_IOUT_CALI_ADDR                0x018f
#define STWLC33_IOUT_ADC_VAL_ADDR             0x0192
#define STWLC33_IOUT_ADC_VAL_LTH              241
#define STWLC33_IOUT_ADC_VAL_HTH              441
#define STWLC33_IOUT_ADC_CRITICAL_VAL         341
#define STWLC33_IOUT_CALI_VAL_ADDR            0x0080

#define IDT9221_OTP_FW_HEAD                   "0x88 0x66"
#define IDT9221_OTP_FW_VERSION_020BH          "0x88 0x66 0x02 0x0b"
#define IDT9221_OTP_FW_VERSION_030FH          "0x88 0x66 0x03 0x0f"
#define STWLC33_OTP_FW_VERSION_1100H          "0x88 0x66 0x02 0x17"


/*
 * idtp9221 command
 */
#define IDT9221_CMD_SET_TX_FOP                (0x03)
#define IDT9221_CMD_GET_TX_FOP                (0x04)
#define IDT9221_CMD_GET_TX_VERSION            (0x05)
#define IDT9221_CMD_GET_TX_IIN                (0x06)
#define IDT9221_CMD_GET_TX_VIN                (0x07)
#define IDT9221_CMD_GET_TX_ADAPTER_TYPE       (0x0b)
#define IDT9221_CMD_SEND_READY                (0x0F)
#define IDT9221_CMD_SEND_SN                   (0x12)
#define IDT9221_CMD_SEND_BATT_TEMP            (0x16)
#define IDT9221_CMD_SEND_BATT_CAPACITY        (0x17)
#define IDT9221_CMD_SET_CURRENT_LIMIT         (0x18)
#define IDT9221_CMD_START_SAMPLE              (0x1d)
#define IDT9221_CMD_STOP_SAMPLE               (0x1e)
#define IDT9221_CMD_RX_BOOST_SUCC             (0x1f)
#define IDT9221_CMD_CERT_SUCC                 (0x20)
#define IDT9221_CMD_CERT_FAIL                 (0x21)
#define IDT9221_CMD_START_CERT                (0x36)
#define IDT9221_CMD_GET_HASH                  (0x38)
#define IDT9221_CMD_GET_TX_ID                 (0x3b)
#define IDT9221_CMD_GET_TX_CAP                (0x41)
#define IDT9221_CMD_GET_TX_EXTRA_CAP          (0x49)

#define IDT9221_CMD_SEND_CHRG_STATE           (0x43)
#define IDT9221_CMD_FIX_TX_FOP                (0x44)
#define IDT9221_CMD_UNFIX_TX_FOP              (0x45)
#define IDT9221_CMD_SEND_QVAL                 (0x48)

#define IDT9221_CMD_ACK                       (0xff)
#define IDT9221_CMD_ACK_HEAD                  (0x1e)

/* cmd: 0x41 */
#define IDT9221_TX_CAP_VOUT_STEP              (100)
#define IDT9221_TX_CAP_IOUT_STEP              (100)
#define IDT9221_TX_CAP_CABLE_OK_MASK          BIT(0)
#define IDT9221_TX_CAP_CAN_BOOST_MASK         BIT(1)
#define IDT9221_TX_CAP_NO_NEED_CERT_MASK      BIT(4)
#define IDT9221_TX_CAP_SUPPORT_SCP_MASK       BIT(5)
#define IDT9221_TX_CAP_SUPPORT_12V_MASK       BIT(6)
#define IDT9221_TX_CAP_SUPPORT_EXTRA_BIT_MASK BIT(7)

/* cmd: 0x49 */
#define IDT9221_TX_CAP_SUPPORT_FAN_MASK       BIT(2)
#define IDT9221_TX_CAP_SUPPORT_TEC_MASK       BIT(3)
#define IDT9221_TX_CAP_SUPPORT_QVAL_MASK      BIT(4)

#define IDT9221_TX_FOP_LEN                    (2)
#define IDT9221_FIXED_FOP_MAX                 (148)
#define IDT9221_FIXED_FOP_MIN                 (120)

#define IDT9221_TX_ADAPTER_TYPE_LEN           (2)
#define IDT9221_TX_ID_LEN                     (3)
#define IDT9221_LIMIT_CURRENT_TIME            (500)
#define IDT9221_BATT_CAPACITY_LEN             (2)
#define IDT9221_BATT_TEMP_LEN                 (1)
#define IDT9221_BATT_TEMP_MAX                 (127)
#define IDT9221_CHRG_STATE_LEN                (1)
#define IDT9221_QVAL_LEN                      (2)

#define IDT9221_SHUTDOWN_SLEEP_TIME           (200)
#define IDT9221_RCV_MSG_SLEEP_TIME            (100)
#define IDT9221_RCV_MSG_SLEEP_CNT             (10)
#define IDT9221_WAIT_FOR_ACK_SLEEP_TIME       (100)
#define IDT9221_WAIT_FOR_ACK_RETRY_CNT        (5)
#define IDT9221_SNED_MSG_RETRY_CNT            (2)
#define IDT9221_GET_TX_PARA_RETRY_CNT         (2)

#define IDT9221_TX_FW_VERSION_LEN             (4)
#define IDT9221_TX_FW_VERSION_STRING_LEN      (32)

#define IDT9221_RX_TMP_BUFF_LEN               (32)

/*
 * idtp9221 TX mode
 */

/* command register */
#define IDT9221_CMD3_ADDR                     (0x0076)
#define IDT9221_CMD3_TX_EN_MASK               BIT(0)
#define IDT9221_CMD3_TX_EN_SHIFT              0
#define IDT9221_CMD3_TX_CLRINT_MASK           BIT(1)
#define IDT9221_CMD3_TX_CLRINT_SHIFT          1
#define IDT9221_CMD3_TX_DIS_MASK              BIT(2)
#define IDT9221_CMD3_TX_DIS_SHIFT             2
#define IDT9221_CMD3_TX_SEND_FSK_MASK         BIT(3)
#define IDT9221_CMD3_TX_SEND_FSK_SHIFT        3
#define IDT9221_CMD3_TX_FOD_EN_MASK           BIT(5)
#define IDT9221_CMD3_TX_FOD_EN_SHIFT          5

/* TX vrect register */
#define IDT9221_TX_VRECT_ADDR                 (0x0072)
#define IDT9221_TX_IIN_ADDR                   (0x006e)
#define IDT9221_TX_VIN_ADDR                   (0x0070)
/* TX FOP register */
#define IDT9221_TX_FOP_ADDR                   (0x0062)
#define IDT9221_TX_MAX_FOP_ADDR               (0x0064)
#define IDT9221_TX_MAX_FOP_VAL                (148) /* kHz */
#define IDT9221_TX_MIN_FOP_ADDR               (0x0066)
#define IDT9221_TX_MIN_FOP_VAL                (113) /* kHz */
/* TX ping frequency  register */
#define IDT9221_TX_PING_FREQUENCY_ADDR        (0x0068)
#define IDT9221_TX_PING_FREQUENCY_MIN         (100) /* kHz */
#define IDT9221_TX_PING_FREQUENCY_MAX         (150) /* kHz */
#define IDT9221_TX_PING_FREQUENCY_INIT        (125)
/* TX OCP register */
#define IDT9221_TX_OCP_ADDR                   (0x006a)
#define IDT9221_TX_OCP_VAL                    (2000)
/* TX OVP register */
#define IDT9221_TX_OVP_ADDR                   (0x006c)
#define IDT9221_TX_OVP_VAL                    (7500)
/* TX ping interval register */
#define IDT9221_TX_PING_INTERVAL_ADDR         (0x0079)
#define IDT9221_TX_PING_INTERVAL_MIN          (200) /* ms */
#define IDT9221_TX_PING_INTERVAL_MAX          (1000) /* ms */
#define IDT9221_TX_PING_INTERVAL_INIT         (500) /* ms */
#define IDT9221_TX_PING_INTERVAL_STEP         (10)
/* TX chip temp register */
#define IDT9221_CHIP_TEMP_ADDR                (0x007A)
/* interrupt status register(0x34), reuse in TX mode */
#define IDT9221_TX_STATUS_EPT_TYPE            (0x01) /* end power transfer */
#define IDT9221_TX_STATUS_START_DPING         (0x02) /* start digital ping */
#define IDT9221_TX_STATUS_GET_SS              (0x04) /* get signal strength */
#define IDT9221_TX_STATUS_GET_ID              (0x08) /* get id packet */
#define IDT9221_TX_STATUS_GET_CFG             (0x10) /* get configure packet */
#define IDT9221_TX_STATUS_GET_PPP             (0x20) /* get ppp*/
/* TX EPT type register */
#define IDT9221_TX_EPT_TYPE_ADDR              (0X0074)
#define IDT9221_TX_EPT_CMD                    (0x0001)
#define IDT9221_TX_EPT_SS                     (0x0002)
#define IDT9221_TX_EPT_ID                     (0x0004)
#define IDT9221_TX_EPT_XID                    (0x0008)
#define IDT9221_TX_EPT_CFG_COUNT_ERR          (0x0010)
#define IDT9221_TX_EPT_PCH                    (0x0020)
#define IDT9221_TX_EPT_FIRSTCEP               (0x0040)
#define IDT9221_TX_EPT_TIMEOUT                (0x0080)
#define IDT9221_TX_EPT_CEP_TIMEOUT            (0x0100)
#define IDT9221_TX_EPT_EPT_RPP_TIMEOUT        (0x0200)
#define IDT9221_TX_EPT_OCP                    (0x0400)
#define IDT9221_TX_EPT_OVP                    (0x0800)
#define IDT9221_TX_EPT_LVP                    (0x1000)
#define IDT9221_TX_EPT_FOD                    (0x2000)
#define IDT9221_TX_EPT_OTP                    (0x4000)
#define IDT9221_TX_EPT_LCP                    (0x8000)
/* TX FOD th register */
#define IDT9221_TX_FOD_THD0_ADDR              (0x00B2)
#define IDT9221_TX_FOD_THD0_VAL               (300)

#define IDT9221_QI_SIGNAL_STRENGTH            (0x01)

#define IDT9221_TX_ADAPTER_OTG                (0x09)
#define IDT9221_TX_ADAPTER_OTG_MAX_VOL        (50)
#define IDT9221_TX_ADAPTER_OTG_MAX_CUR        (5)

struct idtp9221_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct mutex mutex_irq;
	u32 rx_fod_5v[IDT9221_RX_FOD_COEF_LEN];
	u32 rx_fod_9v[IDT9221_RX_FOD_COEF_LEN];
	u32 rx_fod_12v[IDT9221_RX_FOD_COEF_LEN];
	u32 st_rx_fod_5v[STWLC33_RX_FOD_COEF_LEN];
	u32 st_rx_fod_9v[STWLC33_RX_FOD_COEF_LEN];
	u32 st_rx_fod_12v[STWLC33_RX_FOD_COEF_LEN];
	int rx_ss_good_lth;
	int tx_fod_th_5v;
	int gpio_en;
	int gpio_en_valid_val;
	int gpio_sleep_en;
	int gpio_int;
	int irq_int;
	int irq_active;
	u16 irq_val;
	u16 ept_type;
	int irq_cnt;
	int sc_rx_vrmax_gap;
	int i2c_trans_fail_limit;
	struct work_struct rx_ready_check_work;
};

enum wireless_rx_sysfs_type {
	IDT9221_SYSFS_CHIP_ID = 0,
	IDT9221_SYSFS_FW_VERSION,
	IDT9221_SYSFS_PROGRAM_OTP,
	IDT9221_SYSFS_TEST,
	IDT9221_SYSFS_VOUT,
	IDT9221_SYSFS_VRECT,
	IDT9221_SYSFS_VSET,
	IDT9221_SYSFS_IOUT,
};

#endif /* _IDTP9221_H_ */
