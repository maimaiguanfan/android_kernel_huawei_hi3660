


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "oal_profiling.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "oal_net.h"
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif
#include "mac_frame.h"
#include "mac_data.h"
#include "hmac_rx_data.h"
#include "dmac_ext_if.h"
#include "hmac_vap.h"
#include "hmac_ext_if.h"
#include "hmac_frag.h"
#include "hmac_11i.h"
#include "mac_vap.h"
#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
#include "hmac_custom_security.h"
#endif
#ifdef _PRE_WLAN_FEATURE_MCAST
#include "hmac_m2u.h"
#endif

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
#include "hmac_proxy_arp.h"
#endif
#include "hmac_blockack.h"
#include "hmac_tcp_opt.h"

#ifdef _PRE_WLAN_FEATURE_WAPI
#include "hmac_wapi.h"
#endif
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
#include "hmac_proxysta.h"
#endif
#include "hmac_statistic_data_flow.h"

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
#include <linux/ip.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <linux/ieee80211.h>
#include <linux/ipv6.h>
#endif

#ifdef CONFIG_HUAWEI_DUBAI
#include <log/log_usertype.h>
//#include <log/log_usertype/log-usertype.h>
#include <huawei_platform/log/hwlog_kernel.h>
#endif
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RX_DATA_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


#ifdef _PRE_WLAN_DFT_DUMP_FRAME
oal_void  hmac_rx_report_eth_frame(mac_vap_stru   *pst_mac_vap,
                                                      oal_netbuf_stru *pst_netbuf)
{
    oal_uint16              us_user_idx = 0;
    mac_ether_header_stru  *pst_ether_hdr = OAL_PTR_NULL;
    oal_uint32              ul_ret;
    oal_uint8               auc_user_macaddr[WLAN_MAC_ADDR_LEN] = {0};
    oal_switch_enum_uint8   en_eth_switch = 0;
#if 0 //def _PRE_WLAN_DFT_STAT
    hmac_vap_stru           *pst_hmac_vap;
#endif

    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_netbuf))
    {
        return;
    }

    /* ��skb��dataָ��ָ����̫����֡ͷ */
    oal_netbuf_push(pst_netbuf, ETHER_HDR_LEN);
#if 0 //def _PRE_WLAN_DFT_STAT
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
#endif
    /* ����ͳ����Ϣ */
    //HMAC_VAP_DFT_STATS_PKT_INCR(pst_hmac_vap->st_query_stats.ul_rx_pkt_to_lan,1);
    //HMAC_VAP_DFT_STATS_PKT_INCR(pst_hmac_vap->st_query_stats.ul_rx_bytes_to_lan,OAL_NETBUF_LEN(pst_netbuf));
    //OAM_STAT_VAP_INCR(pst_mac_vap->uc_vap_id, rx_pkt_to_lan, 1);                           /* ���ӷ���LAN��֡����Ŀ */
    //OAM_STAT_VAP_INCR(pst_mac_vap->uc_vap_id, rx_bytes_to_lan, OAL_NETBUF_LEN(pst_netbuf));/* ���ӷ���LAN���ֽ��� */

    /* ��ȡĿ���û���Դ��id */
    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_ether_hdr))
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{hmac_rx_report_eth_frame::pst_ether_hdr null.}");
            oal_netbuf_pull(pst_netbuf, ETHER_HDR_LEN);
            return;
        }

        ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap, pst_ether_hdr->auc_ether_shost, &us_user_idx);
        if (OAL_ERR_CODE_PTR_NULL == ul_ret)
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{hmac_rx_report_eth_frame::ul_ret null.}");
            oal_netbuf_pull(pst_netbuf, ETHER_HDR_LEN);
            return;
        }

        if (OAL_FAIL == ul_ret)
        {
            oal_netbuf_pull(pst_netbuf, ETHER_HDR_LEN);
            return;
        }

        oal_set_mac_addr(auc_user_macaddr, pst_ether_hdr->auc_ether_shost);
    }
    else if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        if (0 == pst_mac_vap->us_user_nums)
        {
            oal_netbuf_pull(pst_netbuf, ETHER_HDR_LEN);
            /* SUCC , return */
            return;
        }

        us_user_idx = pst_mac_vap->uc_assoc_vap_id;
        oal_set_mac_addr(auc_user_macaddr, pst_mac_vap->auc_bssid);
    }

    ul_ret = oam_report_eth_frame_get_switch(us_user_idx, OAM_OTA_FRAME_DIRECTION_TYPE_RX, &en_eth_switch);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{hmac_rx_report_eth_frame::oam_report_eth_frame_get_switch failed[%d].}", ul_ret);
        oal_netbuf_pull(pst_netbuf, ETHER_HDR_LEN);
        return;
    }

    if (OAL_SWITCH_ON == en_eth_switch)
    {
        /* ��Ҫ������̫����֡�ϱ� */
        ul_ret = oam_report_eth_frame(auc_user_macaddr,
                             oal_netbuf_data(pst_netbuf),
                             (oal_uint16)OAL_NETBUF_LEN(pst_netbuf),
                             OAM_OTA_FRAME_DIRECTION_TYPE_RX);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{hmac_rx_report_eth_frame::oam_report_eth_frame return err: 0x%x.}\r\n", ul_ret);
        }
    }

    oal_netbuf_pull(pst_netbuf, ETHER_HDR_LEN);

}
#endif


OAL_STATIC OAL_INLINE oal_void  hmac_rx_frame_80211_to_eth(
                                    oal_netbuf_stru     *pst_netbuf,
                                    oal_uint8           *puc_da,
                                    oal_uint8           *puc_sa)
{
    mac_ether_header_stru              *pst_ether_hdr;
    mac_llc_snap_stru                  *pst_snap;
    oal_uint16                          us_ether_type;

    pst_snap = (mac_llc_snap_stru *)oal_netbuf_data(pst_netbuf);
    us_ether_type = pst_snap->us_ether_type;

    /* ��payload��ǰ����6���ֽڣ����Ϻ���8���ֽڵ�snapͷ�ռ䣬������̫��ͷ��14�ֽڿռ� */
    oal_netbuf_push(pst_netbuf, HMAC_RX_DATA_ETHER_OFFSET_LENGTH);
    pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);

    pst_ether_hdr->us_ether_type = us_ether_type;
    oal_set_mac_addr(pst_ether_hdr->auc_ether_shost, puc_sa);
    oal_set_mac_addr(pst_ether_hdr->auc_ether_dhost, puc_da);
}


oal_void  hmac_rx_free_netbuf(oal_netbuf_stru *pst_netbuf, oal_uint16 us_nums)
{
    oal_netbuf_stru    *pst_netbuf_temp;
    oal_uint16           us_netbuf_num;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_netbuf))
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_free_netbuf::pst_netbuf null.}\r\n");
        return;
    }

    for (us_netbuf_num = us_nums; us_netbuf_num > 0; us_netbuf_num--)
    {
        pst_netbuf_temp = OAL_NETBUF_NEXT(pst_netbuf);

        /* ����netbuf��Ӧ��user���ü��� */
        oal_netbuf_free(pst_netbuf);

        pst_netbuf = pst_netbuf_temp;

        if (OAL_PTR_NULL == pst_netbuf)
        {
            if (OAL_UNLIKELY(us_netbuf_num != 1))
            {
                OAM_ERROR_LOG2(0, OAM_SF_RX, "{hmac_rx_free_netbuf::pst_netbuf list broken, us_netbuf_num[%d]us_nums[%d].}", us_netbuf_num, us_nums);
                return;
            }

            break;
        }
    }

}


oal_void  hmac_rx_free_netbuf_list(oal_netbuf_head_stru *pst_netbuf_hdr, oal_uint16 uc_num_buf)
{
    oal_netbuf_stru   *pst_netbuf;
    oal_uint16         us_idx;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_netbuf_hdr))
    {
        OAM_INFO_LOG0(0, OAM_SF_RX, "{hmac_rx_free_netbuf_list::pst_netbuf null.}");
        return;
    }

    OAM_INFO_LOG1(0, OAM_SF_RX, "{hmac_rx_free_netbuf_list::free [%d].}", uc_num_buf);

    for (us_idx = uc_num_buf; us_idx > 0; us_idx--)
    {
        pst_netbuf = oal_netbuf_delist(pst_netbuf_hdr);
        if (OAL_PTR_NULL != pst_netbuf)
        {
            OAM_INFO_LOG0(0, OAM_SF_RX, "{hmac_rx_free_netbuf_list::pst_netbuf null.}");
            oal_netbuf_free(pst_netbuf);
        }
    }

}


OAL_STATIC oal_uint32  hmac_rx_transmit_to_wlan(
                frw_event_hdr_stru   *pst_event_hdr,
                oal_netbuf_head_stru *pst_netbuf_head)
{
    oal_netbuf_stru            *pst_netbuf;         /* ��netbuf����ȡ������ָ��netbuf��ָ�� */
    oal_uint32                  ul_netbuf_num;
    oal_uint32                  ul_ret;
    oal_netbuf_stru            *pst_buf_tmp;        /* �ݴ�netbufָ�룬����whileѭ�� */
    mac_tx_ctl_stru            *pst_tx_ctl;
    mac_vap_stru               *pst_mac_vap;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_event_hdr) || (OAL_PTR_NULL == pst_netbuf_head)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_RX, "{hmac_rx_transmit_to_wlan::param null, %d %d.}", pst_event_hdr, pst_netbuf_head);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡ��ͷ��net buffer */
    pst_netbuf = oal_netbuf_peek(pst_netbuf_head);

    /* ��ȡmac vap �ṹ */
    ul_ret = hmac_tx_get_mac_vap(pst_event_hdr->uc_vap_id, &pst_mac_vap);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        ul_netbuf_num = oal_netbuf_list_len(pst_netbuf_head);
        hmac_rx_free_netbuf(pst_netbuf, (oal_uint16)ul_netbuf_num);
        OAM_WARNING_LOG3(pst_event_hdr->uc_vap_id, OAM_SF_RX, "{hmac_rx_transmit_to_wlan::find vap [%d] failed[%d], free [%d] netbuffer.}",
                        pst_event_hdr->uc_vap_id, ul_ret, ul_netbuf_num);
        return ul_ret;
    }

    /* ѭ������ÿһ��netbuf��������̫��֡�ķ�ʽ���� */
    while (OAL_PTR_NULL != pst_netbuf)
    {
        pst_buf_tmp = OAL_NETBUF_NEXT(pst_netbuf);

        OAL_NETBUF_NEXT(pst_netbuf) = OAL_PTR_NULL;
        OAL_NETBUF_PREV(pst_netbuf) = OAL_PTR_NULL;

        pst_tx_ctl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_netbuf);
        OAL_MEMZERO(pst_tx_ctl, sizeof(mac_tx_ctl_stru));

        pst_tx_ctl->en_event_type = FRW_EVENT_TYPE_WLAN_DTX;
        pst_tx_ctl->uc_event_sub_type = DMAC_TX_WLAN_DTX;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        /*set the queue map id when wlan to wlan*/
        oal_skb_set_queue_mapping(pst_netbuf, WLAN_NORMAL_QUEUE);
#endif

        ul_ret = hmac_tx_lan_to_wlan(pst_mac_vap, pst_netbuf);

        /* ����ʧ�ܣ��Լ������Լ��ͷ�netbuff�ڴ� */
        if(OAL_SUCC != ul_ret)
        {
            hmac_free_netbuf_list(pst_netbuf);
        }

        pst_netbuf = pst_buf_tmp;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_void  hmac_rx_free_amsdu_netbuf(oal_netbuf_stru *pst_netbuf)
{
    oal_netbuf_stru        *pst_netbuf_next;
    while (OAL_PTR_NULL != pst_netbuf)
    {
        pst_netbuf_next = oal_get_netbuf_next(pst_netbuf);
        oal_netbuf_free(pst_netbuf);
        pst_netbuf = pst_netbuf_next;
    }
}


OAL_STATIC oal_void  hmac_rx_clear_amsdu_last_netbuf_pointer(oal_netbuf_stru *pst_netbuf, oal_uint8 uc_num_buf)
{
    if (0 == uc_num_buf)
    {
        pst_netbuf->next = OAL_PTR_NULL;
        return;
    }

    while (pst_netbuf != OAL_PTR_NULL)
    {
        uc_num_buf--;
        if (0 == uc_num_buf)
        {
            pst_netbuf->next = OAL_PTR_NULL;
            break;
        }
        pst_netbuf = oal_get_netbuf_next(pst_netbuf);
    }
}


oal_uint32  hmac_rx_parse_amsdu(
                oal_netbuf_stru                    *pst_netbuf,
                dmac_msdu_stru                     *pst_msdu,
                dmac_msdu_proc_state_stru          *pst_msdu_state,
                mac_msdu_proc_status_enum_uint8    *pen_proc_state)
{
    mac_rx_ctl_stru       *pst_rx_ctrl;                            /* MPDU�Ŀ�����Ϣ */
    oal_uint8              *puc_buffer_data_addr    = OAL_PTR_NULL; /* ָ��netbuf�������ָ�� */
    oal_uint16              us_offset               = 0;            /* submsdu�����dataָ���ƫ�� */
    oal_uint16              us_submsdu_len          = 0;            /* submsdu�ĳ��� */
    oal_uint8               uc_submsdu_pad_len      = 0;            /* submsdu����䳤�� */
    oal_uint8              *puc_submsdu_hdr         = OAL_PTR_NULL; /* ָ��submsduͷ����ָ�� */
    oal_netbuf_stru        *pst_netbuf_prev;
    oal_bool_enum_uint8     b_need_free_netbuf      = OAL_FALSE;
    oal_uint32              ul_need_pull_len;


    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_netbuf))
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_parse_amsdu::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* �״ν���ú�������AMSDU */
    if ((0 == pst_msdu_state->uc_procd_netbuf_nums)
     && (0 == pst_msdu_state->uc_procd_msdu_in_netbuf))
    {
        pst_msdu_state->pst_curr_netbuf      = pst_netbuf;

        /* AMSDUʱ���׸�netbuf���а���802.11ͷ����Ӧ��payload��Ҫƫ�� */
        pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_msdu_state->pst_curr_netbuf);

        pst_msdu_state->puc_curr_netbuf_data   = (oal_uint8*)mac_get_rx_cb_mac_hdr(pst_rx_ctrl) + pst_rx_ctrl->uc_mac_header_len;
        pst_msdu_state->uc_netbuf_nums_in_mpdu = pst_rx_ctrl->bit_buff_nums;
        pst_msdu_state->uc_msdu_nums_in_netbuf = pst_rx_ctrl->uc_msdu_in_buffer;
        pst_msdu_state->us_submsdu_offset      = 0;

        /* ʹnetbuf ָ��amsdu ֡ͷ */
        ul_need_pull_len = (oal_uint32)(pst_msdu_state->puc_curr_netbuf_data - oal_netbuf_payload(pst_netbuf));
        oal_netbuf_pull(pst_msdu_state->pst_curr_netbuf, ul_need_pull_len);
    }

    /* ��ȡsubmsdu��ͷָ�� */
    puc_buffer_data_addr = pst_msdu_state->puc_curr_netbuf_data;
    us_offset            = pst_msdu_state->us_submsdu_offset;
    puc_submsdu_hdr      = puc_buffer_data_addr + us_offset;

    /* 1��netbuf ֻ����һ��msdu */
    if (1 == pst_msdu_state->uc_msdu_nums_in_netbuf)
    {
        mac_get_submsdu_len(puc_submsdu_hdr, &us_submsdu_len);
        //mac_get_submsdu_pad_len(MAC_SUBMSDU_HEADER_LEN + us_submsdu_len, &uc_submsdu_pad_len);
        oal_set_mac_addr(pst_msdu->auc_sa, (puc_submsdu_hdr + MAC_SUBMSDU_SA_OFFSET));
        oal_set_mac_addr(pst_msdu->auc_da, (puc_submsdu_hdr + MAC_SUBMSDU_DA_OFFSET));

        /* ָ��amsdu֡�� */
        oal_netbuf_pull(pst_msdu_state->pst_curr_netbuf, MAC_SUBMSDU_HEADER_LEN);

        if (us_submsdu_len > OAL_NETBUF_LEN(pst_msdu_state->pst_curr_netbuf))
        {
            *pen_proc_state = MAC_PROC_ERROR;
            OAM_WARNING_LOG2(0, OAM_SF_RX, "{hmac_rx_parse_amsdu::us_submsdu_len %d is not valid netbuf len=%d.}", us_submsdu_len,
                        OAL_NETBUF_LEN(pst_msdu_state->pst_curr_netbuf));
            hmac_rx_free_amsdu_netbuf(pst_msdu_state->pst_curr_netbuf);
            return OAL_FAIL;
        }

        oal_netbuf_trim(pst_msdu_state->pst_curr_netbuf, OAL_NETBUF_LEN(pst_msdu_state->pst_curr_netbuf));

        oal_netbuf_put(pst_msdu_state->pst_curr_netbuf, us_submsdu_len);

         /* ֱ��ʹ�ø�netbuf�ϱ����ں� ��ȥһ��netbuf����Ϳ��� */
        b_need_free_netbuf = OAL_FALSE;
        pst_msdu->pst_netbuf = pst_msdu_state->pst_curr_netbuf;

    }
    else
    {
        /* ��ȡsubmsdu�������Ϣ */
        mac_get_submsdu_len(puc_submsdu_hdr, &us_submsdu_len);
        mac_get_submsdu_pad_len(MAC_SUBMSDU_HEADER_LEN + us_submsdu_len, &uc_submsdu_pad_len);
        oal_set_mac_addr(pst_msdu->auc_sa, (puc_submsdu_hdr + MAC_SUBMSDU_SA_OFFSET));
        oal_set_mac_addr(pst_msdu->auc_da, (puc_submsdu_hdr + MAC_SUBMSDU_DA_OFFSET));

        /* ��Ե�ǰ��netbuf�������µ�subnetbuf�������ö�Ӧ��netbuf����Ϣ����ֵ����Ӧ��msdu */
        pst_msdu->pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, (MAC_SUBMSDU_HEADER_LEN + us_submsdu_len + uc_submsdu_pad_len), OAL_NETBUF_PRIORITY_MID);
        if (OAL_PTR_NULL == pst_msdu->pst_netbuf)
        {
            OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_parse_amsdu::pst_netbuf null.}");
            OAM_STAT_VAP_INCR(0, rx_no_buff_dropped, 1);
            hmac_rx_free_amsdu_netbuf(pst_msdu_state->pst_curr_netbuf);
            return OAL_FAIL;
        }

        OAL_MEM_NETBUF_TRACE(pst_msdu->pst_netbuf, OAL_TRUE);

        /* ���ÿһ����msdu���޸�netbuf��end��data��tail��lenָ�� */
        oal_netbuf_put(pst_msdu->pst_netbuf, us_submsdu_len + HMAC_RX_DATA_ETHER_OFFSET_LENGTH);
        oal_netbuf_pull(pst_msdu->pst_netbuf, HMAC_RX_DATA_ETHER_OFFSET_LENGTH);
        oal_memcopy(pst_msdu->pst_netbuf->data, (puc_submsdu_hdr + MAC_SUBMSDU_HEADER_LEN), us_submsdu_len);

        b_need_free_netbuf = OAL_TRUE;
    }

    /* ���ӵ�ǰ�Ѵ����msdu�ĸ��� */
    pst_msdu_state->uc_procd_msdu_in_netbuf++;

    /* ��ȡ��ǰ��netbuf�е���һ��msdu���д��� */
    if (pst_msdu_state->uc_procd_msdu_in_netbuf < pst_msdu_state->uc_msdu_nums_in_netbuf)
    {
        pst_msdu_state->us_submsdu_offset += us_submsdu_len + uc_submsdu_pad_len + MAC_SUBMSDU_HEADER_LEN;
    }
    else if (pst_msdu_state->uc_procd_msdu_in_netbuf == pst_msdu_state->uc_msdu_nums_in_netbuf)
    {
        pst_msdu_state->uc_procd_netbuf_nums++;

        pst_netbuf_prev = pst_msdu_state->pst_curr_netbuf;

        /* ��ȡ��MPDU��Ӧ����һ��netbuf������ */
        if (pst_msdu_state->uc_procd_netbuf_nums < pst_msdu_state->uc_netbuf_nums_in_mpdu)
        {
            pst_msdu_state->pst_curr_netbuf      = OAL_NETBUF_NEXT(pst_msdu_state->pst_curr_netbuf);
            pst_msdu_state->puc_curr_netbuf_data = oal_netbuf_data(pst_msdu_state->pst_curr_netbuf);

            pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_msdu_state->pst_curr_netbuf);

            pst_msdu_state->uc_msdu_nums_in_netbuf  = pst_rx_ctrl->uc_msdu_in_buffer;
            pst_msdu_state->us_submsdu_offset       = 0;
            pst_msdu_state->uc_procd_msdu_in_netbuf = 0;

            /* amsdu �ڶ���netbuf len��0, ��put�����size */
            oal_netbuf_put(pst_msdu_state->pst_curr_netbuf, WLAN_MEM_NETBUF_SIZE2);
        }
        else if (pst_msdu_state->uc_procd_netbuf_nums == pst_msdu_state->uc_netbuf_nums_in_mpdu)
        {
           *pen_proc_state = MAC_PROC_LAST_MSDU;
            if (b_need_free_netbuf)
            {
                oal_netbuf_free(pst_netbuf_prev);
            }
            return OAL_SUCC;
        }
        else
        {
           *pen_proc_state = MAC_PROC_ERROR;
           OAM_WARNING_LOG0(0, OAM_SF_RX, "{hmac_rx_parse_amsdu::pen_proc_state is err for uc_procd_netbuf_nums > uc_netbuf_nums_in_mpdul.}");
           hmac_rx_free_amsdu_netbuf(pst_msdu_state->pst_curr_netbuf);
           return OAL_FAIL;
        }
        if (b_need_free_netbuf)
        {
            oal_netbuf_free(pst_netbuf_prev);
        }
    }
    else
    {
        *pen_proc_state = MAC_PROC_ERROR;
        OAM_WARNING_LOG0(0, OAM_SF_RX, "{hmac_rx_parse_amsdu::pen_proc_state is err for uc_procd_netbuf_nums > uc_netbuf_nums_in_mpdul.}");
        hmac_rx_free_amsdu_netbuf(pst_msdu_state->pst_curr_netbuf);
        return OAL_FAIL;
    }

    *pen_proc_state = MAC_PROC_MORE_MSDU;

    return OAL_SUCC;

}


OAL_STATIC oal_uint32  hmac_rx_prepare_msdu_list_to_wlan(
                hmac_vap_stru                *pst_vap,
                oal_netbuf_head_stru         *pst_netbuf_header,
                oal_netbuf_stru              *pst_netbuf,
                mac_ieee80211_frame_stru     *pst_frame_hdr)
{
    hmac_rx_ctl_stru                   *pst_rx_ctrl;                        /* ָ��MPDU���ƿ���Ϣ��ָ�� */
    dmac_msdu_stru                      st_msdu;                            /* �������������ÿһ��MSDU */
    mac_msdu_proc_status_enum_uint8     en_process_state = MAC_PROC_BUTT;   /* ����AMSDU��״̬ */
    dmac_msdu_proc_state_stru           st_msdu_state    = {0};             /* ��¼MPDU�Ĵ�����Ϣ */
    oal_uint8                          *puc_addr         = OAL_PTR_NULL;
    oal_uint32                          ul_ret;
    oal_uint8                           auc_sa[WLAN_MAC_ADDR_LEN];
    oal_uint8                           auc_da[WLAN_MAC_ADDR_LEN];
#if defined(_PRE_WLAN_FEATURE_WPA) || defined(_PRE_WLAN_FEATURE_WPA2)
    mac_ether_header_stru              *pst_ether_hdr;
#endif
    hmac_user_stru                     *pst_hmac_user = OAL_PTR_NULL;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_netbuf))
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_prepare_msdu_list_to_wlan::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ����MPDU-->MSDU *//* ��MSDU���netbuf�� */
    OAL_MEM_NETBUF_TRACE(pst_netbuf, OAL_TRUE);

    /* ��ȡ��MPDU�Ŀ�����Ϣ */
    pst_rx_ctrl = (hmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    OAL_MEMZERO(&st_msdu, OAL_SIZEOF(dmac_msdu_stru));

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(pst_rx_ctrl->st_rx_info.us_ta_user_idx);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hmac_user))
    {
        if (OAL_FALSE == pst_rx_ctrl->st_rx_info.bit_amsdu_enable)
        {
            /* ��ӡ��net buf�����Ϣ */
            mac_rx_report_80211_frame((oal_uint8 *)&(pst_vap->st_vap_base_info),
                                      (oal_uint8 *)&(pst_rx_ctrl->st_rx_info),
                                      pst_netbuf,
                                      OAM_OTA_TYPE_RX_HMAC_CB);
        }
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* ���һ:����AMSDU�ۺϣ����MPDU��Ӧһ��MSDU��ͬʱ��Ӧһ��NETBUF,��MSDU��ԭ
       ����̫����ʽ֡�Ժ�ֱ�Ӽ��뵽netbuf�������
    */
    if (OAL_FALSE == pst_rx_ctrl->st_rx_info.bit_amsdu_enable)
    {
        pst_netbuf = hmac_defrag_process(pst_hmac_user, pst_netbuf, pst_rx_ctrl->st_rx_info.uc_mac_header_len);
        if (OAL_PTR_NULL == pst_netbuf)
        {
            return OAL_SUCC;
        }

        pst_rx_ctrl     = (hmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
        pst_frame_hdr = (mac_ieee80211_frame_stru *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr;

        /* ��MACͷ�л�ȡԴ��ַ��Ŀ�ĵ�ַ */
        mac_rx_get_sa(pst_frame_hdr, &puc_addr);
        oal_set_mac_addr(auc_sa, puc_addr);

        mac_rx_get_da(pst_frame_hdr, &puc_addr);
        oal_set_mac_addr(auc_da, puc_addr);

        /* ��netbuf��dataָ��ָ��mac frame��payload����Ҳ����ָ����8�ֽڵ�snapͷ */
        oal_netbuf_pull(pst_netbuf, pst_rx_ctrl->st_rx_info.uc_mac_header_len);

        /* ��MSDUת��Ϊ��̫����ʽ��֡ */
        hmac_rx_frame_80211_to_eth(pst_netbuf, auc_da, auc_sa);

        OAL_MEMZERO(OAL_NETBUF_CB(pst_netbuf), OAL_NETBUF_CB_SIZE());


#if defined(_PRE_WLAN_FEATURE_WPA) || defined(_PRE_WLAN_FEATURE_WPA2)
        pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);

        if (OAL_SUCC != hmac_11i_ether_type_filter(pst_vap, &pst_hmac_user->st_user_base_info, pst_ether_hdr->us_ether_type))
        {/* ���հ�ȫ���ݹ��� */

            oam_report_eth_frame(auc_da, (oal_uint8*)pst_ether_hdr, (oal_uint16)OAL_NETBUF_LEN(pst_netbuf), OAM_OTA_FRAME_DIRECTION_TYPE_RX);

            oal_netbuf_free(pst_netbuf);
            OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_portvalid_check_fail_dropped, 1);
            return OAL_FAIL;
        }
        else
#endif
        {
            /* ��MSDU���뵽netbuf������� */
            oal_netbuf_add_to_list_tail(pst_netbuf, pst_netbuf_header);
        }
    }

    else /* �����:AMSDU�ۺ� */
    {
        st_msdu_state.uc_procd_netbuf_nums    = 0;
        st_msdu_state.uc_procd_msdu_in_netbuf = 0;

        /* amsdu ���һ��netbuf nextָ����Ϊ NULL ����ʱ�����ͷ�amsdu netbuf */
        hmac_rx_clear_amsdu_last_netbuf_pointer(pst_netbuf, pst_rx_ctrl->st_rx_info.bit_buff_nums);

        do
        {
            /* ��ȡ��һ��Ҫת����msdu */
            ul_ret = hmac_rx_parse_amsdu(pst_netbuf, &st_msdu, &st_msdu_state, &en_process_state);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX, "{hmac_rx_prepare_msdu_list_to_wlan::hmac_rx_parse_amsdu failed[%d].}", ul_ret);
                return ul_ret;
            }

            /* ��MSDUת��Ϊ��̫����ʽ��֡ */
            hmac_rx_frame_80211_to_eth(st_msdu.pst_netbuf, st_msdu.auc_da, st_msdu.auc_sa);

#if defined(_PRE_WLAN_FEATURE_WPA) || defined(_PRE_WLAN_FEATURE_WPA2)
            pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(st_msdu.pst_netbuf);

            if (OAL_SUCC != hmac_11i_ether_type_filter(pst_vap, &pst_hmac_user->st_user_base_info, pst_ether_hdr->us_ether_type))
            {
                /* ���հ�ȫ���ݹ��� */
                oam_report_eth_frame(st_msdu.auc_da, (oal_uint8*)pst_ether_hdr, (oal_uint16)OAL_NETBUF_LEN(pst_netbuf), OAM_OTA_FRAME_DIRECTION_TYPE_RX);

                oal_netbuf_free(st_msdu.pst_netbuf);
                OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_portvalid_check_fail_dropped, 1);
                continue;
            }
            else
#endif
            {
                /* ��MSDU���뵽netbuf������� */
                oal_netbuf_add_to_list_tail(st_msdu.pst_netbuf, pst_netbuf_header);
            }
        }while (MAC_PROC_LAST_MSDU != en_process_state);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_PKT_MEM_OPT
OAL_STATIC oal_void hmac_pkt_mem_opt_stat_reset(hmac_device_stru *pst_hmac_device, oal_bool_enum_uint8 en_dscr_opt_state)
{
    frw_event_mem_stru    *pst_event_mem;
    frw_event_stru        *pst_event;
    hmac_rx_dscr_opt_stru *pst_dscr_opt = &pst_hmac_device->st_rx_dscr_opt;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hmac_device->pst_device_base_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_pkt_mem_opt_stat_reset::pst_device_base_info null!}");
        return;
    }


    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{hmac_rx_dscr_opt_stat_reset::new_state[%d], pkt_num[%d]}", en_dscr_opt_state, pst_dscr_opt->ul_rx_pkt_num);
    pst_dscr_opt->en_dscr_opt_state = en_dscr_opt_state;
    pst_dscr_opt->ul_rx_pkt_num     = 0;

    /***************************************************************************
        ���¼���dmacģ��,��ͳ����Ϣ����dmac
    ***************************************************************************/
    pst_event_mem = FRW_EVENT_ALLOC(0);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_rx_dscr_opt_timeout_fn::pst_event_mem null.}");
        return;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    /* ��д�¼�ͷ */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                    FRW_EVENT_TYPE_WLAN_CTX,
                    DMAC_WLAN_CTX_EVENT_SUB_TYPE_DSCR_OPT,
                    0,
                    FRW_EVENT_PIPELINE_STAGE_1,
                    pst_hmac_device->pst_device_base_info->uc_chip_id,
                    pst_hmac_device->pst_device_base_info->uc_device_id,
                    0);

    /* �������� */
    pst_event->auc_event_data[0] = pst_dscr_opt->en_dscr_opt_state;

    /* �ַ��¼� */
    frw_event_dispatch_event(pst_event_mem);
    FRW_EVENT_FREE(pst_event_mem);
}


oal_void hmac_pkt_mem_opt_cfg(oal_uint32 ul_cfg_tpye, oal_uint32 ul_cfg_value)
{
    hmac_device_stru      *pst_hmac_device = (hmac_device_stru*)hmac_res_get_mac_dev(0);
    hmac_rx_dscr_opt_stru *pst_dscr_opt;

    if(ul_cfg_tpye > 2)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_rx_dscr_opt_cfg::invalid cfg tpye.}");
        return;
    }
    if(OAL_PTR_NULL == pst_hmac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_rx_dscr_opt_cfg::hmac device is null.}");
        return;
    }

    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{hmac_rx_dscr_opt_cfg::cfg type[%d], cfg value[%d].}", ul_cfg_tpye, ul_cfg_value);
    pst_dscr_opt = &pst_hmac_device->st_rx_dscr_opt;
    if(0 == ul_cfg_tpye)
    {
        pst_dscr_opt->en_dscr_opt_enable = (oal_uint8)ul_cfg_value;
        if(OAL_FALSE == pst_dscr_opt->en_dscr_opt_enable && OAL_TRUE == pst_dscr_opt->en_dscr_opt_state)
        {
            hmac_pkt_mem_opt_stat_reset(pst_hmac_device, OAL_FALSE);
        }
    }
    if(1 == ul_cfg_tpye)
    {
        pst_dscr_opt->ul_rx_pkt_opt_limit = ul_cfg_value;
    }
    if(2 == ul_cfg_tpye)
    {
        pst_dscr_opt->ul_rx_pkt_reset_limit = ul_cfg_value;
    }
}

oal_uint32  hmac_pkt_mem_opt_timeout_fn(oal_void *p_arg)
{
    hmac_device_stru      *pst_hmac_device;
    hmac_rx_dscr_opt_stru *pst_dscr_opt;


    if (OAL_UNLIKELY(OAL_PTR_NULL == p_arg))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_rx_dscr_opt_timeout_fn::p_arg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_device = (hmac_device_stru *)p_arg;
    pst_dscr_opt    = &pst_hmac_device->st_rx_dscr_opt;

    if(OAL_TRUE != pst_dscr_opt->en_dscr_opt_enable)
    {
        return OAL_SUCC;
    }

    OAM_INFO_LOG2(0, OAM_SF_ANY, "{hmac_rx_dscr_opt_timeout_fn::state[%d], pkt_num[%d]}", pst_dscr_opt->en_dscr_opt_state, pst_dscr_opt->ul_rx_pkt_num);

    /* rx_dscrδ����״̬ʱ, ��⵽RXҵ��,���������� */
    if(OAL_FALSE == pst_dscr_opt->en_dscr_opt_state && pst_dscr_opt->ul_rx_pkt_num > pst_dscr_opt->ul_rx_pkt_opt_limit)
    {
        hmac_pkt_mem_opt_stat_reset(pst_hmac_device, OAL_TRUE);
    }
    /* rx_dscr�ѵ���״̬ʱ, δ��⵽RXҵ��,������������,��֤TX���� */
    else if(OAL_TRUE == pst_dscr_opt->en_dscr_opt_state && pst_dscr_opt->ul_rx_pkt_num < pst_dscr_opt->ul_rx_pkt_reset_limit)
    {
        hmac_pkt_mem_opt_stat_reset(pst_hmac_device, OAL_FALSE);
    }
    else
    {
        pst_dscr_opt->ul_rx_pkt_num  = 0;
    }

    return OAL_SUCC;
}

oal_void hmac_pkt_mem_opt_init(hmac_device_stru *pst_hmac_device)
{
    if (OAL_TRUE == pst_hmac_device->st_rx_dscr_opt.st_rx_dscr_opt_timer.en_is_registerd)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_pkt_mem_opt_init::rx_dscr_timer is_registerd.}");
        return;
    }

    pst_hmac_device->st_rx_dscr_opt.en_dscr_opt_state     = OAL_FALSE;
    pst_hmac_device->st_rx_dscr_opt.ul_rx_pkt_num         = 0;
    pst_hmac_device->st_rx_dscr_opt.ul_rx_pkt_opt_limit   = WLAN_PKT_MEM_PKT_OPT_LIMIT;
    pst_hmac_device->st_rx_dscr_opt.ul_rx_pkt_reset_limit = WLAN_PKT_MEM_PKT_RESET_LIMIT;
    pst_hmac_device->st_rx_dscr_opt.en_dscr_opt_enable    = OAL_TRUE;

    FRW_TIMER_CREATE_TIMER(&(pst_hmac_device->st_rx_dscr_opt.st_rx_dscr_opt_timer), hmac_pkt_mem_opt_timeout_fn, WLAN_PKT_MEM_OPT_TIME_MS, pst_hmac_device, OAL_TRUE, OAM_MODULE_ID_HMAC,0);
}

oal_void hmac_pkt_mem_opt_exit(hmac_device_stru *pst_hmac_device)
{
    if (OAL_TRUE == pst_hmac_device->st_rx_dscr_opt.st_rx_dscr_opt_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hmac_device->st_rx_dscr_opt.st_rx_dscr_opt_timer));
    }
}


OAL_STATIC oal_void  hmac_pkt_mem_opt_rx_pkts_stat(hmac_vap_stru *pst_vap, oal_ip_header_stru *pst_ip)
{
    hmac_device_stru *pst_hmac_device = (hmac_device_stru*)hmac_res_get_mac_dev(pst_vap->st_vap_base_info.uc_device_id);

    if (OAL_PTR_NULL == pst_hmac_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_pkt_mem_opt_rx_pkts_stat::hmac_res_get_mac_dev fail.device_id :%d}",pst_vap->st_vap_base_info.uc_device_id);
        return;
    }
    /* ����IP_LEN С�� HMAC_RX_DSCR_OPT_MIN_PKT_LEN�ı��� */
    if (OAL_NET2HOST_SHORT(pst_ip->us_tot_len) < WLAN_PKT_MEM_OPT_MIN_PKT_LEN)
    {
        return;
    }

    if ((MAC_UDP_PROTOCAL == pst_ip->uc_protocol) || (MAC_TCP_PROTOCAL == pst_ip->uc_protocol))
    {
        pst_hmac_device->st_rx_dscr_opt.ul_rx_pkt_num++;
    }
    else
    {
        OAM_INFO_LOG0(0, OAM_SF_RX, "{hmac_rx_dscr_opt_rx_pkts_stat: neither UDP nor TCP ");
    }
}
#endif

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE

OAL_STATIC oal_void hmac_parse_ipv4_packet(oal_void *pst_eth)
{
    const struct iphdr *iph;
    oal_uint32 iphdr_len = 0;
    struct tcphdr *th;
    struct udphdr *uh;
    struct icmphdr *icmph;

    OAL_IO_PRINT(WIFI_WAKESRC_TAG"ipv4 packet.\n");
    iph = (struct iphdr *)((mac_ether_header_stru *)pst_eth + 1);
    iphdr_len = iph->ihl*4;

    OAL_IO_PRINT(WIFI_WAKESRC_TAG"src ip:%d.x.x.%d, dst ip:%d.x.x.%d\n", IPADDR(iph->saddr), IPADDR(iph->daddr));
    if (iph->protocol == IPPROTO_UDP){
        uh = (struct udphdr *)((oal_uint8*)iph + iphdr_len);
        OAL_IO_PRINT(WIFI_WAKESRC_TAG"UDP packet, src port:%d, dst port:%d.\n", OAL_NTOH_16(uh->source), OAL_NTOH_16(uh->dest));
#ifdef CONFIG_HUAWEI_DUBAI
        if (BETA_USER == get_logusertype_flag()) {
            HWDUBAI_LOGE("DUBAI_TAG_PACKET_WAKEUP_UDP_V4", "port=%d", OAL_NTOH_16(uh->dest));
        }
#endif
    }else if(iph->protocol == IPPROTO_TCP){
        th = (struct tcphdr *)((oal_uint8*)iph + iphdr_len);
        OAL_IO_PRINT(WIFI_WAKESRC_TAG"TCP packet, src port:%d, dst port:%d.\n", OAL_NTOH_16(th->source), OAL_NTOH_16(th->dest));
#ifdef CONFIG_HUAWEI_DUBAI
        if (BETA_USER == get_logusertype_flag()) {
            HWDUBAI_LOGE("DUBAI_TAG_PACKET_WAKEUP_TCP_V4", "port=%d", OAL_NTOH_16(th->dest));
        }
#endif
    }else if(iph->protocol == IPPROTO_ICMP){
        icmph = (struct icmphdr *)((oal_uint8*)iph + iphdr_len);
        OAL_IO_PRINT(WIFI_WAKESRC_TAG"ICMP packet, type(%d):%s, code:%d.\n", icmph->type, ((icmph->type == 0)?"ping reply":((icmph->type == 8)?"ping request":"other icmp pkt")), icmph->code);
#ifdef CONFIG_HUAWEI_DUBAI
        if (BETA_USER == get_logusertype_flag()) {
            HWDUBAI_LOGE("DUBAI_TAG_PACKET_WAKEUP", "protocol=%d", (int32_t)iph->protocol);
        }
#endif
    }else if(iph->protocol == IPPROTO_IGMP){
        OAL_IO_PRINT(WIFI_WAKESRC_TAG"IGMP packet.\n");
#ifdef CONFIG_HUAWEI_DUBAI
        if (BETA_USER == get_logusertype_flag()) {
            HWDUBAI_LOGE("DUBAI_TAG_PACKET_WAKEUP", "protocol=%d", (int32_t)iph->protocol);
        }
#endif
    }else{
        OAL_IO_PRINT(WIFI_WAKESRC_TAG"other IPv4 packet.\n");
#ifdef CONFIG_HUAWEI_DUBAI
        if (BETA_USER == get_logusertype_flag()) {
            HWDUBAI_LOGE("DUBAI_TAG_PACKET_WAKEUP", "protocol=%d", (int32_t)iph->protocol);
        }
#endif
    }

    return;
}



OAL_STATIC oal_void hmac_parse_ipv6_packet(oal_void *pst_eth)
{
    struct ipv6hdr *ipv6h;

    OAL_IO_PRINT(WIFI_WAKESRC_TAG"ipv6 packet.\n");
    ipv6h = (struct ipv6hdr *)((mac_ether_header_stru *)pst_eth + 1);
    OAL_IO_PRINT(WIFI_WAKESRC_TAG"version: %d, payload length: %d, nh->nexthdr: %d. \n", ipv6h->version, OAL_NTOH_16(ipv6h->payload_len), ipv6h->nexthdr);
    OAL_IO_PRINT(WIFI_WAKESRC_TAG"ipv6 src addr:%04x:x:x:x:x:x:x:%04x \n",IPADDR6(ipv6h->saddr));
    OAL_IO_PRINT(WIFI_WAKESRC_TAG"ipv6 dst addr:%04x:x:x:x:x:x:x:%04x \n",IPADDR6(ipv6h->daddr));
    if(OAL_IPPROTO_ICMPV6==ipv6h->nexthdr)
    {
        oal_nd_msg_stru  *pst_rx_nd_hdr;
        pst_rx_nd_hdr   = (oal_nd_msg_stru *)(ipv6h + 1);
        OAL_IO_PRINT(WIFI_WAKESRC_TAG"ipv6 nd type: %d. \n", pst_rx_nd_hdr->icmph.icmp6_type);
    }
#ifdef CONFIG_HUAWEI_DUBAI
    if (BETA_USER == get_logusertype_flag()) {
        HWDUBAI_LOGE("DUBAI_TAG_PACKET_WAKEUP", "protocol=%d", IPPROTO_IPV6);
    }
#endif

    return;
}


OAL_STATIC oal_void hmac_parse_arp_packet(oal_void *pst_eth)
{
    const struct iphdr *iph;
    int iphdr_len = 0;
    struct arphdr *arp;

    iph = (struct iphdr *)((mac_ether_header_stru *)pst_eth + 1);
    iphdr_len = iph->ihl*4;
    arp = (struct arphdr *)((oal_uint8*)iph + iphdr_len);
    OAL_IO_PRINT(WIFI_WAKESRC_TAG"ARP packet, hardware type:%d, protocol type:%d, opcode:%d.\n",
                OAL_NTOH_16(arp->ar_hrd), OAL_NTOH_16(arp->ar_pro), OAL_NTOH_16(arp->ar_op));

    return;
}


OAL_STATIC oal_void  hmac_parse_8021x_packet(oal_void *pst_eth)
{
    struct ieee8021x_hdr *hdr = (struct ieee8021x_hdr *)((mac_ether_header_stru *)pst_eth + 1);

    OAL_IO_PRINT(WIFI_WAKESRC_TAG"802.1x frame: version:%d, type:%d, length:%d\n", hdr->version, hdr->type, OAL_NTOH_16(hdr->length));

    return;
}



oal_void hmac_parse_packet(oal_netbuf_stru *pst_netbuf_eth)
{
    oal_uint16 us_type;
    mac_ether_header_stru  *pst_ether_hdr;

    pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf_eth);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_ether_hdr))
    {
        OAL_IO_PRINT(WIFI_WAKESRC_TAG"ether header is null.\n");
        return;
    }

    us_type = pst_ether_hdr->us_ether_type;

    if(us_type == OAL_HOST2NET_SHORT(ETHER_TYPE_IP)){
        hmac_parse_ipv4_packet((oal_void*)pst_ether_hdr);
    }else if (us_type == OAL_HOST2NET_SHORT(ETHER_TYPE_IPV6)){
        hmac_parse_ipv6_packet((oal_void*)pst_ether_hdr);
    }else if(us_type == OAL_HOST2NET_SHORT(ETHER_TYPE_ARP)){
        hmac_parse_arp_packet((oal_void*)pst_ether_hdr);
    }else if(us_type == OAL_HOST2NET_SHORT(ETHER_TYPE_PAE)){
        hmac_parse_8021x_packet((oal_void*)pst_ether_hdr);
    }else{
        OAL_IO_PRINT(WIFI_WAKESRC_TAG"receive protocol type:0x%04x\n", OAL_NTOH_16(us_type));
    }

    return;
}

#endif


OAL_STATIC oal_void  hmac_rx_transmit_msdu_to_lan(hmac_vap_stru *pst_vap, hmac_user_stru *pst_hmac_user, dmac_msdu_stru *pst_msdu)
{
    oal_net_device_stru    *pst_device;
    oal_netbuf_stru        *pst_netbuf;
    mac_ether_header_stru  *pst_ether_hdr;

#if defined(_PRE_WLAN_FEATURE_WPA) || defined(_PRE_WLAN_FEATURE_WPA2)
    oal_uint8              *puc_mac_addr;
#endif
    mac_vap_stru           *pst_mac_vap       = &(pst_vap->st_vap_base_info);
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    hmac_user_stru         *pst_hmac_user_st  = OAL_PTR_NULL;
    mac_ip_header_stru     *pst_ip            = OAL_PTR_NULL;
    oal_uint16              us_assoc_id       = 0xffff;
#endif

#if (defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT))
    dma_addr_t              ul_dma_addr;
    oal_int32               l_ret             = 0;
#endif

    /* ��ȡnetbuf����netbuf��dataָ���Ѿ�ָ��payload�� */
    pst_netbuf = pst_msdu->pst_netbuf;

    OAL_NETBUF_PREV(pst_netbuf) = OAL_PTR_NULL;
    OAL_NETBUF_NEXT(pst_netbuf) = OAL_PTR_NULL;

    hmac_rx_frame_80211_to_eth(pst_netbuf, pst_msdu->auc_da, pst_msdu->auc_sa);

    pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_ether_hdr))
    {
        oal_netbuf_free(pst_netbuf);
        OAM_ERROR_LOG0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX, "{hmac_rx_transmit_msdu_to_lan::pst_ether_hdr null.}");
        return;
    }

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if(OAL_TRUE==wlan_pm_wkup_src_debug_get())
    {
        OAL_IO_PRINT(WIFI_WAKESRC_TAG"rx: hmac_parse_packet_etc!\n");
        hmac_parse_packet(pst_netbuf);
        wlan_pm_wkup_src_debug_set(OAL_FALSE);
    }
#endif

#if defined(_PRE_WLAN_FEATURE_WPA) || defined(_PRE_WLAN_FEATURE_WPA2)
    puc_mac_addr = pst_msdu->auc_ta;
    if (OAL_SUCC != hmac_11i_ether_type_filter(pst_vap, &pst_hmac_user->st_user_base_info, pst_ether_hdr->us_ether_type))
    {
        /* ���հ�ȫ���ݹ��� */
        oam_report_eth_frame(puc_mac_addr, (oal_uint8*)pst_ether_hdr, (oal_uint16)OAL_NETBUF_LEN(pst_netbuf), OAM_OTA_FRAME_DIRECTION_TYPE_RX);

        oal_netbuf_free(pst_netbuf);
        OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_portvalid_check_fail_dropped, 1);
        return;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_MCAST
    if (OAL_PTR_NULL != pst_vap->pst_m2u)
    {
        hmac_m2u_snoop_inspecting(pst_vap, pst_hmac_user, pst_netbuf);
    }
#endif

    /* ��ȡnet device hmac������ʱ����Ҫ��¼netdeviceָ�� */
    pst_device      = pst_vap->pst_net_device;

    /* ��protocolģʽ��ֵ */
    OAL_NETBUF_PROTOCOL(pst_netbuf) = oal_eth_type_trans(pst_netbuf, pst_device);

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    if (mac_vap_is_msta(pst_mac_vap) || mac_vap_is_vsta(pst_mac_vap))
    {
        if (OAL_SUCC != hmac_psta_rx_process(pst_netbuf, pst_vap) ||  OAL_SUCC != hmac_psta_rx_mat(pst_netbuf, pst_vap))
        {
            oal_netbuf_free(pst_netbuf);
            return;
        }
    }
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    if(OAL_FALSE == ETHER_IS_MULTICAST(pst_msdu->auc_da))
    {
        oal_atomic_inc(&(pst_hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_arp_req_process.ul_rx_unicast_pkt_to_lan));
    }
#endif

    /* ��Ϣͳ����֡�ϱ����� */
    /* ����ͳ����Ϣ */
    HMAC_VAP_DFT_STATS_PKT_INCR(pst_vap->st_query_stats.ul_rx_pkt_to_lan,1);
    HMAC_VAP_DFT_STATS_PKT_INCR(pst_vap->st_query_stats.ul_rx_bytes_to_lan,OAL_NETBUF_LEN(pst_netbuf));
    OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_pkt_to_lan, 1); /* ���ӷ���LAN��֡����Ŀ */
    OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_bytes_to_lan, OAL_NETBUF_LEN(pst_netbuf)); /* ���ӷ���LAN���ֽ��� */

#ifdef _PRE_WLAN_DFT_DUMP_FRAME
    //hmac_rx_report_eth_frame(&pst_vap->st_vap_base_info, pst_netbuf);
#endif

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    if ((WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode) && (OAL_TRUE == pst_vap->uc_edca_opt_flag_ap))
    {
        /*lint -e778*/
        if (OAL_HOST2NET_SHORT(ETHER_TYPE_IP) == pst_ether_hdr->us_ether_type)
        {
            if (OAL_SUCC != mac_vap_find_user_by_macaddr(pst_mac_vap, pst_ether_hdr->auc_ether_shost, &us_assoc_id))
            {
                OAM_WARNING_LOG4(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2U, "{hmac_rx_transmit_msdu_to_lan::find_user_by_macaddr[%02x:XX:XX:%02x:%02x:%02x]failed}",
                               (oal_uint32)(pst_ether_hdr->auc_ether_shost[0]),
                               (oal_uint32)(pst_ether_hdr->auc_ether_shost[3]),
                               (oal_uint32)(pst_ether_hdr->auc_ether_shost[4]),
                               (oal_uint32)(pst_ether_hdr->auc_ether_shost[5]));
                oal_netbuf_free(pst_netbuf);
                return;
            }
            pst_hmac_user_st = (hmac_user_stru *)mac_res_get_hmac_user(us_assoc_id);
            if (OAL_PTR_NULL == pst_hmac_user_st)
            {
                OAM_ERROR_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX, "{hmac_rx_transmit_msdu_to_lan::mac_res_get_hmac_user fail. assoc_id: %u}", us_assoc_id);
                oal_netbuf_free(pst_netbuf);
                return;
            }

            pst_ip = (mac_ip_header_stru *)(pst_ether_hdr + 1);

            /* mips�Ż�:�������ҵ��ͳ�����ܲ�10M���� */
            if (((MAC_UDP_PROTOCAL == pst_ip->uc_protocol) && (pst_hmac_user_st->aaul_txrx_data_stat[WLAN_WME_AC_BE][WLAN_RX_UDP_DATA] < (HMAC_EDCA_OPT_PKT_NUM + 10)))
                || ((MAC_TCP_PROTOCAL == pst_ip->uc_protocol) && (pst_hmac_user_st->aaul_txrx_data_stat[WLAN_WME_AC_BE][WLAN_RX_TCP_DATA] < (HMAC_EDCA_OPT_PKT_NUM + 10))))
            {
                hmac_edca_opt_rx_pkts_stat(us_assoc_id, WLAN_TIDNO_BEST_EFFORT, pst_ip);
            }
        }
        /*lint +e778*/
    }
#endif

    OAL_MEM_NETBUF_TRACE(pst_netbuf, OAL_TRUE);
    OAL_MEMZERO(OAL_NETBUF_CB(pst_netbuf), OAL_NETBUF_CB_SIZE());

#ifdef _PRE_WLAN_FEATURE_PKT_MEM_OPT
    hmac_pkt_mem_opt_rx_pkts_stat(pst_vap, (oal_ip_header_stru*)(pst_ether_hdr + 1));
#endif
    OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_NETBUF_FOR_KERNEL);

    /* ��skbת������ */
    if(OAL_TRUE == hmac_get_rxthread_enable())
    {
        hmac_rxdata_netbuf_enqueue(pst_netbuf);

        hmac_rxdata_sched();
    }
    else
    {
        oal_netif_rx_ni(pst_netbuf);
    }

    /* ��λnet_dev->jiffies���� */
    OAL_NETDEVICE_LAST_RX(pst_device) = OAL_TIME_JIFFY;

}


oal_void  hmac_rx_lan_frame_classify(
                hmac_vap_stru              *pst_vap,
                oal_netbuf_stru            *pst_netbuf,
                mac_ieee80211_frame_stru   *pst_frame_hdr)
{
    hmac_rx_ctl_stru                   *pst_rx_ctrl;                        /* ָ��MPDU���ƿ���Ϣ��ָ�� */
    dmac_msdu_stru                      st_msdu;                            /* �������������ÿһ��MSDU */
    mac_msdu_proc_status_enum_uint8     en_process_state = MAC_PROC_BUTT;   /* ����AMSDU��״̬ */
    dmac_msdu_proc_state_stru           st_msdu_state    = {0};             /* ��¼MPDU�Ĵ�����Ϣ */
    oal_uint8                          *puc_addr         = OAL_PTR_NULL;
    oal_uint32                          ul_ret;
    hmac_user_stru                     *pst_hmac_user;
    oal_uint8                           uc_datatype = MAC_DATA_BUTT;
#ifdef _PRE_WLAN_FEATURE_WAPI
    hmac_wapi_stru                     *pst_wapi;
    oal_bool_enum_uint8                 en_is_mcast = OAL_FALSE;
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    frw_event_hdr_stru                  st_event_hdr;
    oal_netbuf_head_stru                st_w2w_netbuf_hdr;
    oal_uint16                          us_user_dix = MAC_INVALID_USER_ID;
#endif

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_vap) || (OAL_PTR_NULL == pst_netbuf) || (OAL_PTR_NULL == pst_frame_hdr)))
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_lan_frame_classify::params null.}");
        return;
    }

    OAL_MEMZERO(&st_msdu, OAL_SIZEOF(dmac_msdu_stru));

    mac_get_transmit_addr(pst_frame_hdr, &puc_addr);

    oal_set_mac_addr(st_msdu.auc_ta, puc_addr);

    /* ��ȡ��MPDU�Ŀ�����Ϣ */
    pst_rx_ctrl = (hmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);



    pst_hmac_user = (hmac_user_stru*)mac_res_get_hmac_user(pst_rx_ctrl->st_rx_info.us_ta_user_idx);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hmac_user))
    {
        OAM_ERROR_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                       "{hmac_rx_lan_frame_classify::pst_hmac_user null, user_idx=%d.}",
                       pst_rx_ctrl->st_rx_info.us_ta_user_idx);

        /* ��ӡ��net buf�����Ϣ */
        OAM_ERROR_LOG4(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                       "{hmac_rx_lan_frame_classify::info in cb, vap id=%d mac_hdr_len=%d, us_frame_len=%d mac_hdr_start_addr=0x%08x.}",
                       pst_rx_ctrl->st_rx_info.bit_vap_id,
                       pst_rx_ctrl->st_rx_info.uc_mac_header_len,
                       pst_rx_ctrl->st_rx_info.us_frame_len,
                       (oal_uint)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr);
        OAM_ERROR_LOG2(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                       "{hmac_rx_lan_frame_classify::net_buf ptr addr=0x%08x, cb ptr addr=0x%08x.}",
                       (oal_uint)pst_netbuf, (oal_uint)pst_rx_ctrl);
#ifdef _PRE_WLAN_DFT_DUMP_FRAME
        mac_rx_report_80211_frame((oal_uint8*) & (pst_vap->st_vap_base_info),
                                  (oal_uint8*) & (pst_rx_ctrl->st_rx_info),
                                  pst_netbuf,
                                  OAM_OTA_TYPE_RX_HMAC_CB);
#endif

        return;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* offload ��amsdu֡���ֳܷɶ��seq��ͬ��netbuf�ϱ� ֻ����last amsdu buffer������λ
    �������׵��������򻺳��������ź���ǿ���ƴ����յ�ba start֮ǰseq��amsdu֡��֡
    */
    if ((OAL_FALSE == pst_rx_ctrl->st_rx_info.bit_amsdu_enable) || (OAL_TRUE == pst_rx_ctrl->st_rx_info.bit_is_last_buffer))
    {
        hmac_ba_update_rx_bitmap(pst_hmac_user, pst_frame_hdr);
    }

    oal_netbuf_list_head_init(&st_w2w_netbuf_hdr);
#else
    hmac_ba_update_rx_bitmap(pst_hmac_user, pst_frame_hdr);
#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
    proc_sniffer_write_file(NULL, 0, (oal_uint8 *)oal_netbuf_payload(pst_netbuf), pst_rx_ctrl->st_rx_info.us_frame_len, 0);
#endif

    /* ���һ:����AMSDU�ۺϣ����MPDU��Ӧһ��MSDU��ͬʱ��Ӧһ��NETBUF */
    if (OAL_FALSE == pst_rx_ctrl->st_rx_info.bit_amsdu_enable)
    {
#ifdef _PRE_WLAN_FEATURE_WAPI
/*lint -e730*/
        en_is_mcast = ETHER_IS_MULTICAST(pst_frame_hdr->auc_address1);

        pst_wapi = hmac_user_get_wapi_ptr(&pst_vap->st_vap_base_info,
                                                        !en_is_mcast,
                                                        pst_hmac_user->st_user_base_info.us_assoc_id);
/*lint +e730*/
        if (OAL_PTR_NULL == pst_wapi)
        {
            OAM_WARNING_LOG0(0, OAM_SF_WPA, "{hmac_rx_lan_frame_classify:: get pst_wapi Err!.}");
            HMAC_USER_STATS_PKT_INCR(pst_hmac_user->ul_rx_pkt_drop, 1);
            return ;
        }

        if ((OAL_TRUE == WAPI_IS_PORT_VALID(pst_wapi))
            && (OAL_PTR_NULL != pst_wapi->wapi_netbuff_rxhandle))
        {
            pst_netbuf = pst_wapi->wapi_netbuff_rxhandle(pst_wapi, pst_netbuf);
            if (OAL_PTR_NULL == pst_netbuf)
            {
                OAM_WARNING_LOG0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX, "{hmac_rx_lan_frame_classify:: wapi decrypt FAIL!}");
                HMAC_USER_STATS_PKT_INCR(pst_hmac_user->ul_rx_pkt_drop, 1);
                return ;
            }

            /* ���»�ȡ��MPDU�Ŀ�����Ϣ */
            pst_rx_ctrl = (hmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
        }
#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */

        pst_netbuf = hmac_defrag_process(pst_hmac_user, pst_netbuf, pst_rx_ctrl->st_rx_info.uc_mac_header_len);
        if (OAL_PTR_NULL == pst_netbuf)
        {
            return;
        }

        /* ���»�ȡ��MPDU�Ŀ�����Ϣ */
        pst_rx_ctrl = (hmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
        pst_frame_hdr = (mac_ieee80211_frame_stru *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr;

        /* ��ӡ���ؼ�֡(dhcp)��Ϣ */
        uc_datatype = pst_rx_ctrl->st_rx_info.bit_data_frame_type;
        if ((OAL_IS_VIP_FRAME(uc_datatype)) && (uc_datatype != MAC_DATA_ARP_REQ))
        {
            OAM_WARNING_LOG4(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX, "{hmac_rx_lan_frame_classify::user[%d], datatype==%u, len==%u, rx_drop_cnt==%u}[1:dhcp 2:arp_req 3:arp_rsp 4:eapol]",
                        pst_rx_ctrl->st_rx_info.us_ta_user_idx,
                        uc_datatype,
                        pst_rx_ctrl->st_rx_info.us_frame_len,
                        pst_hmac_user->ul_rx_pkt_drop);

        }

        /* �Ե�ǰ��msdu���и�ֵ */
        st_msdu.pst_netbuf    = pst_netbuf;

        /* ��ȡԴ��ַ��Ŀ�ĵ�ַ */
        mac_rx_get_sa(pst_frame_hdr, &puc_addr);
        oal_set_mac_addr(st_msdu.auc_sa, puc_addr);

        mac_rx_get_da(pst_frame_hdr, &puc_addr);
        oal_set_mac_addr(st_msdu.auc_da, puc_addr);

        /* ��netbuf��dataָ��ָ��mac frame��payload�� */
        oal_netbuf_pull(pst_netbuf, pst_rx_ctrl->st_rx_info.uc_mac_header_len);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        if (WLAN_VAP_MODE_BSS_AP == pst_vap->st_vap_base_info.en_vap_mode && OAL_SUCC == mac_vap_find_user_by_macaddr(&pst_vap->st_vap_base_info, st_msdu.auc_da, &us_user_dix))
        {
            /* ��MSDUת��Ϊ��̫����ʽ��֡ */
            hmac_rx_frame_80211_to_eth(st_msdu.pst_netbuf, st_msdu.auc_da, st_msdu.auc_sa);
            /* ��MSDU���뵽netbuf������� */
            oal_netbuf_add_to_list_tail(st_msdu.pst_netbuf, &st_w2w_netbuf_hdr);
        }
        else
#endif
        {
            OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_PREPARE_MSDU_INFO);
            /* ��MSDUת����LAN */
            hmac_rx_transmit_msdu_to_lan(pst_vap, pst_hmac_user, &st_msdu);
        }
    }
    /* �����:AMSDU�ۺ� */
    else
    {
        st_msdu_state.uc_procd_netbuf_nums    = 0;
        st_msdu_state.uc_procd_msdu_in_netbuf = 0;

        /* amsdu ���һ��netbuf nextָ����Ϊ NULL ����ʱ�����ͷ�amsdu netbuf */
        hmac_rx_clear_amsdu_last_netbuf_pointer(pst_netbuf, pst_rx_ctrl->st_rx_info.bit_buff_nums);

        do
        {
            /* ��ȡ��һ��Ҫת����msdu */
            ul_ret = hmac_rx_parse_amsdu(pst_netbuf, &st_msdu, &st_msdu_state, &en_process_state);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                                 "{hmac_rx_lan_frame_classify::hmac_rx_parse_amsdu failed[%d].}", ul_ret);
                return;
            }

            OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_PREPARE_MSDU_INFO);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            if (WLAN_VAP_MODE_BSS_AP == pst_vap->st_vap_base_info.en_vap_mode && OAL_SUCC == mac_vap_find_user_by_macaddr(&pst_vap->st_vap_base_info, st_msdu.auc_da, &us_user_dix))
            {
                /* ��MSDUת��Ϊ��̫����ʽ��֡ */
                hmac_rx_frame_80211_to_eth(st_msdu.pst_netbuf, st_msdu.auc_da, st_msdu.auc_sa);
                /* ��MSDU���뵽netbuf������� */
                oal_netbuf_add_to_list_tail(st_msdu.pst_netbuf, &st_w2w_netbuf_hdr);
            }
            else
#endif
            {
                /* ��ÿһ��MSDUת����LAN */
                hmac_rx_transmit_msdu_to_lan(pst_vap, pst_hmac_user, &st_msdu);
            }
        }while (MAC_PROC_LAST_MSDU != en_process_state);
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (WLAN_VAP_MODE_BSS_AP == pst_vap->st_vap_base_info.en_vap_mode)
    {
        OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_TO_LAN);
        /*  ��MSDU�������������̴��� */
        if (OAL_FALSE == oal_netbuf_list_empty(&st_w2w_netbuf_hdr) && OAL_PTR_NULL != oal_netbuf_tail(&st_w2w_netbuf_hdr) &&
            OAL_PTR_NULL != oal_netbuf_peek(&st_w2w_netbuf_hdr))
        {
            st_event_hdr.uc_chip_id = pst_vap->st_vap_base_info.uc_chip_id;
            st_event_hdr.uc_device_id = pst_vap->st_vap_base_info.uc_device_id;
            st_event_hdr.uc_vap_id = pst_vap->st_vap_base_info.uc_vap_id;

            OAL_NETBUF_NEXT((oal_netbuf_tail(&st_w2w_netbuf_hdr))) = OAL_PTR_NULL;
            OAL_NETBUF_PREV((oal_netbuf_peek(&st_w2w_netbuf_hdr))) = OAL_PTR_NULL;

            hmac_rx_transmit_to_wlan(&st_event_hdr, &st_w2w_netbuf_hdr);
        }
        OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_END);
    }
#endif
}


oal_uint32  hmac_rx_copy_netbuff(oal_netbuf_stru  **ppst_dest_netbuf, oal_netbuf_stru  *pst_src_netbuf, oal_uint8 uc_vap_id, mac_ieee80211_frame_stru **ppul_mac_hdr_start_addr)
{
    hmac_rx_ctl_stru  *pst_rx_ctrl;

    if (OAL_PTR_NULL == pst_src_netbuf)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    *ppst_dest_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (OAL_UNLIKELY(OAL_PTR_NULL == *ppst_dest_netbuf))
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_RX, "{hmac_rx_copy_netbuff::pst_netbuf_copy null.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* ��Ϣ���� */
    oal_memcopy(oal_netbuf_cb(*ppst_dest_netbuf), oal_netbuf_cb(pst_src_netbuf), OAL_SIZEOF(hmac_rx_ctl_stru)); //modify src bug
    oal_memcopy(oal_netbuf_data(*ppst_dest_netbuf), oal_netbuf_data(pst_src_netbuf), OAL_NETBUF_LEN(pst_src_netbuf));

    /* ����netbuf���ȡ�TAILָ�� */
    oal_netbuf_put(*ppst_dest_netbuf, oal_netbuf_get_len(pst_src_netbuf));

    /* ����MAC֡ͷ��ָ��copy�󣬶�Ӧ��mac header��ͷ�Ѿ������仯) */
    pst_rx_ctrl = (hmac_rx_ctl_stru *)oal_netbuf_cb(*ppst_dest_netbuf);
    pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr = (oal_uint32 *)oal_netbuf_data(*ppst_dest_netbuf);
    *ppul_mac_hdr_start_addr = (mac_ieee80211_frame_stru *)oal_netbuf_data(*ppst_dest_netbuf);

    return OAL_SUCC;
}


oal_void  hmac_rx_process_data_filter(oal_netbuf_head_stru *pst_netbuf_header, oal_netbuf_stru *pst_temp_netbuf, oal_uint16 us_netbuf_num)
{
    oal_netbuf_stru                    *pst_netbuf;
    hmac_rx_ctl_stru                   *pst_rx_ctrl;
#ifdef _PRE_WLAN_FEATURE_AMPDU
    hmac_user_stru                     *pst_hmac_user;
    mac_ieee80211_frame_stru           *pst_frame_hdr;
#endif
    oal_uint8                           uc_buf_nums;
    mac_vap_stru                       *pst_vap;
    oal_uint32                          ul_ret = OAL_SUCC;
    oal_bool_enum_uint8                 en_is_ba_buf;
    oal_uint8                           uc_netbuf_num;

    while (0 != us_netbuf_num)
    {
        en_is_ba_buf = OAL_FALSE;
        pst_netbuf  = pst_temp_netbuf;
        if (OAL_PTR_NULL == pst_netbuf)
        {
            OAM_WARNING_LOG1(0, OAM_SF_RX, "{hmac_rx_process_data_filter::us_netbuf_num = %d}",us_netbuf_num);
            break;
        }

        pst_rx_ctrl   = (hmac_rx_ctl_stru*)oal_netbuf_cb(pst_netbuf);

        uc_buf_nums   = pst_rx_ctrl->st_rx_info.bit_buff_nums;

        /* ��ȡ��һ��Ҫ�����MPDU */
        oal_netbuf_get_appointed_netbuf(pst_netbuf, uc_buf_nums, &pst_temp_netbuf);
        us_netbuf_num = OAL_SUB(us_netbuf_num, uc_buf_nums);

        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_rx_ctrl->st_rx_info.uc_mac_vap_id);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_vap))
        {
            hmac_rx_free_netbuf_list(pst_netbuf_header, uc_buf_nums);
            OAM_WARNING_LOG0(pst_rx_ctrl->st_rx_info.bit_vap_id, OAM_SF_RX, "{hmac_rx_process_data_filter::pst_vap null.}");
            continue;
        }

        //if (0 == pst_vap->uc_vap_id || WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT < pst_vap->uc_vap_id)
        /* ˫оƬ�£�0��1��������vap id�����������Ҫ����ҵ��vap ��ʵid���������vap mac numֵ�����ж� */
        if (WLAN_SERVICE_VAP_START_ID_PER_BOARD > pst_vap->uc_vap_id || WLAN_VAP_SUPPORT_MAX_NUM_LIMIT < pst_vap->uc_vap_id)
        {
            OAM_ERROR_LOG1(0, OAM_SF_RX, "{hmac_rx_process_data_filter::Invalid vap_id.vap_id[%u]}",pst_vap->uc_vap_id);
            hmac_rx_free_netbuf_list(pst_netbuf_header, uc_buf_nums);
            continue;
        }


#ifdef _PRE_WLAN_FEATURE_AMPDU
        pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(MAC_GET_RX_CB_TA_USER_IDX(&(pst_rx_ctrl->st_rx_info))); //make sure ta user idx is exist

        pst_frame_hdr = (mac_ieee80211_frame_stru *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr;

        if (OAL_PTR_NULL == pst_hmac_user)
        {
            OAM_WARNING_LOG1(0, OAM_SF_RX,
                           "{hmac_rx_process_data_filter::pst_hmac_user is null.index[%d]}", MAC_GET_RX_CB_TA_USER_IDX(&(pst_rx_ctrl->st_rx_info)));
            hmac_rx_free_netbuf_list(pst_netbuf_header, uc_buf_nums);
            continue;
        }

        ul_ret = hmac_ba_filter_serv(pst_vap, pst_hmac_user, pst_rx_ctrl, pst_frame_hdr, pst_netbuf_header, &en_is_ba_buf);


        if (OAL_SUCC != ul_ret)
        {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            //OAM_WARNING_LOG0(pst_rx_ctrl->st_rx_info.bit_vap_id, OAM_SF_RX, "{hmac_rx_process_data_filter::hmac_ba_filter_serv proc error.}");
#endif /* 1151��ʱע�͵�����ӡ */
            hmac_rx_free_netbuf_list(pst_netbuf_header, uc_buf_nums);
            continue;
        }
#endif

        if (OAL_TRUE == en_is_ba_buf)
        {
            continue;
        }

        /*�����buff��reorder���У������¹ҵ�����β������*/
        for (uc_netbuf_num = 0; uc_netbuf_num < uc_buf_nums; uc_netbuf_num++)
        {
            pst_netbuf = oal_netbuf_delist_nolock(pst_netbuf_header);

            if (OAL_LIKELY(OAL_PTR_NULL != pst_netbuf))
            {
                oal_netbuf_list_tail_nolock(pst_netbuf_header, pst_netbuf);
            }
            else
            {
                OAM_WARNING_LOG0(pst_rx_ctrl->st_rx_info.bit_vap_id, OAM_SF_RX, "{hmac_rx_process_data_filter::no buff error.}");
            }
        }
        OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_NON_REORDER_BACK);
    }

}

#ifdef _PRE_WLAN_TCP_OPT
OAL_STATIC  oal_bool_enum_uint8 hmac_transfer_rx_handler(hmac_device_stru* pst_hmac_device,hmac_vap_stru * hmac_vap,oal_netbuf_stru* netbuf)
{
#ifndef WIN32
    hmac_rx_ctl_stru                   *pst_rx_ctrl;                        /* ָ��MPDU���ƿ���Ϣ��ָ�� */
    oal_netbuf_stru* pst_mac_llc_snap_netbuf;

#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
    if(OAL_TRUE == pst_hmac_device->sys_tcp_rx_ack_opt_enable)
    {
        pst_rx_ctrl = (hmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        pst_mac_llc_snap_netbuf = (oal_netbuf_stru*)(netbuf->data + pst_rx_ctrl->st_rx_info.uc_mac_header_len);
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        OAM_WARNING_LOG1(0, OAM_SF_TX,
                             "{hmac_transfer_rx_handler::uc_mac_header_len = %d}\r\n",pst_rx_ctrl->st_rx_info.uc_mac_header_len);
#endif
        if(OAL_TRUE == hmac_judge_rx_netbuf_classify(pst_mac_llc_snap_netbuf))
        {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            OAM_WARNING_LOG0(0, OAM_SF_TX,
                                 "{hmac_transfer_rx_handler::netbuf is tcp ack.}\r\n");
#endif
            oal_spin_lock_bh(&hmac_vap->ast_hmac_tcp_ack[HCC_RX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
            oal_netbuf_list_tail(&hmac_vap->ast_hmac_tcp_ack[HCC_RX].data_queue[HMAC_TCP_ACK_QUEUE],netbuf);
            oal_spin_unlock_bh(&hmac_vap->ast_hmac_tcp_ack[HCC_RX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
            hmac_sched_transfer();
            return OAL_TRUE;
        }
    }
#endif
#endif
    return OAL_FALSE;
}

#endif


oal_uint32  hmac_rx_lan_frame(oal_netbuf_head_stru *pst_netbuf_header)
{
    oal_uint32                 ul_netbuf_num;
    oal_netbuf_stru           *pst_temp_netbuf;
    oal_netbuf_stru           *pst_netbuf;
    oal_uint8                  uc_buf_nums;
    hmac_rx_ctl_stru          *pst_rx_ctrl;
    mac_ieee80211_frame_stru  *pst_frame_hdr;
    hmac_vap_stru             *pst_vap;

    ul_netbuf_num   = oal_netbuf_get_buf_num(pst_netbuf_header);
    pst_temp_netbuf = oal_netbuf_peek(pst_netbuf_header);

    //OAM_INFO_LOG1(0, OAM_SF_RX, "{hmac_rx_lan_frame::prepare %d netbuf up to netdevice.}", ul_netbuf_num);

    while (0 != ul_netbuf_num)
    {
        pst_netbuf = pst_temp_netbuf;
        if(NULL == pst_netbuf)
        {
            break;
        }

        pst_rx_ctrl   = (hmac_rx_ctl_stru*)oal_netbuf_cb(pst_netbuf);
        pst_frame_hdr = (mac_ieee80211_frame_stru *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr;
        uc_buf_nums = pst_rx_ctrl->st_rx_info.bit_buff_nums;

        ul_netbuf_num = OAL_SUB(ul_netbuf_num, uc_buf_nums);
        oal_netbuf_get_appointed_netbuf(pst_netbuf, uc_buf_nums, &pst_temp_netbuf);

        pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_rx_ctrl->st_rx_info.uc_mac_vap_id);
        if (OAL_PTR_NULL == pst_vap)
        {
            OAM_ERROR_LOG1(0, OAM_SF_RX, "{hmac_rx_lan_frame::mac_res_get_hmac_vap null. vap_id:%u}",pst_rx_ctrl->st_rx_info.uc_mac_vap_id);
            continue;
        }

        hmac_rx_lan_frame_classify(pst_vap, pst_netbuf, pst_frame_hdr);
    }

    return OAL_SUCC;
}

oal_uint32  hmac_rx_process_data_ap(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                     *pst_event;
    frw_event_hdr_stru                 *pst_event_hdr;
    dmac_wlan_drx_event_stru           *pst_wlan_rx_event;
    oal_netbuf_stru                    *pst_netbuf;                     /* ���ڱ��浱ǰ�����MPDU�ĵ�һ��netbufָ�� */
    oal_netbuf_stru                    *pst_temp_netbuf;                /* ������ʱ������һ����Ҫ�����netbufָ�� */
    oal_uint16                          us_netbuf_num;                  /* netbuf����ĸ��� */
    oal_netbuf_head_stru                st_netbuf_header;               /* �洢�ϱ������������� */
    hmac_vap_stru                      *pst_hmac_vap;
#ifdef _PRE_WLAN_TCP_OPT
    oal_netbuf_head_stru                st_temp_header;
    hmac_device_stru                   *pst_hmac_device;
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_process_data_ap::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_START);

    /* ��ȡ�¼�ͷ���¼��ṹ��ָ�� */
    pst_event           = (frw_event_stru *)pst_event_mem->puc_data;
    pst_event_hdr       = &(pst_event->st_event_hdr);
    pst_wlan_rx_event   = (dmac_wlan_drx_event_stru *)(pst_event->auc_event_data);
    pst_temp_netbuf     = pst_wlan_rx_event->pst_netbuf;
    us_netbuf_num       = pst_wlan_rx_event->us_netbuf_num;

    OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_BASE_INFO);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_temp_netbuf))
    {
        OAM_ERROR_LOG1(0, OAM_SF_RX, "{hmac_rx_process_data_ap::us_netbuf_num = %d.}",us_netbuf_num);
        return OAL_SUCC; /* ������¼���������Ϊ�˷�ֹ51��UT�ҵ� ���� true */
    }
#ifdef _PRE_WLAN_TCP_OPT
    pst_hmac_device = hmac_res_get_mac_dev(pst_event_hdr->uc_device_id);
    if (OAL_PTR_NULL == pst_hmac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_rx_process_data_ap::pst_hmac_device null.}");
        hmac_rx_free_netbuf(pst_temp_netbuf, us_netbuf_num);
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_process_data_ap::pst_hmac_vap null.}");
        hmac_rx_free_netbuf(pst_temp_netbuf, us_netbuf_num);
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /*��ʱ���mib_info ָ��Ϊ�յ����⣬
      If mib info is null ptr,release the netbuf*/
    if(OAL_PTR_NULL == pst_hmac_vap->st_vap_base_info.pst_mib_info)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_rx_process_data_ap::pst_mib_info null.}");
        hmac_rx_free_netbuf(pst_temp_netbuf, us_netbuf_num);
        return OAL_SUCC;
    }
#endif


    /*ͳ���հ�������������ȫ�ֱ���*/
    hmac_wifi_statistic_rx_packets(us_netbuf_num);


    /*������netbuffȫ��������*/
    oal_netbuf_list_head_init(&st_netbuf_header);
    while (0 != us_netbuf_num)
    {
        pst_netbuf = pst_temp_netbuf;
        if (OAL_PTR_NULL == pst_netbuf)
        {
            break;
        }

        pst_temp_netbuf = OAL_NETBUF_NEXT(pst_netbuf);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        hmac_wifi_statistic_rx_bytes(OAL_NETBUF_LEN(pst_netbuf));
#endif

        oal_netbuf_list_tail_nolock(&st_netbuf_header, pst_netbuf);
        us_netbuf_num--;

    }

    if(0 != us_netbuf_num)
    {
        OAM_ERROR_LOG2(0, OAM_SF_RX, "{hmac_rx_process_data_ap::us_netbuf_num[%d], event_buf_num[%d].}",
                        us_netbuf_num, pst_wlan_rx_event->us_netbuf_num);
    }

    /*��Dmac�ϱ���֡����reorder���й���һ��*/
    hmac_rx_process_data_filter(&st_netbuf_header, pst_wlan_rx_event->pst_netbuf, pst_wlan_rx_event->us_netbuf_num);

#ifdef _PRE_WLAN_TCP_OPT
    oal_netbuf_head_init(&st_temp_header);

    while(!!(pst_temp_netbuf = oal_netbuf_delist(&st_netbuf_header)))
    {
        if(OAL_FALSE == hmac_transfer_rx_handler(pst_hmac_device,pst_hmac_vap,pst_temp_netbuf))
        {
            oal_netbuf_list_tail(&st_temp_header,pst_temp_netbuf);
        }
    }
    /*lint -e522*/
    OAL_WARN_ON(!oal_netbuf_list_empty(&st_netbuf_header));
    /*lint +e522*/
    oal_netbuf_splice_init(&st_temp_header, &st_netbuf_header);
#endif

    hmac_rx_process_data_ap_tcp_ack_opt(pst_hmac_vap,&st_netbuf_header);
    return OAL_SUCC;
}

oal_void  hmac_rx_process_data_ap_tcp_ack_opt(hmac_vap_stru *pst_vap,oal_netbuf_head_stru* pst_netbuf_header)
{
    frw_event_hdr_stru                 st_event_hdr;
    mac_ieee80211_frame_stru           *pst_frame_hdr;                  /* ����mac֡��ָ�� */
    mac_ieee80211_frame_stru           *pst_copy_frame_hdr;             /* ����mac֡��ָ�� */
    oal_uint8                          *puc_da;                         /* �����û�Ŀ�ĵ�ַ��ָ�� */
    hmac_user_stru                     *pst_hmac_da_user;
    oal_uint32                          ul_rslt;
    oal_uint16                          us_user_dix;
    hmac_rx_ctl_stru                   *pst_rx_ctrl;                    /* ÿһ��MPDU�Ŀ�����Ϣ */
    oal_uint16                          us_netbuf_num;                  /* netbuf����ĸ��� */
    oal_uint8                           uc_buf_nums;                    /* ÿ��mpduռ��buf�ĸ��� */
    oal_netbuf_stru                    *pst_netbuf;                     /* ���ڱ��浱ǰ�����MPDU�ĵ�һ��netbufָ�� */
    oal_netbuf_stru                    *pst_temp_netbuf;                /* ������ʱ������һ����Ҫ�����netbufָ�� */
    oal_netbuf_stru                    *pst_netbuf_copy;                /* ���ڱ����鲥֡copy */
    oal_netbuf_head_stru                st_w2w_netbuf_hdr;              /* ����wlan to wlan��netbuf�����ͷ */
#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
        cs_isolation_forward_enum           en_forward;
#endif

    /* ѭ���յ���ÿһ��MPDU�����������:
        1���鲥֡ʱ������WLAN TO WLAN��WLAN TO LAN�ӿ�
        2������������ʵ�����������WLAN TO LAN�ӿڻ���WLAN TO WLAN�ӿ� */
    oal_netbuf_list_head_init(&st_w2w_netbuf_hdr);
    pst_temp_netbuf = oal_netbuf_peek(pst_netbuf_header);
    us_netbuf_num = (oal_uint16)oal_netbuf_get_buf_num(pst_netbuf_header);
    st_event_hdr.uc_chip_id = pst_vap->st_vap_base_info.uc_chip_id;
    st_event_hdr.uc_device_id = pst_vap->st_vap_base_info.uc_device_id;
    st_event_hdr.uc_vap_id = pst_vap->st_vap_base_info.uc_vap_id;

    while (0 != us_netbuf_num)
    {
        pst_netbuf  = pst_temp_netbuf;
        if (OAL_PTR_NULL == pst_netbuf)
        {
            break;
        }

        pst_rx_ctrl   = (hmac_rx_ctl_stru*)oal_netbuf_cb(pst_netbuf);

        /* ��ȡ֡ͷ��Ϣ */
        pst_frame_hdr = (mac_ieee80211_frame_stru *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr;

        /* ��ȡ��ǰMPDUռ�õ�netbuf��Ŀ */
        uc_buf_nums   = pst_rx_ctrl->st_rx_info.bit_buff_nums;

        /* ��ȡ��һ��Ҫ�����MPDU */
        oal_netbuf_get_appointed_netbuf(pst_netbuf, uc_buf_nums, &pst_temp_netbuf);
        us_netbuf_num = OAL_SUB(us_netbuf_num, uc_buf_nums);

        pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_rx_ctrl->st_rx_info.uc_mac_vap_id);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_vap))
        {
            OAM_WARNING_LOG0(pst_rx_ctrl->st_rx_info.bit_vap_id, OAM_SF_RX, "{hmac_rx_process_data_ap::pst_vap null.}");
            hmac_rx_free_netbuf(pst_netbuf, (oal_uint16)uc_buf_nums);
            continue;
        }

        /* ��ȡ���ն˵�ַ  */
        mac_rx_get_da(pst_frame_hdr, &puc_da);

        /* Ŀ�ĵ�ַΪ�鲥��ַʱ������WLAN_TO_WLAN��WLAN_TO_LAN��ת�� */
        if (ETHER_IS_MULTICAST(puc_da))
        {
            OAM_INFO_LOG0(st_event_hdr.uc_vap_id, OAM_SF_RX, "{hmac_rx_lan_frame_classify::the frame is a group frame.}");
            OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_mcast_cnt, 1);

            if (OAL_SUCC != hmac_rx_copy_netbuff(&pst_netbuf_copy, pst_netbuf, pst_rx_ctrl->st_rx_info.uc_mac_vap_id, &pst_copy_frame_hdr))
            {
                OAM_WARNING_LOG0(st_event_hdr.uc_vap_id, OAM_SF_RX, "{hmac_rx_process_data_ap::send mcast pkt to air fail.}");

                OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_no_buff_dropped, 1);
                continue;
            }

            hmac_rx_lan_frame_classify(pst_vap, pst_netbuf, pst_frame_hdr); //�ϱ������

        #ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
            pst_rx_ctrl   = (hmac_rx_ctl_stru*)oal_netbuf_cb(pst_netbuf_copy);

            /* ��ȡ֡ͷ��Ϣ */
            pst_frame_hdr = (mac_ieee80211_frame_stru *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr;
            mac_rx_get_da(pst_frame_hdr, &puc_da);

            en_forward = hmac_isolation_filter(&pst_vap->st_vap_base_info, puc_da);
            if (CS_ISOLATION_FORWORD_DROP == en_forward)
            {
                /* �ͷŵ�ǰ�����MPDUռ�õ�netbuf. 2014.7.29 cause memory leak bug fixed */
                /* OAL_IO_PRINT("isolation drop %d-%d\n",uc_netbuf_num,uc_buf_nums);1-1 */
                hmac_rx_free_netbuf(pst_netbuf_copy, (oal_uint16)uc_buf_nums);
                continue;
            }
        #endif

            /* ��MPDU�����ɵ���MSDU�������е�MSDU���һ��netbuf�� */
            hmac_rx_prepare_msdu_list_to_wlan(pst_vap, &st_w2w_netbuf_hdr, pst_netbuf_copy, pst_copy_frame_hdr);
            continue;
        }

#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
        en_forward = hmac_isolation_filter(&pst_vap->st_vap_base_info, puc_da);
        if (CS_ISOLATION_FORWORD_DROP == en_forward)
        {
            /* �ͷŵ�ǰ�����MPDUռ�õ�netbuf. 2014.7.29 cause memory leak bug fixed */
            /* OAL_IO_PRINT("isolation drop %d-%d\n",uc_netbuf_num,uc_buf_nums);1-1 */
            hmac_rx_free_netbuf(pst_netbuf, (oal_uint16)uc_buf_nums);
            /*return OAL_SUCC; bug fixed */
            continue;
        }
#endif

        /* ��ȡĿ�ĵ�ַ��Ӧ���û�ָ�� */
        ul_rslt = mac_vap_find_user_by_macaddr(&pst_vap->st_vap_base_info, puc_da, &us_user_dix);
        if (OAL_ERR_CODE_PTR_NULL == ul_rslt )  /* �����û�ʧ�� */
        {
            /* �ͷŵ�ǰ�����MPDUռ�õ�netbuf */
            hmac_rx_free_netbuf(pst_netbuf, (oal_uint16)uc_buf_nums);

            OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_da_check_dropped, 1);
            continue;
        }

        /* û���ҵ���Ӧ���û� */
        if (OAL_SUCC != ul_rslt)
        {
            OAM_INFO_LOG0(st_event_hdr.uc_vap_id, OAM_SF_RX, "{hmac_rx_lan_frame_classify::the frame is a unique frame.}");
            /* Ŀ���û�����AP���û����У�����wlan_to_lanת���ӿ� */
            hmac_rx_lan_frame_classify(pst_vap, pst_netbuf, pst_frame_hdr);
            continue;
        }

        /* Ŀ���û�����AP���û����У�����WLAN_TO_WLANת�� */
        pst_hmac_da_user = (hmac_user_stru *)mac_res_get_hmac_user(us_user_dix);

        if (OAL_PTR_NULL == pst_hmac_da_user)
        {
            OAM_WARNING_LOG1(st_event_hdr.uc_vap_id, OAM_SF_RX, "{hmac_rx_lan_frame_classify::pst_hmac_da_user[%d] null.}", us_user_dix);
            OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_da_check_dropped, 1);

            hmac_rx_free_netbuf(pst_netbuf, (oal_uint16)uc_buf_nums);
            continue;
        }

        if (MAC_USER_STATE_ASSOC != pst_hmac_da_user->st_user_base_info.en_user_asoc_state)
        {
            OAM_WARNING_LOG0(st_event_hdr.uc_vap_id, OAM_SF_RX, "{hmac_rx_lan_frame_classify::the station is not associated with ap.}");
            OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_da_check_dropped, 1);

            hmac_rx_free_netbuf(pst_netbuf, (oal_uint16)uc_buf_nums);
            hmac_mgmt_send_deauth_frame(&pst_vap->st_vap_base_info, puc_da, MAC_NOT_AUTHED, OAL_FALSE);

            continue;
        }

        /* ��MPDU�����ɵ���MSDU�������е�MSDU���һ��netbuf�� */
        hmac_rx_prepare_msdu_list_to_wlan(pst_vap, &st_w2w_netbuf_hdr, pst_netbuf, pst_frame_hdr);
    }

    OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_TO_LAN);

    /*  ��MSDU�������������̴��� */
    if (OAL_FALSE == oal_netbuf_list_empty(&st_w2w_netbuf_hdr) && OAL_PTR_NULL != oal_netbuf_tail(&st_w2w_netbuf_hdr) &&
        OAL_PTR_NULL != oal_netbuf_peek(&st_w2w_netbuf_hdr))
    {
        OAL_NETBUF_NEXT((oal_netbuf_tail(&st_w2w_netbuf_hdr))) = OAL_PTR_NULL;
        OAL_NETBUF_PREV((oal_netbuf_peek(&st_w2w_netbuf_hdr))) = OAL_PTR_NULL;

        hmac_rx_transmit_to_wlan(&st_event_hdr, &st_w2w_netbuf_hdr);
    }
    OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_TO_WLAN);

    OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_END);

}


oal_uint32  hmac_rx_process_data_sta(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                     *pst_event;
    frw_event_hdr_stru                 *pst_event_hdr;
    dmac_wlan_drx_event_stru           *pst_wlan_rx_event;
    oal_netbuf_stru                    *pst_netbuf;                /* ������ʱ������һ����Ҫ�����netbufָ�� */
    oal_uint16                          us_netbuf_num;                  /* netbuf����ĸ��� */
    oal_netbuf_head_stru                st_netbuf_header;               /* �洢�ϱ������������� */
    oal_netbuf_stru                    *pst_temp_netbuf;
    hmac_vap_stru                      *pst_hmac_vap;
#ifdef _PRE_WLAN_TCP_OPT
    oal_netbuf_head_stru                st_temp_header;
    hmac_device_stru                   *pst_hmac_device;
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_process_data_sta::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_START);
    OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_DATA_START);

    /* ��ȡ�¼�ͷ���¼��ṹ��ָ�� */
    pst_event           = (frw_event_stru *)pst_event_mem->puc_data;
    pst_event_hdr       = &(pst_event->st_event_hdr);
    pst_wlan_rx_event   = (dmac_wlan_drx_event_stru *)(pst_event->auc_event_data);
    pst_temp_netbuf     = pst_wlan_rx_event->pst_netbuf;
    us_netbuf_num       = pst_wlan_rx_event->us_netbuf_num;

    OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_BASE_INFO);

#ifdef _PRE_WLAN_TCP_OPT
    pst_hmac_device = hmac_res_get_mac_dev(pst_event_hdr->uc_device_id);
    if (OAL_PTR_NULL == pst_hmac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_rx_process_data_sta::pst_hmac_device null.}");
        hmac_rx_free_netbuf(pst_temp_netbuf, us_netbuf_num);
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_process_data_sta::pst_hmac_vap null.}");
        hmac_rx_free_netbuf(pst_temp_netbuf, us_netbuf_num);
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* If mib info is null ptr,release the netbuf */
    if(NULL == pst_hmac_vap->st_vap_base_info.pst_mib_info)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_rx_process_data_sta::pst_mib_info null.}");
        hmac_rx_free_netbuf(pst_temp_netbuf, us_netbuf_num);
        return OAL_SUCC;
    }
#endif


    /*ͳ���հ�������������ȫ�ֱ���*/
    hmac_wifi_statistic_rx_packets(us_netbuf_num);


    /*������netbuffȫ��������*/
    oal_netbuf_list_head_init(&st_netbuf_header);
    while (0 != us_netbuf_num)
    {
        pst_netbuf = pst_temp_netbuf;
        if (OAL_PTR_NULL == pst_netbuf)
        {
            break;
        }

        pst_temp_netbuf = OAL_NETBUF_NEXT(pst_netbuf);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        hmac_wifi_statistic_rx_bytes(OAL_NETBUF_LEN(pst_netbuf));
#endif

        oal_netbuf_list_tail_nolock(&st_netbuf_header, pst_netbuf);
        us_netbuf_num--;
    }

    if(0 != us_netbuf_num)
    {
        OAM_ERROR_LOG2(0, OAM_SF_RX, "{hmac_rx_process_data_sta::us_netbuf_num[%d], event_buf_num[%d].}",
                        us_netbuf_num, pst_wlan_rx_event->us_netbuf_num);
    }

    OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_GET_NETBUF_LIST);

    hmac_rx_process_data_filter(&st_netbuf_header, pst_wlan_rx_event->pst_netbuf, pst_wlan_rx_event->us_netbuf_num);

#ifdef _PRE_WLAN_TCP_OPT
    oal_netbuf_head_init(&st_temp_header);
    while(!!(pst_temp_netbuf = oal_netbuf_delist(&st_netbuf_header)))
    {
        if(OAL_FALSE == hmac_transfer_rx_handler(pst_hmac_device,pst_hmac_vap,pst_temp_netbuf))
        {
            oal_netbuf_list_tail(&st_temp_header,pst_temp_netbuf);
        }
    }
    /*lint -e522*/
    OAL_WARN_ON(!oal_netbuf_list_empty(&st_netbuf_header));
    /*lint +e522*/
    oal_netbuf_splice_init(&st_temp_header, &st_netbuf_header);
#endif
    OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_TCP_ACK_OPT);

    hmac_rx_process_data_sta_tcp_ack_opt(pst_hmac_vap,&st_netbuf_header);
    return OAL_SUCC;
}


oal_uint32  hmac_rx_process_data_sta_tcp_ack_opt(hmac_vap_stru *pst_vap,oal_netbuf_head_stru* pst_netbuf_header)
{
    /*����Ҫ�ϱ���֡��һ���Ӵ���*/
    hmac_rx_lan_frame(pst_netbuf_header);

    OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_END);
    OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_HMAC_END);
#ifdef _PRE_WLAN_PROFLING_MIPS
    oal_profiling_stop_rx_save();
#endif
    return OAL_SUCC;
}

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

