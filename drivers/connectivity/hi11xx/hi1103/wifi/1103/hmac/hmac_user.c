



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "oam_ext_if.h"
#include "dmac_ext_if.h"
#include "hmac_user.h"
#include "hmac_main.h"
#include "hmac_tx_amsdu.h"
#include "hmac_protection.h"
#include "hmac_smps.h"
#include "hmac_ext_if.h"
#include "hmac_config.h"
#include "hmac_mgmt_ap.h"
#include "hmac_chan_mgmt.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
#include "hmac_proxy_arp.h"
#endif

#ifdef _PRE_WLAN_FEATURE_WAPI
#include "hmac_wapi.h"
#endif

#if defined(_PRE_WLAN_FEATURE_MCAST) || defined(_PRE_WLAN_FEATURE_HERA_MCAST)
#include "hmac_m2u.h"
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_main.h"
#endif //_PRE_WLAN_FEATURE_ROAM

#ifdef _PRE_SUPPORT_ACS
#include "hmac_acs.h"
#endif

#include "hmac_blockack.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#if defined (_PRE_WLAN_FEATURE_WDS) || defined (_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
#include "hmac_wds.h"
#endif
#include "hmac_scan.h"
#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
#include "hmac_11k.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_USER_C
/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
hmac_user_stru g_ast_hmac_user[MAC_RES_MAX_USER_LIMIT];
#endif

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


hmac_user_stru*  mac_res_get_hmac_user_alloc_etc(oal_uint16 us_idx)
{
    hmac_user_stru *pst_hmac_user;

    pst_hmac_user = (hmac_user_stru*)_mac_res_get_hmac_user(us_idx);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{mac_res_get_hmac_user_init::pst_hmac_user null,user_idx=%d.}", us_idx);
        return OAL_PTR_NULL;
    }

    /* �ظ������쳣,����Ӱ��ҵ����ʱ��ӡerror���������� */
    if (MAC_USER_ALLOCED == pst_hmac_user->st_user_base_info.uc_is_user_alloced)
    {
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{mac_res_get_hmac_user_init::[E]user has been alloced,user_idx=%d.}", us_idx);
    }

    return pst_hmac_user;
}


hmac_user_stru*  mac_res_get_hmac_user_etc(oal_uint16 us_idx)
{
    hmac_user_stru *pst_hmac_user;

    pst_hmac_user = (hmac_user_stru*)_mac_res_get_hmac_user(us_idx);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        //OAM_ERROR_LOG1(0, OAM_SF_UM, "{mac_res_get_hmac_user_etc::pst_hmac_user null,user_idx=%d.}", us_idx);
        return OAL_PTR_NULL;
    }

    /* �쳣: �û���Դ�ѱ��ͷ� */
    if (MAC_USER_ALLOCED != pst_hmac_user->st_user_base_info.uc_is_user_alloced)
    {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        oal_mem_print_funcname_etc(OAL_RET_ADDR);
#endif
        /* host���ȡ�û�ʱ�û��Ѿ��ͷ��������������ؿ�ָ�룬���������߲����û�ʧ�ܣ����ӡWARNING��ֱ���ͷ�buf����������֧�ȵ� */
        return OAL_PTR_NULL;
    }

    return pst_hmac_user;
}



oal_uint32  hmac_user_alloc_etc(oal_uint16 *pus_user_idx)
{
    hmac_user_stru *pst_hmac_user;
    oal_uint32      ul_rslt;
    oal_uint16      us_user_idx_temp;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pus_user_idx))
    {
        OAM_ERROR_LOG0(0, OAM_SF_UM, "{hmac_user_alloc_etc::pus_user_idx null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ����hmac user�ڴ� */
    /*lint -e413*/
    ul_rslt = mac_res_alloc_hmac_user(&us_user_idx_temp, OAL_OFFSET_OF(hmac_user_stru, st_user_base_info));
    if (OAL_SUCC != ul_rslt)
    {
        OAM_WARNING_LOG1(0, OAM_SF_UM, "{hmac_user_alloc_etc::mac_res_alloc_hmac_user failed[%d].}", ul_rslt);
        return ul_rslt;
    }
    /*lint +e413*/

    pst_hmac_user = mac_res_get_hmac_user_alloc_etc(us_user_idx_temp);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        mac_res_free_mac_user_etc(us_user_idx_temp);
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{hmac_user_alloc_etc::pst_hmac_user null,user_idx=%d.}", us_user_idx_temp);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ʼ��0 */
    OAL_MEMZERO(pst_hmac_user, OAL_SIZEOF(hmac_user_stru));
    /* ���user��Դ�ѱ�alloc */
    pst_hmac_user->st_user_base_info.uc_is_user_alloced = MAC_USER_ALLOCED;

    *pus_user_idx = us_user_idx_temp;

    return OAL_SUCC;
}


oal_uint32  hmac_user_free_etc(oal_uint16 us_idx)
{
    hmac_user_stru *pst_hmac_user;
    oal_uint32      ul_ret;

    pst_hmac_user = mac_res_get_hmac_user_etc(us_idx);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{hmac_user_free_etc::pst_hmac_user null,user_idx=%d.}", us_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

#if 0 //�ظ��ͷ��������⣬����ǰ��ֱ�Ӵ�ӡERROR��Ȼ����ж�λ��
    /* �ظ��ͷ��쳣, �����ͷŲ����� */
    if (MAC_USER_FREED == pst_hmac_user->st_user_base_info.uc_is_user_alloced)
    {
        OAM_WARNING_LOG1(0, OAM_SF_UM, "{hmac_user_free_etc::[E]user has been freed,user_idx=%d.}", us_idx);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        oal_mem_print_funcname_etc(OAL_RET_ADDR);
#endif
    }
#endif

    ul_ret = mac_res_free_mac_user_etc(us_idx);
    if(OAL_SUCC == ul_ret)
    {
        /* ���alloc��־ */
        pst_hmac_user->st_user_base_info.uc_is_user_alloced = MAC_USER_FREED;
    }

    return ul_ret;
}


oal_uint32  hmac_user_free_asoc_req_ie(oal_uint16 us_idx)
{
    hmac_user_stru  *pst_hmac_user;

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(us_idx);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ASSOC, "{hmac_vap_free_asoc_req_ie::pst_hmac_user[%d] null.}", us_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_PTR_NULL != pst_hmac_user->puc_assoc_req_ie_buff)
    {
        OAL_MEM_FREE(pst_hmac_user->puc_assoc_req_ie_buff, OAL_TRUE);
        pst_hmac_user->puc_assoc_req_ie_buff = OAL_PTR_NULL;
        pst_hmac_user->ul_assoc_req_ie_len   = 0;
    }
    else
    {
        pst_hmac_user->ul_assoc_req_ie_len   = 0;
    }
    return OAL_SUCC;
}


oal_uint32  hmac_user_set_asoc_req_ie(hmac_user_stru *pst_hmac_user, oal_uint8 *puc_payload, oal_uint32 ul_payload_len, oal_uint8 uc_reass_flag)
{
    /* �ع����ȹ�������֡ͷ����AP��MAC��ַ  */
    ul_payload_len -= ((uc_reass_flag == OAL_TRUE) ? WLAN_MAC_ADDR_LEN : 0);

    pst_hmac_user->puc_assoc_req_ie_buff = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, (oal_uint16)ul_payload_len, OAL_TRUE);
    if(OAL_PTR_NULL == pst_hmac_user->puc_assoc_req_ie_buff)
    {
        OAM_ERROR_LOG1(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_user_set_asoc_req_ie :: Alloc %u bytes failed for puc_assoc_req_ie_buff failed .}",
                       (oal_uint16)ul_payload_len);
        pst_hmac_user->ul_assoc_req_ie_len = 0;
        return OAL_FAIL;
    }
    oal_memcopy(pst_hmac_user->puc_assoc_req_ie_buff,
                puc_payload + ((uc_reass_flag == OAL_TRUE) ? WLAN_MAC_ADDR_LEN : 0),
                ul_payload_len);
    pst_hmac_user->ul_assoc_req_ie_len = ul_payload_len;

    return OAL_SUCC;
}


oal_uint32  hmac_user_init_etc(hmac_user_stru *pst_hmac_user)
{
    oal_uint8        uc_tid_loop;
    hmac_ba_tx_stru *pst_tx_ba;

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    oal_uint8        uc_ac_idx;
    oal_uint8        uc_data_idx;
#endif

    /* ��ʼ��tid��Ϣ */
    for (uc_tid_loop = 0; uc_tid_loop < WLAN_TID_MAX_NUM; uc_tid_loop++)
    {
        pst_hmac_user->ast_tid_info[uc_tid_loop].uc_tid_no      = (oal_uint8)uc_tid_loop;

        //pst_hmac_user->ast_tid_info[uc_tid_loop].pst_hmac_user  = (oal_void *)pst_hmac_user;
        pst_hmac_user->ast_tid_info[uc_tid_loop].us_hmac_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;

        /* ��ʼ��ba rx������� */
        pst_hmac_user->ast_tid_info[uc_tid_loop].pst_ba_rx_info = OAL_PTR_NULL;
        oal_spin_lock_init(&(pst_hmac_user->ast_tid_info[uc_tid_loop].st_ba_tx_info.st_ba_status_lock));
        pst_hmac_user->ast_tid_info[uc_tid_loop].st_ba_tx_info.en_ba_status     = DMAC_BA_INIT;
        pst_hmac_user->ast_tid_info[uc_tid_loop].st_ba_tx_info.uc_addba_attemps = 0;
        pst_hmac_user->ast_tid_info[uc_tid_loop].st_ba_tx_info.uc_dialog_token  = 0;
        pst_hmac_user->ast_tid_info[uc_tid_loop].st_ba_tx_info.uc_ba_policy     = 0;
        pst_hmac_user->ast_tid_info[uc_tid_loop].en_ba_handle_tx_enable         = OAL_TRUE;
        pst_hmac_user->ast_tid_info[uc_tid_loop].en_ba_handle_rx_enable         = OAL_TRUE;

        pst_hmac_user->auc_ba_flag[uc_tid_loop] = 0;

        /* addba req��ʱ�����������д */
        pst_tx_ba = &pst_hmac_user->ast_tid_info[uc_tid_loop].st_ba_tx_info;
        pst_tx_ba->st_alarm_data.pst_ba = (oal_void *)pst_tx_ba;
        pst_tx_ba->st_alarm_data.uc_tid = uc_tid_loop;
        pst_tx_ba->st_alarm_data.us_mac_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;
        pst_tx_ba->st_alarm_data.uc_vap_id = pst_hmac_user->st_user_base_info.uc_vap_id;

        /* ��ʼ���û���������֡���� */
        pst_hmac_user->puc_assoc_req_ie_buff = OAL_PTR_NULL;
        pst_hmac_user->ul_assoc_req_ie_len   = 0;

        oal_spin_lock_init(&(pst_hmac_user->ast_tid_info[uc_tid_loop].st_ba_timer_lock));
    }

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    for (uc_ac_idx = 0; uc_ac_idx < WLAN_WME_AC_BUTT; uc_ac_idx++)
    {
        for (uc_data_idx = 0; uc_data_idx < WLAN_TXRX_DATA_BUTT; uc_data_idx++)
        {
            pst_hmac_user->aaul_txrx_data_stat[uc_ac_idx][uc_data_idx] = 0;
        }
    }
#endif

    pst_hmac_user->pst_defrag_netbuf         = OAL_PTR_NULL;
    pst_hmac_user->en_user_bw_limit          = OAL_FALSE;
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    pst_hmac_user->st_sa_query_info.ul_sa_query_count      = 0;
    pst_hmac_user->st_sa_query_info.ul_sa_query_start_time = 0;
#endif
    OAL_MEMZERO(&pst_hmac_user->st_defrag_timer, OAL_SIZEOF(frw_timeout_stru));
    pst_hmac_user->ul_rx_pkt_drop = 0;

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    /* ���usrͳ����Ϣ */
    oam_stats_clear_user_stat_info_etc(pst_hmac_user->st_user_base_info.us_assoc_id);
#endif

    pst_hmac_user->ul_first_add_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();

#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
    pst_hmac_user->us_clear_judge_count = 0;
#endif

    return OAL_SUCC;
}

oal_uint32  hmac_user_set_avail_num_space_stream_etc(mac_user_stru *pst_mac_user, wlan_nss_enum_uint8 en_vap_nss)
{
    mac_user_ht_hdl_stru         *pst_mac_ht_hdl;
    mac_vht_hdl_stru             *pst_mac_vht_hdl;
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl_stru              *pst_mac_he_hdl;
#endif
    wlan_nss_enum_uint8           en_user_num_spatial_stream = 0;
    oal_uint32                    ul_ret = OAL_SUCC;

     /* AP(STA)Ϊlegacy�豸��ֻ֧��1�����ߣ�����Ҫ���ж����߸��� */

    /* ��ȡHT��VHT�ṹ��ָ�� */
    pst_mac_ht_hdl  = &(pst_mac_user->st_ht_hdl);
    pst_mac_vht_hdl = &(pst_mac_user->st_vht_hdl);
#ifdef _PRE_WLAN_FEATURE_11AX
    pst_mac_he_hdl = &(pst_mac_user->st_he_hdl);
    if(OAL_TRUE == pst_mac_he_hdl->en_he_capable)
    {
        if(3 != pst_mac_he_hdl->st_he_cap_ie.st_he_mcs_nss.st_rx_he_mcs_below_80mhz.bit_max_he_mcs_for_4ss)
        {
            en_user_num_spatial_stream = WLAN_FOUR_NSS;
        }
        else if(3 != pst_mac_he_hdl->st_he_cap_ie.st_he_mcs_nss.st_rx_he_mcs_below_80mhz.bit_max_he_mcs_for_3ss)
        {
            en_user_num_spatial_stream = WLAN_TRIPLE_NSS;
        }
        else if(3 != pst_mac_he_hdl->st_he_cap_ie.st_he_mcs_nss.st_rx_he_mcs_below_80mhz.bit_max_he_mcs_for_2ss)
        {
            en_user_num_spatial_stream = WLAN_DOUBLE_NSS;
        }
        else if(3 != pst_mac_he_hdl->st_he_cap_ie.st_he_mcs_nss.st_rx_he_mcs_below_80mhz.bit_max_he_mcs_for_1ss)
        {
            en_user_num_spatial_stream = WLAN_SINGLE_NSS;
        }
        else
        {
            OAM_WARNING_LOG0(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{hmac_user_set_avail_num_space_stream_etc::invalid he nss.}");
            ul_ret =  OAL_FAIL;
        }
    }
    else if (OAL_TRUE == pst_mac_vht_hdl->en_vht_capable)
#else
    if (OAL_TRUE == pst_mac_vht_hdl->en_vht_capable)
#endif
    {
        if (3 != pst_mac_vht_hdl->st_rx_max_mcs_map.us_max_mcs_4ss)
        {
            en_user_num_spatial_stream = WLAN_FOUR_NSS;
        }
        else if (3 != pst_mac_vht_hdl->st_rx_max_mcs_map.us_max_mcs_3ss)
        {
            en_user_num_spatial_stream = WLAN_TRIPLE_NSS;
        }
        else if (3 != pst_mac_vht_hdl->st_rx_max_mcs_map.us_max_mcs_2ss)
        {
            en_user_num_spatial_stream = WLAN_DOUBLE_NSS;
        }
        else if (3 != pst_mac_vht_hdl->st_rx_max_mcs_map.us_max_mcs_1ss)
        {
            en_user_num_spatial_stream = WLAN_SINGLE_NSS;
        }
        else
        {
            OAM_WARNING_LOG0(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{hmac_user_set_avail_num_space_stream_etc::invalid vht nss.}");

            ul_ret =  OAL_FAIL;
        }
    }
    else if (OAL_TRUE == pst_mac_ht_hdl->en_ht_capable)
    {
        if (pst_mac_ht_hdl->uc_rx_mcs_bitmask[3] > 0)
        {
            en_user_num_spatial_stream = WLAN_FOUR_NSS;
        }
        else if (pst_mac_ht_hdl->uc_rx_mcs_bitmask[2] > 0)
        {
            en_user_num_spatial_stream = WLAN_TRIPLE_NSS;
        }
        else if (pst_mac_ht_hdl->uc_rx_mcs_bitmask[1] > 0)
        {
            en_user_num_spatial_stream = WLAN_DOUBLE_NSS;
        }
        else if (pst_mac_ht_hdl->uc_rx_mcs_bitmask[0] > 0)
        {
            en_user_num_spatial_stream = WLAN_SINGLE_NSS;
        }
        else
        {
            OAM_WARNING_LOG0(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{hmac_user_set_avail_num_space_stream_etc::invalid ht nss.}");

            ul_ret =  OAL_FAIL;
        }
    }
    else
    {
        en_user_num_spatial_stream = WLAN_SINGLE_NSS;
    }

    /* ��ֵ���û��ṹ����� */
    mac_user_set_num_spatial_stream_etc(pst_mac_user, en_user_num_spatial_stream);
    mac_user_set_avail_num_spatial_stream_etc(pst_mac_user, OAL_MIN(pst_mac_user->en_user_num_spatial_stream, en_vap_nss));

    return ul_ret;
}

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)

OAL_STATIC oal_void hmac_stop_sa_query_timer(hmac_user_stru *pst_hmac_user)
{
    frw_timeout_stru    *pst_sa_query_interval_timer;

    pst_sa_query_interval_timer = &(pst_hmac_user->st_sa_query_info.st_sa_query_interval_timer);
    if (OAL_FALSE != pst_sa_query_interval_timer->en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(pst_sa_query_interval_timer);
    }
}
#endif


#ifdef _PRE_WLAN_FEATURE_WAPI
hmac_wapi_stru * hmac_user_get_wapi_ptr_etc(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_pairwise, oal_uint16 us_pairwise_idx)

{
    hmac_user_stru                  *pst_hmac_user;
    //oal_uint32                       ul_ret;
    oal_uint16                       us_user_index;

    if (OAL_TRUE == en_pairwise)
    {
        us_user_index = us_pairwise_idx;
    }
    else
    {
        us_user_index = pst_mac_vap->us_multi_user_idx;
    }

    //OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_user_get_wapi_ptr_etc::en_pairwise == %u, usridx==%u.}", en_pairwise, us_user_index);
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(us_user_index);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_user_get_wapi_ptr_etc::pst_hmac_user[%d] null.}", us_user_index);
        return OAL_PTR_NULL;
    }

    return &pst_hmac_user->st_wapi;
}
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX

oal_void hmac_compability_ap_tpye_identify_for_btcoex_etc(mac_vap_stru *pst_mac_vap,
    mac_bss_dscr_stru *pst_bss_dscr, oal_uint8 *puc_mac_addr, mac_ap_type_enum_uint16 *pen_ap_type)
{
    wlan_nss_enum_uint8      en_support_max_nss = WLAN_SINGLE_NSS;

    if (OAL_PTR_NULL != pst_bss_dscr)
    {
        /* һ����Ч��ֱ����self-cts,ƻ��·����������������������ʱ���� */

        /* ������Ч����Ҫ����mac��ַԼ�� */
        if(pst_bss_dscr->en_btcoex_blacklist_chip_oui & MAC_BTCOEX_BLACKLIST_LEV0)
        {
            /*����apʶ��: AP OUI + chip OUI + ˫�� + 2G ��Ҫdmac��ˢ��ps����ʱone pkt֡�������� */
            if(MAC_IS_DLINK_AP(puc_mac_addr) || MAC_IS_HAIER_AP(puc_mac_addr) || MAC_IS_FAST_AP(puc_mac_addr) || MAC_IS_TPINK_845_AP(puc_mac_addr))
            {
            #ifdef _PRE_WLAN_FEATURE_M2S
                en_support_max_nss = pst_bss_dscr->en_support_max_nss;
            #endif
                if ((WLAN_DOUBLE_NSS == en_support_max_nss) && (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band))
                {
                    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_COEX, "hmac_compability_ap_tpye_identify_for_btcoex_etc: lev0 btcoex ps self-cts blacklist!");
                    *pen_ap_type |= MAC_AP_TYPE_BTCOEX_PS_BLACKLIST;
                }
            }
            /* JCG·����Ϊ�˼���aptxHD��660,��Ҫ�ر�cts�ظ����� */
            else if(MAC_IS_JCG_AP(puc_mac_addr))
            {
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_COEX, "hmac_compability_ap_tpye_identify_for_btcoex_etc: lev0 JCG btcoex disable cts blacklist!");
                *pen_ap_type |= MAC_AP_TYPE_BTCOEX_DISABLE_CTS;
            }
            else if(MAC_IS_FEIXUN_K3(puc_mac_addr))
            {
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_COEX, "hmac_compability_ap_tpye_identify_for_btcoex_etc: lev0 k3 btcoex ps self-cts blacklist!");
                *pen_ap_type |= MAC_AP_TYPE_BTCOEX_PS_BLACKLIST;
            }
        }
    }
}
#endif


mac_ap_type_enum_uint16 hmac_compability_ap_tpye_identify_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_mac_addr)
{
    mac_ap_type_enum_uint16 en_ap_type = 0;

#if defined(_PRE_WLAN_1103_DDC_BUGFIX) || defined(_PRE_WLAN_FEATURE_BTCOEX)
    wlan_nss_enum_uint8             en_support_max_nss = WLAN_SINGLE_NSS;
#endif

    mac_bss_dscr_stru              *pst_bss_dscr = OAL_PTR_NULL;

    pst_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(pst_mac_vap, puc_mac_addr);

    if(MAC_IS_GOLDEN_AP(puc_mac_addr))
    {
        en_ap_type |= MAC_AP_TYPE_GOLDENAP;
    }

    if (OAL_PTR_NULL != pst_bss_dscr)
    {
#ifdef _PRE_WLAN_1103_DDC_BUGFIX
        /*DDC������: AP OUI + chip OUI + ���� + 2G*/
        if(MAC_IS_BELKIN_AP(puc_mac_addr) || MAC_IS_TRENDNET_AP(puc_mac_addr) || MAC_IS_NETGEAR_WNDR_AP(puc_mac_addr))
        {

#ifdef _PRE_WLAN_FEATURE_M2S
            en_support_max_nss = pst_bss_dscr->en_support_max_nss;
#endif
            if (OAL_TRUE == pst_bss_dscr->en_atheros_chip_oui)
            {
                if((MAC_IS_NETGEAR_WNDR_AP(puc_mac_addr) && WLAN_DOUBLE_NSS == en_support_max_nss)
                    || ((MAC_IS_BELKIN_AP(puc_mac_addr)|| MAC_IS_TRENDNET_AP(puc_mac_addr))
                    && (WLAN_TRIPLE_NSS == en_support_max_nss) && (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)))
                {
                    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "hmac_compability_ap_tpye_identify_etc is in ddc whitelist!");
                    en_ap_type |= MAC_AP_TYPE_DDC_WHITELIST;
                }
            }
        }
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
        hmac_compability_ap_tpye_identify_for_btcoex_etc(pst_mac_vap, pst_bss_dscr, puc_mac_addr, &en_ap_type);
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
        /* ����ʱ��ʶ��360����wifi����2G��TP-LINK7300,����������siso */
        if(MAC_IS_360_AP0(puc_mac_addr) || MAC_IS_360_AP1(puc_mac_addr) || MAC_IS_360_AP2(puc_mac_addr) ||
            (MAC_IS_TP_LINK_7300(pst_bss_dscr) && (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)))
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "hmac_compability_ap_tpye_identify_etc: m2s blacklist!");
            en_ap_type |= MAC_AP_TYPE_M2S;
        }
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
        /* �ر�ʱ��ʶ���Ѷk3�����ܽ������� */
        if(MAC_IS_FEIXUN_K3(puc_mac_addr))
        {
            if (OAL_TRUE == pst_bss_dscr->en_roam_blacklist_chip_oui)
            {
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "hmac_compability_ap_tpye_identify_etc: roam blacklist!");
                en_ap_type |= MAC_AP_TYPE_ROAM;
            }
        }
#endif

#ifdef _PRE_WLAN_FEATURE_160M
        if ((WLAN_AP_CHIP_OUI_BCM == pst_bss_dscr->en_is_tplink_oui)
         && ((pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS)
          && (pst_mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS)))
        {
            en_ap_type |= MAC_AP_TYPE_160M_OP_MODE;
        }
#endif

        if ((OAL_TRUE == pst_bss_dscr->en_atheros_chip_oui) && (MAC_IS_LINKSYS_EA8500_AP(puc_mac_addr)))
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "hmac_compability_ap_tpye_identify_etc: aggr blacklist!");
            en_ap_type |= MAC_AP_TYPE_AGGR_BLACKLIST;
        }
    }

    /* ��׮1102�߼� */
    en_ap_type |=  MAC_AP_TYPE_NORMAL;

    return en_ap_type;
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_void hmac_sdio_to_pcie_switch(mac_vap_stru *pst_mac_vap)
{
     /* ���VAP������5G,���л�ΪPCIE */
    if(WLAN_BAND_5G == pst_mac_vap->st_channel.en_band)
    {
        hi110x_switch_to_hcc_highspeed_chan(1);
    }
}



oal_void hmac_pcie_to_sdio_switch(mac_chip_stru *pst_mac_chip)
{
     /* chip�������������û������л�ΪSDIO */
    if(0 == pst_mac_chip->uc_assoc_user_cnt)
    {
        hi110x_switch_to_hcc_highspeed_chan(0);
    }
}
#endif


oal_void hmac_dbdc_need_kick_user(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev)
{
    oal_uint8       uc_vap_idx;
    mac_vap_stru   *pst_another_vap;
    mac_cfg_kick_user_param_stru  st_kick_user_param;
    oal_uint8       auc_mac_addr[WLAN_MAC_ADDR_LEN] = {0xff,0xff,0xff,0xff,0xff,0xff};

    if(OAL_FALSE == pst_mac_dev->en_dbdc_running)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_UM, "hmac_dbdc_need_kick_user::dbdc running[%d] band[%d]",
                            pst_mac_dev->en_dbdc_running, pst_mac_vap->st_channel.en_band);
        return;
    }

    /* DBDC״̬�£������ߵ����ڹ���״̬���û� */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_dev->uc_vap_num; uc_vap_idx++)
    {
        pst_another_vap = mac_res_get_mac_vap(pst_mac_dev->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_another_vap)
        {
            continue;
        }

        if (pst_another_vap->uc_vap_id == pst_mac_vap->uc_vap_id)
        {
            continue;
        }

        if(MAC_VAP_STATE_STA_JOIN_COMP <= pst_another_vap->en_vap_state && MAC_VAP_STATE_STA_WAIT_ASOC >= pst_another_vap->en_vap_state)
        {
            OAM_WARNING_LOG2(pst_another_vap->uc_vap_id, OAM_SF_UM, "hmac_dbdc_need_kick_user::vap mode[%d] state[%d] kick user",
                                pst_another_vap->en_vap_mode, pst_another_vap->en_vap_state);

            st_kick_user_param.us_reason_code = MAC_UNSPEC_REASON;
            oal_set_mac_addr(st_kick_user_param.auc_mac_addr, auc_mac_addr);
            hmac_config_kick_user_etc(pst_another_vap, OAL_SIZEOF(oal_uint32), (oal_uint8 *)&st_kick_user_param);

            return;
        }
    }
}


oal_uint32  hmac_user_del_etc(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user)
{
    oal_uint16                      us_user_index;
    frw_event_mem_stru             *pst_event_mem;
    frw_event_stru                 *pst_event;
    dmac_ctx_del_user_stru         *pst_del_user_payload;
    hmac_vap_stru                  *pst_hmac_vap;
    mac_device_stru                *pst_mac_device;
    mac_user_stru                  *pst_mac_user;
    oal_uint32                      ul_ret;
    mac_chip_stru                  *pst_mac_chip;
#ifdef _PRE_WLAN_FEATURE_WAPI
    hmac_user_stru                 *pst_hmac_user_multi;
#endif
    mac_ap_type_enum_uint16         en_ap_type = 0;

#ifdef _PRE_WLAN_FEATURE_WMMAC
    oal_uint8                       uc_ac_index = 0;
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_int8                        pc_param[16]    = {0};
    oal_int8                        pc_tmp[8]       = {0};
    oal_uint16                      us_len;
#endif
#endif

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == pst_hmac_user)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_UM, "{hmac_user_del_etc::param null,%d %d.}", pst_mac_vap, pst_hmac_user);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_user = (mac_user_stru*)(&pst_hmac_user->st_user_base_info);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
    {
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{hmac_user_del_etc::pst_mac_user param null,%d.}", pst_mac_user);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_del_etc::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_dbdc_need_kick_user(pst_mac_vap, pst_mac_device);

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_del_etc::del user[%d] start,is multi user[%d], user mac:XX:XX:XX:XX:%02X:%02X}",
                                pst_mac_user->us_assoc_id,
                                pst_mac_user->en_is_multi_user,
                                pst_mac_user->auc_user_mac_addr[4],
                                pst_mac_user->auc_user_mac_addr[5]);
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /*����arp̽��timer*/
    if (OAL_TRUE == pst_hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_arp_req_process.st_delba_opt_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_arp_req_process.st_delba_opt_timer));
    }
#endif

    /*ɾ��userʱ����Ҫ���±�������*/
    ul_ret = hmac_protection_del_user_etc(pst_mac_vap, &(pst_hmac_user->st_user_base_info));
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_UM, "{hmac_user_del_etc::hmac_protection_del_user_etc[%d]}", ul_ret);
    }

    /*ɾ���û�ͳ�Ʊ�����ص���Ϣ����dmacͬ��Ȼ��������Ӧ�ı���ģʽ*/
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        ul_ret = hmac_user_protection_sync_data(pst_mac_vap);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                        "{hmac_ap_up_update_sta_user::protection update mib failed, ret=%d.}", ul_ret);
        }

    }
#endif

     /* ��ȡ�û���Ӧ������ */
    us_user_index = pst_hmac_user->st_user_base_info.us_assoc_id;

    /* ɾ��hmac user �Ĺ�������֡�ռ� */
    hmac_user_free_asoc_req_ie(pst_hmac_user->st_user_base_info.us_assoc_id);

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    hmac_stop_sa_query_timer(pst_hmac_user);
#endif

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
    hmac_proxy_remove_mac(pst_mac_vap, pst_hmac_user->st_user_base_info.auc_user_mac_addr);
#endif

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_del_etc::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef  _PRE_WLAN_FEATURE_VOWIFI
    if (OAL_PTR_NULL != pst_hmac_vap->st_vap_base_info.pst_vowifi_cfg_param)
    {
        if (VOWIFI_LOW_THRES_REPORT == pst_hmac_vap->st_vap_base_info.pst_vowifi_cfg_param->en_vowifi_mode)
        {
            /* ������κ�ȥ��������,�л�vowifi����״̬ */
            hmac_config_vowifi_report_etc((&pst_hmac_vap->st_vap_base_info), 0, OAL_PTR_NULL);
        }
    }
#endif /* _PRE_WLAN_FEATURE_VOWIFI */

    pst_mac_chip = hmac_res_get_mac_chip(pst_mac_device->uc_chip_id);
    if (OAL_PTR_NULL == pst_mac_chip)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_del_etc::pst_mac_chip null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_WAPI
    hmac_wapi_deinit_etc(&pst_hmac_user->st_wapi);

     /*STAģʽ�£����鲥wapi���ܶ˿�*/
    pst_hmac_user_multi = (hmac_user_stru *)mac_res_get_hmac_user_etc(pst_hmac_vap->st_vap_base_info.us_multi_user_idx);
    if (OAL_PTR_NULL == pst_hmac_user_multi)
    {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{hmac_user_del_etc::mac_res_get_hmac_user_etc fail! user_idx[%u]}",pst_hmac_vap->st_vap_base_info.us_multi_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_wapi_reset_port_etc(&pst_hmac_user_multi->st_wapi);

    pst_mac_device->uc_wapi = OAL_FALSE;

#endif

#if defined(_PRE_WLAN_FEATURE_MCAST) || defined(_PRE_WLAN_FEATURE_HERA_MCAST)
    /*�û�ȥ����ʱ���snoop�����еĸó�Ա */
    if (OAL_PTR_NULL != pst_hmac_vap->pst_m2u)
    {
        hmac_m2u_cleanup_snoopwds_node(pst_hmac_user);
    }
#endif

    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {

#ifdef _PRE_WLAN_FEATURE_STA_PM
        mac_vap_set_aid_etc(pst_mac_vap, 0);
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
        
        hmac_roam_wpas_connect_state_notify_etc(pst_hmac_vap, WPAS_CONNECT_STATE_IPADDR_REMOVED);

        hmac_roam_exit_etc(pst_hmac_vap);
#endif //_PRE_WLAN_FEATURE_ROAM

        en_ap_type = hmac_compability_ap_tpye_identify_etc(pst_mac_vap, pst_mac_user->auc_user_mac_addr);
    }

	/* ɾ����Ӧwds�ڵ� */
#if defined (_PRE_WLAN_FEATURE_WDS) || defined (_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
    if ((OAL_TRUE == pst_hmac_user->uc_is_wds)
        && (OAL_SUCC != hmac_wds_del_node(pst_hmac_vap, pst_mac_user->auc_user_mac_addr)))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_del_etc::hmac_wds_del_node fail.}");
    }
#endif

    /***************************************************************************
        ���¼���DMAC��, ɾ��dmac�û�
    ***************************************************************************/
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_ctx_del_user_stru));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_del_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_del_user_payload = (dmac_ctx_del_user_stru *)pst_event->auc_event_data;
    pst_del_user_payload->us_user_idx = us_user_index;
    pst_del_user_payload->en_ap_type  = en_ap_type;
#if (_PRE_OS_VERSION_WIN32 != _PRE_OS_VERSION)
    /* TBD: ��Ӵ˲���51DMT�쳣���ݿ������쳣ԭ�� */
    /* �û� mac��ַ��idx ������һ����Ч����dmac����Ҵ�ɾ�����û� */
    oal_memcopy(pst_del_user_payload->auc_user_mac_addr, pst_mac_user->auc_user_mac_addr, WLAN_MAC_ADDR_LEN);
#endif

    /* ����¼�ͷ */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                        FRW_EVENT_TYPE_WLAN_CTX,
                        DMAC_WLAN_CTX_EVENT_SUB_TYPE_DEL_USER,
                        OAL_SIZEOF(dmac_ctx_del_user_stru),
                        FRW_EVENT_PIPELINE_STAGE_1,
                        pst_mac_vap->uc_chip_id,
                        pst_mac_vap->uc_device_id,
                        pst_mac_vap->uc_vap_id);

    ul_ret = frw_event_dispatch_event_etc(pst_event_mem);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        /* ��ά�⣬���ɾ���û�ʧ�ܣ�ǰ����hmac��Դ�Ĳ��������Ѿ��쳣����Ҫ��λ */
        FRW_EVENT_FREE(pst_event_mem);
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_del_etc::frw_event_dispatch_event_etc failed[%d].}", ul_ret);
        return ul_ret;
    }

    FRW_EVENT_FREE(pst_event_mem);

#ifdef _PRE_WLAN_FEATURE_SMPS
    /* ɾ���û�������SMPS���� */
    //hmac_smps_update_status(pst_mac_vap, &(pst_hmac_user->st_user_base_info), OAL_FALSE);
    mac_user_set_sm_power_save(&pst_hmac_user->st_user_base_info, 0);
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST) && defined(_PRE_WLAN_FEATURE_WMMAC)
    /*ɾ��userʱɾ������addts req��ʱ��ʱ��*/
    for(uc_ac_index = 0; uc_ac_index < WLAN_WME_AC_BUTT; uc_ac_index++)
    {
        if (OAL_TRUE == pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_index].st_addts_timer.en_is_registerd)
        {
            FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_index].st_addts_timer));
        }

        OAL_MEMZERO(&(pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_index]), OAL_SIZEOF(mac_ts_stru));
        pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_index].uc_tsid = 0xFF;
    }
#endif //#if defined(_PRE_PRODUCT_ID_HI110X_HOST) && defined(_PRE_WLAN_FEATURE_WMMAC)

#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    hmac_11k_exit_user(pst_hmac_user);
#endif

    hmac_tid_clear_etc(pst_mac_vap, pst_hmac_user);

    if (pst_hmac_user->st_mgmt_timer.en_is_registerd == OAL_TRUE)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_hmac_user->st_mgmt_timer);
    }

    if (pst_hmac_user->st_defrag_timer.en_is_registerd == OAL_TRUE)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_hmac_user->st_defrag_timer);
    }

    /* ��vap��ɾ���û� */
    mac_vap_del_user_etc(pst_mac_vap, us_user_index);

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    hmac_chan_update_40M_intol_user_etc(pst_mac_vap);
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (pst_mac_vap->us_user_nums == 5)
    {
        /* AP�û��ﵽ5ʱ���������ز���Ϊ�����ļ�ԭ��ֵ */
        oal_itoa(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_USED_MEM_FOR_START), pc_param, 5);
        oal_itoa(hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_USED_MEM_FOR_STOP), pc_tmp, 5);
        pc_param[OAL_STRLEN(pc_param)] = ' ';
        oal_memcopy(pc_param + OAL_STRLEN(pc_param), pc_tmp, OAL_STRLEN(pc_tmp));

        us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);
        hmac_config_sdio_flowctrl_etc(pst_mac_vap, us_len, pc_param);
    }
#endif
#endif
    /* �ͷ��û��ڴ� */
    ul_ret = hmac_user_free_etc(us_user_index);
    if(OAL_SUCC == ul_ret)
    {
        /* chip���ѹ���user����-- */
        mac_chip_dec_assoc_user(pst_mac_chip);
    }
    else
    {
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{hmac_user_del_etc::mac_res_free_mac_user_etc fail[%d].}", ul_ret);
    }

#ifdef _PRE_SUPPORT_ACS
    hmac_acs_register_rescan_timer(pst_mac_vap->uc_device_id);
#endif

    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        hmac_fsm_change_state_etc(pst_hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
    }
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* ɾ��5G�û�ʱ�迼���ǲ�����ΪSDIO */
    hmac_pcie_to_sdio_switch(pst_mac_chip);
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
    pst_hmac_vap->en_roam_prohibit_on = OAL_FALSE;
#endif

    return OAL_SUCC;
}


oal_uint32  hmac_user_add_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_mac_addr, oal_uint16 *pus_user_index)
{
    hmac_vap_stru                  *pst_hmac_vap;
    hmac_user_stru                 *pst_hmac_user;
    oal_uint32                      ul_ret;
    frw_event_mem_stru             *pst_event_mem;
    frw_event_stru                 *pst_event;
    dmac_ctx_add_user_stru         *pst_add_user_payload;
    oal_uint16                      us_user_idx;
    mac_cfg_80211_ucast_switch_stru st_80211_ucast_switch;
    mac_ap_type_enum_uint16         en_ap_type = 0;
    mac_chip_stru                  *pst_mac_chip;
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    oal_uint8                       uc_hipriv_ack = OAL_FALSE;
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_int8                        pc_param[]    = "30 25";
    oal_uint16                      us_len;
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_WAPI
    mac_device_stru                *pst_mac_device;
#endif
    mac_bss_dscr_stru              *pst_bss_dscr;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == puc_mac_addr) || (OAL_PTR_NULL == pus_user_index)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_UM, "{hmac_user_add_etc::param null, %d %d %d.}", pst_mac_vap, puc_mac_addr, pus_user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_UM, "{hmac_user_add_etc::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_WAPI
    pst_mac_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_etc::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

    pst_mac_chip = hmac_res_get_mac_chip(pst_mac_vap->uc_chip_id);
    if (OAL_PTR_NULL == pst_mac_chip)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_etc::pst_mac_chip null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* chip��������û����ж� */
    if (pst_mac_chip->uc_assoc_user_cnt >= mac_chip_get_max_asoc_user(pst_mac_chip->uc_chip_id))
    {
        OAM_WARNING_LOG1(0, OAM_SF_UM, "{hmac_user_add_etc::invalid uc_assoc_user_cnt[%d].}", pst_mac_chip->uc_assoc_user_cnt);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    if (pst_hmac_vap->st_vap_base_info.us_user_nums >= mac_mib_get_MaxAssocUserNums(pst_mac_vap))
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_etc::invalid us_user_nums[%d], us_user_nums_max[%d].}",
                         pst_hmac_vap->st_vap_base_info.us_user_nums, mac_mib_get_MaxAssocUserNums(pst_mac_vap));
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    /* ������û��Ѿ��������򷵻�ʧ�� */
    ul_ret = mac_vap_find_user_by_macaddr_etc(pst_mac_vap, puc_mac_addr, &us_user_idx);
    if (OAL_SUCC == ul_ret)
    {
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
        uc_hipriv_ack = OAL_TRUE;
        hmac_hipriv_proc_write_process_rsp(pst_mac_vap, sizeof(oal_uint8), &uc_hipriv_ack);
#endif
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_etc::mac_vap_find_user_by_macaddr_etc success[%d].}", ul_ret);
        return OAL_FAIL;
    }

    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
    #ifdef _PRE_WLAN_FEATURE_P2P
        if (IS_P2P_CL(pst_mac_vap))
        {
            if (pst_hmac_vap->st_vap_base_info.us_user_nums >= 2)
            {
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_etc::a STA can only associated with 2 ap.}");
                return OAL_FAIL;
            }
        }
        else
    #endif
        {
            if (pst_hmac_vap->st_vap_base_info.us_user_nums >= 1)
            {
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_etc::a STA can only associated with one ap.}");
                return OAL_FAIL;
            }
            en_ap_type = hmac_compability_ap_tpye_identify_etc(pst_mac_vap, puc_mac_addr);
        }
#ifdef _PRE_WLAN_FEATURE_ROAM
        hmac_roam_init_etc(pst_hmac_vap);
#endif //_PRE_WLAN_FEATURE_ROAM

#ifdef _PRE_WLAN_FEATURE_M2S
        pst_hmac_vap->st_hmac_vap_m2s.en_arp_probe_on = OAL_FALSE;
#endif
    }

    /* ����hmac�û��ڴ棬����ʼ��0 */
    ul_ret = hmac_user_alloc_etc(&us_user_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_etc::hmac_user_alloc_etc failed[%d].}", ul_ret);
        return ul_ret;
    }

    /* �����û�����ʹ��useridΪ0������������һ������userid��Ϊaid������Զˣ�����psmʱ����� */
    if (0 == us_user_idx)
    {
        hmac_user_free_etc(us_user_idx);
        ul_ret = hmac_user_alloc_etc(&us_user_idx);
        if ((OAL_SUCC != ul_ret) || (0 == us_user_idx))
        {
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_etc::hmac_user_alloc_etc failed ret[%d] us_user_idx[%d].}", ul_ret, us_user_idx);
            return ul_ret;
        }
    }

    *pus_user_index = us_user_idx;  /* ���θ�ֵ */

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(us_user_idx);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_etc::pst_hmac_user[%d] null.}", us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user->en_user_ap_type = en_ap_type;  /* AP���� */

    /* ��ʼ��mac_user_stru */
    mac_user_init_etc(&(pst_hmac_user->st_user_base_info), us_user_idx, puc_mac_addr,
                  pst_mac_vap->uc_chip_id,
                  pst_mac_vap->uc_device_id,
                  pst_mac_vap->uc_vap_id);

#ifdef _PRE_WLAN_FEATURE_WAPI
    /* ��ʼ������wapi���� */
    hmac_wapi_init_etc(&pst_hmac_user->st_wapi, OAL_TRUE);
    pst_mac_device->uc_wapi = OAL_FALSE;
#endif

    /* ����amsdu�� */
    hmac_amsdu_init_user_etc(pst_hmac_user);

    /***************************************************************************
        ���¼���DMAC��, ����dmac�û�
    ***************************************************************************/
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_ctx_add_user_stru));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        /* �쳣�����ͷ��ڴ棬device�¹����û�����û��++�����ﲻ��Ҫ�жϷ���ֵ��--���� */
        hmac_user_free_etc(us_user_idx);

        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_add_user_payload = (dmac_ctx_add_user_stru *)pst_event->auc_event_data;
    pst_add_user_payload->us_user_idx = us_user_idx;
    pst_add_user_payload->en_ap_type  = en_ap_type;
    oal_set_mac_addr(pst_add_user_payload->auc_user_mac_addr, puc_mac_addr);

    /* ��ȡɨ���bss��Ϣ */
    pst_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(pst_mac_vap, puc_mac_addr);
    if (OAL_PTR_NULL != pst_bss_dscr)
    {
        pst_add_user_payload->c_rssi = pst_bss_dscr->c_rssi;
    }
    else
    {
        pst_add_user_payload->c_rssi = oal_get_real_rssi((oal_int16)OAL_RSSI_INIT_MARKER);
    }

    /* ����¼�ͷ */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                        FRW_EVENT_TYPE_WLAN_CTX,
                        DMAC_WLAN_CTX_EVENT_SUB_TYPE_ADD_USER,
                        OAL_SIZEOF(dmac_ctx_add_user_stru),
                        FRW_EVENT_PIPELINE_STAGE_1,
                        pst_mac_vap->uc_chip_id,
                        pst_mac_vap->uc_device_id,
                        pst_mac_vap->uc_vap_id);

    ul_ret = frw_event_dispatch_event_etc(pst_event_mem);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        /* �쳣�����ͷ��ڴ棬device�¹����û�����û��++�����ﲻ��Ҫ�жϷ���ֵ��--���� */
        hmac_user_free_etc(us_user_idx);
        FRW_EVENT_FREE(pst_event_mem);
        /* ��Ӧ�ó����û����ʧ�ܣ�ʧ����Ҫ��λ����ԭ�� */
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_etc::frw_event_dispatch_event_etc failed[%d].}", ul_ret);
        return ul_ret;
    }

    FRW_EVENT_FREE(pst_event_mem);

    /* ����û���MAC VAP */
    ul_ret = mac_vap_add_assoc_user_etc(pst_mac_vap, us_user_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_etc::mac_vap_add_assoc_user_etc failed[%d].}", ul_ret);

        /* �쳣�����ͷ��ڴ棬device�¹����û�����û��++�����ﲻ��Ҫ�жϷ���ֵ��--���� */
        hmac_user_free_etc(us_user_idx);
        return OAL_FAIL;
    }

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (pst_mac_vap->us_user_nums == 6)
    {
        /* AP�û��ﵽ6ʱ���������ز���ΪStopΪ25��StartΪ30 */
        us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);
        hmac_config_sdio_flowctrl_etc(pst_mac_vap, us_len, pc_param);
    }
#endif
#endif

    /* ��ʼ��hmac user������Ϣ */
    hmac_user_init_etc(pst_hmac_user);

    mac_chip_inc_assoc_user(pst_mac_chip);
   // pst_mac_chip->uc_active_user_cnt++;

    /* ��80211��������֡���أ��۲�������̣������ɹ��˾͹ر� */
    st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_TX;
    st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
    st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_ON;
    st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_ON;
    st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_ON;
    oal_memcopy(st_80211_ucast_switch.auc_user_macaddr,
                (const oal_void *)puc_mac_addr,
                OAL_SIZEOF(st_80211_ucast_switch.auc_user_macaddr));
    hmac_config_80211_ucast_switch_etc(pst_mac_vap,OAL_SIZEOF(st_80211_ucast_switch),(oal_uint8 *)&st_80211_ucast_switch);

    st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_RX;
    st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
    st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_ON;
    st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_ON;
    st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_ON;
    hmac_config_80211_ucast_switch_etc(pst_mac_vap,OAL_SIZEOF(st_80211_ucast_switch),(oal_uint8 *)&st_80211_ucast_switch);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* ���5G�û�ʱ�迼���ǲ�����ΪPCIE */
    hmac_sdio_to_pcie_switch(pst_mac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
    if(en_ap_type & MAC_AP_TYPE_ROAM)
    {
        pst_hmac_vap->en_roam_prohibit_on = OAL_TRUE;
    }
    else
    {
        pst_hmac_vap->en_roam_prohibit_on = OAL_FALSE;
    }
#endif

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_etc::user[%d] mac:%02X:XX:XX:XX:%02X:%02X}",
                                us_user_idx,
                                puc_mac_addr[0],
                                puc_mac_addr[4],
                                puc_mac_addr[5]);

    return OAL_SUCC;
}


oal_uint32  hmac_config_add_user_etc(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_add_user_param_stru    *pst_add_user;
    oal_uint16                      us_user_index;
    hmac_vap_stru                  *pst_hmac_vap;
    hmac_user_stru                 *pst_hmac_user;
    oal_uint32                      ul_ret;
    mac_user_ht_hdl_stru            st_ht_hdl;
    oal_uint32                      ul_rslt;
    mac_device_stru                *pst_mac_device;
    mac_chip_stru                  *pst_mac_chip;

    pst_add_user = (mac_cfg_add_user_param_stru *)puc_param;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);

    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_UM, "{hmac_config_add_user_etc::pst_hmac_vap null.}");
        return OAL_FAIL;
    }

    ul_ret = hmac_user_add_etc(pst_mac_vap, pst_add_user->auc_mac_addr, &us_user_index);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_add_user_etc::hmac_user_add_etc failed.}", ul_ret);
        return ul_ret;
    }

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(us_user_index);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_add_user_etc::pst_hmac_user[%d] null.}", us_user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* TBD hmac_config_add_user_etc �˽ӿ�ɾ������Ӧ������Ҫ���ģ�duankaiyong&guyanjie */

    /* ����qos�򣬺���������Ҫ����ͨ����������������� */
    mac_user_set_qos_etc(&pst_hmac_user->st_user_base_info, OAL_TRUE);

    /* ����HT�� */
    mac_user_get_ht_hdl_etc(&pst_hmac_user->st_user_base_info, &st_ht_hdl);
    st_ht_hdl.en_ht_capable = pst_add_user->en_ht_cap;

    if (OAL_TRUE == pst_add_user->en_ht_cap)
    {
        pst_hmac_user->st_user_base_info.en_cur_protocol_mode                = WLAN_HT_MODE;
        pst_hmac_user->st_user_base_info.en_avail_protocol_mode              = WLAN_HT_MODE;
    }

    /* ����HT��ص���Ϣ:Ӧ���ڹ�����ʱ��ֵ ���ֵ���õĺ������д����� 2012->page:786 */
    st_ht_hdl.uc_min_mpdu_start_spacing = 6;
    st_ht_hdl.uc_max_rx_ampdu_factor    = 3;
    mac_user_set_ht_hdl_etc(&pst_hmac_user->st_user_base_info, &st_ht_hdl);

    mac_user_set_asoc_state_etc(&pst_hmac_user->st_user_base_info, MAC_USER_STATE_ASSOC);

    /* ����amsdu�� */
    hmac_amsdu_init_user_etc(pst_hmac_user);

    /***************************************************************************
        ���¼���DMAC��, ͬ��DMAC����
    ***************************************************************************/
    /* ��������DMAC��Ҫ�Ĳ��� */
    pst_add_user->us_user_idx = us_user_index;

    ul_ret = hmac_config_send_event_etc(&pst_hmac_vap->st_vap_base_info,
                                    WLAN_CFGID_ADD_USER,
                                    us_len,
                                    puc_param);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        /* �쳣�����ͷ��ڴ� */
        ul_rslt = hmac_user_free_etc(us_user_index);
        if(OAL_SUCC == ul_rslt)
        {
            pst_mac_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
            if (OAL_PTR_NULL == pst_mac_device)
            {
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_add_user_etc::pst_mac_device null.}");
                return OAL_ERR_CODE_PTR_NULL;
            }

            pst_mac_chip = hmac_res_get_mac_chip(pst_mac_device->uc_chip_id);
            if (OAL_PTR_NULL == pst_mac_chip)
            {
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_etc::pst_mac_chip null.}");
                return OAL_ERR_CODE_PTR_NULL;
            }

            /* hmac_add_user�ɹ�ʱchip�¹����û����Ѿ�++, �����chip���ѹ���user����Ҫ-- */
            mac_chip_dec_assoc_user(pst_mac_chip);
        }

        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_add_user_etc::hmac_config_send_event_etc failed[%d].}", ul_ret);
        return ul_ret;
    }

    /* �����û���֪ͨ�㷨��ʼ���û��ṹ�� */
    hmac_user_add_notify_alg_etc(&pst_hmac_vap->st_vap_base_info, us_user_index);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (IS_LEGACY_VAP(pst_mac_vap))
    {
        mac_vap_state_change_etc(pst_mac_vap, MAC_VAP_STATE_UP);
    }
#endif

    return OAL_SUCC;
}


oal_uint32  hmac_config_del_user_etc(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_del_user_param_stru    *pst_del_user;
    hmac_user_stru                 *pst_hmac_user;
    hmac_vap_stru                  *pst_hmac_vap;
    oal_uint16                      us_user_index;
    oal_uint32                      ul_ret = 0;

    pst_del_user = (mac_cfg_add_user_param_stru *)puc_param;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);

    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_del_user_etc::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡ�û���Ӧ������ */
    ul_ret = mac_vap_find_user_by_macaddr_etc(pst_mac_vap, pst_del_user->auc_mac_addr, &us_user_index);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_del_user_etc::mac_vap_find_user_by_macaddr_etc failed[%d].}", ul_ret);
        return ul_ret;
    }

    /* ��ȡhmac�û� */
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(us_user_index);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_del_user_etc::pst_hmac_user[%d] null.}", us_user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_user_del_etc(pst_mac_vap, pst_hmac_user);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_del_user_etc::hmac_user_del_etc failed[%d] device_id[%d].}", ul_ret, pst_mac_vap->uc_device_id);
        return ul_ret;
    }

#if 0 //hmac_user_del_etc�����¼�ȥִ��dmac_user_del���û��Ѿ�ɾ�������������¼�ȥdmac�����û�
    /* ��������DMAC��Ҫ�Ĳ��� */
    pst_del_user->us_user_idx = us_user_index;

    /***************************************************************************
        ���¼���DMAC��, ͬ��DMAC����
    ***************************************************************************/
    ul_ret = hmac_config_send_event_etc(&pst_hmac_vap->st_vap_base_info,
                                    WLAN_CFGID_DEL_USER,
                                    us_len,
                                    puc_param);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        /* �쳣�����ͷ��ڴ� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_del_user_etc::hmac_config_send_event_etc failed[%d].}", ul_ret);
        return ul_ret;
    }
#endif
    return OAL_SUCC;
}


oal_uint32  hmac_user_add_multi_user_etc(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_user_index)
{
    oal_uint32      ul_ret;
    oal_uint16      us_user_index;
    mac_user_stru  *pst_mac_user;
#ifdef _PRE_WLAN_FEATURE_WAPI
    hmac_user_stru *pst_hmac_user;
#endif

    ul_ret = hmac_user_alloc_etc(&us_user_index);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_multi_user_etc::hmac_user_alloc_etc failed[%d].}", ul_ret);
        return ul_ret;
    }

    /* ��ʼ���鲥�û�������Ϣ */
    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user_etc(us_user_index);
    if (OAL_PTR_NULL == pst_mac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_multi_user_etc::pst_mac_user[%d] null.}", us_user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_init_etc(pst_mac_user, us_user_index, OAL_PTR_NULL, pst_mac_vap->uc_chip_id,  pst_mac_vap->uc_device_id, pst_mac_vap->uc_vap_id);

    *pus_user_index = us_user_index;

#ifdef _PRE_WLAN_FEATURE_WAPI
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(us_user_index);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_user_add_multi_user_etc::hmac_user[%d] null.}", us_user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ʼ��wapi���� */
    hmac_wapi_init_etc(&pst_hmac_user->st_wapi, OAL_FALSE);
#endif

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_user_add_multi_user_etc, user index[%d].}", us_user_index);

    return OAL_SUCC;
}




oal_uint32  hmac_user_del_multi_user_etc(mac_vap_stru *pst_mac_vap)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    oal_uint32           ul_ret = 0;
#endif

#ifdef _PRE_WLAN_FEATURE_WAPI
    hmac_user_stru      *pst_hmac_user;
#endif

#ifdef _PRE_WLAN_FEATURE_WAPI
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(pst_mac_vap->us_multi_user_idx);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_user_del_multi_user_etc::get hmac_user[%d] null.}",
            pst_mac_vap->us_multi_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_wapi_deinit_etc(&pst_hmac_user->st_wapi);
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    /***************************************************************************
        ���¼���DMAC��, ͬ��DMAC����
    ***************************************************************************/
    ul_ret = hmac_config_send_event_etc(pst_mac_vap,
                                    WLAN_CFGID_DEL_MULTI_USER,
                                    OAL_SIZEOF(oal_uint16),
                                    (oal_uint8 *)&pst_mac_vap->us_multi_user_idx);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        /* �쳣�����ͷ��ڴ� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_del_multi_user_etc::hmac_config_send_event_etc failed[%d].}", ul_ret);
        return ul_ret;
    }
#endif

    hmac_user_free_etc(pst_mac_vap->us_multi_user_idx);

    return OAL_SUCC;
}


#ifdef _PRE_WLAN_FEATURE_WAPI
oal_uint8  hmac_user_is_wapi_connected_etc(oal_uint8 uc_device_id)
{
    oal_uint8               uc_vap_idx;
    hmac_user_stru         *pst_hmac_user_multi;
    mac_device_stru        *pst_mac_device;
    mac_vap_stru           *pst_mac_vap;

    pst_mac_device = mac_res_get_dev_etc(uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{hmac_user_is_wapi_connected_etc::pst_mac_device null.id %u}", uc_device_id);
        return OAL_FALSE;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
        {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "vap is null! vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (!IS_STA(pst_mac_vap))
        {
            continue;
        }

        pst_hmac_user_multi = (hmac_user_stru *)mac_res_get_hmac_user_etc(pst_mac_vap->us_multi_user_idx);
        if ((OAL_PTR_NULL != pst_hmac_user_multi)
            && (OAL_TRUE == WAPI_PORT_FLAG(&pst_hmac_user_multi->st_wapi)))
        {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}
#endif/* #ifdef _PRE_WLAN_FEATURE_WAPI */



oal_uint32  hmac_user_add_notify_alg_etc(mac_vap_stru *pst_mac_vap, oal_uint16 us_user_idx)
{
    frw_event_mem_stru             *pst_event_mem;
    frw_event_stru                 *pst_event;
    dmac_ctx_add_user_stru         *pst_add_user_payload;
    oal_uint32                      ul_ret;
    hmac_user_stru                 *pst_hmac_user;

    /* ���¼���Dmac����dmac����û��㷨���� */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_ctx_add_user_stru));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_user_add_notify_alg_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_add_user_payload = (dmac_ctx_add_user_stru *)pst_event->auc_event_data;
    //oal_memcmp(pst_add_user_payload->auc_bssid, pst_mac_vap->auc_bssid, WLAN_MAC_ADDR_LEN);
    pst_add_user_payload->us_user_idx = us_user_idx;
    pst_add_user_payload->us_sta_aid = pst_mac_vap->us_sta_aid;
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(us_user_idx);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hmac_user))
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{hmac_user_add_notify_alg_etc::null param,pst_hmac_user[%d].}",us_user_idx);
        FRW_EVENT_FREE(pst_event_mem);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_get_vht_hdl_etc(&pst_hmac_user->st_user_base_info, &pst_add_user_payload->st_vht_hdl);
    mac_user_get_ht_hdl_etc(&pst_hmac_user->st_user_base_info, &pst_add_user_payload->st_ht_hdl);
    /* ����¼�ͷ */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                        FRW_EVENT_TYPE_WLAN_CTX,
                        DMAC_WLAN_CTX_EVENT_SUB_TYPE_NOTIFY_ALG_ADD_USER,
                        OAL_SIZEOF(dmac_ctx_add_user_stru),
                        FRW_EVENT_PIPELINE_STAGE_1,
                        pst_mac_vap->uc_chip_id,
                        pst_mac_vap->uc_device_id,
                        pst_mac_vap->uc_vap_id);

    ul_ret = frw_event_dispatch_event_etc(pst_event_mem);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        /* �쳣�����ͷ��ڴ� */
        FRW_EVENT_FREE(pst_event_mem);

        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_user_add_notify_alg_etc::frw_event_dispatch_event_etc failed[%d].}", ul_ret);
        return ul_ret;
    }
    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}


hmac_user_stru  *mac_vap_get_hmac_user_by_addr_etc(mac_vap_stru *pst_mac_vap, oal_uint8  *puc_mac_addr)
{
    oal_uint32              ul_ret;
    oal_uint16              us_user_idx   = 0xffff;
    hmac_user_stru         *pst_hmac_user = OAL_PTR_NULL;

    /*����mac addr��sta����*/
    ul_ret = mac_vap_find_user_by_macaddr_etc(pst_mac_vap, puc_mac_addr, &us_user_idx);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{mac_vap_get_hmac_user_by_addr_etc::find_user_by_macaddr failed[%d].}", ul_ret);
        if (OAL_PTR_NULL != puc_mac_addr)
        {
            OAM_WARNING_LOG3(0, OAM_SF_ANY,"{mac_vap_get_hmac_user_by_addr_etc:: mac_addr[%02x XX XX XX %02x %02x]!.}",
                                puc_mac_addr[0], puc_mac_addr[4], puc_mac_addr[5]);
        }
        return OAL_PTR_NULL;
    }

    /*����sta�����ҵ�user�ڴ�����*/
    pst_hmac_user = mac_res_get_hmac_user_etc(us_user_idx);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_vap_get_hmac_user_by_addr_etc::user[%d] ptr null.}", us_user_idx);
    }
    return pst_hmac_user;
}

/*lint -e19*/
oal_module_symbol(hmac_user_alloc_etc);
oal_module_symbol(hmac_user_init_etc);
oal_module_symbol(hmac_config_kick_user_etc);
oal_module_symbol(mac_vap_get_hmac_user_by_addr_etc);
oal_module_symbol(mac_res_get_hmac_user_etc);
oal_module_symbol(hmac_user_free_asoc_req_ie);
oal_module_symbol(hmac_user_set_asoc_req_ie);
/*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

