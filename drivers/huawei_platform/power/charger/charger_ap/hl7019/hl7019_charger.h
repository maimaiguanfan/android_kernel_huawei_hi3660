/*
 * hl7019_charger.h
 *
 * hl7019 driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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

#ifndef _HL7019_CHARGER_H_
#define _HL7019_CHARGER_H_

#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/workqueue.h>

#ifndef BIT
#define BIT(x)    (1 << (x))
#endif

/*************************struct define area***************************/
struct hl7019_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	int gpio_cd;
	int gpio_int;
	int irq_int;
	int irq_active;
	int cust_cv;
	int hiz_iin_limit;
};

/*************************marco define area***************************/

#define HL7019_REG_NUM                              14

#define HL7019_REG_NONE                             0x00
#define HL7019_REG_NONE_MASK                        0xFF
#define HL7019_REG_NONE_SHIFT                       0x00

/* Register 00h */
#define HL7019_REG_CHG_CTRL0                        0x00

#define HL7019_REG_EN_HIZ_MASK                      0x80
#define HL7019_REG_EN_HIZ_SHIFT                     7
#define REG00_HIZ_ENABLE                            1
#define REG00_HIZ_DISABLE                           0

#define HL7019_REG_VINDPM_MASK                      0x78
#define HL7019_REG_VINDPM_SHIFT                     3
#define VINDPM_BASE                                 3880
#define VINDPM_DEFAULT                              4610
#define VINDPM_LSB                                  90
#define VDPM_BASE                                   4

#define HL7019_REG_IINLIM_MASK                      0x07
#define HL7019_REG_IINLIM_SHIFT                     0
#define REG00_IINLIM_0P1A                           0
#define REG00_IINLIM_0P15A                          1
#define REG00_IINLIM_0P5A                           2
#define REG00_IINLIM_0P9A                           3
#define REG00_IINLIM_1A                             4
#define REG00_IINLIM_1P5A                           5
#define REG00_IINLIM_2A                             6
#define REG00_IINLIM_3A                             7
#define IINLIM_0P1A                                 100

/* Register 01h */
#define HL7019_REG_CHG_CTRL1                        0x01

#define HL7019_REG_WDT_RESET_MASK                   0x40
#define HL7019_REG_WDT_RESET_SHIFT                  6
#define REG01_WDT_RESET                             1

#define HL7019_REG_CHG_OTG_CFG_MASK                 0x30
#define HL7019_REG_CHG_OTG_CFG_SHIFT                4
#define REG01_CHG_OTG_DISABLE                       0
#define REG01_CHG_ENABLE                            1
#define REG01_OTG_ENABLE                            2

#define HL7019_REG_BOOST_ILIM_MASK                  0x01
#define HL7019_REG_BOOST_ILIM_SHIFT                 0
#define REG01_BOOST_ILIM_1A                         0
#define REG01_BOOST_ILIM_2P1A                       1
#define BOOST_ILIM_1P5A                             1500

/* Register 02h */
#define HL7019_REG_CTRL2                            0x02

#define HL7019_REG_ICHG_MASK                        0xFC
#define HL7019_REG_ICHG_SHIFT                       2
#define ICHG_BASE                                   512
#define ICHG_LSB                                    64
#define ICHG_MAX                                    3008
#define ENABLE_ICHG_20PCT                           1
#define DISABLE_ICHG_20PCT                          0
#define ICHG_REFIT_FAC                              2
#define ICHG_LOW_MULTI_FAC                          5

/* Register 03h */
#define HL7019_REG_CTRL3                            0x03

#define HL7019_REG_IPRECHG_MASK                     0xF0
#define HL7019_REG_IPRECHG_SHIFT                    4
#define IPRECHG_BASE                                128
#define IPRECHG_LSB                                 128

#define HL7019_REG_ITERM_MASK                       0x0F
#define HL7019_REG_ITERM_SHIFT                      0
#define ITERM_BASE                                  128
#define ITERM_LSB                                   128

/* Register 04h */
#define HL7019_REG_CTRL4                            0x04

#define HL7019_REG_VREG_MASK                        0xFC
#define HL7019_REG_VREG_SHIFT                       2
#define VREG_BASE                                   3504
#define VREG_LSB                                    16

#define HL7019_REG_VRECHG_MASK                      0x01
#define HL7019_REG_TVRECHG_SHIFT                    0
#define REG04_VRECHG_100MV                          0
#define REG04_VRECHG_300MV                          1

/* Register 05h */
#define HL7019_REG_CTRL5                            0x05

#define HL7019_REG_EN_TERM_MASK                     0x80
#define HL7019_REG_EN_TERM_SHIFT                    7
#define REG05_TERM_ENABLE                           1
#define REG05_TERM_DISABLE                          0

#define HL7019_REG_WDT_MASK                         0x30
#define HL7019_REG_WDT_SHIFT                        4
#define REG05_WDT_DISABLE                           0
#define REG05_WDT_40S                               1
#define REG05_WDT_80S                               2
#define REG05_WDT_160S                              3
#define WDT_BASE                                    0
#define WDT_40S                                     40
#define WDT_80S                                     80

#define HL7019_REG_EN_TIMER_MASK                    0x08
#define HL7019_REG_EN_TIMER_SHIFT                   3
#define REG05_CHG_TIMER_ENABLE                      1
#define REG05_CHG_TIMER_DISABLE                     0

#define HL7019_REG_CHG_TIMER_MASK                   0x06
#define HL7019_REG_CHG_TIMER_SHIFT                  1
#define REG05_CHG_TIMER_5HOURS                      0
#define REG05_CHG_TIMER_8HOURS                      1
#define REG05_CHG_TIMER_12HOURS                     2
#define REG05_CHG_TIMER_20HOURS                     3

/* Register 06h */
#define HL7019_REG_CTRL6                            0x06

#define HL7019_REG_BOOSTV_MASK                      0xF0
#define HL7019_REG_BOOSTV_SHIFT                     4
#define BOOSTV_BASE                                 4550
#define BOOSTV_LSB                                  64
#define BOOSTV_4550                                 4550
#define BOOSTV_5510                                 5510
#define BOOSTV_5000                                 5000

/* Register 07h */
#define HL7019_REG_CTRL7                            0x07

#define HL7019_REG_BATFET_DISABLE_MASK              0x20
#define HL7019_REG_BATFET_DISABLE_SHIFT             5
#define REG07_BATFET_OFF                            1
#define REG07_BATFET_ON                             0

/* Register 08h */
#define HL7019_REG_CTRL8                            0x08

#define HL7019_REG_VBUS_STAT_MASK                   0xC0
#define HL7019_REG_VBUS_STAT_SHIFT                  6
#define REG08_VBUS_TYPE_NONE                        0
#define REG08_VBUS_TYPE_USB                         1
#define REG08_VBUS_TYPE_ADAPTER                     2
#define REG08_VBUS_TYPE_OTG                         3

#define HL7019_REG_CHRG_STAT_MASK                   0x30
#define HL7019_REG_CHRG_STAT_SHIFT                  4
#define REG08_CHRG_STAT_IDLE                        0
#define REG08_CHRG_STAT_PRECHG                      1
#define REG08_CHRG_STAT_FASTCHG                     2
#define REG08_CHRG_STAT_CHGDONE                     3

#define HL7019_REG_DPM_STAT_MASK                    0x08
#define HL7019_REG_DPM_STAT_SHIFT                   3
#define REG0A_DPM_ACTIVE                            1

#define HL7019_REG_PG_STAT_MASK                     0x04
#define HL7019_REG_PG_STAT_SHIFT                    2
#define REG08_POWER_GOOD                            1

/* Register 09h */
#define HL7019_REG_CTRL9                            0x09

#define HL7019_REG_FAULT_WDT_MASK                   0x80
#define HL7019_REG_FAULT_WDT_SHIFT                  7
#define REG09_FAULT_WDT                             1

#define HL7019_REG_FAULT_BOOST_MASK                 0x40
#define HL7019_REG_FAULT_BOOST_SHIFT                6

#define HL7019_REG_FAULT_CHRG_MASK                  0x30
#define HL7019_REG_FAULT_CHRG_SHIFT                 4
#define REG09_FAULT_CHRG_NORMAL                     0
#define REG09_FAULT_CHRG_INPUT                      1
#define REG09_FAULT_CHRG_THERMAL                    2
#define REG09_FAULT_CHRG_TIMER                      3
#define FAULT_CHRG_INPUT                            0x10

#define HL7019_REG_FAULT_BAT_MASK                   0x08
#define HL7019_REG_FAULT_BAT_SHIFT                  3
#define REG09_FAULT_BAT_OVP                         1

/* Register 0Ah */
#define HL7019_REG_CTRL0A                           0x0A

#define HL7019_VENDOR_ID                            0x20

/* Register 0Bh */
#define HL7019_REG_CTRL0B                           0x0B

/* Register 0Ch */
#define HL7019_REG_CTRL0C                           0x0C

/* Register 0Dh */
#define HL7019_REG_CTRL0D                           0x0D

#define HL7019_REG_VINDPM_OFFSET_MASK               0x20
#define HL7019_REG_VINDPM_OFFSET_SHIFT              5
#define REG0D_5V_VDPM                               0
#define REG0D_10V_VDPM                              1

#define HIZ_IIN_FLAG_TRUE                           1
#define HIZ_IIN_FLAG_FALSE                          0

#define MSG_LEN                                     2
#define BUF_LEN                                     26

#define CUST_MIN_CV                                 4350
#define HIZ_IIN_FLAG_TRUE                           1
#define HIZ_IIN_FLAG_FALSE                          0
#define PWR_CHECK_CNT                               40
#define CHIP_INIT_INTERVAL                          1000
#define PWR_CHECK_INTERVAL                          50

#endif /* _HL7019_CHARGER_H_ */
