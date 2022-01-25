



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"

#include "wlan_spec.h"

#include "hmac_ext_if.h"

#include "wal_main.h"
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
#ifdef CONFIG_DOZE_FILTER
#include <huawei_platform/power/wifi_filter/wifi_filter.h>
#endif
#endif /* _PRE_WLAN_FEATURE_IP_FILTER */
#include "wal_config.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_flowctl.h"
#include "wal_config_acs.h"

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
#include "oal_kernel_file.h"
#endif

#include "wal_dfx.h"

#include "hmac_vap.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_MAIN_C
/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
/* HOST CRX�ӱ� */
OAL_STATIC frw_event_sub_table_item_stru g_ast_wal_host_crx_table[WAL_HOST_CRX_SUBTYPE_BUTT];

/* HOST CTX�ֱ� */
OAL_STATIC frw_event_sub_table_item_stru g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_BUTT];

/* HOST DRX�ӱ� */
/* OAL_STATIC frw_event_sub_table_item_stru g_ast_wal_host_drx_table[WAL_HOST_DRX_SUBTYPE_BUTT]; */

/* wal���⹳�Ӻ��� */
oam_wal_func_hook_stru     g_st_wal_drv_func_hook;

oal_wakelock_stru   g_st_wal_wakelock;
#ifdef _PRE_E5_722_PLATFORM
oal_wakelock_stru   g_st_wifi_wakelock;
#endif
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
wal_hw_wlan_filter_ops g_st_ip_filter_ops = {
    .set_filter_enable = wal_set_ip_filter_enable,
    .add_filter_items = wal_add_ip_filter_items,
    .clear_filters = wal_clear_ip_filter,
    .get_filter_pkg_stat = NULL,
};
#else
wal_hw_wlan_filter_ops g_st_ip_filter_ops;
#endif
#endif /* _PRE_WLAN_FEATURE_IP_FILTER */
/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


oal_uint32  wal_event_fsm_init(oal_void)
{
    g_ast_wal_host_crx_table[WAL_HOST_CRX_SUBTYPE_CFG].p_func = wal_config_process_pkt;
    frw_event_table_register(FRW_EVENT_TYPE_HOST_CRX, FRW_EVENT_PIPELINE_STAGE_0, g_ast_wal_host_crx_table);

    /* g_ast_wal_host_drx_table[WAL_HOST_DRX_SUBTYPE_TX].p_func = hmac_tx_lan_to_wlan_ap;
    frw_event_table_register(FRW_EVENT_TYPE_HOST_DRX, FRW_EVENT_PIPELINE_STAGE_0, g_ast_wal_host_drx_table); */

    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_SCAN_COMP_STA].p_func     = wal_scan_comp_proc_sta;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_ASOC_COMP_STA].p_func     = wal_asoc_comp_proc_sta;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_DISASOC_COMP_STA].p_func  = wal_disasoc_comp_proc_sta;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_STA_CONNECT_AP].p_func    = wal_connect_new_sta_proc_ap;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_STA_DISCONNECT_AP].p_func  = wal_disconnect_sta_proc_ap;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_MIC_FAILURE].p_func       = wal_mic_failure_proc;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_RX_MGMT].p_func           = wal_send_mgmt_to_host;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_LISTEN_EXPIRED].p_func    = wal_p2p_listen_timeout;

#ifdef _PRE_SUPPORT_ACS
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_ACS_RESPONSE].p_func      = wal_acs_response_event_handler;
#endif

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_FLOWCTL_BACKP].p_func      = wal_flowctl_backp_event_handler;
#endif

    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_INIT].p_func  = wal_cfg80211_init_evt_handle;
#if (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5)
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_MGMT_TX_STATUS].p_func  = wal_cfg80211_mgmt_tx_status;
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_ROAM_COMP_STA].p_func   = wal_roam_comp_proc_sta;
#endif //_PRE_WLAN_FEATURE_ROAM
#ifdef _PRE_WLAN_FEATURE_11R
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_FT_EVENT_STA].p_func   = wal_ft_event_proc_sta;
#endif //_PRE_WLAN_FEATURE_11R
#ifdef _PRE_WLAN_FEATURE_VOWIFI
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_VOWIFI_REPORT].p_func   = wal_cfg80211_vowifi_report;
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE)
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_SAMPLE_REPORT].p_func   = wal_sample_report2sdt;
#endif
    frw_event_table_register(FRW_EVENT_TYPE_HOST_CTX, FRW_EVENT_PIPELINE_STAGE_0, g_ast_wal_host_ctx_table);

    return OAL_SUCC;
}


oal_uint32  wal_event_fsm_exit(oal_void)
{
    OAL_IO_PRINT("debug2");

    OAL_MEMZERO(g_ast_wal_host_crx_table, OAL_SIZEOF(g_ast_wal_host_crx_table));

    OAL_MEMZERO(g_ast_wal_host_ctx_table, OAL_SIZEOF(g_ast_wal_host_ctx_table));

    return OAL_SUCC;
}

#if defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

/*debug sysfs*/
OAL_STATIC oal_kobject* g_conn_syfs_wal_object = NULL;

oal_int32 wal_wakelock_info_print(char* buf, oal_int32 buf_len)
{
    oal_int32 ret = 0;

#ifdef CONFIG_PRINTK
    if(g_st_wal_wakelock.locked_addr)
    {
        ret +=  OAL_SPRINTF(buf + ret , buf_len - ret,"wakelocked by:%pf\n",
                    (oal_void*)g_st_wal_wakelock.locked_addr);
    }
#endif

    ret +=  OAL_SPRINTF(buf + ret , buf_len - ret,"hold %lu locks\n", g_st_wal_wakelock.lock_count);

    return ret;
}

OAL_STATIC ssize_t  wal_get_wakelock_info(struct device *dev, struct device_attribute *attr, char*buf)
{
    int ret = 0;
    if (NULL == buf)
    {
        OAL_IO_PRINT("buf is null r failed!%s\n",__FUNCTION__);
        return 0;
    }

    if (NULL == attr)
    {
        OAL_IO_PRINT("attr is null r failed!%s\n",__FUNCTION__);
        return 0;
    }

    if (NULL == dev)
    {
        OAL_IO_PRINT("dev is null r failed!%s\n",__FUNCTION__);
        return 0;
    }


    ret += wal_wakelock_info_print(buf,PAGE_SIZE - ret);

    return ret;
}

extern oal_int32 wal_atcmsrv_ioctl_get_rx_pckg(oal_net_device_stru *pst_net_dev, oal_int32 *pl_rx_pckg_succ_num);
OAL_STATIC ssize_t  wal_get_packet_statistics_wlan0_info(struct device *dev, struct device_attribute *attr, char*buf)
{
    ssize_t                     ret = 0;
    oal_net_device_stru*        pst_net_dev;
    mac_vap_stru*               pst_vap;
    hmac_vap_stru*              pst_hmac_vap;
    oal_int32                   l_rx_pckg_succ_num;
    oal_int32                   l_ret;

    if (NULL == buf)
    {
        OAL_IO_PRINT("buf is null r failed!%s\n",__FUNCTION__);
        return 0;
    }

    if (NULL == attr)
    {
        OAL_IO_PRINT("attr is null r failed!%s\n",__FUNCTION__);
        return 0;
    }

    if (NULL == dev)
    {
        OAL_IO_PRINT("dev is null r failed!%s\n",__FUNCTION__);
        return 0;
    }


    pst_net_dev = oal_dev_get_by_name("wlan0");
    if (OAL_PTR_NULL == pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_packet_statistics_wlan0_info_print::oal_dev_get_by_name return null ptr!}\r\n");
        return ret;
    }
    oal_dev_put(pst_net_dev);   /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
    /* ��ȡVAP�ṹ�� */
    pst_vap = (mac_vap_stru *)OAL_NET_DEV_PRIV(pst_net_dev);
    /* ���VAP�ṹ�岻���ڣ�����0 */
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_vap))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_packet_statistics_wlan0_info_print::pst_vap = OAL_PTR_NULL!}\r\n");
        return ret;
    }
    /* ��STAֱ�ӷ��� */
    if (WLAN_VAP_MODE_BSS_STA != pst_vap->en_vap_mode)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_packet_statistics_wlan0_info_print::vap_mode:%d.}\r\n", pst_vap->en_vap_mode);
        return ret;
    }
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_CFG, "{wal_packet_statistics_wlan0_info_print::pst_hmac_vap null.}");
        return ret;
    }

    l_ret = wal_atcmsrv_ioctl_get_rx_pckg(pst_net_dev, &l_rx_pckg_succ_num);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_packet_statistics_wlan0_info_print::wal_atcmsrv_ioctl_get_rx_pckg failed, failed err:%d!}\r\n", l_ret);
        return ret;
    }

    ret +=  OAL_SPRINTF(buf, PAGE_SIZE, "rx_packet=%d\r\n", l_rx_pckg_succ_num);

    return ret;
}

OAL_STATIC DEVICE_ATTR(wakelock, S_IRUGO, wal_get_wakelock_info, NULL);
OAL_STATIC DEVICE_ATTR(packet_statistics_wlan0, S_IRUGO, wal_get_packet_statistics_wlan0_info, NULL);

oal_int32 wal_msg_queue_info_print(char* buf, oal_int32 buf_len)
{
    oal_int32 ret = 0;

    ret +=  OAL_SPRINTF(buf + ret , buf_len - ret,"message count:%u\n", wal_get_request_msg_count());

    return ret;
}

OAL_STATIC ssize_t  wal_get_msg_queue_info(struct device *dev, struct device_attribute *attr, char*buf)
{
    int ret = 0;
    if (NULL == buf)
    {
        OAL_IO_PRINT("buf is null r failed!%s\n",__FUNCTION__);
        return 0;
    }

    if (NULL == attr)
    {
        OAL_IO_PRINT("attr is null r failed!%s\n",__FUNCTION__);
        return 0;
    }

    if (NULL == dev)
    {
        OAL_IO_PRINT("dev is null r failed!%s\n",__FUNCTION__);
        return 0;
    }


    ret += wal_msg_queue_info_print(buf,PAGE_SIZE - ret);

    return ret;
}
OAL_STATIC ssize_t  wal_get_dev_wifi_info_print(char* buf, oal_int32 buf_len)
{
    ssize_t                     ret = 0;
    oal_net_device_stru*        pst_net_dev;
    mac_vap_stru*               pst_vap;
    hmac_vap_stru*              pst_hmac_vap;

    if(NULL == buf)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "%s error: buf is null\r\n",__FUNCTION__);
        return 0;
    }

    pst_net_dev = oal_dev_get_by_name("wlan0");
    if (OAL_PTR_NULL == pst_net_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_get_dev_wifi_info_print::oal_dev_get_by_name return null ptr!}\r\n");
        return ret;
    }
    oal_dev_put(pst_net_dev);   /* ����oal_dev_get_by_name�󣬱������oal_dev_putʹnet_dev�����ü�����һ */
    /* ��ȡVAP�ṹ�� */
    pst_vap = (mac_vap_stru *)OAL_NET_DEV_PRIV(pst_net_dev);
    /* ���VAP�ṹ�岻���ڣ�����0 */
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_vap))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_get_dev_wifi_info_print::pst_vap = OAL_PTR_NULL!}\r\n");
        return ret;
    }
    /* ��STAֱ�ӷ��� */
    if (WLAN_VAP_MODE_BSS_STA != pst_vap->en_vap_mode)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_get_dev_wifi_info_print::vap_mode:%d.}\r\n", pst_vap->en_vap_mode);
        return ret;
    }
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_CFG, "{wal_get_dev_wifi_info_print::pst_hmac_vap null.}");
        return ret;
    }

    ret +=  OAL_SPRINTF(buf + ret , buf_len - ret, "tx_frame_amount:%d\n",pst_hmac_vap->station_info.tx_packets);
    ret +=  OAL_SPRINTF(buf + ret , buf_len - ret, "tx_byte_amount:%d\n",(oal_uint32)pst_hmac_vap->station_info.tx_bytes);
    ret +=  OAL_SPRINTF(buf + ret , buf_len - ret, "tx_data_frame_error_amount:%d\n",pst_hmac_vap->station_info.tx_failed);
    ret +=  OAL_SPRINTF(buf + ret , buf_len - ret, "tx_retrans_amount:%d\n",pst_hmac_vap->station_info.tx_retries);
    ret +=  OAL_SPRINTF(buf + ret , buf_len - ret, "rx_frame_amount:%d\n",pst_hmac_vap->station_info.rx_packets);
    ret +=  OAL_SPRINTF(buf + ret , buf_len - ret, "rx_byte_amount:%d\n",(oal_uint32)pst_hmac_vap->station_info.rx_bytes);
    ret +=  OAL_SPRINTF(buf + ret , buf_len - ret, "rx_beacon_from_assoc_ap:%d\n",pst_hmac_vap->st_station_info_extend.ul_bcn_cnt);
    ret +=  OAL_SPRINTF(buf + ret , buf_len - ret, "ap_distance:%d\n",pst_hmac_vap->st_station_info_extend.uc_distance);
    ret +=  OAL_SPRINTF(buf + ret , buf_len - ret, "disturbing_degree:%d\n",pst_hmac_vap->st_station_info_extend.uc_cca_intr);
    ret +=  OAL_SPRINTF(buf + ret , buf_len - ret, "lost_beacon_amount:%d\n",pst_hmac_vap->st_station_info_extend.ul_bcn_tout_cnt);

    return ret;
}
OAL_STATIC ssize_t  wal_get_dev_wifi_info(struct device *dev, struct device_attribute *attr, char*buf)
{
    int ret = 0;
    if (NULL == buf)
    {
        OAL_IO_PRINT("buf is null r failed!%s\n",__FUNCTION__);
        return 0;
    }

    if (NULL == attr)
    {
        OAL_IO_PRINT("attr is null r failed!%s\n",__FUNCTION__);
        return 0;
    }

    if (NULL == dev)
    {
        OAL_IO_PRINT("dev is null r failed!%s\n",__FUNCTION__);
        return 0;
    }


    ret += wal_get_dev_wifi_info_print(buf,PAGE_SIZE - ret);

    return ret;
}
OAL_STATIC DEVICE_ATTR(dev_wifi_info, S_IRUGO, wal_get_dev_wifi_info, NULL);

OAL_STATIC DEVICE_ATTR(msg_queue, S_IRUGO, wal_get_msg_queue_info, NULL);

OAL_STATIC struct attribute *wal_sysfs_entries[] = {
        &dev_attr_wakelock.attr,
        &dev_attr_msg_queue.attr,
        &dev_attr_packet_statistics_wlan0.attr,
        &dev_attr_dev_wifi_info.attr,
        NULL
};

OAL_STATIC struct attribute_group wal_attribute_group = {
        //.name = "vap",
        .attrs = wal_sysfs_entries,
};

OAL_STATIC oal_int32 wal_sysfs_entry_init(oal_void)
{
    oal_int32       ret = OAL_SUCC;
    oal_kobject*     pst_root_object = NULL;
    pst_root_object = oal_get_sysfs_root_object();
    if(NULL == pst_root_object)
    {
        OAM_ERROR_LOG0(0,OAM_SF_ANY,"{wal_sysfs_entry_init::get sysfs root object failed!}");
        return -OAL_EFAIL;
    }

    g_conn_syfs_wal_object = kobject_create_and_add("wal", pst_root_object);
    if(NULL == g_conn_syfs_wal_object)
    {
        OAM_ERROR_LOG0(0,OAM_SF_ANY,"{wal_sysfs_entry_init::create wal object failed!}");
        return -OAL_EFAIL;
    }

    ret = sysfs_create_group(g_conn_syfs_wal_object, &wal_attribute_group);
    if (ret)
    {
        kobject_put(g_conn_syfs_wal_object);
        OAM_ERROR_LOG0(0,OAM_SF_ANY,"{wal_sysfs_entry_init::sysfs create group failed!}");
        return ret;
    }
    return OAL_SUCC;
}

OAL_STATIC oal_int32 wal_sysfs_entry_exit(oal_void)
{
    if(g_conn_syfs_wal_object)
    {
        sysfs_remove_group(g_conn_syfs_wal_object, &wal_attribute_group);
        kobject_put(g_conn_syfs_wal_object);
    }
    return OAL_SUCC;
}
#endif


oal_int32  wal_main_init(oal_void)
{
    oal_uint32           ul_ret;
    frw_init_enum_uint16 en_init_state;

    oal_wake_lock_init(&g_st_wal_wakelock, "wlan_wal_lock");
#ifdef _PRE_E5_722_PLATFORM
    oal_wake_lock_init(&g_st_wifi_wakelock, "wifi_lock");
    wifi_wake_lock();
#endif
    wal_msg_queue_init();

    en_init_state = frw_get_init_state();
    /* WALģ���ʼ����ʼʱ��˵��HMAC�϶��Ѿ���ʼ���ɹ� */
    if ((FRW_INIT_STATE_BUTT == en_init_state) || (en_init_state < FRW_INIT_STATE_HMAC_CONFIG_VAP_SUCC))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_main_init::en_init_state has a invalid value [%d]!}\r\n", en_init_state);

        frw_timer_delete_all_timer();
        return -OAL_EFAIL;
    }

    wal_event_fsm_init();

    /* ����proc */
    ul_ret = wal_hipriv_create_proc(OAL_PTR_NULL);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_main_init::wal_hipriv_create_proc has a wrong return value [%d]!}\r\n", ul_ret);

        frw_timer_delete_all_timer();
        return -OAL_EFAIL;
    }

    /* ��ʼ��ÿ��deviceӲ���豸��Ӧ��wiphy */
    ul_ret = wal_cfg80211_init();
    if (ul_ret != OAL_SUCC)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_main_init::wal_cfg80211_init has a wrong return value [%d]!}\r\n", ul_ret);

        frw_timer_delete_all_timer();
        return -OAL_EFAIL;
    }

    /* ��host�����WAL��ʼ���ɹ�����Ϊȫ����ʼ���ɹ� */
    frw_set_init_state(FRW_INIT_STATE_ALL_SUCC);


    /* wal���Ӻ�����ʼ�� */
    wal_drv_cfg_func_hook_init();

    /* wal����⹳�Ӻ���ע����oamģ�� */
    oam_wal_func_fook_register(&g_st_wal_drv_func_hook);

#ifdef _PRE_WLAN_FEATURE_P2P
    /* ��ʼ��cfg80211 ɾ�������豸�������� */
    g_pst_del_virtual_inf_workqueue = OAL_CREATE_SINGLETHREAD_WORKQUEUE("cfg80211_del_virtual_inf");
    if (!g_pst_del_virtual_inf_workqueue)
    {
        wal_cfg80211_exit();
        frw_timer_delete_all_timer();
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_main_init::Failed to create cfg80211 del virtual infterface workqueue!}");

        return -OAL_EFAIL;
    }
#endif /* _PRE_WLAN_FEATURE_P2P */

#ifdef _PRE_SUPPORT_ACS
    wal_acs_init();
#endif

#if defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
/*debug sysfs*/
    wal_sysfs_entry_init();
#endif
#ifdef _PRE_WLAN_FEATURE_DFR
    wal_dfx_init();
#endif //#ifdef _PRE_WLAN_FEATURE_DFR
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wal_set_custom_process_func(wal_custom_cali, wal_priv_init_config);
#endif
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    wal_register_ip_filter(&g_st_ip_filter_ops);
#endif /* _PRE_WLAN_FEATURE_IP_FILTER */

    return OAL_SUCC;
}


oal_void  wal_destroy_all_vap(oal_void)
{
#if (_PRE_TEST_MODE_UT != _PRE_TEST_MODE)

    oal_uint8               uc_vap_id = 0;
    oal_net_device_stru    *pst_net_dev;
    oal_wireless_dev_stru  *pst_wdev;
    oal_int8                ac_param[10] = {0};
    OAL_IO_PRINT("wal_destroy_all_vap start");
    for (uc_vap_id = WLAN_SERVICE_VAP_START_ID_PER_BOARD; uc_vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vap_id++)
    {
        pst_net_dev = hmac_vap_get_net_device(uc_vap_id);
        if (OAL_PTR_NULL != pst_net_dev)
        {
            oal_net_close_dev(pst_net_dev);

            wal_hipriv_del_vap(pst_net_dev, ac_param);
            frw_event_process_all_event(0);
        }
    }

    pst_net_dev=oal_dev_get_by_name("wlan0");
    if(OAL_PTR_NULL != pst_net_dev)
    {
        oal_dev_put(pst_net_dev);
        pst_wdev = OAL_NETDEVICE_WDEV(pst_net_dev);
        oal_net_unregister_netdev(pst_net_dev);
        if (pst_wdev)
        {
            OAL_IO_PRINT("%s:free wlan0 wdev\n", __func__);
            OAL_MEM_FREE(pst_wdev, OAL_TRUE);
        }
    }
    pst_net_dev=oal_dev_get_by_name("p2p0");
    if(OAL_PTR_NULL != pst_net_dev)
    {
        oal_dev_put(pst_net_dev);
        pst_wdev = OAL_NETDEVICE_WDEV(pst_net_dev);
        oal_net_unregister_netdev(pst_net_dev);
        if (pst_wdev)
        {
            OAL_IO_PRINT("%s:free p2p0 wdev\n", __func__);
            OAL_MEM_FREE(pst_wdev, OAL_TRUE);
        }
    }

#endif

    return;
}


oal_void  wal_main_exit(oal_void)
{
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wal_set_custom_process_func(OAL_PTR_NULL,OAL_PTR_NULL);
#endif

#if defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
/*debug sysfs*/
    wal_sysfs_entry_exit();
#endif
    /* down�����е�vap */
    wal_destroy_all_vap();

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /* ж��ÿ��deviceӲ���豸��Ӧ��wiphy */
    wal_cfg80211_exit();
#endif
    wal_event_fsm_exit();

    /* ɾ��proc */
    wal_hipriv_remove_proc();


    /* ж�سɹ�ʱ������ʼ��״̬��ΪHMAC��ʼ���ɹ� */
    frw_set_init_state(FRW_INIT_STATE_HMAC_CONFIG_VAP_SUCC);

    /* wal���Ӻ���ȥ��ʼ�� */
    wal_drv_cfg_func_hook_deinit();

    /* ȥע�ṳ�Ӻ��� */
    oam_wal_func_fook_unregister();
#ifdef _PRE_WLAN_FEATURE_P2P
    /* ɾ��cfg80211 ɾ�������豸�������� */
    if (g_pst_del_virtual_inf_workqueue)
    {
        oal_destroy_workqueue(g_pst_del_virtual_inf_workqueue);
        g_pst_del_virtual_inf_workqueue = OAL_PTR_NULL;
    }
#endif

#ifdef _PRE_SUPPORT_ACS
    wal_acs_exit();
#endif
#ifdef _PRE_WLAN_FEATURE_DFR
    wal_dfx_exit();
#endif //#ifdef _PRE_WLAN_FEATURE_DFR

    oal_wake_lock_exit(&g_st_wal_wakelock);
#ifdef _PRE_E5_722_PLATFORM
    wifi_wake_unlock();
    oal_wake_lock_exit(&g_st_wifi_wakelock);
#endif

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    wal_unregister_ip_filter();
#endif /* _PRE_WLAN_FEATURE_IP_FILTER */

}

/*lint -e578*//*lint -e19*/
oal_module_symbol(wal_main_init);
oal_module_symbol(wal_main_exit);

oal_module_license("GPL");
/*lint +e578*//*lint +e19*/




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

