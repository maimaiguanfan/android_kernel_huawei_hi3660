/*
 * da9313.h
 *
 * vsys sc da9313 driver
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

#ifndef _DA9313_H_
#define _DA9313_H_

#ifndef BIT
#define BIT(x)    (1 << (x))
#endif

#define I2C_RETRY_CNT                      (3)
#define DA9313_ADDR_LEN                    (1)
#define BYTE_LEN                           (1)

#define DA9313_CHIP_ENABLE                 (1)
#define DA9313_CHIP_DISABLE                (0)

#define DA9313_PG_NORMAL                   (1)
#define DA9313_PG_ABNOR                    (0)

#define DA9313_NOT_USED                    (0)
#define DA9313_USED                        (1)
#define DA9313_DEVICE_ID_GET_FAIL          (-1)

#define DA9313_RD_BUF_SIZE                 (64)

/* VENDORID reg = 0x01 */
#define DA9313_REG_VENDORID                0x01

/* MODE_CTRL reg = 0x02 */
#define DA9313_REG_MODECTRL                0x02

#define DA9313_REG_MODECTRL_INIT           (0x70)

#define DA9313_REG_AUTOBOOT_MASK           (BIT(6))
#define DA9313_REG_AUTOBOOT_SHIFT          (6)
#define DA9313_REG_RESET_CYCLE_MASK        (BIT(5))
#define DA9313_REG_RESET_CYCLE_SHIFT       (5)
#define DA9313_REG_NONKEY_CYCLE_MASK       (BIT(4))
#define DA9313_REG_NONKEY_CYCLE_SHIFT      (4)
#define DA9313_REG_MSTSLV_IF_EN_MASK       (BIT(3))
#define DA9313_REG_MSTSLV_IF_EN_SHIFT      (3)
#define DA9313_REG_NONKEY_MODE_MASK        (BIT(2))
#define DA9313_REG_NONKEY_MODE_SHIFT       (2)
#define DA9313_REG_SOFTRESET_MASK          (BIT(1))
#define DA9313_REG_SOFTRESET_SHIFT         (1)
#define DA9313_REG_SLEEP_MASK              (BIT(0))
#define DA9313_REG_SLEEP_SHIFT             (0)

#define DA9313_RESET_CYCLE_PWRDOWN         (0)
#define DA9313_RESET_CYCLE_ACTIVE          (1)
#define DA9313_SOFTRESET_DEASSERTED        (0)
#define DA9313_SOFTRESET_ASSERTED          (1)

/* SLEEP_CTRL reg = 0x03 */
#define DA9313_REG_SLEEPCTRL               0x03

#define DA9313_REG_NONKEY_CONT_MASK        (BIT(3))
#define DA9313_REG_NONKEY_CONT_SHIFT       (3)
#define DA9313_REG_PVC_DIS_MASK            (BIT(2))
#define DA9313_REG_PVC_DIS_SHIFT           (2)
#define DA9313_REG_PVC_MASK                (BIT(1))
#define DA9313_REG_PVC_SHIFT               (1)
#define DA9313_REG_VDDIO_DIS_MASK          (BIT(0))
#define DA9313_REG_VDDIO_DIS_SHIFT         (0)

/* PVC_CTRL reg = 0x04 */
#define DA9313_REG_PVCCTRL                 0x04

#define DA9313_REG_PVC_MODE_MASK           (BIT(1))
#define DA9313_REG_PVC_MODE_SHIFT          (1)
#define DA9313_REG_PVC_EN_MASK             (BIT(0))
#define DA9313_REG_PVC_EN_SHIFT            (0)

/* GPIO_CTRL reg = 0x05 */
#define DA9313_REG_GPIOCTRL                0x05

#define DA9313_REG_GPIOCTRL_INIT           (0x51)

#define DA9313_REG_GPIO1_PIN_MASK          (BIT(4) | BIT(5) | BIT(6))
#define DA9313_REG_GPIO1_PIN_SHIFT         (4)
#define DA9313_REG_GPIO0_PIN_MASK          (BIT(0) | BIT(1) | BIT(2))
#define DA9313_REG_GPIO0_PIN_SHIFT         (0)

#define DA9313_GPIO1_PIN_GPI               (0x0)
#define DA9313_GPIO1_PIN_PVC_EN            (0x1)
#define DA9313_GPIO1_PIN_SLEEP_EN          (0x2)
#define DA9313_GPIO1_PIN_PWC               (0x3)
#define DA9313_GPIO1_PIN_GPO               (0x4)
#define DA9313_GPIO1_PIN_PG                (0x5)
#define DA9313_GPIO1_PIN_IRQ               (0x6)
#define DA9313_GPIO1_PIN_DCI               (0x7)

#define DA9313_GPIO0_PIN_GPI               (0x0)
#define DA9313_GPIO0_PIN_PVC_EN            (0x1)
#define DA9313_GPIO0_PIN_SLEEP_EN          (0x2)
#define DA9313_GPIO0_PIN_PWC               (0x3)
#define DA9313_GPIO0_PIN_GPO               (0x4)
#define DA9313_GPIO0_PIN_PG                (0x5)
#define DA9313_GPIO0_PIN_IRQ               (0x6)

/* GPIO_CTRL_2 reg = 0x06 */
#define DA9313_REG_GPIOCTRL2               0x06

#define DA9313_REG_GPIOCTRL2_INIT          0x8B

#define DA9313_REG_GPIO1_PD_MASK           (BIT(7))
#define DA9313_REG_GPIO1_PD_SHIFT          (7)
#define DA9313_REG_GPIO1_PU_MASK           (BIT(6))
#define DA9313_REG_GPIO1_PU_SHIFT          (6)
#define DA9313_REG_GPIO1_MODE_MASK         (BIT(5))
#define DA9313_REG_GPIO1_MODE_SHIFT        (5)
#define DA9313_REG_GPIO1_TYPE_MASK         (BIT(4))
#define DA9313_REG_GPIO1_TYPE_SHIFT        (4)
#define DA9313_REG_GPIO0_PD_MASK           (BIT(3))
#define DA9313_REG_GPIO0_PD_SHIFT          (3)
#define DA9313_REG_GPIO0_PU_MASK           (BIT(2))
#define DA9313_REG_GPIO0_PU_SHIFT          (2)
#define DA9313_REG_GPIO0_MODE_MASK         (BIT(1))
#define DA9313_REG_GPIO0_MODE_SHIFT        (1)
#define DA9313_REG_GPIO0_TYPE_MASK         (BIT(0))
#define DA9313_REG_GPIO0_TYPE_SHIFT        (0)

/* INTERFACE_CTRL reg = 0x07 */
#define DA9313_REG_INTERFACECTRL           0x07

#define DA9313_REG_IF_WRITE_MODE_MASK      (BIT(7))
#define DA9313_REG_IF_WRITE_MODE_SHIFT     (7)
#define DA9313_REG_IF_PM_IF_HSM_MASK       (BIT(6))
#define DA9313_REG_IF_PM_IF_HSM_SHIFT      (6)
#define DA9313_REG_IF_PC_DONE_MASK         (BIT(5))
#define DA9313_REG_IF_PC_DONE_SHIFT        (5)
#define DA9313_REG_IF_I2C_TO_EN_MASK       (BIT(4))
#define DA9313_REG_IF_I2C_TO_EN_SHIFT      (4)
#define DA9313_REG_IF_GPIO0_DEGLITCH_MASK  (BIT(3))
#define DA9313_REG_IF_GPIO0_DEGLITCH_SHIFT (3)
#define DA9313_REG_IF_GPI_DEBOUNCE_MASK    (BIT(0) | BIT(1) | BIT(2))
#define DA9313_REG_IF_GPI_DEBOUNCE_SHIFT   (0)

/* NONKEY_CTRL reg = 0x08 */
#define DA9313_REG_NONKEYCTRL              0x08

#define DA9313_REG_NONKEY_SHUT_DELAY_MASK  (BIT(6) | BIT(7))
#define DA9313_REG_NONKEY_SHUT_DELAY_SHIFT (6)
#define DA9313_REG_NONKEY_KEY_DELAY_MASK   (BIT(4) | BIT(5))
#define DA9313_REG_NONKEY_KEY_DELAY_SHIFT  (4)
#define DA9313_REG_NONKEY_PIN_MASK         (BIT(3))
#define DA9313_REG_NONKEY_PIN_SHIFT        (3)
#define DA9313_REG_NONKEY_DEBOUNCE_MASK    (BIT(0) | BIT(1) | BIT(2))
#define DA9313_REG_NONKEY_DEBOUNCE_SHIFT   (0)

/* MASK reg = 0x09 */
#define DA9313_REG_MASK                    0x09

#define DA9313_REG_MASK_TEMP_WARN_MASK     (BIT(6))
#define DA9313_REG_MASK_TEMP_WARN_SHIFT    (6)
#define DA9313_REG_MASK_VBAT_WARN_MASK     (BIT(5))
#define DA9313_REG_MASK_VBAT_WARN_SHIFT    (5)
#define DA9313_REG_MASK_NONKEY_MASK        (BIT(4))
#define DA9313_REG_MASK_NONKEY_SHIFT       (4)
#define DA9313_REG_MASK_PVC_OC_WARN_MASK   (BIT(3))
#define DA9313_REG_MASK_PVC_OC_WARN_SHIFT  (3)
#define DA9313_REG_MASK_PVC_PG_MASK        (BIT(2))
#define DA9313_REG_MASK_PVC_PG_SHIFT       (2)
#define DA9313_REG_MASK_GPI1_MASK          (BIT(1))
#define DA9313_REG_MASK_GPI1_SHIFT         (1)
#define DA9313_REG_MASK_GPI0_MASK          (BIT(0))
#define DA9313_REG_MASK_GPI0_SHIFT         (0)

/* STATUS reg = 0x0A */
#define DA9313_REG_STATUS                  0x0A

#define DA9313_REG_STS_PVC_OC_WARN_MASK    (BIT(7))
#define DA9313_REG_STS_PVC_OC_WARN_SHIFT   (7)
#define DA9313_REG_STS_TEMP_WARN_MASK      (BIT(6))
#define DA9313_REG_STS_TEMP_WARN_SHIFT     (6)
#define DA9313_REG_STS_VBAT_WARN_MASK      (BIT(5))
#define DA9313_REG_STS_VBAT_WARN_SHIFT     (5)
#define DA9313_REG_STS_NONKEY_MASK         (BIT(3))
#define DA9313_REG_STS_NONKEY_SHIFT        (3)
#define DA9313_REG_STS_PVC_PG_MASK         (BIT(2))
#define DA9313_REG_STS_PVC_PG_SHIFT        (2)
#define DA9313_REG_STS_GPI1_MASK           (BIT(1))
#define DA9313_REG_STS_GPI1_SHIFT          (1)
#define DA9313_REG_STS_GPI0_MASK           (BIT(0))
#define DA9313_REG_STS_GPI0_SHIFT          (0)

/* EVENT reg = 0x0B */
#define DA9313_REG_EVENT                   0x0B

#define DA9313_REG_EVT_KEY_RESET_MASK      (BIT(7))
#define DA9313_REG_EVT_KEY_RESET_SHIFT     (7)
#define DA9313_REG_EVT_TEMP_WARN_MASK      (BIT(6))
#define DA9313_REG_EVT_TEMP_WARN_SHIFT     (6)
#define DA9313_REG_EVT_VBAT_WARN_MASK      (BIT(5))
#define DA9313_REG_EVT_VBAT_WARN_SHIFT     (5)
#define DA9313_REG_EVT_NONKEY_MASK         (BIT(4))
#define DA9313_REG_EVT_NONKEY_SHIFT        (4)
#define DA9313_REG_EVT_PVC_OC_WARN_MASK    (BIT(3))
#define DA9313_REG_EVT_PVC_OC_WARN_SHIFT   (3)
#define DA9313_REG_EVT_PVC_PG_MASK         (BIT(2))
#define DA9313_REG_EVT_PVC_PG_SHIFT        (2)
#define DA9313_REG_EVT_GPI1_MASK           (BIT(1))
#define DA9313_REG_EVT_GPI1_SHIFT          (1)
#define DA9313_REG_EVT_GPI0_MASK           (BIT(0))
#define DA9313_REG_EVT_GPI0_SHIFT          (0)

/* FAULT_LOG reg = 0x0C */
#define DA9313_REG_FAULTLOG                0x0C

#define DA9313_REG_FLT_PVC_RAMPUP_MASK     (BIT(7))
#define DA9313_REG_FLT_PVC_RAMPUP_SHIFT    (7)
#define DA9313_REG_FLT_PD_RESET_MASK       (BIT(6))
#define DA9313_REG_FLT_PD_RESET_SHIFT      (6)
#define DA9313_REG_FLT_GPI_RESET_MASK      (BIT(5))
#define DA9313_REG_FLT_GPI_RESET_SHIFT     (5)
#define DA9313_REG_FLT_NONKEY_RST_MASK     (BIT(4))
#define DA9313_REG_FLT_NONKEY_RST_SHITF    (4)
#define DA9313_REG_FLT_VBAT_UVCRIT_MASK    (BIT(3))
#define DA9313_REG_FLT_VBAT_UVCRIT_SHITF   (3)
#define DA9313_REG_FLT_PVC_OCCRIT_MASK     (BIT(2))
#define DA9313_REG_FLT_PVC_OCCRIT_SHIFT    (2)
#define DA9313_REG_FLT_TEMPCRIT_MASK       (BIT(1))
#define DA9313_REG_FLT_TEMPCRIT_SHIFT      (1)
#define DA9313_REG_FLT_POR_MASK            (BIT(0))
#define DA9313_REG_FLT_POR_SHIFT           (0)

/* LOCK_REG reg = 0x0D */
#define DA9313_REG_LOCK                    0x0D

/* PVC_CONFIG_1 reg = 0x0E */
#define DA9313_REG_PVCCFG1                 0x0E

#define DA9313_REG_PVCCFG1_INIT            (0xF6)

#define DA9313_REG_DROP_MASK               (BIT(6) | BIT(7))
#define DA9313_REG_DROP_SHIFT              (6)
#define DA9313_REG_HYST_MASK               (BIT(4) | BIT(5))
#define DA9313_REG_HYST_SHIFT              (4)
#define DA9313_REG_MS_DROP_MASK            (BIT(2) | BIT(3))
#define DA9313_REG_MS_DROP_SHIFT           (2)
#define DA9313_REG_MS_HYST_MASK            (BIT(0) | BIT(1))
#define DA9313_REG_MS_HYST_SHIFT           (0)

/* PVC_CONFIG_2 reg = 0x0F */
#define DA9313_REG_PVCCONFIG2              0x0F

#define DA9313_REG_SLVIF_PHASE_MASK        (BIT(6) | BIT(7))
#define DA9313_REG_SLVIF_PHASE_SHIFT       (6)
#define DA9313_REG_ILIM_MASK               (BIT(0) | BIT(1) | BIT(2) | BIT(3))
#define DA9313_REG_ILIM_SHIFT              (0)

/* PVC_CONFIG_3 reg = 0x10 */
#define DA9313_REG_PVCCONFIG3              0x10

#define DA9313_REG_PVCCONFIG3_INIT         (0x14)

#define DA9313_REG_PG_DIS_MASK             (BIT(7))
#define DA9313_REG_PG_DIS_SHIFT            (7)
#define DA9313_REG_PG_ADJ_MASK             (BIT(2) | BIT(3) | BIT(4) | \
	BIT(5) | BIT(6))
#define DA9313_REG_PG_ADJ_SHIFT            (2)
#define DA9313_REG_ISTUP_MASK              (BIT(0) | BIT(1))
#define DA9313_REG_ISTUP_SHIFT             (0)

/* CONFIG_A reg = 0x11 */
#define DA9313_REG_CONFIGA                 0x11

#define DA9313_REG_CONFIGA_INIT            (0x68)

#define DA9313_REG_UV_HYS_MASK             (BIT(5) | BIT(6) | BIT(7))
#define DA9313_REG_UV_HYS_SHIFT            (5)
#define DA9313_REG_UV_TH_MASK              (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4))
#define DA9313_REG_UV_TH_SHIFT             (0)

/* CONFIG_C reg = 0x13 */
#define DA9313_REG_CONFIGC                 0x13

#define DA9313_REG_CONFIGC_INIT            (0xC9)

#define DA9313_REG_VDDIO_EN_MASK           (BIT(7))
#define DA9313_REG_VDDIO_EN_SHIFT          (7)
#define DA9313_REG_FAULT_TIME_MASK         (BIT(5) | BIT(6))
#define DA9313_REG_FAULT_TIME_SHIFT        (5)
#define DA9313_REG_VDDIO_CONF_MASK         (BIT(3) | BIT(4))
#define DA9313_REG_VDDIO_CONF_SHIFT        (3)
#define DA9313_REG_VBAT_UV_WARN_MASK       (BIT(0) | BIT(1) | BIT(2))
#define DA9313_REG_VBAT_UV_WARN_SHIFT      (0)

/* CONFIG_D reg = 0x14 */
#define DA9313_REG_CONFIGD                 0x14

#define DA9313_REG_IF_BASE_ADDR1_MASK      (BIT(0) | BIT(1) | BIT(2) | BIT(3))
#define DA9313_REG_IF_BASE_ADDR1_SHIFT     (0)

/* VARIANT_ID reg = 0x30 */
#define DA9313_REG_VARIANTID               0x30

#define DA9313_REG_VARID_MRC_MASK          (BIT(4) | BIT(5) | BIT(6) | BIT(7))
#define DA9313_REG_VARID_MRC_SHIFT         (4)
#define DA9313_REG_VARID_VRC_MASK          (BIT(0) | BIT(1) | BIT(2) | BIT(3))
#define DA9313_REG_VARID_VRC_SHIFT         (0)

/* DEVICE_ID reg = 0x31 */
#define DA9313_REG_DEVICEID                0x31

#define DA9313_DEVICEID_INFO               (0x81)

/* CUSTOMER_ID reg = 0x32 */
#define DA9313_REG_CUSTOMERID              0x32

/* CONFIG_ID reg = 0x33 */
#define DA9313_REG_CONFIGID                0x33

#define DA9313_REG_TOTAL_NUM               (51)

#define DA9313_REG_CTRL_ACCESS_TEST_REGS   0x50
#define DA9313_EN_ACCESS_TEST_REGS         (0x05)
#define DA9313_DIS_ACCESS_TEST_REGS        (0x0)

#define DA9313_REG_CTRL_PVC_SWITCHING      0x39
#define DA9313_STOP_PVC_SWITCHING          (0x0C)

#endif /* _DA9313_H_ */
