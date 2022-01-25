


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "hmac_rx_data.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_ap.h"

#ifdef _PRE_WLAN_CHIP_TEST
#include "dmac_test_main.h"
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_main.h"
#include "hmac_roam_connect.h"
#include "hmac_roam_alg.h"
#endif //_PRE_WLAN_FEATURE_ROAM

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_CLASSIFIER_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) && !defined(_PRE_PC_LINT) && !defined(WIN32))
OAL_STATIC oal_uint8 g_ucLinklossLogSwitch = 0;
#endif

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


oal_uint32  hmac_mgmt_tx_action(
                hmac_vap_stru              *pst_hmac_vap,
                hmac_user_stru             *pst_hmac_user,
                mac_action_mgmt_args_stru  *pst_action_args)
{
    if ((OAL_PTR_NULL == pst_hmac_vap)
     || (OAL_PTR_NULL == pst_hmac_user)
     || (OAL_PTR_NULL == pst_action_args))
    {
        OAM_ERROR_LOG3(0, OAM_SF_TX, "{hmac_mgmt_tx_action::param null, %d %d %d.}", pst_hmac_vap, pst_hmac_user, pst_action_args);
        return OAL_ERR_CODE_PTR_NULL;
    }

    switch (pst_action_args->uc_category)
    {
        case MAC_ACTION_CATEGORY_BA:

            switch (pst_action_args->uc_action)
            {
                case MAC_BA_ACTION_ADDBA_REQ:
                    OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{hmac_mgmt_tx_action::MAC_BA_ACTION_ADDBA_REQ.}");
                    hmac_mgmt_tx_addba_req(pst_hmac_vap, pst_hmac_user, pst_action_args);
                    break;

                #if 0 /* ���Բ�ʹ�øýӿڣ�ֱ�ӵ��� */
                case BA_ACTION_ADDBA_RSP:
                    hmac_mgmt_tx_addba_rsp(hmac_vap_stru * pst_hmac_vap, hmac_user_stru * pst_hmac_user, mac_action_mgmt_args_stru * pst_action_args)
                    break;
                #endif

                case MAC_BA_ACTION_DELBA:
                    OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{hmac_mgmt_tx_action::MAC_BA_ACTION_DELBA.}");
                    hmac_mgmt_tx_delba(pst_hmac_vap, pst_hmac_user, pst_action_args);
                    break;

                default:
                    OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{hmac_mgmt_tx_action::invalid ba type[%d].}", pst_action_args->uc_action);
                    return OAL_FAIL;    /* �������ʹ��޸� */
            }
            break;

        default:
            OAM_INFO_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{hmac_mgmt_tx_action::invalid ba type[%d].}", pst_action_args->uc_category);
            break;
    }

    return OAL_SUCC;
}


oal_uint32  hmac_mgmt_tx_priv_req(
                hmac_vap_stru              *pst_hmac_vap,
                hmac_user_stru             *pst_hmac_user,
                mac_priv_req_args_stru     *pst_priv_req)
{
    mac_priv_req_11n_enum_uint8  en_req_type;

    if ((OAL_PTR_NULL == pst_hmac_vap) || (OAL_PTR_NULL == pst_hmac_user) || (OAL_PTR_NULL == pst_priv_req))
    {
        OAM_ERROR_LOG3(0, OAM_SF_TX, "{hmac_mgmt_tx_priv_req::param null, %d %d %d.}", pst_hmac_vap, pst_hmac_user, pst_priv_req);
        return OAL_ERR_CODE_PTR_NULL;
    }

    en_req_type = pst_priv_req->uc_type;

    switch (en_req_type)
    {
        case MAC_A_MPDU_START:

            hmac_mgmt_tx_ampdu_start(pst_hmac_vap, pst_hmac_user, pst_priv_req);
            break;

        case MAC_A_MPDU_END:
            hmac_mgmt_tx_ampdu_end(pst_hmac_vap, pst_hmac_user, pst_priv_req);
            break;

       default:

            OAM_INFO_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{hmac_mgmt_tx_priv_req::invalid en_req_type[%d].}", en_req_type);
            break;
    };

    return OAL_SUCC;
}

#if 0

OAL_STATIC oal_uint32  hmac_mgmt_rx_action(
                hmac_vap_stru          *pst_vap,
                oal_netbuf_stru        *pst_mgmt_netbuf)
{
    oal_uint8                          *puc_frame_payload;
    mac_ieee80211_frame_stru           *pst_frame_hdr;          /* ����mac֡��ָ�� */
    hmac_rx_ctl_stru                   *pst_rx_ctrl;
    hmac_user_stru                     *pst_hmac_user;
    oal_uint32                          ul_rslt;
    oal_uint16                          us_user_idx;

    if ((OAL_PTR_NULL == pst_vap) || (OAL_PTR_NULL == pst_mgmt_netbuf))
    {
        OAM_ERROR_LOG2(0, OAM_SF_RX, "{hmac_mgmt_rx_action::param null, %d %d.}", pst_vap, pst_mgmt_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡ��MPDU�Ŀ�����Ϣ */
    pst_rx_ctrl = (hmac_rx_ctl_stru *)oal_netbuf_cb(pst_mgmt_netbuf);

    /* ��ȡ֡ͷ��Ϣ */
    pst_frame_hdr = (mac_ieee80211_frame_stru *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr;

    /* ��ȡ���Ͷ˵��û�ָ�� */
    ul_rslt = mac_vap_find_user_by_macaddr(&pst_vap->st_vap_base_info, pst_frame_hdr->auc_address2, &us_user_idx);

    if (OAL_SUCC != ul_rslt)
    {
        OAM_WARNING_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX, "{hmac_mgmt_rx_action::mac_vap_find_user_by_macaddr failed[%d].}", ul_rslt);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_user_idx);

    /* ��ȡpayload��ָ�� */
    puc_frame_payload = (oal_uint8 *)pst_frame_hdr + MAC_80211_FRAME_LEN;

    /* ��ͬACTION֡�ķַ���� */
    switch(puc_frame_payload[MAC_ACTION_OFFSET_CATEGORY])
    {
        case MAC_ACTION_CATEGORY_BA:
        {
            /* BA��ص�ACTION֡�ķַ� */
            switch(puc_frame_payload[MAC_ACTION_OFFSET_ACTION])
            {
                case MAC_BA_ACTION_ADDBA_REQ:
                    hmac_mgmt_rx_addba_req(pst_vap, pst_hmac_user, puc_frame_payload);
                    break;

                case MAC_BA_ACTION_ADDBA_RSP:
                    hmac_mgmt_rx_addba_rsp(pst_vap, pst_hmac_user, puc_frame_payload);
                    break;

                case MAC_BA_ACTION_DELBA:
                    hmac_mgmt_rx_delba(pst_vap, pst_hmac_user, puc_frame_payload);
                    break;

                default:
                    break;
            }
        }

        break;

        default:
            OAM_INFO_LOG0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX, "{hmac_mgmt_rx_action::invalid action type.}");
            break;

    }

    return OAL_SUCC;
}
#endif

oal_uint32  hmac_mgmt_rx_delba_event(frw_event_mem_stru *pst_event_mem)
{

    frw_event_stru                     *pst_event;
    frw_event_hdr_stru                 *pst_event_hdr;
    dmac_ctx_action_event_stru         *pst_delba_event;
    oal_uint8                          *puc_da;                                 /* �����û�Ŀ�ĵ�ַ��ָ�� */
    hmac_vap_stru                      *pst_vap;                                /* vapָ�� */
    hmac_user_stru                     *pst_hmac_user;
    mac_action_mgmt_args_stru           st_action_args;

    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_BA, "{hmac_mgmt_rx_delba_event::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡ�¼�ͷ���¼��ṹ��ָ�� */
    pst_event           = (frw_event_stru *)pst_event_mem->puc_data;
    pst_event_hdr       = &(pst_event->st_event_hdr);
    pst_delba_event     = (dmac_ctx_action_event_stru *)(pst_event->auc_event_data);

    /* ��ȡvap�ṹ��Ϣ */
    pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_vap))
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_delba_event::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡĿ���û���MAC ADDR */
    puc_da = pst_delba_event->auc_mac_addr;

    /* ��ȡ���Ͷ˵��û�ָ�� */
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(&pst_vap->st_vap_base_info, puc_da);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        OAM_WARNING_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_delba_event::mac_vap_find_user_by_macaddr failed.}");
        return OAL_FAIL;
    }

    st_action_args.uc_category = MAC_ACTION_CATEGORY_BA;
    st_action_args.uc_action   = MAC_BA_ACTION_DELBA;
    st_action_args.ul_arg1     = pst_delba_event->uc_tidno;     /* ������֡��Ӧ��TID�� */
    st_action_args.ul_arg2     = pst_delba_event->uc_initiator; /* DELBA�У�����ɾ��BA�Ự�ķ���� */
    st_action_args.ul_arg3     = pst_delba_event->uc_status;    /* DELBA�д���ɾ��reason */
    st_action_args.puc_arg5    = puc_da;                        /* DELBA�д���Ŀ�ĵ�ַ */

    hmac_mgmt_tx_action(pst_vap, pst_hmac_user, &st_action_args);

    return OAL_SUCC;
}


oal_uint32  hmac_rx_process_mgmt_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                     *pst_event;
    frw_event_hdr_stru                 *pst_event_hdr;
    dmac_wlan_crx_event_stru           *pst_crx_event;
    oal_netbuf_stru                    *pst_netbuf;                     /* ���ڱ������ָ֡���NETBUF */
    hmac_vap_stru                      *pst_vap;                        /* vapָ�� */
    oal_uint32                          ul_ret;

    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_process_mgmt_event::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡ�¼�ͷ���¼��ṹ��ָ�� */
    pst_event           = (frw_event_stru *)pst_event_mem->puc_data;
    pst_event_hdr       = &(pst_event->st_event_hdr);
    pst_crx_event       = (dmac_wlan_crx_event_stru *)(pst_event->auc_event_data);
    pst_netbuf     = pst_crx_event->pst_netbuf;

    /* ��ȡvap�ṹ��Ϣ */
    pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_vap))
    {
        OAM_WARNING_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_delba_event::pst_vap null.}");
        /* ������֡�Ŀռ�黹�ڴ�� */
        //oal_netbuf_free(pst_netbuf); /* ���ز��ɹ���������ͳһ�ͷţ����ﲻ��Ҫ�ͷ� */
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ���չ���֡��״̬����һ�����룬����״̬���ӿ� */
    if (WLAN_VAP_MODE_BSS_AP == pst_vap->st_vap_base_info.en_vap_mode)
    {
        ul_ret = hmac_fsm_call_func_ap(pst_vap, HMAC_FSM_INPUT_RX_MGMT, pst_crx_event);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0,OAM_SF_BA,"{hmac_rx_process_mgmt_event::hmac_fsm_call_func_ap fail.err code1 [%u]}",ul_ret);
        }
    }
    else if (WLAN_VAP_MODE_BSS_STA == pst_vap->st_vap_base_info.en_vap_mode)
    {
        ul_ret = hmac_fsm_call_func_sta(pst_vap, HMAC_FSM_INPUT_RX_MGMT, pst_crx_event);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0,OAM_SF_BA,"{hmac_rx_process_mgmt_event::hmac_fsm_call_func_ap fail.err code2 [%u]}",ul_ret);
        }
    }

    /* ����֡ͳһ�ͷŽӿ� */
    oal_netbuf_free(pst_netbuf);

    return OAL_SUCC;
}


oal_uint32  hmac_mgmt_tbtt_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                     *pst_event;
    frw_event_hdr_stru                 *pst_event_hdr;
    hmac_vap_stru                      *pst_hmac_vap;
    hmac_misc_input_stru                st_misc_input;
    oal_uint32                          ul_ret;

    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_mgmt_tbtt_event::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEMZERO(&st_misc_input, OAL_SIZEOF(hmac_misc_input_stru));

    /* ��ȡ�¼�ͷ���¼��ṹ��ָ�� */
    pst_event      = (frw_event_stru *)pst_event_mem->puc_data;
    pst_event_hdr  = &(pst_event->st_event_hdr);

    pst_hmac_vap   = mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_WARNING_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_ANY, "{hmac_mgmt_tbtt_event::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_misc_input.en_type = HMAC_MISC_TBTT;

    /* ����sta״̬����ֻ��sta��tbtt�¼��ϱ���hmac */
    ul_ret = hmac_fsm_call_func_sta(pst_hmac_vap, HMAC_FSM_INPUT_MISC, &st_misc_input);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_ANY, "{hmac_mgmt_tbtt_event::hmac_fsm_call_func_sta fail. erro code is %u}", ul_ret);
    }

    return ul_ret;
}


oal_uint32  hmac_mgmt_send_disasoc_deauth_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                     *pst_event;
    frw_event_hdr_stru                 *pst_event_hdr;
    dmac_diasoc_deauth_event           *pst_disasoc_deauth_event;
    oal_uint8                          *puc_da;                                 /* �����û�Ŀ�ĵ�ַ��ָ�� */
    hmac_vap_stru                      *pst_vap;                                /* vapָ�� */
    hmac_user_stru                     *pst_hmac_user = OAL_PTR_NULL;
    oal_uint32                          ul_rslt;
    oal_uint16                          us_user_idx;
    oal_uint8                           uc_event = 0;
    mac_vap_stru                       *pst_mac_vap = OAL_PTR_NULL;
    oal_uint16                          us_err_code = 0;
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_vap_stru                       *pst_up_vap1;
    mac_vap_stru                       *pst_up_vap2;
    mac_device_stru                    *pst_mac_device;
#endif

    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{hmac_mgmt_send_disasoc_deauth_event::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡ�¼�ͷ���¼��ṹ��ָ�� */
    pst_event           = (frw_event_stru *)pst_event_mem->puc_data;
    pst_event_hdr       = &(pst_event->st_event_hdr);
    pst_disasoc_deauth_event     = (dmac_diasoc_deauth_event *)(pst_event->auc_event_data);

    /* ��ȡvap�ṹ��Ϣ */
    pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_vap))
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_ASSOC, "{hmac_mgmt_send_disasoc_deauth_event::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = &pst_vap->st_vap_base_info;

    /* ��ȡĿ���û���MAC ADDR */
    puc_da      = pst_disasoc_deauth_event->auc_des_addr;
    uc_event    = pst_disasoc_deauth_event->uc_event;
    us_err_code = pst_disasoc_deauth_event->uc_reason;

    /* ����ȥ��֤, δ����״̬�յ�������֡ */
    if(DMAC_WLAN_CRX_EVENT_SUB_TYPE_DEAUTH == uc_event)
    {
        hmac_mgmt_send_deauth_frame(pst_mac_vap,
                                    puc_da,
                                    us_err_code,
                                    OAL_FALSE); // ��PMF

#ifdef _PRE_WLAN_FEATURE_P2P
        pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

        /* �ж�����ƵDBACģʽʱ���޷��ж����ĸ��ŵ��յ�������֡�������ŵ�����Ҫ��ȥ��֤ */
        ul_rslt = mac_device_find_2up_vap(pst_mac_device, &pst_up_vap1, &pst_up_vap2);
        if (OAL_SUCC != ul_rslt)
        {
            return OAL_SUCC;
        }

        if (pst_up_vap1->st_channel.uc_chan_number == pst_up_vap2->st_channel.uc_chan_number)
        {
            return OAL_SUCC;
        }

        /* ��ȡ��һ��VAP */
        if (pst_mac_vap->uc_vap_id != pst_up_vap1->uc_vap_id)
        {
            pst_up_vap2 = pst_up_vap1;
        }

        /* ����һ��VAPҲ��ȥ��֤֡��error code���������ǣ���ȥ��֤֡ʱҪ�޸�Դ��ַ */
        hmac_mgmt_send_deauth_frame(pst_up_vap2,
                                    puc_da,
                                    us_err_code | MAC_SEND_TWO_DEAUTH_FLAG,
                                    OAL_FALSE);
#endif

        return OAL_SUCC;
    }

    /* ��ȡ���Ͷ˵��û�ָ�� */
    ul_rslt = mac_vap_find_user_by_macaddr(pst_mac_vap, puc_da, &us_user_idx);
    if (ul_rslt != OAL_SUCC)
    {

        OAM_WARNING_LOG4(0, OAM_SF_RX, "{hmac_mgmt_send_disasoc_deauth_event::Hmac cannot find USER by addr[%02X:XX:XX:%02X:%02X:%02X], just del DMAC user}",
                  puc_da[0],
                  puc_da[3],
                  puc_da[4],
                  puc_da[5]);

        /* �Ҳ����û���˵���û��Ѿ�ɾ����ֱ�ӷ��سɹ�������Ҫ�����¼���dmacɾ���û�(ͳһ��hmac_user_del������ɾ���û�) */
        return OAL_SUCC;
    }

    /* ��ȡ��hmac user,ʹ��protected��־ */
    pst_hmac_user = mac_res_get_hmac_user(us_user_idx);
    if(OAL_PTR_NULL == pst_hmac_user)
    {
        OAM_ERROR_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_ASSOC, "{hmac_mgmt_send_disasoc_deauth_event::pst_hmac_user null.us_user_idx[%d]}", us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_mgmt_send_disassoc_frame(pst_mac_vap, puc_da, us_err_code, ((OAL_PTR_NULL==pst_hmac_user) ? OAL_FALSE : pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active));

    /* ɾ���û� */
    hmac_user_del(pst_mac_vap, pst_hmac_user);

    return OAL_SUCC;
}

OAL_STATIC mac_reason_code_enum_uint16 hmac_disassoc_reason_exchange(dmac_disasoc_misc_reason_enum_uint16  en_driver_disasoc_reason)
{
    switch (en_driver_disasoc_reason)
    {
        case DMAC_DISASOC_MISC_LINKLOSS:
        case DMAC_DISASOC_MISC_KEEPALIVE:
        case DMAC_DISASOC_MISC_GET_CHANNEL_IDX_FAIL:
            return  MAC_DEAUTH_LV_SS;

        case DMAC_DISASOC_MISC_CHANNEL_MISMATCH:
            return MAC_UNSPEC_REASON;
        default:
            break;
    }
    OAM_WARNING_LOG1(0, OAM_SF_ASSOC, "{hmac_disassoc_reason_exchange::Unkown driver_disasoc_reason[%d].}", en_driver_disasoc_reason);

    return MAC_UNSPEC_REASON;
}


oal_uint32  hmac_proc_disasoc_misc_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru          *pst_event;
    hmac_vap_stru           *pst_hmac_vap;
    hmac_user_stru          *pst_hmac_user;
    oal_bool_enum_uint8      en_is_protected = OAL_FALSE;  /* PMF */
    dmac_disasoc_misc_stru  *pdmac_disasoc_misc_stru;
    mac_reason_code_enum_uint16 en_disasoc_reason_code;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{hmac_proc_disasoc_misc_event::pst_event_mem is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    pdmac_disasoc_misc_stru = (dmac_disasoc_misc_stru*)pst_event->auc_event_data;
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hmac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{hmac_proc_disasoc_misc_event::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_WARNING_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC, "{hmac_proc_disasoc_misc_event::Device noticed to dissasoc user[%d] within reason[%d]!}",
                    pdmac_disasoc_misc_stru->us_user_idx,
                    pdmac_disasoc_misc_stru->en_disasoc_reason);

#ifdef _PRE_WLAN_1102A_CHR
    hmac_chr_set_disasoc_reason(pdmac_disasoc_misc_stru->us_user_idx, pdmac_disasoc_misc_stru->en_disasoc_reason);
#endif

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) && !defined(_PRE_PC_LINT) && !defined(WIN32))
    /* ��ǿ�ź�(����-65dBm)�³���link loss��ͨ��Bcpu���ȫ���Ĵ�����ͬʱ��ά��Ĭ�ϲ���Ч*/
    /* ��Ϊһ��������Ҫ�����ֻ���������ʹ�ã�ʹ��ʱ��Ҫ�ֶ��޸�g_ucLinklossLogSwitch=1��������汾 */
    if (g_ucLinklossLogSwitch && (DMAC_DISASOC_MISC_LINKLOSS == pdmac_disasoc_misc_stru->en_disasoc_reason)&& (pst_hmac_vap->station_info.signal > -65))
    {
        wifi_open_bcpu_set(1);

#ifdef PLATFORM_DEBUG_ENABLE
        debug_uart_read_wifi_mem(OAL_TRUE);
#endif
    }
#endif

    #ifdef _PRE_WLAN_CHIP_TEST
    /* CCA����ʱ����Ҫ����STA��ȥ����, ����д�chip_test�꿪�� */
    if(0 != g_st_dmac_test_mng.uc_chip_test_open)
    {
        /* ����д�cca���� */
        if (1 == g_st_dmac_test_mng.uc_cca_flag)
        {
        }
    }
    #endif


    if (WLAN_VAP_MODE_BSS_AP == pst_hmac_vap->st_vap_base_info.en_vap_mode)
    {
        pst_hmac_user = mac_res_get_hmac_user(pdmac_disasoc_misc_stru->us_user_idx);
        if (OAL_PTR_NULL == pst_hmac_user)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ASSOC, "{hmac_proc_disasoc_misc_event ap::pst_hmac_user[%d] is null.}", pdmac_disasoc_misc_stru->us_user_idx);
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_is_protected = pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active;
#ifdef _PRE_WLAN_1102A_CHR
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_SOFTAP_DISCONNECT, pdmac_disasoc_misc_stru->en_disasoc_reason);
#endif
        /* ���¼��ϱ��ںˣ��Ѿ�ȥ����ĳ��STA */
        hmac_handle_disconnect_rsp_ap(pst_hmac_vap, pst_hmac_user);

        /* ��ȥ����֡ */
        hmac_mgmt_send_disassoc_frame(&pst_hmac_vap->st_vap_base_info, pst_hmac_user->st_user_base_info.auc_user_mac_addr, MAC_ASOC_NOT_AUTH, en_is_protected);

        /* ɾ���û� */
        hmac_user_del(&pst_hmac_vap->st_vap_base_info, pst_hmac_user);

    }
    else
    {

        /* ���û� */
        pst_hmac_user = mac_res_get_hmac_user(pst_hmac_vap->st_vap_base_info.uc_assoc_vap_id);
        if (OAL_PTR_NULL == pst_hmac_user)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ASSOC, "{hmac_proc_disasoc_misc_event::pst_hmac_user[%d] is null.}", pst_hmac_vap->st_vap_base_info.uc_assoc_vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_is_protected = pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active;


        /* �ϱ��������͵�ת�� */
        en_disasoc_reason_code = hmac_disassoc_reason_exchange(pdmac_disasoc_misc_stru->en_disasoc_reason);

        if (pdmac_disasoc_misc_stru->en_disasoc_reason != DMAC_DISASOC_MISC_CHANNEL_MISMATCH)
        {
            /* ����ȥ��֤֡��AP */
            hmac_mgmt_send_disassoc_frame(&pst_hmac_vap->st_vap_base_info, pst_hmac_user->st_user_base_info.auc_user_mac_addr, en_disasoc_reason_code, en_is_protected);
        }

        /* ɾ����Ӧ�û� */
        hmac_user_del(&pst_hmac_vap->st_vap_base_info, pst_hmac_user);
        hmac_sta_handle_disassoc_rsp(pst_hmac_vap, en_disasoc_reason_code);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_ROAM

oal_uint32  hmac_proc_roam_trigger_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru          *pst_event;
    hmac_vap_stru           *pst_hmac_vap;
    oal_int8                 c_rssi;
    hmac_roam_info_stru     *pst_roam_info;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_proc_roam_trigger_event::pst_event_mem is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event    = (frw_event_stru *)pst_event_mem->puc_data;
    c_rssi       = *(oal_int8 *)pst_event->auc_event_data;
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hmac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_proc_roam_trigger_event::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if(OAL_PTR_NULL == pst_roam_info)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ROAM, "{hmac_proc_roam_trigger_event::pst_hmac_vap->pul_roam_info is NULL, Return!}");
        return OAL_ERR_CODE_ROAM_EVENT_UXEXPECT;
    }
    pst_roam_info->st_config.uc_scan_orthogonal = ROAM_SCAN_CHANNEL_ORG_BUTT;
    pst_roam_info->en_roam_trigger = ROAM_TRIGGER_DMAC;

    hmac_roam_trigger_handle(pst_hmac_vap, c_rssi, OAL_TRUE);

    return OAL_SUCC;
}
#endif //_PRE_WLAN_FEATURE_ROAM

/*lint -e19*/
oal_module_symbol(hmac_mgmt_tx_priv_req);
/*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

