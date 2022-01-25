


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "oal_ext_if.h"
#include "oal_profiling.h"
#include "oal_kernel_file.h"
#include "oal_cfg80211.h"

#include "oam_ext_if.h"
#include "frw_ext_if.h"
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
#ifndef  CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
#include "frw_task.h"
#endif
#endif

#include "wlan_spec.h"
#include "wlan_types.h"

#include "mac_vap.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "mac_ie.h"

#include "hmac_ext_if.h"
#include "hmac_chan_mgmt.h"
#include "hmac_tx_data.h"
#include "wal_main.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "wal_regdb.h"
#include "wal_linux_scan.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_bridge.h"
#include "wal_linux_flowctl.h"
#include "wal_linux_atcmdsrv.h"
#include "wal_linux_event.h"
#include "hmac_resource.h"
#include "hmac_p2p.h"

#include "wal_linux_cfg80211.h"
#include "wal_linux_cfgvendor.h"

#include "wal_dfx.h"
#include "hmac_isolation.h"

#ifdef _PRE_WLAN_FEATURE_P2P
#include "wal_linux_cfg80211.h"
#endif

#include "wal_dfx.h"


#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#include "oal_hcc_host_if.h"
#include "plat_cali.h"
#endif

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/notifier.h>
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#endif
#include "hmac_arp_offload.h"
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,59)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "../fs/proc/internal.h"
#endif

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
#include "hmac_auto_adjust_freq.h"
#endif
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
#include "hmac_tx_amsdu.h"
#endif
#ifdef _PRE_WLAN_FEATURE_WDS
#include "hmac_wds.h"
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_main.h"
#endif //_PRE_WLAN_FEATURE_ROAM
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "oal_sdio_comm.h"
#include "oal_net.h"
#include "hisi_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

//#include "dmac_alg_if.h"
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#include "plat_firmware.h"
#endif
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_IOCTL_C
#define MAX_PRIV_CMD_SIZE          4096

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
#define MAX_EQUIPMENT_GET_ARGS_SIZE 300
#endif
#if (defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT))
#define WAL_MAX_SPE_PKT_NUM       512      /* SPE����ķ��ͺͽ����������Լ���Ӧ�İ��ĸ��� */
extern oal_int32 wifi_spe_port_alloc(oal_net_device_stru *pst_netdev, struct spe_port_attr *pst_attr);
extern void wifi_spe_port_free(oal_uint32 ul_port_num);
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "board.h"
#endif
#if defined (WIN32) && (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
#include "hisi_customize_wifi_hi110x.h"
#endif

/*****************************************************************************
  2 �ṹ�嶨��
*****************************************************************************/
typedef struct
{
    oal_int8                    *pc_country;          /* �����ַ��� */
    mac_dfs_domain_enum_uint8    en_dfs_domain;       /* DFS �״��׼ */
}wal_dfs_domain_entry_stru;

typedef struct
{
    oal_uint32 ul_ap_max_user;                      /* ap����û��� */
    oal_int8   ac_ap_mac_filter_mode[257];          /* AP mac��ַ�����������,�256 */
    oal_int32  l_ap_power_flag;                     /* AP�ϵ��־ */
}wal_ap_config_stru;

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
mac_rssi_cfg_table_stru g_ast_mac_rssi_config_table[] =
{
    {"show_info",         MAC_RSSI_LIMIT_SHOW_INFO},
    {"enable",            MAC_RSSI_LIMIT_ENABLE},
    {"delta",             MAC_RSSI_LIMIT_DELTA},
    {"threshold",         MAC_RSSI_LIMIT_THRESHOLD}
};

#ifdef _PRE_FEATURE_FAST_AGING
mac_fast_aging_cfg_table_stru g_ast_dmac_fast_aging_config_table[] =
{
    {"enable",              MAC_FAST_AGING_ENABLE},
    {"timeout",             MAC_FAST_AGING_TIMEOUT},
    {"count",               MAC_FAST_AGING_COUNT},
};
#endif

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
mac_tcp_ack_buf_cfg_table_stru g_ast_hmac_tcp_ack_buf_cfg_table[] =
{
    {"enable",              MAC_TCP_ACK_BUF_ENABLE},
    {"timeout",             MAC_TCP_ACK_BUF_TIMEOUT},
    {"count",               MAC_TCP_ACK_BUF_MAX},
};
#endif

#ifdef _PRE_WLAN_FEATURE_CAR
hmac_car_cfg_table_stru g_ast_hmac_car_config_table[] =
{
    {"device_bw",         MAC_CAR_DEVICE_LIMIT},
    {"vap_bw",            MAC_CAR_VAP_LIMIT},
    {"user_bw",           MAC_CAR_USER_LIMIT},
    {"timer_cycle",       MAC_CAR_TIMER_CYCLE_MS},
    {"car_enable",        MAC_CAR_ENABLE},
    {"show_info",         MAC_CAR_SHOW_INFO},
    {"multicast",         MAC_CAR_MULTICAST},
    {"multicast_pps",     MAC_CAR_MULTICAST_PPS}
};
#endif

OAL_STATIC oal_proc_dir_entry_stru *g_pst_proc_entry = OAL_PTR_NULL;

OAL_STATIC wal_ap_config_stru g_st_ap_config_info = {0};    /* AP������Ϣ,��Ҫ��vap �������·��� */

#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
/*Just For UT*/
OAL_STATIC oal_uint32 g_wal_wid_queue_init_flag = OAL_FALSE;
#endif
OAL_STATIC wal_msg_queue g_wal_wid_msg_queue;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
oal_int8    *g_pac_wal_all_vap[] =
{
    "Hisilicon0",
    "wlan0",
    "wlan-va1",
    "wlan-va2",
    "wlan-va3",
    "Hisilicon1",
    "wlan1",
    "wlan-vai1",
    "wlan-vai2",
    "wlan-vai3"
};

oal_int8    *g_pac_wal_switch_info[] =
{
    "log_level",
    "feature_log_switch"
};

#endif

/* hi1102-cb add sys for 51/02 */
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    struct kobject     *gp_sys_kobject_etc;
#endif

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_int32 wal_hipriv_inetaddr_notifier_call_etc(struct notifier_block *this, oal_uint event, oal_void *ptr);

OAL_STATIC struct notifier_block wal_hipriv_notifier = {
    .notifier_call = wal_hipriv_inetaddr_notifier_call_etc
};

oal_int32 wal_hipriv_inet6addr_notifier_call_etc(struct notifier_block *this, oal_uint event, oal_void *ptr);

OAL_STATIC struct notifier_block wal_hipriv_notifier_ipv6 = {
    .notifier_call = wal_hipriv_inet6addr_notifier_call_etc
};
#endif
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/* ÿ�����µ�������vap��ɵĶ��ƻ�ֻ����һ�Σ�wlan p2p iface�����ظ����� */
OAL_STATIC oal_uint8 g_uc_cfg_once_flag = OAL_FALSE;
/* TBD:�����ɿ��ǿ�wifiʱ��ֻ��staut��aput��һ���ϵ�ʱ��ini�����ļ��ж�ȡ���� */
//OAL_STATIC oal_uint8 g_uc_cfg_flag = OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
extern host_speed_freq_level_stru g_host_speed_freq_level_etc[4];
extern device_speed_freq_level_stru g_device_speed_freq_level_etc[4];
#endif
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
extern oal_bool_enum_uint8 g_aen_tas_switch_en[];
#endif

#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION) && (_PRE_TEST_MODE == _PRE_TEST_MODE_UT)
/* UTģʽ�µ���frw_event_process_all_event */
extern oal_void  frw_event_process_all_event_etc(oal_uint ui_data);
#endif

#ifdef _PRE_WLAN_FEATURE_DFR
extern  hmac_dfr_info_stru    g_st_dfr_info_etc;
#endif //_PRE_WLAN_FEATURE_DFR

#ifdef _PRE_WLAN_CFGID_DEBUG
extern OAL_CONST wal_hipriv_cmd_entry_stru  g_ast_hipriv_cmd_debug_etc[];
#endif

/* ��̬�������� */
OAL_STATIC oal_uint32  wal_hipriv_vap_log_level(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_set_mcast_data_dscr_param(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);

OAL_STATIC oal_uint32  wal_hipriv_setcountry(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_getcountry(oal_net_device_stru *pst_net_dev,oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_set_bw(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
OAL_STATIC oal_uint32  wal_hipriv_always_tx(oal_net_device_stru * pst_net_dev, oal_int8 * pc_param);
//OAL_STATIC oal_uint32 wal_hipriv_always_tx_duty_ratio(oal_net_device_stru * pst_net_dev, oal_int8 * pc_param);

OAL_STATIC oal_uint32  wal_hipriv_always_tx_num(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_always_tx_hw_cfg(oal_net_device_stru * pst_net_dev, oal_int8 * pc_param);
OAL_STATIC oal_uint32  wal_hipriv_always_tx_hw(oal_net_device_stru * pst_net_dev, oal_int8 * pc_param);
//OAL_STATIC oal_uint32 wal_hipriv_always_tx_hw_duty_ratio(oal_net_device_stru * pst_net_dev, oal_int8 * pc_param);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
OAL_STATIC oal_uint32  wal_hipriv_always_tx_51(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
OAL_STATIC oal_uint32  wal_hipriv_always_rx_51(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
OAL_STATIC oal_uint32  wal_hipriv_always_rx(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_pcie_pm_level(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_user_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_add_vap(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param);
#if 0
OAL_STATIC oal_uint32  wal_hipriv_ota_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif

#ifdef _PRE_DEBUG_MODE
#ifdef _PRE_WLAN_DFT_EVENT
OAL_STATIC oal_void  wal_event_report_to_sdt(wal_msg_type_enum_uint8   en_msg_type, oal_uint8 *puc_param, wal_msg_stru *pst_cfg_msg);
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_LTECOEX
OAL_STATIC oal_uint32 wal_ioctl_ltecoex_mode_set(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_WDS
OAL_STATIC oal_uint32  wal_hipriv_wds_vap_mode(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_wds_vap_show(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_wds_sta_add(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_wds_sta_del(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_wds_sta_age(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_DHCP_REQ_DISABLE
OAL_STATIC oal_uint32  wal_hipriv_dhcp_req_disable_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
OAL_STATIC oal_uint32  wal_hipriv_aifsn_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_cw_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_SMPS
OAL_STATIC oal_uint32  wal_hipriv_set_smps_vap_mode(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_set_smps_mode(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
//OAL_STATIC oal_uint32  wal_hipriv_get_smps_mode_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif

OAL_STATIC oal_uint32  wal_hipriv_set_random_mac_addr_scan(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_add_user(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_del_user(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);

OAL_STATIC oal_uint32  wal_hipriv_reg_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
OAL_STATIC oal_uint32  wal_hipriv_sdio_flowctrl(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
#ifdef _PRE_WLAN_DELAY_STATISTIC
OAL_STATIC oal_uint32  wal_hipriv_pkt_time_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
#if 0
OAL_STATIC oal_uint32  wal_hipriv_tdls_prohibited(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_tdls_channel_switch_prohibited(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
OAL_STATIC oal_uint32  wal_hipriv_set_2040_coext_support(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_rx_fcs_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
oal_int32  wal_netdev_open_etc(oal_net_device_stru *pst_net_dev,oal_uint8 uc_entry_flag);
OAL_STATIC oal_int32  wal_net_device_ioctl(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, oal_int32 ul_cmd);
#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) && (!defined(_PRE_PRODUCT_ID_HI110X_HOST)))
OAL_STATIC oal_int32  wal_ioctl_get_assoc_req_ie(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_set_auth_mode(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_set_country_code(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_set_max_user(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_nl80211_priv_connect(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_nl80211_priv_disconnect(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_uint32 wal_get_sta_info(oal_net_sta_link_info_stru *pst_all_sta_link_info,oam_user_stat_info_stru *pst_oam_user_stat,mac_user_stru *pst_mac_user);
OAL_STATIC oal_uint32 wal_get_sta_info_ext(oal_net_sta_link_info_ext_stru *pst_all_sta_link_info_ext,oam_user_stat_info_stru *pst_oam_user_stat,mac_user_stru *pst_mac_user);
OAL_STATIC oal_int32  wal_ioctl_get_all_sta_info(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_set_vendor_ie(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
#ifdef _PRE_WLAN_FEATURE_HILINK
OAL_STATIC oal_int32  wal_ioctl_nl80211_priv_fbt_kick_user(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_set_white_lst_ssidhiden(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_get_cur_channel(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_set_stay_time(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_start_fbt_scan(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_get_sta_11k_ability(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_get_sta_11v_ability(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_change_to_other_ap(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_set_sta_bcn_request(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
OAL_STATIC oal_int32  wal_ioctl_process_vendor_req_cmd(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra);
#endif
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT_DEBUG
OAL_STATIC oal_int32  wal_ioctl_process_vendor_test_cmd(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra);
#endif
#endif
OAL_STATIC oal_int32  wal_ioctl_get_sta_11h_ability(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
#ifdef _PRE_WLAN_FEATURE_11R_AP
OAL_STATIC oal_int32  wal_ioctl_get_sta_11r_ability(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32 wal_ioctl_set_mlme(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);

#endif
OAL_STATIC oal_int32 wal_ioctl_set_channel(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32 wal_ioctl_set_wps_ie(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32 wal_ioctl_set_frag(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32 wal_ioctl_set_rts(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
oal_int32 wal_ioctl_set_ssid(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
#endif
OAL_STATIC oal_net_device_stats_stru*  wal_netdev_get_stats(oal_net_device_stru *pst_net_dev);
OAL_STATIC oal_int32  wal_netdev_set_mac_addr(oal_net_device_stru *pst_net_dev, void *p_addr);

OAL_STATIC oal_uint32  wal_hipriv_set_mode(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_set_freq(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);

/* E5 SPE module relation */
#if (defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT))
OAL_STATIC oal_int32 wal_netdev_spe_init(oal_net_device_stru *pst_net_dev);
OAL_STATIC oal_void wal_netdev_spe_exit(oal_net_device_stru *pst_net_dev);
OAL_STATIC oal_int32 wal_finish_spe_rd(oal_int32 l_port_num, oal_int32 l_src_port_num, oal_netbuf_stru *pst_buf, oal_uint32 ul_dma, oal_uint32 ul_flags);
OAL_STATIC oal_int32 wal_finish_spe_td(oal_int32 l_port_num, oal_netbuf_stru *pst_buf, oal_uint32 ul_flags);
#endif /* defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT) */

#if (defined(_PRE_E5_722_PLATFORM) || defined(_PRE_CPE_711_PLATFORM) || defined(_PRE_CPE_722_PLATFORM))
extern oal_void wlan_set_driver_lock(oal_int32 locked);
#endif

OAL_STATIC oal_uint32  wal_hipriv_rssi_limit(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#ifdef _PRE_FEATURE_FAST_AGING
OAL_STATIC oal_uint32  wal_hipriv_fast_aging_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
OAL_STATIC oal_uint32  wal_hipriv_tcp_ack_buf_cfg(oal_net_device_stru * pst_net_dev, oal_int8 * pc_param);
#endif



#ifdef _PRE_WLAN_FEATURE_CAR
OAL_STATIC oal_uint32  wal_hipriv_car_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
oal_int32 wal_ioctl_get_car_info(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
#endif
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
oal_int32 wal_ioctl_get_blkwhtlst(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
#endif
#ifdef _PRE_WLAN_REPORT_PRODUCT_LOG
OAL_STATIC oal_uint32  wal_hipriv_report_product_log_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
#ifdef _PRE_WLAN_PRODUCT_1151V200
OAL_STATIC oal_uint32 wal_hipriv_80m_rts_debug(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
#ifdef _PRE_WLAN_WEB_CMD_COMM
OAL_STATIC oal_int32  wal_ioctl_get_sta_stat_info(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_iw, oal_int8 *pc_extra);
oal_int32  wal_io2cfg_get_chan_info(oal_net_device_stru *pst_net_dev, mac_cfg_channel_param_stru *pst_channel_param, mac_beacon_param_stru *st_beacon_param);
OAL_STATIC oal_int32 wal_ioctl_set_hwaddr(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_priv_set_essid(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *pst_data, oal_int8 *pc_ssid);
OAL_STATIC oal_int32  wal_ioctl_priv_get_mode(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_priv_get_essid(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iw_point_stru *pst_data, oal_int8 *pc_ssid);
OAL_STATIC oal_int32 wal_ioctl_priv_set_mode(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, void *p_param, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_stru_param(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_w, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_setmac(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra);
#ifdef _PRE_WLAN_FEATURE_DFS
OAL_STATIC oal_int32  wal_ioctl_get_dfs_chn_stat(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_iw, oal_int8 *pc_extra);
#endif
#endif
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)
oal_int32  wal_ioctl_get_power_param(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra);
#endif
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
OAL_STATIC oal_int32  wal_ioctl_pri_chip_check(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_rx_info(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_sta_info(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_set_equipment_param(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra);
#endif
OAL_STATIC oal_uint32  wal_hipriv_set_regdomain_pwr(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_reg_write(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);


OAL_STATIC oal_uint32  wal_hipriv_tpc_log(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_dump_all_rx_dscr(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
OAL_STATIC oal_uint32   wal_hipriv_set_edca_opt_switch_sta(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_UAPSD
OAL_STATIC oal_uint32  wal_hipriv_set_uapsd_cap(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
OAL_STATIC oal_uint32  wal_hipriv_dfs_radartool(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif


OAL_STATIC oal_uint32  wal_hipriv_bgscan_enable(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param);
#ifdef _PRE_WLAN_FEATURE_STA_PM
OAL_STATIC oal_uint32  wal_hipriv_sta_ps_mode(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param);
#ifdef _PRE_PSM_DEBUG_MODE
OAL_STATIC oal_uint32  wal_hipriv_sta_ps_info(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param);
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_SINGLE_CHIP_DUAL_BAND
OAL_STATIC oal_uint32  wal_hipriv_set_restrict_band(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
#if defined(_PRE_WLAN_FEATURE_DBAC) && defined(_PRE_WLAN_FEATRUE_DBAC_DOUBLE_AP_MODE)
OAL_STATIC oal_uint32  wal_hipriv_set_omit_acs(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
OAL_STATIC oal_uint32  wal_hipriv_set_uapsd_para(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param);
#endif
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
/* hi1102-cb add sys for 51/02 */
/* OAL_STATIC ssize_t  wal_hipriv_sys_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
ssize_t (*show)(struct device *dev, struct device_attribute *attr,char *buf);
ssize_t (*store)(struct device *dev, struct device_attribute *attr,const char *buf, size_t count); */
//OAL_STATIC oal_ssize_t  wal_hipriv_sys_write(oal_device_stru *dev, oal_device_attribute_stru *attr, const oal_int8 *buf, oal_size_t count);
//OAL_STATIC oal_ssize_t  wal_hipriv_sys_read(oal_device_stru *dev, oal_device_attribute_stru *attr, oal_int8 *buf);
OAL_STATIC oal_ssize_t  wal_hipriv_sys_write(oal_device_stru *dev, oal_device_attribute_stru *attr, const char *buf, oal_size_t count);
OAL_STATIC oal_ssize_t  wal_hipriv_sys_read(oal_device_stru *dev, oal_device_attribute_stru *attr, char *buf);
OAL_STATIC OAL_DEVICE_ATTR(hipriv, (OAL_S_IRUGO|OAL_S_IWUSR), wal_hipriv_sys_read, wal_hipriv_sys_write);
OAL_STATIC struct attribute *hipriv_sysfs_entries[] = {
        &dev_attr_hipriv.attr,
        NULL
};
OAL_STATIC struct attribute_group hipriv_attribute_group = {
        .attrs = hipriv_sysfs_entries,
};
/* hi1102-cb add sys for 51/02 */
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
OAL_STATIC oal_int32 wal_ioctl_set_p2p_miracast_status(oal_net_device_stru *pst_net_dev, oal_uint8 uc_param);
OAL_STATIC oal_int32  wal_ioctl_set_p2p_noa(oal_net_device_stru * pst_net_dev, mac_cfg_p2p_noa_param_stru * pst_p2p_noa_param);
OAL_STATIC oal_int32  wal_ioctl_set_p2p_ops(oal_net_device_stru * pst_net_dev, mac_cfg_p2p_ops_param_stru * pst_p2p_ops_param);

#endif  /* _PRE_WLAN_FEATURE_P2P */
#ifdef _PRE_WLAN_FEATURE_VOWIFI
OAL_STATIC oal_int32 wal_ioctl_set_vowifi_param(oal_net_device_stru *pst_net_dev, oal_int8* puc_command);
OAL_STATIC oal_int32 wal_ioctl_get_vowifi_param(oal_net_device_stru *pst_net_dev, oal_int8 *puc_command, oal_int32 *pl_value);
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
OAL_STATIC oal_uint32  wal_ioctl_set_m2s_blacklist(oal_net_device_stru *pst_net_dev, oal_uint8 *puc_buf, oal_uint8 uc_m2s_blacklist_cnt);
oal_uint32  wal_ioctl_set_m2s_mss(oal_net_device_stru *pst_net_dev, oal_uint8 uc_m2s_mode);
#endif

#ifdef _PRE_WLAN_FEATURE_WAPI
//OAL_STATIC oal_uint32  wal_hipriv_set_wapi(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_HS20
OAL_STATIC oal_int32 wal_ioctl_set_qos_map(oal_net_device_stru *pst_net_dev,
                                                   hmac_cfg_qos_map_param_stru *pst_qos_map_param);
#endif/* #ifdef _PRE_WLAN_FEATURE_HS20 */

oal_int32 wal_ioctl_set_wps_p2p_ie_etc(oal_net_device_stru  *pst_net_dev,
                                    oal_uint8           *puc_buf,
                                    oal_uint32           ul_len,
                                    en_app_ie_type_uint8 en_type);

OAL_STATIC oal_int32 wal_set_ap_max_user(oal_net_device_stru *pst_net_dev, oal_uint32 ul_ap_max_user);
OAL_STATIC oal_int32 wal_config_mac_filter(oal_net_device_stru *pst_net_dev, oal_int8 *pc_command);
OAL_STATIC oal_int32 wal_kick_sta(oal_net_device_stru *pst_net_dev, oal_uint8 *auc_mac_addr, oal_uint16 us_reason_code);
OAL_STATIC oal_int32  wal_ioctl_set_ap_config(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_assoc_list(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_set_mac_filters(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_set_ap_sta_disassoc(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, oal_int8 *pc_extra);
OAL_STATIC oal_uint32  wal_get_parameter_from_cmd(oal_int8 *pc_cmd, oal_int8 *pc_arg, OAL_CONST oal_int8 *puc_token, oal_uint32 *pul_cmd_offset, oal_uint32 ul_param_max_len);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
OAL_STATIC oal_int32 wal_ioctl_reduce_sar(oal_net_device_stru *pst_net_dev, oal_uint16 ul_tx_power);
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
OAL_STATIC oal_int32 wal_ioctl_tas_pow_ctrl(oal_net_device_stru *pst_net_dev, oal_uint8 uc_coreindex,
                                                   oal_bool_enum_uint8 en_needImprove);
OAL_STATIC oal_int32 wal_ioctl_tas_rssi_access(oal_net_device_stru *pst_net_dev, oal_uint8 uc_coreindex);
#endif
#ifndef CONFIG_HAS_EARLYSUSPEND
OAL_STATIC oal_int32  wal_ioctl_set_suspend_mode(oal_net_device_stru *pst_net_dev, oal_uint8 uc_suspend);
#endif
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
oal_int32 wal_init_wlan_vap_etc(oal_net_device_stru *pst_net_dev);
oal_int32 wal_deinit_wlan_vap_etc(oal_net_device_stru *pst_net_dev);
oal_int32 wal_start_vap_etc(oal_net_device_stru *pst_net_dev);
oal_int32  wal_stop_vap_etc(oal_net_device_stru *pst_net_dev);
OAL_STATIC oal_int32 wal_set_mac_addr(oal_net_device_stru *pst_net_dev);
oal_int32 wal_init_wlan_netdev_etc(oal_wiphy_stru *pst_wiphy, char *dev_name);
oal_int32  wal_setup_ap_etc(oal_net_device_stru *pst_net_dev);
#endif

#ifdef _PRE_WLAN_FEATURE_11D
oal_int32  wal_regdomain_update_for_dfs_etc(oal_net_device_stru *pst_net_dev, oal_int8 *pc_country);
oal_int32  wal_regdomain_update_etc(oal_net_device_stru *pst_net_dev, oal_int8 *pc_country);
#endif

#ifdef _PRE_WLAN_FEATURE_GREEN_AP
OAL_STATIC oal_uint32  wal_hipriv_green_ap_en(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_11K
OAL_STATIC oal_uint32  wal_hipriv_send_neighbor_req(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_beacon_req_table_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
OAL_STATIC oal_uint32  wal_hipriv_voe_enable(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)
OAL_STATIC oal_uint32  wal_hipriv_cali_power(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_set_polynomial_para(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_set_load_mode(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)
OAL_STATIC oal_uint32  wal_hipriv_auto_cali(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_set_cali_vref(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
OAL_STATIC oal_uint32  wal_hipriv_load_ini_power_gain(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_APF
OAL_STATIC oal_uint32  wal_hipriv_apf_filter_list(oal_net_device_stru *pst_net_dev,oal_int8 *pc_param);
#endif
OAL_STATIC oal_uint32  wal_hipriv_remove_app_ie(oal_net_device_stru *pst_net_dev,oal_int8 *pc_param);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
OAL_STATIC oal_int32 wal_ioctl_get_wifi_priv_feature_cap_param(oal_net_device_stru *pst_net_dev, oal_int8 *puc_command, oal_int32 *pl_value);
#endif
#ifdef  _PRE_WLAN_FEATURE_FORCE_STOP_FILTER
OAL_STATIC oal_uint32  wal_ioctl_force_stop_filter(oal_net_device_stru *pst_net_dev,oal_uint8 uc_param);
#endif
extern oal_uint8 wal_cfg80211_convert_value_to_vht_width(oal_int32 l_channel_value);
extern oal_uint32 wal_hipriv_set_rfch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
extern oal_uint32 wal_hipriv_set_rxch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
extern oal_uint32  wal_hipriv_set_nss(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
extern oal_uint32  wal_hipriv_send_cw_signal(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
extern oal_uint32  wal_hipriv_adjust_ppm(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
extern oal_uint32  wal_hipriv_chip_check(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
extern oal_uint32  wal_hipriv_set_txpower(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);

/*****************************************************************************
  ˽���������. ˽�������ʽ:
         �豸�� ������ ����
  hipriv "Hisilicon0 create vap0"
*****************************************************************************/
/*Android private command strings*/
#define CMD_SET_AP_WPS_P2P_IE   "SET_AP_WPS_P2P_IE"
#define CMD_P2P_SET_NOA         "P2P_SET_NOA"
#define CMD_P2P_SET_PS          "P2P_SET_PS"
#define CMD_SET_POWER_ON        "SET_POWER_ON"
#define CMD_SET_POWER_MGMT_ON   "SET_POWER_MGMT_ON"
#define CMD_COUNTRY             "COUNTRY"
#define CMD_GET_CAPA_DBDC       "GET_CAPAB_RSDB"
#define CMD_CAPA_DBDC_SUPP      "RSDB:1"
#define CMD_CAPA_DBDC_NOT_SUPP  "RSDB:0"

#ifdef _PRE_WLAN_FEATURE_LTECOEX
#define CMD_LTECOEX_MODE        "LTECOEX_MODE"
#endif
#define CMD_SET_QOS_MAP         "SET_QOS_MAP"
#define CMD_TX_POWER            "TX_POWER"
#define CMD_WPAS_GET_CUST       "WPAS_GET_CUST"
#define CMD_SET_STA_PM_ON       "SET_STA_PM_ON"

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
/* TAS�����л����� */
#define CMD_SET_MEMO_CHANGE     "SET_ANT_CHANGE"
/* �������� */
#define CMD_MEASURE_TAS_RSSI    "SET_TAS_MEASURE"
/* ̧���� */
#define CMD_SET_TAS_TXPOWER     "SET_TAS_TXPOWER"
/* ��ȡ���� */
#define CMD_TAS_GET_ANT         "TAS_GET_ANT"
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
#define CMD_SET_M2S_SWITCH      "SET_M2S_SWITCH"
#define CMD_SET_M2S_BLACKLIST   "SET_M2S_BLACKLIST"
#endif

#ifdef _PRE_WLAN_FEATURE_VOWIFI
/*
VOWIFI_DETECT SET/GET MODE [param]
VOWIFI_DETECT SET/GET PERIOD [param]
VOWIFI_DETECT SET/GET LOW_THRESHOLD [param]
VOWIFI_DETECT SET/GET HIGH_THRESHOLD [param]
VOWIFI_DETECT SET/GET TRIGGER_COUNT [param]
VOWIFI_DETECT VOWIFi_IS_SUPPORT
*/

#define CMD_VOWIFI_SET_MODE             "VOWIFI_DETECT SET MODE"
#define CMD_VOWIFI_GET_MODE             "VOWIFI_DETECT GET MODE"
#define CMD_VOWIFI_SET_PERIOD           "VOWIFI_DETECT SET PERIOD"
#define CMD_VOWIFI_GET_PERIOD           "VOWIFI_DETECT GET PERIOD"
#define CMD_VOWIFI_SET_LOW_THRESHOLD    "VOWIFI_DETECT SET LOW_THRESHOLD"
#define CMD_VOWIFI_GET_LOW_THRESHOLD    "VOWIFI_DETECT GET LOW_THRESHOLD"
#define CMD_VOWIFI_SET_HIGH_THRESHOLD   "VOWIFI_DETECT SET HIGH_THRESHOLD"
#define CMD_VOWIFI_GET_HIGH_THRESHOLD   "VOWIFI_DETECT GET HIGH_THRESHOLD"
#define CMD_VOWIFI_SET_TRIGGER_COUNT    "VOWIFI_DETECT SET TRIGGER_COUNT"
#define CMD_VOWIFI_GET_TRIGGER_COUNT    "VOWIFI_DETECT GET TRIGGER_COUNT"

#define CMD_VOWIFI_SET_PARAM            "VOWIFI_DETECT SET"
#define CMD_VOWIFI_GET_PARAM            "VOWIFI_DETECT GET"

#define CMD_VOWIFI_IS_SUPPORT_REPLY     "true"
#else
#define CMD_VOWIFI_IS_SUPPORT_REPLY     "false"
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
#define CMD_VOWIFI_IS_SUPPORT           "VOWIFI_DETECT VOWIFI_IS_SUPPORT"

#define CMD_GET_WIFI_PRIV_FEATURE_CAPABILITY   "GET_WIFI_PRIV_FEATURE_CAPABILITY"

#define CMD_SETSUSPENDOPT               "SETSUSPENDOPT"
#define CMD_SETSUSPENDMODE              "SETSUSPENDMODE"

oal_int8 *g_pac_mib_operation_cmd_name[] =
{
    "smps_vap_mode",
    "info",
    "2040_coexistence",
    "freq",
    "mode",
    "set_mcast_data",
    "set_edca_switch_sta",
    "beacon_offload_test",/* debug begin */
    "auto_ba",
    "reassoc_req",
    "coex_preempt_type",
    "protocol_debug",
    "start_scan",
    "start_join",
    "kick_user",
    "ampdu_tx_on",
    "amsdu_tx_on",
    "ampdu_amsdu",
    "frag_threshold",
    "wmm_switch",
    "hide_ssid",
    "set_stbc_cap",
    "set_ldpc_cap",
    "set_psm_para",
    "set_sta_pm_on",
    "enable_pmf",
    "send_pspoll",
    "send_nulldata",
    "set_default_key",
    "add_key",
    "auto_protection",
    "send_2040_coext",
    "set_opmode_notify",
    "m2u_igmp_pkt_xmit",
    "smps_info",
    "vap_classify_tid",
    "essid",
    "bintval",
    "ampdu_mmss",
    "set_tx_classify_switch",
    "custom_info",
    "11v_cfg_wl_mgmt",
    "11v_cfg_bsst",
    "send_radio_meas_req",
    "11k_switch"
};

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
#define CMD_SET_RX_FILTER_ENABLE    "set_rx_filter_enable"
#define CMD_ADD_RX_FILTER_ITEMS     "add_rx_filter_items"
#define CMD_CLEAR_RX_FILTERS        "clear_rx_filters"
#define CMD_GET_RX_FILTER_PKT_STATE "get_rx_filter_pkt_state"

#define CMD_FILTER_SWITCH    "FILTER"
#endif /* _PRE_WLAN_FEATURE_IP_FILTER */

#define CMD_RXFILTER_START  "RXFILTER-START"
#define CMD_RXFILTER_STOP   "RXFILTER-STOP"
#define CMD_MIRACAST_START  "MIRACAST 1"
#define CMD_MIRACAST_STOP   "MIRACAST 0"


OAL_STATIC OAL_CONST wal_hipriv_cmd_entry_stru  g_ast_hipriv_cmd[] =
{
    /************************���ö��ⷢ����˽������*******************/
    {"info",                    wal_hipriv_vap_info_etc},               /* ��ӡvap�����в�����Ϣ: hipriv "vap0 info" */
    {"setcountry",              wal_hipriv_setcountry},             /*���ù��������� hipriv "Hisilicon0 setcountry param"paramȡֵΪ��д�Ĺ������֣����� CN US*/
    {"getcountry",              wal_hipriv_getcountry},             /*��ѯ���������� hipriv "Hisilicon0 getcountry"*/
#ifdef _PRE_WLAN_FEATURE_GREEN_AP
    {"green_ap_en",             wal_hipriv_green_ap_en},            /* green AP����: hipriv "wlan0 green_ap_en 0 | 1" */
#endif
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    {"ip_filter",               wal_hipriv_set_ip_filter_etc},          /* ip filter(���ܵ��Խӿ�)hipriv "wlan0 ip_filter cmd param0 param1 ...."
                                                                       ����:�������� "wlan0 ip_filter set_rx_filter_enable 1/0"
                                                                           ��պ����� "wlan0 ip_filter clear_rx_filters"
                                                                           ���ú����� "wlan0 ip_filter add_rx_filter_items ��Ŀ����(0/1/2...) ��������(protocol0 port0 protocol1 port1...)",Ŀǰ�õ��Խӿڽ�֧��20����Ŀ
                                                                    */
#endif //_PRE_WLAN_FEATURE_IP_FILTER

    {"userinfo",                wal_hipriv_user_info},              /* ��ӡָ��mac��ַuser�����в�����Ϣ: hipriv "vap0 userinfo XX XX XX XX XX XX(16����oal_strtohex)" */
    {"reginfo",                 wal_hipriv_reg_info},               /* ��ӡ�Ĵ�����Ϣ: hipriv "Hisilicon0 reginfo 16|32(51û��16λ�Ĵ�����ȡ����) regtype(soc/mac/phy) startaddr endaddr" */
    {"pcie_pm_level",           wal_hipriv_pcie_pm_level},          /* ����pcie�͹��ļ��� hipriv "Hisilicon0 pcie_pm_level level(01/2/3/4)" */
    {"regwrite",                wal_hipriv_reg_write},              /* ��ӡ�Ĵ�����Ϣ: hipriv "Hisilicon0 regwrite 32/16(51û��16λд�Ĵ�������) regtype(soc/mac/phy) addr val" addr val���붼��16����0x��ͷ */
    {"dump_all_dscr",           wal_hipriv_dump_all_rx_dscr},       /* ��ӡ���еĽ���������, hipriv "Hisilicon0 dump_all_dscr" */
    {"random_mac_addr_scan",    wal_hipriv_set_random_mac_addr_scan}, /* �������mac addrɨ�迪�أ�sh hipriv.sh "Hisilicon0 random_mac_addr_scan 0|1(��|�ر�)" */
    {"bgscan_enable",           wal_hipriv_bgscan_enable},        /* ɨ��ֹͣ�������� hipriv "Hisilicon0 bgscan_enable param1" param1ȡֵ'0' '1',��Ӧ�رպʹ򿪱���ɨ��, '2' ��ʾ�ر�����ɨ�� */
    {"2040_coexistence",        wal_hipriv_set_2040_coext_support},                /* ����20/40����ʹ��: hipriv "vap0 2040_coexistence 0|1" 0��ʾ20/40MHz����ʹ�ܣ�1��ʾ20/40MHz���治ʹ�� */

#ifdef _PRE_WLAN_FEATURE_STA_PM
    {"set_ps_mode",             wal_hipriv_sta_ps_mode},           /* ����PSPOLL���� sh hipriv.sh 'wlan0 set_ps_mode 3'*/
#ifdef _PRE_PSM_DEBUG_MODE
    {"psm_info_debug",          wal_hipriv_sta_ps_info},            /* sta psm��ά��ͳ����Ϣ sh hipriv.sh 'wlan0 psm_info_debug 1' */
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_SMPS
    {"smps_vap_mode",          wal_hipriv_set_smps_vap_mode},            /* ����vap��smpsģʽ sh hipriv.sh 'wlan0 smps_vap_mode 1/2/3' static/dynamic/disable*/
//    {"smps_mode",              wal_hipriv_set_smps_mode},                /* ����device��smpsģʽ sh hipriv.sh 'Hisilicon0 smps_mode 1/2/3' static/dynamic/disable  ����03 device1ֻ������ΪSTATIC*/
//    {"smps_mode_info",         wal_hipriv_get_smps_mode_info},           /* �鿴vap������device��smpsģʽ sh hipriv.sh 'wlan0 smps_mode_info'  1/2/3 = static/dynamic/disable  ����03 device1ֻ������ΪSTATIC*/
#endif
#ifdef _PRE_WLAN_FEATURE_UAPSD
    {"uapsd_en_cap",           wal_hipriv_set_uapsd_cap},        /* hipriv "vap0 uapsd_en_cap 0\1" */
#endif
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    {"set_uapsd_para",          wal_hipriv_set_uapsd_para},        /* ����uapsd�Ĳ�����Ϣ sh hipriv.sh 'wlan0 set_uapsd_para 3 1 1 1 1 */
#endif
    {"create",                  wal_hipriv_add_vap},                /* ����vap˽������Ϊ: hipriv "Hisilicon0 create vap0 ap|sta" */
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    {"al_tx",                   wal_hipriv_always_tx},               /* ���ó���ģʽ:       hipriv "vap0 al_tx <value: 0/1/2>  <len>" ����mac���ƣ�11a,b,g��ֻ֧��4095�������ݷ���,����ʹ��set_mcast_data�����ʽ�������*/
    {"al_tx_1102",              wal_hipriv_always_tx},               /* al_tx_02�����ȱ��� */
    {"al_tx_num",               wal_hipriv_always_tx_num},           /* ���ó�����Ŀ:       hipriv "vap0 al_tx_num <value>" */
    //{"al_tx_duty_ratio",        wal_hipriv_always_tx_duty_ratio},  /* ���ó���ռ�ձ�:     hipriv "vap0 al_tx_duty_ratio <value>  " value��ֵС��100 */
    {"al_tx_hw_cfg ",           wal_hipriv_always_tx_hw_cfg},        /* ����Ӳ������ģʽ:   hipriv "vap0 al_tx_hw_cfg devid XXX XXX" ��devid mode�����ʽ�������*/
    /* ����Ӳ������:       hipriv "vap0 al_tx_hw  0/1/2/3  0/1/2/3  content len times"
    1. 0/1/2/3 Ϊ���ء�bit 1��Ӧdev0/dev1��ѡ��bit 0��Ӧ���ء�������ùرյĻ�����������þͲ���Ҫ�����ˡ�
    �û�ֻҪ���ö�Ӧ��devΪ��/�ؾ��ܹ��رջ��߿������߹ر�Ӳ��������
    2. 0/1/2/3 ����֡�����ݡ�0:ȫ0��1:ȫ1��2:����� 3:�ظ�byte�����ݡ�
    3. content������2������Ϊ3ʱ����ֵ��Ч��������Բ��
    4. len���ݳ��ȡ�
    5. ֡���͵Ĵ���                   */
    {"al_tx_hw",                wal_hipriv_always_tx_hw},              /* ����Ӳ������:       hipriv "vap0 al_tx_hw <switch(x)|device id(x)| flag(xx)|xxxx> <content> <len> <times> <ifs>"*/
    //{"al_tx_hw_duty_ratio",     wal_hipriv_always_tx_hw_duty_ratio}, /* ����Ӳ������ռ�ձ�: hipriv "vap0 al_tx_hw_duty_ratio <value 1~100>"*/
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    {"al_tx_51",                wal_hipriv_always_tx_51},
#endif
#endif
    {"al_rx",                   wal_hipriv_always_rx},               /* ���ó���ģʽ:               hipriv "vap0 al_rx <value: 0/1/2>" */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    {"al_rx_51",                wal_hipriv_always_rx_51},
#endif
    {"rate",                    wal_hipriv_set_rate_etc },               /* ����non-HTģʽ�µ�����:     hipriv "vap0 rate  <value>" */
    {"mcs",                     wal_hipriv_set_mcs_etc  },               /* ����HTģʽ�µ�����:         hipriv "vap0 mcs   <value>" */
    {"mcsac",                   wal_hipriv_set_mcsac_etc},               /* ����VHTģʽ�µ�����:        hipriv "vap0 mcsac <value>" */
#ifdef _PRE_WLAN_FEATURE_11AX
    {"mcsax",                   wal_hipriv_set_mcsax},               /* ����VHTģʽ�µ�����:        hipriv "vap0 mcsac <value>" */
#endif
    {"freq",                    wal_hipriv_set_freq},                    /* ����AP �ŵ� */
    {"mode",                    wal_hipriv_set_mode},                    /* ����AP Э��ģʽ */
    {"bw",                      wal_hipriv_set_bw   },               /* ���ô���:                   hipriv "vap0 bw    <value>" */
    {"set_mcast_data",          wal_hipriv_set_mcast_data_dscr_param},    /* ��ӡ��������Ϣ: hipriv "vap0 set_mcast_data <param name> <value>" */
    {"rx_fcs_info",             wal_hipriv_rx_fcs_info},            /* ��ӡ����֡��FCS��ȷ�������Ϣ:hipriv "vap0 rx_fcs_info 0/1 1/2/3/4" 0/1  0���������1������� */
    {"set_regdomain_pwr",       wal_hipriv_set_regdomain_pwr},      /* ���ù���������͹��ʣ�hipriv "Hisilicon0 set_regdomain_pwr 20",��λdBm */
    {"add_user",                wal_hipriv_add_user},               /* ��������û�����������: hipriv "vap0 add_user xx xx xx xx xx xx(mac��ַ) 0 | 1(HT����λ) "  ���������ĳһ��VAP */
    {"del_user",                wal_hipriv_del_user},               /* ����ɾ���û�����������: hipriv "vap0 del_user xx xx xx xx xx xx(mac��ַ)" ���������ĳһ��VAP */
    {"alg_cfg",                 wal_hipriv_alg_cfg_etc},                /* �㷨��������: hipriv "vap0 alg_cfg sch_vi_limit 10"*/
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    {"set_edca_switch_sta",        wal_hipriv_set_edca_opt_switch_sta},       /* STA�Ƿ���˽��edca�����Ż����� */
#endif
    {"alg_tpc_log",             wal_hipriv_tpc_log},                /* tpc�㷨��־��������:*/
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
    {"sdio_flowctrl",           wal_hipriv_sdio_flowctrl},
#endif
#ifdef _PRE_WLAN_DELAY_STATISTIC
    /* ����ʱ������Կ���: hipriv "Hisilicon0 pkt_time_switch on |off */
    {"pkt_time_switch",         wal_hipriv_pkt_time_switch},
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
    {"radartool",               wal_hipriv_dfs_radartool},
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    {"aifsn_cfg",               wal_hipriv_aifsn_cfg},              /* wfaʹ�ã��̶�ָ��AC��aifsnֵ, sh hipriv.sh "Hisilicon0 aifsn_cfg 0|1(�ָ�|����) 0|1|2|3(be-vo) val" */
    {"cw_cfg",                  wal_hipriv_cw_cfg},                 /* wfaʹ�ã��̶�ָ��AC��cwmaxminֵ, sh hipriv.sh "Hisilicon0 cw_cfg 0|1(�ָ�|����) 0|1|2|3(be-vo) val" */
#endif
#ifdef _PRE_WLAN_FEATURE_11K
    {"send_neighbor_req",       wal_hipriv_send_neighbor_req},          /* sh hipriv.sh "wlan0 send_neighbor_req WiFi1" */
    {"beacon_req_table_switch", wal_hipriv_beacon_req_table_switch}, /* sh hipriv.sh "wlan0 beacon_req_table_switch 0/1" */
#endif
    {"voe_enable",              wal_hipriv_voe_enable},             /* VOE����ʹ�ܿ��ƣ�Ĭ�Ϲر� sh hipriv.sh "wlan0 voe_enable 0/1" (Bit0:11r  Bit1:11V Bit2:11K Bit3:�Ƿ�ǿ�ư���IE70(voe ��֤��Ҫ),Bit4:11r��֤,B5-B6:11k auth operating class  Bit7:��ȡ��Ϣ)*/

    {"log_level",               wal_hipriv_vap_log_level},          /* VAP������־���� hipriv "VAPX log_level {1|2}"  Warning��Error������־��VAPΪά�� */
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)
    {"cali_power",              wal_hipriv_cali_power},             /*ant_value��0��1ʱ��ʵ��Ĺ���ֵ�·���wifi������������ hipriv.sh "Hisilicon0 cali_power ant_value subband_value pow_value0 pow_value 1 pow_value 2"��ant_valueΪ2ʱ����ȡ����ʽֵ*/
    {"set_power_param",         wal_hipriv_set_polynomial_para},    /* ���ö���ʽֵ hipriv.sh "Hisilicon0 set_power_param subband ant0_param ant1_param" */
    {"set_load_mode",         wal_hipriv_set_load_mode},
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    {"load_ini_gain",           wal_hipriv_load_ini_power_gain}, /* ˢ�¶��ƻ��ļ��й���������ز��� hipriv.sh "Hisilicon0 load_ini_gain" */
#endif

#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)
    {"auto_cali",        wal_hipriv_auto_cali},      /* ����У׼�Զ���:hipriv "Hisilicon0 auto_cali" */
    {"cali_vref",        wal_hipriv_set_cali_vref},  /* У׼�����޸�:hipriv "wlan0 cali_vref value" */
#endif
#ifdef _PRE_WLAN_FEATURE_SINGLE_CHIP_DUAL_BAND
    {"restrict_band",    wal_hipriv_set_restrict_band}, /* �����޶�������Ƶ�� */
#endif
#if defined(_PRE_WLAN_FEATURE_DBAC) && defined(_PRE_WLAN_FEATRUE_DBAC_DOUBLE_AP_MODE)
    {"omit_acs",         wal_hipriv_set_omit_acs},      /* ���ú���ACS */
#endif

#ifdef _PRE_WLAN_FEATURE_WDS
    {"wds_vap_mode",   wal_hipriv_wds_vap_mode}, /* ����vap�� wdsʹ��: hipriv "wlan0 wds_vap_mode 0/1/2/3" �ڶ������� 0:�ر�WDS, 1:RootAPģʽ, 2:RepeaterAPģʽ�µ�STA Vap, 3:RepeaterAPģʽ�µ�AP Vap */
    {"wds_vap_show",   wal_hipriv_wds_vap_show}, /* ��ӡWDS Vap��������Ϣ��״̬��Ϣ */
    {"wds_sta_add",   wal_hipriv_wds_sta_add}, /* ���WDS STA��Ϣ�� WDS����: hipriv "wlan0 wds_sta_add StaMac PeerNodeMac" �ڶ������� StaMacΪ����ӵ� WDS STA�� Mac��ַ, ���������� PeerNodeMacΪWDS STA�󶨵�WDS�Զ˽ڵ��Mac(�Զ�ΪSTAģʽ��Vap,����öԶ˽ڵ㲻���ڣ������ʧ��) */
    {"wds_sta_del",   wal_hipriv_wds_sta_del}, /* ɾ��WDS���ж�ӦWDS STA��Ϣ: hipriv "wlan0 wds_sta_del StaMac" �ڶ������� StaMacΪ����ӵ� WDS STA�� Mac��ַ */
    {"wds_sta_age",   wal_hipriv_wds_sta_age}, /* ����WDS����WDS STA��Чʱ��: hipriv "wlan0 wds_sta_age AgeTime" �ڶ������� WDS Sta�����Ծʱ��(��λΪ��) */
#endif

#ifdef _PRE_WLAN_FEATURE_DHCP_REQ_DISABLE
    {"dhcp_req_disable",         wal_hipriv_dhcp_req_disable_switch},           /* dhcp_req_disableģ��Ŀ��ص�����: hipriv "vap0 dhcp_req_disable 0 | 1" */
#endif
#ifdef _PRE_WLAN_FEATURE_SMPS
    {"smps_mode",       wal_hipriv_set_smps_mode},      /* ����smps mode */
#endif
    {"rssi_limit",      wal_hipriv_rssi_limit},         /* rssi���޿��غ���ֵ */
#ifdef _PRE_WLAN_FEATURE_CAR
    {"car_cfg",         wal_hipriv_car_cfg},            /* car���ٲ�������: */
#endif
#ifdef _PRE_FEATURE_FAST_AGING
    {"fast_aging",       wal_hipriv_fast_aging_cfg},    /* �����ϻ� */
#endif

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    {"tcp_ack_buf",       wal_hipriv_tcp_ack_buf_cfg},    /* �����ϻ� */
#endif

#ifdef _PRE_WLAN_REPORT_PRODUCT_LOG
    {"product_log_flag",         wal_hipriv_report_product_log_cfg},            /* ��Ʒlog�ϱ����ã�hipriv "wlan0 product_log_flag 0/1/2"; 0/1��ʾ�ؿ���2��ʾ��ѯ��ǰ���� */
#endif

#ifdef _PRE_WLAN_PRODUCT_1151V200
    {"80m_rts_debug",         wal_hipriv_80m_rts_debug},                        /* 80M rts debugģʽ��hipriv "wlan0 80m_rts_debug 0/1"; 0/1��ʾ�ؿ� */
#endif
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    {"pk_mode_debug",         wal_hipriv_pk_mode_debug},                        /* pkmode���ܵ����޵����ӿ� hipriv "wlan0 pk_mode_debug 0/1(high/low) 0/1/2/3/4(BW) 0/1/2/3(protocol) ��������ֵ" */
#endif
#ifdef _PRE_WLAN_FEATURE_APF
    {"apf_filter_list",       wal_hipriv_apf_filter_list},
#endif
    {"remove_app_ie",         wal_hipriv_remove_app_ie},      /* ͨ��eid�Ƴ��û�̬�·���ĳ��IE hipriv "wlan0 remove_app_ie 0/1 eid" 0�ָ���ie,1���θ�ie */
};

/*****************************************************************************
  net_device�Ϲҽӵ�net_device_ops����
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_net_device_ops_stru g_st_wal_net_dev_ops_etc =
{
    .ndo_get_stats          = wal_netdev_get_stats,
    .ndo_open               = wal_netdev_open_ext,
    .ndo_stop               = wal_netdev_stop_etc,
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    .ndo_start_xmit         = wal_vap_start_xmit,
#else
    .ndo_start_xmit         = wal_bridge_vap_xmit_etc,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
            /* TBD */
#else
    .ndo_set_multicast_list = OAL_PTR_NULL,
#endif

    .ndo_do_ioctl           = wal_net_device_ioctl,
    .ndo_change_mtu         = oal_net_device_change_mtu,
    .ndo_init               = oal_net_device_init,

#if (defined(_PRE_WLAN_FEATURE_FLOWCTL) || defined(_PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL))
    .ndo_select_queue       = wal_netdev_select_queue_etc,
#endif

    .ndo_set_mac_address    = wal_netdev_set_mac_addr
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
oal_ethtool_ops_stru g_st_wal_ethtool_ops_etc = { 0 };
#endif

#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
oal_net_device_ops_stru g_st_wal_net_dev_ops_etc =
{
    oal_net_device_init,
    wal_netdev_open_ext,
    wal_netdev_stop_etc,
    wal_bridge_vap_xmit_etc,
    OAL_PTR_NULL,
    wal_netdev_get_stats,
    wal_net_device_ioctl,
    oal_net_device_change_mtu,
    wal_netdev_set_mac_addr
};
oal_ethtool_ops_stru g_st_wal_ethtool_ops_etc = { 0 };
#endif
/*****************************************************************************
  ��׼ioctl�������.
*****************************************************************************/

/*****************************************************************************
  ˽��ioctl����������嶨��
*****************************************************************************/
OAL_STATIC OAL_CONST oal_iw_priv_args_stru g_ast_iw_priv_args[] =
{
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    {WAL_IOCTL_PRIV_SET_MGMT_FRAME_FILTERS,  OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "set_app_filter"},     /* ����������char, ����Ϊ24 */
#endif
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT_DEBUG
    {WAL_IOCTL_PRIV_SET_VENDOR_REQ,  OAL_IW_PRIV_TYPE_CHAR | 64, 0, "set_vendor_req"},
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    {WAL_IOCTL_PRIV_SET_AP_CFG, OAL_IW_PRIV_TYPE_CHAR |  256, 0,"AP_SET_CFG"},
    {WAL_IOCTL_PRIV_AP_MAC_FLTR, OAL_IW_PRIV_TYPE_CHAR | 256, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_SIZE_FIXED | 0, "AP_SET_MAC_FLTR"},
    {WAL_IOCTL_PRIV_AP_GET_STA_LIST, 0, OAL_IW_PRIV_TYPE_CHAR | 1024, "AP_GET_STA_LIST"},
    {WAL_IOCTL_PRIV_AP_STA_DISASSOC, OAL_IW_PRIV_TYPE_CHAR | 256, OAL_IW_PRIV_TYPE_CHAR | 0, "AP_STA_DISASSOC"},
#else
    {WAL_IOCTL_PRIV_SET_AP_CFG, OAL_IW_PRIV_TYPE_CHAR |  256, 0,"ap_set_cfg"},
    {WAL_IOCTL_PRIV_AP_MAC_FLTR, OAL_IW_PRIV_TYPE_CHAR | 256, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_SIZE_FIXED | 0, "ap_set_mac_fltr"},
    {WAL_IOCTL_PRIV_AP_GET_STA_LIST, 0, OAL_IW_PRIV_TYPE_CHAR | 1024, "ap_get_sta_list"},
    {WAL_IOCTL_PRIV_AP_STA_DISASSOC, OAL_IW_PRIV_TYPE_CHAR | 256, OAL_IW_PRIV_TYPE_CHAR | 0, "ap_sta_disassoc"},
#endif

#ifdef _PRE_WLAN_FEATURE_WDS
    {WLAN_CFGID_WDS_VAP_MODE,   OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,  0,  "wds_vap_mode"},  /* ����VAP��WDSģʽʹ�� */
    {WLAN_CFGID_WDS_STA_ADD,    OAL_IW_PRIV_TYPE_CHAR | 40,                         0,  "wds_sta_add"},
    {WLAN_CFGID_WDS_STA_DEL,    OAL_IW_PRIV_TYPE_CHAR | 40,                         0,  "wds_sta_del"},
    {WLAN_CFGID_WDS_VAP_MODE,   0,   OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_wds_vap_mode"},  /*��ȡVAP��WDSģʽʹ�� */
#endif
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
    {WLAN_CFGID_ADD_BLACK_LIST,     OAL_IW_PRIV_TYPE_CHAR | 40, 0, "blkwhtlst_add"},
    {WLAN_CFGID_DEL_BLACK_LIST,     OAL_IW_PRIV_TYPE_CHAR | 40, 0, "blkwhtlst_del"},
    {WLAN_CFGID_CLR_BLACK_LIST,     OAL_IW_PRIV_TYPE_CHAR | 40, 0, "blkwhtlst_clr"},
    {WLAN_CFGID_BLACKLIST_MODE,     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "blkwhtlst_mode"},  /* ���ú�����ģʽ */
    {WLAN_CFGID_BLACKLIST_MODE,     0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_blkwhtmode"},  /*������ģʽ��ӡ */
#endif

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    {WLAN_CFGID_GET_2040BSS_SW,         0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_obss_sw"},
    {WLAN_CFGID_2040BSS_ENABLE,         OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "set_obss_sw"},
#endif

#ifdef _PRE_DEBUG_MODE
    {WLAN_CFGID_GET_FRW_MAX_EVENT,         0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_frw_max"},
    {WLAN_CFGID_SET_FRW_MAX_EVENT,         OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "set_frw_max"},
#endif
#ifdef _PRE_FEATURE_WAVEAPP_CLASSIFY
    {WLAN_CFGID_GET_WAVEAPP_FLAG,         0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_waveapp_flag"}, /*��ȡ��ǰwaveapp����ʶ��ı��*/
#endif
#ifdef _PRE_WLAN_WEB_CMD_COMM
    {WAL_IOCTL_PRIV_SET_HWADDR,                OAL_IW_PRIV_TYPE_CHAR | 24, 0, "sethwaddr"}, /*set hwaddr*/
    {WLAN_CFGID_WMM_SWITCH,                 OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "wmmswitch"}, /*wmm switch*/
    {WLAN_CFGID_WMM_SWITCH,                 0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_wmmswitch"}, /*wmm switch*/
    {WLAN_CFGID_VAP_WMM_SWITCH,             OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "set_vapwmm"}, /*vap wmm switch*/
    {WLAN_CFGID_VAP_WMM_SWITCH,             0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_vapwmm"}, /*vap wmm switch*/
    {WAL_IOCTL_PRIV_GET_ESSID,                       0, OAL_IW_PRIV_TYPE_CHAR | 33, "get_essid" },
    {WAL_IOCTL_PRIV_SET_ESSID,              OAL_IW_PRIV_TYPE_CHAR | 33, 0, "setessid"},
    {WLAN_CFGID_SET_PRIV_FLAG,              OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "privflag"},
    {WLAN_CFGID_CHANNEL,                0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_channel"},
    {WLAN_CFGID_CHANNEL,                OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "channel"},

    {WLAN_CFGID_TX_POWER,           0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_txpower"},
    {WLAN_CFGID_SET_MAX_USER,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "usernum"},
    {WLAN_CFGID_SET_MAX_USER,       0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_usernum"},
    {WLAN_CFGID_TID,                0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_tid"},
    {WLAN_CFGID_HIDE_SSID,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "ssidhide"},
    {WLAN_CFGID_HIDE_SSID,       0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_ssidhide"},
    {WLAN_CFGID_GLOBAL_SHORTGI,  OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "shortgi"},
    {WLAN_CFGID_GLOBAL_SHORTGI,  0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_shortgi"},
    {WLAN_CFGID_SET_TXBEAMFORM, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "txbeamform"}, /* ����txbeamform */
    {WLAN_CFGID_SET_TXBEAMFORM,  0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_txbeamform"},
    {WLAN_CFGID_SET_NOFORWARD,  OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "noforward"},  /* ����noforward */
    {WLAN_CFGID_SET_NOFORWARD,  0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_noforward"},
    {WLAN_CFGID_SET_RTS_THRESHHOLD,  OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "rts_th"},  /* ����rts_threshold */
    {WLAN_CFGID_SET_RTS_THRESHHOLD,  0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_rts_th"},
    {WLAN_CFGID_FRAG_THRESHOLD_REG,  OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "frag_th"},  /* ����frag_threshold */
    {WLAN_CFGID_FRAG_THRESHOLD_REG,  0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_frag_th"},
    {WLAN_CFGID_ASSOC_NUM,  0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_assoc_num"},
    {WLAN_CFGID_NOISE,      0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_noise"},
    {WLAN_CFGID_GET_BW,     0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_bw"},
    {WLAN_CFGID_PMF,        0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_pmf"},
    {WLAN_CFGID_GET_TEMP,   0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_temp"},  /*��ȡRF  ʵʱ�¶� */
    {WLAN_CFGID_GET_BAND,   0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_band"},
    {WLAN_CFGID_GET_BCAST_RATE, 0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_bcast_rate"},
#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
    {WLAN_CFGID_GET_BSD,         0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_bdsteering"},
#endif
    {WAL_IOCTL_PRIV_GET_STA_STAT_INFO,   OAL_IW_PRIV_TYPE_CHAR | 20, OAL_IW_PRIV_TYPE_BYTE | 500, ""},
    {WLAN_CFGID_QUERY_RSSI,         OAL_IW_PRIV_TYPE_CHAR | 20, OAL_IW_PRIV_TYPE_BYTE | 500, "get_sta_rssi"},
#ifdef _PRE_WLAN_11K_STAT
    {WLAN_CFGID_QUERY_DROP_NUM,     OAL_IW_PRIV_TYPE_CHAR | 20, OAL_IW_PRIV_TYPE_BYTE | 500, "get_drop_num"},
    {WLAN_CFGID_QUERY_TX_DELAY,     OAL_IW_PRIV_TYPE_CHAR | 20, OAL_IW_PRIV_TYPE_BYTE | 500, "get_tx_delay"},
#endif
    {WLAN_CFGID_KICK_USER,          OAL_IW_PRIV_TYPE_CHAR | 40, 0, "kick_user"},
    {WAL_IOCTL_PRIV_SET_PARAM_CHAR, OAL_IW_PRIV_TYPE_CHAR | 50, 0, ""},
#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
    {WLAN_CFGID_BSD_CONFIG,         OAL_IW_PRIV_TYPE_CHAR | 50, 0, "bdsteering"},
#endif
#ifdef _PRE_SUPPORT_ACS
    {WLAN_CFGID_ACS_CONFIG,         OAL_IW_PRIV_TYPE_CHAR | 50, 0, "acs"},
#endif
    {WLAN_CFGID_RSSI_LIMIT_CFG,     OAL_IW_PRIV_TYPE_CHAR | 50, 0, "rssi_limit"},
#ifdef _PRE_WLAN_FEATURE_CAR
    {WLAN_CFGID_CAR_CFG,            OAL_IW_PRIV_TYPE_CHAR | 50, 0, "car_cfg"},
#endif
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    {WLAN_CFGID_TCP_ACK_BUF,          OAL_IW_PRIV_TYPE_CHAR | 50, 0,  "tcp_ack_buf"},
#endif
#ifdef _PRE_FEATURE_FAST_AGING
    {WLAN_CFGID_FAST_AGING,          OAL_IW_PRIV_TYPE_CHAR | 50, 0,  "fast_aging"},
    {WLAN_CFGID_GET_FAST_AGING,      0, OAL_IW_PRIV_TYPE_CHAR | 500, "get_fast_aging"},
#endif

    {WLAN_CFGID_ALG_CFG,            OAL_IW_PRIV_TYPE_CHAR | 50, 0, "alg_cfg"},
    {WLAN_CFGID_DSCP_MAP_TO_TID,    OAL_IW_PRIV_TYPE_CHAR | 50, 0, "dscp_map_to_tid"},
    {WLAN_CFGID_CLEAN_DSCP_TID_MAP, OAL_IW_PRIV_TYPE_CHAR | 50, 0, "del_dscp_to_tid"},
    {WLAN_CFGID_SET_VENDOR_IE,      OAL_IW_PRIV_TYPE_CHAR | 50, 0, "set_ie"},

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    {WAL_IOCTL_PRIV_SET_EQUIPMENT_PARAM,      OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, ""},
    {WLAN_CFGID_SET_RFCH,           OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "txch"},
#ifdef _PRE_DEBUG_MODE
    {WLAN_CFGID_SET_RXCH,           OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "rxch"},
#endif
    {WLAN_CFGID_CURRENT_CHANEL,     OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "freq"},
    {WLAN_CFGID_TX_POWER,           OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "txpower"},
    {WLAN_CFGID_BANDWIDTH,          OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "bw"},
    {WLAN_CFGID_SET_RATE,           OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "rate"},
    {WLAN_CFGID_SET_MCS,            OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "mcs"},
    {WLAN_CFGID_SET_MCSAC,          OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "mcsac"},
    {WLAN_CFGID_SET_MCS,            OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "mcs"},
    {WLAN_CFGID_SET_NSS,            OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "nss"},
    {WLAN_CFGID_SEND_CW_SIGNAL,     OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "cfg_cw_signal"},
    {WLAN_CFGID_SET_ALWAYS_TX,      OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "al_tx"},
    {WLAN_CFGID_SET_ALWAYS_RX,      OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "al_rx"},
    {WLAN_CFGID_ADJUST_PPM,         OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "adjust_ppm"},
#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)
    {WLAN_CFGID_AUTO_CALI,          OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "auto_cali"},
#endif
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)
    {WLAN_CFGID_CALI_POWER,         OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "cali_power"},
    {WLAN_CFGID_GET_CALI_POWER,     OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "get_cali_power"},
    {WLAN_CFGID_SET_POLYNOMIAL_PARA,      OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "set_power_param"},
    {WLAN_CFGID_SET_UPC_PARA,       OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "set_upc_param"},
    {WLAN_CFGID_SET_LOAD_MODE,      OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "set_load_mode"},
#endif

#endif
    {WAL_IOCTL_PRIV_GET_PARAM_CHAR,     0, OAL_IW_PRIV_TYPE_CHAR | 500, ""},
    {WLAN_CFGID_GET_BSSID,              0, OAL_IW_PRIV_TYPE_CHAR | 500, "get_bssid"},
    {WLAN_CFGID_GET_KO_VERSION,         0, OAL_IW_PRIV_TYPE_CHAR | 500, "get_version"},
    {WLAN_CFGID_GET_RATE_INFO,          0, OAL_IW_PRIV_TYPE_CHAR | 500, "get_rate_list"},
    {WLAN_CFGID_GET_WPS_IE,             0, OAL_IW_PRIV_TYPE_CHAR | 500, "get_wps_param"},
    {WLAN_CFGID_GET_DSCP_TID_MAP,       0, OAL_IW_PRIV_TYPE_CHAR | 500, "get_tid_of_dscp"},
    {WLAN_CFGID_GET_BG_NOISE,           0, OAL_IW_PRIV_TYPE_CHAR | 500, "get_bgnoise"},
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    {WLAN_CFGID_ADJUST_PPM,             0, OAL_IW_PRIV_TYPE_CHAR | 500, "get_ppm"},
    {WLAN_CFGID_AUTO_CALI,              0, OAL_IW_PRIV_TYPE_CHAR | 500, "get_cali_ready"},
    {WLAN_CFGID_RX_FCS_INFO,            0, OAL_IW_PRIV_TYPE_CHAR | 500, "get_rx_info"},
    {WLAN_CFGID_GET_STA_INFO,           0, OAL_IW_PRIV_TYPE_CHAR | 500, "get_sta_info"},
    {WLAN_CFGID_CHIP_CHECK_SWITCH,      0, OAL_IW_PRIV_TYPE_CHAR | 500, "chip_check"},
#endif
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)
    {WLAN_CFGID_CALI_POWER,             0, OAL_IW_PRIV_TYPE_CHAR | 500, "get_power_param"},
    {WLAN_CFGID_GET_UPC_PARA,           0, OAL_IW_PRIV_TYPE_CHAR | 500, "get_upc_param"},
#endif
    {WAL_IOCTL_PRIV_GET_STRU,       0, OAL_IW_PRIV_TYPE_BYTE | 500, ""},
    {WLAN_CFGID_GET_VAP_CAP,        0, OAL_IW_PRIV_TYPE_BYTE | 500, "get_vap_cap"},
    {WLAN_CFGID_GET_CHAN_LIST,      0, OAL_IW_PRIV_TYPE_BYTE | 500, "get_chan_list"},
    {WLAN_CFGID_GET_PWR_REF,        0, OAL_IW_PRIV_TYPE_BYTE | 500, "get_pwr_ref"},
    {WLAN_CFGID_AMPDU_TX_ON,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "ampdu_on"},
    {WLAN_CFGID_AMSDU_TX_ON,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "amsdu_on"},
    {WLAN_CFGID_AMPDU_TX_ON,            0,  OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_ampdu_on"},
    {WLAN_CFGID_AMSDU_TX_ON,            0,  OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_amsdu_on"},
    {WLAN_CFGID_NEIGHBOR_SCAN,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "start_ap_scan"},
    {WLAN_CFGID_NEIGHBOR_SCAN,          0,  OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_ap_scan_st"},
    {WLAN_CFGID_GET_NEIGHB_NO,          0,  OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_ap_num"},
#ifdef _PRE_WLAN_FEATURE_WDS
    {WLAN_CFGID_GET_WDS_STA_NUM,        0, OAL_IW_PRIV_TYPE_BYTE | 500, "get_wds_sta_num"},
#endif
    {WLAN_CFGID_GET_ANT_RSSI,           0, OAL_IW_PRIV_TYPE_CHAR | 500, "get_ant_rssi"},
    {WLAN_CFGID_BLOCK_STA_NS,         OAL_IW_PRIV_TYPE_CHAR | 40, 0, "block_sta_ns"},
    {WLAN_CFGID_ANT_RSSI_REPORT,         OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "ant_rssi_on"},
#ifdef _PRE_WLAN_FEATURE_DFS
    {WAL_IOCTL_PRIV_GET_DFS_CHN_STAT,   OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "dfs_chn_stat"},
#endif
#endif

};

/*****************************************************************************
  ˽��ioctl�������.
*****************************************************************************/
#ifdef _PRE_WLAN_WEB_CMD_COMM
OAL_STATIC OAL_CONST oal_iw_handler g_ast_iw_priv_handlers[] =
{
    (oal_iw_handler)wal_ioctl_set_param,                /* SIOCWFIRSTPRIV+0 */  /* sub-ioctl set ��� */
    (oal_iw_handler)wal_ioctl_get_param,                /* SIOCWFIRSTPRIV+1 */  /* sub-ioctl get ��� */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+2 */  /* setkey */
    (oal_iw_handler)wal_ioctl_set_wme_params,           /* SIOCWFIRSTPRIV+3 */  /* setwmmparams */
    (oal_iw_handler)wal_ioctl_priv_set_essid,           /* SIOCWFIRSTPRIV+4 */  /* set essid */
    (oal_iw_handler)wal_ioctl_get_wme_params,           /* SIOCWFIRSTPRIV+5 */  /* getwmmparams */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+6 */  /* setmlme */
    (oal_iw_handler)wal_ioctl_get_stru_param,           /* SIOCWFIRSTPRIV+7 */  /* getchaninfo */
    (oal_iw_handler)wal_ioctl_setcountry,               /* SIOCWFIRSTPRIV+8 */  /* setcountry */
    (oal_iw_handler)wal_ioctl_getcountry,               /* SIOCWFIRSTPRIV+9 */  /* getcountry */
    (oal_iw_handler)wal_ioctl_set_hwaddr,               /* SIOCWFIRSTPRIV+10 */  /* addmac */
    (oal_iw_handler)wal_ioctl_get_param_char,           /* SIOCWFIRSTPRIV+11 */  /* getscanresults */
    (oal_iw_handler)wal_ioctl_setmac,                   /* SIOCWFIRSTPRIV+12 */  /* delmac */
    (oal_iw_handler)wal_ioctl_priv_set_mode,            /* SIOCWFIRSTPRIV+13 */  /* setmac */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+14 */  /* getmac */
    (oal_iw_handler)wal_ioctl_set_param_char,           /* SIOCWFIRSTPRIV+15 */  /* setmac */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+16 */  /* chanswitch */
    (oal_iw_handler)wal_ioctl_priv_get_mode,            /* SIOCWFIRSTPRIV+17 */  /* ��ȡģʽ, ��: iwpriv vapN get_mode */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+18 */  /* ����ģʽ, ��: iwpriv vapN mode 11g */
    (oal_iw_handler)wal_ioctl_priv_get_essid,           /* SIOCWFIRSTPRIV+19 */  /* get essid */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+20 */  /* null */
    (oal_iw_handler)wal_ioctl_get_assoc_list,           /* SIOCWFIRSTPRIV+21 */  /* APUTȡ�ù���STA�б� */
    (oal_iw_handler)wal_ioctl_set_mac_filters,          /* SIOCWFIRSTPRIV+22 */  /* APUT����STA���� */
    (oal_iw_handler)wal_ioctl_set_ap_config,            /* SIOCWFIRSTPRIV+23 */  /* ����APUT���� */
    (oal_iw_handler)wal_ioctl_set_ap_sta_disassoc,      /* SIOCWFIRSTPRIV+24 */  /* APUTȥ����STA */
    (oal_iw_handler)wal_ioctl_get_sta_stat_info,        /* SIOCWFIRSTPRIV+25 */  /* getStatistics */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+26 */  /* sendmgmt */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    (oal_iw_handler)wal_ioctl_set_equipment_param,      /* SIOCWFIRSTPRIV+27 */  /* null  */
#else
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+27 */  /* null */
#endif
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+28 */  /* null */
#ifdef _PRE_WLAN_FEATURE_DFS
    (oal_iw_handler)wal_ioctl_get_dfs_chn_stat,         /* SIOCWFIRSTPRIV+29 */  /* null  */
#else
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+29 */  /* null */
 #endif
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT_DEBUG
    (oal_iw_handler)wal_ioctl_process_vendor_test_cmd,   /* SIOCWFIRSTPRIV+30 */  /* sethbrparams */
#else
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+30 */  /* sethbrparams */
#endif
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    (oal_iw_handler)wal_ioctl_process_vendor_req_cmd,   /* SIOCWFIRSTPRIV+31 */  /* setrxtimeout */
#else
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+31 */  /* setrxtimeout */
#endif
};

#else
OAL_STATIC OAL_CONST oal_iw_handler g_ast_iw_priv_handlers[] =
{
    OAL_PTR_NULL,                /* SIOCWFIRSTPRIV+0 */  /* sub-ioctl set ��� */
    OAL_PTR_NULL,                /* SIOCWFIRSTPRIV+1 */  /* sub-ioctl get ��� */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+2 */  /* setkey */
    OAL_PTR_NULL,           /* SIOCWFIRSTPRIV+3 */  /* setwmmparams */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+4 */  /* delkey */
    OAL_PTR_NULL,           /* SIOCWFIRSTPRIV+5 */  /* getwmmparams */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+6 */  /* setmlme */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+7 */  /* getchaninfo */
    OAL_PTR_NULL,               /* SIOCWFIRSTPRIV+8 */  /* setcountry */
    OAL_PTR_NULL,               /* SIOCWFIRSTPRIV+9 */  /* getcountry */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+10 */  /* addmac */
    OAL_PTR_NULL,           /* SIOCWFIRSTPRIV+11 */  /* getscanresults */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+12 */  /* delmac */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+13 */  /* getchanlist */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+14 */  /* setchanlist */
    OAL_PTR_NULL,           /* SIOCWFIRSTPRIV+15 */  /* setmac */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+16 */  /* chanswitch */
    OAL_PTR_NULL,                 /* SIOCWFIRSTPRIV+17 */  /* ��ȡģʽ, ��: iwpriv vapN get_mode */
    OAL_PTR_NULL,                 /* SIOCWFIRSTPRIV+18 */  /* ����ģʽ, ��: iwpriv vapN mode 11g */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+19 */  /* getappiebuf */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+20 */  /* null */
    (oal_iw_handler)wal_ioctl_get_assoc_list,           /* SIOCWFIRSTPRIV+21 */  /* APUTȡ�ù���STA�б� */
    (oal_iw_handler)wal_ioctl_set_mac_filters,          /* SIOCWFIRSTPRIV+22 */  /* APUT����STA���� */
    (oal_iw_handler)wal_ioctl_set_ap_config,            /* SIOCWFIRSTPRIV+23 */  /* ����APUT���� */
    (oal_iw_handler)wal_ioctl_set_ap_sta_disassoc,      /* SIOCWFIRSTPRIV+24 */  /* APUTȥ����STA */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+25 */  /* getStatistics */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+26 */  /* sendmgmt */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    (oal_iw_handler)wal_ioctl_set_equipment_param,      /* SIOCWFIRSTPRIV+27 */  /* null  */
#else
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+27 */  /* null */
#endif
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+28 */  /* null */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+29 */  /* null */
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT_DEBUG
    (oal_iw_handler)wal_ioctl_process_vendor_test_cmd,   /* SIOCWFIRSTPRIV+30 */  /* sethbrparams */
#else
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+30 */  /* sethbrparams */
#endif
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    (oal_iw_handler)wal_ioctl_process_vendor_req_cmd,   /* SIOCWFIRSTPRIV+31 */  /* setrxtimeout */
#else
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+31 */  /* setrxtimeout */
#endif
};
#endif

#ifdef _PRE_WLAN_WEB_CMD_COMM
/*****************************************************************************
  ˽��ioctl����������嶨��
*****************************************************************************/
OAL_STATIC OAL_CONST oal_iw_priv_args_stru g_ast_iw_priv_cfg_args[] =
{
    {WAL_IOCTL_PRIV_SET_MODE,       OAL_IW_PRIV_TYPE_CHAR | 24, 0, "mode"},     /* ����������char, ����Ϊ24 */
    {WAL_IOCTL_PRIV_GET_MODE,       0, OAL_IW_PRIV_TYPE_CHAR | 24, "get_mode"},
    {WAL_IOCTL_PRIV_SETPARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "setparam"},
    {WAL_IOCTL_PRIV_GETPARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
                                    OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "getparam"},
    {WAL_IOCTL_PRIV_SET_HWADDR,                OAL_IW_PRIV_TYPE_CHAR | 24, 0, "sethwaddr"}, /*set hwaddr*/
    {WLAN_CFGID_CHANNEL,                0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_channel"},
    {WLAN_CFGID_CHANNEL,                OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "channel"},
    {WAL_IOCTL_PRIV_SETPARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, ""},
    {WAL_IOCTL_PRIV_SETPARAM,       OAL_IW_PRIV_TYPE_BYTE | OAL_IW_PRIV_SIZE_FIXED | OAL_IW_PRIV_TYPE_ADDR, 0, ""},
    {WAL_IOCTL_PRIV_GETPARAM,       0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "" },
#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
    {WLAN_CFGID_PACKET_CAPTURE_SWITCH, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "pkt_capture"},
#endif
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    {WAL_IOCTL_PRIV_SET_EQUIPMENT_PARAM,        OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, ""},
    {WLAN_CFGID_ADJUST_PPM,                     OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "adjust_ppm"},
#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)
    {WLAN_CFGID_AUTO_CALI,                      OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "auto_cali"},
#endif
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)
    {WLAN_CFGID_GET_CALI_POWER,                 OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "get_cali_power"},
    {WLAN_CFGID_CALI_POWER,                     OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "cali_power"},
    {WLAN_CFGID_SET_POLYNOMIAL_PARA,            OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "set_power_param"},
    {WLAN_CFGID_SET_UPC_PARA,                   OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "set_upc_param"},
    {WLAN_CFGID_SET_LOAD_MODE,                  OAL_IW_PRIV_TYPE_CHAR | 100, OAL_IW_PRIV_TYPE_CHAR | MAX_EQUIPMENT_GET_ARGS_SIZE, "set_load_mode"},
#endif
#endif

    {WAL_IOCTL_PRIV_GET_PARAM_CHAR,     0, OAL_IW_PRIV_TYPE_CHAR | 300, ""},
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    {WLAN_CFGID_CHIP_CHECK_SWITCH,      0, OAL_IW_PRIV_TYPE_CHAR | 300, "chip_check"},
#endif
    {WLAN_CFGID_GET_DIEID,              0, OAL_IW_PRIV_TYPE_CHAR | 300, "get_dieid"},
    {WLAN_CFGID_ADJUST_PPM,             0, OAL_IW_PRIV_TYPE_CHAR | 300, "get_ppm"},
#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)
    {WLAN_CFGID_AUTO_CALI,              0, OAL_IW_PRIV_TYPE_CHAR | 300, "get_cali_ready"},
#endif
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)
    {WLAN_CFGID_CALI_POWER,             0, OAL_IW_PRIV_TYPE_CHAR | 300, "get_power_param"},
#endif
};

/*****************************************************************************
  ˽��ioctl�������.
*****************************************************************************/
OAL_STATIC OAL_CONST oal_iw_handler g_ast_iw_priv_cfg_handlers[] =
{
    (oal_iw_handler)wal_ioctl_set_param,                /* SIOCWFIRSTPRIV+0 */  /* sub-ioctl set ��� */
    (oal_iw_handler)wal_ioctl_get_param,                /* SIOCWFIRSTPRIV+1 */  /* sub-ioctl get ��� */
    OAL_PTR_NULL,                                                    /* SIOCWFIRSTPRIV+2 */  /* setkey */
    OAL_PTR_NULL,                                                   /* SIOCWFIRSTPRIV+3 */  /* setwmmparams */
    OAL_PTR_NULL,                                                    /* SIOCWFIRSTPRIV+4 */  /* delkey */
    OAL_PTR_NULL,                                                    /* SIOCWFIRSTPRIV+5 */  /* getwmmparams */
    OAL_PTR_NULL,                                                    /* SIOCWFIRSTPRIV+6 */  /* setmlme */
    OAL_PTR_NULL,                                                    /* SIOCWFIRSTPRIV+7 */  /* getchaninfo */
    OAL_PTR_NULL,                                                   /* SIOCWFIRSTPRIV+8 */  /* setcountry */
    OAL_PTR_NULL,                                                    /* SIOCWFIRSTPRIV+9 */  /* getcountry */
    (oal_iw_handler)wal_ioctl_set_hwaddr,               /* SIOCWFIRSTPRIV+10 */
    (oal_iw_handler)wal_ioctl_get_param_char,                       /* SIOCWFIRSTPRIV+11 */  /* getscanresults */
    OAL_PTR_NULL,                                                   /* SIOCWFIRSTPRIV+12 */  /* delmac */
    OAL_PTR_NULL,                                                    /* SIOCWFIRSTPRIV+13 */  /* getchanlist */
    OAL_PTR_NULL,                                                   /* SIOCWFIRSTPRIV+14 */  /* setchanlist */
    OAL_PTR_NULL,                                                   /* SIOCWFIRSTPRIV+15 */  /* kickmac */
    OAL_PTR_NULL,                                                   /* SIOCWFIRSTPRIV+16 */  /* chanswitch */
    (oal_iw_handler)wal_ioctl_priv_get_mode,                 /* SIOCWFIRSTPRIV+17 */  /* ��ȡģʽ, ��: iwpriv vapN get_mode */
    (oal_iw_handler)wal_ioctl_priv_set_mode,                 /* SIOCWFIRSTPRIV+18 */  /* ����ģʽ, ��: iwpriv vapN mode 11g */
    OAL_PTR_NULL,                                           /* SIOCWFIRSTPRIV+19 */  /* get essid */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+20 */  /* null */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+21 */  /* APUTȡ�ù���STA�б� */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+22 */  /* APUT����STA���� */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+23 */  /* ����APUT���� */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+24 */  /* APUTȥ����STA */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+25 */  /* getStatistics */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+26 */  /* sendmgmt */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    (oal_iw_handler)wal_ioctl_set_equipment_param,      /* SIOCWFIRSTPRIV+27 */  /* null  */
#endif
};
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_iw_handler_def_stru g_st_iw_cfg_handler_def =
{
    .standard           = OAL_PTR_NULL,
    .num_standard       = 0,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,59))
#ifdef CONFIG_WEXT_PRIV
    .private            = g_ast_iw_priv_cfg_handlers,
    .num_private        = OAL_ARRAY_SIZE(g_ast_iw_priv_cfg_handlers),
    .private_args       = g_ast_iw_priv_cfg_args,
    .num_private_args   = OAL_ARRAY_SIZE(g_ast_iw_priv_cfg_args),
#endif
#else
    .private                = g_ast_iw_priv_cfg_handlers,
    .num_private        = OAL_ARRAY_SIZE(g_ast_iw_priv_cfg_handlers),
    .private_args       = g_ast_iw_priv_cfg_args,
    .num_private_args   = OAL_ARRAY_SIZE(g_ast_iw_priv_cfg_args),
 #endif
    .get_wireless_stats = OAL_PTR_NULL
};
#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
oal_iw_handler_def_stru g_st_iw_cfg_handler_def =
{
    OAL_PTR_NULL,                      /* ��׼ioctl handler */
    0,
    OAL_ARRAY_SIZE(g_ast_iw_priv_cfg_handlers),
    {0, 0},                                 /* �ֽڶ��� */
    OAL_ARRAY_SIZE(g_ast_iw_priv_cfg_args),
    g_ast_iw_priv_cfg_handlers,                 /* ˽��ioctl handler */
    g_ast_iw_priv_cfg_args,
    OAL_PTR_NULL
};
#endif

#endif

/*****************************************************************************
  ��������iw_handler_def����
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_iw_handler_def_stru g_st_iw_handler_def_etc =
{
    .standard           = OAL_PTR_NULL,
    .num_standard       = 0,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,59))
#ifdef CONFIG_WEXT_PRIV
    .private            = g_ast_iw_priv_handlers,
    .num_private        = OAL_ARRAY_SIZE(g_ast_iw_priv_handlers),
    .private_args       = g_ast_iw_priv_args,
    .num_private_args   = OAL_ARRAY_SIZE(g_ast_iw_priv_args),
#endif
#else
    .private            = g_ast_iw_priv_handlers,
    .num_private        = OAL_ARRAY_SIZE(g_ast_iw_priv_handlers),
    .private_args       = g_ast_iw_priv_args,
    .num_private_args   = OAL_ARRAY_SIZE(g_ast_iw_priv_args),
#endif
    .get_wireless_stats = OAL_PTR_NULL
};

#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
oal_iw_handler_def_stru g_st_iw_handler_def_etc =
{
    OAL_PTR_NULL,                      /* ��׼ioctl handler */
    0,
    OAL_ARRAY_SIZE(g_ast_iw_priv_handlers),
    {0, 0},                                 /* �ֽڶ��� */
    OAL_ARRAY_SIZE(g_ast_iw_priv_args),
    g_ast_iw_priv_handlers,                 /* ˽��ioctl handler */
    g_ast_iw_priv_args,
    OAL_PTR_NULL
};
#endif

/*****************************************************************************
  Э��ģʽ�ַ�������
*****************************************************************************/
OAL_CONST wal_ioctl_mode_map_stru g_ast_mode_map_etc[] =
{
    /* legacy */
    {"11a",         WLAN_LEGACY_11A_MODE,       WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11b",         WLAN_LEGACY_11B_MODE,       WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11bg",        WLAN_MIXED_ONE_11G_MODE,    WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11g",         WLAN_MIXED_TWO_11G_MODE,    WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},

    /* 11n */
    {"11na20",      WLAN_HT_MODE,           WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11ng20",      WLAN_HT_MODE,           WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11na40plus",  WLAN_HT_MODE,           WLAN_BAND_5G,   WLAN_BAND_WIDTH_40PLUS},
    {"11na40minus", WLAN_HT_MODE,           WLAN_BAND_5G,   WLAN_BAND_WIDTH_40MINUS},
    {"11ng40plus",  WLAN_HT_MODE,           WLAN_BAND_2G,   WLAN_BAND_WIDTH_40PLUS},
    {"11ng40minus", WLAN_HT_MODE,           WLAN_BAND_2G,   WLAN_BAND_WIDTH_40MINUS},

    /* 11ac */
    {"11ac20",              WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11ac40plus",          WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_40PLUS},
    {"11ac40minus",         WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_40MINUS},
    {"11ac80plusplus",      WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSPLUS},
    {"11ac80plusminus",     WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSMINUS},
    {"11ac80minusplus",     WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSPLUS},
    {"11ac80minusminus",    WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSMINUS},
#ifdef _PRE_WLAN_FEATURE_160M
    {"11ac160plusplusplus",      WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_160PLUSPLUSPLUS},
    {"11ac160plusplusminus",     WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_160PLUSPLUSMINUS},
    {"11ac160plusminusplus",     WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_160PLUSMINUSPLUS},
    {"11ac160plusminusminus",    WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_160PLUSMINUSMINUS},
    {"11ac160minusplusplus",      WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_160MINUSPLUSPLUS},
    {"11ac160minusplusminus",     WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_160MINUSPLUSMINUS},
    {"11ac160minusminusplus",     WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_160MINUSMINUSPLUS},
    {"11ac160minusminusminus",    WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_160MINUSMINUSMINUS},
#endif

    {"11ac2g20",            WLAN_VHT_MODE,  WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11ac2g40plus",        WLAN_VHT_MODE,  WLAN_BAND_2G,   WLAN_BAND_WIDTH_40PLUS},
    {"11ac2g40minus",       WLAN_VHT_MODE,  WLAN_BAND_2G,   WLAN_BAND_WIDTH_40MINUS},
    /* 11n only and 11ac only, ����20M���� */
    {"11nonly2g",           WLAN_HT_ONLY_MODE,   WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11nonly5g",           WLAN_HT_ONLY_MODE,   WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11aconly",            WLAN_VHT_ONLY_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},

/* 1151�����Լ�ONT��ҳ����Э��ģʽʱ����Я��������չ������Ϣ */
    {"11ng40",              WLAN_HT_MODE,   WLAN_BAND_2G,   WLAN_BAND_WIDTH_40M},
    {"11ac2g40",            WLAN_VHT_MODE,  WLAN_BAND_2G,   WLAN_BAND_WIDTH_40M},
    {"11na40",              WLAN_HT_MODE,   WLAN_BAND_5G,   WLAN_BAND_WIDTH_40M},
    {"11ac40",              WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_40M},
    {"11ac80",              WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_80M},

#ifdef _PRE_WLAN_FEATURE_11AX
    /* 11ax */
    {"11ax2g20",         WLAN_HE_MODE,           WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11ax2g40plus",     WLAN_HE_MODE,           WLAN_BAND_2G,   WLAN_BAND_WIDTH_40PLUS},
    {"11ax2g40minus",    WLAN_HE_MODE,           WLAN_BAND_2G,   WLAN_BAND_WIDTH_40MINUS},
    {"11ax5g20",              WLAN_HE_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11ax5g40plus",          WLAN_HE_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_40PLUS},
    {"11ax5g40minus",         WLAN_HE_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_40MINUS},
    {"11ax5g80plusplus",      WLAN_HE_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSPLUS},
    {"11ax5g80plusminus",     WLAN_HE_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSMINUS},
    {"11ax5g80minusplus",     WLAN_HE_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSPLUS},
    {"11ax5g80minusminus",    WLAN_HE_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSMINUS},
#endif

    {OAL_PTR_NULL}
};


/* ע��! ����Ĳ���������Ҫ�� g_dmac_config_set_dscr_param�еĺ���˳���ϸ�һ��! */
OAL_CONST oal_int8   *pauc_tx_dscr_param_name_etc[WAL_DSCR_PARAM_BUTT] =
{
    "pgl",
    "mtpgl",
    "ta",
    "ra",
    "cc",
    "data0",
    "data1",
    "data2",
    "data3",
    "power",
    "shortgi",
    "preamble",
    "rtscts",
    "lsigtxop",
    "smooth",
    "snding",
    "txbf",
    "stbc",
    "rd_ess",
    "dyn_bw",
    "dyn_bw_exist",
    "ch_bw_exist",
    "rate",
    "mcs",
    "mcsac",
    "mcsax",
    "nss",
    "bw",
    "ltf",
    "gi",
    "txchain"
};

OAL_CONST oal_int8   *pauc_tx_pow_param_name[WAL_TX_POW_PARAM_BUTT] =
{
    "rf_reg_ctl",
    "fix_level",
    "mag_level",
    "ctl_level",
    "amend",
    "no_margin",
    "show_log",
    "sar_level",
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    "tas_pwr_ctrl",
    "tas_rssi_measure",
    "tas_ant_switch",
#endif
};

OAL_STATIC OAL_CONST oal_int8   *pauc_tx_dscr_nss_tbl[WLAN_NSS_LIMIT] =
{
    "1",
    "2",
    "3",
    "4"
};

//#ifdef    _PRE_WLAN_CHIP_TEST
OAL_STATIC OAL_CONST oal_int8   *pauc_bw_tbl[WLAN_BAND_ASSEMBLE_AUTO] =
{
    "20",
    "rsv1",
    "rsv2",
    "rsv3",
    "40",
    "d40",
    "rsv6",
    "rsv7",
    "80",
    "d80",
    "rsv10",
    "rsv11",
    "160",
    "d160",
    "rsv14",
    "80_80"
};

OAL_STATIC OAL_CONST oal_int8   *pauc_non_ht_rate_tbl[WLAN_LEGACY_RATE_VALUE_BUTT] =
{
    "1",
    "2",
    "5.5",
    "11",
    "rsv0",
    "rsv1",
    "rsv2",
    "rsv3",
    "48",
    "24",
    "12",
    "6",
    "54",
    "36",
    "18",
    "9"
};
//#endif  /* _PRE_WLAN_CHIP_TEST */

OAL_CONST wal_ioctl_dyn_cali_stru g_ast_dyn_cali_cfg_map[]=
{
    {"realtime_cali_adjust",    MAC_DYN_CALI_CFG_SET_EN_REALTIME_CALI_ADJUST},
    {"2g_dscr_interval",        MAC_DYN_CALI_CFG_SET_2G_DSCR_INT},
    {"5g_dscr_interval",        MAC_DYN_CALI_CFG_SET_5G_DSCR_INT},
    {"chain_interval",          MAC_DYN_CALI_CFG_SET_CHAIN_INT},
    {"pdet_min_th",             MAC_DYN_CALI_CFG_SET_PDET_MIN_TH},
    {"pdet_max_th",             MAC_DYN_CALI_CFG_SET_PDET_MAX_TH},
#ifdef _PRE_WLAN_DPINIT_CALI
    {"get_dpinit_val",          MAC_DYN_CALI_CFG_GET_DPINIT_VAL},
#endif
    {OAL_PTR_NULL}
};


OAL_CONST wal_ioctl_alg_cfg_stru g_ast_alg_cfg_map_etc[] =
{
    {"sch_vi_ctrl_ena",         MAC_ALG_CFG_SCHEDULE_VI_CTRL_ENA},
    {"sch_bebk_minbw_ena",      MAC_ALG_CFG_SCHEDULE_BEBK_MIN_BW_ENA},
    {"sch_mvap_sch_ena",        MAC_ALG_CFG_SCHEDULE_MVAP_SCH_ENA},
    {"sch_vi_sch_ms",           MAC_ALG_CFG_SCHEDULE_VI_SCH_LIMIT},
    {"sch_vo_sch_ms",           MAC_ALG_CFG_SCHEDULE_VO_SCH_LIMIT},
    {"sch_vi_drop_ms",          MAC_ALG_CFG_SCHEDULE_VI_DROP_LIMIT},
    {"sch_vi_ctrl_ms",          MAC_ALG_CFG_SCHEDULE_VI_CTRL_MS},
    {"sch_vi_life_ms",          MAC_ALG_CFG_SCHEDULE_VI_MSDU_LIFE_MS},
    {"sch_vo_life_ms",          MAC_ALG_CFG_SCHEDULE_VO_MSDU_LIFE_MS},
    {"sch_be_life_ms",          MAC_ALG_CFG_SCHEDULE_BE_MSDU_LIFE_MS},
    {"sch_bk_life_ms",          MAC_ALG_CFG_SCHEDULE_BK_MSDU_LIFE_MS},
    {"sch_vi_low_delay",        MAC_ALG_CFG_SCHEDULE_VI_LOW_DELAY_MS},
    {"sch_vi_high_delay",       MAC_ALG_CFG_SCHEDULE_VI_HIGH_DELAY_MS},
    {"sch_cycle_ms",            MAC_ALG_CFG_SCHEDULE_SCH_CYCLE_MS},
    {"sch_ctrl_cycle_ms",       MAC_ALG_CFG_SCHEDULE_TRAFFIC_CTRL_CYCLE},
    {"sch_cir_nvip_kbps",       MAC_ALG_CFG_SCHEDULE_CIR_NVIP_KBPS},
    {"sch_cir_nvip_be",         MAC_ALG_CFG_SCHEDULE_CIR_NVIP_KBPS_BE},
    {"sch_cir_nvip_bk",         MAC_ALG_CFG_SCHEDULE_CIR_NVIP_KBPS_BK},
    {"sch_cir_vip_kbps",        MAC_ALG_CFG_SCHEDULE_CIR_VIP_KBPS},
    {"sch_cir_vip_be",          MAC_ALG_CFG_SCHEDULE_CIR_VIP_KBPS_BE},
    {"sch_cir_vip_bk",          MAC_ALG_CFG_SCHEDULE_CIR_VIP_KBPS_BK},
    {"sch_cir_vap_kbps",        MAC_ALG_CFG_SCHEDULE_CIR_VAP_KBPS},
    {"sch_sm_delay_ms",         MAC_ALG_CFG_SCHEDULE_SM_TRAIN_DELAY},
    {"sch_drop_pkt_limit",      MAC_ALG_CFG_VIDEO_DROP_PKT_LIMIT},
    {"sch_flowctl_ena",         MAC_ALG_CFG_FLOWCTRL_ENABLE_FLAG},
    {"sch_log_start",           MAC_ALG_CFG_SCHEDULE_LOG_START},
    {"sch_log_end",             MAC_ALG_CFG_SCHEDULE_LOG_END},
    {"sch_method",              MAC_ALG_CFG_SCHEDULE_SCH_METHOD},
    {"sch_fix_mode",            MAC_ALG_CFG_SCHEDULE_FIX_SCH_MODE},
    {"sch_vap_prio",            MAC_ALG_CFG_SCHEDULE_VAP_SCH_PRIO},

    {"txbf_switch",             MAC_ALG_CFG_TXBF_MASTER_SWITCH},
    {"txbf_txmode_enb",         MAC_ALG_CFG_TXBF_TXMODE_ENABLE},
    {"txbf_11nbfee_enb",        MAC_ALG_CFG_TXBF_11N_BFEE_ENABLE},
    {"txbf_2g_bfer",            MAC_ALG_CFG_TXBF_2G_BFER_ENABLE},
    {"txbf_2nss_bfer",          MAC_ALG_CFG_TXBF_2NSS_BFER_ENABLE},
    {"txbf_fix_mode",           MAC_ALG_CFG_TXBF_FIX_MODE},
    {"txbf_fix_sound",          MAC_ALG_CFG_TXBF_FIX_SOUNDING},
    {"txbf_probe_int",          MAC_ALG_CFG_TXBF_PROBE_INT},
    {"txbf_rm_worst",           MAC_ALG_CFG_TXBF_REMOVE_WORST},
    {"txbf_stbl_num",           MAC_ALG_CFG_TXBF_STABLE_NUM},
    {"txbf_probe_cnt",          MAC_ALG_CFG_TXBF_PROBE_COUNT},
#if WLAN_TXBF_BFER_LOG_ENABLE
    {"txbf_log_enb",            MAC_ALG_CFG_TXBF_LOG_ENABLE},
    {"txbf_log_sta",            MAC_ALG_CFG_TXBF_RECORD_LOG_START},
    {"txbf_log_out",            MAC_ALG_CFG_TXBF_LOG_OUTPUT},
#endif
    {"ar_enable",               MAC_ALG_CFG_AUTORATE_ENABLE},                   /* ������ر���������Ӧ�㷨: sh hipriv.sh "vap0 alg_cfg ar_enable [1|0]" */
    {"ar_use_lowest",           MAC_ALG_CFG_AUTORATE_USE_LOWEST_RATE},          /* ������ر�ʹ���������: sh hipriv.sh "vap0 alg_cfg ar_use_lowest [1|0]" */
    {"ar_short_num",            MAC_ALG_CFG_AUTORATE_SHORT_STAT_NUM},           /* ���ö���ͳ�Ƶİ���Ŀ:sh hipriv.sh "vap0 alg_cfg ar_short_num [����Ŀ]" */
    {"ar_short_shift",          MAC_ALG_CFG_AUTORATE_SHORT_STAT_SHIFT},         /* ���ö���ͳ�Ƶİ�λ��ֵ:sh hipriv.sh "vap0 alg_cfg ar_short_shift [λ��ֵ]" */
    {"ar_long_num",             MAC_ALG_CFG_AUTORATE_LONG_STAT_NUM},            /* ���ó���ͳ�Ƶİ���Ŀ:sh hipriv.sh "vap0 alg_cfg ar_long_num [����Ŀ]" */
    {"ar_long_shift",           MAC_ALG_CFG_AUTORATE_LONG_STAT_SHIFT},          /* ���ó���ͳ�Ƶİ�λ��ֵ:sh hipriv.sh "vap0 alg_cfg ar_long_shift [λ��ֵ]" */
    {"ar_min_probe_up_no",      MAC_ALG_CFG_AUTORATE_MIN_PROBE_UP_INTVL_PKTNUM},   /* ������С̽������:sh hipriv.sh "vap0 alg_cfg ar_min_probe_no [����Ŀ]" */
    {"ar_min_probe_down_no",    MAC_ALG_CFG_AUTORATE_MIN_PROBE_DOWN_INTVL_PKTNUM},   /* ������С̽������:sh hipriv.sh "vap0 alg_cfg ar_min_probe_no [����Ŀ]" */
    {"ar_max_probe_no",         MAC_ALG_CFG_AUTORATE_MAX_PROBE_INTVL_PKTNUM},   /* �������̽������:sh hipriv.sh "vap0 alg_cfg ar_max_probe_no [����Ŀ]" */
    {"ar_keep_times",           MAC_ALG_CFG_AUTORATE_PROBE_INTVL_KEEP_TIMES},   /* ����̽�������ִ���:sh hipriv.sh "vap0 alg_cfg ar_keep_times [����]" */
    {"ar_delta_ratio",          MAC_ALG_CFG_AUTORATE_DELTA_GOODPUT_RATIO},      /* ����goodputͻ������(ǧ�ֱȣ���300):sh hipriv.sh "vap0 alg_cfg ar_delta_ratio [ǧ�ֱ�]" */
    {"ar_vi_per_limit",         MAC_ALG_CFG_AUTORATE_VI_PROBE_PER_LIMIT},       /* ����vi��per����(ǧ�ֱȣ���300):sh hipriv.sh "vap0 alg_cfg ar_vi_per_limit [ǧ�ֱ�]" */
    {"ar_vo_per_limit",         MAC_ALG_CFG_AUTORATE_VO_PROBE_PER_LIMIT},       /* ����vo��per����(ǧ�ֱȣ���300):sh hipriv.sh "vap0 alg_cfg ar_vo_per_limit [ǧ�ֱ�]" */
    {"ar_ampdu_time",           MAC_ALG_CFG_AUTORATE_AMPDU_DURATION},           /* ����ampdu��durattionֵ:sh hipriv.sh "vap0 alg_cfg ar_ampdu_time [ʱ��ֵ]" */
    {"ar_cont_loss_num",        MAC_ALG_CFG_AUTORATE_MCS0_CONT_LOSS_NUM},       /* ����mcs0�Ĵ���ʧ������:sh hipriv.sh "vap0 alg_cfg ar_cont_loss_num [����Ŀ]" */
    {"ar_11b_diff_rssi",        MAC_ALG_CFG_AUTORATE_UP_PROTOCOL_DIFF_RSSI},    /* ��������11b��rssi����:sh hipriv.sh "vap0 alg_cfg ar_11b_diff_rssi [��ֵ]" */
    {"ar_rts_mode",             MAC_ALG_CFG_AUTORATE_RTS_MODE},                 /* ����rtsģʽ:sh hipriv.sh "vap0 alg_cfg ar_rts_mode [0(������)|1(����)|2(rate[0]��̬RTS, rate[1..3]����RTS)|3(rate[0]����RTS, rate[1..3]����RTS)]" */
    {"ar_legacy_loss",          MAC_ALG_CFG_AUTORATE_LEGACY_1ST_LOSS_RATIO_TH}, /* ����Legacy�װ�����������:sh hipriv.sh "vap0 alg_cfg ar_legacy_loss [��ֵ]" */
    {"ar_ht_vht_loss",          MAC_ALG_CFG_AUTORATE_HT_VHT_1ST_LOSS_RATIO_TH}, /* ����Legacy�װ�����������:sh hipriv.sh "vap0 alg_cfg ar_ht_vht_loss [��ֵ]" */
    {"ar_stat_log_do",          MAC_ALG_CFG_AUTORATE_STAT_LOG_START},           /* ��ʼ����ͳ����־:sh hipriv.sh "vap0 alg_cfg ar_stat_log_do [mac��ַ] [ҵ�����] [����Ŀ]" ��: sh hipriv.sh "vap0 alg_cfg ar_stat_log_do 06:31:04:E3:81:02 1 1000" */
    {"ar_sel_log_do",           MAC_ALG_CFG_AUTORATE_SELECTION_LOG_START},      /* ��ʼ����ѡ����־:sh hipriv.sh "vap0 alg_cfg ar_sel_log_do [mac��ַ] [ҵ�����] [����Ŀ]" ��: sh hipriv.sh "vap0 alg_cfg ar_sel_log_do 06:31:04:E3:81:02 1 200" */
    {"ar_fix_log_do",           MAC_ALG_CFG_AUTORATE_FIX_RATE_LOG_START},       /* ��ʼ�̶�������־:sh hipriv.sh "vap0 alg_cfg ar_fix_log_do [mac��ַ] [tidno] [per����]" ��: sh hipriv.sh "vap0 alg_cfg ar_sel_log_do 06:31:04:E3:81:02 1 200" */
    {"ar_aggr_log_do",          MAC_ALG_CFG_AUTORATE_AGGR_STAT_LOG_START},      /* ��ʼ�ۺ�����Ӧ��־:sh hipriv.sh "vap0 alg_cfg ar_fix_log_do [mac��ַ] [tidno]" ��: sh hipriv.sh "vap0 alg_cfg ar_sel_log_do 06:31:04:E3:81:02 1 " */
    {"ar_st_log_out",           MAC_ALG_CFG_AUTORATE_STAT_LOG_WRITE},           /* ��ӡ����ͳ����־:sh hipriv.sh "vap0 alg_cfg ar_st_log_out 06:31:04:E3:81:02" */
    {"ar_sel_log_out",          MAC_ALG_CFG_AUTORATE_SELECTION_LOG_WRITE},      /* ��ӡ����ѡ����־:sh hipriv.sh "vap0 alg_cfg ar_sel_log_out 06:31:04:E3:81:02" */
    {"ar_fix_log_out",          MAC_ALG_CFG_AUTORATE_FIX_RATE_LOG_WRITE},       /* ��ӡ�̶�������־:sh hipriv.sh "vap0 alg_cfg ar_fix_log_out 06:31:04:E3:81:02" */
    {"ar_aggr_log_out",         MAC_ALG_CFG_AUTORATE_AGGR_STAT_LOG_WRITE},      /* ��ӡ�̶�������־:sh hipriv.sh "vap0 alg_cfg ar_fix_log_out 06:31:04:E3:81:02" */
    {"ar_disp_rateset",         MAC_ALG_CFG_AUTORATE_DISPLAY_RATE_SET},         /* ��ӡ���ʼ���:sh hipriv.sh "vap0 alg_cfg ar_disp_rateset 06:31:04:E3:81:02" */
    {"ar_cfg_fix_rate",         MAC_ALG_CFG_AUTORATE_CONFIG_FIX_RATE},          /* ���ù̶�����:sh hipriv.sh "vap0 alg_cfg ar_cfg_fix_rate 06:31:04:E3:81:02 0" */
    {"ar_disp_rx_rate",         MAC_ALG_CFG_AUTORATE_DISPLAY_RX_RATE},          /* ��ӡ�������ʼ���:sh hipriv.sh "vap0 alg_cfg ar_disp_rx_rate 06:31:04:E3:81:02" */
    {"ar_log_enable",           MAC_ALG_CFG_AUTORATE_LOG_ENABLE},               /* ������ر���������Ӧ��־: sh hipriv.sh "vap0 alg_cfg ar_log_enable [1|0]" */
    {"ar_max_vo_rate",          MAC_ALG_CFG_AUTORATE_VO_RATE_LIMIT},            /* ��������VO����: sh hipriv.sh "vap0 alg_cfg ar_max_vo_rate [����ֵ]" */
    {"ar_fading_per_th",        MAC_ALG_CFG_AUTORATE_JUDGE_FADING_PER_TH},      /* ������˥����per����ֵ: sh hipriv.sh "vap0 alg_cfg ar_fading_per_th [per����ֵ(ǧ����)]"*/
    {"ar_aggr_opt",             MAC_ALG_CFG_AUTORATE_AGGR_OPT},                 /* ���þۺ�����Ӧ����: sh hipriv.sh "vap0 alg_cfg ar_aggr_opt [1|0]"*/
    {"ar_aggr_pb_intvl",        MAC_ALG_CFG_AUTORATE_AGGR_PROBE_INTVL_NUM},     /* ���þۺ�����Ӧ̽����: sh hipriv.sh "vap0 alg_cfg ar_aggr_pb_intvl [̽����]"*/
    {"ar_aggr_st_shift",        MAC_ALG_CFG_AUTORATE_AGGR_STAT_SHIFT},          /* ���þۺ�����Ӧͳ����λֵ: sh hipriv.sh "vap0 alg_cfg ar_aggr_st_shift [ͳ����λֵ]"*/
    {"ar_dbac_aggrtime",        MAC_ALG_CFG_AUTORATE_DBAC_AGGR_TIME},           /* ����DBACģʽ�µ����ۺ�ʱ��: sh hipriv.sh "vap0 alg_cfg ar_dbac_aggrtime [���ۺ�ʱ��(us)]"*/
    {"ar_dbg_vi_status",        MAC_ALG_CFG_AUTORATE_DBG_VI_STATUS},            /* ���õ����õ�VI״̬: sh hipriv.sh "vap0 alg_cfg ar_dbg_vi_status [0/1/2]"*/
    {"ar_dbg_aggr_log",         MAC_ALG_CFG_AUTORATE_DBG_AGGR_LOG},             /* �ۺ�����Ӧlog����: sh hipriv.sh "vap0 alg_cfg ar_dbg_aggr_log [0/1]"*/
    {"ar_aggr_pck_num",         MAC_ALG_CFG_AUTORATE_AGGR_NON_PROBE_PCK_NUM},   /* �������ʱ仯ʱ�����оۺ�̽��ı�����: sh hipriv.sh "vap0 alg_cfg ar_aggr_pck_num [������]"*/
    {"ar_min_aggr_idx",         MAC_ALG_CFG_AUTORATE_AGGR_MIN_AGGR_TIME_IDX},   /* ��С�ۺ�ʱ������: sh hipriv.sh "vap0 alg_cfg ar_aggr_min_idx [����ֵ]"*/
    {"ar_250us_dper_th",        MAC_ALG_CFG_AUTORATE_AGGR_250US_DELTA_PER_TH},  /* ���þۺ�250us���ϵ�deltaPER����: sh hipriv.sh "vap0 alg_cfg ar_250us_dper_th [����ֵ]"*/
    {"ar_max_aggr_num",         MAC_ALG_CFG_AUTORATE_MAX_AGGR_NUM},             /* �������ۺ���Ŀ: sh hipriv.sh "vap0 alg_cfg ar_max_aggr_num [�ۺ���Ŀ]" */
    {"ar_1mpdu_per_th",         MAC_ALG_CFG_AUTORATE_LIMIT_1MPDU_PER_TH},       /* ������ͽ�MCS���ƾۺ�Ϊ1��PER����: sh hipriv.sh "vap0 alg_cfg ar_1mpdu_per_th [per����ֵ(ǧ����)]" */

    {"ar_btcoxe_probe",         MAC_ALG_CFG_AUTORATE_BTCOEX_PROBE_ENABLE},      /* ������رչ���̽�����: sh hipriv.sh "vap0 alg_cfg ar_btcoxe_probe [1|0]" */
    {"ar_btcoxe_aggr",          MAC_ALG_CFG_AUTORATE_BTCOEX_AGGR_ENABLE},       /* ������رչ���ۺϻ���: sh hipriv.sh "vap0 alg_cfg ar_btcoxe_aggr [1|0]" */
    {"ar_coxe_intvl",           MAC_ALG_CFG_AUTORATE_COEX_STAT_INTVL},          /* ���ù���ͳ��ʱ��������: sh hipriv.sh "vap0 alg_cfg ar_coxe_intvl [ͳ������ms]"*/
    {"ar_coxe_low_th",          MAC_ALG_CFG_AUTORATE_COEX_LOW_ABORT_TH},        /* ���ù���abort�ͱ������޲���: sh hipriv.sh "vap0 alg_cfg ar_coxe_low_th [ǧ����]"*/
    {"ar_coxe_high_th",         MAC_ALG_CFG_AUTORATE_COEX_HIGH_ABORT_TH},       /* ���ù���abort�߱������޲���: sh hipriv.sh "vap0 alg_cfg ar_coxe_high_th [ǧ����]"*/
    {"ar_coxe_agrr_th",         MAC_ALG_CFG_AUTORATE_COEX_AGRR_NUM_ONE_TH},     /* ���ù���ۺ���ĿΪ1�����޲���: sh hipriv.sh "vap0 alg_cfg ar_coxe_agrr_th [ǧ����]"*/

    {"ar_dyn_bw_en",            MAC_ALG_CFG_AUTORATE_DYNAMIC_BW_ENABLE},        /* ��̬��������ʹ�ܿ���: sh hipriv.sh "vap0 alg_cfg ar_dyn_bw_en [0/1]" */
    {"ar_thpt_wave_opt",        MAC_ALG_CFG_AUTORATE_THRPT_WAVE_OPT},           /* �����������Ż�����: sh hipriv.sh "vap0 alg_cfg ar_thpt_wave_opt [0/1]" */
    {"ar_gdpt_diff_th",         MAC_ALG_CFG_AUTORATE_GOODPUT_DIFF_TH},          /* �����ж�������������goodput�����������(ǧ����): sh hipriv.sh "vap0 alg_cfg ar_gdpt_diff_th [goodput����������(ǧ����)]" */
    {"ar_per_worse_th",         MAC_ALG_CFG_AUTORATE_PER_WORSE_TH},             /* �����ж�������������PER��������(ǧ����): sh hipriv.sh "vap0 alg_cfg ar_per_worse_th [PER�������(ǧ����)]" */
    {"ar_cts_no_ack_num",       MAC_ALG_CFG_AUTORATE_RX_CTS_NO_BA_NUM},         /* ���÷�RTS�յ�CTS����DATA������BA�ķ�������жϴ�������: sh hipriv.sh "vap0 alg_cfg ar_cts_no_ba_num [����]" */
    {"ar_vo_aggr",              MAL_ALG_CFG_AUTORATE_VOICE_AGGR},               /* �����Ƿ�֧��voiceҵ��ۺ�: sh hipriv.sh "vap0 alg_cfg ar_vo_aggr [0/1]" */
    {"ar_fast_smth_shft",       MAC_ALG_CFG_AUTORATE_FAST_SMOOTH_SHIFT},        /* ���ÿ���ƽ��ͳ�Ƶ�ƽ������ƫ����: sh hipriv.sh "vap0 alg_cfg ar_fast_smth_shft [ƫ����]" (ȡ255��ʾȡ������ƽ��)*/
    {"ar_fast_smth_aggr_num",   MAC_ALG_CFG_AUTORATE_FAST_SMOOTH_AGGR_NUM},     /* ���ÿ���ƽ��ͳ�Ƶ���С�ۺ���Ŀ����: sh hipriv.sh "vap0 alg_cfg ar_fast_smth_aggr_num [��С�ۺ���Ŀ]" */
    {"ar_sgi_punish_per",       MAC_ALG_CFG_AUTORATE_SGI_PUNISH_PER},           /* ����short GI�ͷ���PER����ֵ(ǧ����): sh hipriv.sh "vap0 alg_cfg ar_sgi_punish_per [PER����ֵ(ǧ����)]" */
    {"ar_sgi_punish_num",       MAC_ALG_CFG_AUTORATE_SGI_PUNISH_NUM},           /* ����short GI�ͷ��ĵȴ�̽����Ŀ: sh hipriv.sh "vap0 alg_cfg ar_sgi_punish_num [�ȴ�̽����Ŀ]" */
#ifdef _PRE_WLAN_FEATURE_MWO_DET
    {"ar_fourth_rate",          MAL_ALG_CFG_AUTORATE_LAST_RATE_RANK_INDEX},     /*���õ������ʵȼ�����΢��¯���ڵ�8ms ��������*/
    {"ar_mwo_det_debug",        MAL_ALG_CFG_AUTORATE_MWO_DET_DEBUG},            /*����autorate��΢��¯����㷨��ά����Ϣ�Ĵ�ӡ����*/
    {"ar_mwo_per_log",          MAL_ALG_CFG_AUTORATE_MWO_DET_PER_LOG},          /*����΢��¯���״̬��perͳ�Ƶ�ʱ����Ĭ��ͳ��ʱ��Ϊ3000ms,��λms */
#endif
    {"ar_rxch_agc_opt",         MAC_ALG_CFG_AUTORATE_RXCH_AGC_OPT},             /* ���ý���ͨ��AGC�Ż�ʹ�ܿ���: sh hipriv.sh "vap0 alg_cfg ar_rxch_agc_opt [1/0]" */
    {"ar_rxch_agc_log",         MAC_ALG_CFG_AUTORATE_RXCH_AGC_LOG},             /* ���ý���ͨ��AGC�Ż���־����: sh hipriv.sh "vap0 alg_cfg ar_rxch_agc_log [1/0]" */
    {"ar_weak_rssi_th",         MAC_ALG_CFG_AUTORATE_WEAK_RSSI_TH},             /* ���ý���ͨ��AGC�Ż������ź�RSSI����: sh hipriv.sh "vap0 alg_cfg ar_weak_rssi_th [RSSI����ֵ]" (����: RSSIΪ-90dBm, �����ֵΪ90) */
    {"ar_rxch_stat_period",     MAC_ALG_CFG_AUTORATE_RXCH_STAT_PERIOD},         /* ���ý���ͨ����ͳ������(���������): sh hipriv.sh "vap0 alg_cfg ar_weak_rssi_th [������Ŀ]" */
    {"ar_rts_one_tcp_dbg",      MAC_ALG_CFG_AUTORATE_RTS_ONE_TCP_DBG},          /* ����RTS��Ե��û�TCP�Ż��ĵ��Կ���: sh hipriv.sh "vap0 alg_cfg ar_rts_one_tcp_dbg [1/0]" */
    {"ar_scan_user_opt",        MAC_ALG_CFG_AUTORATE_SCAN_USER_OPT},            /* �������ɨ���û����Ż�: sh hipriv.sh "vap0 alg_cfg ar_scan_user_opt [1/0]" */
    {"ar_max_tx_cnt",           MAC_ALG_CFG_AUTORATE_MAX_TX_COUNT},             /* ����ÿ�����ʵȼ�����������: sh hipriv.sh "vap0 alg_cfg ar_max_tx_cnt [�������]" */
    {"ar_80M_40M_switch",       MAC_ALG_CFG_AUTORATE_80M_40M_SWITCH},           /* �����Ƿ���80M ��40M�Ŀ���  sh hipriv.sh "vap0 alg_cfg ar_80M_40M_switch [1/0]" */
    {"ar_40M_switch_thr",       MAC_ALG_CFG_AUTORATE_40M_SWITCH_THR},           /* ����80M ��40M mcs����  sh hipriv.sh "vap0 alg_cfg ar_40M_switch_thr [mcs]" */
    {"ar_collision_det",        MAC_ALG_CFG_AUTORATE_COLLISION_DET_EN},
    {"sm_train_num",            MAC_ALG_CFG_SMARTANT_TRAINING_PACKET_NUMBER},
    {"sm_change_ant",           MAC_ALG_CFG_SMARTANT_CHANGE_ANT},
    {"sm_enable",               MAC_ALG_CFG_SMARTANT_ENABLE},
    {"sm_certain_ant",          MAC_ALG_CFG_SMARTANT_CERTAIN_ANT},
    {"sm_start",                MAC_ALG_CFG_SMARTANT_START_TRAIN},
    {"sm_train_packet",         MAC_ALG_CFG_SMARTANT_SET_TRAINING_PACKET_NUMBER},
    {"sm_min_packet",           MAC_ALG_CFG_SMARTANT_SET_LEAST_PACKET_NUMBER},
    {"sm_ant_interval",         MAC_ALG_CFG_SMARTANT_SET_ANT_CHANGE_INTERVAL},
    {"sm_user_interval",        MAC_ALG_CFG_SMARTANT_SET_USER_CHANGE_INTERVAL},
    {"sm_max_period",           MAC_ALG_CFG_SMARTANT_SET_PERIOD_MAX_FACTOR},
    {"sm_change_freq",          MAC_ALG_CFG_SMARTANT_SET_ANT_CHANGE_FREQ},
    {"sm_change_th",            MAC_ALG_CFG_SMARTANT_SET_ANT_CHANGE_THRESHOLD},

    {"anti_inf_imm_en",         MAC_ALG_CFG_ANTI_INTF_IMM_ENABLE},      /* ������������non-directʹ��: sh hipriv.sh "vap0 alg_cfg anti_inf_imm_en 0|1" */
    {"anti_inf_unlock_en",      MAC_ALG_CFG_ANTI_INTF_UNLOCK_ENABLE},   /* ������������dynamic unlockʹ��: sh hipriv.sh "vap0 alg_cfg anti_inf_unlock_en 0|1" */
    {"anti_inf_stat_time",      MAC_ALG_CFG_ANTI_INTF_RSSI_STAT_CYCLE}, /* ������������rssiͳ������: sh hipriv.sh "vap0 anti_inf_stat_time [time]" */
    {"anti_inf_off_time",       MAC_ALG_CFG_ANTI_INTF_UNLOCK_CYCLE},    /* ������������unlock�ر�����: sh hipriv.sh "vap0 anti_inf_off_time [time]" */
    {"anti_inf_off_dur",        MAC_ALG_CFG_ANTI_INTF_UNLOCK_DUR_TIME}, /* ������������unlock�رճ���ʱ��: sh hipriv.sh "vap0 anti_inf_off_dur [time]" */
    {"anti_inf_nav_en",         MAC_ALG_CFG_ANTI_INTF_NAV_IMM_ENABLE},  /* ������nav����ʹ��: sh hipriv.sh "vap0 alg_cfg anti_inf_nav_en 0|1" */
    {"anti_inf_gd_th",          MAC_ALG_CFG_ANTI_INTF_GOODPUT_FALL_TH}, /* ����������goodput�½�����: sh hipriv.sh "vap0 alg_cfg anti_inf_gd_th [num]" */
    {"anti_inf_keep_max",       MAC_ALG_CFG_ANTI_INTF_KEEP_CYC_MAX_NUM},/* ����������̽�Ᵽ�����������: sh hipriv.sh "vap0 alg_cfg anti_inf_keep_max [num]" */
    {"anti_inf_keep_min",       MAC_ALG_CFG_ANTI_INTF_KEEP_CYC_MIN_NUM},/* ����������̽�Ᵽ�����������: sh hipriv.sh "vap0 alg_cfg anti_inf_keep_min [num]" */
    {"anti_inf_per_pro_en",     MAC_ALG_CFG_ANTI_INTF_PER_PROBE_EN},    /* �����������Ƿ�ʹ��tx time̽��: sh hipriv.sh "vap0 anti_inf_tx_pro_en 0|1" */
    {"anti_inf_txtime_th",      MAC_ALG_CFG_ANTI_INTF_TX_TIME_FALL_TH}, /* tx time�½�����: sh hipriv.sh "vap0 alg_cfg anti_inf_txtime_th [val]"*/
    {"anti_inf_per_th",         MAC_ALG_CFG_ANTI_INTF_PER_FALL_TH},     /* per�½�����: sh hipriv.sh "vap0 alg_cfg anti_inf_per_th [val]"*/
    {"anti_inf_gd_jitter_th",   MAC_ALG_CFG_ANTI_INTF_GOODPUT_JITTER_TH},/* goodput��������: sh hipriv.sh "vap0 alg_cfg anti_inf_gd_jitter_th [val]"*/
    {"anti_inf_debug_mode",     MAC_ALG_CFG_ANTI_INTF_DEBUG_MODE},      /* ����������debug�Ĵ�ӡ��Ϣ: sh hipriv.sh "vap0 alg_cfg anti_inf_debug_mode 0|1|2" */

    {"intf_det_cycle",          MAC_ALG_CFG_INTF_DET_CYCLE},            /* ���ø��ż������(ms):sh hipriv.sh "vap0 alg_cfg intf_det_cycle [val]" */
    {"intf_det_mode",           MAC_ALG_CFG_INTF_DET_MODE},             /* ���ø��ż��ģʽ(ͬƵ��/��Ƶ��Ƶ��/����):sh hipriv.sh "vap0 alg_cfg intf_det_mode 0|1" */
    {"intf_det_debug",          MAC_ALG_CFG_INTF_DET_DEBUG},            /* ���ø��ż��debugģʽ(ÿ��bit��ʾһ��):sh hipriv.sh "vap0 alg_cfg intf_det_debug 0|1" */
    {"intf_det_cothr_sta",      MAC_ALG_CFG_INTF_DET_COCH_THR_STA},     /* ���ø��ż��sta��ֵ(ǧ��֮x):sh hipriv.sh "vap0 alg_cfg intf_det_cothr_sta [val]" */
    {"intf_det_nointf_thr_sta", MAC_ALG_CFG_INTF_DET_COCH_NOINTF_STA},  /* ���ø��ż��sta�޸�����ֵ(ǧ��֮x):sh hipriv.sh "vap0 alg_cfg intf_det_nointf_thr_sta [val]" */
    {"intf_det_cothr_udp",      MAC_ALG_CFG_INTF_DET_COCH_THR_UDP},     /* ���ø��ż��ap udp��ֵ(ǧ��֮x):sh hipriv.sh "vap0 alg_cfg intf_det_cothr_udp [val]" */
    {"intf_det_cothr_tcp",      MAC_ALG_CFG_INTF_DET_COCH_THR_TCP},     /* ���ø��ż��ap tcp��ֵ(ǧ��֮x):sh hipriv.sh "vap0 alg_cfg intf_det_cothr_tcp [val]" */
    {"intf_det_adjscan_cyc",    MAC_ALG_CFG_INTF_DET_ADJCH_SCAN_CYC},   /* ���ø��ż����Ƶ����ɨ������:sh hipriv.sh "vap0 alg_cfg intf_det_adjscan_cyc [val]" */
    {"intf_det_adjratio_thr",   MAC_ALG_CFG_INTF_DET_ADJRATIO_THR},     /* ���ø��ż����Ƶ��Ƶ���ŷ�æ����ֵ(ǧ��֮x):sh hipriv.sh "vap0 alg_cfg intf_det_adjratio_thr [val]" */
    {"intf_det_sync_th",        MAC_ALG_CFG_INTF_DET_SYNC_THR},         /* ���ø��ż����Ƶ��Ƶ����sync error��ֵ(ǧ��֮x):sh hipriv.sh "vap0 alg_cfg intf_det_sync_th [val]"*/
    {"intf_det_ave_rssi",       MAC_ALG_CFG_INTF_DET_AVE_RSSI},         /* ���ø��ż����Ƶ��Ƶ����ƽ��rssi��ֵ(ǧ��֮x):sh hipriv.sh "vap0 alg_cfg intf_det_ave_rssi [val]"*/
    {"intf_det_no_adjratio_th", MAC_ALG_CFG_INTF_DET_NO_ADJRATIO_TH},   /* ���ø��ż�����/��Ƶ���ŷ�æ����ֵ(ǧ��֮x):sh hipriv.sh "vap0 alg_cfg intf_det_no_adjratio_th [val]" */
    {"intf_det_no_adjcyc_th",   MAC_ALG_CFG_INTF_DET_NO_ADJCYC_TH},     /* ���ø��ż�����/��Ƶ���ż�����ֵ:sh hipriv.sh "vap0 alg_cfg intf_det_no_adjcyc_th [val]" */
    {"intf_det_collision_th",   MAC_ALG_CFG_INTF_DET_COLLISION_TH},     /* ���ø��ż�����/��Ƶ���ż�����ֵ:sh hipriv.sh "vap0 alg_cfg intf_det_collision_th [val] */
#ifdef _PRE_WLAN_PRODUCT_1151V200
    {"intf_det_ch_busy_log",    MAC_ALG_CFG_INTF_DET_CH_BUSY_LOG},      /* ���ø��ż�����ŵ���æ�ȴ�ӡ����:sh hipriv.sh "Hisilicon0 alg_cfg intf_det_ch_busy_log 0/1" */
#endif
    {"neg_det_noprobe_th",      MAC_ALG_CFG_NEG_DET_NONPROBE_TH},       /* ���ü�⵽�������̽����ֵ:sh hipriv.sh "vap0 alg_cfg neg_det_noprobe_th [val]" */
    {"intf_det_stat_log_do",    MAC_ALG_CFG_INTF_DET_STAT_LOG_START},   /* ��ʼͳ����־:sh hipriv.sh "vap0 alg_intf_det_log intf_det_stat_log_do [val]" */
    {"intf_det_stat_log_out",   MAC_ALG_CFG_INTF_DET_STAT_LOG_WRITE},   /* ��ӡͳ����־:sh hipriv.sh "vap0 alg_intf_det_log intf_det_stat_log_out" */

    {"edca_opt_en_ap",          MAC_ALG_CFG_EDCA_OPT_AP_EN_MODE},       /* apģʽ��edca�Ż�ʹ��ģʽ: sh hipriv.sh "vap0 alg_cfg edca_opt_en_ap 0|1|2" */
    {"edca_opt_en_sta",         MAC_ALG_CFG_EDCA_OPT_STA_EN},           /* staģʽ��edca�Ż�ʹ��ģʽ: sh hipriv.sh "vap0 alg_cfg edca_opt_en_sta 0|1" */
    {"txop_limit_en_sta",       MAC_ALG_CFG_TXOP_LIMIT_STA_EN},         /* staģʽ��edca txop limit�Ż�ʹ��ģʽ: sh hipriv.sh "vap0 alg_cfg txop_limit_en_sta 0|1" */
    {"edca_opt_sta_weight",     MAC_ALG_CFG_EDCA_OPT_STA_WEIGHT},       /* staģʽ��edca�Ż���weightingϵ��: sh hipriv.sh "vap0 alg_cfg edca_opt_sta_weight 0~3"*/
    {"edca_opt_debug_mode",     MAC_ALG_CFG_EDCA_OPT_DEBUG_MODE},       /* �Ƿ��ӡ�����Ϣ�������ڱ��ذ汾���� */
    {"edca_opt_one_tcp_opt",    MAC_ALG_CFG_EDCA_ONE_BE_TCP_OPT},       /* ��Ե��û�BE TCPҵ����Ż�����: sh hipriv.sh "vap0 alg_cfg edca_opt_one_tcp_opt 0|1" */
    {"edca_opt_one_tcp_dbg",    MAC_ALG_CFG_EDCA_ONE_BE_TCP_DBG},       /* ��Ե��û�BE TCPҵ��ĵ��Կ���: sh hipriv.sh "vap0 alg_cfg edca_opt_one_tcp_dbg 0|1" */
    {"edca_opt_one_tcp_th_no_intf",  MAC_ALG_CFG_EDCA_ONE_BE_TCP_TH_NO_INTF},    /* ��Ե��û�BE TCPҵ���EDCA�����޸���ѡ����ֵ: sh hipriv.sh "vap0 alg_cfg edca_opt_one_tcp_th_no_intf [val]" */
    {"edca_opt_one_tcp_th_intf",     MAC_ALG_CFG_EDCA_ONE_BE_TCP_TH_INTF},       /* ��Ե��û�BE TCPҵ���EDCA��������ѡ����ֵ: sh hipriv.sh "vap0 alg_cfg edca_opt_one_tcp_th_intf [val]" */

    {"cca_opt_alg_en_mode",         MAC_ALG_CFG_CCA_OPT_ALG_EN_MODE},           /* CCA�Ż�����ʹ��: sh hipriv.sh "vap0 alg_cfg cca_opt_alg_en_mode 0|1" */
    {"cca_opt_debug_mode",          MAC_ALG_CFG_CCA_OPT_DEBUG_MODE},            /* CCA�Ż�DEBUGģʽ����: sh hipriv.sh "vap0 alg_cfg cca_opt_debug_mode 0|1" */
    {"cca_opt_set_cca_th_debug",    MAC_ALG_CFG_CCA_OPT_SET_CCA_TH_DEBUG},      /* CCA�Ż��ŵ�ɨ���ʱ��(ms):sh hipriv.sh "vap0 alg_cfg cca_opt_set_sync_err_th [time]" */
    {"cca_opt_log",                 MAC_ALG_CFG_CCA_OPT_LOG},                   /* CCA log���� sh hipriv.sh "vap0 alg_cfg cca_opt_log 0|1"*/

    {"tpc_mode",                MAC_ALG_CFG_TPC_MODE},                          /* ����TPC����ģʽ */
    {"tpc_dbg",                 MAC_ALG_CFG_TPC_DEBUG},                         /* ����TPC��debug���� */
    {"tpc_log",                 MAC_ALG_CFG_TPC_LOG},                           /* ����TPC��log����:sh hipriv.sh "vap0 alg_cfg tpc_log 1 */
    {"tpc_stat_log_do",         MAC_ALG_CFG_TPC_STAT_LOG_START},                /* ��ʼ����ͳ����־:sh hipriv.sh "vap0 alg_tpc_log tpc_stat_log_do [mac��ַ] [ҵ�����] [����Ŀ]" ��: sh hipriv.sh "vap0 alg_tpc_log tpc_stat_log_do 06:31:04:E3:81:02 1 1000" */
    {"tpc_stat_log_out",        MAC_ALG_CFG_TPC_STAT_LOG_WRITE},                /* ��ӡ����ͳ����־:sh hipriv.sh "vap0 alg_tpc_log tpc_stat_log_out 06:31:04:E3:81:02" */
    {"tpc_pkt_log_do",          MAC_ALG_CFG_TPC_PER_PKT_LOG_START},             /* ��ʼÿ��ͳ����־:sh hipriv.sh "vap0 alg_tpc_log tpc_pkt_log_do [mac��ַ] [ҵ�����] [����Ŀ]" ��: sh hipriv.sh "vap0 alg_tpc_log tpc_pkt_log_do 06:31:04:E3:81:02 1 1000" */
    {"tpc_get_frame_pow",       MAC_ALG_CFG_TPC_GET_FRAME_POW},                 /* ��ȡ����֡����:sh hipriv.sh "vap0 alg_tpc_log tpc_get_frame_pow beacon_pow" */
    {"tpc_reset_stat",          MAC_ALG_CFG_TPC_RESET_STAT},                    /*�ͷ�ͳ���ڴ�*/
    {"tpc_reset_pkt",           MAC_ALG_CFG_TPC_RESET_PKT},                     /*�ͷ�ÿ���ڴ�*/
    {"tpc_over_temp_th",        MAC_ALG_CFG_TPC_OVER_TMP_TH},                   /* TPC�������� */
    {"tpc_dpd_enable_rate",     MAC_ALG_CFG_TPC_DPD_ENABLE_RATE},               /* ����DPD��Ч������INDEX */
    {"tpc_target_rate_11b",     MAC_ALG_CFG_TPC_TARGET_RATE_11B},               /* 11bĿ���������� */
    {"tpc_target_rate_11ag",    MAC_ALG_CFG_TPC_TARGET_RATE_11AG},              /* 11agĿ���������� */
    {"tpc_target_rate_11n20",   MAC_ALG_CFG_TPC_TARGET_RATE_HT40},              /* 11n20Ŀ���������� */
    {"tpc_target_rate_11n40",   MAC_ALG_CFG_TPC_TARGET_RATE_HT40},              /* 11n40Ŀ���������� */
    {"tpc_target_rate_11ac20",  MAC_ALG_CFG_TPC_TARGET_RATE_VHT20},             /* 11ac20Ŀ���������� */
    {"tpc_target_rate_11ac40",  MAC_ALG_CFG_TPC_TARGET_RATE_VHT40},             /* 11ac40Ŀ���������� */
    {"tpc_target_rate_11ac80",  MAC_ALG_CFG_TPC_TARGET_RATE_VHT80},             /* 11ac80Ŀ���������� */

#ifdef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
    {"traffic_ctl_enable",      MAC_ALG_CFG_TRAFFIC_CTL_ENABLE},
    {"traffic_ctl_timeout",     MAC_ALG_CFG_TRAFFIC_CTL_TIMEOUT},
    {"traffic_ctl_min_thres",   MAC_ALG_CFG_TRAFFIC_CTL_MIN_THRESHOLD},
    {"traffic_ctl_log_debug",   MAC_ALG_CFG_TRAFFIC_CTL_LOG_DEBUG},
    {"traffic_ctl_buf_thres",   MAC_ALG_CFG_TRAFFIC_CTL_BUF_THRESHOLD},
    {"traffic_ctl_buf_adj_enb", MAC_ALG_CFG_TRAFFIC_CTL_BUF_ADJ_ENABLE},
    {"traffic_ctl_buf_adj_num", MAC_ALG_CFG_TRAFFIC_CTL_BUF_ADJ_NUM},
    {"traffic_ctl_buf_adj_cyc", MAC_ALG_CFG_TRAFFIC_CTL_BUF_ADJ_CYCLE},
    {"traffic_ctl_rx_rst_enb",  MAC_ALG_CFG_TRAFFIC_CTL_RX_RESTORE_ENABLE},
    {"traffic_ctl_rx_rst_num",  MAC_ALG_CFG_TRAFFIC_RX_RESTORE_NUM},
    {"traffic_ctl_rx_rst_thres",  MAC_ALG_CFG_TRAFFIC_RX_RESTORE_THRESHOLD},

    {"rx_dscr_ctl_enable",      MAC_ALG_CFG_RX_DSCR_CTL_ENABLE},
    {"rx_dscr_ctl_log_debug",   MAC_ALG_CFG_RX_DSCR_CTL_LOG_DEBUG},
#endif


#ifdef _PRE_WLAN_FEATURE_MWO_DET
    {"mwo_det_enable",          MAC_ALG_CFG_MWO_DET_ENABLE},                    /* ΢��¯���ʹ���ź�*/
    {"mwo_det_end_rssi_th",     MAC_ALG_CFG_MWO_DET_END_RSSI_TH},               /* ֹͣ΢��¯�źŷ���ʱ���ʱ�����߿ڹ������ޣ���λdBm��*/
    {"mwo_det_start_rssi_th",   MAC_ALG_CFG_MWO_DET_START_RSSI_TH},             /* ����΢��¯�źŷ���ʱ���ʱ�����߿ڹ������ޣ���λdBm��*/
    {"mwo_det_debug",           MAC_ALG_CFG_MWO_DET_DEBUG},                     /*������������anti_intf_1thr,c2 ���ڱ�����ѡ���޸������ʣ�c2С�ڴ�����ѡ���и�������*/
#endif

    {OAL_PTR_NULL}
};

OAL_CONST wal_ioctl_tlv_stru   g_ast_set_tlv_table[] =
{
    /* cmd: wlan0 set_tlv xx xx */
    {"tx_pkts_stat",    WLAN_CFGID_SET_DEVICE_PKT_STAT},
    {"mem_free_stat",   WLAN_CFGID_SET_DEVICE_MEM_FREE_STAT},
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    {"auto_freq",       WLAN_CFGID_SET_DEVICE_FREQ},
#endif

    {"set_adc_dac_freq",  WLAN_CFGID_SET_ADC_DAC_FREQ},             /* ����ADC DACƵ�� */

    {"rx_ampdu_num",  WLAN_CFGID_SET_ADDBA_RSP_BUFFER},
    /* cmd: wlan0 set_val xx */
    {"tx_ampdu_type",   WLAN_CFGID_SET_TX_AMPDU_TYPE},   /* ���þۺ����͵Ŀ���*/
    {"tx_ampdu_amsdu",  WLAN_CFGID_AMSDU_AMPDU_SWITCH},  /* ����tx amsdu ampdu���ϾۺϹ��ܵĿ���*/
    {"rx_ampdu_amsdu",  WLAN_CFGID_SET_RX_AMPDU_AMSDU},  /* ����rx ampdu amsdu ���ϾۺϹ��ܵĿ���*/

    {"sk_pacing_shift", WLAN_CFGID_SET_SK_PACING_SHIFT},

    {"trx_stat_log_en", WLAN_CFGID_SET_TRX_STAT_LOG}, /* ����tcp ack����ʱ������ͳ��ά�⿪�أ�����ģ��ɲο� */
    {"mimo_blacklist",  WLAN_CFGID_MIMO_BLACKLIST},   /* ����̽��MIMO���������ƿ��� */
#ifdef _PRE_WLAN_FEATURE_DFS_ENABLE
    /* cmd: wlan0 set_val dfs_debug 0|1 */
    {"dfs_debug",  WLAN_CFGID_SET_DFS_MODE},             /* ����dfs�Ƿ�Ϊ�����ģʽ�Ŀ���*/
#endif

    {OAL_PTR_NULL}
};



OAL_CONST wal_dfs_domain_entry_stru g_ast_dfs_domain_table_etc[] =
{
    {"AE", MAC_DFS_DOMAIN_ETSI},
    {"AL", MAC_DFS_DOMAIN_NULL},
    {"AM", MAC_DFS_DOMAIN_ETSI},
    {"AN", MAC_DFS_DOMAIN_ETSI},
    {"AR", MAC_DFS_DOMAIN_FCC},
    {"AT", MAC_DFS_DOMAIN_ETSI},
    {"AU", MAC_DFS_DOMAIN_FCC},
    {"AZ", MAC_DFS_DOMAIN_ETSI},
    {"BA", MAC_DFS_DOMAIN_ETSI},
    {"BE", MAC_DFS_DOMAIN_ETSI},
    {"BG", MAC_DFS_DOMAIN_ETSI},
    {"BH", MAC_DFS_DOMAIN_ETSI},
    {"BL", MAC_DFS_DOMAIN_NULL},
    {"BN", MAC_DFS_DOMAIN_ETSI},
    {"BO", MAC_DFS_DOMAIN_ETSI},
    {"BR", MAC_DFS_DOMAIN_FCC},
    {"BY", MAC_DFS_DOMAIN_ETSI},
    {"BZ", MAC_DFS_DOMAIN_ETSI},
    {"CA", MAC_DFS_DOMAIN_FCC},
    {"CH", MAC_DFS_DOMAIN_ETSI},
    {"CL", MAC_DFS_DOMAIN_NULL},
    {"CN", MAC_DFS_DOMAIN_CN},
    {"CO", MAC_DFS_DOMAIN_FCC},
    {"CR", MAC_DFS_DOMAIN_FCC},
    {"CS", MAC_DFS_DOMAIN_ETSI},
    {"CY", MAC_DFS_DOMAIN_ETSI},
    {"CZ", MAC_DFS_DOMAIN_ETSI},
    {"DE", MAC_DFS_DOMAIN_ETSI},
    {"DK", MAC_DFS_DOMAIN_ETSI},
    {"DO", MAC_DFS_DOMAIN_FCC},
    {"DZ", MAC_DFS_DOMAIN_NULL},
    {"EC", MAC_DFS_DOMAIN_FCC},
    {"EE", MAC_DFS_DOMAIN_ETSI},
    {"EG", MAC_DFS_DOMAIN_ETSI},
    {"ES", MAC_DFS_DOMAIN_ETSI},
    {"FI", MAC_DFS_DOMAIN_ETSI},
    {"FR", MAC_DFS_DOMAIN_ETSI},
    {"GB", MAC_DFS_DOMAIN_ETSI},
    {"GE", MAC_DFS_DOMAIN_ETSI},
    {"GR", MAC_DFS_DOMAIN_ETSI},
    {"GT", MAC_DFS_DOMAIN_FCC},
    {"HK", MAC_DFS_DOMAIN_FCC},
    {"HN", MAC_DFS_DOMAIN_FCC},
    {"HR", MAC_DFS_DOMAIN_ETSI},
    {"HU", MAC_DFS_DOMAIN_ETSI},
    {"ID", MAC_DFS_DOMAIN_NULL},
    {"IE", MAC_DFS_DOMAIN_ETSI},
    {"IL", MAC_DFS_DOMAIN_ETSI},
    {"IN", MAC_DFS_DOMAIN_NULL},
    {"IQ", MAC_DFS_DOMAIN_NULL},
    {"IR", MAC_DFS_DOMAIN_NULL},
    {"IS", MAC_DFS_DOMAIN_ETSI},
    {"IT", MAC_DFS_DOMAIN_ETSI},
    {"JM", MAC_DFS_DOMAIN_FCC},
    {"JO", MAC_DFS_DOMAIN_ETSI},
    {"JP", MAC_DFS_DOMAIN_MKK},
    {"KP", MAC_DFS_DOMAIN_NULL},
    {"KR", MAC_DFS_DOMAIN_KOREA},
    {"KW", MAC_DFS_DOMAIN_ETSI},
    {"KZ", MAC_DFS_DOMAIN_NULL},
    {"LB", MAC_DFS_DOMAIN_NULL},
    {"LI", MAC_DFS_DOMAIN_ETSI},
    {"LK", MAC_DFS_DOMAIN_FCC},
    {"LT", MAC_DFS_DOMAIN_ETSI},
    {"LU", MAC_DFS_DOMAIN_ETSI},
    {"LV", MAC_DFS_DOMAIN_ETSI},
    {"MA", MAC_DFS_DOMAIN_NULL},
    {"MC", MAC_DFS_DOMAIN_ETSI},
    {"MK", MAC_DFS_DOMAIN_ETSI},
    {"MO", MAC_DFS_DOMAIN_FCC},
    {"MT", MAC_DFS_DOMAIN_ETSI},
    {"MX", MAC_DFS_DOMAIN_FCC},
    {"MY", MAC_DFS_DOMAIN_FCC},
    {"NG", MAC_DFS_DOMAIN_NULL},
    {"NL", MAC_DFS_DOMAIN_ETSI},
    {"NO", MAC_DFS_DOMAIN_ETSI},
    {"NP", MAC_DFS_DOMAIN_NULL},
    {"NZ", MAC_DFS_DOMAIN_FCC},
    {"OM", MAC_DFS_DOMAIN_FCC},
    {"PA", MAC_DFS_DOMAIN_FCC},
    {"PE", MAC_DFS_DOMAIN_FCC},
    {"PG", MAC_DFS_DOMAIN_FCC},
    {"PH", MAC_DFS_DOMAIN_FCC},
    {"PK", MAC_DFS_DOMAIN_NULL},
    {"PL", MAC_DFS_DOMAIN_ETSI},
    {"PR", MAC_DFS_DOMAIN_FCC},
    {"PT", MAC_DFS_DOMAIN_ETSI},
    {"QA", MAC_DFS_DOMAIN_NULL},
    {"RO", MAC_DFS_DOMAIN_ETSI},
    {"RU", MAC_DFS_DOMAIN_FCC},
    {"SA", MAC_DFS_DOMAIN_FCC},
    {"SE", MAC_DFS_DOMAIN_ETSI},
    {"SG", MAC_DFS_DOMAIN_NULL},
    {"SI", MAC_DFS_DOMAIN_ETSI},
    {"SK", MAC_DFS_DOMAIN_ETSI},
    {"SV", MAC_DFS_DOMAIN_FCC},
    {"SY", MAC_DFS_DOMAIN_NULL},
    {"TH", MAC_DFS_DOMAIN_FCC},
    {"TN", MAC_DFS_DOMAIN_ETSI},
    {"TR", MAC_DFS_DOMAIN_ETSI},
    {"TT", MAC_DFS_DOMAIN_FCC},
    {"TW", MAC_DFS_DOMAIN_NULL},
    {"UA", MAC_DFS_DOMAIN_NULL},
    {"US", MAC_DFS_DOMAIN_FCC},
    {"UY", MAC_DFS_DOMAIN_FCC},
    {"UZ", MAC_DFS_DOMAIN_FCC},
    {"VE", MAC_DFS_DOMAIN_FCC},
    {"VN", MAC_DFS_DOMAIN_ETSI},
    {"YE", MAC_DFS_DOMAIN_NULL},
    {"ZA", MAC_DFS_DOMAIN_FCC},
    {"ZW", MAC_DFS_DOMAIN_NULL},
};


/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/

oal_uint32  wal_get_cmd_one_arg_etc(oal_int8 *pc_cmd, oal_int8 *pc_arg, oal_uint32 *pul_cmd_offset)
{
    oal_int8   *pc_cmd_copy;
    oal_uint32  ul_pos = 0;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pc_cmd) || (OAL_PTR_NULL == pc_arg) || (OAL_PTR_NULL == pul_cmd_offset)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_get_cmd_one_arg_etc::pc_cmd/pc_arg/pul_cmd_offset null ptr error %d, %d, %d!}\r\n", pc_cmd, pc_arg, pul_cmd_offset);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pc_cmd_copy = pc_cmd;

    /* ȥ���ַ�����ʼ�Ŀո� */
    while (' ' == *pc_cmd_copy)
    {
        ++pc_cmd_copy;
    }

    while ((' ' != *pc_cmd_copy) && ('\0' != *pc_cmd_copy))
    {
        pc_arg[ul_pos] = *pc_cmd_copy;
        ++ul_pos;
        ++pc_cmd_copy;

        if (OAL_UNLIKELY(ul_pos >= WAL_HIPRIV_CMD_NAME_MAX_LEN))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_get_cmd_one_arg_etc::ul_pos >= WAL_HIPRIV_CMD_NAME_MAX_LEN, ul_pos %d!}\r\n", ul_pos);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }

    pc_arg[ul_pos]  = '\0';

    /* �ַ�������β�����ش�����(������֮����) */
    if (0 == ul_pos)
    {
        OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_get_cmd_one_arg_etc::return param pc_arg is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pul_cmd_offset = (oal_uint32)(pc_cmd_copy - pc_cmd);

    return OAL_SUCC;
}


oal_uint8* wal_get_reduce_sar_ctrl_params(oal_uint8 uc_tx_power_lvl)
{
#ifdef _PRE_WLAN_FEATURE_TPC_OPT
    wlan_cust_nvram_params     *pst_cust_nv_params = hwifi_get_nvram_params_etc();  /* ����͹��ʶ��ƻ����� */
    if ((uc_tx_power_lvl <= CUS_NUM_OF_SAR_LVL) && (uc_tx_power_lvl > 0))
    {
        uc_tx_power_lvl--;
    }
    else
    {
        return OAL_PTR_NULL;
    }

    return pst_cust_nv_params->auc_sar_ctrl_params[uc_tx_power_lvl];
#else
    return OAL_PTR_NULL;
#endif
}


oal_uint32  wal_hipriv_set_fix_rate_pre_config(oal_net_device_stru *pst_net_dev, oal_bool_enum_uint8 en_fix_rate_enable, mac_cfg_set_dscr_param_stru *pc_stu)
{
    mac_vap_stru                   *pst_mac_vap = OAL_PTR_NULL;
    oal_uint32                      ul_ret;
    oal_int8                        ac_autorate_en_cmd[WAL_HIPRIV_CMD_NAME_MAX_LEN] = "ar_enable 1";
    oal_int8                        ac_autorate_dis_cmd[WAL_HIPRIV_CMD_NAME_MAX_LEN] ="ar_enable 0";
    oal_uint8                       uc_ampdu_cfg_idx;
    oal_int8                        ac_sw_ampdu_cmd[WAL_AMPDU_CFG_BUTT][WAL_HIPRIV_CMD_NAME_MAX_LEN] = {{"0"},{"1"}};

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0 , OAM_SF_ANY, "{wal_hipriv_set_fix_rate_pre_config::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ������Ч�̶�����ʱ������autorate���ָ��ۺϿ���ֵ  */
    if(OAL_FALSE == en_fix_rate_enable)
    {
        /*  ����autorate�㷨  */
        ul_ret = wal_hipriv_alg_cfg_etc(pst_net_dev, ac_autorate_en_cmd);
        if(OAL_SUCC != ul_ret)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_hipriv_set_fix_rate_pre_config:: autorate enable command config failed!err[d%]}\n", ul_ret);
            return OAL_FAIL;
        }

        if(OAL_TRUE == pst_mac_vap->st_fix_rate_pre_para.en_rate_cfg_tag)
        {
            /*  �ָ�ampdu�ۺ�  */
            uc_ampdu_cfg_idx = pst_mac_vap->st_fix_rate_pre_para.en_tx_ampdu_last;
            ul_ret = wal_hipriv_ampdu_tx_on(pst_net_dev, ac_sw_ampdu_cmd[uc_ampdu_cfg_idx]);
            if(OAL_SUCC != ul_ret)
            {
                OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_hipriv_set_fix_rate_pre_config:: software ampdu command config failed!err[d%]}\n", ul_ret);
                return OAL_FAIL;
            }

            /*��¼�̶��������ñ��Ϊδ����״̬*/
            pst_mac_vap->st_fix_rate_pre_para.en_rate_cfg_tag = OAL_FALSE;
        }

        return OAL_SUCC;
    }

    /*  �ر�autorate�㷨  */
    ul_ret = wal_hipriv_alg_cfg_etc(pst_net_dev, ac_autorate_dis_cmd);
    if(OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_hipriv_set_fix_rate_pre_config:: set autorate disable failed!err[d%]}\n", ul_ret);
        return OAL_FAIL;
    }

    /*11abgģʽ�����ù̶�����ǰ�ر�ampdu�ۺ�*/
    if(WAL_DSCR_PARAM_RATE == pc_stu->uc_function_index)
    {
        if(OAL_TRUE != pst_mac_vap->st_fix_rate_pre_para.en_rate_cfg_tag)
        {
            /*  ��¼ampdu����״̬  */
            pst_mac_vap->st_fix_rate_pre_para.en_tx_ampdu_last = mac_mib_get_CfgAmpduTxAtive(pst_mac_vap);
            pst_mac_vap->st_fix_rate_pre_para.en_rate_cfg_tag = OAL_TRUE;     /*��¼�̶��������ñ��Ϊ������״̬*/
        }

        /*  �ر�ampdu�ۺ�  */
        ul_ret = wal_hipriv_ampdu_tx_on(pst_net_dev, ac_sw_ampdu_cmd[WAL_AMPDU_DISABLE]);
        if(OAL_SUCC != ul_ret)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_hipriv_set_fix_rate_pre_config:: disable software ampdu failed!err[d%]}\n", ul_ret);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;

}


oal_uint32  wal_hipriv_process_rate_params(oal_net_device_stru *pst_net_dev, oal_int8 *pc_cmd, mac_cfg_set_dscr_param_stru *pc_stu)
{
    oal_uint32                       ul_ret;
    oal_uint32                       ul_off_set;
    oal_int32                        l_val;
    wal_dscr_param_enum_uint8        en_param_index;
    mac_cfg_non_ht_rate_stru* pst_set_non_ht_rate_param;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    ul_ret = wal_get_cmd_one_arg_etc(pc_cmd, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /*  ������Ч����ֵ255ʱ�ָ��Զ����� */
    l_val = oal_strtol(ac_arg, OAL_PTR_NULL, 0);
    if(WAL_IOCTL_IS_INVALID_FIXED_RATE(l_val,pc_stu))
    {
         ul_ret = wal_hipriv_set_fix_rate_pre_config(pst_net_dev, OAL_FALSE, pc_stu);
         if(OAL_SUCC != ul_ret)
         {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params: config invalid rate handle failed!err[%d]}\n", ul_ret);
            return OAL_FAIL;
         }
         return OAL_ERR_CODE_INVALID_CONFIG;
    }

    switch(pc_stu->uc_function_index)
    {
        case WAL_DSCR_PARAM_RATE:
            pst_set_non_ht_rate_param = (mac_cfg_non_ht_rate_stru*)(&(pc_stu->l_value));
            /* ��������*/
            for (en_param_index = 0; en_param_index < WLAN_LEGACY_RATE_VALUE_BUTT; en_param_index++)
            {
                if(!oal_strcmp(pauc_non_ht_rate_tbl[en_param_index], ac_arg))
                {
                    pst_set_non_ht_rate_param->en_rate = en_param_index;
                    break;
                }
            }

            /* ������������TX�������е�Э��ģʽ */
            if (en_param_index <= WLAN_SHORT_11b_11_M_BPS)
            {
                pst_set_non_ht_rate_param->en_protocol_mode = WLAN_11B_PHY_PROTOCOL_MODE;
            }
            else if (en_param_index >= WLAN_LEGACY_OFDM_48M_BPS && en_param_index <= WLAN_LEGACY_OFDM_9M_BPS)
            {
                pst_set_non_ht_rate_param->en_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            }
            else
            {
                OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::invalid rate!}\r\n");
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
            ul_ret = wal_hipriv_set_fix_rate_pre_config(pst_net_dev, OAL_TRUE, pc_stu);
            if(OAL_SUCC != ul_ret)
            {
               OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params: config fixed legacy rate handle failed!err[%d]}\n", ul_ret);
               return OAL_FAIL;
            }
            break;

        case WAL_DSCR_PARAM_MCS:
            l_val = oal_strtol(ac_arg, OAL_PTR_NULL, 0);
            if (l_val < WAL_HIPRIV_HT_MCS_MIN || l_val > WAL_HIPRIV_HT_MCS_MAX)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::input mcs val out of range [%d]!}\r\n", l_val);
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
            pc_stu->l_value = l_val;
            ul_ret = wal_hipriv_set_fix_rate_pre_config(pst_net_dev, OAL_TRUE, pc_stu);
            if(OAL_SUCC != ul_ret)
            {
               OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params: config fixed mcs rate handle failed!err[%d]}\n", ul_ret);
               return OAL_FAIL;
            }
            break;

        case WAL_DSCR_PARAM_MCSAC:
            l_val = oal_strtol(ac_arg, OAL_PTR_NULL, 0);
            if (l_val < WAL_HIPRIV_VHT_MCS_MIN || l_val > WAL_HIPRIV_VHT_MCS_MAX)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::input mcsac val out of range [%d]!}\r\n", l_val);
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
            pc_stu->l_value = l_val;

            ul_ret = wal_hipriv_set_fix_rate_pre_config(pst_net_dev, OAL_TRUE, pc_stu);
            if(OAL_SUCC != ul_ret)
            {
               OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params: config fixed mcsac rate handle failed!err[%d]}\n", ul_ret);
               return OAL_FAIL;
            }

            break;
        case WAL_DSCR_PARAM_MCSAX:
            l_val = oal_strtol(ac_arg, OAL_PTR_NULL, 0);
            if (l_val < WAL_HIPRIV_HE_MCS_MIN || l_val > WAL_HIPRIV_HE_MCS_MAX)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::input mcsax val out of range [%d]!}\r\n", l_val);
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
            pc_stu->l_value = l_val;

            ul_ret = wal_hipriv_set_fix_rate_pre_config(pst_net_dev, OAL_TRUE, pc_stu);
            if(OAL_SUCC != ul_ret)
            {
               OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params: config fixed mcsax rate handle failed!err[%d]}\n", ul_ret);
               return OAL_FAIL;
            }

            break;

        case WAL_DSCR_PARAM_NSS:
            for (en_param_index = 0; en_param_index < WLAN_NSS_LIMIT; en_param_index++)
            {
                if(!oal_strcmp(pauc_tx_dscr_nss_tbl[en_param_index], ac_arg))
                {
                    pc_stu->l_value = en_param_index;
                    break;
                }
            }
            if (WAL_DSCR_PARAM_BUTT == en_param_index)
            {
                OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::invalid param for nss!}\r\n");
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
            break;

        case WAL_DSCR_PARAM_BW:
            for (en_param_index = 0; en_param_index <= WLAN_BAND_ASSEMBLE_160M_DUP; en_param_index++)
            {
                if(!oal_strcmp(pauc_bw_tbl[en_param_index], ac_arg))
                {
                    pc_stu->l_value = en_param_index;
                    break;
                }
            }
            if (en_param_index > WLAN_BAND_ASSEMBLE_160M_DUP)
            {
                OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::invalid param for bandwidth!}\r\n");
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
            break;

        default:
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::invalid command!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
    }
    return OAL_SUCC;
}


oal_void wal_msg_queue_init_etc(oal_void)
{
    OAL_MEMZERO((oal_void*)&g_wal_wid_msg_queue, OAL_SIZEOF(g_wal_wid_msg_queue));
    oal_dlist_init_head(&g_wal_wid_msg_queue.st_head);
    g_wal_wid_msg_queue.count = 0;
    oal_spin_lock_init(&g_wal_wid_msg_queue.st_lock);
    OAL_WAIT_QUEUE_INIT_HEAD(&g_wal_wid_msg_queue.st_wait_queue);
}

OAL_STATIC oal_void _wal_msg_request_add_queue_(wal_msg_request_stru* pst_msg)
{
    oal_dlist_add_tail(&pst_msg->pst_entry, &g_wal_wid_msg_queue.st_head);
    g_wal_wid_msg_queue.count++;
}


oal_uint32 wal_get_request_msg_count_etc(oal_void)
{
    return g_wal_wid_msg_queue.count;
}

oal_uint32 wal_check_and_release_msg_resp_etc(wal_msg_stru   *pst_rsp_msg)
{
    wal_msg_write_rsp_stru     *pst_write_rsp_msg;
    if(OAL_PTR_NULL != pst_rsp_msg)
    {
        oal_uint32 ul_err_code;
        wlan_cfgid_enum_uint16 en_wid;
        pst_write_rsp_msg = (wal_msg_write_rsp_stru *)(pst_rsp_msg->auc_msg_data);
        ul_err_code = pst_write_rsp_msg->ul_err_code;
        en_wid = pst_write_rsp_msg->en_wid;
        oal_free(pst_rsp_msg);

        if (OAL_SUCC != ul_err_code)
        {
            OAM_WARNING_LOG2(0, OAM_SF_SCAN, "{wal_check_and_release_msg_resp_etc::detect err code:[%u],wid:[%u]}",
                           ul_err_code, en_wid);
            return ul_err_code;
        }
    }

    return OAL_SUCC;
}


oal_void wal_msg_request_add_queue_etc(wal_msg_request_stru* pst_msg)
{
#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
    if(OAL_FALSE== g_wal_wid_queue_init_flag)
    {
        wal_msg_queue_init_etc();
        g_wal_wid_queue_init_flag = OAL_TRUE;
    }
#endif
    oal_spin_lock_bh(&g_wal_wid_msg_queue.st_lock);
    _wal_msg_request_add_queue_(pst_msg);
    oal_spin_unlock_bh(&g_wal_wid_msg_queue.st_lock);
}

OAL_STATIC oal_void _wal_msg_request_remove_queue_(wal_msg_request_stru* pst_msg)
{
    g_wal_wid_msg_queue.count--;
    oal_dlist_delete_entry(&pst_msg->pst_entry);
}


oal_void wal_msg_request_remove_queue_etc(wal_msg_request_stru* pst_msg)
{
    oal_spin_lock_bh(&g_wal_wid_msg_queue.st_lock);
    _wal_msg_request_remove_queue_(pst_msg);
    oal_spin_unlock_bh(&g_wal_wid_msg_queue.st_lock);
}


oal_int32  wal_set_msg_response_by_addr_etc(oal_ulong addr,oal_void * pst_resp_mem ,oal_uint32 ul_resp_ret,
                                                 oal_uint32 uc_rsp_len)
{
    oal_int32 l_ret = -OAL_EINVAL;
    oal_dlist_head_stru * pst_pos;
    oal_dlist_head_stru * pst_entry_temp;
    wal_msg_request_stru* pst_request = NULL;

    oal_spin_lock_bh(&g_wal_wid_msg_queue.st_lock);
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_pos, pst_entry_temp, (&g_wal_wid_msg_queue.st_head))
    {
        pst_request = (wal_msg_request_stru *)OAL_DLIST_GET_ENTRY(pst_pos,wal_msg_request_stru,
                                                                  pst_entry);
        if(pst_request->ul_request_address == (oal_ulong)addr)
        {
            /*address match*/
            if(OAL_UNLIKELY(NULL != pst_request->pst_resp_mem))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_set_msg_response_by_addr_etc::wal_set_msg_response_by_addr_etc response had been set!");
            }
            pst_request->pst_resp_mem = pst_resp_mem;
            pst_request->ul_ret = ul_resp_ret;
            pst_request->ul_resp_len = uc_rsp_len;
            l_ret = OAL_SUCC;
            break;
        }
    }
    oal_spin_unlock_bh(&g_wal_wid_msg_queue.st_lock);

    return l_ret;
}


oal_uint32  wal_alloc_cfg_event_etc
                (oal_net_device_stru     *pst_net_dev,
                 frw_event_mem_stru     **ppst_event_mem,
                 oal_void*               pst_resp_addr,
                 wal_msg_stru           **ppst_cfg_msg,
                 oal_uint16               us_len)
{
    mac_vap_stru               *pst_vap;
    frw_event_mem_stru         *pst_event_mem;
    frw_event_stru             *pst_event;
    oal_uint16                  us_resp_len = 0;

    wal_msg_rep_hdr* pst_rep_hdr = NULL;

    pst_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_vap))
    {
        /* ���wifi�ر�״̬�£��·�hipriv������ʾerror��־ */
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_alloc_cfg_event_etc::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr! pst_net_dev=[%p]}", pst_net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }

    us_resp_len += OAL_SIZEOF(wal_msg_rep_hdr);

    us_len += us_resp_len;

    pst_event_mem = FRW_EVENT_ALLOC(us_len);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG2(pst_vap->uc_vap_id, OAM_SF_ANY, "{wal_alloc_cfg_event_etc::pst_event_mem null ptr error,request size:us_len:%d,resp_len:%d}",
                        us_len, us_resp_len);
        return OAL_ERR_CODE_PTR_NULL;
    }

    *ppst_event_mem = pst_event_mem;    /* ���θ�ֵ */

    pst_event = frw_get_event_stru(pst_event_mem);

    /* ��д�¼�ͷ */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CRX,
                       WAL_HOST_CRX_SUBTYPE_CFG,
                       us_len,
                       FRW_EVENT_PIPELINE_STAGE_0,
                       pst_vap->uc_chip_id,
                       pst_vap->uc_device_id,
                       pst_vap->uc_vap_id);

    /*fill the resp hdr*/
    pst_rep_hdr = (wal_msg_rep_hdr*)pst_event->auc_event_data;
    if(NULL == pst_resp_addr)
    {
        /*no response*/
        pst_rep_hdr->ul_request_address = (oal_ulong)0;
    }
    else
    {
        /*need response*/
         pst_rep_hdr->ul_request_address = (oal_ulong)pst_resp_addr;
    }


    *ppst_cfg_msg = (wal_msg_stru *)((oal_uint8*)pst_event->auc_event_data + us_resp_len);  /* ���θ�ֵ */

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE oal_int32 wal_request_wait_event_condition(wal_msg_request_stru *pst_msg_stru)
{
    oal_int32 l_ret = OAL_FALSE;
    oal_spin_lock_bh(&g_wal_wid_msg_queue.st_lock);
    if((NULL != pst_msg_stru->pst_resp_mem) || (OAL_SUCC != pst_msg_stru->ul_ret))
    {
        l_ret = OAL_TRUE;
    }
    oal_spin_unlock_bh(&g_wal_wid_msg_queue.st_lock);
    return l_ret;
}

oal_void wal_cfg_msg_task_sched_etc(oal_void)
{
    OAL_WAIT_QUEUE_WAKE_UP(&g_wal_wid_msg_queue.st_wait_queue);
}


oal_int32 wal_send_cfg_event_etc(oal_net_device_stru      *pst_net_dev,
                                   wal_msg_type_enum_uint8   en_msg_type,
                                   oal_uint16                us_len,
                                   oal_uint8                *puc_param,
                                   oal_bool_enum_uint8       en_need_rsp,
                                   wal_msg_stru            **ppst_rsp_msg)
{
    wal_msg_stru                *pst_cfg_msg;
    frw_event_mem_stru          *pst_event_mem;
    wal_msg_stru                *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                   ul_ret;
    oal_int32                    l_ret;
#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
    mac_vap_stru                *pst_mac_vap;
#endif

    DECLARE_WAL_MSG_REQ_STRU(st_msg_request);

    WAL_MSG_REQ_STRU_INIT(st_msg_request);

    if(NULL != ppst_rsp_msg)
    {
        *ppst_rsp_msg = NULL;
    }

    if (OAL_WARN_ON((OAL_TRUE == en_need_rsp)&&(OAL_PTR_NULL == ppst_rsp_msg)))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_send_cfg_event_etc::OAL_PTR_NULL == ppst_rsp_msg!}\r\n");
        return -OAL_EINVAL;
    }

    /* �����¼� */
    ul_ret = wal_alloc_cfg_event_etc(pst_net_dev, &pst_event_mem,
                                ((OAL_TRUE == en_need_rsp) ? &st_msg_request : NULL),
                                &pst_cfg_msg,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_send_cfg_event_etc::wal_alloc_cfg_event_etc return err code %d!}\r\n", ul_ret);
        return -OAL_ENOMEM;
    }

    /* ��д������Ϣ */
    WAL_CFG_MSG_HDR_INIT(&(pst_cfg_msg->st_msg_hdr),
                         en_msg_type,
                         us_len,
                         WAL_GET_MSG_SN());

    /* ��дWID��Ϣ */
    oal_memcopy(pst_cfg_msg->auc_msg_data, puc_param, us_len);

#ifdef _PRE_DEBUG_MODE
#ifdef _PRE_WLAN_DFT_EVENT
    wal_event_report_to_sdt(en_msg_type, puc_param, pst_cfg_msg);
#endif
#endif

    if (OAL_TRUE == en_need_rsp)
    {
        /*add queue before post event!*/
        wal_msg_request_add_queue_etc(&st_msg_request);
    }

/* �ַ��¼� */
#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0 , OAM_SF_ANY, "{wal_send_cfg_event_etc::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}");
        FRW_EVENT_FREE(pst_event_mem);
        return -OAL_EINVAL;
    }

    frw_event_post_event_etc(pst_event_mem, pst_mac_vap->ul_core_id);
#else
    frw_event_dispatch_event_etc(pst_event_mem);
#endif
    FRW_EVENT_FREE(pst_event_mem);

    /* win32 UTģʽ������һ���¼����� */
#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION) && (_PRE_TEST_MODE == _PRE_TEST_MODE_UT)
    frw_event_process_all_event_etc(0);
#endif

    if (OAL_FALSE == en_need_rsp)
    {
        return OAL_SUCC;
    }

    /*context can't in interrupt*/
    if(OAL_WARN_ON(oal_in_interrupt()))
    {
        DECLARE_DFT_TRACE_KEY_INFO("wal_cfg_in_interrupt", OAL_DFT_TRACE_EXCEP);
    }

    if(OAL_WARN_ON(oal_in_atomic()))
    {
        DECLARE_DFT_TRACE_KEY_INFO("wal_cfg_in_atomic", OAL_DFT_TRACE_EXCEP);
    }

    /***************************************************************************
        �ȴ��¼�����
    ***************************************************************************/
    wal_wake_lock();


    /*lint -e730*//*lint -e666*//* info, boolean argument to function */
    l_ret = OAL_WAIT_EVENT_TIMEOUT((g_wal_wid_msg_queue.st_wait_queue),
                                                (OAL_TRUE == wal_request_wait_event_condition(&st_msg_request)),
                                                (10 * OAL_TIME_HZ));
    /*lint +e730*//*lint +e666*/

    /*response had been set, remove it from the list*/
    if (OAL_TRUE == en_need_rsp)
    {
        wal_msg_request_remove_queue_etc(&st_msg_request);
    }

    if (OAL_WARN_ON(0 == l_ret))
    {
        /* ��ʱ .*/
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_send_cfg_event_etc:: wait queue timeout,10s!}\r\n");
        OAL_IO_PRINT("[E]timeout,request info:%p,ret=%u,addr:0x%lx\n", st_msg_request.pst_resp_mem,
                                               st_msg_request.ul_ret,
                                               st_msg_request.ul_request_address);
        if(NULL != st_msg_request.pst_resp_mem)
        {
            oal_free(st_msg_request.pst_resp_mem);
        }
        wal_wake_unlock();
        DECLARE_DFT_TRACE_KEY_INFO("wal_send_cfg_timeout",OAL_DFT_TRACE_FAIL);
        /*��ӡCFG EVENT�ڴ棬���㶨λ��*/
        oal_print_hex_dump((oal_uint8 *)pst_cfg_msg, (WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len), 32, "cfg event: ");
        //����С��100%������cpu 100%�ᴥ����ӡ�����ڴ�ӡ̫�࣬ȡ����ӡ
        //frw_event_queue_info_etc();
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
#ifndef  CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
        if(g_ast_event_task_etc[0].pst_event_kthread)
        {
            sched_show_task(g_ast_event_task_etc[0].pst_event_kthread);
        }
#endif
#endif
        return -OAL_ETIMEDOUT;
    }
    /*lint +e774*/

    pst_rsp_msg = (wal_msg_stru *)(st_msg_request.pst_resp_mem);
    if (OAL_PTR_NULL == pst_rsp_msg)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_send_cfg_event_etc:: msg mem null!}");
        /*lint -e613*/
        // tscancode-suppress *
        *ppst_rsp_msg  = OAL_PTR_NULL;
        /*lint +e613*/
        wal_wake_unlock();
        return -OAL_EFAUL;
    }

    if (0 == pst_rsp_msg->st_msg_hdr.us_msg_len)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_send_cfg_event_etc:: no msg resp!}");
        /*lint -e613*/
        *ppst_rsp_msg  = OAL_PTR_NULL;
        /*lint +e613*/
        oal_free(pst_rsp_msg);
        wal_wake_unlock();
        return -OAL_EFAUL;
    }
    /* ���������¼����ص�״̬��Ϣ */
    /*lint -e613*/
    *ppst_rsp_msg  = pst_rsp_msg;
    /*lint +e613*/
    wal_wake_unlock();
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_P2P


wlan_p2p_mode_enum_uint8 wal_wireless_iftype_to_mac_p2p_mode_etc(enum nl80211_iftype en_iftype)
{
    wlan_p2p_mode_enum_uint8 en_p2p_mode = WLAN_LEGACY_VAP_MODE;

    switch(en_iftype)
    {
        case NL80211_IFTYPE_P2P_CLIENT:
            en_p2p_mode = WLAN_P2P_CL_MODE;
            break;
        case NL80211_IFTYPE_P2P_GO:
            en_p2p_mode = WLAN_P2P_GO_MODE;
            break;
        case NL80211_IFTYPE_P2P_DEVICE:
            en_p2p_mode = WLAN_P2P_DEV_MODE;
            break;
        case NL80211_IFTYPE_AP:
        case NL80211_IFTYPE_STATION:
            en_p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        default:
            en_p2p_mode = WLAN_P2P_BUTT;
    }
    return en_p2p_mode;
}
#endif


oal_int32  wal_cfg_vap_h2d_event_etc(oal_net_device_stru *pst_net_dev)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_wireless_dev_stru      *pst_wdev;
    mac_wiphy_priv_stru        *pst_wiphy_priv;
    hmac_vap_stru              *pst_cfg_hmac_vap;
    mac_device_stru            *pst_mac_device;
    oal_net_device_stru        *pst_cfg_net_dev;

    oal_int32                   l_ret;
    wal_msg_stru                *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                  ul_err_code;
    wal_msg_write_stru          st_write_msg;

    pst_wdev = OAL_NETDEVICE_WDEV(pst_net_dev);
    if(OAL_PTR_NULL == pst_wdev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event_etc::pst_wdev is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_wiphy_priv  = (mac_wiphy_priv_stru *)oal_wiphy_priv(pst_wdev->wiphy);
    if(OAL_PTR_NULL == pst_wiphy_priv)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event_etc::pst_wiphy_priv is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_device  = pst_wiphy_priv->pst_mac_device;
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event_etc::pst_mac_device is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_hmac_vap= (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->uc_cfg_vap_id);
    if (OAL_PTR_NULL == pst_cfg_hmac_vap)
    {
        OAM_WARNING_LOG1(0,OAM_SF_ANY,"{wal_cfg_vap_h2d_event_etc::mac_res_get_hmac_vap fail.vap_id[%u]}",pst_mac_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;
    if(OAL_PTR_NULL == pst_cfg_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event_etc::pst_cfg_net_dev is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
    ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG_VAP_H2D, OAL_SIZEOF(mac_cfg_vap_h2d_stru));
    ((mac_cfg_vap_h2d_stru *)st_write_msg.auc_value)->pst_net_dev = pst_cfg_net_dev;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
             WAL_MSG_TYPE_WRITE,
             WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_vap_h2d_stru),
             (oal_uint8 *)&st_write_msg,
             OAL_TRUE,
             &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event_etc::wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event_etc::hmac cfg vap h2d fail,err code[%u]\r\n", ul_err_code);
        return -OAL_EINVAL;
    }

#endif

    return OAL_SUCC;

}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

OAL_STATIC oal_int32  wal_host_dev_config(oal_net_device_stru *pst_net_dev, wlan_cfgid_enum_uint16 en_wid)
{
    oal_wireless_dev_stru      *pst_wdev;
    mac_wiphy_priv_stru        *pst_wiphy_priv;
    hmac_vap_stru              *pst_cfg_hmac_vap;
    mac_device_stru            *pst_mac_device;
    oal_net_device_stru        *pst_cfg_net_dev;

    oal_int32                   l_ret;
    wal_msg_stru                *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                  ul_err_code;
    wal_msg_write_stru          st_write_msg;

    pst_wdev = OAL_NETDEVICE_WDEV(pst_net_dev);
    if(NULL == pst_wdev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_host_dev_config::pst_wdev is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_wiphy_priv  = (mac_wiphy_priv_stru *)oal_wiphy_priv(pst_wdev->wiphy);
    if (OAL_PTR_NULL == pst_wiphy_priv)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_host_dev_config::pst_mac_device is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_device  = pst_wiphy_priv->pst_mac_device;
    if(NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_host_dev_config::pst_mac_device is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->uc_cfg_vap_id);
    if (NULL == pst_cfg_hmac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_host_dev_config::pst_cfg_hmac_vap is null vap_id:%d!}\r\n",pst_mac_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;
    if(NULL == pst_cfg_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_host_dev_config::pst_cfg_net_dev is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
    ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, en_wid, 0);

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
             WAL_MSG_TYPE_WRITE,
             WAL_MSG_WRITE_MSG_HDR_LENGTH,
             (oal_uint8 *)&st_write_msg,
             OAL_TRUE,
             &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_host_dev_config::wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_host_dev_config::hmac cfg vap h2d fail,err code[%u]\r\n", ul_err_code);
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}


oal_int32  wal_host_dev_init_etc(oal_net_device_stru *pst_net_dev)
{
    return wal_host_dev_config(pst_net_dev, WLAN_CFGID_HOST_DEV_INIT);
}


OAL_STATIC oal_int32  wal_host_dev_exit(oal_net_device_stru *pst_net_dev)
{
    return wal_host_dev_config(pst_net_dev, WLAN_CFGID_HOST_DEV_EXIT);
}
#endif


#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

OAL_STATIC oal_uint32 hwifi_force_refresh_rf_params(oal_net_device_stru* pst_net_dev)
{
    /* update params */
    if (hwifi_config_init_etc(CUS_TAG_NV))
    {
        return OAL_FAIL;
    }

    /* send data to device */
    return hwifi_config_init_nvram_main_etc(pst_net_dev);
}


oal_int32 hwifi_config_host_global_ini_param(oal_void)
{
//#ifdef _PRE_WLAN_FEATURE_ROAM
    oal_int32                       l_val = 0;
//#endif /* #ifdef _PRE_WLAN_FEATURE_ROAM */
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ

    oal_uint32                      cfg_id;
    oal_uint32                      ul_val;
    oal_int32                       l_cfg_value;
    oal_int8*                       pc_tmp;
    host_speed_freq_level_stru      ast_host_speed_freq_level_tmp[4];
    device_speed_freq_level_stru    ast_device_speed_freq_level_tmp[4];
    oal_uint8                       uc_flag = OAL_FALSE;
    oal_uint8                       uc_index;
#endif /* #ifdef _PRE_WLAN_FEATURE_AUTO_FREQ */
    /******************************************** ���� ********************************************/
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_AMPDU_TX_MAX_NUM);
    g_st_wlan_customize_etc.ul_ampdu_tx_max_num = (WLAN_AMPDU_TX_MAX_NUM >= l_val && 1 <= l_val) ? (oal_uint32)l_val : g_st_wlan_customize_etc.ul_ampdu_tx_max_num;
    OAL_IO_PRINT("hwifi_config_host_global_ini_param::ampdu_tx_max_num:%d", g_st_wlan_customize_etc.ul_ampdu_tx_max_num);
#ifdef _PRE_WLAN_FEATURE_ROAM
    /******************************************** ���� ********************************************/
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_ROAM_SWITCH);
    g_st_wlan_customize_etc.uc_roam_switch          = (0 == l_val || 1 == l_val) ? (oal_uint8)l_val : g_st_wlan_customize_etc.uc_roam_switch;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_SCAN_ORTHOGONAL);
    g_st_wlan_customize_etc.uc_roam_scan_orthogonal = (1 <= l_val) ? (oal_uint8)l_val : g_st_wlan_customize_etc.uc_roam_scan_orthogonal;

    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TRIGGER_B);
    g_st_wlan_customize_etc.c_roam_trigger_b = (oal_int8)l_val;

    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TRIGGER_A);
    g_st_wlan_customize_etc.c_roam_trigger_a = (oal_int8)l_val;

    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_B);
    g_st_wlan_customize_etc.c_roam_delta_b = (oal_int8)l_val;

    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_A);
    g_st_wlan_customize_etc.c_roam_delta_a = (oal_int8)l_val;

    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DENSE_ENV_TRIGGER_B);
    g_st_wlan_customize_etc.c_dense_env_roam_trigger_b = (oal_int8)l_val;

    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DENSE_ENV_TRIGGER_A);
    g_st_wlan_customize_etc.c_dense_env_roam_trigger_a = (oal_int8)l_val;

    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_SCENARIO_ENABLE);
    g_st_wlan_customize_etc.uc_scenario_enable = (oal_uint8)l_val;

    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_CANDIDATE_GOOD_RSSI);
    g_st_wlan_customize_etc.c_candidate_good_rssi = (oal_int8)l_val;

    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_CANDIDATE_GOOD_NUM);
    g_st_wlan_customize_etc.uc_candidate_good_num = (oal_uint8)l_val;

    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_CANDIDATE_WEAK_NUM);
    g_st_wlan_customize_etc.uc_candidate_weak_num = (oal_uint8)l_val;

    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_INTERVAL_VARIABLE);
    g_st_wlan_customize_etc.us_roam_interval = (oal_uint16)l_val;
#endif /* #ifdef _PRE_WLAN_FEATURE_ROAM */

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_HW_AMPDU);
    g_st_ampdu_hw.uc_ampdu_hw_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_HW_AMPDU_TH_HIGH);
    g_st_ampdu_hw.us_throughput_high = (l_val > 0) ? (oal_uint16)l_val : 300;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_HW_AMPDU_TH_LOW);
    g_st_ampdu_hw.us_throughput_low = (l_val > 0) ? (oal_uint16)l_val : 200;
    OAL_IO_PRINT("ampdu_hw enable[%d]H[%u]L[%u]\r\n", g_st_ampdu_hw.uc_ampdu_hw_en,g_st_ampdu_hw.us_throughput_high,g_st_ampdu_hw.us_throughput_low);
#endif


#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_IRQ_AFFINITY);
    g_freq_lock_control_etc.en_irq_affinity = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_IRQ_TH_HIGH);
    g_freq_lock_control_etc.us_throughput_irq_high = (l_val > 0) ? (oal_uint16)l_val : 200;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_IRQ_TH_LOW);
    g_freq_lock_control_etc.us_throughput_irq_low = (l_val > 0) ? (oal_uint16)l_val : 150;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_IRQ_PPS_TH_HIGH);
    g_freq_lock_control_etc.ul_irq_pps_high = (l_val > 0) ? (oal_uint32)l_val : 25000;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_IRQ_PPS_TH_LOW);
    g_freq_lock_control_etc.ul_irq_pps_low = (l_val > 0) ? (oal_uint32)l_val : 5000;
    OAL_IO_PRINT("irq affinity enable[%d]High_th[%u]Low_th[%u]\r\n", g_freq_lock_control_etc.en_irq_affinity,g_freq_lock_control_etc.us_throughput_irq_high,g_freq_lock_control_etc.us_throughput_irq_low);
#endif
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_AMPDU_AMSDU_SKB);
    g_st_tx_large_amsdu.uc_host_large_amsdu_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_AMSDU_AMPDU_TH_HIGH);
    g_st_tx_large_amsdu.us_amsdu_throughput_high = (l_val > 0) ? (oal_uint16)l_val : 300;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_AMSDU_AMPDU_TH_LOW);
    g_st_tx_large_amsdu.us_amsdu_throughput_low = (l_val > 0) ? (oal_uint16)l_val : 200;
    OAL_IO_PRINT("ampdu+amsdu lareg skb en[%d],high[%d],low[%d]\r\n", g_st_tx_large_amsdu.uc_host_large_amsdu_en,g_st_tx_large_amsdu.us_amsdu_throughput_high,g_st_tx_large_amsdu.us_amsdu_throughput_low);
#endif
#ifdef _PRE_WLAN_TCP_OPT
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_FILTER);
    g_st_tcp_ack_filter.uc_tcp_ack_filter_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_FILTER_TH_HIGH);
    g_st_tcp_ack_filter.us_rx_filter_throughput_high = (l_val > 0) ? (oal_uint16)l_val : 50;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_FILTER_TH_LOW);
    g_st_tcp_ack_filter.us_rx_filter_throughput_low = (l_val > 0) ? (oal_uint16)l_val : 20;
    OAL_IO_PRINT("tcp ack filter en[%d],high[%d],low[%d]\r\n", g_st_tcp_ack_filter.uc_tcp_ack_filter_en, g_st_tcp_ack_filter.us_rx_filter_throughput_high, g_st_tcp_ack_filter.us_rx_filter_throughput_low);
#endif


#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA);
    g_st_rx_dyn_bypass_extlna_switch.uc_ini_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    g_st_rx_dyn_bypass_extlna_switch.uc_cur_status = OAL_TRUE;/* Ĭ�ϵ͹��ĳ��� */
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_HIGH);
    g_st_rx_dyn_bypass_extlna_switch.us_throughput_high = (l_val > 0) ? (oal_uint16)l_val : 100;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_LOW);
    g_st_rx_dyn_bypass_extlna_switch.us_throughput_low = (l_val > 0) ? (oal_uint16)l_val : 50;
    OAL_IO_PRINT("DYN_BYPASS_EXTLNA SWITCH en[%d],high[%d],low[%d]\r\n", g_st_rx_dyn_bypass_extlna_switch.uc_ini_en,g_st_rx_dyn_bypass_extlna_switch.us_throughput_high,g_st_rx_dyn_bypass_extlna_switch.us_throughput_low);

    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TX_SMALL_AMSDU);
    g_st_small_amsdu_switch.uc_ini_small_amsdu_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_SMALL_AMSDU_HIGH);
    g_st_small_amsdu_switch.us_small_amsdu_throughput_high = (l_val > 0) ? (oal_uint16)l_val : 300;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_SMALL_AMSDU_LOW);
    g_st_small_amsdu_switch.us_small_amsdu_throughput_low = (l_val > 0) ? (oal_uint16)l_val : 200;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_SMALL_AMSDU_PPS_HIGH);
    g_st_small_amsdu_switch.us_small_amsdu_pps_high = (l_val > 0) ? (oal_uint16)l_val : 25000;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_SMALL_AMSDU_PPS_LOW);
    g_st_small_amsdu_switch.us_small_amsdu_pps_low = (l_val > 0) ? (oal_uint16)l_val : 5000;
    OAL_IO_PRINT("SMALL AMSDU SWITCH en[%d],high[%d],low[%d]\r\n",  g_st_small_amsdu_switch.uc_ini_small_amsdu_en,g_st_small_amsdu_switch.us_small_amsdu_throughput_high,g_st_small_amsdu_switch.us_small_amsdu_throughput_low);



    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TX_TCP_ACK_BUF);
    g_st_tcp_ack_buf_switch.uc_ini_tcp_ack_buf_en= (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_HIGH);
    g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high = (l_val > 0) ? (oal_uint16)l_val : 90;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_LOW);
    g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low = (l_val > 0) ? (oal_uint16)l_val : 30;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_40M);
    g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_40M = (l_val > 0) ? (oal_uint16)l_val : 300;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_LOW_40M);
    g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_40M = (l_val > 0) ? (oal_uint16)l_val : 150;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_80M);
    g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_80M = (l_val > 0) ? (oal_uint16)l_val : 550;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_LOW_80M);
    g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_80M = (l_val > 0) ? (oal_uint16)l_val : 450;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_160M);
    g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_160M = (l_val > 0) ? (oal_uint16)l_val : 800;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_LOW_160M);
    g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_160M = (l_val > 0) ? (oal_uint16)l_val : 700;
    OAL_IO_PRINT("TCP ACK BUF en[%d],20M high[%d],low[%d],40M high[%d],low[%d], 80M high[%d],low[%d],160M high[%d],low[%d].\r\n",
                  g_st_tcp_ack_buf_switch.uc_ini_tcp_ack_buf_en,
                  g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high,
                  g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low,
                  g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_40M,
                  g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_40M,
                  g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_80M,
                  g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_80M,
                  g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_160M,
                  g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_160M);


#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RX_AMPDU_AMSDU_SKB);
    g_uc_host_rx_ampdu_amsdu  = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    OAL_IO_PRINT("Rx:ampdu+amsdu skb en[%d]\r\n", g_uc_host_rx_ampdu_amsdu);

#endif
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    /******************************************** �Զ���Ƶ ********************************************/
    /* config g_host_speed_freq_level_etc */
    pc_tmp = (oal_int8*)&ast_host_speed_freq_level_tmp;

    for(cfg_id = WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_0; cfg_id <= WLAN_CFG_INIT_DDR_FREQ_LIMIT_LEVEL_3; ++cfg_id)
    {
        ul_val = hwifi_get_init_value_etc(CUS_TAG_INI, cfg_id);
        *(oal_uint32*)pc_tmp = ul_val;
        pc_tmp += 4;
    }

    /* config g_device_speed_freq_level_etc */
    pc_tmp = (oal_int8*)&ast_device_speed_freq_level_tmp;
    for(cfg_id = WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_0; cfg_id <= WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_3; ++cfg_id)
    {
        l_cfg_value = hwifi_get_init_value_etc(CUS_TAG_INI, cfg_id);
        if(FREQ_IDLE <= l_cfg_value && FREQ_HIGHEST >= l_cfg_value)
        {
            *pc_tmp = l_cfg_value;
            pc_tmp += 4;
        }
        else
        {
            uc_flag = OAL_TRUE;
            break;
        }
    }

    if(!uc_flag)
    {
        oal_memcopy(&g_host_speed_freq_level_etc, &ast_host_speed_freq_level_tmp, OAL_SIZEOF(g_host_speed_freq_level_etc));
        oal_memcopy(&g_device_speed_freq_level_etc, &ast_device_speed_freq_level_tmp, OAL_SIZEOF(g_device_speed_freq_level_etc));

        for(uc_index = 0;uc_index < 4;uc_index++)
        {
            OAM_WARNING_LOG4(0, OAM_SF_ANY, "{hwifi_config_host_global_ini_param::ul_speed_level = %d,ul_min_cpu_freq = %d,ul_min_ddr_freq = %d,uc_device_type = %d}\r\n",
                g_host_speed_freq_level_etc[uc_index].ul_speed_level,
                g_host_speed_freq_level_etc[uc_index].ul_min_cpu_freq,
                g_host_speed_freq_level_etc[uc_index].ul_min_ddr_freq,
                g_device_speed_freq_level_etc[uc_index].uc_device_type);
        }
    }
#endif /* #ifdef _PRE_WLAN_FEATURE_AUTO_FREQ */
    /******************************************** ɨ�� ********************************************/
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RANDOM_MAC_ADDR_SCAN);
    g_st_wlan_customize_etc.uc_random_mac_addr_scan = !!l_val;

    /******************************************** CAPABILITY ********************************************/
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DISABLE_CAPAB_2GHT40);
    g_st_wlan_customize_etc.uc_disable_capab_2ght40 = (oal_uint8)!!l_val;
    /********************************************factory_lte_gpio_check ********************************************/
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_LTE_GPIO_CHECK_SWITCH);
    g_st_wlan_customize_etc.ul_lte_gpio_check_switch = (oal_uint32)!!l_val;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_ATCMDSRV_ISM_PRIORITY);
    g_st_wlan_customize_etc.ul_ism_priority = (oal_uint32)l_val;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_ATCMDSRV_LTE_RX);
    g_st_wlan_customize_etc.ul_lte_rx = (oal_uint32)l_val;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_ATCMDSRV_LTE_TX);
    g_st_wlan_customize_etc.ul_lte_tx = (oal_uint32)l_val;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_ATCMDSRV_LTE_INACT);
    g_st_wlan_customize_etc.ul_lte_inact = (oal_uint32)l_val;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_ATCMDSRV_ISM_RX_ACT);
    g_st_wlan_customize_etc.ul_ism_rx_act = (oal_uint32)l_val;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_ATCMDSRV_BANT_PRI);
    g_st_wlan_customize_etc.ul_bant_pri = (oal_uint32)l_val;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_ATCMDSRV_BANT_STATUS);
    g_st_wlan_customize_etc.ul_bant_status = (oal_uint32)l_val;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_ATCMDSRV_WANT_PRI);
    g_st_wlan_customize_etc.ul_want_pri = (oal_uint32)l_val;
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_ATCMDSRV_WANT_STATUS);
    g_st_wlan_customize_etc.ul_want_status = (oal_uint32)l_val;
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_VOE_SWITCH);
    g_st_mac_voe_custom_param.en_11k = ((l_val & 0x0F) & BIT0) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_voe_custom_param.en_11v = ((l_val & 0x0F) & BIT1) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_voe_custom_param.en_11r = ((l_val & 0x0F) & BIT2) ? OAL_TRUE : OAL_FALSE;
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    l_val = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_11AX_SWITCH);
    g_pst_mac_device_capability[0].en_11ax_switch = ((l_val & 0x0F) & BIT0) ? OAL_TRUE : OAL_FALSE;
#endif
    return OAL_SUCC;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

OAL_STATIC oal_void hwifi_config_init_ini_country(oal_net_device_stru *pst_cfg_net_dev)
{
    oal_int32                   l_ret;

    l_ret = (oal_int32)wal_hipriv_setcountry(pst_cfg_net_dev, hwifi_get_country_code_etc());

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini_country::wal_send_cfg_event_etc return err code [%d]!}\r\n", l_ret);
    }

    /* ����wifi���ƻ����ù����� */
    g_st_cust_country_code_ignore_flag.en_country_code_ingore_hipriv_flag = OAL_FALSE;
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)

OAL_STATIC oal_void hwifi_config_init_ini_pm(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;
    oal_int32                   l_switch;

    /* ���� */
    l_switch = !!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_POWERMGMT_SWITCH);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_PM_EN, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_switch;

    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini_pm::return err code [%d]!}\r\n", l_ret);
    }
}
#endif // #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)

#ifdef _PRE_WLAN_FEATURE_SMARTANT
OAL_STATIC oal_void hwifi_config_init_ini_dual_antenna(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;
    oal_int32                   l_switch;
    oal_uint8                   *puc_param;
    l_switch = !!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DUAL_ANTENNA_ENABLE);
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DOUBLE_ANT_SW, OAL_SIZEOF(oal_int32));
    puc_param = (oal_uint8 *)(st_write_msg.auc_value);
    *puc_param = (oal_uint8)l_switch;
    *(puc_param + 1) = 1;
    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini_dual_antenna::return err code [%d]!}\r\n", l_ret);
    }
}
#endif

OAL_STATIC oal_void hwifi_config_init_ini_log(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;
    oal_int32                   l_loglevel;

    /* log_level */
    l_loglevel = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_LOGLEVEL);
    if (l_loglevel < OAM_LOG_LEVEL_ERROR ||
        l_loglevel > OAM_LOG_LEVEL_INFO)
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{hwifi_config_init_ini_clock::loglevel[%d] out of range[%d,%d], check value in ini file!}\r\n",
                            l_loglevel, OAM_LOG_LEVEL_ERROR, OAM_LOG_LEVEL_INFO);
        return;
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_ALL_LOG_LEVEL, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_loglevel;
    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini_log::return err code[%d]!}\r\n", l_ret);
    }
}


#if defined(_PRE_PRODUCT_ID_HI110X_HOST)

oal_int8 hwifi_check_pwr_ref_delta(oal_int8 c_pwr_ref_delta)
{
    oal_int c_ret = 0;
    if (c_pwr_ref_delta > WAL_HIPRIV_PWR_REF_DELTA_HI)
    {
        c_ret = WAL_HIPRIV_PWR_REF_DELTA_HI;
    }
    else if (c_pwr_ref_delta < WAL_HIPRIV_PWR_REF_DELTA_LO)
    {
        c_ret = WAL_HIPRIV_PWR_REF_DELTA_LO;
    }
    else
    {
        c_ret = c_pwr_ref_delta;
    }

    return c_ret;
}


oal_void hwifi_cfg_pwr_ref_delta(mac_cfg_customize_rf       *pst_customize_rf)
{
    oal_uint8 uc_rf_idx;
    WLAN_CFG_INIT cfg_id;
    oal_int32 l_pwr_ref_delta;

    /* 2G 20M/40M */
    for (uc_rf_idx = 0; uc_rf_idx < WLAN_RF_CHANNEL_NUMS; uc_rf_idx++)
    {
        cfg_id = (uc_rf_idx == WLAN_RF_CHANNEL_ZERO) ? WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C0_MULT4 : WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C1_MULT4;
        l_pwr_ref_delta = hwifi_get_init_value_etc(CUS_TAG_INI, cfg_id);
        pst_customize_rf->ast_delta_pwr_ref_cfg[uc_rf_idx].c_cfg_delta_pwr_ref_rssi_2g[0] = hwifi_check_pwr_ref_delta(l_pwr_ref_delta & 0xff);
        pst_customize_rf->ast_delta_pwr_ref_cfg[uc_rf_idx].c_cfg_delta_pwr_ref_rssi_2g[1] = hwifi_check_pwr_ref_delta((l_pwr_ref_delta >> 8) & 0xff);
    }

    /* 5G 20M/40M/80M/160M */
    for (uc_rf_idx = 0; uc_rf_idx < WLAN_RF_CHANNEL_NUMS; uc_rf_idx++)
    {
        cfg_id = (uc_rf_idx == WLAN_RF_CHANNEL_ZERO) ? WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C0_MULT4 : WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C1_MULT4;
        l_pwr_ref_delta = hwifi_get_init_value_etc(CUS_TAG_INI, cfg_id);
        pst_customize_rf->ast_delta_pwr_ref_cfg[uc_rf_idx].c_cfg_delta_pwr_ref_rssi_5g[0] = hwifi_check_pwr_ref_delta(l_pwr_ref_delta & 0xff);
        pst_customize_rf->ast_delta_pwr_ref_cfg[uc_rf_idx].c_cfg_delta_pwr_ref_rssi_5g[1] = hwifi_check_pwr_ref_delta((l_pwr_ref_delta >> 8) & 0xff);
        pst_customize_rf->ast_delta_pwr_ref_cfg[uc_rf_idx].c_cfg_delta_pwr_ref_rssi_5g[2] = hwifi_check_pwr_ref_delta((l_pwr_ref_delta >> 16) & 0xff);
        pst_customize_rf->ast_delta_pwr_ref_cfg[uc_rf_idx].c_cfg_delta_pwr_ref_rssi_5g[3] = hwifi_check_pwr_ref_delta((l_pwr_ref_delta >> 24) & 0xff);
    }
}

OAL_STATIC oal_uint32 hwifi_cfg_front_end(oal_uint8 *puc_param)
{
    mac_cfg_customize_rf       *pst_customize_rf;
    oal_uint8                   uc_idx;                 /*�ṹ�������±�*/
    oal_int32                   l_mult4;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    oal_int8                    c_mult4_rf[2];
#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)

    pst_customize_rf = (mac_cfg_customize_rf *)puc_param;

    /* ����: 2g rf */
    for (uc_idx = 0; uc_idx < MAC_NUM_2G_BAND; ++uc_idx)
    {
        /* ��ȡ��2p4g ��band 0.25db��0.1db���ȵ�����ֵ */
        l_mult4  = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND_START + uc_idx);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
        /* rf0 */
        c_mult4_rf[0]  = (oal_int8)(l_mult4 & 0xFF);
        /* rf1 */
        c_mult4_rf[1]  = (oal_int8)((l_mult4 >> 8) & 0xFF);

        if(c_mult4_rf[0]  >= RF_LINE_TXRX_GAIN_DB_2G_MIN && c_mult4_rf[0]  <= RF_LINE_TXRX_GAIN_DB_MAX  &&
           c_mult4_rf[1]  >= RF_LINE_TXRX_GAIN_DB_2G_MIN && c_mult4_rf[1]  <= RF_LINE_TXRX_GAIN_DB_MAX)
        {
            pst_customize_rf->ast_rf_gain_db_rf[0].ac_gain_db_2g[uc_idx].c_rf_gain_db_mult4  = c_mult4_rf[0];
            pst_customize_rf->ast_rf_gain_db_rf[1].ac_gain_db_2g[uc_idx].c_rf_gain_db_mult4  = c_mult4_rf[1];
        }
#else
        if(l_mult4  >= RF_LINE_TXRX_GAIN_DB_2G_MIN && l_mult4  <= 0)
        {
            pst_customize_rf->st_rf_gain_db_rf.ac_gain_db_2g[uc_idx].c_rf_gain_db_mult4  = (oal_int8)l_mult4;
        }
#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
        else
        {
            /* ֵ������Ч��Χ */
            OAM_ERROR_LOG2(0, OAM_SF_CFG, "{hwifi_cfg_front_end::ini_id[%d]value out of range, 2g mult4[0x%0x}!}",
                          WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND_START + uc_idx, l_mult4);
            return OAL_FAIL;
        }
    }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    hwifi_cfg_pwr_ref_delta(pst_customize_rf);

    /* ͨ��radio cap */
    pst_customize_rf->uc_chn_radio_cap = (oal_uint8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_CHANN_RADIO_CAP);

    /*2g �ⲿfem */
    /* RF0 */
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_lna_bypass_gain_db = (oal_int8)CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_lna_gain_db        = (oal_int8)CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_GAIN_DB_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b0_db      = (oal_int8)CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b1_db      = (oal_int8)CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B1_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].uc_pa_gain_lvl_num   = (oal_int8)CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_LVL_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].uc_ext_switch_isexist = (oal_uint8)!!CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].uc_ext_pa_isexist     = (oal_uint8)!!CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_PA_ISEXIST_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].uc_ext_lna_isexist    = (oal_uint8)!!CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_LNA_ISEXIST_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].us_lna_on2off_time_ns = (oal_uint16)CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].us_lna_off2on_time_ns = (oal_uint16)CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_2G));
    /* RF1 */
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_lna_bypass_gain_db = (oal_int8)CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_lna_gain_db        = (oal_int8)CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_GAIN_DB_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b0_db      = (oal_int8)CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b1_db      = (oal_int8)CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B1_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].uc_pa_gain_lvl_num   = (oal_int8)CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_LVL_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].uc_ext_switch_isexist = (oal_uint8)!!CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].uc_ext_pa_isexist     = (oal_uint8)!!CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_PA_ISEXIST_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].uc_ext_lna_isexist    = (oal_uint8)!!CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_LNA_ISEXIST_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].us_lna_on2off_time_ns = (oal_uint16)CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_2G));
    pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].us_lna_off2on_time_ns = (oal_uint16)CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_2G));

    if ((pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_lna_bypass_gain_db < RF_LINE_TXRX_GAIN_DB_2G_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_lna_bypass_gain_db > 0 ||
        pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b0_db < RF_LINE_TXRX_GAIN_DB_2G_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b0_db > 0 ||
        pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b1_db < RF_LINE_TXRX_GAIN_DB_2G_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b1_db > 0 ||
        pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].uc_pa_gain_lvl_num == 0 || pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].uc_pa_gain_lvl_num > MAC_EXT_PA_GAIN_MAX_LVL ||
        pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_lna_gain_db < LNA_GAIN_DB_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ZERO].c_lna_gain_db > LNA_GAIN_DB_MAX) ||
        (pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_lna_bypass_gain_db < RF_LINE_TXRX_GAIN_DB_2G_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_lna_bypass_gain_db > 0 ||
        pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b0_db < RF_LINE_TXRX_GAIN_DB_2G_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b0_db > 0 ||
        pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b1_db < RF_LINE_TXRX_GAIN_DB_2G_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b1_db > 0 ||
        pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].uc_pa_gain_lvl_num == 0 || pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].uc_pa_gain_lvl_num > MAC_EXT_PA_GAIN_MAX_LVL ||
        pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_lna_gain_db < LNA_GAIN_DB_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_lna_gain_db > LNA_GAIN_DB_MAX))
    {
        /* ֵ������Ч��Χ */
        OAM_ERROR_LOG4(0, OAM_SF_CFG,
        "{hwifi_cfg_front_end::2g gain db out of range! rf0 lna_bypass[%d] pa_b0[%d] lna gain[%d] pa_b1[%d]}",
        pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_lna_bypass_gain_db,
        pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b0_db,
        pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_lna_gain_db,
        pst_customize_rf->ast_ext_rf[WLAN_BAND_2G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b1_db);

        return OAL_FAIL;
    }

    /* 2g���ƻ�RF����PAƫ�üĴ���  */
    for (uc_idx = 0; uc_idx < CUS_RF_PA_BIAS_REG_NUM; uc_idx++)
    {
        pst_customize_rf->aul_2g_pa_bias_rf_reg[uc_idx] = (oal_uint32)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_TX2G_PA_GATE_VCTL_REG236 + uc_idx);
    }
#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)

    if (OAL_TRUE == mac_device_check_5g_enable_per_chip())
    {
        /* ����: 5g rf */
        /* ����: fem�ڵ����߿ڵĸ����� */
        for (uc_idx = 0; uc_idx < MAC_NUM_5G_BAND; ++uc_idx)
        {
            /* ��ȡ��5g ��band 0.25db��0.1db���ȵ�����ֵ */
            l_mult4  = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND_START + uc_idx);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
            c_mult4_rf[0]  = (oal_int8)(l_mult4 & 0xFF);
            c_mult4_rf[1]  = (oal_int8)((l_mult4 >> 8) & 0xFF);

            if(c_mult4_rf[0] <= RF_LINE_TXRX_GAIN_DB_MAX && c_mult4_rf[1] <= RF_LINE_TXRX_GAIN_DB_MAX)
            {
                pst_customize_rf->ast_rf_gain_db_rf[0].ac_gain_db_5g[uc_idx].c_rf_gain_db_mult4  = c_mult4_rf[0];
                pst_customize_rf->ast_rf_gain_db_rf[1].ac_gain_db_5g[uc_idx].c_rf_gain_db_mult4  = c_mult4_rf[1];
            }
#else
            if(l_mult4 <= 0)
            {
                pst_customize_rf->st_rf_gain_db_rf.ac_gain_db_5g[uc_idx].c_rf_gain_db_mult4  = (oal_int8)l_mult4;
            }
#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
            else
            {
                /* ֵ������Ч��Χ */
                OAM_ERROR_LOG2(0, OAM_SF_CFG, "{hwifi_cfg_front_end::ini_id[%d]value out of range, 5g mult4[0x%0x}!}\r\n",
                          WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND_START + uc_idx, l_mult4);
                return OAL_FAIL;
            }
        }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
        /*5g �ⲿfem */
        /* RF0 */
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_lna_bypass_gain_db = (oal_int8)CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_lna_gain_db        = (oal_int8)CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_GAIN_DB_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b0_db      = (oal_int8)CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b1_db      = (oal_int8)CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B1_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].uc_pa_gain_lvl_num   = (oal_int8)CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_LVL_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].uc_ext_switch_isexist = (oal_uint8)!!CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].uc_ext_pa_isexist     = (oal_uint8)!!CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_PA_ISEXIST_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].uc_ext_lna_isexist    = (oal_uint8)!!CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_LNA_ISEXIST_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].us_lna_on2off_time_ns = (oal_uint16)CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].us_lna_off2on_time_ns = (oal_uint16)CUS_GET_LOW_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_5G));
        /* RF1 */
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_lna_bypass_gain_db = (oal_int8)CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_lna_gain_db        = (oal_int8)CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_GAIN_DB_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b0_db      = (oal_int8)CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b1_db      = (oal_int8)CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B1_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].uc_pa_gain_lvl_num   = (oal_int8)CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_LVL_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].uc_ext_switch_isexist = (oal_uint8)!!CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].uc_ext_pa_isexist     = (oal_uint8)!!CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_PA_ISEXIST_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].uc_ext_lna_isexist    = (oal_uint8)!!CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_LNA_ISEXIST_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].us_lna_on2off_time_ns = (oal_uint16)CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_5G));
        pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].us_lna_off2on_time_ns = (oal_uint16)CUS_GET_HIGH_16BIT(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_5G));

        /* 5g upc mix_bf_gain_ctl for P10 */
        for (uc_idx = 0; uc_idx < MAC_NUM_5G_BAND; uc_idx++)
        {
            pst_customize_rf->aul_5g_upc_mix_gain_rf_reg[uc_idx] = (oal_uint32)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_TX5G_UPC_MIX_GAIN_CTRL_1 + uc_idx);
        }

        if ((pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_lna_bypass_gain_db < RF_LINE_TXRX_GAIN_DB_5G_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_lna_bypass_gain_db > 0 ||
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b0_db < RF_LINE_TXRX_GAIN_DB_5G_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b0_db > 0 ||
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b1_db < RF_LINE_TXRX_GAIN_DB_5G_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_pa_gain_b1_db > 0 ||
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].uc_pa_gain_lvl_num == 0 || pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].uc_pa_gain_lvl_num > MAC_EXT_PA_GAIN_MAX_LVL ||
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_lna_gain_db < LNA_GAIN_DB_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ZERO].c_lna_gain_db > LNA_GAIN_DB_MAX) ||
            (pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_lna_bypass_gain_db < RF_LINE_TXRX_GAIN_DB_5G_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_lna_bypass_gain_db > 0 ||
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b0_db < RF_LINE_TXRX_GAIN_DB_5G_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b0_db > 0 ||
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b1_db < RF_LINE_TXRX_GAIN_DB_5G_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b1_db > 0 ||
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].uc_pa_gain_lvl_num == 0 || pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].uc_pa_gain_lvl_num > MAC_EXT_PA_GAIN_MAX_LVL ||
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_lna_gain_db < LNA_GAIN_DB_MIN || pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_lna_gain_db > LNA_GAIN_DB_MAX))
        {
            /* ֵ������Ч��Χ */
            OAM_ERROR_LOG4(0, OAM_SF_CFG,
            "{hwifi_cfg_front_end::2g gain db out of range! rf0 lna_bypass[%d] pa_b0[%d] lna gain[%d] pa_b1[%d]}",
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_lna_bypass_gain_db,
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b0_db,
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_lna_gain_db,
            pst_customize_rf->ast_ext_rf[WLAN_BAND_5G][WLAN_RF_CHANNEL_ONE].c_pa_gain_b1_db);

            return OAL_FAIL;
        }
#else
        /*5g �ⲿfem */
        pst_customize_rf->st_ext_rf.c_lna_bypass_gain_db    = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_5G);
        pst_customize_rf->st_ext_rf.c_lna_gain_db           = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LNA_GAIN_DB_5G);
        pst_customize_rf->st_ext_rf.c_pa_gain_b0_db         = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_5G);
        pst_customize_rf->st_ext_rf.uc_ext_switch_isexist   = (oal_uint8)!!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_5G);
        pst_customize_rf->st_ext_rf.uc_ext_pa_isexist       = (oal_uint8)!!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_PA_ISEXIST_5G);
        pst_customize_rf->st_ext_rf.uc_ext_lna_isexist      = (oal_uint8)!!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_LNA_ISEXIST_5G);
        pst_customize_rf->st_ext_rf.us_lna_on2off_time_ns   = (oal_uint16)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_5G);
        pst_customize_rf->st_ext_rf.us_lna_off2on_time_ns   = (oal_uint16)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_5G);
        if (pst_customize_rf->st_ext_rf.c_lna_bypass_gain_db < RF_LINE_TXRX_GAIN_DB_5G_MIN ||
            pst_customize_rf->st_ext_rf.c_lna_bypass_gain_db > 0 ||
            pst_customize_rf->st_ext_rf.c_pa_gain_b0_db < RF_LINE_TXRX_GAIN_DB_5G_MIN ||
            pst_customize_rf->st_ext_rf.c_pa_gain_b0_db > 0 ||
            pst_customize_rf->st_ext_rf.c_lna_gain_db < LNA_GAIN_DB_MIN ||
            pst_customize_rf->st_ext_rf.c_lna_gain_db > LNA_GAIN_DB_MAX)
        {
            /* ֵ������Ч��Χ */
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hwifi_cfg_front_end::gain db rx or tx 5g lna gain db or out of range!}");
            return OAL_FAIL;
        }
#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    }

    pst_customize_rf->uc_far_dist_pow_gain_switch = (oal_uint8)!!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_FAR_DIST_POW_GAIN_SWITCH);
    pst_customize_rf->uc_far_dist_dsss_scale_promote_switch = (oal_uint8)!!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_FAR_DIST_DSSS_SCALE_PROMOTE_SWITCH);

    /* ����: cca�������޵���ֵ */
    {
        oal_int8 c_delta_cca_ed_high_20th_2g = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_2G);
        oal_int8 c_delta_cca_ed_high_40th_2g = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_2G);
        oal_int8 c_delta_cca_ed_high_20th_5g = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_5G);
        oal_int8 c_delta_cca_ed_high_40th_5g = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_5G);
        oal_int8 c_delta_cca_ed_high_80th_5g = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_80TH_5G);

        /* ���ÿһ��ĵ��������Ƿ񳬳�������� */
        if (CUS_DELTA_CCA_ED_HIGH_TH_OUT_OF_RANGE(c_delta_cca_ed_high_20th_2g)
           || CUS_DELTA_CCA_ED_HIGH_TH_OUT_OF_RANGE(c_delta_cca_ed_high_40th_2g)
           || CUS_DELTA_CCA_ED_HIGH_TH_OUT_OF_RANGE(c_delta_cca_ed_high_20th_5g)
           || CUS_DELTA_CCA_ED_HIGH_TH_OUT_OF_RANGE(c_delta_cca_ed_high_40th_5g)
           || CUS_DELTA_CCA_ED_HIGH_TH_OUT_OF_RANGE(c_delta_cca_ed_high_80th_5g))
        {
            OAM_ERROR_LOG4(0, OAM_SF_ANY, "{hwifi_cfg_front_end::one or more delta cca ed high threshold out of range \
                 [delta_20th_2g=%d, delta_40th_2g=%d, delta_20th_5g=%d, delta_40th_5g=%d], please check the value!}",
                                            c_delta_cca_ed_high_20th_2g,
                                            c_delta_cca_ed_high_40th_2g,
                                            c_delta_cca_ed_high_20th_5g,
                                            c_delta_cca_ed_high_40th_5g);
            /* set 0 */
            pst_customize_rf->c_delta_cca_ed_high_20th_2g = 0;
            pst_customize_rf->c_delta_cca_ed_high_40th_2g = 0;
            pst_customize_rf->c_delta_cca_ed_high_20th_5g = 0;
            pst_customize_rf->c_delta_cca_ed_high_40th_5g = 0;
            pst_customize_rf->c_delta_cca_ed_high_80th_5g = 0;
        }
        else
        {
            pst_customize_rf->c_delta_cca_ed_high_20th_2g = c_delta_cca_ed_high_20th_2g;
            pst_customize_rf->c_delta_cca_ed_high_40th_2g = c_delta_cca_ed_high_40th_2g;
            pst_customize_rf->c_delta_cca_ed_high_20th_5g = c_delta_cca_ed_high_20th_5g;
            pst_customize_rf->c_delta_cca_ed_high_40th_5g = c_delta_cca_ed_high_40th_5g;
            pst_customize_rf->c_delta_cca_ed_high_80th_5g = c_delta_cca_ed_high_80th_5g;
        }
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI

OAL_STATIC oal_uint32 hwifi_cfg_init_cus_dyn_cali(mac_cus_dy_cali_param_stru *puc_dyn_cali_param)
{
    oal_int32      l_val;
    oal_uint8      uc_idx    = 0;
    oal_uint8      uc_rf_idx;
    oal_uint8      uc_dy_cal_param_idx;
    oal_uint8      uc_cfg_id        = WLAN_CFG_DTS_2G_CORE0_DPN_CH1;
    oal_uint8      uc_dpn_2g_nv_id  = WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0;
    oal_uint8      uc_dpn_5g_nv_id  = WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C0;
    oal_int8       ac_dpn_nv[HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_CW][MAC_2G_CHANNEL_NUM];
    oal_int8       ac_dpn_5g_nv[OAL_5G_160M_CHANNEL_NUM];
    oal_uint8      uc_num_idx;
    oal_uint8     *puc_cust_nvram_info;
    oal_uint8     *pc_end = ";";
    oal_uint8     *pc_sep = ",";
    oal_int8      *pc_ctx;
    oal_int8      *pc_token;
    oal_uint8      auc_nv_pa_params[CUS_PARAMS_LEN_MAX] = {0};

    for (uc_rf_idx = 0; uc_rf_idx < WLAN_RF_CHANNEL_NUMS; uc_rf_idx++)
    {
        puc_dyn_cali_param->uc_rf_id = uc_rf_idx;

        /* ��̬У׼������ϵ����μ�� */
        for (uc_dy_cal_param_idx = 0; uc_dy_cal_param_idx < DY_CALI_PARAMS_NUM; uc_dy_cal_param_idx++)
        {
            if (!g_ast_pro_line_params[uc_rf_idx][uc_dy_cal_param_idx].l_pow_par2)
            {
                OAM_ERROR_LOG1(0, OAM_SF_CUSTOM, "{hwifi_cfg_init_cus_dyn_cali::unexpected val[%d] s_pow_par2[0] check nv and ini file!}", uc_dy_cal_param_idx);
                return OAL_FAIL;
            }
        }
        oal_memcopy(puc_dyn_cali_param->al_dy_cali_base_ratio_params, g_ast_pro_line_params[uc_rf_idx],
                    OAL_SIZEOF(puc_dyn_cali_param->al_dy_cali_base_ratio_params));

        oal_memcopy(puc_dyn_cali_param->al_dy_cali_base_ratio_ppa_params, &g_ast_pro_line_params[uc_rf_idx][CUS_DY_CALI_PARAMS_NUM],
                    OAL_SIZEOF(puc_dyn_cali_param->al_dy_cali_base_ratio_ppa_params));

        oal_memcopy(puc_dyn_cali_param->as_extre_point_val, gs_extre_point_vals[uc_rf_idx],
                    OAL_SIZEOF(puc_dyn_cali_param->as_extre_point_val));

#ifdef _PRE_WLAN_DPINIT_CALI
        /* DP INIT*/
        if (OAL_FALSE == g_en_nv_dp_init_is_null)
        {
            puc_cust_nvram_info = hwifi_get_nvram_param(uc_rf_idx);
            oal_memcopy(auc_nv_pa_params, puc_cust_nvram_info, OAL_STRLEN(puc_cust_nvram_info));
            pc_token = oal_strtok(auc_nv_pa_params, pc_end, &pc_ctx);
            pc_token = oal_strtok(pc_token, pc_sep, &pc_ctx);
            uc_idx = 0;
            /* ��ȡ���ƻ�ϵ�� */
            while ((OAL_PTR_NULL != pc_token) && (uc_idx < MAC_2G_CHANNEL_NUM))
            {
                puc_dyn_cali_param->ac_dp_init[uc_idx] = (oal_int8)oal_strtol(pc_token, OAL_PTR_NULL, 10);
                pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
                uc_idx++;
            }
        }
        else
        {
            OAL_MEMZERO(puc_dyn_cali_param->ac_dp_init, MAC_2G_CHANNEL_NUM*OAL_SIZEOF(oal_int8));
        }
#endif //#ifdef _PRE_WLAN_DPINIT_CALI

        /* DPN */
        for (uc_idx = 0; uc_idx < MAC_2G_CHANNEL_NUM; uc_idx++)
        {
            l_val = hwifi_get_init_value_etc(CUS_TAG_DTS, uc_cfg_id + uc_idx);
            oal_memcopy(puc_dyn_cali_param->ac_dy_cali_2g_dpn_params[uc_idx], &l_val, CUS_DY_CALI_DPN_PARAMS_NUM*OAL_SIZEOF(oal_int8));
        }
        uc_cfg_id += MAC_2G_CHANNEL_NUM;

        for (uc_idx = HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_11B; uc_idx  <= HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_OFDM40; uc_idx++)
        {
            /* ��ȡ���߼���DPNֵ���� */
            puc_cust_nvram_info = hwifi_get_nvram_param(uc_dpn_2g_nv_id);
            uc_dpn_2g_nv_id++;

            if (0 == OAL_STRLEN(puc_cust_nvram_info))
            {
                continue;
            }

            OAL_MEMZERO(auc_nv_pa_params, OAL_SIZEOF(auc_nv_pa_params));
            oal_memcopy(auc_nv_pa_params, puc_cust_nvram_info, OAL_STRLEN(puc_cust_nvram_info));
            pc_token = oal_strtok(auc_nv_pa_params, pc_end, &pc_ctx);
            pc_token = oal_strtok(pc_token, pc_sep, &pc_ctx);
            uc_num_idx = 0;
            while ((OAL_PTR_NULL != pc_token))
            {
                if (uc_num_idx >= MAC_2G_CHANNEL_NUM)
                {
                    uc_num_idx++;
                    break;
                }
                l_val = oal_strtol(pc_token, OAL_PTR_NULL, 10) / 10;
                pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
                if ((l_val > CUS_DY_CALI_2G_VAL_DPN_MAX) || (l_val < CUS_DY_CALI_2G_VAL_DPN_MIN))
                {
                    OAM_ERROR_LOG3(0, OAM_SF_CUSTOM, "{hwifi_cfg_init_cus_dyn_cali::nvram 2g dpn val[%d] is unexpected uc_idx[%d] uc_num_idx[%d}!}",
                                   l_val, uc_idx, uc_num_idx);
                    l_val = 0;
                }
                ac_dpn_nv[uc_idx][uc_num_idx] = (oal_int8)l_val;
                uc_num_idx++;
            }

            if (MAC_2G_CHANNEL_NUM != uc_num_idx)
            {
                OAM_ERROR_LOG2(0, OAM_SF_CUSTOM, "{hwifi_cfg_init_cus_dyn_cali::nvram 2g dpn num is unexpected uc_id[%d] rf[%d}!}",
                               uc_idx, uc_rf_idx);
                continue;
            }

            for (uc_num_idx = 0; uc_num_idx < MAC_2G_CHANNEL_NUM; uc_num_idx++)
            {
                puc_dyn_cali_param->ac_dy_cali_2g_dpn_params[uc_num_idx][uc_idx] += ac_dpn_nv[uc_idx][uc_num_idx];
            }
        }

        for (uc_idx = 0; uc_idx < CUS_DY_CALI_NUM_5G_BAND; uc_idx++)
        {
            l_val = hwifi_get_init_value_etc(CUS_TAG_DTS, uc_cfg_id + uc_idx);
            oal_memcopy(puc_dyn_cali_param->ac_dy_cali_5g_dpn_params[uc_idx], &l_val, CUS_DY_CALI_DPN_PARAMS_NUM*OAL_SIZEOF(oal_int8));
        }
        uc_cfg_id += DY_CALI_NUM_5G_BAND;

        /* 5G 160M DPN */
        puc_cust_nvram_info = hwifi_get_nvram_param(uc_dpn_5g_nv_id);
        uc_dpn_5g_nv_id++;
        if (OAL_STRLEN(puc_cust_nvram_info))
        {
            OAL_MEMZERO(auc_nv_pa_params, OAL_SIZEOF(auc_nv_pa_params));
            oal_memcopy(auc_nv_pa_params, puc_cust_nvram_info, OAL_STRLEN(puc_cust_nvram_info));
            pc_token = oal_strtok(auc_nv_pa_params, pc_end, &pc_ctx);
            pc_token = oal_strtok(pc_token, pc_sep, &pc_ctx);
            uc_num_idx = 0;
            while ((OAL_PTR_NULL != pc_token))
            {
                if (uc_num_idx >= OAL_5G_160M_CHANNEL_NUM)
                {
                    uc_num_idx++;
                    break;
                }
                l_val = oal_strtol(pc_token, OAL_PTR_NULL, 10) / 10;
                pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
                if ((l_val > CUS_DY_CALI_5G_VAL_DPN_MAX) || (l_val < CUS_DY_CALI_5G_VAL_DPN_MIN))
                {
                    OAM_ERROR_LOG3(0, OAM_SF_CUSTOM, "{hwifi_cfg_init_cus_dyn_cali::nvram 2g dpn val[%d] is unexpected uc_idx[%d] uc_num_idx[%d}!}",
                                   l_val, uc_idx, uc_num_idx);
                    l_val = 0;
                }
                ac_dpn_5g_nv[uc_num_idx] = (oal_int8)l_val;
                uc_num_idx++;
            }

            if (OAL_5G_160M_CHANNEL_NUM != uc_num_idx)
            {
                OAM_ERROR_LOG2(0, OAM_SF_CUSTOM, "{hwifi_cfg_init_cus_dyn_cali::nvram 2g dpn num is unexpected uc_id[%d] rf[%d}!}",
                               uc_idx, uc_rf_idx);
                continue;
            }
            /* 5250  5570 */
            for (uc_num_idx = 0; uc_num_idx < OAL_5G_160M_CHANNEL_NUM; uc_num_idx++)
            {
                puc_dyn_cali_param->ac_dy_cali_5g_dpn_params[uc_num_idx+1][WLAN_BW_CAP_160M] += ac_dpn_5g_nv[uc_num_idx];
            }
        }

        puc_dyn_cali_param++;
    }

    return OAL_SUCC;
}
#endif

#else  //#if defined(_PRE_PRODUCT_ID_HI110X_HOST)

OAL_STATIC oal_uint32 hwifi_cfg_front_end(oal_uint8 *puc_param)
{
    mac_cfg_customize_rf*       pst_customize_rf;
    oal_uint8                   uc_idx;                 /*�ṹ�������±�*/
    oal_int32                   l_mult4;
    oal_int32                   l_mult10;

    pst_customize_rf = (mac_cfg_customize_rf *)puc_param;

    /* ����: 2g rf */
    for (uc_idx = 0; uc_idx < MAC_NUM_2G_BAND; ++uc_idx)
    {
        /* ��ȡ��2p4g ��band 0.25db��0.1db���ȵ�����ֵ */
        l_mult4  = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND_START + 2*uc_idx);
        l_mult10 = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND_START + 2*uc_idx + 1);

        if(l_mult4  >= RF_LINE_TXRX_GAIN_DB_2G_MIN && l_mult4  <= 0 &&
           l_mult10 >= RF_LINE_TXRX_GAIN_DB_2G_MIN && l_mult10 <= 0)
        {
            pst_customize_rf->st_rf_gain_db_rf.ac_gain_db_2g[uc_idx].c_rf_gain_db_mult4  = (oal_int8)l_mult4;
            pst_customize_rf->st_rf_gain_db_rf.ac_gain_db_2g[uc_idx].c_rf_gain_db_mult10 = (oal_int8)l_mult10;
        }
        else
        {
            /* ֵ������Ч��Χ */
            OAM_ERROR_LOG3(0, OAM_SF_CFG, "{hwifi_cfg_front_end::ini_id[%d]value out of range, 2g mult4[0x%0x}mult10[0x%0x]!}\r\n",
                          WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND_START + 2*uc_idx, l_mult4, l_mult10);
            return OAL_FAIL;
        }
    }

    if (OAL_TRUE == mac_device_check_5g_enable_per_chip())
    {
        /* ����: 5g rf */
        /* ����: fem�ڵ����߿ڵĸ����� */
        for (uc_idx = 0; uc_idx < MAC_NUM_5G_BAND; ++uc_idx)
        {
            /* ��ȡ��5g ��band 0.25db��0.1db���ȵ�����ֵ */
            l_mult4  = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND_START + 2*uc_idx);
            l_mult10 = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND_START + 2*uc_idx + 1);

            if(l_mult4 <= 0 && l_mult10 <= 0)
            {
                pst_customize_rf->st_rf_gain_db_rf.ac_gain_db_5g[uc_idx].c_rf_gain_db_mult4  = (oal_int8)l_mult4;
                pst_customize_rf->st_rf_gain_db_rf.ac_gain_db_5g[uc_idx].c_rf_gain_db_mult10 = (oal_int8)l_mult10;
            }
            else
            {
                /* ֵ������Ч��Χ */
                OAM_ERROR_LOG3(0, OAM_SF_CFG, "{hwifi_cfg_front_end::ini_id[%d]value out of range, 5g mult4[0x%0x}mult10[0x%0x]!}\r\n",
                          WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND_START + 2*uc_idx, l_mult4, l_mult10);
                return OAL_FAIL;
            }
        }


        /*5g �ⲿfem */
        pst_customize_rf->st_ext_rf.c_lna_bypass_gain_db    = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LINE_RX_GAIN_DB_5G);
        pst_customize_rf->st_ext_rf.c_lna_gain_db           = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_LNA_GAIN_DB_5G);
        pst_customize_rf->st_ext_rf.c_pa_gain_b0_db    = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_LINE_TX_GAIN_DB_5G);
        pst_customize_rf->st_ext_rf.uc_ext_switch_isexist   = (oal_uint8)!!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_5G);
        pst_customize_rf->st_ext_rf.uc_ext_pa_isexist       = (oal_uint8)!!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_PA_ISEXIST_5G);
        pst_customize_rf->st_ext_rf.uc_ext_lna_isexist      = (oal_uint8)!!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_EXT_LNA_ISEXIST_5G);
        pst_customize_rf->st_ext_rf.us_lna_on2off_time_ns   = (oal_uint16)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_5G);
        pst_customize_rf->st_ext_rf.us_lna_off2on_time_ns   = (oal_uint16)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_5G);
        if (pst_customize_rf->st_ext_rf.c_lna_bypass_gain_db < RF_LINE_TXRX_GAIN_DB_5G_MIN ||
            pst_customize_rf->st_ext_rf.c_lna_bypass_gain_db > 0 ||
            pst_customize_rf->st_ext_rf.c_pa_gain_b0_db < RF_LINE_TXRX_GAIN_DB_5G_MIN ||
            pst_customize_rf->st_ext_rf.c_pa_gain_b0_db > 0 ||
            pst_customize_rf->st_ext_rf.c_lna_gain_db < LNA_GAIN_DB_MIN ||
            pst_customize_rf->st_ext_rf.c_lna_gain_db > LNA_GAIN_DB_MAX)
        {
            /* ֵ������Ч��Χ */
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hwifi_cfg_front_end::gain db rx or tx 5g lna gain db or out of range!}\r\n");
            return OAL_FAIL;
        }
    }

    pst_customize_rf->uc_far_dist_pow_gain_switch = (oal_uint8)!!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_FAR_DIST_POW_GAIN_SWITCH);
    pst_customize_rf->uc_far_dist_dsss_scale_promote_switch = (oal_uint8)!!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_FAR_DIST_DSSS_SCALE_PROMOTE_SWITCH);

    /* ����: cca�������޵���ֵ */
    {
        oal_int8 c_delta_cca_ed_high_20th_2g = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_2G);
        oal_int8 c_delta_cca_ed_high_40th_2g = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_2G);
        oal_int8 c_delta_cca_ed_high_20th_5g = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_5G);
        oal_int8 c_delta_cca_ed_high_40th_5g = (oal_int8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_5G);

        /* ���ÿһ��ĵ��������Ƿ񳬳�������� */
        if (CUS_DELTA_CCA_ED_HIGH_TH_OUT_OF_RANGE(c_delta_cca_ed_high_20th_2g)
           || CUS_DELTA_CCA_ED_HIGH_TH_OUT_OF_RANGE(c_delta_cca_ed_high_40th_2g)
           || CUS_DELTA_CCA_ED_HIGH_TH_OUT_OF_RANGE(c_delta_cca_ed_high_20th_5g)
           || CUS_DELTA_CCA_ED_HIGH_TH_OUT_OF_RANGE(c_delta_cca_ed_high_40th_5g))
        {
            OAM_ERROR_LOG4(0, OAM_SF_ANY, "{hwifi_cfg_front_end::one or more delta cca ed high threshold out of range \
                 [delta_20th_2g=%d, delta_40th_2g=%d, delta_20th_5g=%d, delta_40th_5g=%d], please check the value!}",
                                            c_delta_cca_ed_high_20th_2g,
                                            c_delta_cca_ed_high_40th_2g,
                                            c_delta_cca_ed_high_20th_5g,
                                            c_delta_cca_ed_high_40th_5g);//lint !e571
            /* set 0 */
            pst_customize_rf->c_delta_cca_ed_high_20th_2g = 0;
            pst_customize_rf->c_delta_cca_ed_high_40th_2g = 0;
            pst_customize_rf->c_delta_cca_ed_high_20th_5g = 0;
            pst_customize_rf->c_delta_cca_ed_high_40th_5g = 0;
        }
        else
        {
            pst_customize_rf->c_delta_cca_ed_high_20th_2g = c_delta_cca_ed_high_20th_2g;
            pst_customize_rf->c_delta_cca_ed_high_40th_2g = c_delta_cca_ed_high_40th_2g;
            pst_customize_rf->c_delta_cca_ed_high_20th_5g = c_delta_cca_ed_high_20th_5g;
            pst_customize_rf->c_delta_cca_ed_high_40th_5g = c_delta_cca_ed_high_40th_5g;
        }
    }

    return OAL_SUCC;
}
#endif //#if defined(_PRE_PRODUCT_ID_HI110X_HOST)

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)

OAL_STATIC oal_uint32 hwifi_cfg_init_tx_pwr_cmp(oal_uint8 *puc_param)
{
    oal_uint8                           uc_level;                                               /* �������ʵĵ�λ:0-2 */
    oal_uint8                           uc_tx_pwr_comp_base = WLAN_CFG_INIT_TX_RATIO_LEVEL_0;   /* ���ʲ�������ʼIDֵ */
    mac_cfg_customize_tx_pwr_comp_stru* pst_tx_pwr_comp;

    /* ����: ���ʲ��� */
    pst_tx_pwr_comp = (mac_cfg_customize_tx_pwr_comp_stru*)(puc_param);

    for (uc_level = 0; uc_level < 3; ++uc_level)
    {
        pst_tx_pwr_comp->ast_txratio2pwr[uc_level].us_tx_ratio = (oal_uint16)hwifi_get_init_value_etc(CUS_TAG_INI, uc_tx_pwr_comp_base + 2*uc_level);

        /* �ж�txռ�ձ��Ƿ���Ч */
        if (pst_tx_pwr_comp->ast_txratio2pwr[uc_level].us_tx_ratio > TX_RATIO_MAX)
        {
            OAM_ERROR_LOG2(0, OAM_SF_CFG, "{hwifi_cfg_init_tx_pwr_cmp::d[%d]:tx_ratio[%d] out of range, please check the value!}\r\n",
                                        uc_tx_pwr_comp_base + 2*uc_level, pst_tx_pwr_comp->ast_txratio2pwr[uc_level].us_tx_ratio);
            /* ֵ������Ч��Χ */
            return OAL_FAIL;
        }

        pst_tx_pwr_comp->ast_txratio2pwr[uc_level].us_tx_pwr_comp_val = (oal_uint16)hwifi_get_init_value_etc(CUS_TAG_INI, uc_tx_pwr_comp_base + 2*uc_level + 1);

        /* �жϷ��书�ʲ���ֵ�Ƿ���Ч */
        if (pst_tx_pwr_comp->ast_txratio2pwr[uc_level].us_tx_pwr_comp_val > TX_PWR_COMP_VAL_MAX)
        {
            OAM_ERROR_LOG2(0, OAM_SF_CFG, "{hwifi_config_init_ini_rf::id[%d] tx_pwr_comp_val[%d] out of range!}\r\n",
                                            uc_tx_pwr_comp_base + 2*uc_level + 1, pst_tx_pwr_comp->ast_txratio2pwr[uc_level].us_tx_pwr_comp_val);
            /* ֵ������Ч��Χ */
            return OAL_FAIL;
        }
    }

    pst_tx_pwr_comp->ul_more_pwr = (oal_uint32)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_MORE_PWR);

    /* �жϸ����¶ȶ��ⲹ���ķ��书��ֵ�Ƿ���Ч */
    if (pst_tx_pwr_comp->ul_more_pwr > MORE_PWR_MAX)
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG, "{hwifi_config_init_ini_rf::cfg_id[%d]:more_pwr[%d] out of range!}\r\n",
                                            WLAN_CFG_INIT_MORE_PWR, pst_tx_pwr_comp->ul_more_pwr);
        /* ֵ������Ч��Χ */
        return OAL_FAIL;
    }

    return OAL_SUCC;
 }


OAL_STATIC oal_uint32 hwifi_cfg_init_dts_rf_reg(oal_uint16   *pus_rf_reg)
{
    oal_uint32                  ul_cfg_id;
    oal_uint16                  us_reg_val;

    /* ����RF�Ĵ��� */
    for(ul_cfg_id = WLAN_CFG_DTS_RF_FIRST; ul_cfg_id <= WLAN_CFG_DTS_RF_LAST; ul_cfg_id++)
    {
         us_reg_val = (oal_uint16)hwifi_get_init_value_etc(CUS_TAG_DTS, ul_cfg_id);
        *(pus_rf_reg + ul_cfg_id - WLAN_CFG_DTS_RF_FIRST) = us_reg_val;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hwifi_cfg_init_dts_cus_bt_cali(oal_uint8 *puc_param)
{
    oal_uint32                  ul_cfg_id;
    oal_int32                   l_cfg_value;            /* ��ʱ��������Ӷ��ƻ�get�ӿ��л�ȡ��ֵ */
    oal_int16                  *ps_tmp;                 /* ����ָ�룬���Ա���wifi btУ׼ֵ */
    mac_cus_bt_cali_stru       *pst_cali_bt;

    pst_cali_bt = (mac_cus_bt_cali_stru *)puc_param;

    /* ����: �������ȵ�λ */
    pst_cali_bt->uc_bt_tone_amp_grade = (oal_uint8)hwifi_get_init_value_etc(CUS_TAG_DTS, WLAN_CFG_DTS_BT_CALI_TONE_AMP_GRADE);

    /* bt У׼ֵ */
    ps_tmp = &(pst_cali_bt->us_cali_bt_txpwr_pa_ref_band1);
    for(ul_cfg_id = WLAN_CFG_DTS_BT_CALI_TXPWR_PA_REF_BAND1; ul_cfg_id <= WLAN_CFG_DTS_BT_CALI_TXPWR_PA_REF_BAND8; ul_cfg_id++)
    {
        l_cfg_value = hwifi_get_init_value_etc(CUS_TAG_DTS, ul_cfg_id);
        if(CALI_TXPWR_PA_DC_REF_MIN <= l_cfg_value && CALI_BT_TXPWR_PA_DC_REF_MAX >= l_cfg_value)
        {
            *ps_tmp = (oal_uint16)l_cfg_value;
             ps_tmp++;
        }
        else
        {
            /* ֵ������Ч��Χ */
            OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hwifi_cfg_init_dts_cus_bt_cali::dts id[%d]value[%d]out of range!}\r\n", ul_cfg_id, l_cfg_value);
            return OAL_FAIL;
        }
    }

    /* bt Ƶ��ֵ */
    ps_tmp = &(pst_cali_bt->us_cali_bt_txpwr_numb);
    for(ul_cfg_id = WLAN_CFG_DTS_BT_CALI_TXPWR_PA_NUM; ul_cfg_id <= WLAN_CFG_DTS_BT_CALI_TXPWR_PA_FRE8; ul_cfg_id++)
    {
        l_cfg_value = hwifi_get_init_value_etc(CUS_TAG_DTS, ul_cfg_id);
        if(CALI_TXPWR_PA_DC_FRE_MIN <= l_cfg_value && CALI_TXPWR_PA_DC_FRE_MAX >= l_cfg_value)
        {
            *ps_tmp = (oal_uint16)l_cfg_value;
             ps_tmp++;
        }
        else
        {
            /* ֵ������Ч��Χ */
            OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hwifi_cfg_init_dts_cus_bt_cali::dts id[%d]value[%d]out of range!}\r\n", ul_cfg_id, l_cfg_value);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}
#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)


OAL_STATIC oal_void hwifi_config_init_ini_rf(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru          st_write_msg;
    oal_uint32                  ul_ret;
    oal_uint16                  us_event_len = OAL_SIZEOF(mac_cfg_customize_rf);

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    oal_uint32                  ul_offset = 0;

    us_event_len += OAL_SIZEOF(mac_cfg_customize_tx_pwr_comp_stru);
#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_RF, us_event_len);

    /*lint -e774*/
    /* ���ƻ��·����ܳ����¼��ڴ泤 */
    if (us_event_len > WAL_MSG_WRITE_MAX_LEN)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::event size[%d] larger than msg size[%d]!}\r\n",
                            us_event_len, WAL_MSG_WRITE_MAX_LEN);
        return;
    }
    /*lint +e774*/

    /*  */
    ul_ret = hwifi_cfg_front_end(st_write_msg.auc_value);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::front end rf wrong value, not send cfg!}\r\n");
        return;
    }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    ul_offset += OAL_SIZEOF(mac_cfg_customize_rf);

    /* �����¶ȹ��ʲ��� */
    ul_ret = hwifi_cfg_init_tx_pwr_cmp(st_write_msg.auc_value + ul_offset);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::tx pwr cmp wrong value, not send cfg!}\r\n");
        return;
    }
#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)

    /* ������в���������Ч��Χ�ڣ����·�����ֵ */
    ul_ret = (oal_uint32)wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_event_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::EVENT[wal_send_cfg_event_etc] failed, return err code [%d]!}\r\n", ul_ret);
    }
}


extern oal_bool_enum_uint8 g_en_fact_cali_completed;

OAL_STATIC oal_uint32 hwifi_cfg_init_dts_cus_cali(oal_uint8 *puc_param, oal_uint8 uc_5g_Band_enable)
{
    oal_int32                    l_val;
    oal_int16                    s_ref_val_ch1;
    oal_int16                    s_ref_val_ch0;
    oal_uint8                    uc_idx;             /* �ṹ�������±� */
    mac_cus_dts_cali_stru       *pst_cus_cali;
    oal_uint8                    uc_gm_opt;

    pst_cus_cali = (mac_cus_dts_cali_stru *)puc_param;
    /** ����: TXPWR_PA_DC_REF **/
    /*2G REF: ��13���ŵ�*/
    for (uc_idx = 0; uc_idx < 13; uc_idx++)
    {
        l_val = hwifi_get_init_value_etc(CUS_TAG_DTS, WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_START + uc_idx);
        s_ref_val_ch1 = (oal_int16)((l_val & 0xFFFF0000) >> 16);//lint !e737
        s_ref_val_ch0 = (oal_int16)(l_val & 0x0000FFFF);

        /* 2G�жϲο�ֵ�Ȳ��ж�<0, ����RFͬ��ȷ��, TBD */
        if (s_ref_val_ch0 <= CALI_TXPWR_PA_DC_REF_MAX)
        {
            pst_cus_cali->ast_cali[0].aus_cali_txpwr_pa_dc_ref_2g_val_chan[uc_idx] = s_ref_val_ch0;
        }
        else
        {
            /* ֵ������Ч��Χ */
            OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hwifi_cfg_init_dts_cus_cali::dts 2g ref id[%d]value[%d] out of range!}\r\n",
                            WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_START + uc_idx, s_ref_val_ch0);//lint !e571
            return OAL_FAIL;
        }
#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1102_HOST)
        /* 02����Ҫ����˫ͨ�� */
        if (s_ref_val_ch1 <= CALI_TXPWR_PA_DC_REF_MAX)
        {
                pst_cus_cali->ast_cali[1].aus_cali_txpwr_pa_dc_ref_2g_val_chan[uc_idx] = s_ref_val_ch1;
        }
        else
        {
            /* ֵ������Ч��Χ */
            OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hwifi_cfg_init_dts_cus_cali::dts ch1 2g ref id[%d]value[%d] out of range!}\r\n",
                             WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_START + uc_idx, s_ref_val_ch1);//lint !e571
            return OAL_FAIL;
        }

#endif //#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1102_HOST)

#if (_PRE_TARGET_PRODUCT_TYPE_E5 == _PRE_CONFIG_TARGET_PRODUCT)
        /* E5-5885 B40ͨ·��ֵ */
        l_val = hwifi_get_init_value_etc(CUS_TAG_DTS, WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_B40_START+uc_idx);
        s_ref_val_ch1 = (l_val & 0xFFFF0000) >> 16;
        s_ref_val_ch0 = l_val & 0x0000FFFF;
        if (s_ref_val_ch1 >= 0 && s_ref_val_ch1 <= CALI_TXPWR_PA_DC_REF_MAX
            && s_ref_val_ch0 >= 0 && s_ref_val_ch0 <= CALI_TXPWR_PA_DC_REF_MAX)
        {
            pst_cus_cali->ast_cali[0].aus_cali_txpwr_pa_dc_ref_2g_b40_val[uc_idx] = s_ref_val_ch0;
            pst_cus_cali->ast_cali[1].aus_cali_txpwr_pa_dc_ref_2g_b40_val[uc_idx] = s_ref_val_ch1;
        }
        else
        {
            /* ֵ������Ч��Χ */
            OAM_ERROR_LOG3(0, OAM_SF_ANY, "{hwifi_cfg_init_dts_cus_cali::dts 2g ref id[%d]value ch0[%d]ch1[%d] out of range!}\r\n",
                             WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_B40_START + uc_idx, s_ref_val_ch0, s_ref_val_ch1);
            return OAL_FAIL;
        }
#endif //#if (_PRE_TARGET_PRODUCT_TYPE_E5 == _PRE_CONFIG_TARGET_PRODUCT)
    }

    /*5G REF: ��7��band*/
    if (uc_5g_Band_enable)
    {
        for (uc_idx = 0; uc_idx < 7; ++uc_idx)
        {
            l_val = hwifi_get_init_value_etc(CUS_TAG_DTS, WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_START + uc_idx);
            s_ref_val_ch1 = (oal_int16)((l_val & 0xFFFF0000) >> 16);//lint !e737
            s_ref_val_ch0 = (oal_int16)(l_val & 0x0000FFFF);

            if (s_ref_val_ch0 >= 0 && s_ref_val_ch0 <= CALI_TXPWR_PA_DC_REF_MAX)
            {
                pst_cus_cali->ast_cali[0].aus_cali_txpwr_pa_dc_ref_5g_val_band[uc_idx] = s_ref_val_ch0;
            }
            else
            {
                /* ֵ������Ч��Χ */
                OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hwifi_cfg_init_dts_cus_cali::dts 5g ref id[%d]value[%d] out of range!}\r\n",
                                WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_START + uc_idx, s_ref_val_ch0);//lint !e571
                return OAL_FAIL;
            }
#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1102_HOST)
            if (s_ref_val_ch1 >= 0 && s_ref_val_ch1 <= CALI_TXPWR_PA_DC_REF_MAX)
            {
                    pst_cus_cali->ast_cali[1].aus_cali_txpwr_pa_dc_ref_5g_val_band[uc_idx] = s_ref_val_ch1;
            }
            else
            {
                /* ֵ������Ч��Χ */
                OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hwifi_cfg_init_dts_cus_cali::dts ch1 5g ref id[%d]value[%d] out of range!}\r\n",
                                WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_START + uc_idx, s_ref_val_ch1);//lint !e571
                return OAL_FAIL;
            }
#endif //#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1102_HOST)
        }
    }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    hwifi_cfg_init_dts_cus_bt_cali((oal_uint8 *)(&pst_cus_cali->st_cali_bt));
#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)

    /* ����BAND 5G ENABLE */
    pst_cus_cali->uc_band_5g_enable = !!uc_5g_Band_enable;

    /* ���õ������ȵ�λ */
    pst_cus_cali->uc_tone_amp_grade = (oal_uint8)hwifi_get_init_value_etc(CUS_TAG_DTS, WLAN_CFG_DTS_CALI_TONE_AMP_GRADE);

    /* ����DPDУ׼���� */
#ifdef _PRE_WLAN_ONLINE_DPD
    for (uc_idx = 0; uc_idx < MAC_DPD_CALI_CUS_PARAMS_NUM; uc_idx++)
    {
        /* ͨ��0 */
        l_val = hwifi_get_init_value_etc(CUS_TAG_DTS, WLAN_CFG_DTS_DPD_CALI_START + uc_idx);
        pst_cus_cali->ast_dpd_cali_para[0].aul_dpd_cali_cus_dts[uc_idx] = l_val;
        /* ͨ��1 */
        l_val = hwifi_get_init_value_etc(CUS_TAG_DTS, WLAN_CFG_DTS_DPD_CALI_START + uc_idx + MAC_DPD_CALI_CUS_PARAMS_NUM);
        pst_cus_cali->ast_dpd_cali_para[1].aul_dpd_cali_cus_dts[uc_idx] = l_val;
    }
#endif

    /* ���ö�̬У׼���� */
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    l_val = hwifi_get_init_value_etc(CUS_TAG_DTS, WLAN_CFG_DTS_DYN_CALI_DSCR_ITERVL);
    pst_cus_cali->aus_dyn_cali_dscr_interval[WLAN_BAND_2G] = (oal_uint16)(l_val & 0x0000FFFF);

    if (uc_5g_Band_enable)
    {
        pst_cus_cali->aus_dyn_cali_dscr_interval[WLAN_BAND_5G] = (oal_uint16)((l_val & 0xFFFF0000) >> 16);
    }

    l_val = hwifi_get_init_value_etc(CUS_TAG_DTS, WLAN_CFG_DTS_DYN_CALI_OPT_SWITCH);
    uc_gm_opt = (l_val & BIT2) >> NUM_1_BITS;

    if ((l_val & 0x3) >> 1)
    {
        /* ����Ӧѡ�� */
        l_val = !g_en_fact_cali_completed;
    }
    else
    {
        l_val &= BIT0;
    }

    pst_cus_cali->en_dyn_cali_opt_switch = l_val | uc_gm_opt;

    l_val = hwifi_get_init_value_etc(CUS_TAG_DTS, WLAN_CFG_DTS_DYN_CALI_GM0_DB10_AMEND);
    pst_cus_cali->as_gm0_dB10_amend[WLAN_RF_CHANNEL_ZERO] = (oal_int16)CUS_GET_LOW_16BIT(l_val);
    pst_cus_cali->as_gm0_dB10_amend[WLAN_RF_CHANNEL_ONE]  = (oal_int16)CUS_GET_HIGH_16BIT(l_val);
#endif //#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI

    return OAL_SUCC;
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

OAL_STATIC oal_uint32 hwifi_cfg_init_dts_fcc_band_edge_limit(oal_uint8 *puc_param, oal_uint8 uc_fcc_auth_band_num)
{
    mac_cus_band_edge_limit_stru   *pst_band_edge_limit;
    oal_uint8                       uc_max_txpwr;
    oal_uint16                      us_dbb_scale;
    oal_uint8                       uc_idx;             /* �ṹ�������±� */

    pst_band_edge_limit = (mac_cus_band_edge_limit_stru *)puc_param;

    for (uc_idx = 0; uc_idx < uc_fcc_auth_band_num; uc_idx++)
    {
        /* ��ֵidx��txpwr */
        uc_max_txpwr = (oal_uint8)hwifi_get_init_value_etc(CUS_TAG_DTS, WLAN_CFG_DTS_BAND_EDGE_LIMIT_TXPWR_START + uc_idx);
        if (uc_max_txpwr >= MAX_TXPOWER_MIN && uc_max_txpwr <= MAX_TXPOWER_MAX)
        {
            pst_band_edge_limit[uc_idx].uc_index       = uc_idx;
            pst_band_edge_limit[uc_idx].uc_max_txpower = uc_max_txpwr;
        }
        else
        {
            /* ֵ������Ч��Χ */
            OAM_ERROR_LOG2(0, OAM_SF_CFG, "{hwifi_cfg_init_dts_fcc_band_edge_limit::dts id[%d]value[%d]out of range!}\r\n",
                            WLAN_CFG_DTS_BAND_EDGE_LIMIT_TXPWR_START + uc_idx, uc_max_txpwr);
            return OAL_FAIL;
        }

        /* ��ֵscale */
        us_dbb_scale = (oal_uint16)hwifi_get_init_value_etc(CUS_TAG_DTS, WLAN_CFG_DTS_BAND_EDGE_LIMIT_SCALE_START + uc_idx);
        if (us_dbb_scale > 0 && us_dbb_scale <= MAX_DBB_SCALE)
        {
            pst_band_edge_limit[uc_idx].us_dbb_scale = us_dbb_scale;
        }
        else
        {
            /* ֵ������Ч��Χ */
            OAM_ERROR_LOG2(0, OAM_SF_CFG, "{hwifi_cfg_init_dts_fcc_band_edge_limit::dts id[%d]value[%d]out of range!}\r\n",
                            WLAN_CFG_DTS_BAND_EDGE_LIMIT_TXPWR_START + uc_idx, us_dbb_scale);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hwifi_config_init_dts_cali(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_ret;
    mac_cus_dts_cali_stru           st_cus_cali;
    mac_cus_band_edge_limit_stru   *pst_band_edge_limit;
    oal_uint16                      us_offset = 0;
    oal_uint8                       uc_fcc_auth_band_num;   /* ʵ����Ҫ�������õ�FCC��֤band����2g:3��,5g:3�� */
    oal_bool_enum_uint8             en_5g_Band_enable;    /* mac device�Ƿ�֧��5g���� */

    if (OAL_WARN_ON(OAL_PTR_NULL == pst_cfg_net_dev))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ���Ӳ���Ƿ���Ҫʹ��5g */
    en_5g_Band_enable = mac_device_check_5g_enable_per_chip();

    /* ����У׼����TXPWR_PA_DC_REF */
    ul_ret = hwifi_cfg_init_dts_cus_cali((oal_uint8 *)&st_cus_cali, en_5g_Band_enable);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{hwifi_config_init_dts_cali::init dts cus cali failed ret[%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /* ����FCC��֤ */
    uc_fcc_auth_band_num = en_5g_Band_enable ? NUM_OF_BAND_EDGE_LIMIT : NUM_OF_BAND_EDGE_LIMIT >> 1;
    /* �����ڴ��űߴ�������Ϣ,�������������ͷ�,�����ڴ��С: 6*4 = 24�ֽ� */
    pst_band_edge_limit = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, NUM_OF_BAND_EDGE_LIMIT * OAL_SIZEOF(mac_cus_band_edge_limit_stru), OAL_TRUE);
    if (OAL_PTR_NULL == pst_band_edge_limit)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hwifi_config_init_dts_cali::alloc fcc mem fail, return null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hwifi_cfg_init_dts_fcc_band_edge_limit((oal_uint8 *)pst_band_edge_limit, uc_fcc_auth_band_num);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{hwifi_config_init_dts_cali::init dts fcc band edge_limit failed ret[%d]!}\r\n", ul_ret);
        OAL_MEM_FREE(pst_band_edge_limit, OAL_TRUE);
        return ul_ret;
    }

    /* ������в���������Ч��Χ�ڣ����·�����ֵ */
    oal_memcopy(st_write_msg.auc_value, (oal_int8*)&st_cus_cali, OAL_SIZEOF(mac_cus_dts_cali_stru));
    us_offset += OAL_SIZEOF(mac_cus_dts_cali_stru);
    oal_memcopy(st_write_msg.auc_value + us_offset, (oal_int8*)pst_band_edge_limit, NUM_OF_BAND_EDGE_LIMIT * OAL_SIZEOF(mac_cus_band_edge_limit_stru));
    us_offset += (NUM_OF_BAND_EDGE_LIMIT * OAL_SIZEOF(mac_cus_band_edge_limit_stru));

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_DTS_CALI, us_offset);
    ul_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_offset,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);//lint !e732

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_dts_cali::wal_send_cfg_event_etc failed, ret[%d]!}\r\n", ul_ret);
        OAL_MEM_FREE(pst_band_edge_limit, OAL_TRUE);
        return ul_ret;
    }

    /* �ͷ�pst_band_edge_limit�ڴ� */
    OAL_MEM_FREE(pst_band_edge_limit, OAL_TRUE);
    return OAL_SUCC;
}


oal_uint32 hwifi_config_init_nvram_main_etc(oal_net_device_stru *pst_cfg_net_dev)
{
    /* nvram �����ṹ�� */
    struct nvram_params_stru
    {
        oal_uint8       uc_index;           /* �±��ʾƫ�� */
        oal_uint8       uc_max_txpower;     /* ����͹��� */
        oal_uint16      us_dbb_scale;       /* DBB��ֵ */
    };
    struct nvram_params_stru* pst_nvram_params;
    oal_uint16          aus_nv_params[NUM_OF_NV_PARAMS];
    oal_uint16          aus_d_value_txpwr_slave_params[WLAN_BAND_BUTT];
    oal_uint8           uc_idx;
    wal_msg_write_stru  st_write_msg;
    oal_int32           l_ret;
    oal_uint16          us_offset = NUM_OF_NV_MAX_TXPOWER * OAL_SIZEOF(struct nvram_params_stru);

    oal_memcopy(aus_nv_params, hwifi_get_nvram_params_etc(), OAL_SIZEOF(aus_nv_params));

    /* �����ڴ���NVRAM������Ϣ,�������������ͷ�,�ڴ��С:46 * 4 = 164�ֽ� */
    pst_nvram_params = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, us_offset, OAL_TRUE);
    if (OAL_PTR_NULL == pst_nvram_params)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{hwifi_config_init_nvram_main_etc::alloc nvram params mem[%d] fail!}\r\n", us_offset);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* �ṹ�����鸳ֵ */
    for(uc_idx = 0; uc_idx < NUM_OF_NV_MAX_TXPOWER; uc_idx++)
    {
        pst_nvram_params[uc_idx].uc_index = uc_idx;
        if (aus_nv_params[2 * uc_idx] > 0 && aus_nv_params[2 * uc_idx] <= MAX_TXPOWER_MAX &&
            aus_nv_params[2 * uc_idx + 1] <= MAX_DBB_SCALE)
        {
            pst_nvram_params[uc_idx].uc_max_txpower = (oal_uint8)aus_nv_params[2 * uc_idx];
            pst_nvram_params[uc_idx].us_dbb_scale = aus_nv_params[2 * uc_idx + 1];
        }
        else
        {
            OAM_ERROR_LOG3(0, OAM_SF_CFG, "{hwifi_config_init_nvram_main_etc:: %dth pwr[0x%x] or scale[0x%x], check dts file or nvm!}\r\n",
                            uc_idx, aus_nv_params[2 * uc_idx], aus_nv_params[2 * uc_idx + 1]);

            /* ������Чֵ��Χ��ʹ��Ĭ��ֵ */
            pst_nvram_params[uc_idx].uc_max_txpower = MAX_TXPOWER_MAX;
            pst_nvram_params[uc_idx].us_dbb_scale   = MAX_DBB_SCALE;
        }
    }

    /* d-value for slave parms */
    aus_d_value_txpwr_slave_params[WLAN_BAND_2G] = aus_nv_params[2 * uc_idx];
    aus_d_value_txpwr_slave_params[WLAN_BAND_5G] = aus_nv_params[2 * uc_idx + 1];

    OAM_INFO_LOG3(0, OAM_SF_CFG, "{hwifi_config_init_nvram_main_etc:: %dth D-value 2.4g[0x%x], 5g[0x%x]!}",
                    2 * uc_idx, aus_nv_params[2 * uc_idx], aus_nv_params[2 * uc_idx + 1]);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_NVRAM_PARAM, us_offset + OAL_SIZEOF(aus_d_value_txpwr_slave_params));
    oal_memcopy(st_write_msg.auc_value, (oal_void *)pst_nvram_params, us_offset);
    oal_memcopy((&st_write_msg.auc_value[0] + us_offset), (oal_void *)aus_d_value_txpwr_slave_params, OAL_SIZEOF(aus_d_value_txpwr_slave_params));

    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_offset + OAL_SIZEOF(aus_d_value_txpwr_slave_params),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_nvram_main_etc::return err code [%d]!}\r\n", l_ret);
        /* �ͷ�pst_nvram_params�ڴ� */
        OAL_MEM_FREE(pst_nvram_params, OAL_TRUE);
        return OAL_FAIL;
    }

    /* �ͷ�pst_nvram_params�ڴ� */
    OAL_MEM_FREE(pst_nvram_params, OAL_TRUE);

    return OAL_SUCC;
}

#else //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)


OAL_STATIC oal_uint32 hwifi_config_init_dts_cali(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_ret;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    mac_cus_dts_rf_reg              st_rf_reg;
#endif
    mac_cus_dts_cali_stru           st_cus_cali;
    oal_uint32                      ul_offset = 0;
    oal_bool_enum                   en_5g_Band_enable;    /* mac device�Ƿ�֧��5g���� */

    if (OAL_WARN_ON(OAL_PTR_NULL == pst_cfg_net_dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hwifi_config_init_dts_cali::pst_cfg_net_dev is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ���Ӳ���Ƿ���Ҫʹ��5g */
    en_5g_Band_enable = mac_device_check_5g_enable_per_chip();

    /* ����У׼����TXPWR_PA_DC_REF */
    ul_ret = hwifi_cfg_init_dts_cus_cali((oal_uint8 *)&st_cus_cali, en_5g_Band_enable);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{hwifi_config_init_dts_cali::init dts cus cali failed ret[%d]!}\r\n", ul_ret);
        return ul_ret;
    }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    /* ����RF�Ĵ��� */
    hwifi_cfg_init_dts_rf_reg((oal_uint16*)&st_rf_reg);
#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)

    /* ������в���������Ч��Χ�ڣ����·�����ֵ */
    oal_memcopy(st_write_msg.auc_value, (oal_int8*)&st_cus_cali, OAL_SIZEOF(mac_cus_dts_cali_stru));
    ul_offset += OAL_SIZEOF(mac_cus_dts_cali_stru);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    oal_memcopy(st_write_msg.auc_value + ul_offset, (oal_int8*)&st_rf_reg, OAL_SIZEOF(mac_cus_dts_rf_reg));
    ul_offset += OAL_SIZEOF(mac_cus_dts_rf_reg);
#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_DTS_CALI, ul_offset);
    ul_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + ul_offset,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_dts_cali::wal_send_cfg_event_etc failed, ret[%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    OAM_WARNING_LOG0(0, OAM_SF_CFG, "{hwifi_config_init_dts_cali::wal_send_cfg_event send succ}");

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI

OAL_STATIC oal_uint32 hwifi_config_init_cus_dyn_cali(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_ret;
    oal_uint32                      ul_offset = 0;
    mac_cus_dy_cali_param_stru      st_dy_cus_cali[WLAN_RF_CHANNEL_NUMS];
    oal_uint8                       uc_rf_id;
    mac_cus_dy_cali_param_stru      *pst_dy_cus_cali;
    wal_msg_stru                    *pst_rsp_msg;
    wal_msg_write_rsp_stru          *pst_write_rsp_msg;

    if (OAL_WARN_ON(OAL_PTR_NULL == pst_cfg_net_dev))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ���ö�̬У׼����TXPWR_PA_DC_REF */
    OAL_MEMZERO(st_dy_cus_cali, OAL_SIZEOF(mac_cus_dy_cali_param_stru)*WLAN_RF_CHANNEL_NUMS);
    ul_ret = hwifi_cfg_init_cus_dyn_cali(st_dy_cus_cali);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hwifi_config_init_cus_dyn_cali::init cus dyn cali failed ret[%d]!}", ul_ret);
        return ul_ret;
    }

    for (uc_rf_id = 0; uc_rf_id < WLAN_RF_CHANNEL_NUMS; uc_rf_id++)
    {
        pst_dy_cus_cali = &st_dy_cus_cali[uc_rf_id];
        pst_rsp_msg     = OAL_PTR_NULL;

        /* ������в���������Ч��Χ�ڣ����·�����ֵ */
        oal_memcopy(st_write_msg.auc_value, (oal_int8*)pst_dy_cus_cali, OAL_SIZEOF(mac_cus_dy_cali_param_stru));
        ul_offset = OAL_SIZEOF(mac_cus_dy_cali_param_stru);
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_DYN_CALI_PARAM, ul_offset);
        ul_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + ul_offset,
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_TRUE,
                                   &pst_rsp_msg);

        if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_cus_dyn_cali::wal_send_cfg_event_etc failed, ret[%d]!}", ul_ret);
            return ul_ret;
        }

        if(OAL_PTR_NULL != pst_rsp_msg)
        {
            pst_write_rsp_msg = (wal_msg_write_rsp_stru *)(pst_rsp_msg->auc_msg_data);

            if (OAL_SUCC != pst_write_rsp_msg->ul_err_code)
            {
                OAM_ERROR_LOG2(0, OAM_SF_SCAN, "{wal_check_and_release_msg_resp_etc::detect err code:[%u],wid:[%u]}",
                                pst_write_rsp_msg->ul_err_code, pst_write_rsp_msg->en_wid);
                oal_free(pst_rsp_msg);
                return OAL_FAIL;
            }

            oal_free(pst_rsp_msg);
        }
    }

    return OAL_SUCC;
}
#endif


oal_uint32 hwifi_config_init_nvram_main_etc(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru  st_write_msg;
    oal_int32           l_ret;
    oal_uint16          us_offset = OAL_SIZEOF(wlan_cust_nvram_params); /* ����4����׼���� */

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_NVRAM_PARAM, us_offset);
    oal_memcopy(st_write_msg.auc_value, hwifi_get_nvram_params_etc(), us_offset);
    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_offset,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_nvram_main_etc::return err code [%d]!}\r\n", l_ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)


OAL_STATIC oal_void hwifi_config_init_ini_main(oal_net_device_stru *pst_cfg_net_dev)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* ������ */
    hwifi_config_init_ini_country(pst_cfg_net_dev);
#endif
    /* �͹��� */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    hwifi_config_init_ini_pm(pst_cfg_net_dev);
#endif //#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    /* ��ά�ɲ� */
    hwifi_config_init_ini_log(pst_cfg_net_dev);
    /* RF */
    hwifi_config_init_ini_rf(pst_cfg_net_dev);
#ifdef _PRE_WLAN_FEATURE_SMARTANT
    hwifi_config_init_ini_dual_antenna(pst_cfg_net_dev);
#endif
}


oal_uint32 hwifi_config_init_dts_main_etc(oal_net_device_stru *pst_cfg_net_dev)
{
    oal_uint32 ul_ret = OAL_SUCC;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    /* �·���̬У׼���� */
    hwifi_config_init_cus_dyn_cali(pst_cfg_net_dev);
#endif
#endif

    /* У׼ */
    if (OAL_SUCC != hwifi_config_init_dts_cali(pst_cfg_net_dev))
    {
        return OAL_FAIL;
    }
    /* У׼�ŵ���һ������ */
    return ul_ret;
}


OAL_STATIC oal_int32 hwifi_config_init_ini_wlan(oal_net_device_stru *pst_net_dev)
{
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_P2P

OAL_STATIC oal_int32 hwifi_config_init_ini_p2p(oal_net_device_stru *pst_net_dev)
{
    return OAL_SUCC;
}
#endif


oal_int32 hwifi_config_init_ini_etc(oal_net_device_stru *pst_net_dev)
{
    oal_net_device_stru        *pst_cfg_net_dev;
    mac_vap_stru               *pst_mac_vap;
    oal_wireless_dev_stru      *pst_wdev;
    mac_wiphy_priv_stru        *pst_wiphy_priv;
    hmac_vap_stru              *pst_cfg_hmac_vap;
    mac_device_stru            *pst_mac_device;
    oal_int8                    ac_wlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    oal_int8                    ac_p2p_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];

    if(OAL_PTR_NULL== pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini_etc::pst_net_dev is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_PTR_NULL== pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini_etc::pst_mac_vap is null}\r\n");
        return -OAL_EINVAL;
    }

    pst_wdev = OAL_NETDEVICE_WDEV(pst_net_dev);
    if(OAL_PTR_NULL == pst_wdev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini_etc::pst_wdev is null!}\r\n");
        return -OAL_EFAUL;
    }

    pst_wiphy_priv  = (mac_wiphy_priv_stru *)oal_wiphy_priv(pst_wdev->wiphy);
    if (OAL_PTR_NULL == pst_wiphy_priv)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini_etc::pst_wiphy_priv is null!}\r\n");
        return -OAL_EFAUL;
    }
    pst_mac_device  = pst_wiphy_priv->pst_mac_device;
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini_etc::pst_mac_device is null!}\r\n");
        return -OAL_EFAUL;
    }

    pst_cfg_hmac_vap= (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->uc_cfg_vap_id);
    if (OAL_PTR_NULL == pst_cfg_hmac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini_etc::pst_cfg_hmac_vap is null, vap_id:%d!}\r\n",pst_mac_device->uc_cfg_vap_id);
        return -OAL_EFAUL;
    }

    pst_cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;

    if(OAL_PTR_NULL == pst_cfg_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini_etc::pst_cfg_net_dev is null!}\r\n");
        return -OAL_EFAUL;
    }

    OAL_SPRINTF(ac_wlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "wlan%d", pst_mac_device->uc_device_id);
    OAL_SPRINTF(ac_p2p_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "p2p%d", pst_mac_device->uc_device_id);

#ifdef _PRE_WLAN_FEATURE_P2P
    if((NL80211_IFTYPE_STATION == pst_wdev->iftype) || (NL80211_IFTYPE_P2P_DEVICE == pst_wdev->iftype) || (NL80211_IFTYPE_AP == pst_wdev->iftype))
#else
    if((NL80211_IFTYPE_STATION == pst_wdev->iftype) || (NL80211_IFTYPE_AP == pst_wdev->iftype))
#endif
    {
        if(!g_uc_cfg_once_flag)
        {
            hwifi_config_init_nvram_main_etc(pst_cfg_net_dev);
            hwifi_config_init_ini_main(pst_cfg_net_dev);
            g_uc_cfg_once_flag = OAL_TRUE;
        }
        if(0 == (oal_strcmp(ac_wlan_netdev_name, pst_net_dev->name)))
        {
            hwifi_config_init_ini_wlan(pst_net_dev);
        }
#ifdef _PRE_WLAN_FEATURE_P2P
        else if(0 == (oal_strcmp(ac_p2p_netdev_name, pst_net_dev->name)))
        {
            hwifi_config_init_ini_p2p(pst_net_dev);
        }
#endif
        else
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini_etc::net_dev is not wlan0 or p2p0!}\r\n");
            return OAL_SUCC;
        }
    }

    return OAL_SUCC;
}

oal_void hwifi_config_init_force_etc(oal_void)
{
    /* �����ϵ�ʱ��ΪFALSE */
    g_uc_cfg_once_flag = OAL_FALSE;

    hwifi_config_host_global_ini_param();
}


OAL_STATIC oal_uint32  wal_hipriv_load_ini_power_gain(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;
    oal_uint16                      us_len;

    oal_memcopy(st_write_msg.auc_value, pc_param, OAL_STRLEN(pc_param));

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);

    /* ˢ��txpwr��dbb scale */
    if (hwifi_force_refresh_rf_params(pst_net_dev) != OAL_SUCC)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_refresh_ini_power_gain::refresh rf(max_txpwr & dbb scale) params failed!}");
        return OAL_FAIL;
    }

    /* ˢ��DTS���� */
    if (hwifi_config_init_etc(CUS_TAG_DTS))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_refresh_ini_power_gain::refresh CUS_TAG_DTS params failed!}");
        return OAL_FAIL;
    }


    /***************************************************************************
                              ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_LOAD_INI_PWR_GAIN, us_len);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                             (oal_uint8 *)&st_write_msg,
                             OAL_FALSE,
                             OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_refresh_ini_power_gain::wal_send_cfg_event_etc return err_code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

/* E5 SPE module ralation */
#if (defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT))


OAL_STATIC oal_int32 wal_finish_spe_td(oal_int32 l_port_num, oal_netbuf_stru *pst_buf, oal_uint32 ul_flags)
{
    //oal_net_device_stru *pst_net_dev;

    //pst_net_dev = spe_hook.port_netdev(l_port_num);
    if(OAL_WARN_ON(!spe_hook.port_netdev))
    {
         oal_print_hi11xx_log(HI11XX_LOG_ERR,"%s error: spe_hook.port_netdev is null",__FUNCTION__);
         return OAL_FAIL;
    };

    oal_dma_unmap_single(NULL, pst_buf->dma, pst_buf->len, OAL_TO_DEVICE);

    //if (OAL_UNLIKELY(NULL != pst_net_dev) && OAL_UNLIKELY(NULL != pst_buf))
    if (OAL_UNLIKELY(NULL != pst_buf))
    {
        dev_kfree_skb_any(pst_buf);
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_finish_spe_rd(oal_int32 l_port_num, oal_int32 l_src_port_num,
    oal_netbuf_stru *pst_buf, oal_uint32 ul_dma, oal_uint32 ul_flags)
{
    oal_net_device_stru       *pst_net_dev;

    if(OAL_WARN_ON(!spe_hook.port_netdev))
    {
         oal_print_hi11xx_log(HI11XX_LOG_ERR,"%s error: spe_hook.port_netdev is null",__FUNCTION__);
         return OAL_NETDEV_TX_BUSY;
    };

    pst_net_dev = spe_hook.port_netdev(l_port_num);

    oal_dma_unmap_single(NULL, ul_dma, pst_buf->len, OAL_FROM_DEVICE);

    return wal_vap_start_xmit(pst_buf, pst_net_dev);
}


OAL_STATIC oal_int32 wal_netdev_spe_init(oal_net_device_stru *pst_net_dev)
{
    oal_int32             l_ret = 0;
    oal_int32             l_port_num = 0;
    mac_vap_stru          *pst_mac_vap;

    struct spe_port_attr  st_spe_port_attr;

    pst_mac_vap = (mac_vap_stru*)OAL_NET_DEV_PRIV(pst_net_dev);

    OAL_MEMZERO(&st_spe_port_attr, sizeof(struct spe_port_attr));

    /* ����SPE�˿����� */
    st_spe_port_attr.desc_ops.finish_rd =  wal_finish_spe_rd;
    st_spe_port_attr.desc_ops.finish_td =  wal_finish_spe_td;
    st_spe_port_attr.rd_depth = WAL_MAX_SPE_PKT_NUM;
    st_spe_port_attr.td_depth = WAL_MAX_SPE_PKT_NUM;
    st_spe_port_attr.attach_brg = spe_attach_brg_normal;
    st_spe_port_attr.net = pst_net_dev;
    st_spe_port_attr.rd_skb_num = st_spe_port_attr.rd_depth;
    st_spe_port_attr.rd_skb_size = 2048;
    st_spe_port_attr.enc_type = spe_enc_none;
    st_spe_port_attr.stick_mode = 0;

    /* ����SPE�˿� */
    l_port_num = wifi_spe_port_alloc(pst_net_dev, &st_spe_port_attr);

    if(0 > l_port_num)
    {
        OAL_IO_PRINT("wal_netdev_spe_init::spe port alloc failed;\n");
        return -OAL_FAIL;
    }
    pst_mac_vap->ul_spe_portnum = (oal_uint32)l_port_num;

    OAL_IO_PRINT("wal_netdev_spe_init::vap_id::%d;port_num::%d\n", pst_mac_vap->uc_vap_id, pst_mac_vap->ul_spe_portnum);

    return l_ret;
}

OAL_STATIC oal_void wal_netdev_spe_exit(oal_net_device_stru *pst_net_dev)
{
    mac_vap_stru          *pst_mac_vap;

    pst_mac_vap = (mac_vap_stru*)OAL_NET_DEV_PRIV(pst_net_dev);

    wifi_spe_port_free(pst_mac_vap->ul_spe_portnum);

    OAL_IO_PRINT("wal_netdev_spe_exit::vap_id::%d, spe port(%d) free.\n", pst_mac_vap->uc_vap_id, pst_mac_vap->ul_spe_portnum);
}
#endif /* defined(CONFIG_BALONG_SPE) && _PRE_WLAN_SPE_SUPPORT */

mac_device_stru *wal_get_macdev_by_netdev(oal_net_device_stru *pst_net_dev)
{
    oal_wireless_dev_stru    *pst_wdev;
    mac_wiphy_priv_stru      *pst_wiphy_priv;

    pst_wdev = OAL_NETDEVICE_WDEV(pst_net_dev);
    if(OAL_PTR_NULL == pst_wdev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_get_macdev_by_netdev::pst_wdev is null!}\r\n");
        return OAL_PTR_NULL;
    }
    pst_wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(pst_wdev->wiphy));
    return pst_wiphy_priv->pst_mac_device;

}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef _PRE_WLAN_FEATURE_DFR
OAL_STATIC oal_bool_enum_uint8 wal_dfr_recovery_check(oal_net_device_stru *pst_net_dev)
{
    oal_wireless_dev_stru      *pst_wdev;
    mac_wiphy_priv_stru        *pst_wiphy_priv;
    mac_device_stru            *pst_mac_device;

    if(OAL_PTR_NULL == pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_dfr_recovery_check::pst_net_dev is null!}\r\n");
        return OAL_FALSE;
    }

    pst_wdev = OAL_NETDEVICE_WDEV(pst_net_dev);
    if(OAL_PTR_NULL == pst_wdev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_dfr_recovery_check::pst_wdev is null!}\r\n");
        return OAL_FALSE;
    }

    pst_wiphy_priv  = (mac_wiphy_priv_stru *)oal_wiphy_priv(pst_wdev->wiphy);
    if(OAL_PTR_NULL == pst_wiphy_priv)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_dfr_recovery_check::pst_wiphy_priv is null!}\r\n");
        return OAL_FALSE;
    }

    pst_mac_device  = pst_wiphy_priv->pst_mac_device;
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_dfr_recovery_check::pst_mac_device is null!}\r\n");
        return OAL_FALSE;
    }

    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_dfr_recovery_check::recovery_flag:%d, uc_vap_num:%d.}\r\n" ,
                     g_st_dfr_info_etc.bit_ready_to_recovery_flag, pst_mac_device->uc_vap_num);

    if((OAL_TRUE == g_st_dfr_info_etc.bit_ready_to_recovery_flag)
       && (!pst_mac_device->uc_vap_num))
    {
        /* DFR�ָ�,�ڴ���ҵ��VAPǰ�·�У׼�Ȳ���,ֻ�·�һ�� */
        return OAL_TRUE;
    }

    return OAL_FALSE;
}
#endif /* #ifdef _PRE_WLAN_FEATURE_DFR */
#endif


OAL_STATIC oal_int32  _wal_netdev_open(oal_net_device_stru *pst_net_dev,oal_uint8 uc_entry_flag)
{
    wal_msg_write_stru      st_write_msg;
    oal_int32               l_ret;
    wal_msg_stru           *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32              ul_err_code;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    oal_wireless_dev_stru   *pst_wdev;
#endif

#if (defined(_PRE_PRODUCT_ID_HI110X_HOST)&&(_PRE_OS_VERSION_WIN32 != _PRE_OS_VERSION))
    oal_int32                ul_check_hw_status = 0;
#endif

#if !defined(_PRE_PRODUCT_ID_HI110X_HOST)
    mac_vap_stru               *pst_mac_vap;
    hmac_device_stru           *pst_hmac_device;
#endif
    oal_netdev_priv_stru       *pst_netdev_priv;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_netdev_open_etc::pst_net_dev is null ptr!}\r\n");
        return -OAL_EFAUL;
    }

    OAL_IO_PRINT("wal_netdev_open_etc,dev_name is:%s\n", pst_net_dev->name);
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_open_etc::iftype:%d.!}\r\n", pst_net_dev->ieee80211_ptr->iftype);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    g_uc_netdev_is_open_etc = OAL_TRUE;

    l_ret = wlan_pm_open_etc();
    if (OAL_FAIL == l_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_netdev_open_etc::wlan_pm_open_etc Fail!}\r\n");
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_OPEN, CHR_WIFI_DRV_ERROR_POWER_ON);
        return -OAL_EFAIL;
    }
    else if(OAL_ERR_CODE_ALREADY_OPEN != l_ret)
    {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        /* �����ϵ�ʱ��ΪFALSE */
        hwifi_config_init_force_etc();
#endif

        /* �ϵ�host device_stru��ʼ��*/
        l_ret = wal_host_dev_init_etc(pst_net_dev);
        if(OAL_SUCC != l_ret)
        {
            OAL_IO_PRINT("wal_host_dev_init_etc FAIL %d \r\n",l_ret);
            return -OAL_EFAIL;
        }
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    /* dfr ��δ����wlan1 */
    else if (wal_dfr_recovery_check(pst_net_dev))
    {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        g_uc_custom_cali_done_etc = OAL_TRUE;
        wal_custom_cali_etc();
        hwifi_config_init_force_etc();
#endif
    }
#endif  /* #ifdef _PRE_WLAN_FEATURE_DFR */

    // ����vap�Ĵ���,������ͨ����־λ�ж��Ƿ���Ҫ�·�device
    l_ret = wal_cfg_vap_h2d_event_etc(pst_net_dev);
    if(OAL_SUCC != l_ret)
    {
        OAL_IO_PRINT("wal_cfg_vap_h2d_event_etc FAIL %d \r\n",l_ret);
        return -OAL_EFAIL;
    }
    OAL_IO_PRINT("wal_cfg_vap_h2d_event_etc succ \r\n");
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    if(OAL_TRUE == g_st_ap_config_info.l_ap_power_flag)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_netdev_open_etc::power state is on,in ap mode, start vap later.}\r\n");

        /* �˱�����ʱ��һ�Σ���ֹ Android framework����ģʽ�л�ǰ�·�����up���� */
        g_st_ap_config_info.l_ap_power_flag = OAL_FALSE;
        oal_net_tx_wake_all_queues(pst_net_dev);/*�������Ͷ��� */
        return OAL_SUCC;
    }

    if (((NL80211_IFTYPE_STATION == pst_net_dev->ieee80211_ptr->iftype) || (NL80211_IFTYPE_P2P_DEVICE == pst_net_dev->ieee80211_ptr->iftype)))/*�ŵ�����--ȡ��name�ж�*/
    {
        l_ret = wal_init_wlan_vap_etc(pst_net_dev);
        if(OAL_SUCC != l_ret)
        {
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_OPEN, CHR_WIFI_DRV_ERROR_POWER_ON);
            return -OAL_EFAIL;
        }
    }
    else if(NL80211_IFTYPE_AP == pst_net_dev->ieee80211_ptr->iftype)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_netdev_open_etc::ap mode,no need to start vap.!}\r\n");
        oal_net_tx_wake_all_queues(pst_net_dev);/*�������Ͷ��� */
        return OAL_SUCC;
    }
#else
    if((NL80211_IFTYPE_AP == pst_net_dev->ieee80211_ptr->iftype)&&(OAL_TRUE == uc_entry_flag))
    {
        pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
        if(OAL_UNLIKELY(NULL == pst_mac_vap))
        {
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "{_wal_netdev_open::can't get mac vap from netdevice priv data!}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_vap->uc_device_id);
        if(OAL_UNLIKELY(NULL == pst_hmac_device))
        {
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "{_wal_netdev_open::hmac device is NULL!}");
            return OAL_ERR_CODE_PTR_NULL;
        }

#ifdef _PRE_WLAN_PRODUCT_1151V200
        if(0 == hmac_calc_up_ap_num_etc(mac_res_get_dev_etc(pst_mac_vap->uc_device_id)))
        {
            //OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{_wal_netdev_open::wifi led on, band: %d}\r\n", pst_mac_vap->st_channel.en_band);
            if (WLAN_BAND_5G == pst_mac_vap->st_channel.en_band)
            {
                oal_notify_wlan_status(OAL_WIFI_BAND_5G, OAL_WIFI_UP);
            }
            else
            {
                oal_notify_wlan_status(OAL_WIFI_BAND_2G, OAL_WIFI_UP);
            }
        }
#endif

        if(!pst_hmac_device->en_start_via_priv)
        {
           OAL_NETDEVICE_FLAGS(pst_net_dev) |= OAL_IFF_RUNNING;
           return OAL_SUCC;
        }
    }
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    hwifi_config_init_ini_etc(pst_net_dev);
#endif
#if ((_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE))
    /* �͹��Ķ��ƻ����� */
    (OAL_TRUE == g_wlan_pm_switch_etc) ? wlan_pm_enable_etc() : wlan_pm_disable_etc();
#endif // #if ((_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE))

#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_START_VAP, OAL_SIZEOF(mac_cfg_start_vap_param_stru));
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wdev    = pst_net_dev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_netdev_open_etc::wal_wireless_iftype_to_mac_p2p_mode_etc return BUFF}\r\n");
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_OPEN, CHR_WIFI_DRV_ERROR_POWER_ON);
        return -OAL_EINVAL;
    }
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;
#endif
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_mgmt_rate_init_flag = OAL_TRUE;

    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->l_ifindex = pst_net_dev->ifindex;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_start_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    wal_hipriv_wait_rsp(pst_net_dev, NULL);
#endif

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_open_etc::wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_open_etc::hmac start vap fail,err code[%u]!}\r\n", ul_err_code);
#if !defined(_PRE_PRODUCT_ID_HI110X_HOST) && (defined _PRE_WLAN_PRODUCT_1151V200)
        pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
        if(0 == hmac_calc_up_ap_num_etc(mac_res_get_dev_etc(pst_mac_vap->uc_device_id)))
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{_wal_netdev_open::wifi led off cause of up failed, band: %d}\r\n", pst_mac_vap->st_channel.en_band);
            if (WLAN_BAND_5G == pst_mac_vap->st_channel.en_band)
            {
                oal_notify_wlan_status(OAL_WIFI_BAND_5G, OAL_WIFI_DOWN);
            }
            else
            {
                oal_notify_wlan_status(OAL_WIFI_BAND_2G, OAL_WIFI_DOWN);
            }
        }
#endif

        return -OAL_EINVAL;
    }

    OAL_NETDEVICE_FLAGS(pst_net_dev) |= OAL_IFF_RUNNING;

    pst_netdev_priv = (oal_netdev_priv_stru *)OAL_NET_DEV_WIRELESS_PRIV(pst_net_dev);
    if (pst_netdev_priv->uc_napi_enable
        && (!pst_netdev_priv->uc_state)
        && ((NL80211_IFTYPE_STATION == pst_net_dev->ieee80211_ptr->iftype)
        || (NL80211_IFTYPE_P2P_DEVICE == pst_net_dev->ieee80211_ptr->iftype)
        || (NL80211_IFTYPE_P2P_CLIENT == pst_net_dev->ieee80211_ptr->iftype)))
    {
        oal_napi_enable(&pst_netdev_priv->st_napi);
        pst_netdev_priv->uc_state = 1;
    }


    oal_net_tx_wake_all_queues(pst_net_dev);/*�������Ͷ��� */
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST)&&(_PRE_OS_VERSION_WIN32 != _PRE_OS_VERSION))
    /*1102Ӳ��fem��DEVICE����HOST�麸lna�ջټ��,ֻ��wlan0ʱ��ӡ*/
    if((NL80211_IFTYPE_STATION == pst_net_dev->ieee80211_ptr->iftype)
        && (0 == (oal_strcmp("wlan0", pst_net_dev->name))))
    {
        wal_atcmsrv_ioctl_get_fem_pa_status_etc(pst_net_dev,&ul_check_hw_status);
    }
#endif

    return OAL_SUCC;
}

oal_int32  wal_netdev_open_ext(oal_net_device_stru *pst_net_dev)
{
    return  wal_netdev_open_etc(pst_net_dev,OAL_TRUE);
}


oal_int32  wal_netdev_open_etc(oal_net_device_stru *pst_net_dev,oal_uint8 uc_entry_flag)
{
    oal_int32 ret;

    if (OAL_NETDEVICE_FLAGS(pst_net_dev) & OAL_IFF_RUNNING)
    {
        return OAL_SUCC;
    }


#ifdef _PRE_WLAN_FEATURE_DFR
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_lock(&g_st_dfr_info_etc.wifi_excp_mutex);
#endif
#endif

    wal_wake_lock();
    ret = _wal_netdev_open(pst_net_dev,uc_entry_flag);
    wal_wake_unlock();

#ifdef _PRE_WLAN_FEATURE_DFR
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_unlock(&g_st_dfr_info_etc.wifi_excp_mutex);
#endif
#endif

#if (defined(_PRE_E5_722_PLATFORM) || defined(_PRE_CPE_711_PLATFORM) || defined(_PRE_CPE_722_PLATFORM))
    wlan_set_driver_lock(true);
#endif
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /*��¼wlan0 ����ʱ��*/
    if (oal_strncmp("wlan0", pst_net_dev->name, OAL_STRLEN("wlan0")) == 0)
    {
        g_st_wifi_radio_stat_etc.ull_wifi_on_time_stamp = OAL_TIME_JIFFY;
    }
#endif

    return ret;
}
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE


oal_uint32 wal_custom_cali_etc(oal_void)
{
    oal_net_device_stru *pst_net_dev;
    oal_uint32           ul_ret = 0;

    pst_net_dev = oal_dev_get_by_name("Hisilicon0");  //ͨ��cfg vap0����c0 c1У׼
    if (OAL_WARN_ON(OAL_PTR_NULL == pst_net_dev))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }
    else
    {
        /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
        oal_dev_put(pst_net_dev);
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_custom_cali_etc::the net_device is already exist!}\r\n");
    }

    hwifi_config_init_nvram_main_etc(pst_net_dev);
    hwifi_config_init_ini_main(pst_net_dev);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (g_uc_custom_cali_done_etc == OAL_TRUE)
    {
        /* У׼�����·� */
        wal_send_cali_data_etc(pst_net_dev);
    }
    else
    {
        g_uc_custom_cali_done_etc = OAL_TRUE;
    }

    wal_send_cali_matrix_data(pst_net_dev);
#endif

    /* �·����� */
    ul_ret = hwifi_config_init_dts_main_etc(pst_net_dev);

    return ul_ret;
}


oal_int32 wal_set_custom_process_func_etc(oal_void)
{
#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1151)
    struct custom_process_func_handler* pst_custom_process_func_handler;
    pst_custom_process_func_handler = oal_get_custom_process_func_etc();
    if(OAL_PTR_NULL == pst_custom_process_func_handler)
    {
        OAM_ERROR_LOG0(0,OAM_SF_ANY,"{hmac_set_auto_freq_process_func get handler failed!}");
    }
    else
    {
        pst_custom_process_func_handler->p_custom_cali_func = wal_custom_cali_etc;
    }
#else
    wal_custom_cali_etc();
#endif
    return OAL_SUCC;
}
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */


OAL_STATIC oal_int32  _wal_netdev_stop(oal_net_device_stru *pst_net_dev)
{
    wal_msg_write_stru          st_write_msg;
    oal_uint32                  ul_err_code;
    wal_msg_stru               *pst_rsp_msg = OAL_PTR_NULL;
    oal_int32                   l_ret;
    mac_vap_stru               *pst_mac_vap;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8    en_p2p_mode;
    oal_wireless_dev_stru      *pst_wdev;
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    mac_device_stru            *pst_mac_device;
#endif
#endif
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    mac_device_stru          *pst_mac_dev;
    oal_int8                 ac_wlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    oal_int8                 ac_p2p_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    oal_int8                 ac_hwlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    oal_netdev_priv_stru     *pst_netdev_priv;
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_netdev_stop_etc::pst_net_dev is null ptr!}\r\n");
        return -OAL_EFAUL;
    }

    /*stop the netdev's queues*/
    oal_net_tx_stop_all_queues(pst_net_dev);/* ֹͣ���Ͷ��� */
    wal_force_scan_complete_etc(pst_net_dev, OAL_TRUE);

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    OAL_IO_PRINT("wal_netdev_stop_etc,dev_name is:%s\n", pst_net_dev->name);
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_stop_etc::iftype:%d.!}", pst_net_dev->ieee80211_ptr->iftype);

    /* APģʽ��,��ģʽ�л�ʱdown��ɾ�� vap */
    if(NL80211_IFTYPE_AP == pst_net_dev->ieee80211_ptr->iftype)
    {
        l_ret = wal_netdev_stop_ap_etc(pst_net_dev);
        return l_ret;
    }
#endif

    /* ���netdev����running״̬����ֱ�ӷ��سɹ� */
    if (0 == (OAL_NETDEVICE_FLAGS(pst_net_dev) & OAL_IFF_RUNNING))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{_wal_netdev_stop::vap is already down!}");
        return OAL_SUCC;
    }

    /* ���netdev��mac vap�Ѿ��ͷţ���ֱ�ӷ��سɹ� */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{_wal_netdev_stop:: mac vap of netdevice is down!iftype:[%d]}", pst_net_dev->ieee80211_ptr->iftype);
        return OAL_SUCC;
    }

    if (MAC_VAP_STATE_INIT == pst_mac_vap->en_vap_state)
    {
        OAM_WARNING_LOG3(0, OAM_SF_ANY, "{_wal_netdev_stop::vap is already down!iftype:[%d] vap mode[%d] p2p mode[%d]!}",
            pst_net_dev->ieee80211_ptr->iftype, pst_mac_vap->en_vap_mode, pst_mac_vap->en_p2p_mode);
        if(WLAN_P2P_DEV_MODE != pst_mac_vap->en_p2p_mode)
        {
            return OAL_SUCC;
        }
    }

    /***************************************************************************
                           ���¼���wal�㴦��
    ***************************************************************************/
    /* ��дWID��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DOWN_VAP, OAL_SIZEOF(mac_cfg_down_vap_param_stru));
    ((mac_cfg_down_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wdev    = pst_net_dev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_netdev_stop_etc::wal_wireless_iftype_to_mac_p2p_mode_etc return BUFF}\r\n");
        return -OAL_EINVAL;
    }
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;
#endif

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_down_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    wal_hipriv_wait_rsp(pst_net_dev, NULL);
#endif

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        if (OAL_PTR_NULL == OAL_NET_DEV_PRIV(pst_net_dev))
        {
            /* �ر�net_device���������Ӧvap ��null�����flags running��־ */
            OAL_NETDEVICE_FLAGS(pst_net_dev) &= (~OAL_IFF_RUNNING);
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_stop_etc::net_device's vap is null, set flag not running, if_idx:%d}", pst_net_dev->ifindex);
        }
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_stop_etc::wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_stop_etc::hmac stop vap fail!err code [%d]}\r\n", ul_err_code);
        return -OAL_EFAIL;
    }

#ifdef _PRE_WLAN_PRODUCT_1151V200
    if(0 == hmac_calc_up_ap_num_etc(mac_res_get_dev_etc(pst_mac_vap->uc_device_id)))
    {
        //OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_netdev_stop_etc::wifi led off, band: %d}\r\n", pst_mac_vap->st_channel.en_band);
        if (WLAN_BAND_5G == pst_mac_vap->st_channel.en_band)
        {
            oal_notify_wlan_status(OAL_WIFI_BAND_5G, OAL_WIFI_DOWN);
        }
        else
        {
            oal_notify_wlan_status(OAL_WIFI_BAND_2G, OAL_WIFI_DOWN);
        }
    }
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    pst_mac_dev = wal_get_macdev_by_netdev(pst_net_dev);
    if(OAL_PTR_NULL == pst_mac_dev)
    {
        OAL_IO_PRINT("wal_deinit_wlan_vap_etc::wal_get_macdev_by_netdev FAIL\r\n");
        return -OAL_EFAIL;
    }
        /* ͨ��device id��ȡnetdev���� */
    OAL_SPRINTF(ac_wlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "wlan%d", pst_mac_dev->uc_device_id);
    OAL_SPRINTF(ac_p2p_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "p2p%d",  pst_mac_dev->uc_device_id);
    OAL_SPRINTF(ac_hwlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "hwlan%d",  pst_mac_dev->uc_device_id);

    if ((NL80211_IFTYPE_STATION == pst_net_dev->ieee80211_ptr->iftype)
        && ((0 == oal_strcmp(ac_wlan_netdev_name, pst_net_dev->name)) || (0 == oal_strcmp(ac_hwlan_netdev_name, pst_net_dev->name))))
    {
        l_ret = wal_deinit_wlan_vap_etc(pst_net_dev);
        if (OAL_SUCC != l_ret)
        {
            return l_ret;
        }

#ifdef _PRE_WLAN_FEATURE_P2P
        pst_mac_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
        if(OAL_PTR_NULL != pst_mac_device)
        {
            wal_del_p2p_group_etc(pst_mac_device);
        }
    }
#endif

    if (((NL80211_IFTYPE_STATION == pst_net_dev->ieee80211_ptr->iftype) || (NL80211_IFTYPE_P2P_DEVICE == pst_net_dev->ieee80211_ptr->iftype))
         && (0 == oal_strcmp(ac_p2p_netdev_name, pst_net_dev->name)))
    {
        l_ret = wal_deinit_wlan_vap_etc(pst_net_dev);
        if (OAL_SUCC != l_ret)
        {
            return l_ret;
        }
    }
#endif

    pst_netdev_priv = (oal_netdev_priv_stru *)OAL_NET_DEV_WIRELESS_PRIV(pst_net_dev);
   if (pst_netdev_priv->uc_napi_enable
        && ((NL80211_IFTYPE_STATION == pst_net_dev->ieee80211_ptr->iftype)
        || (NL80211_IFTYPE_P2P_DEVICE == pst_net_dev->ieee80211_ptr->iftype)
        || (NL80211_IFTYPE_P2P_CLIENT == pst_net_dev->ieee80211_ptr->iftype)))
    {
    #ifndef WIN32
        oal_netbuf_list_purge(&pst_netdev_priv->st_rx_netbuf_queue);
    #endif
        oal_napi_disable(&pst_netdev_priv->st_napi);
        pst_netdev_priv->uc_state = 0;
    }

    return OAL_SUCC;
}

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)

OAL_STATIC oal_int32  wal_set_power_on(oal_net_device_stru *pst_net_dev, oal_int32 power_flag)
{
    oal_int32    l_ret = 0;

    //ap���µ磬����VAP
    if (0 == power_flag)//�µ�
    {
        /* �µ�host device_struȥ��ʼ��*/
        wal_host_dev_exit(pst_net_dev);

        wal_wake_lock();
        wlan_pm_close_etc();
        wal_wake_unlock();

        g_st_ap_config_info.l_ap_power_flag = OAL_FALSE;
    }
    else if (1 == power_flag) //�ϵ�
    {
        g_st_ap_config_info.l_ap_power_flag = OAL_TRUE;

        wal_wake_lock();
        l_ret = wlan_pm_open_etc();
        wal_wake_unlock();
        if (OAL_FAIL==l_ret)
        {
             OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_set_power_on::wlan_pm_open_etc Fail!}\r\n");
             return -OAL_EFAIL;
        }
        else if(OAL_ERR_CODE_ALREADY_OPEN != l_ret)
        {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
            /* �����ϵ�ʱ��ΪFALSE */
            hwifi_config_init_force_etc();
#endif
            // �����ϵ糡�����·�����VAP
            l_ret = wal_cfg_vap_h2d_event_etc(pst_net_dev);
            if(OAL_SUCC!= l_ret)
            {
                return -OAL_EFAIL;
            }
        }

        /* �ϵ�host device_stru��ʼ��*/
        l_ret = wal_host_dev_init_etc(pst_net_dev);
        if(OAL_SUCC != l_ret)
        {
            OAL_IO_PRINT("wal_set_power_on FAIL %d \r\n",l_ret);
            return -OAL_EFAIL;
        }
    }
    else
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_set_power_on::pupower_flag:%d error.}\r\n", power_flag);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_void wal_set_power_mgmt_on(oal_uint power_mgmt_flag)
{
    struct wlan_pm_s  *pst_wlan_pm;
    pst_wlan_pm = wlan_pm_get_drv_etc();
    if(NULL != pst_wlan_pm)
    {
        /* apģʽ�£��Ƿ������µ����,1:����,0:������ */
        pst_wlan_pm->ul_apmode_allow_pm_flag = power_mgmt_flag;
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_set_power_mgmt_on::wlan_pm_get_drv_etc return null.");
    }

}


oal_int32 wal_netdev_stop_ap_etc(oal_net_device_stru *pst_net_dev)
{
    oal_int32          l_ret;

    if (NL80211_IFTYPE_AP != pst_net_dev->ieee80211_ptr->iftype)
    {
        return OAL_SUCC;
    }

    /* ����ɨ��,�Է���20/40Mɨ������йر�AP */
    wal_force_scan_complete_etc(pst_net_dev, OAL_TRUE);

    /* AP�ر��л���STAģʽ,ɾ�����vap */
    if(OAL_SUCC != wal_stop_vap_etc(pst_net_dev))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_netdev_stop_ap_etc::wal_stop_vap_etc enter a error.}");
    }
    l_ret = wal_deinit_wlan_vap_etc(pst_net_dev);
    if(OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_netdev_stop_ap_etc::wal_deinit_wlan_vap_etc enter a error.}");
        return l_ret;
    }

    /* Del aput����Ҫ�л�netdev iftype״̬��station */
    pst_net_dev->ieee80211_ptr->iftype = NL80211_IFTYPE_STATION;

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    /* aput�µ� */
    wal_set_power_mgmt_on(OAL_TRUE);
    l_ret = wal_set_power_on(pst_net_dev, OAL_FALSE);
    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_netdev_stop_ap_etc::wal_set_power_on fail [%d]!}", l_ret);
        return l_ret;
    }
#endif /* #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) */
    return OAL_SUCC;

}
#endif

oal_int32  wal_netdev_stop_etc(oal_net_device_stru *pst_net_dev)
{
    oal_int32 ret;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_netdev_stop_etc::pst_net_dev is null ptr!}\r\n");
        return -OAL_EFAUL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    //mutex_lock(&g_st_dfr_info_etc.wifi_excp_mutex);
#endif
#endif
    wal_wake_lock();
    ret = _wal_netdev_stop(pst_net_dev);
    wal_wake_unlock();

    if(OAL_SUCC == ret)
    {
        OAL_NETDEVICE_FLAGS(pst_net_dev) &= ~OAL_IFF_RUNNING;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    //mutex_unlock(&g_st_dfr_info_etc.wifi_excp_mutex);
#endif
#endif

#if (defined(_PRE_E5_722_PLATFORM) || defined(_PRE_CPE_711_PLATFORM) || defined(_PRE_CPE_722_PLATFORM))
    wlan_set_driver_lock(false);
#endif

    return ret;
}


OAL_STATIC oal_net_device_stats_stru*  wal_netdev_get_stats(oal_net_device_stru *pst_net_dev)
{
    oal_net_device_stats_stru  *pst_stats = &(pst_net_dev->stats);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

#ifdef _PRE_DEBUG_MODE
    mac_vap_stru               *pst_mac_vap;
    oam_stat_info_stru         *pst_oam_stat;

    oam_vap_stat_info_stru     *pst_oam_vap_stat;

    pst_mac_vap  = OAL_NET_DEV_PRIV(pst_net_dev);
    pst_oam_stat = OAM_STAT_GET_STAT_ALL();

    pst_oam_vap_stat = &(pst_oam_stat->ast_vap_stat_info[pst_mac_vap->uc_vap_id]);

    /* ����ͳ����Ϣ��net_device */
    pst_stats->rx_packets   = pst_oam_vap_stat->ul_rx_pkt_to_lan;
    pst_stats->rx_bytes     = pst_oam_vap_stat->ul_rx_bytes_to_lan;
    pst_stats->rx_dropped   = pst_oam_vap_stat->ul_rx_ta_check_dropped +
                              pst_oam_vap_stat->ul_rx_da_check_dropped +
                              pst_oam_vap_stat->ul_rx_key_search_fail_dropped +
                              pst_oam_vap_stat->ul_rx_tkip_mic_fail_dropped +
                              pst_oam_vap_stat->ul_rx_replay_fail_dropped +
                              pst_oam_vap_stat->ul_rx_11i_check_fail_dropped +
                              pst_oam_vap_stat->ul_rx_wep_check_fail_dropped +
                              pst_oam_vap_stat->ul_rx_alg_process_dropped +
                              pst_oam_vap_stat->ul_rx_null_frame_dropped +
                              pst_oam_vap_stat->ul_rx_abnormal_dropped +
                              pst_oam_vap_stat->ul_rx_mgmt_abnormal_dropped +
                              pst_oam_vap_stat->ul_rx_pspoll_process_dropped +
                              pst_oam_vap_stat->ul_rx_bar_process_dropped +
                              pst_oam_vap_stat->ul_rx_no_buff_dropped +
                              pst_oam_vap_stat->ul_rx_defrag_process_dropped +
                              pst_oam_vap_stat->ul_rx_de_mic_fail_dropped +
                              pst_oam_vap_stat->ul_rx_portvalid_check_fail_dropped;

    pst_stats->tx_packets   = pst_oam_vap_stat->ul_tx_pkt_num_from_lan;
    pst_stats->tx_bytes     = pst_oam_vap_stat->ul_tx_bytes_from_lan;
    pst_stats->tx_dropped   = pst_oam_vap_stat->ul_tx_abnormal_msdu_dropped +
                              pst_oam_vap_stat->ul_tx_security_check_faild +
                              pst_oam_vap_stat->ul_tx_abnormal_mpdu_dropped +
                              pst_oam_vap_stat->ul_tx_uapsd_process_dropped +
                              pst_oam_vap_stat->ul_tx_psm_process_dropped +
                              pst_oam_vap_stat->ul_tx_alg_process_dropped;
#endif
#elif defined(_PRE_PRODUCT_ID_HI110X_HOST)
    mac_vap_stru               *pst_mac_vap;
    oam_stat_info_stru         *pst_oam_stat;

    oam_vap_stat_info_stru     *pst_oam_vap_stat;

    pst_mac_vap  = OAL_NET_DEV_PRIV(pst_net_dev);
    pst_oam_stat = OAM_STAT_GET_STAT_ALL();

    if(NULL == pst_mac_vap)
    {
        return pst_stats;
    }

    if(pst_mac_vap->uc_vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_netdev_get_stats error vap id %u", pst_mac_vap->uc_vap_id);
        return pst_stats;
    }

    pst_oam_vap_stat = &(pst_oam_stat->ast_vap_stat_info[pst_mac_vap->uc_vap_id]);

    /* ����ͳ����Ϣ��net_device */
    pst_stats->rx_packets   = pst_oam_vap_stat->ul_rx_pkt_to_lan;
    pst_stats->rx_bytes     = pst_oam_vap_stat->ul_rx_bytes_to_lan;


    pst_stats->tx_packets   = pst_oam_vap_stat->ul_tx_pkt_num_from_lan;
    pst_stats->tx_bytes     = pst_oam_vap_stat->ul_tx_bytes_from_lan;
#endif
    return pst_stats;
}


OAL_STATIC oal_int32  _wal_netdev_set_mac_addr(oal_net_device_stru *pst_net_dev, void *p_addr)
{
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    oal_sockaddr_stru            *pst_mac_addr = OAL_PTR_NULL;
#if defined(_PRE_WLAN_FEATURE_SMP_SUPPORT) || (defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT))
    mac_vap_stru                 *pst_mac_vap = OAL_PTR_NULL;
#endif
#else
    oal_sockaddr_stru            *pst_mac_addr = OAL_PTR_NULL;
    oal_uint32                    ul_ret = 0;
#if defined(_PRE_WLAN_FEATURE_SMP_SUPPORT) || (defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT))
    mac_vap_stru                 *pst_mac_vap = OAL_PTR_NULL;
#endif
    frw_event_mem_stru           *pst_event_mem = OAL_PTR_NULL;
    wal_msg_stru                 *pst_cfg_msg = OAL_PTR_NULL;
    wal_msg_write_stru           *pst_write_msg = OAL_PTR_NULL;
    mac_cfg_staion_id_param_stru *pst_param = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_wireless_dev_stru        *pst_wdev = OAL_PTR_NULL; /* ����P2P ������p2p0 �� p2p-p2p0 MAC ��ַ��wlan0 ��ȡ */
#endif
#endif

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == p_addr)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_netdev_set_mac_addr::pst_net_dev or p_addr null ptr error %d, %d!}\r\n", pst_net_dev, p_addr);

        return -OAL_EFAUL;
    }

    if (oal_netif_running(pst_net_dev))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_netdev_set_mac_addr::cannot set address; device running!}\r\n");

        return -OAL_EBUSY;
    }
    /*lint +e774*//*lint +e506*/

    pst_mac_addr = (oal_sockaddr_stru *)p_addr;

    if (ETHER_IS_MULTICAST(pst_mac_addr->sa_data))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_netdev_set_mac_addr::can not set group/broadcast addr!}\r\n");

        return -OAL_EINVAL;
    }

    oal_set_mac_addr((oal_uint8 *)(pst_net_dev->dev_addr), (oal_uint8 *)(pst_mac_addr->sa_data));


#if (defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT))
    /* mac��ַ���޸�Ҫ֪ͨ��spe��Ӧ��port�� ����spe������Ӳ������ */
    if(spe_hook.is_enable && spe_hook.is_enable() && (spe_mode_normal == spe_hook.mode()))
    {
        pst_mac_vap = (mac_vap_stru *)OAL_NET_DEV_PRIV(pst_net_dev);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{_wal_netdev_set_mac_addr::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}\r\n");
            return -OAL_EINVAL;
        }
        spe_hook.port_ioctl(pst_mac_vap->ul_spe_portnum, spe_port_ioctl_set_mac, (int)pst_net_dev->dev_addr);
    }
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    return OAL_SUCC;
#else

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    ul_ret = wal_alloc_cfg_event_etc(pst_net_dev, &pst_event_mem, NULL, &pst_cfg_msg, (WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_staion_id_param_stru)));     /* �����¼� */
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_set_mac_addr::wal_alloc_cfg_event_etc fail!err code[%u]}\r\n",ul_ret);
        return -OAL_ENOMEM;
    }

    /* ��д������Ϣ */
    WAL_CFG_MSG_HDR_INIT(&(pst_cfg_msg->st_msg_hdr),
                         WAL_MSG_TYPE_WRITE,
                         WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_staion_id_param_stru),
                         WAL_GET_MSG_SN());

    /* ��дWID��Ϣ */
    pst_write_msg = (wal_msg_write_stru *)pst_cfg_msg->auc_msg_data;
    WAL_WRITE_MSG_HDR_INIT(pst_write_msg, WLAN_CFGID_STATION_ID, OAL_SIZEOF(mac_cfg_staion_id_param_stru));

    pst_param = (mac_cfg_staion_id_param_stru *)pst_write_msg->auc_value;   /* ��дWID��Ӧ�Ĳ��� */
    oal_set_mac_addr(pst_param->auc_station_id, (oal_uint8 *)(pst_mac_addr->sa_data));
#ifdef _PRE_WLAN_FEATURE_P2P
    /* ��д�·�net_device ��Ӧp2p ģʽ */
    pst_wdev = (oal_wireless_dev_stru *)pst_net_dev->ieee80211_ptr;
    pst_param->en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == pst_param->en_p2p_mode)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_netdev_set_mac_addr::wal_wireless_iftype_to_mac_p2p_mode_etc return BUFF}\r\n");
        FRW_EVENT_FREE(pst_event_mem);
        return -OAL_EINVAL;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
    pst_mac_vap = (mac_vap_stru *)OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{_wal_netdev_set_mac_addr::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}\r\n");
        FRW_EVENT_FREE(pst_event_mem);
        return -OAL_EINVAL;
    }

    frw_event_post_event_etc(pst_event_mem, pst_mac_vap->ul_core_id);
#else
    frw_event_dispatch_event_etc(pst_event_mem);
#endif

    FRW_EVENT_FREE(pst_event_mem);

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    wal_hipriv_wait_rsp(pst_net_dev, p_addr);
#endif

    return OAL_SUCC;
#endif
}

OAL_STATIC oal_int32  wal_netdev_set_mac_addr(oal_net_device_stru *pst_net_dev, void *p_addr)
{
    oal_int32 ret;
    wal_wake_lock();
    ret = _wal_netdev_set_mac_addr(pst_net_dev, p_addr);
    wal_wake_unlock();

    if(ret != OAL_SUCC)
    {
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_OPEN, CHR_WIFI_DRV_ERROR_POWER_ON_SET_MAC_ADDR);
    }

    return ret;
}


oal_int32 wal_android_priv_cmd_etc(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, oal_int32 ul_cmd)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    wal_android_wifi_priv_cmd_stru  st_priv_cmd;
    oal_int8    *pc_command         = OAL_PTR_NULL;
    oal_int32    l_ret              = 0;
    oal_int32    l_value;
    oal_int32   *pl_value;
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    oal_int32    l_param_1 = 0;
    oal_int32    l_param_2 = 0;
    oal_int8     ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};     /* Ԥ��Э��ģʽ�ַ����ռ� */
    oal_uint32   ul_off_set;
    oal_int8    *pc_cmd_copy;
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (!capable(CAP_NET_ADMIN))
    {
        return -EPERM;
    }
#endif

    if (OAL_PTR_NULL == pst_ifr->ifr_data)
    {
        l_ret = -OAL_EINVAL;
        return l_ret;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::dfr_process_status[%d]!}",
                g_st_dfr_info_etc.bit_device_reset_process_flag);
        return OAL_SUCC;
    }
#endif //#ifdef _PRE_WLAN_FEATURE_DFR
    if (oal_copy_from_user((oal_uint8 *)&st_priv_cmd, pst_ifr->ifr_data, sizeof(wal_android_wifi_priv_cmd_stru)))
    {
        l_ret = -OAL_EINVAL;
        return l_ret;
    }

    if (st_priv_cmd.l_total_len > MAX_PRIV_CMD_SIZE || st_priv_cmd.l_total_len < 0)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::too long priavte command. len:%d. }\r\n", st_priv_cmd.l_total_len);
        l_ret = -OAL_EINVAL;
        return l_ret;
    }

    /* �����ڴ汣��wpa_supplicant �·������������ */
    pc_command = oal_memalloc((oal_uint32)(st_priv_cmd.l_total_len + 5));/* total len Ϊpriv cmd ����buffer ���� */
    if (OAL_PTR_NULL == pc_command)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::mem alloc failed.}\r\n");

        l_ret = -OAL_ENOMEM;
        return l_ret;
    }

    /* ����wpa_supplicant ����ں�̬�� */
    oal_memset(pc_command, 0, (oal_uint32)(st_priv_cmd.l_total_len + 5));

    l_ret = (oal_int32)oal_copy_from_user(pc_command, pst_ifr->ifr_data + 8, (oal_uint32)(st_priv_cmd.l_total_len));
    if (l_ret != 0)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::oal_copy_from_user: -OAL_EFAIL }\r\n");
        l_ret = -OAL_EFAIL;
        oal_free(pc_command);
        return l_ret;
    }
    pc_command[st_priv_cmd.l_total_len] = '\0';
    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::Android private cmd total_len:%d, used_len:%d.}\r\n",
                st_priv_cmd.l_total_len, st_priv_cmd.l_used_len);

    if (oal_strncasecmp(pc_command, CMD_SET_AP_WPS_P2P_IE, OAL_STRLEN(CMD_SET_AP_WPS_P2P_IE)) == 0)
    {
        oal_uint32 skip = OAL_STRLEN(CMD_SET_AP_WPS_P2P_IE) + 1;
        /* �ṹ������ */
        oal_app_ie_stru *pst_wps_p2p_ie;
        pst_wps_p2p_ie = (oal_app_ie_stru *)(pc_command + skip);

        /*lint -e413*/
        if((skip + pst_wps_p2p_ie->ul_ie_len + OAL_OFFSET_OF(oal_app_ie_stru, auc_ie)) > (oal_uint32)st_priv_cmd.l_total_len)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::SET_AP_WPS_P2P_IE param len is too short. need %d.}\r\n",(skip + pst_wps_p2p_ie->ul_ie_len));
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
        /*lint +e413*/
        l_ret = wal_ioctl_set_wps_p2p_ie_etc(pst_net_dev,
                                            pst_wps_p2p_ie->auc_ie,
                                            pst_wps_p2p_ie->ul_ie_len,
                                            pst_wps_p2p_ie->en_app_ie_type);
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    else if (0 == oal_strncasecmp(pc_command, CMD_MIRACAST_START, OAL_STRLEN(CMD_MIRACAST_START)))
    {
        OAM_WARNING_LOG0(0, OAM_SF_M2S, "{wal_android_priv_cmd_etc::Miracast start.}\r\n");
        l_ret = wal_ioctl_set_p2p_miracast_status(pst_net_dev, OAL_TRUE);
    }
    else if (0 == oal_strncasecmp(pc_command, CMD_MIRACAST_STOP, OAL_STRLEN(CMD_MIRACAST_STOP)))
    {
        OAM_WARNING_LOG0(0, OAM_SF_M2S, "{wal_android_priv_cmd_etc::Miracast stop.}\r\n");
        l_ret = wal_ioctl_set_p2p_miracast_status(pst_net_dev, OAL_FALSE);
    }
    else if(oal_strncasecmp(pc_command, CMD_P2P_SET_NOA, OAL_STRLEN(CMD_P2P_SET_NOA)) == 0)
    {
        oal_uint32 skip = OAL_STRLEN(CMD_P2P_SET_NOA) + 1;
        mac_cfg_p2p_noa_param_stru  st_p2p_noa_param;
        if ((skip + OAL_SIZEOF(st_p2p_noa_param)) > (oal_uint32)st_priv_cmd.l_total_len)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_P2P_SET_NOA param len is too short. need %d.}\r\n", skip + OAL_SIZEOF(st_p2p_noa_param));
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
        oal_memcopy(&st_p2p_noa_param, pc_command + skip, OAL_SIZEOF(mac_cfg_p2p_noa_param_stru));

        l_ret = wal_ioctl_set_p2p_noa(pst_net_dev,&st_p2p_noa_param);
    }
    else if(oal_strncasecmp(pc_command, CMD_P2P_SET_PS, OAL_STRLEN(CMD_P2P_SET_PS)) == 0)
    {
        oal_uint32 skip = OAL_STRLEN(CMD_P2P_SET_PS) + 1;
        mac_cfg_p2p_ops_param_stru  st_p2p_ops_param;
        if ((skip + OAL_SIZEOF(st_p2p_ops_param)) > (oal_uint32)st_priv_cmd.l_total_len)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_P2P_SET_PS param len is too short.need %d.}\r\n", skip + OAL_SIZEOF(st_p2p_ops_param));
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
        oal_memcopy(&st_p2p_ops_param, pc_command + skip, OAL_SIZEOF(mac_cfg_p2p_ops_param_stru));

        l_ret = wal_ioctl_set_p2p_ops(pst_net_dev,&st_p2p_ops_param);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_HS20
    else if (0 == oal_strncasecmp(pc_command, CMD_SET_QOS_MAP, OAL_STRLEN(CMD_SET_QOS_MAP)))
    {
        oal_uint32 skip = OAL_STRLEN(CMD_SET_QOS_MAP) + 1;
        hmac_cfg_qos_map_param_stru st_qos_map_param;
        if ((skip + OAL_SIZEOF(st_qos_map_param)) > (oal_uint32)st_priv_cmd.l_total_len)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_SET_QOS_MAP param len is too short.need %d.}\r\n", skip + OAL_SIZEOF(st_qos_map_param));
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
        oal_memcopy(&st_qos_map_param, pc_command + skip, OAL_SIZEOF(hmac_cfg_qos_map_param_stru));

        l_ret = wal_ioctl_set_qos_map(pst_net_dev,&st_qos_map_param);
    }
#endif
    else if(0 == oal_strncasecmp(pc_command, CMD_SET_POWER_ON, OAL_STRLEN(CMD_SET_POWER_ON)))
    {
        oal_int32 power_flag = -1;
        oal_uint32 command_len = OAL_STRLEN(pc_command);
        /* ��ʽ:SET_POWER_ON 1 or SET_POWER_ON 0 */
        if(command_len < (OAL_STRLEN((oal_int8 *)CMD_SET_POWER_ON) + 2))
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_SET_POWER_ON cmd len must equal or larger than 18. Now the cmd len:%d.}\r\n", command_len);

            oal_free(pc_command);
            return -OAL_EFAIL;
        }

        power_flag = oal_atoi(pc_command + OAL_STRLEN(CMD_SET_POWER_ON) + 1);

        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_SET_POWER_ON command,power flag:%d}\r\n", power_flag);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        //ap���µ磬����VAP
        if (0 == power_flag)//�µ�
        {
            /* �µ�host device_struȥ��ʼ��*/
            wal_host_dev_exit(pst_net_dev);

            wal_wake_lock();
            wlan_pm_close_etc();
            wal_wake_unlock();

            g_st_ap_config_info.l_ap_power_flag = OAL_FALSE;
        }
        else if (1 == power_flag) //�ϵ�
        {
            g_st_ap_config_info.l_ap_power_flag = OAL_TRUE;

            wal_wake_lock();
            l_ret = wlan_pm_open_etc();
            wal_wake_unlock();
            if (OAL_FAIL==l_ret)
            {
                 OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::wlan_pm_open_etc Fail!}\r\n");
                 oal_free(pc_command);
                 return -OAL_EFAIL;
            }
            else if(OAL_ERR_CODE_ALREADY_OPEN != l_ret)
            {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
                /* �����ϵ�ʱ��ΪFALSE */
                hwifi_config_init_force_etc();
#endif
                // �����ϵ糡�����·�����VAP
                l_ret = wal_cfg_vap_h2d_event_etc(pst_net_dev);
                if(OAL_SUCC!= l_ret)
                {
                    oal_free(pc_command);
                    return -OAL_EFAIL;
                }

                /* �ϵ�host device_stru��ʼ��*/
                l_ret = wal_host_dev_init_etc(pst_net_dev);
                if(OAL_SUCC != l_ret)
                {
                    OAL_IO_PRINT("wal_host_dev_init_etc FAIL %d \r\n",l_ret);
                    oal_free(pc_command);
                    return -OAL_EFAIL;
                }
            }
        }
        else
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::pupower_flag:%d error.}\r\n", power_flag);
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
#endif
    }
    else if(0 == oal_strncasecmp(pc_command, CMD_SET_POWER_MGMT_ON, OAL_STRLEN(CMD_SET_POWER_MGMT_ON)))
    {
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        struct wlan_pm_s  *pst_wlan_pm;
#endif

        oal_uint power_mgmt_flag = OAL_TRUE;  /* APģʽ,Ĭ�ϵ�Դ�����ǿ����� */
        oal_uint32 command_len = OAL_STRLEN(pc_command);
        /* ��ʽ:CMD_SET_POWER_MGMT_ON 1 or CMD_SET_POWER_MGMT_ON 0 */
        if(command_len < (OAL_STRLEN((oal_int8 *)CMD_SET_POWER_MGMT_ON) + 2))
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_SET_POWER_MGMT_ON cmd len:%d is error.}\r\n", command_len);

            oal_free(pc_command);
            return -OAL_EFAIL;
        }

        power_mgmt_flag = (oal_uint)oal_atoi(pc_command + OAL_STRLEN((oal_int8 *)CMD_SET_POWER_MGMT_ON) + 1);

        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_SET_POWER_MGMT_ON command,power_mgmt flag:%u}\r\n", power_mgmt_flag);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        pst_wlan_pm = wlan_pm_get_drv_etc();
        if(NULL != pst_wlan_pm)
        {
            pst_wlan_pm->ul_apmode_allow_pm_flag = power_mgmt_flag;
        }
        else
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::wlan_pm_get_drv_etc return null.");
        }
#endif

    }
    else if(0 == oal_strncasecmp(pc_command, CMD_COUNTRY, OAL_STRLEN(CMD_COUNTRY)))
    {
    #ifdef _PRE_WLAN_FEATURE_11D
        const oal_int8 *country_code;
        oal_int8        auc_country_code[3] = {0};
        oal_int32       l_ret;

        /* ��ʽ:COUNTRY CN */
        if(OAL_STRLEN(pc_command) < (OAL_STRLEN((oal_int8 *)CMD_COUNTRY) + 3))
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::puc_command len error.}\r\n");

            oal_free(pc_command);
            return -OAL_EFAIL;
        }

        country_code = pc_command + OAL_STRLEN((oal_int8 *)CMD_COUNTRY) + 1;

        oal_memcopy(auc_country_code, country_code, 2);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        if (OAL_TRUE == g_st_cust_country_code_ignore_flag.en_country_code_ingore_ini_flag)
        {
            OAM_WARNING_LOG3(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::wlan_pm_set_country[%c %c] is ignore, ini[%d]",
                            auc_country_code[0], auc_country_code[1], g_st_cust_country_code_ignore_flag.en_country_code_ingore_ini_flag);

            oal_free(pc_command);
            return OAL_SUCC;
        }
#endif

        l_ret = wal_regdomain_update_for_dfs_etc(pst_net_dev, auc_country_code);

        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::return err code [%d]!}\r\n", l_ret);

            oal_free(pc_command);
            return -OAL_EFAIL;
        }

        l_ret = wal_regdomain_update_etc(pst_net_dev, auc_country_code);

        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::return err code [%d]!}\r\n", l_ret);

            oal_free(pc_command);
            return -OAL_EFAIL;
        }
    #else
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::_PRE_WLAN_FEATURE_11D is not define!}\r\n");
    #endif
    }
    else if(0 == oal_strncasecmp(pc_command, CMD_GET_CAPA_DBDC, OAL_STRLEN(CMD_GET_CAPA_DBDC)))
    {
        oal_int32      cmd_len = OAL_STRLEN(CMD_CAPA_DBDC_SUPP);
        oal_int32      ret_len = 0;

        if ((oal_uint32)st_priv_cmd.l_total_len < OAL_STRLEN(CMD_CAPA_DBDC_SUPP))
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_GET_CAPA_DBDC length is to short. need %d}\r\n", OAL_STRLEN(CMD_CAPA_DBDC_SUPP));
            oal_free(pc_command);
            return -OAL_EFAIL;
        }

        /* ��buf���� */
        ret_len = OAL_MAX(st_priv_cmd.l_total_len, cmd_len);
        oal_memset(pc_command, 0, (oal_uint32)(ret_len + 1));
        pc_command[ret_len] = '\0';

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
        /* hi1103 support DBDC */
        oal_memcopy(pc_command, CMD_CAPA_DBDC_SUPP, OAL_STRLEN(CMD_CAPA_DBDC_SUPP));
#else
        /* other don't support DBDC */
        oal_memcopy(pc_command, CMD_CAPA_DBDC_NOT_SUPP, OAL_STRLEN(CMD_CAPA_DBDC_NOT_SUPP));
#endif

        l_ret = oal_copy_to_user(pst_ifr->ifr_data+8, pc_command, ret_len);
        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_android_priv_cmd_etc:CMD_GET_CAPA_DBDC Failed to copy ioctl_data to user !");
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_GET_CAPA_DBDC reply len=%d, l_ret=%d}\r\n", OAL_STRLEN(pc_command), l_ret);
    }
#ifdef _PRE_WLAN_FEATURE_LTECOEX
    else if(0 == oal_strncasecmp(pc_command, CMD_LTECOEX_MODE, OAL_STRLEN(CMD_LTECOEX_MODE)))
    {
        oal_int32      l_ret;
        oal_int8       ltecoex_mode;

        /* ��ʽ:LTECOEX_MODE 1 or LTECOEX_MODE 0 */
        if(OAL_STRLEN(pc_command) < (OAL_STRLEN((oal_int8 *)CMD_LTECOEX_MODE) + 2))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_LTECOEX_MODE length is to short [%d].}\r\n", OAL_STRLEN(pc_command));

            oal_free(pc_command);
            return -OAL_EFAIL;
        }
        ltecoex_mode = oal_atoi(pc_command + OAL_STRLEN((oal_int8 *)CMD_LTECOEX_MODE) + 1);

        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_LTECOEX_MODE command,ltecoex mode:%d}\r\n", ltecoex_mode);

        l_ret = (oal_int32)wal_ioctl_ltecoex_mode_set(pst_net_dev, (oal_int8*)&ltecoex_mode);
        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::return err code [%d]!}\r\n", l_ret);
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
    }
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    else if (oal_strncasecmp(pc_command, CMD_TX_POWER, OAL_STRLEN(CMD_TX_POWER)) == 0)
    {
        oal_uint32  ul_skip = OAL_STRLEN((oal_int8 *)CMD_TX_POWER) + 1;
        oal_uint16  ul_txpwr = (oal_uint16)oal_atoi(pc_command + ul_skip);
        l_ret = wal_ioctl_reduce_sar(pst_net_dev, ul_txpwr);
        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::reduce sar return err code [%d]!}\r\n", l_ret);
            oal_free(pc_command);
            /* ������ӡ�����룬���سɹ�����ֹsupplicant �ۼ�4�� ioctlʧ�ܵ���wifi�쳣���� */
            return OAL_SUCC;
        }
    }
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    else if (oal_strncasecmp(pc_command, CMD_SET_MEMO_CHANGE, OAL_STRLEN(CMD_SET_MEMO_CHANGE)) == 0)
    {
        if ((OAL_TRUE == g_aen_tas_switch_en[WLAN_RF_CHANNEL_ZERO]) || (OAL_TRUE == g_aen_tas_switch_en[WLAN_RF_CHANNEL_ONE]))
        {
            /* 0:Ĭ��̬ 1:tas̬ */
            l_param_1 = oal_atoi(pc_command + OAL_STRLEN((oal_int8 *)CMD_SET_MEMO_CHANGE) + 1);
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_SET_MEMO_CHANGE antIndex[%d].}", l_param_1);
            l_ret = board_wifi_tas_set(l_param_1);
        }
    }
    else if (oal_strncasecmp(pc_command, CMD_TAS_GET_ANT, OAL_STRLEN(CMD_TAS_GET_ANT)) == 0)
    {
        oal_free(pc_command);
        return board_get_wifi_tas_gpio_state();
    }
    else if (oal_strncasecmp(pc_command, CMD_MEASURE_TAS_RSSI, OAL_STRLEN(CMD_MEASURE_TAS_RSSI)) == 0)
    {
        l_param_1 = !!oal_atoi(pc_command + OAL_STRLEN((oal_int8 *)CMD_MEASURE_TAS_RSSI) + 1);
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_MEASURE_TAS_RSSI coreIndex[%d].}", l_param_1);
        /* �������� */
        l_ret = wal_ioctl_tas_rssi_access(pst_net_dev, l_param_1);
    }
    else if (oal_strncasecmp(pc_command, CMD_SET_TAS_TXPOWER, OAL_STRLEN(CMD_SET_TAS_TXPOWER)) == 0)
    {
        /* taş���� */
        pc_cmd_copy = pc_command;
        pc_cmd_copy += OAL_STRLEN((oal_int8 *)CMD_SET_TAS_TXPOWER);
        l_value = wal_get_cmd_one_arg_etc(pc_cmd_copy, ac_name, &ul_off_set);
        if (OAL_SUCC != l_value)
        {
             OAM_WARNING_LOG1(0, OAM_SF_TPC, "{wal_android_priv_cmd_etc::CMD_SET_TAS_TXPOWER return err_code [%d]!}\r\n", l_ret);
             oal_free(pc_command);
             return OAL_SUCC;
        }
        l_param_1 = !!oal_atoi(ac_name);
        /* ��ȡneedImprove���� */
        pc_cmd_copy += ul_off_set;
        l_ret = wal_get_cmd_one_arg_etc(pc_cmd_copy, ac_name, &ul_off_set);
        if (OAL_SUCC == l_ret)
        {
            l_param_2 = !!oal_atoi(ac_name);
            OAM_WARNING_LOG2(0, OAM_SF_TPC, "{wal_android_priv_cmd_etc::CMD_SET_TAS_TXPOWER coreIndex[%d] needImprove[%d].}",
                             l_param_1, l_param_2);
            /* TAS����̧���� */
            l_ret = wal_ioctl_tas_pow_ctrl(pst_net_dev, l_param_1, l_param_2);
        }
        else
        {
             oal_free(pc_command);
             return OAL_SUCC;
        }
    }
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    else if (oal_strncasecmp(pc_command, CMD_WPAS_GET_CUST, OAL_STRLEN(CMD_WPAS_GET_CUST)) == 0)
    {
        /* ��buf���� */
        oal_memset(pc_command, 0, st_priv_cmd.l_total_len + 1);
        pc_command[st_priv_cmd.l_total_len] = '\0';

        /* ��ȡȫ�����ƻ����ã���������ȡdisable_capab_ht40 */
        hwifi_config_host_global_ini_param();

        /* ��ֵht40��ֹλ */
        *pc_command = g_st_wlan_customize_etc.uc_disable_capab_2ght40;

        if(oal_copy_to_user(pst_ifr->ifr_data+8, pc_command, (oal_uint32)(st_priv_cmd.l_total_len)))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_android_priv_cmd_etc: Failed to copy ioctl_data to user !");
            oal_free(pc_command);
            /* ���ش���֪ͨsupplicant����ʧ�ܣ�supplicant���������������� */
            return -OAL_EFAIL;
        }
    }
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_VOWIFI
    else if (oal_strncasecmp(pc_command, CMD_VOWIFI_SET_PARAM, OAL_STRLEN(CMD_VOWIFI_SET_PARAM)) == 0)
    {
        l_ret = wal_ioctl_set_vowifi_param(pst_net_dev, pc_command);
        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::VOWIFI_SET_PARAM return err code [%d]!}", l_ret);
            oal_free(pc_command);
            return -OAL_EFAIL;
        }

    }
    else if (oal_strncasecmp(pc_command, CMD_VOWIFI_GET_PARAM, OAL_STRLEN(CMD_VOWIFI_GET_PARAM)) == 0)
    {
        l_value = 0;
        l_ret = wal_ioctl_get_vowifi_param(pst_net_dev, pc_command, &l_value);
        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_VOWIFI_GET_MODE(%d) return err code [%d]!}", l_ret);
            oal_free(pc_command);
            return -OAL_EFAIL;
        }

        /* ��buf���� */
        oal_memset(pc_command, 0, (oal_uint32)(st_priv_cmd.l_total_len + 1));
        pc_command[st_priv_cmd.l_total_len] = '\0';
        pl_value  = (oal_int32 *)pc_command;
        *pl_value = l_value;

        if(oal_copy_to_user(pst_ifr->ifr_data+8, pc_command, (oal_uint32)(st_priv_cmd.l_total_len)))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_android_priv_cmd_etc:CMD_VOWIFi_GET_MODE Failed to copy ioctl_data to user !");
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
    }
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
    else if(oal_strncasecmp(pc_command, CMD_GET_WIFI_PRIV_FEATURE_CAPABILITY, OAL_STRLEN(CMD_GET_WIFI_PRIV_FEATURE_CAPABILITY)) == 0)
    {
        l_value = 0;
        l_ret = wal_ioctl_get_wifi_priv_feature_cap_param(pst_net_dev, pc_command, &l_value);
        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_GET_WIFI_PRVI_FEATURE_CAPABILITY(%d) return err code [%d]!}", l_ret);
            oal_free(pc_command);
            return -OAL_EFAIL;
        }

        /* ��buf���� */
        oal_memset(pc_command, 0, (oal_uint32)(st_priv_cmd.l_total_len + 1));
        pc_command[st_priv_cmd.l_total_len] = '\0';
        pl_value  = (oal_int32 *)pc_command;
        *pl_value = l_value;

        OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_android_priv_cmd_etc::CMD_GET_WIFI_PRVI_FEATURE_CAPABILITY = [%x]!",*pl_value);

        if(oal_copy_to_user(pst_ifr->ifr_data+8, pc_command, (oal_uint32)(st_priv_cmd.l_total_len)))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_android_priv_cmd_etc:CMD_GET_WIFI_PRVI_FEATURE_CAPABILITY Failed to copy ioctl_data to user !");
            oal_free(pc_command);
            return -OAL_EFAIL;
        }

    }
    else if (oal_strncasecmp(pc_command, CMD_VOWIFI_IS_SUPPORT, OAL_STRLEN(CMD_VOWIFI_IS_SUPPORT)) == 0)
    {
        oal_int32      cmd_len = OAL_STRLEN(CMD_VOWIFI_IS_SUPPORT_REPLY);
        oal_int32      ret_len = 0;

        if ((oal_uint32)st_priv_cmd.l_total_len < OAL_STRLEN(CMD_VOWIFI_IS_SUPPORT_REPLY))
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_VOWIFI_IS_SUPPORT length is to short. need %d}\r\n", OAL_STRLEN(CMD_VOWIFI_IS_SUPPORT_REPLY));
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
        /* ��buf���� */
        ret_len = OAL_MAX(st_priv_cmd.l_total_len, cmd_len);
        oal_memset(pc_command, 0, (oal_uint32)(ret_len + 1));
        pc_command[ret_len] = '\0';
        oal_memcopy(pc_command, CMD_VOWIFI_IS_SUPPORT_REPLY, OAL_STRLEN(CMD_VOWIFI_IS_SUPPORT_REPLY));
        if(oal_copy_to_user(pst_ifr->ifr_data+8, pc_command, ret_len))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_android_priv_cmd_etc:CMD_VOWIFI_IS_SUPPORT Failed to copy ioctl_data to user !");
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
    }
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    else if (oal_strncasecmp(pc_command, CMD_FILTER_SWITCH, OAL_STRLEN(CMD_FILTER_SWITCH)) == 0)
    {
#ifdef CONFIG_DOZE_FILTER
        oal_int32 l_on;
        oal_uint32 command_len = OAL_STRLEN(pc_command);

        /* ��ʽ:FILTER 1 or FILTER 0 */
        if(command_len < (OAL_STRLEN((oal_int8 *)CMD_FILTER_SWITCH) + 2))
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_FILTER_SWITCH cmd len must equal or larger than 8. Now the cmd len:%d.}\r\n", command_len);

            oal_free(pc_command);
            return -OAL_EFAIL;
        }

        l_on = oal_atoi(pc_command + OAL_STRLEN((oal_int8 *)CMD_FILTER_SWITCH) + 1);

        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_FILTER_SWITCH %d.}", l_on);

        /* �����ں˽ӿڵ��� gWlanFilterOps.set_filter_enable */
        l_ret = hw_set_net_filter_enable(l_on);
        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_FILTER_SWITCH return err code [%d]!}", l_ret);
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
#else
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::Not support CMD_FILTER_SWITCH.}");
#endif
    }
#endif /* _PRE_WLAN_FEATURE_IP_FILTER */

#ifndef CONFIG_HAS_EARLYSUSPEND
    else if(0 == oal_strncasecmp(pc_command, CMD_SETSUSPENDOPT, OAL_STRLEN(CMD_SETSUSPENDOPT)))
    {
        /* Ŀǰ��û�з����ϲ��·�������,��ά����ֺ��ٷ��� */
        OAM_ERROR_LOG1(0,0,"wal_android_priv_cmd_etc:CMD_SETSUSPENDOPT param%d",*(pc_command + strlen(CMD_SETSUSPENDOPT) + 1) - '0');
        l_ret = OAL_SUCC;
    }
    else if (0 == oal_strncasecmp(pc_command, CMD_SETSUSPENDMODE, OAL_STRLEN(CMD_SETSUSPENDMODE)))
    {
        l_ret = wal_ioctl_set_suspend_mode(pst_net_dev, *(pc_command + strlen(CMD_SETSUSPENDMODE) + 1) - '0');
    }
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
   else if(0 == oal_strncasecmp(pc_command, CMD_SET_STA_PM_ON, OAL_STRLEN(CMD_SET_STA_PM_ON)))
   {
        wal_msg_write_stru                  st_write_msg;
        mac_cfg_ps_open_stru               *pst_sta_pm_open;

        if (OAL_STRLEN(pc_command) < (OAL_STRLEN((oal_int8 *)CMD_SET_STA_PM_ON + 2)))
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_SET_STA_PM_ON puc_command len error.}\r\n");

            oal_free(pc_command);
            return -OAL_EFAIL;
        }

        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_STA_PM_ON, OAL_SIZEOF(mac_cfg_ps_open_stru));

        /* ��������������� */
        pst_sta_pm_open = (mac_cfg_ps_open_stru *)(st_write_msg.auc_value);
        /* MAC_STA_PM_SWITCH_ON / MAC_STA_PM_SWITCH_OFF */
        pst_sta_pm_open->uc_pm_enable      = *(pc_command + OAL_STRLEN((oal_int8 *)CMD_SET_STA_PM_ON) + 1) - '0';
        pst_sta_pm_open->uc_pm_ctrl_type   = MAC_STA_PM_CTRL_TYPE_HOST;

        l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ps_open_stru),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);

        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::CMD_SET_STA_PM_ON return err code [%d]!}\r\n", l_ret);
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
   }
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
    else if(0 == oal_strncasecmp(pc_command, CMD_SET_M2S_SWITCH, OAL_STRLEN(CMD_SET_M2S_SWITCH)))
    {
        oal_uint8 uc_m2s_switch_on;

        if (OAL_STRLEN(pc_command) < (OAL_STRLEN((oal_int8 *)CMD_SET_M2S_SWITCH + 2)))
        {
            OAM_WARNING_LOG0(0, OAM_SF_M2S, "{wal_android_priv_cmd_etc::CMD_SET_M2S_SWITCH puc_command len error.}\r\n");

            oal_free(pc_command);
            return -OAL_EFAIL;
        }

        uc_m2s_switch_on = (oal_uint8)oal_atoi(pc_command + OAL_STRLEN((oal_int8 *)CMD_SET_M2S_SWITCH) + 1);

        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_android_priv_cmd_etc::CMD_SET_M2S_SWITCH %d.}", uc_m2s_switch_on);

        l_ret = wal_ioctl_set_m2s_mss(pst_net_dev, uc_m2s_switch_on);

        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_android_priv_cmd_etc::CMD_SET_M2S_SWITCH return err code [%d]!}\r\n", l_ret);
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
    }
    else if(0 == oal_strncasecmp(pc_command, CMD_SET_M2S_BLACKLIST, OAL_STRLEN(CMD_SET_M2S_BLACKLIST)))
    {
         oal_uint32 skip = OAL_STRLEN(CMD_SET_M2S_BLACKLIST) + 1;

         OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_android_priv_cmd_etc::CMD_SET_M2S_BLACKLIST[%d].}",  *(pc_command + skip));

         l_ret = wal_ioctl_set_m2s_blacklist(pst_net_dev, (pc_command + skip +1), *(pc_command + skip));
         if (OAL_UNLIKELY(OAL_SUCC != l_ret))
         {
             OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_android_priv_cmd_etc::CMD_SET_M2S_BLACKLIST return err code [%d]!}\r\n", l_ret);
             oal_free(pc_command);
             return -OAL_EFAIL;
         }
    }
#endif
#ifdef  _PRE_WLAN_FEATURE_FORCE_STOP_FILTER
    else if (0 == oal_strncasecmp(pc_command, CMD_RXFILTER_START, OAL_STRLEN(CMD_RXFILTER_START)))
    {
        wal_ioctl_force_stop_filter(pst_net_dev, OAL_FALSE);
    }
    else if (0 == oal_strncasecmp(pc_command, CMD_RXFILTER_STOP, OAL_STRLEN(CMD_RXFILTER_STOP)))
    {
        wal_ioctl_force_stop_filter(pst_net_dev, OAL_TRUE);
    }
#endif
    else
    {
        /* �������ڲ�֧�ֵ�������سɹ��������ϲ�wpa_supplicant��Ϊioctlʧ�ܣ������쳣����wifi */
        //OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd_etc::ignore unknow private command.}\r\n");
        l_ret = OAL_SUCC;
    }

    oal_free(pc_command);
    return l_ret;
#else
    return  OAL_SUCC;
#endif
}


#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) && (!defined(_PRE_PRODUCT_ID_HI110X_HOST)))
oal_int32 wal_witp_wifi_priv_cmd(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr)
{
    oal_net_dev_ioctl_data_stru     *pst_ioctl_data;  //����ʱ��������ջ֡���ȳ���1024����Щƽ̨���벻�����޸�Ϊ��̬����
    oal_int32                       l_ret              = 0;
    oal_bool_enum_uint8             en_get_flag        = OAL_FALSE;    //���get�ӿ�
    oal_uint32                      ul_get_data_size   = 0;    //get�ӿ��û��ռ���Ҫ���ص�����size

    if ((OAL_PTR_NULL == pst_ifr->ifr_data)||(OAL_PTR_NULL == pst_net_dev))
    {
        return -OAL_EINVAL;
    }

    pst_ioctl_data = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,OAL_SIZEOF(oal_net_dev_ioctl_data_stru),OAL_TRUE);
    if(OAL_PTR_NULL == pst_ioctl_data)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,"wal_witp_wifi_priv_cmd:Failed to alloc data(size=%u)!",OAL_SIZEOF(oal_net_dev_ioctl_data_stru));
        return -OAL_EINVAL;
    }

    /*���û�̬���ݿ������ں�̬*/
    if (oal_copy_from_user(pst_ioctl_data, pst_ifr->ifr_data, sizeof(oal_net_dev_ioctl_data_stru)))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_witp_wifi_priv_cmd:Failed to copy ioctl_data from user !");
        OAL_MEM_FREE(pst_ioctl_data,OAL_TRUE);
        return -OAL_EINVAL;
    }

    if((HWIFI_IOCTL_CMD_SET_AP_AUTH_ALG == pst_ioctl_data->l_cmd) ||
        (HWIFI_IOCTL_CMD_SET_WPS_IE == pst_ioctl_data->l_cmd) ||
        (HWIFI_IOCTL_CMD_SET_SSID == pst_ioctl_data->l_cmd) ||
        (HWIFI_IOCTL_CMD_SET_MAX_USER == pst_ioctl_data->l_cmd) ||
        (HWIFI_IOCTL_CMD_SET_FRAG == pst_ioctl_data->l_cmd) ||
        (HWIFI_IOCTL_CMD_SET_RTS == pst_ioctl_data->l_cmd) ||
        (HWIFI_IOCTL_CMD_SET_BEACON == pst_ioctl_data->l_cmd))
    {
        mac_vap_stru *pst_mac_vap;
        pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
        if(WLAN_VAP_MODE_CONFIG == pst_mac_vap->en_vap_mode && (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap->pst_mib_info)))
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_witp_wifi_priv_cmd::pst_mib_info is null!}\r\n");
            return -OAL_EINVAL;
        }
    }

    switch(pst_ioctl_data->l_cmd)
    {
        case HWIFI_IOCTL_CMD_GET_STA_ASSOC_REQ_IE:
        {
            l_ret = wal_ioctl_get_assoc_req_ie(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.assoc_req_ie);
            break;
        }
        case HWIFI_IOCTL_CMD_SET_AP_AUTH_ALG:
        {
            l_ret = wal_ioctl_set_auth_mode(pst_net_dev, pst_ioctl_data);

            break;
        }
        case HWIFI_IOCTL_CMD_SET_COUNTRY:
        {
            l_ret = wal_ioctl_set_country_code(pst_net_dev, pst_ioctl_data);
            break;
        }
        case HWIFI_IOCTL_CMD_SET_WPS_IE:
        {
            l_ret = wal_ioctl_set_wps_ie(pst_net_dev, pst_ioctl_data);
            break;
        }
        case HWIFI_IOCTL_CMD_SET_SSID:
        {
            l_ret = wal_ioctl_set_ssid(pst_net_dev, pst_ioctl_data);
            break;
        }
        case HWIFI_IOCTL_CMD_SET_MAX_USER:
        {
            l_ret = wal_ioctl_set_max_user(pst_net_dev, pst_ioctl_data);
            break;
        }
        case HWIFI_IOCTL_CMD_PRIV_CONNECT:
        {
            l_ret = wal_ioctl_nl80211_priv_connect(pst_net_dev, pst_ioctl_data);
            break;
        }
        case HWIFI_IOCTL_CMD_PRIV_DISCONNECT:
        {
            l_ret = wal_ioctl_nl80211_priv_disconnect(pst_net_dev, pst_ioctl_data);
            break;
        }
        case HWIFI_IOCTL_CMD_SET_FREQ:
        {
            l_ret = wal_ioctl_set_channel(pst_net_dev, pst_ioctl_data);
            break;
        }
        case HWIFI_IOCTL_CMD_SET_FRAG:
        {
            l_ret = wal_ioctl_set_frag(pst_net_dev, pst_ioctl_data);
            break;
        }
        case HWIFI_IOCTL_CMD_SET_RTS:
        {
            l_ret = wal_ioctl_set_rts(pst_net_dev, pst_ioctl_data);
            break;
        }
        case HWIFI_IOCTL_CMD_GET_ALL_STA_INFO:
        {
            pst_ioctl_data->pri_data.all_sta_link_info_ext.sta_info_ext = OAL_PTR_NULL;
            l_ret = wal_ioctl_get_all_sta_info(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.all_sta_link_info);
            break;
        }
        case HWIFI_IOCTL_CMD_SET_VENDOR_IE:
        {
            l_ret = wal_ioctl_set_vendor_ie(pst_net_dev, pst_ioctl_data);
            break;
        }
#ifdef _PRE_WLAN_FEATURE_HILINK
        case HWIFI_IOCTL_CMD_PRIV_KICK_USER:
        {
            l_ret = wal_ioctl_nl80211_priv_fbt_kick_user(pst_net_dev, pst_ioctl_data);
            break;
        }
        case HWIFI_IOCTL_CMD_SET_WHITE_LST_SSIDHIDEN:
        {
            l_ret = wal_ioctl_set_white_lst_ssidhiden(pst_net_dev, pst_ioctl_data);
            break;
        }
        case HWIFI_IOCTL_CMD_FBT_SCAN:
        {
            l_ret = wal_ioctl_start_fbt_scan(pst_net_dev, pst_ioctl_data);
            break;
        }
        case HWIFI_IOCTL_CMD_GET_STA_11K_ABILITY:
        {
            l_ret = wal_ioctl_get_sta_11k_ability(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.fbt_get_sta_11k_ability);
            break;
        }
        case HWIFI_IOCTL_CMD_SET_STA_BCN_REQUEST:
        {
            l_ret = wal_ioctl_set_sta_bcn_request(pst_net_dev, pst_ioctl_data);
            break;
        }
        case HWIFI_IOCTL_CMD_GET_STA_11V_ABILITY:
        {
            l_ret = wal_ioctl_get_sta_11v_ability(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.fbt_get_sta_11v_ability);
            break;
        }
        case HWIFI_IOCTL_CMD_11V_CHANGE_AP:
        {
            l_ret = wal_ioctl_change_to_other_ap(pst_net_dev, pst_ioctl_data);
            break;
        }
        case HWIFI_IOCTL_CMD_GET_CUR_CHANNEL:
        {
            l_ret = wal_ioctl_get_cur_channel(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.fbt_get_cur_channel);
            break;
        }
        case HWIFI_IOCTL_CMD_SET_SCAN_STAY_TIME:
        {
            l_ret = wal_ioctl_set_stay_time(pst_net_dev, pst_ioctl_data);
            break;
        }

#endif
        case HWIFI_IOCTL_CMD_GET_STA_11H_ABILITY:
        {
            l_ret = wal_ioctl_get_sta_11h_ability(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.fbt_get_sta_11h_ability);
            break;
        }
#ifdef _PRE_WLAN_FEATURE_11R_AP
        case HWIFI_IOCTL_CMD_GET_STA_11R_ABILITY:
        {
            l_ret = wal_ioctl_get_sta_11r_ability(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.fbt_get_sta_11r_ability);
            break;
        }
        case HWIFI_IOCTL_CMD_SET_MLME:
        {
            l_ret = wal_ioctl_set_mlme(pst_net_dev, pst_ioctl_data);
            break;
        }
#endif
#ifdef _PRE_WLAN_WEB_CMD_COMM
        case HWIFI_IOCTL_CMD_SET_BEACON:
        {
            l_ret = wal_cfg2io_set_beacon(pst_net_dev, &(pst_ioctl_data->pri_data.st_cfg_ap));
            break;
        }

        case HWIFI_IOCTL_CMD_GET_NEIGHB_INFO:
        {
            l_ret = wal_config_get_neighb_info(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.st_ap_info);
            break;
        }

        case HWIFI_IOCTL_CMD_GET_HW_STAT:
        {
            l_ret = wal_config_get_hw_flow_stat(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.st_machw_stat);
            break;
        }

        case HWIFI_IOCTL_CMD_GET_WME_STAT:
        {
            l_ret = wal_config_get_wme_stat(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.st_wme_stat);
            break;
        }

        case HWIFI_IOCTL_CMD_GET_TX_DELAY_AC:
        {
            l_ret = wal_config_get_tx_delay_ac(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.st_tx_delay_ac);
            break;
        }
#endif
#if defined(_PRE_WLAN_FEATURE_MCAST) || defined(_PRE_WLAN_FEATURE_HERA_MCAST)
        case HWIFI_IOCTL_CMD_GET_SNOOP_TABLE:
        {
            l_ret = wal_config_get_snoop_table(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.st_all_snoop_group);
            break;
        }
#endif
#ifdef _PRE_WLAN_FEATURE_WDS
        case HWIFI_IOCTL_CMD_GET_VAP_WDS_INFO:
        {
            l_ret = wal_config_get_wds_vap_info(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.st_wds_info);
            break;
        }
#endif
        case HWIFI_IOCTL_CMD_GET_ALL_STA_INFO_EXT:
        {
            l_ret = wal_ioctl_get_all_sta_info(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.all_sta_link_info_ext);
            break;
        }
#ifdef _PRE_WLAN_FEATURE_CAR
        case HWIFI_IOCTL_CMD_GET_CAR_INFO:
        {
            l_ret = wal_ioctl_get_car_info(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.st_car_info);
            break;
        }
#endif
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
        case HWIFI_IOCTL_CMD_GET_BLKWHTLST:
        {
            l_ret = wal_ioctl_get_blkwhtlst(pst_net_dev, pst_ioctl_data);
            en_get_flag = OAL_TRUE;
            ul_get_data_size = OAL_SIZEOF(pst_ioctl_data->pri_data.st_blkwhtlst);
            break;
        }
#endif
        default:
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_witp_wifi_priv_cmd::no matched CMD !}\r\n");
            l_ret = -OAL_EFAIL;
            break;
    }

    if((OAL_SUCC == l_ret)&&(en_get_flag))
    {
        if (oal_copy_to_user(pst_ifr->ifr_data,pst_ioctl_data,ul_get_data_size+OAL_SIZEOF(pst_ioctl_data->l_cmd)))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_witp_wifi_priv_cmd:Failed to copy ioctl_data to user !");
            l_ret = -OAL_EINVAL;
        }
    }

    OAL_MEM_FREE(pst_ioctl_data,OAL_TRUE);
    return (l_ret);
}
#endif


oal_int32 wal_net_device_ioctl(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, oal_int32 ul_cmd)
{
    oal_int32                           l_ret   = 0;

    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pst_ifr))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_net_device_ioctl::pst_dev %p, pst_ifr %p!}\r\n",
                       pst_net_dev, pst_ifr);
        return -OAL_EFAUL;
    }

    if (OAL_PTR_NULL == pst_ifr->ifr_data)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_net_device_ioctl::pst_ifr->ifr_data is NULL, ul_cmd[0x%x]!}\r\n", ul_cmd);
        return -OAL_EFAUL;
    }

    /* 1102 wpa_supplicant ͨ��ioctl �·����� */
    if (WAL_SIOCDEVPRIVATE+1 == ul_cmd)
    {
        l_ret = wal_android_priv_cmd_etc(pst_net_dev, pst_ifr, ul_cmd);
        return l_ret;
    }
#if (_PRE_OS_VERSION_WIN32 != _PRE_OS_VERSION)

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /*atcmdsrv ͨ��ioctl�·�����*/
    else if ( (WAL_SIOCDEVPRIVATE + 2) == ul_cmd )
    {
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        if (!capable(CAP_NET_ADMIN))
        {
            return -EPERM;
        }
#endif
        if (HI1XX_ANDROID_BUILD_VARIANT_ROOT == hi11xx_get_android_build_variant())
        {
            wal_wake_lock();
            l_ret = wal_atcmdsrv_wifi_priv_cmd_etc(pst_net_dev, pst_ifr, ul_cmd);
            wal_wake_unlock();
        }

        return l_ret;
    }
#endif
#endif
#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) && (!defined(_PRE_PRODUCT_ID_HI110X_HOST)))
    /* 51ioctl����ع����� */
    /*51 ͨ��ioctl�·�����*/
    else if(WAL_SIOCDEVPRIVATE == ul_cmd)
    {
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        if (!capable(CAP_NET_ADMIN))
        {
            return -EPERM;
        }
#endif
        l_ret = wal_witp_wifi_priv_cmd(pst_net_dev, pst_ifr);
        return l_ret;
    }
    /* 51ioctl����ع����� */
#endif
    else
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_net_device_ioctl::unrecognised cmd[0x%x]!}\r\n", ul_cmd);
        return OAL_SUCC;
    }
}



OAL_STATIC oal_uint32  wal_hipriv_set_mode(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_int8                    ac_mode_str[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};     /* Ԥ��Э��ģʽ�ַ����ռ� */
    oal_uint8                   uc_prot_idx;
    mac_cfg_mode_param_stru    *pst_mode_param;
    wal_msg_write_stru          st_write_msg;
    oal_uint32                  ul_off_set;
    oal_uint32                  ul_ret;
    oal_int32                   l_ret;
    //mac_vap_stru                *pst_mac_vap;
    wal_msg_stru               *pst_rsp_msg = OAL_PTR_NULL;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pc_param)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_hipriv_set_mode::pst_net_dev/p_param null ptr error %d!}\r\n", pst_net_dev, pc_param);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

#if 0

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_UNLIKELY(NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_hipriv_set_mode::can't get mac vap from netdevice priv data!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* �豸��up״̬���������ã�������down */
    if (pst_mac_vap->en_vap_state != MAC_VAP_STATE_INIT)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_mode::device is busy, please down it first %d!}\r\n", pst_mac_vap->en_vap_state);
        return -OAL_EBUSY;
    }
#endif
    /* pc_paramָ����ģʽ����, ����ȡ����ŵ�ac_mode_str�� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_mode_str, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mode::wal_get_cmd_one_arg_etc vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    ac_mode_str[OAL_SIZEOF(ac_mode_str) - 1] = '\0';    /* ȷ����null��β */

    for (uc_prot_idx = 0; OAL_PTR_NULL != g_ast_mode_map_etc[uc_prot_idx].pc_name; uc_prot_idx++)
    {
        l_ret = oal_strcmp(g_ast_mode_map_etc[uc_prot_idx].pc_name, ac_mode_str);

        if (0 == l_ret)
        {
            break;
        }
    }

    if (OAL_PTR_NULL == g_ast_mode_map_etc[uc_prot_idx].pc_name)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_mode::unrecognized protocol string!}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }


    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_MODE, OAL_SIZEOF(mac_cfg_mode_param_stru));

    pst_mode_param = (mac_cfg_mode_param_stru *)(st_write_msg.auc_value);
    pst_mode_param->en_protocol  = g_ast_mode_map_etc[uc_prot_idx].en_mode;
    pst_mode_param->en_band      = g_ast_mode_map_etc[uc_prot_idx].en_band;
    pst_mode_param->en_bandwidth = g_ast_mode_map_etc[uc_prot_idx].en_bandwidth;

    OAM_INFO_LOG3(0, OAM_SF_CFG, "{wal_hipriv_set_mode::protocol[%d],band[%d],bandwidth[%d]!}\r\n",
                            pst_mode_param->en_protocol, pst_mode_param->en_band, pst_mode_param->en_bandwidth);

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_mode_param_stru),
                                (oal_uint8 *)&st_write_msg,
                                OAL_TRUE,
                                &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mode::wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_set_mode fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_set_freq(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_freq;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_freq[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                  ul_ret;
    oal_int32                   l_ret;
    wal_msg_stru                *pst_rsp_msg = OAL_PTR_NULL;

    /*set freq��������FIFO�����������������ֱ�����ŵ������ŵ�����Ҫ�ȹس����ٿ������Ż���Ч*/
#if 0
    mac_vap_stru                *pst_mac_vap;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_UNLIKELY(NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_hipriv_set_freq::can't get mac vap from netdevice priv data!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* �豸��up״̬���������ã�������down */
    if (pst_mac_vap->en_vap_state != MAC_VAP_STATE_INIT)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_freq::device is busy, please down it firs %d!}\r\n", pst_mac_vap->en_vap_state);
        return -OAL_EBUSY;
    }
#endif
    /* pc_paramָ���´�����net_device��name, ����ȡ����ŵ�ac_name�� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_freq, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_freq::wal_get_cmd_one_arg_etc vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    l_freq = oal_atoi(ac_freq);
    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_freq::l_freq = %d!}\r\n", l_freq);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
#ifdef _PRE_WLAN_WEB_CMD_COMM
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CHANNEL, OAL_SIZEOF(oal_int32));
#else
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CURRENT_CHANEL, OAL_SIZEOF(oal_int32));
#endif
    *((oal_int32 *)(st_write_msg.auc_value)) = l_freq;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_freq::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

        /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_set_freq fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11D

OAL_STATIC oal_bool_enum_uint8  wal_is_alpha_upper(oal_int8 c_letter)
{
    if (c_letter >= 'A' && c_letter <= 'Z')
    {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


oal_uint8  wal_regdomain_get_band_etc(oal_uint32 ul_start_freq, oal_uint32 ul_end_freq)
{
    if (ul_start_freq > 2400 && ul_end_freq < 2500)
    {
        return MAC_RC_START_FREQ_2;
    }
    else if (ul_start_freq > 5000 && ul_end_freq < 5870)
    {
        return MAC_RC_START_FREQ_5;
    }
    else if (ul_start_freq > 4900 && ul_end_freq < 4999)
    {
        return MAC_RC_START_FREQ_5;
    }
    else
    {
        return MAC_RC_START_FREQ_BUTT;
    }
}


oal_uint8  wal_regdomain_get_bw_etc(oal_uint8 uc_bw)
{
    oal_uint8 uc_bw_map;

    switch (uc_bw)
    {
        case 80:
                uc_bw_map = MAC_CH_SPACING_80MHZ;
                break;
        case 40:
                uc_bw_map = MAC_CH_SPACING_40MHZ;
                break;
        case 20:
                uc_bw_map = MAC_CH_SPACING_20MHZ;
                break;
        default:
                uc_bw_map = MAC_CH_SPACING_BUTT;
                break;
    };

    return uc_bw_map;
}


oal_uint32  wal_regdomain_get_channel_2g_etc(oal_uint32 ul_start_freq, oal_uint32 ul_end_freq)
{
    oal_uint32 ul_freq;
    oal_uint32 ul_i;
    oal_uint32 ul_ch_bmap = 0;

    for (ul_freq = ul_start_freq + 10; ul_freq <= (ul_end_freq - 10); ul_freq++)
    {
        for (ul_i = 0; ul_i < MAC_CHANNEL_FREQ_2_BUTT; ul_i++)
        {
            if (ul_freq == g_ast_freq_map_2g_etc[ul_i].us_freq)
            {
                ul_ch_bmap |= (1 << ul_i);
            }
        }
    }

    return ul_ch_bmap;
}


oal_uint32  wal_regdomain_get_channel_5g_etc(oal_uint32 ul_start_freq, oal_uint32 ul_end_freq)
{
    oal_uint32 ul_freq;
    oal_uint32 ul_i;
    oal_uint32 ul_ch_bmap = 0;

    for (ul_freq = ul_start_freq + 10; ul_freq <= (ul_end_freq - 10); ul_freq += 5)
    {
        for (ul_i = 0; ul_i < MAC_CHANNEL_FREQ_5_BUTT; ul_i++)
        {
            if (ul_freq == g_ast_freq_map_5g_etc[ul_i].us_freq)
            {
                ul_ch_bmap |= (1 << ul_i);
            }
        }
    }

    return ul_ch_bmap;

}


oal_uint32  wal_regdomain_get_channel_etc(oal_uint8 uc_band, oal_uint32 ul_start_freq, oal_uint32 ul_end_freq)
{
    oal_uint32 ul_ch_bmap = 0;;

    switch (uc_band)
    {
        case MAC_RC_START_FREQ_2:
            ul_ch_bmap = wal_regdomain_get_channel_2g_etc(ul_start_freq, ul_end_freq);
            break;

        case MAC_RC_START_FREQ_5:
            ul_ch_bmap = wal_regdomain_get_channel_5g_etc(ul_start_freq, ul_end_freq);
            break;

        default:
            break;
    }

    return ul_ch_bmap;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,3,0))
extern oal_ieee80211_supported_band g_st_supported_band_2ghz_info;
oal_uint32 wal_linux_update_wiphy_channel_list_num_etc(oal_net_device_stru *pst_net_dev, oal_wiphy_stru *pst_wiphy)
{
    oal_uint16 us_len;
    oal_uint32 ul_ret;
    mac_vendor_cmd_channel_list_stru st_channel_list;
    mac_vap_stru                    *pst_mac_vap;

    if (pst_wiphy == OAL_PTR_NULL || pst_net_dev == OAL_PTR_NULL)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num_etc::wiphy %p, net_dev %p}", pst_wiphy, pst_net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (pst_mac_vap == OAL_PTR_NULL)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num_etc::NET_DEV_PRIV is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_config_vendor_cmd_get_channel_list_etc(pst_mac_vap, &us_len, (oal_uint8 *)(&st_channel_list));
    if (ul_ret != OAL_SUCC)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num_etc::get_channel_list fail. %d}", ul_ret);
        return ul_ret;
    }

    /* ֻ����2G�ŵ�������5G�ŵ����ڴ���DFS �����Ҵ�����㲢������,����Ҫ�޸� */
    g_st_supported_band_2ghz_info.n_channels = st_channel_list.uc_channel_num_2g;

    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num_etc::2g_channel_num = %d, 5g_channel_num = %d}",
                                st_channel_list.uc_channel_num_2g,
                                st_channel_list.uc_channel_num_5g);
    return OAL_SUCC;
}
#endif




OAL_STATIC OAL_INLINE oal_void  wal_get_dfs_domain(mac_regdomain_info_stru *pst_mac_regdom, OAL_CONST oal_int8 *pc_country)
{
    oal_uint32    u_idx;
    oal_uint32    ul_size = OAL_ARRAY_SIZE(g_ast_dfs_domain_table_etc);

    for (u_idx = 0; u_idx < ul_size; u_idx++)
    {
        if (0 == oal_strcmp(g_ast_dfs_domain_table_etc[u_idx].pc_country, pc_country))
        {
            pst_mac_regdom->en_dfs_domain = g_ast_dfs_domain_table_etc[u_idx].en_dfs_domain;

            return;
        }
    }

    pst_mac_regdom->en_dfs_domain = MAC_DFS_DOMAIN_NULL;
}


OAL_STATIC oal_void  wal_regdomain_fill_info(OAL_CONST oal_ieee80211_regdomain_stru *pst_regdom, mac_regdomain_info_stru *pst_mac_regdom)
{
    oal_uint32  ul_i;
    oal_uint32  ul_start;
    oal_uint32  ul_end;
    oal_uint8   uc_band;
    oal_uint8   uc_bw;

    /* ���ƹ����ַ��� */
    pst_mac_regdom->ac_country[0] = pst_regdom->alpha2[0];
    pst_mac_regdom->ac_country[1] = pst_regdom->alpha2[1];
    pst_mac_regdom->ac_country[2] = 0;

    /* ��ȡDFS��֤��׼���� */
    wal_get_dfs_domain(pst_mac_regdom, pst_regdom->alpha2);

    /* ����������� */
    pst_mac_regdom->uc_regclass_num = (oal_uint8)pst_regdom->n_reg_rules;

    /* ����������Ϣ */
    for (ul_i = 0; ul_i < pst_regdom->n_reg_rules; ul_i++)
    {
        /* ��д�������Ƶ��(2.4G��5G) */
        ul_start = pst_regdom->reg_rules[ul_i].freq_range.start_freq_khz / 1000;
        ul_end   = pst_regdom->reg_rules[ul_i].freq_range.end_freq_khz / 1000;
        uc_band  = wal_regdomain_get_band_etc(ul_start, ul_end);
        pst_mac_regdom->ast_regclass[ul_i].en_start_freq = uc_band;

        /* ��д����������������� */
        uc_bw = (oal_uint8)(pst_regdom->reg_rules[ul_i].freq_range.max_bandwidth_khz / 1000);
        pst_mac_regdom->ast_regclass[ul_i].en_ch_spacing = wal_regdomain_get_bw_etc(uc_bw);

        /* ��д�������ŵ�λͼ */
        pst_mac_regdom->ast_regclass[ul_i].ul_channel_bmap = wal_regdomain_get_channel_etc(uc_band, ul_start, ul_end);

        /* ��ǹ�������Ϊ */
        pst_mac_regdom->ast_regclass[ul_i].uc_behaviour_bmap = 0;

        if (pst_regdom->reg_rules[ul_i].flags & NL80211_RRF_DFS)
        {
            pst_mac_regdom->ast_regclass[ul_i].uc_behaviour_bmap |= MAC_RC_DFS;
        }

        if (pst_regdom->reg_rules[ul_i].flags & NL80211_RRF_NO_INDOOR)
        {
            pst_mac_regdom->ast_regclass[ul_i].uc_behaviour_bmap |= MAC_RC_NO_INDOOR;
        }

        if (pst_regdom->reg_rules[ul_i].flags & NL80211_RRF_NO_OUTDOOR)
        {
            pst_mac_regdom->ast_regclass[ul_i].uc_behaviour_bmap |= MAC_RC_NO_OUTDOOR;
        }

        /* ��串���������͹��� */
        pst_mac_regdom->ast_regclass[ul_i].uc_coverage_class = 0;
        pst_mac_regdom->ast_regclass[ul_i].uc_max_reg_tx_pwr = (oal_uint8)(pst_regdom->reg_rules[ul_i].power_rule.max_eirp / 100);
        pst_mac_regdom->ast_regclass[ul_i].us_max_tx_pwr     = (oal_uint16)(pst_regdom->reg_rules[ul_i].power_rule.max_eirp / 10);

    }
}


oal_int32  wal_regdomain_update_etc(oal_net_device_stru *pst_net_dev, oal_int8 *pc_country)
{
#ifndef _PRE_SUPPORT_ACS
    oal_uint8                               uc_dev_id;
    mac_device_stru                        *pst_device;
#endif
    OAL_CONST oal_ieee80211_regdomain_stru *pst_regdom;
    oal_uint16                              us_size;
    mac_regdomain_info_stru                *pst_mac_regdom;
    wal_msg_write_stru                      st_write_msg;
    mac_cfg_country_stru                   *pst_param;
    oal_int32                               l_ret;
    wal_msg_stru                           *pst_rsp_msg = OAL_PTR_NULL;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    oal_int8 old_pc_country[COUNTRY_CODE_LEN] = {'9','9'};
#endif
#ifndef _PRE_SUPPORT_ACS
    mac_vap_stru                           *pst_mac_vap = OAL_PTR_NULL;
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    oal_memcopy(old_pc_country, hwifi_get_country_code_etc(), COUNTRY_CODE_LEN);
    hwifi_set_country_code_etc(pc_country, COUNTRY_CODE_LEN);

    /* ����µĹ�����;ɹ��Ҵ���һ��regdomain����ˢ��RF������ֻ���¹����� */
    if (OAL_TRUE == hwifi_is_regdomain_changed_etc((oal_uint8 *)old_pc_country, (oal_uint8 *)pc_country))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update_etc::regdomain changed, refresh rf params.!}\r\n");

        /* ˢ�²���ʧ�ܣ�Ϊ�˱�֤������͹��ʲ�����Ӧ */
        /* �����������ԭ���Ĺ����룬���θ���ʧ�� */
        if (hwifi_force_refresh_rf_params(pst_net_dev) != OAL_SUCC)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY,
                "{wal_regdomain_update_etc::refresh rf(max_txpwr & dbb scale) params failed. Set country back.!}\r\n");
            hwifi_set_country_code_etc(old_pc_country, COUNTRY_CODE_LEN);
        }
    }
#endif


    if (!wal_is_alpha_upper(pc_country[0]) || !wal_is_alpha_upper(pc_country[1]))
    {
        if (('9' == pc_country[0]) && ('9' == pc_country[1]))
        {
            OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update_etc::set regdomain to 99!}\r\n");
        }
        else
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update_etc::country str is invalid!}\r\n");
            return -OAL_EINVAL;
        }
    }

    pst_regdom = wal_regdb_find_db_etc(pc_country);
    if (OAL_PTR_NULL == pst_regdom)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update_etc::no regdomain db was found!}\r\n");
        return -OAL_EINVAL;
    }

    us_size = (oal_uint16)(OAL_SIZEOF(mac_regclass_info_stru) * pst_regdom->n_reg_rules + MAC_RD_INFO_LEN);

    /* �����ڴ��Ź�������Ϣ�����ڴ�ָ����Ϊ�¼�payload����ȥ */
    /* �˴�������ڴ����¼��������ͷ�(hmac_config_set_country_etc) */
    pst_mac_regdom = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, us_size, OAL_TRUE);
    if (OAL_PTR_NULL == pst_mac_regdom)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_regdomain_update_etc::alloc regdom mem fail(size:%d), return null ptr!}\r\n",us_size);
        return -OAL_ENOMEM;
    }

    wal_regdomain_fill_info(pst_regdom, pst_mac_regdom);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_COUNTRY, OAL_SIZEOF(mac_cfg_country_stru));

    /* ��дWID��Ӧ�Ĳ��� */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    pst_mac_regdom->en_regdomain = hwifi_get_regdomain_from_country_code(pc_country);
#endif
    pst_param = (mac_cfg_country_stru *)(st_write_msg.auc_value);
    pst_param->p_mac_regdom = pst_mac_regdom;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_country_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_regdomain_update_etc::return err code %d!}\r\n", l_ret);
        if((-OAL_ETIMEDOUT != l_ret)&&(-OAL_EFAUL != l_ret))
        {
            OAL_MEM_FREE(pst_mac_regdom, OAL_TRUE);
            pst_mac_regdom = OAL_PTR_NULL;
        }
        if (OAL_PTR_NULL != pst_rsp_msg)
        {
            oal_free(pst_rsp_msg);
        }
        return l_ret;
    }
    oal_free(pst_rsp_msg);

    /* ������֧��ACSʱ������hostapd��������Ϣ; �������֧��ACS������Ҫ���£�����hostapd�޷�����DFS�ŵ� */
#ifndef _PRE_SUPPORT_ACS
    pst_mac_vap = (mac_vap_stru *)OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update_etc::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}");
        return -OAL_FAIL;
    }

    uc_dev_id = pst_mac_vap->uc_device_id;
    pst_device = mac_res_get_dev_etc(uc_dev_id);

    if((OAL_PTR_NULL != pst_device) && (OAL_PTR_NULL != pst_device->pst_wiphy))
    {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,3,0))
        
        wal_linux_update_wiphy_channel_list_num_etc(pst_net_dev, pst_device->pst_wiphy);
#endif
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_regdomain_update_etc::update regdom to kernel.%c,%c}\r\n",
            pc_country[0], pc_country[1]);
        wal_cfg80211_reset_bands_etc(uc_dev_id);
        oal_wiphy_apply_custom_regulatory(pst_device->pst_wiphy, pst_regdom);
        
        wal_cfg80211_save_bands_etc(uc_dev_id);
    }
#endif

    return OAL_SUCC;
}

oal_int32  wal_regdomain_update_for_dfs_etc(oal_net_device_stru *pst_net_dev, oal_int8 *pc_country)
{
    OAL_CONST oal_ieee80211_regdomain_stru *pst_regdom;
    oal_uint16                              us_size;
    mac_regdomain_info_stru                *pst_mac_regdom;
    wal_msg_write_stru                      st_write_msg;
    mac_dfs_domain_enum_uint8              *pst_param;
    oal_int32                               l_ret;

    if (!wal_is_alpha_upper(pc_country[0]) || !wal_is_alpha_upper(pc_country[1]))
    {
        if (('9' == pc_country[0]) && ('9' == pc_country[1]))
        {
            OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs_etc::set regdomain to 99!}\r\n");
        }
        else
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs_etc::country str is invalid!}\r\n");
            return -OAL_EINVAL;
        }
    }

    pst_regdom = wal_regdb_find_db_etc(pc_country);
    if (OAL_PTR_NULL == pst_regdom)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs_etc::no regdomain db was found!}\r\n");
        return -OAL_EINVAL;
    }

    us_size = (oal_uint16)(OAL_SIZEOF(mac_regclass_info_stru) * pst_regdom->n_reg_rules + MAC_RD_INFO_LEN);

    /* �����ڴ��Ź�������Ϣ,�ڱ������������ͷ� */
    pst_mac_regdom = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, us_size, OAL_TRUE);
    if (OAL_PTR_NULL == pst_mac_regdom)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs_etc::alloc regdom mem fail, return null ptr!us_size[%d]}\r\n", us_size);
        return -OAL_ENOMEM;
    }

    wal_regdomain_fill_info(pst_regdom, pst_mac_regdom);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_COUNTRY_FOR_DFS, OAL_SIZEOF(mac_dfs_domain_enum_uint8));

    /* ��дWID��Ӧ�Ĳ��� */
    pst_param = (mac_dfs_domain_enum_uint8 *)(st_write_msg.auc_value);
    *pst_param = pst_mac_regdom->en_dfs_domain;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_dfs_domain_enum_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        /*pst_mac_regdom�ڴ棬�˴��ͷ� */
        OAL_MEM_FREE(pst_mac_regdom, OAL_TRUE);
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs_etc::return err code %d!}\r\n", l_ret);
        return l_ret;
    }
    /*pst_mac_regdom�ڴ棬�˴��ͷ� */
    OAL_MEM_FREE(pst_mac_regdom, OAL_TRUE);

    return OAL_SUCC;
}


oal_uint32  wal_regdomain_update_sta_etc(oal_uint8 uc_vap_id)
{
    oal_int8                      *pc_desired_country;

    oal_net_device_stru           *pst_net_dev;
    oal_int32                      l_ret;
    oal_bool_enum_uint8            us_updata_rd_by_ie_switch = OAL_FALSE;

    hmac_vap_get_updata_rd_by_ie_switch_etc(uc_vap_id, &us_updata_rd_by_ie_switch);

    if(OAL_TRUE == us_updata_rd_by_ie_switch)
    {
        pc_desired_country = hmac_vap_get_desired_country_etc(uc_vap_id);

        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_desired_country))
        {
            OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta_etc::pc_desired_country is null ptr!}\r\n");
            return OAL_ERR_CODE_PTR_NULL;
        }


        /* �����Ĺ�����ȫΪ0����ʾ�Զ�AP�Ĺ����벻���ڣ�����sta��ǰĬ�ϵĹ����� */
        if ((0 == pc_desired_country[0]) && (0 == pc_desired_country[1]))
        {
            OAM_INFO_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta_etc::ap does not have country ie, use default!}\r\n");
            return OAL_SUCC;
        }

        pst_net_dev = hmac_vap_get_net_device_etc(uc_vap_id);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
        {
            OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta_etc::pst_net_dev is null ptr!}\r\n");
            return OAL_ERR_CODE_PTR_NULL;
        }

        l_ret = wal_regdomain_update_for_dfs_etc(pst_net_dev, pc_desired_country);
        if (l_ret != OAL_SUCC)
        {
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta_etc::wal_regdomain_update_etc err code %d!}\r\n", l_ret);
            return OAL_FAIL;
        }

        l_ret = wal_regdomain_update_etc(pst_net_dev, pc_desired_country);
        if (l_ret != OAL_SUCC)
        {
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta_etc::wal_regdomain_update_etc err code %d!}\r\n", l_ret);
            return OAL_FAIL;
        }


        OAM_INFO_LOG2(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta_etc::country is %u, %u!}\r\n", (oal_uint8)pc_desired_country[0], (oal_uint8)pc_desired_country[1]);

    }
    else
    {
        OAM_INFO_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta_etc::us_updata_rd_by_ie_switch is OAL_FALSE!}\r\n");
    }
    return OAL_SUCC;
}


oal_int32 wal_regdomain_update_country_code_etc(oal_net_device_stru *pst_net_dev, oal_int8 *pc_country)
{
    oal_int32   l_ret;

    if (pst_net_dev == OAL_PTR_NULL || pc_country == OAL_PTR_NULL)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_regdomain_update_country_code_etc::null ptr.net_dev %p, country %p!}",
                        pst_net_dev, pc_country);
        return -OAL_EFAIL;
    }

    /* ���ù����뵽wifi ���� */
    l_ret = wal_regdomain_update_for_dfs_etc(pst_net_dev, pc_country);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_regdomain_update_country_code_etc::update_for_dfs return err code [%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    l_ret = wal_regdomain_update_etc(pst_net_dev, pc_country);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_regdomain_update_country_code_etc::update return err code [%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

#endif

#if 0

OAL_STATIC oal_int32  wal_ioctl_gettid(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_w, oal_int8 *pc_extra)
{

    oal_int32                       l_ret;
    wal_msg_query_stru              st_query_msg;
    wal_msg_stru                   *pst_rsp_msg;
    wal_msg_rsp_stru               *pst_query_rsp_msg;
    mac_cfg_get_tid_stru           *pst_get_tid;
    oal_iw_point_stru              *pst_w = (oal_iw_point_stru *)p_w;

    /***************************************************************************
       ���¼���wal�㴦��
    ***************************************************************************/
    st_query_msg.en_wid = WLAN_CFGID_TID;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                              WAL_MSG_TYPE_QUERY,
                              WAL_MSG_WID_LENGTH,
                              (oal_uint8 *)&st_query_msg,
                              OAL_TRUE,
                              &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
       return l_ret;
    }

    /* ��������Ϣ */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);

    /* ҵ���� */
    pst_get_tid = (mac_cfg_get_tid_stru*)(pst_query_rsp_msg->auc_value);

    oal_memcopy(pc_extra, &pst_get_tid->en_tid, OAL_SIZEOF(pst_get_tid->en_tid));
    pst_w->length = OAL_SIZEOF(pst_get_tid->en_tid);

    if(NULL != pst_rsp_msg)
    {
        oal_free(pst_rsp_msg);
    }
    return OAL_SUCC;
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)


oal_int32  wal_set_random_mac_to_mib_etc(oal_net_device_stru *pst_net_dev)
{
    oal_uint32                    ul_ret;
    frw_event_mem_stru           *pst_event_mem;
    wal_msg_stru                 *pst_cfg_msg;
    wal_msg_write_stru           *pst_write_msg;
    mac_cfg_staion_id_param_stru *pst_param;
    mac_vap_stru                 *pst_mac_vap;
    oal_uint8                    *puc_mac_addr;
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_wireless_dev_stru        *pst_wdev; /* ����P2P ������p2p0 �� p2p-p2p0 MAC ��ַ��wlan0 ��ȡ */
    mac_device_stru              *pst_mac_device;
    wlan_p2p_mode_enum_uint8      en_p2p_mode = WLAN_LEGACY_VAP_MODE;
    oal_uint8                     auc_primary_mac_addr[WLAN_MAC_ADDR_LEN] = {0};    /* MAC��ַ */
#endif
    oal_uint8                     *auc_wlan_addr;
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_uint8                     *auc_p2p0_addr;
#endif
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        /* ��ʱ��ӣ��Ժ�ɾ����add:20151116����Ч��1month */
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib_etc::pst_mac_vap NULL}");
        oal_msleep(500);
        pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
        if(NULL == pst_mac_vap)
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib_etc::pst_mac_vap NULL}");
            return OAL_FAIL;
        }
        /* ��ʱ��ӣ��Ժ�ɾ�� */
    }
    if(OAL_PTR_NULL == pst_mac_vap->pst_mib_info)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_random_mac_to_mib_etc::vap->mib_info is NULL !}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    auc_wlan_addr = mac_mib_get_StationID(pst_mac_vap);
#ifdef _PRE_WLAN_FEATURE_P2P
    auc_p2p0_addr = mac_mib_get_p2p0_dot11StationID(pst_mac_vap);
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
    /* ��ȡwlan0 MAC ��ַ������p2p0/p2p-p2p0 MAC ��ַ */
    pst_mac_device = (mac_device_stru *)mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib_etc::pst_mac_device NULL, device_id:%d}",pst_mac_vap->uc_device_id);
        return OAL_FAIL;
    }
    pst_wdev = pst_net_dev->ieee80211_ptr;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device->st_p2p_info.pst_primary_net_device))
    {
        /* random mac will be used. hi1102-cb (#include <linux/etherdevice.h>)    */
        oal_random_ether_addr(auc_primary_mac_addr);
        auc_primary_mac_addr[0] &= (~0x02);
        auc_primary_mac_addr[1] = 0x11;
        auc_primary_mac_addr[2] = 0x02;
    }
    else
    {
#ifndef _PRE_PC_LINT
        if(OAL_LIKELY(OAL_PTR_NULL != OAL_NETDEVICE_MAC_ADDR(pst_mac_device->st_p2p_info.pst_primary_net_device)))
        {
            oal_memcopy(auc_primary_mac_addr, OAL_NETDEVICE_MAC_ADDR(pst_mac_device->st_p2p_info.pst_primary_net_device), WLAN_MAC_ADDR_LEN);
        }
        else
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib_etc() pst_primary_net_device; addr is null}\r\n");
            return OAL_FAIL;
        }
#endif
    }

    switch (pst_wdev->iftype)
    {
        case NL80211_IFTYPE_P2P_DEVICE:
           en_p2p_mode = WLAN_P2P_DEV_MODE;
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
            /* ����P2P device MAC ��ַ��������mac ��ַbit ����Ϊ1 */
            oal_memcopy(auc_p2p0_addr,
                        OAL_NETDEVICE_MAC_ADDR(pst_net_dev),
                        WLAN_MAC_ADDR_LEN);

#else
            /* ����P2P device MAC ��ַ��������mac ��ַbit ����Ϊ1 */
            oal_memcopy(auc_p2p0_addr,
                        auc_primary_mac_addr,
                        WLAN_MAC_ADDR_LEN);
            auc_p2p0_addr[0] |= 0x02;
#endif
            break;
        case NL80211_IFTYPE_P2P_CLIENT:
            en_p2p_mode = WLAN_P2P_CL_MODE;

            /* ����P2P interface MAC ��ַ */
            oal_memcopy(auc_wlan_addr,
                        auc_primary_mac_addr,
                        WLAN_MAC_ADDR_LEN);
            auc_wlan_addr[0] |= 0x02;
            auc_wlan_addr[4] ^= 0x80;
            break;
        case NL80211_IFTYPE_P2P_GO:
            en_p2p_mode = WLAN_P2P_GO_MODE;
            /* ����P2P interface MAC ��ַ */
            oal_memcopy(auc_wlan_addr,
                        auc_primary_mac_addr,
                        WLAN_MAC_ADDR_LEN);
            auc_wlan_addr[0] |= 0x02;
            auc_wlan_addr[4] ^= 0x80;
            break;
        default:
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
            if(0 == (oal_strcmp("p2p0", pst_net_dev->name)))
            {
                en_p2p_mode = WLAN_P2P_DEV_MODE;
                 /* ����P2P device MAC ��ַ��������mac ��ַbit ����Ϊ1 */
                oal_memcopy(auc_p2p0_addr,
                            OAL_NETDEVICE_MAC_ADDR(pst_net_dev),
                            WLAN_MAC_ADDR_LEN);
                break;
            }

            oal_memcopy(auc_wlan_addr,
                        OAL_NETDEVICE_MAC_ADDR(pst_net_dev),
                        WLAN_MAC_ADDR_LEN);
#else
            /* random mac will be used. hi1102-cb (#include <linux/etherdevice.h>)    */
            oal_random_ether_addr(auc_wlan_addr);
            auc_wlan_addr[0]&=(~0x02);
            auc_wlan_addr[1]=0x11;
            auc_wlan_addr[2]=0x02;
#endif
            break;
    }
#else
    /* random mac will be used. hi1102-cb (#include <linux/etherdevice.h>)    */
    oal_random_ether_addr(auc_wlan_addr);
    auc_wlan_addr[0]&=(~0x02);
    auc_wlan_addr[1]=0x11;
    auc_wlan_addr[2]=0x02;
#endif

    /* send the random mac to dmac */
    /***************************************************************************
        ���¼���wal�㴦��   copy from wal_netdev_set_mac_addr()
        gong TBD : ��Ϊ����ͨ�õ�config�ӿ�
    ***************************************************************************/
    ul_ret = wal_alloc_cfg_event_etc(pst_net_dev, &pst_event_mem, NULL, &pst_cfg_msg, (WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_staion_id_param_stru)));     /* �����¼� */
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib_etc() fail; return %d!}\r\n", ul_ret);
        return -OAL_ENOMEM;
    }

    /* ��д������Ϣ */
    WAL_CFG_MSG_HDR_INIT(&(pst_cfg_msg->st_msg_hdr),
                         WAL_MSG_TYPE_WRITE,
                         WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_staion_id_param_stru),
                         WAL_GET_MSG_SN());

    /* ��дWID��Ϣ */
    pst_write_msg = (wal_msg_write_stru *)pst_cfg_msg->auc_msg_data;
    WAL_WRITE_MSG_HDR_INIT(pst_write_msg, WLAN_CFGID_STATION_ID, OAL_SIZEOF(mac_cfg_staion_id_param_stru));

    pst_param = (mac_cfg_staion_id_param_stru *)pst_write_msg->auc_value;   /* ��дWID��Ӧ�Ĳ��� */
#ifdef _PRE_WLAN_FEATURE_P2P
    /* ���ʹ��P2P����Ҫ��netdevice ��Ӧ��P2P ģʽ�����ò��������õ�hmac ��dmac */
    /* �Ա�ײ�ʶ���䵽p2p0 ��p2p-p2p0 cl */
    pst_param->en_p2p_mode = en_p2p_mode;
    if (en_p2p_mode == WLAN_P2P_DEV_MODE)
    {
        puc_mac_addr = mac_mib_get_p2p0_dot11StationID(pst_mac_vap);
    }
    else
#endif
    {
        puc_mac_addr = mac_mib_get_StationID(pst_mac_vap);
    }
    oal_set_mac_addr(pst_param->auc_station_id, puc_mac_addr);

    OAL_IO_PRINT("wal_set_random_mac_to_mib_etc,mac is:%.2x:%.2x:%.2x\n",
                  puc_mac_addr[0], puc_mac_addr[1], puc_mac_addr[2]);

    frw_event_dispatch_event_etc(pst_event_mem);    /* �ַ��¼� */
    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32  wal_hipriv_setcountry(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
#ifdef _PRE_WLAN_FEATURE_11D
    oal_int32                        l_ret;
    oal_uint32                       ul_ret;
    oal_uint32                       ul_off_set;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int8                        *puc_para;

    /* �豸��up״̬���������ã�������down */
    if (0 != (OAL_IFF_RUNNING & OAL_NETDEVICE_FLAGS(pst_net_dev)))
    {
        OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::country is %d, %d!}\r\n", OAL_NETDEVICE_FLAGS(pst_net_dev));
        return OAL_EBUSY;
    }

    /* ��ȡ�������ַ��� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    puc_para = &ac_arg[0];

    l_ret = wal_regdomain_update_for_dfs_etc(pst_net_dev, puc_para);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::regdomain_update return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
    l_ret = wal_regdomain_update_etc(pst_net_dev, puc_para);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::regdomain_update return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
#else
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_setcountry::_PRE_WLAN_FEATURE_11D is not define!}\r\n");
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    g_st_cust_country_code_ignore_flag.en_country_code_ingore_hipriv_flag = OAL_TRUE;
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_getcountry(oal_net_device_stru *pst_net_dev,oal_int8 *pc_param)
{
#ifdef _PRE_WLAN_FEATURE_11D

    wal_msg_query_stru           st_query_msg;
    oal_int32                    l_ret;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    st_query_msg.en_wid = WLAN_CFGID_COUNTRY;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH ,
                               (oal_uint8 *)&st_query_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
       OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_getcountry::wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
       return (oal_uint32)l_ret;
    }

#else
    OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_hipriv_getcountry::_PRE_WLAN_FEATURE_11D is not define!}\r\n");
#endif

    return OAL_SUCC;
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,34))

OAL_STATIC oal_void *wal_sta_info_seq_start(struct seq_file *f, loff_t *pos)
{
    if (0 == *pos)
    {
        return f->private;
    }
    else
    {
        return NULL;
    }
}


OAL_STATIC oal_int32 wal_sta_info_seq_show(struct seq_file *f, void *v)
{
#define TID_STAT_TO_USER(_stat) ((_stat[0])+(_stat[1])+(_stat[2])+(_stat[3])+(_stat[4])+(_stat[5])+(_stat[6])+(_stat[7]))
#define BW_ENUM_TO_NUMBER(_bw)  ((_bw) == 0 ? 20 : (_bw) == 1 ? 40 : 80)

    mac_vap_stru                    *pst_mac_vap  = (mac_vap_stru *)v;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_dlist_head_stru             *pst_entry;
    oal_dlist_head_stru             *pst_dlist_tmp;
    mac_user_stru                   *pst_user_tmp;
    hmac_user_stru                  *pst_hmac_user_tmp;
    oal_uint8                       *puc_addr;
    oal_uint16                       us_idx = 1;
    oam_stat_info_stru              *pst_oam_stat;
    oam_user_stat_info_stru         *pst_oam_user_stat;
    oal_uint32                       ul_curr_time;
    oal_int8                        *pac_protocol2string[] = {"11a", "11b", "11g", "11g", "11g", "11n", "11ac", "11n", "11ac", "11n","error"};
    wal_msg_write_stru               st_write_msg;
    oal_int32                        l_ret;
    mac_cfg_query_rssi_stru         *pst_query_rssi_param;
    mac_cfg_query_rate_stru         *pst_query_rate_param;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_sta_info_seq_show: hmac vap is null. vap id:%d", pst_mac_vap->uc_vap_id);
        return 0;
    }

    /* step1. ͬ��Ҫ��ѯ��dmac��Ϣ */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
        if (OAL_PTR_NULL == pst_user_tmp)
        {
            continue;
        }

        pst_hmac_user_tmp = mac_res_get_hmac_user_etc(pst_user_tmp->us_assoc_id);
        if (OAL_PTR_NULL == pst_hmac_user_tmp)
        {
            continue;
        }

        if (OAL_PTR_NULL == pst_dlist_tmp)
        {
            /* ��forѭ���̻߳���ͣ���ڼ����ɾ���û��¼��������pst_dlist_tmpΪ�ա�Ϊ��ʱֱ��������ȡdmac��Ϣ */
            break;
        }

        /***********************************************************************/
        /*                  ��ȡdmac user��RSSI��Ϣ                            */
        /***********************************************************************/
        OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_RSSI, OAL_SIZEOF(mac_cfg_query_rssi_stru));
        pst_query_rssi_param = (mac_cfg_query_rssi_stru *)st_write_msg.auc_value;

        pst_query_rssi_param->us_user_id = pst_user_tmp->us_assoc_id; /* ���û���id����ȥ */

        l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_query_rssi_stru),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_sta_info_seq_show: send query rssi cfg event ret:%d", l_ret);
        }

        pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;
        /*lint -e730*/
        l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), 5*OAL_TIME_HZ);
        /*lint +e730*/
        if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_sta_info_seq_show: query rssi timeout. ret:%d", l_ret);
        }

        /***********************************************************************/
        /*                  ��ȡdmac user��������Ϣ                            */
        /***********************************************************************/
        OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_RATE, OAL_SIZEOF(mac_cfg_query_rate_stru));
        pst_query_rate_param = (mac_cfg_query_rate_stru *)st_write_msg.auc_value;

        pst_query_rate_param->us_user_id = pst_user_tmp->us_assoc_id; /* ���û���id����ȥ */

        l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_query_rate_stru),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_sta_info_seq_show: send query rate cfg event ret:%d", l_ret);
        }

        pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;
        /*lint -e730*/
        l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q, (OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), 5*OAL_TIME_HZ);
        /*lint +e730*/
        if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_sta_info_seq_show: query rate timeout. ret:%d", l_ret);
        }

    }

    /* step2. proc�ļ�����û���Ϣ */
    seq_printf(f, "Total user nums: %d\n", pst_mac_vap->us_user_nums);
    seq_printf(f, "-- STA info table --\n");

    pst_oam_stat = OAM_STAT_GET_STAT_ALL();
    ul_curr_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
        if (OAL_PTR_NULL == pst_user_tmp)
        {
            continue;
        }

        pst_hmac_user_tmp = mac_res_get_hmac_user_etc(pst_user_tmp->us_assoc_id);
        if (OAL_PTR_NULL == pst_hmac_user_tmp)
        {
            continue;
        }

        pst_oam_user_stat = &(pst_oam_stat->ast_user_stat_info[pst_user_tmp->us_assoc_id]);
        puc_addr = pst_user_tmp->auc_user_mac_addr;

        seq_printf(f, "%2d: aid: %d\n"
                      "    MAC ADDR: %02X:%02X:%02X:%02X:%02X:%02X\n"
                      "    status: %d\n"
                      "    BW: %d\n"
                      "    NSS: %d\n"
                      "    RSSI: %d\n"
                      "    phy type: %s\n"
                      "    TX rate: %dkbps\n"
                      "    RX rate: %dkbps\n"
                      "    RX rate_min: %dkbps\n"
                      "    RX rate_max: %dkbps\n"
                      "    user online time: %us\n"
                      "    TX packets succ: %u\n"
                      "    TX packets fail: %u\n"
                      "    RX packets succ: %u\n"
                      "    RX packets fail: %u\n"
                      "    TX power: %ddBm\n"
                      "    TX bytes: %u\n"
                      "    RX bytes: %u\n"
                      "    TX retries: %u\n"
#ifdef _PRE_WLAN_DFT_STAT
                      "    Curr_rate PER: %u\n"
                      "    Best_rate PER: %u\n"
                      "    Tx Throughput: %u\n" /*������*/
#endif
                      ,us_idx,
                      pst_user_tmp->us_assoc_id,
                      puc_addr[0],puc_addr[1],puc_addr[2],puc_addr[3],puc_addr[4],puc_addr[5],
                      pst_user_tmp->en_user_asoc_state,     /* status */
                      BW_ENUM_TO_NUMBER(pst_user_tmp->en_avail_bandwidth),
                      (pst_user_tmp->en_avail_num_spatial_stream+1),   /* NSS,��1��Ϊ�˷����û��鿴�����������0��ʾ���� */
                      pst_hmac_user_tmp->c_rssi,
                      pac_protocol2string[pst_user_tmp->en_avail_protocol_mode],
                      pst_hmac_user_tmp->ul_tx_rate,
                      pst_hmac_user_tmp->ul_rx_rate,
                      pst_hmac_user_tmp->ul_rx_rate_min,
                      pst_hmac_user_tmp->ul_rx_rate_max,
                      (oal_uint32)OAL_TIME_GET_RUNTIME(pst_hmac_user_tmp->ul_first_add_time, ul_curr_time)/1000,
                      TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_succ_num)+TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_in_ampdu),
                      TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_fail_num)+TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_fail_in_ampdu),
                      pst_oam_user_stat->ul_rx_mpdu_num,   /* RX packets succ */
                      0,
                      20,                                  /* TX power, �ݲ�ʹ�� ��������tpc��ȡtx_power�ӿ� */
                      TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_bytes)+TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_ampdu_bytes),
                      pst_oam_user_stat->ul_rx_mpdu_bytes, /* RX bytes */
                      pst_oam_user_stat->ul_tx_ppdu_retries/* TX retries */
#ifdef _PRE_WLAN_DFT_STAT
                      ,pst_hmac_user_tmp->uc_cur_per,
                      pst_hmac_user_tmp->uc_bestrate_per,
                      0
#endif
                      );

        us_idx++;

        if (OAL_PTR_NULL == pst_dlist_tmp)
        {
            break;
        }
    }

#undef TID_STAT_TO_USER
#undef BW_ENUM_TO_NUMBER
    return 0;
}


OAL_STATIC oal_void *wal_sta_info_seq_next(struct seq_file *f, void *v, loff_t *pos)
{
    return NULL;
}


OAL_STATIC oal_void wal_sta_info_seq_stop(struct seq_file *f, void *v)
{

}

/*****************************************************************************
    dmac_sta_info_seq_ops: ����seq_file ops
*****************************************************************************/
OAL_STATIC OAL_CONST struct seq_operations wal_sta_info_seq_ops = {
    .start = wal_sta_info_seq_start,
    .next  = wal_sta_info_seq_next,
    .stop  = wal_sta_info_seq_stop,
    .show  = wal_sta_info_seq_show
};


OAL_STATIC oal_int32 wal_sta_info_seq_open(struct inode *inode, struct file *filp)
{
    oal_int32               l_ret;
    struct seq_file        *pst_seq_file;
    struct proc_dir_entry  *pde = PDE(inode);

    l_ret = seq_open(filp, &wal_sta_info_seq_ops);
    if (OAL_SUCC == l_ret)
    {
        pst_seq_file = (struct seq_file *)filp->private_data;

        pst_seq_file->private = pde->data;
    }

    return l_ret;
}

/*****************************************************************************
    gst_sta_info_proc_fops: ����sta info proc fops
*****************************************************************************/
OAL_STATIC OAL_CONST struct file_operations gst_sta_info_proc_fops = {
    .owner      = THIS_MODULE,
    .open       = wal_sta_info_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = seq_release
};


OAL_STATIC int  wal_read_vap_info_proc(char *page, char **start, off_t off,
                   int count, int *eof, void *data)
{
    int                      len;
    mac_vap_stru            *pst_mac_vap = (mac_vap_stru *)data;
    oam_stat_info_stru      *pst_oam_stat;
    oam_vap_stat_info_stru  *pst_oam_vap_stat;

#ifdef _PRE_WLAN_DFT_STAT
    mac_cfg_query_ani_stru          *pst_query_ani_param;
    wal_msg_write_stru               st_write_msg;
    oal_int32                        l_ret;
    hmac_vap_stru                   *pst_hmac_vap;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_read_vap_info_proc: hmac vap is null. vap id:%d", pst_mac_vap->uc_vap_id);
        return 0;
    }
    /***********************************************************************/
    /*                  ��ȡdmac vap��ANI��Ϣ                            */
    /***********************************************************************/
    OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_RSSI, OAL_SIZEOF(mac_cfg_query_ani_stru));
    pst_query_ani_param = (mac_cfg_query_ani_stru *)st_write_msg.auc_value;

    l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_query_ani_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_read_vap_info_proc: send query ani cfg event ret:%d", l_ret);
    }

    pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;
    /*lint -e730*/
    l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), 5*OAL_TIME_HZ);
    /*lint +e730*/
    if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_read_vap_info_proc: query ani timeout. ret:%d", l_ret);
    }
#endif

    pst_oam_stat = OAM_STAT_GET_STAT_ALL();

    pst_oam_vap_stat = &(pst_oam_stat->ast_vap_stat_info[pst_mac_vap->uc_vap_id]);

    len = OAL_SPRINTF(page, PAGE_SIZE, "vap stats:\n"
                        "  TX bytes: %u\n"
                        "  TX packets: %u\n"
                        "  TX packets error: %u\n"
                        "  TX packets discard: %u\n"
                        "  TX unicast packets: %u\n"
                        "  TX multicast packets: %u\n"
                        "  TX broadcast packets: %u\n",
                        pst_oam_vap_stat->ul_tx_bytes_from_lan,
                        pst_oam_vap_stat->ul_tx_pkt_num_from_lan,
                        pst_oam_vap_stat->ul_tx_abnormal_msdu_dropped+pst_oam_vap_stat->ul_tx_security_check_faild+pst_oam_vap_stat->ul_tx_abnormal_mpdu_dropped,
                        pst_oam_vap_stat->ul_tx_uapsd_process_dropped+pst_oam_vap_stat->ul_tx_psm_process_dropped+pst_oam_vap_stat->ul_tx_alg_process_dropped,
                        pst_oam_vap_stat->ul_tx_pkt_num_from_lan - pst_oam_vap_stat->ul_tx_m2u_mcast_cnt,
                        0,
                        pst_oam_vap_stat->ul_tx_m2u_mcast_cnt);

    if ( len >= PAGE_SIZE )
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_read_vap_info_proc:len: %d out of page size: %d", len, PAGE_SIZE);
        return (PAGE_SIZE - 1);
    }
    len += OAL_SPRINTF(page+len, PAGE_SIZE - len, "  RX bytes: %u\n"
                             "  RX packets: %u\n"
                             "  RX packets error: %u\n"
                             "  RX packets discard: %u\n"
                             "  RX unicast packets: %u\n"
                             "  RX multicast packets: %u\n"
                             "  RX broadcast packets: %u\n"
                             "  RX unhnown protocol packets: %u\n"
#ifdef _PRE_WLAN_DFT_STAT
                             "  Br_rate_num: %u\n"
                             "  Nbr_rate_num: %u\n"
                             "  Max_rate: %u\n"
                             "  Min_rate: %u\n"
                             "  Channel num: %d\n"
                             "  ANI:\n"
                             "    dmac_device_distance: %d\n"
                             "    cca_intf_state: %d\n"
                             "    co_intf_state: %d\n"
#endif
                             ,pst_oam_vap_stat->ul_rx_bytes_to_lan,
                             pst_oam_vap_stat->ul_rx_pkt_to_lan,
                             pst_oam_vap_stat->ul_rx_defrag_process_dropped+pst_oam_vap_stat->ul_rx_alg_process_dropped+pst_oam_vap_stat->ul_rx_abnormal_dropped,
                             pst_oam_vap_stat->ul_rx_no_buff_dropped+pst_oam_vap_stat->ul_rx_ta_check_dropped+pst_oam_vap_stat->ul_rx_da_check_dropped+pst_oam_vap_stat->ul_rx_replay_fail_dropped+pst_oam_vap_stat->ul_rx_key_search_fail_dropped,
                             pst_oam_vap_stat->ul_rx_pkt_to_lan - pst_oam_vap_stat->ul_rx_mcast_cnt,
                             0,
                             pst_oam_vap_stat->ul_rx_mcast_cnt,
                             0
#ifdef _PRE_WLAN_DFT_STAT
                             ,pst_mac_vap->st_curr_sup_rates.uc_br_rate_num,
                             pst_mac_vap->st_curr_sup_rates.uc_nbr_rate_num,
                             pst_mac_vap->st_curr_sup_rates.uc_max_rate,
                             pst_mac_vap->st_curr_sup_rates.uc_min_rate,
                             pst_mac_vap->st_channel.uc_chan_number,
                             pst_hmac_vap->uc_device_distance,
                             pst_hmac_vap->uc_intf_state_cca,
                             pst_hmac_vap->uc_intf_state_co
#endif
                             );

    if ( len >= PAGE_SIZE )
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_read_vap_info_proc:len: %d out of page size: %d", len, PAGE_SIZE);
        return (PAGE_SIZE - 1);
    }
    return len;
}


OAL_STATIC int  wal_read_rf_info_proc(char *page, char **start, off_t off,
                   int count, int *eof, void *data)
{
    int                  len;
    mac_vap_stru        *pst_mac_vap = (mac_vap_stru *)data;

    len = OAL_SPRINTF(page, PAGE_SIZE, "rf info:\n  channel_num: %d\n",
                        pst_mac_vap->st_channel.uc_chan_number);
    if (len >= PAGE_SIZE)
    {
        len = PAGE_SIZE - 1;
    }
    return len;
}


OAL_STATIC oal_void wal_add_vap_proc_file(mac_vap_stru *pst_mac_vap, oal_int8 *pc_name)
{
    hmac_vap_stru                       *pst_hmac_vap;
    oal_proc_dir_entry_stru             *pst_proc_dir;
    oal_proc_dir_entry_stru             *pst_proc_vapinfo;
    oal_proc_dir_entry_stru             *pst_proc_stainfo;
    oal_proc_dir_entry_stru             *pst_proc_mibrf;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_add_vap_proc_file: pst_hmac_vap is null ptr!");
        return;
    }

    pst_proc_dir = proc_mkdir(pc_name, NULL);
    if (OAL_PTR_NULL == pst_proc_dir)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "wal_add_vap_proc_file: proc_mkdir return null");
        return;
    }

    pst_proc_vapinfo = oal_create_proc_entry("ap_info", 420, pst_proc_dir);
    if (OAL_PTR_NULL == pst_proc_vapinfo)
    {
        oal_remove_proc_entry(pc_name, NULL);
        return;
    }

    pst_proc_stainfo = oal_create_proc_entry("sta_info", 420, pst_proc_dir);
    if (OAL_PTR_NULL == pst_proc_stainfo)
    {
        oal_remove_proc_entry("ap_info", pst_proc_dir);
        oal_remove_proc_entry(pc_name, NULL);
        return;
    }

    pst_proc_mibrf = oal_create_proc_entry("mib_rf", 420, pst_proc_dir);
    if (OAL_PTR_NULL == pst_proc_mibrf)
    {
        oal_remove_proc_entry("ap_info", pst_proc_dir);
        oal_remove_proc_entry("sta_info", pst_proc_dir);
        oal_remove_proc_entry(pc_name, NULL);
        return;
    }

    /* vap info */
    pst_proc_vapinfo->read_proc  = wal_read_vap_info_proc;
    pst_proc_vapinfo->data       = pst_mac_vap;

    /* sta info�������ļ��Ƚϴ��proc file��ͨ��proc_fops�ķ�ʽ��� */
    pst_proc_stainfo->data       = pst_mac_vap;
    pst_proc_stainfo->proc_fops  = &gst_sta_info_proc_fops;

    /* rf info */
    pst_proc_mibrf->read_proc  = wal_read_rf_info_proc;
    pst_proc_mibrf->data       = pst_mac_vap;

    pst_hmac_vap->pst_proc_dir = pst_proc_dir;
}


OAL_STATIC oal_void wal_del_vap_proc_file(oal_net_device_stru *pst_net_dev)
{
    mac_vap_stru   *pst_mac_vap;
    hmac_vap_stru  *pst_hmac_vap;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_del_vap_proc_file: pst_mac_vap is null ptr! pst_net_dev:%x", (oal_uint32)pst_net_dev);
        return;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_del_vap_proc_file: pst_hmac_vap is null ptr. mac vap id:%d", pst_mac_vap->uc_vap_id);
        return;
    }

    if (pst_hmac_vap->pst_proc_dir)
    {
        oal_remove_proc_entry("mib_rf", pst_hmac_vap->pst_proc_dir);
        oal_remove_proc_entry("sta_info", pst_hmac_vap->pst_proc_dir);
        oal_remove_proc_entry("ap_info", pst_hmac_vap->pst_proc_dir);
        oal_remove_proc_entry(pst_hmac_vap->auc_name, NULL);
        pst_hmac_vap->pst_proc_dir = OAL_PTR_NULL;
    }
}
#elif (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,59))

OAL_STATIC oal_int32 wal_sta_info_seq_show_sec(struct seq_file *f, void *v)
{
#define TID_STAT_TO_USER(_stat) ((_stat[0])+(_stat[1])+(_stat[2])+(_stat[3])+(_stat[4])+(_stat[5])+(_stat[6])+(_stat[7]))
#define BW_ENUM_TO_NUMBER(_bw)  ((_bw) == 0 ? 20 : (_bw) == 1 ? 40 : 80)

    mac_vap_stru                    *pst_mac_vap  = (mac_vap_stru *)(f->private);
    hmac_vap_stru                   *pst_hmac_vap;
    oal_dlist_head_stru             *pst_entry;
    oal_dlist_head_stru             *pst_dlist_tmp;
    mac_user_stru                   *pst_user_tmp;
    hmac_user_stru                  *pst_hmac_user_tmp;
    oal_uint8                       *puc_addr;
    oal_uint16                       us_idx = 1;
    oam_stat_info_stru              *pst_oam_stat;
    oam_user_stat_info_stru         *pst_oam_user_stat;
    oal_uint32                       ul_curr_time;
    oal_int8                        *pac_protocol2string[] = {"11a", "11b", "11g", "11g", "11g", "11n", "11ac", "11n", "11ac", "11n","error"};
    wal_msg_write_stru               st_write_msg;
    oal_int32                        l_ret;
    mac_cfg_query_rssi_stru         *pst_query_rssi_param;
    mac_cfg_query_rate_stru         *pst_query_rate_param;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_sta_info_seq_show: hmac vap is null. vap id:%d", pst_mac_vap->uc_vap_id);
        return 0;
    }
    /* step1. ͬ��Ҫ��ѯ��dmac��Ϣ */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);

        pst_hmac_user_tmp = mac_res_get_hmac_user_etc(pst_user_tmp->us_assoc_id);
        if (OAL_PTR_NULL == pst_hmac_user_tmp)
        {
            continue;
        }

        if (OAL_PTR_NULL == pst_dlist_tmp)
        {
            /* ��forѭ���̻߳���ͣ���ڼ����ɾ���û��¼��������pst_dlist_tmpΪ�ա�Ϊ��ʱֱ��������ȡdmac��Ϣ */
            break;
        }

        /***********************************************************************/
        /*                  ��ȡdmac user��RSSI��Ϣ                            */
        /***********************************************************************/
        OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_RSSI, OAL_SIZEOF(mac_cfg_query_rssi_stru));
        pst_query_rssi_param = (mac_cfg_query_rssi_stru *)st_write_msg.auc_value;

        pst_query_rssi_param->us_user_id = pst_user_tmp->us_assoc_id; /* ���û���id����ȥ */

        l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_query_rssi_stru),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_sta_info_seq_show: send query rssi cfg event ret:%d", l_ret);
        }

        pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;
        /*lint -save -e774 */
        /*lint -e730*/
        l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), 5*OAL_TIME_HZ);
        /*lint +e730*/
        /*lint -restore */
        if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_sta_info_seq_show: query rssi timeout. ret:%d", l_ret);
        }

        /***********************************************************************/
        /*                  ��ȡdmac user��������Ϣ                            */
        /***********************************************************************/
        OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_RATE, OAL_SIZEOF(mac_cfg_query_rate_stru));
        pst_query_rate_param = (mac_cfg_query_rate_stru *)st_write_msg.auc_value;

        pst_query_rate_param->us_user_id = pst_user_tmp->us_assoc_id; /* ���û���id����ȥ */

        l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_query_rate_stru),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_sta_info_seq_show: send query rate cfg event ret:%d", l_ret);
        }

        pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;
        /*lint -save -e774 */
        /*lint -e730*/
        l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q, (OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), 5*OAL_TIME_HZ);
        /*lint +e730*/
        /*lint -restore */
        if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_sta_info_seq_show: query rate timeout. ret:%d", l_ret);
        }

    }

    /* step2. proc�ļ�����û���Ϣ */
    seq_printf(f, "Total user nums: %d\n", pst_mac_vap->us_user_nums);
    seq_printf(f, "-- STA info table --\n");

    pst_oam_stat = OAM_STAT_GET_STAT_ALL();
    ul_curr_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);

        pst_hmac_user_tmp = mac_res_get_hmac_user_etc(pst_user_tmp->us_assoc_id);
        if (OAL_PTR_NULL == pst_hmac_user_tmp)
        {
            continue;
        }

        pst_oam_user_stat = &(pst_oam_stat->ast_user_stat_info[pst_user_tmp->us_assoc_id]);
        puc_addr = pst_user_tmp->auc_user_mac_addr;

        seq_printf(f, "%2d: aid: %d\n"
                      "    MAC ADDR: %02X:%02X:%02X:%02X:%02X:%02X\n"
                      "    status: %d\n"
                      "    BW: %d\n"
                      "    NSS: %d\n"
                      "    RSSI: %d\n"
                      "    phy type: %s\n"
                      "    TX rate: %dkbps\n"
                      "    RX rate: %dkbps\n"
                      "    RX rate_min: %dkbps\n"
                      "    RX rate_max: %dkbps\n"
                      "    user online time: %us\n"
                      "    TX packets succ: %u\n"
                      "    TX packets fail: %u\n"
                      "    RX packets succ: %u\n"
                      "    RX packets fail: %u\n"
                      "    TX power: %ddBm\n"
                      "    TX bytes: %u\n"
                      "    RX bytes: %u\n"
                      "    TX retries: %u\n"
#ifdef _PRE_WLAN_DFT_STAT
                      "    Curr_rate PER: %u\n"
                      "    Best_rate PER: %u\n"
                      "    Tx Throughput: %u\n" /*������*/
#endif
                      ,us_idx,
                      pst_user_tmp->us_assoc_id,
                      puc_addr[0],puc_addr[1],puc_addr[2],puc_addr[3],puc_addr[4],puc_addr[5],
                      pst_user_tmp->en_user_asoc_state,     /* status */
                      BW_ENUM_TO_NUMBER(pst_user_tmp->en_avail_bandwidth),
                      (pst_user_tmp->en_avail_num_spatial_stream+1),   /* NSS */
                      pst_hmac_user_tmp->c_rssi,
                      pac_protocol2string[pst_user_tmp->en_avail_protocol_mode],
                      pst_hmac_user_tmp->ul_tx_rate,
                      pst_hmac_user_tmp->ul_rx_rate,
                      pst_hmac_user_tmp->ul_rx_rate_min,
                      pst_hmac_user_tmp->ul_rx_rate_max,
                      (oal_uint32)OAL_TIME_GET_RUNTIME(pst_hmac_user_tmp->ul_first_add_time, ul_curr_time)/1000,
                      TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_succ_num)+TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_in_ampdu),
                      TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_fail_num)+TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_fail_in_ampdu),
                      pst_oam_user_stat->ul_rx_mpdu_num,   /* RX packets succ */
                      0,
                      20,                                  /* TX power, �ݲ�ʹ�� ��������tpc��ȡtx_power�ӿ� */
                      TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_bytes)+TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_ampdu_bytes),
                      pst_oam_user_stat->ul_rx_mpdu_bytes, /* RX bytes */
                      pst_oam_user_stat->ul_tx_ppdu_retries/* TX retries */
#ifdef _PRE_WLAN_DFT_STAT
                      ,pst_hmac_user_tmp->uc_cur_per,
                      pst_hmac_user_tmp->uc_bestrate_per,
                      0
#endif
                      );

        us_idx++;

        if (OAL_PTR_NULL == pst_dlist_tmp)
        {
            break;
        }
    }

#undef TID_STAT_TO_USER
#undef BW_ENUM_TO_NUMBER
    return 0;
}


OAL_STATIC oal_int32 wal_vap_info_seq_show_sec(struct seq_file *f, void *v)
{
    mac_vap_stru                    *pst_mac_vap  = (mac_vap_stru *)(f->private);
    oam_stat_info_stru              *pst_oam_stat;
    oam_vap_stat_info_stru          *pst_oam_vap_stat;

#ifdef _PRE_WLAN_DFT_STAT
    mac_cfg_query_ani_stru          *pst_query_ani_param;
    wal_msg_write_stru               st_write_msg;
    oal_int32                        l_ret;
    hmac_vap_stru                   *pst_hmac_vap;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_read_vap_info_proc: hmac vap is null. vap id:%d", pst_mac_vap->uc_vap_id);
        return 0;
    }
    /***********************************************************************/
    /*                  ��ȡdmac vap��ANI��Ϣ                            */
    /***********************************************************************/
    OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_RSSI, OAL_SIZEOF(mac_cfg_query_ani_stru));
    pst_query_ani_param = (mac_cfg_query_ani_stru *)st_write_msg.auc_value;

    l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_query_ani_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_read_vap_info_proc: send query ani cfg event ret:%d", l_ret);
    }

    pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;
    /*lint -save -e774 */
    /*lint -e730*/
    l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), 5*OAL_TIME_HZ);
    /*lint +e730*/
    /*lint -restore */
    if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_read_vap_info_proc: query ani timeout. ret:%d", l_ret);
    }
#endif

    pst_oam_stat = OAM_STAT_GET_STAT_ALL();

    pst_oam_vap_stat = &(pst_oam_stat->ast_vap_stat_info[pst_mac_vap->uc_vap_id]);
    seq_printf(f, "vap stats:\n"
                        "  TX bytes: %u\n"
                        "  TX packets: %u\n"
                        "  TX packets error: %u\n"
                        "  TX packets discard: %u\n"
                        "  TX unicast packets: %u\n"
                        "  TX multicast packets: %u\n"
                        "  TX broadcast packets: %u\n",
                        pst_oam_vap_stat->ul_tx_bytes_from_lan,
                        pst_oam_vap_stat->ul_tx_pkt_num_from_lan,
                        pst_oam_vap_stat->ul_tx_abnormal_msdu_dropped+pst_oam_vap_stat->ul_tx_security_check_faild+pst_oam_vap_stat->ul_tx_abnormal_mpdu_dropped,
                        pst_oam_vap_stat->ul_tx_uapsd_process_dropped+pst_oam_vap_stat->ul_tx_psm_process_dropped+pst_oam_vap_stat->ul_tx_alg_process_dropped,
                        pst_oam_vap_stat->ul_tx_pkt_num_from_lan - pst_oam_vap_stat->ul_tx_m2u_mcast_cnt,
                        0,
                        pst_oam_vap_stat->ul_tx_m2u_mcast_cnt);

    seq_printf(f, "  RX bytes: %u\n"
                             "  RX packets: %u\n"
                             "  RX packets error: %u\n"
                             "  RX packets discard: %u\n"
                             "  RX unicast packets: %u\n"
                             "  RX multicast packets: %u\n"
                             "  RX broadcast packets: %u\n"
                             "  RX unhnown protocol packets: %u\n"
#ifdef _PRE_WLAN_DFT_STAT
                             "  Br_rate_num: %u\n"
                             "  Nbr_rate_num: %u\n"
                             "  Max_rate: %u\n"
                             "  Min_rate: %u\n"
                             "  Channel num: %d\n"
                             "  ANI:\n"
                             "    dmac_device_distance: %d\n"
                             "    cca_intf_state: %d\n"
                             "    co_intf_state: %d\n"
#endif
                            ,pst_oam_vap_stat->ul_rx_bytes_to_lan,
                             pst_oam_vap_stat->ul_rx_pkt_to_lan,
                             pst_oam_vap_stat->ul_rx_defrag_process_dropped+pst_oam_vap_stat->ul_rx_alg_process_dropped+pst_oam_vap_stat->ul_rx_abnormal_dropped,
                             pst_oam_vap_stat->ul_rx_no_buff_dropped+pst_oam_vap_stat->ul_rx_ta_check_dropped+pst_oam_vap_stat->ul_rx_da_check_dropped+pst_oam_vap_stat->ul_rx_replay_fail_dropped+pst_oam_vap_stat->ul_rx_key_search_fail_dropped,
                             pst_oam_vap_stat->ul_rx_pkt_to_lan - pst_oam_vap_stat->ul_rx_mcast_cnt,
                             0,
                             pst_oam_vap_stat->ul_rx_mcast_cnt,
                             0
#ifdef _PRE_WLAN_DFT_STAT
                             ,pst_mac_vap->st_curr_sup_rates.uc_br_rate_num,
                             pst_mac_vap->st_curr_sup_rates.uc_nbr_rate_num,
                             pst_mac_vap->st_curr_sup_rates.uc_max_rate,
                             pst_mac_vap->st_curr_sup_rates.uc_min_rate,
                             pst_mac_vap->st_channel.uc_chan_number,
                             pst_hmac_vap->uc_device_distance,
                             pst_hmac_vap->uc_intf_state_cca,
                             pst_hmac_vap->uc_intf_state_co
#endif
                            );

    return 0;

}

OAL_STATIC oal_int32 wal_sta_info_proc_open(struct inode *inode, struct file *filp)
{
    oal_int32               l_ret;
    struct proc_dir_entry  *pde = PDE(inode);
    l_ret = single_open(filp, wal_sta_info_seq_show_sec, pde->data);

    return l_ret;

}

OAL_STATIC oal_int32 wal_vap_info_proc_open(struct inode *inode, struct file *filp)
{
    oal_int32               l_ret;
    struct proc_dir_entry  *pde = PDE(inode);
    l_ret = single_open(filp, wal_vap_info_seq_show_sec, pde->data);

    return l_ret;

}

static ssize_t wal_sta_info_proc_write(struct file* filp, const char __user* buffer, size_t len, loff_t* off)
{
    char mode;
    if (len < 1)
    {
        return -EINVAL;
    }

    if (copy_from_user(&mode, buffer, sizeof(mode)))
    {
        return -EFAULT;
    }

    return len;
}

static ssize_t wal_vap_info_proc_write(struct file* filp, const char __user* buffer, size_t len, loff_t* off)
{
    char mode;
    if (len < 1)
    {
        return -EINVAL;
    }

    if (copy_from_user(&mode, buffer, sizeof(mode)))
    {
        return -EFAULT;
    }

    return len;
}

/*****************************************************************************
    gst_sta_info_proc_fops_sec: ����sta info proc fops
*****************************************************************************/
OAL_STATIC OAL_CONST struct file_operations gst_sta_info_proc_fops_sec = {
    .owner      = THIS_MODULE,
    .open       = wal_sta_info_proc_open,
    .read       = seq_read,
    .write      = wal_sta_info_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};

/*****************************************************************************
    gst_vap_info_proc_fops_sec: ����vap info proc fops
*****************************************************************************/
OAL_STATIC OAL_CONST struct file_operations gst_vap_info_proc_fops_sec = {
    .owner      = THIS_MODULE,
    .open       = wal_vap_info_proc_open,
    .read       = seq_read,
    .write      = wal_vap_info_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};


OAL_STATIC oal_void wal_add_vap_proc_file_sec(mac_vap_stru *pst_mac_vap, oal_int8 *pc_name)
{
    hmac_vap_stru                       *pst_hmac_vap;
    oal_proc_dir_entry_stru             *pst_proc_dir;
    oal_proc_dir_entry_stru             *pst_proc_vapinfo;
    oal_proc_dir_entry_stru             *pst_proc_stainfo;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_add_vap_proc_file: pst_hmac_vap is null ptr!");
        return;
    }

    pst_proc_dir = proc_mkdir(pc_name, NULL);
    if (OAL_PTR_NULL == pst_proc_dir)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "wal_add_vap_proc_file: proc_mkdir return null");
        return;
    }

    pst_proc_vapinfo = proc_create("ap_info", 420, pst_proc_dir, &gst_vap_info_proc_fops_sec);
    if (OAL_PTR_NULL == pst_proc_vapinfo)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "wal_add_vap_proc_file: proc_create return null");
        oal_remove_proc_entry(pc_name, NULL);
        return;
    }

    pst_proc_stainfo = proc_create("sta_info", 420, pst_proc_dir, &gst_sta_info_proc_fops_sec);
    if (OAL_PTR_NULL == pst_proc_stainfo)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "wal_add_vap_proc_file: proc_create return null");
        oal_remove_proc_entry("ap_info", pst_proc_dir);
        oal_remove_proc_entry(pc_name, NULL);
        return;
    }

    pst_proc_vapinfo->data       = pst_mac_vap;
    pst_proc_stainfo->data       = pst_mac_vap;

    pst_hmac_vap->pst_proc_dir = pst_proc_dir;
}


OAL_STATIC oal_void wal_del_vap_proc_file_sec(oal_net_device_stru *pst_net_dev)
{
    mac_vap_stru   *pst_mac_vap;
    hmac_vap_stru  *pst_hmac_vap;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        //OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_del_vap_proc_file: pst_mac_vap is null ptr! pst_net_dev:%x", (oal_uint32)pst_net_dev);
        return;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_del_vap_proc_file: pst_hmac_vap is null ptr. mac vap id:%d", pst_mac_vap->uc_vap_id);
        return;
    }

    if (pst_hmac_vap->pst_proc_dir)
    {
        oal_remove_proc_entry("sta_info", pst_hmac_vap->pst_proc_dir);
        oal_remove_proc_entry("ap_info", pst_hmac_vap->pst_proc_dir);
        oal_remove_proc_entry(pst_hmac_vap->auc_name, NULL);
        pst_hmac_vap->pst_proc_dir = OAL_PTR_NULL;
    }
}
#endif

#if (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT))
extern int fastip_attach_wifi(struct net_device *dev);
extern void fastip_detach_wifi(unsigned int idx);
#endif  /* (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT)) */


OAL_STATIC oal_uint32  wal_hipriv_add_vap(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    oal_net_device_stru        *pst_net_dev;
    wal_msg_write_stru          st_write_msg;
    wal_msg_stru               *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                  ul_err_code;
    oal_uint32                  ul_ret;
    oal_int32                   l_ret;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int8                    ac_mode[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wlan_vap_mode_enum_uint8    en_mode;
    mac_vap_stru               *pst_mac_vap;

    oal_wireless_dev_stru      *pst_wdev;

    mac_vap_stru               *pst_cfg_mac_vap;
    mac_device_stru            *pst_mac_device;
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    psta_mode_enum_uint8        en_psta_mode = PSTA_MODE_NONE;
    oal_uint8                   uc_rep_id = 0;
    oal_int8                    ac_repid_str[16];
#endif  //_PRE_WLAN_FEATURE_PROXYSTA

#ifdef _PRE_WLAN_FEATURE_SINGLE_PROXYSTA
    proxysta_mode_enum_uint8    en_proxysta_mode = PROXYSTA_MODE_NONE;
#endif  // _PRE_WLAN_FEATURE_SINGLE_PROXYSTA

#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8    en_p2p_mode = WLAN_LEGACY_VAP_MODE;
#endif
#if (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT))
    oal_int32                   fastip_idx;
#endif  /* (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT)) */


    /* pc_paramָ���´�����net_device��name, ����ȡ����ŵ�ac_name�� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_get_cmd_one_arg_etc vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    /* ac_name length��Ӧ����OAL_IF_NAME_SIZE */
    if (OAL_IF_NAME_SIZE <=  OAL_STRLEN(ac_name))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_vap:: vap name overlength is %d!}\r\n", OAL_STRLEN(ac_name));
        /* ��������vap name��Ϣ */
        oal_print_hex_dump((oal_uint8 *)ac_name, OAL_IF_NAME_SIZE, 32, "vap name lengh is overlong:");
        return OAL_FAIL;
    }

    pc_param += ul_off_set;

    /* pc_param ָ��'ap|sta', ����ȡ���ŵ�ac_mode�� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_mode, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_get_cmd_one_arg_etc vap name return err_code %d!}\r\n", ul_ret);
        return (oal_uint32)ul_ret;
    }

    /* ����ac_mode�ַ�����Ӧ��ģʽ */
    if (0 == (oal_strcmp("ap", ac_mode)))
    {
        en_mode = WLAN_VAP_MODE_BSS_AP;
    }
    else if (0 == (oal_strcmp("sta", ac_mode)))
    {
        en_mode = WLAN_VAP_MODE_BSS_STA;
    }
#ifdef _PRE_WLAN_FEATURE_SINGLE_PROXYSTA
    else if (0 == oal_strcmp("psta", ac_mode))  // create proxy sta
    {
        en_mode           = WLAN_VAP_MODE_BSS_STA;
        en_proxysta_mode  = PROXYSTA_MODE_SSTA;
    }
    else if (0 == oal_strcmp("pap", ac_mode))  // create  proxysta ap
    {
        en_mode           = WLAN_VAP_MODE_BSS_AP;
        en_proxysta_mode  = PROXYSTA_MODE_RAP;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    else if (0 == oal_strcmp("msta", ac_mode))  // create main sta
    {
        en_mode       = WLAN_VAP_MODE_BSS_STA;
        en_psta_mode  = PSTA_MODE_MSTA;
        if (OAL_SUCC == wal_get_cmd_one_arg_etc(pc_param + ul_off_set, ac_repid_str, &ul_off_set))
        {
            uc_rep_id = (oal_uint8)oal_atoi(ac_repid_str);
        }
    }
    else if (0 == oal_strcmp("vsta", ac_mode))  // create virtaul proxysta sta
    {
        en_mode       = WLAN_VAP_MODE_BSS_STA;
        en_psta_mode  = PSTA_MODE_VSTA;
        if (OAL_SUCC == wal_get_cmd_one_arg_etc(pc_param + ul_off_set, ac_repid_str, &ul_off_set))
        {
            uc_rep_id = (oal_uint8)oal_atoi(ac_repid_str);
        }
    }
    else if (0 == oal_strcmp("pbss", ac_mode))  // create virtaul proxysta sta
    {
        en_mode       = WLAN_VAP_MODE_BSS_AP;
        en_psta_mode  = PSTA_MODE_PBSS;
        if (OAL_SUCC == wal_get_cmd_one_arg_etc(pc_param + ul_off_set, ac_repid_str, &ul_off_set))
        {
            uc_rep_id = (oal_uint8)oal_atoi(ac_repid_str);
        }
    }
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
    /* ����P2P ���VAP */
    else if (0 == (oal_strcmp("p2p_device", ac_mode)))
    {
        en_mode = WLAN_VAP_MODE_BSS_STA;
        en_p2p_mode = WLAN_P2P_DEV_MODE;
    }
    else if (0 == (oal_strcmp("p2p_cl", ac_mode)))
    {
        en_mode = WLAN_VAP_MODE_BSS_STA;
        en_p2p_mode = WLAN_P2P_CL_MODE;
    }
    else if (0 == (oal_strcmp("p2p_go", ac_mode)))
    {
        en_mode = WLAN_VAP_MODE_BSS_AP;
        en_p2p_mode = WLAN_P2P_GO_MODE;
    }
#endif  /* _PRE_WLAN_FEATURE_P2P */
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_add_vap::the mode param is invalid!}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* ���������net device�Ѿ����ڣ�ֱ�ӷ��� */
    /* ����dev_name�ҵ�dev */
    pst_net_dev = oal_dev_get_by_name(ac_name);
    if (OAL_PTR_NULL != pst_net_dev)
    {
        /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
        oal_dev_put(pst_net_dev);

        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_add_vap::the net_device is already exist!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡmac device */
    pst_cfg_mac_vap = OAL_NET_DEV_PRIV(pst_cfg_net_dev);
    pst_mac_device  = mac_res_get_dev_etc(pst_cfg_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_WARNING_LOG0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_add_vap::pst_mac_device is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if defined(_PRE_WLAN_FEATURE_FLOWCTL)
    pst_net_dev = oal_net_alloc_netdev_mqs(OAL_SIZEOF(oal_netdev_priv_stru), ac_name, oal_ether_setup, WAL_NETDEV_SUBQUEUE_MAX_NUM, 1);    /* �˺�����һ����δ���˽�г��ȣ��˴����漰Ϊ0 */
#elif defined(_PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL)
    pst_net_dev = oal_net_alloc_netdev_mqs(OAL_SIZEOF(oal_netdev_priv_stru), ac_name, oal_ether_setup, WLAN_NET_QUEUE_BUTT, 1);    /* �˺�����һ����δ���˽�г��ȣ��˴����漰Ϊ0 */
#else
    pst_net_dev = oal_net_alloc_netdev(OAL_SIZEOF(oal_netdev_priv_stru), ac_name, oal_ether_setup);    /* �˺�����һ����δ���˽�г��ȣ��˴����漰Ϊ0 */
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
    {
        OAM_WARNING_LOG0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_add_vap::pst_net_dev null ptr error!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_wdev = (oal_wireless_dev_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,OAL_SIZEOF(oal_wireless_dev_stru), OAL_FALSE);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_wdev))
    {
        OAM_ERROR_LOG0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_add_vap::alloc mem, pst_wdev is null ptr!}\r\n");
        oal_net_free_netdev(pst_net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_memset(pst_wdev, 0, OAL_SIZEOF(oal_wireless_dev_stru));

    /* ��netdevice���и�ֵ */
#ifdef CONFIG_WIRELESS_EXT
    pst_net_dev->wireless_handlers             = &g_st_iw_handler_def_etc;
#endif /* CONFIG_WIRELESS_EXT */
    /* OAL_NETDEVICE_OPS(pst_net_dev)             = &g_st_wal_net_dev_ops_etc; */
    pst_net_dev->netdev_ops                    = &g_st_wal_net_dev_ops_etc;

    OAL_NETDEVICE_DESTRUCTOR(pst_net_dev)      = oal_net_free_netdev;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,44))
    OAL_NETDEVICE_MASTER(pst_net_dev)          = OAL_PTR_NULL;
#endif

    OAL_NETDEVICE_IFALIAS(pst_net_dev)         = OAL_PTR_NULL;
    OAL_NETDEVICE_WATCHDOG_TIMEO(pst_net_dev)  = 5;
    OAL_NETDEVICE_WDEV(pst_net_dev)            = pst_wdev;
    OAL_NETDEVICE_QDISC(pst_net_dev, OAL_PTR_NULL);
#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
//    OAL_NETDEVICE_TX_QUEUE_LEN(pst_net_dev) = 0;
#endif

    pst_wdev->netdev = pst_net_dev;

    if (WLAN_VAP_MODE_BSS_AP == en_mode)
    {
        pst_wdev->iftype = NL80211_IFTYPE_AP;
    }
    else if (WLAN_VAP_MODE_BSS_STA == en_mode)
    {
        pst_wdev->iftype = NL80211_IFTYPE_STATION;
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    if (WLAN_P2P_DEV_MODE == en_p2p_mode)
    {
        pst_wdev->iftype = NL80211_IFTYPE_P2P_DEVICE;
    }
    else if (WLAN_P2P_CL_MODE == en_p2p_mode)
    {
        pst_wdev->iftype = NL80211_IFTYPE_P2P_CLIENT;
    }
    else if (WLAN_P2P_GO_MODE == en_p2p_mode)
    {
        pst_wdev->iftype = NL80211_IFTYPE_P2P_GO;
    }
#endif  /* _PRE_WLAN_FEATURE_P2P */

    pst_wdev->wiphy = pst_mac_device->pst_wiphy;

    OAL_NETDEVICE_FLAGS(pst_net_dev) &= ~OAL_IFF_RUNNING;   /* ��net device��flag��Ϊdown */

    /* st_write_msg�������������ֹ���ֳ�Ա��Ϊ��غ�û�п���û�и�ֵ�����ַ�0���쳣ֵ������ṹ����P2P modeû����P2P�����������������Ϊ������û����ȷ��ֵ */
    oal_memset(&st_write_msg, 0, OAL_SIZEOF(wal_msg_write_stru));

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ADD_VAP, OAL_SIZEOF(mac_cfg_add_vap_param_stru));
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->en_vap_mode  = en_mode;
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->uc_cfg_vap_indx = pst_cfg_mac_vap->uc_vap_id;
#ifdef  _PRE_WLAN_FEATURE_PROXYSTA
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->en_psta_mode = en_psta_mode;
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->uc_rep_id    = uc_rep_id >= WLAN_PROXY_STA_MAX_REP ? 0 : uc_rep_id;
#endif
#ifdef _PRE_WLAN_FEATURE_SINGLE_PROXYSTA
     ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->en_proxysta_mode = en_proxysta_mode;
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
   ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode  = en_p2p_mode;
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->bit_11ac2g_enable = (oal_uint8)!!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_11AC2G_ENABLE);
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->bit_disable_capab_2ght40 = g_st_wlan_customize_etc.uc_disable_capab_2ght40;
#endif
    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_add_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAL_MEM_FREE(pst_wdev, OAL_FALSE);
        oal_net_free_netdev(pst_net_dev);
        OAM_WARNING_LOG1(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_add_vap::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_add_vap::hmac add vap fail,err code[%u]!}\r\n", ul_err_code);
        /* �쳣�����ͷ��ڴ� */
        OAL_MEM_FREE(pst_wdev, OAL_FALSE);
        oal_net_free_netdev(pst_net_dev);
        return ul_err_code;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

    if ((WLAN_LEGACY_VAP_MODE == en_p2p_mode) && (OAL_PTR_NULL == pst_mac_device->st_p2p_info.pst_primary_net_device))
    {
        /* �������wlan0�� �򱣴�wlan0 Ϊ��net_device,p2p0 ��p2p-p2p0 MAC ��ַ����netdevice ��ȡ */
        pst_mac_device->st_p2p_info.pst_primary_net_device = pst_net_dev;
    }

    if(OAL_SUCC != wal_set_random_mac_to_mib_etc(pst_net_dev))
    {
        /* �쳣�����ͷ��ڴ� */
        /* �쳣�����ͷ��ڴ� */
        OAL_MEM_FREE(pst_wdev, OAL_FALSE);
        oal_net_free_netdev(pst_net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    } /* set random mac to mib ; for hi1102-cb */
#endif

    /* ����netdevice��MAC��ַ��MAC��ַ��HMAC�㱻��ʼ����MIB�� */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
#ifdef _PRE_WLAN_FEATURE_P2P
    if (WLAN_P2P_CL_MODE != en_p2p_mode)
    {
        pst_mac_vap->en_vap_state = MAC_VAP_STATE_INIT;
    }
#else
    pst_mac_vap->en_vap_state = MAC_VAP_STATE_INIT;
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
    if (en_p2p_mode == WLAN_P2P_DEV_MODE)
    {
        oal_set_mac_addr((oal_uint8 *)OAL_NETDEVICE_MAC_ADDR(pst_net_dev), mac_mib_get_p2p0_dot11StationID(pst_mac_vap));

        pst_mac_device->st_p2p_info.uc_p2p0_vap_idx = pst_mac_vap->uc_vap_id;
    }
    else
#endif
    {
        oal_set_mac_addr((oal_uint8 *)OAL_NETDEVICE_MAC_ADDR(pst_net_dev), mac_mib_get_StationID(pst_mac_vap));
    }


    /* ע��net_device */
    ul_ret = (oal_uint32)oal_net_register_netdev(pst_net_dev);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_add_vap::oal_net_register_netdev return error code %d!}\r\n", ul_ret);

        /* �쳣�����ͷ��ڴ� */
        /* ��ɾ��vap�¼��ͷŸ������vap  */
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DEL_VAP, OAL_SIZEOF(mac_cfg_del_vap_param_stru));

        l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_del_vap_param_stru),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_TRUE,
                                   &pst_rsp_msg);

        if (OAL_SUCC != wal_check_and_release_msg_resp_etc(pst_rsp_msg))
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_check_and_release_msg_resp_etc fail.}");
        }
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_send_cfg_event_etc fail,err code %d!}\r\n",l_ret);
        }


        OAL_MEM_FREE(pst_wdev, OAL_FALSE);
        oal_net_free_netdev(pst_net_dev);
        return ul_ret;
    }

    /* E5 SPE module init */
#if (defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT))
    if(spe_hook.is_enable && spe_hook.is_enable())
    {
        if(wal_netdev_spe_init(pst_net_dev))
        {
            OAL_IO_PRINT("wal_netdev_open_etc::spe init failed!!\n");
        }
    }
#endif

#if (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT))
        fastip_idx = fastip_attach_wifi(pst_net_dev);
        if (-1 != fastip_idx)
        {
            pst_mac_vap->ul_fastip_idx = (oal_uint32)fastip_idx;
        }
#endif  /* (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT)) */


    /* ����VAP��Ӧ��proc�ļ� */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,34))
    wal_add_vap_proc_file(pst_mac_vap, ac_name);
#elif (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,59))
    wal_add_vap_proc_file_sec(pst_mac_vap, ac_name);
#endif

    return OAL_SUCC;
}



oal_uint32  wal_hipriv_del_vap_etc(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru           st_write_msg;
    wal_msg_stru               *pst_rsp_msg = {0};
    oal_int32                    l_ret;
    oal_wireless_dev_stru       *pst_wdev;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8     en_p2p_mode = WLAN_LEGACY_VAP_MODE;
#endif
    mac_vap_stru                *pst_mac_vap;
    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pc_param)))
    {
        // ��������ӿڵ�ģ����ܲ�ֹһ��,��Ҫ�ϲ㱣֤�ɿ�ɾ��
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_hipriv_del_vap_etc::pst_net_dev or pc_param null ptr error %d, %d!}\r\n",
                       pst_net_dev, pc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* �豸��up״̬������ɾ����������down */
    if (OAL_UNLIKELY(0 != (OAL_IFF_RUNNING & OAL_NETDEVICE_FLAGS(pst_net_dev))))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_del_vap_etc::device is busy, please down it first %d!}\r\n", OAL_NETDEVICE_FLAGS(pst_net_dev));
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_UNLIKELY(NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_hipriv_del_vap_etc::can't get mac vap from netdevice priv data!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if(WLAN_VAP_MODE_CONFIG == pst_mac_vap->en_vap_mode)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_del_vap_etc::invalid parameters, mac vap mode: %d}", pst_mac_vap->en_vap_mode);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* �豸��up״̬������ɾ����������down */
#ifdef _PRE_WLAN_FEATURE_P2P
    if ((pst_mac_vap->en_vap_state != MAC_VAP_STATE_INIT)
         &&(pst_mac_vap->en_p2p_mode != WLAN_P2P_CL_MODE))
#else
    if (pst_mac_vap->en_vap_state != MAC_VAP_STATE_INIT)
#endif
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_del_vap_etc::device is busy, please down it first %d!}\r\n", pst_mac_vap->en_vap_state);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* E5 SPE module relation */
#if (defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT))
    if(spe_hook.is_enable && spe_hook.is_enable())
    {
         wal_netdev_spe_exit(pst_net_dev);
    }
#endif
#if (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT))
    fastip_detach_wifi(pst_mac_vap->ul_fastip_idx);
#endif  /* (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT)) */


    /* ɾ��vap��Ӧ��proc�ļ� */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,34))
    wal_del_vap_proc_file(pst_net_dev);
#elif (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,59))
    wal_del_vap_proc_file_sec(pst_net_dev);
#endif

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    //ɾ��vap ʱ��Ҫ��������ֵ��
    /* st_write_msg�������������ֹ���ֳ�Ա��Ϊ��غ�û�п���û�и�ֵ�����ַ�0���쳣ֵ������ṹ����P2P modeû����P2P�����������������Ϊ������û����ȷ��ֵ */
    oal_memset(&st_write_msg, 0, OAL_SIZEOF(wal_msg_write_stru));

    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
    pst_wdev = pst_net_dev->ieee80211_ptr;
#ifdef _PRE_WLAN_FEATURE_P2P
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_del_vap_etc::wal_wireless_iftype_to_mac_p2p_mode_etc return BUFF}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;

#endif


    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DEL_VAP, OAL_SIZEOF(mac_cfg_del_vap_param_stru));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_del_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_SUCC != wal_check_and_release_msg_resp_etc(pst_rsp_msg))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_del_vap_etc::wal_check_and_release_msg_resp_etc fail}");
    }

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_del_vap_etc::return err code %d}\r\n", l_ret);
        /* ȥע�� */
        oal_net_unregister_netdev(pst_net_dev);
        OAL_MEM_FREE(pst_wdev, OAL_TRUE);
        return (oal_uint32)l_ret;
    }

    /* ȥע�� */
    oal_net_unregister_netdev(pst_net_dev);
    OAL_MEM_FREE(pst_wdev, OAL_TRUE);

    return OAL_SUCC;
}


oal_uint32  wal_hipriv_vap_info_etc(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru           st_write_msg;
    oal_int32                    l_ret;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_VAP_INFO, OAL_SIZEOF(oal_int32));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    wal_hipriv_wait_rsp(pst_net_dev, pc_param);
#endif

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_vap_info_etc::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;

}
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET

oal_uint32  wal_hipriv_pk_mode_debug(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    /* pkmode���ܵ����޵����ӿ� hipriv "wlan0 pk_mode_debug 0/1(high/low) 0/1/2/3/4(BW) 0/1/2/3(protocol) ��������ֵ"  */
    /*
        BW:20M     40M    80M   160M   80+80M

        protocol:lagency: HT: VHT: HE:
    */
    /*
    PKģʽ���޻���:
    {(��λMbps)  20M     40M    80M   160M   80+80M
    lagency:    {valid, valid, valid, valid, valid},   (����Э��ģʽû��pk mode )
    HT:         {72, 150, valid, valid, valid},
    VHT:        {86, 200, 433, 866, 866},
    HE:         {valid, valid, valid, valid, valid},   (�ݲ�֧��11ax��pk mode)
    };

    PKģʽ��������:
    �ߵ�λ����: g_st_pk_mode_high_th_table = PKģʽ���޻��� * 70% *1024 *1024 /8  (��λ�ֽ�)
    �͵�λ����: g_st_pk_mode_low_th_table  = PKģʽ���޻��� * 20% *1024 *1024 /8  (��λ�ֽ�)

    */
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;
    oal_uint32                      ul_ret;
    oal_uint32                      ul_off_set = 0;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    oal_uint32                      auc_pk_mode_param[4] = {0};

    #if 0
    if (OAL_SIZEOF(st_write_msg.auc_value) < 4)
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_hipriv_pk_mode_debug::wal_msg_write_stru too small[%d]!}", WAL_MSG_WRITE_MAX_LEN);
        return OAL_FAIL;
    }
    #endif

    /* st_write_msg��������� */
    oal_memset(&st_write_msg, 0, OAL_SIZEOF(wal_msg_write_stru));

   /* 0.��ȡ��0������: ������/������ */
    ul_ret = wal_get_cmd_one_arg_etc((oal_int8*)pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_hipriv_pk_mode_debug::wal_get_cmd_one_arg1 return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    auc_pk_mode_param[0] = (oal_uint32)oal_atoi(ac_name);
    if (auc_pk_mode_param[0] > 3)
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_hipriv_pk_mode_debug::high/low param ERROR [%d]!}", auc_pk_mode_param[0]);
        return OAL_FAIL;
    }


    pc_param = pc_param + ul_off_set;

    /* 1.��ȡ��һ������: BW */
    ul_ret = wal_get_cmd_one_arg_etc((oal_int8*)pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_hipriv_pk_mode_debug::wal_get_cmd_one_arg1 return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    auc_pk_mode_param[1] = (oal_uint32)oal_atoi(ac_name);
    if (auc_pk_mode_param[1] >= WLAN_BW_CAP_BUTT)
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_hipriv_pk_mode_debug::BW param ERROR [%d]!}", auc_pk_mode_param[1]);
        return OAL_FAIL;
    }


    pc_param = pc_param + ul_off_set;

    /* 2.��ȡ�ڶ�������: protocol */
    ul_ret = wal_get_cmd_one_arg_etc((oal_int8*)pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_hipriv_pk_mode_debug::wal_get_cmd_one_arg1 return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    auc_pk_mode_param[2] = (oal_uint32)oal_atoi(ac_name);
    if (auc_pk_mode_param[2] >= WLAN_PROTOCOL_CAP_BUTT)
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_hipriv_pk_mode_debug::protocol param ERROR [%d]!}", auc_pk_mode_param[2]);
        return OAL_FAIL;
    }

    pc_param = pc_param + ul_off_set;


    /* 3.��ȡ����������: ����ֵ */
    ul_ret = wal_get_cmd_one_arg_etc((oal_int8*)pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_hipriv_pk_mode_debug::wal_get_cmd_one_arg1 return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    auc_pk_mode_param[3] = (oal_uint32)oal_atoi(ac_name);

    pc_param = pc_param + ul_off_set;

    /*��������������� */
    st_write_msg.auc_value[0] = auc_pk_mode_param[0];
    st_write_msg.auc_value[1] = auc_pk_mode_param[1];
    st_write_msg.auc_value[2] = auc_pk_mode_param[2];
    st_write_msg.auc_value[3] = auc_pk_mode_param[3];

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SYNC_PK_MODE, OAL_SIZEOF(auc_pk_mode_param));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(auc_pk_mode_param),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pk_mode_debug::return err code %d!}", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;

}
#endif
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
#define MAX_HIPRIV_IP_FILTER_BTABLE_SIZE 129

oal_uint32  wal_hipriv_set_ip_filter_etc(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_int32                l_items_cnt;
    oal_int32                l_items_idx;
    oal_int32                l_enable;
    oal_uint32               ul_ret;
    oal_uint32               ul_off_set;

    oal_int8                 ac_cmd[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    oal_int8                 ac_cmd_param[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    wal_hw_wifi_filter_item  ast_items[MAX_HIPRIV_IP_FILTER_BTABLE_SIZE];

    l_enable     = 0;
    l_items_cnt = 0;
    OAL_MEMZERO((oal_uint8 *)ast_items, OAL_SIZEOF(wal_hw_wifi_filter_item)*MAX_HIPRIV_IP_FILTER_BTABLE_SIZE);


    /* ��ȡ�������� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_cmd, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter_etc::wal_get_cmd_one_arg_etc vap name return err_code %d!}", ul_ret);
        return ul_ret;
    }

    if(0 == oal_strncmp(ac_cmd, CMD_CLEAR_RX_FILTERS, OAL_STRLEN(CMD_CLEAR_RX_FILTERS)))
    {
        /* ����� */
        ul_ret = (oal_uint32)wal_clear_ip_filter_etc();
        return ul_ret;
    }

    pc_param += ul_off_set;
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_cmd_param, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter_etc::get cmd_param return err_code %d!}", ul_ret);
        return ul_ret;
    }

    if(0 == oal_strncmp(ac_cmd, CMD_SET_RX_FILTER_ENABLE, OAL_STRLEN(CMD_SET_RX_FILTER_ENABLE)))
    {
        /* ʹ��/�رչ��� */
        l_enable = oal_atoi(ac_cmd_param);
        ul_ret = (oal_uint32)wal_set_ip_filter_enable_etc(l_enable);
        return ul_ret;

    }
    else if(0 == oal_strncmp(ac_cmd, CMD_ADD_RX_FILTER_ITEMS, OAL_STRLEN(CMD_ADD_RX_FILTER_ITEMS)))
    {
        /* ���º����� */
        /* ��ȡ������Ŀ�� */
        l_items_cnt = oal_atoi(ac_cmd_param);
        l_items_cnt = OAL_MIN(MAX_HIPRIV_IP_FILTER_BTABLE_SIZE, l_items_cnt);

        /* ��ȡ������Ŀ */
        for (l_items_idx = 0; l_items_idx < l_items_cnt; l_items_idx++)
        {

            /* ��ȡprotocol X*/
            pc_param += ul_off_set;
            ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_cmd_param, &ul_off_set);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter_etc::get item_params return err_code %d!}", ul_ret);
                return ul_ret;
            }
            ast_items[l_items_idx].protocol = (oal_uint8)oal_atoi(ac_cmd_param);


            /* ��ȡportX*/
            pc_param += ul_off_set;
            ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_cmd_param, &ul_off_set);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter_etc::get item_params return err_code %d!}", ul_ret);
                return ul_ret;
            }
            ast_items[l_items_idx].port = (oal_uint16)oal_atoi(ac_cmd_param);
        }


        ul_ret = (oal_uint32)wal_add_ip_filter_items_etc(ast_items, l_items_cnt);
        return ul_ret;
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter_etc::cmd_one_arg no support!}");
        return OAL_FAIL;
    }

}

#endif //_PRE_WLAN_FEATURE_IP_FILTER
#if 0

OAL_STATIC oal_uint32  wal_hipriv_tdls_prohibited(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_tmp;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32                   l_ret;
    oal_uint32                  ul_ret;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pc_param)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_hipriv_tdls_prohibited::pst_net_dev or pc_param null ptr error %d, %d!}\r\n", pst_net_dev, pc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* TDLS���ÿ��ص�����: hipriv "vap0 tdls_prohi 0 | 1"
        �˴���������"1"��"0"����ac_name
    */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_tdls_prohibited::wal_get_cmd_one_arg_etc return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    /* ��Խ������Ĳ�ͬ�������TDLS���ÿ��� */
    if (0 == (oal_strcmp("0", ac_name)))
    {
        l_tmp = 0;
    }
    else if (0 == (oal_strcmp("1", ac_name)))
    {
        l_tmp = 1;
    }
    else
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_tdls_prohibited::the tdls_prohibited command is error %d!}\r\n", ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_TDLS_PROHI, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_tmp;  /* ��������������� */

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_tdls_prohibited::err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_tdls_channel_switch_prohibited(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_tmp;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32                   l_ret;
    oal_uint32                  ul_ret;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pc_param)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_hipriv_tdls_channel_switch_prohibited::pst_net_dev or pc_param null ptr error %d, %d!}\r\n", pst_net_dev, pc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* TDLS���ÿ��ص�����: hipriv "vap0 tdls_chaswi_prohi 0 | 1"
        �˴���������"1"��"0"����ac_name
    */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_tdls_channel_switch_prohibited::wal_get_cmd_one_arg_etc return err_code, %d!}\r\n", ul_ret);
        return ul_ret;
    }

    /* ��Խ������Ĳ�ͬ�������TDLS�ŵ��л����ÿ��� */
    if (0 == (oal_strcmp("0", ac_name)))
    {
        l_tmp = 0;
    }
    else if (0 == (oal_strcmp("1", ac_name)))
    {
        l_tmp = 1;
    }
    else
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_tdls_channel_switch_prohibited::the channel_switch_prohibited switch command is error %d!}\r\n", ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_TDLS_CHASWI_PROHI, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_tmp;  /* ��������������� */

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_tdls_channel_switch_prohibited::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32  wal_hipriv_set_2040_coext_support(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    oal_uint8                       uc_csp;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_2040_coext_support::wal_get_cmd_one_arg_etc return err_code %d!}\r\n", ul_ret);
         return ul_ret;
    }

    if (0 == (oal_strcmp("0", ac_name)))
    {
        uc_csp = 0;
    }
    else if (0 == (oal_strcmp("1", ac_name)))
    {
        uc_csp = 1;
    }
    else
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_2040_coext_support::the 2040_coexistence command is erro %d!}\r\n", ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_2040_COEXISTENCE, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = uc_csp;  /* ��������������� */

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_2040_coext_support::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}



OAL_STATIC  oal_uint32  wal_hipriv_rx_fcs_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32                   l_ret;
    oal_uint32                  ul_ret;
    mac_cfg_rx_fcs_info_stru   *pst_rx_fcs_info;
    mac_cfg_rx_fcs_info_stru    st_rx_fcs_info;  /* ��ʱ�����ȡ��use����Ϣ */

    /* ��ӡ����֡��FCS��ȷ�������Ϣ:sh hipriv.sh "vap0 rx_fcs_info 0/1 1-4" 0/1  0���������1������� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info::wal_get_cmd_one_arg_etc return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    st_rx_fcs_info.ul_data_op = (oal_uint32)oal_atoi(ac_name);

    if (st_rx_fcs_info.ul_data_op > 1)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info::the ul_data_op command is error %d!}\r\n", ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* ƫ�ƣ�ȡ��һ������ */
    pc_param = pc_param + ul_off_set;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info::wal_get_cmd_one_arg_etc return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    st_rx_fcs_info.ul_print_info = (oal_uint32)oal_atoi(ac_name);

    if (st_rx_fcs_info.ul_print_info > 4)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info::the ul_print_info command is error %d!}\r\n", ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_RX_FCS_INFO, OAL_SIZEOF(mac_cfg_rx_fcs_info_stru));

    /* ��������������� */
    pst_rx_fcs_info = (mac_cfg_rx_fcs_info_stru *)(st_write_msg.auc_value);
    pst_rx_fcs_info->ul_data_op    = st_rx_fcs_info.ul_data_op;
    pst_rx_fcs_info->ul_print_info = st_rx_fcs_info.ul_print_info;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_rx_fcs_info_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_vap_log_level(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    mac_vap_stru               *pst_mac_vap;
    oam_log_level_enum_uint8    en_level_val;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_param[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                  ul_ret;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    wal_msg_write_stru          st_write_msg;
#endif

    /* OAM logģ��Ŀ��ص�����: hipriv "Hisilicon0[vapx] log_level {1/2}"
       1-2(error��warning)������־��vap����Ϊά�ȣ�
    */

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_vap_log_level::null pointer.}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡ��־���� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_param, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        return ul_ret;
    }

    en_level_val = (oam_log_level_enum_uint8)oal_atoi(ac_param);
    if ((en_level_val<OAM_LOG_LEVEL_ERROR) || (en_level_val>OAM_LOG_LEVEL_INFO))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_feature_log_level::invalid switch value[%d].}", en_level_val);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /*hipriv "Hisilicon0 log_level 1|2|3" ��������vip id��log*/
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if(0 == oal_strcmp("Hisilicon0", pst_net_dev->name))
    {
        hmac_config_set_all_log_level_etc(pst_mac_vap, OAL_SIZEOF(oam_log_level_enum_uint8), (oal_uint8*)&en_level_val);
        return OAL_SUCC;
    }
#endif

#if 0  /* ��Ʒ����ǰ֧��info����ȫ�ֿ� */
    if ((0 != oal_strcmp("1", ac_param)) && (0 != oal_strcmp("2", ac_param)))
    {
        /* INFO���� */
        if (0 == oal_strcmp("3", ac_param))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_vap_log_level::not support info level config. use[feature_log_switch]command}\r\n");
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
        }
        else
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_feature_log_level::invalid switch value}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }
#endif
    ul_ret = oam_log_set_vap_level_etc(pst_mac_vap->uc_vap_id, en_level_val);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
// Ŀǰ֧��02 device ����log ���� ���������ĺϲ�����
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_LOG_LEVEL, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = en_level_val;
    ul_ret |= (oal_uint32)wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_vap_log_level::return err code[%d]!}\r\n", ul_ret);
        return ul_ret;
    }

#endif
    return ul_ret;
}

#ifdef _PRE_WLAN_FEATURE_GREEN_AP

OAL_STATIC oal_uint32  wal_hipriv_green_ap_en(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_tmp;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32                   l_ret;
    oal_uint32                  ul_ret;

    /* OAM eventģ��Ŀ��ص�����: hipriv "wlan0 green_ap_en 0 | 1"
        �˴���������"1"��"0"����ac_name
    */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_green_ap_en::wal_get_cmd_one_arg_etc return err_code[%d]}\r\n", ul_ret);
        return ul_ret;
    }

    /* ��Խ������Ĳ�ͬ�����eventģ����в�ͬ������ */

    l_tmp = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_GREEN_AP_EN, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_tmp;  /* ��������������� */

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_green_ap_en::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_LTECOEX

OAL_STATIC oal_uint32 wal_ioctl_ltecoex_mode_set(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_LTECOEX_MODE_SET, OAL_SIZEOF(oal_uint32));

    /* ��������������� */
    *((oal_uint8 *)(st_write_msg.auc_value)) = *pc_param;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_ltecoex_mode_set::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

OAL_STATIC oal_uint32  wal_hipriv_aifsn_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    mac_edca_cfg_stru               st_edca_cfg;
    oal_int32                       l_ret;
    oal_uint32                      ul_ret;

    OAL_MEMZERO(&st_edca_cfg, OAL_SIZEOF(st_edca_cfg));

    /* ��ȡ���ÿ��� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_aifsn_cfg::get wfa switch fail, return err_code[%d]!}", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_edca_cfg.en_switch = (oal_bool_enum_uint8)oal_atoi(ac_name);

    /* ��ȡac */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_aifsn_cfg::get wfa ac fail, return err_code[%d]!}", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_edca_cfg.en_ac = (wlan_wme_ac_type_enum_uint8)oal_atoi(ac_name);

    if (OAL_TRUE == st_edca_cfg.en_switch)
    {
        /* ��ȡ����ֵ */
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_aifsn_cfg::get wfa val fail, return err_code[%d]!}", ul_ret);
            return ul_ret;
        }
        pc_param += ul_off_set;
        st_edca_cfg.us_val = (oal_uint16)oal_atoi(ac_name);
    }
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_WFA_CFG_AIFSN, OAL_SIZEOF(st_edca_cfg));

    /* ��������������� */
    oal_memcopy(st_write_msg.auc_value,
                (const oal_void *)&st_edca_cfg,
                OAL_SIZEOF(st_edca_cfg));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_edca_cfg),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_aifsn_cfg::return err code[%d]!}", ul_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_cw_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    mac_edca_cfg_stru               st_edca_cfg;
    oal_int32                       l_ret;
    oal_uint32                      ul_ret;

    OAL_MEMZERO(&st_edca_cfg, OAL_SIZEOF(st_edca_cfg));

    /* ��ȡ���ÿ��� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_cw_cfg::get wfa switch fail, return err_code[%d]!}", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_edca_cfg.en_switch = (oal_bool_enum_uint8)oal_atoi(ac_name);

    /* ��ȡac */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_cw_cfg::get wfa ac fail, return err_code[%d]!}", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_edca_cfg.en_ac = (wlan_wme_ac_type_enum_uint8)oal_atoi(ac_name);

    if (OAL_TRUE == st_edca_cfg.en_switch)
    {
        /* ��ȡ����ֵ */
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_cw_cfg::get wfa val fail, return err_code[%d]!}", ul_ret);
            return ul_ret;
        }
        pc_param += ul_off_set;
        st_edca_cfg.us_val = (oal_uint16)oal_strtol(ac_name, OAL_PTR_NULL, 0);
    }
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_WFA_CFG_CW, OAL_SIZEOF(st_edca_cfg));

    /* ��������������� */
    oal_memcopy(st_write_msg.auc_value,
                (const oal_void *)&st_edca_cfg,
                OAL_SIZEOF(st_edca_cfg));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_edca_cfg),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_aifsn_cfg::return err code[%d]!}", ul_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif

OAL_STATIC oal_uint32  wal_hipriv_set_random_mac_addr_scan(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    oal_uint8                       uc_rand_mac_addr_scan_switch;

    /* sh hipriv.sh "Hisilicon0 random_mac_addr_scan 0|1(����)" */

    /* ��ȡ֡���� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_random_mac_addr_scan::get switch return err_code[%d]!}", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    uc_rand_mac_addr_scan_switch = (oal_uint8)oal_atoi(ac_name);

    /* ���ص�ȡֵ��ΧΪ0|1,�������Ϸ����ж� */
    if (uc_rand_mac_addr_scan_switch > 1)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_random_mac_addr_scan::param is error, switch_value[%d]!}",
                         uc_rand_mac_addr_scan_switch);
        return OAL_FAIL;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_RANDOM_MAC_ADDR_SCAN, OAL_SIZEOF(oal_uint32));
    *((oal_int32 *)(st_write_msg.auc_value)) = (oal_uint32)uc_rand_mac_addr_scan_switch;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_random_mac_addr_scan::return err code[%d]!}", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#if 0

OAL_STATIC oal_uint32  wal_hipriv_ota_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru                  st_write_msg;
    oal_int32                           l_ota_type;
    oal_int32                           l_param;
    oal_uint32                          ul_off_set = 0;
    oal_int8                            ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    oal_int32                           l_ret;
    oal_uint32                          ul_ret;
    wal_specific_event_type_param_stru *pst_specific_event_param;

    /* OAM otaģ��Ŀ��ص�����: hipriv "Hisilicon0 ota_switch event_type 0 | 1",
       �˴������Ƚ�����ota����(oam_ota_type_enum_uint8),Ȼ��������ز���"1"��"0"����ac_name
    */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_ota_switch::wal_get_cmd_one_arg_etc fails!}\r\n");
        return ul_ret;
    }
    l_ota_type = oal_atoi((const oal_int8 *)ac_name);

    /* �������� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param + ul_off_set, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_ota_switch::wal_get_cmd_one_arg_etc fails!}\r\n");
        return ul_ret;
    }
    l_param = oal_atoi((const oal_int8 *)ac_name);



    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_OTA_SWITCH, OAL_SIZEOF(wal_specific_event_type_param_stru));
    pst_specific_event_param = (wal_specific_event_type_param_stru *)st_write_msg.auc_value;
    pst_specific_event_param->l_event_type = l_ota_type;
    pst_specific_event_param->l_param      = l_param;

    OAL_IO_PRINT("wal_hipriv_ota_switch: en_ota_type:%d  en_switch_type:%d \n", l_ota_type, l_param);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(wal_specific_event_type_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ota_switch::return err code[%d]!}\r\n", ul_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
OAL_STATIC oal_uint32  wal_hipriv_set_smps_vap_mode(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                    l_tmp;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32                   l_ret;
    oal_uint32                  ul_ret;

    /* �˴���������"1"��"2"��"3"����ac_name */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_SMPS, "{wal_hipriv_set_smps_vap_mode::wal_get_cmd_one_arg_etc return err_code[%d]}\r\n", ul_ret);
        return ul_ret;
    }

    /* ��Խ������Ĳ�ͬ������ò�ͬSMPSģʽ */
    if (0 == (oal_strcmp("1", ac_name)))
    {
        l_tmp = 1;
    }
    else if (0 == (oal_strcmp("2", ac_name)))
    {
        l_tmp = 2;
    }
    else if (0 == (oal_strcmp("3", ac_name)))
    {
        l_tmp = 3;
    }
    else
    {
        OAM_ERROR_LOG1(0, OAM_SF_SMPS, "{wal_hipriv_set_smps_vap_mode::error cmd[%d],input 1/2/3!}", ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SMPS_VAP_MODE, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_tmp;  /* ��������������� */

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_SMPS, "{wal_hipriv_set_smps_vap_mode::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;


}



OAL_STATIC oal_uint32  wal_hipriv_set_smps_mode(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                    l_tmp;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32                   l_ret;
    oal_uint32                  ul_ret;

    /* �˴���������"1"��"2"��"3"����ac_name */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_SMPS, "{wal_hipriv_set_smps_mode::wal_get_cmd_one_arg_etc return err_code[%d]}\r\n", ul_ret);
        return ul_ret;
    }

    /* ��Խ������Ĳ�ͬ������ò�ͬSMPSģʽ */
    if (0 == (oal_strcmp("1", ac_name)))
    {
        l_tmp = 1;
    }
    else if (0 == (oal_strcmp("2", ac_name)))
    {
        l_tmp = 2;
    }
    else if (0 == (oal_strcmp("3", ac_name)))
    {
        l_tmp = 3;
    }
    else
    {
        OAM_ERROR_LOG1(0, OAM_SF_SMPS, "{wal_hipriv_set_smps_mode::error cmd[%d],input 1/2/3!}", ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SMPS_MODE, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_tmp;  /* ��������������� */

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_SMPS, "{wal_hipriv_set_smps_mode::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#if 0 // device smps��������Ҫ���ã�m2s�л�����Ҫ��smps�����߼�����vap����������������ֱ��ɾ���� ����smps������Ҫ�ٷſ�

OAL_STATIC oal_uint32  wal_hipriv_get_smps_mode_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru           st_write_msg;
    oal_int32                    l_ret;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SMPS_EN, OAL_SIZEOF(oal_int32));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_get_smps_mode_info::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;

}
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_UAPSD

OAL_STATIC oal_uint32  wal_hipriv_set_uapsd_cap(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_tmp;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32                   l_ret;
    oal_uint32                  ul_ret;

    /* �˴���������"1"��"0"����ac_name */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_uapsd_cap::wal_get_cmd_one_arg_etc return err_code[%d]}\r\n", ul_ret);
        return ul_ret;
    }

    /* ��Խ������Ĳ�ͬ�����UAPSD���ؽ��в�ͬ������ */
    if (0 == (oal_strcmp("0", ac_name)))
    {
        l_tmp = 0;
    }
    else if (0 == (oal_strcmp("1", ac_name)))
    {
        l_tmp = 1;
    }
    else
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_uapsd_cap::the log switch command is error [%d]!}\r\n", ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_UAPSD_EN, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_tmp;  /* ��������������� */

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_event_switch::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32  wal_hipriv_add_user(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    mac_cfg_add_user_param_stru    *pst_add_user_param;
    mac_cfg_add_user_param_stru     st_add_user_param;  /* ��ʱ�����ȡ��use����Ϣ */
    oal_uint32                      ul_get_addr_idx;

    /*
        ��������û�����������: hipriv "vap0 add_user xx xx xx xx xx xx(mac��ַ) 0 | 1(HT����λ) "
        ���������ĳһ��VAP
    */

    /* ��ȡmac��ַ */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_user::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    OAL_MEMZERO((oal_uint8*)&st_add_user_param, OAL_SIZEOF(st_add_user_param));
    oal_strtoaddr(ac_name, st_add_user_param.auc_mac_addr);
    /* ƫ�ƣ�ȡ��һ������ */
    pc_param = pc_param + ul_off_set;

    /* ��ȡ�û���HT��ʶ */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_user::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /* ��Խ������Ĳ�ͬ�����user��HT�ֶν��в�ͬ������ */
    if (0 == (oal_strcmp("0", ac_name)))
    {
        st_add_user_param.en_ht_cap = 0;
    }
    else if (0 == (oal_strcmp("1", ac_name)))
    {
        st_add_user_param.en_ht_cap = 1;
    }
    else
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_user::the mod switch command is error [%d]!}\r\n", ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ADD_USER, OAL_SIZEOF(mac_cfg_add_user_param_stru));

    /* ��������������� */
    pst_add_user_param = (mac_cfg_add_user_param_stru *)(st_write_msg.auc_value);
    for (ul_get_addr_idx = 0; ul_get_addr_idx < WLAN_MAC_ADDR_LEN; ul_get_addr_idx++)
    {
        pst_add_user_param->auc_mac_addr[ul_get_addr_idx] = st_add_user_param.auc_mac_addr[ul_get_addr_idx];
    }
    pst_add_user_param->en_ht_cap = st_add_user_param.en_ht_cap;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_add_user_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    wal_hipriv_wait_rsp(pst_net_dev, pc_param);
#endif

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_user::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_del_user(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    mac_cfg_del_user_param_stru    *pst_del_user_param;
    mac_cfg_del_user_param_stru     st_del_user_param;  /* ��ʱ�����ȡ��use����Ϣ */
    oal_uint32                      ul_get_addr_idx;

    /*
        ����ɾ���û�����������: hipriv "vap0 del_user xx xx xx xx xx xx(mac��ַ)"
        ���������ĳһ��VAP
    */

    /* ��ȡmac��ַ */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_del_user::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    OAL_MEMZERO((oal_uint8*)&st_del_user_param, OAL_SIZEOF(st_del_user_param));
    oal_strtoaddr(ac_name, st_del_user_param.auc_mac_addr);

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DEL_USER, OAL_SIZEOF(mac_cfg_add_user_param_stru));

    /* ��������������� */
    pst_del_user_param = (mac_cfg_add_user_param_stru *)(st_write_msg.auc_value);
    for (ul_get_addr_idx = 0; ul_get_addr_idx < WLAN_MAC_ADDR_LEN; ul_get_addr_idx++)
    {
        pst_del_user_param->auc_mac_addr[ul_get_addr_idx] = st_del_user_param.auc_mac_addr[ul_get_addr_idx];
    }

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_add_user_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_del_user::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_user_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    mac_vap_stru                    *pst_mac_vap;
    wal_msg_write_stru              st_write_msg;
    oal_int32                      l_ret;
    mac_cfg_user_info_param_stru    *pst_user_info_param;
    oal_uint8                       auc_mac_addr[6] = {0};    /* ��ʱ�����ȡ��use��mac��ַ��Ϣ */
    oal_uint8                       uc_char_index;
    oal_uint16                      us_user_idx;

    /* ȥ���ַ����Ŀո� */
    pc_param++;

    /* ��ȡmac��ַ,16����ת�� */
    for (uc_char_index = 0; uc_char_index < 12; uc_char_index++)
    {
        if (':' == *pc_param)
        {
            pc_param++;
            if (0 != uc_char_index)
            {
                uc_char_index--;
            }

            continue;
        }

        auc_mac_addr[uc_char_index/2] =
        (oal_uint8)(auc_mac_addr[uc_char_index/2] * 16 * (uc_char_index % 2) +
                                        oal_strtohex(pc_param));
        pc_param++;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_USER_INFO, OAL_SIZEOF(mac_cfg_user_info_param_stru));

    /* ����mac��ַ���û� */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);

    l_ret = (oal_int32)mac_vap_find_user_by_macaddr_etc(pst_mac_vap, auc_mac_addr, &us_user_idx);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_user_info::no such user!}\r\n");
        return OAL_FAIL;
    }

    /* ��������������� */
    pst_user_info_param              = (mac_cfg_user_info_param_stru *)(st_write_msg.auc_value);
    pst_user_info_param->us_user_idx = us_user_idx;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_user_info_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_user_info::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_set_mcast_data_dscr_param(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    mac_cfg_set_dscr_param_stru     *pst_set_dscr_param;
    wal_dscr_param_enum_uint8        en_param_index;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_DSCR, OAL_SIZEOF(mac_cfg_set_dscr_param_stru));

    /* ��������������������� */
    pst_set_dscr_param = (mac_cfg_set_dscr_param_stru *)(st_write_msg.auc_value);

    /* ��ȡ�������ֶ����������ַ��� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcast_data_dscr_param::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    /* ������������һ���ֶ� */
    for (en_param_index = 0; en_param_index < WAL_DSCR_PARAM_BUTT; en_param_index++)
    {
        if(!oal_strcmp(pauc_tx_dscr_param_name_etc[en_param_index], ac_arg))
        {
            break;
        }
    }

    /* ��������Ƿ��� */
    if (WAL_DSCR_PARAM_BUTT == en_param_index)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_mcast_data_dscr_param::no such param for tx dscr!}\r\n");
        return OAL_FAIL;
    }

    pst_set_dscr_param->uc_function_index = en_param_index;

    /*�������ʡ��ռ�����������*/
    if(en_param_index >= WAL_DSCR_PARAM_RATE && en_param_index <= WAL_DSCR_PARAM_BW)
    {
        ul_ret = wal_hipriv_process_rate_params(pst_net_dev, pc_param, pst_set_dscr_param);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcast_data_dscr_param::wal_hipriv_process_ucast_params return err_code [%d]!}\r\n", ul_ret);
            return ul_ret;
        }
    }
    else
    {
        /* ����Ҫ����Ϊ����ֵ */
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcast_data_dscr_param::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
            return ul_ret;
        }
        pc_param += ul_off_set;
        pst_set_dscr_param->l_value = oal_strtol(ac_arg, OAL_PTR_NULL, 0);
    }

    /* �鲥����֡���������� tpye = MAC_VAP_CONFIG_MCAST_DATA */
    pst_set_dscr_param->en_type = MAC_VAP_CONFIG_MCAST_DATA;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_set_dscr_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcast_data_dscr_param::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;

}



oal_uint32  wal_hipriv_set_rate_etc(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
//#ifdef _PRE_WLAN_CHIP_TEST
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    mac_cfg_non_ht_rate_stru         *pst_set_rate_param;
    wlan_legacy_rate_value_enum_uint8  en_rate_index;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wal_msg_stru                *pst_rsp_msg = OAL_PTR_NULL;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_RATE, OAL_SIZEOF(mac_cfg_non_ht_rate_stru));

    /* ��������������������� */
    pst_set_rate_param = (mac_cfg_non_ht_rate_stru *)(st_write_msg.auc_value);

    /* ��ȡ����ֵ�ַ��� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_rate_etc::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /* ����������Ϊ��һ������ */
    for (en_rate_index = 0; en_rate_index < WLAN_LEGACY_RATE_VALUE_BUTT; en_rate_index++)
    {
        if(!oal_strcmp(pauc_non_ht_rate_tbl[en_rate_index], ac_arg))
        {
            break;
        }
    }

    /* ������������TX�������е�Э��ģʽ */
    if (en_rate_index <= WLAN_SHORT_11b_11_M_BPS)
    {
        pst_set_rate_param->en_protocol_mode = WLAN_11B_PHY_PROTOCOL_MODE;
    }
    else if (en_rate_index >= WLAN_LEGACY_OFDM_48M_BPS && en_rate_index <= WLAN_LEGACY_OFDM_9M_BPS)
    {
        pst_set_rate_param->en_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_rate_etc::invalid rate!}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* ����Ҫ����Ϊ����ֵ */
    pst_set_rate_param->en_rate = en_rate_index;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_non_ht_rate_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_rate_etc::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
        /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_set_rate_etc fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }
//#endif  /* _PRE_WLAN_CHIP_TEST */
    return OAL_SUCC;
}


oal_uint32  wal_hipriv_set_mcs_etc(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
//#ifdef _PRE_WLAN_CHIP_TEST
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    mac_cfg_tx_comp_stru             *pst_set_mcs_param;
    oal_int32                        l_mcs;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32                        l_idx = 0;
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_MCS, OAL_SIZEOF(mac_cfg_tx_comp_stru));

    /* ��������������������� */
    pst_set_mcs_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    /* ��ȡ����ֵ�ַ��� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs_etc::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /* ��������Ϸ��Լ�� */
    while ('\0' != ac_arg[l_idx])
    {
        if (isdigit(ac_arg[l_idx]))
        {
            l_idx++;
            continue;
        }
        else
        {
            l_idx++;
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_mcs_etc::input illegal!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* ����Ҫ����Ϊ����ֵ */
    l_mcs = oal_atoi(ac_arg);

    if (l_mcs < WAL_HIPRIV_HT_MCS_MIN || l_mcs > WAL_HIPRIV_HT_MCS_MAX)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs_etc::input val out of range [%d]!}\r\n", l_mcs);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_mcs_param->uc_param = (oal_uint8)l_mcs;
    pst_set_mcs_param->en_protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_tx_comp_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs_etc::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
        /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_set_mcs_etc fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }
//#endif  /* _PRE_WLAN_CHIP_TEST */
    return OAL_SUCC;
}


oal_uint32  wal_hipriv_set_mcsac_etc(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
//#ifdef _PRE_WLAN_CHIP_TEST
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    mac_cfg_tx_comp_stru             *pst_set_mcs_param;
    oal_int32                        l_mcs;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32                        l_idx = 0;
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_MCSAC, OAL_SIZEOF(mac_cfg_tx_comp_stru));

    /* ��������������������� */
    pst_set_mcs_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    /* ��ȡ����ֵ�ַ��� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsac_etc::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /* ��������Ϸ��Լ�� */
    while ('\0' != ac_arg[l_idx])
    {
        if (isdigit(ac_arg[l_idx]))
        {
            l_idx++;
            continue;
        }
        else
        {
            l_idx++;
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_mcsac_etc::input illegal!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* ����Ҫ����Ϊ����ֵ */
    l_mcs = oal_atoi(ac_arg);

    if (l_mcs < WAL_HIPRIV_VHT_MCS_MIN || l_mcs > WAL_HIPRIV_VHT_MCS_MAX)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs_etc::input val out of range [%d]!}\r\n", l_mcs);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_mcs_param->uc_param = (oal_uint8)l_mcs;
    pst_set_mcs_param->en_protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_tx_comp_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsac_etc::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
        /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_set_mcsac_etc fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }

//#endif  /* _PRE_WLAN_CHIP_TEST */
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11AX

oal_uint32  wal_hipriv_set_mcsax(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    mac_cfg_tx_comp_stru             *pst_set_mcs_param;
    oal_int32                        l_mcs;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32                        l_idx = 0;
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_MCSAX, OAL_SIZEOF(mac_cfg_tx_comp_stru));

    /* ��������������������� */
    pst_set_mcs_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    /* ��ȡ����ֵ�ַ��� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsax::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /* ��������Ϸ��Լ�� */
    while ('\0' != ac_arg[l_idx])
    {
        if (isdigit(ac_arg[l_idx]))
        {
            l_idx++;
            continue;
        }
        else
        {
            l_idx++;
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_mcsax::input illegal!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* ����Ҫ����Ϊ����ֵ */
    l_mcs = oal_atoi(ac_arg);

    if (l_mcs < WAL_HIPRIV_HE_MCS_MIN || l_mcs > WAL_HIPRIV_HE_MCS_MAX)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsax::input val out of range [%d]!}\r\n", l_mcs);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_mcs_param->uc_param = (oal_uint8)l_mcs;
    pst_set_mcs_param->en_protocol_mode = WLAN_HE_SU_FORMAT;/* TODO:Ĭ����SU PPDU */

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_tx_comp_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsax::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
        /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_set_mcsax fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32  wal_hipriv_set_bw(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
//#ifdef _PRE_WLAN_CHIP_TEST
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    mac_cfg_tx_comp_stru             *pst_set_bw_param;
    hal_channel_assemble_enum_uint8  en_bw_index;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_BW, OAL_SIZEOF(mac_cfg_tx_comp_stru));

    /* ��������������������� */
    pst_set_bw_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    /* ��ȡ����ֵ�ַ��� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_bw::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /* ����Ҫ����Ϊ����ֵ */
    for (en_bw_index = 0; en_bw_index < WLAN_BAND_ASSEMBLE_AUTO; en_bw_index++)
    {
        if(!oal_strcmp(pauc_bw_tbl[en_bw_index], ac_arg))
        {
            break;
        }
    }

    /* ��������Ƿ��� */
    if (en_bw_index >= WLAN_BAND_ASSEMBLE_AUTO)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_bw::not support this bandwidth!}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_bw_param->uc_param = (oal_uint8)(en_bw_index);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_tx_comp_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_bw::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
    /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_set_bw fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }
//#endif  /* _PRE_WLAN_CHIP_TEST */
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX

OAL_STATIC oal_uint32  wal_hipriv_always_tx(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    mac_cfg_tx_comp_stru             *pst_set_bcast_param;
    oal_int8                         ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint8                        en_tx_flag = HAL_ALWAYS_TX_DISABLE;
    mac_rf_payload_enum_uint8        en_payload_flag = RF_PAYLOAD_RAND;
    oal_uint32                       ul_len = 2000;
    wal_msg_stru                *pst_rsp_msg = OAL_PTR_NULL;
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_ALWAYS_TX, OAL_SIZEOF(mac_cfg_tx_comp_stru));

    /* ��������������������� */
    pst_set_bcast_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    /* ��ȡ����ģʽ���ر�־ */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }
    en_tx_flag = (oal_uint8)oal_atoi(ac_name);

    /* �ݲ�֧�־ۺϳ��� */
    if (en_tx_flag >= HAL_ALWAYS_TX_AMPDU_ENABLE)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_always_tx::input should be 0 or 1.}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* �رյ�����²���Ҫ��������Ĳ��� */
    if (HAL_ALWAYS_TX_DISABLE != en_tx_flag)
    {
        /* ack_policy������������ */

        /* ��ȡpayload_flag���� */
        pc_param = pc_param + ul_off_set;
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        /* ������payload_flag��len����û�����ã�����Ĭ��RF_PAYLOAD_RAND 2000 */
        if (OAL_SUCC == ul_ret)
        {
            en_payload_flag = (oal_uint8)oal_atoi(ac_name);
            if(en_payload_flag >= RF_PAYLOAD_BUTT)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::payload flag err[%d]!}\r\n", en_payload_flag);
                return OAL_ERR_CODE_INVALID_CONFIG;
            }

            /* ��ȡlen���� */
            pc_param = pc_param + ul_off_set;
            ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
            if (OAL_SUCC != ul_ret)
            {
                 OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
                 return ul_ret;
            }
            ul_len = (oal_uint16)oal_atoi(ac_name);
            if(ul_len > 65535)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::len [%u] overflow!}\r\n", ul_len);
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
        }
    }


    pst_set_bcast_param->en_payload_flag = en_payload_flag;
    pst_set_bcast_param->ul_payload_len = ul_len;
    pst_set_bcast_param->uc_param = en_tx_flag;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_tx_comp_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_always_tx fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}



OAL_STATIC oal_uint32  wal_hipriv_always_tx_num(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                      *pul_num;
    oal_uint32                       ul_ret;
    oal_int8                         ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                       ul_off_set;
    oal_int32                        l_ret;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_ALWAYS_TX_NUM, OAL_SIZEOF(oal_uint32));

    /* ��ȡ�������� */
    pul_num = (oal_uint32 *)(st_write_msg.auc_value);

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_packet_xmit::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }
    *pul_num = (oal_uint32)oal_atoi(ac_name);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;

}


OAL_STATIC oal_uint32  wal_hipriv_always_tx_hw_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    mac_cfg_al_tx_hw_cfg_stru       *pst_cfg;
    oal_int8                         ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int8                        *pc_end;
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_ALWAYS_TX_HW_CFG, OAL_SIZEOF(mac_cfg_al_tx_hw_cfg_stru));

    /* ��������������������� */
    pst_cfg = (mac_cfg_al_tx_hw_cfg_stru *)(st_write_msg.auc_value);

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_hw_cfg::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }
    pc_param = pc_param + ul_off_set;
    pst_cfg->ul_devid = (oal_uint32)oal_strtol(ac_name, &pc_end, 16);

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_hw_cfg::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }
    pc_param = pc_param + ul_off_set;
    pst_cfg->ul_mode = (oal_uint32)oal_strtol(ac_name, &pc_end, 16);

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_hw_cfg::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }
    pc_param = pc_param + ul_off_set;
    pst_cfg->ul_rate = (oal_uint32)oal_strtol(ac_name, &pc_end, 16);


    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_al_tx_hw_cfg_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_hw_cfg::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}




OAL_STATIC oal_uint32  wal_hipriv_always_tx_hw(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    mac_cfg_al_tx_hw_stru           *pst_al_hw_tx;
    oal_int8                         ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint8                        uc_value;
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_ALWAYS_TX_HW, OAL_SIZEOF(mac_cfg_al_tx_hw_stru));

    /* ��������������������� */
    pst_al_hw_tx = (mac_cfg_al_tx_hw_stru *)(st_write_msg.auc_value);

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_hw::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    uc_value = (oal_uint8)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;
    //uc_dev_id = (pst_al_hw_tx->bit_switch & 0x2);
    pst_al_hw_tx->bit_switch = uc_value & 0x01;
    pst_al_hw_tx->bit_dev_id = uc_value & 0x02;
    pst_al_hw_tx->bit_flag   = (uc_value & 0x0c) >> 2; /* 0b1100 */
    if (OAL_SWITCH_ON == pst_al_hw_tx->bit_switch)
    {

        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
             OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_hw::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
             return ul_ret;
        }
        pc_param = pc_param + ul_off_set;
        pst_al_hw_tx->uc_content = (oal_uint8)oal_atoi(ac_name);


        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
             OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_hw::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
             return ul_ret;
        }
        pc_param = pc_param + ul_off_set;
        pst_al_hw_tx->ul_len = (oal_uint32)oal_atoi(ac_name);
        if(65535 < pst_al_hw_tx->ul_len)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_hw::len [%u] overflow!}\r\n", pst_al_hw_tx->ul_len);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
             OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_hw::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
             return ul_ret;
        }
        pc_param += ul_off_set;
        pst_al_hw_tx->ul_times = (oal_uint32)oal_atoi(ac_name);

        /* ��ȡ֡������� */
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
             OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_hw::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
             return ul_ret;
        }
        pst_al_hw_tx->ul_ifs = (oal_uint32)oal_atoi(ac_name);
        if(pst_al_hw_tx->ul_ifs > 65535)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_hw::al_tx ifs [%u] overflow!}\r\n", pst_al_hw_tx->ul_ifs);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }


    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_al_tx_hw_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_hw::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}



#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

OAL_STATIC oal_uint32  wal_hipriv_always_tx_51(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
#if defined (_PRE_WLAN_CHIP_TEST) || defined (_PRE_WLAN_FEATURE_ALWAYS_TX)
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    mac_cfg_tx_comp_stru             *pst_set_bcast_param;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_switch_enum_uint8            en_tx_flag;
    wlan_tx_ack_policy_enum_uint8    en_ack_policy;
    mac_rf_payload_enum_uint8        en_payload_flag;
    oal_uint32                       ul_len;
    mac_vap_stru                    *pst_mac_vap;

    //OAL_IO_PRINT("%s\n", __FUNCTION__);
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_ALWAYS_TX_51, OAL_SIZEOF(mac_cfg_tx_comp_stru));

    /* ��������������������� */
    pst_set_bcast_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    /* ��ȡ����ģʽ���ر�־ */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_51::get tx_flag return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    en_tx_flag = (oal_uint8)oal_atoi(ac_arg);

    if (en_tx_flag > HAL_ALWAYS_TX_RF)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_always_tx_51::input should be 0 or 1.}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_bcast_param->uc_param = en_tx_flag;

    /* ��ȡack_policy���� */
    pc_param = pc_param + ul_off_set;
    ul_ret   = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);

    if (!oal_strcmp(ac_arg, ""))
    {
        pst_mac_vap = (mac_vap_stru *)OAL_NET_DEV_PRIV(pst_net_dev);
        if (OAL_UNLIKELY(OAL_PTR_NULL  == pst_mac_vap))
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_always_tx_51::Get mac vap failed.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        pst_set_bcast_param->en_ack_policy = 1;
        pst_set_bcast_param->en_payload_flag = 2;
        pst_set_bcast_param->ul_payload_len = 4000;
    }
    else
    {
    en_ack_policy = (wlan_tx_ack_policy_enum_uint8)oal_atoi(ac_arg);
    if (en_ack_policy >= WLAN_TX_NO_EXPLICIT_ACK)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_51::ack policy flag err[%d]!}\r\n", en_ack_policy);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    pst_set_bcast_param->en_ack_policy = en_ack_policy;

    /* ��ȡpayload_flag���� */
    pc_param = pc_param + ul_off_set;
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_51::wal_get_cmd_one_arg_etc return err_code [%u]!}\r\n", ul_ret);
        return ul_ret;
    }
    en_payload_flag = (mac_rf_payload_enum_uint8)oal_atoi(ac_arg);
    if (en_payload_flag >= RF_PAYLOAD_BUTT)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_51::payload flag err[%d]!}\r\n", en_payload_flag);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    pst_set_bcast_param->en_payload_flag = en_payload_flag;

    /* ��ȡlen���� */
    pc_param = pc_param + ul_off_set;
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_51::wal_get_cmd_one_arg_etc return err_code [%u]!}\r\n", ul_ret);
        return ul_ret;
    }
    ul_len = (oal_uint32)oal_atoi(ac_arg);
    if (ul_len >= 65535)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_51::len [%u] overflow!}\r\n", ul_len);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    pst_set_bcast_param->ul_payload_len = ul_len;
    }

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_tx_comp_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    wal_hipriv_wait_rsp(pst_net_dev, pc_param);
#endif

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
#endif
    return OAL_SUCC;
}
#endif
#endif


OAL_STATIC oal_uint32  wal_hipriv_always_rx(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint8                        uc_rx_flag;
    oal_int32                        l_idx = 0;
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;

    /* ��ȡ����ģʽ���ر�־ */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_rx::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /* ��������Ϸ��Լ�� */
    while ('\0' != ac_arg[l_idx])
    {
        if (isdigit(ac_arg[l_idx]))
        {
            l_idx++;
            continue;
        }
        else
        {
            l_idx++;
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_always_rx::input illegal!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* ���������ֵ�ַ���ת��Ϊ���� */
    uc_rx_flag = (oal_uint8)oal_atoi(ac_arg);

    if ( uc_rx_flag > HAL_ALWAYS_RX_RESERVED)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_always_rx::input should be 0 or 1.}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    *(oal_uint8 *)(st_write_msg.auc_value) = uc_rx_flag;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_ALWAYS_RX, OAL_SIZEOF(oal_uint8));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_rx::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
        /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_always_rx fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

OAL_STATIC oal_uint32  wal_hipriv_always_rx_51(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint8                        uc_rx_flag;
    oal_int32                        l_idx = 0;

    //OAL_IO_PRINT("%s\n", __FUNCTION__);
    /* ��ȡ����ģʽ���ر�־ */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_rx_51::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /* ��������Ϸ��Լ�� */
    while ('\0' != ac_arg[l_idx])
    {
        if (isdigit(ac_arg[l_idx]))
        {
            l_idx++;
            continue;
        }
        else
        {
            l_idx++;
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_always_rx_51::input illegal!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* ���������ֵ�ַ���ת��Ϊ���� */
    uc_rx_flag = (oal_uint8)oal_atoi(ac_arg);

    if ( uc_rx_flag > HAL_ALWAYS_RX_RESERVED)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_always_rx_51::input should be 0 or 1.}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    *(oal_uint8 *)(st_write_msg.auc_value) = uc_rx_flag;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_ALWAYS_RX_51, OAL_SIZEOF(oal_uint8));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    wal_hipriv_wait_rsp(pst_net_dev, pc_param);
#endif

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_rx_51::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32  wal_hipriv_pcie_pm_level(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_int8                    ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32                   l_ret;
    oal_uint16                  us_len;
    oal_uint32                  ul_ret;
    oal_uint32                  ul_off_set;
    mac_cfg_pcie_pm_level_stru     *pst_pcie_pm_level;

    /* �����ʽ: hipriv "Hisilicon0 pcie_pm_level level(0/1/2/3/4)" */
    pst_pcie_pm_level = (mac_cfg_pcie_pm_level_stru *)st_write_msg.auc_value;

    /* ppm */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pcie_pm_level::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    pst_pcie_pm_level->uc_pcie_pm_level = (oal_uint8)oal_atoi(ac_arg);
    if (pst_pcie_pm_level->uc_pcie_pm_level > 4)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pcie_pm_level::pcie pm level must in set(0/1/2/3/4);\r\n", pst_pcie_pm_level->uc_pcie_pm_level);
        return ul_ret;
    }

    us_len = OAL_SIZEOF(mac_cfg_pcie_pm_level_stru);
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_PCIE_PM_LEVEL, us_len);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                             (oal_uint8 *)&st_write_msg,
                             OAL_FALSE,
                             OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pcie_pm_level::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_find_cmd(oal_int8 *pc_cmd_name, wal_hipriv_cmd_entry_stru **pst_cmd_id)
{
    oal_uint32                en_cmd_idx;
    int                       l_ret;
    *pst_cmd_id               = NULL;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pc_cmd_name) || (OAL_PTR_NULL == pst_cmd_id)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_hipriv_find_cmd::pc_cmd_name/puc_cmd_id null ptr error [%d] [%d]!}\r\n", pc_cmd_name, pst_cmd_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (en_cmd_idx = 0; en_cmd_idx < OAL_ARRAY_SIZE(g_ast_hipriv_cmd); en_cmd_idx++)
    {
        l_ret = oal_strcmp(g_ast_hipriv_cmd[en_cmd_idx].pc_cmd_name, pc_cmd_name);

        if (0 == l_ret)
        {
            *pst_cmd_id = (wal_hipriv_cmd_entry_stru*)&g_ast_hipriv_cmd[en_cmd_idx];
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_find_cmd::wal_hipriv_find_cmd en_cmd_idx = %d.}\r\n", en_cmd_idx);

            return OAL_SUCC;
        }
    }

#ifdef _PRE_WLAN_CFGID_DEBUG
    for (en_cmd_idx = 0; en_cmd_idx < wal_hipriv_get_debug_cmd_size_etc(); en_cmd_idx++)
    {
        l_ret = oal_strcmp(g_ast_hipriv_cmd_debug_etc[en_cmd_idx].pc_cmd_name, pc_cmd_name);

        if (0 == l_ret)
        {
            *pst_cmd_id = (wal_hipriv_cmd_entry_stru*)&g_ast_hipriv_cmd_debug_etc[en_cmd_idx];
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_find_cmd::wal_hipriv_find_cmd en_cmd_idx = %d.}\r\n", en_cmd_idx);

            return OAL_SUCC;
        }
    }
#endif

    OAM_IO_PRINTK("cmd name[%s] is not exist. \r\n", pc_cmd_name);
    return OAL_FAIL;
}


OAL_STATIC oal_uint32  wal_hipriv_get_cmd_net_dev(oal_int8 *pc_cmd, oal_net_device_stru **ppst_net_dev, oal_uint32 *pul_off_set)
{
    oal_net_device_stru  *pst_net_dev;
    oal_int8              ac_dev_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32            ul_ret;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pc_cmd) || (OAL_PTR_NULL == ppst_net_dev) || (OAL_PTR_NULL == pul_off_set)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_net_dev::pc_cmd/ppst_net_dev/pul_off_set null ptr error [%d] [%d] [%d]!}\r\n", pc_cmd, ppst_net_dev, pul_off_set);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = wal_get_cmd_one_arg_etc(pc_cmd, ac_dev_name, pul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_net_dev::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /* ����dev_name�ҵ�dev */
    pst_net_dev = oal_dev_get_by_name(ac_dev_name);
    if (OAL_PTR_NULL == pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_net_dev::oal_dev_get_by_name return null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
    oal_dev_put(pst_net_dev);

    *ppst_net_dev = pst_net_dev;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_get_cmd_id(oal_int8 *pc_cmd, wal_hipriv_cmd_entry_stru **pst_cmd_id, oal_uint32 *pul_off_set)
{
    oal_int8                    ac_cmd_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                  ul_ret;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pc_cmd) || (OAL_PTR_NULL == pst_cmd_id) || (OAL_PTR_NULL == pul_off_set)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_id::pc_cmd/puc_cmd_id/pul_off_set null ptr error [%d] [%d] [%d]!}\r\n", pc_cmd, pst_cmd_id, pul_off_set);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = wal_get_cmd_one_arg_etc(pc_cmd, ac_cmd_name, pul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_id::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /* �����������ҵ�����ö�� */
    ul_ret = wal_hipriv_find_cmd(ac_cmd_name, pst_cmd_id);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_id::wal_hipriv_find_cmd return error cod [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32  wal_hipriv_parse_cmd_etc(oal_int8 *pc_cmd)
{
    oal_net_device_stru        *pst_net_dev;
    wal_hipriv_cmd_entry_stru  *pst_hipriv_cmd_entry = NULL;
    oal_uint32                  ul_off_set = 0;
    oal_uint32                  ul_ret;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    oal_uint32                  ul_num;
    oal_int8                    cmd_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_vap_stru               *pst_mac_vap;
    oal_uint32                  ul_mib_op_cmd_num,ul_index;
#endif
    if (OAL_UNLIKELY(OAL_PTR_NULL == pc_cmd))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd_etc::pc_cmd null ptr error!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
        cmd��ʽԼ��
        �����豸�� ����      ����   Hisilicon0 create vap0
        1~15Byte   1~15Byte
    **************************** ***********************************************/
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    //OAL_IO_PRINT("test_by_minos init\n");

    ul_ret = wal_hipriv_get_cmd_net_dev(pc_cmd, &pst_net_dev, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd_etc::wal_hipriv_get_cmd_net_dev return error code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    pc_cmd += ul_off_set;

    ul_ret = wal_get_cmd_one_arg_etc(pc_cmd, cmd_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_id::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    /* ������vap,����mib����������ǿռ�� */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd_etc::pst_mac_vap null ptr error!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    ul_mib_op_cmd_num = OAL_SIZEOF(g_pac_mib_operation_cmd_name) >> 2;
    if(WLAN_VAP_MODE_CONFIG == pst_mac_vap->en_vap_mode)
    {
        for(ul_index = 0; ul_index < ul_mib_op_cmd_num; ul_index++)
        {
            if(0 == oal_strcmp(cmd_name,g_pac_mib_operation_cmd_name[ul_index]))
            {
                if(OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap->pst_mib_info))
                {
                    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_id::pst_mib_info is null!}\r\n");
                    return OAL_ERR_CODE_PTR_NULL;
                }
            }
        }
    }

    if((0 == oal_strcmp(cmd_name, g_pac_wal_switch_info[0])) ||(0 == oal_strcmp(cmd_name, g_pac_wal_switch_info[1])) )
    {
        ul_ret  = wal_hipriv_get_cmd_id(pc_cmd, &pst_hipriv_cmd_entry, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd_etc::wal_hipriv_get_cmd_id return error code [%d]!}\r\n", ul_ret);
            return ul_ret;
        }

        pc_cmd += ul_off_set;

        for(ul_num=0; ul_num<(sizeof(g_pac_wal_all_vap)/sizeof(g_pac_wal_all_vap[0])); ul_num++ )
        {

            ul_ret = wal_hipriv_get_cmd_net_dev(g_pac_wal_all_vap[ul_num], &pst_net_dev, &ul_off_set);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd_etc::wal_hipriv_get_cmd_net_dev return error code [%d]!}\r\n", ul_ret);
                return ul_ret;
            }
                    /* ���������Ӧ�ĺ��� */
            ul_ret = pst_hipriv_cmd_entry->p_func(pst_net_dev, pc_cmd);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd_etc::g_ast_hipriv_cmd return error code [%d]!}\r\n", ul_ret);
                return ul_ret;
            }
        }
    }
    else
    {
        ul_ret  = wal_hipriv_get_cmd_id(pc_cmd, &pst_hipriv_cmd_entry, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd_etc::wal_hipriv_get_cmd_id return error code [%d]!}\r\n", ul_ret);
            return ul_ret;
        }

        pc_cmd += ul_off_set;

        /* ���������Ӧ�ĺ��� */
        ul_ret = pst_hipriv_cmd_entry->p_func(pst_net_dev, pc_cmd);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd_etc::g_ast_hipriv_cmd return error code [%d]!}\r\n", ul_ret);
            return ul_ret;
        }
    }

#else
    ul_ret = wal_hipriv_get_cmd_net_dev(pc_cmd, &pst_net_dev, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd_etc::wal_hipriv_get_cmd_net_dev return error code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    pc_cmd += ul_off_set;
    ul_ret  = wal_hipriv_get_cmd_id(pc_cmd, &pst_hipriv_cmd_entry, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd_etc::wal_hipriv_get_cmd_id return error code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    pc_cmd += ul_off_set;
    /* ���������Ӧ�ĺ��� */
    ul_ret = pst_hipriv_cmd_entry->p_func(pst_net_dev, pc_cmd);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd_etc::g_ast_hipriv_cmd return error code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
#endif
    return OAL_SUCC;
}

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT

OAL_STATIC oal_ssize_t  wal_hipriv_sys_write(oal_device_stru *dev, oal_device_attribute_stru *attr, const char *pc_buffer, oal_size_t count)
//OAL_STATIC oal_ssize_t  wal_hipriv_sys_write(oal_device_stru *dev, oal_device_attribute_stru *attr, const oal_int8 *pc_buffer, oal_size_t count)
{
    oal_int8  *pc_cmd;
    oal_uint32 ul_ret;
    oal_uint32 ul_len = (oal_uint32)count;

    if (ul_len > WAL_HIPRIV_CMD_MAX_LEN)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sys_write::ul_len>WAL_HIPRIV_CMD_MAX_LEN, ul_len [%d]!}\r\n", ul_len);
        return -OAL_EINVAL;
    }

    pc_cmd = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, WAL_HIPRIV_CMD_MAX_LEN, OAL_TRUE);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pc_cmd))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_proc_write::alloc mem return null ptr!}\r\n");
        return -OAL_ENOMEM;
    }

    OAL_MEMZERO(pc_cmd, WAL_HIPRIV_CMD_MAX_LEN);

    /*ul_ret = oal_copy_from_user(pc_cmd, pc_buffer, ul_len);*/
    oal_memcopy(pc_cmd, pc_buffer, ul_len);

    pc_cmd[ul_len - 1] = '\0';

    ul_ret = wal_hipriv_parse_cmd_etc(pc_cmd);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_proc_write::parse cmd return err code[%d]!}\r\n", ul_ret);
    }

    OAL_MEM_FREE(pc_cmd, OAL_TRUE);

    return (oal_int32)ul_len;
}


#define SYS_READ_MAX_STRING_LEN (4096-40)   /* ��ǰ�����ַ�����20�ֽ��ڣ�Ԥ��40��֤���ᳬ�� */
OAL_STATIC oal_ssize_t  wal_hipriv_sys_read(oal_device_stru *dev, oal_device_attribute_stru *attr, char *pc_buffer)
{
    oal_uint32              ul_cmd_idx;
    oal_uint32              buff_index = 0;

    for (ul_cmd_idx = 0; ul_cmd_idx < OAL_ARRAY_SIZE(g_ast_hipriv_cmd); ul_cmd_idx++)
    {
        buff_index += OAL_SPRINTF(pc_buffer+buff_index,(SYS_READ_MAX_STRING_LEN-buff_index),
            "\t%s\n",g_ast_hipriv_cmd[ul_cmd_idx].pc_cmd_name);

        if(buff_index > SYS_READ_MAX_STRING_LEN){
            buff_index += OAL_SPRINTF(pc_buffer+buff_index,(SYS_READ_MAX_STRING_LEN-buff_index),"\tmore...\n");
            break;
        }
    }
#ifdef _PRE_WLAN_CFGID_DEBUG
    for (ul_cmd_idx = 0; ul_cmd_idx < wal_hipriv_get_debug_cmd_size_etc(); ul_cmd_idx++)
    {
        buff_index += OAL_SPRINTF(pc_buffer+buff_index,(SYS_READ_MAX_STRING_LEN-buff_index),
            "\t%s\n",g_ast_hipriv_cmd_debug_etc[ul_cmd_idx].pc_cmd_name);

        if(buff_index > SYS_READ_MAX_STRING_LEN){
            buff_index += OAL_SPRINTF(pc_buffer+buff_index,(SYS_READ_MAX_STRING_LEN-buff_index),"\tmore...\n");
            break;
        }
    }
#endif

    return buff_index;
}

#endif  /* _PRE_OS_VERSION_LINUX */

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST

oal_int32  wal_hipriv_wait_rsp(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
#if (_PRE_TEST_MODE_UT != _PRE_TEST_MODE)
    mac_vap_stru                *pst_mac_vap;
    hmac_vap_stru               *pst_hmac_vap;
    oal_int                     i_leftime;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(0,OAM_SF_ANY,"{wal_hipriv_wait_rsp::mac_res_get_hmac_vap fail.vap_id[%u]}",pst_mac_vap->uc_vap_id);
        return -OAL_EINVAL;
    }

    /*lint -e730*/
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(OAL_TRUE == pst_hmac_vap->st_hipriv_ack_stats.uc_get_hipriv_ack_flag),100);
    /*lint +e730*/
    if ( i_leftime > 0 )
    {
        //OAL_IO_PRINT("Success\n");
        if (pst_hmac_vap->st_hipriv_ack_stats.ul_buf_len >= HMAC_HIPRIV_ACK_BUF_SIZE)
        {
            oal_copy_to_user((oal_void *)pst_hmac_vap->st_hipriv_ack_stats.pc_buffer, (oal_void *)pst_hmac_vap->st_hipriv_ack_stats.auc_data, HMAC_HIPRIV_ACK_BUF_SIZE);
        }
    }
#endif

    return OAL_TRUE;
}
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
    // use sys filesystem instead
#else

OAL_STATIC oal_int32  wal_hipriv_proc_write(oal_file_stru *pst_file, const oal_int8 *pc_buffer, oal_uint32 ul_len, oal_void *p_data)
{
    oal_int8                    *pc_cmd;
    oal_uint32                  ul_ret;
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (_PRE_TEST_MODE_UT != _PRE_TEST_MODE)
    oal_net_device_stru         *pst_net_dev;
    mac_vap_stru                *pst_mac_vap;
    hmac_vap_stru               *pst_hmac_vap;
    oal_uint32                  ul_off_set;
#endif

    if (ul_len > WAL_HIPRIV_CMD_MAX_LEN)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_proc_write::ul_len>WAL_HIPRIV_CMD_MAX_LEN, ul_len [%d]!}\r\n", ul_len);
        return -OAL_EINVAL;
    }

    pc_cmd = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, WAL_HIPRIV_CMD_MAX_LEN, OAL_TRUE);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pc_cmd))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_proc_write::alloc mem return null ptr!}\r\n");
        return -OAL_ENOMEM;
    }

    OAL_MEMZERO(pc_cmd, WAL_HIPRIV_CMD_MAX_LEN);

    ul_ret = oal_copy_from_user((oal_void *)pc_cmd, pc_buffer, ul_len);

    /* copy_from_user������Ŀ���Ǵ��û��ռ俽�����ݵ��ں˿ռ䣬ʧ�ܷ���û�б��������ֽ������ɹ�����0 */
    if (ul_ret > 0)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_proc_write::oal_copy_from_user return ul_ret[%d]!}\r\n", ul_ret);
        OAL_MEM_FREE(pc_cmd, OAL_TRUE);

        return -OAL_EFAUL;
    }

    pc_cmd[ul_len - 1] = '\0';

#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (_PRE_TEST_MODE_UT != _PRE_TEST_MODE)
    /*װ�����Ե�vap_info��ʼ��*/
    ul_ret = wal_hipriv_get_cmd_net_dev(pc_cmd, &pst_net_dev, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_proc_write::wal_hipriv_get_cmd_net_dev return error code [%d]!}\r\n", ul_ret);
        return -OAL_EFAIL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(0,OAM_SF_ANY,"{wal_hipriv_proc_write::mac_res_get_hmac_vap fail.vap_id[%u]}",pst_mac_vap->uc_vap_id);
        return -OAL_EINVAL;
    }
    pst_hmac_vap->st_hipriv_ack_stats.uc_get_hipriv_ack_flag = OAL_FALSE;
    pst_hmac_vap->st_hipriv_ack_stats.pc_buffer = (oal_int8 *)pc_buffer;
    pst_hmac_vap->st_hipriv_ack_stats.ul_buf_len = ul_len;
    pst_hmac_vap->st_hipriv_ack_stats.auc_data[0] = 0xFF;
    pst_hmac_vap->st_hipriv_ack_stats.auc_data[1] = 0;
    pst_hmac_vap->st_hipriv_ack_stats.auc_data[2] = 0;
#endif

    ul_ret = wal_hipriv_parse_cmd_etc(pc_cmd);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_proc_write::parse cmd return err code[%d]!}\r\n", ul_ret);
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (_PRE_TEST_MODE_UT != _PRE_TEST_MODE)
        if (pst_hmac_vap->st_hipriv_ack_stats.ul_buf_len >= HMAC_HIPRIV_ACK_BUF_SIZE)
        {
            oal_copy_to_user((oal_void *)pc_buffer, (oal_void *)pst_hmac_vap->st_hipriv_ack_stats.auc_data, HMAC_HIPRIV_ACK_BUF_SIZE);
        }
#endif
    }

    OAL_MEM_FREE(pc_cmd, OAL_TRUE);

    return (oal_int32)ul_len;

}
#endif

oal_uint32  wal_hipriv_create_proc_etc(oal_void *p_proc_arg)
{
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    oal_uint32           ul_ret;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
        /* TBD */
        g_pst_proc_entry = OAL_PTR_NULL;
#else

    /* 420ʮ���ƶ�Ӧ�˽�����0644 linuxģʽ���� S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH); */
    /* S_IRUSR�ļ������߾߿ɶ�ȡȨ��, S_IWUSR�ļ������߾߿�д��Ȩ��, S_IRGRP�û���߿ɶ�ȡȨ��, S_IROTH�����û��߿ɶ�ȡȨ�� */
    g_pst_proc_entry = oal_create_proc_entry(WAL_HIPRIV_PROC_ENTRY_NAME, 420, NULL);
    if (OAL_PTR_NULL == g_pst_proc_entry)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_create_proc_etc::oal_create_proc_entry return null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_pst_proc_entry->data  = p_proc_arg;
    g_pst_proc_entry->nlink = 1;        /* linux����procĬ��ֵ */
    g_pst_proc_entry->read_proc  = OAL_PTR_NULL;

    g_pst_proc_entry->write_proc = (write_proc_t *)wal_hipriv_proc_write;
#endif

    /* hi1102-cb add sys for 51/02 */
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    gp_sys_kobject_etc = oal_get_sysfs_root_object_etc();
    if(NULL == gp_sys_kobject_etc)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_create_proc_etc::get sysfs root object failed!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = (oal_uint32)oal_debug_sysfs_create_group(gp_sys_kobject_etc,&hipriv_attribute_group);
    if (ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_create_proc_etc::hipriv_attribute_group create failed!}");
        ul_ret = OAL_ERR_CODE_PTR_NULL;
        return ul_ret;
    }
#endif

    return OAL_SUCC;
}


oal_uint32  wal_hipriv_remove_proc_etc(void)
{
/* ж��ʱɾ��sysfs */

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    if(NULL != gp_sys_kobject_etc)
    {
        oal_debug_sysfs_remove_group(gp_sys_kobject_etc, &hipriv_attribute_group);
        kobject_del(gp_sys_kobject_etc);
        gp_sys_kobject_etc = NULL;
    }
    oal_conn_sysfs_root_obj_exit_etc();
    oal_conn_sysfs_root_boot_obj_exit_etc();
#endif

    if (g_pst_proc_entry)
    {
        oal_remove_proc_entry(WAL_HIPRIV_PROC_ENTRY_NAME, NULL);
        g_pst_proc_entry = OAL_PTR_NULL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_reg_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;
    oal_uint16                      us_len;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    oal_memcopy(st_write_msg.auc_value, pc_param, OAL_STRLEN(pc_param));

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_REG_INFO, us_len);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_reg_info::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))

OAL_STATIC oal_uint32  wal_hipriv_sdio_flowctrl(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;
    oal_uint16                      us_len;

    if (OAL_UNLIKELY(WAL_MSG_WRITE_MAX_LEN <= OAL_STRLEN(pc_param)))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sdio_flowctrl:: pc_param overlength is %d}\n",OAL_STRLEN(pc_param));
        return OAL_FAIL;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    oal_memcopy(st_write_msg.auc_value, pc_param, OAL_STRLEN(pc_param));

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SDIO_FLOWCTRL, us_len);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sdio_flowctrl::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32 wal_hipriv_set_regdomain_pwr(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32                  ul_off_set;
    oal_int8                    ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                  ul_ret;
    oal_int32                   l_ret;
    oal_int32                   l_pwr;
    wal_msg_write_stru          st_write_msg;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "wal_hipriv_set_regdomain_pwr, get arg return err %d", ul_ret);
        return ul_ret;
    }

    l_pwr = oal_atoi(ac_name);
    if (l_pwr <= 0 || l_pwr > 100)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "invalid value, %d", l_pwr);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_REGDOMAIN_PWR, OAL_SIZEOF(mac_cfg_regdomain_max_pwr_stru));

    ((mac_cfg_regdomain_max_pwr_stru *)st_write_msg.auc_value)->uc_pwr        = (oal_uint8)l_pwr;
    ((mac_cfg_regdomain_max_pwr_stru *)st_write_msg.auc_value)->en_exceed_reg = OAL_FALSE;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                       WAL_MSG_TYPE_WRITE,
                       WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                       (oal_uint8 *)&st_write_msg,
                       OAL_FALSE,
                       OAL_PTR_NULL);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0,OAM_SF_CFG,"{wal_hipriv_set_regdomain_pwr::wal_send_cfg_event_etc fail.return err code %d}",l_ret);
    }

    return (oal_uint32)l_ret;
}


OAL_STATIC oal_uint32  wal_hipriv_dump_all_rx_dscr(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru           st_write_msg;
    oal_int32                    l_ret;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DUMP_ALL_RX_DSCR, OAL_SIZEOF(oal_int32));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_dump_all_rx_dscr::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}



OAL_STATIC oal_uint32  wal_hipriv_reg_write(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{

    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;
    oal_uint16                      us_len;

    /***************************************************************************
                              ���¼���wal�㴦��
    ***************************************************************************/
    oal_memcopy(st_write_msg.auc_value, pc_param, OAL_STRLEN(pc_param));

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_REG_WRITE, us_len);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                             (oal_uint8 *)&st_write_msg,
                             OAL_FALSE,
                             OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_reg_write::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}



#ifdef _PRE_WLAN_FEATURE_DFS

OAL_STATIC oal_uint32  wal_hipriv_dfs_radartool(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru    st_write_msg;
    oal_uint16            us_len;
    oal_int32             l_ret;

    if (OAL_UNLIKELY(WAL_MSG_WRITE_MAX_LEN <= OAL_STRLEN(pc_param)))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_dfs_radartool:: pc_param overlength is %d}\n", OAL_STRLEN(pc_param));
        oal_print_hex_dump((oal_uint8 *)pc_param, WAL_MSG_WRITE_MAX_LEN, 32, "wal_hipriv_dfs_radartool: param is overlong:");
        return OAL_FAIL;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    oal_memcopy(st_write_msg.auc_value, pc_param, OAL_STRLEN(pc_param));

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_RADARTOOL, us_len);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_dfs_radartool::return err code [%d]!}\r\n", l_ret);

        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif   /* end of _PRE_WLAN_FEATURE_DFS */


oal_uint32  wal_hipriv_alg_cfg_etc(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    mac_ioctl_alg_param_stru       *pst_alg_param = OAL_PTR_NULL;
    wal_ioctl_alg_cfg_stru          st_alg_cfg;
    oal_uint8                       uc_map_index = 0;
    oal_int32                       l_ret;
    wal_msg_stru                *pst_rsp_msg = OAL_PTR_NULL;

    pst_alg_param = (mac_ioctl_alg_param_stru *)(st_write_msg.auc_value);

    /* ��ȡ���ò������� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg_etc::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    /* Ѱ��ƥ������� */
    st_alg_cfg = g_ast_alg_cfg_map_etc[0];
    while(OAL_PTR_NULL != st_alg_cfg.pc_name)
    {
        if (0 == oal_strcmp(st_alg_cfg.pc_name, ac_name))
        {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map_etc[++uc_map_index];
    }

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg:: alg_cfg command[%d]!}\r\n", st_alg_cfg.en_alg_cfg);

    /* û���ҵ���Ӧ������򱨴� */
    if( OAL_PTR_NULL == st_alg_cfg.pc_name)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg_etc::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* ��¼�����Ӧ��ö��ֵ */
    pst_alg_param->en_alg_cfg = g_ast_alg_cfg_map_etc[uc_map_index].en_alg_cfg;

    /* ��ȡ��������ֵ */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param + ul_off_set, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg_etc::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    /* ��¼��������ֵ */
    pst_alg_param->ul_value = (oal_uint32)oal_atoi(ac_name);

    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ALG_PARAM, OAL_SIZEOF(mac_ioctl_alg_param_stru));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_ioctl_alg_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        return (oal_uint32)l_ret;
    }
    /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_alg_cfg_etc fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;

}



OAL_STATIC oal_uint32  wal_hipriv_tpc_log(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru                      st_write_msg;
    oal_uint32                              ul_off_set;
    oal_int8                                ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                              ul_ret;
    oal_int32                               l_ret;
    mac_ioctl_alg_tpc_log_param_stru       *pst_alg_tpc_log_param = OAL_PTR_NULL;
    wal_ioctl_alg_cfg_stru                  st_alg_cfg;
    oal_uint8                               uc_map_index = 0;
    oal_bool_enum_uint8                     en_stop_flag = OAL_FALSE;

    pst_alg_tpc_log_param = (mac_ioctl_alg_tpc_log_param_stru *)(st_write_msg.auc_value);

    /* ��ȡ���ò������� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param = pc_param + ul_off_set;

    /* Ѱ��ƥ������� */
    st_alg_cfg = g_ast_alg_cfg_map_etc[0];
    while(OAL_PTR_NULL != st_alg_cfg.pc_name)
    {
        if (0 == oal_strcmp(st_alg_cfg.pc_name, ac_name))
        {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map_etc[++uc_map_index];
    }

    /* û���ҵ���Ӧ������򱨴� */
    if( OAL_PTR_NULL == st_alg_cfg.pc_name)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* ��¼�����Ӧ��ö��ֵ */
    pst_alg_tpc_log_param->en_alg_cfg = g_ast_alg_cfg_map_etc[uc_map_index].en_alg_cfg;

    /* ���ֻ�ȡ�ض�֡���ʺ�ͳ����־�����:��ȡ����ֻ���ȡ֡���� */
    if (MAC_ALG_CFG_TPC_GET_FRAME_POW == pst_alg_tpc_log_param->en_alg_cfg)
    {
        /* ��ȡ���ò������� */
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
            return ul_ret;
        }
        /* ��¼�����Ӧ��֡���� */
        pst_alg_tpc_log_param->pc_frame_name = ac_name;
    }
    else
    {
        ul_ret = wal_hipriv_get_mac_addr_etc(pc_param, pst_alg_tpc_log_param->auc_mac_addr, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::wal_hipriv_get_mac_addr_etc failed!}\r\n");
            return ul_ret;
        }
        pc_param += ul_off_set;

        while ((' ' == *pc_param) || ('\0' == *pc_param))
        {
            if ('\0' == *pc_param)
            {
                en_stop_flag = OAL_TRUE;
                break;
            }
            ++ pc_param;
        }

        /* ��ȡҵ������ֵ */
        if (OAL_TRUE != en_stop_flag)
        {
            ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
                return ul_ret;
            }

            pst_alg_tpc_log_param->uc_ac_no = (oal_uint8)oal_atoi(ac_name);
            pc_param = pc_param + ul_off_set;

            en_stop_flag = OAL_FALSE;
            while ((' ' == *pc_param) || ('\0' == *pc_param))
            {
                if ('\0' == *pc_param)
                {
                    en_stop_flag = OAL_TRUE;
                    break;
                }
                ++ pc_param;
            }

            if (OAL_TRUE != en_stop_flag)
            {
                /* ��ȡ��������ֵ */
                ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
                if (OAL_SUCC != ul_ret)
                {
                    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
                    return ul_ret;
                }

                /* ��¼��������ֵ */
                pst_alg_tpc_log_param->us_value = (oal_uint16)oal_atoi(ac_name);
            }
        }
    }

    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ALG_PARAM, OAL_SIZEOF(mac_ioctl_alg_tpc_log_param_stru));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_ioctl_alg_tpc_log_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;

}

#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) && (!defined(_PRE_PRODUCT_ID_HI110X_HOST)))

oal_int32 wal_ioctl_get_assoc_req_ie(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    mac_vap_stru                   *pst_mac_vap;
    oal_uint16                      us_len;
    oal_int32                       l_ret;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_assoc_req_ie::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr!}\r\n");
        return -OAL_EINVAL;
    }

    l_ret = (oal_int32)wal_config_get_assoc_req_ie_etc(pst_mac_vap, &us_len, (oal_uint8 *)pst_ioctl_data);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_assoc_req_ie::return err code [%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}



OAL_STATIC oal_int32 wal_ioctl_set_auth_mode(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    mac_vap_stru                   *pst_mac_vap;
    oal_uint16                      us_len = sizeof(oal_uint32);
    oal_uint32                      ul_ret;
    wlan_auth_alg_mode_enum_uint8   en_auth_mode;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_auth_mode::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr!}\r\n");
        return -OAL_EINVAL;
    }

    /* hostapd.conf�ļ���ֱ������Ϊopen = 1��shared =2��open and shred= 3
     ioctlֱ�Ӵ���������Ҫ�����ں˽�����ʽ��������open = WLAN_WITP_AUTH_OPEN_SYSTEM
     shared = WLAN_WITP_AUTH_SHARED_KEY�� open and shred = WLAN_WITP_AUTH_AUTOMATIC */

    switch(pst_ioctl_data->pri_data.auth_params.auth_alg)
    {
          case 1:
              en_auth_mode = WLAN_WITP_AUTH_OPEN_SYSTEM;
              break;
          case 2:
              en_auth_mode = WLAN_WITP_AUTH_SHARED_KEY;
              break;
          case 3:
              en_auth_mode = WLAN_WITP_AUTH_AUTOMATIC;
              break;
          default:
              OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_auth_mode:uc_auth_mode error[%d]!}", pst_ioctl_data->pri_data.auth_params.auth_alg);
              return -OAL_EINVAL;
     }

    ul_ret = hmac_config_set_auth_mode_etc(pst_mac_vap, us_len, &en_auth_mode);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_auth_mode::hmac_config_set_auth_mode_etc return err code [%d]!}\r\n", ul_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_ioctl_set_max_user(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    mac_vap_stru                   *pst_mac_vap;
    oal_uint16                      us_len = sizeof(oal_uint32);
    oal_uint32                      ul_ret;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_max_user::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr!}\r\n");
        return -OAL_EINVAL;
    }

    ul_ret = hmac_config_set_max_user_etc(pst_mac_vap, us_len, (pst_ioctl_data->pri_data.ul_vap_max_user));
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_max_user::hmac_config_set_max_user_etc return err code [%d]!}\r\n", ul_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


oal_int32 wal_ioctl_set_ssid(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    mac_vap_stru                   *pst_mac_vap;
    oal_uint16                      us_len;
    oal_uint32                      ul_ret;
    mac_cfg_ssid_param_stru         st_ssid_param;
    oal_uint8                        uc_ssid_temp;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_ssid::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr!}\r\n");
        return -OAL_EINVAL;
    }
    /* ��֤�����strlen������� */
    pst_ioctl_data->pri_data.ssid[OAL_IEEE80211_MAX_SSID_LEN+3] = '\0';
    uc_ssid_temp = (oal_uint8)OAL_STRLEN((oal_int8 *)pst_ioctl_data->pri_data.ssid);
    us_len = OAL_MIN(uc_ssid_temp, OAL_IEEE80211_MAX_SSID_LEN);

    st_ssid_param.uc_ssid_len = (oal_uint8)us_len;
    oal_memcopy(st_ssid_param.ac_ssid, (const oal_void *)(pst_ioctl_data->pri_data.ssid), (oal_uint32)us_len);
    ul_ret = hmac_config_set_ssid_etc(pst_mac_vap, OAL_SIZEOF(st_ssid_param), (oal_uint8 *)&st_ssid_param);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_ssid::hmac_config_set_ssid_etc return err code [%d]!}\r\n", ul_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}




oal_int32 wal_ioctl_set_country_code(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
#ifdef _PRE_WLAN_FEATURE_11D
    oal_int8                        auc_country_code[4] = {0};
    oal_int32                       l_ret;

    oal_memcopy(auc_country_code, pst_ioctl_data->pri_data.country_code.auc_country_code, 3);

    l_ret = wal_regdomain_update_for_dfs_etc(pst_net_dev, auc_country_code);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_country_code::return err code [%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    l_ret = wal_regdomain_update_etc(pst_net_dev, auc_country_code);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_country_code::return err code [%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }
#endif

    return OAL_SUCC;
}

oal_int32  wal_ioctl_nl80211_priv_connect(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    mac_conn_param_stru                 st_mac_conn_param;
    oal_int32                           l_ret                   = OAL_SUCC;

    /* ��ʼ���������Ӳ��� */
    OAL_MEMZERO(&st_mac_conn_param, OAL_SIZEOF(mac_conn_param_stru));

    /* �����ں��·��� freq to channel_number eg.1,2,36,40...  */
    st_mac_conn_param.uc_channel    = (oal_uint8)oal_ieee80211_frequency_to_channel(pst_ioctl_data->pri_data.cfg80211_connect_params.l_freq);

    /* �����ں��·��� ssid */
    st_mac_conn_param.uc_ssid_len   = (oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.ssid_len;
    if(st_mac_conn_param.uc_ssid_len)
    {
        if(oal_copy_from_user(st_mac_conn_param.auc_ssid,(oal_uint8 *)pst_ioctl_data->pri_data.cfg80211_connect_params.auc_ssid,st_mac_conn_param.uc_ssid_len))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_connect::oal_copy_from_user fail!}\r\n");
            return -OAL_EFAIL;
        }
    }
    else
    {
        OAL_MEMZERO(st_mac_conn_param.auc_ssid, OAL_SIZEOF(st_mac_conn_param.auc_ssid));
    }


    if(pst_ioctl_data->pri_data.cfg80211_connect_params.auc_bssid)
    {
        st_mac_conn_param.auc_bssid = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,OAL_MAC_ADDR_LEN,OAL_TRUE);
        if(OAL_PTR_NULL == st_mac_conn_param.auc_bssid)
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_connect::mem alloc fail!}\r\n");
            return -OAL_EFAIL;
        }
        if(oal_copy_from_user(st_mac_conn_param.auc_bssid,(oal_uint8 *)pst_ioctl_data->pri_data.cfg80211_connect_params.puc_bssid,OAL_MAC_ADDR_LEN))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_connect::oal_copy_from_user fail!}\r\n");
            return -OAL_EFAIL;
        }
    }
    else
    {
        OAL_MEMZERO(st_mac_conn_param.auc_bssid, OAL_MAC_ADDR_LEN);
    }
    /* �����ں��·��İ�ȫ��ز��� */
    /* ��֤���� */
    st_mac_conn_param.en_auth_type  = pst_ioctl_data->pri_data.cfg80211_connect_params.en_auth_type;

    /* �������� */
    st_mac_conn_param.en_privacy    = pst_ioctl_data->pri_data.cfg80211_connect_params.en_privacy;

    /* IE�·� */
    st_mac_conn_param.ul_ie_len     = (oal_uint32)pst_ioctl_data->pri_data.cfg80211_connect_params.ie_len;
    if(st_mac_conn_param.ul_ie_len)
    {
        st_mac_conn_param.puc_ie = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,(oal_uint16)st_mac_conn_param.ul_ie_len,OAL_TRUE);
        if(OAL_PTR_NULL == st_mac_conn_param.puc_ie)
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_connect::mem alloc fail!}\r\n");
            return -OAL_EFAIL;
        }
        if(oal_copy_from_user(st_mac_conn_param.puc_ie,pst_ioctl_data->pri_data.cfg80211_connect_params.puc_ie,st_mac_conn_param.ul_ie_len))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_connect::oal_copy_from_user fail!}\r\n");
            OAL_MEM_FREE(st_mac_conn_param.puc_ie,OAL_TRUE);
            return -OAL_EFAIL;
        }
    }
    else
    {
        st_mac_conn_param.puc_ie = OAL_PTR_NULL;
    }

    /* ���ü��ܲ��� */
    if (0 != pst_ioctl_data->pri_data.cfg80211_connect_params.en_privacy)
    {
        if (0 != pst_ioctl_data->pri_data.cfg80211_connect_params.uc_wep_key_len)
        {
            /* ����wep������Ϣ */
            st_mac_conn_param.uc_wep_key_len           = pst_ioctl_data->pri_data.cfg80211_connect_params.uc_wep_key_len;
            st_mac_conn_param.puc_wep_key = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,st_mac_conn_param.uc_wep_key_len,OAL_TRUE);
            if(OAL_PTR_NULL == st_mac_conn_param.puc_wep_key)
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_connect::mem alloc fail!}\r\n");
                OAL_MEM_FREE(st_mac_conn_param.puc_ie,OAL_TRUE);
                return -OAL_EFAIL;
            }
            if(oal_copy_from_user((oal_uint8*)st_mac_conn_param.puc_wep_key,pst_ioctl_data->pri_data.cfg80211_connect_params.puc_wep_key,st_mac_conn_param.uc_wep_key_len))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_connect::oal_copy_from_user fail!}\r\n");
                OAL_MEM_FREE(st_mac_conn_param.puc_ie,OAL_TRUE);
                OAL_MEM_FREE((oal_uint8*)st_mac_conn_param.puc_wep_key,OAL_TRUE);
                return -OAL_EFAIL;
            }

            st_mac_conn_param.uc_wep_key_index         = pst_ioctl_data->pri_data.cfg80211_connect_params.uc_wep_key_index;
        }

        if (0 != pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.n_akm_suites)
        {
            /* ����WPA/WPA2 ������Ϣ������������ֻ��һ���׼� */
            st_mac_conn_param.st_crypto.ul_wpa_versions    = pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.wpa_versions;
            st_mac_conn_param.st_crypto.ul_group_suite     = MAC_RSN_CHIPER_OUI((oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.cipher_group);
            st_mac_conn_param.st_crypto.aul_pair_suite[0]  = MAC_RSN_CHIPER_OUI((oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.ciphers_pairwise[0]);
            st_mac_conn_param.st_crypto.aul_akm_suite[0]   = MAC_RSN_CHIPER_OUI((oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.akm_suites[0]);

            if (WITP_WPA_VERSION_1 == st_mac_conn_param.st_crypto.ul_wpa_versions)
            {
                st_mac_conn_param.st_crypto.ul_group_suite    = MAC_WPA_CHIPER_OUI((oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.cipher_group);
                st_mac_conn_param.st_crypto.aul_pair_suite[0] = MAC_WPA_CHIPER_OUI((oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.ciphers_pairwise[0]);
                st_mac_conn_param.st_crypto.aul_akm_suite[0]  = MAC_WPA_CHIPER_OUI((oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.akm_suites[0]);
            }
        }

        OAM_WARNING_LOG3(0, OAM_SF_WPA, "wal_ioctl_nl80211_priv_connect::get_rsn_cipher group_suite[0]:0x%x pair_suite[0]:0x%x akm_suite[0]:0x%x",
                     st_mac_conn_param.st_crypto.ul_group_suite, st_mac_conn_param.st_crypto.aul_pair_suite[0], st_mac_conn_param.st_crypto.aul_akm_suite[0]);
    }


    /* ���¼����������������� */
    l_ret = wal_cfg80211_start_connect_etc(pst_net_dev, &st_mac_conn_param);
    if( OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_connect::wal_cfg80211_start_connect_etc fail!}\r\n");
        l_ret = -OAL_EFAIL;
    }

    if ((0 != pst_ioctl_data->pri_data.cfg80211_connect_params.en_privacy)\
        &&(0 != pst_ioctl_data->pri_data.cfg80211_connect_params.uc_wep_key_len))
    {
        OAL_MEM_FREE((oal_uint8*)st_mac_conn_param.puc_wep_key,OAL_TRUE);
    }
    OAL_MEM_FREE(st_mac_conn_param.puc_ie,OAL_TRUE);

    return l_ret;
}


oal_int32  wal_ioctl_nl80211_priv_disconnect(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    mac_cfg_kick_user_param_stru    st_mac_cfg_kick_user_param;
    oal_int32                       l_ret;
    mac_user_stru                   *pst_mac_user;
    mac_vap_stru                    *pst_mac_vap;


    /* �����ں��·���connect���� */
    oal_memset(&st_mac_cfg_kick_user_param, 0, OAL_SIZEOF(mac_cfg_kick_user_param_stru));

    /* �����ں��·���ȥ����ԭ��  */
    st_mac_cfg_kick_user_param.us_reason_code = pst_ioctl_data->pri_data.kick_user_params.us_reason_code;

    /* ��д��sta������ap mac ��ַ*/
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_disconnect::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}\r\n");
        return -OAL_EINVAL;
    }
    pst_mac_user = mac_res_get_mac_user_etc(pst_mac_vap->us_assoc_vap_id);
    if (OAL_PTR_NULL == pst_mac_user)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_disconnect:: mac_res_get_mac_user_etc pst_mac_user  is nul!}\r\n");
        return OAL_SUCC;
    }

    oal_memcopy(st_mac_cfg_kick_user_param.auc_mac_addr, pst_mac_user->auc_user_mac_addr, WLAN_MAC_ADDR_LEN);

    l_ret = wal_cfg80211_start_disconnect_etc(pst_net_dev, &st_mac_cfg_kick_user_param);
    if( OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_disconnect::wal_cfg80211_start_disconnect_etc fail!}\r\n");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


oal_uint32 wal_get_sta_info(oal_net_sta_link_info_stru *pst_all_sta_link_info,oam_user_stat_info_stru *pst_oam_user_stat,mac_user_stru *pst_mac_user)
{
#define TID_STAT_TO_USER(_stat) (((_stat)[0])+((_stat)[1])+((_stat)[2])+((_stat)[3])+((_stat)[4])+((_stat)[5])+((_stat)[6])+((_stat)[7]))


    hmac_user_stru   *pst_hmac_user_tmp;
    oal_int8          c_rssi_temp;

    if ((OAL_PTR_NULL == pst_all_sta_link_info) ||
        (OAL_PTR_NULL == pst_oam_user_stat) ||
        (OAL_PTR_NULL == pst_mac_user))
    {
        OAM_WARNING_LOG3(0, OAM_SF_CFG, "{wal_get_sta_info::NULL pointer!pst_all_sta_link_info:%p,pst_oam_user_stat:%p,pst_mac_user:%p}",
                         pst_all_sta_link_info,pst_oam_user_stat,pst_mac_user);
        return OAL_FAIL;
    }
    pst_hmac_user_tmp = mac_res_get_hmac_user_etc(pst_mac_user->us_assoc_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hmac_user_tmp))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_get_sta_info::pst_hmac_user_tmp is null ptr.}\r\n");
        return OAL_EINVAL;
    }
    oal_memcopy(pst_all_sta_link_info->addr, pst_mac_user->auc_user_mac_addr, WLAN_MAC_ADDR_LEN);

    if ((pst_hmac_user_tmp->c_rssi > OAL_RSSI_SIGNAL_MAX) || (pst_hmac_user_tmp->c_rssi < OAL_RSSI_SIGNAL_MIN))
    {
        c_rssi_temp = (oal_int8)HMAC_RSSI_SIGNAL_INVALID;
    }
    else
    {
        c_rssi_temp = pst_hmac_user_tmp->c_rssi + HMAC_FBT_RSSI_ADJUST_VALUE;
        if (c_rssi_temp < 0)
        {
            c_rssi_temp = 0;
        }
        if (c_rssi_temp > HMAC_FBT_RSSI_MAX_VALUE)
        {
            c_rssi_temp = HMAC_FBT_RSSI_MAX_VALUE;
        }
    }
    pst_all_sta_link_info->rx_rssi        = (oal_uint8)c_rssi_temp;
    pst_all_sta_link_info->tx_pwr         = 0;                                         /* ��ʱ����ʵ�� */
    pst_all_sta_link_info->rx_rate        = pst_hmac_user_tmp->ul_rx_rate;
    pst_all_sta_link_info->tx_rate        = pst_hmac_user_tmp->ul_tx_rate;
    pst_all_sta_link_info->rx_minrate     = pst_hmac_user_tmp->ul_rx_rate_min;
    pst_all_sta_link_info->rx_maxrate     = pst_hmac_user_tmp->ul_rx_rate_max;
    pst_all_sta_link_info->rx_bytes       = pst_oam_user_stat->ul_rx_mpdu_bytes;
    pst_all_sta_link_info->tx_bytes       = TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_bytes)+TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_ampdu_bytes);
    pst_all_sta_link_info->tx_frames_rty  = pst_oam_user_stat->ul_tx_ppdu_retries;/* TX retries */
    pst_all_sta_link_info->tx_frames_all  = TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_succ_num)+TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_in_ampdu);
    pst_all_sta_link_info->tx_frames_fail = TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_fail_num)+TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_fail_in_ampdu);
    pst_all_sta_link_info->SNR            = 0;                                             /* ��ʱ����ʵ�� */
    pst_all_sta_link_info->tx_minrate     = pst_hmac_user_tmp->ul_tx_rate_min;
    pst_all_sta_link_info->tx_maxrate     = pst_hmac_user_tmp->ul_tx_rate_max;

#if 0
    OAL_IO_PRINT("sta_info]mac: %02x:%02x:%02x:%02x:%02x:%02x\n",pst_all_sta_link_info->addr[0],pst_all_sta_link_info->addr[1],
                pst_all_sta_link_info->addr[2],pst_all_sta_link_info->addr[3],pst_all_sta_link_info->addr[4],pst_all_sta_link_info->addr[5]);
    OAL_IO_PRINT("[sta_info]rx_rssi:%d,tx_pwr:%d,rx_rate:%d,tx_rate:%d,rx_minrate:%d,rx_maxrate:%d,rx_bytes:%lld,tx_bytes:%lld,tx_frames_rty:%d,tx_frames_all:%d,tx_frames_fail:%d,SNR:%d,tx_minrate:%d,tx_maxrate:%d\n",
                pst_all_sta_link_info->rx_rssi,
                pst_all_sta_link_info->tx_pwr,
                pst_all_sta_link_info->rx_rate,
                pst_all_sta_link_info->tx_rate,
                pst_all_sta_link_info->rx_minrate,
                pst_all_sta_link_info->rx_maxrate,
                pst_all_sta_link_info->rx_bytes,
                pst_all_sta_link_info->tx_bytes,
                pst_all_sta_link_info->tx_frames_rty,
                pst_all_sta_link_info->tx_frames_all,
                pst_all_sta_link_info->tx_frames_fail,
                pst_all_sta_link_info->SNR,
                pst_all_sta_link_info->tx_minrate,
                pst_all_sta_link_info->tx_maxrate
                );
#endif

#undef TID_STAT_TO_USER
    return OAL_SUCC;
}


oal_uint32 wal_get_sta_info_ext(oal_net_sta_link_info_ext_stru *pst_all_sta_link_info_ext,oam_user_stat_info_stru *pst_oam_user_stat,mac_user_stru *pst_mac_user)
{
    hmac_user_stru  *pst_hmac_user_tmp;
    oal_uint32                       ul_curr_time;

    if ((OAL_PTR_NULL == pst_all_sta_link_info_ext) ||
        (OAL_PTR_NULL == pst_oam_user_stat) ||
        (OAL_PTR_NULL == pst_mac_user))
    {
        OAM_WARNING_LOG3(0, OAM_SF_CFG, "{wal_get_sta_info_ext::NULL pointer!pst_all_sta_link_info:%p,pst_oam_user_stat:%p,pst_mac_user:%p}",
                         pst_all_sta_link_info_ext,pst_oam_user_stat,pst_mac_user);
        return OAL_FAIL;
    }
    pst_hmac_user_tmp = mac_res_get_hmac_user_etc(pst_mac_user->us_assoc_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hmac_user_tmp))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_get_sta_info_ext::pst_hmac_user_tmp is null ptr.}\r\n");
        return OAL_EINVAL;
    }
    ul_curr_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    pst_all_sta_link_info_ext->uc_auth_st = pst_hmac_user_tmp->st_user_base_info.en_user_asoc_state;
    pst_all_sta_link_info_ext->uc_sta_num = pst_hmac_user_tmp->st_user_base_info.en_avail_num_spatial_stream + 1;
    pst_all_sta_link_info_ext->uc_work_mode = pst_hmac_user_tmp->st_user_base_info.en_avail_protocol_mode;
    pst_all_sta_link_info_ext->ul_associated_time = (oal_uint32)OAL_TIME_GET_RUNTIME(pst_hmac_user_tmp->ul_first_add_time, ul_curr_time)/1000;
    pst_all_sta_link_info_ext->uc_ps_st = pst_hmac_user_tmp->uc_ps_st;
    pst_all_sta_link_info_ext->en_wmm_switch = pst_hmac_user_tmp->en_wmm_switch;
    pst_all_sta_link_info_ext->c_noise = pst_hmac_user_tmp->c_free_power;
#if 0
    OAL_IO_PRINT("[Check]auth_st:%d,sta_num:%d,work_mode:%d,associated_time:%d,ps_st=%d,wmm_switch:%d,noise=%d\n",
            pst_all_sta_link_info_ext->uc_auth_st,
            pst_all_sta_link_info_ext->uc_sta_num,
            pst_all_sta_link_info_ext->uc_work_mode,
            pst_all_sta_link_info_ext->ul_associated_time,
            pst_all_sta_link_info_ext->uc_ps_st,
            pst_all_sta_link_info_ext->en_wmm_switch,
            pst_all_sta_link_info_ext->c_noise
            );
#endif
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_CAR

oal_int32 wal_ioctl_get_car_info(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    oal_car_info_stru              *pst_car_info;
    mac_device_stru                *pst_mac_dev;
    hmac_device_stru               *pst_hmac_dev;
    mac_vap_stru                   *pst_mac_vap;
    hmac_vap_stru                  *pst_hmac_vap;
    mac_user_stru                  *pst_mac_user;
    hmac_user_stru                 *pst_hmac_user;
    oal_uint8                       uc_vap_index;
    oal_uint8                       uc_vap_num = 0;
    oal_uint16                      us_user_index = 0;  // 0-128
    oal_dlist_head_stru            *pst_list_pos;
    oal_car_user_limit_stru        *pst_car_user_cfg_start; //�ں�̬Ϊ�û���Ϣ������ڴ棬�ں�����β��Ҫcopy to user�û�̬
    oal_car_user_limit_stru        *pst_car_user_cfg;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CAR, "{wal_ioctl_get_car_info::OAL_NET_DEV_PRIV, return null!}");
        return OAL_EINVAL;
    }

    /* ��ȡdevice�ṹ�� */
    pst_mac_dev = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_dev)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CAR, "{wal_ioctl_get_car_info::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_dev = hmac_res_get_mac_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_hmac_dev)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CAR, "{wal_ioctl_get_car_info::pst_hmac_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    //��ʼ��ֵ
    pst_car_info = &(pst_ioctl_data->pri_data.st_car_info);

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CAR, "{wal_ioctl_get_car_info::us_user_buf_max=%d.}", pst_car_info->us_user_buf_max);
    if (WLAN_ASSOC_USER_MAX_NUM < pst_car_info->us_user_buf_max)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CAR, "{wal_ioctl_get_car_info::WLAN_ASSOC_USER_MAX_NUM=%d.}", WLAN_ASSOC_USER_MAX_NUM);
        return OAL_EINVAL;
    }

    pst_car_user_cfg_start = (oal_car_user_limit_stru *)oal_memalloc((oal_uint32)(pst_car_info->us_user_buf_max * OAL_SIZEOF(oal_car_user_limit_stru)));
    if (OAL_PTR_NULL == pst_car_user_cfg_start)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CAR, "{wal_ioctl_get_car_info::pst_car_user_cfg_start null.}");
        return -OAL_EINVAL;
    }

    //����ԭʼ��vapid
    pst_car_info->uc_ori_vap_id = pst_mac_vap->uc_vap_id;

    //device
    pst_car_info->uc_device_id = pst_mac_dev->uc_device_id;
    pst_car_info->us_car_ctl_cycle_ms = pst_hmac_dev->us_car_ctl_cycle_ms;
    pst_car_info->en_car_enable_flag = pst_hmac_dev->en_car_enable_flag;
    pst_car_info->aul_car_packet_drop_num[OAL_CAR_UPLINK] = pst_hmac_dev->ul_car_packet_drop_num[OAL_CAR_UPLINK];
    pst_car_info->aul_car_packet_drop_num[OAL_CAR_DOWNLINK] = pst_hmac_dev->ul_car_packet_drop_num[OAL_CAR_DOWNLINK];
    pst_car_info->ul_car_mcast_packet_drop_num = pst_hmac_dev->ul_car_multicast_packet_drop_num;
    pst_car_info->ul_car_mcast_packet_pps_drop_num = pst_hmac_dev->ul_car_multicast_packet_pps_drop_num;

    //device ucast
    pst_car_info->st_car_device_cfg.st_car_device_ucast_cfg[OAL_CAR_UPLINK].en_car_limit_flag = pst_hmac_dev->st_car_device_cfg[OAL_CAR_UPLINK].en_car_limit_flag;
    pst_car_info->st_car_device_cfg.st_car_device_ucast_cfg[OAL_CAR_UPLINK].ul_car_limit_kbps = pst_hmac_dev->st_car_device_cfg[OAL_CAR_UPLINK].ul_car_limit_kbps;
    pst_car_info->st_car_device_cfg.st_car_device_ucast_cfg[OAL_CAR_DOWNLINK].en_car_limit_flag = pst_hmac_dev->st_car_device_cfg[OAL_CAR_DOWNLINK].en_car_limit_flag;
    pst_car_info->st_car_device_cfg.st_car_device_ucast_cfg[OAL_CAR_DOWNLINK].ul_car_limit_kbps = pst_hmac_dev->st_car_device_cfg[OAL_CAR_DOWNLINK].ul_car_limit_kbps;

    //device mcast
    pst_car_info->st_car_device_cfg.st_car_device_mcast_cfg.en_car_limit_flag = pst_hmac_dev->st_car_device_multicast_cfg.en_car_limit_flag;
    pst_car_info->st_car_device_cfg.st_car_device_mcast_cfg.ul_car_limit_kbps = pst_hmac_dev->st_car_device_multicast_cfg.ul_car_limit_kbps;
    pst_car_info->st_car_device_cfg.ul_car_orgin_mcast_pps_num = pst_hmac_dev->st_car_device_multicast_cfg.ul_car_orgin_multicast_pps_num*1000/pst_hmac_dev->us_car_ctl_cycle_ms;

    //vap user
    /* ����device������vap */
    for (uc_vap_index = 0; uc_vap_index <pst_mac_dev->uc_vap_num; uc_vap_index++)
    {
        pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_dev->auc_vap_id[uc_vap_index]);
        if (OAL_PTR_NULL == pst_mac_vap)
        {
            OAM_ERROR_LOG1(0, OAM_SF_CAR, "{wal_ioctl_get_car_info::pst_vap null,vap_id=%d.}", pst_mac_dev->auc_vap_id[uc_vap_index]);
            continue;
        }

        /* VAPģʽ�ж�, ֻ��apģʽ�²���CAR, ����ģʽ�£�����ʾCAR��Ϣ */
        if (WLAN_VAP_MODE_BSS_AP != pst_mac_vap->en_vap_mode)
        {
             OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CAR, "{wal_ioctl_get_car_info:: CAR only used in AP mode; en_vap_mode=%d.}", pst_mac_vap->en_vap_mode);
             continue;
        }

        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
        if (OAL_PTR_NULL == pst_hmac_vap)
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CAR, "{wal_ioctl_get_car_info::pst_hmac_vap null}");
            continue;
        }

        //��дvap��Ϣ
        pst_car_info->ast_car_vap_cfg[uc_vap_num].uc_vap_id = pst_mac_vap->uc_vap_id;
        pst_car_info->ast_car_vap_cfg[uc_vap_num].ast_vap_car_cfg[OAL_CAR_UPLINK].en_car_limit_flag = pst_hmac_vap->st_car_vap_cfg[OAL_CAR_UPLINK].en_car_limit_flag;
        pst_car_info->ast_car_vap_cfg[uc_vap_num].ast_vap_car_cfg[OAL_CAR_UPLINK].ul_car_limit_kbps = pst_hmac_vap->st_car_vap_cfg[OAL_CAR_UPLINK].ul_car_limit_kbps;
        pst_car_info->ast_car_vap_cfg[uc_vap_num].ast_vap_car_cfg[OAL_CAR_DOWNLINK].en_car_limit_flag = pst_hmac_vap->st_car_vap_cfg[OAL_CAR_DOWNLINK].en_car_limit_flag;
        pst_car_info->ast_car_vap_cfg[uc_vap_num].ast_vap_car_cfg[OAL_CAR_DOWNLINK].ul_car_limit_kbps = pst_hmac_vap->st_car_vap_cfg[OAL_CAR_DOWNLINK].ul_car_limit_kbps;

        uc_vap_num++;

        /* ����vap������user */
        WAL_VAP_FOREACH_USER(pst_mac_user, &pst_hmac_vap->st_vap_base_info, pst_list_pos)
        {
            /* ����û���δ�����ɹ����򲻷���˽����Ϣ�ṹ�� */
            if (MAC_USER_STATE_ASSOC != pst_mac_user->en_user_asoc_state)
            {
                continue;
            }
            //��ȡhmac user
            pst_hmac_user = mac_res_get_hmac_user_etc(pst_mac_user->us_assoc_id);
            if (OAL_PTR_NULL == pst_hmac_user)
            {
                OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CAR, "{wal_ioctl_get_car_info::pst_hmac_user null}");
                continue;
            }

            //���user�����������ϲ㴫������user��������return
            if (us_user_index >= pst_car_info->us_user_buf_max)
            {
                break;
            }

            //��ȡ��һ��user���ڴ�
            pst_car_user_cfg = (oal_car_user_limit_stru *)(pst_car_user_cfg_start + us_user_index);

            //��дuser��Ϣ
            pst_car_user_cfg->us_assoc_id = pst_mac_user->us_assoc_id;
            oal_memcopy(pst_car_user_cfg->auc_user_mac_addr, pst_mac_user->auc_user_mac_addr, OAL_MAC_ADDR_LEN);
            pst_car_user_cfg->ast_user_car_cfg[OAL_CAR_UPLINK].en_car_limit_flag = pst_hmac_user->st_car_user_cfg[OAL_CAR_UPLINK].en_car_limit_flag;
            pst_car_user_cfg->ast_user_car_cfg[OAL_CAR_UPLINK].ul_car_limit_kbps = pst_hmac_user->st_car_user_cfg[OAL_CAR_UPLINK].ul_car_limit_kbps;
            pst_car_user_cfg->ast_user_car_cfg[OAL_CAR_DOWNLINK].en_car_limit_flag = pst_hmac_user->st_car_user_cfg[OAL_CAR_DOWNLINK].en_car_limit_flag;
            pst_car_user_cfg->ast_user_car_cfg[OAL_CAR_DOWNLINK].ul_car_limit_kbps = pst_hmac_user->st_car_user_cfg[OAL_CAR_DOWNLINK].ul_car_limit_kbps;

            pst_car_user_cfg->uc_vap_id = pst_mac_vap->uc_vap_id;

            us_user_index++;

        }
    }

    pst_car_info->us_user_cnt = us_user_index;

    //���û���Ϣ���ں�̬�������û�̬
    if(OAL_PTR_NULL != pst_car_info->pst_car_user_cfg)
    {
        oal_copy_to_user(pst_car_info->pst_car_user_cfg, pst_car_user_cfg_start, ((oal_uint32)(pst_car_info->us_user_buf_max * OAL_SIZEOF(oal_car_user_limit_stru))));
    }

    oal_free(pst_car_user_cfg_start);

    return OAL_SUCC;
}
#endif

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)

oal_int32 wal_ioctl_get_blkwhtlst(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    mac_vap_stru                        *pst_mac_vap;
    hmac_vap_stru                       *pst_hmac_vap;
    mac_blacklist_info_stru             *pst_blacklist_info;
    mac_blacklist_stru                  *pst_blacklist;
    oal_uint8                            uc_blacklist_index;
    oal_blkwhtlst_stru                  *pst_blkwhtlst;


    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_blkwhtlst::pst_mac_vap is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_blkwhtlst: hmac vap is null. vap id:%d", pst_mac_vap->uc_vap_id);
        return -OAL_EINVAL;
    }

    pst_blacklist_info = pst_hmac_vap->pst_blacklist_info;
    if (OAL_PTR_NULL == pst_blacklist_info)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_ioctl_get_blkwhtlst::pst_blacklist_info null.}");
        return -OAL_EINVAL;
    }

    pst_blkwhtlst = &(pst_ioctl_data->pri_data.st_blkwhtlst);
    pst_blkwhtlst->uc_blkwhtlst_cnt = 0;
    pst_blkwhtlst->uc_mode = 0;
    if((CS_BLACKLIST_MODE_BLACK == pst_blacklist_info->uc_mode)
        || (CS_BLACKLIST_MODE_WHITE == pst_blacklist_info->uc_mode))
    {
        pst_blkwhtlst->uc_mode = pst_blacklist_info->uc_mode;
        pst_blkwhtlst->uc_blkwhtlst_cnt = pst_blacklist_info->uc_list_num;
    }
    else
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_get_blkwhtlst::BLACKLIST not enable! num=%d}",pst_blacklist_info->uc_list_num);
        return OAL_SUCC;
    }

    for (uc_blacklist_index = 0; uc_blacklist_index < pst_blacklist_info->uc_list_num; uc_blacklist_index++)
    {
        pst_blacklist = &pst_blacklist_info->ast_black_list[uc_blacklist_index];
        oal_memcopy(pst_blkwhtlst->auc_blkwhtlst_mac[uc_blacklist_index], pst_blacklist->auc_mac_addr, OAL_MAC_ADDR_LEN);
        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_ioctl_get_blkwhtlst::src mac=%02X:XX:XX:%02X:%02X:%02X}",pst_blacklist->auc_mac_addr[0],pst_blacklist->auc_mac_addr[3],pst_blacklist->auc_mac_addr[4],pst_blacklist->auc_mac_addr[5]);
    }
    return OAL_SUCC;
}
#endif


oal_int32 wal_ioctl_get_all_sta_info(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    mac_vap_stru                        *pst_mac_vap;
    hmac_vap_stru                       *pst_hmac_vap;
    oal_dlist_head_stru                 *pst_entry;
    oal_dlist_head_stru                 *pst_dlist_tmp;
    mac_user_stru                       *pst_user_tmp;
    oal_uint16                           us_sta_cnt = 0;
    oam_stat_info_stru                  *pst_oam_stat;
    oam_user_stat_info_stru             *pst_oam_user_stat;
    wal_msg_write_stru                   st_write_msg;
    oal_int32                            l_ret;
    mac_cfg_query_rssi_stru             *pst_query_rssi_param;
    mac_cfg_query_rate_stru             *pst_query_rate_param;
    mac_cfg_query_psst_stru             *pst_query_psst_param;
    oal_net_sta_link_info_stru          *pst_all_sta_link_info_start;
    oal_net_sta_link_info_ext_stru      *pst_all_sta_link_info_ext_start;
    oal_net_all_sta_link_info_ext_stru  *pst_all_sta_link_info_ext_data;
    oal_uint32                           ul_ret;
    oal_uint16                           us_user_num;
    oal_uint32                           ul_alloc_cnt;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_all_sta_info::pst_mac_vap is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_all_sta_info: hmac vap is null. vap id:%d", pst_mac_vap->uc_vap_id);
        return -OAL_EINVAL;
    }

    pst_all_sta_link_info_ext_data = &(pst_ioctl_data->pri_data.all_sta_link_info_ext);
    pst_all_sta_link_info_ext_data->sta_cnt     = 0;
    pst_all_sta_link_info_ext_data->cur_channel = pst_mac_vap->st_channel.uc_chan_number;
    if (0 == pst_all_sta_link_info_ext_data->buf_cnt)
    {
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HILINK, "{wal_ioctl_get_all_sta_info:: buf_cnt is [0].}");
        return OAL_SUCC;
    }

    /* ��ȡ��vap�µ��ѹ���sta��Ŀ�����λ�ȡÿ��sta��ͳ����Ϣ��copy����ʱ�������� */
    us_user_num = pst_mac_vap->us_user_nums;
    if (0 == us_user_num)
    {
        return OAL_SUCC;
    }
    //���������ı���vap���û������ǲ���ȫ�ģ����Դ�ʱ������ݵ�ǰ��¼���û���������Դ���������Ĳ��������
    //�������������ϲ㴫�����Ĳ��������ڴ���Դ
    ul_alloc_cnt = pst_all_sta_link_info_ext_data->buf_cnt;
    if(ul_alloc_cnt > mac_mib_get_MaxAssocUserNums(pst_mac_vap))
    {
        ul_alloc_cnt = mac_mib_get_MaxAssocUserNums(pst_mac_vap);
    }

    pst_all_sta_link_info_start = (oal_net_sta_link_info_stru *)oal_memalloc((oal_uint32)(ul_alloc_cnt * OAL_SIZEOF(oal_net_sta_link_info_stru)));
    if (OAL_PTR_NULL == pst_all_sta_link_info_start)
    {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HILINK, "{wal_ioctl_get_all_sta_info:: OAL_MALLOC failed sta num [%d].}",ul_alloc_cnt );
        return -OAL_EINVAL;
    }
    pst_all_sta_link_info_ext_start = (oal_net_sta_link_info_ext_stru *)oal_memalloc((oal_uint32)(ul_alloc_cnt * OAL_SIZEOF(oal_net_sta_link_info_ext_stru)));
    if (OAL_PTR_NULL == pst_all_sta_link_info_ext_start)
    {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HILINK, "{wal_ioctl_get_all_sta_info:: OAL_MALLOC failed sta num [%d].}",ul_alloc_cnt );
        oal_free(pst_all_sta_link_info_start);
        return OAL_FAIL;
    }

    /* step1. ͬ��Ҫ��ѯ��dmac��Ϣ */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
        if (OAL_PTR_NULL == pst_dlist_tmp)
        {
            /* ��forѭ���̻߳���ͣ���ڼ����ɾ���û��¼��������pst_dlist_tmpΪ�ա�Ϊ��ʱֱ��������ȡdmac��Ϣ */
            break;
        }

        /***********************************************************************/
        /*                  ��ȡdmac user��RSSI��Ϣ                            */
        /***********************************************************************/
        OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_RSSI, OAL_SIZEOF(mac_cfg_query_rssi_stru));
        pst_query_rssi_param = (mac_cfg_query_rssi_stru *)st_write_msg.auc_value;
        pst_query_rssi_param->us_user_id = pst_user_tmp->us_assoc_id; /* ���û���id����ȥ */

        pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;
        l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_query_rssi_stru),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_all_sta_info: send query rssi cfg event ret:%d", l_ret);
        }

        /*lint -e730*/
        l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT((pst_hmac_vap->query_wait_q), (OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), (5*OAL_TIME_HZ));
        /*lint +e730*/
        if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_all_sta_info: query rssi timeout. ret:%d", l_ret);
        }

        /***********************************************************************/
        /*                  ��ȡdmac user��������Ϣ                            */
        /***********************************************************************/
        OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_RATE, OAL_SIZEOF(mac_cfg_query_rate_stru));
        pst_query_rate_param = (mac_cfg_query_rate_stru *)st_write_msg.auc_value;

        pst_query_rate_param->us_user_id = pst_user_tmp->us_assoc_id; /* ���û���id����ȥ */
        pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;
        l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_query_rate_stru),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_all_sta_info: send query rate cfg event ret:%d", l_ret);
        }

        /*lint -e730*/
        l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT((pst_hmac_vap->query_wait_q), (OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), (5*OAL_TIME_HZ));
        /*lint +e730*/
        if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_all_sta_info: query rate timeout. ret:%d", l_ret);
        }

        /***********************************************************************/
        /*                  ��ȡdmac user��ʡ����״̬��Ϣ��Ϣ                            */
        /***********************************************************************/
        OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_PSST, OAL_SIZEOF(mac_cfg_query_psst_stru));
        pst_query_psst_param = (mac_cfg_query_psst_stru *)st_write_msg.auc_value;

        pst_query_psst_param->us_user_id = pst_user_tmp->us_assoc_id; /* ���û���id����ȥ */

        pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;
        l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_query_psst_stru),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_all_sta_info: send query rate cfg event ret:%d", l_ret);
        }

        /*lint -e730*/
        l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT((pst_hmac_vap->query_wait_q), (OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), (5*OAL_TIME_HZ));
        /*lint +e730*/
        if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_all_sta_info: query rate timeout. ret:%d", l_ret);
        }

    }

    /* step2. ��������û���Ϣ */
    pst_oam_stat = OAM_STAT_GET_STAT_ALL();

    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
    {
        oal_net_sta_link_info_stru          *pst_all_sta_link_info = pst_all_sta_link_info_start + us_sta_cnt;
        oal_net_sta_link_info_ext_stru      *pst_all_sta_link_info_ext = pst_all_sta_link_info_ext_start + us_sta_cnt;

        pst_user_tmp = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
        if(pst_user_tmp->us_assoc_id >= WLAN_USER_MAX_USER_LIMIT)
        {
            continue;
        }
        pst_oam_user_stat = &(pst_oam_stat->ast_user_stat_info[pst_user_tmp->us_assoc_id]);
        ul_ret = wal_get_sta_info(pst_all_sta_link_info, pst_oam_user_stat, pst_user_tmp);
        if (OAL_SUCC != ul_ret)
        {
            break;
        }
        pst_all_sta_link_info_ext->en_band = pst_mac_vap->st_channel.en_band;
#if 0
        if(OAL_PTR_NULL != pst_all_sta_link_info_ext)
        {
            OAL_IO_PRINT("[Check]band=%d\n",pst_all_sta_link_info_ext->en_band);
        }
#endif
        ul_ret = wal_get_sta_info_ext(pst_all_sta_link_info_ext, pst_oam_user_stat, pst_user_tmp);
        if (OAL_SUCC != ul_ret)
        {
            break;
        }
        OAM_WARNING_LOG4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HILINK, "{wal_ioctl_get_all_sta_info:: get all sta info:us_sta_index[%d],rssi [%d], mac[%x%x]}",
        us_sta_cnt, pst_all_sta_link_info->rx_rssi, pst_all_sta_link_info->addr[4],pst_all_sta_link_info->addr[5]);

        us_sta_cnt++;
        if(us_sta_cnt >= ul_alloc_cnt)
        {
            break;
        }

        if (OAL_PTR_NULL == pst_dlist_tmp)
        {
            break;
        }
    }

    /* ����ȡͳ����Ϣ���뵽ioctl�����У��������û��ռ䣬
       ���û��������뵽pri_data.all_sta_link_info.sta_cnt�� */
    if(OAL_PTR_NULL != pst_all_sta_link_info_ext_data->sta_info)
    {
        oal_copy_to_user(pst_all_sta_link_info_ext_data->sta_info, pst_all_sta_link_info_start, (OAL_SIZEOF(oal_net_sta_link_info_stru)*us_sta_cnt));
    }

    if(OAL_PTR_NULL != pst_all_sta_link_info_ext_data->sta_info_ext)
    {
        oal_copy_to_user(pst_all_sta_link_info_ext_data->sta_info_ext, pst_all_sta_link_info_ext_start, (OAL_SIZEOF(oal_net_sta_link_info_ext_stru)*us_sta_cnt));
    }

    pst_all_sta_link_info_ext_data->sta_cnt     = us_sta_cnt;
    pst_all_sta_link_info_ext_data->cur_channel = pst_mac_vap->st_channel.uc_chan_number;
    OAM_WARNING_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HILINK, "{wal_ioctl_get_all_sta_info:: cur_channel[%d],sta_num [%d]}\n",
                                        pst_mac_vap->st_channel.uc_chan_number,pst_all_sta_link_info_ext_data->sta_cnt);
    oal_free(pst_all_sta_link_info_start);
    oal_free(pst_all_sta_link_info_ext_start);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_HILINK

oal_int32 wal_ioctl_start_fbt_scan(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    wal_msg_write_stru              st_write_msg;
    oal_hilink_scan_params         *pst_mac_cfg_fbt_scan_params;
    mac_vap_stru                   *pst_mac_vap;
    oal_uint16                      us_len;
    oal_int32                       l_ret;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_start_fbt_scan::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr!}\r\n");
        return -OAL_EINVAL;
    }

    /* ��ȡioctl���ò��� */
    us_len = OAL_SIZEOF(oal_hilink_scan_params);
    pst_mac_cfg_fbt_scan_params = (oal_hilink_scan_params *)(st_write_msg.auc_value);
    oal_memcopy(pst_mac_cfg_fbt_scan_params, &(pst_ioctl_data->pri_data.fbt_scan_params), us_len);

    OAM_WARNING_LOG4(0, OAM_SF_CFG, "{wal_ioctl_start_fbt_scan::en_is_on [%d],ul_channel [%d], mac[%02x:%02x]}\r\n",
                    pst_mac_cfg_fbt_scan_params->en_is_on,
                    pst_mac_cfg_fbt_scan_params->ul_channel,
                    pst_mac_cfg_fbt_scan_params->auc_mac[4],
                    pst_mac_cfg_fbt_scan_params->auc_mac[5]);
    /***************************************************************************
                              ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_FBT_START_SCAN, us_len);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                             (oal_uint8 *)&st_write_msg,
                             OAL_FALSE,
                             OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_ioctl_start_fbt_scan::return err code [%d]!}\r\n", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}


oal_int32 wal_ioctl_get_sta_11k_ability(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    oal_int32                       l_ret;
    oal_uint32                      ul_err_code;
    wal_msg_write_stru              st_write_msg;
    wal_msg_stru                   *pst_rsp_msg;
    oal_hilink_get_sta_11k_ability *pst_get_sta_11k_ability;

    pst_get_sta_11k_ability = (oal_hilink_get_sta_11k_ability*)(st_write_msg.auc_value);
    oal_memcopy(pst_get_sta_11k_ability,&(pst_ioctl_data->pri_data.fbt_get_sta_11k_ability),OAL_SIZEOF(oal_hilink_get_sta_11k_ability));
    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, (oal_uint16)WLAN_CFGID_FBT_GET_STA_11K_ABILITY, OAL_SIZEOF(oal_hilink_get_sta_11k_ability));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               (oal_uint8)WAL_MSG_TYPE_WRITE,
                               (oal_uint16)(WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_hilink_get_sta_11k_ability)),
                               (oal_uint8 *)&st_write_msg,
                               (oal_uint8)OAL_TRUE,
                               &pst_rsp_msg);
    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_get_sta_11k_ability:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if (OAL_SUCC != ul_err_code)
    {
        OAM_ERROR_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_get_sta_11k_ability::wal_send_cfg_event_etc return err code:[%x]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


oal_int32 wal_ioctl_get_sta_11v_ability(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    oal_int32                       l_ret;
    oal_uint32                      ul_err_code;
    wal_msg_write_stru              st_write_msg;
    wal_msg_stru                   *pst_rsp_msg;
    oal_hilink_get_sta_11v_ability *pst_get_sta_11v_ability;

    pst_get_sta_11v_ability = (oal_hilink_get_sta_11v_ability*)(st_write_msg.auc_value);
    oal_memcopy(pst_get_sta_11v_ability,&(pst_ioctl_data->pri_data.fbt_get_sta_11v_ability),OAL_SIZEOF(oal_hilink_get_sta_11v_ability));
    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, (oal_uint16)WLAN_CFGID_FBT_GET_STA_11V_ABILITY, OAL_SIZEOF(oal_hilink_get_sta_11v_ability));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               (oal_uint8)WAL_MSG_TYPE_WRITE,
                               (oal_uint16)(WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_hilink_get_sta_11v_ability)),
                               (oal_uint8 *)&st_write_msg,
                               (oal_uint8)OAL_TRUE,
                               &pst_rsp_msg);
    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_get_sta_11v_ability:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if (OAL_SUCC != ul_err_code)
    {
        OAM_ERROR_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_get_sta_11v_ability::wal_send_cfg_event_etc return err code:[%x]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}



oal_int32 wal_ioctl_change_to_other_ap(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    oal_int32                           l_ret;
    oal_uint32                          ul_err_code;
    wal_msg_write_stru                  st_write_msg;
    wal_msg_stru                       *pst_rsp_msg;
    oal_hilink_change_sta_to_target_ap *pst_change_sta_to_target_ap;

    pst_change_sta_to_target_ap = (oal_hilink_change_sta_to_target_ap*)(st_write_msg.auc_value);
    oal_memcopy(pst_change_sta_to_target_ap,&(pst_ioctl_data->pri_data.fbt_change_sta_to_target_ap),OAL_SIZEOF(oal_hilink_change_sta_to_target_ap));
    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, (oal_uint16)WLAN_CFGID_FBT_CHANGE_STA_TO_TARGET_AP, OAL_SIZEOF(oal_hilink_change_sta_to_target_ap));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               (oal_uint8)WAL_MSG_TYPE_WRITE,
                               (oal_uint16)(WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_hilink_change_sta_to_target_ap)),
                               (oal_uint8 *)&st_write_msg,
                               (oal_uint8)OAL_TRUE,
                               &pst_rsp_msg);
    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_change_to_other_ap:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if (OAL_SUCC != ul_err_code)
    {
        OAM_ERROR_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_change_to_other_ap::wal_send_cfg_event_etc return err code:[%x]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


oal_int32 wal_ioctl_set_sta_bcn_request(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    oal_int32                           l_ret;
    oal_uint32                          ul_err_code;
    wal_msg_write_stru                  st_write_msg;
    wal_msg_stru                       *pst_rsp_msg;
    oal_hilink_neighbor_bcn_req        *pst_neighbor_bcn_req;

    pst_neighbor_bcn_req = (oal_hilink_neighbor_bcn_req*)(st_write_msg.auc_value);
    oal_memcopy(pst_neighbor_bcn_req,&(pst_ioctl_data->pri_data.fbt_11k_sta_neighbor_bcn_req),OAL_SIZEOF(oal_hilink_neighbor_bcn_req));
    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, (oal_uint16)WLAN_CFGID_FBT_NEIGHBOR_BCN_REQ, OAL_SIZEOF(oal_hilink_neighbor_bcn_req));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               (oal_uint8)WAL_MSG_TYPE_WRITE,
                               (oal_uint16)(WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_hilink_neighbor_bcn_req)),
                               (oal_uint8 *)&st_write_msg,
                               (oal_uint8)OAL_TRUE,
                               &pst_rsp_msg);
    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_set_sta_bcn_request:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if (OAL_SUCC != ul_err_code)
    {
        OAM_ERROR_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_set_sta_bcn_request::wal_send_cfg_event_etc return err code:[%x]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}



oal_int32 wal_ioctl_get_cur_channel(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    oal_int32                       l_ret;
    oal_uint32                      ul_err_code;
    wal_msg_write_stru              st_write_msg;
    wal_msg_stru                   *pst_rsp_msg;
    oal_hilink_current_channel     *pst_current_channel;

    pst_current_channel = (oal_hilink_current_channel*)(st_write_msg.auc_value);
    oal_memcopy(pst_current_channel,&(pst_ioctl_data->pri_data.fbt_get_cur_channel),OAL_SIZEOF(oal_hilink_current_channel));
    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, (oal_uint16)WLAN_CFGID_FBT_FET_CURRENT_CHANNEL, OAL_SIZEOF(oal_hilink_current_channel));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               (oal_uint8)WAL_MSG_TYPE_WRITE,
                               (oal_uint16)(WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_hilink_current_channel)),
                               (oal_uint8 *)&st_write_msg,
                               (oal_uint8)OAL_TRUE,
                               &pst_rsp_msg);
    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_get_cur_channel:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_ERROR_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_get_cur_channel::wal_send_cfg_event_etc return err code:[%x]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}



oal_int32 wal_ioctl_set_stay_time(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    oal_int32                       l_ret;
    oal_uint32                      ul_err_code;
    wal_msg_write_stru              st_write_msg;
    wal_msg_stru                   *pst_rsp_msg;
    oal_hilink_set_scan_time_param *pst_set_scan_time_param;

    pst_set_scan_time_param = (oal_hilink_set_scan_time_param*)(st_write_msg.auc_value);
    oal_memcopy(pst_set_scan_time_param,&(pst_ioctl_data->pri_data.fbt_set_scan_stay_time),OAL_SIZEOF(oal_hilink_set_scan_time_param));
    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, (oal_uint16)WLAN_CFGID_FBT_SET_SCAN_TIME_PARAM, OAL_SIZEOF(oal_hilink_set_scan_time_param));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               (oal_uint8)WAL_MSG_TYPE_WRITE,
                               (oal_uint16)(WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_hilink_set_scan_time_param)),
                               (oal_uint8 *)&st_write_msg,
                               (oal_uint8)OAL_TRUE,
                               &pst_rsp_msg);
    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_set_stay_time:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_ERROR_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_set_stay_time::wal_send_cfg_event_etc return err code:[%x]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


oal_int32  wal_ioctl_nl80211_priv_fbt_kick_user(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    mac_cfg_kick_user_param_stru    st_mac_cfg_kick_user_param;
    oal_int32                       l_ret;
    mac_vap_stru                   *pst_mac_vap;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_fbt_kick_user::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}\r\n");
        return -OAL_EINVAL;
    }

    /* ����ǹ㲥��ַ,����rej��Ϊ0��ֱ�ӷ��ز������� */
    if ((oal_is_broadcast_ether_addr(pst_ioctl_data->pri_data.fbt_kick_user_params.auc_mac))
    && (OAL_FALSE != pst_ioctl_data->pri_data.fbt_kick_user_params.uc_rej_user))
    {
        return OAL_SUCC;
    }

    /* �����ں��·���connect���� */
    oal_memset(&st_mac_cfg_kick_user_param, 0, OAL_SIZEOF(mac_cfg_kick_user_param_stru));

    st_mac_cfg_kick_user_param.us_reason_code = pst_ioctl_data->pri_data.fbt_kick_user_params.us_reason_code;
    oal_memcopy(st_mac_cfg_kick_user_param.auc_mac_addr, pst_ioctl_data->pri_data.fbt_kick_user_params.auc_mac, WLAN_MAC_ADDR_LEN);
    st_mac_cfg_kick_user_param.uc_rej_user = pst_ioctl_data->pri_data.fbt_kick_user_params.uc_rej_user;
    st_mac_cfg_kick_user_param.uc_kick_user = pst_ioctl_data->pri_data.fbt_kick_user_params.uc_kick_user;

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_fbt_kick_user::uc_rej_user[%d], uc_kick_user[%d], mac[%x:%x].}\r\n",
                                st_mac_cfg_kick_user_param.uc_rej_user,
                                st_mac_cfg_kick_user_param.uc_kick_user,
                                st_mac_cfg_kick_user_param.auc_mac_addr[4],
                                st_mac_cfg_kick_user_param.auc_mac_addr[5]);

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    st_mac_cfg_kick_user_param.uc_mlme_phase_mask = (oal_uint8)IEEE80211_MLME_PHASE_AUTH | (oal_uint8)IEEE80211_MLME_PHASE_ASSOC;
#endif
    l_ret = wal_cfg80211_fbt_kick_user(pst_net_dev, &st_mac_cfg_kick_user_param);
    if( OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_fbt_kick_user::wal_cfg80211_start_disconnect_etc fail!}\r\n");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}



oal_int32 wal_ioctl_set_white_lst_ssidhiden(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    wal_msg_write_stru              st_write_msg;
    oal_hilink_white_node_stru     *pst_white_lst;
    wal_msg_stru                   *pst_rsp_msg;
    oal_uint32                      ul_err_code;
    oal_int32                       l_ret;

    pst_white_lst = (oal_hilink_white_node_stru*)(st_write_msg.auc_value);
    OAL_MEMZERO(pst_white_lst, OAL_SIZEOF(oal_hilink_white_node_stru));
    oal_memcopy(pst_white_lst->auc_mac, pst_ioctl_data->pri_data.set_white_lst_ssidhiden_params.auc_mac, OAL_MAC_ADDR_LEN);
    pst_white_lst->en_white_list_operate = pst_ioctl_data->pri_data.set_white_lst_ssidhiden_params.en_white_list_operate;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, (oal_uint16)WLAN_CFGID_SET_WHITE_LIST_SSIDHIDEN, OAL_SIZEOF(oal_hilink_white_node_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               (oal_uint8)WAL_MSG_TYPE_WRITE,
                               (oal_uint16)(WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_hilink_white_node_stru)),
                               (oal_uint8 *)&st_write_msg,
                               (oal_uint8)OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_white_lst_ssidhiden:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_white_lst_ssidhiden::wal_send_cfg_event_etc return err code:[%x]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;

}

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT

oal_int32 wal_ioctl_set_hilink_ie(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru *pst_fbt_sta)
{
    oal_int32                             l_ret;
    oal_uint32                            ul_ret;
    oal_app_ie_stru                      *pst_okc_ie;
    oal_ieee80211req_getset_appiebuf_stru st_app_ie;
    ul_ret = oal_copy_from_user(&st_app_ie, pst_fbt_sta->param.pst_app_ie, sizeof(oal_ieee80211req_getset_appiebuf_stru));
    if (ul_ret > 0)
    {
        OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_set_hilink_ie::oal_copy_from_user p_app_ie return error code %d!}\r\n", ul_ret);
        return -OAL_EFAUL;
    }

    if ((st_app_ie.ul_app_frmtype != (oal_uint32)IEEE80211_FRAME_TYPE_AP_OKC &&
        st_app_ie.ul_app_frmtype != (oal_uint32)IEEE80211_FRAME_TYPE_STA_OKC)
        || (st_app_ie.ul_app_buflen > WLAN_WPS_IE_MAX_SIZE))
    {
        OAM_ERROR_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_set_hilink_ie::app_frmtype error %d!}\r\n", st_app_ie.ul_app_frmtype);
        return -OAL_EINVAL;
    }

    pst_okc_ie = oal_memalloc(sizeof(oal_app_ie_stru));
    if (pst_okc_ie == OAL_PTR_NULL)
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{wal_ioctl_set_hilink_ie::oal_memalloc error!}\r\n");
        return -OAL_ENOMEM;
    }

    ul_ret = oal_copy_from_user(pst_okc_ie->auc_ie, st_app_ie.puc_app_buf, st_app_ie.ul_app_buflen);
    if (ul_ret > 0)
    {
        oal_free(pst_okc_ie);
        OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_set_hilink_ie::oal_copy_from_user app_ie return error code %d!}\r\n", ul_ret);
        return -OAL_EFAUL;
    }
    pst_okc_ie->en_app_ie_type = OAL_APP_OKC_PROBE_IE;
    pst_okc_ie->ul_ie_len = st_app_ie.ul_app_buflen;

    OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_set_hilink_ie::app_frmtype is %d!}\r\n", st_app_ie.ul_app_frmtype);
    l_ret = wal_cfg80211_set_okc_ie(pst_net_dev, pst_okc_ie);
    oal_free(pst_okc_ie);

    return l_ret;
}


oal_int32 wal_ioctl_set_mgmt_frame_filters(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru *pst_fbt_sta)
{
    oal_uint32                  ul_mgmt_frame_filters;

    ul_mgmt_frame_filters = pst_fbt_sta->param.st_filter.ul_app_filterype;

    OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_set_mgmt_frame_filters::mgmt filters 0x%04x!}\r\n", ul_mgmt_frame_filters);
    return wal_cfg80211_set_mgmt_frame_filter(pst_net_dev, &ul_mgmt_frame_filters);
}


oal_int32 wal_ioctl_start_fbt_iwpriv_scan(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru *pst_fbt_sta,
                                                   oal_int32 en_operate)
{
    oal_hilink_scan_params st_fbt_scan_params;
    oal_ieee80211req_action_sta_stru *pst_detect_sta;

    OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_start_fbt_iwpriv_scan, opeation:%d !}\r\n", en_operate);

    pst_detect_sta = &pst_fbt_sta->param.st_action_sta;
    OAL_MEMZERO(&st_fbt_scan_params, OAL_SIZEOF(oal_hilink_scan_params));
    oal_memcopy(st_fbt_scan_params.auc_mac, pst_detect_sta->auc_mac_addr,OAL_MAC_ADDR_LEN);
    st_fbt_scan_params.ul_channel = pst_detect_sta->param.us_channel;

    if (en_operate == IEEE80211_VENDOR_ADD_DETECT)
    {
        st_fbt_scan_params.en_is_on = 1;
        st_fbt_scan_params.ul_interval = 30000;
    }
    else
    {
        st_fbt_scan_params.en_is_on = 0;
        st_fbt_scan_params.ul_interval = 0;
    }
    return wal_cfg80211_start_fbt_scan(pst_net_dev, &st_fbt_scan_params);
}


oal_int32 wal_ioctl_set_hiden_blacklist(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru *pst_fbt_sta,
                                                oal_int32 en_operate)
{
    mac_cfg_kick_user_param_stru    st_mac_cfg_kick_user_param;
    oal_int32                       l_ret;
    mac_vap_stru                   *pst_mac_vap;
    oal_ieee80211req_action_sta_stru *pst_action_sta;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{wal_ioctl_set_hiden_blacklist::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}\r\n");
        return -OAL_EINVAL;
    }

    OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_set_hiden_blacklist, operation:%d !}\r\n", en_operate);
    pst_action_sta = &pst_fbt_sta->param.st_action_sta;
    if (pst_action_sta->param.uc_mlme_phase_mask == 0)
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{wal_ioctl_set_hiden_blacklist::uc_mlme_phase_mask is 0.}\r\n");
        return -OAL_EINVAL;
    }

    /* ����ǹ㲥��ַ,����rej��Ϊ0��ֱ�ӷ��ز������� */
    if ((oal_is_broadcast_ether_addr(pst_action_sta->auc_mac_addr))
        && (IEEE80211_VENDOR_ADD_REJECT_STA == en_operate))
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{wal_ioctl_set_hiden_blacklist::add broadcast addr.}\r\n");
        return OAL_SUCC;
    }

    oal_memset(&st_mac_cfg_kick_user_param, 0, OAL_SIZEOF(mac_cfg_kick_user_param_stru));
    st_mac_cfg_kick_user_param.us_reason_code = 0;
    oal_memcopy(st_mac_cfg_kick_user_param.auc_mac_addr, pst_action_sta->auc_mac_addr, WLAN_MAC_ADDR_LEN);
    st_mac_cfg_kick_user_param.uc_mlme_phase_mask = pst_action_sta->param.uc_mlme_phase_mask;
    if (IEEE80211_VENDOR_ADD_REJECT_STA == en_operate)
    {
        st_mac_cfg_kick_user_param.uc_rej_user = OAL_TRUE;
        st_mac_cfg_kick_user_param.uc_kick_user = OAL_FALSE;
    }
    else
    {
        st_mac_cfg_kick_user_param.uc_rej_user = OAL_FALSE;
        st_mac_cfg_kick_user_param.uc_kick_user = OAL_FALSE;
    }

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{wal_ioctl_set_hiden_blacklist::uc_rej_user[%d], uc_kick_user[%d], mac[%x:%x].}\r\n",
                                st_mac_cfg_kick_user_param.uc_rej_user,
                                st_mac_cfg_kick_user_param.uc_kick_user,
                                st_mac_cfg_kick_user_param.auc_mac_addr[4],
                                st_mac_cfg_kick_user_param.auc_mac_addr[5]);

    l_ret = wal_cfg80211_fbt_kick_user(pst_net_dev, &st_mac_cfg_kick_user_param);
    return l_ret;
}


oal_int32 wal_ioctl_set_hiden_whitelist(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru *pst_fbt_sta,
                                                oal_int32 en_operate)
{
    oal_hilink_white_node_stru      st_white_lst;
    oal_int32                       l_ret = 0;
    oal_ieee80211req_action_sta_stru *pst_action_sta;

    OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_set_hiden_whitelist, operation:%d !}\r\n", en_operate);

    pst_action_sta = &pst_fbt_sta->param.st_action_sta;
    OAL_MEMZERO(&st_white_lst, OAL_SIZEOF(oal_hilink_white_node_stru));
    oal_memcopy(st_white_lst.auc_mac, pst_action_sta->auc_mac_addr, OAL_MAC_ADDR_LEN);
    st_white_lst.en_white_list_operate = (en_operate == IEEE80211_VENDOR_ADD_ACL_EXCEPT) ?
        OAL_WHITE_LIST_OPERATE_ADD : OAL_WHITE_LIST_OPERATE_DEL;
    st_white_lst.uc_flag = pst_action_sta->param.uc_flag;

    l_ret = wal_cfg80211_set_hiden_whitelist(pst_net_dev, &st_white_lst);
    return l_ret;
}


oal_int32 wal_ioctl_get_vap_diag_info(oal_net_device_stru *pst_net_dev, oal_vap_status_diag_info_stru *pst_vap_diag_info)
{
    mac_vap_stru                       *pst_mac_vap;
    hmac_vap_stru                      *pst_hmac_vap;
    oal_int32                           l_ret;
    mac_device_stru                    *pst_mac_dev;
    wal_msg_write_stru                  st_write_msg;
    mac_vap_status_diag_info_stru      *pst_ret_vap_diag_info;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{wal_ioctl_get_vap_diag_info::pst_mac_vap is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "wal_ioctl_get_vap_diag_info: hmac vap is null. vap id:%d", pst_mac_vap->uc_vap_id);
        return -OAL_EINVAL;
    }

    /***********************************************************************/
    /*                  ��ȡdmac vap��diag info��Ϣ                            */
    /***********************************************************************/
    OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_GET_VAP_DIAG_INFO, OAL_SIZEOF(mac_vap_status_diag_info_stru));
    pst_hmac_vap->auc_query_flag[QUERY_ID_VAP_DIAG_INFO] = OAL_FALSE;
    l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_vap_status_diag_info_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "wal_ioctl_get_vap_diag_info: send vap diag info event ret:%d", l_ret);
    }

    /*lint -e730*/
    l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(OAL_TRUE == pst_hmac_vap->auc_query_flag[QUERY_ID_VAP_DIAG_INFO]), 5*OAL_TIME_HZ);
    /*lint +e730*/
    if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "wal_ioctl_get_vap_diag_info: query vap diag info timeout. ret:%d", l_ret);
    }

    pst_ret_vap_diag_info = (mac_vap_status_diag_info_stru *)pst_hmac_vap->st_param_char.auc_buff;

    /* baisc setting */
    oal_memcopy(pst_vap_diag_info->auc_bssid, pst_mac_vap->auc_bssid, WLAN_MAC_ADDR_LEN);  //BSSID
    pst_vap_diag_info->uc_cur_channel = pst_mac_vap->st_channel.uc_chan_number;           //��ǰ�ŵ�
    pst_vap_diag_info->uc_offset_second_channel = pst_ret_vap_diag_info->uc_offset_second_channel; //��ѡ�ŵ�  80M��ʱ����ŵ���α�ʾ
    pst_vap_diag_info->uc_bandwidth = pst_mac_vap->st_channel.en_bandwidth;               //20M:20��40M:40��20/40M:60, ���ʹ�ñ�׼��������������Լ��
    /* dynamic config parameters */
    pst_vap_diag_info->uc_dig = 0;
    pst_vap_diag_info->uc_cca_shreshold= 0;
    pst_vap_diag_info->uc_channel_usage = pst_ret_vap_diag_info->uc_channel_usage;                                   //ռ�ձȣ�40%��Ӧֵ40  zhoukedou band steering
    pst_vap_diag_info->ul_hisi_dig = pst_ret_vap_diag_info->ul_hisi_dig;                                     //�������� ��mcs��� 3.11
    pst_vap_diag_info->auc_hisi_cca_shreshold[0] = pst_ret_vap_diag_info->auc_hisi_cca_shreshold[0];                 //��ȷ�Ϻ���   3.10.1
    pst_vap_diag_info->auc_hisi_cca_shreshold[1] = pst_ret_vap_diag_info->auc_hisi_cca_shreshold[1];
    pst_vap_diag_info->uc_sta_cnt = (oal_uint8)pst_mac_vap->us_user_nums;                            //�ն�����

    pst_mac_dev = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (pst_mac_dev != NULL)
    {
        pst_vap_diag_info->uc_scan_mode = pst_mac_dev->en_curr_scan_state;                //��ɨ�裺1��δɨ�裺0
    }
    else
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{wal_ioctl_get_vap_diag_info::pst_mac_dev null.}");
        pst_vap_diag_info->uc_scan_mode = 0;
    }

    pst_vap_diag_info->us_fa = pst_ret_vap_diag_info->us_fa;                                        /* invalid frame - crc err .etc*///1������Ч֡��������Ҫ����ͳ�� hal_get_rx_err_count
    pst_vap_diag_info->us_cca = pst_ret_vap_diag_info->us_cca;                                      /* valid frame, can be recongnized*///1������Ч֡����,��Ҫ����ͳ��
    pst_vap_diag_info->ul_channel_busy = pst_ret_vap_diag_info->ul_channel_busy;
    /* agg setting */
    pst_vap_diag_info->ul_wait_agg_time = pst_ret_vap_diag_info->ul_wait_agg_time;                  //�ۺϵȴ�ʱ�䣨ms)  //todo
    /* buffer use status */
    pst_vap_diag_info->ul_skb_remain_buffer = pst_ret_vap_diag_info->ul_skb_remain_buffer;          //skbʣ��buff
    pst_vap_diag_info->ul_vo_remain_count = pst_ret_vap_diag_info->ul_vo_remain_count;              //vo����ʣ��buff
    pst_vap_diag_info->ul_vi_remain_count = pst_ret_vap_diag_info->ul_vi_remain_count;              //WLAN_TID_MPDU_NUM_LIMIT- mac_device_stru->us_total_mpdu_num;  //vi����ʣ��buff
    pst_vap_diag_info->ul_be_remain_count = pst_ret_vap_diag_info->ul_be_remain_count;              //WLAN_TID_MPDU_NUM_LIMIT- mac_device_stru->us_total_mpdu_num;  //be����ʣ��buff
    pst_vap_diag_info->ul_bk_remain_count = pst_ret_vap_diag_info->ul_bk_remain_count;              //WLAN_TID_MPDU_NUM_LIMIT- mac_device_stru->us_total_mpdu_num;   //bk����ʣ��buff
    /* edca setting  */
    pst_vap_diag_info->ul_vo_edca = pst_ret_vap_diag_info->ul_vo_edca;                              //1151�ļĴ�������ֵ
    pst_vap_diag_info->ul_vi_edca = pst_ret_vap_diag_info->ul_vi_edca;
    pst_vap_diag_info->ul_be_edca = pst_ret_vap_diag_info->ul_be_edca;
    pst_vap_diag_info->ul_bk_edca = pst_ret_vap_diag_info->ul_bk_edca;                              //hi1151_vap_get_edca_machw_cw  3.9.1

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "wal_ioctl_get_vap_diag_info: bssid[3-5]:%02x:%02x:%02x\r\n",
                    pst_vap_diag_info->auc_bssid[3], pst_vap_diag_info->auc_bssid[4], pst_vap_diag_info->auc_bssid[5]);
    return OAL_SUCC;
}


oal_void wal_ioctl_get_user_diag_info(hmac_vap_stru *pst_hmac_vap, mac_user_stru *pst_user_tmp)
{
    oal_int32                            l_ret;
    wal_msg_write_stru                   st_write_msg;
    mac_cfg_query_rssi_stru             *pst_query_rssi_param;
    mac_cfg_query_rate_stru             *pst_query_rate_param;
    mac_cfg_query_psst_stru             *pst_query_psst_param;
    mac_sta_status_diag_info_stru       *pst_dmac_sta_diag_info;

    /***********************************************************************/
    /*                  ��ȡdmac user��RSSI��Ϣ                            */
    /***********************************************************************/
    OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_RSSI, OAL_SIZEOF(mac_cfg_query_rssi_stru));
    pst_query_rssi_param = (mac_cfg_query_rssi_stru *)st_write_msg.auc_value;
    pst_query_rssi_param->us_user_id = pst_user_tmp->us_assoc_id; /* ���û���id����ȥ */

    pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;
    l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_query_rssi_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_user_diag_info: send query rssi cfg event ret:%d", l_ret);
    }

    /*lint -e730*/
    l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), 5*OAL_TIME_HZ);
    /*lint +e730*/
    if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
    {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_user_diag_info: query rssi timeout. ret:%d", l_ret);
    }

    /***********************************************************************/
    /*                  ��ȡdmac user��������Ϣ                            */
    /***********************************************************************/
    OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_RATE, OAL_SIZEOF(mac_cfg_query_rate_stru));
    pst_query_rate_param = (mac_cfg_query_rate_stru *)st_write_msg.auc_value;

    pst_query_rate_param->us_user_id = pst_user_tmp->us_assoc_id; /* ���û���id����ȥ */
    pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;
    l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_query_rate_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_user_diag_info: send query rate cfg event ret:%d", l_ret);
    }

    /*lint -e730*/
    l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q, (OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), 5*OAL_TIME_HZ);
    /*lint +e730*/
    if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
    {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_user_diag_info: query rate timeout. ret:%d", l_ret);
    }

    /***********************************************************************/
    /*                  ��ȡdmac user��ʡ����״̬��Ϣ��Ϣ                            */
    /***********************************************************************/
    OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_PSST, OAL_SIZEOF(mac_cfg_query_psst_stru));
    pst_query_psst_param = (mac_cfg_query_psst_stru *)st_write_msg.auc_value;

    pst_query_psst_param->us_user_id = pst_user_tmp->us_assoc_id; /* ���û���id����ȥ */

    pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;
    l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_query_psst_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_user_diag_info: send query rate cfg event ret:%d", l_ret);
    }

    /*lint -e730*/
    l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q, (OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), 5*OAL_TIME_HZ);
    /*lint +e730*/
    if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
    {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_user_diag_info: query rate timeout. ret:%d", l_ret);
    }

    /***********************************************************************/
    /*                  ��ȡdmac user��diag info��Ϣ                            */
    /***********************************************************************/
    OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_GET_STA_DIAG_INFO, OAL_SIZEOF(mac_sta_status_diag_info_stru));
    pst_dmac_sta_diag_info = (mac_sta_status_diag_info_stru *)st_write_msg.auc_value;
    pst_dmac_sta_diag_info->pst_mac_usr = pst_user_tmp;

    pst_hmac_vap->auc_query_flag[QUERY_ID_STA_DIAG_INFO] = OAL_FALSE;
    l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_sta_status_diag_info_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HILINK, "wal_ioctl_get_all_sta_diag_info: send sta diag info event ret:%d", l_ret);
    }

    /*lint -e730*/
    l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(OAL_TRUE == pst_hmac_vap->auc_query_flag[QUERY_ID_STA_DIAG_INFO]), 5*OAL_TIME_HZ);
    /*lint +e730*/
    if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
    {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HILINK, "wal_ioctl_get_all_sta_diag_info: query sta diag info timeout. ret:%d", l_ret);
    }
}


oal_int32 wal_get_sta_diag_info(mac_vap_stru *pst_mac_vap, oal_sta_status_diag_info_stru *pst_sta_diag_info,
                                        oam_user_stat_info_stru *pst_oam_user_stat, mac_user_stru *pst_mac_user,
                                        mac_sta_status_diag_info_stru *pst_ret_sta_diag_info)
{
#define TID_STAT_TO_USER(_stat) (((_stat)[0])+((_stat)[1])+((_stat)[2])+((_stat)[3])+((_stat)[4])+((_stat)[5])+((_stat)[6])+((_stat)[7]))
    hmac_user_stru   *pst_hmac_user_tmp;
    oal_int8          c_rssi_temp;

    pst_hmac_user_tmp = mac_res_get_hmac_user_etc(pst_mac_user->us_assoc_id);
    if (OAL_PTR_NULL == pst_hmac_user_tmp)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{wal_get_sta_diag_info::mac_res_get_hmac_user_etc null}");
        return -OAL_ERR_CODE_PTR_NULL;
    }

    oal_memcopy(pst_sta_diag_info->auc_mac, pst_mac_user->auc_user_mac_addr, WLAN_MAC_ADDR_LEN);
    pst_sta_diag_info->auc_chip_vendor[0] = pst_hmac_user_tmp->auc_ie_oui[0];
    pst_sta_diag_info->auc_chip_vendor[1] = pst_hmac_user_tmp->auc_ie_oui[1];
    pst_sta_diag_info->auc_chip_vendor[2] = pst_hmac_user_tmp->auc_ie_oui[2];
    pst_sta_diag_info->uc_wl_mode = pst_hmac_user_tmp->st_user_base_info.en_protocol_mode;
    pst_sta_diag_info->uc_agg_mode = pst_ret_sta_diag_info->uc_agg_mode;
    if ((pst_ret_sta_diag_info->c_rssi > OAL_RSSI_SIGNAL_MAX) || (pst_ret_sta_diag_info->c_rssi < OAL_RSSI_SIGNAL_MIN))
    {
        c_rssi_temp = (oal_int8)HMAC_RSSI_SIGNAL_INVALID;
    }
    else
    {
        c_rssi_temp = pst_ret_sta_diag_info->c_rssi + HMAC_FBT_RSSI_ADJUST_VALUE;
        if (c_rssi_temp < 0)
        {
            c_rssi_temp = 0;
        }
        if (c_rssi_temp > HMAC_FBT_RSSI_MAX_VALUE)
        {
            c_rssi_temp = HMAC_FBT_RSSI_MAX_VALUE;
        }
    }
    pst_sta_diag_info->uc_rssi        = (oal_uint8)c_rssi_temp;
    pst_sta_diag_info->uc_snr = 0;
    pst_sta_diag_info->uc_tx_bandwidth = pst_hmac_user_tmp->st_user_base_info.en_cur_bandwidth;
    pst_sta_diag_info->uc_rx_bandwidth = pst_hmac_user_tmp->st_user_base_info.en_cur_bandwidth;
    pst_sta_diag_info->us_tx_rate = (oal_uint16)(pst_hmac_user_tmp->ul_tx_rate/1000/8);
    pst_sta_diag_info->us_rx_rate = (oal_uint16)(pst_hmac_user_tmp->ul_rx_rate/1000/8);
    pst_sta_diag_info->us_tx_minrate = (oal_uint16)(pst_hmac_user_tmp->ul_tx_rate_min/1000/8);
    pst_sta_diag_info->us_tx_maxrate = (oal_uint16)(pst_hmac_user_tmp->ul_tx_rate_max/1000/8);
    pst_sta_diag_info->ul_sleep_times = pst_ret_sta_diag_info->ul_sleep_times;
    pst_sta_diag_info->ul_tx_retry_cnt = pst_oam_user_stat->ul_tx_ppdu_retries;
    pst_sta_diag_info->ul_tx_pkts = TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_succ_num)+TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_in_ampdu);
    pst_sta_diag_info->ul_tx_fail = TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_fail_num)+TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_fail_in_ampdu);

    pst_sta_diag_info->ul_tx_th = pst_oam_user_stat->ul_curr_tx_bytes - pst_oam_user_stat->ul_last_tx_bytes;
    pst_sta_diag_info->ul_tx_prev_rate = pst_oam_user_stat->ul_last_tx_bytes - pst_oam_user_stat->ul_last2_tx_bytes;

    if ((pst_sta_diag_info->ul_tx_fail < (pst_sta_diag_info->ul_tx_fail + pst_sta_diag_info->ul_tx_pkts)/200)
        || (pst_sta_diag_info->ul_tx_fail == 0))

    {   //����ʧ�ܱ�����С�ڷ�������0.5% or ����ʧ������Ϊ0
        pst_sta_diag_info->uc_per = 0;  //��֡�ʣ��ٷֱ�ֵ��40%Ϊ40  ���� ���ļ�������ʼ���
    }
    else if (pst_sta_diag_info->ul_tx_fail > pst_sta_diag_info->ul_tx_pkts)
    {
        pst_sta_diag_info->uc_per = 255;
    }
    else
    {
        unsigned int tx_fail = pst_sta_diag_info->ul_tx_fail;
        unsigned int tx_pkts = pst_sta_diag_info->ul_tx_pkts;
        if (tx_pkts > 0x7fffffff)
        {
            tx_pkts /= 100;
            tx_fail /= 100;
        }
        pst_sta_diag_info->uc_per = (oal_uint8)(tx_fail * 100 / (tx_pkts + tx_fail));
    }

    pst_sta_diag_info->uc_max_agg_num = pst_ret_sta_diag_info->uc_max_agg_num;
    pst_sta_diag_info->uc_rts_rate = pst_ret_sta_diag_info->uc_rts_rate;
    pst_sta_diag_info->uc_rts_retry_cnt = pst_ret_sta_diag_info->uc_rts_retry_cnt;
    pst_sta_diag_info->ul_rx_pkts = pst_oam_user_stat->ul_rx_mpdu_num;
    pst_sta_diag_info->ul_tx_unicast_bytes = pst_ret_sta_diag_info->ul_tx_unicast_bytes;
    pst_sta_diag_info->ul_rx_unicast_bytes = pst_ret_sta_diag_info->ul_rx_unicast_bytes;
    pst_sta_diag_info->ul_tx_mcast_bytes = pst_ret_sta_diag_info->ul_tx_mcast_bytes;
    pst_sta_diag_info->ul_rx_mcast_bytes = pst_ret_sta_diag_info->ul_rx_mcast_bytes;
    oal_memcopy(pst_sta_diag_info->aul_sta_tx_mcs_cnt, pst_ret_sta_diag_info->aul_sta_tx_mcs_cnt,
                  sizeof(pst_sta_diag_info->aul_sta_tx_mcs_cnt));
    oal_memcopy(pst_sta_diag_info->aul_sta_rx_mcs_cnt, pst_ret_sta_diag_info->aul_sta_rx_mcs_cnt,
                  sizeof(pst_sta_diag_info->aul_sta_rx_mcs_cnt));
    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "wal_get_sta_diag_info: mac[3-5]:%02x:%02x:%02x\r\n",
                     pst_sta_diag_info->auc_mac[3], pst_sta_diag_info->auc_mac[4],pst_sta_diag_info->auc_mac[5]);
    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "wal_get_sta_diag_info: rssi %d, tx_uc_bytes %d, rx_uc 0x%x, \r\n",
                     pst_sta_diag_info->uc_rssi, pst_sta_diag_info->ul_tx_unicast_bytes, pst_sta_diag_info->ul_rx_unicast_bytes);

#undef TID_STAT_TO_USER
        return OAL_SUCC;
}


oal_int32 wal_ioctl_get_all_sta_diag_info(oal_net_device_stru *pst_net_dev,  oal_sta_status_diag_req_stru *pst_sta_diag_req,
                                          oal_sta_status_diag_info_stru *pst_all_sta_diag_info)
{
    mac_vap_stru                        *pst_mac_vap;
    hmac_vap_stru                       *pst_hmac_vap;
    oal_dlist_head_stru                 *pst_entry;
    oal_dlist_head_stru                 *pst_dlist_tmp;
    mac_user_stru                       *pst_user_tmp;
    oal_uint16                           us_sta_cnt = 0;
    oam_stat_info_stru                  *pst_oam_stat;
    oam_user_stat_info_stru             *pst_oam_user_stat;
    oal_sta_status_diag_info_stru       *pst_curr_sta_diag_info;
    mac_sta_status_diag_info_stru       *pst_dmac_sta_diag_info;
    oal_int32                            l_ret;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{wal_ioctl_get_all_sta_diag_info::pst_mac_vap is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{wal_ioctl_get_all_sta_diag_info: hmac vap is null. vap id:%d", pst_mac_vap->uc_vap_id);
        return -OAL_EINVAL;
    }

    /* ��ȡ��vap�µ��ѹ���sta��Ŀ�����λ�ȡÿ��sta��ͳ����Ϣ��copy����ʱ�������� */
    if (0 == pst_mac_vap->us_user_nums)
    {
        pst_sta_diag_req->ul_sta_cnt = 0;
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{wal_ioctl_get_all_sta_diag_info:: user_cnt is 0.}");
        return OAL_SUCC;
    }

    /* step2. ��������û���Ϣ */
    pst_oam_stat = OAM_STAT_GET_STAT_ALL();

    pst_curr_sta_diag_info = pst_all_sta_diag_info;
    /* step1. ͬ��Ҫ��ѯ��dmac��Ϣ */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
        if (OAL_PTR_NULL == pst_dlist_tmp)
        {
            /* ��forѭ���̻߳���ͣ���ڼ����ɾ���û��¼��������pst_dlist_tmpΪ�ա�Ϊ��ʱֱ��������ȡdmac��Ϣ */
            break;
        }

        if(pst_user_tmp->us_assoc_id >= WLAN_USER_MAX_USER_LIMIT)
        {
            continue;
        }

        if ((us_sta_cnt + 1) * OAL_SIZEOF(oal_sta_status_diag_info_stru) >  pst_sta_diag_req->ul_date_len)
        {
            break;
        }

        wal_ioctl_get_user_diag_info(pst_hmac_vap, pst_user_tmp);

        pst_dmac_sta_diag_info = (mac_sta_status_diag_info_stru *)pst_hmac_vap->st_param_char.auc_buff;
        pst_oam_user_stat = &(pst_oam_stat->ast_user_stat_info[pst_user_tmp->us_assoc_id]);
        l_ret = wal_get_sta_diag_info(pst_mac_vap, pst_curr_sta_diag_info, pst_oam_user_stat, pst_user_tmp, pst_dmac_sta_diag_info);
        if (OAL_SUCC != l_ret)
        {
            break;
        }
        pst_curr_sta_diag_info++;
        us_sta_cnt++;
    }

    /* ����ȡͳ����Ϣ���뵽ioctl�����У��������û��ռ䣬
       ���û��������뵽pri_data.all_sta_link_info.sta_cnt�� */
    pst_sta_diag_req->ul_sta_cnt = us_sta_cnt;
    pst_sta_diag_req->ul_channel = pst_hmac_vap->st_vap_base_info.st_channel.uc_chan_idx;
    return OAL_SUCC;
}


oal_int32 wal_ioctl_get_one_sta_diag_info(oal_net_device_stru *pst_net_dev, oal_ieee80211req_action_sta_stru *pst_action_sta,
                                          oal_sta_status_diag_info_stru  *pst_sta_diag_info)
{
    mac_vap_stru                        *pst_mac_vap;
    hmac_vap_stru                       *pst_hmac_vap;
    oal_dlist_head_stru                 *pst_entry;
    oal_dlist_head_stru                 *pst_dlist_tmp;
    mac_user_stru                       *pst_user_tmp;
    oam_stat_info_stru                  *pst_oam_stat;
    oam_user_stat_info_stru             *pst_oam_user_stat;
    mac_sta_status_diag_info_stru       *pst_dmac_sta_diag_info;
    oal_int32                            l_ret = OAL_SUCC;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{wal_ioctl_get_one_sta_diag_info::pst_mac_vap is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{wal_ioctl_get_one_sta_diag_info: hmac vap is null. vap id:%d", pst_mac_vap->uc_vap_id);
        return -OAL_EINVAL;
    }

    /* step2. ��������û���Ϣ */
    pst_oam_stat = OAM_STAT_GET_STAT_ALL();

    /* step1. ͬ��Ҫ��ѯ��dmac��Ϣ */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
        if (OAL_PTR_NULL == pst_dlist_tmp)
        {
            /* ��forѭ���̻߳���ͣ���ڼ����ɾ���û��¼��������pst_dlist_tmpΪ�ա�Ϊ��ʱֱ��������ȡdmac��Ϣ */
            break;
        }

        if(pst_user_tmp->us_assoc_id >= WLAN_USER_MAX_USER_LIMIT)
        {
            continue;
        }

        if (oal_memcmp(pst_user_tmp->auc_user_mac_addr, pst_action_sta->auc_mac_addr, OAL_MAC_ADDR_LEN) != 0)
        {
            continue;
        }

        wal_ioctl_get_user_diag_info(pst_hmac_vap, pst_user_tmp);

        pst_dmac_sta_diag_info = (mac_sta_status_diag_info_stru *)pst_hmac_vap->st_param_char.auc_buff;
        pst_oam_user_stat = &(pst_oam_stat->ast_user_stat_info[pst_user_tmp->us_assoc_id]);
        l_ret = wal_get_sta_diag_info(pst_mac_vap, pst_sta_diag_info, pst_oam_user_stat, pst_user_tmp, pst_dmac_sta_diag_info);
        return l_ret;
    }

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "wal_get_sta_diag_info:sta no exist, mac[3-5]:%02x:%02x:%02x\r\n",
                     pst_action_sta->auc_mac_addr[3], pst_action_sta->auc_mac_addr[4],pst_action_sta->auc_mac_addr[5]);
    return -OAL_EINVAL;
}


oal_int32 wal_ioctl_set_sensing_bssid(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru *pst_vendor_req,
                                                oal_int32 en_operate)
{
    dmac_sensing_bssid_cfg_stru       st_sensing_bssid;
    oal_int32                         l_ret = 0;
    oal_ieee80211req_action_sta_stru *pst_action_sta;

    OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_set_sensing_bssid, operation:%d !}\r\n", en_operate);

    pst_action_sta = &pst_vendor_req->param.st_action_sta;
    OAL_MEMZERO(&st_sensing_bssid, OAL_SIZEOF(dmac_sensing_bssid_cfg_stru));
    oal_memcopy(st_sensing_bssid.auc_mac_addr, pst_action_sta->auc_mac_addr, OAL_MAC_ADDR_LEN);
    st_sensing_bssid.en_operation = (en_operate == IEEE80211_VENDOR_ADD_SENSING_MAC) ?
        OAL_SENSING_BSSID_OPERATE_ADD : OAL_SENSING_BSSID_OPERATE_DEL;

    l_ret = wal_cfg80211_set_sensing_bssid(pst_net_dev, (oal_void *)&st_sensing_bssid);
    return l_ret;
}



oal_void wal_ioctl_query_sensing_bssid_info(hmac_vap_stru *pst_hmac_vap)
{
    oal_int32                            l_ret;
    wal_msg_write_stru                   st_write_msg;

    /***********************************************************************/
    /*                  ��ȡdmac user��RSSI��Ϣ                            */
    /***********************************************************************/
    OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_GET_SENSING_BSSID_INFO, OAL_SIZEOF(dmac_query_sensing_bssid_stru));

    pst_hmac_vap->auc_query_flag[QUERY_ID_SENSING_BSSID_INFO] = OAL_FALSE;
    l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(dmac_query_sensing_bssid_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "wal_ioctl_query_sensing_bssid_info: send query sensing bssid cfg event ret:%d", l_ret);
    }

    /*lint -e730*/
    l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(OAL_TRUE == pst_hmac_vap->auc_query_flag[QUERY_ID_SENSING_BSSID_INFO]), 5*OAL_TIME_HZ);
    /*lint +e730*/
    if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
    {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "wal_ioctl_query_sensing_bssid_info: query sensing bssid timeout. ret:%d", l_ret);
    }
}


oal_int32 wal_ioctl_get_sensing_bssid_info(oal_net_device_stru *pst_net_dev, oal_ieee80211req_getset_buf_stru *pst_getset_buf,
                                           oal_uint8  *puc_sensing_buf)
{
    mac_vap_stru                     *pst_mac_vap;
    hmac_vap_stru                    *pst_hmac_vap;
    ieee80211req_wifi_rssi_stru      *pst_sensing_bssid_info;
    dmac_query_sensing_bssid_stru    *pst_ret_sensing_info;
    oal_int32                         l_out_info_len = 0;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_sta_info::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{wal_ioctl_get_sensing_bssid_info: hmac vap is null. vap id:%d", pst_mac_vap->uc_vap_id);
        return -OAL_EINVAL;
    }

    pst_hmac_vap->st_param_char.l_buff_len= 0;
    wal_ioctl_query_sensing_bssid_info(pst_hmac_vap);
    pst_ret_sensing_info = (dmac_query_sensing_bssid_stru *)pst_hmac_vap->st_param_char.auc_buff;
    pst_sensing_bssid_info = (ieee80211req_wifi_rssi_stru *)puc_sensing_buf;
    for(l_out_info_len = 0; l_out_info_len < pst_hmac_vap->st_param_char.l_buff_len; l_out_info_len += (oal_int32)sizeof(dmac_query_sensing_bssid_stru))
    {
        if ((oal_uint32)((oal_uint8 *)(pst_sensing_bssid_info+1) - puc_sensing_buf) > pst_getset_buf->ul_buf_len)
        {
            break;
        }

        pst_sensing_bssid_info->ul_timestamp = pst_ret_sensing_info->ul_timestamp;
        pst_sensing_bssid_info->l_rssi = pst_ret_sensing_info->c_rssi;
        oal_memcopy(pst_sensing_bssid_info->auc_local, pst_mac_vap->auc_bssid, WLAN_MAC_ADDR_LEN);
        oal_memcopy(pst_sensing_bssid_info->auc_bssid, pst_ret_sensing_info->auc_mac_addr, WLAN_MAC_ADDR_LEN);
        pst_sensing_bssid_info++;
        pst_ret_sensing_info++;
    }
    pst_getset_buf->ul_buf_len = (oal_uint32)((oal_uint8 *)pst_sensing_bssid_info - puc_sensing_buf);
    return OAL_SUCC;
}

#endif
#endif

#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE

oal_int32 wal_ioctl_send_action_frame(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru  *pst_vendor_req)
{
    mac_vap_stru                    *pst_mac_vap;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_ieee80211req_send_raw_stru  st_send_frame_msg;
    oal_mgmt_tx_stru                *pst_mgmt_tx;
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret = 0;
    oal_int                         i_leftime;
    oal_uint32                      ul_ret = 0;
    oal_uint8                       *puc_send_buff;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev) || OAL_UNLIKELY(OAL_PTR_NULL == pst_vendor_req))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_send_action_frame::null ptr, pst_net_dev[0x%x] pst_vendor_req[0x%x].}\r\n",
            pst_net_dev, pst_vendor_req);
        return -OAL_EINVAL;
    }
    /* ��ȡ�����͵�֡��Ϣ */
    ul_ret = oal_copy_from_user(&st_send_frame_msg, pst_vendor_req->param.pst_raw_msg, OAL_SIZEOF(oal_ieee80211req_send_raw_stru));
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_ioctl_send_action_frame::pst_raw_msg copy_from_user return error code %d!}\r\n", ul_ret);
        return -OAL_EFAUL;
    }
    puc_send_buff = (oal_uint8 *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, st_send_frame_msg.us_len, OAL_TRUE);
    if (OAL_PTR_NULL == puc_send_buff)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_ioctl_send_action_frame:: malloc buff of send frame msg is null.}\r\n");
        return -OAL_EINVAL;
    }
    ul_ret = oal_copy_from_user(puc_send_buff, st_send_frame_msg.puc_msg, st_send_frame_msg.us_len);
    if (OAL_SUCC != ul_ret)
    {
        OAL_MEM_FREE(puc_send_buff, OAL_TRUE);
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_ioctl_send_action_frame::pst_raw_msg copy_from_user return error code %d!}\r\n", ul_ret);
        return -OAL_EFAUL;
    }
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAL_MEM_FREE(puc_send_buff, OAL_TRUE);
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_ioctl_send_action_frame::can't get mac vap from netdevice priv data.}\r\n");
        return -OAL_EINVAL;
    }
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAL_MEM_FREE(puc_send_buff, OAL_TRUE);
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_ioctl_send_action_frame::pst_hmac_vap ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    /* �ϲ㴫��������֡û�����MAC ����ַ���˴�������� */
    /* DA is address of STA addr ���ϲ㴫������ */
    oal_set_mac_addr(puc_send_buff + WLAN_HDR_ADDR1_OFFSET, st_send_frame_msg.auc_mac_addr);
    /* SA��ֵΪ�����MAC��ַ */
    oal_set_mac_addr(puc_send_buff + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));
    /* TA��ֵΪVAP��BSSID */
    oal_set_mac_addr(puc_send_buff + WLAN_HDR_ADDR3_OFFSET, pst_mac_vap->auc_bssid);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* 3.1 ��д msg ��Ϣͷ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SEND_ACTION_FRAME, OAL_SIZEOF(oal_ieee80211req_send_raw_stru));

    /* 3.2 ��д msg ��Ϣ�� ��֡��buffָ���滻���ں�ָ̬�� */
    st_send_frame_msg.puc_msg = puc_send_buff;
    oal_memcopy(st_write_msg.auc_value, &st_send_frame_msg, OAL_SIZEOF(oal_ieee80211req_send_raw_stru));

    /* 3.3 ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_ieee80211req_send_raw_stru),
                                (oal_uint8 *)&st_write_msg,
                                OAL_FALSE,
                                OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAL_MEM_FREE(puc_send_buff, OAL_TRUE);
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_send_action_frame::wal_send_cfg_event_etc return err code %d!}\r\n", l_ret);
        return OAL_FAIL;
    }

    pst_mgmt_tx = &(pst_hmac_vap->st_mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete= OAL_FALSE;
    pst_mgmt_tx->mgmt_tx_status  = OAL_FAIL;
    /*lint -e730*/
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_mgmt_tx->st_wait_queue, OAL_TRUE== pst_mgmt_tx->mgmt_tx_complete,  (oal_uint32)OAL_MSECS_TO_JIFFIES(WAL_MGMT_TX_TIMEOUT_MSEC));

    // ������ɺ��ͷ��ڴ�
    OAL_MEM_FREE(puc_send_buff, OAL_TRUE);

    if ( 0 == i_leftime)
    {
        /* ��ʱ����ʱ */
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_send_action_frame::action tx timeout!}\r\n");
        return OAL_FAIL;
    }
    else if (i_leftime < 0)
    {
        /* ��ʱ���ڲ����� */
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_send_action_frame::action tx timer error!}\r\n");
        return OAL_FAIL;
    }
    else
    {
        /* ��������  */
        OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_ioctl_send_action_frame::action tx commpleted!}\r\n");
        return (oal_int32)pst_mgmt_tx->mgmt_tx_status;
    }
}


oal_int32 wal_ioctl_disassoc_sta(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru  *pst_vendor_req)
{
    oal_ieee80211req_action_sta_stru    *pst_disassoc_sta_msg;
    oal_int32                       l_ret             = 0;
    oal_uint8                       auc_mac_addr[WLAN_MAC_ADDR_LEN] = {0};
    oal_uint16                      us_reason_code;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev) || OAL_UNLIKELY(OAL_PTR_NULL == pst_vendor_req))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_disassoc_sta::null ptr, pst_net_dev[0x%x] pst_vendor_req[0x%x].}\r\n",
            pst_net_dev, pst_vendor_req);
        return -OAL_EINVAL;
    }

    /* 1. ��ȡ�����͵�֡��Ϣ */
    pst_disassoc_sta_msg = &(pst_vendor_req->param.st_action_sta);
    if (OAL_PTR_NULL == pst_disassoc_sta_msg)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_ioctl_disassoc_sta::disassoc sta msg ptr is null.}\r\n");
        return -OAL_EINVAL;
    }

    /* 2. ���������ȡMAC */
    oal_set_mac_addr(auc_mac_addr, pst_disassoc_sta_msg->auc_mac_addr);

    /* 3. ����ȥ����STA��reason code */
    us_reason_code = pst_disassoc_sta_msg->param.us_reason_code;

    /* 4.���ýӿ�ȥ����STA */
    l_ret = wal_kick_sta(pst_net_dev, auc_mac_addr, us_reason_code);

    return l_ret;
}


oal_int32 wal_ioctl_get_reg_domain(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru  *pst_vendor_req)
{
    oal_int32                       l_ret;
    wal_msg_query_stru              st_query_msg;
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;
    wal_msg_rsp_stru                *pst_query_rsp_msg;
    oal_uint8                       auc_country[WLAN_COUNTRY_STR_LEN];
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    regdomain_enum_uint8            en_regdomain;
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev) || OAL_UNLIKELY(OAL_PTR_NULL == pst_vendor_req))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_get_reg_domain::null ptr pst_net_dev[0x%x] pst_vendor_req[0x%x].}\r\n",
            pst_net_dev, pst_vendor_req);
        return -OAL_EINVAL;
    }
    /***************************************************************************
       ���¼���wal�㴦��  �Ȼ�ȡ����������
    ***************************************************************************/
    st_query_msg.en_wid = WLAN_CFGID_COUNTRY;
    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                              WAL_MSG_TYPE_QUERY,
                              WAL_MSG_WID_LENGTH,
                              (oal_uint8 *)&st_query_msg,
                              OAL_TRUE,
                              &pst_rsp_msg);
    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_reg_domain:: wal_send_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ ��ȡ������ */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);
    oal_memcopy(auc_country, pst_query_rsp_msg->auc_value, WLAN_COUNTRY_STR_LEN);
    oal_free(pst_rsp_msg);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    /* ���ݹ�������ȡ������ ��ת��Ϊhera��Ʒ�����ö�� Ŀǰֻ�������������� */
    en_regdomain = hwifi_get_regdomain_from_country_code(auc_country, OAL_TRUE);
    switch(en_regdomain)
    {
        case REGDOMAIN_FCC:
            pst_vendor_req->param.st_domain.en_domain_code = OAL_WIFI_REG_DOMAIN_FCC;
            break;
        case REGDOMAIN_ETSI:
            pst_vendor_req->param.st_domain.en_domain_code = OAL_WIFI_REG_DOMAIN_ETSI;
            break;
        case REGDOMAIN_JAPAN:
            pst_vendor_req->param.st_domain.en_domain_code = OAL_WIFI_REG_DOMAIN_MKK;
            break;
        case REGDOMAIN_COMMON:
            pst_vendor_req->param.st_domain.en_domain_code = OAL_WIFI_REG_DOMAIN_GLOBAL;
            break;
        default:
            pst_vendor_req->param.st_domain.en_domain_code = OAL_WIFI_REG_DOMAIN_MAX;
            break;
    }
#endif
    return OAL_SUCC;
}


oal_int32 wal_ioctl_set_mgmt_frame_ie(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru  *pst_vendor_req)
{
    oal_int l_ret = OAL_FAIL;
    oal_uint8   ul_ret = 0;
    oal_ieee80211req_set_ie_stru    st_set_ie_data;
    wal_msg_write_stru              st_write_msg;
    oal_uint8                       *puc_ie_content = OAL_PTR_NULL;
    mac_vap_ie_set_stru             *pst_set_ie_msg = OAL_PTR_NULL;
    oal_uint16                      us_msg_len = 0;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev) || OAL_UNLIKELY(OAL_PTR_NULL == pst_vendor_req))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_mgmt_frame_ie::null ptr pst_net_dev[0x%x] pst_vendor_req[0x%x].}\r\n",
            pst_net_dev, pst_vendor_req);
        return -OAL_EINVAL;
    }
    ul_ret = oal_copy_from_user(&st_set_ie_data, pst_vendor_req->param.pst_ie, OAL_SIZEOF(oal_ieee80211req_set_ie_stru));
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_mgmt_frame_ie::set_ie_data copy_from_user return error code %d!}\r\n", ul_ret);
        return -OAL_EFAUL;
    }
    puc_ie_content = (oal_uint8 *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, (oal_uint16)st_set_ie_data.ul_len, OAL_TRUE);
    if(OAL_PTR_NULL == puc_ie_content)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_ioctl_set_mgmt_frame_ie:: malloc buff of ie set content is null.}\r\n");
        return -OAL_EINVAL;
    }
    ul_ret = oal_copy_from_user(puc_ie_content, st_set_ie_data.puc_content, st_set_ie_data.ul_len);
    if (OAL_SUCC != ul_ret)
    {
        OAL_MEM_FREE(puc_ie_content, OAL_TRUE);
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_mgmt_frame_ie::set_ie_data copy_from_user return error code %d!}\r\n", ul_ret);
        return -OAL_EFAUL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* 3.1 ��д msg ��Ϣͷ */
    us_msg_len = OAL_SIZEOF(mac_vap_ie_set_stru) + (oal_uint16)st_set_ie_data.ul_len;
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_MGMT_FRAME_IE, us_msg_len);

    /* 3.2 ��д msg ��Ϣ�� */
    pst_set_ie_msg = (mac_vap_ie_set_stru *)(st_write_msg.auc_value);
    pst_set_ie_msg->us_ie_content_len = (oal_uint16)st_set_ie_data.ul_len;
    pst_set_ie_msg->en_eid = (oal_uint8)st_set_ie_data.ul_elment_id;
    pst_set_ie_msg->en_set_type = (oal_uint8)st_set_ie_data.ul_type;
    oal_memcopy(pst_set_ie_msg->auc_ie_content, puc_ie_content, st_set_ie_data.ul_len);

    /* 3.3 ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + us_msg_len,
                                (oal_uint8 *)&st_write_msg,
                                OAL_FALSE,
                                OAL_PTR_NULL);
    /* �ͷ������ڴ� */
    OAL_MEM_FREE(puc_ie_content, OAL_TRUE);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_mgmt_frame_ie::wal_send_cfg_event_etc return err code %d!}\r\n", l_ret);
        return OAL_FAIL;
    }
    return l_ret;
}


oal_int32 wal_ioctl_set_mgmt_frame_cap_info(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru  *pst_vendor_req)
{
    oal_int l_ret = OAL_FAIL;
    oal_ieee80211req_set_cap_stru   *pst_set_cap_data;
    wal_msg_write_stru              st_write_msg;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev) || OAL_UNLIKELY(OAL_PTR_NULL == pst_vendor_req))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_mgmt_frame_cap_info::null ptr pst_net_dev[0x%x] pst_vendor_req[0x%x].}\r\n",
            pst_net_dev, pst_vendor_req);
        return -OAL_EINVAL;
    }
    pst_set_cap_data = &pst_vendor_req->param.st_cap_info;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* 3.1 ��д msg ��Ϣͷ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_MGMT_CAP_INFO, OAL_SIZEOF(oal_ieee80211req_set_cap_stru));

    /* 3.2 ��д msg ��Ϣ�� */
    oal_memcopy(st_write_msg.auc_value, pst_set_cap_data, OAL_SIZEOF(oal_ieee80211req_set_cap_stru));

    /* 3.3 ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_ieee80211req_set_cap_stru),
                                (oal_uint8 *)&st_write_msg,
                                OAL_FALSE,
                                OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_mgmt_frame_cap_info::wal_send_cfg_event_etc return err code %d!}\r\n", l_ret);
        return OAL_FAIL;
    }
    return l_ret;
}
#endif // _PRE_WLAN_FEATURE_11KV_INTERFACE

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT_DEBUG


oal_uint32 wal_ioctl_show_vendor_test_cmd(oal_uint32 ul_cmd_id)
{
    const char *puc_vendor_req_cmd[] =
    {
        "IEEE80211_VENDOR_ADD_VENDOR_IE  = 0",
        "IEEE80211_VENDOR_MGMT_FILTER",
        "IEEE80211_VENDOR_ADD_DETECT",
        "IEEE80211_VENDOR_REMOVE_DETECT",
        "IEEE80211_VENDOR_ADD_ACL_EXCEPT",
        "IEEE80211_VENDOR_REMOVE_ACL_EXCEPT",
        "IEEE80211_VENDOR_GET_VAP_STATUS",
        "IEEE80211_VENDOR_GET_STA_STATUS",
        "IEEE80211_VENDOR_GET_ONE_STA_STATUS",
        "IEEE80211_VENDOR_ADD_REJECT_STA",
        "IEEE80211_VENDOR_REMOVE_REJECT_STA",
        "IEEE80211_VENDOR_SEND_RAW",
        "IEEE80211_VENDOR_DISASSOC_STA",
        "IEEE80211_VENDOR_GET_REG_DOMAIN",
        "IEEE80211_VENDOR_SET_IE",
        "IEEE80211_VENDOR_SET_CAP_INFO",
        "IEEE80211_VENDOR_ADD_SENSING_MAC",
        "IEEE80211_VENDOR_REMOVE_SENSING_MAC",
        "IEEE80211_VENDOR_GET_SENSING_AP_RSSI",
        "IEEE80211_VENDOR_MAX",
    };
    oal_uint32 n;

    if (ul_cmd_id < OAL_ARRAY_SIZE(puc_vendor_req_cmd))
    {
        OAL_IO_PRINT("vendor_req_cmd:%s.", puc_vendor_req_cmd[ul_cmd_id]);
        return OAL_SUCC;
    }

    else
    {
        OAL_IO_PRINT("show cmd info\r\n");
        OAL_IO_PRINT("cmd format: \"cmd-id mac-addr|okc_ie|filter [para]\"\r\n");
        OAL_IO_PRINT("   example: \"2 00:01:02:03:04:05 0000\"\r\n");
        OAL_IO_PRINT("   cmd:IEEE80211_VENDOR_ADD_DETECT, sta: 00:01:02:03:04:05, channel:0000\"\r\n");
        OAL_IO_PRINT("vendor_req_cmd id list:\r\n");
        for (n = 0; n < OAL_ARRAY_SIZE(puc_vendor_req_cmd); n++)
        {
            OAL_IO_PRINT("\t%s,\r\n", puc_vendor_req_cmd[n]);
        }
        return OAL_FAIL;
    }
}

#define SHOW_DIAG_INFO(desc, value)   printk("%s : %d(0x%x)\r\n", desc, value, value)
#define SHOW_DIAG_INFO_N(desc, value, num) \
    do { oal_uint32 n; \
        printk("%s:", desc); \
        for (n = 0; n < num; n++) { printk("%x-", value[n]); }\
        printk("\r\n"); \
    } while(0)

oal_void wal_ioctl_show_vap_info(oal_vap_status_diag_info_stru *pst_vap_diag_info)
{
    SHOW_DIAG_INFO_N("auc_bssid", pst_vap_diag_info->auc_bssid, ETH_ALEN);
    SHOW_DIAG_INFO("uc_cur_channel", pst_vap_diag_info->uc_cur_channel);
    SHOW_DIAG_INFO("uc_offset_second_channel", pst_vap_diag_info->uc_offset_second_channel);
    SHOW_DIAG_INFO("uc_bandwidth", pst_vap_diag_info->uc_bandwidth);
    SHOW_DIAG_INFO("uc_dig", pst_vap_diag_info->uc_dig);
    SHOW_DIAG_INFO("uc_cca_shreshold", pst_vap_diag_info->uc_cca_shreshold);
    SHOW_DIAG_INFO("uc_channel_usage", pst_vap_diag_info->uc_channel_usage);
    SHOW_DIAG_INFO("ul_hisi_dig", pst_vap_diag_info->ul_hisi_dig);
    SHOW_DIAG_INFO_N("auc_hisi_cca_shreshold", pst_vap_diag_info->auc_hisi_cca_shreshold, 2);
    SHOW_DIAG_INFO("uc_sta_cnt", pst_vap_diag_info->uc_sta_cnt);
    SHOW_DIAG_INFO("uc_scan_mode", pst_vap_diag_info->uc_scan_mode);
    SHOW_DIAG_INFO("us_fa", pst_vap_diag_info->us_fa);
    SHOW_DIAG_INFO("us_cca", pst_vap_diag_info->us_cca);
    SHOW_DIAG_INFO("ul_channel_busy", pst_vap_diag_info->ul_channel_busy);
    SHOW_DIAG_INFO("ul_wait_agg_time", pst_vap_diag_info->ul_wait_agg_time);
    SHOW_DIAG_INFO("ul_skb_remain_buffer", pst_vap_diag_info->ul_skb_remain_buffer);
    SHOW_DIAG_INFO("ul_vo_remain_count", pst_vap_diag_info->ul_vo_remain_count);
    SHOW_DIAG_INFO("ul_vi_remain_count", pst_vap_diag_info->ul_vi_remain_count);
    SHOW_DIAG_INFO("ul_be_remain_count", pst_vap_diag_info->ul_be_remain_count);
    SHOW_DIAG_INFO("ul_bk_remain_count", pst_vap_diag_info->ul_bk_remain_count);
    SHOW_DIAG_INFO("ul_vo_edca", pst_vap_diag_info->ul_vo_edca);
    SHOW_DIAG_INFO("ul_vi_edca", pst_vap_diag_info->ul_vi_edca);
    SHOW_DIAG_INFO("ul_be_edca", pst_vap_diag_info->ul_be_edca);
    SHOW_DIAG_INFO("ul_bk_edca", pst_vap_diag_info->ul_bk_edca);
}

oal_void wal_ioctl_show_one_sta_info(oal_sta_status_diag_info_stru *pst_sta_diag_info)
{
    SHOW_DIAG_INFO_N("auc_mac", pst_sta_diag_info->auc_mac, 6);
    SHOW_DIAG_INFO("uc_db_mode", pst_sta_diag_info->uc_db_mode);
    SHOW_DIAG_INFO_N("auc_chip_vendor", pst_sta_diag_info->auc_chip_vendor, 3);
    SHOW_DIAG_INFO("uc_wl_mode", pst_sta_diag_info->uc_wl_mode);
    SHOW_DIAG_INFO("uc_agg_mode", pst_sta_diag_info->uc_agg_mode);
    SHOW_DIAG_INFO("uc_rssi", pst_sta_diag_info->uc_rssi);
    SHOW_DIAG_INFO("uc_snr", pst_sta_diag_info->uc_snr);
    SHOW_DIAG_INFO("uc_tx_bandwidth", pst_sta_diag_info->uc_tx_bandwidth);
    SHOW_DIAG_INFO("uc_rx_bandwidth", pst_sta_diag_info->uc_rx_bandwidth);
    SHOW_DIAG_INFO("us_tx_rate", pst_sta_diag_info->us_tx_rate);
    SHOW_DIAG_INFO("us_rx_rate", pst_sta_diag_info->us_rx_rate);
    SHOW_DIAG_INFO("us_tx_minrate", pst_sta_diag_info->us_tx_minrate);
    SHOW_DIAG_INFO("us_tx_maxrate", pst_sta_diag_info->us_tx_maxrate);
    SHOW_DIAG_INFO("ul_sleep_times", pst_sta_diag_info->ul_sleep_times);
    SHOW_DIAG_INFO("ul_tx_retry_cnt", pst_sta_diag_info->ul_tx_retry_cnt);
    SHOW_DIAG_INFO("uc_per", pst_sta_diag_info->uc_per);
    SHOW_DIAG_INFO("uc_max_agg_num", pst_sta_diag_info->uc_max_agg_num);
    SHOW_DIAG_INFO("uc_rts_rate", pst_sta_diag_info->uc_rts_rate);
    SHOW_DIAG_INFO("uc_rts_retry_cnt", pst_sta_diag_info->uc_rts_retry_cnt);
    SHOW_DIAG_INFO("ul_tx_pkts", pst_sta_diag_info->ul_tx_pkts);
    SHOW_DIAG_INFO("ul_tx_fail", pst_sta_diag_info->ul_tx_fail);
    SHOW_DIAG_INFO("ul_rx_pkts", pst_sta_diag_info->ul_rx_pkts);
    SHOW_DIAG_INFO("ul_tx_unicast_bytes", pst_sta_diag_info->ul_tx_unicast_bytes);
    SHOW_DIAG_INFO("ul_rx_unicast_bytes", pst_sta_diag_info->ul_rx_unicast_bytes);
    SHOW_DIAG_INFO("ul_tx_mcast_bytes", pst_sta_diag_info->ul_tx_mcast_bytes);
    SHOW_DIAG_INFO("ul_rx_mcast_bytes", pst_sta_diag_info->ul_rx_mcast_bytes);
    SHOW_DIAG_INFO_N("aul_sta_tx_mcs_cnt", pst_sta_diag_info->aul_sta_tx_mcs_cnt, 16);
    SHOW_DIAG_INFO_N("aul_sta_rx_mcs_cnt", pst_sta_diag_info->aul_sta_rx_mcs_cnt, 16);
    SHOW_DIAG_INFO("ul_tx_spending", pst_sta_diag_info->ul_tx_spending);
    SHOW_DIAG_INFO("ul_tx_th", pst_sta_diag_info->ul_tx_th);
    SHOW_DIAG_INFO("ul_tx_prev_rate", pst_sta_diag_info->ul_tx_prev_rate);
}

oal_void wal_ioctl_show_sensing_info(oal_ieee80211req_getset_buf_stru *pst_buf_stru)
{
    ieee80211req_wifi_rssi_stru      *pst_sensing_bssid_info;
    oal_uint32                        ul_out_info_len;
    oal_uint32                        ul_sensing_stru_len = sizeof(ieee80211req_wifi_rssi_stru);

    pst_sensing_bssid_info = (ieee80211req_wifi_rssi_stru *)pst_buf_stru->puc_buf;
    for(ul_out_info_len = 0; (ul_out_info_len + ul_sensing_stru_len) < pst_buf_stru->ul_buf_len; ul_out_info_len += ul_sensing_stru_len)
    {
        SHOW_DIAG_INFO("ul_timestamp", pst_sensing_bssid_info->ul_timestamp);
        SHOW_DIAG_INFO("l_rssi", pst_sensing_bssid_info->l_rssi);
        SHOW_DIAG_INFO_N("auc_local", pst_sensing_bssid_info->auc_local, 6);
        SHOW_DIAG_INFO_N("auc_bssid", pst_sensing_bssid_info->auc_bssid, 6);
        pst_sensing_bssid_info++;
    }
}


oal_int32 wal_ioctl_set_mgmt_frame_ie_test(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru  *pst_vendor_req)
{
    oal_int l_ret = OAL_FAIL;
    oal_ieee80211req_set_ie_stru    st_set_ie_data;
    wal_msg_write_stru              st_write_msg;
    oal_uint8                       *puc_ie_content = OAL_PTR_NULL;
    mac_vap_ie_set_stru             *pst_set_ie_msg = OAL_PTR_NULL;
    oal_uint16                      us_msg_len = 0;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev) || OAL_UNLIKELY(OAL_PTR_NULL == pst_vendor_req))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_mgmt_frame_ie_test::null ptr pst_net_dev[0x%x] pst_vendor_req[0x%x].}\r\n",
            pst_net_dev, pst_vendor_req);
        return -OAL_EINVAL;
    }
    oal_memcopy(&st_set_ie_data, pst_vendor_req->param.pst_ie, OAL_SIZEOF(oal_ieee80211req_set_ie_stru));
    puc_ie_content = st_set_ie_data.puc_content;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* 3.1 ��д msg ��Ϣͷ */
    us_msg_len = OAL_SIZEOF(mac_vap_ie_set_stru) + (oal_uint16)st_set_ie_data.ul_len;
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_MGMT_FRAME_IE, us_msg_len);

    /* 3.2 ��д msg ��Ϣ�� */
    pst_set_ie_msg = (mac_vap_ie_set_stru *)(st_write_msg.auc_value);
    pst_set_ie_msg->us_ie_content_len = (oal_uint16)st_set_ie_data.ul_len;
    pst_set_ie_msg->en_eid = (oal_uint8)st_set_ie_data.ul_elment_id;
    pst_set_ie_msg->en_set_type = (oal_uint8)st_set_ie_data.ul_type;
    oal_memcopy(pst_set_ie_msg->auc_ie_content, puc_ie_content, st_set_ie_data.ul_len);

    /* 3.3 ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + us_msg_len,
                                (oal_uint8 *)&st_write_msg,
                                OAL_FALSE,
                                OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_mgmt_frame_ie_test::wal_send_cfg_event_etc return err code %d!}\r\n", l_ret);
        return OAL_FAIL;
    }
    return l_ret;
}


oal_uint32 g_ul_first_show_req = 0;
oal_int32 wal_ioctl_process_vendor_test_cmd(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
 #define SHOW_VENDOR_REQ_STA(mac)  OAL_IO_PRINT("sta_addr[%02x:%02x:%02x:%02x:%02x:%02x].\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5])
 #define SHOW_VENDOR_REQ_PARA(str, value)  OAL_IO_PRINT("%s:%d\r\n", str, value)
    oal_int32   l_ret = 0;
    oal_uint32  ul_ret = 0;
    oal_uint32  ul_off_set = 0;
    oal_uint8   uc_para_index = 0;
    oal_int8    *pc_cmd_str = pc_extra;     /* format: [cmd] [mac-addr] *//* ��������ʡ����length�ֶ� */
    oal_int8    ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint8   auc_mac_addr[WLAN_MAC_ADDR_LEN];
    oal_ieee80211_vendor_req_stru   st_vendor_req;
    oal_ieee80211req_action_sta_stru *pst_action_sta = &st_vendor_req.param.st_action_sta;
    oal_ieee80211req_send_raw_stru  *pst_raw_msg;                   /* ����action֡ */
    oal_ieee80211req_set_ie_stru    st_ie_set;                      /* ����IE��Ϣ format: 13 [eid] [type] [len] [content] */
    oal_ieee80211req_set_cap_stru   st_cap_info;                    /* ����capinfo */
    oal_iw_point_stru               *pst_param = (oal_iw_point_stru *)p_param;
    /* 11v bss transition request frame �����ھ�ap�б� */
    oal_uint8   auc_action_buff[] = {0xd0,0,0,0,0,0x0,1,2,3,4,5,0x0,1,2,3,4,5,0x0,1,2,3,4,5,0x0,0x0,
                0x0a,0x07,0x01,0x01,0xff,0xff,0x00};

    OAL_IO_PRINT("input vendor-req cmd:\"%s\".\r\n", pc_extra);
    OAL_MEMZERO(&st_vendor_req, sizeof(st_vendor_req));
    // str = '1 ff:ff:ff:ff:ff:ff xxxx"

    /* ��ȡ�������� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_cmd_str, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_process_vendor_test_cmd::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    st_vendor_req.us_cmd = (oal_uint16) oal_atoi(ac_name);
    pc_cmd_str += ul_off_set;
    while (' ' == *pc_cmd_str)
    {
        ++pc_cmd_str;
    }

    if (g_ul_first_show_req == 0)
    {
        wal_ioctl_show_vendor_test_cmd(0xff);
        g_ul_first_show_req = 1;
        return OAL_SUCC;
    }

    if (wal_ioctl_show_vendor_test_cmd(st_vendor_req.us_cmd) != OAL_SUCC)
    {
        return OAL_SUCC;
    }

    st_vendor_req.us_cmd += IEEE80211_VENDOR_ADD_VENDOR_IE;
    switch(st_vendor_req.us_cmd)
    {
        case IEEE80211_VENDOR_ADD_DETECT:
        case IEEE80211_VENDOR_REMOVE_DETECT:
        case IEEE80211_VENDOR_ADD_REJECT_STA:
        case IEEE80211_VENDOR_REMOVE_REJECT_STA:
        case IEEE80211_VENDOR_ADD_ACL_EXCEPT:
        case IEEE80211_VENDOR_REMOVE_ACL_EXCEPT:
        case IEEE80211_VENDOR_SEND_RAW:
        case IEEE80211_VENDOR_GET_ONE_STA_STATUS:
        case IEEE80211_VENDOR_ADD_SENSING_MAC:
        case IEEE80211_VENDOR_REMOVE_SENSING_MAC:
            oal_strtoaddr(pc_cmd_str, pst_action_sta->auc_mac_addr);
            pc_cmd_str += 19;
            pst_action_sta->param.us_channel = (oal_uint16)oal_strtol(pc_cmd_str, NULL, 16);
            break;
        default:
            break;
    }

    l_ret = -OAL_EINVAL;
    switch(st_vendor_req.us_cmd)
    {
        default:
            wal_ioctl_show_vendor_test_cmd(0xff);
            break;
        case IEEE80211_VENDOR_ADD_VENDOR_IE:
            {
                oal_app_ie_stru   *pst_okc_ie;
                pst_okc_ie = oal_memalloc(sizeof(oal_app_ie_stru));
                pst_okc_ie->en_app_ie_type = OAL_APP_OKC_PROBE_IE;
                pst_okc_ie->ul_ie_len = OAL_STRLEN(pc_extra+2);
                oal_memcopy(pst_okc_ie->auc_ie, (pc_extra+2), pst_okc_ie->ul_ie_len);
                OAL_IO_PRINT("OKC_IE:%s.\r\n", pst_okc_ie->auc_ie);
                l_ret = wal_cfg80211_set_okc_ie(pst_net_dev, pst_okc_ie);
                oal_free(pst_okc_ie);
            }
            break;
        case IEEE80211_VENDOR_MGMT_FILTER:
            {
                oal_uint32                  ul_mgmt_frame_filters;
                ul_mgmt_frame_filters = (oal_uint32)oal_strtol(pc_extra+2, NULL, 16);
                OAL_IO_PRINT("mgmt-frame-filter:0x%04x.\r\n", ul_mgmt_frame_filters);
                l_ret = wal_cfg80211_set_mgmt_frame_filter(pst_net_dev, &ul_mgmt_frame_filters);
            }
            break;
        case IEEE80211_VENDOR_ADD_DETECT:
        case IEEE80211_VENDOR_REMOVE_DETECT:
            SHOW_VENDOR_REQ_STA(pst_action_sta->auc_mac_addr);
            SHOW_VENDOR_REQ_PARA("channel", pst_action_sta->param.us_channel);
            l_ret = wal_ioctl_start_fbt_iwpriv_scan(pst_net_dev, &st_vendor_req, st_vendor_req.us_cmd);
            break;
        case IEEE80211_VENDOR_ADD_REJECT_STA:
        case IEEE80211_VENDOR_REMOVE_REJECT_STA:
            SHOW_VENDOR_REQ_STA(pst_action_sta->auc_mac_addr);
            SHOW_VENDOR_REQ_PARA("mlme_phase_mask", pst_action_sta->param.us_channel);
            l_ret = wal_ioctl_set_hiden_blacklist(pst_net_dev, &st_vendor_req, st_vendor_req.us_cmd);
            break;
        case IEEE80211_VENDOR_ADD_ACL_EXCEPT:
        case IEEE80211_VENDOR_REMOVE_ACL_EXCEPT:
            SHOW_VENDOR_REQ_STA(pst_action_sta->auc_mac_addr);
            SHOW_VENDOR_REQ_PARA("flag", pst_action_sta->param.uc_flag);
            l_ret = wal_ioctl_set_hiden_whitelist(pst_net_dev, &st_vendor_req, st_vendor_req.us_cmd);
            break;
        case IEEE80211_VENDOR_GET_VAP_STATUS:
            {
                oal_vap_status_diag_info_stru vap_status;
                l_ret = wal_ioctl_get_vap_diag_info(pst_net_dev, &vap_status);
                wal_ioctl_show_vap_info(&vap_status);
            }
            break;
        case  IEEE80211_VENDOR_GET_STA_STATUS:
            {
                oal_uint32 n;
                oal_sta_status_diag_req_stru st_sta_status_req;
                st_vendor_req.param.pst_sta_status_req = &st_sta_status_req;
                st_sta_status_req.ul_date_len = sizeof(oal_sta_status_diag_info_stru) * 32;
                st_sta_status_req.pst_data = oal_memalloc(st_sta_status_req.ul_date_len);
                st_sta_status_req.ul_sta_cnt = 0;
                l_ret = wal_ioctl_get_all_sta_diag_info(pst_net_dev, &st_sta_status_req, st_sta_status_req.pst_data);
                for (n = 0; n < st_sta_status_req.ul_sta_cnt; n++)
                {
                    wal_ioctl_show_one_sta_info(&st_sta_status_req.pst_data[n]);
                }
                oal_free(st_sta_status_req.pst_data);
            }
            break;
        case IEEE80211_VENDOR_GET_ONE_STA_STATUS:
            {
                oal_sta_status_diag_info_stru  st_sta_diag_info;
                SHOW_VENDOR_REQ_STA(pst_action_sta->auc_mac_addr);
                l_ret = wal_ioctl_get_one_sta_diag_info(pst_net_dev, pst_action_sta, &st_sta_diag_info);
                wal_ioctl_show_one_sta_info(&st_sta_diag_info);
            }
            break;
         case IEEE80211_VENDOR_ADD_SENSING_MAC:
         case IEEE80211_VENDOR_REMOVE_SENSING_MAC:
             SHOW_VENDOR_REQ_STA(pst_action_sta->auc_mac_addr);
             l_ret = wal_ioctl_set_sensing_bssid(pst_net_dev, &st_vendor_req, st_vendor_req.us_cmd);
             break;
         case IEEE80211_VENDOR_GET_SENSING_AP_RSSI:
            {
                oal_uint8  auc_sensing_buf[512];
                oal_ieee80211req_getset_buf_stru st_getset_buf;
                st_getset_buf.ul_buf_len = 512;
                st_getset_buf.puc_buf = auc_sensing_buf;
                l_ret = wal_ioctl_get_sensing_bssid_info(pst_net_dev, &st_getset_buf, auc_sensing_buf);
                wal_ioctl_show_sensing_info(&st_getset_buf);
            }
            break;
        case IEEE80211_VENDOR_SEND_RAW:
            {
                /* ��ȡMAC��ַ */
                ul_ret = wal_get_cmd_one_arg_etc(pc_cmd_str, ac_name, &ul_off_set);
                if (OAL_SUCC != ul_ret)
                {
                     OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_process_vendor_test_cmd::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
                     return ul_ret;
                }
                oal_strtoaddr(ac_name, auc_mac_addr);
                pst_raw_msg = (oal_ieee80211req_send_raw_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_MANAGEMENT_FRAME_LEN, OAL_TRUE);
                if(OAL_PTR_NULL != pst_raw_msg)
                {
                    oal_memcopy(pst_raw_msg->auc_mac_addr, auc_mac_addr, WLAN_MAC_ADDR_LEN);
                    pst_raw_msg->us_len = OAL_SIZEOF(auc_action_buff);
                    oal_memcopy(pst_raw_msg->puc_msg, auc_action_buff, pst_raw_msg->us_len);
                    st_vendor_req.param.pst_raw_msg = pst_raw_msg;
                    l_ret = wal_ioctl_send_action_frame(pst_net_dev, &st_vendor_req);
                    OAL_MEM_FREE(pst_raw_msg, OAL_TRUE);
                }
            }
            break;
        case IEEE80211_VENDOR_GET_REG_DOMAIN:
            {
                l_ret = wal_ioctl_get_reg_domain(pst_net_dev, &st_vendor_req);
                pst_param->length = (oal_uint16)OAL_SIZEOF(st_vendor_req.param.st_domain);
                oal_memcopy(pc_extra, &st_vendor_req.param.st_domain, pst_param->length);
                OAL_IO_PRINT("domain:%d\r\n", st_vendor_req.param.st_domain.en_domain_code);
            }
            break;
        case IEEE80211_VENDOR_SET_IE:
            {
                /* ��ȡ��Ҫ���õ�IE��Ϣ */
                ul_ret = wal_get_cmd_one_arg_etc(pc_cmd_str, ac_name, &ul_off_set);
                if (OAL_SUCC != ul_ret)
                {
                     OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_process_vendor_test_cmd::wal_get_cmd_one_arg_etc[eid] return err_code [%d]!}\r\n", ul_ret);
                     return ul_ret;
                }
                st_ie_set.ul_elment_id = oal_atoi(ac_name);         /* EID */
                pc_cmd_str += ul_off_set;
                ul_ret = wal_get_cmd_one_arg_etc(pc_cmd_str, ac_name, &ul_off_set);
                if (OAL_SUCC != ul_ret)
                {
                     OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_process_vendor_test_cmd::wal_get_cmd_one_arg_etc[type] return err_code [%d]!}\r\n", ul_ret);
                     return ul_ret;
                }
                st_ie_set.ul_type = oal_atoi(ac_name);         /* TYPE */
                pc_cmd_str += ul_off_set;
                ul_ret = wal_get_cmd_one_arg_etc(pc_cmd_str, ac_name, &ul_off_set);
                if (OAL_SUCC != ul_ret)
                {
                     OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_process_vendor_test_cmd::wal_get_cmd_one_arg_etc[len] return err_code [%d]!}\r\n", ul_ret);
                     return ul_ret;
                }
                st_ie_set.ul_len = oal_atoi(ac_name);         /* LEN */
                pc_cmd_str += ul_off_set;
                /* ����ָ�� */
                st_ie_set.puc_content = (oal_uint8 *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, st_ie_set.ul_len, OAL_TRUE);
                if (OAL_PTR_NULL != st_ie_set.puc_content)
                {
                    /* ��ȡcontent���� */
                    ul_ret = wal_get_cmd_one_arg_etc(pc_cmd_str, ac_name, &ul_off_set);
                    if (OAL_SUCC != ul_ret)
                    {
                         OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_process_vendor_test_cmd::wal_get_cmd_one_arg_etc[content] return err_code [%d]!}\r\n", ul_ret);
                         return ul_ret;
                    }
                    if (OAL_STRLEN(ac_name) != 2*st_ie_set.ul_len)
                    {
                        OAM_WARNING_LOG0(0, OAM_SF_HILINK, "{wal_ioctl_process_vendor_test_cmd::content length error!}\r\n");
                        return -OAL_EINVAL;
                    }
                    for (uc_para_index=0; uc_para_index<st_ie_set.ul_len; uc_para_index++)
                    {
                        st_ie_set.puc_content[uc_para_index] = oal_strtohex(&ac_name[2*uc_para_index])*16 + oal_strtohex(&ac_name[2*uc_para_index+1]);
                    }
                    st_vendor_req.param.pst_ie = &st_ie_set;
                    l_ret = wal_ioctl_set_mgmt_frame_ie_test(pst_net_dev, &st_vendor_req);
                    OAL_MEM_FREE(st_ie_set.puc_content, OAL_TRUE);
                    st_ie_set.puc_content = OAL_PTR_NULL;
                }
            }
            break;
        case IEEE80211_VENDOR_SET_CAP_INFO:
            {
                 /* ��ȡ��Ҫ���õ�cap��Ϣ */
                ul_ret = wal_get_cmd_one_arg_etc(pc_cmd_str, ac_name, &ul_off_set);
                if (OAL_SUCC != ul_ret)
                {
                     OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_process_vendor_test_cmd::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
                     return ul_ret;
                }
                st_cap_info.ul_type = oal_atoi(ac_name);         /* TYPE */
                pc_cmd_str += ul_off_set;
                ul_ret = wal_get_cmd_one_arg_etc(pc_cmd_str, ac_name, &ul_off_set);
                if (OAL_SUCC != ul_ret)
                {
                     OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_process_vendor_test_cmd::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
                     return ul_ret;
                }
                st_cap_info.us_capbility = (oal_uint16)oal_atoi(ac_name);         /* cap info */
                oal_memcopy(&st_vendor_req.param.st_cap_info, &st_cap_info, OAL_SIZEOF(oal_ieee80211req_set_cap_stru));
                l_ret = wal_ioctl_set_mgmt_frame_cap_info(pst_net_dev, &st_vendor_req);
            }
            break;
    }
    return l_ret;
}

#endif

#if defined(_PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT) || defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)


oal_int32 wal_ioctl_vendor_req_get_vap_status(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru *pst_vendor_req)
{
    oal_int32                       l_ret;
    oal_vap_status_diag_info_stru      *pst_vap_diag_info;
    if (pst_vendor_req->us_len < sizeof(oal_vap_status_diag_info_stru))
    {
        OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_get_vap_diag_info:: buf_len is [%d].}", pst_vendor_req->us_len);
        return -OAL_EINVAL;
    }

    pst_vap_diag_info = (oal_vap_status_diag_info_stru *)oal_memalloc(OAL_SIZEOF(oal_vap_status_diag_info_stru));
    if (OAL_PTR_NULL == pst_vap_diag_info)
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{wal_ioctl_get_vap_diag_info::pst_vap_diag_info null.}");
        return -OAL_ERR_CODE_PTR_NULL;
    }

    l_ret = wal_ioctl_get_vap_diag_info(pst_net_dev, pst_vap_diag_info);
    oal_copy_to_user(pst_vendor_req->param.pst_vap_status, pst_vap_diag_info, OAL_SIZEOF(oal_vap_status_diag_info_stru));
    oal_free(pst_vap_diag_info);
    return l_ret;
}


oal_int32 wal_ioctl_vendor_req_get_all_sta_status(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru *pst_vendor_req)
{
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    oal_sta_status_diag_req_stru    st_sta_diag_req;
    oal_sta_status_diag_info_stru  *pst_all_sta_diag_info;

    ul_ret = oal_copy_from_user(&st_sta_diag_req, pst_vendor_req->param.pst_sta_status_req, sizeof(oal_sta_status_diag_req_stru));
    if (ul_ret > 0)
    {
        OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_get_all_sta_diag_info::oal_copy_from_user return error code %d!}\r\n", ul_ret);
        return -OAL_EFAUL;
    }

    if (st_sta_diag_req.ul_date_len < sizeof(oal_sta_status_diag_info_stru))
    {
        OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_get_all_sta_diag_info:: buf_len is [%d].}", st_sta_diag_req.ul_date_len);
        return -OAL_EINVAL;
    }
    pst_all_sta_diag_info = (oal_sta_status_diag_info_stru *)oal_memalloc(st_sta_diag_req.ul_date_len);
    if (OAL_PTR_NULL == pst_all_sta_diag_info)
    {
        OAM_WARNING_LOG1(0, OAM_SF_HILINK, "{wal_ioctl_get_all_sta_diag_info:: OAL_MALLOC failed buf len [%d].}", st_sta_diag_req.ul_date_len);
        return -OAL_EINVAL;
    }
    l_ret = wal_ioctl_get_all_sta_diag_info(pst_net_dev, &st_sta_diag_req, pst_all_sta_diag_info);
    oal_copy_to_user(st_sta_diag_req.pst_data, pst_all_sta_diag_info, (st_sta_diag_req.ul_sta_cnt * OAL_SIZEOF(oal_sta_status_diag_info_stru)));
    oal_copy_to_user(pst_vendor_req->param.pst_sta_status_req, &st_sta_diag_req, OAL_SIZEOF(oal_sta_status_diag_req_stru));
    oal_free(pst_all_sta_diag_info);
    return l_ret;
}



oal_int32 wal_ioctl_vendor_req_get_one_sta_status(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru *pst_vendor_req)
{
    oal_int32                            l_ret;
    oal_ieee80211req_action_sta_stru    *pst_action_sta;
    oal_sta_status_diag_info_stru       *pst_sta_diag_info;

    pst_action_sta = &pst_vendor_req->param.st_action_sta;
    pst_sta_diag_info = (oal_sta_status_diag_info_stru *)oal_memalloc((oal_uint16)OAL_SIZEOF(oal_sta_status_diag_info_stru));
    if (OAL_PTR_NULL == pst_sta_diag_info)
    {
        OAM_WARNING_LOG0(0, OAM_SF_HILINK, "{wal_ioctl_get_one_sta_diag_info:: OAL_MALLOC failed.}");
        return -OAL_EINVAL;
    }
    l_ret = wal_ioctl_get_one_sta_diag_info(pst_net_dev, pst_action_sta, pst_sta_diag_info);
    oal_copy_to_user(pst_action_sta->param.pst_sta_status, pst_sta_diag_info, OAL_SIZEOF(oal_sta_status_diag_info_stru));
    oal_free(pst_sta_diag_info);
    return l_ret;
}


oal_int32 wal_ioctl_vendor_req_get_sensing_bssid_status(oal_net_device_stru *pst_net_dev, oal_ieee80211_vendor_req_stru *pst_vendor_req)
{
    oal_int32                            l_ret;

    oal_ieee80211req_getset_buf_stru    *pst_getset_buf;
    oal_uint8                           *puc_sensing_buf;

    pst_getset_buf = &pst_vendor_req->param.st_getset_buf;
    if ((OAL_PTR_NULL == pst_getset_buf->puc_buf) || (pst_getset_buf->ul_buf_len < sizeof(ieee80211req_wifi_rssi_stru)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_get_sensing_bssid_info::error buf %p/buf_len %d.}\r\n",
            pst_getset_buf->puc_buf, pst_getset_buf->ul_buf_len);
        return -OAL_EINVAL;
    }

    puc_sensing_buf = oal_memalloc(pst_getset_buf->ul_buf_len);
    if (OAL_PTR_NULL == puc_sensing_buf )
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_sensing_bssid_info::oal_memalloc %d faild.}\r\n", pst_getset_buf->ul_buf_len);
        return -OAL_ENOMEM;
    }

    l_ret = wal_ioctl_get_sensing_bssid_info(pst_net_dev, pst_getset_buf, puc_sensing_buf);
    oal_copy_to_user(pst_getset_buf->puc_buf, puc_sensing_buf, pst_getset_buf->ul_buf_len);
    oal_free(puc_sensing_buf);

    return l_ret;
}


oal_int32 wal_ioctl_process_vendor_req_cmd(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    oal_iw_point_stru              *pst_param = (oal_iw_point_stru *)p_param;
    oal_ieee80211_vendor_req_stru  *pst_vendor_req;

    if ((pst_param->length < sizeof(oal_ieee80211_vendor_req_stru)) || (pc_extra == NULL))
    {
        return -OAL_EINVAL;
    }

    pst_vendor_req = (oal_ieee80211_vendor_req_stru *)oal_memalloc(pst_param->length);
    if (pst_vendor_req == OAL_PTR_NULL)
    {
        return -OAL_ENOMEM;
    }

    /*���û�̬���ݿ������ں�̬*/
    ul_ret = oal_copy_from_user(pst_vendor_req, pst_param->pointer, pst_param->length);
    if (ul_ret > 0)
    {
        oal_free(pst_vendor_req);
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_process_vendor_req_cmd::oal_copy_from_user return error code %d!}\r\n", ul_ret);
        return -OAL_EFAUL;
    }

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_process_vendor_req_cmd::input str length is %d!}\r\n", pst_param->length);

    l_ret = -OAL_EINVAL;
    switch(pst_vendor_req->us_cmd)
    {
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
        case IEEE80211_VENDOR_ADD_VENDOR_IE:
            l_ret = wal_ioctl_set_hilink_ie(pst_net_dev, pst_vendor_req);
            break;
        case IEEE80211_VENDOR_MGMT_FILTER:
            l_ret = wal_ioctl_set_mgmt_frame_filters(pst_net_dev, pst_vendor_req);
            break;
        case IEEE80211_VENDOR_ADD_DETECT:
        case IEEE80211_VENDOR_REMOVE_DETECT:
            l_ret = wal_ioctl_start_fbt_iwpriv_scan(pst_net_dev, pst_vendor_req, pst_vendor_req->us_cmd);
            break;
        case IEEE80211_VENDOR_ADD_REJECT_STA:
        case IEEE80211_VENDOR_REMOVE_REJECT_STA:
            l_ret = wal_ioctl_set_hiden_blacklist(pst_net_dev, pst_vendor_req, pst_vendor_req->us_cmd);
            break;
        case IEEE80211_VENDOR_ADD_ACL_EXCEPT:
        case IEEE80211_VENDOR_REMOVE_ACL_EXCEPT:
            l_ret = wal_ioctl_set_hiden_whitelist(pst_net_dev, pst_vendor_req, pst_vendor_req->us_cmd);
            break;
        case IEEE80211_VENDOR_GET_VAP_STATUS:
            l_ret = wal_ioctl_vendor_req_get_vap_status(pst_net_dev, pst_vendor_req);
            break;
        case IEEE80211_VENDOR_GET_STA_STATUS:
            l_ret = wal_ioctl_vendor_req_get_all_sta_status(pst_net_dev, pst_vendor_req);
            break;
        case IEEE80211_VENDOR_GET_ONE_STA_STATUS:
            l_ret = wal_ioctl_vendor_req_get_one_sta_status(pst_net_dev, pst_vendor_req);
            break;
        case IEEE80211_VENDOR_ADD_SENSING_MAC:
        case IEEE80211_VENDOR_REMOVE_SENSING_MAC:
            l_ret = wal_ioctl_set_sensing_bssid(pst_net_dev, pst_vendor_req, pst_vendor_req->us_cmd);
            break;
        case IEEE80211_VENDOR_GET_SENSING_AP_RSSI:
            l_ret = wal_ioctl_vendor_req_get_sensing_bssid_status(pst_net_dev, pst_vendor_req);
            break;
#endif
#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE
        case IEEE80211_VENDOR_SEND_RAW:
            l_ret = wal_ioctl_send_action_frame(pst_net_dev, pst_vendor_req);
            break;
        case IEEE80211_VENDOR_DISASSOC_STA:
            l_ret = wal_ioctl_disassoc_sta(pst_net_dev, pst_vendor_req);
            break;
        case IEEE80211_VENDOR_GET_REG_DOMAIN:
            {
                l_ret = wal_ioctl_get_reg_domain(pst_net_dev, pst_vendor_req);
                break;
            }
        case IEEE80211_VENDOR_SET_IE:
            l_ret = wal_ioctl_set_mgmt_frame_ie(pst_net_dev, pst_vendor_req);
            break;
        case IEEE80211_VENDOR_SET_CAP_INFO:
            l_ret = wal_ioctl_set_mgmt_frame_cap_info(pst_net_dev, pst_vendor_req);
            break;
#endif
        default:
            break;
    }

    oal_copy_to_user(pst_param->pointer, pst_vendor_req, pst_param->length);
    oal_free(pst_vendor_req);
    return l_ret;
}
#endif // defined(_PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT) || defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)


oal_int32 wal_ioctl_get_sta_11h_ability(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    oal_int32                       l_ret;
    oal_uint32                      ul_err_code;
    wal_msg_write_stru              st_write_msg;
    wal_msg_stru                   *pst_rsp_msg;
    oal_hilink_get_sta_11h_ability *pst_get_sta_11h_ability;
    oal_ulong                      *pul_temp;

    pst_get_sta_11h_ability = (oal_hilink_get_sta_11h_ability*)(st_write_msg.auc_value);
    oal_memcopy(pst_get_sta_11h_ability, &(pst_ioctl_data->pri_data.fbt_get_sta_11h_ability), OAL_SIZEOF(oal_hilink_get_sta_11h_ability));
    pul_temp  = (oal_ulong*)(pst_get_sta_11h_ability + 1);
    *pul_temp = (oal_ulong)(st_write_msg.auc_value);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, (oal_uint16)WLAN_CFGID_FBT_GET_STA_11H_ABILITY, OAL_SIZEOF(oal_hilink_get_sta_11h_ability) + OAL_SIZEOF(pul_temp));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               (oal_uint8)WAL_MSG_TYPE_WRITE,
                               (oal_uint16)(WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_hilink_get_sta_11h_ability) + OAL_SIZEOF(pul_temp)),
                               (oal_uint8 *)&st_write_msg,
                               (oal_uint8)OAL_TRUE,
                               &pst_rsp_msg);
    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_ioctl_get_sta_11h_ability:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if (OAL_SUCC != ul_err_code)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_ioctl_get_sta_11h_ability::wal_send_cfg_event_etc return err code:[%x]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }
    pst_ioctl_data->pri_data.fbt_get_sta_11h_ability.en_support_11h = pst_get_sta_11h_ability->en_support_11h;

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11R_AP

oal_int32 wal_ioctl_get_sta_11r_ability(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    oal_int32                       l_ret;
    oal_uint32                      ul_err_code;
    wal_msg_write_stru              st_write_msg;
    wal_msg_stru                   *pst_rsp_msg;
    oal_hilink_get_sta_11r_ability *pst_get_sta_11r_ability;
    oal_ulong                      *pul_temp;

    pst_get_sta_11r_ability = (oal_hilink_get_sta_11r_ability*)(st_write_msg.auc_value);
    oal_memcopy(pst_get_sta_11r_ability,&(pst_ioctl_data->pri_data.fbt_get_sta_11r_ability),OAL_SIZEOF(oal_hilink_get_sta_11r_ability));
    pul_temp  = (oal_ulong*)(pst_get_sta_11r_ability + 1);
    *pul_temp = (oal_ulong)(st_write_msg.auc_value);
    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, (oal_uint16)WLAN_CFGID_FBT_GET_STA_11R_ABILITY, OAL_SIZEOF(oal_hilink_get_sta_11r_ability) + OAL_SIZEOF(pul_temp));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               (oal_uint8)WAL_MSG_TYPE_WRITE,
                               (oal_uint16)(WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_hilink_get_sta_11r_ability) + OAL_SIZEOF(pul_temp)),
                               (oal_uint8 *)&st_write_msg,
                               (oal_uint8)OAL_TRUE,
                               &pst_rsp_msg);
    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_ioctl_get_sta_11r_ability:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if (OAL_SUCC != ul_err_code)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_ioctl_get_sta_11r_ability::wal_send_cfg_event_etc return err code:[%x]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_11R_AP

oal_int32 wal_ioctl_set_mlme(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    wal_msg_write_stru               st_write_msg;
    oal_mlme_ie_stru                *pst_mlme_ie = OAL_PTR_NULL;
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                       ul_err_code;
    oal_int32                        l_ret = 0;

    pst_mlme_ie = (oal_mlme_ie_stru *)(st_write_msg.auc_value);

    pst_mlme_ie->en_mlme_type   = pst_ioctl_data->pri_data.set_mlme.en_mlme_type;
    pst_mlme_ie->uc_seq         = pst_ioctl_data->pri_data.set_mlme.uc_seq;
    pst_mlme_ie->us_reason      = pst_ioctl_data->pri_data.set_mlme.us_reason;

    oal_memcopy(pst_mlme_ie->auc_macaddr, pst_ioctl_data->pri_data.set_mlme.auc_macaddr, OAL_MAC_ADDR_LEN);

    pst_mlme_ie->us_optie_len = pst_ioctl_data->pri_data.set_mlme.us_optie_len;
    oal_memcopy(pst_mlme_ie->auc_optie, pst_ioctl_data->pri_data.set_mlme.auc_optie, pst_mlme_ie->us_optie_len);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_MLME, OAL_SIZEOF(oal_mlme_ie_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_mlme_ie_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_mlme:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_mlme::wal_send_cfg_event_etc return err code:[%x]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;

}
#endif


oal_int32 wal_ioctl_set_channel(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    mac_cfg_channel_param_stru          *pst_channel_param;
    wal_msg_write_stru                   st_write_msg;
    mac_device_stru                     *pst_device;
    mac_vap_stru                        *pst_mac_vap;
    oal_wiphy_stru                      *pst_wiphy;
    oal_ieee80211_channel_stru          *pst_channel;
    wlan_channel_bandwidth_enum_uint8    en_bandwidth;
    oal_int32                            l_freq;
    oal_int32                            l_channel;
    oal_uint8                            uc_vht_width;
    oal_int32                            l_sec_channel_offset;
    oal_int32                            l_center_freq1;
    oal_int32                            l_center_freq2;
    oal_int32                            l_bandwidth;
    oal_int32                            l_ret;
    wal_msg_stru                        *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                          ul_err_code;
#ifdef _PRE_WLAN_FEATURE_DFS
    oal_uint8                           uc_chan_idx = 0;
#endif
    l_freq               = pst_ioctl_data->pri_data.freq.l_freq;
    l_channel            = pst_ioctl_data->pri_data.freq.l_channel;
    l_sec_channel_offset = pst_ioctl_data->pri_data.freq.l_sec_channel_offset;
    l_center_freq1       = oal_ieee80211_frequency_to_channel(pst_ioctl_data->pri_data.freq.l_center_freq1);
    l_center_freq2       = oal_ieee80211_frequency_to_channel(pst_ioctl_data->pri_data.freq.l_center_freq2);
    l_bandwidth          = pst_ioctl_data->pri_data.freq.l_bandwidth;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_channel::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}\r\n");
        return -OAL_EINVAL;
    }

    pst_device  = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_channel::pst_device is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_wiphy   = pst_device->pst_wiphy;
    pst_channel = oal_ieee80211_get_channel(pst_wiphy, l_freq);
    l_channel   = pst_channel->hw_value;
    uc_vht_width = wal_cfg80211_convert_value_to_vht_width(l_bandwidth);

    /* �ж��ŵ��ڲ��ڹ������� */
    l_ret = (oal_int32)mac_is_channel_num_valid_etc((oal_uint8)pst_channel->band, (oal_uint8)l_channel);
    if (l_ret != OAL_SUCC)
    {
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_channel::channel num is invalid. band, ch num [%d] [%d]!}\r\n", pst_channel->band, l_channel);
        return -OAL_EINVAL;
    }

    /* �����ں˴���ֵ��WITP ����ֵת�� */
    if (80 == l_bandwidth || 160 == l_bandwidth)
    {
        en_bandwidth = mac_get_bandwith_from_center_freq_seg0_seg1(uc_vht_width, (oal_uint8)l_channel, (oal_uint8)l_center_freq1, (oal_uint8)l_center_freq2);
    }
    else if (40 == l_bandwidth)
    {
        switch (l_sec_channel_offset) {
            case -1:
                en_bandwidth     = WLAN_BAND_WIDTH_40MINUS;
                break;
            case 1:
                en_bandwidth     = WLAN_BAND_WIDTH_40PLUS;
                break;
            default:
                en_bandwidth     = WLAN_BAND_WIDTH_20M;
                break;
        }
    }
    else
    {
        en_bandwidth     = WLAN_BAND_WIDTH_20M;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/

    /* ��д��Ϣ */
    pst_channel_param = (mac_cfg_channel_param_stru *)(st_write_msg.auc_value);
    pst_channel_param->uc_channel   = (oal_uint8)pst_channel->hw_value;
    pst_channel_param->en_band      = pst_channel->band;
    pst_channel_param->en_bandwidth = en_bandwidth;

#ifdef _PRE_WLAN_FEATURE_DFS
    /* ������õ��ŵ����״�ռ�ã��������� */
    ul_err_code = mac_get_channel_idx_from_num_etc(pst_channel_param->en_band, pst_channel_param->uc_channel, &uc_chan_idx);
    if (OAL_ERR_CODE_INVALID_CONFIG != ul_err_code && MAC_CHAN_BLOCK_DUE_TO_RADAR  == pst_device->st_ap_channel_list[uc_chan_idx].en_ch_status)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_ioctl_set_channel::The channel %d is used by radar.\n", pst_channel_param->uc_channel);
        return OAL_FAIL;
    }
#endif

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_SET_CHANNEL, OAL_SIZEOF(mac_cfg_channel_param_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_channel_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_channel:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_channel::wal_check_and_release_msg_resp_etc fail return err code:[%d]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

static oal_uint32 wal_ioctl_parse_wps_p2p_ie(oal_app_ie_stru *pst_app_ie, oal_uint8 *puc_src, oal_uint32 ul_src_len)
{
    oal_uint8                      *puc_ie             = OAL_PTR_NULL;
    oal_uint32                      ul_ie_len;
    oal_uint8                      *puc_buf_remain;
    oal_uint32                      ul_len_remain;

    if(OAL_PTR_NULL == pst_app_ie || OAL_PTR_NULL == puc_src)
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG, "{wal_ioctl_parse_wps_p2p_ie::param is NULL, pst_app_ie=[%p], puc_src=[%p]!}\r\n",pst_app_ie, puc_src);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if(0 == ul_src_len || WLAN_WPS_IE_MAX_SIZE < ul_src_len)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_ioctl_parse_wps_p2p_ie::ul_src_len=[%d] is invailid!}\r\n",ul_src_len);
        return OAL_FAIL;
    }

    pst_app_ie->ul_ie_len    = 0;
    puc_buf_remain           = puc_src;
    ul_len_remain            = ul_src_len;

    while(ul_len_remain > MAC_IE_HDR_LEN)
    {
        /*MAC_EID_WPS,MAC_EID_P2P ID��Ϊ221 */
        puc_ie = mac_find_ie_etc(MAC_EID_P2P, puc_buf_remain, (oal_int32)ul_len_remain);
        if(OAL_PTR_NULL != puc_ie)
        {
            ul_ie_len = (oal_uint8)puc_ie[1] + MAC_IE_HDR_LEN;
            if((ul_ie_len > (WLAN_WPS_IE_MAX_SIZE - pst_app_ie->ul_ie_len)) ||
               (ul_src_len < ((oal_uint16)(puc_ie - puc_src) + ul_ie_len)))
            {
                OAM_WARNING_LOG3(0, OAM_SF_CFG, "{wal_ioctl_parse_wps_p2p_ie::uc_ie_len=[%d], left buffer sieze=[%d], src_end_len=[%d],param invalid!}\r\n",
                ul_ie_len, WLAN_WPS_IE_MAX_SIZE - pst_app_ie->ul_ie_len, puc_ie - puc_src + ul_ie_len);
                return OAL_FAIL;
            }
            oal_memcopy(&(pst_app_ie->auc_ie[pst_app_ie->ul_ie_len]), puc_ie, ul_ie_len);
            pst_app_ie->ul_ie_len  += ul_ie_len;
            puc_buf_remain          = puc_ie + ul_ie_len;
            ul_len_remain           = ul_src_len -(oal_uint32)(puc_buf_remain - puc_src);
        }
        else
        {
            break;
        }
    }

    if(pst_app_ie->ul_ie_len > 0)
    {
        return OAL_SUCC;
    }

    return OAL_FAIL;
}


oal_int32 wal_ioctl_set_wps_p2p_ie_etc(oal_net_device_stru  *pst_net_dev,
                                    oal_uint8           *puc_buf,
                                    oal_uint32           ul_len,
                                    en_app_ie_type_uint8 en_type)
{
    wal_msg_write_stru              st_write_msg;
    oal_app_ie_stru                 st_app_ie;
    oal_uint32                      ul_err_code;
    oal_int32                       l_ret              = 0;
    wal_msg_stru                   *pst_rsp_msg        = OAL_PTR_NULL;
    oal_w2h_app_ie_stru            *pst_w2h_wps_p2p_ie = OAL_PTR_NULL;
    mac_vap_stru                   *pst_mac_vap;

    if (WLAN_WPS_IE_MAX_SIZE < ul_len)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_wps_p2p_ie_etc:: wrong ul_len: [%u]!}\r\n",
                            ul_len);
        return -OAL_EFAIL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_ioctl_set_wps_p2p_ie_etc::pst_mac_vap is null}");
        return -OAL_EINVAL;
    }

    OAL_MEMZERO(&st_app_ie, OAL_SIZEOF(oal_app_ie_stru));
    switch (en_type) {
        case OAL_APP_BEACON_IE:
        case OAL_APP_PROBE_RSP_IE:
        case OAL_APP_ASSOC_RSP_IE:
            st_app_ie.en_app_ie_type = en_type;
            break;
        default:
            OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_wps_p2p_ie_etc:: wrong type: [%x]!}\r\n",
                            en_type);
            return -OAL_EFAIL;
    }

    if(OAL_FALSE == IS_LEGACY_VAP(pst_mac_vap))
    {
        if(OAL_SUCC != wal_ioctl_parse_wps_p2p_ie(&st_app_ie, puc_buf , ul_len))
        {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_wps_p2p_ie_etc::Type=[%d], parse p2p ie fail,!}\r\n", en_type);
            return -OAL_EFAIL;
        }
    }
    else
    {
        oal_memcopy(st_app_ie.auc_ie, puc_buf, ul_len);
        st_app_ie.ul_ie_len = ul_len;
    }

    OAM_WARNING_LOG3(0, OAM_SF_CFG, "{wal_ioctl_set_wps_p2p_ie_etc::p2p_ie_type=[%d], ul_len=[%d], st_app_ie.ul_ie_len=[%d]!}\r\n",
    en_type, ul_len, st_app_ie.ul_ie_len);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    pst_w2h_wps_p2p_ie                 = (oal_w2h_app_ie_stru *)st_write_msg.auc_value;
    pst_w2h_wps_p2p_ie->en_app_ie_type = st_app_ie.en_app_ie_type;
    pst_w2h_wps_p2p_ie->ul_ie_len      = st_app_ie.ul_ie_len;
    pst_w2h_wps_p2p_ie->puc_data_ie    = st_app_ie.auc_ie;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_WPS_P2P_IE, OAL_SIZEOF(oal_w2h_app_ie_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_w2h_app_ie_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_wps_p2p_ie_etc:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_wps_p2p_ie_etc::wal_check_and_release_msg_resp_etc fail return err code: [%d]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
OAL_STATIC oal_int32  wal_ioctl_set_p2p_miracast_status(oal_net_device_stru *pst_net_dev,oal_uint8 uc_param)
{
    wal_msg_write_stru           st_write_msg;
    oal_int32                    l_ret;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_P2P_MIRACAST_STATUS, OAL_SIZEOF(oal_uint8));

    st_write_msg.auc_value[0] = uc_param;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_miracast_status::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_miracast_status::uc_param[%d].}", uc_param);
    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_set_p2p_noa(oal_net_device_stru *pst_net_dev, mac_cfg_p2p_noa_param_stru *pst_p2p_noa_param)
{
    wal_msg_write_stru              st_write_msg;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                      ul_err_code;
    oal_int32                       l_ret = 0;

    oal_memcopy(st_write_msg.auc_value, pst_p2p_noa_param, OAL_SIZEOF(mac_cfg_p2p_noa_param_stru));

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_P2P_PS_NOA, OAL_SIZEOF(mac_cfg_p2p_noa_param_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_p2p_noa_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_p2p_noa:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_p2p_noa::wal_check_and_release_msg_resp_etc fail return err code:  [%d]!}\r\n",
                           ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_set_p2p_ops(oal_net_device_stru *pst_net_dev, mac_cfg_p2p_ops_param_stru *pst_p2p_ops_param)
{
    wal_msg_write_stru              st_write_msg;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                      ul_err_code;
    oal_int32                       l_ret = 0;

    oal_memcopy(st_write_msg.auc_value, pst_p2p_ops_param, OAL_SIZEOF(mac_cfg_p2p_ops_param_stru));

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_P2P_PS_OPS, OAL_SIZEOF(mac_cfg_p2p_ops_param_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_p2p_ops_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_p2p_ops:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_p2p_ops::wal_check_and_release_msg_resp_etc fail return err code:[%d]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_HS20

OAL_STATIC oal_int32 wal_ioctl_set_qos_map(oal_net_device_stru *pst_net_dev, hmac_cfg_qos_map_param_stru *pst_qos_map_param)
{
    wal_msg_write_stru              st_write_msg;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                      ul_err_code;
    oal_int32                       l_ret = 0;

    oal_memcopy(st_write_msg.auc_value, pst_qos_map_param, OAL_SIZEOF(hmac_cfg_qos_map_param_stru));

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_QOS_MAP, OAL_SIZEOF(hmac_cfg_qos_map_param_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(hmac_cfg_qos_map_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_HS20, "{wal_ioctl_set_qos_map:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_HS20, "{wal_ioctl_set_qos_map::wal_check_and_release_msg_resp_etc return err code: [%x!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
#endif // _PRE_WLAN_FEATURE_HS20

#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) && (!defined(_PRE_PRODUCT_ID_HI110X_HOST)))

oal_int32 wal_ioctl_set_wps_ie(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    wal_msg_write_stru              st_write_msg;
    oal_app_ie_stru                *pst_wps_ie = OAL_PTR_NULL;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                      ul_err_code;
    oal_int32                       l_ret = 0;

    pst_wps_ie = (oal_app_ie_stru *)st_write_msg.auc_value;
    pst_wps_ie->ul_ie_len      = pst_ioctl_data->pri_data.st_app_ie.ul_ie_len;
    pst_wps_ie->en_app_ie_type = pst_ioctl_data->pri_data.st_app_ie.en_app_ie_type;
    if (pst_wps_ie->ul_ie_len > WLAN_WPS_IE_MAX_SIZE)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_wps_ie::invalid ie len:[%d]!}\r\n",
                        pst_wps_ie->ul_ie_len);
        return -OAL_EINVAL;
    }

    oal_memcopy(pst_wps_ie->auc_ie, pst_ioctl_data->pri_data.st_app_ie.auc_ie, pst_wps_ie->ul_ie_len);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_WPS_IE, OAL_SIZEOF(oal_app_ie_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_app_ie_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_wps_ie:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_wps_ie::wal_check_and_release_msg_resp_etc return err code:[%x]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


oal_int32 wal_ioctl_set_frag(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    wal_msg_write_stru              st_write_msg;
    mac_cfg_frag_threshold_stru    *pst_threshold;
    oal_uint32                      ul_threshold = 0;
    oal_uint16                      us_len;
    oal_int32                       l_ret = 0;

    /* ��ȡ��Ƭ���� */
    ul_threshold = (oal_uint32)pst_ioctl_data->pri_data.l_frag;

    pst_threshold = (mac_cfg_frag_threshold_stru *)(st_write_msg.auc_value);
    pst_threshold->ul_frag_threshold = ul_threshold;

    /***************************************************************************
                              ���¼���wal�㴦��
    ***************************************************************************/
    us_len = OAL_SIZEOF(mac_cfg_frag_threshold_stru);
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_FRAG_THRESHOLD_REG, us_len);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                             (oal_uint8 *)&st_write_msg,
                             OAL_FALSE,
                             OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_frag::return err code [%d]!}\r\n", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}


oal_int32 wal_ioctl_set_rts(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    wal_msg_write_stru              st_write_msg;
    mac_cfg_rts_threshold_stru     *pst_threshold;
    oal_uint32                      ul_threshold = 0;
    oal_uint16                      us_len;
    oal_int32                       l_ret = 0;

    /* ��ȡ��Ƭ���� */
    ul_threshold = (oal_uint32)pst_ioctl_data->pri_data.l_rts;

    pst_threshold = (mac_cfg_rts_threshold_stru *)(st_write_msg.auc_value);
    pst_threshold->ul_rts_threshold = ul_threshold;

    OAM_INFO_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_rts::rts [%d]!}\r\n", ul_threshold);
    /***************************************************************************
                              ���¼���wal�㴦��
    ***************************************************************************/
    us_len = OAL_SIZEOF(mac_cfg_rts_threshold_stru);
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_RTS_THRESHHOLD, us_len);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                             (oal_uint8 *)&st_write_msg,
                             OAL_FALSE,
                             OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_rts::return err code [%d]!}\r\n", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

OAL_STATIC oal_int32 wal_ioctl_reduce_sar(oal_net_device_stru *pst_net_dev, oal_uint16 ul_tx_power)
{
#ifdef _PRE_WLAN_FEATURE_TPC_OPT
    oal_uint8                   uc_lvl_idx = 0;
    oal_int32                   l_ret;
    wal_msg_write_stru          st_write_msg;
    oal_uint8                   auc_sar_ctrl_params[CUS_NUM_OF_SAR_PARAMS];
    oal_uint8                  *puc_sar_ctrl_params;

    OAM_WARNING_LOG1(0, OAM_SF_TPC, "wal_ioctl_reduce_sar::supplicant set tx_power[%d] for reduce SAR purpose.\r\n", ul_tx_power);

    /***************************************************************************
        ����10XX�����ϲ��·��Ľ�SAR��λ��
        ��ǰ��λ������"����WiFi���߽�SAR sensor��
        �������ֵ�WiFi������WiFi��Modemһ����"��Ԥ����
        ����Ҫ1001~1020����
        ����        ��λ          ����0        ����1    ����2��RPC��   ����3(Ant�Ƿ�SAR sensor����)
                              �Ƿ����Ƶͬ��                           Ant1 Ant3
        Head SAR    ��λ1001        N           CE��    receiver on     NA  NA
                    ��λ1002        Y           CE��    receiver on     NA  NA
                    ��λ1003        N           FCC��   receiver on     NA  NA
                    ��λ1004        Y           FCC��   receiver on     NA  NA
        -------------------------------------------------------------------------
        Body SAR    ��λ1005        N           CE��    receiver off    0   0
                    ��λ1006        N           CE��    receiver off    0   1
                    ��λ1007        N           CE��    receiver off    1   0
                    ��λ1008        N           CE��    receiver off    1   1
                    ��λ1009        Y           CE��    receiver off    0   0
                    ��λ1010        Y           CE��    receiver off    0   1
                    ��λ1011        Y           CE��    receiver off    1   0
                    ��λ1012        Y           CE��    receiver off    1   1
                    ��λ1013        N           FCC��   receiver off    0   0
                    ��λ1014        N           FCC��   receiver off    0   1
                    ��λ1015        N           FCC��   receiver off    1   0
                    ��λ1016        N           FCC��   receiver off    1   1
                    ��λ1017        Y           FCC��   receiver off    0   0
                    ��λ1018        Y           FCC��   receiver off    0   1
                    ��λ1019        Y           FCC��   receiver off    1   0
                    ��λ1020        Y           FCC��   receiver off    1   1
    ***************************************************************************/

    if ((ul_tx_power >= 1001) && (ul_tx_power <= 1020))
    {
        uc_lvl_idx = ul_tx_power - 1000;
    }
    puc_sar_ctrl_params = wal_get_reduce_sar_ctrl_params(uc_lvl_idx);
    if (OAL_PTR_NULL == puc_sar_ctrl_params)
    {
        oal_memset(auc_sar_ctrl_params, 0xFF, OAL_SIZEOF(auc_sar_ctrl_params));
        puc_sar_ctrl_params = auc_sar_ctrl_params;
    }

    /* vapδ����ʱ��������supplicant���� */
    if (OAL_PTR_NULL == OAL_NET_DEV_PRIV(pst_net_dev))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "wal_ioctl_reduce_sar::vap not created yet, ignore the cmd!");
        return -OAL_EINVAL;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_REDUCE_SAR, OAL_SIZEOF(oal_uint8)*CUS_NUM_OF_SAR_PARAMS);
    oal_memcopy(st_write_msg.auc_value, puc_sar_ctrl_params, OAL_SIZEOF(oal_uint8)*CUS_NUM_OF_SAR_PARAMS);
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                       WAL_MSG_TYPE_WRITE,
                       WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int8)*CUS_NUM_OF_SAR_PARAMS,
                       (oal_uint8 *)&st_write_msg,
                       OAL_FALSE,
                       OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_ioctl_reduce_sar::wal_send_cfg_event_etc failed, error no[%d]!\r\n", l_ret);
        return l_ret;
    }
#endif
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH

OAL_STATIC oal_int32 wal_ioctl_tas_pow_ctrl(oal_net_device_stru *pst_net_dev, oal_uint8 uc_coreindex,
                                                                     oal_bool_enum_uint8 en_needImprove)
{
#ifdef _PRE_WLAN_FEATURE_TPC_OPT
    oal_int32                   l_ret;
    wal_msg_write_stru          st_write_msg;
    mac_cfg_tas_pwr_ctrl_stru   st_tas_pow_ctrl_params;
    mac_device_stru             *pst_mac_device;

    if (OAL_FALSE == g_aen_tas_switch_en[uc_coreindex])
    {
        /* ��ǰ���߲�֧��TAS�л����� */
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_ioctl_tas_pow_ctrl::core[%d] is not supported!", uc_coreindex);
        return OAL_SUCC;
    }

    st_tas_pow_ctrl_params.en_need_improved = en_needImprove;
    st_tas_pow_ctrl_params.uc_core_idx      = uc_coreindex;

    /* vapδ����ʱ��������supplicant���� */
    if (OAL_PTR_NULL == OAL_NET_DEV_PRIV(pst_net_dev))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "wal_ioctl_tas_pow_ctrl::vap not created yet, ignore the cmd!");
        return -OAL_EINVAL;
    }

    pst_mac_device = mac_res_get_dev_etc(0);
    /* ����ǵ�VAP,�򲻴��� */
    if (1 != mac_device_calc_up_vap_num_etc(pst_mac_device))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "wal_ioctl_tas_pow_ctrl::abort for more than 1 vap");
        return OAL_SUCC;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_TAS_PWR_CTRL, OAL_SIZEOF(mac_cfg_tas_pwr_ctrl_stru));
    oal_memcopy(st_write_msg.auc_value, &st_tas_pow_ctrl_params, OAL_SIZEOF(mac_cfg_tas_pwr_ctrl_stru));
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                       WAL_MSG_TYPE_WRITE,
                       WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_tas_pwr_ctrl_stru),
                       (oal_uint8 *)&st_write_msg,
                       OAL_FALSE,
                       OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_ioctl_tas_pow_ctrl::wal_send_cfg_event_etc failed, error no[%d]!\r\n", l_ret);
        return l_ret;
    }
#endif
    return OAL_SUCC;
}


 
 OAL_STATIC oal_int32 wal_ioctl_tas_rssi_access(oal_net_device_stru *pst_net_dev, oal_uint8 uc_coreindex)
 {
     oal_int32                   l_ret;
     wal_msg_write_stru          st_write_msg;

     /* vapδ����ʱ��������supplicant���� */
     if (OAL_PTR_NULL == OAL_NET_DEV_PRIV(pst_net_dev))
     {
         OAM_WARNING_LOG0(0, OAM_SF_CFG, "wal_ioctl_tas_rssi_access::vap not created yet, ignore the cmd!");
         return -OAL_EINVAL;
     }

     if (OAL_FALSE == g_aen_tas_switch_en[uc_coreindex])
     {
         /* ��ǰ���߲�֧��TAS�л����� */
         OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_ioctl_tas_rssi_access::core[%d] is not supported!", uc_coreindex);
         return OAL_SUCC;
     }

     /***************************************************************************
                                 ���¼���wal�㴦��
     ***************************************************************************/
     WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_TAS_RSSI_ACCESS, OAL_SIZEOF(oal_uint8));
     st_write_msg.auc_value[0] = uc_coreindex;
     l_ret = wal_send_cfg_event_etc(pst_net_dev,
                        WAL_MSG_TYPE_WRITE,
                        WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                        (oal_uint8 *)&st_write_msg,
                        OAL_FALSE,
                        OAL_PTR_NULL);
     if (OAL_UNLIKELY(OAL_SUCC != l_ret))
     {
         OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_ioctl_tas_rssi_access::wal_send_cfg_event_etc failed, error no[%d]!\r\n", l_ret);
         return l_ret;
     }

     return OAL_SUCC;
 }
#endif

#endif
#endif

OAL_STATIC oal_uint32 wal_get_parameter_from_cmd(oal_int8 *pc_cmd, oal_int8 *pc_arg, OAL_CONST oal_int8 *puc_token, oal_uint32 *pul_cmd_offset, oal_uint32 ul_param_max_len)
{
    oal_int8   *pc_cmd_copy;
    oal_int8    ac_cmd_copy[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN];
    oal_uint32  ul_pos = 0;
    oal_uint32  ul_arg_len;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pc_cmd) || (OAL_PTR_NULL == pc_arg) || (OAL_PTR_NULL == pul_cmd_offset)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_get_parameter_from_cmd::pc_cmd/pc_arg/pul_cmd_offset null ptr error %d, %d, %d, %d!}\r\n", pc_cmd, pc_arg, pul_cmd_offset);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pc_cmd_copy = pc_cmd;

    /* ȥ���ַ�����ʼ�Ķ��� */
    while (',' == *pc_cmd_copy)
    {
        ++pc_cmd_copy;
    }
    /* ȡ�ö���ǰ���ַ��� */
    while ((',' != *pc_cmd_copy) && ('\0' != *pc_cmd_copy))
    {
        ac_cmd_copy[ul_pos] = *pc_cmd_copy;
        ++ul_pos;
        ++pc_cmd_copy;

        if (OAL_UNLIKELY(ul_pos >= ul_param_max_len))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_get_parameter_from_cmd::ul_pos >= WAL_HIPRIV_CMD_NAME_MAX_LEN, ul_pos %d!}\r\n", ul_pos);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }
    ac_cmd_copy[ul_pos]  = '\0';
    /* �ַ�������β�����ش����� */
    if (0 == ul_pos)
    {
        OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_get_parameter_from_cmd::return param pc_arg is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    *pul_cmd_offset = (oal_uint32)(pc_cmd_copy - pc_cmd);

    /* ����ַ����Ƿ����������ǰ�������ַ�*/
    if (0 != oal_memcmp(ac_cmd_copy, puc_token, OAL_STRLEN(puc_token)))
    {
        return OAL_FAIL;
    }
    else
    {
        /* �۳�ǰ�������ַ����ش�����*/
        ul_arg_len = OAL_STRLEN(ac_cmd_copy) - OAL_STRLEN(puc_token);
        oal_memcopy(pc_arg, ac_cmd_copy + OAL_STRLEN(puc_token), ul_arg_len);
        pc_arg[ul_arg_len]  = '\0';
    }
    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_set_ap_max_user(oal_net_device_stru *pst_net_dev, oal_uint32 ul_ap_max_user)
{
    wal_msg_write_stru          st_write_msg;
    wal_msg_stru               *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                  ul_err_code;
    oal_int32                   l_ret;

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_set_ap_max_user:: ap_max_user is : %u.}\r\n", ul_ap_max_user);

    if(ul_ap_max_user == 0)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_set_ap_max_user::invalid ap max user(%u),ignore this set.}\r\n", ul_ap_max_user);
        return OAL_SUCC;
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_MAX_USER, OAL_SIZEOF(ul_ap_max_user));
     *((oal_uint32 *)st_write_msg.auc_value) = ul_ap_max_user;
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(ul_ap_max_user),
                                (oal_uint8 *)&st_write_msg,
                                OAL_TRUE,
                                &pst_rsp_msg);

    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_set_ap_max_user:: wal_send_cfg_event_etc return err code %d!}\r\n", l_ret);

        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_set_ap_max_user::wal_check_and_release_msg_resp_etc return err code: [%d]!}\r\n",
                           ul_err_code);
        return -OAL_EFAIL;
    }
    /* ÿ����������û�����ɺ󣬶����Ϊ�Ƿ�ֵ0 **/
    //g_st_ap_config_info.ul_ap_max_user = 0;

    return l_ret;
}


OAL_STATIC oal_int32 wal_config_mac_filter(oal_net_device_stru *pst_net_dev, oal_int8 *pc_command)
{
    oal_int8                    ac_parsed_command[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN];
    oal_int8                   *pc_parse_command;
    oal_uint32                  ul_mac_mode;
    oal_uint32                  ul_mac_cnt = 0;
    oal_uint32                  ul_i;
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
    wal_msg_write_stru          st_write_msg;
    oal_uint16                  us_len;
    wal_msg_stru               *pst_rsp_msg = OAL_PTR_NULL;
   oal_uint32                   ul_err_code;
    oal_int32                   l_ret = 0;
    mac_blacklist_stru         *pst_blklst;
#endif
    oal_uint32                  ul_ret = 0;
    oal_uint32                  ul_off_set;


    if (OAL_PTR_NULL == pc_command)
    {
        return -OAL_EINVAL;
    }
    pc_parse_command = pc_command;

    /* ����MAC_MODE*/
    ul_ret = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC_MODE=", &ul_off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_config_mac_filter::wal_get_parameter_from_cmd return err_code %u.}\r\n", ul_ret);
        return -OAL_EINVAL;
    }
    /* �������Ƿ�Ϸ� 0,1,2*/
    ul_mac_mode = (oal_uint32)oal_atoi(ac_parsed_command);
    if(ul_mac_mode > 2)
    {
        OAM_WARNING_LOG4(0, OAM_SF_ANY, "{wal_config_mac_filter::invalid MAC_MODE[%c%c%c%c]!}\r\n",
                    (oal_uint8)ac_parsed_command[0],
                    (oal_uint8)ac_parsed_command[1],
                    (oal_uint8)ac_parsed_command[2],
                    (oal_uint8)ac_parsed_command[3]);
        return -OAL_EINVAL;
    }

    /* ���ù���ģʽ*/
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
    ul_ret = wal_hipriv_send_cfg_uint32_data_etc(pst_net_dev,ac_parsed_command, WLAN_CFGID_BLACKLIST_MODE);
    if(OAL_SUCC != ul_ret)
    {
        return (oal_int32)ul_ret;
    }
#endif
    /* ����MAC_CNT*/
    pc_parse_command += ul_off_set;
    ul_ret = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC_CNT=", &ul_off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_config_mac_filter::wal_get_parameter_from_cmd return err_code [%u]!}\r\n", ul_ret);
        return -OAL_EINVAL;
    }
    ul_mac_cnt = (oal_uint32)oal_atoi(ac_parsed_command);

    for (ul_i = 0; ul_i < ul_mac_cnt; ul_i++)
    {
        pc_parse_command += ul_off_set;
        ul_ret = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC=", &ul_off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
        if(OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_config_mac_filter::wal_get_parameter_from_cmd return err_code [%u]!}\r\n", ul_ret);
            return -OAL_EINVAL;
        }
        /* 5.1  �������Ƿ����MAC����*/
        if(WLAN_MAC_ADDR_LEN * 2 != OAL_STRLEN(ac_parsed_command))
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_config_mac_filter::invalid MAC format}\r\n");
            return -OAL_EINVAL;
        }
        /*6. ��ӹ����豸*/
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
        /***************************************************************************
                             ���¼���wal�㴦��
        ***************************************************************************/
        OAL_MEMZERO((oal_uint8*)&st_write_msg, OAL_SIZEOF(st_write_msg));
        pst_blklst = (mac_blacklist_stru*)(st_write_msg.auc_value);
        oal_strtoaddr(ac_parsed_command, pst_blklst->auc_mac_addr); /* ���ַ� ac_name ת�������� mac_add[6] */

        us_len = OAL_SIZEOF(mac_blacklist_stru);

        if(ul_i == (ul_mac_cnt - 1))
        {
            /* �����е�mac��ַ�������ɺ󣬲Ž��й����û�ȷ�ϣ��Ƿ���Ҫɾ�� */
            WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ADD_BLACK_LIST, us_len);
        }
        else
        {
            WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ADD_BLACK_LIST_ONLY, us_len);
        }

        /* 6.1  ������Ϣ*/
        l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                    WAL_MSG_TYPE_WRITE,
                                    WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                    (oal_uint8 *)&st_write_msg,
                                    OAL_TRUE,
                                    &pst_rsp_msg);

        if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_config_mac_filter:: wal_send_cfg_event_etc return err code %d!}\r\n", l_ret);
            return l_ret;
        }

        /* 6.2  ��ȡ���صĴ����� */
        ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
        if(OAL_SUCC != ul_err_code)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_config_mac_filter::wal_check_and_release_msg_resp_etc return err code:[%x]!}\r\n",
                               ul_err_code);
            return -OAL_EFAIL;
        }
#endif
    }

    /* ÿ���������mac��ַ���˺���մ��м���� */
    //oal_memset(g_st_ap_config_info.ac_ap_mac_filter_mode, 0 ,OAL_SIZEOF(g_st_ap_config_info.ac_ap_mac_filter_mode));

    return OAL_SUCC;
}



OAL_STATIC oal_int32 wal_kick_sta(oal_net_device_stru *pst_net_dev, oal_uint8 *auc_mac_addr, oal_uint16 us_reason_code)
{
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
    wal_msg_write_stru          st_write_msg;
    wal_msg_stru               *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                  ul_err_code;
    mac_cfg_kick_user_param_stru   *pst_kick_user_param;
    oal_int32                       l_ret = 0;
#endif


    if(NULL == auc_mac_addr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_kick_sta::argument auc_mac_addr is null.\n");
        return -OAL_EFAIL;
    }

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_KICK_USER, OAL_SIZEOF(mac_cfg_kick_user_param_stru));

    pst_kick_user_param = (mac_cfg_kick_user_param_stru *)(st_write_msg.auc_value);
    oal_set_mac_addr(pst_kick_user_param->auc_mac_addr, auc_mac_addr);

    pst_kick_user_param->us_reason_code = us_reason_code;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_kick_user_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_kick_sta:: wal_send_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* 4.4  ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_kick_sta::wal_check_and_release_msg_resp_etc return err code: [%x]!}\r\n",
                               ul_err_code);
        return -OAL_EFAIL;
    }
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_ioctl_set_ap_config(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, oal_int8 *pc_extra)
{
    oal_int8                   *pc_command        = OAL_PTR_NULL;
    oal_int8                   *pc_parse_command  = OAL_PTR_NULL;
    oal_int32                   l_ret             = OAL_SUCC;
    oal_uint32                  ul_ret            = OAL_SUCC;
    oal_int8                    ac_parse_command[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN];
    oal_uint32                  ul_off_set;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev || OAL_PTR_NULL == pst_wrqu))
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters:: param is OAL_PTR_NULL , pst_net_dev = %p, pst_wrqu = %p}",pst_net_dev,pst_wrqu);
        return -OAL_EFAIL;
    }

    /* 1. �����ڴ汣��netd �·������������ */
    pc_command = oal_memalloc((oal_int32)(pst_wrqu->data.length + 1));
    if (OAL_PTR_NULL == pc_command)
    {
        return -OAL_ENOMEM;
    }
    /* 2. ����netd ����ں�̬�� */
    oal_memset(pc_command, 0, (oal_uint32)(pst_wrqu->data.length + 1));
    ul_ret = oal_copy_from_user(pc_command, pst_wrqu->data.pointer , (oal_uint32)(pst_wrqu->data.length));
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_config::oal_copy_from_user: -OAL_EFAIL }\r\n");
        oal_free(pc_command);
        return -OAL_EFAIL;
    }
    pc_command[pst_wrqu->data.length] = '\0';

    OAL_IO_PRINT("wal_ioctl_set_ap_config,data len:%u,command is:%s\n", (oal_uint32)pst_wrqu->data.length, pc_command);

    pc_parse_command = pc_command;
    /* 3.   �������� */
    /* 3.1  ����ASCII_CMD*/
    ul_ret = wal_get_parameter_from_cmd(pc_parse_command, ac_parse_command, "ASCII_CMD=", &ul_off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_ap_config::wal_get_parameter_from_cmd ASCII_CMD return err_code [%u]!}\r\n", ul_ret);
        oal_free(pc_command);
        return -OAL_EINVAL;
    }

    if ((0 != oal_strcmp("AP_CFG", ac_parse_command)))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_config::sub_command != 'AP_CFG' }");
        OAL_IO_PRINT("{wal_ioctl_set_ap_config::sub_command %6s...!= 'AP_CFG' }",ac_parse_command);
        oal_free(pc_command);
        return -OAL_EINVAL;
    }

    /* 3.2  ����CHANNEL��Ŀǰ������netd�·���channel��Ϣ*/
    pc_parse_command += ul_off_set;
    ul_ret = wal_get_parameter_from_cmd(pc_parse_command, ac_parse_command, "CHANNEL=", &ul_off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_ap_config::wal_get_parameter_from_cmd CHANNEL return err_code [%u]!}\r\n", ul_ret);
        oal_free(pc_command);
        return -OAL_EINVAL;
    }

    /* 3.3  ����MAX_SCB*/
    pc_parse_command += ul_off_set;
    ul_ret = wal_get_parameter_from_cmd(pc_parse_command, ac_parse_command, "MAX_SCB=", &ul_off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_ap_config::wal_get_parameter_from_cmd MAX_SCB return err_code [%u]!}\r\n", ul_ret);
        oal_free(pc_command);
        return -OAL_EINVAL;
    }

    g_st_ap_config_info.ul_ap_max_user = (oal_uint32)oal_atoi(ac_parse_command);

    if (OAL_PTR_NULL != OAL_NET_DEV_PRIV(pst_net_dev))
    {
        l_ret = wal_set_ap_max_user(pst_net_dev, (oal_uint32)oal_atoi(ac_parse_command));
    }

    /* 5. �����ͷ��ڴ�*/
    oal_free(pc_command);
    return l_ret;
}


OAL_STATIC oal_int32  wal_ioctl_get_assoc_list(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, oal_int8 *pc_extra)
{
    oal_int32                       l_ret;
    wal_msg_query_stru              st_query_msg;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    wal_msg_rsp_stru               *pst_query_rsp_msg;
    oal_int8                       *pc_sta_list;
    oal_netbuf_stru                *pst_response_netbuf;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev ||OAL_PTR_NULL == pst_info || OAL_PTR_NULL == pst_wrqu || OAL_PTR_NULL == pc_extra))
    {
        OAM_WARNING_LOG4(0, OAM_SF_ANY, "{wal_ioctl_get_assoc_list:: param is OAL_PTR_NULL ,psy_dev=%p, pst_info = %p , pst_wrqu = %p , pc_extra = %p}\n",
        pst_net_dev, pst_info, pst_wrqu, pc_extra);
        return -OAL_EFAIL;
    }

    /* �ϲ����κ�ʱ�򶼿����·��������Ҫ���жϵ�ǰnetdev��״̬����ʱ���� */
    if (OAL_UNLIKELY(OAL_PTR_NULL == OAL_NET_DEV_PRIV(pst_net_dev)))
    {
        return -OAL_EFAIL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    st_query_msg.en_wid = WLAN_CFGID_GET_STA_LIST;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_assoc_list:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);
    /* ҵ���� */
    if(pst_query_rsp_msg->us_len >= OAL_SIZEOF(pst_response_netbuf))
    {
        /* ��ȡhmac�����netbufָ�� */
        oal_memcopy(&pst_response_netbuf,pst_query_rsp_msg->auc_value,OAL_SIZEOF(pst_response_netbuf));
        if(NULL != pst_response_netbuf)
        {
            /* ����ap�����sta��ַ��Ϣ */
            pc_sta_list = (oal_int8*)OAL_NETBUF_DATA(pst_response_netbuf);
            pst_wrqu->data.length = (oal_uint16)(OAL_NETBUF_LEN(pst_response_netbuf) + 1);
            oal_memcopy(pc_extra, pc_sta_list, pst_wrqu->data.length);
            pc_extra[OAL_NETBUF_LEN(pst_response_netbuf)] = '\0';
            oal_netbuf_free(pst_response_netbuf);
        }
        else
        {
            l_ret = -OAL_ENOMEM;
        }
    }
    else
    {
        oal_print_hex_dump((oal_uint8 * )pst_rsp_msg->auc_msg_data, pst_query_rsp_msg->us_len, 32, "query msg: ");
        l_ret = -OAL_EINVAL;
    }

    if(OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_assoc_list::process failed,ret=%d}", l_ret);
    }

    oal_free(pst_rsp_msg);
    return l_ret;

}


OAL_STATIC oal_int32  wal_ioctl_set_mac_filters(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, oal_int8 *pc_extra)
{
    mac_vap_stru               *pst_vap           = OAL_PTR_NULL;
    oal_int8                   *pc_command        = OAL_PTR_NULL;
    oal_int32                   l_ret             = 0;
    oal_uint32                  ul_ret            = 0;
    oal_int8                    ac_parsed_command[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN];
    oal_int8                   *pc_parse_command;
    oal_uint32                  ul_mac_mode;
    oal_uint32                  ul_mac_cnt = 0;
    oal_uint8                   auc_mac_addr[WLAN_MAC_ADDR_LEN];
    oal_uint32                  ul_off_set;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev ||OAL_PTR_NULL == pst_info || OAL_PTR_NULL == pst_wrqu || OAL_PTR_NULL == pc_extra))
    {
        OAM_WARNING_LOG4(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters:: param is OAL_PTR_NULL ,pst_net_dev=%p, pst_info = %p , pst_wrqu = %p , pc_extra = %p}\n",
        pst_net_dev, pst_info, pst_wrqu, pc_extra);
        return -OAL_EFAIL;
    }

    /* 1. �����ڴ汣��netd �·������������ */
    pc_command = oal_memalloc((oal_int32)(pst_wrqu->data.length + 1));
    if (OAL_PTR_NULL == pc_command)
    {
        return -OAL_ENOMEM;
    }

    /* 2. ����netd ����ں�̬�� */
    oal_memset(pc_command, 0, (oal_uint32)(pst_wrqu->data.length + 1));
    ul_ret = oal_copy_from_user(pc_command, pst_wrqu->data.pointer , (oal_uint32)(pst_wrqu->data.length));
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::oal_copy_from_user: -OAL_EFAIL }\r\n");
        oal_free(pc_command);
        return -OAL_EFAIL;
    }
    pc_command[pst_wrqu->data.length] = '\0';

    OAL_IO_PRINT("wal_ioctl_set_mac_filters,data len:%d, command is:%s\n", pst_wrqu->data.length, pc_command);

    pc_parse_command = pc_command;

    oal_memset(g_st_ap_config_info.ac_ap_mac_filter_mode, 0, OAL_SIZEOF(g_st_ap_config_info.ac_ap_mac_filter_mode));
    oal_strncpy(g_st_ap_config_info.ac_ap_mac_filter_mode, pc_command, OAL_SIZEOF(g_st_ap_config_info.ac_ap_mac_filter_mode) - 1);

    pst_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::netdevice vap is null,just save it.}\r\n");
        oal_free(pc_command);
        return OAL_SUCC;
    }

    /* 3  ����MAC_MODE*/
    ul_ret = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC_MODE=", &ul_off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::wal_get_parameter_from_cmd return err_code [%u]!}\r\n", ul_ret);
        oal_free(pc_command);
        return -OAL_EINVAL;
    }
    /* 3.1 �������Ƿ�Ϸ� 0,1,2*/
    ul_mac_mode = (oal_uint32)oal_atoi(ac_parsed_command);
    if(ul_mac_mode > 2)
    {
        OAM_WARNING_LOG4(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::invalid MAC_MODE[%c%c%c%c]!}",
                (oal_uint8)ac_parsed_command[0],
                (oal_uint8)ac_parsed_command[1],
                (oal_uint8)ac_parsed_command[2],
                (oal_uint8)ac_parsed_command[3]);
        oal_free(pc_command);
        return -OAL_EINVAL;
    }

    /* 5 ����MAC_CNT*/
    pc_parse_command += ul_off_set;
    ul_ret = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC_CNT=", &ul_off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::wal_get_parameter_from_cmd return err_code [%u]!}\r\n", ul_ret);
        oal_free(pc_command);
        return -OAL_EINVAL;
    }
    ul_mac_cnt = (oal_uint32)oal_atoi(ac_parsed_command);

    wal_config_mac_filter(pst_net_dev, pc_command);

    /* ����ǰ�����ģʽ�����·�����MAC��ַΪ�գ����������κ��豸��������Ҫȥ���������Ѿ�������STA */
    if((0 == ul_mac_cnt) && (2 == ul_mac_mode))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::delete all user!}");

        oal_memset(auc_mac_addr, 0xff, OAL_ETH_ALEN);
        l_ret = wal_kick_sta(pst_net_dev, auc_mac_addr, MAC_AUTH_NOT_VALID);
    }

    oal_free(pc_command);
    return l_ret;
}


OAL_STATIC oal_int32  wal_ioctl_set_ap_sta_disassoc(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, oal_int8 *pc_extra)
{
    oal_int8                       *pc_command        = OAL_PTR_NULL;
    oal_int32                       l_ret             = 0;
    oal_uint32                      ul_ret            = 0;
    oal_int8                        ac_parsed_command[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN] = {0};
    oal_uint8                       auc_mac_addr[WLAN_MAC_ADDR_LEN] = {0};
    oal_uint32                      ul_off_set;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev || OAL_PTR_NULL == pst_wrqu))
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc:: param is OAL_PTR_NULL , pst_net_dev = %p, pst_wrqu = %p}",
        pst_net_dev,pst_wrqu);
        return -OAL_EFAIL;
    }

    /* 1. �����ڴ汣��netd �·������������ */
    pc_command = oal_memalloc((oal_int32)(pst_wrqu->data.length + 1));
    if (OAL_PTR_NULL == pc_command)
    {
        return -OAL_ENOMEM;
    }

    /* 2. ����netd ����ں�̬�� */
    oal_memset(pc_command, 0, (oal_uint32)(pst_wrqu->data.length + 1));
    ul_ret = oal_copy_from_user(pc_command, pst_wrqu->data.pointer , (oal_uint32)(pst_wrqu->data.length));
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc::oal_copy_from_user: -OAL_EFAIL }\r\n");
        oal_free(pc_command);
        return -OAL_EFAIL;
    }
    pc_command[pst_wrqu->data.length] = '\0';

    OAL_IO_PRINT("wal_ioctl_set_ap_sta_disassoc,command is:%s\n", pc_command);

    /* 3. ���������ȡMAC */
    ul_ret = wal_get_parameter_from_cmd(pc_command, ac_parsed_command, "MAC=", &ul_off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc::wal_get_parameter_from_cmd MAC return err_code [%u]!}\r\n", ul_ret);
        oal_free(pc_command);
        return -OAL_EINVAL;
    }
    /* 3.1  �������Ƿ����MAC����*/
    if(WLAN_MAC_ADDR_LEN * 2 != OAL_STRLEN(ac_parsed_command))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc::invalid MAC format}\r\n");
        oal_free(pc_command);
        return -OAL_EINVAL;
    }
    oal_strtoaddr(ac_parsed_command, auc_mac_addr); /* ���ַ� ac_name ת�������� mac_add[6] */

    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc::Geting CMD from APP to DISASSOC!!}");
    l_ret = wal_kick_sta(pst_net_dev, auc_mac_addr, MAC_AUTH_NOT_VALID);

    /* 5. �����ͷ��ڴ�*/
    oal_free(pc_command);
    return l_ret;

}

#ifdef _PRE_DEBUG_MODE
#ifdef _PRE_WLAN_DFT_EVENT

OAL_STATIC oal_void  wal_event_report_to_sdt(wal_msg_type_enum_uint8   en_msg_type,
                                       oal_uint8                *puc_param,
                                       wal_msg_stru             *pst_cfg_msg)
{
    oam_event_type_enum_uint16   en_event_type = OAM_EVENT_TYPE_BUTT;
    oal_uint8                    auc_event[50] = {0};

    if (WAL_MSG_TYPE_QUERY == en_msg_type)
    {
        en_event_type = OAM_EVENT_WID_QUERY;
    }
    else if (WAL_MSG_TYPE_WRITE == en_msg_type)
    {
        en_event_type = OAM_EVENT_WID_WRITE;
    }

    /* ����WID,������ǰ�����ֽ���WID */
    oal_memcopy((oal_void *)auc_event, (const oal_void *)puc_param, OAL_SIZEOF(oal_uint16));

    /* ������Ϣͷ */
    oal_memcopy((oal_void *)&auc_event[2], (const oal_void *)&(pst_cfg_msg->st_msg_hdr), OAL_SIZEOF(wal_msg_hdr_stru));

    WAL_EVENT_WID(BROADCAST_MACADDR, 0, en_event_type, auc_event);
}
#endif
#endif


oal_uint32  wal_hipriv_get_mac_addr_etc(oal_int8 *pc_param, oal_uint8 auc_mac_addr[], oal_uint32 *pul_total_offset)
{
    oal_uint32                      ul_off_set      = 0;
    oal_uint32                      ul_ret          = OAL_SUCC;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    /* ��ȡmac��ַ */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_get_mac_addr_etc::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }
    oal_strtoaddr(ac_name, auc_mac_addr);

    *pul_total_offset = ul_off_set;

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP

OAL_STATIC oal_uint32  wal_hipriv_set_edca_opt_switch_sta(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru  st_write_msg;
    oal_uint8        uc_flag         = 0;
    oal_uint8       *puc_value       = 0;
    oal_uint32       ul_ret          = OAL_SUCC;
    oal_uint32       ul_off_set      = 0;
    oal_int32        l_ret           = OAL_SUCC;
    mac_vap_stru    *pst_mac_vap     = OAL_PTR_NULL;
    oal_int8         ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    // sh hipriv.sh "vap0 set_edca_switch_sta 1/0"

    /* ��ȡmac_vap */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (WLAN_VAP_MODE_BSS_STA != pst_mac_vap->en_vap_mode)
    {
       OAM_WARNING_LOG0(0, OAM_SF_EDCA, "{wal_hipriv_set_edca_opt_switch_sta:: only STA_MODE support}");
       return OAL_FAIL;
    }

    /* ��ȡ���ò��� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_EDCA, "{wal_hipriv_set_edca_opt_switch_sta::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    uc_flag = (oal_uint8)oal_atoi(ac_name);

    /* �Ƿ����ò��� */
    if (uc_flag > 1)
    {
        OAM_WARNING_LOG0(0, OAM_SF_EDCA, "wal_hipriv_set_edca_opt_switch_sta, invalid config, should be 0 or 1");
        return OAL_SUCC;
    }

    /* �����¼��ڴ� */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_EDCA_OPT_SWITCH_STA, OAL_SIZEOF(oal_uint8));
    puc_value = (oal_uint8 *)(st_write_msg.auc_value);
    *puc_value = uc_flag;

    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_EDCA, "{wal_hipriv_set_edca_opt_switch_sta:: return err_code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif


oal_uint32  wal_hipriv_send_cfg_uint32_data_etc(oal_net_device_stru *pst_net_dev,
    oal_int8 *pc_param, wlan_cfgid_enum_uint16 cfgid)
{
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;
    oal_uint16                      us_len;
    oal_uint32                      ul_ret = OAL_SUCC;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    oal_uint32                      ul_off_set = 0;
    oal_uint32                      set_value = 0;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    OAL_MEMZERO(&st_write_msg,OAL_SIZEOF(wal_msg_write_stru));
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_send_cfg_uint32_data_etc:wal_get_cmd_one_arg_etc fail!}\r\n");
        return ul_ret;
    }

    pc_param += ul_off_set;
    set_value = (oal_uint32)oal_atoi((const oal_int8 *)ac_name);

    us_len = 4; /* OAL_SIZEOF(oal_uint32) */
    *(oal_uint32 *)(st_write_msg.auc_value) = set_value;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, cfgid, us_len);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                              WAL_MSG_TYPE_WRITE,
                              WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                              (oal_uint8 *)&st_write_msg,
                              OAL_FALSE,
                              OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_send_cfg_uint32_data_etc:wal_send_cfg_event_etc return [%d].}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_bgscan_enable(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32                           ul_off_set;
    oal_int8                             ac_stop[2];
    oal_uint32                           ul_ret;
    oal_int32                            l_ret;
    wal_msg_write_stru                   st_write_msg;
    oal_uint8                 *pen_bgscan_enable_flag;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_stop, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_SCAN, "wal_hipriv_bgscan_enable: get first arg fail.");
        return OAL_FAIL;
    }

    /***************************************************************************
                            ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFIGD_BGSCAN_ENABLE, OAL_SIZEOF(oal_bool_enum_uint8));

    /* ��������������� */
    pen_bgscan_enable_flag = (oal_uint8 *)(st_write_msg.auc_value);
   *pen_bgscan_enable_flag = (oal_uint8)oal_atoi(ac_stop);

    OAM_WARNING_LOG1(0, OAM_SF_SCAN, "wal_hipriv_bgscan_enable:: bgscan_enable_flag=%d.", *pen_bgscan_enable_flag);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                       WAL_MSG_TYPE_WRITE,
                       WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_bool_enum_uint8),
                       (oal_uint8 *)&st_write_msg,
                       OAL_FALSE,
                       OAL_PTR_NULL);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_bgscan_enable::wal_send_cfg_event_etc fail.return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_DHCP_REQ_DISABLE

OAL_STATIC oal_uint32  wal_hipriv_dhcp_req_disable_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32                           ul_off_set;
    oal_int8                             ac_stop[2];
    oal_uint32                           ul_ret;
    oal_int32                            l_ret;
    wal_msg_write_stru                   st_write_msg;
    oal_bool_enum_uint8                 *pen_dhcp_req_disable_flag;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_stop, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "wal_hipriv_dhcp_req_disable_switch: get first arg fail.");
        return OAL_FAIL;
    }

    /***************************************************************************
                            ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DHCP_REQ_DISABLE_SWITCH, OAL_SIZEOF(oal_bool_enum_uint8));

    /* ��������������� */
    pen_dhcp_req_disable_flag = (oal_bool_enum_uint8 *)(st_write_msg.auc_value);
   *pen_dhcp_req_disable_flag = (oal_bool_enum_uint8)oal_atoi(ac_stop);

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_hipriv_dhcp_req_disable_switch:: pen_dhcp_req_disable_flag= %d.", *pen_dhcp_req_disable_flag);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                       WAL_MSG_TYPE_WRITE,
                       WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_bool_enum_uint8),
                       (oal_uint8 *)&st_write_msg,
                       OAL_FALSE,
                       OAL_PTR_NULL);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_dhcp_req_disable_switch::wal_send_cfg_event_etc fail.return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM

OAL_STATIC oal_uint32  wal_hipriv_sta_ps_mode(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_int8                         ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    oal_uint8                        uc_vap_ps_mode;
    mac_cfg_ps_mode_param_stru       *pst_ps_mode_param;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sta_ps_enable::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    uc_vap_ps_mode = (oal_uint8)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_PS_MODE, OAL_SIZEOF(mac_cfg_ps_mode_param_stru));

    /* ��������������� */
    pst_ps_mode_param = (mac_cfg_ps_mode_param_stru *)(st_write_msg.auc_value);
    pst_ps_mode_param->uc_vap_ps_mode   = uc_vap_ps_mode;

    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ps_mode_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_PWR, "{wal_hipriv_sta_ps_enable::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_PSM_DEBUG_MODE

OAL_STATIC oal_uint32  wal_hipriv_sta_ps_info(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_int8                         ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    oal_uint8                        uc_psm_info_enable;
    oal_uint8                        uc_psm_debug_mode;
    mac_cfg_ps_info_stru            *pst_ps_info;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sta_ps_info::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    uc_psm_info_enable = (oal_uint8)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sta_ps_info::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    uc_psm_debug_mode = (oal_uint8)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SHOW_PS_INFO, OAL_SIZEOF(mac_cfg_ps_info_stru));

    /* ��������������� */
    pst_ps_info = (mac_cfg_ps_info_stru *)(st_write_msg.auc_value);
    pst_ps_info->uc_psm_info_enable   = uc_psm_info_enable;
    pst_ps_info->uc_psm_debug_mode    = uc_psm_debug_mode;

    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ps_info_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_PWR, "{wal_hipriv_sta_ps_info::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#endif
#endif

#ifdef _PRE_WLAN_FEATURE_STA_UAPSD

OAL_STATIC oal_uint32  wal_hipriv_set_uapsd_para(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_int8                         ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    mac_cfg_uapsd_sta_stru          *pst_uapsd_param;
    oal_uint8                       uc_max_sp_len;
    oal_uint8                       uc_ac;
    oal_uint8                       uc_delivery_enabled[WLAN_WME_AC_BUTT];
    oal_uint8                       uc_trigger_enabled[WLAN_WME_AC_BUTT];

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_uapsd_para::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    uc_max_sp_len = (oal_uint8)oal_atoi(ac_name);

    for (uc_ac = 0; uc_ac < WLAN_WME_AC_BUTT; uc_ac++)
    {
        pc_param = pc_param + ul_off_set;
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
             OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_uapsd_para::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
             return ul_ret;
        }

        /* delivery_enabled�Ĳ������� */
        uc_delivery_enabled[uc_ac] = (oal_uint8)oal_atoi(ac_name);

        /* trigger_enabled ���������� */
        uc_trigger_enabled[uc_ac] = (oal_uint8)oal_atoi(ac_name);
    }
    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_UAPSD_PARA, OAL_SIZEOF(mac_cfg_uapsd_sta_stru));

    /* ��������������� */
    pst_uapsd_param = (mac_cfg_uapsd_sta_stru *)(st_write_msg.auc_value);
    pst_uapsd_param->uc_max_sp_len   = uc_max_sp_len;
    for (uc_ac = 0; uc_ac < WLAN_WME_AC_BUTT; uc_ac++)
    {
        pst_uapsd_param->uc_delivery_enabled[uc_ac] = uc_delivery_enabled[uc_ac];
        pst_uapsd_param->uc_trigger_enabled[uc_ac]  = uc_trigger_enabled[uc_ac];
    }

    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_uapsd_sta_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_PWR, "{wal_hipriv_set_uapsd_para::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)

oal_int32 wal_start_vap_etc(oal_net_device_stru *pst_net_dev)
{
    wal_msg_write_stru      st_write_msg;
    oal_int32               l_ret;
    wal_msg_stru           *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32              ul_err_code;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    oal_wireless_dev_stru   *pst_wdev;
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_start_vap_etc::pst_net_dev is null ptr!}\r\n");
        return -OAL_EFAUL;
    }

    OAL_IO_PRINT("wal_start_vap_etc,dev_name is:%s\n", pst_net_dev->name);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_START_VAP, OAL_SIZEOF(mac_cfg_start_vap_param_stru));
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wdev    = pst_net_dev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_start_vap_etc::wal_wireless_iftype_to_mac_p2p_mode_etc return BUFF}\r\n");
        return -OAL_EINVAL;
    }
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_start_vap_etc::en_p2p_mode:%d}\r\n", en_p2p_mode);
#endif
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_mgmt_rate_init_flag = OAL_TRUE;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_start_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_start_vap_etc::wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_start_vap_etc::hmac start vap fail, err code[%d]!}\r\n", ul_err_code);
        return -OAL_EINVAL;
    }


    if (0 == (OAL_NETDEVICE_FLAGS(pst_net_dev) & OAL_IFF_RUNNING))
    {
        OAL_NETDEVICE_FLAGS(pst_net_dev) |= OAL_IFF_RUNNING;
    }

    /* APģʽ,����VAP��,�������Ͷ��� */
    oal_net_tx_wake_all_queues(pst_net_dev);/*�������Ͷ��� */

    return OAL_SUCC;
}


oal_int32  wal_stop_vap_etc(oal_net_device_stru *pst_net_dev)
{
    wal_msg_write_stru      st_write_msg;
    wal_msg_stru           *pst_rsp_msg = OAL_PTR_NULL;
    oal_int32               l_ret;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    oal_wireless_dev_stru   *pst_wdev;
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_stop_vap_etc::pst_net_dev is null ptr!}\r\n");
        return -OAL_EFAUL;
    }

    if (0 == (OAL_NETDEVICE_FLAGS(pst_net_dev) & OAL_IFF_RUNNING))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_stop_vap::vap is already down,continue to reset hmac vap state.}\r\n");
    }

    OAL_IO_PRINT("wal_stop_vap_etc,dev_name is:%s\n", pst_net_dev->name);

    /***************************************************************************
                           ���¼���wal�㴦��
    ***************************************************************************/
    /* ��дWID��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DOWN_VAP, OAL_SIZEOF(mac_cfg_down_vap_param_stru));
    ((mac_cfg_down_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wdev    = pst_net_dev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_stop_vap_etc::wal_wireless_iftype_to_mac_p2p_mode_etc return BUFF}\r\n");
        return -OAL_EINVAL;
    }
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_stop_vap_etc::en_p2p_mode:%d}\r\n", en_p2p_mode);
#endif

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_down_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_stop_vap_etc::wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp_etc(pst_rsp_msg))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "wal_stop_vap_etc::wal_check_and_release_msg_resp_etc fail");
    }

    return OAL_SUCC;
}



oal_int32 wal_init_wlan_vap_etc(oal_net_device_stru *pst_net_dev)
{
    oal_net_device_stru        *pst_cfg_net_dev;
    wal_msg_write_stru          st_write_msg;
    wal_msg_stru               *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                  ul_err_code;
    mac_vap_stru               *pst_mac_vap;
    oal_wireless_dev_stru      *pst_wdev;
    mac_wiphy_priv_stru        *pst_wiphy_priv;
    mac_vap_stru               *pst_cfg_mac_vap;
    hmac_vap_stru              *pst_cfg_hmac_vap;
    mac_device_stru            *pst_mac_device;
    oal_int8                    ac_wlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    oal_int8                    ac_p2p_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    oal_int32                   l_ret;
    oal_int8                    ac_hwlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    wlan_vap_mode_enum_uint8    en_vap_mode;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8    en_p2p_mode = WLAN_LEGACY_VAP_MODE;
#endif

    if(NULL == pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::pst_net_dev is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL != pst_mac_vap)
    {
        if (MAC_VAP_STATE_BUTT != pst_mac_vap->en_vap_state)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::pst_mac_vap is already exist}\r\n");
            return OAL_SUCC;
        }
        /* netdev�µ�vap�Ѿ���ɾ������Ҫ���´����͹��� */
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_init_wlan_vap_etc::pst_mac_vap is already free, need creat again!!}");
        OAL_NET_DEV_PRIV(pst_net_dev) = OAL_PTR_NULL;
    }

    pst_wdev = OAL_NETDEVICE_WDEV(pst_net_dev);
    if(NULL == pst_wdev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::pst_wdev is null!}\r\n");
        return -OAL_EFAUL;
    }

    pst_wiphy_priv  = (mac_wiphy_priv_stru *)oal_wiphy_priv(pst_wdev->wiphy);
    pst_mac_device  = pst_wiphy_priv->pst_mac_device;
    if(NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::pst_mac_device is null!}\r\n");
        return -OAL_EFAUL;
    }

    /* ͨ��device id��ȡwlanX�� p2pX��netdev�� */
    OAL_SPRINTF(ac_wlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "wlan%d", pst_mac_device->uc_device_id);
    OAL_SPRINTF(ac_p2p_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "p2p%d",  pst_mac_device->uc_device_id);
    OAL_SPRINTF(ac_hwlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "hwlan%d", pst_mac_device->uc_device_id);

    pst_cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->uc_cfg_vap_id);
    if (NULL == pst_cfg_mac_vap)
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::pst_cfg_mac_vap is null! vap_id:%d,deviceid[%d]}\r\n",pst_mac_device->uc_cfg_vap_id,pst_mac_device->uc_device_id);
        return -OAL_EFAUL;
    }
    pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->uc_cfg_vap_id);
    if (NULL == pst_cfg_hmac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::pst_cfg_hmac_vap is null! vap_id:%d}\r\n",pst_mac_device->uc_cfg_vap_id);
        return -OAL_EFAUL;
    }


    pst_cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;
    if(NULL == pst_cfg_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::pst_cfg_net_dev is null!}\r\n");
        return -OAL_EFAUL;
    }

    /* ������WIFI��AP��ʱ����VAP */
    if((NL80211_IFTYPE_STATION == pst_wdev->iftype) || (NL80211_IFTYPE_P2P_DEVICE == pst_wdev->iftype))
    {
        if(0 == (oal_strcmp(ac_wlan_netdev_name, pst_net_dev->name)) ||  0 == (oal_strcmp(ac_hwlan_netdev_name, pst_net_dev->name)))
        {
            en_vap_mode = WLAN_VAP_MODE_BSS_STA;
        }
#ifdef _PRE_WLAN_FEATURE_P2P
        else if(0 == (oal_strcmp(ac_p2p_netdev_name, pst_net_dev->name)))
        {
            en_vap_mode = WLAN_VAP_MODE_BSS_STA;
            en_p2p_mode= WLAN_P2P_DEV_MODE;
        }
#endif
        else
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::net_dev is not wlan or p2p[%d]!}\r\n",pst_mac_device->uc_device_id);
            return OAL_SUCC;
        }
    }
    else if(NL80211_IFTYPE_AP == pst_wdev->iftype)
    {
        en_vap_mode = WLAN_VAP_MODE_BSS_AP;
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::net_dev is not wlan0 or p2p0!}\r\n");
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::en_vap_mode:%d,en_p2p_mode:%d}\r\n",
                     en_vap_mode, en_p2p_mode);
#endif

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ADD_VAP, OAL_SIZEOF(mac_cfg_add_vap_param_stru));
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev     = pst_net_dev;
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->en_vap_mode     = en_vap_mode;
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->uc_cfg_vap_indx = pst_cfg_mac_vap->uc_vap_id;
#ifdef _PRE_WLAN_FEATURE_P2P
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode      = en_p2p_mode;
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->bit_11ac2g_enable = (oal_uint8)!!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_11AC2G_ENABLE);
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->bit_disable_capab_2ght40 = g_st_wlan_customize_etc.uc_disable_capab_2ght40;
#endif
    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_add_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_init_wlan_vap_etc::return err code %d!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_init_wlan_vap_etc::hmac add vap fail, err code[%u]!}\r\n", ul_err_code);
        return -OAL_EFAIL;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if(OAL_SUCC != wal_set_random_mac_to_mib_etc(pst_net_dev))
    {
        OAM_WARNING_LOG0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_init_wlan_vap_etc::wal_set_random_mac_to_mib_etc fail!}\r\n");
        return -OAL_EFAUL;
    }
#endif

    /* ����netdevice��MAC��ַ��MAC��ַ��HMAC�㱻��ʼ����MIB�� */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}");
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    if (en_p2p_mode == WLAN_P2P_DEV_MODE)
    {
        pst_mac_device->st_p2p_info.uc_p2p0_vap_idx = pst_mac_vap->uc_vap_id;
    }
#endif

    if(NL80211_IFTYPE_AP == pst_wdev->iftype)
    {
        /* APģʽ��ʼ������ʼ����������û�����mac��ַ����ģʽ */
        if(g_st_ap_config_info.ul_ap_max_user > 0)
        {
            wal_set_ap_max_user(pst_net_dev, g_st_ap_config_info.ul_ap_max_user);
        }

        if(OAL_STRLEN(g_st_ap_config_info.ac_ap_mac_filter_mode) > 0)
        {
            wal_config_mac_filter(pst_net_dev, g_st_ap_config_info.ac_ap_mac_filter_mode);
        }
    }

    return OAL_SUCC;
}



oal_int32 wal_deinit_wlan_vap_etc(oal_net_device_stru *pst_net_dev)
{
    wal_msg_write_stru           st_write_msg;
    wal_msg_stru                *pst_rsp_msg = OAL_PTR_NULL;
    mac_vap_stru                *pst_mac_vap;
    oal_int32                    l_ret;
    oal_int32                    l_del_vap_flag = OAL_TRUE;
    oal_int8                     ac_wlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    oal_int8                     ac_p2p_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    oal_int8                     ac_hwlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    mac_device_stru             *pst_mac_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8    en_p2p_mode = WLAN_LEGACY_VAP_MODE;
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_deinit_wlan_vap_etc::pst_del_vap_param null ptr !}\r\n");
        return -OAL_EINVAL;
    }


    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_deinit_wlan_vap_etc::pst_mac_vap is already null}\r\n");
        return OAL_SUCC;
    }

    pst_mac_dev = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if(OAL_PTR_NULL == pst_mac_dev)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,"wal_deinit_wlan_vap_etc::mac_res_get_dev_etc id[%d] FAIL",pst_mac_vap->uc_device_id);
        return -OAL_EFAIL;
    }
        /* ͨ��device id��ȡnetdev���� */
    OAL_SPRINTF(ac_wlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "wlan%d", pst_mac_dev->uc_device_id);
    OAL_SPRINTF(ac_p2p_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "p2p%d",  pst_mac_dev->uc_device_id);
    OAL_SPRINTF(ac_hwlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "hwlan%d",  pst_mac_dev->uc_device_id);

    /* ������WIFI��AP�ر�ʱɾ��VAP */
    if ((0 != (oal_strcmp(ac_wlan_netdev_name, pst_net_dev->name)))
        && (0 != (oal_strcmp(ac_p2p_netdev_name, pst_net_dev->name)))
        && (0 != (oal_strcmp(ac_hwlan_netdev_name, pst_net_dev->name))))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_deinit_wlan_vap_etc::net_dev is not wlan or p2p!}\r\n");
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    if (0 == oal_strcmp(ac_p2p_netdev_name, pst_net_dev->name))
    {
        en_p2p_mode = WLAN_P2P_DEV_MODE;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_deinit_wlan_vap_etc::en_p2p_mode:%d}\r\n", en_p2p_mode);
#endif

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    //ɾ��vap ʱ��Ҫ��������ֵ��
    /* st_write_msg�������������ֹ���ֳ�Ա��Ϊ��غ�û�п���û�и�ֵ�����ַ�0���쳣ֵ������ṹ����vap modeû����ȷ��ֵ */
    oal_memset(&st_write_msg, 0, OAL_SIZEOF(wal_msg_write_stru));

    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;
#endif

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DEL_VAP, OAL_SIZEOF(mac_cfg_del_vap_param_stru));
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_del_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_SUCC != wal_check_and_release_msg_resp_etc(pst_rsp_msg))
    {
        OAM_WARNING_LOG0(0,OAM_SF_ANY,"wal_deinit_wlan_vap_etc::wal_check_and_release_msg_resp_etc fail.");
        /*can't set net dev's vap ptr to null when
          del vap wid process failed!*/
        l_del_vap_flag = OAL_FALSE;
    }

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_deinit_wlan_vap_etc::return error code %d}\r\n", l_ret);
        if(-OAL_ENOMEM == l_ret || -OAL_EFAIL == l_ret)
        {
            /*wid had't processed*/
           l_del_vap_flag = OAL_FALSE;
        }
    }

    if(OAL_TRUE == l_del_vap_flag)
    {
        OAL_NET_DEV_PRIV(pst_net_dev) = NULL;
    }

    return l_ret;
}



OAL_STATIC oal_int32 wal_set_mac_addr(oal_net_device_stru *pst_net_dev)
{
    oal_uint8                     auc_primary_mac_addr[WLAN_MAC_ADDR_LEN] = {0};    /* MAC��ַ */
    oal_wireless_dev_stru        *pst_wdev;
    mac_wiphy_priv_stru          *pst_wiphy_priv;
    mac_device_stru              *pst_mac_device;
    oal_int8                    ac_hwlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];

    pst_wdev = OAL_NETDEVICE_WDEV(pst_net_dev);
    pst_wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(pst_wdev->wiphy));
    pst_mac_device = pst_wiphy_priv->pst_mac_device;

#ifdef _PRE_WLAN_FEATURE_P2P
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device->st_p2p_info.pst_primary_net_device))
    {
        /* random mac will be used. hi1102-cb (#include <linux/etherdevice.h>)    */
        oal_random_ether_addr(auc_primary_mac_addr);
        auc_primary_mac_addr[0] &= (~0x02);
        auc_primary_mac_addr[1] = 0x11;
        auc_primary_mac_addr[2] = 0x02;
    }
    else
    {
#ifndef _PRE_PC_LINT
        if(OAL_LIKELY(OAL_PTR_NULL != OAL_NETDEVICE_MAC_ADDR(pst_mac_device->st_p2p_info.pst_primary_net_device)))
        {
            oal_memcopy(auc_primary_mac_addr, OAL_NETDEVICE_MAC_ADDR(pst_mac_device->st_p2p_info.pst_primary_net_device), WLAN_MAC_ADDR_LEN);
        }
        else
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_get_mac_addr() pst_primary_net_device; addr is null}\r\n");
            return OAL_FAIL;
        }
#endif
    }

    switch (pst_wdev->iftype)
    {
        case NL80211_IFTYPE_P2P_DEVICE:
        {
            /* ����P2P device MAC ��ַ��������mac ��ַbit ����Ϊ1 */
            auc_primary_mac_addr[0] |= 0x02;
            oal_set_mac_addr((oal_uint8 *)OAL_NETDEVICE_MAC_ADDR(pst_net_dev), auc_primary_mac_addr);
            break;
        }
        default:
        {
            /*�ŵ�����--add ����hwlan name�ж�*/
            OAL_SPRINTF(ac_hwlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "hwlan%d",  pst_mac_device->uc_device_id);
            if(0 == (oal_strcmp(ac_hwlan_netdev_name, pst_net_dev->name)))
            {
                auc_primary_mac_addr[0] |= 0x02;
                auc_primary_mac_addr[5] += 1;
            }
            else
            {

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
                hwifi_get_mac_addr_etc(auc_primary_mac_addr);
                auc_primary_mac_addr[0]&=(~0x02);
#else
                oal_random_ether_addr(auc_primary_mac_addr);
                auc_primary_mac_addr[0]&=(~0x02);
                auc_primary_mac_addr[1]=0x11;
                auc_primary_mac_addr[2]=0x02;
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
            }
            oal_set_mac_addr((oal_uint8 *)OAL_NETDEVICE_MAC_ADDR(pst_net_dev), auc_primary_mac_addr);
            break;
        }
    }
#else
            /*�ŵ�����--add ����hwlan name�ж�*/
            oal_random_ether_addr(auc_primary_mac_addr);
            OAL_SPRINTF(ac_hwlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "hwlan%d",  pst_mac_device->uc_device_id);
            if(0 == (oal_strcmp(ac_hwlan_netdev_name, pst_net_dev->name)))
            {
                auc_primary_mac_addr[0] |= 0x02;
                auc_primary_mac_addr[5] += 1;
            }
            else
            {
                auc_primary_mac_addr[0]&=(~0x02);
                auc_primary_mac_addr[1]=0x11;
                auc_primary_mac_addr[2]=0x02;
            }
            oal_set_mac_addr((oal_uint8 *)OAL_NETDEVICE_MAC_ADDR(pst_net_dev), auc_primary_mac_addr);
#endif

    return OAL_SUCC;
}


oal_int32 wal_init_wlan_netdev_etc(oal_wiphy_stru *pst_wiphy, char *dev_name)
{
    mac_device_stru            *pst_mac_device;
    oal_int8                    ac_wlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    oal_int8                    ac_p2p_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    oal_net_device_stru        *pst_net_dev;
    oal_wireless_dev_stru      *pst_wdev;
    mac_wiphy_priv_stru        *pst_wiphy_priv;
    enum nl80211_iftype         en_type;
    oal_int32                   l_ret;
    oal_int8                    ac_hwlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    oal_netdev_priv_stru        *pst_netdev_priv;

    if((NULL == pst_wiphy) || (NULL == dev_name))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::pst_wiphy or dev_name is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_wiphy_priv  = (mac_wiphy_priv_stru *)(oal_wiphy_priv(pst_wiphy));
    if (OAL_PTR_NULL == pst_wiphy_priv)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::pst_wiphy_priv is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = pst_wiphy_priv->pst_mac_device;
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::pst_wiphy_priv is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ͨ��device id��ȡnetdev���� */
    OAL_SPRINTF(ac_wlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "wlan%d", pst_mac_device->uc_device_id);
    OAL_SPRINTF(ac_p2p_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "p2p%d",  pst_mac_device->uc_device_id);
    /*�ŵ�����-add*/
    OAL_SPRINTF(ac_hwlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "hwlan%d",  pst_mac_device->uc_device_id);

    /*�ŵ�����--���hwlan name�ж�*/
    if (0 == (oal_strcmp(ac_wlan_netdev_name, dev_name)) ||0 == (oal_strcmp(ac_hwlan_netdev_name, dev_name)))
    {
        en_type = NL80211_IFTYPE_STATION;
    }
    else if (0 == (oal_strcmp(ac_p2p_netdev_name, dev_name)))
    {
        en_type = NL80211_IFTYPE_P2P_DEVICE;
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::dev name is not wlan or p2p}\r\n");
        return OAL_SUCC;
    }
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::en_type is %d}\r\n", en_type);

    /* ���������net device�Ѿ����ڣ�ֱ�ӷ��� */
    /* ����dev_name�ҵ�dev */
    pst_net_dev = oal_dev_get_by_name(dev_name);
    if (OAL_PTR_NULL != pst_net_dev)
    {
        /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
        oal_dev_put(pst_net_dev);

        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::the net_device is already exist!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if defined(_PRE_WLAN_FEATURE_FLOWCTL)
    pst_net_dev = oal_net_alloc_netdev_mqs(OAL_SIZEOF(oal_netdev_priv_stru), dev_name, oal_ether_setup, WAL_NETDEV_SUBQUEUE_MAX_NUM, 1);    /* �˺�����һ����δ���˽�г��ȣ��˴����漰Ϊ0 */
#elif defined(_PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL)
    pst_net_dev = oal_net_alloc_netdev_mqs(OAL_SIZEOF(oal_netdev_priv_stru), dev_name, oal_ether_setup, WLAN_NET_QUEUE_BUTT, 1);    /* �˺�����һ����δ���˽�г��ȣ��˴����漰Ϊ0 */
#else
    pst_net_dev = oal_net_alloc_netdev(OAL_SIZEOF(oal_netdev_priv_stru), dev_name, oal_ether_setup);    /* �˺�����һ����δ���˽�г��ȣ��˴����漰Ϊ0 */
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::oal_net_alloc_netdev return null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_wdev = (oal_wireless_dev_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,OAL_SIZEOF(oal_wireless_dev_stru), OAL_FALSE);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_wdev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::alloc mem, pst_wdev is null ptr!}\r\n");
        oal_net_free_netdev(pst_net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_memset(pst_wdev, 0, OAL_SIZEOF(oal_wireless_dev_stru));

#ifdef _PRE_WLAN_FEATURE_GSO
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::[GSO] add NETIF_F_SG}");
    pst_net_dev->features |= NETIF_F_SG;
    pst_net_dev->hw_features |= NETIF_F_SG;
#endif

    /* ��netdevice���и�ֵ */
#ifdef CONFIG_WIRELESS_EXT
    pst_net_dev->wireless_handlers             = &g_st_iw_handler_def_etc;
#endif
    pst_net_dev->netdev_ops                    = &g_st_wal_net_dev_ops_etc;

    pst_net_dev->ethtool_ops                   = &g_st_wal_ethtool_ops_etc;

    OAL_NETDEVICE_DESTRUCTOR(pst_net_dev)      = oal_net_free_netdev;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,44))
    OAL_NETDEVICE_MASTER(pst_net_dev)          = OAL_PTR_NULL;
#endif

    OAL_NETDEVICE_IFALIAS(pst_net_dev)         = OAL_PTR_NULL;
    OAL_NETDEVICE_WATCHDOG_TIMEO(pst_net_dev)  = 5;
    OAL_NETDEVICE_WDEV(pst_net_dev)            = pst_wdev;
    OAL_NETDEVICE_QDISC(pst_net_dev, OAL_PTR_NULL);

    pst_wdev->netdev = pst_net_dev;
    pst_wdev->iftype = en_type;
    pst_wdev->wiphy = pst_wiphy;
    pst_wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(pst_wiphy));

#ifdef _PRE_WLAN_FEATURE_P2P
    /*�ŵ�����--add wlan name �ж�*/
    if ((NL80211_IFTYPE_STATION == en_type) && (0 == (oal_strcmp(ac_wlan_netdev_name, dev_name))))
    {
        /* �������wlan0�� �򱣴�wlan0 Ϊ��net_device,p2p0 ��p2p-p2p0 MAC ��ַ����netdevice ��ȡ */
        pst_wiphy_priv->pst_mac_device->st_p2p_info.pst_primary_net_device = pst_net_dev;
    }
    /*�ŵ�����--add hwlan name �ж�*/
    else if((NL80211_IFTYPE_STATION == en_type) && (0 == (oal_strcmp(ac_hwlan_netdev_name, dev_name))))
    {
        /*netdevice ָ����ʱ���أ�δʹ��*/
        pst_wiphy_priv->pst_mac_device->st_p2p_info.pst_second_net_device = pst_net_dev;
    }
    else if(NL80211_IFTYPE_P2P_DEVICE == en_type)
    {
        pst_wiphy_priv->pst_mac_device->st_p2p_info.pst_p2p_net_device = pst_net_dev;
    }
#endif
    OAL_NETDEVICE_FLAGS(pst_net_dev) &= ~OAL_IFF_RUNNING;   /* ��net device��flag��Ϊdown */

    wal_set_mac_addr(pst_net_dev);

    pst_netdev_priv = (oal_netdev_priv_stru *)OAL_NET_DEV_WIRELESS_PRIV(pst_net_dev);
    pst_netdev_priv->uc_napi_enable     = OAL_TRUE;
    pst_netdev_priv->uc_gro_enable      = OAL_TRUE;
    pst_netdev_priv->uc_napi_weight     = NAPI_POLL_WEIGHT_LEV1;
    pst_netdev_priv->ul_queue_len_max   = NAPI_NETDEV_PRIV_QUEUE_LEN_MAX;
    pst_netdev_priv->uc_state           = 0;
    pst_netdev_priv->ul_period_pkts     = 0;
    pst_netdev_priv->ul_period_start    = 0;

    oal_netbuf_list_head_init(&pst_netdev_priv->st_rx_netbuf_queue);
    oal_netif_napi_add(pst_net_dev, &pst_netdev_priv->st_napi, hmac_rxdata_polling, NAPI_POLL_WEIGHT_LEV1);

    /* ע��net_device */
    l_ret = oal_net_register_netdev(pst_net_dev);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {

        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::oal_net_register_netdev return error code %d!}\r\n", l_ret);

        OAL_MEM_FREE(pst_wdev, OAL_FALSE);
        oal_net_free_netdev(pst_net_dev);

        return l_ret;
    }

    return OAL_SUCC;
}


oal_int32  wal_setup_ap_etc(oal_net_device_stru *pst_net_dev)
{
    oal_int32 l_ret;
    mac_vap_stru                *pst_mac_vap;
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    wal_set_power_mgmt_on(OAL_FALSE);
    l_ret = wal_set_power_on(pst_net_dev, OAL_TRUE);
    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_setup_ap_etc::wal_set_power_on fail [%d]!}", l_ret);
        return l_ret;
    }
#endif /* #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) */

    if (OAL_NETDEVICE_FLAGS(pst_net_dev) & OAL_IFF_RUNNING)
    {
        /* �л���APǰ��������豸����UP״̬����Ҫ��down wlan0�����豸 */
        OAL_IO_PRINT("wal_setup_ap:stop netdevice:%.16s", pst_net_dev->name);
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_setup_ap_etc:: oal_iff_running! now, stop netdevice}");
        wal_netdev_stop_etc(pst_net_dev);
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(pst_mac_vap)
    {

        if (pst_mac_vap->en_vap_state != MAC_VAP_STATE_INIT)
        {
            /* �л���APǰ��������豸����UP״̬����Ҫ��down wlan0�����豸 */
            OAL_IO_PRINT("wal_setup_ap_etc:stop netdevice:%s", pst_net_dev->name);
            wal_netdev_stop_etc(pst_net_dev);
        }
    }

    pst_net_dev->ieee80211_ptr->iftype = NL80211_IFTYPE_AP;

    l_ret = wal_init_wlan_vap_etc(pst_net_dev);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if(OAL_SUCC == l_ret)
    {
        //hwifi_config_init_ini_etc(pst_net_dev);
#if ((_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE))
        /* �͹��Ķ��ƻ����� */
        (OAL_TRUE == g_wlan_pm_switch_etc) ? wlan_pm_enable_etc() : wlan_pm_disable_etc();
#endif // #if ((_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE))
    }
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
    return l_ret;
}

#endif

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD

oal_uint32 wal_hipriv_register_inetaddr_notifier_etc(oal_void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == register_inetaddr_notifier(&wal_hipriv_notifier))
    {
        return OAL_SUCC;
    }

    OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_register_inetaddr_notifier_etc::register inetaddr notifier failed.}");
    return OAL_FAIL;

#else
    return OAL_SUCC;
#endif
}


oal_uint32 wal_hipriv_unregister_inetaddr_notifier_etc(oal_void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == unregister_inetaddr_notifier(&wal_hipriv_notifier))
    {
        return OAL_SUCC;
    }

    OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_unregister_inetaddr_notifier_etc::hmac_unregister inetaddr notifier failed.}");
    return OAL_FAIL;

#else
    return OAL_SUCC;
#endif
}


oal_uint32 wal_hipriv_register_inet6addr_notifier_etc(oal_void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == register_inet6addr_notifier(&wal_hipriv_notifier_ipv6))
    {
        return OAL_SUCC;
    }

    OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_register_inet6addr_notifier_etc::register inetaddr6 notifier failed.}");
    return OAL_FAIL;

#else
    return OAL_SUCC;
#endif
}


oal_uint32 wal_hipriv_unregister_inet6addr_notifier_etc(oal_void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == unregister_inet6addr_notifier(&wal_hipriv_notifier_ipv6))
    {
        return OAL_SUCC;
    }

    OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_unregister_inet6addr_notifier_etc::hmac_unregister inetaddr6 notifier failed.}");
    return OAL_FAIL;

#else
    return OAL_SUCC;
#endif
}


#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

oal_int32 wal_hipriv_inetaddr_notifier_call_etc(struct notifier_block *this, oal_uint event, oal_void *ptr)
{
    /*
     * Notification mechanism from kernel to our driver. This function is called by the Linux kernel
     * whenever there is an event related to an IP address.
     * ptr : kernel provided pointer to IP address that has changed
     */
    struct in_ifaddr    *pst_ifa       = (struct in_ifaddr *)ptr;
    mac_vap_stru        *pst_mac_vap   = OAL_PTR_NULL;
    hmac_vap_stru       *pst_hmac_vap  = OAL_PTR_NULL;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_ifa))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call_etc::pst_ifa is NULL.}");
        return NOTIFY_DONE;
    }
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_ifa->ifa_dev->dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call_etc::pst_ifa->idev->dev is NULL.}");
        return NOTIFY_DONE;
     }

    /* Filter notifications meant for non Hislicon devices */
    if (pst_ifa->ifa_dev->dev->netdev_ops != &g_st_wal_net_dev_ops_etc)
    {
        return NOTIFY_DONE;
    }

    pst_mac_vap = (mac_vap_stru *)OAL_NET_DEV_PRIV(pst_ifa->ifa_dev->dev);
    if (OAL_UNLIKELY(NULL == pst_mac_vap))
    {
        OAM_WARNING_LOG1(0, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call_etc::Get mac vap failed, when %d(UP:1 DOWN:2 UNKNOWN:others) ipv4 address.}", event);
        return NOTIFY_DONE;
    }

    if (ipv4_is_linklocal_169(pst_ifa->ifa_address))
    {
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call_etc::Invalid IPv4[%d.X.X.%d], MASK[0x%08X].}",
                         ((oal_uint8 *)&(pst_ifa->ifa_address))[0],
                         ((oal_uint8 *)&(pst_ifa->ifa_address))[3],
                         pst_ifa->ifa_mask);
        return NOTIFY_DONE;
    }

    wal_wake_lock();

    switch (event)
    {
        case NETDEV_UP:
        {
            OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call_etc::Up IPv4[%d.X.X.%d], MASK[0x%08X].}",
                             ((oal_uint8 *)&(pst_ifa->ifa_address))[0],
                             ((oal_uint8 *)&(pst_ifa->ifa_address))[3],
                             pst_ifa->ifa_mask);
            hmac_arp_offload_set_ip_addr_etc(pst_mac_vap, DMAC_CONFIG_IPV4, DMAC_IP_ADDR_ADD, &(pst_ifa->ifa_address), &(pst_ifa->ifa_mask));

            if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
            {
                pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
                if (OAL_PTR_NULL == pst_hmac_vap)
                {
                    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_hipriv_inetaddr_notifier_call_etc:: pst_hmac_vap null.uc_vap_id[%d]}",
                        pst_mac_vap->uc_vap_id);

                    wal_wake_unlock();

                    return NOTIFY_DONE;
                }
                /* ��ȡ��IP��ַ��ʱ�����͹��� */
            #if _PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE
                if((MAX_FAST_PS==pst_hmac_vap->uc_ps_mode)||(AUTO_FAST_PS==pst_hmac_vap->uc_ps_mode))
                {
                    wlan_pm_set_timeout_etc((g_wlan_min_fast_ps_idle>1)?(g_wlan_min_fast_ps_idle-1): g_wlan_min_fast_ps_idle);
                }
                else
                {
                    wlan_pm_set_timeout_etc(WLAN_SLEEP_DEFAULT_CHECK_CNT);
                }
            #endif

                /* ��ȡ��IP��ַ��ʱ��֪ͨ���μ�ʱ */
            #ifdef _PRE_WLAN_FEATURE_ROAM
                hmac_roam_wpas_connect_state_notify_etc(pst_hmac_vap, WPAS_CONNECT_STATE_IPADDR_OBTAINED);
            #endif

            }
            break;
        }


        case NETDEV_DOWN:
        {
#if _PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE
            wlan_pm_set_timeout_etc(WLAN_SLEEP_LONG_CHECK_CNT);
#endif
            OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call_etc::Down IPv4[%d.X.X.%d], MASK[0x%08X]..}",
                             ((oal_uint8 *)&(pst_ifa->ifa_address))[0],
                             ((oal_uint8 *)&(pst_ifa->ifa_address))[3],
                             pst_ifa->ifa_mask);
            hmac_arp_offload_set_ip_addr_etc(pst_mac_vap, DMAC_CONFIG_IPV4, DMAC_IP_ADDR_DEL, &(pst_ifa->ifa_address), &(pst_ifa->ifa_mask));

            if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
            {
                /* ��ȡ��IP��ַ��ʱ��֪ͨ���μ�ʱ */
            #ifdef _PRE_WLAN_FEATURE_ROAM
                pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
                if (OAL_PTR_NULL == pst_hmac_vap)
                {
                    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_hipriv_inetaddr_notifier_call_etc:: pst_hmac_vap null.uc_vap_id[%d]}",
                        pst_mac_vap->uc_vap_id);

                    wal_wake_unlock();

                    return NOTIFY_DONE;
                }
                hmac_roam_wpas_connect_state_notify_etc(pst_hmac_vap, WPAS_CONNECT_STATE_IPADDR_REMOVED);
            #endif
            }
            break;
        }

        default:
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call_etc::Unknown notifier event[%d].}", event);
            break;
        }
    }
    wal_wake_unlock();

    return NOTIFY_DONE;
}



oal_int32 wal_hipriv_inet6addr_notifier_call_etc(struct notifier_block *this, oal_uint event, oal_void *ptr)
{
    /*
     * Notification mechanism from kernel to our driver. This function is called by the Linux kernel
     * whenever there is an event related to an IP address.
     * ptr : kernel provided pointer to IP address that has changed
     */
    struct inet6_ifaddr    *pst_ifa       = (struct inet6_ifaddr *)ptr;
    mac_vap_stru           *pst_mac_vap;

    if (OAL_UNLIKELY(NULL == pst_ifa))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call_etc::pst_ifa is NULL.}");
        return NOTIFY_DONE;
    }
    if (OAL_UNLIKELY(NULL == pst_ifa->idev->dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call_etc::pst_ifa->idev->dev is NULL.}");
        return NOTIFY_DONE;
     }


    /* Filter notifications meant for non Hislicon devices */
    if (pst_ifa->idev->dev->netdev_ops != &g_st_wal_net_dev_ops_etc)
    {
        return NOTIFY_DONE;
    }

    pst_mac_vap = (mac_vap_stru *)OAL_NET_DEV_PRIV(pst_ifa->idev->dev);
    if (OAL_UNLIKELY(NULL == pst_mac_vap))
    {
        OAM_WARNING_LOG1(0, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call_etc::Get mac vap failed, when %d(UP:1 DOWN:2 UNKNOWN:others) ipv6 address.}", event);
        return NOTIFY_DONE;
    }

    switch (event)
    {
        case NETDEV_UP:
        {
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call_etc::UP IPv6[%04x:%04x:XXXX:XXXX:XXXX:XXXX:%04x:%04x].}",
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[0]),
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[1]),
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[6]),
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[7]));
            hmac_arp_offload_set_ip_addr_etc(pst_mac_vap, DMAC_CONFIG_IPV6, DMAC_IP_ADDR_ADD, &(pst_ifa->addr), &(pst_ifa->addr));
            break;
        }

        case NETDEV_DOWN:
        {
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call_etc::DOWN IPv6[%04x:%04x:XXXX:XXXX:XXXX:XXXX:%04x:%04x].}",
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[0]),
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[1]),
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[6]),
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[7]));
            hmac_arp_offload_set_ip_addr_etc(pst_mac_vap, DMAC_CONFIG_IPV6, DMAC_IP_ADDR_DEL, &(pst_ifa->addr), &(pst_ifa->addr));
            break;
        }

        default:
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call_etc::Unknown notifier event[%d].}", event);
            break;
        }
    }

    return NOTIFY_DONE;
}
#endif /* #if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) */
#endif /* #ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD */
#ifdef _PRE_WLAN_FEATURE_11K

OAL_STATIC oal_uint32  wal_hipriv_send_neighbor_req(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_offset = 0;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    mac_cfg_ssid_param_stru          *pst_ssid;
    oal_uint8                        uc_str_len;

    uc_str_len = OS_STR_LEN(pc_param);
    uc_str_len = (uc_str_len > 1)?uc_str_len-1:uc_str_len;

    /* ��ȡSSID�ַ��� */
    if (0 != uc_str_len)
    {
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_offset);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_send_neighbor_req::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
            return ul_ret;
        }
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SEND_NEIGHBOR_REQ, OAL_SIZEOF(mac_cfg_ssid_param_stru));
    pst_ssid = (mac_cfg_ssid_param_stru *)st_write_msg.auc_value;
    pst_ssid->uc_ssid_len = uc_str_len;
    oal_memcopy(pst_ssid->ac_ssid, ac_arg, pst_ssid->uc_ssid_len);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ssid_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_send_neighbor_req::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
    return OAL_SUCC;
}

OAL_STATIC oal_uint32  wal_hipriv_beacon_req_table_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_offset = 0;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint8                        uc_switch;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_offset);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_beacon_req_table_switch::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    uc_switch = (oal_uint8)oal_atoi(ac_arg);

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_BCN_TABLE_SWITCH, OAL_SIZEOF(oal_uint8));
    st_write_msg.auc_value[0] = uc_switch;
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_beacon_req_table_switch::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
    return OAL_SUCC;
}
#endif //_PRE_WLAN_FEATURE_11K


OAL_STATIC oal_uint32  wal_hipriv_voe_enable(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_offset = 0;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint16                       us_switch;
    oal_uint16                      *ps_value;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_offset);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_voe_enable::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    us_switch = (oal_uint16)oal_atoi(ac_arg);
    us_switch = us_switch & 0xFFFF;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_VOE_ENABLE, OAL_SIZEOF(oal_uint16));
    ps_value = (oal_uint16 *)st_write_msg.auc_value;
    *ps_value  = us_switch;
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint16),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_voe_enable::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_VOWIFI

OAL_STATIC oal_int32 wal_ioctl_set_vowifi_param(oal_net_device_stru *pst_net_dev, oal_int8* puc_command)
{

    oal_int32                   l_ret;
    oal_uint16                  us_len;
    wal_msg_write_stru          st_write_msg;
    mac_cfg_vowifi_stru        *pst_cfg_vowifi;
    mac_vowifi_cmd_enum_uint8   en_vowifi_cmd_id;
    oal_uint8                   uc_param;

    /* vapδ����ʱ��������supplicant���� */
    if (OAL_PTR_NULL == OAL_NET_DEV_PRIV(pst_net_dev))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "wal_ioctl_set_vowifi_param::vap not created yet, ignore the cmd!");
        return -OAL_EINVAL;
    }

    if (oal_memcmp(puc_command, CMD_VOWIFI_SET_MODE, OAL_STRLEN(CMD_VOWIFI_SET_MODE)) == 0)
    {
        uc_param  = (oal_uint8)oal_atoi((oal_int8*)puc_command + OAL_STRLEN((oal_int8 *)CMD_VOWIFI_SET_MODE) + 1);
        en_vowifi_cmd_id = VOWIFI_SET_MODE;
    }
    else if (oal_memcmp(puc_command, CMD_VOWIFI_SET_PERIOD, OAL_STRLEN(CMD_VOWIFI_SET_PERIOD)) == 0)
    {
        uc_param  = (oal_uint8)oal_atoi((oal_int8*)puc_command + OAL_STRLEN((oal_int8 *)CMD_VOWIFI_SET_PERIOD) + 1);
        en_vowifi_cmd_id = VOWIFI_SET_PERIOD;
    }
    else if (oal_memcmp(puc_command, CMD_VOWIFI_SET_LOW_THRESHOLD, OAL_STRLEN(CMD_VOWIFI_SET_LOW_THRESHOLD)) == 0)
    {
        uc_param  = (oal_uint8)oal_atoi((oal_int8*)puc_command + OAL_STRLEN((oal_int8 *)CMD_VOWIFI_SET_LOW_THRESHOLD) + 1);
        en_vowifi_cmd_id = VOWIFI_SET_LOW_THRESHOLD;
    }
    else if (oal_memcmp(puc_command, CMD_VOWIFI_SET_HIGH_THRESHOLD, OAL_STRLEN(CMD_VOWIFI_SET_HIGH_THRESHOLD)) == 0)
    {
        uc_param  = (oal_uint8)oal_atoi((oal_int8*)puc_command + OAL_STRLEN((oal_int8 *)CMD_VOWIFI_SET_HIGH_THRESHOLD) + 1);
        en_vowifi_cmd_id = VOWIFI_SET_HIGH_THRESHOLD;
    }
    else if (oal_memcmp(puc_command, CMD_VOWIFI_SET_TRIGGER_COUNT, OAL_STRLEN(CMD_VOWIFI_SET_TRIGGER_COUNT)) == 0)
    {
        uc_param  = (oal_uint8)oal_atoi((oal_int8*)puc_command + OAL_STRLEN((oal_int8 *)CMD_VOWIFI_SET_TRIGGER_COUNT) + 1);
        en_vowifi_cmd_id = VOWIFI_SET_TRIGGER_COUNT;
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "wal_ioctl_set_vowifi_param::invalid cmd!");
        return -OAL_EINVAL;
    }


    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_vowifi_param::supplicant set VoWiFi_param cmd(%d), value[%d] }", en_vowifi_cmd_id, uc_param);

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    us_len         = OAL_SIZEOF(mac_cfg_vowifi_stru);
    pst_cfg_vowifi = (mac_cfg_vowifi_stru *)(st_write_msg.auc_value);
    pst_cfg_vowifi->en_vowifi_cfg_cmd = en_vowifi_cmd_id;
    pst_cfg_vowifi->uc_value          = uc_param;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_VOWIFI_INFO, us_len);
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                       WAL_MSG_TYPE_WRITE,
                       WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                       (oal_uint8 *)&st_write_msg,
                       OAL_FALSE,
                       OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_ioctl_set_vowifi_param::wal_send_cfg_event_etc failed, error no[%d]!\r\n", l_ret);
        return l_ret;
    }
    return OAL_SUCC;
}

OAL_STATIC oal_int32 wal_ioctl_get_vowifi_param(oal_net_device_stru *pst_net_dev, oal_int8 *puc_command, oal_int32 *pl_value)
{
    mac_vap_stru *pst_mac_vap;

    /* vapδ����ʱ��������supplicant���� */
    if (OAL_PTR_NULL == OAL_NET_DEV_PRIV(pst_net_dev))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "wal_ioctl_get_vowifi_param::vap not created yet, ignore the cmd!");
        return -OAL_EINVAL;
    }

    /* ��ȡmac_vap */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap->pst_vowifi_cfg_param)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_ioctl_get_vowifi_param::pst_vowifi_cfg_param is null.}");
        return OAL_SUCC;
    }

    if (oal_memcmp(puc_command, CMD_VOWIFI_GET_MODE, OAL_STRLEN(CMD_VOWIFI_GET_MODE)) == 0)
    {
        *pl_value  = (oal_int)pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode;
    }
    else if (oal_memcmp(puc_command, CMD_VOWIFI_GET_PERIOD, OAL_STRLEN(CMD_VOWIFI_GET_PERIOD)) == 0)
    {
        *pl_value  = (oal_int)pst_mac_vap->pst_vowifi_cfg_param->us_rssi_period_ms/1000;
    }
    else if (oal_memcmp(puc_command, CMD_VOWIFI_GET_LOW_THRESHOLD, OAL_STRLEN(CMD_VOWIFI_GET_LOW_THRESHOLD)) == 0)
    {
        *pl_value  = (oal_int)pst_mac_vap->pst_vowifi_cfg_param->c_rssi_low_thres;
    }
    else if (oal_memcmp(puc_command, CMD_VOWIFI_GET_HIGH_THRESHOLD, OAL_STRLEN(CMD_VOWIFI_GET_HIGH_THRESHOLD)) == 0)
    {
        *pl_value  = (oal_int)pst_mac_vap->pst_vowifi_cfg_param->c_rssi_high_thres;
    }
    else if (oal_memcmp(puc_command, CMD_VOWIFI_GET_TRIGGER_COUNT, OAL_STRLEN(CMD_VOWIFI_GET_TRIGGER_COUNT)) == 0)
    {
        *pl_value  = (oal_int)pst_mac_vap->pst_vowifi_cfg_param->uc_trigger_count_thres;
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "wal_ioctl_get_vowifi_param::invalid cmd!");
        *pl_value  = 0xffffffff;
        return -OAL_EINVAL;
    }

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_vowifi_param::supplicant get VoWiFi_param value[%d] }",*pl_value);

    return OAL_SUCC;
}

#endif /* _PRE_WLAN_FEATURE_VOWIFI */

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

OAL_STATIC oal_int32 wal_ioctl_get_wifi_priv_feature_cap_param(oal_net_device_stru *pst_net_dev, oal_int8 *puc_command, oal_int32 *pl_value)
{
    mac_vap_stru                    *pst_mac_vap;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_uint32                       ul_value = 0;

    /* vapδ����ʱ��������supplicant���� */
    if (OAL_PTR_NULL == OAL_NET_DEV_PRIV(pst_net_dev))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "wal_ioctl_get_wifi_priv_feature_cap_param::vap not created yet, ignore the cmd!");
        return -OAL_EINVAL;
    }

    /* ��ȡmac_vap */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_ioctl_get_wifi_priv_feature_cap_param::netdevice->mac_vap is null.}");
        return OAL_SUCC;
    }

    *pl_value    = 0;
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_ioctl_get_wifi_priv_feature_cap_param::pst_hmac_vap is null.}");
        return OAL_SUCC;
    }

#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11R) || defined(_PRE_WLAN_FEATURE_11K_EXTERN)
    /*11k����*/
    if(OAL_TRUE == pst_hmac_vap->bit_11k_enable)
    {
        ul_value |=  BIT(WAL_WIFI_FEATURE_SUPPORT_11K);
    }

    /*11v����*/
    if(OAL_TRUE == pst_hmac_vap->bit_11v_enable)
    {
        ul_value |= BIT(WAL_WIFI_FEATURE_SUPPORT_11V);
    }

    if(OAL_TRUE == pst_hmac_vap->bit_11r_enable)
    {
        ul_value |= BIT(WAL_WIFI_FEATURE_SUPPORT_11R);
    }
#endif
    ul_value     |= BIT(WAL_WIFI_FEATURE_SUPPORT_VOWIFI_NAT_KEEP_ALIVE);

    *pl_value = ul_value;

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_IP_FILTER

oal_int32 wal_set_ip_filter_enable_etc(oal_int32 l_on)
{
    oal_uint16   us_len;
    oal_int32    l_ret;
    oal_uint32   ul_netbuf_len;
    oal_netbuf_stru            *pst_netbuf;
    mac_vap_stru               *pst_mac_vap;
    oal_net_device_stru        *pst_net_dev;
    wal_msg_write_stru          st_write_msg;
    mac_ip_filter_cmd_stru      st_ip_filter_cmd;
    mac_ip_filter_cmd_stru     *pst_cmd_info;

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_set_ip_filter_enable_etc:: dfr_process_status[%d]!}",
            g_st_dfr_info_etc.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif //#ifdef _PRE_WLAN_FEATURE_DFR

    if (l_on < 0)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_set_ip_filter_enable_etc::Invalid input parameter, on/off %d!}", l_on);
        return -OAL_EINVAL;
    }

    pst_net_dev = oal_dev_get_by_name("wlan0");
    if (OAL_PTR_NULL == pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_set_ip_filter_enable_etc::wlan0 not exist!}");
        return -OAL_EINVAL;
    }
    /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
    oal_dev_put(pst_net_dev);

    /* vapδ����ʱ���������·������� */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_set_ip_filter_enable_etc::vap not created yet, ignore the cmd!}");
        return -OAL_EINVAL;
    }

    if (OAL_TRUE != pst_mac_vap->st_cap_flag.bit_ip_filter)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_ip_filter_enable_etc::Func not enable, ignore the cmd!}");
        return -OAL_EINVAL;
    }

    /* ׼���������� */
    ul_netbuf_len = OAL_SIZEOF(st_ip_filter_cmd);
    OAL_MEMZERO((oal_uint8 *)&st_ip_filter_cmd, ul_netbuf_len);
    st_ip_filter_cmd.en_cmd        = MAC_IP_FILTER_ENABLE;
    st_ip_filter_cmd.en_enable     = (l_on > 0)? OAL_TRUE : OAL_FALSE;

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_ip_filter_enable_etc::IP_filter on/off(%d).}",
                        st_ip_filter_cmd.en_enable);

    /* ����ռ� ������˹��� */
    pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,ul_netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_ip_filter_enable_etc::netbuf alloc null,size %d.}", ul_netbuf_len);
        return -OAL_EINVAL;
    }
    OAL_MEMZERO(((oal_uint8 *)OAL_NETBUF_DATA(pst_netbuf)), ul_netbuf_len);
    pst_cmd_info = (mac_ip_filter_cmd_stru *)OAL_NETBUF_DATA(pst_netbuf);

    /* ��¼���˹��� */
    oal_memcopy((oal_uint8 *)pst_cmd_info, (oal_uint8 *)(&st_ip_filter_cmd), ul_netbuf_len);
    oal_netbuf_put(pst_netbuf, ul_netbuf_len);

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    us_len = OAL_SIZEOF(pst_netbuf);

    /* ��д msg ��Ϣͷ*/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_IP_FILTER, us_len);

    /* �������netbuf�׵�ַ��д��msg��Ϣ���� */
    oal_memcopy(st_write_msg.auc_value, (oal_uint8 *)&pst_netbuf, us_len);

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_ip_filter_enable_etc::wal_send_cfg_event_etc failed, error no[%d]!}", l_ret);
        oal_netbuf_free(pst_netbuf);
        return l_ret;
    }

    return OAL_SUCC;
}

oal_int32 wal_add_ip_filter_items_etc(wal_hw_wifi_filter_item *pst_items, oal_int32 l_count)
{
    oal_uint16   us_len;
    oal_int32    l_ret;
    oal_uint32   ul_netbuf_len;
    oal_uint32   ul_items_idx;
    oal_netbuf_stru            *pst_netbuf;
    mac_vap_stru               *pst_mac_vap;
    oal_net_device_stru        *pst_net_dev;
    wal_msg_write_stru          st_write_msg;
    mac_ip_filter_cmd_stru      st_ip_filter_cmd;
    mac_ip_filter_cmd_stru      *pst_cmd_info;

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_add_ip_filter_items_etc:: dfr_process_status[%d]!}",
            g_st_dfr_info_etc.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif //#ifdef _PRE_WLAN_FEATURE_DFR

    if ((OAL_PTR_NULL == pst_items)||(l_count <= 0))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_add_ip_filter_items_etc::Invalid input parameter, pst_items %p, l_count %d!}", pst_items,l_count);
        return -OAL_EINVAL;
    }

    pst_net_dev = oal_dev_get_by_name("wlan0");
    if (OAL_PTR_NULL == pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_add_ip_filter_items_etc::wlan0 not exist!}");
        return -OAL_EINVAL;
    }
    /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
    oal_dev_put(pst_net_dev);

    /* vapδ����ʱ���������·������� */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_add_ip_filter_items_etc::vap not created yet, ignore the cmd!.}");
        return -OAL_EINVAL;
    }

    if (OAL_TRUE != pst_mac_vap->st_cap_flag.bit_ip_filter)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_add_ip_filter_items_etc::Func not enable, ignore the cmd!.}");
        return -OAL_EINVAL;
    }

    /* ׼�������¼� */
    OAL_MEMZERO((oal_uint8 *)&st_ip_filter_cmd, OAL_SIZEOF(st_ip_filter_cmd));
    st_ip_filter_cmd.en_cmd    = MAC_IP_FILTER_UPDATE_BTABLE;

    /* ���ڱ���������С���ƣ�ȡ�����ɵĹ�����Ŀ����Сֵ */
    st_ip_filter_cmd.uc_item_count = OAL_MIN((MAC_MAX_IP_FILTER_BTABLE_SIZE / OAL_SIZEOF(mac_ip_filter_item_stru)), l_count);
    if (st_ip_filter_cmd.uc_item_count < l_count)
    {
       OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_add_ip_filter_items_etc::Btable(%d) is too small to store %d items!}",
                        st_ip_filter_cmd.uc_item_count,
                        l_count);
    }

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_add_ip_filter_items_etc::Start updating btable, items_cnt(%d).}",
                        st_ip_filter_cmd.uc_item_count);

    /* ѡ�������¼��ռ�Ĵ�С */
    ul_netbuf_len = (st_ip_filter_cmd.uc_item_count * OAL_SIZEOF(mac_ip_filter_item_stru)) + OAL_SIZEOF(st_ip_filter_cmd);


    /* ����ռ� ������˹��� */
    pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,ul_netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_add_ip_filter_items_etc::netbuf alloc null,size %d.}", ul_netbuf_len);
        return -OAL_EINVAL;
    }
    OAL_MEMZERO(((oal_uint8 *)OAL_NETBUF_DATA(pst_netbuf)), ul_netbuf_len);
    pst_cmd_info = (mac_ip_filter_cmd_stru *)OAL_NETBUF_DATA(pst_netbuf);

    /* ��¼���˹��� */
    oal_memcopy((oal_uint8 *)pst_cmd_info, (oal_uint8 *)(&st_ip_filter_cmd), OAL_SIZEOF(st_ip_filter_cmd));
    oal_netbuf_put(pst_netbuf, ul_netbuf_len);

    for(ul_items_idx = 0; ul_items_idx < st_ip_filter_cmd.uc_item_count; ul_items_idx++)
    {
        pst_cmd_info->ast_filter_items[ul_items_idx].uc_protocol = (oal_uint8)pst_items[ul_items_idx].protocol;
        pst_cmd_info->ast_filter_items[ul_items_idx].us_port     = (oal_uint16)pst_items[ul_items_idx].port;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    us_len = OAL_SIZEOF(pst_netbuf);

    /* ��д msg ��Ϣͷ*/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_IP_FILTER, us_len);

    /* �������netbuf�׵�ַ��д��msg��Ϣ���� */
    oal_memcopy(st_write_msg.auc_value, (oal_uint8 *)&pst_netbuf, us_len);

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_add_ip_filter_items_etc::wal_send_cfg_event_etc failed, error no[%d]!}", l_ret);
        oal_netbuf_free(pst_netbuf);
        return l_ret;
    }


    return OAL_SUCC;

}

oal_int32 wal_clear_ip_filter_etc()
{
    oal_uint16   us_len;
    oal_int32    l_ret;
    oal_uint32   ul_netbuf_len;
    oal_netbuf_stru            *pst_netbuf;
    mac_vap_stru               *pst_mac_vap;
    oal_net_device_stru        *pst_net_dev;
    wal_msg_write_stru          st_write_msg;
    mac_ip_filter_cmd_stru      st_ip_filter_cmd;
    mac_ip_filter_cmd_stru     *pst_cmd_info;

#ifdef _PRE_WLAN_FEATURE_DFR
        if (g_st_dfr_info_etc.bit_device_reset_process_flag)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_clear_ip_filter_etc:: dfr_process_status[%d]!}",
                g_st_dfr_info_etc.bit_device_reset_process_flag);
            return -OAL_EFAIL;
        }
#endif //#ifdef _PRE_WLAN_FEATURE_DFR

    pst_net_dev = oal_dev_get_by_name("wlan0");
    if (OAL_PTR_NULL == pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_clear_ip_filter_etc::wlan0 not exist!}");
        return -OAL_EINVAL;
    }

    /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
    oal_dev_put(pst_net_dev);

    /* vapδ����ʱ���������·������� */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_clear_ip_filter_etc::vap not created yet, ignore the cmd!.}");
        return -OAL_EINVAL;
    }

    if (OAL_TRUE != pst_mac_vap->st_cap_flag.bit_ip_filter)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_clear_ip_filter_etc::Func not enable, ignore the cmd!.}");
        return -OAL_EINVAL;
    }

    /* ��������� */
    OAL_MEMZERO((oal_uint8 *)&st_ip_filter_cmd, OAL_SIZEOF(st_ip_filter_cmd));
    st_ip_filter_cmd.en_cmd        = MAC_IP_FILTER_CLEAR;

    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_clear_ip_filter_etc::Now start clearing the list.}");

    /* ѡ�������¼��ռ�Ĵ�С */
    ul_netbuf_len = OAL_SIZEOF(st_ip_filter_cmd);

    /* ����ռ� ������˹��� */
    pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,ul_netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_clear_ip_filter_etc::netbuf alloc null,size %d.}", ul_netbuf_len);
        return -OAL_EINVAL;
    }
    OAL_MEMZERO(((oal_uint8 *)OAL_NETBUF_DATA(pst_netbuf)), ul_netbuf_len);
    pst_cmd_info = (mac_ip_filter_cmd_stru *)OAL_NETBUF_DATA(pst_netbuf);

    /* ��¼���˹��� */
    oal_memcopy((oal_uint8 *)pst_cmd_info, (oal_uint8 *)(&st_ip_filter_cmd), ul_netbuf_len);
    oal_netbuf_put(pst_netbuf, ul_netbuf_len);

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    us_len = OAL_SIZEOF(pst_netbuf);

    /* ��д msg ��Ϣͷ*/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_IP_FILTER, us_len);

    /* �������netbuf�׵�ַ��д��msg��Ϣ���� */
    oal_memcopy(st_write_msg.auc_value, (oal_uint8 *)&pst_netbuf, us_len);

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_clear_ip_filter_etc::wal_send_cfg_event_etc failed, error no[%d]!}", l_ret);
        oal_netbuf_free(pst_netbuf);
        return l_ret;
    }

    return OAL_SUCC;
}
oal_int32 wal_register_ip_filter_etc(wal_hw_wlan_filter_ops *pst_ip_filter_ops)
{
#ifdef CONFIG_DOZE_FILTER
    if (OAL_PTR_NULL == pst_ip_filter_ops)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_register_ip_filter_etc::pg_st_ip_filter_ops is null !}");
        return -OAL_EINVAL;
    }
    hw_register_wlan_filter(pst_ip_filter_ops);
#else
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_register_ip_filter_etc:: Not support CONFIG_DOZE_FILTER!}");
#endif
    return OAL_SUCC;
}

oal_int32 wal_unregister_ip_filter_etc()
{
#ifdef CONFIG_DOZE_FILTER
    hw_unregister_wlan_filter();
#else
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_unregister_ip_filter_etc:: Not support CONFIG_DOZE_FILTER!}");
#endif
    return OAL_SUCC;
}

#else
oal_int32 wal_set_ip_filter_enable_etc(oal_int32 l_on)
{
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_set_ip_filter_enable_etc::Ip_filter not support!}");
    return -OAL_EFAIL;
}
oal_int32 wal_add_ip_filter_items_etc(wal_hw_wifi_filter_item *pst_items, oal_int32 l_count)
{
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_add_ip_filter_items_etc::Ip_filter not support!}");
    return -OAL_EFAIL;
}

oal_int32 wal_clear_ip_filter_etc()
{
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_clear_ip_filter_etc::Ip_filter not support!}");

    return -OAL_EFAIL;
}


#endif //_PRE_WLAN_FEATURE_IP_FILTER

/*lint -e19*/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,44))
oal_module_symbol(wal_hipriv_proc_write);
#endif
oal_module_symbol(wal_hipriv_get_mac_addr_etc);
/*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)

OAL_STATIC oal_uint32  wal_hipriv_cali_power(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_freq;
    oal_int32                   l_ch;
    oal_int32                   al_power[WLAN_DYNC_CALI_POW_PD_PARAM_NUM];
    oal_uint8                   uc_count;
    mac_cfg_cali_power_stru     *pst_power_param;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                  ul_ret;
    oal_int32                   l_ret;
    wal_msg_stru                *pst_rsp_msg = OAL_PTR_NULL;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CALI_POWER, OAL_SIZEOF(mac_cfg_cali_power_stru));
    pst_power_param = (mac_cfg_cali_power_stru *)(st_write_msg.auc_value);

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_cali_power::wal_get_cmd_one_arg_etc ch return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    l_ch = oal_atoi(ac_name);
    if(l_ch < 0 || l_ch > 2)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_cali_power::ch is invalid!}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    pst_power_param->uc_ch = (oal_uint8)l_ch;
    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_hipriv_cali_power::l_ch = %d!}\r\n", l_ch);

    /* l_chΪ2ʱ���ǻ�ȡ����ʽֵ */
    if(l_ch != 2)
    {
        pc_param = pc_param + ul_off_set;
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_cali_power::wal_get_cmd_one_arg_etc freq return err_code %d!}\r\n", ul_ret);
            return ul_ret;
        }

        l_freq = oal_atoi(ac_name);
        pst_power_param->uc_freq = (oal_uint8)l_freq;
        OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_hipriv_cali_power::l_freq = %d!}\r\n", l_freq);

        for(uc_count = 0; uc_count < WLAN_DYNC_CALI_POW_PD_PARAM_NUM; uc_count++)
        {
            pc_param = pc_param + ul_off_set;
            ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_cali_power::wal_get_cmd_one_arg_etc freq return err_code %d!}\r\n", ul_ret);
                return ul_ret;
            }

            al_power[uc_count] = oal_atoi(ac_name);
            pst_power_param->as_power[uc_count] = (oal_int16)al_power[uc_count];
            OAM_INFO_LOG2(0, OAM_SF_ANY, "{wal_hipriv_cali_power::al_powe[%d] = %d!}\r\n", uc_count, al_power[uc_count]);
        }
    }

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_cali_power_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_cali_power::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_cali_power fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }

//#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
//    wal_hipriv_wait_rsp(pst_net_dev, pc_param);
//#endif

    return OAL_SUCC;
}

OAL_STATIC oal_uint32  wal_hipriv_set_polynomial_para(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_freq;
    oal_int8                        ac_polynomial[5] = {0};
    oal_uint8                       uc_count;
    oal_uint8                       uc_ch_count;
    mac_cfg_polynomial_para_stru    *pst_polynomial_param;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int8                        *pst_name;
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_POLYNOMIAL_PARA, OAL_SIZEOF(mac_cfg_polynomial_para_stru));
    pst_polynomial_param = (mac_cfg_polynomial_para_stru *)(st_write_msg.auc_value);

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_polynomial_para::wal_get_cmd_one_arg_etc ch return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    l_freq = oal_atoi(ac_name);
    pst_polynomial_param->uc_freq = (oal_uint8)l_freq;
    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_polynomial_para::l_freq = %d!}\r\n", l_freq);

    for(uc_ch_count = 0; uc_ch_count < 2; uc_ch_count++)
    {
        pc_param = pc_param + ul_off_set;
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_polynomial_para::wal_get_cmd_one_arg_etc freq return err_code %d!}\r\n", ul_ret);
            return ul_ret;
        }

        /*ÿ������ʽֵռ��2��byte��������ֵ��ת��Ϊʮ�������ַ���������Ϊ2*2*3=12 */
        if(OAL_STRLEN(ac_name) != 12)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_polynomial_para::polynomial_para value not right}\r\n");
            return OAL_EINVAL;
        }

        pst_name = ac_name;
        for(uc_count = 0; uc_count < WLAN_DYNC_CALI_POW_PD_PARAM_NUM; uc_count++)
        {
            oal_strncpy(ac_polynomial, pst_name, 4);
            //OAL_IO_PRINT("ac_polynomial:%s\n", ac_polynomial);
            pst_polynomial_param->as_polynomial_para[uc_ch_count][uc_count] = (oal_int16)oal_strtol(ac_polynomial, NULL, 16);

            OAM_INFO_LOG3(0, OAM_SF_ANY, "{wal_hipriv_set_polynomial_para::as_polynomial_para[%d][%d] = %d!}\r\n",
                uc_ch_count, uc_count, pst_polynomial_param->as_polynomial_para[uc_ch_count][uc_count]);//lint !e571
            pst_name += 4;
        }
    }

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_polynomial_para_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_polynomial_para::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_set_polynomial_para fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_set_upc_params(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_int8                        ac_para[5] = {0};
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int8                       *pst_name;
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    mac_cfg_show_upc_paras_stru    *pst_upc_param;
    oal_uint8                       uc_param_len;
    oal_uint8                       uc_param_num;
    oal_uint8                       uc_param_idx;
    mac_vap_stru                    *pst_mac_vap;
    mac_device_stru                 *pst_mac_device;
    oal_uint8                       uc_vap_idx;
    mac_vap_stru                    *pst_vap;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_UNLIKELY(NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_hipriv_auto_cali::can't get mac vap from netdevice priv data!}");
        return OAL_EINVAL;
    }

    pst_mac_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{wal_hipriv_auto_cali::mac_res_get_dev_etc fail.device_id = %u}",
                           pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ����device������vap */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_vap)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{wal_hipriv_auto_cali::pst_mac_vap(%d) null.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }
        /* �豸��up״̬���������ã�������down */
        if (pst_vap->en_vap_state != MAC_VAP_STATE_INIT)
        {
            OAM_ERROR_LOG2(pst_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_auto_cali::device is busy, please down it first, vap id: %d, state: %d!}\r\n",
                pst_vap->uc_vap_id, pst_vap->en_vap_state);
            return OAL_EBUSY;
        }
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_UPC_PARA, OAL_SIZEOF(mac_cfg_show_upc_paras_stru));
    pst_upc_param = (mac_cfg_show_upc_paras_stru *)(st_write_msg.auc_value);

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_upc_params::wal_get_cmd_one_arg_etc ch return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    uc_param_len = OAL_STRLEN(ac_name);

    /*ÿ������ֵռ��2��byte��5g 14��ֵ��2g 6��ֵ��ת��Ϊʮ�������ַ���������Ϊ4*14+13=69����4*6+5=29 */
    /* iwpriv Hisilicon0 set_upc_param "0010_0020_0020_0010_0020_0020" */
    /* iwpriv Hisilicon1 set_upc_param "0010_0020_0020_0010_0020_0020_0020_0020_0010_0020_0020_0010_0020_0020" */

    if(uc_param_len == WLAN_DYNC_CALI_5G_UPC_PARAM_STR_LEN)
    {
        uc_param_num = (uc_param_len - 13) >> 2;
    }
    else if(uc_param_len == WLAN_DYNC_CALI_2G_UPC_PARAM_STR_LEN)
    {
        uc_param_num = (uc_param_len - 5) >> 2;
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_upc_params::polynomial_para value not right}\r\n");
        return OAL_EINVAL;
    }


    pst_name = ac_name;

    for(uc_param_idx = 0; uc_param_idx < uc_param_num; uc_param_idx++)
    {
        oal_strncpy(ac_para, pst_name, 4);
        pst_upc_param->aus_upc_paras[uc_param_idx] = (oal_int16)oal_strtol(ac_para, NULL, 16);

        OAM_INFO_LOG2(0, OAM_SF_ANY, "{wal_hipriv_set_upc_params::aus_upc_para[%d] = %d!}\r\n",
            uc_param_idx,  pst_upc_param->aus_upc_paras[uc_param_idx]);

        pst_name += 5;
    }



    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_show_upc_paras_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_upc_params::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_set_upc_params fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_set_load_mode(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint8                       uc_load_mode;
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;

    /* ��ȡУ׼ͨ�� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_load_mode::wal_get_cmd_one_arg_etc vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    uc_load_mode = (oal_uint8)oal_atoi(ac_arg);

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_LOAD_MODE, OAL_SIZEOF(oal_uint8));
    *((oal_uint8 *)(st_write_msg.auc_value)) = uc_load_mode;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_load_mode::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_set_load_mode fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

#endif

#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)

OAL_STATIC oal_uint32  wal_hipriv_auto_cali(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;
    //oal_uint16                      us_len;
    oal_int8                        ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint8                       uc_cali_type;
    oal_uint32                      ul_ret;
    oal_uint32                      ul_off_set;
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;
    mac_vap_stru                    *pst_mac_vap;
    mac_device_stru                 *pst_mac_device;
    oal_uint8                       uc_vap_idx;
    mac_vap_stru                    *pst_vap;

    if (OAL_UNLIKELY(WAL_MSG_WRITE_MAX_LEN <= OAL_STRLEN(pc_param)))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_auto_cali:: pc_param overlength is %d}\n", OAL_STRLEN(pc_param));
        return OAL_FAIL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_UNLIKELY(NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_hipriv_auto_cali::can't get mac vap from netdevice priv data!}");
        return OAL_EINVAL;
    }

    pst_mac_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{wal_hipriv_auto_cali::mac_res_get_dev_etc fail.device_id = %u}",
                           pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ����device������vap */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_vap)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{wal_hipriv_auto_cali::pst_mac_vap(%d) null.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }
        /* �豸��up״̬���������ã�������down */
        if (pst_vap->en_vap_state != MAC_VAP_STATE_INIT)
        {
            OAM_ERROR_LOG2(pst_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_auto_cali::device is busy, please down it first, vap id: %d, state: %d!}\r\n",
                pst_vap->uc_vap_id, pst_vap->en_vap_state);
            return OAL_EBUSY;
        }
    }

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        uc_cali_type = 0xff;
    }
    else
    {
        uc_cali_type = (oal_uint8)oal_atoi(ac_arg);
    }
    *(st_write_msg.auc_value) = uc_cali_type;
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_auto_cali::cali type = %x!}\r\n", uc_cali_type);
    //oal_memcopy(st_write_msg.auc_value, pc_param, OAL_STRLEN(pc_param));

    //st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    //us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);


    /***************************************************************************
                              ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_AUTO_CALI, OAL_SIZEOF(uc_cali_type));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(uc_cali_type),
                             (oal_uint8 *)&st_write_msg,
                             OAL_TRUE,
                             &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_auto_cali::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

        /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_auto_cali fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_auto_cali::wal_send_cfg_event_etc return err_code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_set_cali_vref(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                  ul_ret;
    oal_uint8                   uc_chain_idx;
    oal_uint8                   uc_band_idx;
    oal_uint16                  us_vref_value;
    mac_cfg_set_cali_vref_stru *pst_cali_vref;

    /* ��ȡУ׼ͨ�� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_cali_ref::wal_get_cmd_one_arg_etc vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }
    uc_chain_idx = (oal_uint8)oal_atoi(ac_arg);

    /* ��ȡУ׼vref idx */
    pc_param = pc_param + ul_off_set;
    ul_ret   = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_cali_ref::wal_get_cmd_one_arg_etc vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }
    uc_band_idx = (oal_uint8)oal_atoi(ac_arg);

    /* ��ȡУ׼vrefֵ */
    pc_param = pc_param + ul_off_set;
    ul_ret   = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_cali_ref::wal_get_cmd_one_arg_etc vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    us_vref_value = (oal_uint16)oal_atoi(ac_arg);

    OAL_IO_PRINT("chain(%d):us_cali_ref(%d) = %u.\r\n",uc_chain_idx,uc_band_idx, us_vref_value);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CALI_VREF, OAL_SIZEOF(mac_cfg_set_cali_vref_stru));

    /* ��������������� */
    pst_cali_vref = (mac_cfg_set_cali_vref_stru *)(st_write_msg.auc_value);
    pst_cali_vref->uc_chain_idx = uc_chain_idx;
    pst_cali_vref->uc_band_idx  = uc_band_idx;
    pst_cali_vref->us_vref_value = us_vref_value;

    /***************************************************************************
                                    ���¼���wal�㴦��
    ***************************************************************************/

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_set_cali_vref_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_cali_ref::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    OAL_IO_PRINT("wal_hipriv_set_cali_ref:OAL_SUCC.\r\n");

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_SINGLE_CHIP_DUAL_BAND

OAL_STATIC oal_uint32  wal_hipriv_set_restrict_band(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru   st_write_msg;
    oal_int32            l_ret;
    oal_int8             ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32           ul_offset;
    oal_uint32           ul_ret;
    oal_uint8            uc_val;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_offset);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_restrict_band::need argument}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    uc_val = (oal_uint8)oal_atoi(ac_arg);

    if ((uc_val != WLAN_BAND_2G) && (uc_val != WLAN_BAND_5G) && (uc_val != WLAN_BAND_BUTT))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_restrict_band::invalid argument=%d", uc_val);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    st_write_msg.auc_value[0] = uc_val;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_RESTRICT_BAND, OAL_SIZEOF(oal_uint8));
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_restrict_band::wal_send_cfg_event_etc failed}");
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif
#if defined(_PRE_WLAN_FEATURE_DBAC) && defined(_PRE_WLAN_FEATRUE_DBAC_DOUBLE_AP_MODE)

OAL_STATIC oal_uint32  wal_hipriv_set_omit_acs(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru   st_write_msg;
    oal_int32            l_ret;
    oal_int8             ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32           ul_offset;
    oal_uint8            uc_val;
    oal_uint32           ul_ret;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_offset);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_omit_acs::need argument}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    uc_val = (oal_uint8)oal_atoi(ac_arg);

    if ((uc_val != OAL_TRUE) && (uc_val != OAL_FALSE))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_omit_acs::invalid argument=%d", uc_val);

        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    st_write_msg.auc_value[0] = uc_val;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_OMIT_ACS, OAL_SIZEOF(oal_uint8));
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_omit_acs::wal_send_cfg_event_etc failed}");
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_WDS

OAL_STATIC oal_uint32  wal_hipriv_wds_vap_mode(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret = OAL_SUCC;
    oal_uint8                       uc_wds_vap_mode = 0;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_wds_vap_mode::wal_get_cmd_one_arg_etc return err_code %d!}\r\n", ul_ret);
         return ul_ret;
    }

    uc_wds_vap_mode = (oal_uint8)oal_atoi(ac_name);

    if ( WDS_MODE_BUTT <= uc_wds_vap_mode )
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_hipriv_wds_vap_mode::invalid parameter.}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_WDS_VAP_MODE, OAL_SIZEOF(oal_uint8));
    *((oal_uint8 *)(st_write_msg.auc_value)) = uc_wds_vap_mode;  /* ��ȡWDSģʽ */

    ul_ret = (oal_uint32)wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_wds_vap_mode::return err code %d!}\r\n", ul_ret);
    }

    return ul_ret;
}


OAL_STATIC oal_uint32  wal_hipriv_wds_vap_show(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_ret = OAL_SUCC;


    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_WDS_VAP_SHOW, OAL_SIZEOF(oal_uint8));

    ul_ret = (oal_uint32)wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_wds_vap_show::return err code %d!}\r\n", ul_ret);
    }

    return ul_ret;
}


OAL_STATIC oal_uint32  wal_hipriv_wds_sta_add(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret = OAL_SUCC;
    mac_cfg_wds_sta_stru            *pst_wds_sta = OAL_PTR_NULL;
    mac_cfg_wds_sta_stru            st_wds_sta;      /* ��ʱ�����ȡ��use����Ϣ */

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_wds_sta_add::wal_get_cmd_one_arg_etc 1 return err_code %d!}\r\n", ul_ret);
         return ul_ret;
    }
    oal_strtoaddr(ac_name, st_wds_sta.auc_sta_mac);

    /* ƫ�ƣ�ȡ��һ������ */
    pc_param = pc_param + ul_off_set;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_wds_sta_add::wal_get_cmd_one_arg_etc 2 return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    oal_strtoaddr(ac_name, st_wds_sta.auc_node_mac);

    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_WDS_STA_ADD, OAL_SIZEOF(mac_cfg_wds_sta_stru));

    /* ��������������� */
    pst_wds_sta     = (mac_cfg_wds_sta_stru *)(st_write_msg.auc_value);
    oal_set_mac_addr(pst_wds_sta->auc_sta_mac, st_wds_sta.auc_sta_mac);
    oal_set_mac_addr(pst_wds_sta->auc_node_mac, st_wds_sta.auc_node_mac);

    ul_ret = (oal_uint32)wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_wds_sta_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_wds_sta_add::return err code %d!}\r\n", ul_ret);
    }

    return ul_ret;
}


OAL_STATIC oal_uint32  wal_hipriv_wds_sta_del(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret = OAL_SUCC;
    mac_cfg_wds_sta_stru            *pst_wds_sta = OAL_PTR_NULL;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_wds_sta_del::wal_get_cmd_one_arg_etc return err_code %d!}\r\n", ul_ret);
         return ul_ret;
    }

    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_WDS_STA_DEL, OAL_SIZEOF(mac_cfg_wds_sta_stru));

    /* ��������������� */
    pst_wds_sta     = (mac_cfg_wds_sta_stru *)(st_write_msg.auc_value);
    oal_strtoaddr(ac_name, pst_wds_sta->auc_sta_mac);
    oal_set_mac_addr_zero(pst_wds_sta->auc_node_mac);

    ul_ret = (oal_uint32)wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_wds_sta_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_wds_sta_del::return err code %d!}\r\n", ul_ret);
    }

    return ul_ret;
}


OAL_STATIC oal_uint32  wal_hipriv_wds_sta_age(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret = OAL_SUCC;
    oal_uint32                      ul_sta_aging = 0;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_wds_sta_age::wal_get_cmd_one_arg_etc return err_code %d!}\r\n", ul_ret);
         return ul_ret;
    }

    ul_sta_aging = (oal_uint32)oal_atoi(ac_name);

    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_WDS_STA_AGE, OAL_SIZEOF(oal_uint8));
    *((oal_uint32 *)(st_write_msg.auc_value)) = ul_sta_aging;  /* ��ȡWDSģʽ */

    ul_ret = (oal_uint32)wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_wds_sta_age::return err code %d!}\r\n", ul_ret);
    }

    return ul_ret;
}

#endif


OAL_STATIC oal_uint32  wal_hipriv_rssi_limit(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    mac_cfg_rssi_limit_stru        *pst_rssi_limit;
    oal_uint8                       uc_idx;

    /* ���������������,����ʼ�� */
    pst_rssi_limit = (mac_cfg_rssi_limit_stru *)(st_write_msg.auc_value);
    oal_memset((oal_void *)pst_rssi_limit, 0, OAL_SIZEOF(mac_cfg_rssi_limit_stru));

    //��ȡrssi_limit ����
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    pc_param = pc_param + ul_off_set;
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_rssi_limit::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    //��������Ĳ�������ȡ��Ӧ�Ĳ�����������
    for (uc_idx = 0; uc_idx < MAC_RSSI_LIMIT_TYPE_BUTT; uc_idx++)
    {
        if(0 == oal_strcmp(ac_name, g_ast_mac_rssi_config_table[uc_idx].puc_car_name))
        {
            break;
        }
    }
    if (MAC_RSSI_LIMIT_TYPE_BUTT == uc_idx)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CAR, "{wal_hipriv_rssi_limit:: parameter error !\r\n");
        return uc_idx;
    }
    pst_rssi_limit->en_rssi_limit_type = g_ast_mac_rssi_config_table[uc_idx].en_rssi_cfg_id;

    //��ȡ�����ľ�������ֵ
    if (MAC_RSSI_LIMIT_SHOW_INFO != pst_rssi_limit->en_rssi_limit_type)
    {
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        pc_param = pc_param + ul_off_set;
        if (OAL_SUCC != ul_ret)
        {
             OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_rssi_limit::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
             return ul_ret;
        }
    }

    switch (pst_rssi_limit->en_rssi_limit_type)
    {
        case MAC_RSSI_LIMIT_SHOW_INFO: break;
        case MAC_RSSI_LIMIT_DELTA:
            pst_rssi_limit->c_rssi_delta = (oal_int8)oal_atoi(ac_name); break;
        case MAC_RSSI_LIMIT_ENABLE:
            pst_rssi_limit->en_rssi_limit_enable_flag = (oal_bool_enum_uint8)oal_atoi(ac_name); break;
        case MAC_RSSI_LIMIT_THRESHOLD:
            pst_rssi_limit->c_rssi = (oal_int8)oal_atoi(ac_name);
            /*�����õ�rssiֵ�����˺Ϸ���Χ������ֱ�ӷ���*/
            if ((pst_rssi_limit->c_rssi < OAL_RSSI_SIGNAL_MIN) || (pst_rssi_limit->c_rssi > OAL_RSSI_SIGNAL_MAX))
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_rssi_limit::rssi limit error data[%d]!}\r\n", (oal_int32)pst_rssi_limit->c_rssi);
                return OAL_FAIL;
            }

            if (pst_rssi_limit->c_rssi > WLAN_FAR_DISTANCE_RSSI)
            {
                OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_hipriv_rssi_limit::rssi_limit exceed WLAN_FAR_DISTANCE_RSSI! s_rssi[%d], WLAN_FAR_DISTANCE_RSSI[%d]}\r\n",
                    (oal_int32)pst_rssi_limit->c_rssi, WLAN_FAR_DISTANCE_RSSI);
            }
            break;
        default:
            break;
    }

    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_RSSI_LIMIT_CFG, OAL_SIZEOF(mac_cfg_rssi_limit_stru));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_rssi_limit_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CAR, "{wal_hipriv_rssi_limit::return err code [%d]!}\r\n", ul_ret);
        return (oal_uint32)ul_ret;
    }

    return OAL_SUCC;

}

#ifdef _PRE_WLAN_REPORT_PRODUCT_LOG

OAL_STATIC oal_uint32  wal_hipriv_report_product_log_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    oal_uint8                       uc_flag;
    oal_uint8                      *pst_product_log_param;
    oal_int32                       l_ret;

    /* ��������������� */
    pst_product_log_param = (oal_uint8 *)(st_write_msg.auc_value);
    oal_memset((oal_void *)pst_product_log_param, 0, OAL_SIZEOF(oal_uint8));

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    pc_param = pc_param + ul_off_set;
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_report_product_log_cfg::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    uc_flag = (oal_uint8)oal_atoi(ac_name);

    // 0 ��1 ���ƿ���; 2 ��ȡ����ֵ
    if ((0 != uc_flag ) && (1 != uc_flag ) && (2 != uc_flag ))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_report_product_log_cfg:: input invalid[%d]}\r\n",uc_flag);
        return OAL_FAIL;
    }

    *pst_product_log_param = uc_flag;

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_report_product_log_cfg:: param=%d}\r\n", *pst_product_log_param);

    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_PRODUCT_LOG, OAL_SIZEOF(oal_uint8));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_report_product_log_cfg::return err code [%d]!}\r\n", ul_ret);
        return (oal_uint32)ul_ret;
    }

    return OAL_SUCC;

}
#endif

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
OAL_STATIC oal_uint32  wal_hipriv_tcp_ack_buf_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    oal_int32                       l_tmp;
    oal_uint8                       uc_idx;
    mac_cfg_tcp_ack_buf_stru        *pst_tcp_ack_param;

    pst_tcp_ack_param = (mac_cfg_tcp_ack_buf_stru *)(st_write_msg.auc_value);
    oal_memset((oal_void *)pst_tcp_ack_param, 0, OAL_SIZEOF(mac_cfg_tcp_ack_buf_stru));

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }
    pc_param = pc_param + ul_off_set;

    for (uc_idx = 0; uc_idx < MAC_TCP_ACK_BUF_TYPE_BUTT; uc_idx++)
    {
        if(0 == oal_strcmp(ac_name, g_ast_hmac_tcp_ack_buf_cfg_table[uc_idx].puc_string))
        {
            break;
        }
    }
    if (MAC_TCP_ACK_BUF_TYPE_BUTT == uc_idx)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg:: parameter error !\r\n");
        return uc_idx;
    }
    pst_tcp_ack_param->en_cmd = g_ast_hmac_tcp_ack_buf_cfg_table[uc_idx].en_tcp_ack_buf_cfg_id;

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    l_tmp = (oal_int32)oal_atoi(ac_name);
    if ((l_tmp < 0) || (l_tmp > 10*1000))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::  car param[%d] invalid! }\r\n", oal_atoi(ac_name));
        return OAL_FAIL;
    }

    if (MAC_TCP_ACK_BUF_ENABLE == pst_tcp_ack_param->en_cmd)
    {
        if ((OAL_FALSE == (oal_uint8)l_tmp) || (OAL_TRUE == (oal_uint8)l_tmp))
        {
            pst_tcp_ack_param->en_enable = (oal_uint8)l_tmp;
        }
        else
        {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg:: en_fast_aging_flag[%d] must be 0 or 1.}", (oal_uint8)l_tmp);
            return OAL_FAIL;
        }

    }
    if (MAC_TCP_ACK_BUF_TIMEOUT == pst_tcp_ack_param->en_cmd)
    {
        if (0 == (oal_uint8)l_tmp)
        {
            OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg:: timer_ms shoule not be 0.}");
            return OAL_FAIL;
        }

        pst_tcp_ack_param->uc_timeout_ms = (oal_uint8)l_tmp;
    }
    if (MAC_TCP_ACK_BUF_MAX == pst_tcp_ack_param->en_cmd)
    {
        if (0 == (oal_uint8)l_tmp)
        {
            OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg:: uc_count_limit shoule not be 0.}");
            return OAL_FAIL;
        }

        pst_tcp_ack_param->uc_count_limit = (oal_uint8)l_tmp;
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_TCP_ACK_BUF, OAL_SIZEOF(mac_cfg_tcp_ack_buf_stru));

    OAM_WARNING_LOG4(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::en_cmd[%d], en_enable[%d], uc_timeout_ms[%d] uc_count_limit[%d]!}\r\n",
        pst_tcp_ack_param->en_cmd,
        pst_tcp_ack_param->en_enable,
        pst_tcp_ack_param->uc_timeout_ms,
        pst_tcp_ack_param->uc_count_limit);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_tcp_ack_buf_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::return err code [%d]!}\r\n", ul_ret);
        return (oal_uint32)ul_ret;
    }

    return OAL_SUCC;


}
#endif


#ifdef _PRE_FEATURE_FAST_AGING
OAL_STATIC oal_uint32  wal_hipriv_fast_aging_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    oal_int32                       l_tmp;
    oal_uint8                       uc_idx;
    mac_cfg_fast_aging_stru        *pst_aging_cfg_param;

    /* ��������������� */
    pst_aging_cfg_param = (mac_cfg_fast_aging_stru *)(st_write_msg.auc_value);
    oal_memset((oal_void *)pst_aging_cfg_param, 0, OAL_SIZEOF(mac_cfg_fast_aging_stru));

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    pc_param = pc_param + ul_off_set;

    //��������Ĳ�������ȡ��Ӧ�Ĳ�������
    for (uc_idx = 0; uc_idx < MAC_FAST_AGING_TYPE_BUTT; uc_idx++)
    {
        if(0 == oal_strcmp(ac_name, g_ast_dmac_fast_aging_config_table[uc_idx].puc_string))
        {
            break;
        }
    }
    if (MAC_FAST_AGING_TYPE_BUTT == uc_idx)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CAR, "{wal_hipriv_fast_aging_cfg:: parameter error !\r\n");
        return uc_idx;
    }
    pst_aging_cfg_param->en_cmd = g_ast_dmac_fast_aging_config_table[uc_idx].en_fast_aging_cfg_id;


    //��ȡ������ֵ
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    pc_param = pc_param + ul_off_set;
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_CAR, "{wal_hipriv_fast_aging_cfg::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    l_tmp = (oal_int32)oal_atoi(ac_name);
    //������Ҫ >= 0 ��< 10*1000
    if ((l_tmp < 0) || (l_tmp > 10*1000))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CAR, "{wal_hipriv_fast_aging_cfg::  car param[%d] invalid! }\r\n", oal_atoi(ac_name));
        return OAL_FAIL;
    }

    if (MAC_FAST_AGING_ENABLE == pst_aging_cfg_param->en_cmd)
    {
        if ((OAL_FALSE == (oal_uint8)l_tmp) || (OAL_TRUE == (oal_uint8)l_tmp))
        {
            pst_aging_cfg_param->en_enable = (oal_uint8)l_tmp;
        }
        else
        {
            OAM_WARNING_LOG1(0, OAM_SF_CAR, "{wal_hipriv_fast_aging_cfg:: en_fast_aging_flag[%d] must be 0 or 1.}", (oal_uint8)l_tmp);
            return OAL_FAIL;
        }

    }
    if (MAC_FAST_AGING_TIMEOUT == pst_aging_cfg_param->en_cmd)
    {
        if (0 == (oal_uint16)l_tmp)
        {
            OAM_WARNING_LOG0(0, OAM_SF_CAR, "{wal_hipriv_fast_aging_cfg:: timer_ms shoule not be 0.}");
            return OAL_FAIL;
        }

        pst_aging_cfg_param->us_timeout_ms = (oal_uint16)l_tmp;
    }
    if (MAC_FAST_AGING_COUNT == pst_aging_cfg_param->en_cmd)
    {
        if (0 == (oal_uint8)l_tmp)
        {
            OAM_WARNING_LOG0(0, OAM_SF_CAR, "{wal_hipriv_fast_aging_cfg:: uc_count_limit shoule not be 0.}");
            return OAL_FAIL;
        }

        pst_aging_cfg_param->uc_count_limit = (oal_uint8)l_tmp;
    }

/***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_FAST_AGING, OAL_SIZEOF(mac_cfg_fast_aging_stru));

    OAM_WARNING_LOG4(0, OAM_SF_CAR, "{wal_hipriv_fast_aging_cfg::en_cmd[%d], en_fast_aging[%d], timer[%d] uc_count_limit[%d]!}\r\n",
        pst_aging_cfg_param->en_cmd,
        pst_aging_cfg_param->en_enable,
        pst_aging_cfg_param->us_timeout_ms,
        pst_aging_cfg_param->uc_count_limit);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_fast_aging_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CAR, "{wal_hipriv_fast_aging_cfg::return err code [%d]!}\r\n", ul_ret);
        return (oal_uint32)ul_ret;
    }

    return OAL_SUCC;


}

#endif

#ifdef _PRE_WLAN_FEATURE_CAR

OAL_STATIC oal_uint32  wal_hipriv_car_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{

    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    oal_uint32                      ul_tmp;
    oal_uint8                       uc_idx;
    mac_cfg_car_stru               *pst_car_cfg_param;

    /* ��������������� */
    pst_car_cfg_param = (mac_cfg_car_stru *)(st_write_msg.auc_value);
    oal_memset((oal_void *)pst_car_cfg_param, 0, OAL_SIZEOF(mac_cfg_car_stru));

    //��ȡdevice ���� vap ���� user ���� timer ���� enable
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    pc_param = pc_param + ul_off_set;
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_CAR, "{wal_hipriv_car_cfg::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    //��������Ĳ�������ȡ��Ӧ�Ĳ�������
    for (uc_idx = 0; uc_idx < MAC_CAR_TYPE_BUTT; uc_idx++)
    {
        if(0 == oal_strcmp(ac_name, g_ast_hmac_car_config_table[uc_idx].puc_car_name))
        {
            break;
        }
    }
    if (MAC_CAR_TYPE_BUTT == uc_idx)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CAR, "{wal_hipriv_car_cfg:: parameter error !\r\n");
        return uc_idx;
    }
    pst_car_cfg_param->en_car_flag = g_ast_hmac_car_config_table[uc_idx].en_car_cfg_id;

    //�����device_bw��vap_bw��user_bw����Ҫ��ȡon����down
    if ((MAC_CAR_DEVICE_LIMIT == pst_car_cfg_param->en_car_flag) || (MAC_CAR_VAP_LIMIT == pst_car_cfg_param->en_car_flag)|| (MAC_CAR_USER_LIMIT == pst_car_cfg_param->en_car_flag))
    {
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        pc_param = pc_param + ul_off_set;
        if(0 == oal_strcmp(ac_name, "up"))
        {
            pst_car_cfg_param->uc_car_up_down_type = HMAC_CAR_UPLINK ;
        }
        else if (0 == oal_strcmp(ac_name, "down"))
        {
            pst_car_cfg_param->uc_car_up_down_type = 1 ;
        }
        else
        {
            return OAL_FAIL;
        }
    }
    //�����user�Ļ���Ҫ��ȡmac��ַ
    if (MAC_CAR_USER_LIMIT == pst_car_cfg_param->en_car_flag)
    {
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        pc_param = pc_param + ul_off_set;
        if (OAL_SUCC != ul_ret)
        {
             OAM_WARNING_LOG1(0, OAM_SF_CAR, "{wal_hipriv_car_cfg::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
             return ul_ret;
        }
        /* ��ȡmac��ַ */
        oal_strtoaddr(ac_name, pst_car_cfg_param->auc_user_macaddr);
    }

    //����show_infoʱ����ȡlimit_kbps ����timer_cycle ����car_enable
    if ((MAC_CAR_SHOW_INFO != pst_car_cfg_param->en_car_flag))
    {
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        pc_param = pc_param + ul_off_set;
        if (OAL_SUCC != ul_ret)
        {
             OAM_WARNING_LOG1(0, OAM_SF_CAR, "{wal_hipriv_car_cfg::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
             return ul_ret;
        }

        //������Ҫ >= 0 ���������ٲ��ܳ���1000*1024kbps
        if ((0 > oal_atoi(ac_name)) || (1000*1024 < oal_atoi(ac_name)))
        {
            OAM_WARNING_LOG1(0, OAM_SF_CAR, "{wal_hipriv_car_cfg::  car param[%d] invalid! }\r\n", oal_atoi(ac_name));
            return OAL_FAIL;
        }

        ul_tmp = (oal_uint32)oal_atoi(ac_name);
        if (MAC_CAR_TIMER_CYCLE_MS == pst_car_cfg_param->en_car_flag)
        {
            if (0 == (oal_uint16)ul_tmp)
            {
                OAM_WARNING_LOG0(0, OAM_SF_CAR, "{wal_hipriv_car_cfg:: timer_ms shoule not be 0.}");
                return OAL_FAIL;
            }

            pst_car_cfg_param->us_car_timer_cycle_ms = (oal_uint16)ul_tmp;
        }
        else if (MAC_CAR_ENABLE == pst_car_cfg_param->en_car_flag)
        {
            if ((OAL_FALSE == (oal_uint8)ul_tmp) || (OAL_TRUE == (oal_uint8)ul_tmp))
            {
                pst_car_cfg_param->en_car_enable_flag = (oal_uint8)ul_tmp;
            }
            else
            {
                OAM_WARNING_LOG1(0, OAM_SF_CAR, "{wal_hipriv_car_cfg:: car_flag[%d] must be 0 or 1.}", (oal_uint8)ul_tmp);
                return OAL_FAIL;
            }
        }
        else if (MAC_CAR_MULTICAST_PPS == pst_car_cfg_param->en_car_flag)
        {
            pst_car_cfg_param->ul_car_multicast_pps_num = ul_tmp;
        }
        else
        {
            pst_car_cfg_param->ul_bw_limit_kbps = ul_tmp;
        }
    }

    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CAR_CFG, OAL_SIZEOF(mac_cfg_car_stru));

    OAM_WARNING_LOG4(0, OAM_SF_CAR, "{wal_hipriv_car_cfg::en_car_flag[%d], uc_car_up_down_type[%d], timer[%d] kbps[%d]!}\r\n",
        pst_car_cfg_param->en_car_flag,
        pst_car_cfg_param->uc_car_up_down_type,
        pst_car_cfg_param->us_car_timer_cycle_ms,
        pst_car_cfg_param->ul_bw_limit_kbps);
    OAM_WARNING_LOG2(0, OAM_SF_CAR, "{wal_hipriv_car_cfg::uc_car_enable[%d], multicast_pps_num[%d]!}\r\n", pst_car_cfg_param->en_car_enable_flag, pst_car_cfg_param->ul_car_multicast_pps_num);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_car_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CAR, "{wal_hipriv_car_cfg::return err code [%d]!}\r\n", ul_ret);
        return (oal_uint32)ul_ret;
    }

    return OAL_SUCC;

}
#endif

#ifdef _PRE_WLAN_WEB_CMD_COMM

OAL_STATIC oal_int32 wal_ioctl_set_hwaddr(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    oal_iw_point_stru          *pst_param;
    oal_uint32                  ul_ret;
    oal_int32                    l_ret;
    wal_msg_write_stru          st_write_msg;
    oal_sockaddr_stru           st_addr;
    oal_int8                    cmd_str[20] = {0};     /* Ԥ���ַ����ռ� */
    mac_cfg_staion_id_param_stru *pst_cfg = OAL_PTR_NULL;
    mac_vap_stru                 *pst_mac_vap;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == p_param)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_hwaddr::pst_net_dev/p_param null ptr error %d!}\r\n", pst_net_dev, p_param);
        return -OAL_EFAUL;
    }

    pst_param = (oal_iw_point_stru *)p_param;
    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_hwaddr::input str length is %d!}\r\n", pst_param->length);

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_hwaddr::pst_mac_vap NULL}");
        return OAL_FAIL;
    }
    if(WLAN_VAP_MODE_CONFIG != pst_mac_vap->en_vap_mode || pst_param->length >= 20)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_hwaddr::vap mode[%d] or length[%d] invalid}", pst_mac_vap->en_vap_mode, pst_param->length);
        return OAL_EINVAL;
    }

    ul_ret = oal_copy_from_user(cmd_str, pst_param->pointer, pst_param->length);

    /* copy_from_user������Ŀ���Ǵ��û��ռ俽�����ݵ��ں˿ռ䣬ʧ�ܷ���û�б��������ֽ������ɹ�����0 */
    if (ul_ret > 0)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_hwaddr::oal_copy_from_user return error code %d!}\r\n", ul_ret);
        return -OAL_EFAUL;
    }

    oal_strtoaddr(cmd_str, (oal_uint8 *)st_addr.sa_data);

    OAM_INFO_LOG3(0, OAM_SF_ANY, "{wal_ioctl_set_hwaddr::the mac address is 0x%x:XX::0x%x:0x%x!}\r\n", (oal_uint8)st_addr.sa_data[5], (oal_uint8)st_addr.sa_data[1], (oal_uint8)st_addr.sa_data[0]);

    if (oal_netif_running(pst_net_dev))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_hwaddr::cannot set address; device running!}\r\n");

        return -OAL_EBUSY;
    }
    /*lint +e774*//*lint +e506*/

    if (ETHER_IS_MULTICAST(st_addr.sa_data))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_hwaddr::can not set group/broadcast addr!}\r\n");

        return -OAL_EINVAL;
    }

    oal_set_mac_addr((oal_uint8 *)(pst_net_dev->dev_addr), (oal_uint8 *)(st_addr.sa_data));

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    return OAL_SUCC;
#endif

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_HWADDR, OAL_SIZEOF(mac_cfg_staion_id_param_stru));

    pst_cfg = (mac_cfg_staion_id_param_stru *)st_write_msg.auc_value;   /* ��дWID��Ӧ�Ĳ��� */
    oal_set_mac_addr(pst_cfg->auc_station_id, (oal_uint8 *)(st_addr.sa_data));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_staion_id_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    return l_ret;
}


OAL_STATIC oal_int32 wal_ioctl_setmac(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    oal_iw_point_stru                               *pst_param;
    oal_uint16                                      us_len;
    oal_int32                                        l_ret;
    oal_uint16                                      us_subid;
    wal_msg_write_stru                              st_write_msg;
    oal_int8                                        *puc_str = 0;
    mac_cfg_kick_user_param_stru                    *pst_kick_user_param;
    oal_uint32                                      ul_ret;
    oal_int8                                         uc_str[40] = {0};     /* Ԥ���ַ����ռ� */
    oal_uint32                                      ul_off_set = 0;
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
    mac_blacklist_stru                              *pst_blklst;
#endif

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == p_param)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_setmac::pst_net_dev/p_param null ptr error %d!}\r\n", pst_net_dev, p_param);
        return -OAL_EFAUL;
    }
    OAL_MEMZERO((oal_uint8*)&st_write_msg, OAL_SIZEOF(st_write_msg));

    pst_param = (oal_iw_point_stru *)p_param;
    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_ioctl_setmac::input str length is %d!}\r\n", pst_param->length);

    us_subid = pst_param->flags;
    puc_str  = (oal_int8 *)pst_param->pointer;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/

    switch (us_subid)
    {
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
        case WLAN_CFGID_KICK_USER:
        {
            us_len = OAL_MAC_ADDR_LEN * sizeof(oal_uint8);
            pst_kick_user_param = (mac_cfg_kick_user_param_stru *)(st_write_msg.auc_value);
            pst_kick_user_param->us_reason_code = MAC_UNSPEC_REASON;
            oal_strtoaddr(puc_str, pst_kick_user_param->auc_mac_addr);
        }
        break;
        case WLAN_CFGID_CLR_BLACK_LIST:
        {
            us_len = OAL_MAC_ADDR_LEN * sizeof(oal_uint8);
        }
        break;
#endif
#ifdef _PRE_WLAN_FEATURE_WDS
        case WLAN_CFGID_WDS_STA_ADD:
        {
            ul_ret = wal_get_cmd_one_arg_etc(puc_str, uc_str, &ul_off_set);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_setmac::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
                return OAL_FAIL;
            }
            oal_strtoaddr(uc_str, st_write_msg.auc_value);
            /* ƫ�ƣ�ȡ��һ������ */
            puc_str = puc_str + ul_off_set;
            ul_ret = wal_get_cmd_one_arg_etc(puc_str, uc_str, &ul_off_set);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_setmac::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
                return OAL_FAIL;
            }
            oal_strtoaddr(uc_str, st_write_msg.auc_value+OAL_MAC_ADDR_LEN);
            us_len = OAL_MAC_ADDR_LEN * 2 * sizeof(oal_uint8);
        }
        break;
#endif

        case WLAN_CFGID_BLOCK_STA_NS:
        {
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
            *((oal_int32 *)(st_write_msg.auc_value)) = 1;
            WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_BLACKLIST_MODE, OAL_SIZEOF(oal_int32));

            /* ������Ϣ */
            l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                                       (oal_uint8 *)&st_write_msg,
                                       OAL_FALSE,
                                       OAL_PTR_NULL);

            if (OAL_UNLIKELY(OAL_SUCC != l_ret))
            {
                OAM_WARNING_LOG2(0, OAM_SF_CFG, "{wal_ioctl_setmac:wal_send_cfg_event_etc: %d return[%d].}\r\n", WLAN_CFGID_BLACKLIST_MODE, l_ret);
                return OAL_EINVAL;
            }

            us_subid = WLAN_CFGID_ADD_BLACK_LIST;
            pst_blklst = (mac_blacklist_stru*)(st_write_msg.auc_value);
            ul_ret = wal_get_cmd_one_arg_etc(puc_str, uc_str, &ul_off_set);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_setmac::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
                return OAL_FAIL;
            }
            oal_strtoaddr(uc_str, pst_blklst->auc_mac_addr);
            /* ƫ�ƣ�ȡ��һ������ */
            puc_str = puc_str + ul_off_set;
            ul_ret = wal_get_cmd_one_arg_etc(puc_str, uc_str, &ul_off_set);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_setmac::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
                return OAL_FAIL;
            }
            pst_blklst->ul_aging_time = (oal_uint32)(oal_atoi(uc_str));
            us_len = OAL_SIZEOF(mac_blacklist_stru);
            break;
#else
            return OAL_EINVAL;
#endif
        }

        default:
        {
            oal_strtoaddr(puc_str, st_write_msg.auc_value);
            us_len = OAL_MAC_ADDR_LEN * sizeof(oal_uint8);
        }
        break;
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, us_subid, us_len);

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_ioctl_setmac:wal_send_cfg_event_etc return[%d].}\r\n", l_ret);
    }
    return l_ret;
}


OAL_STATIC oal_int32  wal_ioctl_get_stru_param(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_w, oal_int8 *pc_extra)
{
    oal_iw_point_stru               *pst_w = (oal_iw_point_stru *)p_w;
    oal_uint32                      ul_subioctl_id;
    wal_msg_query_stru              st_query_msg;
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;
    wal_msg_rsp_stru                *pst_query_rsp_msg;
    oal_int32                       l_ret;
    mac_vap_stru                    *pst_mac_vap;
    hmac_vap_stru                   *pst_hmac_vap;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_param::pst_mac_vap is null!}\r\n");
        return -OAL_EINVAL;
    }

    ul_subioctl_id = pst_w->flags;
    st_query_msg.en_wid = (oal_uint16)ul_subioctl_id;
    if((WLAN_CFGID_GET_PWR_REF == ul_subioctl_id))
    {
        l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                   WAL_MSG_TYPE_QUERY,
                                   WAL_MSG_WID_LENGTH,
                                   (oal_uint8 *)&st_query_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_stru_param::return err code %d!}\r\n", l_ret);
            return l_ret;
        }

        pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
        if (OAL_PTR_NULL == pst_hmac_vap)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_stru_param: hmac vap is null. vap id:%d", pst_mac_vap->uc_vap_id);
            return 0;
        }
        pst_hmac_vap->auc_query_flag[QUERY_ID_PWR_REF] = OAL_FALSE;
        /*lint -e730*/
        l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT((pst_hmac_vap->query_wait_q), (OAL_TRUE == pst_hmac_vap->auc_query_flag[QUERY_ID_PWR_REF]), (5*OAL_TIME_HZ));
        /*lint +e730*/
        if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_stru_param: query pwr ref timeout. ret:%d", l_ret);
        }
        pst_w->length = OAL_SIZEOF(mac_cfg_param_char_stru);
        oal_memcopy(pc_extra, &(pst_hmac_vap->st_param_char), pst_w->length);
    }
    else
    {
        /***************************************************************************
            ���¼���wal�㴦��
        ***************************************************************************/
        l_ret = wal_send_cfg_event_etc(pst_net_dev,
                            WAL_MSG_TYPE_QUERY,
                            WAL_MSG_WID_LENGTH,
                            (oal_uint8 *)&st_query_msg,
                            OAL_TRUE,
                            &pst_rsp_msg);

        if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
        {
            OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_ioctl_get_stru_param:: wal_alloc_cfg_event_etc return err code %d! id=%d}\r\n", l_ret,ul_subioctl_id);
            return l_ret;
        }

        /* ��������Ϣ */
        pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);
        if(!pst_query_rsp_msg->us_len)
        {
            pst_w->length = 0;
            oal_free(pst_rsp_msg);
            return OAL_SUCC;
        }
        switch (ul_subioctl_id)
        {
            /* ҵ���� */
            case WLAN_CFGID_GET_VAP_CAP:
            {
                mac_vap_param_stru *pst_vap_param;
                pst_vap_param = (mac_vap_param_stru *)(pst_query_rsp_msg->auc_value);
                pst_w->length = OAL_SIZEOF(mac_vap_param_stru);
                oal_memcopy(pc_extra, pst_vap_param, OAL_SIZEOF(mac_vap_param_stru));
            }
            break;
            case WLAN_CFGID_GET_CHAN_LIST:
            {
                mac_chan_list_stru *pst_chan_list;
                pst_chan_list = (mac_chan_list_stru *)(pst_query_rsp_msg->auc_value);
                pst_w->length = OAL_SIZEOF(mac_chan_list_stru);
                oal_memcopy(pc_extra, pst_chan_list, OAL_SIZEOF(mac_chan_list_stru));
            }
            break;
#ifdef _PRE_WLAN_FEATURE_WDS
            case WLAN_CFGID_GET_WDS_STA_NUM:
            {
                oal_wds_info_stru *pst_wds_info;
                pst_wds_info = (oal_wds_info_stru*)(pst_query_rsp_msg->auc_value);
                pst_w->length = OAL_SIZEOF(oal_wds_info_stru);
                oal_memcopy(pc_extra, pst_wds_info, pst_w->length);
                break;
            }
#endif
            default:
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_stru_param::CFG_ID invalid!: [%d].}\r\n", ul_subioctl_id);
                oal_free(pst_rsp_msg);
                return -OAL_EINVAL;
            }
        }
        oal_free(pst_rsp_msg);
    }
    return OAL_SUCC;
}


oal_int32  wal_io2cfg_get_chan_info(oal_net_device_stru *pst_net_dev, mac_cfg_channel_param_stru *pst_channel_param,
                                        mac_beacon_param_stru *st_beacon_param)
{
    oal_int32                       l_ret;
    wal_msg_query_stru              st_query_msg;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    wal_msg_rsp_stru               *pst_query_rsp_msg;
    oal_uint8                       uc_prot_idx;
    mac_cfg_mode_param_stru        *pst_mode_param;
    mac_vap_stru                   *pst_mac_vap;

    /* ��ȡvap id */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);

    st_query_msg.en_wid = WLAN_CFGID_MODE;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_io2cfg_get_chan_info::wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);
    pst_mode_param = (mac_cfg_mode_param_stru *)(pst_query_rsp_msg->auc_value);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_io2cfg_get_chan_info::null pointer.}\r\n");
        oal_free(pst_rsp_msg);
        return -OAL_EFAUL;
    }

    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        for (uc_prot_idx = 0; OAL_PTR_NULL != g_ast_mode_map_etc[uc_prot_idx].pc_name; uc_prot_idx++)
        {
            if ((g_ast_mode_map_etc[uc_prot_idx].en_mode == pst_mode_param->en_protocol) &&
                (g_ast_mode_map_etc[uc_prot_idx].en_band == pst_mode_param->en_band) &&
                (g_ast_mode_map_etc[uc_prot_idx].en_bandwidth == pst_mode_param->en_bandwidth))
            {
                break;
            }
        }
    }
    /* STAģʽ��Ƶ�κ�Ƶ���ں�AP����֮������Ӧ���˴����Ƚ�Э��ģʽ */
    else if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        for (uc_prot_idx = 0; OAL_PTR_NULL != g_ast_mode_map_etc[uc_prot_idx].pc_name; uc_prot_idx++)
        {
            if (g_ast_mode_map_etc[uc_prot_idx].en_mode == pst_mode_param->en_protocol)
            {
                break;
            }
        }
    }
    else
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_io2cfg_get_chan_info::vap mode error %d.}\r\n", pst_mac_vap->en_vap_mode);
        oal_free(pst_rsp_msg);
        return -OAL_EINVAL;
    }

    if (OAL_PTR_NULL == g_ast_mode_map_etc[uc_prot_idx].pc_name)
    {
        OAM_ERROR_LOG4(0, OAM_SF_ANY, "{wal_io2cfg_get_chan_info::invalid mode value, idx is %d, protocol is %d, band is %d, bandwidth is %d}\r\n",
            uc_prot_idx, pst_mode_param->en_protocol, pst_mode_param->en_band, pst_mode_param->en_bandwidth);
        oal_free(pst_rsp_msg);
        return -OAL_EINVAL;
    }

    pst_channel_param->en_band = g_ast_mode_map_etc[uc_prot_idx].en_band;
    pst_channel_param->en_bandwidth = g_ast_mode_map_etc[uc_prot_idx].en_bandwidth;
    st_beacon_param->en_protocol = g_ast_mode_map_etc[uc_prot_idx].en_mode;

    oal_free(pst_rsp_msg);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    st_query_msg.en_wid = WLAN_CFGID_CHANNEL;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_io2cfg_get_chan_info::return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);

    /* ҵ���� */
    pst_channel_param->uc_channel = *((oal_uint8 *)(pst_query_rsp_msg->auc_value));

    oal_free(pst_rsp_msg);

    return l_ret;
}

OAL_STATIC oal_int32  wal_ioctl_priv_set_essid(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *pst_data, oal_int8 *pc_ssid)
{
    oal_uint8                       uc_ssid_len;
    oal_int32                       l_ret;
    wal_msg_write_stru              st_write_msg;
    mac_cfg_ssid_param_stru        *pst_param;
    mac_vap_stru                   *pst_mac_vap;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_priv_set_essid::pst_mac_vap is null!}\r\n");
        return -OAL_EFAUL;
    }

    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        /* �豸��up״̬����APʱ�����������ã�������down */
        if (0 != (OAL_IFF_RUNNING & OAL_NETDEVICE_FLAGS(pst_net_dev)))
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_priv_set_essid::device is busy, please down it firste %d!}\r\n", OAL_NETDEVICE_FLAGS(pst_net_dev));
            return -OAL_EBUSY;
        }
    }

    pc_ssid = oal_strim(pc_ssid);                   /* ȥ���ַ�����ʼ��β�Ŀո� */

    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_priv_set_essid:: pst_data->flags, pst_data->lengt !}\r\n");

    //uc_ssid_len = (oal_uint8)pst_data->length;    /* ���Ȳ����� \0 */
    uc_ssid_len = (oal_uint8)OAL_STRLEN(pc_ssid);

    if (uc_ssid_len > WLAN_SSID_MAX_LEN - 1)        /* -1Ϊ\0Ԥ���ռ� */
    {
        uc_ssid_len = WLAN_SSID_MAX_LEN - 1;
    }

    OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_priv_set_essid:: ssid length is %d!}\r\n", uc_ssid_len);
    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SSID, OAL_SIZEOF(mac_cfg_ssid_param_stru));

    /* ��дWID��Ӧ�Ĳ��� */
    pst_param = (mac_cfg_ssid_param_stru *)(st_write_msg.auc_value);
    pst_param->uc_ssid_len = uc_ssid_len;
    oal_memcopy(pst_param->ac_ssid, pc_ssid, uc_ssid_len);

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ssid_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_priv_set_essid:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_priv_get_mode(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    oal_int32                       l_ret;
    wal_msg_query_stru              st_query_msg;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    wal_msg_rsp_stru               *pst_query_rsp_msg;
    oal_iw_point_stru              *pst_point;
    oal_uint8                       uc_prot_idx;
    mac_cfg_mode_param_stru        *pst_mode_param;
    oal_int8                       *pc_err_str = "Error protocal";
    mac_vap_stru                   *pst_mac_vap;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/

    st_query_msg.en_wid = WLAN_CFGID_MODE;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_priv_get_mode::wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);

    /* ҵ���� */
    pst_point = (oal_iw_point_stru *)p_param;

    pst_mode_param = (mac_cfg_mode_param_stru *)(pst_query_rsp_msg->auc_value);

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_priv_get_mode::null pointer.}\r\n");
        oal_free(pst_rsp_msg);
        return -OAL_EFAUL;
    }

    if ((WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode) || (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode))
    {
        for (uc_prot_idx = 0; OAL_PTR_NULL != g_ast_mode_map_etc[uc_prot_idx].pc_name; uc_prot_idx++)
        {
            if ((g_ast_mode_map_etc[uc_prot_idx].en_mode == pst_mode_param->en_protocol) &&
                (g_ast_mode_map_etc[uc_prot_idx].en_band == pst_mode_param->en_band) &&
                (g_ast_mode_map_etc[uc_prot_idx].en_bandwidth == pst_mode_param->en_bandwidth))
            {
                break;
            }
        }
    }
    else
    {
        oal_free(pst_rsp_msg);
        return OAL_SUCC;
    }

    if (OAL_PTR_NULL == g_ast_mode_map_etc[uc_prot_idx].pc_name)
    {
        pst_point->length = (oal_uint16)OAL_STRLEN(pc_err_str);
        oal_memcopy(pc_extra, pc_err_str, pst_point->length);
        oal_free(pst_rsp_msg);
        return OAL_SUCC;
    }

    pst_point->length = (oal_uint16)OAL_STRLEN(g_ast_mode_map_etc[uc_prot_idx].pc_name);
    oal_memcopy(pc_extra, g_ast_mode_map_etc[uc_prot_idx].pc_name, pst_point->length);

    oal_free(pst_rsp_msg);
    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_priv_get_essid(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iw_point_stru *pst_data, oal_int8 *pc_ssid)
{
    oal_int32                       l_ret;
    wal_msg_query_stru              st_query_msg;
    mac_cfg_ssid_param_stru        *pst_ssid;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    wal_msg_rsp_stru               *pst_query_rsp_msg;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    st_query_msg.en_wid = WLAN_CFGID_SSID;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_priv_get_essid:: wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);
    /* ҵ���� */
    pst_ssid = (mac_cfg_ssid_param_stru *)(pst_query_rsp_msg->auc_value);

    pst_data->flags = 1;    /* ���ó��α�־Ϊ��Ч */
    pst_data->length = pst_ssid->uc_ssid_len;

    oal_memcopy(pc_ssid, pst_ssid->ac_ssid, pst_ssid->uc_ssid_len);

    oal_free(pst_rsp_msg);
    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_ioctl_priv_set_mode(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, void *p_param, oal_int8 *pc_extra)
{
    oal_iw_point_stru          *pst_param;
    oal_uint32                  ul_ret;
    oal_int32                   l_ret;
    oal_int8                    ac_mode_str[24] = {0};     /* Ԥ��Э��ģʽ�ַ����ռ� */
    oal_int8                    ac_result[100] = {0};
    oal_uint8                   uc_prot_idx;
    mac_cfg_mode_param_stru    *pst_mode_param;
    wal_msg_write_stru          st_write_msg;
    mac_vap_stru                *pst_mac_vap;
    mac_device_stru             *pst_mac_device;
    oal_uint8                   uc_vap_idx;
    mac_vap_stru                *pst_vap;
    wal_msg_stru                *pst_rsp_msg = OAL_PTR_NULL;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == p_param)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_priv_set_mode::pst_net_dev/p_param null ptr error %d!}\r\n", pst_net_dev, p_param);
        return -OAL_EFAUL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_UNLIKELY(NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_ioctl_priv_set_mode::can't get mac vap from netdevice priv data!}");
        return -OAL_EINVAL;
    }

    pst_mac_device              = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{wal_ioctl_priv_set_mode::mac_res_get_dev_etc fail.device_id = %u}",
                           pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ����device������vap */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_vap)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{wal_ioctl_priv_set_mode::pst_mac_vap(%d) null.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }
#ifndef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
        /* �豸��up״̬���������ã�������down */
        if (pst_vap->en_vap_state != MAC_VAP_STATE_INIT)
        {
            OAM_ERROR_LOG2(pst_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_priv_set_mode::device is busy, please down it first, vap id: %d, state: %d!}\r\n",
                pst_vap->uc_vap_id, pst_vap->en_vap_state);
            return -OAL_EBUSY;
        }
#endif
    }

    pst_param = (oal_iw_point_stru *)p_param;
    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_ioctl_priv_set_mode::input str length is %d!}\r\n", pst_param->length);

    if (pst_param->length > OAL_SIZEOF(ac_mode_str))    /* �ַ����ж�, ���Ȱ���\0 */
    {
        pst_param->length =  OAL_SIZEOF(ac_mode_str);
    }

    ul_ret = oal_copy_from_user(ac_mode_str, pst_param->pointer, pst_param->length);

    /* copy_from_user������Ŀ���Ǵ��û��ռ俽�����ݵ��ں˿ռ䣬ʧ�ܷ���û�б��������ֽ������ɹ�����0 */
    if (ul_ret > 0)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_priv_set_mode::oal_copy_from_user return error code %d!}\r\n", ul_ret);
        return -OAL_EFAUL;
    }

    ac_mode_str[OAL_SIZEOF(ac_mode_str) - 1] = '\0';    /* ȷ����null��β */

    for (uc_prot_idx = 0; OAL_PTR_NULL != g_ast_mode_map_etc[uc_prot_idx].pc_name; uc_prot_idx++)
    {
        l_ret = oal_strcmp(g_ast_mode_map_etc[uc_prot_idx].pc_name, ac_mode_str);

        if (0 == l_ret)
        {
            break;
        }
    }

    if (OAL_PTR_NULL == g_ast_mode_map_etc[uc_prot_idx].pc_name)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_priv_set_mode::unrecognized protocol string!}\r\n");
        return -OAL_EINVAL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_PRIV_MODE, OAL_SIZEOF(mac_cfg_mode_param_stru));

    pst_mode_param = (mac_cfg_mode_param_stru *)(st_write_msg.auc_value);
    pst_mode_param->en_protocol  = g_ast_mode_map_etc[uc_prot_idx].en_mode;
    pst_mode_param->en_band      = g_ast_mode_map_etc[uc_prot_idx].en_band;
    pst_mode_param->en_bandwidth = g_ast_mode_map_etc[uc_prot_idx].en_bandwidth;

    //�û����õ�ģʽ�ַ�������δЯ��������չ��Ϣ����Я���Ĵ�����չ��Ϣ�뵱ǰ�ŵ���ƥ��
    //��������ݵ�ǰ�ŵ��Զ�ƥ����ʵĴ�����չ����
    if(0 == pst_mac_vap->st_channel.uc_chan_number || (MAC_ACS_SW_INIT == mac_get_acs_switch(pst_mac_device)))
    {
        //�ŵ��Ż�δ������δ���ô�����չ����ʱ������������һ��Ĭ�ϵ���չ����
        if(WLAN_BAND_WIDTH_40M == pst_mode_param->en_bandwidth)
        {
            pst_mode_param->en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
        }
        else if(WLAN_BAND_WIDTH_80M == pst_mode_param->en_bandwidth)
        {
            pst_mode_param->en_bandwidth = WLAN_BAND_WIDTH_80PLUSPLUS;
        }
    }
    else
    {
        pst_mode_param->en_bandwidth = mac_regdomain_get_support_bw_mode(pst_mode_param->en_bandwidth,pst_mac_vap->st_channel.uc_chan_number);
    }

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_priv_set_mode::protocol=%d,band=%d,bw=%d,ch=%d!}\r\n",
                     pst_mode_param->en_protocol,
                     pst_mode_param->en_band,
                     pst_mode_param->en_bandwidth,
                     pst_mac_vap->st_channel.uc_chan_number);

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_mode_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_priv_set_mode::wal_alloc_cfg_event_etc return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_ioctl_priv_set_mode fail, err code[%u]!}\r\n", ul_ret);
        return -((oal_int32)ul_ret);
    }

    pst_param->length = (oal_uint16)OAL_SPRINTF(ac_result, sizeof(ac_result), "%s\nsuccess",
                ac_mode_str);
    oal_memcopy(pc_extra, ac_result, pst_param->length);
    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_get_sta_stat_info(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    oal_iw_point_stru           *pst_param;
    oal_uint32                  ul_ret;
    oal_int8                    cmd_str[20] = {0};     /* Ԥ���ַ����ռ� */
    oal_uint8                   auc_mac[WLAN_MAC_ADDR_LEN];
    oal_sockaddr_stru           st_addr;
    oal_int                     *param;
    oal_uint32                   ul_subioctl_id;
    oal_uint32                  *pst_ul_param;


    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == p_param)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_get_sta_stat_info::pst_net_dev/p_param null ptr error %d!}\r\n", pst_net_dev, p_param);
        return -OAL_EFAUL;
    }

    pst_param = (oal_iw_point_stru *)p_param;
    if (pst_param->length >= 20)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_sta_stat_info::param length invalid %d!}\r\n", pst_param->length);
        return -OAL_EINVAL;
    }

    ul_ret = oal_copy_from_user(cmd_str, pst_param->pointer, pst_param->length);

    /* copy_from_user������Ŀ���Ǵ��û��ռ俽�����ݵ��ں˿ռ䣬ʧ�ܷ���û�б��������ֽ������ɹ�����0 */
    if (ul_ret > 0)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_sta_stat_info::oal_copy_from_user return error code %d!}\r\n", ul_ret);
        return -OAL_EFAUL;
    }
    oal_strtoaddr(cmd_str, (oal_uint8 *)st_addr.sa_data);
    OAM_WARNING_LOG3(0, OAM_SF_ANY, "{wal_ioctl_get_sta_stat_info::the mac address is 0x%x:XX::0x%x:0x%x!}\r\n", (oal_uint8)st_addr.sa_data[5], (oal_uint8)st_addr.sa_data[1], (oal_uint8)st_addr.sa_data[0]);

    oal_set_mac_addr(auc_mac, (oal_uint8 *)(st_addr.sa_data));
    OAM_WARNING_LOG3(0, OAM_SF_ANY, "{wal_ioctl_get_sta_stat_info::the mac address is 0x%x:XX::0x%x:0x%x!}\r\n", auc_mac[0], auc_mac[1], auc_mac[2]);

    ul_subioctl_id = pst_param->flags;

    if (WLAN_CFGID_QUERY_RSSI == ul_subioctl_id)
    {
        param = (oal_int *)pc_extra;
        /* ����rssiֵ */
        param[0] = wal_config_get_sta_rssi(pst_net_dev, auc_mac, (oal_uint8 *)pc_extra);
        pst_param->length = OAL_SIZEOF(oal_int);

        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_sta_stat_info::rssi=%d}\r\n", &param[0]);
    }
#ifdef _PRE_WLAN_11K_STAT
    else if (WLAN_CFGID_QUERY_DROP_NUM == ul_subioctl_id)
    {
        /* �����β������ */
        pst_ul_param = wal_config_get_sta_drop_num(pst_net_dev, auc_mac, (oal_uint8 *)pc_extra);
        if(OAL_PTR_NULL == pst_ul_param)
        {
            return OAL_FAIL;
        }
        oal_memcopy(pc_extra, pst_ul_param, OAL_SIZEOF(oal_uint32)*WLAN_WME_AC_BUTT);
        pst_param->length = OAL_SIZEOF(oal_uint32)*WLAN_WME_AC_BUTT;

        OAM_WARNING_LOG4(0, OAM_SF_ANY, "{wal_ioctl_get_sta_stat_info::drop_num=%d %d %d %d}\r\n",
            pst_ul_param[0], pst_ul_param[1], pst_ul_param[2], pst_ul_param[3]);
    }
    else if (WLAN_CFGID_QUERY_TX_DELAY == ul_subioctl_id)
    {
        /* �������ʱֵ */
        pst_ul_param = wal_config_get_sta_tx_delay(pst_net_dev, auc_mac, (oal_uint8 *)pc_extra);
        if(OAL_PTR_NULL == pst_ul_param)
        {
            return OAL_FAIL;
        }
        oal_memcopy(pc_extra, pst_ul_param, OAL_SIZEOF(oal_uint32)*3);
        pst_param->length = OAL_SIZEOF(oal_uint32)*3;

        OAM_WARNING_LOG3(0, OAM_SF_ANY, "{wal_ioctl_get_sta_stat_info::max tx_delay=%d, min_tx_delay=%d, ave_tx_delay=%d}\r\n",
            pst_ul_param[0], pst_ul_param[1], pst_ul_param[2]);
    }
#endif
    else
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_sta_stat_info::no subioctl_id %d }\r\n", ul_subioctl_id);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_DFS

OAL_STATIC oal_int32  wal_ioctl_get_dfs_chn_stat(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_iw, oal_int8 *pc_extra)
{
    oal_int *param;

    param = (oal_int *)pc_extra;

    param[0] = (oal_int)wal_config_get_dfs_chn_status(pst_net_dev, (oal_uint8 *)pc_extra);

    return OAL_SUCC;
}
#endif

#endif

#ifdef _PRE_WLAN_PRODUCT_1151V200

OAL_STATIC oal_uint32 wal_hipriv_80m_rts_debug(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    oal_uint8                       uc_flag;
    oal_uint8                      *pst_param;
    oal_int32                       l_ret;

    /* ��������������� */
    pst_param = (oal_uint8 *)(st_write_msg.auc_value);
    oal_memset((oal_void *)pst_param, 0, OAL_SIZEOF(oal_uint8));

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_80m_rts_debug::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    uc_flag = (oal_uint8)oal_atoi(ac_name);

    *pst_param = uc_flag;

    /***************************************************************************
                            ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_80M_RTS_DEBUG, OAL_SIZEOF(oal_uint8));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                              WAL_MSG_TYPE_WRITE,
                              WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                              (oal_uint8 *)&st_write_msg,
                              OAL_FALSE,
                              OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
       OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_80m_rts_debug::return err code [%d]!}\r\n", ul_ret);
       return (oal_uint32)ul_ret;
    }

    return OAL_SUCC;
}
#endif

#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)

OAL_STATIC oal_int32  wal_ioctl_get_cali_power(oal_net_device_stru *pst_net_dev, oal_iw_point_stru *pst_param, oal_int8 *pc_param, oal_int8 *pc_result)
{
    mac_vap_stru                    *pst_mac_vap;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_int32                       i_leftime;
    oal_int32                       l_ret_len = 0;
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_freq;
    oal_int32                       l_ch;
    oal_uint8                       uc_count;
    mac_cfg_cali_power_stru         *pst_power_param;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_cali_power::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,"{wal_ioctl_get_cali_power::mac_res_get_hmac_vap failed!}");
        return -OAL_FAIL;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;

    //ul_ret = wal_hipriv_cali_power(pst_net_dev, ac_param);
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_GET_CALI_POWER, OAL_SIZEOF(mac_cfg_cali_power_stru));
    pst_power_param = (mac_cfg_cali_power_stru *)(st_write_msg.auc_value);

    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_cali_power::wal_get_cmd_one_arg_etc ch return err_code %d!}\r\n", ul_ret);
        /* �������������ȡ����cali powerֵ */
        pst_power_param->uc_ch = 2;
    }
    else
    {
        l_ch = oal_atoi(ac_name);
        if(l_ch < 0 || l_ch > 1)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_cali_power::ch is invalid!}\r\n");
            return -OAL_ERR_CODE_INVALID_CONFIG;
        }
        pst_power_param->uc_ch = (oal_uint8)l_ch;
        OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_cali_power::l_ch = %d!}\r\n", l_ch);

        pc_param = pc_param + ul_off_set;
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_cali_power::wal_get_cmd_one_arg_etc freq return err_code %d!}\r\n", ul_ret);
            return -((oal_int32)ul_ret);
        }

        l_freq = oal_atoi(ac_name);
        pst_power_param->uc_freq = (oal_uint8)l_freq;
        OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_cali_power::l_freq = %d!}\r\n", l_freq);
    }

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_cali_power_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_cali_power::return err code [%d]!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_ioctl_get_cali_power fail, err code[%u]!}\r\n", ul_ret);
        return -((oal_int32)ul_ret);
    }

        /*�����ȴ�dmac�ϱ�*/
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(oal_uint32)(OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag),WAL_ATCMDSRB_GET_RX_PCKT);

    if ( 0 == i_leftime)
    {
        /* ��ʱ��û���ϱ�ɨ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_get_cali_power wait for %ld ms timeout!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_ETIMEDOUT;
    }
    else if (i_leftime < 0)
    {
        /* ��ʱ���ڲ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_get_cali_power wait for %ld ms error!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_ETIMEDOUT;
    }
    else
    {
        l_ret_len = OAL_SPRINTF(pc_result, MAX_EQUIPMENT_GET_ARGS_SIZE, "\n");
        pst_param->length = l_ret_len;
        pc_result += l_ret_len;

        for(uc_count = 0; uc_count < pst_hmac_vap->st_polynomial_paras.l_length; uc_count++)
        {
            l_ret_len = OAL_SPRINTF(pc_result, MAX_EQUIPMENT_GET_ARGS_SIZE, "%03d ", pst_hmac_vap->st_polynomial_paras.as_polynomial_paras[uc_count]);
            pc_result += l_ret_len;
            pst_param->length += l_ret_len;
        }

        l_ret_len = OAL_SPRINTF(pc_result, MAX_EQUIPMENT_GET_ARGS_SIZE, "\nsuccess\n");
        pc_result += l_ret_len;
        pst_param->length += l_ret_len;

        return OAL_SUCC;
    }
}

oal_int32  wal_ioctl_get_power_param(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    oal_uint32                      ul_ret;
    oal_iw_point_stru               *pst_w = (oal_iw_point_stru *)p_param;
    mac_vap_stru                    *pst_mac_vap;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_int32                       i_leftime;
    oal_int8                        ac_param[] = "2";
    oal_int32                       l_ret_len = 0;
    oal_uint8                       uc_count;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_power_param::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,"{wal_ioctl_get_power_param::mac_res_get_hmac_vap failed!}");
        return OAL_FAIL;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;

    ul_ret = wal_hipriv_cali_power(pst_net_dev, ac_param);

    if (OAL_SUCC != ul_ret)
    {
         OAL_IO_PRINT("{wal_ioctl_get_power_param return err_code [%d]!}\r\n", ul_ret);
         return -((oal_int32)ul_ret);
    }

        /*�����ȴ�dmac�ϱ�*/
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(oal_uint32)(OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag),WAL_ATCMDSRB_GET_RX_PCKT);

    if ( 0 == i_leftime)
    {
        /* ��ʱ��û���ϱ�ɨ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_get_power_param wait for %ld ms timeout!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_ETIMEDOUT;
    }
    else if (i_leftime < 0)
    {
        /* ��ʱ���ڲ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_get_power_param ait for %ld ms error!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_ETIMEDOUT;
    }
    else
    {
        l_ret_len = OAL_SPRINTF(pc_extra, 300, "\n");
        pst_w->length = l_ret_len;
        pc_extra += l_ret_len;

        for(uc_count = 0; uc_count < pst_hmac_vap->st_polynomial_paras.l_length; uc_count++)
        {
            if((uc_count != 0) && (0 == uc_count%3))
            {
                l_ret_len = OAL_SPRINTF(pc_extra, 300, "_");
                pst_w->length += l_ret_len;
                pc_extra += l_ret_len;
            }
            l_ret_len = OAL_SPRINTF(pc_extra, 300, "%04x", (oal_uint16)pst_hmac_vap->st_polynomial_paras.as_polynomial_paras[uc_count]);
            pst_w->length += l_ret_len;
            pc_extra += l_ret_len;
        }

        l_ret_len = OAL_SPRINTF(pc_extra, 300, "\nsuccess\n");
        pst_w->length += l_ret_len;

        return OAL_SUCC;
    }
}


OAL_STATIC oal_int32  wal_ioctl_get_upc_params(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    mac_vap_stru                    *pst_mac_vap;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_int32                       i_leftime;
    oal_int32                       l_ret_len = 0;
    wal_msg_write_stru              st_write_msg;
    oal_uint8                       uc_count;
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;
    oal_iw_point_stru               *pst_w = (oal_iw_point_stru *)p_param;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_upc_param::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,"{wal_ioctl_get_upc_param::mac_res_get_hmac_vap failed!}");
        return -OAL_FAIL;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_GET_UPC_PARA, OAL_SIZEOF(mac_cfg_show_upc_paras_stru));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_show_upc_paras_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_upc_param::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_ioctl_get_upc_param fail, err code[%u]!}\r\n", ul_ret);
        return ul_ret;
    }

    /*�����ȴ�dmac�ϱ�*/
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(oal_uint32)(OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag),WAL_ATCMDSRB_GET_RX_PCKT);

    if ( 0 == i_leftime)
    {
        /* ��ʱ��û���ϱ�ɨ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_get_upc_param wait for %ld ms timeout!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_ETIMEDOUT;
    }
    else if (i_leftime < 0)
    {
        /* ��ʱ���ڲ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_get_upc_param wait for %ld ms error!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_ETIMEDOUT;
    }
    else
    {
        l_ret_len = OAL_SPRINTF(pc_extra, MAX_EQUIPMENT_GET_ARGS_SIZE, "\n");
        pst_w->length = l_ret_len;
        pc_extra += l_ret_len;

        for(uc_count = 0; uc_count < pst_hmac_vap->st_upc_paras.ul_length; uc_count++)
        {
            if(uc_count != 0)
            {
                l_ret_len = OAL_SPRINTF(pc_extra, MAX_EQUIPMENT_GET_ARGS_SIZE, "_");
                pc_extra += l_ret_len;
                pst_w->length += l_ret_len;
            }

            l_ret_len = OAL_SPRINTF(pc_extra, MAX_EQUIPMENT_GET_ARGS_SIZE, "%04x", pst_hmac_vap->st_upc_paras.aus_upc_paras[uc_count]);
            pc_extra += l_ret_len;
            pst_w->length += l_ret_len;
        }

        l_ret_len = OAL_SPRINTF(pc_extra, MAX_EQUIPMENT_GET_ARGS_SIZE, "\nsuccess\n");
        pc_extra += l_ret_len;
        pst_w->length += l_ret_len;

        return OAL_SUCC;
    }
}

#endif

#ifdef _PRE_FEATURE_FAST_AGING
oal_int32  wal_ioctl_get_fast_aging(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    oal_int32                       l_ret;
    oal_iw_point_stru               *pst_w = (oal_iw_point_stru *)p_param;
    wal_msg_write_stru              st_query_msg;
    mac_cfg_fast_aging_stru        *pst_aging_cfg_param;
    mac_vap_stru                   *pst_mac_vap;
    hmac_vap_stru                  *pst_hmac_vap;

    /***************************************************************************
                                ���¼���wal�㴦��
        ***************************************************************************/
    st_query_msg.en_wid = (oal_uint16)WLAN_CFGID_GET_FAST_AGING;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_ppm::return err code %d!}\r\n", l_ret);
        return -l_ret;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_param_char::pst_mac_vap is null!}\r\n");
        return -OAL_EINVAL;
    }
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_param_char: hmac vap is null. vap id:%d", pst_mac_vap->uc_vap_id);
        return 0;
    }

    pst_hmac_vap->auc_query_flag[QUERY_ID_FAST_AGING] = OAL_FALSE;
    /*lint -e730*/
    l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT((pst_hmac_vap->query_wait_q), (OAL_TRUE == pst_hmac_vap->auc_query_flag[QUERY_ID_FAST_AGING]), (5*OAL_TIME_HZ));
    /*lint +e730*/
    if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_param_char: query rssi timeout. ret:%d", l_ret);
    }

    pst_aging_cfg_param = (mac_cfg_fast_aging_stru*)pst_hmac_vap->st_param_char.auc_buff;


    OAL_SPRINTF(pc_extra, WLAN_IWPRIV_MAX_BUFF_LEN,
                "fast aging: enable=%d, timeout=%d, count=%d\nsuccess\n",
                pst_aging_cfg_param->en_enable, pst_aging_cfg_param->us_timeout_ms, pst_aging_cfg_param->uc_count_limit);
    pst_w->length = (oal_uint16)OAL_STRLEN(pc_extra);




    return OAL_SUCC;
}

#endif

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST

oal_int32  wal_ioctl_get_ppm(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    oal_int32                       l_ret;
    oal_iw_point_stru               *pst_w = (oal_iw_point_stru *)p_param;
    wal_msg_write_stru              st_query_msg;
    oal_int8                        ac_result[100] = {0};
    oal_int32                       l_ret_len = 0;
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;
    wal_msg_rsp_stru                *pst_query_rsp_msg = OAL_PTR_NULL;
    oal_int8                        c_ppm;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    st_query_msg.en_wid = WLAN_CFGID_ADJUST_PPM;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_ppm::return err code %d!}\r\n", l_ret);
        return -l_ret;
    }

    /* ��������Ϣ */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);
    c_ppm = *((oal_int8 *)(pst_query_rsp_msg->auc_value));

    oal_free(pst_rsp_msg);

    l_ret_len = OAL_SPRINTF(ac_result, sizeof(ac_result),
                "%d\nsuccess\n", c_ppm);
    oal_memcopy(pc_extra, ac_result, OAL_STRLEN(ac_result));
    pst_w->length = (oal_uint16)l_ret_len;

    return OAL_SUCC;
}

oal_int32  wal_ioctl_get_cali_status(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    oal_int32                       l_ret;
    oal_iw_point_stru               *pst_w = (oal_iw_point_stru *)p_param;
    wal_msg_write_stru              st_query_msg;
    oal_int8                        ac_result[100] = {0};
    oal_int32                       l_ret_len = 0;
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;
    wal_msg_rsp_stru                *pst_query_rsp_msg = OAL_PTR_NULL;
    oal_bool_enum_uint8             en_cali_rdy;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    st_query_msg.en_wid = WLAN_CFGID_AUTO_CALI;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_ppm::return err code %d!}\r\n", l_ret);
        return -l_ret;
    }

    /* ��������Ϣ */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);
    en_cali_rdy = *((oal_int8 *)(pst_query_rsp_msg->auc_value));

    oal_free(pst_rsp_msg);

    l_ret_len = OAL_SPRINTF(ac_result, sizeof(ac_result),
                "%s\nsuccess\n", (en_cali_rdy) ? "ready":"not ready");
    oal_memcopy(pc_extra, ac_result, OAL_STRLEN(ac_result));
    pst_w->length = (oal_uint16)l_ret_len;

    return OAL_SUCC;
}

OAL_STATIC oal_int32  wal_ioctl_get_rx_info(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    oal_uint32                      ul_ret;
    oal_iw_point_stru               *pst_w = (oal_iw_point_stru *)p_param;
    mac_vap_stru                    *pst_mac_vap;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_int32                       i_leftime;
    oal_int8                        ac_param[] = "1 2";
    oal_int8                        ac_result[100] = {0};
    oal_int32                       l_ret_len = 0;
    oal_int8                        *pst_fail_str = "\nfail";

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_rx_info::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,"{wal_ioctl_get_rx_info::mac_res_get_hmac_vap failed!}");
        return OAL_FAIL;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_rx_pkct_flag = OAL_FALSE;

    ul_ret = wal_hipriv_rx_fcs_info(pst_net_dev, ac_param);

    if (OAL_SUCC != ul_ret)
    {
         OAL_IO_PRINT("{wal_ioctl_get_rx_info return err_code [%d]!}\r\n", ul_ret);
         goto fail;
    }

        /*�����ȴ�dmac�ϱ�*/
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(oal_uint32)(OAL_TRUE == pst_hmac_vap->st_atcmdsrv_get_status.uc_get_rx_pkct_flag),WAL_ATCMDSRB_GET_RX_PCKT);

    if ( 0 == i_leftime)
    {
        /* ��ʱ��û���ϱ�ɨ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_get_rx_info wait for %ld ms timeout!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        goto fail;
    }
    else if (i_leftime < 0)
    {
        /* ��ʱ���ڲ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_get_rx_info ait for %ld ms error!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        goto fail;
    }
    else
    {
#if defined( _PRE_WLAN_WEB_CMD_COMM) && defined(_PRE_WLAN_11K_STAT)
        /* ��������  */
        l_ret_len = OAL_SPRINTF(ac_result, sizeof(ac_result),
            "\nRX OK:%d"
            "\nRX ERROR:%d"
            "\nsuccess\n",
            pst_hmac_vap->st_atcmdsrv_get_status.ul_rx_pkct_succ_num,
            pst_hmac_vap->st_wme_stat.ul_rx_fail_num[OAL_WME_AC_BE]);
#else
        /* ��������  */
        l_ret_len = OAL_SPRINTF(ac_result, sizeof(ac_result),
            "\nRX OK:%d"
            "\nsuccess\n",
            pst_hmac_vap->st_atcmdsrv_get_status.ul_rx_pkct_succ_num);
#endif
        if (l_ret_len < 0)
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_get_rx_info format string error: %d!}", l_ret_len);
            goto fail;
        }
        else if ((oal_uint32)l_ret_len > sizeof(ac_result))
        {
            l_ret_len = sizeof(ac_result);
        }
        oal_memcopy(pc_extra, ac_result, OAL_STRLEN(ac_result));
        pst_w->length = (oal_uint16)l_ret_len;
        return OAL_SUCC;
    }

fail:
    pst_w->length = OAL_STRLEN(pst_fail_str);
    oal_memcopy(pc_extra, pst_fail_str, OAL_STRLEN(pst_fail_str));
    return -OAL_EINVAL;
}

oal_int32  wal_ioctl_get_fem_pa_status(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    oal_int32                       l_ret;
    oal_iw_point_stru               *pst_w = (oal_iw_point_stru *)p_param;
    mac_vap_stru                    *pst_mac_vap;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_int32                       i_leftime;
    wal_msg_write_stru              st_write_msg;
    oal_int8                        ac_result[100] = {0};
    oal_int32                       l_ret_len = 0;
    oal_uint16                      uc_check_hw_status;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_fem_pa_status::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,"{wal_ioctl_get_fem_pa_status::mac_res_get_hmac_vap failed!}");
        return OAL_FAIL;
    }

    pst_hmac_vap->st_atcmdsrv_get_status.uc_check_fem_pa_flag = OAL_FALSE;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CHECK_FEM_PA, 0);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_fem_pa_status::return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /*�����ȴ�dmac�ϱ�*/
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(OAL_TRUE == pst_hmac_vap->st_atcmdsrv_get_status.uc_check_fem_pa_flag), WAL_ATCMDSRB_NORM_TIME);
    if ( 0 == i_leftime)
    {
        /* ��ʱ��û���ϱ�ɨ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_get_fem_pa_status::reg wait for %ld ms timeout!}",
                         ((WAL_ATCMDSRB_NORM_TIME * 1000)/OAL_TIME_HZ));
        //OAL_IO_PRINT("%s timeout\n", __func__);
        return -OAL_EINVAL;
    }
    else if (i_leftime < 0)
    {
        /* ��ʱ���ڲ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_get_fem_pa_status::reg wait for %ld ms error!}",
                         ((WAL_ATCMDSRB_NORM_TIME * 1000)/OAL_TIME_HZ));
        //OAL_IO_PRINT("%s error\n", __func__);
        return -OAL_EINVAL;
    }
    else
    {
        uc_check_hw_status = pst_hmac_vap->st_atcmdsrv_get_status.ul_check_fem_pa_status >> 4;
        //OAL_IO_PRINT("%s uc_check_hw_status:%x\n", __func__, uc_check_hw_status);
        if(uc_check_hw_status)
        {
            l_ret_len = OAL_SPRINTF(ac_result, sizeof(ac_result),
                "\nfem check fail:%x\n", uc_check_hw_status);
            oal_memcopy(pc_extra, ac_result, OAL_STRLEN(ac_result));
            pst_w->length = (oal_uint16)l_ret_len;

            return -OAL_EFAIL;
        }

        return OAL_SUCC;
    }

}

OAL_STATIC oal_int32  wal_ioctl_pri_chip_check(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    oal_iw_point_stru               *pst_w = (oal_iw_point_stru *)p_param;
    mac_vap_stru                    *pst_mac_vap;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_int8                        ac_result[100] = {0};
    oal_int32                       l_ret_len = 0;
    oal_int8                        *pst_fail_str = "\nchip check fail";

    //OAL_IO_PRINT("%s enter\n", __func__);
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_pri_chip_check::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,"{wal_ioctl_pri_chip_check::mac_res_get_hmac_vap failed!}");
        return -OAL_FAIL;
    }

    //OAL_IO_PRINT("%s call wal_hipriv_chip_check\n", __func__);
    /* оƬ�Լ� */
    ul_ret = wal_hipriv_chip_check(pst_net_dev, "1");
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_hipriv_chip_check fail, err code[%u]!}\r\n", ul_ret);

        pst_w->length = OAL_STRLEN(pst_fail_str);
        oal_memcopy(pc_extra, pst_fail_str, OAL_STRLEN(pst_fail_str));
        return OAL_SUCC;
    }

    //OAL_IO_PRINT("%s call wal_ioctl_get_fem_pa_status\n", __func__);
    /* ��ȡfem����� */
    l_ret = wal_ioctl_get_fem_pa_status(pst_net_dev, pst_info, p_param, pc_extra);

    if(OAL_SUCC == l_ret)
    {
        //OAL_IO_PRINT("%s call wal_ioctl_get_fem_pa_status\n", __func__);
        l_ret_len = OAL_SPRINTF(ac_result, sizeof(ac_result),
            "\nsuccess\n");
        oal_memcopy(pc_extra, ac_result, OAL_STRLEN(ac_result));
        pst_w->length = (oal_uint16)l_ret_len;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_int32  wal_ioctl_get_sta_info(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    oal_iw_point_stru               *pst_w = (oal_iw_point_stru *)p_param;
    mac_vap_stru                    *pst_mac_vap;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_int32                       l_ret_len = 0;
    oal_dlist_head_stru             *pst_entry;
    oal_dlist_head_stru             *pst_dlist_tmp;
    mac_user_stru                   *pst_user_tmp;
    hmac_user_stru                  *pst_hmac_user_tmp;
    mac_cfg_query_rate_stru         *pst_query_rate_param;
    wal_msg_write_stru               st_write_msg;
    oal_int32                        l_ret;
    oal_uint8                       *puc_addr;
    oal_uint16                      us_idx = 1;
    oal_int8                        ac_result[100];

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_sta_info::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,"{wal_ioctl_get_sta_info::mac_res_get_hmac_vap failed!}");
        return -OAL_FAIL;
    }
    l_ret_len = OAL_SPRINTF(pc_extra, WLAN_IWPRIV_MAX_BUFF_LEN, "\n");
    pst_w->length = l_ret_len;

    pc_extra += l_ret_len;
    l_ret_len = OAL_SPRINTF(pc_extra, WLAN_IWPRIV_MAX_BUFF_LEN, "Total user nums: %d\n", pst_mac_vap->us_user_nums);
    pst_w->length += l_ret_len;
    pc_extra += l_ret_len;

    l_ret_len = OAL_SPRINTF(pc_extra, WLAN_IWPRIV_MAX_BUFF_LEN, "-- STA info table --\n");
    pst_w->length += l_ret_len;

    pc_extra += l_ret_len;

    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
        pst_hmac_user_tmp = mac_res_get_hmac_user_etc(pst_user_tmp->us_assoc_id);
        if (OAL_PTR_NULL == pst_hmac_user_tmp)
        {
            continue;
        }

        if (OAL_PTR_NULL == pst_dlist_tmp)
        {
            /* ��forѭ���̻߳���ͣ���ڼ����ɾ���û��¼��������pst_dlist_tmpΪ�ա�Ϊ��ʱֱ��������ȡdmac��Ϣ */
            break;
        }
#if 0
        /***********************************************************************/
        /*                  ��ȡdmac user��RSSI��Ϣ                            */
        /***********************************************************************/
        OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_RSSI, OAL_SIZEOF(mac_cfg_query_rssi_stru));
        pst_query_rssi_param = (mac_cfg_query_rssi_stru *)st_write_msg.auc_value;

        pst_query_rssi_param->us_user_id = pst_user_tmp->us_assoc_id; /* ���û���id����ȥ */

        l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_query_rssi_stru),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_sta_info_seq_show: send query rssi cfg event ret:%d", l_ret);
        }

        pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;
        /*lint -e730*/
        l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), 5*OAL_TIME_HZ);
        /*lint +e730*/
        if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_sta_info_seq_show: query rssi timeout. ret:%d", l_ret);
        }
#endif
        /***********************************************************************/
        /*                  ��ȡdmac user��������Ϣ                            */
        /***********************************************************************/
        OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(st_write_msg));
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_RATE, OAL_SIZEOF(mac_cfg_query_rate_stru));
        pst_query_rate_param = (mac_cfg_query_rate_stru *)st_write_msg.auc_value;

        pst_query_rate_param->us_user_id = pst_user_tmp->us_assoc_id; /* ���û���id����ȥ */

        l_ret = wal_send_cfg_event_etc(pst_hmac_vap->pst_net_device,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_query_rate_stru),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_sta_info: send query rate cfg event ret:%d", l_ret);
        }

        pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;
        /*lint -e730*/
        l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT((pst_hmac_vap->query_wait_q), (OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), (5*OAL_TIME_HZ));
        /*lint +e730*/
        if (l_ret <= 0) /* �ȴ���ʱ���쳣 */
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_ioctl_get_sta_info: query rate timeout. ret:%d", l_ret);
        }

        puc_addr = pst_user_tmp->auc_user_mac_addr;
        l_ret_len = OAL_SPRINTF(ac_result, 100,
                        "%2d: aid: %d\n"
                        "MAC ADDR: %02X:%02X:%02X:%02X:%02X:%02X\n"
                        "TX rate: %dkbps\n"
                        "RX rate: %dkbps\n"
                        ,us_idx,
                        pst_user_tmp->us_assoc_id,
                        puc_addr[0],puc_addr[1],puc_addr[2],puc_addr[3],puc_addr[4],puc_addr[5],
                        pst_hmac_user_tmp->ul_tx_rate,
                        pst_hmac_user_tmp->ul_rx_rate);

        pst_w->length += l_ret_len;
        if(pst_w->length > 500)
        {
            pst_w->length -= l_ret_len;
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "sta info message is to much to print");
            break;
        }
        oal_memcopy(pc_extra, ac_result, l_ret_len);
        pc_extra += l_ret_len;

        us_idx++;

    }

    return 0;
}
#endif

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST

OAL_STATIC oal_int32  wal_ioctl_set_equipment_param(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
{
    oal_iw_point_stru               *pst_param;
    oal_uint16                      us_len;
    oal_uint16                      us_subioctl_id;
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    wal_msg_write_stru              st_write_msg;
    oal_int8                        ac_cmd_str[70] = {0};     /* Ԥ���ַ����ռ� */
    //oal_int8                        ac_user_param[] = "07:06:05:04:03:02 1";
    mac_vap_stru                    *pst_mac_vap;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_int8                        ac_result[MAX_EQUIPMENT_GET_ARGS_SIZE] = {0};
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;


    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == p_param)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_equipment_param::pst_net_dev/p_param null ptr error %d!}\r\n", pst_net_dev, p_param);
        return -OAL_EFAUL;
    }
    OAL_MEMZERO((oal_uint8*)&st_write_msg, OAL_SIZEOF(st_write_msg));

    pst_param = (oal_iw_point_stru *)p_param;
    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_equipment_param::input str length is %d!}\r\n", pst_param->length);

    us_subioctl_id = pst_param->flags;

    us_len = (oal_uint16)OAL_STRLEN(pst_param->pointer);
    if ( us_len < 70 )
    {
        ul_ret = oal_copy_from_user(ac_cmd_str, pst_param->pointer, (oal_uint32)us_len);
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_equipment_param::input data length out of range!}\r\n");
        return -OAL_EFAUL;
    }

    /* copy_from_user������Ŀ���Ǵ��û��ռ俽�����ݵ��ں˿ռ䣬ʧ�ܷ���û�б��������ֽ������ɹ�����0 */
    if (ul_ret > 0)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_equipment_param::oal_copy_from_user return error code %d!}\r\n", ul_ret);
        return -OAL_EFAUL;
    }

    OAM_WARNING_LOG4(0, OAM_SF_ANY, "{wal_ioctl_set_equipment_param::the string is %c %c %c len=%d}\r\n",(oal_uint8)ac_cmd_str[0], (oal_uint8)ac_cmd_str[1], (oal_uint8)ac_cmd_str[2],us_len);

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);

    switch(us_subioctl_id)
    {
        case WLAN_CFGID_SET_RFCH:
            ul_ret = wal_hipriv_set_rfch(pst_net_dev, ac_cmd_str);
            break;
    #ifdef _PRE_DEBUG_MODE
        case WLAN_CFGID_SET_RXCH:
            ul_ret = wal_hipriv_set_rxch(pst_net_dev, ac_cmd_str);
            break;
    #endif
        case WLAN_CFGID_TX_POWER:
            ul_ret = wal_hipriv_set_txpower(pst_net_dev, ac_cmd_str);
            break;
        case WLAN_CFGID_CURRENT_CHANEL:
            ul_ret = wal_hipriv_set_freq(pst_net_dev, ac_cmd_str);
            break;
        case WLAN_CFGID_BANDWIDTH:
            ul_ret = wal_hipriv_set_bw(pst_net_dev, ac_cmd_str);
            break;
        case WLAN_CFGID_SET_RATE:
            ul_ret = wal_hipriv_set_rate_etc(pst_net_dev, ac_cmd_str);
            break;
        case WLAN_CFGID_SET_MCS:
            ul_ret = wal_hipriv_set_mcs_etc(pst_net_dev, ac_cmd_str);
            break;
        case WLAN_CFGID_SET_MCSAC:
            ul_ret = wal_hipriv_set_mcsac_etc(pst_net_dev, ac_cmd_str);
            break;
        case WLAN_CFGID_SET_NSS:
            ul_ret = wal_hipriv_set_nss(pst_net_dev, ac_cmd_str);
            break;
        case WLAN_CFGID_SEND_CW_SIGNAL:
            ul_ret = wal_hipriv_send_cw_signal(pst_net_dev, ac_cmd_str);
            break;
        case WLAN_CFGID_SET_ALWAYS_TX:
            ul_ret = wal_hipriv_always_tx(pst_net_dev, ac_cmd_str);
            break;
        case WLAN_CFGID_SET_ALWAYS_RX:
            ul_ret = wal_hipriv_always_rx(pst_net_dev, ac_cmd_str);
            break;
        case WLAN_CFGID_ADJUST_PPM:
            ul_ret = wal_hipriv_adjust_ppm(pst_net_dev, ac_cmd_str);
            break;
#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)
        case WLAN_CFGID_AUTO_CALI:
            ul_ret = wal_hipriv_auto_cali(pst_net_dev, ac_cmd_str);
            break;
#endif
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)
        case WLAN_CFGID_CALI_POWER:
            ul_ret = wal_hipriv_cali_power(pst_net_dev, ac_cmd_str);
            break;
        case WLAN_CFGID_GET_CALI_POWER:
            ul_ret = wal_ioctl_get_cali_power(pst_net_dev, pst_param, ac_cmd_str, pc_extra);
            return ul_ret;
        case WLAN_CFGID_SET_POLYNOMIAL_PARA:
            ul_ret = wal_hipriv_set_polynomial_para(pst_net_dev, ac_cmd_str);
            break;
        case WLAN_CFGID_SET_UPC_PARA:
            ul_ret = wal_hipriv_set_upc_params(pst_net_dev, ac_cmd_str);
            break;
        case WLAN_CFGID_SET_LOAD_MODE:
            ul_ret = wal_hipriv_set_load_mode(pst_net_dev, ac_cmd_str);
            break;
#endif
        default:
            /***************************************************************************
            ���¼���wal�㴦��
            ***************************************************************************/
            /* ��д��Ϣ */
            oal_memcopy((oal_uint8 *)st_write_msg.auc_value, (oal_uint8 *)ac_cmd_str, (oal_uint32)us_len);
            st_write_msg.auc_value[us_len] = '\0';
            us_len = (oal_uint16)(us_len + 1);
            WAL_WRITE_MSG_HDR_INIT(&st_write_msg, us_subioctl_id, us_len);



            /* ������Ϣ */
            l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                       (oal_uint8 *)&st_write_msg,
                                       OAL_TRUE,
                                       &pst_rsp_msg);

            if (OAL_UNLIKELY(OAL_SUCC != l_ret))
            {
                OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_equipment_param:wal_send_cfg_event_etc fail return[%d].}\r\n", l_ret);
                return l_ret;
            }

            /* ��ȡ���صĴ����� */
            ul_ret = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
            break;
    }

    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_ioctl_set_equipment_param fail, err code[%u]!}\r\n", ul_ret);
        return -((oal_int32)ul_ret);
    }


    pst_param->length = OAL_SPRINTF(ac_result, sizeof(ac_result), "%s\nsuccess",
                ac_cmd_str);
    if (pst_param->length > sizeof(ac_result))
    {
        pst_param->length = sizeof(ac_result);
    }
    oal_memcopy(pc_extra, ac_result, pst_param->length);
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_M2S

OAL_STATIC oal_uint32  wal_ioctl_set_m2s_blacklist(oal_net_device_stru *pst_net_dev, oal_uint8 *puc_buf, oal_uint8 uc_m2s_blacklist_cnt)
{
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;
    mac_m2s_ie_stru                *pst_m2s_ie;

    /* st_write_msg��������� */
    OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(wal_msg_write_stru));

    if (uc_m2s_blacklist_cnt > WLAN_M2S_BLACKLIST_MAX_NUM)
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_ioctl_set_m2s_blacklist::blacklist[%d] is beyond scope.}", uc_m2s_blacklist_cnt);
        return OAL_FAIL;
    }

    /***************************************************************************
          ���¼���wal�㴦��
      ***************************************************************************/
    pst_m2s_ie = (mac_m2s_ie_stru *)st_write_msg.auc_value;

    pst_m2s_ie->uc_blacklist_cnt = uc_m2s_blacklist_cnt;

    oal_memcopy(pst_m2s_ie->ast_m2s_blacklist, puc_buf, uc_m2s_blacklist_cnt * OAL_SIZEOF(wlan_m2s_mgr_vap_stru));

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_M2S_BLACKLIST, OAL_SIZEOF(mac_m2s_ie_stru));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_m2s_ie_stru),
                             (oal_uint8 *)&st_write_msg,
                             OAL_FALSE,
                             OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_ioctl_set_m2s_blacklist::wal_send_cfg_event_etc return err code = [%d].}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


oal_uint32  wal_ioctl_set_m2s_mss(oal_net_device_stru *pst_net_dev, oal_uint8 uc_m2s_mode)
{
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;

    /* st_write_msg��������� */
    OAL_MEMZERO(&st_write_msg, OAL_SIZEOF(wal_msg_write_stru));

    if (uc_m2s_mode > MAC_M2S_COMMAND_MODE_GET_STATE)
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_ioctl_set_m2s_mss::uc_m2s_mode[%d] is beyond scope.}", uc_m2s_mode);
        return OAL_FAIL;
    }

    /***************************************************************************
          ���¼���wal�㴦��
      ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_M2S_MSS, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = uc_m2s_mode;  /* ��������������� */

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                             (oal_uint8 *)&st_write_msg,
                             OAL_FALSE,
                             OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{wal_ioctl_set_m2s_mss::wal_send_cfg_event_etc return err code = [%d].}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifndef CONFIG_HAS_EARLYSUSPEND

OAL_STATIC oal_int32  wal_ioctl_set_suspend_mode(oal_net_device_stru *pst_net_dev, oal_uint8 uc_suspend)
{
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret = 0;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_net_dev)))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_suspend_mode::pst_net_dev null ptr error!}");
        return -OAL_EFAUL;
    }

    st_write_msg.auc_value[0] = uc_suspend;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_SUSPEND_MODE, OAL_SIZEOF(uc_suspend));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(uc_suspend),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    return l_ret;
}
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_APF

OAL_STATIC oal_uint32  wal_hipriv_apf_filter_list(oal_net_device_stru *pst_net_dev,oal_int8 *pc_param)
{
    wal_msg_write_stru           st_write_msg;
    oal_int32                    l_ret;
    mac_apf_filter_cmd_stru      st_apf_filter_cmd;

    st_apf_filter_cmd.en_cmd_type = APF_GET_FILTER_CMD;
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_APF_FILTER, OAL_SIZEOF(st_apf_filter_cmd));
    oal_memcopy(st_write_msg.auc_value, &st_apf_filter_cmd, OAL_SIZEOF(st_apf_filter_cmd));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_apf_filter_cmd),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
       OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_80m_rts_debug::return err code [%d]!}\r\n", l_ret);
       return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32  wal_hipriv_remove_app_ie(oal_net_device_stru *pst_net_dev,oal_int8 *pc_param)
{
    wal_msg_write_stru           st_write_msg;
    oal_int32                    l_ret;
    oal_uint32                   ul_ret;
    oal_uint32                   ul_off_set;
    oal_int8                     ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    const oal_uint8              uc_remove_ie_len = 2;  /* 0/1 eid*/

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_REMOVE_APP_IE, uc_remove_ie_len);

    /* ��ȡ�Ƴ������ͣ�������ֻ֧��0/1 �Ƴ����߻ָ� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_remove_app_ie::wal_get_cmd_one_arg_etc type return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }
    st_write_msg.auc_value[0] = (oal_uint8)oal_atoi(ac_name);
    /* ��ȡ������EID */
    pc_param += ul_off_set;
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_remove_app_ie::wal_get_cmd_one_arg_etc eid return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }
    st_write_msg.auc_value[1] = (oal_uint8)oal_atoi(ac_name);

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + uc_remove_ie_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_remove_app_ie::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_DELAY_STATISTIC

OAL_STATIC oal_uint32 wal_hipriv_pkt_time_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32          ul_off_set;
    oal_uint32          ul_ret;
    oal_int8            ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wal_msg_write_stru  st_write_msg;
    user_delay_switch_stru st_switch_cmd;

    /* ��ȡ�����ַ��� */
    ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pkt_time_switch::wal_get_cmd_one_arg return err_code of 1st parameter [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    if (0 == (oal_strcmp("on", ac_arg)))
    {
        st_switch_cmd.dmac_stat_enable = 1;

        /*�ڶ���������ͳ������֡������*/
        pc_param += ul_off_set;
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pkt_time_switch::wal_get_cmd_one_arg return er_code of 2nd parameter [%d]!}\r\n", ul_ret);
            return ul_ret;
        }
        st_switch_cmd.dmac_packet_count_num = (oal_uint32)oal_atoi(ac_arg);

        /*�������������ϱ����*/
        pc_param += ul_off_set;
        ul_ret = wal_get_cmd_one_arg_etc(pc_param, ac_arg, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pkt_time_switch::wal_get_cmd_one_arg return err_code of 3rd parameter [%d]!}\r\n", ul_ret);
            return ul_ret;
        }
        st_switch_cmd.dmac_report_interval = (oal_uint32)oal_atoi(ac_arg);

    }
    else if (0 == (oal_strcmp("off", ac_arg)))
    {
        st_switch_cmd.dmac_stat_enable = 0;
        st_switch_cmd.dmac_packet_count_num = 0;
        st_switch_cmd.dmac_report_interval = 0;
    }
    else
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_hipriv_pkt_time_switch::invalid parameter!}");
        return OAL_FAIL;
    }
    oal_memcopy(st_write_msg.auc_value, &st_switch_cmd,  OAL_SIZEOF(st_switch_cmd));
    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_PKT_TIME_SWITCH, OAL_SIZEOF(st_switch_cmd));
    ul_ret = (oal_uint32)wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_switch_cmd),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_pkt_time_switch::return err code %d!}\r\n", ul_ret);
    }
    return ul_ret;
}
#endif

#ifdef  _PRE_WLAN_FEATURE_FORCE_STOP_FILTER

OAL_STATIC oal_uint32  wal_ioctl_force_stop_filter(oal_net_device_stru *pst_net_dev,oal_uint8 uc_param)
{
    wal_msg_write_stru           st_write_msg;
    oal_int32                    l_ret;
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_FORCE_STOP_FILTER, OAL_SIZEOF(oal_uint8));

    st_write_msg.auc_value[0] = uc_param;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_ioctl_force_stop_filter::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_force_stop_filter::uc_param[%d].}", uc_param);
    return OAL_SUCC;
}
#endif
