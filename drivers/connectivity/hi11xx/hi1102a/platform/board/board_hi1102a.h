

#ifndef __BOARD_HI1102A_H__
#define __BOARD_HI1102A_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/


/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define BOARD_VERSION_NAME_HI1102A           "hi1102a"

#define BFGX_AND_WIFI_CFG_HI1102A_PILOT_PATH       "/vendor/firmware/hi1102a/pilot/bfgx_and_wifi_cfg"
#define WIFI_CFG_HI1102A_PILOT_PATH                "/vendor/firmware/hi1102a/pilot/wifi_cfg"
#define BFGX_CFG_HI1102A_PILOT_PATH                "/vendor/firmware/hi1102a/pilot/bfgx_cfg"
#define RAM_CHECK_CFG_HI1102A_PILOT_PATH           "/vendor/firmware/hi1102a/pilot/reg_mem_test_cfg"
#define RAM_BCPU_CHECK_CFG_HI1102A_PILOT_PATH      "/vendor/firmware/hi1102a/pilot/reg_bcpu_mem_test_cfg"
#define BFGX_AND_HITALK_CFG_HI1102A_HITALK_PATH    "/vendor/firmware/hi1102a/hitalk/bfgx_and_hitalk_cfg"
#define HITALK_CFG_HI1102A_HITALK_PATH             "/vendor/firmware/hi1102a/hitalk/hitalk_cfg"
/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern int32 hi1102a_bfgx_dev_power_off(void);
extern int32 hi1102a_bfgx_dev_power_on(void);
extern void hi1102a_bfgx_disable(void);
extern void hi1102a_bfgx_enable(void);
extern void hi1102a_bfgx_subsys_reset(void);
extern int32 hi1102a_board_flowctrl_gpio_init(void);
extern int32 hi1102a_board_get_power_pinctrl(struct platform_device *pdev);
extern int32 hi1102a_board_ir_ctrl_init(struct platform_device *pdev);
extern int32 hi1102a_board_power_off(uint32 ul_subsystem);
extern int32 hi1102a_board_power_on(uint32 ul_subsystem);
extern int32 hi1102a_board_power_reset(uint32 ul_subsystem);
extern int32 hi1102a_board_wakeup_gpio_init(void);
extern int32 hi1102a_check_evb_or_fpga(void);
extern void hi1102a_chip_power_off(void);
extern void hi1102a_chip_power_on(void);
extern void hi1102a_free_board_flowctrl_gpio(void);
extern void hi1102a_free_board_power_gpio(void);
extern void hi1102a_free_board_wakeup_gpio(void);
extern int32 hi1102a_get_board_pmu_clk32k(void);
extern int32 hi1102a_get_board_power_gpio(void);
extern int32 hi1102a_get_board_uart_port(void);
extern int32 hi1102a_get_ini_file_name_from_dts(int8 *dts_prop, int8 *prop_value, uint32 size);
extern void hi1102a_tcxo_disable(void);
extern void hi1102a_tcxo_enable(void);
extern int32 hi1102a_wifi_disable(void);
extern int32 hi1102a_wifi_enable(void);
extern int32 hi1102a_wifi_subsys_reset(void);
extern int32 hi1102a_wlan_power_off(void);
extern int32 hi1102a_wlan_power_on(void);
extern int32 hi1102a_hitalk_power_off(void);
extern int32 hi1102a_hitalk_power_on(void);


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of board_hi1102a.h */
