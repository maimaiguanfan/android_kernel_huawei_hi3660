
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif

#include "board.h"


#ifndef __BOARD_HI1103_H__
#define __BOARD_HI1103_H__
/*****************************************************************************
  1 Include other Head file
*****************************************************************************/

/*****************************************************************************
  2 Define macro
*****************************************************************************/
#define BOARD_VERSION_NAME_HI1103           "hi1103"

#define GPIO_BASE_ADDR                      (0X50004000)
#define WLAN_HOST2DEV_GPIO                  ((unsigned int)(1 << 1))
#define WLAN_DEV2HOST_GPIO                  ((unsigned int)(1 << 0))

#define GPIO_LEVEL_CONFIG_REGADDR           0x0  /*GPIO�ܽŵĵ�ƽֵ���߻����ͼĴ���*/
#define GPIO_INOUT_CONFIG_REGADDR           0x04 /*GPIO�ܽŵ����ݷ������*/
#define GPIO_TYPE_CONFIG_REGADDR            0x30 /*GPIO�ܽŵ�ģʽ�Ĵ���:IO or INT*/
#define GPIO_INT_POLARITY_REGADDR           0x3C /*GPIO�жϼ��ԼĴ���*/
#define GPIO_INT_TYPE_REGADDR               0x38 /*GPIO�жϴ������ͼĴ���:��ƽ��������ش���*/
#define GPIO_INT_CLEAR_REGADDR              0x4C /*GPIO����жϼĴ�����ֻ�Ա��ش������ж���Ч*/
#define GPIO_LEVEL_GET_REGADDR              0x50 /*GPIO�ܽŵ�ǰ��ƽֵ�Ĵ���*/
#define GPIO_INTERRUPT_DEBOUNCE_REGADDR     0x48 /*GPIO�ܽ��Ƿ�ʹ��ȥ����*/

/*******just for p10+ start**********/
#define DTS_PROP_TCXO_CTRL_ENABLE           "hi110x,tcxo_ctrl_enable"
#define DTS_PROP_GPIO_TCXO_1P95V            "hi110x,gpio_tcxo_1p95v"
#define PROC_NAME_GPIO_TCXO_1P95V           "hi110x_tcxo_1p95v"
/*******just for p10+ end************/

/*test ssi write bcpu code*/
//#define SSI_DOWNLOAD_BFGX_FIRMWARE
/*****************************************************************************
  4 EXTERN VARIABLE
*****************************************************************************/

/*****************************************************************************
  5 EXTERN FUNCTION
*****************************************************************************/
extern int32 hi1103_get_board_power_gpio(void);
extern void  hi1103_free_board_power_gpio_etc(void);
extern int32 hi1103_board_wakeup_gpio_init_etc(void);
extern void  hi1103_free_board_wakeup_gpio_etc(void);
extern int32 hi1103_board_wifi_tas_gpio_init_etc(void);
extern void  hi1103_free_board_wifi_tas_gpio_etc(void);
extern int32 hi1103_check_pmu_clk_share(void);
extern int32 hi1103_bfgx_dev_power_on(void);
extern int32 hi1103_bfgx_dev_power_off(void);
extern int32 hi1103_wlan_power_off(void);
extern int32 hi1103_wlan_power_on(void);
extern int32  hi1103_board_power_on(uint32 ul_subsystem);
extern int32  hi1103_board_power_off(uint32 ul_subsystem);
extern int32  hi1103_board_power_reset(uint32 ul_subsystem);
extern int32  hi1103_wifi_subsys_reset(void);
extern void  hi1103_bfgx_subsys_reset(void);
extern int32 hi1103_get_board_pmu_clk32k(void);
extern int32 hi1103_get_board_uart_port(void);
extern int32 hi1103_board_ir_ctrl_init(struct platform_device *pdev);
extern int32 hi1103_check_evb_or_fpga(void);
extern int32 hi1103_board_get_power_pinctrl(struct platform_device *pdev);
extern int32 hi1103_get_ini_file_name_from_dts(int8 *dts_prop, int8 *prop_value, uint32 size);
extern void  hi1103_chip_power_on(void);
extern void  hi1103_bfgx_enable(void);
extern int32  hi1103_wifi_enable(void);
extern void  hi1103_chip_power_off(void);
extern void  hi1103_bfgx_disable(void);
extern int32  hi1103_wifi_disable(void);
#endif

#ifdef __cplusplus
    #if __cplusplus
            }
    #endif
#endif


