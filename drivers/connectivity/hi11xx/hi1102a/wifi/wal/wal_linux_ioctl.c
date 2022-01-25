


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
#include "oal_main.h"

#include "oam_ext_if.h"
#include "frw_ext_if.h"

#include "wlan_spec.h"
#include "wlan_types.h"

#include "mac_vap.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "mac_ie.h"

#include "hmac_ext_if.h"
#include "hmac_chan_mgmt.h"

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


#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
#include "hmac_statistic_data_flow.h"
#endif

#ifdef _PRE_WLAN_TCP_OPT
#include "hmac_tcp_opt.h"
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_main.h"
#endif //_PRE_WLAN_FEATURE_ROAM
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

//#include "dmac_alg_if.h"
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#include "plat_firmware.h"
#endif
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
#include "hmac_tx_amsdu.h"
#endif
#include "hmac_statistic_data_flow.h"
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_IOCTL_C
#define MAX_PRIV_CMD_SIZE          4096
#define WAL_MAX_SPE_PORT_NUM       8       /* SPE���˿ں�Ϊ8 */
#define WAL_MAX_SPE_PKT_NUM       512     /* SPE����ķ��ͺͽ����������Լ���Ӧ�İ��ĸ��� */
#define PPS_VALUE_MIN              0     /* ���ƻ��·���pps value������ˮ�� */
#define PPS_VALUE_MAX              16000

#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
//#define CUS_NUM_OF_SAR_PARAMS           8  /* ���ƻ���SAR���� 5G_BAND1~7 2.4G */
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

OAL_STATIC oal_proc_dir_entry_stru *g_pst_proc_entry;

OAL_STATIC wal_ap_config_stru g_st_ap_config_info = {0};    /* AP������Ϣ,��Ҫ��vap �������·��� */

#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
/*Just For UT*/
OAL_STATIC oal_uint32 g_wal_wid_queue_init_flag = OAL_FALSE;
#endif
OAL_STATIC wal_msg_queue g_wal_wid_msg_queue;

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
mac_tcp_ack_buf_cfg_table_stru g_ast_hmac_tcp_ack_buf_cfg_table[] =
{
    {"enable",              MAC_TCP_ACK_BUF_ENABLE},
    {"timeout",             MAC_TCP_ACK_BUF_TIMEOUT},
    {"count",               MAC_TCP_ACK_BUF_MAX},
};
#endif

mac_ps_params_cfg_table_stru g_ast_hmac_ps_params_cfg_table[] =
{
    {"timeout",             MAC_PS_PARAMS_TIMEOUT},
    {"count",               MAC_PS_PARAMS_RESTART_COUNT},
    {"all",                 MAC_PS_PARAMS_ALL},
};


/* hi1102-cb add sys for 51/02 */
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    struct kobject     *gp_sys_kobject;
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    extern oal_uint8         g_uc_custom_cali_done;
#endif


#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_int32 wal_hipriv_inetaddr_notifier_call(struct notifier_block *this, oal_uint event, oal_void *ptr);

OAL_STATIC struct notifier_block wal_hipriv_notifier = {
    .notifier_call = wal_hipriv_inetaddr_notifier_call
};

oal_int32 wal_hipriv_inet6addr_notifier_call(struct notifier_block *this, oal_uint event, oal_void *ptr);

OAL_STATIC struct notifier_block wal_hipriv_notifier_ipv6 = {
    .notifier_call = wal_hipriv_inet6addr_notifier_call
};
#endif
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/* ÿ�����µ�������vap��ɵĶ��ƻ�ֻ����һ�Σ�wlan p2p iface�����ظ����� */
OAL_STATIC oal_uint8 g_uc_cfg_once_flag = OAL_FALSE;
/* ֻ��staut��aput��һ���ϵ�ʱ��ini�����ļ��ж�ȡ���� */
OAL_STATIC oal_uint8 g_uc_cfg_flag = OAL_FALSE;
extern host_speed_freq_level_stru g_host_speed_freq_level[4];
extern device_speed_freq_level_stru g_device_speed_freq_level[4];
extern oal_uint32 band_5g_enabled;
extern oal_uint32 hcc_assemble_count;
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION) && (_PRE_TEST_MODE == _PRE_TEST_MODE_UT)
/* UTģʽ�µ���frw_event_process_all_event */
extern oal_void  frw_event_process_all_event(oal_uint ui_data);
#endif

#ifdef _PRE_WLAN_FEATURE_DFR
extern  hmac_dfr_info_stru    g_st_dfr_info;
#endif //_PRE_WLAN_FEATURE_DFR

#ifdef _PRE_WLAN_CFGID_DEBUG
extern OAL_CONST wal_hipriv_cmd_entry_stru  g_ast_hipriv_cmd_debug[];
#endif

#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && defined(_PRE_E5_722_PLATFORM)
typedef oal_uint32 *(*hipriv_entry_t)(void *, void*, void *);
extern oal_int32 reg_at_hipriv_entry(hipriv_entry_t hipriv_entry);
#endif

/* ��̬�������� */
OAL_STATIC oal_uint32  wal_hipriv_vap_log_level(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_set_mcast_data_dscr_param(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);

OAL_STATIC oal_uint32  wal_hipriv_setcountry(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_getcountry(oal_net_device_stru *pst_net_dev,oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_set_bw(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_set_pow_rf_ctl(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
OAL_STATIC oal_uint32 wal_hipriv_always_tx_1102(oal_net_device_stru * pst_net_dev, oal_int8 * pc_param);
OAL_STATIC oal_uint32  wal_hipriv_always_tx_num(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif

OAL_STATIC oal_uint32  wal_hipriv_always_rx(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_pcie_pm_level(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_user_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_add_vap(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param);

#ifdef _PRE_DEBUG_MODE
#ifdef _PRE_WLAN_DFT_EVENT
OAL_STATIC oal_void  wal_event_report_to_sdt(wal_msg_type_enum_uint8   en_msg_type, oal_uint8 *puc_param, wal_msg_stru *pst_cfg_msg);
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
OAL_STATIC oal_uint32 wal_hipriv_btcoex_status_print(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_LTECOEX
OAL_STATIC oal_uint32 wal_ioctl_ltecoex_mode_set(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
OAL_STATIC oal_uint32  wal_hipriv_aifsn_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_cw_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif

OAL_STATIC oal_uint32  wal_hipriv_set_random_mac_addr_scan(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_add_user(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_del_user(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);

OAL_STATIC oal_uint32  wal_hipriv_reg_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
OAL_STATIC oal_uint32  wal_hipriv_sdio_flowctrl(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
OAL_STATIC oal_uint32  wal_hipriv_set_2040_coext_support(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_rx_fcs_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
oal_int32  wal_netdev_open(oal_net_device_stru *pst_net_dev);
OAL_STATIC oal_int32  wal_net_device_ioctl(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, oal_int32 ul_cmd);
#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) && (!defined(_PRE_PRODUCT_ID_HI110X_HOST)))
OAL_STATIC oal_int32  wal_ioctl_get_assoc_req_ie(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_set_auth_mode(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_set_country_code(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32 wal_ioctl_set_max_user(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_nl80211_priv_connect(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_nl80211_priv_disconnect(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
#ifdef _PRE_WLAN_FEATURE_HILINK
OAL_STATIC oal_int32  wal_ioctl_nl80211_priv_fbt_kick_user(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32  wal_ioctl_set_okc_ie(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32 wal_net_dev_ioctl_get_all_sta_info(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC oal_int32 wal_ioctl_start_fbt_scan(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
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
#ifdef _PRE_BT_FITTING_DATA_COLLECT
OAL_STATIC oal_uint32  wal_hipriv_init_bt_env(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_set_bt_freq(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_set_bt_upc_by_freq(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_print_bt_gm(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
OAL_STATIC oal_uint32  wal_hipriv_set_tx_ba_policy(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_int32  wal_ioctl_set_mode(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, void *p_param, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_set_freq(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iw_freq_stru *pst_freq, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_set_txpower(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iw_param_stru *pst_param, oal_int8 *pc_extra);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
OAL_STATIC oal_uint32  wal_ioctl_get_mode(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_ioctl_get_essid(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_ioctl_get_bss_type(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_ioctl_set_bss_type(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_ioctl_get_freq(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_ioctl_get_txpower(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#else
OAL_STATIC oal_int32  wal_ioctl_get_mode(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, void *p_param, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_essid(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iw_point_stru *pst_data, oal_int8 *pc_ssid);
OAL_STATIC oal_int32  wal_ioctl_set_essid(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iw_point_stru *pst_data, oal_int8 *pc_ssid);
OAL_STATIC oal_int32  wal_ioctl_get_bss_type(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_uint32 *pul_type, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_set_bss_type(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_uint32 *pul_type, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_freq(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iw_freq_stru *pst_freq, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_txpower(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iw_param_stru *pst_param, oal_int8 *pc_extra);
#endif

/* E5 SPE module relation */
#if (defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT))
OAL_STATIC oal_int32 wal_netdev_spe_init(oal_net_device_stru *pst_net_dev);
OAL_STATIC oal_void wal_netdev_spe_exit(oal_net_device_stru *pst_net_dev);
OAL_STATIC oal_int32 wal_finish_spe_rd(oal_int32 l_port_num, oal_int32 l_src_port_num, oal_netbuf_stru *pst_buf, oal_uint32 ul_dma, oal_uint32 ul_flags);
OAL_STATIC oal_int32 wal_finish_spe_td(oal_int32 l_port_num, oal_netbuf_stru *pst_buf, oal_uint32 ul_flags);
#endif /* defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT) */

OAL_STATIC oal_int32  wal_ioctl_get_apaddr(
                oal_net_device_stru         *pst_net_dev,
                oal_iw_request_info_stru    *pst_info,
                oal_sockaddr_stru           *pst_addr,
                oal_int8                    *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_iwrate(
                oal_net_device_stru         *pst_net_dev,
                oal_iw_request_info_stru    *pst_info,
                oal_iw_param_stru           *pst_param,
                oal_int8                    *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_iwsense(
                oal_net_device_stru         *pst_net_dev,
                oal_iw_request_info_stru    *pst_info,
                oal_iw_param_stru           *pst_param,
                oal_int8                    *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_rtsthres(
                oal_net_device_stru         *pst_net_dev,
                oal_iw_request_info_stru    *pst_info,
                oal_iw_param_stru           *pst_param,
                oal_int8                    *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_fragthres(
                oal_net_device_stru         *pst_net_dev,
                oal_iw_request_info_stru    *pst_info,
                oal_iw_param_stru           *pst_param,
                oal_int8                    *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_iwencode(
                oal_net_device_stru         *pst_net_dev,
                oal_iw_request_info_stru    *pst_info,
                oal_iw_point_stru           *pst_param,
                oal_int8                    *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_iwrange(
                oal_net_device_stru         *pst_net_dev,
                oal_iw_request_info_stru    *pst_info,
                oal_iw_point_stru           *pst_param,
                oal_int8                    *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_param(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_iw, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_set_param(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_iw, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_iwname(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_int8* pc_name, oal_int8* pc_extra);
OAL_STATIC oal_int32  wal_ioctl_set_wme_params(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_iw, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_wme_params(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_iw, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_setcountry(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_w, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_getcountry(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_w, oal_int8 *pc_extra);
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
#ifdef _PRE_WLAN_FEATURE_STA_PM
OAL_STATIC oal_int32 wal_ioctl_set_sta_pm(oal_net_device_stru *pst_net_dev, mac_pm_switch_enum_uint8 en_pm_enable, mac_pm_ctrl_type_enum_uint8 en_pm_ctrl_type);
#endif
OAL_STATIC oal_uint32  wal_hipriv_bgscan_enable(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param);
#ifdef _PRE_WLAN_FEATURE_STA_PM
OAL_STATIC oal_uint32  wal_hipriv_sta_ps_mode(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param);
#ifdef _PRE_PSM_DEBUG_MODE
OAL_STATIC oal_uint32  wal_hipriv_sta_ps_info(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param);
#endif
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
OAL_STATIC OAL_DEVICE_ATTR(hipriv, OAL_S_IRUGO|OAL_S_IWUSR, wal_hipriv_sys_read, wal_hipriv_sys_write);
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
OAL_STATIC oal_int32  wal_ioctl_set_p2p_noa(oal_net_device_stru * pst_net_dev, mac_cfg_p2p_noa_param_stru * pst_p2p_noa_param);
OAL_STATIC oal_int32 wal_ioctl_reduce_sar(oal_net_device_stru *pst_net_dev, oal_uint16 us_tx_power);
OAL_STATIC oal_int32  wal_ioctl_set_p2p_ops(oal_net_device_stru * pst_net_dev, mac_cfg_p2p_ops_param_stru * pst_p2p_ops_param);

#endif  /* _PRE_WLAN_FEATURE_P2P */

#ifdef _PRE_WLAN_FEATURE_VOWIFI
OAL_STATIC oal_int32 wal_ioctl_set_vowifi_param(oal_net_device_stru *pst_net_dev, oal_int8* puc_command);
OAL_STATIC oal_int32 wal_ioctl_get_vowifi_param(oal_net_device_stru *pst_net_dev, oal_int8 *puc_command, oal_int32 *pl_value);
#endif

#ifdef _PRE_WLAN_FEATURE_WAPI
//OAL_STATIC oal_uint32  wal_hipriv_set_wapi(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_HS20
OAL_STATIC oal_int32 wal_ioctl_set_qos_map(oal_net_device_stru *pst_net_dev,
                                                   hmac_cfg_qos_map_param_stru *pst_qos_map_param);
#endif/* #ifdef _PRE_WLAN_FEATURE_HS20 */

oal_int32 wal_ioctl_set_wps_p2p_ie(oal_net_device_stru  *pst_net_dev,
                                    oal_uint8           *puc_buf,
                                    oal_uint32           ul_len,
                                    en_app_ie_type_uint8 en_type);

OAL_STATIC oal_int32 wal_set_ap_max_user(oal_net_device_stru *pst_net_dev, oal_uint32 ul_ap_max_user);
OAL_STATIC oal_int32 wal_config_mac_filter(oal_net_device_stru *pst_net_dev, oal_int8 *pc_command);
OAL_STATIC oal_int32 wal_kick_sta(oal_net_device_stru *pst_net_dev, oal_uint8 *auc_mac_addr);
OAL_STATIC oal_int32  wal_ioctl_set_ap_config(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_get_assoc_list(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_set_mac_filters(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, oal_int8 *pc_extra);
OAL_STATIC oal_int32  wal_ioctl_set_ap_sta_disassoc(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, oal_int8 *pc_extra);
OAL_STATIC oal_uint32  wal_get_parameter_from_cmd(oal_int8 *pc_cmd, oal_int8 *pc_arg, OAL_CONST oal_int8 *puc_token, oal_uint32 *pul_cmd_offset, oal_uint32 ul_param_max_len);

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
oal_int32 wal_init_wlan_vap(oal_net_device_stru *pst_net_dev);
oal_int32 wal_deinit_wlan_vap(oal_net_device_stru *pst_net_dev);
oal_int32 wal_start_vap(oal_net_device_stru *pst_net_dev);
oal_int32  wal_stop_vap(oal_net_device_stru *pst_net_dev);
OAL_STATIC oal_int32 wal_set_mac_addr(oal_net_device_stru *pst_net_dev);
oal_int32 wal_init_wlan_netdev(oal_wiphy_stru *pst_wiphy, char *dev_name);
oal_int32  wal_setup_ap(oal_net_device_stru *pst_net_dev);
#endif


#ifdef _PRE_WLAN_FEATURE_GREEN_AP
OAL_STATIC oal_uint32  wal_hipriv_green_ap_en(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_11K
OAL_STATIC oal_uint32  wal_hipriv_send_neighbor_req(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_beacon_req_table_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
OAL_STATIC oal_uint32  wal_hipriv_voe_enable(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#ifdef _PRE_WLAN_FEATURE_APF
OAL_STATIC oal_uint32  wal_hipriv_apf_filter_list(oal_net_device_stru *pst_net_dev,oal_int8 *pc_param);
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
OAL_STATIC oal_uint32 wal_hipriv_set_pm_debug_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_int32 wal_ioctl_get_wifi_priv_feature_cap_param(oal_net_device_stru *pst_net_dev, oal_int8 *puc_command, oal_int32 *pl_value);
#ifndef CONFIG_HAS_EARLYSUSPEND
OAL_STATIC oal_int32  wal_ioctl_set_suspend_mode(oal_net_device_stru *pst_net_dev, oal_uint8 uc_suspend);
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
OAL_STATIC oal_uint32  wal_hipriv_tcp_ack_buf_cfg(oal_net_device_stru * pst_net_dev, oal_int8 * pc_param);
#endif
OAL_STATIC oal_uint32  wal_hipriv_ps_params_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#ifdef _PRE_WLAN_RF_CALI
OAL_STATIC oal_uint32  wal_hipriv_auto_cali(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_set_cali_vref(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
OAL_STATIC oal_uint32  wal_hipriv_mcs_set_check_enable(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_hipriv_set_sar_level(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
OAL_STATIC oal_uint32  wal_ioctl_force_pass_filter(oal_net_device_stru *pst_net_dev, oal_uint8 uc_param);
#ifdef _PRE_WLAN_FEATURE_BTCOEX
OAL_STATIC oal_uint32  wal_hipriv_set_btcoex_params(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);
#endif
OAL_STATIC oal_uint32  wal_hipriv_set_tx_pow_param(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param);

oal_int32 wal_set_nbfh(oal_net_device_stru *pst_net_dev);

/*****************************************************************************
  ˽���������. ˽�������ʽ:
         �豸�� ������ ����
  hipriv "Hisilicon0 create vap0"
*****************************************************************************/
/*Android private command strings*/
#define CMD_SET_AP_WPS_P2P_IE   "SET_AP_WPS_P2P_IE"
#define CMD_P2P_SET_NOA         "P2P_SET_NOA"
#define CMD_P2P_SET_PS          "P2P_SET_PS"
#define CMD_COUNTRY             "COUNTRY"
#ifdef _PRE_WLAN_FEATURE_LTECOEX
#define CMD_LTECOEX_MODE        "LTECOEX_MODE"
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
#define CMD_SET_STA_PM_ON       "SET_STA_PM_ON"
#endif
#define CMD_SET_QOS_MAP         "SET_QOS_MAP"
#define CMD_TX_POWER            "TX_POWER"
#define CMD_WPAS_GET_CUST       "WPAS_GET_CUST"
#define CMD_SET_HITALK_STR      "SET_HITALK"

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
#define CMD_VOWIFI_IS_SUPPORT           "VOWIFI_DETECT VOWIFi_IS_SUPPORT"
#define CMD_GET_WIFI_PRIV_FEATURE_CAPABILITY   "GET_WIFI_PRIV_FEATURE_CAPABILITY"

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
#define CMD_SET_RX_FILTER_ENABLE    "set_rx_filter_enable"
#define CMD_ADD_RX_FILTER_ITEMS     "add_rx_filter_items"
#define CMD_CLEAR_RX_FILTERS        "clear_rx_filters"
#define CMD_GET_RX_FILTER_PKT_STATE "get_rx_filter_pkt_state"

#define CMD_FILTER_SWITCH    "FILTER"
#endif /* _PRE_WLAN_FEATURE_IP_FILTER */

#define CMD_SETSUSPENDOPT               "SETSUSPENDOPT"
#define CMD_SETSUSPENDMODE              "SETSUSPENDMODE"

#define CMD_RXFILTER_START              "RXFILTER-START"
#define CMD_RXFILTER_STOP               "RXFILTER-STOP"

OAL_STATIC OAL_CONST wal_hipriv_cmd_entry_stru  g_ast_hipriv_cmd[] =
{
    /************************���ö��ⷢ����˽������*******************/
    {"info",                    wal_hipriv_vap_info},               /* ��ӡvap�����в�����Ϣ: hipriv "vap0 info" */
    {"setcountry",              wal_hipriv_setcountry},            /*���ù��������� hipriv "Hisilicon0 setcountry param"paramȡֵΪ��д�Ĺ������֣����� CN US*/
    {"getcountry",              wal_hipriv_getcountry},            /*��ѯ���������� hipriv "Hisilicon0 getcountry"*/
#ifdef _PRE_WLAN_FEATURE_GREEN_AP
    {"green_ap_en",             wal_hipriv_green_ap_en},              /* green AP����: hipriv "wlan0 green_ap_en 0 | 1" */
#endif
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    {"ip_filter", wal_hipriv_set_ip_filter},                       /* ip filter(���ܵ��Խӿ�)hipriv "wlan0 ip_filter cmd param0 param1 ...."
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
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    {"coex_print",              wal_hipriv_btcoex_status_print},    /* ��ӡ����ά����Ϣ��sh hipriv.sh "coex_print" */
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM
    {"set_ps_mode",             wal_hipriv_sta_ps_mode},           /* ����PSPOLL���� sh hipriv.sh 'wlan0 set_ps_mode 3 0'*/
#ifdef _PRE_PSM_DEBUG_MODE
    {"psm_info_debug",          wal_hipriv_sta_ps_info},            /* sta psm��ά��ͳ����Ϣ sh hipriv.sh 'wlan0 psm_info_debug 1' */
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_UAPSD
    {"uapsd_en_cap",           wal_hipriv_set_uapsd_cap},        /* hipriv "vap0 uapsd_en_cap 0\1" */
#endif
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    {"set_uapsd_para",          wal_hipriv_set_uapsd_para},        /* ����uapsd�Ĳ�����Ϣ sh hipriv.sh 'wlan0 set_uapsd_para 3 1 1 1 1 */
#endif
    {"create",                  wal_hipriv_add_vap},                /* ����vap˽������Ϊ: hipriv "Hisilicon0 create vap0 ap|sta" */
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    {"al_tx_1102",              wal_hipriv_always_tx_1102},          /* ���ó���ģʽ:               hipriv "vap0 al_tx <value: 0/1/2>  <len>" ����mac���ƣ�11a,b,g��ֻ֧��4095�������ݷ���,����ʹ��set_mcast_data�����ʽ�������*/
    {"al_tx_num",               wal_hipriv_always_tx_num},           /* ���ó�����Ŀ:               hipriv "vap0 al_tx_num <value>" */
#endif
    {"al_rx",                   wal_hipriv_always_rx},               /* ���ó���ģʽ:               hipriv "vap0 al_rx <value: 0/1/2>" */
    {"rate",                    wal_hipriv_set_rate },               /* ����non-HTģʽ�µ�����:     hipriv "vap0 rate  <value>" */
    {"mcs",                     wal_hipriv_set_mcs  },               /* ����HTģʽ�µ�����:         hipriv "vap0 mcs   <value>" */
    {"mcsac",                   wal_hipriv_set_mcsac},               /* ����VHTģʽ�µ�����:        hipriv "vap0 mcsac <value>" */
    {"freq",                    wal_hipriv_set_freq},                    /* ����AP �ŵ� */
    {"mode",                    wal_hipriv_set_mode},                    /* ����AP Э��ģʽ */
    {"bw",                      wal_hipriv_set_bw   },               /* ���ô���:                   hipriv "vap0 bw    <value>" */
    {"set_mcast_data",          wal_hipriv_set_mcast_data_dscr_param},    /* ��ӡ��������Ϣ: hipriv "vap0 set_mcast_data <param name> <value>" */
    {"rx_fcs_info",             wal_hipriv_rx_fcs_info},            /* ��ӡ����֡��FCS��ȷ�������Ϣ:hipriv "vap0 rx_fcs_info 0/1" 0/1  0���������1������� */
    {"set_regdomain_pwr",       wal_hipriv_set_regdomain_pwr},      /* ���ù���������͹��ʣ�hipriv "Hisilicon0 set_regdomain_pwr 20",��λdBm */
    {"add_user",                wal_hipriv_add_user},               /* ��������û�����������: hipriv "vap0 add_user xx xx xx xx xx xx(mac��ַ) 0 | 1(HT����λ) "  ���������ĳһ��VAP */
    {"del_user",                wal_hipriv_del_user},               /* ����ɾ���û�����������: hipriv "vap0 del_user xx xx xx xx xx xx(mac��ַ)" ���������ĳһ��VAP */
    {"alg_cfg",                 wal_hipriv_alg_cfg},                /* �㷨��������: hipriv "vap0 alg_cfg sch_vi_limit 10"*/
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    {"set_edca_switch_sta",        wal_hipriv_set_edca_opt_switch_sta},       /* STA�Ƿ���˽��edca�����Ż����� */
#endif
    {"alg_tpc_log",             wal_hipriv_tpc_log},                /* tpc�㷨��־��������:*/
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
    {"sdio_flowctrl",           wal_hipriv_sdio_flowctrl},
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
    {"radartool",               wal_hipriv_dfs_radartool},
#endif
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    {"tcp_ack_buf",             wal_hipriv_tcp_ack_buf_cfg},        /* tcp ack buf */
#endif
    {"ps_params",               wal_hipriv_ps_params_cfg},          /* ps_params host�·��͹��Ĳ��� */
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
#ifdef _PRE_WLAN_FEATURE_APF
    {"apf_filter_list",         wal_hipriv_apf_filter_list},
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    {"pm_debug",                wal_hipriv_set_pm_debug_switch},    /* �͹���debug���� */
#endif
#ifdef _PRE_WLAN_RF_CALI
    {"auto_cali",        wal_hipriv_auto_cali},      /* ����У׼�Զ���:hipriv "Hisilicon0 auto_cali" */
    {"cali_vref",        wal_hipriv_set_cali_vref},  /* У׼�����޸�:hipriv "wlan0 cali_vref value" */
#endif
    {"mcs_check_enable",         wal_hipriv_mcs_set_check_enable},
    {"set_sar_level",            wal_hipriv_set_sar_level},
    {"set_rf_ctl_enable",        wal_hipriv_set_pow_rf_ctl},
    {"set_tx_ba_policy",         wal_hipriv_set_tx_ba_policy},
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    {"set_btcoex_params",        wal_hipriv_set_btcoex_params},
#endif
    {"set_tx_pow",               wal_hipriv_set_tx_pow_param},  /* ���÷��͹��ʲ���: hipriv "vap0 set_tx_pow <param name> <value>" */

#ifdef _PRE_BT_FITTING_DATA_COLLECT
    {"bt_init_env",              wal_hipriv_init_bt_env},
    {"bt_freq",                  wal_hipriv_set_bt_freq},
    {"bt_upc_by_freq",           wal_hipriv_set_bt_upc_by_freq},
    {"bt_print_gm",              wal_hipriv_print_bt_gm},
#endif
};

/*****************************************************************************
  net_device�Ϲҽӵ�net_device_ops����
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_net_device_ops_stru g_st_wal_net_dev_ops =
{
    .ndo_get_stats          = wal_netdev_get_stats,
    .ndo_open               = wal_netdev_open,
    .ndo_stop               = wal_netdev_stop,
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    .ndo_start_xmit         = wal_vap_start_xmit,
#else
    .ndo_start_xmit         = wal_bridge_vap_xmit,
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
    .ndo_select_queue       = wal_netdev_select_queue,
#endif

    .ndo_set_mac_address    = wal_netdev_set_mac_addr
};
#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
oal_net_device_ops_stru g_st_wal_net_dev_ops =
{
    oal_net_device_init,
    wal_netdev_open,
    wal_netdev_stop,
    wal_bridge_vap_xmit,
    OAL_PTR_NULL,
    wal_netdev_get_stats,
    wal_net_device_ioctl,
    oal_net_device_change_mtu,
    wal_netdev_set_mac_addr
};
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_ethtool_ops_stru g_st_wal_ethtool_ops = { 0 };
#endif

/*****************************************************************************
  ��׼ioctl�������.
*****************************************************************************/
OAL_STATIC OAL_CONST oal_iw_handler g_ast_iw_handlers[] =
{
    OAL_PTR_NULL,                               /* SIOCSIWCOMMIT, */
    (oal_iw_handler)wal_ioctl_get_iwname,       /* SIOCGIWNAME, */
    OAL_PTR_NULL,                               /* SIOCSIWNWID, */
    OAL_PTR_NULL,                               /* SIOCGIWNWID, */
    (oal_iw_handler)wal_ioctl_set_freq,         /* SIOCSIWFREQ, ����Ƶ��/�ŵ� */
    (oal_iw_handler)wal_ioctl_get_freq,         /* SIOCGIWFREQ, ��ȡƵ��/�ŵ� */
    (oal_iw_handler)wal_ioctl_set_bss_type,     /* SIOCSIWMODE, ����bss type */
    (oal_iw_handler)wal_ioctl_get_bss_type,     /* SIOCGIWMODE, ��ȡbss type */
    OAL_PTR_NULL,                               /* SIOCSIWSENS, */
    (oal_iw_handler)wal_ioctl_get_iwsense,      /* SIOCGIWSENS, */
    OAL_PTR_NULL,                               /* SIOCSIWRANGE, */ /* not used */
    (oal_iw_handler)wal_ioctl_get_iwrange,      /* SIOCGIWRANGE, */
    OAL_PTR_NULL,                               /* SIOCSIWPRIV, */  /* not used */
    OAL_PTR_NULL,                               /* SIOCGIWPRIV, */  /* kernel code */
    OAL_PTR_NULL,                               /* SIOCSIWSTATS, */ /* not used */
    OAL_PTR_NULL,                               /* SIOCGIWSTATS, */
    OAL_PTR_NULL,                               /* SIOCSIWSPY, */
    OAL_PTR_NULL,                               /* SIOCGIWSPY, */
    OAL_PTR_NULL,                               /* -- hole -- */
    OAL_PTR_NULL,                               /* -- hole -- */
    OAL_PTR_NULL,                               /* SIOCSIWAP, */
    (oal_iw_handler)wal_ioctl_get_apaddr,       /* SIOCGIWAP, */
    OAL_PTR_NULL,                               /* SIOCSIWMLME, */
    OAL_PTR_NULL,                               /* SIOCGIWAPLIST, */
    OAL_PTR_NULL,                               /* SIOCSIWSCAN, */
    OAL_PTR_NULL,                               /* SIOCGIWSCAN, */
    (oal_iw_handler)wal_ioctl_set_essid,        /* SIOCSIWESSID, ����ssid */
    (oal_iw_handler)wal_ioctl_get_essid,        /* SIOCGIWESSID, ��ȡssid */
    OAL_PTR_NULL,                               /* SIOCSIWNICKN */
    OAL_PTR_NULL,                               /* SIOCGIWNICKN */
    OAL_PTR_NULL,                               /* -- hole -- */
    OAL_PTR_NULL,                               /* -- hole -- */
    OAL_PTR_NULL,                               /* SIOCSIWRATE */
    (oal_iw_handler)wal_ioctl_get_iwrate,       /* SIOCGIWRATE */
    OAL_PTR_NULL,                               /* SIOCSIWRTS */
    (oal_iw_handler)wal_ioctl_get_rtsthres,     /* SIOCGIWRTS */
    OAL_PTR_NULL,                               /* SIOCSIWFRAG */
    (oal_iw_handler)wal_ioctl_get_fragthres,    /* SIOCGIWFRAG */
    (oal_iw_handler)wal_ioctl_set_txpower,      /* SIOCSIWTXPOW, ���ô��书������ */
    (oal_iw_handler)wal_ioctl_get_txpower,      /* SIOCGIWTXPOW, ���ô��书������ */
    OAL_PTR_NULL,                               /* SIOCSIWRETRY */
    OAL_PTR_NULL,                               /* SIOCGIWRETRY */
    OAL_PTR_NULL,                               /* SIOCSIWENCODE */
    (oal_iw_handler)wal_ioctl_get_iwencode,     /* SIOCGIWENCODE */
    OAL_PTR_NULL,                               /* SIOCSIWPOWER */
    OAL_PTR_NULL,                               /* SIOCGIWPOWER */
    OAL_PTR_NULL,                               /* -- hole -- */
    OAL_PTR_NULL,                               /* -- hole -- */
    OAL_PTR_NULL,                               /* SIOCSIWGENIE */
    OAL_PTR_NULL,                               /* SIOCGIWGENIE */
    OAL_PTR_NULL,                               /* SIOCSIWAUTH */
    OAL_PTR_NULL,                               /* SIOCGIWAUTH */
    OAL_PTR_NULL,                               /* SIOCSIWENCODEEXT */
    OAL_PTR_NULL                                /* SIOCGIWENCODEEXT */
};

/*****************************************************************************
  ˽��ioctl����������嶨��
*****************************************************************************/
OAL_STATIC OAL_CONST oal_iw_priv_args_stru g_ast_iw_priv_args[] =
{
    {WAL_IOCTL_PRIV_SET_MODE,       OAL_IW_PRIV_TYPE_CHAR | 24, 0, "mode"},     /* ����������char, ����Ϊ24 */
    {WAL_IOCTL_PRIV_GET_MODE,       0, OAL_IW_PRIV_TYPE_CHAR | 24, "get_mode"},
    {WAL_IOCTL_PRIV_SET_COUNTRY,    OAL_IW_PRIV_TYPE_CHAR | 3, 0,  "setcountry"},
    {WAL_IOCTL_PRIV_GET_COUNTRY,    0, OAL_IW_PRIV_TYPE_CHAR | 3,  "getcountry"},

    {WAL_IOCTL_PRIV_SET_AP_CFG, OAL_IW_PRIV_TYPE_CHAR |  256, 0,"AP_SET_CFG"},
    {WAL_IOCTL_PRIV_AP_MAC_FLTR, OAL_IW_PRIV_TYPE_CHAR | 256, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_SIZE_FIXED | 0, "AP_SET_MAC_FLTR"},
    {WAL_IOCTL_PRIV_AP_GET_STA_LIST, 0, OAL_IW_PRIV_TYPE_CHAR | 1024, "AP_GET_STA_LIST"},
    {WAL_IOCTL_PRIV_AP_STA_DISASSOC, OAL_IW_PRIV_TYPE_CHAR | 256, OAL_IW_PRIV_TYPE_CHAR | 0, "AP_STA_DISASSOC"},

    /* sub-ioctl������� */
    {WAL_IOCTL_PRIV_SETPARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "setparam"},
    {WAL_IOCTL_PRIV_GETPARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
                                    OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "getparam"},

    /* sub-ioctl��־��nameΪ'\0', ����1��ʾset��������1������, get����õ�1��ֵ */
    {WAL_IOCTL_PRIV_SETPARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, ""},
    {WAL_IOCTL_PRIV_SETPARAM,       OAL_IW_PRIV_TYPE_BYTE | OAL_IW_PRIV_SIZE_FIXED | OAL_IW_PRIV_TYPE_ADDR, 0, ""},
    {WAL_IOCTL_PRIV_GETPARAM,       0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "" },
    {WAL_IOCTL_PRIV_GETPARAM,       0, OAL_IW_PRIV_TYPE_BYTE | OAL_IW_PRIV_SIZE_FIXED | OAL_IW_PRIV_TYPE_ADDR, ""},
    {WLAN_CFGID_SHORTGI,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "shortgi20"},
    {WLAN_CFGID_SHORTGI,            0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_shortgi20"},
    {WLAN_CFGID_SHORTGI_FORTY,      OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "shortgi40"},
    {WLAN_CFGID_SHORTGI_FORTY,      0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_shortgi40"},
    {WLAN_CFGID_SHORTGI_EIGHTY,     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "shortgi80"},
    {WLAN_CFGID_SHORTGI_EIGHTY,     0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_shortgi80"},

    {WLAN_CFGID_SHORT_PREAMBLE,     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "shpreamble"},
    {WLAN_CFGID_SHORT_PREAMBLE,     0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_shpreamble"},
#ifdef _PRE_WLAN_FEATURE_MONITOR
    {WLAN_CFGID_ADDR_FILTER,        OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "addr_filter"},
    {WLAN_CFGID_ADDR_FILTER,        0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_addr_filter"},
#endif
    {WLAN_CFGID_PROT_MODE,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "protmode"},
    {WLAN_CFGID_PROT_MODE,          0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_protmode"},
    {WLAN_CFGID_AUTH_MODE,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "authmode"},
    {WLAN_CFGID_AUTH_MODE,          0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_authmode"},
    {WLAN_CFGID_BEACON_INTERVAL,    OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "bintval"},
    {WLAN_CFGID_BEACON_INTERVAL,    0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_bintval"},
    {WLAN_CFGID_NO_BEACON,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "nobeacon"},
    {WLAN_CFGID_NO_BEACON,          0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_nobeacon"},
    {WLAN_CFGID_TX_CHAIN,           OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "txchainmask"},
    {WLAN_CFGID_TX_CHAIN,           0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_txchainmask"},
    {WLAN_CFGID_RX_CHAIN,           OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "rxchainmask"},
    {WLAN_CFGID_RX_CHAIN,           0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_rxchainmask"},
    {WLAN_CFGID_CONCURRENT,         OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "concurrent"},
    {WLAN_CFGID_CONCURRENT,         0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_concurrent"},
    {WLAN_CFGID_TID,                0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_tid"},

#ifdef _PRE_WLAN_FEATURE_UAPSD
    /*U-APSD����*/
    {WLAN_CFGID_UAPSD_EN ,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "uapsden"},
    {WLAN_CFGID_UAPSD_EN,           0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_uapsden"},
#endif
    {WLAN_CFGID_DTIM_PERIOD,        OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "dtim_period"},
    {WLAN_CFGID_DTIM_PERIOD,        0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_dtim_period"},

    /* EDCA������������ sub-ioctl��� */
    {WAL_IOCTL_PRIV_SET_WMM_PARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 3, 0, "setwmmparam"},
    {WAL_IOCTL_PRIV_GET_WMM_PARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2,
                                         OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, "getwmmparam"},

    /* sub-ioctl��־��nameΪ'\0', 2��ʾset�������������� */
    {WAL_IOCTL_PRIV_SET_WMM_PARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, ""},
    {WAL_IOCTL_PRIV_GET_WMM_PARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
                                         OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "" },
    {WLAN_CFGID_EDCA_TABLE_CWMIN,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "cwmin"},
    {WLAN_CFGID_EDCA_TABLE_CWMIN,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_cwmin"},
    {WLAN_CFGID_EDCA_TABLE_CWMAX,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "cwmax"},
    {WLAN_CFGID_EDCA_TABLE_CWMAX,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_cwmax"},
    {WLAN_CFGID_EDCA_TABLE_AIFSN,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "aifsn"},
    {WLAN_CFGID_EDCA_TABLE_AIFSN,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_aifsn"},
    {WLAN_CFGID_EDCA_TABLE_TXOP_LIMIT,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "txoplimit"},
    {WLAN_CFGID_EDCA_TABLE_TXOP_LIMIT,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_txoplimit"},
    {WLAN_CFGID_EDCA_TABLE_MSDU_LIFETIME,    OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "lifetime"},
    {WLAN_CFGID_EDCA_TABLE_MSDU_LIFETIME,    OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_lifetime"},
    {WLAN_CFGID_EDCA_TABLE_MANDATORY,        OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "mandatory"},
    {WLAN_CFGID_EDCA_TABLE_MANDATORY,        OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_mandatory"},

    {WLAN_CFGID_QEDCA_TABLE_CWMIN,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "qcwmin"},
    {WLAN_CFGID_QEDCA_TABLE_CWMIN,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_qcwmin"},
    {WLAN_CFGID_QEDCA_TABLE_CWMAX,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "qcwmax"},
    {WLAN_CFGID_QEDCA_TABLE_CWMAX,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_qcwmax"},
    {WLAN_CFGID_QEDCA_TABLE_AIFSN,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "qaifsn"},
    {WLAN_CFGID_QEDCA_TABLE_AIFSN,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_qaifsn"},
    {WLAN_CFGID_QEDCA_TABLE_TXOP_LIMIT,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "qtxoplimit"},
    {WLAN_CFGID_QEDCA_TABLE_TXOP_LIMIT,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_qtxoplimit"},
    {WLAN_CFGID_QEDCA_TABLE_MSDU_LIFETIME,    OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "qlifetime"},
    {WLAN_CFGID_QEDCA_TABLE_MSDU_LIFETIME,    OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_qlifetime"},
    {WLAN_CFGID_QEDCA_TABLE_MANDATORY,        OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "qmandatory"},
    {WLAN_CFGID_QEDCA_TABLE_MANDATORY,        OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_qmandatory"},
#ifdef _PRE_WLAN_FEATURE_SMPS
    {WLAN_CFGID_SMPS_MODE,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "smps_mode"},
    {WLAN_CFGID_SMPS_MODE,          0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_smps_mode"},
    {WLAN_CFGID_SMPS_EN,            0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_smps_en"},
#endif

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
    {WLAN_CFGID_PROXYARP_EN,   OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "proxyarp_en"}, /* ʹ��proxy arp */
#endif

};

/*****************************************************************************
  ˽��ioctl�������.
*****************************************************************************/
OAL_STATIC OAL_CONST oal_iw_handler g_ast_iw_priv_handlers[] =
{
    (oal_iw_handler)wal_ioctl_set_param,                /* SIOCWFIRSTPRIV+0 */  /* sub-ioctl set ��� */
    (oal_iw_handler)wal_ioctl_get_param,                /* SIOCWFIRSTPRIV+1 */  /* sub-ioctl get ��� */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+2 */  /* setkey */
    (oal_iw_handler)wal_ioctl_set_wme_params,           /* SIOCWFIRSTPRIV+3 */  /* setwmmparams */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+4 */  /* delkey */
    (oal_iw_handler)wal_ioctl_get_wme_params,           /* SIOCWFIRSTPRIV+5 */  /* getwmmparams */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+6 */  /* setmlme */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+7 */  /* getchaninfo */
    (oal_iw_handler)wal_ioctl_setcountry,               /* SIOCWFIRSTPRIV+8 */  /* setcountry */
    (oal_iw_handler)wal_ioctl_getcountry,               /* SIOCWFIRSTPRIV+9 */  /* getcountry */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+10 */  /* addmac */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+11 */  /* getscanresults */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+12 */  /* delmac */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+13 */  /* getchanlist */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+14 */  /* setchanlist */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+15 */  /* kickmac */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+16 */  /* chanswitch */
    (oal_iw_handler)wal_ioctl_get_mode,                 /* SIOCWFIRSTPRIV+17 */  /* ��ȡģʽ, ��: iwpriv vapN get_mode */
    (oal_iw_handler)wal_ioctl_set_mode,                 /* SIOCWFIRSTPRIV+18 */  /* ����ģʽ, ��: iwpriv vapN mode 11g */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+19 */  /* getappiebuf */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+20 */  /* null */
    (oal_iw_handler)wal_ioctl_get_assoc_list,           /* SIOCWFIRSTPRIV+21 */  /* APUTȡ�ù���STA�б� */
    (oal_iw_handler)wal_ioctl_set_mac_filters,          /* SIOCWFIRSTPRIV+22 */  /* APUT����STA���� */
    (oal_iw_handler)wal_ioctl_set_ap_config,            /* SIOCWFIRSTPRIV+23 */  /* ����APUT���� */
    (oal_iw_handler)wal_ioctl_set_ap_sta_disassoc,      /* SIOCWFIRSTPRIV+24 */  /* APUTȥ����STA */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+25 */  /* getStatistics */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+26 */  /* sendmgmt */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+27 */  /* null  */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+28 */  /* null */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+29 */  /* getaclmac */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+30 */  /* sethbrparams */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+29 */  /* getaclmac */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+30 */  /* sethbrparams */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+31 */  /* setrxtimeout */
};

/*****************************************************************************
  ��������iw_handler_def����
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_iw_handler_def_stru g_st_iw_handler_def =
{
    .standard           = g_ast_iw_handlers,
    .num_standard       = OAL_ARRAY_SIZE(g_ast_iw_handlers),
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
oal_iw_handler_def_stru g_st_iw_handler_def =
{
    g_ast_iw_handlers,                      /* ��׼ioctl handler */
    OAL_ARRAY_SIZE(g_ast_iw_handlers),
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
OAL_CONST wal_ioctl_mode_map_stru g_ast_mode_map[] =
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

    {"11ac2g20",            WLAN_VHT_MODE,  WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11ac2g40plus",        WLAN_VHT_MODE,  WLAN_BAND_2G,   WLAN_BAND_WIDTH_40PLUS},
    {"11ac2g40minus",       WLAN_VHT_MODE,  WLAN_BAND_2G,   WLAN_BAND_WIDTH_40MINUS},
    /* 11n only and 11ac only, ����20M���� */
    {"11nonly2g",           WLAN_HT_ONLY_MODE,   WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11nonly5g",           WLAN_HT_ONLY_MODE,   WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11aconly",            WLAN_VHT_ONLY_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},

/* 1151�������� */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    {"11ng40",              WLAN_HT_MODE,   WLAN_BAND_2G,   WLAN_BAND_WIDTH_40M},
    {"11na40",              WLAN_HT_MODE,   WLAN_BAND_5G,   WLAN_BAND_WIDTH_40M},
    {"11ac40",              WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_40M},
    {"11ac80",              WLAN_VHT_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_80M},
#endif

    {OAL_PTR_NULL}
};

OAL_CONST oal_int8   *pauc_tx_pow_param_name[WAL_TX_POW_PARAM_BUTT] =
{
    "rf_reg_ctl",
    "mag_level",
    "ctl_level",
    "show_log",
};

/* ע��! ����Ĳ���������Ҫ�� g_dmac_config_set_dscr_param�еĺ���˳���ϸ�һ��! */
OAL_CONST oal_int8   *pauc_tx_dscr_param_name[WAL_DSCR_PARAM_BUTT] =
{
    "fbm",
    "pgl",
    "mtpgl",
    "sae",
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
    "ch_bw_exist"
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
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
OAL_CONST wal_ioctl_dyn_cali_stru g_ast_dyn_cali_cfg_map[]=
{
    {"realtime_cali_adjust",    MAC_DYN_CALI_CFG_SET_EN_REALTIME_CALI_ADJUST},
    {"2g_dscr_interval",        MAC_DYN_CALI_CFG_SET_2G_DSCR_INT},
    {"5g_dscr_interval",        MAC_DYN_CALI_CFG_SET_5G_DSCR_INT},
    {"pdet_min_th",             MAC_DYN_CALI_CFG_SET_PDET_MIN_TH},
    {"pdet_max_th",             MAC_DYN_CALI_CFG_SET_PDET_MAX_TH},
    {OAL_PTR_NULL}
};
#endif

OAL_CONST wal_ioctl_alg_cfg_stru g_ast_alg_cfg_map[] =
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
    {"sch_vap_prio",            MAC_ALG_CFG_SCHEDULE_VAP_SCH_PRIO},

    {"txbf_switch",             MAC_ALG_CFG_TXBF_MASTER_SWITCH},
    {"txbf_txmode_enb",         MAC_ALG_CFG_TXBF_TXMODE_ENABLE},
    {"txbf_bfer_enb",           MAC_ALG_CFG_TXBF_TXBFER_ENABLE},
    {"txbf_bfee_enb",           MAC_ALG_CFG_TXBF_TXBFEE_ENABLE},
    {"txbf_11nbfee_enb",        MAC_ALG_CFG_TXBF_11N_BFEE_ENABLE},
    {"txbf_txstbc_enb",         MAC_ALG_CFG_TXBF_TXSTBC_ENABLE},
    {"txbf_rxstbc_enb",         MAC_ALG_CFG_TXBF_RXSTBC_ENABLE},
    {"txbf_2g_bfer",            MAC_ALG_CFG_TXBF_2G_BFER_ENABLE},
    {"txbf_2nss_bfer",          MAC_ALG_CFG_TXBF_2NSS_BFER_ENABLE},
    {"txbf_fix_mode",           MAC_ALG_CFG_TXBF_FIX_MODE},
    {"txbf_fix_sound",          MAC_ALG_CFG_TXBF_FIX_SOUNDING},
    {"txbf_probe_int",          MAC_ALG_CFG_TXBF_PROBE_INT},
    {"txbf_rm_worst",           MAC_ALG_CFG_TXBF_REMOVE_WORST},
    {"txbf_stbl_num",           MAC_ALG_CFG_TXBF_STABLE_NUM},
    {"txbf_probe_cnt",          MAC_ALG_CFG_TXBF_PROBE_COUNT},
    {"txbf_log_enb",            MAC_ALG_CFG_TXBF_LOG_ENABLE},
    {"txbf_log_sta",            MAC_ALG_CFG_TXBF_RECORD_LOG_START},
    {"txbf_log_out",            MAC_ALG_CFG_TXBF_LOG_OUTPUT},
    {"ar_enable",               MAC_ALG_CFG_AUTORATE_ENABLE},                   /* ������ر���������Ӧ�㷨: sh hipriv.sh "vap0 alg_cfg ar_enable [1|0]" */
    {"ar_use_lowest",           MAC_ALG_CFG_AUTORATE_USE_LOWEST_RATE},          /* ������ر�ʹ���������: sh hipriv.sh "vap0 alg_cfg ar_use_lowest [1|0]" */
    {"ar_short_num",            MAC_ALG_CFG_AUTORATE_SHORT_STAT_NUM},           /* ���ö���ͳ�Ƶİ���Ŀ:sh hipriv.sh "vap0 alg_cfg ar_short_num [����Ŀ]" */
    {"ar_short_shift",          MAC_ALG_CFG_AUTORATE_SHORT_STAT_SHIFT},         /* ���ö���ͳ�Ƶİ�λ��ֵ:sh hipriv.sh "vap0 alg_cfg ar_short_shift [λ��ֵ]" */
    {"ar_long_num",             MAC_ALG_CFG_AUTORATE_LONG_STAT_NUM},            /* ���ó���ͳ�Ƶİ���Ŀ:sh hipriv.sh "vap0 alg_cfg ar_long_num [����Ŀ]" */
    {"ar_long_shift",           MAC_ALG_CFG_AUTORATE_LONG_STAT_SHIFT},          /* ���ó���ͳ�Ƶİ�λ��ֵ:sh hipriv.sh "vap0 alg_cfg ar_long_shift [λ��ֵ]" */
    {"ar_min_probe_no_up",      MAC_ALG_CFG_AUTORATE_MIN_PROBE_UP_INTVL_PKTNUM},     /* ������С����̽������:sh hipriv.sh "vap0 alg_cfg ar_min_probe_no_up [����Ŀ]" */
    {"ar_min_probe_no_down",    MAC_ALG_CFG_AUTORATE_MIN_PROBE_DOWN_INTVL_PKTNUM}, /* ������С����̽������:sh hipriv.sh "vap0 alg_cfg ar_min_probe_no_down [����Ŀ]" */
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

    {"edca_opt_co_ch_time",     MAC_ALG_CFG_EDCA_OPT_CO_CH_DET_CYCLE},  /* ͬƵ���ż������: sh hipriv.sh "vap0 alg_cfg edca_opt_co_ch_time [time]" */
    {"edca_opt_fix_param",      MAC_ALG_CFG_EDCA_FIX_PARAM_ENABLE},     /* �̶�edca�Ż���edca��������ֹ�Ĵ���ˢ��:sh hipriv.sh "vap0 alg_cfg edca_opt_fix_param 0|1" */
    {"edca_opt_en_ap",          MAC_ALG_CFG_EDCA_OPT_AP_EN_MODE},       /* apģʽ��edca�Ż�ʹ��ģʽ: sh hipriv.sh "vap0 alg_cfg edca_opt_en_ap 0|1|2" */
    {"edca_opt_en_sta",         MAC_ALG_CFG_EDCA_OPT_STA_EN},           /* staģʽ��edca�Ż�ʹ��ģʽ: sh hipriv.sh "vap0 alg_cfg edca_opt_en_sta 0|1" */
    {"txop_limit_en_sta",       MAC_ALG_CFG_TXOP_LIMIT_STA_EN},         /* staģʽ��edca txop limit�Ż�ʹ��ģʽ: sh hipriv.sh "vap0 alg_cfg txop_limit_en_sta 0|1" */
    {"edca_opt_sta_weight",     MAC_ALG_CFG_EDCA_OPT_STA_WEIGHT},       /* staģʽ��edca�Ż���weightingϵ��: sh hipriv.sh "vap0 alg_cfg edca_opt_sta_weight 0~3"*/
    {"edca_opt_nondir_th",      MAC_ALG_CFG_EDCA_OPT_NONDIR_TH},        /* non-direct��ռ�ձ����� sh hipriv.sh "vap0 alg_cfg edca_opt_nondir_th [val]" */
    {"edca_opt_th_udp",         MAC_ALG_CFG_EDCA_OPT_TH_UDP},           /* apģʽ��UDPҵ���Ӧ���б����� sh hipriv.sh "vap0 alg_cfg edca_opt_th_udp [val]" */
    {"edca_opt_th_tcp",         MAC_ALG_CFG_EDCA_OPT_TH_TCP},           /* apģʽ��tcPҵ���Ӧ���б����� sh hipriv.sh "vap0 alg_cfg edca_opt_th_tcp [val]" */
    {"edca_opt_debug_mode",     MAC_ALG_CFG_EDCA_OPT_DEBUG_MODE},       /* �Ƿ��ӡ�����Ϣ�������ڱ��ذ汾���� */
    {"edca_opt_fe_port_opt",    MAC_ALG_CFG_EDCA_OPT_FE_PORT_OPT},      /* ��԰��׿��Ż�����: sh hipriv.sh "vap0 alg_cfg edca_opt_fe_port_opt 0|1" */
    {"edca_opt_fe_port_dbg",    MAC_ALG_CFG_EDCA_OPT_FE_PORT_DBG},      /* ��԰��׿ڵ��Կ���: sh hipriv.sh "vap0 alg_cfg edca_opt_fe_port_dbg 0|1" */
    {"edca_opt_mpdu_dec_ratio", MAC_ALG_CFG_EDCA_OPT_MPDU_DEC_RATIO_TH},/* 0x21���������0x64��������MDPU���ٵı�������(%): sh hipriv.sh "vap0 alg_cfg edca_opt_mpdu_dec_ratio [ratio(%)]" */
    {"edca_opt_def_cw_mpdu_th", MAC_ALG_CFG_EDCA_OPT_DEFAULT_CW_MPDU_TH},/* Ĭ�ϲ���0x64�·��ͳɹ���MPDU����: sh hipriv.sh "vap0 alg_cfg edca_opt_def_cw_mpdu_th [num]" */

    {"cca_opt_alg_en_mode",         MAC_ALG_CFG_CCA_OPT_ALG_EN_MODE},           /* CCA�Ż�����ʹ��: sh hipriv.sh "vap0 alg_cfg cca_opt_alg_en_mode 0|1" */
    {"cca_opt_debug_mode",          MAC_ALG_CFG_CCA_OPT_DEBUG_MODE},            /* CCA�Ż�DEBUGģʽ����: sh hipriv.sh "vap0 alg_cfg cca_opt_debug_mode 0|1" */
    {"cca_opt_set_t1_counter_time", MAC_ALG_CFG_CCA_OPT_SET_T1_COUNTER_TIME},   /* CCA�Ż�T1��ʱ����:sh hipriv.sh "vap0 alg_cfg cca_opt_set_t1_counter_time [time]" */
    {"cca_opt_set_t2_counter_time", MAC_ALG_CFG_CCA_OPT_SET_T2_COUNTER_TIME},   /* CCA�Ż�T2��ʱ����:sh hipriv.sh "vap0 alg_cfg cca_opt_set_t2_counter_time [time]" */
    {"cca_opt_set_ilde_cnt_th",     MAC_ALG_CFG_CCA_OPT_SET_ILDE_CNT_TH},       /* CCA�Ż��ж��Ƿ����ƽ��RSSI�Ŀ��й��ʷ�0ֵ�Ĵ�������:sh hipriv.sh "vap0 alg_cfg cca_opt_set_ilde_cnt_th [val]" */
    {"cca_opt_set_duty_cyc_th",     MAC_ALG_CFG_CCA_OPT_SET_DUTY_CYC_TH},       /* CCA�Ż��ж��Ƿ����ǿ��Ƶ����Ƶ���ŵķ�æ����ֵ:sh hipriv.sh "vap0 alg_cfg cca_opt_set_duty_cyc_th [val]" */
    {"cca_opt_set_aveg_rssi_th",    MAC_ALG_CFG_CCA_OPT_SET_AVEG_RSSI_TH},      /* CCA�Ż��ж��Ƿ������Ƶ����Ƶ���ŵ�sync error��ֵ:sh hipriv.sh "vap0 alg_cfg cca_opt_set_aveg_rssi_th [val]" */
    {"cca_opt_set_chn_scan_cyc",    MAC_ALG_CFG_CCA_OPT_SET_CHN_SCAN_CYC},      /* CCA�Ż��ж��Ƿ������Ƶ����Ƶ���ŵ�pri20/40/80�������ֵ:sh hipriv.sh "vap0 alg_cfg cca_opt_set_chn_scan_cyc [val]" */
    {"cca_opt_set_sync_err_th",     MAC_ALG_CFG_CCA_OPT_SET_SYNC_ERR_TH},       /* CCA�Ż��ŵ�ɨ���ʱ��(ms):sh hipriv.sh "vap0 alg_cfg cca_opt_set_sync_err_th [time]" */
    {"cca_opt_set_cca_th_debug",    MAC_ALG_CFG_CCA_OPT_SET_CCA_TH_DEBUG},      /* CCA�Ż��ŵ�ɨ���ʱ��(ms):sh hipriv.sh "vap0 alg_cfg cca_opt_set_sync_err_th [time]" */
    {"cca_opt_log",                 MAC_ALG_CFG_CCA_OPT_LOG},                   /* CCA log���� sh hipriv.sh "vap0 alg_cfg cca_opt_log 0|1"*/
    {"cca_opt_stat_log_do",         MAC_ALG_CFG_CCA_OPT_STAT_LOG_START},        /* ��ʼͳ����־:sh hipriv.sh "vap0 alg_cca_opt_log cca_opt_stat_log_do [val]"  */
    {"cca_opt_stat_log_out",        MAC_ALG_CFG_CCA_OPT_STAT_LOG_WRITE},        /* ��ӡͳ����־:sh hipriv.sh "vap0 alg_cca_opt_log cca_opt_stat_log_out" */
    {"cca_opt_set_collision_ratio_th",MAC_ALG_CFG_CCA_OPT_SET_COLLISION_RATIO_TH},  /* CCA����������ײ������(ms):sh hipriv.sh "vap0 alg_cfg cca_opt_set_collision_ratio_th [val]" */
    {"cca_opt_set_goodput_loss_th", MAC_ALG_CFG_CCA_OPT_SET_GOODPUT_LOSS_TH},       /* CCA��������goddput����(ms):sh hipriv.sh "vap0 alg_cfg cca_opt_set_goodput_loss_th [val]" */
    {"cca_opt_set_max_intvl_num",   MAC_ALG_CFG_CCA_OPT_SET_MAX_INTVL_NUM},         /* CCA�����������̽����(ms):sh hipriv.sh "vap0 alg_cfg cca_opt_set_max_intvl_num [val]" */
    {"cca_opt_set_non_intf_cyc_num",       MAC_ALG_CFG_CCA_OPT_NON_INTF_CYCLE_NUM_TH},/* CCA�޸��ż���������(ms):sh hipriv.sh "vap0 alg_cfg cca_opt_set_non_intf_cyc_num [val]" */
    {"cca_opt_set_non_intf_duty_cyc_th",   MAC_ALG_CFG_CCA_OPT_NON_INTF_DUTY_CYC_TH},/* CCA�޸��ż���������(ms):sh hipriv.sh "vap0 alg_cfg cca_opt_set_non_intf_duty_cyc_th [val]" */

    {"tpc_mode",                MAC_ALG_CFG_TPC_MODE},                          /* ����TPC����ģʽ */
    {"tpc_dbg",                 MAC_ALG_CFG_TPC_DEBUG},                         /* ����TPC��debug���� */
    {"tpc_pow_lvl",             MAC_ALG_CFG_TPC_POWER_LEVEL},                   /* ����TPC���ʵȼ�(0,1,2,3), �ڹ̶�����ģʽ��ʹ�� */
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
    {"tpc_show_log_info",       MAC_ALG_CFG_TPC_SHOW_LOG_INFO},                 /* ��ӡTPC����־��Ϣ:sh hipriv.sh "vap0 alg_cfg tpc_show_log_info */
    {"tpc_no_margin_pow",       MAC_ALG_CFG_TPC_NO_MARGIN_POW},                 /* 51����û���������� */
    {"tpc_power_amend",         MAC_ALG_CFG_TPC_POWER_AMEND},                   /* tx power�ڴ��ڲ�ƽ̹��tpc���й���������Ĭ��Ϊ0*/
    {"temp_pro_get_info",       MAC_ALG_CFG_TEMP_PRO_GET_INFO},                 /* ���±�����ȡά����Ϣ */

    {OAL_PTR_NULL}
};

OAL_CONST wal_dfs_domain_entry_stru g_ast_dfs_domain_table[] =
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
    {"CN", MAC_DFS_DOMAIN_NULL},
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

oal_uint32  wal_get_cmd_one_arg(oal_int8 *pc_cmd, oal_int8 *pc_arg, oal_uint32 *pul_cmd_offset)
{
    oal_int8   *pc_cmd_copy;
    oal_uint32  ul_pos = 0;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pc_cmd) || (OAL_PTR_NULL == pc_arg) || (OAL_PTR_NULL == pul_cmd_offset)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_get_cmd_one_arg::pc_cmd/pc_arg/pul_cmd_offset null ptr error %d, %d, %d!}\r\n", pc_cmd, pc_arg, pul_cmd_offset);
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
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_get_cmd_one_arg::ul_pos >= WAL_HIPRIV_CMD_NAME_MAX_LEN, ul_pos %d!}\r\n", ul_pos);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }

    pc_arg[ul_pos]  = '\0';

    /* �ַ�������β�����ش����� */
    if (0 == ul_pos)
    {
        OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_get_cmd_one_arg::return param pc_arg is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pul_cmd_offset = (oal_uint32)(pc_cmd_copy - pc_cmd);

    return OAL_SUCC;
}


oal_void wal_msg_queue_init(oal_void)
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


oal_uint32 wal_get_request_msg_count(oal_void)
{
    return g_wal_wid_msg_queue.count;
}

oal_uint32 wal_check_and_release_msg_resp(wal_msg_stru   *pst_rsp_msg)
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
            OAM_WARNING_LOG2(0, OAM_SF_SCAN, "{wal_check_and_release_msg_resp::detect err code:[%u],wid:[%u]}",
                           ul_err_code, en_wid);
            return ul_err_code;
        }
    }

    return OAL_SUCC;
}


oal_void wal_msg_request_add_queue(wal_msg_request_stru* pst_msg)
{
#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
    if(OAL_FALSE== g_wal_wid_queue_init_flag)
    {
        wal_msg_queue_init();
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


oal_void wal_msg_request_remove_queue(wal_msg_request_stru* pst_msg)
{
    oal_spin_lock_bh(&g_wal_wid_msg_queue.st_lock);
    _wal_msg_request_remove_queue_(pst_msg);
    oal_spin_unlock_bh(&g_wal_wid_msg_queue.st_lock);
}


oal_int32  wal_set_msg_response_by_addr(oal_ulong addr,oal_void * pst_resp_mem ,oal_uint32 ul_resp_ret,
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
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_set_msg_response_by_addr::wal_set_msg_response_by_addr response had been set!");
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


oal_uint32  wal_alloc_cfg_event
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
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_alloc_cfg_event::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr! pst_net_dev=[%p]}", pst_net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }

    us_resp_len += OAL_SIZEOF(wal_msg_rep_hdr);

    us_len += us_resp_len;

    pst_event_mem = FRW_EVENT_ALLOC(us_len);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG2(pst_vap->uc_vap_id, OAM_SF_ANY, "{wal_alloc_cfg_event::pst_event_mem null ptr error,request size:us_len:%d,resp_len:%d}",
                        us_len, us_resp_len);
        return OAL_ERR_CODE_PTR_NULL;
    }

    *ppst_event_mem = pst_event_mem;    /* ���θ�ֵ */

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

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

oal_void wal_cfg_msg_task_sched(oal_void)
{
    OAL_WAIT_QUEUE_WAKE_UP(&g_wal_wid_msg_queue.st_wait_queue);
}


oal_int32 wal_send_cfg_event(oal_net_device_stru      *pst_net_dev,
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
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_send_cfg_event::OAL_PTR_NULL == ppst_rsp_msg!}\r\n");
        return -OAL_EINVAL;
    }

    /* �����¼� */
    ul_ret = wal_alloc_cfg_event(pst_net_dev, &pst_event_mem,
                                ((OAL_TRUE == en_need_rsp) ? &st_msg_request : NULL),
                                &pst_cfg_msg,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_send_cfg_event::wal_alloc_cfg_event return err code %d!}\r\n", ul_ret);
        return -OAL_ENOMEM;
    }

    /* ��д������Ϣ */
    WAL_CFG_MSG_HDR_INIT(&(pst_cfg_msg->st_msg_hdr),
                         en_msg_type,
                         us_len,
                         WAL_GET_MSG_SN());

    /* ��дWID��Ϣ */
    oal_memcopy(pst_cfg_msg->auc_msg_data, puc_param, us_len);

#ifdef _PRE_WLAN_DFT_EVENT
    wal_event_report_to_sdt(en_msg_type, puc_param, pst_cfg_msg);
#endif

    if (OAL_TRUE == en_need_rsp)
    {
        /*add queue before post event!*/
        wal_msg_request_add_queue(&st_msg_request);
    }

/* �ַ��¼� */
#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0 , OAM_SF_ANY, "{wal_send_cfg_event::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}");
        FRW_EVENT_FREE(pst_event_mem);
        return -OAL_EINVAL;
    }

    frw_event_post_event(pst_event_mem, pst_mac_vap->ul_core_id);
#else
    frw_event_dispatch_event(pst_event_mem);
#endif
    FRW_EVENT_FREE(pst_event_mem);

    /* win32 UTģʽ������һ���¼����� */
#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION) && (_PRE_TEST_MODE == _PRE_TEST_MODE_UT)
    frw_event_process_all_event(0);
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


    /*lint -e730*//* info, boolean argument to function */
    l_ret = OAL_WAIT_EVENT_TIMEOUT(g_wal_wid_msg_queue.st_wait_queue,
                                                OAL_TRUE == wal_request_wait_event_condition(&st_msg_request),
                                                10 * OAL_TIME_HZ);
    /*lint +e730*/

    /*response had been set, remove it from the list*/
    if (OAL_TRUE == en_need_rsp)
    {
        wal_msg_request_remove_queue(&st_msg_request);
    }

    if (OAL_WARN_ON(0 == l_ret))
    {
        /* ��ʱ .*/
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_send_cfg_event:: wait queue timeout,10s!}\r\n");
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
        frw_event_queue_info();
        return -OAL_ETIMEDOUT;
    }
    /*lint +e774*/

    pst_rsp_msg = (wal_msg_stru *)(st_msg_request.pst_resp_mem);
    if (OAL_PTR_NULL == pst_rsp_msg)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_send_cfg_event:: msg mem null!}");
        /*lint -e613*/
        *ppst_rsp_msg  = OAL_PTR_NULL;
        /*lint +e613*/
        wal_wake_unlock();
        return -OAL_EFAUL;
    }

    if (0 == pst_rsp_msg->st_msg_hdr.us_msg_len)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_send_cfg_event:: no msg resp!}");
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


wlan_p2p_mode_enum_uint8 wal_wireless_iftype_to_mac_p2p_mode(enum nl80211_iftype en_iftype)
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


oal_int32  wal_cfg_vap_h2d_event(oal_net_device_stru *pst_net_dev)
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
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::pst_wdev is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_wiphy_priv  = (mac_wiphy_priv_stru *)oal_wiphy_priv(pst_wdev->wiphy);
    if (OAL_PTR_NULL == pst_wiphy_priv)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::pst_wiphy_priv is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device  = pst_wiphy_priv->pst_mac_device;
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::pst_mac_device is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_hmac_vap= (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->uc_cfg_vap_id);
    if (OAL_PTR_NULL == pst_cfg_hmac_vap)
    {
        OAM_WARNING_LOG1(0,OAM_SF_ANY,"{wal_cfg_vap_h2d_event::mac_res_get_hmac_vap fail.vap_id[%u]}",pst_mac_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;
    if(NULL == pst_cfg_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::pst_cfg_net_dev is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
    ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG_VAP_H2D, OAL_SIZEOF(mac_cfg_vap_h2d_stru));
    ((mac_cfg_vap_h2d_stru *)st_write_msg.auc_value)->pst_net_dev = pst_cfg_net_dev;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
             WAL_MSG_TYPE_WRITE,
             WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_vap_h2d_stru),
             (oal_uint8 *)&st_write_msg,
             OAL_TRUE,
             &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::hmac cfg vap h2d fail,err code[%u]\r\n", ul_err_code);
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

    if (OAL_PTR_NULL == pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_host_dev_config::pst_net_dev is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
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
    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
             WAL_MSG_TYPE_WRITE,
             WAL_MSG_WRITE_MSG_HDR_LENGTH,
             (oal_uint8 *)&st_write_msg,
             OAL_TRUE,
             &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::hmac cfg vap h2d fail,err code[%u]\r\n", ul_err_code);
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}


oal_int32  wal_host_dev_init(oal_net_device_stru *pst_net_dev)
{
    return wal_host_dev_config(pst_net_dev, WLAN_CFGID_HOST_DEV_INIT);
}


oal_int32  wal_host_dev_exit(oal_net_device_stru *pst_net_dev)
{
    return wal_host_dev_config(pst_net_dev, WLAN_CFGID_HOST_DEV_EXIT);
}
#endif


#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

oal_uint32 hwifi_config_init_base_power_main(oal_net_device_stru* pst_net_dev)
{
    wlan_customize_power_params_stru  *pst_nvram;
    wal_msg_write_stru       st_write_msg;
    oal_int32                l_ret;

    pst_nvram = hwifi_get_nvram_params();

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_BASE_POWER,  OAL_SIZEOF(oal_uint8)*CUS_BASE_POWER_NUM);
    oal_memcopy(st_write_msg.auc_value, pst_nvram->auc_base_power, OAL_SIZEOF(oal_uint8)*CUS_BASE_POWER_NUM);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8)*CUS_BASE_POWER_NUM,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_base_power_main::return err code [%d]!}\r\n", l_ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


oal_uint32 hwifi_config_init_fcc_ce_power_main(oal_net_device_stru* pst_net_dev)
{
    wlan_customize_power_params_stru  *pst_nvram;
    wal_msg_write_stru                 st_write_msg;
    oal_int32                          l_ret;
    regdomain_enum                     en_regdomain = REGDOMAIN_COMMON;

    en_regdomain = hwifi_get_regdomain_from_country_code(hwifi_get_country_code());
    if ((en_regdomain != REGDOMAIN_FCC) && (en_regdomain != REGDOMAIN_ETSI))
    {
        OAM_WARNING_LOG0(0,0,"hwifi_config_init_fcc_ce_power_main:the country is not a fcc or ce country");
        return OAL_SUCC;
    }
    pst_nvram = hwifi_get_nvram_params();

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_FCC_CE_POWER, OAL_SIZEOF(wlan_fcc_ce_power_limit_stru));
    oal_memcopy(st_write_msg.auc_value, &pst_nvram->st_fcc_ce_txpwer_limit, OAL_SIZEOF(wlan_fcc_ce_power_limit_stru));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(wlan_fcc_ce_power_limit_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_fcc_ce_power_main::return err code [%d]!}\r\n", l_ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32 hwifi_force_refresh_rf_params(oal_net_device_stru* pst_net_dev)
{
    /* update params */
    if (hwifi_config_init(CUS_TAG_NV))
    {
        return OAL_FAIL;
    }

    /* �����ƻ�������Ϣ�·���device */
    hwifi_config_init_fcc_ce_power_main(pst_net_dev);

    /* send data to device */
    return hwifi_config_init_nvram_main(pst_net_dev);
}


OAL_STATIC oal_int32 hwifi_config_host_global_ini_param(oal_void)
{
#ifdef _PRE_WLAN_FEATURE_ROAM
    oal_int32                       l_val = 0;
#endif /* #ifdef _PRE_WLAN_FEATURE_ROAM */
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ

    oal_uint32                      cfg_id;
    oal_int32                       l_cfg_value;
    oal_int8*                       pc_tmp;
    device_speed_freq_level_stru    ast_device_speed_freq_level_tmp[4];
    host_speed_freq_level_stru      ast_host_speed_freq_level_tmp[4];
    oal_uint8                       uc_flag = OAL_FALSE;
    oal_uint8                       uc_index;
    hmac_vap_stru *             pst_cfg_hmac_vap;
    oal_net_device_stru        *pst_cfg_net_dev;
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;
#endif /* #ifdef _PRE_WLAN_FEATURE_AUTO_FREQ */
    /******************************************** ���� ********************************************/
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_AMPDU_TX_MAX_NUM);
    g_st_wlan_customize.ul_ampdu_tx_max_num = (WLAN_AMPDU_TX_MAX_NUM >= l_val && 1 <= l_val) ? (oal_uint32)l_val : g_st_wlan_customize.ul_ampdu_tx_max_num;
    OAL_IO_PRINT("hisi_customize_wifi::ampdu_tx_max_num:%d", g_st_wlan_customize.ul_ampdu_tx_max_num);
#ifdef _PRE_WLAN_FEATURE_ROAM
    /******************************************** ���� ********************************************/
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_ROAM_SWITCH);
    g_st_wlan_customize.uc_roam_switch          = (0 == l_val || 1 == l_val) ? (oal_uint8)l_val : g_st_wlan_customize.uc_roam_switch;
    g_st_wlan_customize.uc_roam_scan_band       = band_5g_enabled ? (BIT0|BIT1) : BIT0;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SCAN_ORTHOGONAL);
    g_st_wlan_customize.uc_roam_scan_orthogonal = (1 <= l_val) ? (oal_uint8)l_val : g_st_wlan_customize.uc_roam_scan_orthogonal;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TRIGGER_B);
    g_st_wlan_customize.c_roam_trigger_b = (oal_int8)l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TRIGGER_A);
    g_st_wlan_customize.c_roam_trigger_a = (oal_int8)l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_B);
    g_st_wlan_customize.c_roam_delta_b = (oal_int8)l_val;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_A);
    g_st_wlan_customize.c_roam_delta_a = (oal_int8)l_val;
#endif /* #ifdef _PRE_WLAN_FEATURE_ROAM */
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    /******************************************** �Զ���Ƶ ********************************************/
    /* config pps_value*/
    uc_index = 0;
    for(cfg_id = WLAN_CFG_INIT_PPS_THRESHOLD_LEVEL_0; cfg_id <= WLAN_CFG_INIT_PPS_THRESHOLD_LEVEL_3; ++cfg_id)
    {
        l_cfg_value = hwifi_get_init_value(CUS_TAG_INI, cfg_id);

        if(l_cfg_value >= PPS_VALUE_MIN && l_cfg_value <= PPS_VALUE_MAX)
        {
            ast_host_speed_freq_level_tmp[uc_index].ul_speed_level = l_cfg_value;
            uc_index++;
        }
        else
        {
            uc_flag = OAL_TRUE;
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "hwifi_config_host_global_ini_param :: The pps value [%d] is out of range!! ",l_cfg_value);
            break;
        }
    }

    /* config g_device_speed_freq_level */
    pc_tmp = (oal_int8*)&ast_device_speed_freq_level_tmp;
    for(cfg_id = WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_0; cfg_id <= WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_3; ++cfg_id)
    {
        l_cfg_value = hwifi_get_init_value(CUS_TAG_INI, cfg_id);
        if(l_cfg_value >= FREQ_IDLE && l_cfg_value <= FREQ_HIGHEST)
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
        /* ֻ�������������ṹ��������Ԫ�ض���ȷ����ֵ�ŻὫ���϶��ƻ���ֵ�䵽device�Զ���Ƶ�У����򱣳�Ĭ��ֵ */
        oal_memcopy(&g_host_speed_freq_level, &ast_host_speed_freq_level_tmp, OAL_SIZEOF(g_host_speed_freq_level));
        oal_memcopy(&g_device_speed_freq_level, &ast_device_speed_freq_level_tmp, OAL_SIZEOF(g_device_speed_freq_level));

        for(uc_index = 0;uc_index < 4;uc_index++)
        {
            OAM_WARNING_LOG2(0, OAM_SF_ANY, "{hwifi_config_host_global_ini_param::1.uc_device_type = [%d],2. pps_value = [%d]}\r\n",
                            g_device_speed_freq_level[uc_index].uc_device_type, g_host_speed_freq_level[uc_index].ul_speed_level);
        }

        /* �·����ƻ�������device */
        pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(0);
        if (OAL_PTR_NULL == pst_cfg_hmac_vap)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_host_global_ini_param::pst_cfg_hmac_vap is null��}\r\n");
            return -OAL_EFAUL;
        }

        pst_cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;
        if(OAL_PTR_NULL == pst_cfg_net_dev)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_host_global_ini_param::pst_cfg_net_dev is null!}\r\n");
            return -OAL_EFAUL;
        }

        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_DEVICE_FREQ_VALUE, OAL_SIZEOF(oal_int32));
        *((oal_int8 *)(st_write_msg.auc_value)) = OAL_TRUE;

        l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);

        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_host_global_ini_param::return err code [%d]!}\r\n", l_ret);
        }
    }
#endif /* #ifdef _PRE_WLAN_FEATURE_AUTO_FREQ */
    /******************************************** ɨ�� ********************************************/
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RANDOM_MAC_ADDR_SCAN);
    g_st_wlan_customize.uc_random_mac_addr_scan = !!l_val;

    /******************************************** CAPABILITY ********************************************/
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DISABLE_CAPAB_2GHT40);
    g_st_wlan_customize.uc_disable_capab_2ght40 = (oal_uint8)!!l_val;
    /********************************************factory_lte_gpio_check ********************************************/
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_LTE_GPIO_CHECK_SWITCH);
    g_st_wlan_customize.ul_lte_gpio_check_switch = (oal_uint32)!!l_val;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_ATCMDSRV_LTE_ISM_PRIORITY);
    g_st_wlan_customize.ul_lte_ism_priority = (oal_uint32)l_val;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_ATCMDSRV_LTE_RX_ACT);
    g_st_wlan_customize.ul_lte_rx_act = (oal_uint32)l_val;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_ATCMDSRV_LTE_TX_ACT);
    g_st_wlan_customize.ul_lte_tx_act = (oal_uint32)l_val;

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_AMPDU_AMSDU_SKB);
    g_st_tx_large_amsdu.uc_tx_amsdu_ampdu_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_AMSDU_AMPDU_TH_HIGH);
    g_st_tx_large_amsdu.us_amsdu_ampdu_throughput_high = (l_val > 0) ? (oal_uint16)l_val : WLAN_AMSDU_AMPDU_THROUGHPUT_THRESHOLD_HIGH;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_AMSDU_AMPDU_TH_LOW);
    g_st_tx_large_amsdu.us_amsdu_ampdu_throughput_low = (l_val > 0) ? (oal_uint16)l_val : WLAN_AMSDU_AMPDU_THROUGHPUT_THRESHOLD_LOW;
    OAL_IO_PRINT("ampdu+amsdu lareg skb en[%d],high[%d],low[%d]\r\n", g_st_tx_large_amsdu.uc_tx_amsdu_ampdu_en,g_st_tx_large_amsdu.us_amsdu_ampdu_throughput_high,g_st_tx_large_amsdu.us_amsdu_ampdu_throughput_low);
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RX_AMPDU_AMSDU_SKB);
    g_uc_host_rx_ampdu_amsdu  = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    OAL_IO_PRINT("Rx:ampdu+amsdu skb en[%d]\r\n", g_uc_host_rx_ampdu_amsdu);
#endif

#ifdef _PRE_WLAN_TCP_OPT
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_FILTER);
    g_st_tcp_ack_opt_th_params.uc_tcp_ack_filter_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_FILTER_TH_HIGH);
    g_st_tcp_ack_opt_th_params.us_rx_filter_throughput_high = (l_val > 0) ? (oal_uint16)l_val : WLAN_TCP_ACK_FILTER_TH_HIGH;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_FILTER_TH_LOW);
    g_st_tcp_ack_opt_th_params.us_rx_filter_throughput_low = (l_val > 0) ? (oal_uint16)l_val : WLAN_TCP_ACK_FILTER_TH_LOW;
    OAL_IO_PRINT("tcp ack filter en[%d],high[%d],low[%d]\r\n", g_st_tcp_ack_opt_th_params.uc_tcp_ack_filter_en, g_st_tcp_ack_opt_th_params.us_rx_filter_throughput_high, g_st_tcp_ack_opt_th_params.us_rx_filter_throughput_low);
#endif

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_BINDCPU);
    g_st_thread_bindcpu.en_irq_bindcpu= (l_val > 0) ? OAL_TRUE : OAL_FALSE;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_BINDCPU_MASK);
    g_st_thread_bindcpu.uc_cpumask = (l_val > 0) ? (oal_uint8) l_val : WLAN_BINDCPU_DEFAULT_MASK;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_BINDCPU_TXHIGH);
    g_st_thread_bindcpu.us_tx_throughput_irq_high= (l_val > 0) ? (oal_uint16)l_val : WLAN_TX_BUSY_CPU_THROUGHT;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_BINDCPU_TXLOW);
    g_st_thread_bindcpu.us_tx_throughput_irq_low = (l_val > 0) ? (oal_uint16)l_val : WLAN_TX_IDLE_CPU_THROUGHT;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_BINDCPU_RXHIGH);
    g_st_thread_bindcpu.us_rx_throughput_irq_high = (l_val > 0) ? (oal_uint16)l_val : WLAN_RX_BUSY_CPU_THROUGHT;

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_BINDCPU_RXLOW);
    g_st_thread_bindcpu.us_rx_throughput_irq_low = (l_val > 0) ? (oal_uint16)l_val : WLAN_RX_IDLE_CPU_THROUGHT;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TX_TCP_ACK_BUF);
    g_st_tcp_ack_buf_switch.uc_ini_tcp_ack_buf_en= (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_HIGH);
    g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high = (l_val > 0) ? (oal_uint16)l_val : WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_LOW);
    g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low = (l_val > 0) ? (oal_uint16)l_val : WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_40M);
    g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_40M = (l_val > 0) ? (oal_uint16)l_val : WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_40M;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_LOW_40M);
    g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_40M = (l_val > 0) ? (oal_uint16)l_val : WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW_40M;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_80M);
    g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_80M = (l_val > 0) ? (oal_uint16)l_val : WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_80M;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TCP_ACK_BUF_LOW_80M);
    g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_80M = (l_val > 0) ? (oal_uint16)l_val : WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW_80M;
    OAL_IO_PRINT("TCP ACK BUF en[%d],20M_high[%d],20M_low[%d],40M_high[%d],40M_low[%d],80M_high[%d],80M_low[%d]...\r\n",
        g_st_tcp_ack_buf_switch.uc_ini_tcp_ack_buf_en,
        g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high,g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low,
        g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_40M,g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_40M,
        g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_80M,g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_80M);

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA);
    g_st_rx_dyn_bypass_extlna_switch.uc_ini_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    g_st_rx_dyn_bypass_extlna_switch.uc_cur_status = OAL_TRUE;/* Ĭ�ϵ͹��ĳ��� */
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_HIGH);
    g_st_rx_dyn_bypass_extlna_switch.us_throughput_high = (l_val > 0) ? (oal_uint16)l_val : WLAN_DYN_BYPASS_EXTLNA_THROUGHPUT_THRESHOLD_HIGH;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_LOW);
    g_st_rx_dyn_bypass_extlna_switch.us_throughput_low = (l_val > 0) ? (oal_uint16)l_val : WLAN_DYN_BYPASS_EXTLNA_THROUGHPUT_THRESHOLD_LOW;
    OAL_IO_PRINT("DYN_BYPASS_EXTLNA SWITCH en[%d],high[%d],low[%d]\r\n", g_st_rx_dyn_bypass_extlna_switch.uc_ini_en,g_st_rx_dyn_bypass_extlna_switch.us_throughput_high,g_st_rx_dyn_bypass_extlna_switch.us_throughput_low);

    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_TX_SMALL_AMSDU);
    g_st_small_amsdu_switch.uc_ini_small_amsdu_en = (l_val > 0) ? OAL_TRUE : OAL_FALSE;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SMALL_AMSDU_HIGH);
    g_st_small_amsdu_switch.us_small_amsdu_throughput_high = (l_val > 0) ? (oal_uint16)l_val : WLAN_SMALL_AMSDU_THROUGHPUT_THRESHOLD_HIGH;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SMALL_AMSDU_LOW);
    g_st_small_amsdu_switch.us_small_amsdu_throughput_low = (l_val > 0) ? (oal_uint16)l_val : WLAN_SMALL_AMSDU_THROUGHPUT_THRESHOLD_LOW;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SMALL_AMSDU_PPS_HIGH);
    g_st_small_amsdu_switch.us_small_amsdu_pps_high = (l_val > 0) ? (oal_uint16)l_val : WLAN_SMALL_AMSDU_PPS_THRESHOLD_HIGH;
    l_val = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SMALL_AMSDU_PPS_LOW);
    g_st_small_amsdu_switch.us_small_amsdu_pps_low = (l_val > 0) ? (oal_uint16)l_val : WLAN_SMALL_AMSDU_PPS_THRESHOLD_LOW;
    OAL_IO_PRINT("SMALL AMSDU SWITCH en[%d],high_throughput [%d],low_throughput [%d],pps_high [%d], pps_low [%d]...\r\n",
        g_st_small_amsdu_switch.uc_ini_small_amsdu_en,
        g_st_small_amsdu_switch.us_small_amsdu_throughput_high,g_st_small_amsdu_switch.us_small_amsdu_throughput_low,
        g_st_small_amsdu_switch.us_small_amsdu_pps_high,g_st_small_amsdu_switch.us_small_amsdu_pps_low);

#endif

    return OAL_SUCC;
}

OAL_STATIC oal_void hwifi_config_init_ini_perf(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;
    oal_int8                    pc_param[18]    = {0};
    oal_int8                    pc_tmp[8]       = {0};
    oal_uint16                  us_len;
    oal_uint8                   uc_sdio_assem_h2d;
    oal_uint8                   uc_sdio_assem_d2h;

    /* SDIO FLOWCTRL */
    //device�����Ϸ����ж�
    oal_itoa(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_USED_MEM_FOR_START), pc_param, 8);
    oal_itoa(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_USED_MEM_FOR_STOP), pc_tmp, 8);
    pc_param[OAL_STRLEN(pc_param)] = ' ';
    oal_memcopy(pc_param + OAL_STRLEN(pc_param), pc_tmp, OAL_STRLEN(pc_tmp));

    oal_memcopy(st_write_msg.auc_value, pc_param, OAL_STRLEN(pc_param));
    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';
    us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SDIO_FLOWCTRL, us_len);

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini_perf::return err code [%d]!}\r\n", l_ret);
    }

    /* SDIO ASSEMBLE COUNT:H2D */
    uc_sdio_assem_h2d = (oal_uint8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SDIO_H2D_ASSEMBLE_COUNT);
    //�ж�ֵ�ĺϷ���
    if (uc_sdio_assem_h2d >= 1 && uc_sdio_assem_h2d <= HISDIO_HOST2DEV_SCATT_MAX)
    {
        hcc_assemble_count = uc_sdio_assem_h2d;
    }
    else
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hwifi_config_init_ini_perf::sdio_assem_h2d[%d] out of range(0,%d], check value in ini file!}\r\n",
                            uc_sdio_assem_h2d, HISDIO_HOST2DEV_SCATT_MAX);
    }

    /* SDIO ASSEMBLE COUNT:D2H */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_D2H_HCC_ASSEMBLE_CNT, OAL_SIZEOF(oal_int32));
    uc_sdio_assem_d2h = (oal_uint8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_SDIO_D2H_ASSEMBLE_COUNT);

    //�ж�ֵ�ĺϷ���
    if(uc_sdio_assem_d2h >= 1 && uc_sdio_assem_d2h <= HISDIO_DEV2HOST_SCATT_MAX)
    {
        *((oal_int32 *)(st_write_msg.auc_value)) = uc_sdio_assem_d2h;
    }
    else
    {
        *((oal_int32 *)(st_write_msg.auc_value)) = HISDIO_DEV2HOST_SCATT_MAX;
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hwifi_config_init_ini_perf::sdio_assem_d2h[%d] out of range(0,%d], check value in ini file!}\r\n",
                            uc_sdio_assem_d2h, HISDIO_DEV2HOST_SCATT_MAX);
    }

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini_perf::return err code [%d]!}\r\n", l_ret);
    }
}

OAL_STATIC oal_void hwifi_config_init_ini_linkloss(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_LINKLOSS_THRESHOLD, OAL_SIZEOF(mac_cfg_linkloss_threshold));
    ((mac_cfg_linkloss_threshold *)(st_write_msg.auc_value))->uc_linkloss_threshold_wlan_bt     = (oal_uint8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_WLAN_BT);
    ((mac_cfg_linkloss_threshold *)(st_write_msg.auc_value))->uc_linkloss_threshold_wlan_dbac   = (oal_uint8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_WLAN_DBAC);
    ((mac_cfg_linkloss_threshold *)(st_write_msg.auc_value))->uc_linkloss_threshold_wlan_normal = (oal_uint8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_WLAN_NORMAL);

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_linkloss_threshold),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini_linkloss::wal_send_cfg_event return err code [%d]!}\r\n", l_ret);
    }
}

OAL_STATIC oal_void hwifi_config_init_ini_country(oal_net_device_stru *pst_cfg_net_dev)
{
    oal_int32                   l_ret;

    l_ret = (oal_int32)wal_hipriv_setcountry(pst_cfg_net_dev, hwifi_get_country_code());

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini_country::wal_send_cfg_event return err code [%d]!}\r\n", l_ret);
    }
}

OAL_STATIC oal_void hwifi_config_init_ini_pm(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;
    oal_int32                   l_switch;

    /*************************** �͹���ps_mode���ƻ� *****************************/
    g_wlan_ps_mode                   = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_PS_MODE);
    /*************************** ˽�ж��ƻ� *******************************/

    /* ���� */
    l_switch = !!hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_POWERMGMT_SWITCH);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_PM_SWITCH, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_switch;

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
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
#ifdef _PRE_WLAN_FEATURE_SMARTANT
OAL_STATIC oal_void hwifi_config_init_ini_dual_antenna(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;
    oal_int32                   l_switch;
    oal_uint8                   *puc_param;
    l_switch = !!hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DUAL_ANTENNA_ENABLE);
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DOUBLE_ANT_SW, OAL_SIZEOF(oal_int32));
    puc_param = (oal_uint8 *)(st_write_msg.auc_value);
    *puc_param = (oal_uint8)l_switch;
    *(puc_param + 1) = 1;
    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
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
    l_loglevel = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LOGLEVEL);
    if (l_loglevel < OAM_LOG_LEVEL_ERROR ||
        l_loglevel > OAM_LOG_LEVEL_INFO)
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{hwifi_config_init_ini_clock::loglevel[%d] out of range[%d,%d], check value in ini file!}\r\n",
                            l_loglevel, OAM_LOG_LEVEL_ERROR, OAM_LOG_LEVEL_INFO);
        return;
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_ALL_LOG_LEVEL, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_loglevel;
    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
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

OAL_STATIC oal_void hwifi_config_init_ini_rf(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;
    oal_uint8                   uc_idx;                 /*�ṹ�������±�*/
    oal_uint8                   uc_error_param = OAL_FALSE;    /* ������Ч�Ա�־���������ֵ���Ϸ�����Ϊ1�����в������·� */
    wlan_cfg_customize_rf*               pst_customize_rf;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_RF, OAL_SIZEOF(wlan_cfg_customize_rf));

    pst_customize_rf = (wlan_cfg_customize_rf *)(st_write_msg.auc_value);

    /* ����: 2g rf */
    for (uc_idx=0; uc_idx<MAC_NUM_2G_BAND; ++uc_idx)
    {
        /* ��ȡ��2p4g ��band 0.25db��0.1db���ȵ�����ֵ */
        oal_int8 c_mult4 = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND_START+2*uc_idx);
        oal_int8 c_mult10 = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND_START+2*uc_idx+1);
        if(c_mult4 >= RF_LINE_TXRX_GAIN_DB_2G_MIN && c_mult4 <= 0 &&
           c_mult10 >= RF_LINE_TXRX_GAIN_DB_2G_MIN && c_mult10 <= 0 && !uc_error_param)
        {
            pst_customize_rf->ac_gain_db_2g[uc_idx].c_rf_gain_db_mult4 = c_mult4;
            pst_customize_rf->ac_gain_db_2g[uc_idx].c_rf_gain_db_mult10 = c_mult10;
        }
        else
        {
            /* ֵ������Ч��Χ�������ΪTRUE */
            uc_error_param = OAL_TRUE;
            OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::[ini]value out of range, config id range[%d,%d], check these values in ini file!}\r\n",
                            WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND_START+2*uc_idx, WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND_START+2*uc_idx+1);
        }
    }
    if (band_5g_enabled)
    {
        /* ����: 5g rf */
        /* ����: fem�ڵ����߿ڵĸ����� */
        for (uc_idx=0; uc_idx<MAC_NUM_5G_BAND; ++uc_idx)
        {
            /* ��ȡ��5g ��band 0.25db��0.1db���ȵ�����ֵ */
            oal_int8 c_mult4 = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND_START+2*uc_idx);
            oal_int8 c_mult10 = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND_START+2*uc_idx+1);

            if(c_mult4 <= 0 && c_mult10 <= 0)
            {
                pst_customize_rf->ac_gain_db_5g[uc_idx].c_rf_gain_db_mult4 = c_mult4;
                pst_customize_rf->ac_gain_db_5g[uc_idx].c_rf_gain_db_mult10 = c_mult10;
            }
            else
            {
                /* ֵ������Ч��Χ�������ΪTRUE */
                uc_error_param = OAL_TRUE;
                OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::[ini]value out of range, config id range[%d,%d], check these values in ini file!}\r\n",
                                WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND_START+2*uc_idx, WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND_START+2*uc_idx+1);
            }
        }

        pst_customize_rf->c_rf_line_rx_gain_db_5g       = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_LINE_RX_GAIN_DB_5G);
        pst_customize_rf->c_lna_gain_db_5g              = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LNA_GAIN_DB_5G);
        pst_customize_rf->c_rf_line_tx_gain_db_5g       = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_RF_LINE_TX_GAIN_DB_5G);
        pst_customize_rf->uc_ext_switch_isexist_5g      = (oal_uint8)!!hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_5G);
        pst_customize_rf->uc_ext_pa_isexist_5g          = (oal_uint8)!!hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_PA_ISEXIST_5G);
        pst_customize_rf->uc_ext_lna_isexist_5g         = (oal_uint8)!!hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_EXT_LNA_ISEXIST_5G);
        pst_customize_rf->us_lna_on2off_time_ns_5g      = (oal_uint16)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_5G);
        pst_customize_rf->us_lna_off2on_time_ns_5g      = (oal_uint16)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_5G);
        if (!((pst_customize_rf->c_rf_line_rx_gain_db_5g >= RF_LINE_TXRX_GAIN_DB_5G_MIN && pst_customize_rf->c_rf_line_rx_gain_db_5g <= 0) &&
            (pst_customize_rf->c_rf_line_tx_gain_db_5g >= RF_LINE_TXRX_GAIN_DB_5G_MIN && pst_customize_rf->c_rf_line_tx_gain_db_5g <= 0) &&
            (pst_customize_rf->c_lna_gain_db_5g >= LNA_GAIN_DB_MIN && pst_customize_rf->c_lna_gain_db_5g <= LNA_GAIN_DB_MAX)))
        {
            OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::cfg_id_range[%d,%d] value out of range, please check these values!}\r\n",
                            WLAN_CFG_INIT_RF_LINE_RX_GAIN_DB_5G, WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_5G);
            /* ֵ������Ч��Χ�������ΪTRUE */
            uc_error_param = OAL_TRUE;
        }
    }
    pst_customize_rf->uc_far_dist_pow_gain_switch = (oal_uint8)!!hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_FAR_DIST_POW_GAIN_SWITCH);
    pst_customize_rf->uc_far_dist_dsss_scale_promote_switch = (oal_uint8)!!hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_FAR_DIST_DSSS_SCALE_PROMOTE_SWITCH);

    /* ����: cca�������޵���ֵ */
    {
        oal_int8 c_delta_cca_ed_high_20th_2g = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_2G);
        oal_int8 c_delta_cca_ed_high_40th_2g = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_2G);
        oal_int8 c_delta_cca_ed_high_20th_5g = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_5G);
        oal_int8 c_delta_cca_ed_high_40th_5g = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_5G);

        /* ���ÿһ��ĵ��������Ƿ񳬳�������� */
        if (CUS_DELTA_CCA_ED_HIGH_TH_OUT_OF_RANGE(c_delta_cca_ed_high_20th_2g) ||
            CUS_DELTA_CCA_ED_HIGH_TH_OUT_OF_RANGE(c_delta_cca_ed_high_40th_2g) ||
            CUS_DELTA_CCA_ED_HIGH_TH_OUT_OF_RANGE(c_delta_cca_ed_high_20th_5g) ||
            CUS_DELTA_CCA_ED_HIGH_TH_OUT_OF_RANGE(c_delta_cca_ed_high_40th_5g))
        {
            OAM_ERROR_LOG4(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::one or more delta cca ed high threshold out of range \
                                            [delta_20th_2g=%d, delta_40th_2g=%d, delta_20th_5g=%d, delta_40th_5g=%d], please check the value!}\r\n",
                                            c_delta_cca_ed_high_20th_2g,
                                            c_delta_cca_ed_high_40th_2g,
                                            c_delta_cca_ed_high_20th_5g,
                                            c_delta_cca_ed_high_40th_5g);
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

    pst_customize_rf->c_delta_pwr_ref_2g_20m = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_PWR_REF_2G_20M);
    pst_customize_rf->c_delta_pwr_ref_2g_40m = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_PWR_REF_2G_40M);
    pst_customize_rf->c_delta_pwr_ref_5g_20m = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_PWR_REF_5G_20M);
    pst_customize_rf->c_delta_pwr_ref_5g_40m = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_PWR_REF_5G_40M);
    pst_customize_rf->c_delta_pwr_ref_5g_80m = (oal_int8)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DELTA_PWR_REF_5G_80M);

    /* ��������������в���ȷ�ģ�ֱ�ӷ��� */
    if (uc_error_param)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::one or more params have wrong value, do not send cfg event!}\r\n");
        return;
    }
    /* ������в���������Ч��Χ�ڣ����·�����ֵ */
    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(wlan_cfg_customize_rf),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::EVENT[wal_send_cfg_event] failed, return err code [%d]!}\r\n", l_ret);
    }
}
#ifdef _PRE_WLAN_DOWNLOAD_PM
OAL_STATIC oal_void hwifi_config_init_ini_download_pm(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru          st_write_msg;
    oal_uint16                  us_download_rate_limit_pps;
    oal_uint32                  l_ret;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_DOWNLOAD_RATE_LIMIT, OAL_SIZEOF(us_download_rate_limit_pps));
    us_download_rate_limit_pps = (oal_uint16)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DOWNLOAD_RATE_LIMIT_PPS);
    *(oal_uint16*)st_write_msg.auc_value = us_download_rate_limit_pps;

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint16),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini_download_pm::return err code [%d]!}\r\n", l_ret);
    }
}
#endif

oal_int32 hwifi_config_init_ini_priv(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru          st_write_msg;
    oal_uint32                  l_ret;
    wlan_cfg_customize_priv*     pst_customize_priv;
    oal_int32                   l_priv_value;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_PRIV, OAL_SIZEOF(wlan_cfg_customize_priv));
    pst_customize_priv = (wlan_cfg_customize_priv*)(st_write_msg.auc_value);
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_CALI_MASK, &l_priv_value);
    if (OAL_SUCC == l_ret)
    {
        pst_customize_priv->us_cali_mask = (oal_uint16)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_TEMP_PRO_THRESHOLD, &l_priv_value);
    if (OAL_SUCC == l_ret)
    {
        pst_customize_priv->ul_temp_pro_threshold = (oal_uint32)l_priv_value;
    }
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_FAST_CHECK_CNT, &l_priv_value);
    if (OAL_SUCC == l_ret)
    {
        pst_customize_priv->uc_fast_check_cnt = (oal_uint8)l_priv_value;
        g_wlan_fast_check_cnt = (l_priv_value > 0) ? (oal_uint8)l_priv_value : HMAC_PSM_TIMER_MIDIUM_CNT;
    }
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_VOE_SWITCH, &l_priv_value);
    if (OAL_SUCC == l_ret)
    {
        pst_customize_priv->uc_voe_switch = (oal_uint8)l_priv_value;
        g_st_mac_voe_custom_param.en_11k = (l_priv_value & BIT0) ? OAL_TRUE : OAL_FALSE;
        g_st_mac_voe_custom_param.en_11v = (l_priv_value & BIT1) ? OAL_TRUE : OAL_FALSE;
        g_st_mac_voe_custom_param.en_11r = (l_priv_value & BIT2) ? OAL_TRUE : OAL_FALSE;
    }
#ifdef _PRE_WLAN_FEATURE_TEMP_PROTECT
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_TEMP_PRO_EN, &l_priv_value);
    if (OAL_SUCC == l_ret)
    {
        pst_customize_priv->st_temp_pri_custom.uc_temp_pro_enable = (oal_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_TEMP_PRO_REDUCE_PWR_EN, &l_priv_value);
    if (OAL_SUCC == l_ret)
    {
        pst_customize_priv->st_temp_pri_custom.uc_temp_pro_reduce_pwr_enable = (oal_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_TEMP_PRO_SAFE_TH, &l_priv_value);
    if (OAL_SUCC == l_ret)
    {
        pst_customize_priv->st_temp_pri_custom.us_temp_pro_safe_th = (oal_uint16)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_TEMP_PRO_OVER_TH, &l_priv_value);
    if (OAL_SUCC == l_ret)
    {
        pst_customize_priv->st_temp_pri_custom.us_temp_pro_over_th = (oal_uint16)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_TEMP_PRO_PA_OFF_TH, &l_priv_value);
    if (OAL_SUCC == l_ret)
    {
        pst_customize_priv->st_temp_pri_custom.us_temp_pro_pa_off_th = (oal_uint16)l_priv_value;
    }
#endif
    l_ret = hwifi_get_init_priv_value(WLAN_DSSS2OFDM_DBB_PWR_BO_VAL, &l_priv_value);
    if (OAL_SUCC == l_ret)
    {
        pst_customize_priv->s_dsss2ofdm_dbb_pwr_bo_val = (oal_int16)l_priv_value;
    }
#ifdef _PRE_WLAN_FEATURE_DYN_BYPASS_EXTLNA
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_DYN_BYPASS_EXTLNA, &l_priv_value);
    if (OAL_SUCC == l_ret)
    {
        pst_customize_priv->uc_dyn_bypass_extlna_enable = (oal_uint8) l_priv_value;
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "hwifi_config_init_ini_priv: EXTLNA[%d]", pst_customize_priv->uc_dyn_bypass_extlna_enable);
    }
#endif
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_HCC_FLOWCTRL_TYPE, &l_priv_value);
    if (OAL_SUCC == l_ret)
    {
        pst_customize_priv->uc_hcc_flowctrl_type = (oal_uint8) l_priv_value;
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "hwifi_config_init_ini_priv: Non high priority flowctrl type[%d]", pst_customize_priv->uc_hcc_flowctrl_type);
    }
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_5G_EXT_FEM_TYPE, &l_priv_value);
    if (OAL_SUCC == l_ret)
    {
        pst_customize_priv->uc_5g_ext_fem_type = (oal_uint8)l_priv_value;
    }

    /* ������в���������Ч��Χ�ڣ����·�����ֵ */
    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(wlan_cfg_customize_priv),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::EVENT[wal_send_cfg_event] failed, return err code [%d]!}\r\n", l_ret);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
extern oal_bool_enum_uint8 g_en_fact_cali_completed;
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI

OAL_STATIC oal_uint32 hwifi_cfg_init_cus_dyn_cali(wlan_cus_dy_cali_param_stru *puc_dyn_cali_param)
{
    oal_int32      l_val;
    oal_uint8      uc_idx    = 0;
    oal_uint8      uc_dy_cal_param_idx;
    oal_uint8      uc_cfg_id        = WLAN_CFG_DTS_2G_CORE0_DPN_CH1;
    oal_uint8      uc_dpn_2g_nv_id  = WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0;
    oal_int8       ac_dpn_nv[HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_CW][MAC_2G_CHANNEL_NUM];
    oal_uint8      uc_num_idx;
    oal_uint8     *puc_cust_nvram_info;
    oal_uint8     *pc_end = ";";
    oal_uint8     *pc_sep = ",";
    oal_int8      *pc_ctx;
    oal_int8      *pc_token;
    oal_uint8      auc_nv_pa_params[DY_CALI_PARAMS_LEN] = {0};
    oal_uint8      uc_gm_opt;

    /* ��̬У׼������ϵ����μ�� */
    for (uc_dy_cal_param_idx = 0; uc_dy_cal_param_idx < DY_CALI_PARAMS_NUM; uc_dy_cal_param_idx++)
    {
        if (!g_as_pro_line_params[uc_dy_cal_param_idx].l_pow_par2)
        {
            OAM_ERROR_LOG0(0, OAM_SF_CUSTOM, "{hwifi_cfg_init_cus_dyn_cali::unexpected val l_pow_par2[0] check nv and ini file!}");
            return OAL_FAIL;
        }
    }
    oal_memcopy(puc_dyn_cali_param->al_dy_cali_base_ratio_params, g_as_pro_line_params,
                OAL_SIZEOF(puc_dyn_cali_param->al_dy_cali_base_ratio_params));
    oal_memcopy(puc_dyn_cali_param->al_dy_cali_base_ratio_ppa_params, &g_as_pro_line_params[CUS_DY_CALI_PARAMS_NUM],
                OAL_SIZEOF(puc_dyn_cali_param->al_dy_cali_base_ratio_ppa_params));
    oal_memcopy(puc_dyn_cali_param->as_extre_point_val, gs_extre_point_vals,
                OAL_SIZEOF(puc_dyn_cali_param->as_extre_point_val));
    // BT power fit params
    oal_memcopy(puc_dyn_cali_param->al_bt_power_fit_params, &g_as_pro_line_params[CUS_DY_CALI_PARAMS_NUM + 1],
                OAL_SIZEOF(puc_dyn_cali_param->al_bt_power_fit_params));
    // BT PPA Vdet fit params
    oal_memcopy(puc_dyn_cali_param->al_bt_ppavdet_fit_params, &g_as_pro_line_params[CUS_DY_CALI_PARAMS_NUM + 2],
                OAL_SIZEOF(puc_dyn_cali_param->al_bt_ppavdet_fit_params));

    /* DPN */
    for (uc_idx = 0; uc_idx < MAC_2G_CHANNEL_NUM; uc_idx++)
    {
        l_val = hwifi_get_init_value(CUS_TAG_DTS, uc_cfg_id + uc_idx);
        oal_memcopy(puc_dyn_cali_param->ac_dy_cali_2g_dpn_params[uc_idx], &l_val, DY_CALI_DPN_PARAMS_NUM*OAL_SIZEOF(oal_int8));
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
            OAM_ERROR_LOG1(0, OAM_SF_CUSTOM, "{hwifi_cfg_init_cus_dyn_cali::nvram 2g dpn num is unexpected uc_id[%d]!}",
                           uc_idx);
            continue;
        }

        for (uc_num_idx = 0; uc_num_idx < MAC_2G_CHANNEL_NUM; uc_num_idx++)
        {
            puc_dyn_cali_param->ac_dy_cali_2g_dpn_params[uc_num_idx][uc_idx] += ac_dpn_nv[uc_idx][uc_num_idx];
        }
    }

    for (uc_idx = 0; uc_idx < CUS_DY_CALI_NUM_5G_BAND; uc_idx++)
    {
        l_val = hwifi_get_init_value(CUS_TAG_DTS, uc_cfg_id + uc_idx);
        oal_memcopy(puc_dyn_cali_param->ac_dy_cali_5g_dpn_params[uc_idx], &l_val, CUS_DY_CALI_DPN_PARAMS_NUM*OAL_SIZEOF(oal_int8));
    }
    uc_cfg_id += DY_CALI_NUM_5G_BAND;

    l_val = hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_DYN_CALI_DSCR_ITERVL);
    puc_dyn_cali_param->aus_dyn_cali_dscr_interval[WLAN_BAND_2G] = (oal_uint16)(l_val & 0x0000FFFF);

    if (band_5g_enabled)
    {
        puc_dyn_cali_param->aus_dyn_cali_dscr_interval[WLAN_BAND_5G] = (oal_uint16)((l_val & 0xFFFF0000) >> 16);
    }

    /* en tx power pdebuf opt */
    l_val = hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_DYN_CALI_OPT_SWITCH);
    uc_gm_opt = (l_val & BIT2) >> NUM_1_BITS;

    if ((l_val & 0x3) >> 1)
    {
        /* ����Ӧѡ������в���У׼���Ͳ���gm���������û�в���У׼������gm���� */
        l_val = !g_en_fact_cali_completed;
    }
    else
    {
        /* ��ȡ�·�ֵ */
        l_val &= BIT0;
    }

    puc_dyn_cali_param->uc_tx_power_pdbuf_opt = ((oal_uint8)l_val) | uc_gm_opt;

    /* g_as_gm0_dB10 */
    l_val = hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_DYN_CALI_GM0_DB10_AMEND);
    puc_dyn_cali_param->s_gm0_dB10 = (oal_int16)l_val;

    /* 5G����У׼UPC����ֵ�����ƻ��·� */
    l_val = hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_5G_UPC_UP_LIMIT);
    puc_dyn_cali_param->uc_5g_upc_upper_limit = (oal_uint8)l_val;

    /* 5G IQУ׼���˹��ʣ����ƻ��·� */
    l_val = hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_5G_IQ_BACKOFF_POWER);
    puc_dyn_cali_param->c_5g_iq_cali_backoff_pow = (oal_int8)l_val;

    /* 5G IQУ׼�������ʣ����ƻ��·� */
    l_val = hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_5G_IQ_CALI_POWER);
    puc_dyn_cali_param->us_5g_iq_cali_pow = (oal_uint16)l_val;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hwifi_config_init_cus_dyn_cali(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_ret;
    oal_uint32                      ul_offset = 0;
    wlan_cus_dy_cali_param_stru     st_dy_cus_cali;

    if (OAL_WARN_ON(OAL_PTR_NULL == pst_cfg_net_dev))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ���ö�̬У׼����TXPWR_PA_DC_REF */
    OAL_MEMZERO(&st_dy_cus_cali, OAL_SIZEOF(wlan_cus_dy_cali_param_stru));
    ul_ret = hwifi_cfg_init_cus_dyn_cali(&st_dy_cus_cali);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hwifi_config_init_cus_dyn_cali::init cus dyn cali failed ret[%d]!}", ul_ret);
        return ul_ret;
    }

    /* ������в���������Ч��Χ�ڣ����·�����ֵ */
    oal_memcopy(st_write_msg.auc_value, (oal_int8*)&st_dy_cus_cali, OAL_SIZEOF(st_dy_cus_cali));
    ul_offset = OAL_SIZEOF(st_dy_cus_cali);
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_DYN_CALI_PARAM, ul_offset);
    ul_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + ul_offset,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_cus_dyn_cali::wal_send_cfg_event failed, ret[%d]!}", ul_ret);
        return ul_ret;
    }
    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32 hwifi_config_init_dts_cali(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;
    wlan_cfg_customize_rf_reg   st_rf_reg;
    wlan_cus_cali_stru          st_cus_cali;
    oal_uint32                  ul_cfg_id;
    oal_int8                    c_cfg_value;                 /* ��ʱ��������Ӷ��ƻ�get�ӿ��л�ȡ��ֵ */
    oal_uint8                   uc_error_param = OAL_FALSE;    /* ������Ч�Ա�־���������ֵ���Ϸ�����Ϊ1�����в������·� */
    oal_uint8                   uc_idx         = 0;             /* �ṹ�������±� */
    oal_uint32                  ul_offset      = 0;
    oal_uint16                 *pus_rf_reg;             /* ָ��rf���õĵ�һ���Ĵ��� */
#ifdef _PRE_BT_FITTING_DATA_COLLECT
    oal_int32                   l_params[CUS_BT_FREQ_NUM] = {0};
#endif

    /** ����: TXPWR_PA_DC_REF **/
    /*2G REF: ��13���ŵ�*/
    for (uc_idx=0; uc_idx<13; ++uc_idx)
    {
        oal_int16 s_ref_val = (oal_int16)hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_START+uc_idx);
        if (s_ref_val >= 0 && s_ref_val <= CALI_TXPWR_PA_DC_REF_MAX)
        {
            st_cus_cali.aus_cali_txpwr_pa_dc_ref_2g_val[uc_idx] = s_ref_val;
        }
        else
        {
            /* ֵ������Ч��Χ�������ΪTRUE */
            uc_error_param = OAL_TRUE;
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_dts_cali::[dts]2g ref value out of range, config id[%d], check the value in dts file!}\r\n",
                            WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_START+uc_idx);
        }
    }
    /*5G REF: ��7��band*/
    if (band_5g_enabled)
    {
        oal_int16* ps_ref_5g = &st_cus_cali.us_cali_txpwr_pa_dc_ref_5g_val_band1;
        for (uc_idx=0; uc_idx<7; ++uc_idx)
        {
            oal_int16 s_ref_val = (oal_int16)hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_START+uc_idx);
            if (s_ref_val >= 0 && s_ref_val <= CALI_TXPWR_PA_DC_REF_MAX)
            {
                *(ps_ref_5g+uc_idx) = s_ref_val;/* [false alarm]:mac_cus_dts_cali_stru�ṹ���Ա������Զ����������ָ������ֵ����ָ�벻��Խ��*/
            }
            else
            {
                /* ֵ������Ч��Χ�������ΪTRUE */
                uc_error_param = OAL_TRUE;
                OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_dts_cali::[dts]5g ref value out of range, config id[%d], check the value in dts file!}\r\n",
                                WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_START+uc_idx);
            }
        }
    }

    /** ����: BAND 5G ENABLE **/
    st_cus_cali.uc_band_5g_enable = !!band_5g_enabled;

    /** ����: �������ȵ�λ **/
    st_cus_cali.uc_tone_amp_grade = (oal_uint8)hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_CALI_TONE_AMP_GRADE);


#ifdef _PRE_BT_FITTING_DATA_COLLECT
    // BT DPN values for Tx Power calibration
    uc_error_param = hwifi_config_get_bt_dpn(l_params, OAL_SIZEOF(l_params));
    for (uc_idx = 0; uc_idx < CUS_BT_FREQ_NUM; uc_idx++)
    {
        if (CALI_TXPWR_DPN_MIN <= l_params[uc_idx] && CALI_TXPWR_DPN_MAX >= l_params[uc_idx])
        {
            st_cus_cali.ac_cali_bt_txpwr_dpn_params[uc_idx] = l_params[uc_idx];
        }
        else
        {
            /* ֵ������Ч��Χ�������ΪTRUE */
            uc_error_param = OAL_TRUE;
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_dts_cali::[dts]value out of range, config id[%d], check the value in dts file!}\r\n",
                            WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND1 + uc_idx / 10);
        }
    }
#endif
    // BT frequency for Tx Power calibration
    st_cus_cali.s_cali_bt_txpwr_num = (oal_int16)hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_BT_CALI_TXPWR_NUM);
    ul_cfg_id = WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ1;
    for (uc_idx = 0; uc_idx < CUS_BT_TXPWR_FREQ_NUM_MAX; uc_idx++)
    {
        c_cfg_value = (oal_int8)hwifi_get_init_value(CUS_TAG_DTS, ul_cfg_id + uc_idx);
        if (CALI_TXPWR_FREQ_MIN <= c_cfg_value && CALI_TXPWR_FREQ_MAX >= c_cfg_value)
        {
            st_cus_cali.auc_cali_bt_txpwr_freq[uc_idx] = (oal_uint8)c_cfg_value;
        }
        else
        {
            /* ֵ������Ч��Χ�������ΪTRUE */
            uc_error_param = OAL_TRUE;
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_dts_cali::[dts]value out of range, config id[%d], check the value in dts file!}\r\n", ul_cfg_id + uc_idx);
        }
    }

    // BT Gm calibration related
    st_cus_cali.uc_bt_gm_cali_en = (oal_uint8)hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_BT_CALI_GM_CALI_EN);
    st_cus_cali.s_bt_gm0_dB10 = (oal_int16)hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_BT_CALI_GM0_DB10);
    st_cus_cali.uc_bt_insertion_loss = (oal_uint8)hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_BT_CALI_INSERTION_LOSS);
    // BT base power
    st_cus_cali.uc_bt_base_power = (oal_int16)hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_BT_CALI_BASE_POWER);
    // BT is DPN calculating
    st_cus_cali.uc_bt_is_dpn_calc = (oal_int16)hwifi_get_init_value(CUS_TAG_DTS, WLAN_CFG_DTS_BT_CALI_IS_DPN_CALC);

    /** ����: RF�Ĵ��� **/
    pus_rf_reg = (oal_uint16*)&st_rf_reg;
    for(ul_cfg_id = WLAN_CFG_DTS_RF_FIRST; ul_cfg_id <= WLAN_CFG_DTS_RF_LAST; ++ul_cfg_id)
    {
        oal_uint16 us_reg_val = (oal_uint16)hwifi_get_init_value(CUS_TAG_DTS, ul_cfg_id);
        *(pus_rf_reg+ul_cfg_id-WLAN_CFG_DTS_RF_FIRST) = us_reg_val;
    }

    /* ��������������в���ȷ�ģ�ֱ�ӷ��� */
    if (uc_error_param)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::one or more params have wrong value, do not send cfg event!}\r\n");
        /* �ͷ�pst_band_edge_limit�ڴ� */
        return OAL_FAIL;
    }
    /* ������в���������Ч��Χ�ڣ����·�����ֵ */
    oal_memcopy(st_write_msg.auc_value, (oal_int8*)&st_cus_cali, OAL_SIZEOF(wlan_cus_cali_stru));
    ul_offset += OAL_SIZEOF(wlan_cus_cali_stru);
    oal_memcopy(st_write_msg.auc_value + ul_offset, (oal_int8*)&st_rf_reg, OAL_SIZEOF(wlan_cfg_customize_rf_reg));
    ul_offset += OAL_SIZEOF(wlan_cfg_customize_rf_reg);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_DTS_CALI, ul_offset);
    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + ul_offset,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_dts_cali::wal_send_cfg_event failed, error no[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
    return OAL_SUCC;
}

OAL_STATIC oal_void hwifi_config_init_ini_main(oal_net_device_stru *pst_cfg_net_dev)
{
    /* ���� */
    hwifi_config_init_ini_perf(pst_cfg_net_dev);
    /* LINKLOSS */
    hwifi_config_init_ini_linkloss(pst_cfg_net_dev);
    /* ������ */
    hwifi_config_init_ini_country(pst_cfg_net_dev);
    /* �͹��� */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    hwifi_config_init_ini_pm(pst_cfg_net_dev);
#endif
    /* ��ά�ɲ� */
    hwifi_config_init_ini_log(pst_cfg_net_dev);
    /* RF */
    hwifi_config_init_ini_rf(pst_cfg_net_dev);
#ifdef _PRE_WLAN_FEATURE_SMARTANT
    hwifi_config_init_ini_dual_antenna(pst_cfg_net_dev);
#endif
#ifdef _PRE_WLAN_DOWNLOAD_PM
    hwifi_config_init_ini_download_pm(pst_cfg_net_dev);
#endif
}

oal_bool_enum hwifi_config_init_nvram_main(oal_net_device_stru *pst_cfg_net_dev)
{
    wal_msg_write_stru                 st_write_msg;
    wlan_customize_power_params_stru  *pst_nvram;
    oal_int32                          l_ret;
    oal_uint32                         ul_offset = NUM_OF_NV_MAX_TXPOWER * OAL_SIZEOF(oal_uint8);

    pst_nvram = hwifi_get_nvram_params();

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CUS_NVRAM_PARAM, ul_offset);
    oal_memcopy(st_write_msg.auc_value, pst_nvram->ac_delt_txpower, ul_offset);

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + ul_offset,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_nvram_main::return err code [%d]!}\r\n", l_ret);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

oal_uint32 hwifi_config_init_dts_main(oal_net_device_stru *pst_cfg_net_dev)
{
    /* �·�base power */
    hwifi_config_init_base_power_main(pst_cfg_net_dev);
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    /* �·���̬У׼���� */
    hwifi_config_init_cus_dyn_cali(pst_cfg_net_dev);
#endif
    /* У׼ */
    return hwifi_config_init_dts_cali(pst_cfg_net_dev);
    /* У׼�ŵ���һ������ */
}

OAL_STATIC oal_int32 hwifi_config_init_ini_wlan(oal_net_device_stru *pst_net_dev)
{
    return OAL_SUCC;
}

OAL_STATIC oal_int32 hwifi_config_init_ini_p2p(oal_net_device_stru *pst_net_dev)
{
    return OAL_SUCC;
}

oal_int32 hwifi_config_init_ini(oal_net_device_stru *pst_net_dev)
{
    oal_net_device_stru        *pst_cfg_net_dev;
    mac_vap_stru               *pst_mac_vap;
    oal_wireless_dev_stru      *pst_wdev;
    mac_wiphy_priv_stru        *pst_wiphy_priv;
    mac_vap_stru               *pst_cfg_mac_vap;
    hmac_vap_stru              *pst_cfg_hmac_vap;
    mac_device_stru            *pst_mac_device;

    if(OAL_PTR_NULL== pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini::pst_net_dev is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_PTR_NULL== pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini::pst_mac_vap is null}\r\n");
        return -OAL_EINVAL;
    }

    pst_wdev = OAL_NETDEVICE_WDEV(pst_net_dev);
    if(OAL_PTR_NULL == pst_wdev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini::pst_wdev is null!}\r\n");
        return -OAL_EFAUL;
    }

    pst_wiphy_priv  = (mac_wiphy_priv_stru *)oal_wiphy_priv(pst_wdev->wiphy);
    if (OAL_PTR_NULL == pst_wiphy_priv)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini::pst_wiphy_priv is null!}\r\n");
        return -OAL_EFAUL;
    }

    pst_mac_device  = pst_wiphy_priv->pst_mac_device;
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini::pst_mac_device is null!}\r\n");
        return -OAL_EFAUL;
    }

    pst_cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->uc_cfg_vap_id);
    if (OAL_PTR_NULL == pst_cfg_mac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini::pst_cfg_mac_vap is null, vap_id:%d!}\r\n",pst_mac_device->uc_cfg_vap_id);
        return -OAL_EFAUL;
    }
    pst_cfg_hmac_vap= (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->uc_cfg_vap_id);
    if (OAL_PTR_NULL == pst_cfg_hmac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hwifi_config_init_ini::pst_cfg_hmac_vap is null, vap_id:%d!}\r\n",pst_mac_device->uc_cfg_vap_id);
        return -OAL_EFAUL;
    }


    pst_cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;

    if(OAL_PTR_NULL == pst_cfg_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini::pst_cfg_net_dev is null!}\r\n");
        return -OAL_EFAUL;
    }

    if((NL80211_IFTYPE_STATION == pst_wdev->iftype) || (NL80211_IFTYPE_P2P_DEVICE == pst_wdev->iftype) || (NL80211_IFTYPE_AP == pst_wdev->iftype))
    {
        if(!g_uc_cfg_once_flag)
        {
            hwifi_config_init_nvram_main(pst_net_dev);
            hwifi_config_init_fcc_ce_power_main(pst_net_dev);
            hwifi_config_init_ini_main(pst_cfg_net_dev);
            g_uc_cfg_once_flag = OAL_TRUE;
        }
        if(0 == (oal_strcmp("wlan0", pst_net_dev->name)))
        {
            hwifi_config_init_ini_wlan(pst_net_dev);
        }
#ifdef _PRE_WLAN_FEATURE_P2P
        else if(0 == (oal_strcmp("p2p0", pst_net_dev->name)))
        {
            hwifi_config_init_ini_p2p(pst_net_dev);
        }
#endif
        else
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hwifi_config_init_ini::net_dev is not wlan0 or p2p0!}\r\n");
            return OAL_SUCC;
        }
    }

    return OAL_SUCC;
}

oal_void hwifi_config_init_force(oal_void)
{
    /* �����ϵ�ʱ��ΪFALSE */
    g_uc_cfg_once_flag = OAL_FALSE;
    if(!g_uc_cfg_flag)
    {
        hwifi_config_host_global_ini_param();
        g_uc_cfg_flag = OAL_TRUE;
    }
}

#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

/* E5 SPE module ralation */
#if (defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT))


OAL_STATIC oal_int32 wal_finish_spe_td(oal_int32 l_port_num, oal_netbuf_stru *pst_buf, oal_uint32 ul_flags)
{
    oal_net_device_stru *pst_net_dev;

    if(OAL_WARN_ON(!spe_hook.port_netdev))
    {
         oal_print_hi11xx_log(HI11XX_LOG_ERR,"%s error: spe_hook.port_netdev is null",__FUNCTION__);
         return OAL_FAIL;
    }

    pst_net_dev = spe_hook.port_netdev(l_port_num);

    oal_dma_unmap_single(NULL, pst_buf->dma, pst_buf->len, OAL_TO_DEVICE);

    if (OAL_UNLIKELY(NULL != pst_net_dev) && OAL_UNLIKELY(NULL != pst_buf))
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
    }
    pst_net_dev = spe_hook.port_netdev(l_port_num);

    if(pst_buf->data != phys_to_virt(ul_dma))
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_finish_spe_rd::netbuf->data::%x;dma_addr:%x;!}\r\n", pst_buf->data, phys_to_virt(ul_dma));
        pst_buf->data = phys_to_virt(ul_dma);
    }

    oal_dma_unmap_single(NULL, ul_dma, pst_buf->len, OAL_FROM_DEVICE);

    return wal_vap_start_xmit(pst_buf, pst_net_dev);
}


OAL_STATIC oal_int32 wal_netdev_spe_init(oal_net_device_stru *pst_net_dev)
{
    oal_int32             l_ret = 0;
    oal_uint32            ul_port_num = 0;
    mac_vap_stru          *pst_mac_vap;

    struct spe_port_attr  st_spe_port_attr;

    pst_mac_vap = (mac_vap_stru*)OAL_NET_DEV_PRIV(pst_net_dev);

    OAL_MEMZERO(&st_spe_port_attr, sizeof(struct spe_port_attr));

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

    ul_port_num = spe_hook.port_alloc(&st_spe_port_attr);
    if(ul_port_num > WAL_MAX_SPE_PORT_NUM)
    {
        OAL_IO_PRINT("wal_netdev_spe_init::spe port alloc failed;\n");
        return -OAL_FAIL;
    }
    pst_mac_vap->ul_spe_portnum = ul_port_num;

    l_ret = spe_hook.port_enable(ul_port_num);
    OAL_IO_PRINT("wal_netdev_spe_init::vap_id::%d;port_enable ret::%d;port_num::%d\n", pst_mac_vap->uc_vap_id, l_ret, ul_port_num);

    return l_ret;
}

OAL_STATIC oal_void wal_netdev_spe_exit(oal_net_device_stru *pst_net_dev)
{
    mac_vap_stru          *pst_mac_vap;
    oal_uint32             ul_port_num;
    pst_mac_vap = (mac_vap_stru*)OAL_NET_DEV_PRIV(pst_net_dev);

    ul_port_num = pst_mac_vap->ul_spe_portnum;

    (oal_void)spe_hook.port_disable(ul_port_num);
    (oal_void)spe_hook.port_free(ul_port_num);

    OAL_IO_PRINT("wal_netdev_spe_exit::spe port(%d) disable!\n", ul_port_num);
}
#endif /* defined(CONFIG_BALONG_SPE) && _PRE_WLAN_SPE_SUPPORT */


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
                     g_st_dfr_info.bit_ready_to_recovery_flag, pst_mac_device->uc_vap_num);

    if((OAL_TRUE == g_st_dfr_info.bit_ready_to_recovery_flag)
       && (!pst_mac_device->uc_vap_num))
    {
        /* DFR�ָ�,�ڴ���ҵ��VAPǰ�·�У׼�Ȳ���,ֻ�·�һ�� */
        return OAL_TRUE;
    }

    return OAL_FALSE;
}
#endif /* #ifdef _PRE_WLAN_FEATURE_DFR */
#endif


OAL_STATIC oal_int32  _wal_netdev_open(oal_net_device_stru *pst_net_dev)
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
            oal_int32               ul_check_hw_status = 0;
#endif

    oal_netdev_priv_stru    *pst_netdev_priv;

    OAL_IO_PRINT("wal_netdev_open,dev_name is:%.16s\n", pst_net_dev->name);
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_open::iftype:%d.!}\r\n", pst_net_dev->ieee80211_ptr->iftype);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    g_uc_netdev_is_open = OAL_TRUE;

#ifdef _PRE_WLAN_NARROW_BAND
    if(g_uc_hitalk_status)
    {
        l_ret = hitalk_pm_open();
    }
    else
#endif
    {
         l_ret = wlan_pm_open();
    }

    if (OAL_FAIL == l_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_netdev_open::wlan_pm_open Fail!}\r\n");
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_OPEN, CHR_WIFI_DRV_ERROR_POWER_ON);
        return -OAL_EFAIL;
    }
    else if(OAL_ERR_CODE_ALREADY_OPEN != l_ret)
    {
#ifdef _PRE_WLAN_FEATURE_DFR
        wal_dfr_init_param();
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        /* �����ϵ�ʱ��ΪFALSE */
        hwifi_config_init_force();
#endif
        // �����ϵ糡�����·�����VAP
        l_ret = wal_cfg_vap_h2d_event(pst_net_dev);
        if(OAL_SUCC != l_ret)
        {
            OAL_IO_PRINT("wal_cfg_vap_h2d_event FAIL %d \r\n",l_ret);
            return -OAL_EFAIL;
        }
        OAL_IO_PRINT("wal_cfg_vap_h2d_event succ \r\n");
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    else if (wal_dfr_recovery_check(pst_net_dev))
    {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        g_uc_custom_cali_done = OAL_TRUE;
        wal_priv_init_config();
        wal_custom_cali();
        hwifi_config_init_force();
#endif
        // �����ϵ糡�����·�����VAP
        l_ret = wal_cfg_vap_h2d_event(pst_net_dev);
        if(OAL_SUCC != l_ret)
        {
            OAL_IO_PRINT("DFR:wal_cfg_vap_h2d_event FAIL %d \r\n",l_ret);
            return -OAL_EFAIL;
        }
        OAL_IO_PRINT("DFR:wal_cfg_vap_h2d_event succ \r\n");
    }
#endif  /* #ifdef _PRE_WLAN_FEATURE_DFR */

#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)

    if(OAL_TRUE == g_st_ap_config_info.l_ap_power_flag)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_netdev_open::power state is on,in ap mode, start vap later.}\r\n");

        /* �˱�����ʱ��һ�Σ���ֹ Android framework����ģʽ�л�ǰ�·�����up���� */
        g_st_ap_config_info.l_ap_power_flag = OAL_FALSE;
        oal_net_tx_wake_all_queues(pst_net_dev);/*�������Ͷ��� */
        return OAL_SUCC;
    }

    /* �ϵ�host device_stru��ʼ��*/
    l_ret = wal_host_dev_init(pst_net_dev);
    if(OAL_SUCC != l_ret)
    {
        OAL_IO_PRINT("wal_host_dev_init FAIL %d \r\n",l_ret);
        return -OAL_EFAIL;
    }
    if (((NL80211_IFTYPE_STATION == pst_net_dev->ieee80211_ptr->iftype) || (NL80211_IFTYPE_P2P_DEVICE == pst_net_dev->ieee80211_ptr->iftype))
         &&((0 == (oal_strcmp("wlan0", pst_net_dev->name))) || (0 == (oal_strcmp("p2p0", pst_net_dev->name)))))
    {
        l_ret = wal_init_wlan_vap(pst_net_dev);
        if(OAL_SUCC != l_ret)
        {
		#ifdef _PRE_WLAN_1102A_CHR
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_OPEN, CHR_WIFI_DRV_ERROR_POWER_ON);
		#endif
			return -OAL_EFAIL;
        }
    }
    else if(NL80211_IFTYPE_AP == pst_net_dev->ieee80211_ptr->iftype)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_netdev_open::ap mode,no need to start vap.!}\r\n");
        oal_net_tx_wake_all_queues(pst_net_dev);/*�������Ͷ��� */
        return OAL_SUCC;
    }
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hwifi_config_init_ini(pst_net_dev);
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
#ifdef _PRE_WLAN_NARROW_BAND
    if(g_uc_hitalk_status & NBFH_ON_MASK)
    {
        wal_set_nbfh(pst_net_dev);
    }
#endif

    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_START_VAP, OAL_SIZEOF(mac_cfg_start_vap_param_stru));
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wdev    = pst_net_dev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_netdev_open::wal_wireless_iftype_to_mac_p2p_mode return BUFF}\r\n");
	#ifdef _PRE_WLAN_1102A_CHR
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_OPEN, CHR_WIFI_DRV_ERROR_POWER_ON);
	#endif
		return -OAL_EINVAL;
    }
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;
#endif
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_mgmt_rate_init_flag = OAL_TRUE;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
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
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_open::wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_open::hmac start vap fail,err code[%u]!}\r\n", ul_err_code);
        return -OAL_EINVAL;
    }

    if (0 == (OAL_NETDEVICE_FLAGS(pst_net_dev) & OAL_IFF_RUNNING))
    {
        OAL_NETDEVICE_FLAGS(pst_net_dev) |= OAL_IFF_RUNNING;
    }

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
        wal_atcmsrv_ioctl_get_fem_pa_status(pst_net_dev,&ul_check_hw_status);
    }
#endif

    return OAL_SUCC;
}

oal_int32  wal_netdev_open(oal_net_device_stru *pst_net_dev)
{
    oal_int32 ret;

    if (OAL_PTR_NULL == pst_net_dev)
    {
        return -OAL_EFAUL;
    }

    if (OAL_NETDEVICE_FLAGS(pst_net_dev) & OAL_IFF_RUNNING)
    {
        return OAL_SUCC;
    }

    wal_wake_lock();
    ret = _wal_netdev_open(pst_net_dev);
    wal_wake_unlock();

    if (OAL_SUCC != ret)
    {
        CHR_EXCEPTION(CHR_WIFI_DRV(CHR_WIFI_DRV_EVENT_OPEN,CHR_WIFI_DRV_ERROR_POWER_ON));
    }

#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && defined(_PRE_E5_722_PLATFORM)
    if (reg_at_hipriv_entry(wal_atcmdsrv_wifi_priv_cmd))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_net_device_ioctl:: register at hipriv entry failed}\r\n");
    }
    //OAL_IO_PRINT("wangweigang::wal_netdev_open:reg_at_hipriv_entry.\n");
#endif

#if (defined(_PRE_PRODUCT_ID_HI110X_HOST) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /* ��¼wlan0 ����ʱ�� */
    if (oal_strncmp("wlan0", pst_net_dev->name, OAL_STRLEN("wlan0")) == 0)
    {
        g_st_wifi_radio_stat.ull_wifi_on_time_stamp = OAL_TIME_JIFFY;
    }
#endif
    return ret;
}
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE


oal_uint32 wal_custom_cali(oal_void)
{
    oal_net_device_stru *pst_net_dev;

    pst_net_dev = oal_dev_get_by_name("Hisilicon0");
    if (OAL_PTR_NULL != pst_net_dev)
    {
        /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
        oal_dev_put(pst_net_dev);

        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hisi_customize_wifi host_module_init::the net_device is already exist!}\r\n");
    }
    else
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hisi_customize_wifi host_module_init::Hisilicon0 do NOT exist!}\r\n");
        return OAL_FAIL;
    }

    if (g_uc_custom_cali_done == OAL_TRUE)
    {
        /* У׼�����·� */
        wal_send_cali_data(pst_net_dev);
    }
    else
    {
        g_uc_custom_cali_done = OAL_TRUE;
    }

    wal_send_cali_matrix_data(pst_net_dev);

    /* �·����� */
    return hwifi_config_init_dts_main(pst_net_dev);
}


oal_uint32 wal_priv_init_config(oal_void)
{
    oal_net_device_stru     *pst_net_dev = oal_dev_get_by_name("Hisilicon0");
    if (OAL_PTR_NULL != pst_net_dev)
    {
        /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
        oal_dev_put(pst_net_dev);
    }
    else
    {
        OAM_ERROR_LOG0(0, OAM_SF_CUSTOM, "{hisi_customize_wifi host_module_init::Hisilicon0 do NOT exist!}\r\n");
        return OAL_FAIL;
    }

    return (oal_uint32)hwifi_config_init_ini_priv(pst_net_dev);

}

oal_int32 wal_set_custom_process_func(custom_cali_func p_cus_fun, priv_ini_config_func p_priv_func)
{
    struct custom_process_func_handler* pst_custom_process_func_handler;
    pst_custom_process_func_handler = oal_get_custom_process_func();
    if(OAL_PTR_NULL == pst_custom_process_func_handler)
    {
        OAM_ERROR_LOG0(0,OAM_SF_ANY,"{wal_set_custom_process_func:: get handler failed!}");
    }
    else
    {
        pst_custom_process_func_handler->p_custom_cali_func     = p_cus_fun;
        pst_custom_process_func_handler->p_priv_ini_config_func = p_priv_func;
    }

    return OAL_SUCC;
}
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

OAL_STATIC oal_int32  wal_set_power_on(oal_net_device_stru *pst_net_dev, oal_int32 power_flag)
{
    oal_int32    l_ret = 0;

    //ap���µ磬����VAP
    if (0 == power_flag)//�µ�
    {
        /* �µ�host device_struȥ��ʼ��*/
        wal_host_dev_exit(pst_net_dev);

        wal_wake_lock();
        wlan_pm_close();
        wal_wake_unlock();

        g_st_ap_config_info.l_ap_power_flag = OAL_FALSE;
    }
    else if (1 == power_flag) //�ϵ�
    {
        g_st_ap_config_info.l_ap_power_flag = OAL_TRUE;

        wal_wake_lock();
        l_ret = wlan_pm_open();
        wal_wake_unlock();
        if (OAL_FAIL==l_ret)
        {
             OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_set_power_on::wlan_pm_open Fail!}\r\n");
             return -OAL_EFAIL;
        }
        else if(OAL_ERR_CODE_ALREADY_OPEN != l_ret)
        {
#ifdef _PRE_WLAN_FEATURE_DFR
            wal_dfr_init_param();
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
            /* �����ϵ�ʱ��ΪFALSE */
            hwifi_config_init_force();
#endif
            // �����ϵ糡�����·�����VAP
            l_ret = wal_cfg_vap_h2d_event(pst_net_dev);
            if(OAL_SUCC!= l_ret)
            {
                return -OAL_EFAIL;
            }
        }

        /* �ϵ�host device_stru��ʼ��*/
        l_ret = wal_host_dev_init(pst_net_dev);
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
    pst_wlan_pm = wlan_pm_get_drv();
    if(NULL != pst_wlan_pm)
    {
        /* apģʽ�£��Ƿ������µ����,1:����,0:������ */
        pst_wlan_pm->ul_apmode_allow_pm_flag = power_mgmt_flag;
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_set_power_mgmt_on::wlan_pm_get_drv return null.");
    }

}

#endif /*  (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) */

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)

oal_int32 wal_netdev_stop_ap(oal_net_device_stru *pst_net_dev)
{
    oal_int32          l_ret;
    oal_netdev_priv_stru       *pst_netdev_priv;
    if (NL80211_IFTYPE_AP != pst_net_dev->ieee80211_ptr->iftype)
    {
        return OAL_SUCC;
    }

    /* ����ɨ��,�Է���20/40Mɨ������йر�AP */
    wal_force_scan_complete(pst_net_dev, OAL_TRUE);
    /* AP�ر��л���STAģʽ,ɾ�����vap */
    if(OAL_SUCC != wal_stop_vap(pst_net_dev))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_netdev_stop_ap::wal_stop_vap enter a error.}");
    }
    l_ret = wal_deinit_wlan_vap(pst_net_dev);
    if(OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_netdev_stop_ap::wal_deinit_wlan_vap enter a error.}");
        return l_ret;
    }
    pst_netdev_priv = (oal_netdev_priv_stru *)OAL_NET_DEV_WIRELESS_PRIV(pst_net_dev);

    /* Del aput����Ҫ�л�netdev iftype״̬��station */
    pst_net_dev->ieee80211_ptr->iftype = NL80211_IFTYPE_STATION;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /* aput�µ� */
    wal_set_power_mgmt_on(OAL_TRUE);
    l_ret = wal_set_power_on(pst_net_dev, OAL_FALSE);
    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_netdev_stop_ap::wal_set_power_on fail [%d]!}", l_ret);
        return l_ret;
    }
#endif /* #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) */
    return OAL_SUCC;

}

OAL_STATIC oal_int32  wal_netdev_stop_sta_p2p(oal_net_device_stru *pst_net_dev)
{
    oal_int32                   l_ret;
    mac_vap_stru               *pst_mac_vap;
    mac_device_stru            *pst_mac_device;

    /* wlan0/p2p0 downʱ ɾ��VAP */
    if (((NL80211_IFTYPE_STATION == pst_net_dev->ieee80211_ptr->iftype) || (NL80211_IFTYPE_P2P_DEVICE == pst_net_dev->ieee80211_ptr->iftype))
         &&((0 == (oal_strcmp("wlan0", pst_net_dev->name))) || (0 == (oal_strcmp("p2p0", pst_net_dev->name)))))
    {
#ifdef _PRE_WLAN_FEATURE_P2P
        /* ����ɾ��p2pС�� */
        pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
        if(OAL_PTR_NULL == pst_mac_vap)
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_netdev_stop_sta_p2p::pst_mac_vap is null, netdev released.}\r\n");
            return OAL_SUCC;
        }
        pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
        /* ����ɾ��p2pС�� */
        if(OAL_PTR_NULL != pst_mac_device)
        {
            wal_del_p2p_group(pst_mac_device);
        }
#endif

        l_ret = wal_deinit_wlan_vap(pst_net_dev);
        if(OAL_SUCC!=l_ret)
        {
            return l_ret;
        }

    }
    return OAL_SUCC;
}

#endif /* #if defined(_PRE_PRODUCT_ID_HI110X_HOST) */

OAL_STATIC oal_int32  _wal_netdev_stop(oal_net_device_stru *pst_net_dev)
{
    wal_msg_write_stru          st_write_msg;
    oal_uint32                  ul_err_code;
    wal_msg_stru               *pst_rsp_msg = OAL_PTR_NULL;
    oal_int32                   l_ret;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8    en_p2p_mode;
    oal_wireless_dev_stru      *pst_wdev;
#endif

    oal_netdev_priv_stru       *pst_netdev_priv;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_netdev_stop::pst_net_dev is null ptr!}\r\n");
        return -OAL_EFAUL;
    }

    /*stop the netdev's queues*/
    oal_net_tx_stop_all_queues(pst_net_dev);/* ֹͣ���Ͷ��� */
    wal_force_scan_complete(pst_net_dev, OAL_TRUE);

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_stop::iftype:%d.!}\r\n", pst_net_dev->ieee80211_ptr->iftype);

    OAL_IO_PRINT("wal_netdev_stop,dev_name is:%.16s\n", pst_net_dev->name);

    if(NL80211_IFTYPE_AP == pst_net_dev->ieee80211_ptr->iftype)
    {
        l_ret = wal_netdev_stop_ap(pst_net_dev);
        return l_ret;
    }

#endif

    /* ���netdev����running״̬����ֱ�ӷ��سɹ� */
    if (0 == (OAL_NETDEVICE_FLAGS(pst_net_dev) & OAL_IFF_RUNNING))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_netdev_stop::vap is already down!}\r\n");
        return OAL_SUCC;
    }

    /***************************************************************************
                           ���¼���wal�㴦��
    ***************************************************************************/
    /* ��дWID��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DOWN_VAP, OAL_SIZEOF(mac_cfg_down_vap_param_stru));
    ((mac_cfg_down_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wdev    = pst_net_dev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_netdev_stop::wal_wireless_iftype_to_mac_p2p_mode return BUFF}\r\n");
        return -OAL_EINVAL;
    }
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;
#endif

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
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
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_stop::net_device's vap is null, set flag not running, if_idx:%d}", pst_net_dev->ifindex);
        }
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_stop::wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_stop::hmac stop vap fail!err code [%d]}\r\n", ul_err_code);
        return -OAL_EFAIL;
    }


#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    l_ret = wal_netdev_stop_sta_p2p(pst_net_dev);
    if (OAL_SUCC != l_ret)
    {
        return l_ret;
    }

#endif

    pst_netdev_priv = (oal_netdev_priv_stru *)OAL_NET_DEV_WIRELESS_PRIV(pst_net_dev);
    if (pst_netdev_priv->uc_napi_enable
        && ((NL80211_IFTYPE_STATION == pst_net_dev->ieee80211_ptr->iftype)
        || (NL80211_IFTYPE_P2P_DEVICE == pst_net_dev->ieee80211_ptr->iftype)
        || (NL80211_IFTYPE_P2P_CLIENT == pst_net_dev->ieee80211_ptr->iftype)))
    {
        oal_netbuf_queue_purge(&pst_netdev_priv->st_rx_netbuf_queue);
        oal_napi_disable(&pst_netdev_priv->st_napi);
        pst_netdev_priv->uc_state = 0;
    }

    return OAL_SUCC;
}

oal_int32  wal_netdev_stop(oal_net_device_stru *pst_net_dev)
{
    oal_int32 ret;
    wal_wake_lock();
    ret = _wal_netdev_stop(pst_net_dev);
    wal_wake_unlock();

    return ret;
}


OAL_STATIC oal_net_device_stats_stru*  wal_netdev_get_stats(oal_net_device_stru *pst_net_dev)
{
    oal_net_device_stats_stru  *pst_stats = &(pst_net_dev->stats);
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
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
    oal_sockaddr_stru            *pst_mac_addr = OAL_PTR_NULL;
    frw_event_mem_stru           *pst_event_mem = OAL_PTR_NULL;
    wal_msg_stru                 *pst_cfg_msg = OAL_PTR_NULL;
    wal_msg_write_stru           *pst_write_msg = OAL_PTR_NULL;
    mac_cfg_staion_id_param_stru *pst_param = OAL_PTR_NULL;
    oal_uint32                    ul_ret = 0;
#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
    mac_vap_stru                 *pst_mac_vap = OAL_PTR_NULL;
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_wireless_dev_stru        *pst_wdev = OAL_PTR_NULL; /* ����P2P ������p2p0 �� p2p-p2p0 MAC ��ַ��wlan0 ��ȡ */
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

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    return OAL_SUCC;
#endif

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    ul_ret = wal_alloc_cfg_event(pst_net_dev, &pst_event_mem, NULL, &pst_cfg_msg, (WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_staion_id_param_stru)));     /* �����¼� */
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_netdev_set_mac_addr::wal_alloc_cfg_event fail!err code[%u]}\r\n",ul_ret);
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
    pst_param->en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == pst_param->en_p2p_mode)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_netdev_set_mac_addr::wal_wireless_iftype_to_mac_p2p_mode return BUFF}\r\n");
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

    frw_event_post_event(pst_event_mem, pst_mac_vap->ul_core_id);
#else
    frw_event_dispatch_event(pst_event_mem);
#endif

    FRW_EVENT_FREE(pst_event_mem);

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    wal_hipriv_wait_rsp(pst_net_dev, p_addr);
#endif

    return OAL_SUCC;
}

OAL_STATIC oal_int32  wal_netdev_set_mac_addr(oal_net_device_stru *pst_net_dev, void *p_addr)
{
    oal_int32 ret;
    wal_wake_lock();
    ret = _wal_netdev_set_mac_addr(pst_net_dev, p_addr);
    wal_wake_unlock();

    if(ret != OAL_SUCC)
    {
	#ifdef _PRE_WLAN_1102A_CHR
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_OPEN, CHR_WIFI_DRV_ERROR_POWER_ON_SET_MAC_ADDR);
	#endif
	}

    return ret;
}


OAL_STATIC oal_uint32 wal_ioctl_force_pass_filter(oal_net_device_stru *pst_net_dev, oal_uint8 uc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32          l_ret;
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_FORCE_PASS_FILTER, OAL_SIZEOF(oal_uint8));

    st_write_msg.auc_value[0] = uc_param;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
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


oal_int32 wal_android_priv_cmd(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, oal_int32 ul_cmd)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

    wal_android_wifi_priv_cmd_stru  st_priv_cmd;
    oal_int8    *pc_command         = OAL_PTR_NULL;
    oal_int32    l_ret              = 0;

#ifdef _PRE_WLAN_FEATURE_VOWIFI
    oal_int32    l_value;
    oal_int32    *pl_value;
#endif /* _PRE_WLAN_FEATURE_VOWIFI */

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
    if (g_st_dfr_info.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::dfr_process_status[%d]!}",
                g_st_dfr_info.bit_device_reset_process_flag);
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
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::too long priavte command. len:%d. }\r\n", st_priv_cmd.l_total_len);
        l_ret = -OAL_EINVAL;
        return l_ret;
    }

    /* �����ڴ汣��wpa_supplicant �·������������ */
    pc_command = oal_memalloc((oal_uint32)(st_priv_cmd.l_total_len + 1));/* total len Ϊpriv cmd ����buffer ���� */
    if (OAL_PTR_NULL == pc_command)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd::mem alloc failed.}\r\n");

        l_ret = -OAL_ENOMEM;
        return l_ret;
    }

    /* ����wpa_supplicant ����ں�̬�� */
    oal_memset(pc_command, 0, (oal_uint32)(st_priv_cmd.l_total_len + 1));

    l_ret = (oal_int32)oal_copy_from_user(pc_command, pst_ifr->ifr_data + 8, (oal_uint32)(st_priv_cmd.l_total_len));

    if (l_ret != 0)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd::oal_copy_from_user: -OAL_EFAIL }\r\n");
        l_ret = -OAL_EFAIL;
        oal_free(pc_command);
        return l_ret;
    }
    pc_command[st_priv_cmd.l_total_len] = '\0';
    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_android_priv_cmd::Android private cmd total_len:%d, used_len:%d}\r\n",
                st_priv_cmd.l_total_len, st_priv_cmd.l_used_len);

#ifdef _PRE_WLAN_NARROW_BAND
    if (oal_strncmp(pc_command, CMD_SET_HITALK_STR, OAL_STRLEN(CMD_SET_HITALK_STR)) == 0)
    {
        g_uc_hitalk_status = NARROW_BAND_ON_MASK;
        OAM_WARNING_LOG0(0, 0, "wal_android_priv_cmd: set hitalk enable");
    }
    else
#endif
    if (oal_strncmp(pc_command, CMD_SET_AP_WPS_P2P_IE, OAL_STRLEN(CMD_SET_AP_WPS_P2P_IE)) == 0)
    {
        oal_uint32 skip = OAL_STRLEN(CMD_SET_AP_WPS_P2P_IE) + 1;
        /* �ṹ������ */
        oal_app_ie_stru *pst_wps_p2p_ie;
        pst_wps_p2p_ie = (oal_app_ie_stru *)(pc_command + skip);

        /*lint -e413*/
        if((skip + pst_wps_p2p_ie->ul_ie_len + OAL_OFFSET_OF(oal_app_ie_stru, auc_ie)) > (oal_uint32)st_priv_cmd.l_total_len)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::SET_AP_WPS_P2P_IE param len is too short. need %d.}\r\n",(skip + pst_wps_p2p_ie->ul_ie_len));
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
        /*lint +e413*/
        l_ret = wal_ioctl_set_wps_p2p_ie(pst_net_dev,
                                            pst_wps_p2p_ie->auc_ie,
                                            pst_wps_p2p_ie->ul_ie_len,
                                            pst_wps_p2p_ie->en_app_ie_type);
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    else if(oal_strncmp(pc_command, CMD_P2P_SET_NOA, OAL_STRLEN(CMD_P2P_SET_NOA)) == 0)
    {
        oal_uint32 skip = OAL_STRLEN(CMD_P2P_SET_NOA) + 1;
        mac_cfg_p2p_noa_param_stru  st_p2p_noa_param;
        if ((skip + OAL_SIZEOF(st_p2p_noa_param)) > (oal_uint32)st_priv_cmd.l_total_len)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::CMD_P2P_SET_NOA param len is too short. need %d.}\r\n", skip + OAL_SIZEOF(st_p2p_noa_param));
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
        oal_memcopy(&st_p2p_noa_param, pc_command + skip, OAL_SIZEOF(mac_cfg_p2p_noa_param_stru));

        l_ret = wal_ioctl_set_p2p_noa(pst_net_dev,&st_p2p_noa_param);
    }
    else if(oal_strncmp(pc_command, CMD_P2P_SET_PS, OAL_STRLEN(CMD_P2P_SET_PS)) == 0)
    {
        oal_uint32 skip = OAL_STRLEN(CMD_P2P_SET_PS) + 1;
        mac_cfg_p2p_ops_param_stru  st_p2p_ops_param;
        if ((skip + OAL_SIZEOF(st_p2p_ops_param)) > (oal_uint32)st_priv_cmd.l_total_len)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::CMD_P2P_SET_PS param len is too short.need %d.}\r\n", skip + OAL_SIZEOF(st_p2p_ops_param));
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
        oal_memcopy(&st_p2p_ops_param, pc_command + skip, OAL_SIZEOF(mac_cfg_p2p_ops_param_stru));

        l_ret = wal_ioctl_set_p2p_ops(pst_net_dev,&st_p2p_ops_param);
    }
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM
   else if(0 == oal_strncmp(pc_command, CMD_SET_STA_PM_ON, OAL_STRLEN(CMD_SET_STA_PM_ON)))
   {
        mac_pm_switch_enum_uint8            en_pm_enable;

        if (OAL_STRLEN(pc_command) < (OAL_STRLEN((oal_int8 *)CMD_SET_STA_PM_ON + 2)))
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd::CMD_SET_STA_PM_ON puc_command len error.}\r\n");

            oal_free(pc_command);
            return -OAL_EFAIL;
        }
        en_pm_enable = *(pc_command + OAL_STRLEN((oal_int8 *)CMD_SET_STA_PM_ON) + 1);

        l_ret = wal_ioctl_set_sta_pm(pst_net_dev, en_pm_enable, MAC_STA_PM_CTRL_TYPE_HOST);
   }
#endif

#ifdef _PRE_WLAN_FEATURE_HS20
    else if (0 == oal_strncmp(pc_command, CMD_SET_QOS_MAP, OAL_STRLEN(CMD_SET_QOS_MAP)))
    {
        oal_uint32 skip = OAL_STRLEN(CMD_SET_QOS_MAP) + 1;
        hmac_cfg_qos_map_param_stru st_qos_map_param;
        if ((skip + OAL_SIZEOF(st_qos_map_param)) > (oal_uint32)st_priv_cmd.l_total_len)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::CMD_SET_QOS_MAP param len is too short.need %d.}\r\n", skip + OAL_SIZEOF(st_qos_map_param));
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
        oal_memcopy(&st_qos_map_param, pc_command + skip, OAL_SIZEOF(hmac_cfg_qos_map_param_stru));

        l_ret = wal_ioctl_set_qos_map(pst_net_dev,&st_qos_map_param);
    }
#endif
    else if(0 == oal_strncmp(pc_command, CMD_COUNTRY, OAL_STRLEN(CMD_COUNTRY)))
    {
    #ifdef _PRE_WLAN_FEATURE_11D
        const oal_int8 *country_code;
        oal_int8        auc_country_code[3] = {0};
        oal_int32       l_ret;

        /* ��ʽ:COUNTRY CN */
        if(OAL_STRLEN(pc_command) < (OAL_STRLEN((oal_int8 *)CMD_COUNTRY) + 3))
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd::puc_command len error.}\r\n");

            oal_free(pc_command);
            return -OAL_EFAIL;
        }

        country_code = pc_command + OAL_STRLEN((oal_int8 *)CMD_COUNTRY) + 1;

        oal_memcopy(auc_country_code, country_code, 2);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        /* ��ini��coutry_code=99ʱ��������android�޸�country_code���������ʹ��ini��country_code */
        if (OAL_TRUE == g_st_cust_country_code_ignore_flag.en_country_code_ingore_ini_flag)
        {
            OAM_WARNING_LOG3(0, OAM_SF_ANY, "{wal_android_priv_cmd::wlan_pm_set_country[%c %c] is ignore, ignore_ini_flag is [%d]",
                            auc_country_code[0], auc_country_code[1], g_st_cust_country_code_ignore_flag.en_country_code_ingore_ini_flag);

            oal_free(pc_command);
            return OAL_SUCC;
        }

#endif


#ifdef _PRE_WLAN_FEATURE_DFS
        l_ret = wal_regdomain_update_for_dfs(pst_net_dev, auc_country_code);

        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::return err code [%d]!}\r\n", l_ret);

            oal_free(pc_command);
            return -OAL_EFAIL;
        }
#endif
        l_ret = wal_regdomain_update(pst_net_dev, auc_country_code);

        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::return err code [%d]!}\r\n", l_ret);

            oal_free(pc_command);
            return -OAL_EFAIL;
        }
    #else
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd::_PRE_WLAN_FEATURE_11D is not define!}\r\n");
    #endif
    }
#ifdef _PRE_WLAN_FEATURE_LTECOEX
    else if(0 == oal_strncmp(pc_command, CMD_LTECOEX_MODE, OAL_STRLEN(CMD_LTECOEX_MODE)))
    {
        oal_int32      l_ret;
        oal_int8       ltecoex_mode;

        /* ��ʽ:LTECOEX_MODE 1 or LTECOEX_MODE 0 */
        if(OAL_STRLEN(pc_command) < (OAL_STRLEN((oal_int8 *)CMD_LTECOEX_MODE) + 2))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::CMD_LTECOEX_MODE length is to short [%d].}\r\n", OAL_STRLEN(pc_command));

            oal_free(pc_command);
            return -OAL_EFAIL;
        }
        ltecoex_mode = oal_atoi(pc_command + OAL_STRLEN((oal_int8 *)CMD_LTECOEX_MODE) + 1);

        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::CMD_LTECOEX_MODE command,ltecoex mode:%d}\r\n", ltecoex_mode);

        l_ret = (oal_int32)wal_ioctl_ltecoex_mode_set(pst_net_dev, (oal_int8*)&ltecoex_mode);
        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::return err code [%d]!}\r\n", l_ret);
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
    }
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    else if (oal_strncmp(pc_command, CMD_TX_POWER, OAL_STRLEN(CMD_TX_POWER)) == 0)
    {
        oal_uint32  ul_skip = OAL_STRLEN((oal_int8 *)CMD_TX_POWER) + 1;
        oal_uint16   us_txpwr = (oal_uint16)oal_atoi(pc_command + ul_skip);
        l_ret = wal_ioctl_reduce_sar(pst_net_dev, us_txpwr);
        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::return err code [%d]!}\r\n", l_ret);
            oal_free(pc_command);
            /* ������ӡ�����룬���سɹ�����ֹsupplicant �ۼ�4�� ioctlʧ�ܵ���wifi�쳣���� */
            return OAL_SUCC;
        }
    }
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    else if (oal_strncmp(pc_command, CMD_WPAS_GET_CUST, OAL_STRLEN(CMD_WPAS_GET_CUST)) == 0)
    {
        /* ��buf���� */
        oal_memset(pc_command, 0, st_priv_cmd.l_total_len + 1);
        pc_command[st_priv_cmd.l_total_len] = '\0';

        /* ��ȡȫ�����ƻ����ã���������ȡdisable_capab_ht40 */
        hwifi_config_init_force();

        /* ��ֵht40��ֹλ */
        *pc_command = g_st_wlan_customize.uc_disable_capab_2ght40;

        if(oal_copy_to_user(pst_ifr->ifr_data+8, pc_command, (oal_uint32)(st_priv_cmd.l_total_len)))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_android_priv_cmd: Failed to copy ioctl_data to user !");
            oal_free(pc_command);
            /* ���ش���֪ͨsupplicant����ʧ�ܣ�supplicant���������������� */
            return -OAL_EFAIL;
        }
    }
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_VOWIFI
    else if (oal_strncmp(pc_command, CMD_VOWIFI_SET_PARAM, OAL_STRLEN(CMD_VOWIFI_SET_PARAM)) == 0)
    {
        l_ret = wal_ioctl_set_vowifi_param(pst_net_dev, pc_command);
        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::VOWIFI_SET_PARAM return err code [%d]!}", l_ret);
            oal_free(pc_command);
            return -OAL_EFAIL;
        }

    }
    else if (oal_strncmp(pc_command, CMD_VOWIFI_GET_PARAM, OAL_STRLEN(CMD_VOWIFI_GET_PARAM)) == 0)
    {
        l_value = 0;
        l_ret = wal_ioctl_get_vowifi_param(pst_net_dev, pc_command, &l_value);
        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::CMD_VOWIFI_GET_MODE(%d) return err code [%d]!}", l_ret);
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
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_android_priv_cmd:CMD_VOWIFi_GET_MODE Failed to copy ioctl_data to user !");
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
    }
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
#ifndef CONFIG_HAS_EARLYSUSPEND
    else if(0 == oal_strncmp(pc_command, CMD_SETSUSPENDOPT, OAL_STRLEN(CMD_SETSUSPENDOPT)))
    {
        /* Ŀǰ��û�з����ϲ��·�������,��ά����ֺ��ٷ��� */
        OAM_ERROR_LOG1(0,0,"wal_android_priv_cmd:CMD_SETSUSPENDOPT param%d",*(pc_command + OAL_STRLEN(CMD_SETSUSPENDOPT) + 1) - '0');
        l_ret = OAL_SUCC;
    }
    else if (0 == oal_strncmp(pc_command, CMD_SETSUSPENDMODE, OAL_STRLEN(CMD_SETSUSPENDMODE)))
    {
        l_ret = wal_ioctl_set_suspend_mode(pst_net_dev, *(pc_command + OAL_STRLEN(CMD_SETSUSPENDMODE) + 1) - '0');
    }
#endif
    else if(oal_strncmp(pc_command, CMD_GET_WIFI_PRIV_FEATURE_CAPABILITY, OAL_STRLEN(CMD_GET_WIFI_PRIV_FEATURE_CAPABILITY)) == 0)
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

    else if (oal_strncmp(pc_command, CMD_VOWIFI_IS_SUPPORT, OAL_STRLEN(CMD_VOWIFI_IS_SUPPORT)) == 0)
    {
        if ((oal_uint32)st_priv_cmd.l_total_len < OAL_STRLEN(CMD_VOWIFI_IS_SUPPORT_REPLY))
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::CMD_VOWIFI_IS_SUPPORT length is to short. need %d}\r\n", OAL_STRLEN(CMD_VOWIFI_IS_SUPPORT_REPLY));
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
        /* ��buf���� */
        oal_memset(pc_command, 0, (oal_uint32)(st_priv_cmd.l_total_len + 1));
        pc_command[st_priv_cmd.l_total_len] = '\0';
        oal_memcopy(pc_command, CMD_VOWIFI_IS_SUPPORT_REPLY, OAL_STRLEN(CMD_VOWIFI_IS_SUPPORT_REPLY));
        if(oal_copy_to_user(pst_ifr->ifr_data+8, pc_command, OAL_STRLEN(CMD_VOWIFI_IS_SUPPORT_REPLY)))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_android_priv_cmd:CMD_VOWIFI_IS_SUPPORT Failed to copy ioctl_data to user !");
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
    }
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    else if (oal_strncmp(pc_command, CMD_FILTER_SWITCH, OAL_STRLEN(CMD_FILTER_SWITCH)) == 0)
    {
#ifdef CONFIG_DOZE_FILTER
        oal_int32 l_on;
        oal_uint32 command_len = OAL_STRLEN(pc_command);

        /* ��ʽ:FILTER 1 or FILTER 0 */
        if(command_len < (OAL_STRLEN((oal_int8 *)CMD_FILTER_SWITCH) + 2))
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::CMD_FILTER_SWITCH cmd len must equal or larger than 8. Now the cmd len:%d.}\r\n", command_len);

            oal_free(pc_command);
            return -OAL_EFAIL;
        }

        l_on = oal_atoi(pc_command + OAL_STRLEN((oal_int8 *)CMD_FILTER_SWITCH) + 1);

        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::CMD_FILTER_SWITCH %d.}", l_on);

        /* �����ں˽ӿڵ��� gWlanFilterOps.set_filter_enable */
        l_ret = hw_set_net_filter_enable(l_on);
        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::CMD_FILTER_SWITCH return err code [%d]!}", l_ret);
            oal_free(pc_command);
            return -OAL_EFAIL;
        }
#else
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd::Not support CMD_FILTER_SWITCH.}");
#endif
    }
#endif /* _PRE_WLAN_FEATURE_IP_FILTER */
    else if (0 == oal_strncmp(pc_command, CMD_RXFILTER_START, OAL_STRLEN(CMD_RXFILTER_START)))
    {
        wal_ioctl_force_pass_filter(pst_net_dev, OAL_TRUE);
    }
    else if (0 == oal_strncmp(pc_command, CMD_RXFILTER_STOP, OAL_STRLEN(CMD_RXFILTER_STOP)))
    {
        wal_ioctl_force_pass_filter(pst_net_dev, OAL_FALSE);
    }

    else
    {
        /* �������ڲ�֧�ֵ�������سɹ��������ϲ�wpa_supplicant��Ϊioctlʧ�ܣ������쳣����wifi */
        //OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_android_priv_cmd::ignore unknow private command.}\r\n");
        l_ret = OAL_SUCC;
    }

    oal_free(pc_command);
    return l_ret;
#else
    return  OAL_SUCC;
#endif
}


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
        l_ret = wal_android_priv_cmd(pst_net_dev, pst_ifr, ul_cmd);
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
    //    OAM_WARNING_LOG1(0, OAM_SF_ANY,"atcmdsrv_ioctl_cmd,cmd=0x%x", ul_cmd);
        wal_wake_lock();
        l_ret = wal_atcmdsrv_wifi_priv_cmd(pst_net_dev, pst_ifr, ul_cmd);
        wal_wake_unlock();
        return l_ret;
    }
#endif
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

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pc_param)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_hipriv_set_mode::pst_net_dev/p_param null ptr error %d!}\r\n", pst_net_dev, pc_param);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* pc_paramָ����ģʽ����, ����ȡ����ŵ�ac_mode_str�� */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_mode_str, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mode::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    ac_mode_str[OAL_SIZEOF(ac_mode_str) - 1] = '\0';    /* ȷ����null��β */

    for (uc_prot_idx = 0; OAL_PTR_NULL != g_ast_mode_map[uc_prot_idx].pc_name; uc_prot_idx++)
    {
        l_ret = oal_strcmp(g_ast_mode_map[uc_prot_idx].pc_name, ac_mode_str);

        if (0 == l_ret)
        {
            break;
        }
    }

    if (OAL_PTR_NULL == g_ast_mode_map[uc_prot_idx].pc_name)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_mode::unrecognized protocol string!}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }


    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_MODE, OAL_SIZEOF(mac_cfg_mode_param_stru));

    pst_mode_param = (mac_cfg_mode_param_stru *)(st_write_msg.auc_value);
    pst_mode_param->en_protocol  = g_ast_mode_map[uc_prot_idx].en_mode;
    pst_mode_param->en_band      = g_ast_mode_map[uc_prot_idx].en_band;
    pst_mode_param->en_bandwidth = g_ast_mode_map[uc_prot_idx].en_bandwidth;

    OAM_INFO_LOG3(0, OAM_SF_CFG, "{wal_hipriv_set_mode::protocol[%d],band[%d],bandwidth[%d]!}\r\n",
                            pst_mode_param->en_protocol, pst_mode_param->en_band, pst_mode_param->en_bandwidth);

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_mode_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    wal_hipriv_wait_rsp(pst_net_dev, pc_param);
#endif

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mode::wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
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

    /* pc_paramָ���´�����net_device��name, ����ȡ����ŵ�ac_name�� */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_freq, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_freq::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    l_freq = oal_atoi(ac_freq);
    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_freq::l_freq = %d!}\r\n", l_freq);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CURRENT_CHANEL, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_freq;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
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
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_freq::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_BT_FITTING_DATA_COLLECT
OAL_STATIC oal_uint32  wal_hipriv_init_bt_env(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_INIT_BT_ENV, OAL_SIZEOF(oal_uint8));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_init_bt_env::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32  wal_hipriv_set_bt_freq(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_freq;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_freq[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                  ul_ret;
    oal_int32                   l_ret;

    /* pc_paramָ���´�����net_device��name, ����ȡ����ŵ�ac_name�� */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_freq, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_bt_freq::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    l_freq = oal_atoi(ac_freq);
    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_bt_freq::l_freq = %d!}\r\n", l_freq);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_BT_FREQ, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_freq;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_bt_freq::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32  wal_hipriv_set_bt_upc_by_freq(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_freq;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_freq[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                  ul_ret;
    oal_int32                   l_ret;

    /* pc_paramָ���´�����net_device��name, ����ȡ����ŵ�ac_name�� */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_freq, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_bt_upc_by_freq::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    l_freq = oal_atoi(ac_freq);
    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_bt_upc_by_freq::l_freq = %d!}\r\n", l_freq);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_BT_UPC_BY_FREQ, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_freq;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_bt_upc_by_freq::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32  wal_hipriv_print_bt_gm(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_PRINT_BT_GM, OAL_SIZEOF(oal_uint8));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_print_bt_gm::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32  wal_hipriv_set_tx_ba_policy(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru          st_write_msg;
    oal_uint32                  ul_tx_ba_policy;
    oal_uint32                  ul_off_set;
    oal_int8                    ac_value[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                  ul_ret;
    oal_int32                   l_ret;

    /* pc_paramָ���´�����net_device��name, ����ȡ����ŵ�ac_name�� */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_tx_ba_policy::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    ul_tx_ba_policy = (oal_uint32)oal_atoi(ac_value);
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_tx_ba_policy::ul_tx_ba_policy = %d!}\r\n", ul_tx_ba_policy);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_TX_BA_POLICY, OAL_SIZEOF(oal_uint32));
    *((oal_uint32 *)(st_write_msg.auc_value)) = ul_tx_ba_policy;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_tx_ba_policy::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}



OAL_STATIC oal_int32 wal_ioctl_set_mode(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, void *p_param, oal_int8 *pc_extra)
{
    oal_iw_point_stru          *pst_param;
    oal_uint32                  ul_ret;
    oal_int32                   l_ret;
    oal_int8                    ac_mode_str[24] = {0};     /* Ԥ��Э��ģʽ�ַ����ռ� */
    oal_uint8                   uc_prot_idx;
    mac_cfg_mode_param_stru    *pst_mode_param;
    wal_msg_write_stru          st_write_msg;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == p_param)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_mode::pst_net_dev/p_param null ptr error %d!}\r\n", pst_net_dev, p_param);
        return -OAL_EFAUL;
    }

    /* �豸��up״̬���������ã�������down */
    if (0 != (OAL_IFF_RUNNING & OAL_NETDEVICE_FLAGS(pst_net_dev)))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_mode::device is busy, please down it first %d!}\r\n", OAL_NETDEVICE_FLAGS(pst_net_dev));
        return -OAL_EBUSY;
    }
    pst_param = (oal_iw_point_stru *)p_param;
    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_mode::input str length is %d!}\r\n", pst_param->length);

    if (pst_param->length > OAL_SIZEOF(ac_mode_str))    /* �ַ����ж�, ���Ȱ���\0 */
    {
        pst_param->length =  OAL_SIZEOF(ac_mode_str);
    }

    ul_ret = oal_copy_from_user(ac_mode_str, pst_param->pointer, pst_param->length);

    /* copy_from_user������Ŀ���Ǵ��û��ռ俽�����ݵ��ں˿ռ䣬ʧ�ܷ���û�б��������ֽ������ɹ�����0 */
    if (ul_ret > 0)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_mode::oal_copy_from_user return error code %d!}\r\n", ul_ret);
        return -OAL_EFAUL;
    }

    ac_mode_str[OAL_SIZEOF(ac_mode_str) - 1] = '\0';    /* ȷ����null��β */

    for (uc_prot_idx = 0; OAL_PTR_NULL != g_ast_mode_map[uc_prot_idx].pc_name; uc_prot_idx++)
    {
        l_ret = oal_strcmp(g_ast_mode_map[uc_prot_idx].pc_name, ac_mode_str);

        if (0 == l_ret)
        {
            break;
        }
    }

    if (OAL_PTR_NULL == g_ast_mode_map[uc_prot_idx].pc_name)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_mode::unrecognized protocol string!}\r\n");
        return -OAL_EINVAL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_MODE, OAL_SIZEOF(mac_cfg_mode_param_stru));

    pst_mode_param = (mac_cfg_mode_param_stru *)(st_write_msg.auc_value);
    pst_mode_param->en_protocol  = g_ast_mode_map[uc_prot_idx].en_mode;
    pst_mode_param->en_band      = g_ast_mode_map[uc_prot_idx].en_band;
    pst_mode_param->en_bandwidth = g_ast_mode_map[uc_prot_idx].en_bandwidth;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_mode_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_mode::wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_set_freq(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iw_freq_stru *pst_freq, oal_int8 *pc_extra)
{
    oal_int32                   l_ret;
    wal_msg_write_stru          st_write_msg;


    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pst_freq))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_freq::param null, pst_net_dev = %p, pst_freq = %p.}", pst_net_dev, pst_freq);
        return -OAL_EINVAL;
    }

    /* �豸��up״̬���������ã�������down */
    if (0 != (OAL_IFF_RUNNING & OAL_NETDEVICE_FLAGS(pst_net_dev)))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_freq::device is busy, please down it firs %d!}\r\n", OAL_NETDEVICE_FLAGS(pst_net_dev));
        return -OAL_EBUSY;
    }

    OAM_INFO_LOG4(0, OAM_SF_ANY, "{wal_ioctl_set_freq::pst_freq: m = %u, e = %u, i = %u, flags = %u!}\r\n",
                 (oal_uint32)pst_freq->m, (oal_uint16)pst_freq->e, pst_freq->i, pst_freq->flags);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CURRENT_CHANEL, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = pst_freq->m;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_freq::return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_set_txpower(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iw_param_stru *pst_param, oal_int8 *pc_extra)
{
    oal_int32                   l_ret;
    wal_msg_write_stru          st_write_msg;

    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pst_param))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_txpower::param null, pst_net_dev = %p, pst_param = %p.}", pst_net_dev, pst_param);
        return -OAL_EINVAL;
    }

    /* �豸��up״̬���������ã�������down */
    if (0 != (OAL_IFF_RUNNING & OAL_NETDEVICE_FLAGS(pst_net_dev)))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_txpower::device is busy, please down it first %d!}\r\n", OAL_NETDEVICE_FLAGS(pst_net_dev));
        return -OAL_EBUSY;
    }

    OAM_INFO_LOG4(0, OAM_SF_ANY, "{wal_ioctl_set_txpower::pst_param: value= %d, fixed = %d, disabled = %d, flags = %d!}\r\n",
                 pst_param->value, pst_param->fixed, pst_param->disabled, pst_param->flags);

    if (pst_param->flags != OAL_IW_TXPOW_DBM)       /* ��������������Ͳ���dBm���򷵻ش��� */
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_txpower::invalid argument!}\r\n");
        return -OAL_EINVAL;
    }

    if (pst_param->value > 0xFF || pst_param->value < 0)   /* �����쳣*/
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_txpower::invalid argument!,value[%d]}\r\n",pst_param->value);
        return -OAL_EINVAL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_TX_POWER, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = pst_param->value;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_txpower::return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
/* 1102 ��ʹ��iwconfig iwpriv ������hipriv �ӿ� */

OAL_STATIC oal_uint32  wal_ioctl_get_mode(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return OAL_FAIL;
}

OAL_STATIC oal_uint32  wal_ioctl_get_essid(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return OAL_FAIL;
}


oal_uint32  wal_ioctl_set_essid(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint8                       uc_ssid_len;
    oal_int32                       l_ret;
    wal_msg_write_stru              st_write_msg;
    mac_cfg_ssid_param_stru        *pst_param;
    mac_vap_stru                   *pst_mac_vap;
    oal_uint32                      ul_off_set;
    oal_int8                       *pc_ssid;
    oal_int8                        ac_ssid[WLAN_SSID_MAX_LEN] = {0};
    oal_uint32                      ul_ret;

    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pc_param))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_essid::param null, pst_net_dev = %p, pc_param = %p.}", pst_net_dev, pc_param);
        return -OAL_EINVAL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_essid::pst_mac_vap is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        /* �豸��up״̬����APʱ�����������ã�������down */
        if (0 != (OAL_IFF_RUNNING & OAL_NETDEVICE_FLAGS(pst_net_dev)))
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_essid::device is busy, please down it firste %d!}\r\n", OAL_NETDEVICE_FLAGS(pst_net_dev));
            return -OAL_EBUSY;
        }
    }

    /* pc_paramָ����ģʽ����, ����ȡ����ŵ�ac_mode_str�� */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_ssid, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_essid::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }

    pc_ssid       = ac_ssid;
    pc_ssid       = oal_strim(ac_ssid);                   /* ȥ���ַ�����ʼ��β�Ŀո� */
    uc_ssid_len = (oal_uint8)OAL_STRLEN(pc_ssid);

    OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_essid:: ssid length %d!}\r\n", uc_ssid_len);

    if (uc_ssid_len > WLAN_SSID_MAX_LEN - 1)        /* -1Ϊ\0Ԥ���ռ� */
    {
        uc_ssid_len = WLAN_SSID_MAX_LEN - 1;
    }

    OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_essid:: ssid length is %d!}\r\n", uc_ssid_len);
    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SSID, OAL_SIZEOF(mac_cfg_ssid_param_stru));

    /* ��дWID��Ӧ�Ĳ��� */
    pst_param = (mac_cfg_ssid_param_stru *)(st_write_msg.auc_value);
    pst_param->uc_ssid_len = uc_ssid_len;
    oal_memcopy(pst_param->ac_ssid, pc_ssid, uc_ssid_len);

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ssid_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_essid:: wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
OAL_STATIC oal_uint32  wal_ioctl_get_bss_type(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return OAL_FAIL;
}
OAL_STATIC oal_uint32  wal_ioctl_set_bss_type(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return OAL_FAIL;
}
OAL_STATIC oal_uint32  wal_ioctl_get_freq(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return OAL_FAIL;
}

OAL_STATIC oal_uint32  wal_ioctl_get_txpower(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return OAL_FAIL;
}

#else

OAL_STATIC oal_int32  wal_ioctl_get_mode(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_param, oal_int8 *pc_extra)
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
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_mode::wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
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
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_mode::null pointer.}\r\n");
        oal_free(pst_rsp_msg);
        return -OAL_EFAUL;
    }

    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        for (uc_prot_idx = 0; OAL_PTR_NULL != g_ast_mode_map[uc_prot_idx].pc_name; uc_prot_idx++)
        {
            if ((g_ast_mode_map[uc_prot_idx].en_mode == pst_mode_param->en_protocol) &&
                (g_ast_mode_map[uc_prot_idx].en_band == pst_mode_param->en_band) &&
                (g_ast_mode_map[uc_prot_idx].en_bandwidth == pst_mode_param->en_bandwidth))
            {
                break;
            }
        }
    }
    /* STAģʽ��Ƶ�κ�Ƶ���ں�AP����֮������Ӧ���˴����Ƚ�Э��ģʽ */
    else if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        for (uc_prot_idx = 0; OAL_PTR_NULL != g_ast_mode_map[uc_prot_idx].pc_name; uc_prot_idx++)
        {
            if (g_ast_mode_map[uc_prot_idx].en_mode == pst_mode_param->en_protocol)
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

    if (OAL_PTR_NULL == g_ast_mode_map[uc_prot_idx].pc_name)
    {
        pst_point->length = (oal_uint16)OAL_STRLEN(pc_err_str);
        oal_memcopy(pc_extra, pc_err_str, pst_point->length);
        oal_free(pst_rsp_msg);
        return OAL_SUCC;
    }

    pst_point->length = (oal_uint16)OAL_STRLEN(g_ast_mode_map[uc_prot_idx].pc_name);
    oal_memcopy(pc_extra, g_ast_mode_map[uc_prot_idx].pc_name, pst_point->length);

    oal_free(pst_rsp_msg);
    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_set_essid(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iw_point_stru *pst_data, oal_int8 *pc_ssid)
{
    oal_uint8                       uc_ssid_len;
    oal_int32                       l_ret;
    wal_msg_write_stru              st_write_msg;
    mac_cfg_ssid_param_stru        *pst_param;
    mac_vap_stru                   *pst_mac_vap;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_essid::pst_mac_vap is null!}\r\n");
        return -OAL_EFAUL;
    }

    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        /* �豸��up״̬����APʱ�����������ã�������down */
        if (0 != (OAL_IFF_RUNNING & OAL_NETDEVICE_FLAGS(pst_net_dev)))
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_essid::device is busy, please down it firste %d!}\r\n", OAL_NETDEVICE_FLAGS(pst_net_dev));
            return -OAL_EBUSY;
        }
    }

    pc_ssid = oal_strim(pc_ssid);                   /* ȥ���ַ�����ʼ��β�Ŀո� */

    OAM_INFO_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_essid:: pst_data->flags, pst_data->lengt %d, %d!}\r\n", pst_data->flags, pst_data->length);

    //uc_ssid_len = (oal_uint8)pst_data->length;    /* ���Ȳ����� \0 */
    uc_ssid_len = (oal_uint8)OAL_STRLEN(pc_ssid);

    if (uc_ssid_len > WLAN_SSID_MAX_LEN - 1)        /* -1Ϊ\0Ԥ���ռ� */
    {
        uc_ssid_len = WLAN_SSID_MAX_LEN - 1;
    }

    OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_essid:: ssid length is %d!}\r\n", uc_ssid_len);
    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SSID, OAL_SIZEOF(mac_cfg_ssid_param_stru));

    /* ��дWID��Ӧ�Ĳ��� */
    pst_param = (mac_cfg_ssid_param_stru *)(st_write_msg.auc_value);
    pst_param->uc_ssid_len = uc_ssid_len;
    oal_memcopy(pst_param->ac_ssid, pc_ssid, uc_ssid_len);

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ssid_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_essid:: wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_get_essid(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iw_point_stru *pst_data, oal_int8 *pc_ssid)
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
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_essid:: wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
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


OAL_STATIC oal_int32  wal_ioctl_get_bss_type(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_uint32 *pul_type, oal_int8 *pc_extra)
{
    oal_int32                       l_ret;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    wal_msg_query_stru              st_query_msg;
    wal_msg_rsp_stru               *pst_query_rsp_msg;
    oal_uint32                      ul_type;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    st_query_msg.en_wid = WLAN_CFGID_BSS_TYPE;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_bss_type::wal_ioctl_get_bss_type return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);

    /* ҵ���� */
    ul_type = *((oal_uint32 *)pst_query_rsp_msg->auc_value);

    *pul_type = OAL_IW_MODE_AUTO;

    if (WLAN_MIB_DESIRED_BSSTYPE_INFRA == ul_type)
    {
        *pul_type = OAL_IW_MODE_INFRA;
    }

    oal_free(pst_rsp_msg);

    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_set_bss_type(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_uint32 *pul_type, oal_int8 *pc_extra)
{
    oal_uint32                      ul_type;
    oal_int32                       l_ret;
    wal_msg_write_stru              st_write_msg;
    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pul_type))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_bss_type::param null, pst_net_dev = %p, pul_type = %p.}", pst_net_dev, pul_type);
        return -OAL_EINVAL;
    }

    /* �豸��up״̬���������ã�������down */
    if (0 != (OAL_IFF_RUNNING & OAL_NETDEVICE_FLAGS(pst_net_dev)))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_bss_type::device is busy, please down it first %d!}\r\n", OAL_NETDEVICE_FLAGS(pst_net_dev));
        return -OAL_EBUSY;
    }

    ul_type = *pul_type;

    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_bss_type::type value is %d!}\r\n", ul_type);

    if (ul_type < WLAN_MIB_DESIRED_BSSTYPE_INFRA || ul_type >= WLAN_MIB_DESIRED_BSSTYPE_BUTT)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_bss_type::input type is invalid!}\r\n");
        return -OAL_EINVAL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_BSS_TYPE, OAL_SIZEOF(ul_type));
    *((oal_uint32 *)st_write_msg.auc_value) = ul_type;    /* ��дWID��Ӧ�Ĳ��� */

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(ul_type),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_bss_type::return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_get_freq(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iw_freq_stru *pst_freq, oal_int8 *pc_extra)
{
    oal_int32                       l_ret;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    wal_msg_query_stru              st_query_msg;
    wal_msg_rsp_stru               *pst_queue_rsp_msg;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    st_query_msg.en_wid = WLAN_CFGID_CURRENT_CHANEL;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_freq::return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    pst_queue_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);

    /* ҵ���� */
    pst_freq->m = *((oal_int32 *)(pst_queue_rsp_msg->auc_value));
    pst_freq->e = 0;

    oal_free(pst_rsp_msg);

    return OAL_SUCC;
}




OAL_STATIC oal_int32  wal_ioctl_get_txpower(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_iw_param_stru *pst_param, oal_int8 *pc_extra)
{
    oal_int32                       l_ret;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    wal_msg_query_stru              st_query_msg;
    wal_msg_rsp_stru               *pst_query_rsp_msg = OAL_PTR_NULL;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    st_query_msg.en_wid = WLAN_CFGID_TX_POWER;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_txpower::return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ*/
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);

    /* ҵ���� */
    pst_param->value    = *((oal_int32 *)(pst_query_rsp_msg->auc_value));
    pst_param->fixed    = 1;
    pst_param->disabled = 0;
    pst_param->flags    = OAL_IW_TXPOW_DBM;

    oal_free(pst_rsp_msg);
    return OAL_SUCC;
}

#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44)) *//* 1102 ��ʹ��iwconfig ������hipriv �ӿ� */


OAL_STATIC oal_int32  wal_ioctl_get_apaddr(
                oal_net_device_stru         *pst_net_dev,
                oal_iw_request_info_stru    *pst_info,
                oal_sockaddr_stru           *pst_addr,
                oal_int8                    *pc_extra)
{
    mac_vap_stru   *pst_mac_vap;
    oal_uint8       auc_zero_addr[WLAN_MAC_ADDR_LEN] = {0};
    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pst_addr))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_get_apaddr::param null, pst_net_dev = %p, pst_addr = %p.}", pst_net_dev, pst_addr);
        return -OAL_EINVAL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_apaddr::pst_mac_vap is null!}\r\n");
        return -OAL_EFAUL;
    }

    if(MAC_VAP_STATE_UP == pst_mac_vap->en_vap_state)
    {
        oal_set_mac_addr((oal_uint8 *)pst_addr->sa_data, pst_mac_vap->auc_bssid);
    }
    else
    {
        oal_set_mac_addr((oal_uint8 *)pst_addr->sa_data, auc_zero_addr);
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_get_iwrate(
                oal_net_device_stru         *pst_net_dev,
                oal_iw_request_info_stru    *pst_info,
                oal_iw_param_stru           *pst_param,
                oal_int8                    *pc_extra)
{
    /* iwconfig��ȡrate����֧�ִ�����򷵻�-1 */

    return -OAL_EFAIL;
}


OAL_STATIC oal_int32  wal_ioctl_get_iwsense(
                oal_net_device_stru         *pst_net_dev,
                oal_iw_request_info_stru    *pst_info,
                oal_iw_param_stru           *pst_param,
                oal_int8                    *pc_extra)
{
    /* iwconfig��ȡsense����֧�ִ�����򷵻�-1 */

    return -OAL_EFAIL;
}


OAL_STATIC oal_int32  wal_ioctl_get_rtsthres(
                oal_net_device_stru         *pst_net_dev,
                oal_iw_request_info_stru    *pst_info,
                oal_iw_param_stru           *pst_param,
                oal_int8                    *pc_extra)
{
    mac_vap_stru   *pst_mac_vap;
    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pst_param))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_get_rtsthres::param null, pst_net_dev = %p, pst_param = %p.}", pst_net_dev, pst_param);
        return -OAL_EINVAL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_rtsthres::pst_mac_vap is null!}\r\n");
        return -OAL_EFAUL;
    }

    pst_param->value    = (oal_int32)pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11RTSThreshold;
    pst_param->disabled = (WLAN_RTS_MAX == pst_param->value);
    pst_param->fixed    = 1;

    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_get_fragthres(
                oal_net_device_stru         *pst_net_dev,
                oal_iw_request_info_stru    *pst_info,
                oal_iw_param_stru           *pst_param,
                oal_int8                    *pc_extra)
{
    mac_vap_stru   *pst_mac_vap;
    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pst_param))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_get_fragthres::param null, pst_net_dev = %p, pst_param = %p.}", pst_net_dev, pst_param);
        return -OAL_EINVAL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_ioctl_get_fragthres::pst_mac_vap is null!}\r\n");
        return -OAL_EFAUL;
    }

    pst_param->value    = (oal_int32)pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11FragmentationThreshold;
    pst_param->disabled = (WLAN_FRAG_THRESHOLD_MAX == pst_param->value);
    pst_param->fixed    = 1;

    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_get_iwencode(
                oal_net_device_stru         *pst_net_dev,
                oal_iw_request_info_stru    *pst_info,
                oal_iw_point_stru           *pst_param,
                oal_int8                    *pc_extra)
{
    /* ��֧��iwconfig��ȡencode��ֱ�ӷ���-1 */

    return -OAL_EFAIL;
}


OAL_STATIC oal_int32  wal_ioctl_get_iwrange(
                oal_net_device_stru         *pst_net_dev,
                oal_iw_request_info_stru    *pst_info,
                oal_iw_point_stru           *pst_param,
                oal_int8                    *pc_extra)
{

    return -OAL_EFAIL;
}



OAL_STATIC oal_int32  wal_ioctl_get_param(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_iw, oal_int8 *pc_extra)
{
    oal_int32                       l_ret;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    wal_msg_query_stru              st_query_msg;
    wal_msg_rsp_stru               *pst_query_rsp_msg;
    oal_int32                      *pl_param;

    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pc_extra))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_get_param::param null, pst_net_dev = %p, pc_extra = %p.}", pst_net_dev, pc_extra);
        return -OAL_EINVAL;
    }

    pl_param = (oal_int32 *)pc_extra;
    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_param::return err code %d!}\r\n", pl_param[0]);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    st_query_msg.en_wid = (oal_uint16)pl_param[0];

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_param::return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);

    /* ҵ���� */
    pl_param[0] = *((oal_int32 *)(pst_query_rsp_msg->auc_value));

    oal_free(pst_rsp_msg);
    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_set_param(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_iw, oal_int8 *pc_extra)
{
    oal_int32                       l_error = 0;
    oal_int32                       l_ret;
    oal_int32                      *pl_param;
    oal_int32                       l_subioctl_id;
    oal_int32                       l_value;
    wal_msg_write_stru              st_write_msg;

    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pc_extra))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_param::param null, pst_net_dev = %p, pc_extra = %p.}", pst_net_dev, pc_extra);
        return -OAL_EINVAL;
    }

    /* �豸��up״̬���������ã�������down */
    if (0 != (OAL_IFF_RUNNING & OAL_NETDEVICE_FLAGS(pst_net_dev)))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_param::device is busy, please down it first %d!}\r\n", OAL_NETDEVICE_FLAGS(pst_net_dev));
        return -OAL_EBUSY;
    }

    pl_param      = (oal_int32 *)pc_extra;
    l_subioctl_id = pl_param[0];    /* ��ȡsub-ioctl��ID */
    l_value       = pl_param[1];    /* ��ȡҪ���õ�ֵ */
    OAM_INFO_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_param::the subioctl_id and value is %d, %d!}\r\n", l_subioctl_id, l_value);

    if (l_value < 0)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_param::input value is negative %d!}\r\n", l_value);
        return -OAL_EINVAL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, (oal_uint16)l_subioctl_id, OAL_SIZEOF(oal_int32));

    switch (l_subioctl_id)                                                      /* ����sub-ioctl id��дWID */
    {
        case WLAN_CFGID_PROT_MODE:
            if (l_value >= WLAN_PROT_BUTT)  /* ������� */
            {
                l_error = -OAL_EINVAL;
            }

            break;

        case WLAN_CFGID_AUTH_MODE:
            if (l_value >= WLAN_WITP_ALG_AUTH_BUTT)  /* ������� */
            {
                l_error = -OAL_EINVAL;
            }

            break;

        case WLAN_CFGID_BEACON_INTERVAL:
            if (l_value > WLAN_BEACON_INTVAL_MAX || l_value < WLAN_BEACON_INTVAL_MIN)
            {
                l_error = -OAL_EINVAL;
            }

            break;

        case WLAN_CFGID_TX_CHAIN:
            if (l_value > 0xF)
            {
                l_error = -OAL_EINVAL;
            }

            break;

        case WLAN_CFGID_RX_CHAIN:
            /* ����У�飬ֻ��ȡ0x1(ͨ��1), 0x2(ͨ��2), 0x3(˫ͨ��) */
            if ((l_value < 0x1)||(l_value > 0x3))
            {
                OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_param::input rx_chain invalid %d!}\r\n", l_value);
                l_error = -OAL_EINVAL;
            }

            break;

        case WLAN_CFGID_CONCURRENT:
            if (l_value > WLAN_MAX_ASSOC_USER_CFG || l_value < 1)  /* ������飬�������û���1~200 */
            {
                l_error = -OAL_EINVAL;
            }

            break;

        case WLAN_CFGID_DTIM_PERIOD:
            if (l_value > WLAN_DTIM_PERIOD_MAX || l_value < WLAN_DTIM_PERIOD_MIN)
            {
                OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_param::input dtim_period invalid %d!}\r\n", l_value);
                l_error = -OAL_EINVAL;
            }

            break;

        default:
            break;
    }

    if (0 != l_error)           /* �����쳣 */
    {
        return l_error;
    }

    *((oal_int32 *)(st_write_msg.auc_value)) = l_value;   /* ��дset��Ϣ��payload */

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_param::return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}




OAL_STATIC oal_int32  wal_ioctl_set_wme_params(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_iw, oal_int8 *pc_extra)
{
    oal_int32                       l_error = 0;
    oal_int32                       l_ret;
    oal_int32                      *pl_param;
    oal_int32                       l_subioctl_id;
    oal_int32                       l_ac;
    oal_int32                       l_value;
    wal_msg_write_stru              st_write_msg;
    wal_msg_wmm_stru               *pst_wmm_params;

    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pc_extra))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_wme_params::param null, pst_net_dev = %p, pc_extra = %p.}", pst_net_dev, pc_extra);
        return -OAL_EINVAL;
    }

    /* �豸��up״̬���������ã�������down */
    if (0 != (OAL_IFF_RUNNING & OAL_NETDEVICE_FLAGS(pst_net_dev)))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_wme_params::device is busy, please down it first %d!}\r\n", OAL_NETDEVICE_FLAGS(pst_net_dev));
        return -OAL_EBUSY;
    }

    pl_param      = (oal_int32 *)pc_extra;
    l_subioctl_id = pl_param[0];    /* ��ȡsub-ioctl��ID */
    l_ac          = pl_param[1];
    l_value       = pl_param[2];    /* ��ȡҪ���õ�ֵ */

    OAM_INFO_LOG3(0, OAM_SF_ANY, "{wal_ioctl_set_wme_params::the subioctl_id,l_ac,value is %d, %d, %d!}\r\n", l_subioctl_id, l_ac, l_value);

    /* acȡֵ0~3, value����Ϊ��ֵ */
    if ((l_value < 0) || (l_ac < 0) || (l_ac >= WLAN_WME_AC_BUTT))
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_wme_params::input value is negative %d, %d!}\r\n", l_value, l_ac);
        return -OAL_EINVAL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    switch (l_subioctl_id)                                                      /* ����sub-ioctl id��дWID */
    {
        case WLAN_CFGID_EDCA_TABLE_CWMIN:
            if ((l_value > WLAN_QEDCA_TABLE_CWMIN_MAX) || (l_value < WLAN_QEDCA_TABLE_CWMIN_MIN))
            {
                l_error = -OAL_EINVAL;
            }
            break;

        case WLAN_CFGID_EDCA_TABLE_CWMAX:
            if ((l_value > WLAN_QEDCA_TABLE_CWMAX_MAX) || (l_value < WLAN_QEDCA_TABLE_CWMAX_MIN))
            {
                l_error = -OAL_EINVAL;
            }
            break;

        case WLAN_CFGID_EDCA_TABLE_AIFSN:
            if ((l_value < WLAN_QEDCA_TABLE_AIFSN_MIN) || (l_value > WLAN_QEDCA_TABLE_AIFSN_MAX))
            {
                l_error = -OAL_EINVAL;
            }
            break;

        case WLAN_CFGID_EDCA_TABLE_TXOP_LIMIT:
            if ((l_value > WLAN_QEDCA_TABLE_TXOP_LIMIT_MAX) || (l_value < WLAN_QEDCA_TABLE_TXOP_LIMIT_MIN))
            {
                l_error = -OAL_EINVAL;
            }
            break;

        case WLAN_CFGID_EDCA_TABLE_MSDU_LIFETIME:
            if (l_value > WLAN_QEDCA_TABLE_MSDU_LIFETIME_MAX)
            {
                l_error = -OAL_EINVAL;
            }
            break;

        case WLAN_CFGID_EDCA_TABLE_MANDATORY:
            if ((OAL_TRUE != l_value) &&  (OAL_FALSE != l_value))
            {
                l_error = -OAL_EINVAL;
            }
            break;

        case WLAN_CFGID_QEDCA_TABLE_CWMIN:
            if ((l_value > WLAN_QEDCA_TABLE_CWMIN_MAX) || (l_value < WLAN_QEDCA_TABLE_CWMIN_MIN))
            {
                l_error = -OAL_EINVAL;
            }
            break;

        case WLAN_CFGID_QEDCA_TABLE_CWMAX:
            if ((l_value > WLAN_QEDCA_TABLE_CWMAX_MAX) || (l_value < WLAN_QEDCA_TABLE_CWMAX_MIN))
            {
                l_error = -OAL_EINVAL;
            }
            break;

        case WLAN_CFGID_QEDCA_TABLE_AIFSN:
            if ((l_value < WLAN_QEDCA_TABLE_AIFSN_MIN) || (l_value > WLAN_QEDCA_TABLE_AIFSN_MAX))
            {
                l_error = -OAL_EINVAL;
            }
            break;

        case WLAN_CFGID_QEDCA_TABLE_TXOP_LIMIT:
            if (l_value > WLAN_QEDCA_TABLE_TXOP_LIMIT_MAX)
            {
                l_error = -OAL_EINVAL;
            }
            break;

        case WLAN_CFGID_QEDCA_TABLE_MSDU_LIFETIME:
            if (l_value > WLAN_QEDCA_TABLE_MSDU_LIFETIME_MAX)
            {
                l_error = -OAL_EINVAL;
            }
            break;

        case WLAN_CFGID_QEDCA_TABLE_MANDATORY:
            if ((OAL_TRUE != l_value) &&  (OAL_FALSE != l_value))
            {
                l_error = -OAL_EINVAL;
            }

            break;

        default:
            break;
    }

    if (0 != l_error)           /* �����쳣 */
    {
        return l_error;
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, (oal_uint16)l_subioctl_id, OAL_SIZEOF(wal_msg_wmm_stru));

    pst_wmm_params               = (wal_msg_wmm_stru *)(st_write_msg.auc_value);
    pst_wmm_params->en_cfg_id    = (oal_uint16)l_subioctl_id;
    pst_wmm_params->ul_ac        = (oal_uint32)l_ac;                     /* ��дset��Ϣ��payload */
    pst_wmm_params->ul_value     = (oal_uint32)l_value;                  /* ��дset��Ϣ��payload */

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(wal_msg_wmm_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_wme_params::return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}



OAL_STATIC oal_int32  wal_ioctl_get_wme_params(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_iw, oal_int8 *pc_extra)
{
    oal_int *param;

    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pc_extra))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_get_wme_params::param null, pst_net_dev = %p, pc_extra = %p.}", pst_net_dev, pc_extra);
        return -OAL_EINVAL;
    }

    param = (oal_int *)pc_extra;

    param[0] = (oal_int)wal_config_get_wmm_params(pst_net_dev, (oal_uint8 *)pc_extra);

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


oal_uint8  wal_regdomain_get_band(oal_uint32 ul_start_freq, oal_uint32 ul_end_freq)
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


oal_uint8  wal_regdomain_get_bw(oal_uint8 uc_bw)
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


oal_uint32  wal_regdomain_get_channel_2g(oal_uint32 ul_start_freq, oal_uint32 ul_end_freq)
{
    oal_uint32 ul_freq;
    oal_uint32 ul_i;
    oal_uint32 ul_ch_bmap = 0;

    for (ul_freq = ul_start_freq + 10; ul_freq <= (ul_end_freq - 10); ul_freq++)
    {
        for (ul_i = 0; ul_i < MAC_CHANNEL_FREQ_2_BUTT; ul_i++)
        {
            if (ul_freq == g_ast_freq_map_2g[ul_i].us_freq)
            {
                ul_ch_bmap |= (1 << ul_i);
            }
        }
    }

    return ul_ch_bmap;
}


oal_uint32  wal_regdomain_get_channel_5g(oal_uint32 ul_start_freq, oal_uint32 ul_end_freq)
{
    oal_uint32 ul_freq;
    oal_uint32 ul_i;
    oal_uint32 ul_ch_bmap = 0;

    for (ul_freq = ul_start_freq + 10; ul_freq <= (ul_end_freq - 10); ul_freq += 5)
    {
        for (ul_i = 0; ul_i < MAC_CHANNEL_FREQ_5_BUTT; ul_i++)
        {
            if (ul_freq == g_ast_freq_map_5g[ul_i].us_freq)
            {
                ul_ch_bmap |= (1 << ul_i);
            }
        }
    }

    return ul_ch_bmap;

}


oal_uint32  wal_regdomain_get_channel(oal_uint8 uc_band, oal_uint32 ul_start_freq, oal_uint32 ul_end_freq)
{
    oal_uint32 ul_ch_bmap = 0;;

    switch (uc_band)
    {
        case MAC_RC_START_FREQ_2:
            ul_ch_bmap = wal_regdomain_get_channel_2g(ul_start_freq, ul_end_freq);
            break;

        case MAC_RC_START_FREQ_5:
            ul_ch_bmap = wal_regdomain_get_channel_5g(ul_start_freq, ul_end_freq);
            break;

        default:
            break;
    }

    return ul_ch_bmap;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,3,0))
extern oal_ieee80211_supported_band hi1151_band_2ghz;

oal_uint32 wal_linux_update_wiphy_channel_list_num(oal_net_device_stru *pst_net_dev, oal_wiphy_stru *pst_wiphy)
{
    oal_uint16 us_len;
    oal_uint32 ul_ret;
    mac_vendor_cmd_channel_list_stru st_channel_list;
    mac_vap_stru                    *pst_mac_vap;

    if (pst_wiphy == OAL_PTR_NULL || pst_net_dev == OAL_PTR_NULL)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num::wiphy %p, net_dev %p}", pst_wiphy, pst_net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (pst_mac_vap == OAL_PTR_NULL)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num::NET_DEV_PRIV is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_config_vendor_cmd_get_channel_list(pst_mac_vap, &us_len, (oal_uint8 *)(&st_channel_list));
    if (ul_ret != OAL_SUCC)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num::get_channel_list fail. %d}", ul_ret);
        return ul_ret;
    }

    /* ֻ����2G�ŵ�������5G �ŵ����ڴ���DFS �����Ҵ�����㲢�����⣬����Ҫ�޸� */
    hi1151_band_2ghz.n_channels = st_channel_list.uc_channel_num_2g;

    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num::2g_channel_num = %d, 5g_channel_num = %d}",
                                st_channel_list.uc_channel_num_2g,
                                st_channel_list.uc_channel_num_5g);
    return OAL_SUCC;
}
#endif

OAL_STATIC OAL_INLINE oal_void  wal_get_dfs_domain(mac_regdomain_info_stru *pst_mac_regdom, OAL_CONST oal_int8 *pc_country)
{
    oal_uint32    u_idx;
    oal_uint32    ul_size = OAL_ARRAY_SIZE(g_ast_dfs_domain_table);

    for (u_idx = 0; u_idx < ul_size; u_idx++)
    {
        if (0 == oal_strcmp(g_ast_dfs_domain_table[u_idx].pc_country, pc_country))
        {
            pst_mac_regdom->en_dfs_domain = g_ast_dfs_domain_table[u_idx].en_dfs_domain;

            return;
        }
    }

    pst_mac_regdom->en_dfs_domain = MAC_DFS_DOMAIN_NULL;
}


OAL_STATIC oal_void  wal_regdomain_fill_info(OAL_CONST oal_ieee80211_regdomain_stru *pst_regdom, mac_regdomain_info_stru *pst_mac_regdom)
{
    oal_uint32          ul_i;
    oal_uint32          ul_start;
    oal_uint32          ul_end;
    oal_uint8           uc_band;
    oal_uint8           uc_bw;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    regdomain_enum      en_regdomin;
#endif
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
        uc_band  = wal_regdomain_get_band(ul_start, ul_end);
        pst_mac_regdom->ast_regclass[ul_i].en_start_freq = uc_band;

        /* ��д����������������� */
        uc_bw = (oal_uint8)(pst_regdom->reg_rules[ul_i].freq_range.max_bandwidth_khz / 1000);
        pst_mac_regdom->ast_regclass[ul_i].en_ch_spacing = wal_regdomain_get_bw(uc_bw);

        /* ��д�������ŵ�λͼ */
        pst_mac_regdom->ast_regclass[ul_i].ul_channel_bmap = wal_regdomain_get_channel(uc_band, ul_start, ul_end);

        /* ��ǹ�������Ϊ */
        pst_mac_regdom->ast_regclass[ul_i].uc_behaviour_bmap = 0;

        if (pst_regdom->reg_rules[ul_i].flags & NL80211_RRF_DFS)
        {
            pst_mac_regdom->ast_regclass[ul_i].uc_behaviour_bmap |= MAC_RC_DFS;
        }

        /* ��串���������͹��� */
        pst_mac_regdom->ast_regclass[ul_i].uc_coverage_class = 0;
        pst_mac_regdom->ast_regclass[ul_i].uc_max_reg_tx_pwr = (oal_uint8)(pst_regdom->reg_rules[ul_i].power_rule.max_eirp / 100);
        pst_mac_regdom->ast_regclass[ul_i].uc_max_tx_pwr     = (oal_uint8)(pst_regdom->reg_rules[ul_i].power_rule.max_eirp / 100);

    }
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    en_regdomin = hwifi_get_regdomain_from_country_code(pst_mac_regdom->ac_country);
    /* ����Ƿ�ΪFCC��CE���� */
    pst_mac_regdom->uc_regdomain_type = en_regdomin;
#endif
}


oal_int32  wal_regdomain_update(oal_net_device_stru *pst_net_dev, oal_int8 *pc_country)
{
#ifndef _PRE_SUPPORT_ACS
    oal_uint8                               uc_dev_id;
    mac_device_stru                        *pst_device;
    mac_board_stru                         *pst_hmac_board;
#endif
    OAL_CONST oal_ieee80211_regdomain_stru *pst_regdom;
    oal_uint16                              us_size;
    mac_regdomain_info_stru                *pst_mac_regdom;
    wal_msg_write_stru                      st_write_msg;
    mac_cfg_country_stru                   *pst_param;
    oal_int32                               l_ret;
#ifndef _PRE_PLAT_FEATURE_CUSTOMIZE
    oal_int8                               *pc_current_country;
#endif
    wal_msg_stru                           *pst_rsp_msg = OAL_PTR_NULL;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    oal_int8 old_pc_country[COUNTRY_CODE_LEN] = {'9','9'};
    oal_memcopy(old_pc_country, hwifi_get_country_code(), COUNTRY_CODE_LEN);
    hwifi_set_country_code(pc_country, COUNTRY_CODE_LEN);

    /* ����µĹ�����;ɹ��Ҵ���һ��regdomain����ˢ��RF������ֻ���¹����� */
    if (OAL_TRUE == hwifi_is_regdomain_changed(old_pc_country, pc_country))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update::regdomain changed, refresh rf params.!}\r\n");

        /* ˢ�²���ʧ�ܣ�Ϊ�˱�֤������͹��ʲ�����Ӧ */
        /* �����������ԭ���Ĺ����룬���θ���ʧ�� */
        if (hwifi_force_refresh_rf_params(pst_net_dev) != OAL_SUCC)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY,
                "{wal_regdomain_update::refresh rf(max_txpwr & dbb scale) params failed. Set country back.!}\r\n");
            hwifi_set_country_code(old_pc_country, COUNTRY_CODE_LEN);
        }
    }
#endif

    if (!wal_is_alpha_upper(pc_country[0]) || !wal_is_alpha_upper(pc_country[1]))
    {
        if (('9' == pc_country[0]) && ('9' == pc_country[1]))
        {
            OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update::set regdomain to 99!}\r\n");
        }
        else
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update::country str is invalid!}\r\n");
            return -OAL_EINVAL;
        }
    }

#ifndef _PRE_PLAT_FEATURE_CUSTOMIZE
    pc_current_country = mac_regdomain_get_country();
    /* ��ǰ��������Ҫ���õĹ�����һ�£�ֱ�ӷ��� */
    if ((pc_country[0] == pc_current_country[0])
        && (pc_country[1] == pc_current_country[1]))
    {
        return OAL_SUCC;
    }
#endif /* #ifndef _PRE_PLAT_FEATURE_CUSTOMIZE */

    pst_regdom = wal_regdb_find_db(pc_country);
    if (OAL_PTR_NULL == pst_regdom)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update::no regdomain db was found!}\r\n");
        return -OAL_EINVAL;
    }

    us_size = (oal_uint16)(OAL_SIZEOF(mac_regclass_info_stru) * pst_regdom->n_reg_rules + MAC_RD_INFO_LEN);

    /* �����ڴ��Ź�������Ϣ�����ڴ�ָ����Ϊ�¼�payload����ȥ */
    /* �˴�������ڴ����¼��������ͷ�(hmac_config_set_country) */
    pst_mac_regdom = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, us_size, OAL_TRUE);
    if (OAL_PTR_NULL == pst_mac_regdom)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_regdomain_update::alloc regdom mem fail(size:%d), return null ptr!}\r\n",us_size);
        return -OAL_ENOMEM;
    }

    wal_regdomain_fill_info(pst_regdom, pst_mac_regdom);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_COUNTRY, OAL_SIZEOF(mac_cfg_country_stru));

    /* ��дWID��Ӧ�Ĳ��� */
    pst_param = (mac_cfg_country_stru *)(st_write_msg.auc_value);
    pst_param->p_mac_regdom = pst_mac_regdom;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_country_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_regdomain_update::return err code %d!}\r\n", l_ret);
        if ((-OAL_ETIMEDOUT != l_ret) && (-OAL_EFAUL != l_ret))
        {
            OAL_MEM_FREE(pst_mac_regdom, OAL_TRUE);
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
    hmac_board_get_instance(&pst_hmac_board);
    uc_dev_id = pst_hmac_board->ast_chip[0].auc_device_id[0];
    pst_device = mac_res_get_dev(uc_dev_id);
    if((OAL_PTR_NULL != pst_device) && (OAL_PTR_NULL != pst_device->pst_wiphy))
    {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,3,0))
        
        wal_linux_update_wiphy_channel_list_num(pst_net_dev, pst_device->pst_wiphy);
#endif
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update::update regdom to kernel.}\r\n");
        wal_cfg80211_reset_bands();
        oal_wiphy_apply_custom_regulatory(pst_device->pst_wiphy, pst_regdom);
        
        wal_cfg80211_save_bands();
    }
#endif

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_DFS

oal_int32  wal_regdomain_update_for_dfs(oal_net_device_stru *pst_net_dev, oal_int8 *pc_country)
{
    OAL_CONST oal_ieee80211_regdomain_stru *pst_regdom;
    oal_uint16                              us_size;
    mac_regdomain_info_stru                *pst_mac_regdom;
    wal_msg_write_stru                      st_write_msg;
    mac_dfs_domain_enum_uint8              *pst_param;
    oal_int32                               l_ret;
    oal_int8                               *pc_current_country;

    if (!wal_is_alpha_upper(pc_country[0]) || !wal_is_alpha_upper(pc_country[1]))
    {
        if (('9' == pc_country[0]) && ('9' == pc_country[1]))
        {
            OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs::set regdomain to 99!}\r\n");
        }
        else
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs::country str is invalid!}\r\n");
            return -OAL_EINVAL;
        }
    }

    pc_current_country = mac_regdomain_get_country();

    /* ��ǰ��������Ҫ���õĹ�����һ�£�ֱ�ӷ��� */
    if ((pc_country[0] == pc_current_country[0])
        && (pc_country[1] == pc_current_country[1]))
    {
        return OAL_SUCC;
    }

    pst_regdom = wal_regdb_find_db(pc_country);
    if (OAL_PTR_NULL == pst_regdom)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs::no regdomain db was found!}\r\n");
        return -OAL_EINVAL;
    }

    us_size = (oal_uint16)(OAL_SIZEOF(mac_regclass_info_stru) * pst_regdom->n_reg_rules + MAC_RD_INFO_LEN);

    /* �����ڴ��Ź�������Ϣ,�ڱ������������ͷ� */
    pst_mac_regdom = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, us_size, OAL_TRUE);
    if (OAL_PTR_NULL == pst_mac_regdom)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs::alloc regdom mem fail, return null ptr!}\r\n");
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
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_dfs_domain_enum_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        /*pst_mac_regdom�ڴ棬�˴��ͷ� */
        OAL_MEM_FREE(pst_mac_regdom, OAL_TRUE);
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs::return err code %d!}\r\n", l_ret);
        return l_ret;
    }
    /*pst_mac_regdom�ڴ棬�˴��ͷ� */
    OAL_MEM_FREE(pst_mac_regdom, OAL_TRUE);

    return OAL_SUCC;
}
#endif

oal_uint32  wal_regdomain_update_sta(oal_uint8 uc_vap_id)
{
    oal_int8                      *pc_desired_country;

    oal_net_device_stru           *pst_net_dev;
    oal_int32                      l_ret;
    oal_bool_enum_uint8            us_updata_rd_by_ie_switch = OAL_FALSE;

    hmac_vap_get_updata_rd_by_ie_switch(uc_vap_id,&us_updata_rd_by_ie_switch);

    if(OAL_TRUE== us_updata_rd_by_ie_switch)
    {
        pc_desired_country = hmac_vap_get_desired_country(uc_vap_id);

        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_desired_country))
        {
            OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::pc_desired_country is null ptr!}\r\n");
            return OAL_ERR_CODE_PTR_NULL;
        }


        /* �����Ĺ�����ȫΪ0����ʾ�Զ�AP�Ĺ����벻���ڣ�����sta��ǰĬ�ϵĹ����� */
        if ((0 == pc_desired_country[0]) && (0 == pc_desired_country[1]))
        {
            OAM_INFO_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::ap does not have country ie, use default!}\r\n");
            return OAL_SUCC;
        }

        pst_net_dev = hmac_vap_get_net_device(uc_vap_id);

#ifdef _PRE_WLAN_FEATURE_DFS
        l_ret = wal_regdomain_update_for_dfs(pst_net_dev, pc_desired_country);
        if (l_ret != OAL_SUCC)
        {
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::wal_regdomain_update err code %d!}\r\n", l_ret);
            return OAL_FAIL;
        }
#endif
        l_ret = wal_regdomain_update(pst_net_dev, pc_desired_country);
        if (l_ret != OAL_SUCC)
        {
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::wal_regdomain_update err code %d!}\r\n", l_ret);
            return OAL_FAIL;
        }


        OAM_INFO_LOG2(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::country is %u, %u!}\r\n", (oal_uint8)pc_desired_country[0], (oal_uint8)pc_desired_country[1]);

    }
    else
    {
        OAM_INFO_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::us_updata_rd_by_ie_switch is OAL_FALSE!}\r\n");
    }
    return OAL_SUCC;
}


oal_int32 wal_regdomain_update_country_code(oal_net_device_stru *pst_net_dev, oal_int8 *pc_country)
{
    oal_int32   l_ret;

    if (pst_net_dev == OAL_PTR_NULL || pc_country == OAL_PTR_NULL)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_regdomain_update_country_code::null ptr.net_dev %p, country %p!}",
                        pst_net_dev, pc_country);
        return -OAL_EFAIL;
    }

#ifdef _PRE_WLAN_FEATURE_DFS
    /* ���ù����뵽wifi ���� */
    l_ret = wal_regdomain_update_for_dfs(pst_net_dev, pc_country);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_regdomain_update_country_code::update_for_dfs return err code [%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }
#endif

    l_ret = wal_regdomain_update(pst_net_dev, pc_country);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_regdomain_update_country_code::update return err code [%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

#endif


OAL_STATIC oal_int32  wal_ioctl_setcountry(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_w, oal_int8 *pc_extra)
{
#ifdef _PRE_WLAN_FEATURE_11D
    oal_int32  l_ret;

    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pc_extra))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_setcountry::param null, pst_net_dev = %p, pc_extra = %p.}", pst_net_dev, pc_extra);
        return -OAL_EINVAL;
    }

    /* �豸��up״̬���������ã�������down */
    if (0 != (OAL_IFF_RUNNING & OAL_NETDEVICE_FLAGS(pst_net_dev)))
    {
        OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_ioctl_setcountry::country is %d, %d!}\r\n", OAL_NETDEVICE_FLAGS(pst_net_dev));
        return -OAL_EBUSY;
    }

    l_ret = wal_regdomain_update(pst_net_dev, pc_extra);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_setcountry::regdomain_update return err code %d!}\r\n", l_ret);
        return l_ret;
    }
#else
    OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_ioctl_setcountry:_PRE_WLAN_FEATURE_11D is not define!}\r\n");
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_ioctl_getcountry(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_void *p_w, oal_int8 *pc_extra)
{
#ifdef _PRE_WLAN_FEATURE_11D
    oal_int32                       l_ret;
    wal_msg_query_stru              st_query_msg;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    wal_msg_rsp_stru               *pst_query_rsp_msg;
    mac_cfg_get_country_stru       *pst_get_country;
    oal_iw_point_stru              *pst_w = (oal_iw_point_stru *)p_w;

    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pc_extra))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_getcountry::param null, pst_net_dev = %p, pc_extra = %p.}", pst_net_dev, pc_extra);
        return -OAL_EINVAL;
    }

    /***************************************************************************
       ���¼���wal�㴦��
    ***************************************************************************/
    st_query_msg.en_wid = WLAN_CFGID_COUNTRY;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                              WAL_MSG_TYPE_QUERY,
                              WAL_MSG_WID_LENGTH,
                              (oal_uint8 *)&st_query_msg,
                              OAL_TRUE,
                              &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_getcountry:: wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);

    /* ҵ���� */
    pst_get_country = (mac_cfg_get_country_stru*)(pst_query_rsp_msg->auc_value);

    oal_memcopy(pc_extra, pst_get_country->ac_country, WLAN_COUNTRY_STR_LEN);
    pst_w->length = WLAN_COUNTRY_STR_LEN;

    oal_free(pst_rsp_msg);

#endif

    return OAL_SUCC;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)


oal_int32  wal_set_random_mac_to_mib(oal_net_device_stru *pst_net_dev)
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

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL == pst_mac_vap)
    {
        /* ��ʱ��ӣ��Ժ�ɾ����add:20151116����Ч��1month */
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib::pst_mac_vap NULL}");
        oal_msleep(500);
        pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
        if(NULL == pst_mac_vap)
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib::pst_mac_vap NULL}");
            return OAL_FAIL;
        }
        /* ��ʱ��ӣ��Ժ�ɾ�� */
    }

    if(OAL_PTR_NULL == pst_mac_vap->pst_mib_info)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_random_mac_to_mib::vap->mib_info is NULL !}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    /* ��ȡwlan0 MAC ��ַ������p2p0/p2p-p2p0 MAC ��ַ */
    pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib::pst_mac_device NULL, device_id:%d}",pst_mac_vap->uc_device_id);
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
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib() pst_primary_net_device; addr is null}\r\n");
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
            oal_memcopy(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID,
                        OAL_NETDEVICE_MAC_ADDR(pst_net_dev),
                        WLAN_MAC_ADDR_LEN);

#else
            /* ����P2P device MAC ��ַ��������mac ��ַbit ����Ϊ1 */
            oal_memcopy(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID,
                        auc_primary_mac_addr,
                        WLAN_MAC_ADDR_LEN);
            pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID[0] |= 0x02;
#endif
            break;
        case NL80211_IFTYPE_P2P_CLIENT:
            en_p2p_mode = WLAN_P2P_CL_MODE;
            /* ����P2P interface MAC ��ַ */
            oal_memcopy(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID,
                        auc_primary_mac_addr,
                        WLAN_MAC_ADDR_LEN);
            pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID[0] |= 0x02;
            pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID[4] ^= 0x80;
            break;
        case NL80211_IFTYPE_P2P_GO:
            en_p2p_mode = WLAN_P2P_GO_MODE;
            /* ����P2P interface MAC ��ַ */
            oal_memcopy(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID,
                        auc_primary_mac_addr,
                        WLAN_MAC_ADDR_LEN);
            pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID[0] |= 0x02;
            pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID[4] ^= 0x80;
            break;
        default:
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
            if(0 == (oal_strcmp("p2p0", pst_net_dev->name)))
            {
                en_p2p_mode = WLAN_P2P_DEV_MODE;
                 /* ����P2P device MAC ��ַ��������mac ��ַbit ����Ϊ1 */
                oal_memcopy(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID,
                            OAL_NETDEVICE_MAC_ADDR(pst_net_dev),
                            WLAN_MAC_ADDR_LEN);
                break;
            }

            oal_memcopy(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID,
                        OAL_NETDEVICE_MAC_ADDR(pst_net_dev),
                        WLAN_MAC_ADDR_LEN);
#else
            /* random mac will be used. hi1102-cb (#include <linux/etherdevice.h>)    */
            oal_random_ether_addr(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);
            pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID[0]&=(~0x02);
            pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID[1]=0x11;
            pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID[2]=0x02;
#endif
            break;
    }
#else
    /* random mac will be used. hi1102-cb (#include <linux/etherdevice.h>)    */
    oal_random_ether_addr(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID[0]&=(~0x02);
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID[1]=0x11;
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID[2]=0x02;
#endif

    /* send the random mac to dmac */
    /***************************************************************************
        ���¼���wal�㴦��   copy from wal_netdev_set_mac_addr()
        gong TBD : ��Ϊ����ͨ�õ�config�ӿ�
    ***************************************************************************/
    ul_ret = wal_alloc_cfg_event(pst_net_dev, &pst_event_mem, NULL, &pst_cfg_msg, (WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_staion_id_param_stru)));     /* �����¼� */
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib() fail; return %d!}\r\n", ul_ret);
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
        puc_mac_addr = pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID;
    }
    else
#endif
    {
        puc_mac_addr = pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID;
    }
    oal_set_mac_addr(pst_param->auc_station_id, puc_mac_addr);

    OAL_IO_PRINT("wal_set_random_mac_to_mib,mac is:%.2x:%.2x:%.2x\n",
                  puc_mac_addr[0], puc_mac_addr[1], puc_mac_addr[2]);

    frw_event_dispatch_event(pst_event_mem);    /* �ַ��¼� */
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    puc_para = &ac_arg[0];

#ifdef _PRE_WLAN_FEATURE_DFS
    l_ret = wal_regdomain_update_for_dfs(pst_net_dev, puc_para);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::regdomain_update return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
#endif

    l_ret = wal_regdomain_update(pst_net_dev, puc_para);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::regdomain_update return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }


#else
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_setcountry::_PRE_WLAN_FEATURE_11D is not define!}\r\n");

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

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH ,
                               (oal_uint8 *)&st_query_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
       OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_getcountry::wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
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
    oal_uint32                      *pul_param;
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

        pst_hmac_user_tmp = mac_res_get_hmac_user(pst_user_tmp->us_assoc_id);
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

        l_ret = wal_send_cfg_event(pst_hmac_vap->pst_net_device,
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

        l_ret = wal_send_cfg_event(pst_hmac_vap->pst_net_device,
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

        pst_hmac_user_tmp = mac_res_get_hmac_user(pst_user_tmp->us_assoc_id);
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
                      (pst_user_tmp->uc_avail_num_spatial_stream+1),   /* NSS */
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

    l_ret = wal_send_cfg_event(pst_hmac_vap->pst_net_device,
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

    return len;
}


OAL_STATIC int  wal_read_rf_info_proc(char *page, char **start, off_t off,
                   int count, int *eof, void *data)
{
    int                  len;
    mac_vap_stru        *pst_mac_vap = (mac_vap_stru *)data;

    len = OAL_SPRINTF(page, PAGE_SIZE, "rf info:\n  channel_num: %d\n",
                        pst_mac_vap->st_channel.uc_chan_number);

    return len;
}
#endif



OAL_STATIC oal_void wal_add_vap_proc_file(mac_vap_stru *pst_mac_vap, oal_int8 *pc_name)
{
#if  (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,34))
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
#endif
}


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
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8    en_p2p_mode = WLAN_LEGACY_VAP_MODE;
#endif


    /* pc_paramָ���´�����net_device��name, ����ȡ����ŵ�ac_name�� */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ul_ret);
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_mode, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ul_ret);
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
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    else if (0 == oal_strcmp("msta", ac_mode))  // create main sta
    {
        en_mode       = WLAN_VAP_MODE_BSS_STA;
        en_psta_mode  = PSTA_MODE_MSTA;
        if (OAL_SUCC == wal_get_cmd_one_arg(pc_param + ul_off_set, ac_repid_str, &ul_off_set))
        {
            uc_rep_id = (oal_uint8)oal_atoi(ac_repid_str);
        }
    }
    else if (0 == oal_strcmp("vsta", ac_mode))  // create virtaul proxysta sta
    {
        en_mode       = WLAN_VAP_MODE_BSS_STA;
        en_psta_mode  = PSTA_MODE_VSTA;
        if (OAL_SUCC == wal_get_cmd_one_arg(pc_param + ul_off_set, ac_repid_str, &ul_off_set))
        {
            uc_rep_id = (oal_uint8)oal_atoi(ac_repid_str);
        }
    }
    else if (0 == oal_strcmp("pbss", ac_mode))  // create virtaul proxysta sta
    {
        en_mode       = WLAN_VAP_MODE_BSS_AP;
        en_psta_mode  = PSTA_MODE_PBSS;
        if (OAL_SUCC == wal_get_cmd_one_arg(pc_param + ul_off_set, ac_repid_str, &ul_off_set))
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
    pst_mac_device  = mac_res_get_dev(pst_cfg_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_WARNING_LOG0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_add_vap::pst_mac_device is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if defined(_PRE_WLAN_FEATURE_FLOWCTL)
    pst_net_dev = oal_net_alloc_netdev_mqs(OAL_SIZEOF(oal_netdev_priv_stru), ac_name, oal_ether_setup, WAL_NETDEV_SUBQUEUE_MAX_NUM, 1);    /* �˺�����һ����δ���˽�г��� */
#elif defined(_PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL)
    pst_net_dev = oal_net_alloc_netdev_mqs(OAL_SIZEOF(oal_netdev_priv_stru), ac_name, oal_ether_setup, WLAN_NET_QUEUE_BUTT, 1);    /* �˺�����һ����δ���˽�г��� */
#else
    pst_net_dev = oal_net_alloc_netdev(OAL_SIZEOF(oal_netdev_priv_stru), ac_name, oal_ether_setup);    /* �˺�����һ����δ���˽�г���*/
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
    pst_net_dev->wireless_handlers             = &g_st_iw_handler_def;
#endif
    /* OAL_NETDEVICE_OPS(pst_net_dev)             = &g_st_wal_net_dev_ops; */
    pst_net_dev->netdev_ops                    = &g_st_wal_net_dev_ops;

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
#ifdef _PRE_WLAN_FEATURE_P2P
   ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode  = en_p2p_mode;
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->bit_11ac2g_enable = (oal_uint8)!!hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_11AC2G_ENABLE);
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->bit_disable_capab_2ght40 = g_st_wlan_customize.uc_disable_capab_2ght40;
#endif
    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
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
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
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

    if(OAL_SUCC != wal_set_random_mac_to_mib(pst_net_dev))
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
    if (en_p2p_mode == WLAN_P2P_DEV_MODE)
    {
        oal_set_mac_addr((oal_uint8 *)OAL_NETDEVICE_MAC_ADDR(pst_net_dev), pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID);

        pst_mac_device->st_p2p_info.uc_p2p0_vap_idx = pst_mac_vap->uc_vap_id;
    }
    else
#endif
    {
        oal_set_mac_addr((oal_uint8 *)OAL_NETDEVICE_MAC_ADDR(pst_net_dev), pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);
    }


    /* ע��net_device */
    ul_ret = (oal_uint32)oal_net_register_netdev(pst_net_dev);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_add_vap::oal_net_register_netdev return error code %d!}\r\n", ul_ret);

        /* �쳣�����ͷ��ڴ� */
        /* ��ɾ��vap�¼��ͷŸ������vap  */
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DEL_VAP, OAL_SIZEOF(mac_cfg_del_vap_param_stru));

        l_ret = wal_send_cfg_event(pst_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_del_vap_param_stru),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_TRUE,
                                   &pst_rsp_msg);

        if (OAL_SUCC != wal_check_and_release_msg_resp(pst_rsp_msg))
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_check_and_release_msg_resp fail.}");
        }
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_send_cfg_event fail,err code %d!}\r\n",l_ret);
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
            OAL_IO_PRINT("wal_netdev_open::spe init failed!!\n");
        }
    }
#endif

    /* ����VAP��Ӧ��proc�ļ� */
    wal_add_vap_proc_file(pst_mac_vap, ac_name);

    return OAL_SUCC;
}



OAL_STATIC oal_void wal_del_vap_proc_file(oal_net_device_stru *pst_net_dev)
{
#if  (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,34))
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
#endif
}


oal_uint32  wal_hipriv_del_vap(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru           st_write_msg;
    wal_msg_stru               *pst_rsp_msg = OAL_PTR_NULL;
    oal_int32                    l_ret;
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_wireless_dev_stru       *pst_wdev;
    wlan_p2p_mode_enum_uint8     en_p2p_mode = WLAN_LEGACY_VAP_MODE;
#endif

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pc_param)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_hipriv_del_vap::pst_net_dev or pc_param null ptr error %d, %d!}\r\n",
                       pst_net_dev, pc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* �豸��up״̬������ɾ����������down */
    if (OAL_UNLIKELY(0 != (OAL_IFF_RUNNING & OAL_NETDEVICE_FLAGS(pst_net_dev))))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_del_vap::device is busy, please down it first %d!}\r\n", OAL_NETDEVICE_FLAGS(pst_net_dev));
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* E5 SPE module relation */
#if (defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT))
    if(spe_hook.is_enable && spe_hook.is_enable())
    {
         wal_netdev_spe_exit(pst_net_dev);
    }
#endif

    /* ɾ��vap��Ӧ��proc�ļ� */
    wal_del_vap_proc_file(pst_net_dev);

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    //ɾ��vap ʱ��Ҫ��������ֵ��
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wdev = pst_net_dev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_del_vap::wal_wireless_iftype_to_mac_p2p_mode return BUFF}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;

#endif

    OAL_MEM_FREE(OAL_NETDEVICE_WDEV(pst_net_dev), OAL_TRUE);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DEL_VAP, OAL_SIZEOF(mac_cfg_del_vap_param_stru));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_del_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_SUCC != wal_check_and_release_msg_resp(pst_rsp_msg))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_del_vap::wal_check_and_release_msg_resp fail}");
    }

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_del_vap::return err code %d}\r\n", l_ret);
        /* ȥע�� */
        oal_net_unregister_netdev(pst_net_dev);
        return (oal_uint32)l_ret;
    }

    /* ȥע�� */
    oal_net_unregister_netdev(pst_net_dev);

    return OAL_SUCC;
}


oal_uint32  wal_hipriv_vap_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru           st_write_msg;
    oal_int32                    l_ret;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_VAP_INFO, OAL_SIZEOF(oal_int32));

    l_ret = wal_send_cfg_event(pst_net_dev,
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
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_vap_info::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;

}
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
#define MAX_HIPRIV_IP_FILTER_BTABLE_SIZE 129

oal_uint32  wal_hipriv_set_ip_filter(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_cmd, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::wal_get_cmd_one_arg vap name return err_code %d!}", ul_ret);
        return ul_ret;
    }

    if(0 == oal_strncmp(ac_cmd, CMD_CLEAR_RX_FILTERS, OAL_STRLEN(CMD_CLEAR_RX_FILTERS)))
    {
        /* ����� */
        ul_ret = (oal_uint32)wal_clear_ip_filter();
        return ul_ret;
    }

    pc_param += ul_off_set;
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_cmd_param, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::get cmd_param return err_code %d!}", ul_ret);
        return ul_ret;
    }

    if(0 == oal_strncmp(ac_cmd, CMD_SET_RX_FILTER_ENABLE, OAL_STRLEN(CMD_SET_RX_FILTER_ENABLE)))
    {
        /* ʹ��/�رչ��� */
        l_enable = oal_atoi(ac_cmd_param);
        ul_ret = (oal_uint32)wal_set_ip_filter_enable(l_enable);
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

            /* ��ȡprotocolX*/
            pc_param += ul_off_set;
            ul_ret = wal_get_cmd_one_arg(pc_param, ac_cmd_param, &ul_off_set);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::get item_params return err_code %d!}", ul_ret);
                return ul_ret;
            }
            ast_items[l_items_idx].protocol = (oal_uint8)oal_atoi(ac_cmd_param);


            /* ��ȡportX*/
            pc_param += ul_off_set;
            ul_ret = wal_get_cmd_one_arg(pc_param, ac_cmd_param, &ul_off_set);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::get item_params return err_code %d!}", ul_ret);
                return ul_ret;
            }
            ast_items[l_items_idx].port = (oal_uint16)oal_atoi(ac_cmd_param);
        }

        ul_ret = (oal_uint32)wal_add_ip_filter_items(ast_items, l_items_cnt);
        return ul_ret;
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::cmd_one_arg no support!}");
        return OAL_FAIL;
    }
}

#endif //_PRE_WLAN_FEATURE_IP_FILTER


OAL_STATIC oal_uint32  wal_hipriv_set_2040_coext_support(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    oal_uint8                       uc_csp;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_2040_coext_support::wal_get_cmd_one_arg return err_code %d!}\r\n", ul_ret);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info::wal_get_cmd_one_arg return err_code %d!}\r\n", ul_ret);
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

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info::wal_get_cmd_one_arg return err_code %d!}\r\n", ul_ret);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_rx_fcs_info_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    wal_hipriv_wait_rsp(pst_net_dev, pc_param);
#endif

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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_param, &ul_off_set);
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

    ul_ret = oam_log_set_vap_level(pst_mac_vap->uc_vap_id, en_level_val);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
// Ŀǰ֧��02 device ����log ���� ���������ĺϲ�����
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_LOG_LEVEL, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = en_level_val;
    ul_ret |= (oal_uint32)wal_send_cfg_event(pst_net_dev,
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_green_ap_en::wal_get_cmd_one_arg return err_code[%d]}\r\n", ul_ret);
        return ul_ret;
    }

    /* ��Խ������Ĳ�ͬ�����eventģ����в�ͬ������ */

    l_tmp = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_GREEN_AP_EN, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_tmp;  /* ��������������� */

    l_ret = wal_send_cfg_event(pst_net_dev,
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
#ifdef _PRE_WLAN_FEATURE_BTCOEX

OAL_STATIC oal_uint32 wal_hipriv_btcoex_status_print(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;

    OAL_MEMZERO((oal_uint8*)&st_write_msg, OAL_SIZEOF(st_write_msg));

    /* sh hipriv.sh "vap_name coex_print" */

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_BTCOEX_STATUS_PRINT, OAL_SIZEOF(oal_uint32));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_TXOP, "{wal_hipriv_btcoex_status_print::return err code[%d]!}\r\n", l_ret);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_aifsn_cfg::get wfa switch fail, return err_code[%d]!}", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_edca_cfg.en_switch = (oal_bool_enum_uint8)oal_atoi(ac_name);

    /* ��ȡac */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
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
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_cw_cfg::get wfa switch fail, return err_code[%d]!}", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_edca_cfg.en_switch = (oal_bool_enum_uint8)oal_atoi(ac_name);

    /* ��ȡac */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
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
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_uapsd_cap::wal_get_cmd_one_arg return err_code[%d]}\r\n", ul_ret);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_user::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    OAL_MEMZERO((oal_uint8*)&st_add_user_param, OAL_SIZEOF(st_add_user_param));
    oal_strtoaddr(ac_name, st_add_user_param.auc_mac_addr);
    /* ƫ�ƣ�ȡ��һ������ */
    pc_param = pc_param + ul_off_set;

    /* ��ȡ�û���HT��ʶ */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_user::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_del_user::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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

    l_ret = (oal_int32)mac_vap_find_user_by_macaddr(pst_mac_vap, auc_mac_addr, &us_user_idx);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_user_info::no such user!}\r\n");
        return OAL_FAIL;
    }

    /* ��������������� */
    pst_user_info_param              = (mac_cfg_user_info_param_stru *)(st_write_msg.auc_value);
    pst_user_info_param->us_user_idx = us_user_idx;

    l_ret = wal_send_cfg_event(pst_net_dev,
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcast_data_dscr_param::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    /* ������������һ���ֶ� */
    for (en_param_index = 0; en_param_index < WAL_DSCR_PARAM_BUTT; en_param_index++)
    {
        if(!oal_strcmp(pauc_tx_dscr_param_name[en_param_index], ac_arg))
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

    /* ����Ҫ����Ϊ����ֵ */
    pst_set_dscr_param->l_value = oal_strtol(pc_param, OAL_PTR_NULL, 0);

    /* �鲥����֡���������� tpye = MAC_VAP_CONFIG_MCAST_DATA */
    pst_set_dscr_param->en_type = MAC_VAP_CONFIG_MCAST_DATA;

    l_ret = wal_send_cfg_event(pst_net_dev,
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



oal_uint32  wal_hipriv_set_rate(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
//#ifdef _PRE_WLAN_CHIP_TEST
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    mac_cfg_non_ht_rate_stru         *pst_set_rate_param;
    wlan_legacy_rate_value_enum_uint8  en_rate_index;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_RATE, OAL_SIZEOF(mac_cfg_non_ht_rate_stru));

    /* ��������������������� */
    pst_set_rate_param = (mac_cfg_non_ht_rate_stru *)(st_write_msg.auc_value);

    /* ��ȡ����ֵ�ַ��� */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_rate::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_rate::invalid rate!}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* ����Ҫ����Ϊ����ֵ */
    pst_set_rate_param->en_rate = en_rate_index;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_non_ht_rate_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    wal_hipriv_wait_rsp(pst_net_dev, pc_param);
#endif

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_rate::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
//#endif  /* _PRE_WLAN_CHIP_TEST */
    return OAL_SUCC;
}


oal_uint32  wal_hipriv_set_mcs(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
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

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_MCS, OAL_SIZEOF(mac_cfg_tx_comp_stru));

    /* ��������������������� */
    pst_set_mcs_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    /* ��ȡ����ֵ�ַ��� */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_mcs::input illegal!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* ����Ҫ����Ϊ����ֵ */
    l_mcs = oal_atoi(ac_arg);

    if (l_mcs < WAL_HIPRIV_HT_MCS_MIN || l_mcs > WAL_HIPRIV_HT_MCS_MAX)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs::input val out of range [%d]!}\r\n", l_mcs);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_mcs_param->uc_param = (oal_uint8)l_mcs;
    pst_set_mcs_param->en_protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;

    l_ret = wal_send_cfg_event(pst_net_dev,
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
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
//#endif  /* _PRE_WLAN_CHIP_TEST */
    return OAL_SUCC;
}


oal_uint32  wal_hipriv_set_mcsac(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
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

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_MCSAC, OAL_SIZEOF(mac_cfg_tx_comp_stru));

    /* ��������������������� */
    pst_set_mcs_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    /* ��ȡ����ֵ�ַ��� */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsac::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_mcsac::input illegal!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* ����Ҫ����Ϊ����ֵ */
    l_mcs = oal_atoi(ac_arg);

    if (l_mcs < WAL_HIPRIV_VHT_MCS_MIN || l_mcs > WAL_HIPRIV_VHT_MCS_MAX)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs::input val out of range [%d]!}\r\n", l_mcs);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_mcs_param->uc_param = (oal_uint8)l_mcs;
    pst_set_mcs_param->en_protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;

    l_ret = wal_send_cfg_event(pst_net_dev,
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
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsac::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
//#endif  /* _PRE_WLAN_CHIP_TEST */
    return OAL_SUCC;
}



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

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_BW, OAL_SIZEOF(mac_cfg_tx_comp_stru));

    /* ��������������������� */
    pst_set_bw_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    /* ��ȡ����ֵ�ַ��� */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_bw::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_bw::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
//#endif  /* _PRE_WLAN_CHIP_TEST */
    return OAL_SUCC;
}



OAL_STATIC oal_uint32  wal_hipriv_set_pow_rf_ctl(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    oal_bool_enum_uint8              en_rf_regctl = OAL_TRUE;
    oal_uint8                        uc_arg_val   = 0;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    /* ��ȡ����ֵ�ַ��� */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_pow_rf_ctl::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    uc_arg_val = (oal_uint8)oal_atoi(ac_arg);

    if (uc_arg_val != 0)
    {
        en_rf_regctl = OAL_TRUE;
    }
    else
    {
        en_rf_regctl = OAL_FALSE;
    }
    OAM_WARNING_LOG1(0,OAM_SF_ANY,"wal_hipriv_set_pow_rf_ctl:en_rf_regctl=%d",en_rf_regctl);
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_POW_RF_CTL, OAL_SIZEOF(oal_bool_enum_uint8));
    st_write_msg.auc_value[0] = en_rf_regctl;
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_bool_enum_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_pow_rf_ctl::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
    return OAL_SUCC;
}


#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
OAL_STATIC oal_uint32  wal_hipriv_always_tx_1102(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret;
    mac_cfg_tx_comp_stru             *pst_set_bcast_param;
    oal_int8                         ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_switch_enum_uint8            en_tx_flag = OAL_SWITCH_OFF;
    mac_rf_payload_enum_uint8        en_payload_flag = RF_PAYLOAD_ALL_ZERO;
    oal_uint32                       ul_len = 0;
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_ALWAYS_TX_1102, OAL_SIZEOF(mac_cfg_tx_comp_stru));

    /* ��������������������� */
    pst_set_bcast_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_packet_xmit::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }
    en_tx_flag = (oal_uint8)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    if (OAL_SWITCH_OFF != en_tx_flag)
    {
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
             OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_packet_xmit::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
             return ul_ret;
        }
        pc_param = pc_param + ul_off_set;
        en_payload_flag = (oal_uint8)oal_atoi(ac_name);
        if(en_payload_flag >= RF_PAYLOAD_BUTT)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::payload flag err[%d]!}\r\n", en_payload_flag);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
             OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_packet_xmit::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
             return ul_ret;
        }
        ul_len = (oal_uint16)oal_atoi(ac_name);
        if(65535 < ul_len)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::len [%u] overflow!}\r\n", ul_len);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
        pc_param += ul_off_set;


    }

    /* �رյ�����²���Ҫ��������Ĳ��� */
    pst_set_bcast_param->en_payload_flag = en_payload_flag;
    pst_set_bcast_param->ul_payload_len = ul_len;
    pst_set_bcast_param->uc_param = en_tx_flag;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_tx_comp_stru),
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

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_num::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }
    *pul_num = (oal_uint32)oal_atoi(ac_name);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx_num::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;

}

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

    /* ��ȡ����ģʽ���ر�־ */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_rx::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_rx::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pcie_pm_level::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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


OAL_STATIC oal_int32  wal_ioctl_get_iwname(oal_net_device_stru *pst_net_dev, oal_iw_request_info_stru *pst_info, oal_int8* pc_name, oal_int8* pc_extra)
{
    oal_int8    ac_iwname[] = "IEEE 802.11";

    if ((OAL_PTR_NULL == pst_net_dev) || (OAL_PTR_NULL == pc_name))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_ioctl_get_iwname::param null, pst_net_dev = %p, pc_name = %p.}", pst_net_dev, pc_name);
        return -OAL_EINVAL;
    }

    oal_memcopy(pc_name, ac_iwname, OAL_SIZEOF(ac_iwname));

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

            return OAL_SUCC;
        }
    }

#ifdef _PRE_WLAN_CFGID_DEBUG
    for (en_cmd_idx = 0; en_cmd_idx < wal_hipriv_get_debug_cmd_size(); en_cmd_idx++)
    {
        l_ret = oal_strcmp(g_ast_hipriv_cmd_debug[en_cmd_idx].pc_cmd_name, pc_cmd_name);

        if (0 == l_ret)
        {
            *pst_cmd_id = (wal_hipriv_cmd_entry_stru*)&g_ast_hipriv_cmd_debug[en_cmd_idx];

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

    ul_ret = wal_get_cmd_one_arg(pc_cmd, ac_dev_name, pul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_net_dev::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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

    ul_ret = wal_get_cmd_one_arg(pc_cmd, ac_cmd_name, pul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_id::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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


oal_uint32  wal_hipriv_parse_cmd(oal_int8 *pc_cmd)
{
    oal_net_device_stru        *pst_net_dev;
    wal_hipriv_cmd_entry_stru * pst_hipriv_cmd_entry = NULL;
    oal_uint32                  ul_off_set = 0;
    oal_uint32                  ul_ret;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pc_cmd))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd::pc_cmd null ptr error!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
        cmd��ʽԼ��
        �����豸�� ����      ����   Hisilicon0 create vap0
        1~15Byte   1~15Byte
    **************************** ***********************************************/
    ul_ret = wal_hipriv_get_cmd_net_dev(pc_cmd, &pst_net_dev, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd::wal_hipriv_get_cmd_net_dev return error code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    pc_cmd += ul_off_set;
    ul_ret  = wal_hipriv_get_cmd_id(pc_cmd, &pst_hipriv_cmd_entry, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd::wal_hipriv_get_cmd_id return error code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    pc_cmd += ul_off_set;

    /* ���������Ӧ�ĺ��� */
    ul_ret = pst_hipriv_cmd_entry->p_func(pst_net_dev, pc_cmd);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd::g_ast_hipriv_cmd return error code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

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

    //printk("sys write: %s\n",pc_cmd);
    OAM_IO_PRINTK(" %s\n", pc_cmd);

    ul_ret = wal_hipriv_parse_cmd(pc_cmd);
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
    for (ul_cmd_idx = 0; ul_cmd_idx < wal_hipriv_get_debug_cmd_size(); ul_cmd_idx++)
    {
        buff_index += OAL_SPRINTF(pc_buffer+buff_index,(SYS_READ_MAX_STRING_LEN-buff_index),
            "\t%s\n",g_ast_hipriv_cmd_debug[ul_cmd_idx].pc_cmd_name);

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
        OAL_IO_PRINT("Success\n");
        oal_copy_to_user((oal_void *)pst_hmac_vap->st_hipriv_ack_stats.pc_buffer, (oal_void *)pst_hmac_vap->st_hipriv_ack_stats.auc_data, HMAC_HIPRIV_ACK_BUF_SIZE);
    }
#endif

    return OAL_TRUE;
}


OAL_STATIC oal_int32  wal_hipriv_proc_write(oal_file_stru *pst_file, oal_int8 *pc_buffer, oal_uint32 ul_len, oal_void *p_data)
#else
OAL_STATIC oal_int32  wal_hipriv_proc_write(oal_file_stru *pst_file, const oal_int8 *pc_buffer, oal_uint32 ul_len, oal_void *p_data)
#endif
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
    pst_hmac_vap->st_hipriv_ack_stats.pc_buffer = pc_buffer;
    pst_hmac_vap->st_hipriv_ack_stats.auc_data[0] = 0xFF;
    pst_hmac_vap->st_hipriv_ack_stats.auc_data[1] = 0;
    pst_hmac_vap->st_hipriv_ack_stats.auc_data[2] = 0;
#endif

    ul_ret = wal_hipriv_parse_cmd(pc_cmd);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_proc_write::parse cmd return err code[%d]!}\r\n", ul_ret);
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (_PRE_TEST_MODE_UT != _PRE_TEST_MODE)
        oal_copy_to_user((oal_void *)pc_buffer, (oal_void *)pst_hmac_vap->st_hipriv_ack_stats.auc_data, ul_len);
#endif
    }

    OAL_MEM_FREE(pc_cmd, OAL_TRUE);

    return (oal_int32)ul_len;

}


oal_uint32  wal_hipriv_create_proc(oal_void *p_proc_arg)
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
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_create_proc::oal_create_proc_entry return null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_pst_proc_entry->data  = p_proc_arg;
    g_pst_proc_entry->nlink = 1;        /* linux����procĬ��ֵ */
    g_pst_proc_entry->read_proc  = OAL_PTR_NULL;

    g_pst_proc_entry->write_proc = (write_proc_t *)wal_hipriv_proc_write;
#endif

    /* hi1102-cb add sys for 51/02 */
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    gp_sys_kobject = oal_get_sysfs_root_object();
    if(NULL == gp_sys_kobject)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_create_proc::get sysfs root object failed!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = (oal_uint32)sysfs_create_group(gp_sys_kobject,&hipriv_attribute_group);
    if (ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_create_proc::hipriv_attribute_group create failed!}");
        ul_ret = OAL_ERR_CODE_PTR_NULL;
        return ul_ret;
    }
#endif

    return OAL_SUCC;
}


oal_uint32  wal_hipriv_remove_proc(void)
{
/* ж��ʱɾ��sysfs */

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    if(NULL != gp_sys_kobject)
    {
        sysfs_remove_group(gp_sys_kobject, &hipriv_attribute_group);
        gp_sys_kobject = NULL;
    }
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

    oal_remove_proc_entry(WAL_HIPRIV_PROC_ENTRY_NAME, NULL);
#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)

    oal_remove_proc_entry(WAL_HIPRIV_PROC_ENTRY_NAME, g_pst_proc_entry);
#endif
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sdio_flowctrl:: pc_param overlength is %d}\n", OAL_STRLEN(pc_param));
        return OAL_FAIL;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    oal_memcopy(st_write_msg.auc_value, pc_param, OAL_STRLEN(pc_param));

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SDIO_FLOWCTRL, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
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

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
                       WAL_MSG_TYPE_WRITE,
                       WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                       (oal_uint8 *)&st_write_msg,
                       OAL_FALSE,
                       OAL_PTR_NULL);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0,OAM_SF_CFG,"{wal_hipriv_set_regdomain_pwr::wal_send_cfg_event fail.return err code %d}",l_ret);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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


oal_uint32  wal_hipriv_alg_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    mac_ioctl_alg_param_stru       *pst_alg_param = OAL_PTR_NULL;
    wal_ioctl_alg_cfg_stru          st_alg_cfg;
    oal_uint8                       uc_map_index = 0;
    oal_int32                       l_ret;

    pst_alg_param = (mac_ioctl_alg_param_stru *)(st_write_msg.auc_value);

    /* ��ȡ���ò������� */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    /* Ѱ��ƥ������� */
    st_alg_cfg = g_ast_alg_cfg_map[0];
    while(OAL_PTR_NULL != st_alg_cfg.pc_name)
    {
        if (0 == oal_strcmp(st_alg_cfg.pc_name, ac_name))
        {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map[++uc_map_index];
    }

    /* û���ҵ���Ӧ������򱨴� */
    if( OAL_PTR_NULL == st_alg_cfg.pc_name)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* ��¼�����Ӧ��ö��ֵ */
    pst_alg_param->en_alg_cfg = g_ast_alg_cfg_map[uc_map_index].en_alg_cfg;

    /* ��ȡ��������ֵ */
    ul_ret = wal_get_cmd_one_arg(pc_param + ul_off_set, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    /* ��¼��������ֵ */
    pst_alg_param->ul_value = (oal_uint32)oal_atoi(ac_name);

    /***************************************************************************
                             ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ALG_PARAM, OAL_SIZEOF(mac_ioctl_alg_param_stru));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_ioctl_alg_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        return (oal_uint32)l_ret;
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param = pc_param + ul_off_set;

    /* Ѱ��ƥ������� */
    st_alg_cfg = g_ast_alg_cfg_map[0];
    while(OAL_PTR_NULL != st_alg_cfg.pc_name)
    {
        if (0 == oal_strcmp(st_alg_cfg.pc_name, ac_name))
        {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map[++uc_map_index];
    }

    /* û���ҵ���Ӧ������򱨴� */
    if( OAL_PTR_NULL == st_alg_cfg.pc_name)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* ��¼�����Ӧ��ö��ֵ */
    pst_alg_tpc_log_param->en_alg_cfg = g_ast_alg_cfg_map[uc_map_index].en_alg_cfg;

    /* ���ֻ�ȡ�ض�֡���ʺ�ͳ����־�����:��ȡ����ֻ���ȡ֡���� */
    if (MAC_ALG_CFG_TPC_GET_FRAME_POW == pst_alg_tpc_log_param->en_alg_cfg)
    {
        /* ��ȡ���ò������� */
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
            return ul_ret;
        }
        /* ��¼�����Ӧ��֡���� */
        pst_alg_tpc_log_param->pc_frame_name = ac_name;
        en_stop_flag = OAL_TRUE;
    }
    else
    {
        ul_ret = wal_hipriv_get_mac_addr(pc_param, pst_alg_tpc_log_param->auc_mac_addr, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::wal_hipriv_get_mac_addr failed!}\r\n");
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
            ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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
                ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
                if (OAL_SUCC != ul_ret)
                {
                    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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

    l_ret = (oal_int32)wal_config_get_assoc_req_ie(pst_mac_vap, &us_len, (oal_uint8 *)pst_ioctl_data);

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

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_auth_mode::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr!}\r\n");
        return -OAL_EINVAL;
    }

    ul_ret = hmac_config_set_auth_mode(pst_mac_vap, us_len, (oal_uint8 *)&(pst_ioctl_data->pri_data.auth_params));
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_auth_mode::return err code [%d]!}\r\n", ul_ret);
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

    ul_ret = hmac_config_set_max_user(pst_mac_vap, us_len, (pst_ioctl_data->pri_data.ul_vap_max_user));
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_max_user::hmac_config_set_max_user return err code [%d]!}\r\n", ul_ret);
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
    ul_ret = hmac_config_set_ssid(pst_mac_vap, OAL_SIZEOF(st_ssid_param), (oal_uint8 *)&st_ssid_param);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_ssid::hmac_config_set_ssid return err code [%d]!}\r\n", ul_ret);
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

#ifdef _PRE_WLAN_FEATURE_DFS
    l_ret = wal_regdomain_update_for_dfs(pst_net_dev, auc_country_code);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_country_code::return err code [%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }
#endif
    l_ret = wal_regdomain_update(pst_net_dev, auc_country_code);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_country_code::return err code [%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

#else
    OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_country_code::_PRE_WLAN_FEATURE_11D is not define!}\r\n");
#endif

    return OAL_SUCC;
}

oal_int32  wal_ioctl_nl80211_priv_connect(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    mac_cfg80211_connect_param_stru     st_mac_cfg80211_connect_param;
    oal_uint8                           uc_loop                 = 0;
    oal_uint8                           uc_akm_suite_num        = 0;
    oal_uint8                           uc_pairwise_cipher_num  = 0;
    oal_int32                           l_ret                   = 0;

    /* ��ʼ���������Ӳ��� */
    oal_memset(&st_mac_cfg80211_connect_param, 0, OAL_SIZEOF(mac_cfg80211_connect_param_stru));

    /* �����ں��·��� freq to channel_number eg.1,2,36,40...  */
    st_mac_cfg80211_connect_param.uc_channel    = (oal_uint8)oal_ieee80211_frequency_to_channel(pst_ioctl_data->pri_data.cfg80211_connect_params.l_freq);

    /* �����ں��·��� ssid */
    st_mac_cfg80211_connect_param.uc_ssid_len   = OAL_MIN((oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.ssid_len, WLAN_SSID_MAX_LEN);
    oal_memcopy(st_mac_cfg80211_connect_param.auc_ssid, (oal_uint8 *)pst_ioctl_data->pri_data.cfg80211_connect_params.puc_ssid, st_mac_cfg80211_connect_param.uc_ssid_len);

    /* �����ں��·��� bssid */
    oal_set_mac_addr(st_mac_cfg80211_connect_param.auc_bssid, (oal_uint8 *)pst_ioctl_data->pri_data.cfg80211_connect_params.puc_bssid);

    /* �����ں��·��İ�ȫ��ز��� */
    /* ��֤���� */
    st_mac_cfg80211_connect_param.en_auth_type  = pst_ioctl_data->pri_data.cfg80211_connect_params.en_auth_type;

    /* �������� */
    st_mac_cfg80211_connect_param.en_privacy    = pst_ioctl_data->pri_data.cfg80211_connect_params.en_privacy;

    /* IE�·� */
    st_mac_cfg80211_connect_param.puc_ie        = pst_ioctl_data->pri_data.cfg80211_connect_params.puc_ie;
    st_mac_cfg80211_connect_param.ul_ie_len     = (oal_uint32)pst_ioctl_data->pri_data.cfg80211_connect_params.ie_len;

    /* ���ü��ܲ��� */
    if (0 != pst_ioctl_data->pri_data.cfg80211_connect_params.en_privacy)
    {
        if ((0 != pst_ioctl_data->pri_data.cfg80211_connect_params.uc_wep_key_len) && (0 == pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.n_akm_suites))
        {
            /* ����wep������Ϣ */
            st_mac_cfg80211_connect_param.puc_wep_key            = pst_ioctl_data->pri_data.cfg80211_connect_params.puc_wep_key;
            st_mac_cfg80211_connect_param.uc_wep_key_len         = pst_ioctl_data->pri_data.cfg80211_connect_params.uc_wep_key_len;
            st_mac_cfg80211_connect_param.uc_wep_key_index       = pst_ioctl_data->pri_data.cfg80211_connect_params.uc_wep_key_index;
            st_mac_cfg80211_connect_param.st_crypto.cipher_group = (oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.cipher_group;
        }
        else if (0 != pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.n_akm_suites)
        {
            /* ����WPA/WPA2 ������Ϣ */
            st_mac_cfg80211_connect_param.st_crypto.wpa_versions = (oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.wpa_versions;
            st_mac_cfg80211_connect_param.st_crypto.cipher_group = (oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.cipher_group;
            st_mac_cfg80211_connect_param.st_crypto.n_ciphers_pairwise = (oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.n_ciphers_pairwise;
            st_mac_cfg80211_connect_param.st_crypto.n_akm_suites = (oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.n_akm_suites;
            st_mac_cfg80211_connect_param.st_crypto.control_port = (oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.control_port;

            uc_pairwise_cipher_num = st_mac_cfg80211_connect_param.st_crypto.n_ciphers_pairwise;
            for (uc_loop = 0; uc_loop < uc_pairwise_cipher_num; uc_loop++)
            {
                st_mac_cfg80211_connect_param.st_crypto.ciphers_pairwise[uc_loop] = (oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.ciphers_pairwise[uc_loop];
            }

            uc_akm_suite_num = st_mac_cfg80211_connect_param.st_crypto.n_akm_suites;
            for (uc_loop = 0; uc_loop < uc_akm_suite_num; uc_loop++)
            {
                st_mac_cfg80211_connect_param.st_crypto.akm_suites[uc_loop] = (oal_uint8)pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.akm_suites[uc_loop];
            }
        }
    else if (mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS, st_mac_cfg80211_connect_param.puc_ie, (oal_int32)(st_mac_cfg80211_connect_param.ul_ie_len)))
        {
            OAM_WARNING_LOG0(0, OAM_SF_WPS, "wal_ioctl_nl80211_priv_connect::WPS ie is included in puc_ie! \n");
        }
        else
        {
            OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_connect::set_key fail! uc_wep_key_len [%d] n_akm_suites [%d]} puc_wep_key[0x%x]\r\n",
                           pst_ioctl_data->pri_data.cfg80211_connect_params.uc_wep_key_len,
                           pst_ioctl_data->pri_data.cfg80211_connect_params.st_crypto.n_akm_suites,
                           pst_ioctl_data->pri_data.cfg80211_connect_params.puc_wep_key);
            return -OAL_EFAIL;
        }

    }

    /* ���¼����������������� */
    l_ret = wal_cfg80211_start_connect(pst_net_dev, &st_mac_cfg80211_connect_param);
    if( OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_connect::wal_cfg80211_start_connect fail!}\r\n");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
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
    pst_mac_user = mac_res_get_mac_user(pst_mac_vap->uc_assoc_vap_id);
    if (OAL_PTR_NULL == pst_mac_user)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_disconnect:: mac_res_get_mac_user pst_mac_user  is nul!}\r\n");
        return OAL_SUCC;
    }

    oal_memcopy(st_mac_cfg_kick_user_param.auc_mac_addr, pst_mac_user->auc_user_mac_addr, WLAN_MAC_ADDR_LEN);

    l_ret = wal_cfg80211_start_disconnect(pst_net_dev, &st_mac_cfg_kick_user_param);
    if( OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_disconnect::wal_cfg80211_start_disconnect fail!}\r\n");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_HILINK

oal_int32 wal_ioctl_start_fbt_scan(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    wal_msg_write_stru              st_write_msg;
    mac_cfg_fbt_scan_params_stru   *pst_mac_cfg_fbt_scan_params;
    mac_vap_stru                   *pst_mac_vap;
    oal_uint16                      us_len;
    oal_int32                       l_ret;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_net_dev_ioctl_get_all_sta_info::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr!}\r\n");
        return -OAL_EINVAL;
    }

    /* ��ȡioctl���ò��� */
    us_len = OAL_SIZEOF(mac_cfg_fbt_scan_params_stru);
    pst_mac_cfg_fbt_scan_params = (mac_cfg_fbt_scan_params_stru *)(st_write_msg.auc_value);
    oal_memcopy(pst_mac_cfg_fbt_scan_params, &(pst_ioctl_data->pri_data.fbt_scan_params), us_len);

    OAM_WARNING_LOG4(0, OAM_SF_CFG, "{wal_ioctl_start_fbt_scan::en_is_on [%d],ul_channel [%d], mac[%x:%x]}\r\n",
                    pst_mac_cfg_fbt_scan_params->en_is_on,
                    pst_mac_cfg_fbt_scan_params->ul_channel,
                    pst_mac_cfg_fbt_scan_params->mac[4],
                    pst_mac_cfg_fbt_scan_params->mac[5]);
    /***************************************************************************
                              ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_FBT_START_SCAN, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
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



oal_int32 wal_net_dev_ioctl_get_all_sta_info(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    mac_vap_stru                   *pst_mac_vap;
    oal_uint16                      us_len;
    oal_int32                       l_ret;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_net_dev_ioctl_get_all_sta_info::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr!}\r\n");
        return -OAL_EINVAL;
    }

    l_ret = (oal_int32)wal_config_get_all_sta_info(pst_mac_vap, &us_len, (oal_uint8 *)pst_ioctl_data);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_net_dev_ioctl_get_all_sta_info::return err code [%d]!}\r\n", l_ret);
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

    l_ret = wal_cfg80211_fbt_kick_user(pst_net_dev, &st_mac_cfg_kick_user_param);
    if( OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_nl80211_priv_fbt_kick_user::wal_cfg80211_start_disconnect fail!}\r\n");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


oal_int32 wal_ioctl_set_okc_ie(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    wal_msg_write_stru              st_write_msg;
    oal_app_ie_stru                 st_okc_ie;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                      ul_err_code;
    oal_int32                       l_ret = 0;

    OAL_MEMZERO(&st_okc_ie, OAL_SIZEOF(oal_app_ie_stru));
    st_okc_ie.ul_ie_len      = pst_ioctl_data->pri_data.st_app_ie.ul_ie_len;
    st_okc_ie.en_app_ie_type = pst_ioctl_data->pri_data.st_app_ie.en_app_ie_type;
    l_ret = (oal_int32)oal_copy_from_user(st_okc_ie.auc_ie, pst_ioctl_data->pri_data.st_app_ie.auc_ie, st_okc_ie.ul_ie_len);

    /* copy_from_user������Ŀ���Ǵ��û��ռ俽�����ݵ��ں˿ռ䣬ʧ�ܷ���û�б��������ֽ������ɹ�����0 */
    if(l_ret != 0)
    {
        OAM_WARNING_LOG2(0, OAM_SF_CFG, "{wal_ioctl_set_okc_ie::copy app ie fail.ie_type[%d], ie_len[%d]}",
                        st_okc_ie.en_app_ie_type,
                        st_okc_ie.ul_ie_len);
        return -OAL_EFAIL;
    }

    oal_memcopy(st_write_msg.auc_value, &st_okc_ie, OAL_SIZEOF(oal_app_ie_stru));

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_OKC_IE, OAL_SIZEOF(oal_app_ie_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_app_ie_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_okc_ie:: wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_okc_ie::wal_check_and_release_msg_resp fail return err code:[%x]!}\r\n",
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
    oal_int32                            l_sec_channel_offset;
    oal_int32                            l_center_freq1;
    oal_int32                            l_bandwidth;
    oal_int32                            l_ret;
    wal_msg_stru                        *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                          ul_err_code;

    l_freq               = pst_ioctl_data->pri_data.freq.l_freq;
    l_channel            = pst_ioctl_data->pri_data.freq.l_channel;
    l_sec_channel_offset = pst_ioctl_data->pri_data.freq.l_sec_channel_offset;
    l_center_freq1       = pst_ioctl_data->pri_data.freq.l_center_freq1;
    l_bandwidth          = pst_ioctl_data->pri_data.freq.l_bandwidth;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_ioctl_set_channel::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}\r\n");
        return -OAL_EINVAL;
    }

    pst_device  = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_channel::pst_device is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_wiphy   = pst_device->pst_wiphy;
    pst_channel = oal_ieee80211_get_channel(pst_wiphy, l_freq);
    l_channel   = pst_channel->hw_value;

    /* �ж��ŵ��ڲ��ڹ������� */
    l_ret = (oal_int32)mac_is_channel_num_valid(pst_channel->band, (oal_uint8)l_channel);
    if (l_ret != OAL_SUCC)
    {
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_channel::channel num is invalid. band, ch num [%d] [%d]!}\r\n", pst_channel->band, l_channel);
        return -OAL_EINVAL;
    }

    /* �����ں˴���ֵ��WITP ����ֵת�� */
    if (80 == l_bandwidth)
    {
        en_bandwidth = mac_get_bandwith_from_center_freq_seg0((oal_uint8)l_channel, (oal_uint8)((l_center_freq1 - 5000)/5));
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

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_SET_CHANNEL, OAL_SIZEOF(mac_cfg_channel_param_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_channel_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_channel:: wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_set_channel::wal_check_and_release_msg_resp fail return err code:[%d]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif


oal_uint32 wal_ioctl_parse_wps_p2p_ie(oal_app_ie_stru *pst_app_ie, oal_uint8 *puc_src, oal_uint32 ul_src_len)
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
        puc_ie = mac_find_ie(MAC_EID_P2P, puc_buf_remain, (oal_int32)ul_len_remain);
        if(OAL_PTR_NULL != puc_ie)
        {
            ul_ie_len = (oal_uint8)puc_ie[1] + MAC_IE_HDR_LEN;
            if((ul_ie_len > (WLAN_WPS_IE_MAX_SIZE - pst_app_ie->ul_ie_len)) ||
               (ul_src_len < ((oal_uint32)(puc_ie - puc_src) + ul_ie_len)))
            {
                OAM_WARNING_LOG3(0, OAM_SF_CFG, "{wal_ioctl_parse_wps_p2p_ie::uc_ie_len=[%d], left buffer sieze=[%d], src_end_len=[%d],param invalid!}\r\n", \
                ul_ie_len, WLAN_WPS_IE_MAX_SIZE - pst_app_ie->ul_ie_len, (oal_uint32)(puc_ie - puc_src) + ul_ie_len);
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



oal_int32 wal_ioctl_set_wps_p2p_ie(oal_net_device_stru  *pst_net_dev,
                                    oal_uint8           *puc_buf,
                                    oal_uint32           ul_len,
                                    en_app_ie_type_uint8 en_type)
{
    wal_msg_write_stru              st_write_msg;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                      ul_err_code;
    oal_app_ie_stru                 st_wps_p2p_ie;
    mac_vap_stru                   *pst_mac_vap;
    oal_w2h_app_ie_stru            *pst_w2h_wps_p2p_ie = OAL_PTR_NULL;
    oal_int32                       l_ret = 0;

    if (WLAN_WPS_IE_MAX_SIZE < ul_len)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_wps_p2p_ie:: wrong ul_len: [%u]!}\r\n",
                            ul_len);
        return -OAL_EFAIL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_ioctl_set_wps_p2p_ie::pst_mac_vap is null}");
        return -OAL_EINVAL;
    }

    OAL_MEMZERO(&st_wps_p2p_ie, OAL_SIZEOF(st_wps_p2p_ie));
    switch (en_type) {
        case OAL_APP_BEACON_IE:
        case OAL_APP_PROBE_RSP_IE:
        case OAL_APP_ASSOC_RSP_IE:
            st_wps_p2p_ie.en_app_ie_type = en_type;
            break;
        default:
            OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_wps_p2p_ie:: wrong type: [%x]!}\r\n",
                            en_type);
            return -OAL_EFAIL;
    }


    if(OAL_FALSE == IS_LEGACY_VAP(pst_mac_vap))
    {
        if(OAL_SUCC != wal_ioctl_parse_wps_p2p_ie(&st_wps_p2p_ie, puc_buf , ul_len))
        {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_wps_p2p_ie::Type=[%d], parse p2p ie fail,!}\r\n", en_type);
            return -OAL_EFAIL;
        }
    }
    else
    {
        oal_memcopy(st_wps_p2p_ie.auc_ie, puc_buf, ul_len);
        st_wps_p2p_ie.ul_ie_len = ul_len;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    pst_w2h_wps_p2p_ie                 = (oal_w2h_app_ie_stru *)st_write_msg.auc_value;
    pst_w2h_wps_p2p_ie->en_app_ie_type = st_wps_p2p_ie.en_app_ie_type;
    pst_w2h_wps_p2p_ie->ul_ie_len      = st_wps_p2p_ie.ul_ie_len;
    pst_w2h_wps_p2p_ie->puc_data_ie    = st_wps_p2p_ie.auc_ie;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_WPS_P2P_IE, OAL_SIZEOF(oal_w2h_app_ie_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_w2h_app_ie_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_wps_p2p_ie:: wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_wps_p2p_ie::wal_check_and_release_msg_resp fail return err code: [%d]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_P2P

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
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_p2p_noa_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_p2p_noa:: wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_p2p_noa::wal_check_and_release_msg_resp fail return err code:  [%d]!}\r\n",
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
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_p2p_ops_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_p2p_ops:: wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_p2p_ops::wal_check_and_release_msg_resp fail return err code:[%d]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
OAL_STATIC oal_int32 wal_ioctl_set_sta_pm(oal_net_device_stru *pst_net_dev, mac_pm_switch_enum_uint8 en_pm_enable, mac_pm_ctrl_type_enum_uint8 en_pm_ctrl_type)
{
    wal_msg_write_stru                  st_write_msg;
    mac_cfg_ps_open_stru               *pst_sta_pm_open;
    oal_int32                           l_ret;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_STA_PM_ON, OAL_SIZEOF(mac_cfg_ps_open_stru));

    /* ��������������� */
    pst_sta_pm_open = (mac_cfg_ps_open_stru *)(st_write_msg.auc_value);
    /* MAC_STA_PM_SWITCH_ON / MAC_STA_PM_SWITCH_OFF */
    pst_sta_pm_open->uc_pm_enable      = en_pm_enable;
    pst_sta_pm_open->uc_pm_ctrl_type   = en_pm_ctrl_type;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ps_open_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_android_priv_cmd::CMD_SET_STA_PM_ON return err code [%d]!}\r\n", l_ret);
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
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(hmac_cfg_qos_map_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_HS20, "{wal_ioctl_set_qos_map:: wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG2(0, OAM_SF_HS20, "{wal_ioctl_set_qos_map::wal_check_and_release_msg_resp fail return err code: [%x] [%x]!}\r\n",
                        l_ret, ul_err_code);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
#endif // _PRE_WLAN_FEATURE_HS20

#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) && (!defined(_PRE_PRODUCT_ID_HI110X_HOST)))

oal_int32 wal_ioctl_set_wps_ie(oal_net_device_stru *pst_net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data)
{
    wal_msg_write_stru              st_write_msg;
    oal_app_ie_stru                 st_wps_ie;
    wal_msg_stru                   *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                      ul_err_code;
    oal_int32                       l_ret = 0;

    OAL_MEMZERO(&st_wps_ie, OAL_SIZEOF(oal_app_ie_stru));
    st_wps_ie.ul_ie_len      = pst_ioctl_data->pri_data.st_app_ie.ul_ie_len;
    st_wps_ie.en_app_ie_type = pst_ioctl_data->pri_data.st_app_ie.en_app_ie_type;
    l_ret = (oal_int32)oal_copy_from_user(st_wps_ie.auc_ie, pst_ioctl_data->pri_data.st_app_ie.auc_ie, st_wps_ie.ul_ie_len);

    /* copy_from_user������Ŀ���Ǵ��û��ռ俽�����ݵ��ں˿ռ䣬ʧ�ܷ���û�б��������ֽ������ɹ�����0 */
    if(l_ret != 0)
    {
        OAM_WARNING_LOG2(0, OAM_SF_CFG, "{wal_ioctl_set_wps_ie::copy app ie fail.ie_type[%d], ie_len[%d]}",
                        st_wps_ie.en_app_ie_type,
                        st_wps_ie.ul_ie_len);
        return -OAL_EFAIL;
    }

    oal_memcopy(st_write_msg.auc_value, &st_wps_ie, OAL_SIZEOF(oal_app_ie_stru));

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_WPS_IE, OAL_SIZEOF(oal_app_ie_stru));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_app_ie_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_wps_ie:: wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_ioctl_set_wps_ie::wal_check_and_release_msg_resp fail return err code:[%x]!}\r\n",
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
extern oal_uint8                        g_auc_sar_params[CUS_NUM_OF_SAR_LVL][CUS_SAR_NUM];
#endif

oal_uint8* wal_get_reduce_sar_ctrl_params(oal_uint8 uc_tx_power_lvl)
{
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wlan_customize_power_params_stru     *pst_cust_nv_params = hwifi_get_nvram_params();  /* ����͹��ʶ��ƻ����� */
    if ((uc_tx_power_lvl <= CUS_NUM_OF_SAR_LVL) && (uc_tx_power_lvl > 0))
    {
        uc_tx_power_lvl--;
    }
    else
    {
        return OAL_PTR_NULL;
    }
    oal_memcopy(pst_cust_nv_params->auc_sar_ctrl_params,g_auc_sar_params[uc_tx_power_lvl],OAL_SIZEOF(oal_uint8) * CUS_SAR_NUM);
    return pst_cust_nv_params->auc_sar_ctrl_params;
#else
    return OAL_PTR_NULL;
#endif
}


OAL_STATIC oal_int32 wal_ioctl_reduce_sar(oal_net_device_stru *pst_net_dev, oal_uint16 us_tx_power)
{
    oal_int32                   l_ret;
    wal_msg_write_stru          st_write_msg;
    oal_uint8                  *puc_sar_ctrl_params;
    oal_uint8                   auc_sar_ctrl_params[CUS_SAR_NUM];
    oal_uint8                   uc_lvl_idx = 0;

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_ioctl_reduce_sar::supplicant set tx_power[%d] for reduce SAR purpose.\r\n", us_tx_power);
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

    if ((us_tx_power >= 1001) && (us_tx_power <= 1020))
    {
        uc_lvl_idx = us_tx_power - 1000;
    }
    puc_sar_ctrl_params = wal_get_reduce_sar_ctrl_params(uc_lvl_idx);
    if (OAL_PTR_NULL == puc_sar_ctrl_params)
    {
        oal_memset(auc_sar_ctrl_params, 0xFF, OAL_SIZEOF(oal_uint8)*CUS_SAR_NUM);
        puc_sar_ctrl_params = auc_sar_ctrl_params;
    }
    /* vapδ����ʱ��������supplicant���� */
    if (NULL == OAL_NET_DEV_PRIV(pst_net_dev))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "wal_ioctl_reduce_sar::vap not created yet, ignore the cmd!");
        return -OAL_EINVAL;
    }
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_REDUCE_SAR, OAL_SIZEOF(oal_uint8)*CUS_SAR_NUM);
    oal_memcopy(st_write_msg.auc_value, puc_sar_ctrl_params, OAL_SIZEOF(oal_uint8)*CUS_SAR_NUM);
    l_ret = wal_send_cfg_event(pst_net_dev,
                       WAL_MSG_TYPE_WRITE,
                       WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8)*CUS_SAR_NUM,
                       (oal_uint8 *)&st_write_msg,
                       OAL_FALSE,
                       OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_ioctl_reduce_sar::wal_send_cfg_event failed, error no[%d]!\r\n", l_ret);
        return l_ret;
    }
    return OAL_SUCC;
}

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
    l_ret = wal_send_cfg_event(pst_net_dev,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(ul_ap_max_user),
                                (oal_uint8 *)&st_write_msg,
                                OAL_TRUE,
                                &pst_rsp_msg);

    if ((OAL_SUCC != l_ret) || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_set_ap_max_user:: wal_send_cfg_event return err code %d!}\r\n", l_ret);

        return l_ret;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_set_ap_max_user::wal_check_and_release_msg_resp fail return err code: [%d]!}\r\n",
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
#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
    wal_msg_write_stru          st_write_msg;
    oal_uint16                  us_len;
    wal_msg_stru               *pst_rsp_msg = OAL_PTR_NULL;
   oal_uint32                   ul_err_code;
    oal_int32                   l_ret = 0;
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
#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
    ul_ret = wal_hipriv_send_cfg_uint32_data(pst_net_dev,ac_parsed_command, WLAN_CFGID_BLACKLIST_MODE);
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
#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
        /***************************************************************************
                             ���¼���wal�㴦��
        ***************************************************************************/
        OAL_MEMZERO((oal_uint8*)&st_write_msg, OAL_SIZEOF(st_write_msg));
        oal_strtoaddr(ac_parsed_command, st_write_msg.auc_value); /* ���ַ� ac_name ת�������� mac_add[6] */

        us_len = OAL_MAC_ADDR_LEN; /* OAL_SIZEOF(oal_uint8); */

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
        l_ret = wal_send_cfg_event(pst_net_dev,
                                    WAL_MSG_TYPE_WRITE,
                                    WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                    (oal_uint8 *)&st_write_msg,
                                    OAL_TRUE,
                                    &pst_rsp_msg);

        if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_config_mac_filter:: wal_send_cfg_event return err code %d!}\r\n", l_ret);
            return l_ret;
        }

        /* 6.2  ��ȡ���صĴ����� */
        ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
        if(OAL_SUCC != ul_err_code)
        {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_config_mac_filter::wal_check_and_release_msg_resp fail return err code:[%x]!}\r\n",
                               ul_err_code);
            return -OAL_EFAIL;
        }
#endif
    }

    /* ÿ���������mac��ַ���˺���մ��м���� */
    //oal_memset(g_st_ap_config_info.ac_ap_mac_filter_mode, 0 ,OAL_SIZEOF(g_st_ap_config_info.ac_ap_mac_filter_mode));

    return OAL_SUCC;
}



OAL_STATIC oal_int32 wal_kick_sta(oal_net_device_stru *pst_net_dev, oal_uint8 *auc_mac_addr)
{
#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
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

#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_KICK_USER, OAL_SIZEOF(mac_cfg_kick_user_param_stru));

    pst_kick_user_param = (mac_cfg_kick_user_param_stru *)(st_write_msg.auc_value);
    oal_set_mac_addr(pst_kick_user_param->auc_mac_addr, auc_mac_addr);

    pst_kick_user_param->us_reason_code = MAC_AUTH_NOT_VALID;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_kick_user_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_kick_sta:: wal_send_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* 4.4  ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_kick_sta::wal_check_and_release_msg_resp fail return err code: [%x]!}\r\n",
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
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters:: param is OAL_PTR_NULL , pst_net_dev = %p, pst_wrqu = %p}",
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

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev || OAL_PTR_NULL == pst_info || OAL_PTR_NULL == pst_wrqu || OAL_PTR_NULL == pc_extra))
    {
        OAM_WARNING_LOG4(0, OAM_SF_ANY, "{wal_ioctl_get_assoc_list:: param is OAL_PTR_NULL , pst_net_dev = %p,pst_info = %p , pst_wrqu = %p , pc_extra = %p}\n",
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
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if ((OAL_SUCC != l_ret)  || (OAL_PTR_NULL == pst_rsp_msg))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_ioctl_get_assoc_list:: wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
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

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev || OAL_PTR_NULL == pst_info || OAL_PTR_NULL == pst_wrqu || OAL_PTR_NULL == pc_extra))
    {
        OAM_WARNING_LOG4(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters:: param is OAL_PTR_NULL , pst_net_dev = %p,pst_info = %p , pst_wrqu = %p , pc_extra = %p}\n",
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
        l_ret = wal_kick_sta(pst_net_dev, auc_mac_addr);
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
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters:: param is OAL_PTR_NULL , pst_net_dev = %p, pst_wrqu = %p}",
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
    l_ret = wal_kick_sta(pst_net_dev, auc_mac_addr);

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


oal_uint32  wal_hipriv_get_mac_addr(oal_int8 *pc_param, oal_uint8 auc_mac_addr[], oal_uint32 *pul_total_offset)
{
    oal_uint32                      ul_off_set      = 0;
    oal_uint32                      ul_ret          = OAL_SUCC;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    /* ��ȡmac��ַ */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_get_mac_addr::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_EDCA, "{wal_hipriv_set_edca_opt_switch_sta::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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
    l_ret = wal_send_cfg_event(pst_net_dev,
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

#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY

oal_uint32  wal_hipriv_send_cfg_uint32_data(oal_net_device_stru *pst_net_dev,
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_send_cfg_uint32_data:wal_get_cmd_one_arg fail!}\r\n");
        return ul_ret;
    }

    pc_param += ul_off_set;
    set_value = (oal_uint32)oal_atoi((const oal_int8 *)ac_name);

    us_len = 4; /* OAL_SIZEOF(oal_uint32) */
    *(oal_uint32 *)(st_write_msg.auc_value) = set_value;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, cfgid, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
                              WAL_MSG_TYPE_WRITE,
                              WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                              (oal_uint8 *)&st_write_msg,
                              OAL_FALSE,
                              OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_send_cfg_uint32_data:wal_send_cfg_event return [%d].}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif  /* _PRE_WLAN_FEATURE_CUSTOM_SECURITY */


OAL_STATIC oal_uint32  wal_hipriv_bgscan_enable(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32                           ul_off_set;
    oal_int8                             ac_stop[2];
    oal_uint32                           ul_ret;
    oal_int32                            l_ret;
    wal_msg_write_stru                   st_write_msg;
    oal_uint8                 *pen_bgscan_enable_flag;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_stop, &ul_off_set);
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

    OAM_WARNING_LOG1(0, OAM_SF_SCAN, "wal_hipriv_bgscan_enable:: bgscan_enable_flag= %d.", *pen_bgscan_enable_flag);

    l_ret = wal_send_cfg_event(pst_net_dev,
                       WAL_MSG_TYPE_WRITE,
                       WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_bool_enum_uint8),
                       (oal_uint8 *)&st_write_msg,
                       OAL_FALSE,
                       OAL_PTR_NULL);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_bgscan_enable::wal_send_cfg_event fail.return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

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

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sta_ps_enable::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
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

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sta_ps_info::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    uc_psm_info_enable = (oal_uint8)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sta_ps_info::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
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

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_uapsd_para::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    uc_max_sp_len = (oal_uint8)oal_atoi(ac_name);

    for (uc_ac = 0; uc_ac < WLAN_WME_AC_BUTT; uc_ac++)
    {
        pc_param = pc_param + ul_off_set;
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
        if (OAL_SUCC != ul_ret)
        {
             OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_uapsd_para::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
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

oal_int32 wal_set_nbfh(oal_net_device_stru *pst_net_dev)
{
    wal_msg_write_stru      st_write_msg;
    oal_int32               l_ret;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_set_nbfh::pst_net_dev is null ptr!}\r\n");
        return -OAL_EFAUL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_NBFH, 0);

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_set_nbfh::wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
    }

    return l_ret;
}


oal_int32 wal_start_vap(oal_net_device_stru *pst_net_dev)
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
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_start_vap::pst_net_dev is null ptr!}\r\n");
        return -OAL_EFAUL;
    }

    OAL_IO_PRINT("wal_start_vap,dev_name is:%.16s\n", pst_net_dev->name);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_START_VAP, OAL_SIZEOF(mac_cfg_start_vap_param_stru));
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wdev    = pst_net_dev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_start_vap::wal_wireless_iftype_to_mac_p2p_mode return BUFF}\r\n");
        return -OAL_EINVAL;
    }
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_start_vap::en_p2p_mode:%d}\r\n", en_p2p_mode);
#endif
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_mgmt_rate_init_flag = OAL_TRUE;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_start_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_start_vap::wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /* ��������Ϣ */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_start_vap::hmac start vap fail, err code[%d]!}\r\n", ul_err_code);
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


oal_int32  wal_stop_vap(oal_net_device_stru *pst_net_dev)
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
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_stop_vap::pst_net_dev is null ptr!}\r\n");
        return -OAL_EFAUL;
    }

    /* �������up״̬������ֱ�ӷ��سɹ�,��ֹnetdevice״̬��VAP״̬��һ�µ���� */
    if (0 == (OAL_NETDEVICE_FLAGS(pst_net_dev) & OAL_IFF_RUNNING))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_stop_vap::vap is already down,continue to reset hmac vap state.}\r\n");
    }

    OAL_IO_PRINT("wal_stop_vap,dev_name is:%.16s\n", pst_net_dev->name);

    /***************************************************************************
                           ���¼���wal�㴦��
    ***************************************************************************/
    /* ��дWID��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DOWN_VAP, OAL_SIZEOF(mac_cfg_down_vap_param_stru));
    ((mac_cfg_down_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wdev    = pst_net_dev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_stop_vap::wal_wireless_iftype_to_mac_p2p_mode return BUFF}\r\n");
        return -OAL_EINVAL;
    }
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_stop_vap::en_p2p_mode:%d}\r\n", en_p2p_mode);
#endif

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_down_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_stop_vap::wal_alloc_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp(pst_rsp_msg))
    {
        OAM_WARNING_LOG0(0,OAM_SF_ANY,"wal_stop_vap::wal_check_and_release_msg_resp fail");
    }


    return OAL_SUCC;
}



oal_int32 wal_init_wlan_vap(oal_net_device_stru *pst_net_dev)
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
    oal_int32                   l_ret;

    wlan_vap_mode_enum_uint8    en_vap_mode;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8    en_p2p_mode = WLAN_LEGACY_VAP_MODE;
#endif

    if(NULL == pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap::pst_net_dev is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(NULL != pst_mac_vap)
    {
        if (MAC_VAP_STATE_BUTT != pst_mac_vap->en_vap_state)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap::pst_mac_vap is already exist}\r\n");
            return OAL_SUCC;
        }
        /* netdev�µ�vap�Ѿ���ɾ������Ҫ���´����͹��� */
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_init_wlan_vap::pst_mac_vap is already free, need creat again!!}");
        OAL_NET_DEV_PRIV(pst_net_dev) = OAL_PTR_NULL;
    }

    pst_wdev = OAL_NETDEVICE_WDEV(pst_net_dev);
    if(NULL == pst_wdev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap::pst_wdev is null!}\r\n");
        return -OAL_EFAUL;
    }

    pst_wiphy_priv  = (mac_wiphy_priv_stru *)oal_wiphy_priv(pst_wdev->wiphy);
    if (OAL_PTR_NULL == pst_wiphy_priv)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap::pst_wiphy_priv is null!}\r\n");
        return -OAL_EFAUL;
    }
    pst_mac_device  = pst_wiphy_priv->pst_mac_device;
    if(NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap::pst_mac_device is null!}\r\n");
        return -OAL_EFAUL;
    }

    pst_cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->uc_cfg_vap_id);
    if (NULL == pst_cfg_mac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_init_wlan_vap::pst_cfg_mac_vap is null! vap_id:%d}\r\n",pst_mac_device->uc_cfg_vap_id);
        return -OAL_EFAUL;
    }
    pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->uc_cfg_vap_id);
    if (NULL == pst_cfg_hmac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_init_wlan_vap::pst_cfg_hmac_vap is null! vap_id:%d}\r\n",pst_mac_device->uc_cfg_vap_id);
        return -OAL_EFAUL;
    }


    pst_cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;
    if(NULL == pst_cfg_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap::pst_cfg_net_dev is null!}\r\n");
        return -OAL_EFAUL;
    }

    /* ������WIFI��AP��ʱ����VAP */
    if((NL80211_IFTYPE_STATION == pst_wdev->iftype) || (NL80211_IFTYPE_P2P_DEVICE == pst_wdev->iftype))
    {
        if(0 == (oal_strcmp("wlan0", pst_net_dev->name)))
        {
            en_vap_mode = WLAN_VAP_MODE_BSS_STA;
        }
#ifdef _PRE_WLAN_FEATURE_P2P
        else if(0 == (oal_strcmp("p2p0", pst_net_dev->name)))
        {
            en_vap_mode = WLAN_VAP_MODE_BSS_STA;
            en_p2p_mode= WLAN_P2P_DEV_MODE;
        }
#endif
        else
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap::net_dev is not wlan0 or p2p0!}\r\n");
            return OAL_SUCC;
        }
    }
    else if(NL80211_IFTYPE_AP == pst_wdev->iftype)
    {
        en_vap_mode = WLAN_VAP_MODE_BSS_AP;
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap::net_dev is not wlan0 or p2p0!}\r\n");
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_init_wlan_vap::en_vap_mode:%d,en_p2p_mode:%d}\r\n",
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
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->bit_11ac2g_enable = (oal_uint8)!!hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_11AC2G_ENABLE);
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->bit_disable_capab_2ght40 = g_st_wlan_customize.uc_disable_capab_2ght40;
#endif
    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_add_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_init_wlan_vap::return err code %d!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    /* ��ȡ���صĴ����� */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_init_wlan_vap::hmac add vap fail, err code[%u]!}\r\n", ul_err_code);
        return -OAL_EFAIL;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if(OAL_SUCC != wal_set_random_mac_to_mib(pst_net_dev))
    {
        OAM_WARNING_LOG0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_init_wlan_vap::wal_set_random_mac_to_mib fail!}\r\n");
        return -OAL_EFAUL;
    }
#endif

    /* ����netdevice��MAC��ַ��MAC��ַ��HMAC�㱻��ʼ����MIB�� */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_vap::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}");
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



oal_int32 wal_deinit_wlan_vap(oal_net_device_stru *pst_net_dev)
{
    wal_msg_write_stru           st_write_msg;
    wal_msg_stru               *pst_rsp_msg = OAL_PTR_NULL;
    mac_vap_stru                *pst_mac_vap;
    oal_int32                    l_ret;
    oal_int32                    l_del_vap_flag = OAL_TRUE;

#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8    en_p2p_mode = WLAN_LEGACY_VAP_MODE;
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_deinit_wlan_vap::pst_del_vap_param null ptr !}\r\n");
        return -OAL_EINVAL;
    }


    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_deinit_wlan_vap::pst_mac_vap is already null}\r\n");
        return OAL_SUCC;
    }

    /* ������WIFI��AP�ر�ʱɾ��VAP */
    if ((0 != (oal_strcmp("wlan0", pst_net_dev->name))) && (0 != (oal_strcmp("p2p0", pst_net_dev->name))))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_deinit_wlan_vap::net_dev is not wlan0 or p2p0!}\r\n");
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    if(0 == oal_strcmp("p2p0", pst_net_dev->name))
    {
        en_p2p_mode = WLAN_P2P_DEV_MODE;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_deinit_wlan_vap::en_p2p_mode:%d}\r\n", en_p2p_mode);
#endif

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    //ɾ��vap ʱ��Ҫ��������ֵ��
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;
#endif

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DEL_VAP, OAL_SIZEOF(mac_cfg_del_vap_param_stru));
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_del_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_SUCC != wal_check_and_release_msg_resp(pst_rsp_msg))
    {
        OAM_WARNING_LOG0(0,OAM_SF_ANY,"wal_deinit_wlan_vap::wal_check_and_release_msg_resp fail.");
        /*can't set net dev's vap ptr to null when
          del vap wid process failed!*/
        l_del_vap_flag = OAL_FALSE;
    }

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_deinit_wlan_vap::return error code %d}\r\n", l_ret);
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
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
            hwifi_get_mac_addr(auc_primary_mac_addr);
            auc_primary_mac_addr[0]&=(~0x02);
#else
            oal_random_ether_addr(auc_primary_mac_addr);
            auc_primary_mac_addr[0]&=(~0x02);
            auc_primary_mac_addr[1]=0x11;
            auc_primary_mac_addr[2]=0x02;
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

            oal_set_mac_addr((oal_uint8 *)OAL_NETDEVICE_MAC_ADDR(pst_net_dev), auc_primary_mac_addr);
            break;
        }
    }
#else
    oal_random_ether_addr(auc_primary_mac_addr);
    auc_primary_mac_addr[0]&=(~0x02);
    auc_primary_mac_addr[1]=0x11;
    auc_primary_mac_addr[2]=0x02;

    oal_set_mac_addr((oal_uint8 *)OAL_NETDEVICE_MAC_ADDR(pst_net_dev), auc_primary_mac_addr);
#endif

    return OAL_SUCC;
}


oal_int32 wal_init_wlan_netdev(oal_wiphy_stru *pst_wiphy, char *dev_name)
{
    oal_net_device_stru        *pst_net_dev;
    oal_wireless_dev_stru      *pst_wdev;
    mac_wiphy_priv_stru        *pst_wiphy_priv;
    enum nl80211_iftype         en_type;
    oal_int32                   l_ret;

    oal_netdev_priv_stru       *pst_netdev_priv;

    if((NULL == pst_wiphy) || (NULL == dev_name))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::pst_wiphy or dev_name is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (0 == (oal_strcmp("wlan0", dev_name)))
    {
        en_type = NL80211_IFTYPE_STATION;
    }
    else if (0 == (oal_strcmp("p2p0", dev_name)))
    {
        en_type = NL80211_IFTYPE_P2P_DEVICE;
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::dev name is not wlan0 or p2p0}\r\n");
        return OAL_SUCC;
    }
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_init_wlan_netdev::en_type is %d}\r\n", en_type);

    /* ���������net device�Ѿ����ڣ�ֱ�ӷ��� */
    /* ����dev_name�ҵ�dev */
    pst_net_dev = oal_dev_get_by_name(dev_name);
    if (OAL_PTR_NULL != pst_net_dev)
    {
        /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
        oal_dev_put(pst_net_dev);

        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::the net_device is already exist!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if defined(_PRE_WLAN_FEATURE_FLOWCTL)
    pst_net_dev = oal_net_alloc_netdev_mqs(OAL_SIZEOF(oal_netdev_priv_stru), dev_name, oal_ether_setup, WAL_NETDEV_SUBQUEUE_MAX_NUM, 1);    /* �˺�����һ����δ���˽�г��� */
#elif defined(_PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL)
    pst_net_dev = oal_net_alloc_netdev_mqs(OAL_SIZEOF(oal_netdev_priv_stru), dev_name, oal_ether_setup, WLAN_NET_QUEUE_BUTT, 1);    /* �˺�����һ����δ���˽�г���*/
#else
    pst_net_dev = oal_net_alloc_netdev(OAL_SIZEOF(oal_netdev_priv_stru), dev_name, oal_ether_setup);    /* �˺�����һ����δ���˽�г���*/
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::oal_net_alloc_netdev return null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_wdev = (oal_wireless_dev_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,OAL_SIZEOF(oal_wireless_dev_stru), OAL_FALSE);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_wdev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::alloc mem, pst_wdev is null ptr!}\r\n");
        oal_net_free_netdev(pst_net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_memset(pst_wdev, 0, OAL_SIZEOF(oal_wireless_dev_stru));

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    pst_net_dev->features    |= NETIF_F_SG;
    pst_net_dev->hw_features |= NETIF_F_SG;
#endif /* _PRE_OS_VERSION_LINUX == _PRE_OS_VERSION */

    /* ��netdevice���и�ֵ */
#ifdef CONFIG_WIRELESS_EXT
    pst_net_dev->wireless_handlers             = &g_st_iw_handler_def;
#endif
    pst_net_dev->netdev_ops                    = &g_st_wal_net_dev_ops;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    pst_net_dev->ethtool_ops                   = &g_st_wal_ethtool_ops;
#endif

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
    if ((NL80211_IFTYPE_STATION == en_type))
    {
        /* �������wlan0�� �򱣴�wlan0 Ϊ��net_device,p2p0 ��p2p-p2p0 MAC ��ַ����netdevice ��ȡ */
        pst_wiphy_priv->pst_mac_device->st_p2p_info.pst_primary_net_device = pst_net_dev;
    }
    else if(NL80211_IFTYPE_P2P_DEVICE == en_type)
    {
        pst_wiphy_priv->pst_mac_device->st_p2p_info.pst_p2p_net_device = pst_net_dev;
    }
#endif
    OAL_NETDEVICE_FLAGS(pst_net_dev) &= ~OAL_IFF_RUNNING;   /* ��net device��flag��Ϊdown */

    wal_set_mac_addr(pst_net_dev);

    /* NAPI pri netdevע�������net_deviceǰ�� */
    pst_netdev_priv = (oal_netdev_priv_stru *)OAL_NET_DEV_WIRELESS_PRIV(pst_net_dev);
    pst_netdev_priv->uc_napi_enable   = OAL_TRUE;
    pst_netdev_priv->uc_gro_enable    = OAL_TRUE;
    pst_netdev_priv->uc_napi_weight   = NAPI_POLL_WEIGHT_MAX;
    pst_netdev_priv->uc_napi_dyn_weight = OAL_FALSE;
    pst_netdev_priv->uc_state         = 0;
    pst_netdev_priv->ul_queue_len_max = NAPI_NETDEV_PRIV_QUEUE_LEN_MAX;
    pst_netdev_priv->ul_period_pkts   = 0;
    pst_netdev_priv->ul_period_start  = 0;
    oal_netbuf_list_head_init(&pst_netdev_priv->st_rx_netbuf_queue);
    /* poll������� */
    oal_netif_napi_add(pst_net_dev, &pst_netdev_priv->st_napi, hmac_rxdata_polling, NAPI_POLL_WEIGHT_MAX);

    /* ע��net_device */
    l_ret = oal_net_register_netdev(pst_net_dev);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {

        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_init_wlan_netdev::oal_net_register_netdev return error code %d!}\r\n", l_ret);

        OAL_MEM_FREE(pst_wdev, OAL_FALSE);
        oal_net_free_netdev(pst_net_dev);

        return l_ret;
    }

    return OAL_SUCC;
}


oal_int32  wal_setup_ap(oal_net_device_stru *pst_net_dev)
{
    oal_int32 l_ret;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    wal_set_power_mgmt_on(OAL_FALSE);
    l_ret = wal_set_power_on(pst_net_dev, OAL_TRUE);
    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_setup_ap::wal_set_power_on fail [%d]!}", l_ret);
        return l_ret;
    }
#endif /* #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) */
    if (OAL_NETDEVICE_FLAGS(pst_net_dev) & OAL_IFF_RUNNING)
    {
        /* �л���APǰ��������豸����UP״̬����Ҫ��down wlan0�����豸 */
        OAL_IO_PRINT("wal_setup_ap:stop netdevice:%.16s", pst_net_dev->name);
        wal_netdev_stop(pst_net_dev);
    }

    pst_net_dev->ieee80211_ptr->iftype = NL80211_IFTYPE_AP;

    l_ret = wal_init_wlan_vap(pst_net_dev);
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if(OAL_SUCC == l_ret)
    {
        hwifi_config_init_ini(pst_net_dev);
    }
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
    return l_ret;
}

#endif

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD

oal_uint32 wal_hipriv_register_inetaddr_notifier(oal_void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == register_inetaddr_notifier(&wal_hipriv_notifier))
    {
        return OAL_SUCC;
    }

    OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_register_inetaddr_notifier::register inetaddr notifier failed.}");
    return OAL_FAIL;
#else
    return OAL_SUCC;
#endif
}


oal_uint32 wal_hipriv_unregister_inetaddr_notifier(oal_void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == unregister_inetaddr_notifier(&wal_hipriv_notifier))
    {
        return OAL_SUCC;
    }

    OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_unregister_inetaddr_notifier::hmac_unregister inetaddr notifier failed.}");
    return OAL_FAIL;
#else
    return OAL_SUCC;
#endif
}


oal_uint32 wal_hipriv_register_inet6addr_notifier(oal_void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == register_inet6addr_notifier(&wal_hipriv_notifier_ipv6))
    {
        return OAL_SUCC;
    }

    OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_register_inet6addr_notifier::register inetaddr6 notifier failed.}");
    return OAL_FAIL;
#else
    return OAL_SUCC;
#endif
}


oal_uint32 wal_hipriv_unregister_inet6addr_notifier(oal_void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == unregister_inet6addr_notifier(&wal_hipriv_notifier_ipv6))
    {
        return OAL_SUCC;
    }

    OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_unregister_inet6addr_notifier::hmac_unregister inetaddr6 notifier failed.}");
    return OAL_FAIL;
#else
    return OAL_SUCC;
#endif
}


#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

oal_int32 wal_hipriv_inetaddr_notifier_call(struct notifier_block *this, oal_uint event, oal_void *ptr)
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
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call::pst_ifa is NULL.}");
        return NOTIFY_DONE;
    }
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_ifa->ifa_dev->dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call::pst_ifa->idev->dev is NULL.}");
        return NOTIFY_DONE;
     }

    /* Filter notifications meant for non Hislicon devices */
    if (pst_ifa->ifa_dev->dev->netdev_ops != &g_st_wal_net_dev_ops)
    {
        return NOTIFY_DONE;
    }

    pst_mac_vap = (mac_vap_stru *)OAL_NET_DEV_PRIV(pst_ifa->ifa_dev->dev);
    if (OAL_UNLIKELY(NULL == pst_mac_vap))
    {
        OAM_WARNING_LOG1(0, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call::Get mac vap failed, when %d(UP:1 DOWN:2 UNKNOWN:others) ipv4 address.}", event);
        return NOTIFY_DONE;
    }

    wal_wake_lock();

    switch (event)
    {
        case NETDEV_UP:
        {
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call::Up IPv4[%d.X.X.%d], MASK[0x%08X], vap_mode[%d].}",
                             ((oal_uint8 *)&(pst_ifa->ifa_address))[0],
                             ((oal_uint8 *)&(pst_ifa->ifa_address))[3],
                             pst_ifa->ifa_mask,
                             pst_mac_vap->en_vap_mode);
            hmac_arp_offload_set_ip_addr(pst_mac_vap, DMAC_CONFIG_IPV4, DMAC_IP_ADDR_ADD, &(pst_ifa->ifa_address), &(pst_ifa->ifa_mask));

            if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
            {
                /* ��ȡ��IP��ַ��ʱ�����͹��� */
            #if _PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE
                wlan_pm_set_timeout(WLAN_SLEEP_DEFAULT_CHECK_CNT);
            #endif

                /* ��ȡ��IP��ַ��ʱ��֪ͨ���μ�ʱ */
            #ifdef _PRE_WLAN_FEATURE_ROAM
                pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
                if (OAL_PTR_NULL == pst_hmac_vap)
                {
                    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_hipriv_inetaddr_notifier_call:: pst_hmac_vap null.uc_vap_id[%d]}",
                        pst_mac_vap->uc_vap_id);

                    wal_wake_unlock();

                    return NOTIFY_DONE;
                }

                pst_hmac_vap->uc_ip_addr_obtained_num++;
                hmac_roam_wpas_connect_state_notify(pst_hmac_vap, WPAS_CONNECT_STATE_IPADDR_OBTAINED);
            #endif

            }
            break;
        }


        case NETDEV_DOWN:
        {
#if _PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE
            wlan_pm_set_timeout(WLAN_SLEEP_LONG_CHECK_CNT);
#endif
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call::Down IPv4[%d.X.X.%d], MASK[0x%08X] vap_mode[%d]..}",
                             ((oal_uint8 *)&(pst_ifa->ifa_address))[0],
                             ((oal_uint8 *)&(pst_ifa->ifa_address))[3],
                             pst_ifa->ifa_mask,
                             pst_mac_vap->en_vap_mode);
            hmac_arp_offload_set_ip_addr(pst_mac_vap, DMAC_CONFIG_IPV4, DMAC_IP_ADDR_DEL, &(pst_ifa->ifa_address), &(pst_ifa->ifa_mask));

            if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
            {
                /* ��ȡ��IP��ַ��ʱ��֪ͨ���μ�ʱ */
            #ifdef _PRE_WLAN_FEATURE_ROAM
                pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
                if (OAL_PTR_NULL == pst_hmac_vap)
                {
                    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_hipriv_inetaddr_notifier_call:: pst_hmac_vap null.uc_vap_id[%d]}",
                        pst_mac_vap->uc_vap_id);

                    wal_wake_unlock();

                    return NOTIFY_DONE;
                }

                if (0 != pst_hmac_vap->uc_ip_addr_obtained_num)
                {
                    pst_hmac_vap->uc_ip_addr_obtained_num--;
                }
                else
                {
                    /* �쳣ά�⣬aput�µ�ʱ��vap_mode��תΪSTA���յ�down�¼�����ʱuc_ip_addr_obtained_numΪ0 */
                    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "wal_hipriv_inetaddr_notifier_call::ip_obtained_num is zero, p2p_mode[%d]", pst_mac_vap->en_p2p_mode);
                }

                if (0 == pst_hmac_vap->uc_ip_addr_obtained_num)
                {
                    hmac_roam_wpas_connect_state_notify(pst_hmac_vap, WPAS_CONNECT_STATE_IPADDR_REMOVED);
                }
            #endif
            }
            break;
        }

        default:
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call::Unknown notifier event[%d].}", event);
            break;
        }
    }
    wal_wake_unlock();

    return NOTIFY_DONE;
}



oal_int32 wal_hipriv_inet6addr_notifier_call(struct notifier_block *this, oal_uint event, oal_void *ptr)
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
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call::pst_ifa is NULL.}");
        return NOTIFY_DONE;
    }
    if (OAL_UNLIKELY(NULL == pst_ifa->idev->dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call::pst_ifa->idev->dev is NULL.}");
        return NOTIFY_DONE;
     }


    /* Filter notifications meant for non Hislicon devices */
    if (pst_ifa->idev->dev->netdev_ops != &g_st_wal_net_dev_ops)
    {
        return NOTIFY_DONE;
    }

    pst_mac_vap = (mac_vap_stru *)OAL_NET_DEV_PRIV(pst_ifa->idev->dev);
    if (OAL_UNLIKELY(NULL == pst_mac_vap))
    {
        OAM_WARNING_LOG1(0, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call::Get mac vap failed, when %d(UP:1 DOWN:2 UNKNOWN:others) ipv6 address.}", event);
        return NOTIFY_DONE;
    }

    switch (event)
    {
        case NETDEV_UP:
        {
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call::UP IPv6[%04x:%04x:XXXX:XXXX:XXXX:XXXX:%04x:%04x].}",
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[0]),
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[1]),
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[6]),
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[7]));
            hmac_arp_offload_set_ip_addr(pst_mac_vap, DMAC_CONFIG_IPV6, DMAC_IP_ADDR_ADD, &(pst_ifa->addr), &(pst_ifa->addr));
            break;
        }

        case NETDEV_DOWN:
        {
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call::DOWN IPv6[%04x:%04x:XXXX:XXXX:XXXX:XXXX:%04x:%04x].}",
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[0]),
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[1]),
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[6]),
                             OAL_NET2HOST_SHORT((pst_ifa->addr.s6_addr16)[7]));
            hmac_arp_offload_set_ip_addr(pst_mac_vap, DMAC_CONFIG_IPV6, DMAC_IP_ADDR_DEL, &(pst_ifa->addr), &(pst_ifa->addr));
            break;
        }

        default:
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call::Unknown notifier event[%d].}", event);
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
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_offset);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_send_neighbor_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_offset);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_beacon_req_table_switch::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    uc_switch = (oal_uint8)oal_atoi(ac_arg);

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_BCN_TABLE_SWITCH, OAL_SIZEOF(oal_uint8));
    st_write_msg.auc_value[0] = uc_switch;
    l_ret = wal_send_cfg_event(pst_net_dev,
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
    oal_uint16                      *pus_value;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_offset);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_voe_enable::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    us_switch = (oal_uint16)oal_atoi(ac_arg);
    us_switch = us_switch & 0xFFFF;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_VOE_ENABLE, OAL_SIZEOF(oal_uint16));
    pus_value = (oal_uint16 *)st_write_msg.auc_value;
    *pus_value = us_switch;
    l_ret = wal_send_cfg_event(pst_net_dev,
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
    l_ret = wal_send_cfg_event(pst_net_dev,
                       WAL_MSG_TYPE_WRITE,
                       WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                       (oal_uint8 *)&st_write_msg,
                       OAL_FALSE,
                       OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_ioctl_set_vowifi_param::wal_send_cfg_event failed, error no[%d]!\r\n", l_ret);
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

    /*11r����*/
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

oal_int32 wal_set_ip_filter_enable(oal_int32 l_on)
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
    if (g_st_dfr_info.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_set_ip_filter_enable:: dfr_process_status[%d]!}",
            g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif //#ifdef _PRE_WLAN_FEATURE_DFR

    if (l_on < 0)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_set_ip_filter_enable::Invalid input parameter, on/off %d!}", l_on);
        return -OAL_EINVAL;
    }

    pst_net_dev = oal_dev_get_by_name("wlan0");
    if (OAL_PTR_NULL == pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_set_ip_filter_enable::wlan0 not exist!}");
        return -OAL_EINVAL;
    }
    /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
    oal_dev_put(pst_net_dev);

    /* vapδ����ʱ���������·������� */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_set_ip_filter_enable::vap not created yet, ignore the cmd!}");
        return -OAL_EINVAL;
    }

    if (OAL_TRUE != pst_mac_vap->st_cap_flag.bit_ip_filter)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_ip_filter_enable::Func not enable, ignore the cmd!}");
        return -OAL_EINVAL;
    }

    /* ׼���������� */
    ul_netbuf_len = OAL_SIZEOF(st_ip_filter_cmd);
    OAL_MEMZERO((oal_uint8 *)&st_ip_filter_cmd, ul_netbuf_len);
    st_ip_filter_cmd.en_cmd        = MAC_IP_FILTER_ENABLE;
    st_ip_filter_cmd.en_enable     = (l_on > 0)? OAL_TRUE : OAL_FALSE;

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_ip_filter_enable::IP_filter on/off(%d).}",
                        st_ip_filter_cmd.en_enable);

    /* ����ռ� ������˹��� */
    pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,ul_netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_ip_filter_enable::netbuf alloc null,size %d.}", ul_netbuf_len);
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
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_ip_filter_enable::wal_send_cfg_event failed, error no[%d]!}", l_ret);
        oal_netbuf_free(pst_netbuf);
        return l_ret;
    }

    return OAL_SUCC;
}

oal_int32 wal_add_ip_filter_items(wal_hw_wifi_filter_item *pst_items, oal_int32 l_count)
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
    if (g_st_dfr_info.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_add_ip_filter_items:: dfr_process_status[%d]!}",
            g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif //#ifdef _PRE_WLAN_FEATURE_DFR

    if ((OAL_PTR_NULL == pst_items)||(l_count <= 0))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_add_ip_filter_items::Invalid input parameter, pst_items %p, l_count %d!}", pst_items,l_count);
        return -OAL_EINVAL;
    }

    pst_net_dev = oal_dev_get_by_name("wlan0");
    if (OAL_PTR_NULL == pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_add_ip_filter_items::wlan0 not exist!}");
        return -OAL_EINVAL;
    }
    /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
    oal_dev_put(pst_net_dev);

    /* vapδ����ʱ���������·������� */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_add_ip_filter_items::vap not created yet, ignore the cmd!.}");
        return -OAL_EINVAL;
    }

    if (OAL_TRUE != pst_mac_vap->st_cap_flag.bit_ip_filter)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_add_ip_filter_items::Func not enable, ignore the cmd!.}");
        return -OAL_EINVAL;
    }

    /* ׼�������¼� */
    OAL_MEMZERO((oal_uint8 *)&st_ip_filter_cmd, OAL_SIZEOF(st_ip_filter_cmd));
    st_ip_filter_cmd.en_cmd    = MAC_IP_FILTER_UPDATE_BTABLE;

    /* ���ڱ���������С���ƣ�ȡ�����ɵĹ�����Ŀ����Сֵ */
    st_ip_filter_cmd.uc_item_count = OAL_MIN((MAC_MAX_IP_FILTER_BTABLE_SIZE / OAL_SIZEOF(mac_ip_filter_item_stru)), l_count);
    if (st_ip_filter_cmd.uc_item_count < l_count)
    {
       OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_add_ip_filter_items::Btable(%d) is too small to store %d items!}",
                        st_ip_filter_cmd.uc_item_count,
                        l_count);
    }

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_add_ip_filter_items::Start updating btable, items_cnt(%d).}",
                        st_ip_filter_cmd.uc_item_count);

    /* ѡ�������¼��ռ�Ĵ�С */
    ul_netbuf_len = (st_ip_filter_cmd.uc_item_count * OAL_SIZEOF(mac_ip_filter_item_stru)) + OAL_SIZEOF(st_ip_filter_cmd);


    /* ����ռ� ������˹��� */
    pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,ul_netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_add_ip_filter_items::netbuf alloc null,size %d.}", ul_netbuf_len);
        return -OAL_EINVAL;
    }
    OAL_MEMZERO(((oal_uint8 *)OAL_NETBUF_DATA(pst_netbuf)), ul_netbuf_len);
    pst_cmd_info = (mac_ip_filter_cmd_stru *)OAL_NETBUF_DATA(pst_netbuf);

    /* ��¼���˹��� */
    oal_memcopy((oal_uint8 *)pst_cmd_info, (oal_uint8 *)(&st_ip_filter_cmd), OAL_SIZEOF(st_ip_filter_cmd));
    oal_netbuf_put(pst_netbuf, ul_netbuf_len);

    for(ul_items_idx = 0; ul_items_idx < st_ip_filter_cmd.uc_item_count; ul_items_idx++)
    {
        pst_cmd_info->ast_filter_items_items[ul_items_idx].uc_protocol = (oal_uint8)pst_items[ul_items_idx].protocol;
        pst_cmd_info->ast_filter_items_items[ul_items_idx].us_port     = (oal_uint16)pst_items[ul_items_idx].port;
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
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_add_ip_filter_items::wal_send_cfg_event failed, error no[%d]!}", l_ret);
        oal_netbuf_free(pst_netbuf);
        return l_ret;
    }


    return OAL_SUCC;

}

oal_int32 wal_clear_ip_filter()
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
        if (g_st_dfr_info.bit_device_reset_process_flag)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_clear_ip_filter:: dfr_process_status[%d]!}",
                g_st_dfr_info.bit_device_reset_process_flag);
            return -OAL_EFAIL;
        }
#endif //#ifdef _PRE_WLAN_FEATURE_DFR

    pst_net_dev = oal_dev_get_by_name("wlan0");
    if (OAL_PTR_NULL == pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_clear_ip_filter::wlan0 not exist!}");
        return -OAL_EINVAL;
    }

    /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
    oal_dev_put(pst_net_dev);

    /* vapδ����ʱ���������·������� */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_clear_ip_filter::vap not created yet, ignore the cmd!.}");
        return -OAL_EINVAL;
    }

    if (OAL_TRUE != pst_mac_vap->st_cap_flag.bit_ip_filter)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_clear_ip_filter::Func not enable, ignore the cmd!.}");
        return -OAL_EINVAL;
    }

    /* ��������� */
    OAL_MEMZERO((oal_uint8 *)&st_ip_filter_cmd, OAL_SIZEOF(st_ip_filter_cmd));
    st_ip_filter_cmd.en_cmd        = MAC_IP_FILTER_CLEAR;

    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_clear_ip_filter::Now start clearing the list.}");

    /* ѡ�������¼��ռ�Ĵ�С */
    ul_netbuf_len = OAL_SIZEOF(st_ip_filter_cmd);

    /* ����ռ� ������˹��� */
    pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,ul_netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_clear_ip_filter::netbuf alloc null,size %d.}", ul_netbuf_len);
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
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_clear_ip_filter::wal_send_cfg_event failed, error no[%d]!}", l_ret);
        oal_netbuf_free(pst_netbuf);
        return l_ret;
    }

    return OAL_SUCC;
}
oal_int32 wal_register_ip_filter(wal_hw_wlan_filter_ops *pst_ip_filter_ops)
{
#ifdef CONFIG_DOZE_FILTER
    if (OAL_PTR_NULL == pst_ip_filter_ops)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_register_ip_filter::pg_st_ip_filter_ops is null !}");
        return -OAL_EINVAL;
    }
    hw_register_wlan_filter(pst_ip_filter_ops);
#else
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_register_ip_filter:: Not support CONFIG_DOZE_FILTER!}");
#endif
    return OAL_SUCC;
}

oal_int32 wal_unregister_ip_filter()
{
#ifdef CONFIG_DOZE_FILTER
    hw_unregister_wlan_filter();
#else
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_unregister_ip_filter:: Not support CONFIG_DOZE_FILTER!}");
#endif
    return OAL_SUCC;
}

#else
oal_int32 wal_set_ip_filter_enable(oal_int32 l_on)
{
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_set_ip_filter_enable::Ip_filter not support!}");
    return -OAL_EFAIL;
}
oal_int32 wal_add_ip_filter_items(wal_hw_wifi_filter_item *pst_items, oal_int32 l_count)
{
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_add_ip_filter_items::Ip_filter not support!}");
    return -OAL_EFAIL;
}

oal_int32 wal_clear_ip_filter()
{
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_clear_ip_filter::Ip_filter not support!}");

    return -OAL_EFAIL;
}


#endif //_PRE_WLAN_FEATURE_IP_FILTER

/*lint -e19*/
oal_module_symbol(wal_hipriv_proc_write);
oal_module_symbol(wal_hipriv_get_mac_addr);
#ifdef _PRE_WLAN_FEATURE_HILINK
oal_module_symbol(wal_config_get_all_sta_info);
#endif
/*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#ifdef _PRE_WLAN_RF_CALI

OAL_STATIC oal_uint32  wal_hipriv_auto_cali(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;
    oal_uint16                      us_len;


    if (OAL_UNLIKELY(WAL_MSG_WRITE_MAX_LEN <= OAL_STRLEN(pc_param)))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_auto_cali:: pc_param overlength is %d}\n", OAL_STRLEN(pc_param));
        return OAL_FAIL;
    }

    oal_memcopy(st_write_msg.auc_value, pc_param, OAL_STRLEN(pc_param));

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);


    /***************************************************************************
                              ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_AUTO_CALI, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                             (oal_uint8 *)&st_write_msg,
                             OAL_FALSE,
                             OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_auto_cali::wal_send_cfg_event return err_code [%d]!}\r\n", l_ret);
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
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_cali_ref::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }
    uc_chain_idx = (oal_uint8)oal_atoi(ac_arg);

    /* ��ȡУ׼vref idx */
    pc_param = pc_param + ul_off_set;
    ul_ret   = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_cali_ref::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }
    uc_band_idx = (oal_uint8)oal_atoi(ac_arg);

    /* ��ȡУ׼vrefֵ */
    pc_param = pc_param + ul_off_set;
    ul_ret   = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_cali_ref::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ul_ret);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

OAL_STATIC oal_uint32 wal_hipriv_set_pm_debug_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    mac_vap_stru                   *pst_mac_vap;
    oal_uint32                      ul_ret        = 0;
    oal_int32                       l_ret         = 0;
    oal_uint32                      ul_offset;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int8                        ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN];
    mac_pm_debug_cfg_stru          *pst_pm_debug_cfg;
    oal_uint8                       uc_switch;
    oal_bool_enum_uint8             en_cmd_updata = OAL_FALSE;
    oal_uint16                      us_val = 0;

    /* sh hipriv.sh "wlan0 pm_debug srb " */

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_pm_debug_switch::pst_mac_vap is null!}");
        return OAL_FAIL;
    }

    pst_pm_debug_cfg = (mac_pm_debug_cfg_stru*)(st_write_msg.auc_value);

    OAL_MEMZERO(pst_pm_debug_cfg, OAL_SIZEOF(mac_pm_debug_cfg_stru));

    do
    {
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_offset);
        if ((OAL_SUCC != ul_ret) && (0 != ul_offset))
        {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_set_pm_debug_switch::cmd format err, ret:%d;!!}\r\n", ul_ret);
            return ul_ret;
        }
        pc_param += ul_offset;

        if (OAL_FALSE == en_cmd_updata)
        {
            en_cmd_updata = OAL_TRUE;
        }
        else if (0 == ul_offset)
        {
            break;
        }

        ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, &ul_offset);
        if ((OAL_SUCC != ul_ret) || ((!isdigit(ac_value[0])) && (0 != oal_strcmp("help", ac_value))))
        {
            OAL_IO_PRINT("CMD format::sh hipriv.sh 'wlan0 pm_debug [srb 0|1]'\r\n");
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug [srb 0|1]'!!}\r\n");
            return ul_ret;
        }
        pc_param += ul_offset;
#ifdef _PRE_WLAN_FEATURE_APF
        if (0 == oal_strcmp("apf", ac_name))
        {
            if (0 == oal_strcmp("help", ac_value))
            {
                OAM_WARNING_LOG0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug apf [0|1]'}");
                return OAL_SUCC;
            }

            uc_switch = (oal_uint8)oal_atoi(ac_value);
            if (uc_switch > 1)
            {
                OAM_ERROR_LOG1(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug apf [0|1]', input[%d]!!}", uc_switch);
                return OAL_FAIL;
            }
            pst_pm_debug_cfg->ul_cmd_bit_map |= BIT(MAC_PM_DEBUG_APF);
            pst_pm_debug_cfg->uc_apf_switch = uc_switch;

            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_hipriv_set_pm_debug_switch::apf switch[%d].}", uc_switch);
        }
        else if (0 == oal_strcmp("ao", ac_name))
        {
            if (0 == oal_strcmp("help", ac_value))
            {
                OAM_WARNING_LOG0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug ao [0|1]'}");
                return OAL_SUCC;
            }

            uc_switch = (oal_uint8)oal_atoi(ac_value);
            if (uc_switch > 1)
            {
                OAM_ERROR_LOG1(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug ao [0|1]', input[%d]!!}", uc_switch);
                return OAL_FAIL;
            }
            pst_pm_debug_cfg->ul_cmd_bit_map |= BIT(MAC_PM_DEBUG_AO);
            pst_pm_debug_cfg->uc_ao_switch = uc_switch;

            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_hipriv_set_pm_debug_switch::arp offload switch[%d].}", uc_switch);
        }
#endif
#ifdef _PRE_PM_DYN_SET_TBTT_OFFSET
        else if (0 == oal_strcmp("dto", ac_name))
        {
            if (0 == oal_strcmp("help", ac_value))
            {
                OAM_WARNING_LOG0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug dto [0|1]'}");
                return OAL_SUCC;
            }

            uc_switch = (oal_uint8)oal_atoi(ac_value);
            if (uc_switch > 1)
            {
                OAM_ERROR_LOG1(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug dto [0|1]', input[%d]!!}", uc_switch);
                return OAL_FAIL;
            }
            pst_pm_debug_cfg->ul_cmd_bit_map |= BIT(MAC_PM_DEBUG_DTO);
            pst_pm_debug_cfg->uc_dto_switch = uc_switch;

            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_hipriv_set_pm_debug_switch::dto switch[%d].}", uc_switch);
        }
        else if(0 == oal_strcmp("torv", ac_name))
        {
            if(0 == oal_strcmp("help",ac_value))
            {
                OAM_WARNING_LOG0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug torv [0~550]'}");
                return OAL_SUCC;
            }

            us_val = (oal_uint16)oal_atoi(ac_value);
            if(us_val > 550)
            {
                OAM_ERROR_LOG1(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug torv [0~550]', intput[%d]!!}",us_val);
                return OAL_FAIL;
            }
            pst_pm_debug_cfg->ul_cmd_bit_map |= BIT(MAC_PM_DEBUG_TORV);
            pst_pm_debug_cfg->us_torv_val = us_val;

            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_hipriv_set_pm_debug_switch::torv val[%d].}", us_val);
        }
        else
        {
            OAL_IO_PRINT("CMD format::sh hipriv.sh 'wlan0 pm_debug [srb 0|1] [dto 0|1] [nfi 0|1] [apf 0|1] [ao 0|1] [dto 0|1] [torv 0~550]'");
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug [srb 0|1] [dto 0|1] [nfi 0|1] [apf 0|1] [dto 0|1] [torv 0~550]'!!}");
            return OAL_FAIL;
        }
#endif


    }while(*pc_param != '\0');

    /***************************************************************************
                               ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_PM_DEBUG_SWITCH, OAL_SIZEOF(mac_pm_debug_cfg_stru));

    l_ret = wal_send_cfg_event(pst_net_dev,
                              WAL_MSG_TYPE_WRITE,
                              WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_pm_debug_cfg_stru),
                              (oal_uint8 *)&st_write_msg,
                              OAL_FALSE,
                              OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
       OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_dbdc_debug_switch::return err code[%d]!}", l_ret);
       return (oal_uint32)l_ret;
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
    mac_cfg_tcp_ack_buf_stru       *pst_tcp_ack_param;

    pst_tcp_ack_param = (mac_cfg_tcp_ack_buf_stru *)(st_write_msg.auc_value);
    oal_memset((oal_void *)pst_tcp_ack_param, 0, OAL_SIZEOF(mac_cfg_tcp_ack_buf_stru));

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
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

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
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

    l_ret = wal_send_cfg_event(pst_net_dev,
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

OAL_STATIC oal_uint32  wal_hipriv_ps_params_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    oal_int32                       l_tmp;
    oal_uint8                       uc_idx;
    mac_cfg_ps_params_stru         *pst_ps_params;

    pst_ps_params = (mac_cfg_ps_params_stru *)(st_write_msg.auc_value);
    oal_memset((oal_void *)pst_ps_params, 0, OAL_SIZEOF(mac_cfg_ps_params_stru));

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_ps_params_cfg::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }
    pc_param = pc_param + ul_off_set;

    for (uc_idx = 0; uc_idx < MAC_PS_PARAMS_TYPE_BUTT; uc_idx++)
    {
        if(0 == oal_strcmp(ac_name, g_ast_hmac_ps_params_cfg_table[uc_idx].puc_string))
        {
            break;
        }
    }
    if (MAC_PS_PARAMS_TYPE_BUTT == uc_idx)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_hipriv_ps_params_cfg:: parameter error !\r\n");
        return uc_idx;
    }
    pst_ps_params->en_cmd = g_ast_hmac_ps_params_cfg_table[uc_idx].en_ps_params_cfg_id;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_ps_params_cfg::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    l_tmp = (oal_int32)oal_atoi(ac_name);
    if ((l_tmp < 0) || (l_tmp > 255))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_ps_params_cfg::  car param[%d] invalid! }\r\n", oal_atoi(ac_name));
        return OAL_FAIL;
    }

    if (MAC_PS_PARAMS_TIMEOUT == pst_ps_params->en_cmd)
    {
        if (0 == (oal_uint8)l_tmp)
        {
            OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_hipriv_ps_params_cfg:: timer_ms shoule not be 0.}");
            return OAL_FAIL;
        }

        pst_ps_params->uc_timeout_ms = (oal_uint8)l_tmp;
    }
    if (MAC_PS_PARAMS_RESTART_COUNT == pst_ps_params->en_cmd)
    {
        if (0 == (oal_uint8)l_tmp)
        {
            OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_hipriv_ps_params_cfg:: uc_restart_count shoule not be 0.}");
            return OAL_FAIL;
        }

        pst_ps_params->uc_restart_count = (oal_uint8)l_tmp;
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_PS_PARAMS, OAL_SIZEOF(mac_cfg_ps_params_stru));

    OAM_WARNING_LOG3(0, OAM_SF_CFG, "{wal_hipriv_ps_params_cfg::en_cmd[%d], uc_timeout_ms[%d] uc_restart_count[%d]!}\r\n",
        pst_ps_params->en_cmd,
        pst_ps_params->uc_timeout_ms,
        pst_ps_params->uc_restart_count);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ps_params_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_ps_params_cfg::return err code [%d]!}\r\n", ul_ret);
        return (oal_uint32)ul_ret;
    }

    return OAL_SUCC;


}

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
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_APF, OAL_SIZEOF(st_apf_filter_cmd));
    oal_memcopy(st_write_msg.auc_value, &st_apf_filter_cmd, OAL_SIZEOF(st_apf_filter_cmd));

    /* ������Ϣ */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_apf_filter_cmd),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    return (oal_uint32)l_ret;
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
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(uc_suspend),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    return l_ret;
}
#endif
#endif


OAL_STATIC oal_uint32  wal_hipriv_mcs_set_check_enable(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32                           ul_off_set;
    oal_int8                             ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                           ul_ret;
    oal_int32                            l_ret;
    wal_msg_write_stru                   st_write_msg;
    oal_bool_enum_uint8                 *pen_mcs_set_check_enable;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "wal_hipriv_ht_check_stop: get first arg fail.");
        return OAL_FAIL;
    }

    /***************************************************************************
                            ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFIGD_MCS_SET_CHECK_ENABLE, OAL_SIZEOF(oal_bool_enum_uint8));

    /* ��������������� */
    pen_mcs_set_check_enable  = (oal_bool_enum_uint8 *)(st_write_msg.auc_value);
    *pen_mcs_set_check_enable = (oal_bool_enum_uint8)oal_atoi(ac_arg);

    OAM_WARNING_LOG1(0, OAM_SF_SCAN, "wal_hipriv_mcs_set_check_enable_flag= %d.", *pen_mcs_set_check_enable);

    l_ret = wal_send_cfg_event(pst_net_dev,
                       WAL_MSG_TYPE_WRITE,
                       WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_bool_enum_uint8),
                       (oal_uint8 *)&st_write_msg,
                       OAL_FALSE,
                       OAL_PTR_NULL);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_hipriv_mcs_set_check_enable::wal_send_cfg_event fail.return err code [%d]!\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  wal_hipriv_set_tx_pow_param(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru               st_write_msg;
    mac_cfg_set_tx_pow_param_stru   *pst_set_tx_pow_param;
    oal_int8                         ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                       ul_off_set;
    oal_uint32                       ul_ret;
    oal_int32                        l_ret = OAL_SUCC;
    wal_dscr_param_enum_uint8        en_param_index;
    oal_uint8                        uc_value = 0;

    /* �����ʽ: hipriv "wlan0 set_tx_pow rf_reg_ctl 0/1" ,   0:��ʹ��, 1:ʹ��            */
    /* �����ʽ: hipriv "wlan0 set_tx_pow mag_level 0/1/2/3"  ���ù���֡���ʵȼ�          */
    /* �����ʽ: hipriv "wlan0 set_tx_pow ctl_level 0/1/2/3"  ���ÿ���֡���ʵȼ�          */
    /* �����ʽ: hipriv "wlan0 set_tx_pow show_log type (bw chn)" ��ʾ������־            */
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_TX_POW, OAL_SIZEOF(mac_cfg_set_tx_pow_param_stru));

    /* ��������������������� */
    pst_set_tx_pow_param = (mac_cfg_set_tx_pow_param_stru *)(st_write_msg.auc_value);
    OAL_MEMZERO(pst_set_tx_pow_param, OAL_SIZEOF(mac_cfg_set_tx_pow_param_stru));

    /* ��ȡ�������ֶ����������ַ��� */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /* ������������һ���ֶ� */
    for (en_param_index = 0; en_param_index < WAL_TX_POW_PARAM_BUTT; en_param_index++)
    {
        if(!oal_strcmp(pauc_tx_pow_param_name[en_param_index], ac_arg))
        {
            break;
        }
    }

    /* ��������Ƿ��� */
    if (WAL_TX_POW_PARAM_BUTT == en_param_index)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::no such param for tx pow!}\r\n");
        return OAL_FAIL;
    }

    pst_set_tx_pow_param->en_type = en_param_index;

    /* ��ȡ��һ������ */
    pc_param += ul_off_set;
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
    }
    else
    {
        uc_value = (oal_uint8)oal_atoi(ac_arg);
    }
    pst_set_tx_pow_param->auc_value[0] = uc_value;

    /*lint -e571 */
    /* ����У�� */
    switch(en_param_index)
    {
        case WAL_TX_POW_PARAM_SET_RF_REG_CTL:
            if (uc_value >= 2)
            {
                //OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::input val out of range [%d]!}\r\n", uc_value);
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
            break;
        case WAL_TX_POW_PARAM_SET_MAG_LEVEL:
        case WAL_TX_POW_PARAM_SET_CTL_LEVEL:
            if (uc_value > 4)
            {
                //OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::input pow level val out of range [%d]!}\r\n", uc_value);
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
            break;
        case WAL_TX_POW_PARAM_SHOW_LOG:
            if (pst_set_tx_pow_param->auc_value[0] == 1) //Ϊ0��ʾ��ѯgmֵ��Ϊ1��ʾ��ѯiq expect pow��صĻ�·����
            {
                //evm iq cali expect pow bw & chn
                /* ��ȡ��һ������bw */
                pc_param += ul_off_set;
                ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
                if (OAL_SUCC != ul_ret)
                {
                    OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::show log arg1 bw missing!}");
                    break;
                }
                pst_set_tx_pow_param->auc_value[1] = (oal_uint8)oal_atoi(ac_arg);

                /* ��ȡ��һ������chn_idx */
                pc_param += ul_off_set;
                ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
                if (OAL_SUCC != ul_ret)
                {
                    OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::show log arg2 chn_indx missing!}");
                    break;
                }
                pst_set_tx_pow_param->auc_value[2] = (oal_uint8)oal_atoi(ac_arg);
            }
            break;
        default:
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::error input type!}\r\n");
            //OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::input type[%d], val[%d]!}\r\n", en_param_index, uc_value);
            return OAL_FAIL;
    }
    /*lint +e571 */


    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_set_tx_pow_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}



OAL_STATIC oal_uint32  wal_hipriv_set_sar_level(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32                           ul_off_set;
    oal_int8                             ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                           ul_ret;
    oal_int32                            l_ret;
    oal_uint8                            uc_sar_level = 0;
    oal_uint16                           us_tx_power  = 1000;//���ϲ��·��������룬�ο�03�����ϲ��·�����Ϊ1001~1003

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "wal_hipriv_set_sar_level: get first arg fail.");
        return OAL_FAIL;
    }
    uc_sar_level =  (oal_uint8)oal_atoi(ac_arg);
    OAM_WARNING_LOG1(0,OAM_SF_ANY,"wal_hipriv_set_sar_level:sar level[%d]",uc_sar_level);
    /* SAR�ĵ�λ��1��20�� */
    if ((uc_sar_level > 20) || (0 == uc_sar_level))
    {
        /* �ֶ���������������·���� */
        OAM_ERROR_LOG1(0,OAM_SF_ANY,"wal_hipriv_set_sar_level:uc_sar_level[%d] is not in the range 1~20",uc_sar_level);
        return OAL_FAIL;
    }

    us_tx_power += uc_sar_level;

    l_ret = wal_ioctl_reduce_sar(pst_net_dev, us_tx_power);
    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(0,OAM_SF_ANY,"wal_hipriv_set_sar_level: reduce sar fail,l_ret[%d]",l_ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_BTCOEX

OAL_STATIC oal_uint32  wal_hipriv_set_btcoex_params(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_off_set;
    oal_int8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32                      ul_ret;
    oal_int32                       l_ret;
    mac_btcoex_cfg_type_enum_uint8  en_cfg_type;
    oal_uint8                       uc_cfg_value;
    mac_btcoex_cfg_stru            *pst_btcoex_params;

    pst_btcoex_params = (mac_btcoex_cfg_stru *)(st_write_msg.auc_value);
    oal_memset((oal_void *)pst_btcoex_params, 0, OAL_SIZEOF(mac_btcoex_cfg_stru));

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_set_btcoex_params::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }
    en_cfg_type = (oal_uint8)oal_atoi(ac_name);

    pc_param = pc_param + ul_off_set;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, &ul_off_set);
    if (OAL_SUCC != ul_ret)
    {
         OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_set_btcoex_params::wal_get_cmd_one_arg_etc return err_code [%d]!}\r\n", ul_ret);
         return ul_ret;
    }

    uc_cfg_value = (oal_uint8)oal_atoi(ac_name);

    pst_btcoex_params->en_cfg_type  = en_cfg_type;
    pst_btcoex_params->uc_cfg_value = uc_cfg_value;
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_BTCOEX_PARAMS, OAL_SIZEOF(mac_btcoex_cfg_stru));

    OAM_WARNING_LOG2(0, OAM_SF_CFG, "{wal_hipriv_set_btcoex_params::en_cfg_type[%d], uc_cfg_value[%d]!}\r\n", en_cfg_type, uc_cfg_value);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_btcoex_cfg_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_set_btcoex_params::return err code [%d]!}\r\n", l_ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;


}
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif



