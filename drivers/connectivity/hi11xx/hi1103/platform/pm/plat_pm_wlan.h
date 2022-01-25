

#ifndef __PLAT_PM_WLAN_H__
#define __PLAT_PM_WLAN_H__

/*****************************************************************************
  1 Include other Head file
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/mutex.h>
#include <linux/kernel.h>
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) )
#include <linux/pm_wakeup.h>
#endif
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio.h>

#include <linux/fb.h>
#endif
#include "oal_hcc_bus.h"

#include "oal_ext_if.h"

#ifdef WIN32
#include "plat_type.h"
#endif

#define WLAN_WAKUP_MSG_WAIT_TIMEOUT     (1000)
#define WLAN_SLEEP_MSG_WAIT_TIMEOUT     (10000)
#define WLAN_POWEROFF_ACK_WAIT_TIMEOUT  (1000)
#define WLAN_OPEN_BCPU_WAIT_TIMEOUT     (1000)
#define WLAN_HALT_BCPU_TIMEOUT          (1000)
#define WLAN_SLEEP_TIMER_PERIOD         (20)    /*˯�߶�ʱ��50ms��ʱ*/
#define WLAN_SLEEP_DEFAULT_CHECK_CNT    (5)     /*Ĭ��100ms*/
#define WLAN_SLEEP_LONG_CHECK_CNT       (20)    /*�����׶�,�ӳ���400ms*/
#define WLAN_SLEEP_FAST_CHECK_CNT       (1)     /*fast sleep,20ms*/
#define WLAN_WAKELOCK_HOLD_TIME         (500)   /*hold wakelock 500ms*/

#define WLAN_SDIO_MSG_RETRY_NUM         (3)
#define WLAN_WAKEUP_FAIL_MAX_TIMES      (1)     /* �������ٴ�wakeupʧ�ܣ��ɽ���DFR���� */
#define WLAN_PACKET_CHECK_TIME          (5000)  /* ���Ѻ�ÿ5s��ӡһ�α��ĸ������ڳ����Ƿ��쳣��debug*/
#define WLAN_SLEEP_FORBID_CHECK_TIME    (2*60*1000) /* ����2����sleep forbid*/


#define WLAN_PM_MODULE               "[wlan]"


enum WLAN_PM_CPU_FREQ_ENUM
{
    WLCPU_40MHZ     =   1,
    WLCPU_80MHZ     =   2,
    WLCPU_160MHZ    =   3,
    WLCPU_240MHZ    =   4,
    WLCPU_320MHZ    =   5,
    WLCPU_480MHZ    =   6,
};

enum WLAN_PM_SLEEP_STAGE
{
    SLEEP_STAGE_INIT    = 0,  //��ʼ
    SLEEP_REQ_SND       = 1,  //sleep request�������
    SLEEP_ALLOW_RCV     = 2,  //�յ�allow sleep response
    SLEEP_DISALLOW_RCV  = 3,  //�յ�allow sleep response
    SLEEP_CMD_SND       = 4,  //����˯��reg�������
};

#define ALLOW_IDLESLEEP     (1)
#define DISALLOW_IDLESLEEP  (0)

#define WIFI_PM_POWERUP_EVENT          (3)
#define WIFI_PM_POWERDOWN_EVENT        (2)
#define WIFI_PM_SLEEP_EVENT            (1)
#define WIFI_PM_WAKEUP_EVENT           (0)

/*****************************************************************************
  3 STRUCT DEFINE
*****************************************************************************/
typedef oal_bool_enum_uint8 (*wifi_srv_get_pm_pause_func)(oal_void);
typedef oal_void (*wifi_srv_open_notify)(oal_bool_enum_uint8);
typedef oal_void (*wifi_srv_pm_state_notify)(oal_bool_enum_uint8);

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
typedef oal_void (*wifi_srv_data_wkup_print_en_func)(oal_bool_enum_uint8);
#endif
struct wifi_srv_callback_handler
{
    wifi_srv_get_pm_pause_func     p_wifi_srv_get_pm_pause_func;
    wifi_srv_open_notify           p_wifi_srv_open_notify;
    wifi_srv_pm_state_notify       p_wifi_srv_pm_state_notify;
};


struct wlan_pm_s
{
    hcc_bus                *pst_bus;              //����oal_bus ��ָ��

    oal_uint                ul_wlan_pm_enable;    //pmʹ�ܿ���
    oal_uint                ul_wlan_power_state;  //wlan power on state
    oal_uint                ul_apmode_allow_pm_flag;   /* apģʽ�£��Ƿ������µ����,1:����,0:������ */

    volatile oal_uint       ul_wlan_dev_state;    //wlan sleep state

    oal_workqueue_stru*     pst_pm_wq;           //pm work quque
    oal_work_stru           st_wakeup_work;       //����work
    oal_work_stru           st_sleep_work;        //sleep work
    oal_work_stru           st_ram_reg_test_work;  //ram_reg_test work

    oal_timer_list_stru     st_watchdog_timer;   //sleep watch dog
    oal_timer_list_stru     st_deepsleep_delay_timer;
    oal_wakelock_stru       st_deepsleep_wakelock;
    oal_uint32              ul_packet_cnt;        //˯��������ͳ�Ƶ�packet����
    oal_uint32              ul_packet_total_cnt;  //���ϴλ��������packet�������������for debug
    unsigned long           ul_packet_check_time;
    unsigned long           ul_sleep_forbid_check_time;
    oal_uint32              ul_wdg_timeout_cnt;  //timeout check cnt
    oal_uint32              ul_wdg_timeout_curr_cnt;  //timeout check current cnt
    volatile oal_uint       ul_sleep_stage;      //˯�߹��̽׶α�ʶ

    oal_completion          st_open_bcpu_done;
    oal_completion          st_close_bcpu_done;
    oal_completion          st_close_done;
//    oal_completion          st_device_ready;
    oal_completion          st_wakeup_done;
    oal_completion          st_sleep_request_ack;
    oal_completion          st_halt_bcpu_done;
    oal_completion          st_wifi_powerup_done;

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    oal_uint32              ul_wkup_src_print_en;
#endif

    struct wifi_srv_callback_handler st_wifi_srv_handler;

    /* ά��ͳ�� */
    oal_uint32              ul_open_cnt;
    oal_uint32              ul_open_bcpu_done_callback;
    oal_uint32              ul_close_bcpu_done_callback;
    oal_uint32              ul_close_cnt;
    oal_uint32              ul_close_done_callback;
    oal_uint32              ul_wakeup_succ;
    oal_uint32              ul_wakeup_succ_work_submit;
	oal_uint32              ul_wakeup_dev_ack;
    oal_uint32              ul_wakeup_done_callback;
    oal_uint32              ul_wakeup_fail_wait_sdio;
    oal_uint32              ul_wakeup_fail_timeout;
    oal_uint32              ul_wakeup_fail_set_reg;
    oal_uint32              ul_wakeup_fail_submit_work;

    oal_uint32              ul_sleep_succ;
    oal_uint32              ul_sleep_feed_wdg_cnt;
    oal_uint32              ul_sleep_fail_request;
    oal_uint32              ul_sleep_fail_wait_timeout;
    oal_uint32              ul_sleep_fail_set_reg;
    oal_uint32              ul_sleep_request_host_forbid;
    oal_uint32              ul_sleep_fail_forbid;
    oal_uint32              ul_sleep_fail_forbid_cnt;/*forbid ��������˯�߳ɹ��������ά����Ϣ*/
    oal_uint32              ul_sleep_work_submit;


};
typedef struct wlan_memdump_s{
    int32 addr;
    int32 len;
    int32 en;
}wlan_memdump_t;

/*****************************************************************************
  4 EXTERN VARIABLE
*****************************************************************************/
extern oal_bool_enum g_wlan_pm_switch_etc;
extern oal_uint8 g_wlan_device_pm_switch;
extern oal_uint8 g_wlan_min_fast_ps_idle;
extern oal_uint8 g_wlan_max_fast_ps_idle;
extern oal_uint8 g_wlan_auto_ps_thresh;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
extern oal_uint8 g_uc_custom_cali_done_etc;
#endif

/*****************************************************************************
  5 EXTERN FUNCTION
*****************************************************************************/
extern struct wlan_pm_s*  wlan_pm_get_drv_etc(oal_void);
extern oal_void wlan_pm_debug_sleep_etc(void);
extern oal_void wlan_pm_debug_wakeup_etc(void);
extern void wlan_pm_dump_host_info_etc(void);
extern oal_int32 wlan_pm_host_info_print_etc(struct wlan_pm_s *pst_wlan_pm,char* buf,oal_int32 buf_len);
extern void wlan_pm_dump_device_info_etc(void);
extern oal_void wlan_pm_debug_wake_lock_etc(void);
extern oal_void wlan_pm_debug_wake_unlock_etc(void);
extern struct wlan_pm_s*  wlan_pm_init_etc(oal_void);
extern oal_uint  wlan_pm_exit_etc(oal_void);
extern oal_uint32 wlan_pm_is_poweron_etc(oal_void);
extern oal_int32 wlan_pm_open_etc(oal_void);
extern oal_uint32 wlan_pm_close_etc(oal_void);
extern oal_uint32 wlan_pm_close_by_shutdown(oal_void);
extern oal_uint wlan_pm_init_dev_etc(void);
extern oal_uint wlan_pm_wakeup_dev_etc(oal_void);
extern oal_uint wlan_pm_wakeup_host_etc(void);
extern oal_uint  wlan_pm_open_bcpu_etc(oal_void);
extern oal_uint wlan_pm_state_get_etc(void);
extern oal_uint32 wlan_pm_enable_etc(oal_void);
extern oal_uint32 wlan_pm_disable_etc(oal_void);
extern oal_uint32 wlan_pm_statesave(oal_void);
extern oal_uint32 wlan_pm_staterestore(oal_void);
extern oal_uint32 wlan_pm_disable_check_wakeup_etc(oal_int32 flag);
struct wifi_srv_callback_handler* wlan_pm_get_wifi_srv_handler_etc(oal_void);
extern oal_void wlan_pm_wakeup_dev_ack_etc(oal_void);
extern oal_void  wlan_pm_set_timeout_etc(oal_uint32 ul_timeout);
extern oal_int32 wlan_pm_poweroff_cmd_etc(oal_void);
extern oal_int32 wlan_pm_shutdown_bcpu_cmd_etc(oal_void);
extern oal_void  wlan_pm_feed_wdg_etc(oal_void);
extern oal_int32 wlan_pm_stop_wdg_etc(struct wlan_pm_s *pst_wlan_pm_info);
extern void wlan_pm_info_clean_etc(void);
extern void wlan_pm_wkup_src_debug_set(oal_uint32 ul_en);
extern oal_uint32 wlan_pm_wkup_src_debug_get(void);
extern wlan_memdump_t* get_wlan_memdump_cfg(void);
extern oal_int32     wlan_mem_check_mdelay;
extern oal_int32           bfgx_mem_check_mdelay;
extern wlan_memdump_t* get_wlan_memdump_cfg(void);

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
extern oal_int32 wlan_device_mem_check_etc(oal_int32 l_runing_test_mode);
extern oal_int32 wlan_device_mem_check_result_etc(unsigned long long *time);
extern oal_void wlan_device_mem_check_work_etc(oal_work_stru *pst_worker);
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_int32 wlan_pm_register_notifier(struct notifier_block *nb);
oal_void wlan_pm_unregister_notifier(struct notifier_block *nb);
#endif

#ifdef CONFIG_HUAWEI_DSM
extern void hw_1103_register_wifi_dsm_client(void);
extern void hw_1103_unregister_wifi_dsm_client(void);
#endif

#endif

