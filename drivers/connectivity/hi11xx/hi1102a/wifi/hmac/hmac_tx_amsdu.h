

#ifndef __HMAC_TX_AMSDU_H__
#define __HMAC_TX_AMSDU_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_tx_data.h"
#include "hmac_main.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_AMSDU_H
/*****************************************************************************
  2 �궨��
*****************************************************************************/
/* HT������Ϣ��amsdu����λ */
#define HT_CAP_AMSDU_LEN 0x0800

/*1102 amsdu��������15ms FPGA 1500,1103��Ϊ5ms��1102aҲ��5ms */
#define HMAC_AMSDU_LIFE_TIME    5


#define HMAC_AMSDU_INIT_MSDU_HEAD(_pst_amsdu)   \
{\
    (_pst_amsdu)->st_msdu_head.pst_next = (oal_netbuf_stru *)&((_pst_amsdu)->st_msdu_head);\
    (_pst_amsdu)->st_msdu_head.pst_prev = (oal_netbuf_stru *)&((_pst_amsdu)->st_msdu_head);\
}

/* �̰��ۺ������� */
#define HMAC_AMSDU_SHORT_PACKET_NUM     0x02

/* С��500�ֽڵİ�Ϊ�̰�    */
#define HMAC_AMSDU_SHORT_PACKET_LEN     500

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/


/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  7 STRUCT����
*****************************************************************************/

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/



OAL_STATIC OAL_INLINE oal_void  hmac_amsdu_set_maxnum(hmac_amsdu_stru *pst_amsdu, oal_uint8 uc_max_num)
{
    if (uc_max_num > WLAN_AMSDU_MAX_NUM)
    {
        pst_amsdu->uc_amsdu_maxnum = WLAN_AMSDU_MAX_NUM;
    }
    else
    {
        pst_amsdu->uc_amsdu_maxnum = uc_max_num;
    }
}


OAL_STATIC OAL_INLINE oal_void  hmac_amsdu_set_maxsize(hmac_amsdu_stru    *pst_amsdu,
                                                       hmac_user_stru     *pst_hmac_user,
                                                       oal_uint16          us_max_size)
{
    if (us_max_size > pst_hmac_user->us_amsdu_maxsize)
    {
        pst_amsdu->us_amsdu_maxsize = pst_hmac_user->us_amsdu_maxsize;
    }
    else
    {
        pst_amsdu->us_amsdu_maxsize = us_max_size;
    }

    if (1 == pst_hmac_user->st_user_base_info.st_ht_hdl.uc_htc_support)
    {
        /* Account for HT-MAC Header, FCS & Security headers */
        pst_amsdu->us_amsdu_maxsize -= (30 + 4 + 16);
    }
    else
    {
        /* Account for QoS-MAC Header, FCS & Security headers */
        pst_amsdu->us_amsdu_maxsize -= (26 + 4 + 16);
    }

    if (1 == pst_hmac_user->uc_is_wds)
    {
        /* Account for the 4th address in WDS-MAC Header */
        pst_amsdu->us_amsdu_maxsize -= (6);
    }
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_amsdu_is_short_pkt(oal_uint32 ul_frame_len)
{
    if (ul_frame_len < HMAC_AMSDU_SHORT_PACKET_LEN)
    {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
  10 ��������
*****************************************************************************/
extern oal_uint32  hmac_amsdu_notify(hmac_vap_stru *pst_vap, hmac_user_stru *pst_user, oal_netbuf_stru *pst_buf);
extern oal_void    hmac_amsdu_init_user(hmac_user_stru *pst_hmac_user_sta);
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
extern oal_void hmac_tx_encap_large_skb_amsdu(hmac_vap_stru *pst_vap, hmac_user_stru *pst_user,oal_netbuf_stru *pst_buf,mac_tx_ctl_stru *pst_tx_ctl);
#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of hmac_tx_amsdu.h */
