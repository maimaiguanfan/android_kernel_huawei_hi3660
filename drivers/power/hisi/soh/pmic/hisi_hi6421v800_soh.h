#ifndef _HISI_HI6421V800_SOH_H_
#define _HISI_HI6421V800_SOH_H_


#include <linux/mfd/hisi_pmic.h>
#include <pmic_interface.h>

#ifndef BIT
#define BIT(x)      (1 << (x))
#endif

/*pmic register write lock/unlock*/
#define SOH_WRITE_PRO	                            PMIC_DEBUG_WRITE_PRO_ADDR(0)
#define SOH_WRITE_LOCK		                        0x56
#define SOH_WRITE_UNLOCK	                        0xA9

/*soh macro definition start */

#define SOH_MODE_ADDR                                PMIC_SOH_MODE_ADDR(0)
#define ACR_DIS                                      (0)
#define DCR_DISABLE                                  (0)
#define OVP_DIS                                      (0)
#define SOH_OVP_DIS                                  (0)
#define ACR_EN                                       (1)
#define DCR_ENABLE                                   (2)
#define SOH_OVP_EN                                   (3)

#define SOH_FLAG_INT_ADDR                            PMIC_ACR_SOH_IRQ_ADDR(0)
#define SOH_FLAG_INT_MASK                            (0x7c)
#define SOH_VBAT_UVP_BASE                            PMIC_SOH_VBAT_UVP_L_ADDR(0)
#define SOH_VBAT_UVP_NUM                             (2)
#define SOH_VBAT_UVP_SHIFT                           (11)
#define SOH_VBAT_UVP_THD                             (3000)

/*soh macro definition end */

/*acr macro definition start */

#define ACR_FLAG_EVENT_ADDR                         PMIC_ACR_SOH_RECORD_ADDR(0)//PMIC_ACR_SOH_IRQ_ADDR(0)
#define ACR_OCP_EVENT_ADDR                          PMIC_NP_SCP1_ADDR(0)
#define ACR_FLAG_EVENT_BIT                          BIT(PMIC_ACR_SOH_RECORD_np_acr_flag_START)
#define ACR_OCP_EVNET_BIT                           BIT(PMIC_NP_SCP1_np_acr_ocp_START)

#define ACR_FLAG_INT_ADDR                           PMIC_ACR_SOH_IRQ_ADDR(0)
#define ACR_OCP_INT_ADDR                            PMIC_SCP_IRQ1_ADDR(0)
#define ACR_FLAG_INT_BIT                            BIT(PMIC_ACR_SOH_IRQ_acr_flag_START)

#define ACR_OCP_INT_BIT                             BIT(PMIC_SCP_IRQ1_discharge_ocp_START)
#define ACR_DATA_L_SHIFT                            0
#define ACR_DATA_H_SHIFT                            8

#define ACR_SOH_IRQ_REG                             PMIC_IRQ_MASK_11_ADDR(0)
#define ACR_SOH_INT_MASK_BIT                        (0xff)
#define DCR_IRQ_REG                                 PMIC_DCR_IRQ_ADDR(0)
#define DCR_MASK_BIT                                (0xff)

#define ACR_MUL_SEL_ADDR                            PMIC_ACR_CFG0_ADDR(0)
#define ACR_MUL_MASK                                0x3f
#define ACR_MUL_SHIFT                               (PMIC_ACR_CFG0_acr_mul_sel_START)

#define ACR_H_DATA_BASE                             PMIC_ACR_DATA0_H_ADDR(0)
#define ACR_L_DATA_BASE                             PMIC_ACR_DATA0_L_ADDR(0)
#define ACR_DATA_REG_NUM                            2


#define ACR_DATA_FIFO_DEPTH                         8


#define ACR_CAL_MAGNIFICATION                       1000000
#define ACR_MOHM_TO_UOHM                            1000
#define INVALID_TEMP                                (-99)
#define ACR_GET_TEMP_RETRY                          100
#define ACR_ADC_WAIT_US                             15


#define ACR_CHIP_TEMP_CFG_ADDR                      PMIC_ACR_RESERVE_CFG_ADDR(0)
#define ACR_CHIP_TEMP_EN_ADDR                       PMIC_ACRADC_CTRL_ADDR(0)
#define ACR_CHIP_TEMP_ADC_START_ADDR                PMIC_ACRADC_START_ADDR(0)
#define ACR_CHIP_TEMP_ADC_STATUS_ADDR               PMIC_ACRCONV_STATUS_ADDR(0)
#define ACR_CHIP_TEMP_ADC_DATA1_ADDR                PMIC_ACRADC_DATA1_ADDR(0)
#define ACR_CHIP_TEMP_ADC_DATA0_ADDR                PMIC_ACRADC_DATA0_ADDR(0)
#define ACR_CHIP_TEMP_CFG                           0x4
#define ACR_CHIP_TEMP_EN                            0x2
#define ACR_CHIP_TEMP_ADC_START                     0x1
#define ACR_CHIP_TEMP_CFG_DEFAULT                   0
#define ACR_CHIP_TEMP_EN_DEFAULT                    0x80
#define ACR_CHIP_TEMP_ADC_START_DEFAULT             0
#define ACR_CHIP_TEMP_ADC_READY                     1
#define ACR_CHIP_TEMP_L_SHIFT                       4
#define ACR_CHIP_TEMP_H_SHIFT                       4

/*acr macro definition end */

/*dcr macro definition start */

#define DCR_CFG_ADDR                             PMIC_DCR_CONFIG_ADDR(0)
#define DCR_CFG_VAL                              (0x09)

#define DCR_COUNT_ADDR                           PMIC_DCR_COUNT_CONFIG_ADDR(0)
#define DCR_COUNT_1                                  (0x00)


#define DCR_FLAG_INT_MASK_REG                       PMIC_IRQ_MASK_14_ADDR(0)
#define DCR_FLAG_INT_MASK_BIT                       (0xff)

#define DCR_IRQ_FLAG_INT_REG                        PMIC_DCR_IRQ_ADDR(0)
#define DCR_FLAG_MASK                               (0xff)

#define DCR_DONE1_FLAG_MASK                         BIT(PMIC_DCR_IRQ_dcr_done1_START)
#define DCR_DONE2_FLAG_MASK                         BIT(PMIC_DCR_IRQ_dcr_done2_START)
#define DCR_DONE_FLAG_MASK                          BIT(PMIC_DCR_IRQ_dcr_end_START)

#define DCR_TIMER_CFG0_REG                           PMIC_DCR_TIMER_CONFIG0_ADDR(0)
#define DCR_TIMER_CFG1_REG                           PMIC_DCR_TIMER_CONFIG1_ADDR(0)
#define DCR_TIMER1_MASK                              (0xf0)
#define DCR_TIMER2_MASK                              (0x0f)
#define DCR_TIMER3_MASK                              (0x0f)
#define DCR_TIMER1_SHIFT                             (4)

#define DCR_TIMER_8_MS                                (0x01)
#define DCR_TIMER_250_MS                              (0x05)
#define DCR_TIMER_1000_MS                             (0x06)

#define DCR_TIME_35_SEC                              (0x0c)
#define DCR_TIME_60_SEC                              (0x0d)
#define DCR_TIME_120_SEC                             (0x0e)
#define DCR_TIME_300_SEC                             (0x0f)

#define DCR_TIME_1                                   (0x0c)
#define DCR_TIME_2                                   (0x0d)
#define DCR_TIME_3                                   (0x0e)
#define DCR_TIME_4                                   (0x0f)

#define DCR_V1_BASE_REG                              PMIC_VBAT1_0_ADDR(0)
#define DCR_I1_BASE_REG                              PMIC_IBAT1_0_ADDR(0)
#define DCR_V_BASE_REG                              PMIC_VBAT2_0_ADDR(0)
#define DCR_I_BASE_REG                              PMIC_IBAT2_0_ADDR(0)
#define DCR_VI_REG_NUM                              3
#define DCR_FIFO_LEN                                (1)

/*dcr macro definition end */

/*pd ocv macro definition start */

#define PD_OCV_ONOFF_REG                            PMIC_CLJ_DEBUG1_ADDR(0)
#define PD_OCV_ONOFF_BITMASK                        BIT(PMIC_CLJ_DEBUG1_coul_ocv_onoff_START)

#define PD_OCV_V_OCV_BASE                           PMIC_V_OCV_PRE1_OUT0_ADDR(0)
#define PD_OCV_I_OCV_BASE                           PMIC_I_OCV_PRE1_OUT0_ADDR(0)
#define PD_OCV_REG_NUM                              (3)

#define PD_OCV_FIFO_LEN                             (4)
#define PD_OCV_RTC_OCV_REG                          PMIC_RTC_OCV_OUT_ADDR(0)
#define PD_OCV_RTC_SHIFT(index)                     (6 - (index * 2))
#define PD_OCV_TEMP_REG_NUM                         (2)
#define PD_OCV_TEMP_CODE_SHIFT                      (8)

#define PD_OCV_RTC_OCV_MASK                         (0x03)

#define RTC_MIN_0                                   (0x00)
#define RTC_MIN_10                                  (0x01)
#define RTC_MIN_30                                  (0x02)
#define RTC_MIN_60                                  (0x03)

#define RTC_VAL_MIN_0    (0)
#define RTC_VAL_MIN_10   (10 * 60 )
#define RTC_VAL_MIN_30   (30 * 60 )
#define RTC_VAL_MIN_60   (60 * 60 )

#define PD_OCV_T_OCV_BASE                           PMIC_T_OCV_PRE1_OUT0_ADDR(0)

#define PD_OCV_SHIFT                                8
#define PD_OCV_DEFAULT                              0xffffff
#define PD_OCV_OFFSET_DEFAULT                       0

/*pd ocv macro definition end */

/*ovp macro definition start */

#define SOH_TIME_10S                               0
#define SOH_TIME_20S                               1
#define SOH_TIME_30S                               2
#define SOH_TIME_40S                               3

#define SOH_OVH_VOL_H_REG(x)                      (PMIC_OVP_VBAT_OVH_TH0_L_ADDR(0) + 4*x)
#define SOH_OVH_VOL_L_REG(x)                      (PMIC_OVP_VBAT_OVH_TH0_L_ADDR(0) + 4*x)
#define SOH_OVH_TEMP_H_REG(x)                     (PMIC_OVP_TBAT_OVH_TH0_L_ADDR(0) + 4*x)
#define SOH_OVH_TEMP_L_REG(x)                     (PMIC_OVP_TBAT_OVH_TH0_L_ADDR(0) + 4*x)

#define OVP_TEMP_SHIFT                             4
#define OVP_TEMP_H_ADDR_MASK                       0x0ff0
#define OVP_TEMP_L_ADDR_MASK                       0x000f
#define OVP_VOL_H_ADDR_MASK                        0x0f0000
#define OVP_VOL_H_ADDR_SHIFT                       16
#define OVP_VOL_L_ADDR_MASK                        0x00ff00
#define OVP_VOL_L_ADDR_SHIFT                       8

#define SOH_OVP_IRQ_MASK_REG                       PMIC_IRQ_MASK_11_ADDR(0)
#define SOH_OVL_IRQ_MASK                           BIT(1)
#define SOH_OVH_IRQ_MASK                           SOH_OVL_IRQ_MASK

#define OVP_MAX_VOL_MV                             4500
#define OVP_MIN_VOL_MV                             0

#define OVP_THRED_CNT                              3

#define SOH_OVP_IDIS_SEL                           PMIC_ACR_CFG0_ADDR(0)
#define OVP_IDIS_MASK                              0xf0
#define OVP_IDIS_SHIFT                             (PMIC_ACR_CFG0_ovp_idis_sel_START)
#define OVP_IDIS_50MA                              0
#define OVP_IDIS_200MA                             0xf
/*ovp macro definition end */

#endif


