


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "oal_net.h"
#include "oal_cfg80211.h"
#include "wlan_spec.h"
#include "wal_linux_event.h"
#include "wal_linux_scan.h"
#include "wal_linux_cfg80211.h"
#include "wal_main.h"
#include "wal_linux_rx_rsp.h"
#include "hmac_vap.h"
#include "hmac_device.h"
#include "mac_device.h"
#include "hmac_resource.h"

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <net/cfg80211.h>
#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)

#endif

#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION) && (_PRE_TEST_MODE == _PRE_TEST_MODE_UT)
    /* UTģʽ�µ���frw_event_process_all_event */
    extern oal_void  frw_event_process_all_event_etc(oal_uint ui_data);
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_SCAN_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


OAL_STATIC oal_void  wal_inform_bss_frame(wal_scanned_bss_info_stru *pst_scanned_bss_info, oal_void *p_data)
{
    oal_cfg80211_bss_stru        *pst_cfg80211_bss;
    oal_wiphy_stru               *pst_wiphy;
    oal_ieee80211_channel_stru   *pst_ieee80211_channel;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
    struct timespec ts;
#else
    struct timeval tv;
#endif
#endif

    if ((OAL_PTR_NULL == pst_scanned_bss_info) || (OAL_PTR_NULL == p_data))
    {
        OAM_ERROR_LOG2(0, OAM_SF_SCAN, "{wal_inform_bss_frame::input param pointer is null, pst_scanned_bss_info[%p], p_data[%p]!}",
                       pst_scanned_bss_info, p_data);
        return;
    }

    pst_wiphy = (oal_wiphy_stru *)p_data;

    pst_ieee80211_channel = oal_ieee80211_get_channel(pst_wiphy, (oal_int32)pst_scanned_bss_info->s_freq);

    if(NULL == pst_ieee80211_channel)
    {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{wal_inform_bss_frame::get channel failed, wrong s_freq[%d]}",
                       (oal_int32)pst_scanned_bss_info->s_freq);
        return;
    }

    pst_scanned_bss_info->l_signal = pst_scanned_bss_info->l_signal * 100;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/* ������������ɨ����������cts��֤2��ɨ���bss��timestampʱ��һ��(��һ��û��ɨ�赽) */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
    get_monotonic_boottime(&ts);
    pst_scanned_bss_info->pst_mgmt->u.probe_resp.timestamp = ((u64)(ts.tv_sec)*1000000)//lint !e571
        + ts.tv_nsec / 1000;
#else
    do_gettimeofday(&tv);
    pst_scanned_bss_info->pst_mgmt->u.probe_resp.timestamp = ((u64)tv.tv_sec*1000000)
        + tv.tv_usec;
#endif
#endif

    /* ����ϱ��ں�bss ��Ϣ */
    pst_cfg80211_bss = oal_cfg80211_inform_bss_frame_etc(pst_wiphy, pst_ieee80211_channel, pst_scanned_bss_info->pst_mgmt, pst_scanned_bss_info->ul_mgmt_len, pst_scanned_bss_info->l_signal, GFP_ATOMIC);
    if (pst_cfg80211_bss != NULL)
    {
        oal_cfg80211_put_bss_etc(pst_wiphy, pst_cfg80211_bss);
    }

    return;
}


oal_void wal_update_bss_etc(oal_wiphy_stru      *pst_wiphy,
                        hmac_bss_mgmt_stru  *pst_bss_mgmt,
                        oal_uint8           *puc_bssid)
{
    wal_scanned_bss_info_stru     st_scanned_bss_info;
    oal_cfg80211_bss_stru        *pst_cfg80211_bss;
    hmac_scanned_bss_info        *pst_scanned_bss;
    mac_ieee80211_frame_stru     *pst_frame_hdr;
    oal_dlist_head_stru          *pst_entry;
    mac_bss_dscr_stru            *pst_bss_dscr;
    oal_uint8                     uc_chan_number;
    enum ieee80211_band           en_band;
    oal_int32                     l_channel;
    oal_bool_enum_uint8           en_inform_bss = OAL_FALSE;

    if (pst_wiphy == OAL_PTR_NULL || pst_bss_mgmt == OAL_PTR_NULL || puc_bssid == OAL_PTR_NULL)
    {
        OAM_WARNING_LOG3(0, OAM_SF_ASSOC, "{wal_update_bss_etc::null pointer.wiphy %p, bss_mgmt %p, bssid %p.",
                        pst_wiphy, pst_bss_mgmt, puc_bssid);
        return;
    }

    pst_bss_dscr = OAL_PTR_NULL;
    /* ��ȡ�� */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    /* �ӱ���ɨ�����л�ȡbssid ��Ӧ��bss ��Ϣ */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_bss_mgmt->st_bss_list_head))
    {
        pst_scanned_bss = OAL_DLIST_GET_ENTRY(pst_entry, hmac_scanned_bss_info, st_dlist_head);
        pst_bss_dscr    = &(pst_scanned_bss->st_bss_dscr_info);

        /* ��ɨ�����жԱȹ�����MAC ��ַ�������ͬ������ǰ�˳� */
        if (oal_memcmp(puc_bssid, pst_bss_dscr->auc_bssid, WLAN_MAC_ADDR_LEN) == 0)
        {
            break;
        }
        pst_bss_dscr = OAL_PTR_NULL;
    }

    /* ����� */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    if (OAL_PTR_NULL == pst_bss_dscr)
    {
        OAM_WARNING_LOG4(0, OAM_SF_ASSOC, "{wal_update_bss_etc::do not find correspond bss from scan result."
                        "%02X:XX:XX:%02X:%02X:%02X}",
                        puc_bssid[0], puc_bssid[3], puc_bssid[4], puc_bssid[5]);
        return;
    }

    /* �����Ƿ����ں��У����û�и�bss  �������bss ���������bss ʱ�� */
    pst_cfg80211_bss = oal_cfg80211_get_bss_etc(pst_wiphy,
                        OAL_PTR_NULL,
                        puc_bssid,
                        (oal_uint8 *)(pst_bss_dscr->ac_ssid),
                        OAL_STRLEN(pst_bss_dscr->ac_ssid));

    if (pst_cfg80211_bss != OAL_PTR_NULL)
    {
        l_channel = (oal_int32)oal_ieee80211_frequency_to_channel((oal_int32)(pst_cfg80211_bss->channel->center_freq));

        /* ����ϵ�ssid���ŵ���һ�£���Ҫunlink֮�������ϱ��µ�FRW���ں˻�ȡ��old�ŵ�����ɺ����ٴι�����Ϊ�ŵ�������޷������ɹ� */
        if(pst_bss_dscr->st_channel.uc_chan_number != (oal_uint8)l_channel)
        {
            OAM_WARNING_LOG2(0, OAM_SF_ASSOC, "{wal_update_bss_etc::current kernel bss channel[%d] need to update to channel[%d].",
                   (oal_uint8)l_channel, pst_bss_dscr->st_channel.uc_chan_number);

            oal_cfg80211_unlink_bss_etc(pst_wiphy, pst_cfg80211_bss);
            en_inform_bss = OAL_TRUE;
        }
        else
        {
            oal_cfg80211_put_bss_etc(pst_wiphy, pst_cfg80211_bss);
        }
    }
    else
    {
        en_inform_bss = OAL_TRUE;
    }

    /* ��Ҫinform bss���ں� */
    if(en_inform_bss)
    {
        uc_chan_number = pst_bss_dscr->st_channel.uc_chan_number;
        en_band        = (enum ieee80211_band)pst_bss_dscr->st_channel.en_band;

        /* ��ʼ�� */
        oal_memset(&st_scanned_bss_info, 0, OAL_SIZEOF(wal_scanned_bss_info_stru));

        /* ��дBSS �ź�ǿ�� */
        st_scanned_bss_info.l_signal    = pst_bss_dscr->c_rssi;

        /* ��bss�����ŵ�������Ƶ�� */
        st_scanned_bss_info.s_freq      = (oal_int16)oal_ieee80211_channel_to_frequency(uc_chan_number, en_band);

        /* �����ָ֡��ͳ��� */
        st_scanned_bss_info.pst_mgmt    = (oal_ieee80211_mgmt_stru *)(pst_bss_dscr->auc_mgmt_buff);
        st_scanned_bss_info.ul_mgmt_len = pst_bss_dscr->ul_mgmt_len;

        /* ��ȡ�ϱ���ɨ�����Ĺ���֡��֡ͷ */
        pst_frame_hdr  = (mac_ieee80211_frame_stru *)pst_bss_dscr->auc_mgmt_buff;

        /* ���ɨ��������յ���֡������beacon���ͣ�ͳһ�޸�Ϊprobe rsp�����ϱ���
           Ϊ�˽���ϱ��ں˵�ɨ����beacon֡�������е����⣬�����⣬��01���ֹ� */
        if (WLAN_BEACON == pst_frame_hdr->st_frame_control.bit_sub_type)
        {
            /* �޸�beacon֡����Ϊprobe rsp */
            pst_frame_hdr->st_frame_control.bit_sub_type = WLAN_PROBE_RSP;
        }

        wal_inform_bss_frame(&st_scanned_bss_info, pst_wiphy);
    }

    return;
}



oal_void  wal_inform_all_bss_etc(oal_wiphy_stru  *pst_wiphy, hmac_bss_mgmt_stru  *pst_bss_mgmt, oal_uint8   uc_vap_id)
{
    mac_bss_dscr_stru              *pst_bss_dscr;
    hmac_scanned_bss_info          *pst_scanned_bss;
    oal_dlist_head_stru            *pst_entry;
    mac_ieee80211_frame_stru       *pst_frame_hdr;
    wal_scanned_bss_info_stru       st_scanned_bss_info;
    oal_uint32                      ul_ret;
    oal_uint32                      ul_bss_num_not_in_regdomain = 0;
    oal_uint32                      ul_bss_num = 0;
    oal_uint8                       uc_chan_number;
    enum ieee80211_band             en_band;

#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11R) || defined(_PRE_WLAN_FEATURE_11K_EXTERN)
    hmac_vap_stru                  *pst_hmac_vap;
    oal_uint8                       uc_voe_11r_auth;
    /* ��ȡhmac vap */
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_SCAN,
                         "{wal_inform_all_bss_etc::hmac_vap is null, vap_id[%d]!}");
        return;
    }

    uc_voe_11r_auth = pst_hmac_vap->bit_voe_11r_auth;
    if(1 == uc_voe_11r_auth)
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_SCAN,
                             "{wal_inform_all_bss_etc::uc_11r_auth=[%d]!}",uc_voe_11r_auth);
    }
#endif
    /* ��ȡ�� */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    /* ����ɨ�赽��bss��Ϣ */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_bss_mgmt->st_bss_list_head))
    {
        pst_scanned_bss = OAL_DLIST_GET_ENTRY(pst_entry, hmac_scanned_bss_info, st_dlist_head);
        if (OAL_PTR_NULL == pst_scanned_bss)
        {
            OAM_ERROR_LOG0(uc_vap_id, OAM_SF_SCAN,
                "{wal_inform_all_bss_etc::pst_scanned_bss is null. }");
            continue;
        }
        pst_bss_dscr    = &(pst_scanned_bss->st_bss_dscr_info);

        uc_chan_number = pst_bss_dscr->st_channel.uc_chan_number;
        en_band        = (enum ieee80211_band)pst_bss_dscr->st_channel.en_band;

        /* �ж��ŵ��ǲ����ڹ������ڣ�������ڣ����ϱ��ں� */
        ul_ret = mac_is_channel_num_valid_etc(en_band, uc_chan_number);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG2(uc_vap_id, OAM_SF_SCAN,
                             "{wal_inform_all_bss_etc::curr channel[%d] and band[%d] is not in regdomain.}",
                             uc_chan_number, en_band);
            ul_bss_num_not_in_regdomain++;
            continue;
        }
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11R) || defined(_PRE_WLAN_FEATURE_11K_EXTERN)
        if(OAL_FALSE == uc_voe_11r_auth)
#endif
        {/*voe 11r ��֤ʱ������ʱ������*/
            /* �ϱ�WAL_SCAN_REPORT_LIMIT���ڵ�ɨ���� */
            if (oal_time_after32(OAL_TIME_GET_STAMP_MS(), (pst_bss_dscr->ul_timestamp + WAL_SCAN_REPORT_LIMIT)))
            {
                continue;
            }
        }

        /* ��ʼ�� */
        oal_memset(&st_scanned_bss_info, 0, OAL_SIZEOF(wal_scanned_bss_info_stru));

        st_scanned_bss_info.l_signal    = pst_bss_dscr->c_rssi;

        /*��bss�����ŵ�������Ƶ��*/
        st_scanned_bss_info.s_freq      = (oal_int16)oal_ieee80211_channel_to_frequency(uc_chan_number, en_band);

        /* �����ָ֡��ͳ��� */
        st_scanned_bss_info.pst_mgmt    = (oal_ieee80211_mgmt_stru *)(pst_bss_dscr->auc_mgmt_buff);
        st_scanned_bss_info.ul_mgmt_len = pst_bss_dscr->ul_mgmt_len;

        /* ��ȡ�ϱ���ɨ�����Ĺ���֡��֡ͷ */
        pst_frame_hdr  = (mac_ieee80211_frame_stru *)pst_bss_dscr->auc_mgmt_buff;

        /* ���ɨ��������յ���֡������beacon���ͣ�ͳһ�޸�Ϊprobe rsp�����ϱ���
           Ϊ�˽���ϱ��ں˵�ɨ����beacon֡�������е����⣬�����⣬��01���ֹ� */
    	if (WLAN_BEACON == pst_frame_hdr->st_frame_control.bit_sub_type)
    	{
    	    /* �޸�beacon֡����Ϊprobe rsp */
    		pst_frame_hdr->st_frame_control.bit_sub_type = WLAN_PROBE_RSP;
    	}

        /* �ϱ�ɨ�������ں� */
        wal_inform_bss_frame(&st_scanned_bss_info, pst_wiphy);
        ul_bss_num++;
    }

    /* ����� */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    OAM_WARNING_LOG3(uc_vap_id, OAM_SF_SCAN,
                     "{wal_inform_all_bss_etc::there are %d bss not in regdomain, so inform kernel bss num is [%d] in [%d]!}",
                     ul_bss_num_not_in_regdomain, ul_bss_num, (pst_bss_mgmt->ul_bss_num - ul_bss_num_not_in_regdomain));

    return;
}


OAL_STATIC oal_void free_scan_param_resource(mac_cfg80211_scan_param_stru *pst_scan_param)
{
    if (OAL_PTR_NULL != pst_scan_param->pul_channels_2G)
    {
        oal_free(pst_scan_param->pul_channels_2G);
        pst_scan_param->pul_channels_2G = OAL_PTR_NULL;
    }
    if (OAL_PTR_NULL != pst_scan_param->pul_channels_5G)
    {
        oal_free(pst_scan_param->pul_channels_5G);
        pst_scan_param->pul_channels_5G = OAL_PTR_NULL;
    }
    if (OAL_PTR_NULL != pst_scan_param->puc_ie)
    {
        oal_free(pst_scan_param->puc_ie);
        pst_scan_param->puc_ie = OAL_PTR_NULL;
    }
}


OAL_STATIC oal_uint32  wal_set_scan_channel(
                    oal_cfg80211_scan_request_stru    *pst_request,
                    mac_cfg80211_scan_param_stru      *pst_scan_param)

{
    oal_uint32  ul_loop;
    oal_uint32  ul_num_chan_2G;
    oal_uint32  ul_num_chan_5G;

    if (OAL_PTR_NULL == pst_request || OAL_PTR_NULL == pst_scan_param)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_get_scan_channel_num::set scan channel from scan request failed, pst_request[%p] null ptr or pst_scan_param[%p] null ptr.}",
            pst_request, pst_scan_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (0 == pst_request->n_channels)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_get_scan_channel_num::channels in scan requst is zero.}");
        return OAL_FAIL;
    }

    pst_scan_param->pul_channels_2G = oal_memalloc(pst_request->n_channels * OAL_SIZEOF(oal_uint32));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_scan_param->pul_channels_2G))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_scan_work_func_etc::2.4G channel alloc mem return null ptr!}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    pst_scan_param->pul_channels_5G = oal_memalloc(pst_request->n_channels * OAL_SIZEOF(oal_uint32));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_scan_param->pul_channels_5G))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_scan_work_func_etc::5G channel alloc mem return null ptr!}");
        free_scan_param_resource(pst_scan_param);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    ul_num_chan_2G = 0;
    ul_num_chan_5G = 0;

    for (ul_loop = 0; ul_loop < pst_request->n_channels; ul_loop++)
    {
        oal_uint16  us_center_freq;
        oal_uint32  ul_chan;

        us_center_freq = pst_request->channels[ul_loop]->center_freq;

        /* ��������Ƶ�ʣ������ŵ��� */
        ul_chan = (oal_uint32)oal_ieee80211_frequency_to_channel((oal_int32)us_center_freq);

        if (us_center_freq <= WAL_MAX_FREQ_2G)
        {
            pst_scan_param->pul_channels_2G[ul_num_chan_2G++] = ul_chan;
        }
        else
        {
            pst_scan_param->pul_channels_5G[ul_num_chan_5G++] = ul_chan;
        }
    }

    pst_scan_param->uc_num_channels_2G = (oal_uint8)ul_num_chan_2G;
    pst_scan_param->uc_num_channels_5G = (oal_uint8)ul_num_chan_5G;

    if (0 == ul_num_chan_2G)
    {
        oal_free(pst_scan_param->pul_channels_2G);
        pst_scan_param->pul_channels_2G = OAL_PTR_NULL;
    }
    if (0 == ul_num_chan_5G)
    {
        oal_free(pst_scan_param->pul_channels_5G);
        pst_scan_param->pul_channels_5G = OAL_PTR_NULL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_void wal_set_scan_ssid(oal_cfg80211_scan_request_stru *pst_request, mac_cfg80211_scan_param_stru *pst_scan_param)
{
    oal_int32   l_loop;
    oal_int32   l_ssid_num;

    if ((OAL_PTR_NULL == pst_request) || (OAL_PTR_NULL == pst_scan_param))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_set_scan_ssid::scan failed, null ptr, pst_request[%p], pst_scan_param[%p].}",
          pst_request, pst_scan_param);

        return;
    }

    pst_scan_param->en_scan_type = OAL_ACTIVE_SCAN; /* active scan */
    pst_scan_param->l_ssid_num   = 0;

    /* ȡ�ں��·���ssid�ĸ��� */
    l_ssid_num = pst_request->n_ssids;
    if (l_ssid_num > WLAN_SCAN_REQ_MAX_SSID)
    {
        /* ����û��·���ָ��ssid�ĸ�����������֧�ֵ�����������ȡ����֧�ֵ�ָ��ssid�������� */
        l_ssid_num = WLAN_SCAN_REQ_MAX_SSID;
    }

    /* ���û��·���ssid��Ϣ��������Ӧ�Ľṹ���� */
    if ((l_ssid_num > 0) && (OAL_PTR_NULL != pst_request->ssids))
    {
        pst_scan_param->l_ssid_num = l_ssid_num;

        for (l_loop = 0; l_loop < l_ssid_num; l_loop++)
        {
            pst_scan_param->st_ssids[l_loop].uc_ssid_len = pst_request->ssids[l_loop].ssid_len;
            if (pst_scan_param->st_ssids[l_loop].uc_ssid_len > OAL_IEEE80211_MAX_SSID_LEN)
            {
                OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_set_scan_ssid::ssid scan set failed, ssid_len[%d] is exceed, max[%d].}",
                  pst_scan_param->st_ssids[l_loop].uc_ssid_len, OAL_IEEE80211_MAX_SSID_LEN);

                pst_scan_param->st_ssids[l_loop].uc_ssid_len = OAL_IEEE80211_MAX_SSID_LEN;
            }

            oal_memcopy(pst_scan_param->st_ssids[l_loop].auc_ssid, pst_request->ssids[l_loop].ssid,
              pst_scan_param->st_ssids[l_loop].uc_ssid_len);
        }
	}
}


OAL_STATIC oal_uint32  wal_wait_for_scan_timeout_fn(void *p_arg)
{
    hmac_vap_stru                  *pst_hmac_vap = (hmac_vap_stru *)p_arg;
    mac_vap_stru                   *pst_mac_vap  = &(pst_hmac_vap->st_vap_base_info);
    hmac_device_stru               *pst_hmac_device;
    hmac_bss_mgmt_stru             *pst_bss_mgmt;
    hmac_scan_stru                 *pst_scan_mgmt;
    oal_wiphy_stru                 *pst_wiphy;

    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{wal_wait_for_scan_timeout_fn:: 5 seconds scan timeout proc.}");

    /* ���ݵ�ǰɨ������ͺ͵�ǰvap��״̬�������л�vap��״̬��ɨ���쳣�����У��ϱ��ں�ɨ��״̬Ϊɨ����� */
    if ((WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode) &&
        (MAC_VAP_STATE_STA_WAIT_SCAN == pst_mac_vap->en_vap_state))
    {
        /* �ı�vap״̬��SCAN_COMP */
        mac_vap_state_change_etc(pst_mac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
    }

    /* ��ȡhmac device */
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_hmac_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{wal_wait_for_scan_complete_time_out::pst_hmac_device[%d] is null.}",
                        pst_mac_vap->uc_device_id);
        return OAL_FAIL;
    }

    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);
    pst_wiphy      = pst_hmac_device->pst_device_base_info->pst_wiphy;

    /* ��ȡɨ�����Ĺ���ṹ��ַ */
    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* �����ں��·���ɨ��request��Դ���� */
    oal_spin_lock(&(pst_scan_mgmt->st_scan_request_spinlock));

    if (OAL_PTR_NULL != pst_scan_mgmt->pst_request)
    {
        /* �ϱ�ɨ�赽�����е�bss */
        wal_inform_all_bss_etc(pst_wiphy, pst_bss_mgmt, pst_mac_vap->uc_vap_id);

        /* ֪ͨ kernel scan �Ѿ����� */
        oal_cfg80211_scan_done_etc(pst_scan_mgmt->pst_request, 0);

        pst_scan_mgmt->pst_request = OAL_PTR_NULL;
        pst_scan_mgmt->en_complete = OAL_TRUE;

        /* �ñ������Ż�ʱ��֤OAL_WAIT_QUEUE_WAKE_UP�����ִ�� */
        OAL_SMP_MB();
        OAL_WAIT_QUEUE_WAKE_UP_INTERRUPT(&pst_scan_mgmt->st_wait_queue);
    }

    /* ֪ͨ���ںˣ��ͷ���Դ����� */
    oal_spin_unlock(&(pst_scan_mgmt->st_scan_request_spinlock));

    return OAL_SUCC;
}


OAL_STATIC oal_void  wal_start_timer_for_scan_timeout(oal_uint8   uc_vap_id)
{
    hmac_vap_stru               *pst_hmac_vap = OAL_PTR_NULL;
    mac_device_stru             *pst_mac_device = OAL_PTR_NULL;
    oal_uint32                   ul_timeout;

    /* ��ʱʱ��Ϊ5�� */
    ul_timeout = WAL_MAX_SCAN_TIME_PER_SCAN_REQ;

    /* ��ȡhmac vap */
    pst_hmac_vap = mac_res_get_hmac_vap(uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_SCAN, "{wal_start_timer_for_scan_timeout::pst_hmac_vap is null!}");
        return;
    }

    /* ��ȡmac device */
    pst_mac_device = mac_res_get_dev_etc(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_SCAN, "{wal_start_timer_for_scan_timeout::pst_mac_device is null!}");
        return;
    }

    /* ����ɨ�豣����ʱ������ָ��ʱ��û���ϱ�ɨ�����������ϱ�ɨ����� */
    FRW_TIMER_CREATE_TIMER(&(pst_hmac_vap->st_scan_timeout),
                           wal_wait_for_scan_timeout_fn,
                           ul_timeout,
                           pst_hmac_vap,
                           OAL_FALSE,
                           OAM_MODULE_ID_WAL,
                           pst_mac_device->ul_core_id);

    return;
}


oal_uint32 wal_scan_work_func_etc(hmac_scan_stru                     *pst_scan_mgmt,
                                  oal_net_device_stru                   *pst_netdev,
                                  oal_cfg80211_scan_request_stru        *pst_request)
{
    mac_cfg80211_scan_param_stru st_scan_param;
    oal_uint32                   ul_ret;
    mac_vap_stru                *pst_mac_vap = OAL_NET_DEV_PRIV(pst_netdev);
    hmac_vap_stru               *pst_hmac_vap;
    oal_uint8                    uc_vap_id;
    oal_uint8                   *puc_ie;
#ifdef _PRE_WLAN_FEATURE_AP_PM
    oal_uint8                    uc_para = 0;
#endif

    if(NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_scan_work_func_etc::pst_mac_vap is null!}");
        return OAL_FAIL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;

    oal_memset(&st_scan_param, 0, sizeof(mac_cfg80211_scan_param_stru));

    /* �����ں��·���ɨ���ŵ��б� */
    ul_ret = wal_set_scan_channel(pst_request, &st_scan_param);
    if (ul_ret != OAL_SUCC)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_scan_work_func_etc::wal_set_scan_channel proc failed, err_code[%d]!}", ul_ret);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_AP_PM
    ul_ret = wal_config_sta_scan_connect_event(pst_netdev, &uc_para);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_scan_work_func_etc:: exit wow fail!}\r\n");
    }
#endif

    /* �����ں��·���ssid */
    wal_set_scan_ssid(pst_request, &st_scan_param);

    /* �����ں��·���ie */
    st_scan_param.ul_ie_len = pst_request->ie_len;
    if(st_scan_param.ul_ie_len > 0)
    {
        puc_ie = (oal_uint8 *)oal_memalloc(pst_request->ie_len);
        if (OAL_PTR_NULL == puc_ie)
        {
            OAM_ERROR_LOG2(0, OAM_SF_CFG, "{wal_scan_work_func_etc::puc_ie(%d)(%p) alloc mem return null ptr!}",
            pst_request->ie_len,pst_request->ie);
            free_scan_param_resource(&st_scan_param);
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
        oal_memcopy(puc_ie, pst_request->ie, st_scan_param.ul_ie_len);
        st_scan_param.puc_ie = puc_ie;
    }

    /*�û����·���scan flag ת������һ�׶ν����������ݲ�����dmac,��Android P �汾�ϲ�������ɺ��ٵ���ʵ��Ӧ�� */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
    if((pst_request->flags >> WLAN_NL80211_SCAN_FLAG_BIT_LOW_SPAN) & 0x1)
    {
        st_scan_param.uc_scan_flag |= BIT(WLAN_SCAN_FLAG_LOW_SPAN);/*ʹ�ò���ɨ��*/
        OAM_WARNING_LOG2(0, OAM_SF_SCAN, "{wal_scan_work_func_etc::kernel_scan_req_flags=[%d],uc_scan_flag =[%d]}\r\n",
        pst_request->flags, st_scan_param.uc_scan_flag);
    }
#endif

    /* P2P WLAN/P2P ��������£�����ɨ���ssid �ж��Ƿ�Ϊp2p device �����ɨ�裬
        ssid Ϊ"DIRECT-"����Ϊ��p2p device �����ɨ�� */
    /* �����·�ɨ���device �Ƿ�Ϊp2p device(p2p0) */
    st_scan_param.bit_is_p2p0_scan      = OAL_FALSE;


    if (IS_P2P_SCAN_REQ(pst_request))
    {
        st_scan_param.bit_is_p2p0_scan = OAL_TRUE;
    }

    /* ���¼�ǰ��ֹ�첽�������ɨ���,����ͬ������ */
    pst_scan_mgmt->en_complete = OAL_FALSE;

    /* ������ʱ��������ɨ�賬ʱ���� */
    /* ����������post event�����п��ܴ������ȣ�����ɨ������¼����ڶ�ʱ�����������յ��³�ʱ��ʱ������ */
    wal_start_timer_for_scan_timeout(uc_vap_id);

    /* ���¼���֪ͨ��������ɨ�� */
    ul_ret = wal_cfg80211_start_scan_etc(pst_netdev, &st_scan_param);
    if( OAL_SUCC != ul_ret)
    {
        pst_hmac_vap = mac_res_get_hmac_vap(uc_vap_id);
        if (pst_hmac_vap && pst_hmac_vap->st_scan_timeout.en_is_registerd)
        {
            FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_hmac_vap->st_scan_timeout);
        }

        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_scan_work_func_etc::wal_cfg80211_start_scan_etc proc failed, err_code[%d]!}", ul_ret);
        /* ��������£���hmac�ͷ� */
        free_scan_param_resource(&st_scan_param);

        pst_scan_mgmt->en_complete = OAL_TRUE;
        return OAL_FAIL;
    }

    /* win32 UTģʽ������һ���¼����� */
#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION) && (_PRE_TEST_MODE == _PRE_TEST_MODE_UT)
    frw_event_process_all_event_etc(0);
#endif

    return OAL_SUCC;
}


oal_int32 wal_send_scan_abort_msg_etc(oal_net_device_stru   *pst_net_dev)
{
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_pedding_data = 0;       /* ������ݣ���ʹ�ã�ֻ��Ϊ�˸��ýӿ� */
    oal_int32                       l_ret = 0;
    wal_msg_stru                    *pst_rsp_msg = OAL_PTR_NULL;

    /* ���¼�֪ͨdevice����ֹɨ�� */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SCAN_ABORT, OAL_SIZEOF(ul_pedding_data));

    oal_memcopy(st_write_msg.auc_value, (oal_int8 *)&ul_pedding_data, OAL_SIZEOF(ul_pedding_data));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(ul_pedding_data),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{wal_send_scan_abort_msg_etc::fail to stop scan, error[%d]}", l_ret);
        return l_ret;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp_etc(pst_rsp_msg))
    {
        OAM_WARNING_LOG0(0,OAM_SF_SCAN,"{wal_send_scan_abort_msg_etc::wal_check_and_release_msg_resp_etc fail.}");
    }

    return OAL_SUCC;
}


oal_int32 wal_force_scan_complete_etc(oal_net_device_stru   *pst_net_dev,
                                         oal_bool_enum          en_is_aborted)
{
    mac_vap_stru            *pst_mac_vap;
    hmac_vap_stru           *pst_hmac_vap;
    hmac_device_stru        *pst_hmac_device;
    hmac_scan_stru          *pst_scan_mgmt;

    pst_mac_vap  = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{wal_force_scan_complete_etc:: mac_vap of net_dev is deleted!iftype:[%d]}",pst_net_dev->ieee80211_ptr->iftype);
        return OAL_SUCC;
    }

    /* ��ȡhmac device */
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_hmac_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{wal_force_scan_complete_etc::pst_hmac_device[%d] is null!}",
                         pst_mac_vap->uc_device_id);
        return -OAL_EINVAL;
    }

    /* ��ȡhmac vap */
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{wal_force_scan_complete_etc::hmac_vap is null, vap_id[%d]!}", pst_mac_vap->uc_vap_id);
        return -OAL_EINVAL;
    }

    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);

    /* ����������ڲ���ɨ�� */
    if (OAL_PTR_NULL == pst_scan_mgmt->pst_request)
    {
        /* �ж��Ƿ�����ڲ�ɨ�裬������ڣ�Ҳ��Ҫֹͣ */
        if ((OAL_TRUE == pst_scan_mgmt->en_is_scanning) &&
            (pst_mac_vap->uc_vap_id == pst_scan_mgmt->st_scan_record_mgmt.uc_vap_id))
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                             "{wal_force_scan_complete_etc::may be internal scan, stop scan!}");
            /* ��ֹɨ�� */
            wal_send_scan_abort_msg_etc(pst_net_dev);
        }

        return OAL_SUCC;
    }

    /* �����ں��·���ɨ��request��Դ���� */
    oal_spin_lock(&(pst_scan_mgmt->st_scan_request_spinlock));

    /* ������ϲ��·���ɨ��������֪ͨ�ں�ɨ��������ڲ�ɨ�費��֪ͨ */
    if ((OAL_PTR_NULL != pst_scan_mgmt->pst_request) && OAL_WDEV_MATCH(pst_net_dev, pst_scan_mgmt->pst_request))
    {
        /* ɾ���ȴ�ɨ�賬ʱ��ʱ�� */
        if (OAL_TRUE == pst_hmac_vap->st_scan_timeout.en_is_registerd)
        {
            FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hmac_vap->st_scan_timeout));
        }

        /* �ϱ��ں�ɨ���� */
        wal_inform_all_bss_etc(pst_hmac_device->pst_device_base_info->pst_wiphy,
                           &(pst_scan_mgmt->st_scan_record_mgmt.st_bss_mgmt),
                           pst_mac_vap->uc_vap_id);

        /* ֪ͨ�ں�ɨ����ֹ */
        oal_cfg80211_scan_done_etc(pst_scan_mgmt->pst_request, en_is_aborted);

        pst_scan_mgmt->pst_request = OAL_PTR_NULL;
        pst_scan_mgmt->en_complete = OAL_TRUE;
        /* ֪ͨ���ںˣ��ͷ���Դ����� */
        oal_spin_unlock(&(pst_scan_mgmt->st_scan_request_spinlock));
        /* �·�device��ֹɨ�� */
        wal_send_scan_abort_msg_etc(pst_net_dev);

        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{wal_force_scan_complete_etc::vap_id[%d] notify kernel scan abort!}",
                         pst_mac_vap->uc_vap_id);

        /* �ñ������Ż�ʱ��֤OAL_WAIT_QUEUE_WAKE_UP�����ִ�� */
        OAL_SMP_MB();
        OAL_WAIT_QUEUE_WAKE_UP_INTERRUPT(&pst_scan_mgmt->st_wait_queue);
    }
    else
    {
        /* ֪ͨ���ںˣ��ͷ���Դ����� */
        oal_spin_unlock(&(pst_scan_mgmt->st_scan_request_spinlock));
    }

    return OAL_SUCC;
}


oal_int32 wal_force_scan_complete_for_disconnect_scene(oal_net_device_stru   *pst_net_dev)
{
    mac_vap_stru            *pst_mac_vap;
    hmac_vap_stru           *pst_hmac_vap;
    hmac_device_stru        *pst_hmac_device;
    hmac_scan_stru          *pst_scan_mgmt;

    pst_mac_vap  = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{wal_force_scan_complete_for_disconnect_scene:: mac_vap of net_dev is deleted!iftype:[%d]}",pst_net_dev->ieee80211_ptr->iftype);
        return OAL_SUCC;
    }

    /* ��ȡhmac device */
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_hmac_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{wal_force_scan_complete_for_disconnect_scene::pst_hmac_device[%d] is null!}",
                         pst_mac_vap->uc_device_id);
        return -OAL_EINVAL;
    }

    /* ��ȡhmac vap */
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{wal_force_scan_complete_for_disconnect_scene::hmac_vap is null, vap_id[%d]!}", pst_mac_vap->uc_vap_id);
        return -OAL_EINVAL;
    }

    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);

    /* ����������ڲ���ɨ�� */
    if (OAL_PTR_NULL == pst_scan_mgmt->pst_request)
    {
        /* �ж��Ƿ�����ڲ�ɨ�裬������ڣ�Ҳ��Ҫֹͣ */
        if ((OAL_TRUE == pst_scan_mgmt->en_is_scanning) &&
            (pst_mac_vap->uc_vap_id == pst_scan_mgmt->st_scan_record_mgmt.uc_vap_id))
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                             "{wal_force_scan_complete_for_disconnect_scene::may be internal scan, stop scan!}");
            /* ��ֹɨ�� */
            wal_send_scan_abort_msg_etc(pst_net_dev);
        }

        return OAL_SUCC;
    }

    if ((OAL_PTR_NULL != pst_scan_mgmt->pst_request) && OAL_WDEV_MATCH(pst_net_dev, pst_scan_mgmt->pst_request))
    {
        /* �·�device��ֹɨ�� */
        wal_send_scan_abort_msg_etc(pst_net_dev);
        pst_scan_mgmt->st_scan_record_mgmt.en_scan_rsp_status = MAC_SCAN_ABORT_SYNC;
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{wal_force_scan_complete_for_disconnect_scene::vap_id[%d] notify kernel scan abort!}",
                         pst_mac_vap->uc_vap_id);
    }

    return OAL_SUCC;
}


oal_int32 wal_stop_sched_scan_etc(oal_net_device_stru *pst_netdev)
{
    hmac_device_stru               *pst_hmac_device;
    hmac_scan_stru                 *pst_scan_mgmt;
    mac_vap_stru                   *pst_mac_vap;
    wal_msg_write_stru              st_write_msg;
    oal_uint32                      ul_pedding_data = 0;       /* ������ݣ���ʹ�ã�ֻ��Ϊ�˸��ýӿ� */
    oal_int32                       l_ret = 0;

    /* �����Ϸ��Լ�� */
    if (OAL_PTR_NULL == pst_netdev)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{wal_stop_sched_scan_etc::pst_netdev is null}");
        return -OAL_EINVAL;
    }

    /* ͨ��net_device �ҵ���Ӧ��mac_device_stru �ṹ */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_netdev);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{wal_stop_sched_scan_etc:: pst_mac_vap is null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_hmac_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{wal_stop_sched_scan_etc:: pst_mac_device[%d] is null!}",
                    pst_mac_vap->uc_device_id);
        return -OAL_EINVAL;
    }

    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);

    OAM_WARNING_LOG2(0, OAM_SF_SCAN, "{wal_stop_sched_scan_etc::sched scan req[0x%x],sched scan complete[%d]}",pst_scan_mgmt->pst_sched_scan_req,pst_scan_mgmt->en_sched_scan_complete);

    if ((OAL_PTR_NULL != pst_scan_mgmt->pst_sched_scan_req) &&
        (OAL_TRUE != pst_scan_mgmt->en_sched_scan_complete))
    {
        /* �������ɨ������δִ�У����ϱ�����ɨ���� */
        //if (OAL_PTR_NULL == pst_scan_mgmt->pst_request)
        {
            oal_cfg80211_sched_scan_result_etc(pst_hmac_device->pst_device_base_info->pst_wiphy);
        }

        pst_scan_mgmt->pst_sched_scan_req     = OAL_PTR_NULL;
        pst_scan_mgmt->en_sched_scan_complete = OAL_TRUE;

        /* ���¼�֪ͨdevice��ֹͣPNO����ɨ�� */
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_STOP_SCHED_SCAN, OAL_SIZEOF(ul_pedding_data));

        oal_memcopy(st_write_msg.auc_value, (oal_int8 *)&ul_pedding_data, OAL_SIZEOF(ul_pedding_data));

        l_ret = wal_send_cfg_event_etc(pst_netdev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(ul_pedding_data),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{wal_stop_sched_scan_etc::fail to stop pno sched scan, error[%d]}", l_ret);
        }
    }

    return OAL_SUCC;
}


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

