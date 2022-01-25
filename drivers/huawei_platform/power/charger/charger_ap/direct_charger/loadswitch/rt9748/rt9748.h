/*
 * rt9748.h
 *
 * rt9748 driver
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

#ifndef _RT9748_H_
#define _RT9748_H_

#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <huawei_platform/power/direct_charger.h>

#ifndef BIT
#define BIT(x)    (1 << (x))
#endif

struct rt9748_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct nty_data nty_data;
	int gpio_int;
	int irq_int;
	int irq_active;
	int gpio_en;
	int chip_already_init;
	int device_id;
	int sense_r_mohm;
};

enum loadswitch {
	loadswitch_rt9748 = 0,
	loadswitch_bq25870 = 1,
	loadswitch_fair_child = 2,
	loadswitch_nxp = 3,
};

#define SENSE_R_2_MOHM                     20 /* 2 mohm */
#define SENSE_R_2P5_MOHM                   25 /* 2.5 mohm */
#define SENSE_R_5_MOHM                     50 /* 5 mohm */
#define SENSE_R_10_MOHM                    100 /* 10 mohm */

#define DEVICE_ID_RICHTEK                  0
#define DEVICE_ID_TI                       1
#define DEVICE_ID_FSA                      2
#define DEVICE_ID1_NXP                     3
#define DEVICE_ID2_NXP                     7
#define REG0_DEV_ID                        0x07
#define DEVICE_ID_GET_FAIL                 -1
#define GET_BIT_3_MASK                     1
#define IS_RICHTEK                         1
#define NOT_USED                           0
#define USED                               1

#define RT9748_INIT_FINISH                 1
#define RT9748_NOT_INIT                    0
#define RT9748_ENABLE_INTERRUPT_NOTIFY     1
#define RT9748_DISABLE_INTERRUPT_NOTIFY    0

#define LENTH_OF_BYTE                      8
#define LOADSWITCH_DEV_INFO_REG0           0x00

#define RT9748_DEVICE_INFO                 0x0
#define RT9748_EVENT_1_MASK                0x1
#define RT9748_EVENT_1_MASK_INIT           0x06

#define RT9748_EVENT_2_MASK                0x02
#define RT9748_EVENT_2_MASK_INIT           0xcc

#define RT9748_EVENT_1                     0x03
#define RT9748_VBUS_OVP_FLT                BIT(7)
#define RT9748_IBUS_REVERSE_OCP_FLT        BIT(0)

#define RT9748_EVENT_2                     0x04
#define RT9748_VDROP_OVP_FLT               BIT(4)
#define RT9748_OTP_FLT                     BIT(1)
#define RT9748_INPUT_OCP_FLT               BIT(0)

#define RT9748_EVENT_1_EN                  0x05
#define RT9748_EVENT_1_EN_INIT             0xf8
#define RT9748_PD_EN                       0x01
#define RT9748_PD_EN_MASK                  0x01
#define RT9748_PD_EN_SHIFT                 0

#define RT9748_CONTROL                     0x06
#define RT9748_CONTROL_INIT                0x0e
#define RT9748_CHARGE_EN_MASK              (BIT(4))
#define RT9748_CHARGE_EN_SHIFT             4
#define RT9748_WATCH_DOG_CONFIG_MASK       (BIT(2) | BIT(3))
#define RT9748_WATCH_DOG_CONFIG_SHIFT      2
#define RT9748_WATCH_DOG_1500MS            1500
#define RT9748_WATCH_DOG_STEP              500

#define RT9748_ADC_CTRL                    0x07
#define RT9748_ADC_CTRL_INIT               0x87
#define RT9748_ADC_EN_MASK                 (BIT(3))
#define RT9748_ADC_EN_SHIFT                3

#define RT9748_SAMPLE_EN                   0x08
#define RT9748_SAMPLE_EN_INIT              0xfc

#define BQ25870_EVENT_1_MASK               0x01
#define BQ25870_EVENT_1_MASK_INIT          0x7e

#define BQ25870_EVENT_2_MASK               0x02
#define BQ25870_EVENT_2_MASK_INIT          0x7c

#define BQ25870_EVENT_1                    0x03
#define BQ25870_EVENT_2                    0x04

#define BQ25870_EVENT_1_EN                 0x05
#define BQ25870_EVENT_1_EN_INIT            0xf8

#define BQ25870_CONTROL                    0x06
#define BQ25870_CONTROL_INIT               0x0e
#define BQ25870_CHARGE_EN_MASK             (BIT(4))
#define BQ25870_CHARGE_EN_SHIFT            4

#define BQ25870_ADC_CTRL                   0x07
#define BQ25870_ADC_CTRL_INIT              0x87
#define BQ25870_ADC_EN_MASK                (BIT(3))
#define BQ25870_ADC_EN_SHIFT               3

#define BQ25870_SAMPLE_EN                  0x08
#define BQ25870_SAMPLE_EN_INIT             0xfc

#define RT9748_PROT_DLY_OCP                0x09
#define RT9748_PROT_DLY_OCP_INIT           0x00
#define RT9748_REG_INIT_MASK               (BIT(0) | BIT(1))
#define RT9748_REG_INIT_SHIFT              0
#define RT9748_IOC_OCP_MAX_6500_MA         6500
#define RT9748_IOC_OCP_MAX_7500_MA         7500
#define RT9748_IOC_OCP_MIN_0_MA            0
#define RT9748_IOC_OCP_OFFSET_0_MA         0
#define RT9748_IOC_OCP_STEP                500
#define RT9748_IOC_OCP_SHIFT               4
#define RT9748_IOC_OCP_MASK                (BIT(4) | BIT(5) | BIT(6) | BIT(7))

#define BQ25870_PROT_DLY_OCP               0x09
#define BQ25870_PROT_DLY_OCP_INIT          0x00
#define BQ25870_PROT_RES_OCP_INIT          0x01
#define BQ25870_REG_INIT_MASK              (BIT(0) | BIT(1))
#define BQ25870_RES_OCP_INIT_MASK          (BIT(1))
#define BQ25870_REG_INIT_SHIFT             0
#define BQ25870_RES_OCP_INIT_SHIFT         1
#define BQ25870_IOC_OCP_MAX_7500_MA        7500
#define BQ25870_IOC_OCP_MIN_0_MA           0
#define BQ25870_IOC_OCP_OFFSET_0_MA        0
#define BQ25870_IOC_OCP_STEP               500
#define BQ25870_IOC_OCP_SHIFT              4
#define BQ25870_IOC_OCP_MASK               (BIT(4) | BIT(5) | BIT(6) | BIT(7))

#define RT9748_VBUS_OVP                    0x0a
#define RT9748_VBUS_OVP_MAX_6500_MV        6500
#define RT9748_VBUS_OVP_MIN_4200_MV        4200
#define RT9748_VBUS_OVP_OFFSET_4200_MV     4200
#define RT9748_VBUS_OVP_STEP               25
#define RT9748_VBUS_OVP_SHIFT              0
#define RT9748_VBUS_OVP_MASK               (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define BQ25870_VBUS_OVP                   0x0a
#define BQ25870_VBUS_OVP_MAX_6510_MV       6510
#define BQ25870_VBUS_OVP_MIN_4200_MV       4200
#define BQ25870_VBUS_OVP_OFFSET_4200_MV    4200
#define BQ25870_VBUS_OVP_STEP              30
#define BQ25870_VBUS_OVP_SHIFT             0
#define BQ25870_VBUS_OVP_MASK              (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define RT9748_VOUT_REG                    0x0b
#define RT9748_VOUT_REG_MAX_5000_MV        5000
#define RT9748_VOUT_REG_MIN_4200_MV        4200
#define RT9748_VOUT_REG_OFFSET_4200_MV     4200
#define RT9748_VOUT_REG_STEP               10
#define RT9748_VOUT_REG_SHIFT              0
#define RT9748_VOUT_REG_MASK               (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define BQ25870_VOUT_REG                   0x0b
#define BQ25870_VOUT_REG_MAX_4975_MV       4975
#define BQ25870_VOUT_REG_MIN_4200_MV       4200
#define BQ25870_VOUT_REG_OFFSET_4200_MV    4200
#define BQ25870_VOUT_REG_STEP              25
#define BQ25870_VOUT_REG_SHIFT             1
#define BQ25870_VOUT_REG_MASK              (BIT(1) | BIT(2) | BIT(3) | \
	BIT(4) | BIT(5) | BIT(6))

#define RT9748_VDROP_OVP                   0x0c
#define RT9748_VDROP_OVP_MAX_1000_MV       1000
#define RT9748_VDROP_OVP_MIN_0_MV          0
#define RT9748_VDROP_OVP_OFFSET_0_MV       0
#define RT9748_VDROP_OVP_STEP              10
#define RT9748_VDROP_OVP_SHIFT             0
#define RT9748_VDROP_OVP_MASK              (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define BQ25870_VDROP_OVP                  0x0c
#define BQ25870_VDROP_OVP_MAX_1000_MV      1000
#define BQ25870_VDROP_OVP_MIN_0_MV         0
#define BQ25870_VDROP_OVP_OFFSET_0_MV      0
#define BQ25870_VDROP_OVP_STEP             10
#define BQ25870_VDROP_OVP_SHIFT            1
#define BQ25870_VDROP_OVP_MASK             (BIT(1) | BIT(2) | BIT(3) | \
	BIT(4) | BIT(5) | BIT(6) | BIT(7))

#define RT9748_VDROP_ALM                   0x0d
#define RT9748_VDROP_ALM_MAX_1000_MV       1000
#define RT9748_VDROP_ALM_MIN_0_MV          0
#define RT9748_VDROP_ALM_OFFSET_0_MV       0
#define RT9748_VDROP_ALM_STEP              10
#define RT9748_VDROP_ALM_SHIFT             0
#define RT9748_VDROP_ALM_MASK              (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))


#define BQ25870_VDROP_ALM                  0x0d
#define BQ25870_VDROP_ALM_MAX_1000_MV      1000
#define BQ25870_VDROP_ALM_MIN_0_MV         0
#define BQ25870_VDROP_ALM_OFFSET_0_MV      0
#define BQ25870_VDROP_ALM_STEP             10
#define BQ25870_VDROP_ALM_SHIFT            1
#define BQ25870_VDROP_ALM_MASK             (BIT(1) | BIT(2) | BIT(3) | \
	BIT(4) | BIT(5) | BIT(6) | BIT(7))

#define RT9748_VBAT_REG                    0x0e
#define RT9748_VBAT_REG_MAX_5000_MV        5000
#define RT9748_VBAT_REG_MIN_4200_MV        4200
#define RT9748_VBAT_REG_OFFSET_4200_MV     4200
#define RT9748_VBAT_REG_STEP               10
#define RT9748_VBAT_REG_SHIFT              0
#define RT9748_VBAT_REG_MASK               (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define BQ25870_VBAT_REG                   0x0e
#define BQ25870_VBAT_REG_MAX_4975_MV       4975
#define BQ25870_VBAT_REG_MIN_4200_MV       4200
#define BQ25870_VBAT_REG_OFFSET_4200_MV    4200
#define BQ25870_VBAT_REG_STEP              12.5
#define BQ25870_VBAT_REG_SHIFT             0
#define BQ25870_VBAT_REG_MASK              (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define RT9748_IBAT_OCP                    0x0f
#define RT9748_IBAT_OCP_MAX_6350_MA        6350
#define RT9748_IBAT_OCP_MIN_400_MA         400
#define RT9748_IBAT_OCP_OFFSET_0_MA        0
#define RT9748_IBAT_OCP_STEP               50
#define RT9748_IBAT_OCP_SHIFT              0
#define RT9748_IBAT_OCP_MASK               (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define BQ25870_IBAT_OCP                   0x0f
#define BQ25870_IBAT_OCP_MAX_6350_MA       6350
#define BQ25870_IBAT_OCP_MIN_0_MA          0
#define BQ25870_IBAT_OCP_OFFSET_0_MA       0
#define BQ25870_IBAT_OCP_STEP              50
#define BQ25870_IBAT_OCP_SHIFT             0
#define BQ25870_IBAT_OCP_MASK              (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define RT9748_IBUS_OCP                    0x10
#define RT9748_IBUS_OCP_MAX_6350_MA        6350
#define RT9748_IBUS_OCP_MIN_400_MA         400
#define RT9748_IBUS_OCP_OFFSET_0_MA        0
#define RT9748_IBUS_OCP_STEP               50
#define RT9748_IBUS_OCP_SHIFT              0
#define RT9748_IBUS_OCP_MASK               (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define BQ25870_IBUS_OCP                   0x10
#define BQ25870_IBUS_OCP_MAX_6300_MA       6300
#define BQ25870_IBUS_OCP_MIN_0_MA          0
#define BQ25870_IBUS_OCP_OFFSET_0_MA       0
#define BQ25870_IBUS_OCP_STEP              100
#define BQ25870_IBUS_OCP_SHIFT             1
#define BQ25870_IBUS_OCP_MASK              (BIT(1) | BIT(2) | BIT(3) | \
	BIT(4) | BIT(5) | BIT(6))

#define RT9748_TBUS_OTP                    0x11
#define RT9748_TBAT_OTP                    0x12

#define RT9748_VBUS_ADC2                   0x13
#define RT9748_VBUS_ADC1                   0x14
#define RT9748_VBUS_HIGH_LSB               256
#define RT9748_VBUS_LOW_LSB                1
#define RT9748_VBUS_ADC_MASK               (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4))
#define RT9748_VBUS_POLARITY_MASK          (BIT(7))

#define BQ25870_VBUS_ADC2                  0x13
#define BQ25870_VBUS_ADC1                  0x14
#define BQ25870_VBUS_HIGH_LSB              256
#define BQ25870_VBUS_LOW_LSB               1
#define BQ25870_VBUS_ADC_MASK              (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define BQ25870_VBUS_POLARITY_MASK         (BIT(7))

#define RT9748_IBUS_ADC2                   0x15
#define RT9748_IBUS_HIGH_LSB               256
#define RT9748_IBUS_LOW_LSB                1
#define RT9748_IBUS_ADC_MASK               (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4))
#define RT9748_IBUS_POLARITY_MASK          (BIT(7))
#define RT9748_IBUS_ADC1                   0x16

#define BQ25870_IBUS_ADC2                  0x15
#define BQ25870_IBUS_HIGH_LSB              256
#define BQ25870_IBUS_LOW_LSB               1
#define BQ25870_IBUS_ADC_MASK              (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define BQ25870_IBUS_POLARITY_MASK         (BIT(7))
#define BQ25870_IBUS_ADC1                  0x16

#define RT9748_VOUT_ADC2                   0x17
#define RT9748_VOUT_ADC1                   0x18
#define RT9748_VDROP_ADC2                  0x19
#define RT9748_VDROP_POLARITY_MASK         (BIT(7))
#define RT9748_VDROP_ADC_MASK              (BIT(0) | BIT(1))
#define RT9748_VDROP_HIGH_LSB              256
#define RT9748_VDROP_LOW_LSB               1
#define RT9748_VDROP_ADC1                  0x1a

#define BQ25870_VOUT_ADC2                  0x17
#define BQ25870_VOUT_ADC1                  0x18
#define BQ25870_VDROP_ADC2                 0x19
#define BQ25870_VDROP_POLARITY_MASK        (BIT(7))
#define BQ25870_VDROP_ADC_MASK             (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define BQ25870_VDROP_HIGH_LSB             256
#define BQ25870_VDROP_LOW_LSB              1
#define BQ25870_VDROP_ADC1                 0x1a

#define RT9748_VBAT_ADC2                   0x1b
#define RT9748_VBAT_ADC1                   0x1c
#define RT9748_VBAT_HIGH_LSB               256
#define RT9748_VBAT_LOW_LSB                1
#define RT9748_VBAT_ADC_MASK               (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4))
#define RT9748_VBAT_POLARITY_MASK          (BIT(7))

#define BQ25870_VBAT_ADC2                  0x1b
#define BQ25870_VBAT_ADC1                  0x1c
#define BQ25870_VBAT_HIGH_LSB              256
#define BQ25870_VBAT_LOW_LSB               1
#define BQ25870_VBAT_ADC_MASK              (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define BQ25870_VBAT_POLARITY_MASK         (BIT(7))

#define RT9748_IBAT_ADC2                   0x1d
#define RT9748_IBAT_ADC1                   0x1e
#define RT9748_IBAT_HIGH_LSB               256
#define RT9748_IBAT_LOW_LSB                1
#define RT9748_IBAT_ADC_MASK               (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4))
#define RT9748_IBAT_POLARITY_MASK          (BIT(7))

#define BQ25870_IBAT_ADC2                  0x1d
#define BQ25870_IBAT_ADC1                  0x1e
#define BQ25870_IBAT_HIGH_LSB              256
#define BQ25870_IBAT_LOW_LSB               1
#define BQ25870_IBAT_ADC_MASK              (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define BQ25870_IBAT_POLARITY_MASK         (BIT(7))

#define RT9748_TDIE_ADC1                   0x23
#define RT9748_EVENT_STATUS                0x24

#define RT9748_REV_CURRENT_SELECT          0x26
#define RT9748_REV_CURRENT_SELECT_INIT     0x06

/*FSC FAN54161*/
#define FAN54161_DEVICE_INFO               0x0
#define FAN54161_EVENT_1_MASK              0x1
#define FAN54161_EVENT_1_MASK_INIT         0x06

#define FAN54161_EVENT_2_MASK              0x02
#define FAN54161_EVENT_2_MASK_INIT         0xcc

#define FAN54161_EVENT_1                   0x03
#define FAN54161_VBUS_OVP_FLT              BIT(7)
#define FAN54161_IBUS_REVERSE_OCP_FLT      BIT(0)

#define FAN54161_EVENT_2                   0x04
#define FAN54161_VDROP_OVP_FLT             BIT(4)
#define FAN54161_OTP_FLT                   BIT(1)
#define FAN54161_INPUT_OCP_FLT             BIT(0)

#define FAN54161_EVENT_1_EN                0x05
#define FAN54161_EVENT_1_EN_INIT           0xf8
#define FAN54161_PD_EN                     0x01
#define FAN54161_PD_EN_MASK                0x01
#define FAN54161_PD_EN_SHIFT               0

#define FAN54161_CONTROL                   0x06
#define FAN54161_CONTROL_INIT              0x0e
#define FAN54161_CHARGE_EN_MASK            (BIT(4))
#define FAN54161_CHARGE_EN_SHIFT           4

#define FAN54161_ADC_CTRL                  0x07
#define FAN54161_ADC_CTRL_INIT             0x87
#define FAN54161_ADC_EN_MASK               (BIT(3))
#define FAN54161_ADC_EN_SHIFT              3

#define FAN54161_SAMPLE_EN                 0x08
#define FAN54161_SAMPLE_EN_INIT            0xfc

#define FAN54161_PROT_DLY_OCP              0x09
#define FAN54161_PROT_DLY_OCP_INIT         0x00
#define FAN54161_REG_INIT_MASK             (BIT(0))
#define FAN54161_REG_INIT_SHIFT            0
#define FAN54161_IOC_OCP_MAX_6500_MA       6500
#define FAN54161_IOC_OCP_MAX_7500_MA       7500
#define FAN54161_IOC_OCP_MIN_0_MA          0
#define FAN54161_IOC_OCP_OFFSET_0_MA       0
#define FAN54161_IOC_OCP_STEP              500
#define FAN54161_IOC_OCP_SHIFT             4
#define FAN54161_IOC_OCP_MASK              (BIT(4) | BIT(5) | BIT(6) | BIT(7))

#define FAN54161_VBUS_OVP                  0x0a
#define FAN54161_VBUS_OVP_MAX_6500_MV      6500
#define FAN54161_VBUS_OVP_MIN_4200_MV      4200
#define FAN54161_VBUS_OVP_OFFSET_4200_MV   4200
#define FAN54161_VBUS_OVP_STEP             25
#define FAN54161_VBUS_OVP_SHIFT            0
#define FAN54161_VBUS_OVP_MASK             (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define FAN54161_VOUT_REG                  0x0b
#define FAN54161_VOUT_REG_MAX_5000_MV      5000
#define FAN54161_VOUT_REG_MIN_4200_MV      4200
#define FAN54161_VOUT_REG_OFFSET_4200_MV   4200
#define FAN54161_VOUT_REG_STEP             10
#define FAN54161_VOUT_REG_SHIFT            0
#define FAN54161_VOUT_REG_MASK             (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define FAN54161_VDROP_OVP                 0x0c
#define FAN54161_VDROP_OVP_MAX_1000_MV     1000
#define FAN54161_VDROP_OVP_MIN_0_MV        0
#define FAN54161_VDROP_OVP_OFFSET_0_MV     0
#define FAN54161_VDROP_OVP_STEP            5
#define FAN54161_VDROP_OVP_SHIFT           0
#define FAN54161_VDROP_OVP_MASK            (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6) | BIT(7))

#define FAN54161_VDROP_ALM                 0x0d
#define FAN54161_VDROP_ALM_MAX_1000_MV     1000
#define FAN54161_VDROP_ALM_MIN_0_MV        0
#define FAN54161_VDROP_ALM_OFFSET_0_MV     0
#define FAN54161_VDROP_ALM_STEP            5
#define FAN54161_VDROP_ALM_SHIFT           0
#define FAN54161_VDROP_ALM_MASK            (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6) | BIT(7))

#define FAN54161_VBAT_REG                  0x0e
#define FAN54161_VBAT_REG_MAX_5000_MV      5000
#define FAN54161_VBAT_REG_MIN_4200_MV      4200
#define FAN54161_VBAT_REG_OFFSET_4200_MV   4200
#define FAN54161_VBAT_REG_STEP             10
#define FAN54161_VBAT_REG_SHIFT            0
#define FAN54161_VBAT_REG_MASK             (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define FAN54161_IBAT_OCP                  0x0f
#define FAN54161_IBAT_OCP_MAX_6350_MA      6350
#define FAN54161_IBAT_OCP_MIN_100_MA       100
#define FAN54161_IBAT_OCP_OFFSET_0_MA      0
#define FAN54161_IBAT_OCP_STEP             50
#define FAN54161_IBAT_OCP_SHIFT            0
#define FAN54161_IBAT_OCP_MASK             (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define FAN54161_IBUS_OCP                  0x10
#define FAN54161_IBUS_OCP_MAX_6500_MA      6500
#define FAN54161_IBUS_OCP_MIN_100_MA       100
#define FAN54161_IBUS_OCP_OFFSET_0_MA      0
#define FAN54161_IBUS_OCP_STEP             50
#define FAN54161_IBUS_OCP_SHIFT            0
#define FAN54161_IBUS_OCP_MASK             (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6) | BIT(7))

#define FAN54161_TBUS_OTP                  0x11
#define FAN54161_TBAT_OTP                  0x12

#define FAN54161_VBUS_ADC2                 0x13
#define FAN54161_VBUS_ADC1                 0x14
#define FAN54161_VBUS_HIGH_LSB             256
#define FAN54161_VBUS_LOW_LSB              1
#define FAN54161_VBUS_ADC_MASK             (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4))
#define FAN54161_VBUS_POLARITY_MASK        (BIT(7))

#define FAN54161_IBUS_ADC2                 0x15
#define FAN54161_IBUS_HIGH_LSB             256
#define FAN54161_IBUS_LOW_LSB              1
#define FAN54161_IBUS_ADC_MASK             (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4))
#define FAN54161_IBUS_POLARITY_MASK        (BIT(7))
#define FAN54161_IBUS_ADC1                 0x16

#define FAN54161_VOUT_ADC2                 0x17
#define FAN54161_VOUT_ADC1                 0x18
#define FAN54161_VDROP_ADC2                0x19
#define FAN54161_VDROP_POLARITY_MASK       (BIT(7))
#define FAN54161_VDROP_ADC_MASK            (BIT(0) | BIT(1))
#define FAN54161_VDROP_HIGH_LSB            256
#define FAN54161_VDROP_LOW_LSB             1
#define FAN54161_VDROP_ADC1                0x1a

#define FAN54161_VBAT_ADC2                 0x1b
#define FAN54161_VBAT_ADC1                 0x1c
#define FAN54161_VBAT_HIGH_LSB             256
#define FAN54161_VBAT_LOW_LSB              1
#define FAN54161_VBAT_ADC_MASK             (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4))
#define FAN54161_VBAT_POLARITY_MASK        (BIT(7))

#define FAN54161_IBAT_ADC2                 0x1d
#define FAN54161_IBAT_ADC1                 0x1e
#define FAN54161_IBAT_HIGH_LSB             256
#define FAN54161_IBAT_LOW_LSB              1
#define FAN54161_IBAT_ADC_MASK             (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4))
#define FAN54161_IBAT_POLARITY_MASK        (BIT(7))

#define FAN54161_TDIE_ADC1                 0x23

/* nxp */
#define PCA9498UK_DEVICE_INFO              0x0
#define PCA9498UK_EVENT_1_MASK             0x1
#define PCA9498UK_EVENT_1_MASK_INIT        0x06

#define PCA9498UK_EVENT_2_MASK             0x02
#define PCA9498UK_EVENT_2_MASK_INIT        0xcc

#define PCA9498UK_EVENT_1                  0x03
#define PCA9498UK_VBUS_OVP_FLT             BIT(7)
#define PCA9498UK_IBUS_REVERSE_OCP_FLT     BIT(0)

#define PCA9498UK_EVENT_2                  0x04
#define PCA9498UK_VDROP_OVP_FLT            BIT(4)
#define PCA9498UK_OTP_FLT                  BIT(1)
#define PCA9498UK_INPUT_OCP_FLT            BIT(0)

#define PCA9498UK_EVENT_1_EN               0x05
#define PCA9498UK_EVENT_1_EN_INIT          0xf8
#define PCA9498UK_PD_EN                    0x01
#define PCA9498UK_PD_EN_MASK               0x01
#define PCA9498UK_PD_EN_SHIFT              0

#define PCA9498UK_CONTROL                  0x06
#define PCA9498UK_CONTROL_INIT             0x0e
#define PCA9498UK_CHARGE_EN_MASK           (BIT(4))
#define PCA9498UK_CHARGE_EN_SHIFT          4

#define PCA9498UK_ADC_CTRL                 0x07
#define PCA9498UK_ADC_CTRL_INIT            0x87
#define PCA9498UK_ADC_EN_MASK              (BIT(3))
#define PCA9498UK_ADC_EN_SHIFT             3

#define PCA9498UK_SAMPLE_EN                0x08
#define PCA9498UK_SAMPLE_EN_INIT           0xfc

#define PCA9498UK_PROT_DLY_OCP             0x09
#define PCA9498UK_PROT_DLY_OCP_INIT        0x00
#define PCA9498UK_REG_INIT_MASK            (BIT(0) | BIT(1))
#define PCA9498UK_REG_INIT_SHIFT           0
#define PCA9498UK_IOC_OCP_MAX_6500_MA      6500
#define PCA9498UK_IOC_OCP_MAX_7500_MA      7500
#define PCA9498UK_IOC_OCP_MIN_0_MA         0
#define PCA9498UK_IOC_OCP_OFFSET_0_MA      0
#define PCA9498UK_IOC_OCP_STEP             500
#define PCA9498UK_IOC_OCP_SHIFT            4
#define PCA9498UK_IOC_OCP_MASK             (BIT(4) | BIT(5) | BIT(6) | BIT(7))

#define PCA9498UK_VBUS_OVP                 0x0a
#define PCA9498UK_VBUS_OVP_MAX_6500_MV     6500
#define PCA9498UK_VBUS_OVP_MIN_4200_MV     4200
#define PCA9498UK_VBUS_OVP_OFFSET_4200_MV  4200
#define PCA9498UK_VBUS_OVP_STEP            25
#define PCA9498UK_VBUS_OVP_SHIFT           0
#define PCA9498UK_VBUS_OVP_MASK            (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define PCA9498UK_VOUT_REG                 0x0b
#define PCA9498UK_VOUT_REG_MAX_5000_MV     5000
#define PCA9498UK_VOUT_REG_MIN_4200_MV     4200
#define PCA9498UK_VOUT_REG_OFFSET_4200_MV  4200
#define PCA9498UK_VOUT_REG_STEP            10
#define PCA9498UK_VOUT_REG_SHIFT           0
#define PCA9498UK_VOUT_REG_MASK            (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define PCA9498UK_VDROP_OVP                0x0c
#define PCA9498UK_VDROP_OVP_MAX_1000_MV    1000
#define PCA9498UK_VDROP_OVP_MIN_0_MV       0
#define PCA9498UK_VDROP_OVP_OFFSET_0_MV    0
#define PCA9498UK_VDROP_OVP_STEP           5
#define PCA9498UK_VDROP_OVP_SHIFT          0
#define PCA9498UK_VDROP_OVP_MASK           (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6)  | BIT(7))

#define PCA9498UK_VDROP_ALM                0x0d
#define PCA9498UK_VDROP_ALM_MAX_1000_MV    1000
#define PCA9498UK_VDROP_ALM_MIN_0_MV       0
#define PCA9498UK_VDROP_ALM_OFFSET_0_MV    0
#define PCA9498UK_VDROP_ALM_STEP           5
#define PCA9498UK_VDROP_ALM_SHIFT          0
#define PCA9498UK_VDROP_ALM_MASK           (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6)  | BIT(7))

#define PCA9498UK_VBAT_REG                 0x0e
#define PCA9498UK_VBAT_REG_MAX_5000_MV     5000
#define PCA9498UK_VBAT_REG_MIN_4200_MV     4200
#define PCA9498UK_VBAT_REG_OFFSET_4200_MV  4200
#define PCA9498UK_VBAT_REG_STEP            10
#define PCA9498UK_VBAT_REG_SHIFT           0
#define PCA9498UK_VBAT_REG_MASK            (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))

#define PCA9498UK_IBAT_OCP                 0x0f
#define PCA9498UK_IBAT_OCP_MAX_6350_MA     6350
#define PCA9498UK_IBAT_OCP_MIN_400_MA      400
#define PCA9498UK_IBAT_OCP_OFFSET_0_MA     0
#define PCA9498UK_IBAT_OCP_STEP            50
#define PCA9498UK_IBAT_OCP_SHIFT           0
#define PCA9498UK_IBAT_OCP_MASK            (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6) | BIT(7))

#define PCA9498UK_IBUS_OCP                 0x10
#define PCA9498UK_IBUS_OCP_MAX_6500_MA     6500
#define PCA9498UK_IBUS_OCP_MIN_400_MA      400
#define PCA9498UK_IBUS_OCP_OFFSET_0_MA     0
#define PCA9498UK_IBUS_OCP_STEP            50
#define PCA9498UK_IBUS_OCP_SHIFT           0
#define PCA9498UK_IBUS_OCP_MASK            (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6) | BIT(7))

#define PCA9498UK_TBUS_OTP                 0x11
#define PCA9498UK_TBAT_OTP                 0x12

#define PCA9498UK_VBUS_ADC2                0x13
#define PCA9498UK_VBUS_ADC1                0x14
#define PCA9498UK_VBUS_HIGH_LSB            256
#define PCA9498UK_VBUS_LOW_LSB             1
#define PCA9498UK_VBUS_ADC_MASK            (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4))
#define PCA9498UK_VBUS_POLARITY_MASK       (BIT(7))

#define PCA9498UK_IBUS_ADC2                0x15
#define PCA9498UK_IBUS_HIGH_LSB            256
#define PCA9498UK_IBUS_LOW_LSB             1
#define PCA9498UK_IBUS_ADC_MASK            (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4))
#define PCA9498UK_IBUS_POLARITY_MASK       (BIT(7))
#define PCA9498UK_IBUS_ADC1                0x16

#define PCA9498UK_VOUT_ADC2                0x17
#define PCA9498UK_VOUT_ADC1                0x18
#define PCA9498UK_VDROP_ADC2               0x19
#define PCA9498UK_VDROP_POLARITY_MASK      (BIT(7))
#define PCA9498UK_VDROP_ADC_MASK           (BIT(0) | BIT(1))
#define PCA9498UK_VDROP_HIGH_LSB           256
#define PCA9498UK_VDROP_LOW_LSB            1
#define PCA9498UK_VDROP_ADC1               0x1a

#define PCA9498UK_VBAT_ADC2                0x1b
#define PCA9498UK_VBAT_ADC1                0x1c
#define PCA9498UK_VBAT_HIGH_LSB            256
#define PCA9498UK_VBAT_LOW_LSB             1
#define PCA9498UK_VBAT_ADC_MASK            (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4))
#define PCA9498UK_VBAT_POLARITY_MASK       (BIT(7))

#define PCA9498UK_IBAT_ADC2                0x1d
#define PCA9498UK_IBAT_ADC1                0x1e
#define PCA9498UK_IBAT_HIGH_LSB            256
#define PCA9498UK_IBAT_LOW_LSB             1
#define PCA9498UK_IBAT_ADC_MASK            (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4))
#define PCA9498UK_IBAT_POLARITY_MASK       (BIT(7))

#define PCA9498UK_TDIE_ADC1                0x23
#define PCA9498UK_EVENT_STATUS             0x24

#define LS_GET_DEV_ID_SHIFT                3

void ls_i2c_mutex_lock(void);
void ls_i2c_mutex_unlock(void);

#endif /* _RT9748_H_ */
