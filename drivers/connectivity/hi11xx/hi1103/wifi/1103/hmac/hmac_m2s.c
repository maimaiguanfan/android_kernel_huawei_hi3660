

#ifndef __HMAC_M2S_C__
#define __HMAC_M2S_C__

#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif

#ifdef _PRE_WLAN_FEATURE_M2S

/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "hmac_ext_if.h"
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_m2s.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#include "hmac_fsm.h"
#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_main.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_M2S_C

/*****************************************************************************
  2 ��������
*****************************************************************************/

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/

oal_void hmac_m2s_vap_arp_probe_process(oal_void *p_arg, oal_bool_enum_uint8 en_arp_detect_on)
{
    mac_vap_stru                 *pst_mac_vap = (mac_vap_stru *)p_arg;
    hmac_vap_stru                *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_m2s_vap_arp_probe_start_etc::pst_hmac_vap null.}");
        return;
    }

    pst_hmac_vap->st_hmac_vap_m2s.en_arp_probe_on = en_arp_detect_on;

    OAM_WARNING_LOG1(0, OAM_SF_M2S, "{hmac_m2s_vap_arp_probe_process: en_arp_probe_on[%d].}", pst_hmac_vap->st_hmac_vap_m2s.en_arp_probe_on);
}


OAL_STATIC oal_uint32 hmac_m2s_arp_probe_timeout(oal_void *p_arg)
{
    hmac_vap_m2s_stru            *pst_hmac_vap_m2s;
    hmac_vap_stru                *pst_hmac_vap;
    hmac_user_stru               *pst_hmac_user;
    oal_uint32                    ui_val;
    oal_bool_enum_uint8           en_reassoc_codeid;

    pst_hmac_vap = (hmac_vap_stru *)p_arg;
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap_m2s = &(pst_hmac_vap->st_hmac_vap_m2s);

    ui_val = oal_atomic_read(&(pst_hmac_vap_m2s->ul_rx_unicast_pkt_to_lan));
    if(0 == ui_val)
    {
        pst_hmac_vap_m2s->uc_rx_no_pkt_count++;

        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S, "{hmac_m2s_arp_probe_timeout::rx_arp_pkt fail cnt[%d]!}",
            pst_hmac_vap_m2s->uc_rx_no_pkt_count);

        if(pst_hmac_vap_m2s->uc_rx_no_pkt_count > M2S_ARP_FAIL_REASSOC_NUM)
        {
            pst_hmac_user = mac_res_get_hmac_user_etc(pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
            if (OAL_PTR_NULL == pst_hmac_user)
            {
                OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S, "hmac_m2s_arp_probe_timeout: pst_hmac_user is null ptr. user id:%d", pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
                return OAL_ERR_CODE_PTR_NULL;
            }

            /* �ع����߼���ʱ�رգ���ͳ�Ƴ�����Щ�������ֲ�ͨ�������Ƴ����ſ� */
            OAM_WARNING_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S, "{hmac_m2s_arp_probe_timeout::user rssi[%d] threhold[%d] need to reassoc to resume.}",
                pst_hmac_user->c_rssi, WLAN_FAR_DISTANCE_RSSI);

            /* ֹͣarp̽�� */
            pst_hmac_vap_m2s->en_arp_probe_on = OAL_FALSE;

            pst_hmac_vap_m2s->uc_rx_no_pkt_count = 0;

            /* ��Զ���Ŵ����ع����߼� */
            if(pst_hmac_user->c_rssi >= WLAN_FAR_DISTANCE_RSSI)
            {
                /* ����reassoc req */
                hmac_roam_start_etc(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE, NULL, ROAM_TRIGGER_M2S);


                en_reassoc_codeid = OAL_TRUE;
            }
            else
            {
                en_reassoc_codeid = OAL_FALSE;
            }
#ifdef _PRE_WLAN_1103_CHR
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_MIMO_TO_SISO_FAIL, en_reassoc_codeid);
#endif
        }
    }
    else
    {
        /* ֹͣarp̽�� */
        pst_hmac_vap_m2s->en_arp_probe_on = OAL_FALSE;

        pst_hmac_vap_m2s->uc_rx_no_pkt_count = 0;

        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{hmac_m2s_arp_probe_timeout: detect succ, en_arp_probe_on[%d].}", pst_hmac_vap_m2s->en_arp_probe_on);
    }

    oal_atomic_set(&pst_hmac_vap_m2s->ul_rx_unicast_pkt_to_lan, 0);

    return OAL_SUCC;
}


oal_void hmac_m2s_arp_fail_process(oal_netbuf_stru *pst_netbuf, oal_void *p_arg)
{
    hmac_vap_stru                    *pst_hmac_vap;
    hmac_vap_m2s_stru                *pst_hmac_vap_m2s;
    mac_ether_header_stru            *pst_mac_ether_hdr;
    oal_uint8                         uc_data_type;

    pst_hmac_vap = (hmac_vap_stru *)p_arg;

    pst_hmac_vap_m2s = &(pst_hmac_vap->st_hmac_vap_m2s);

    /* ֻҪͳ�ƹ��ܴ򿪣�����Ҫ��һ��̽�� */
    if(OAL_TRUE == pst_hmac_vap_m2s->en_arp_probe_on)
    {
        pst_mac_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);

        /* ���������Ѿ�����飬����û��Ҫ��������� */
        uc_data_type =  mac_get_data_type_from_8023_etc((oal_uint8 *)pst_mac_ether_hdr, MAC_NETBUFF_PAYLOAD_ETH);

        /* ���ͷ��򴴽���ʱ������δ�����ʱ�� */
        if((MAC_DATA_ARP_REQ == uc_data_type) && (OAL_FALSE == pst_hmac_vap_m2s->st_arp_probe_timer.en_is_registerd))
        {
            /* ÿ��������ʱ��֮ǰ����,��֤ͳ�Ƶ�ʱ�� */
            oal_atomic_set(&(pst_hmac_vap_m2s->ul_rx_unicast_pkt_to_lan), 0);

            FRW_TIMER_CREATE_TIMER(&(pst_hmac_vap_m2s->st_arp_probe_timer),
                       hmac_m2s_arp_probe_timeout,
                       M2S_ARP_PROBE_TIMEOUT,
                       pst_hmac_vap,
                       OAL_FALSE,
                       OAM_MODULE_ID_HMAC,
                       pst_hmac_vap->st_vap_base_info.ul_core_id);
        }
    }
}

#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of __HMAC_M2S_C__ */

