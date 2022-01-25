

#ifndef __HMAC_USER_H__
#define __HMAC_USER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "mac_user.h"
#include "mac_resource.h"
#include "hmac_ext_if.h"
#include "dmac_ext_if.h"
#include "hmac_edca_opt.h"
#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "hmac_btcoex.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_USER_H

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define HMAC_ADDBA_EXCHANGE_ATTEMPTS        10  /* ��ͼ����ADDBA�Ự������������ */

#define HMAC_USER_IS_AMSDU_SUPPORT(_user, _tid)         (((_user)->uc_amsdu_supported) & (0x01 << ((_tid) & 0x07)))

#define HMAC_USER_SET_AMSDU_SUPPORT(_user, _tid)        (((_user)->uc_amsdu_supported) |= (0x01 << ((_tid) & 0x07)))

#define HMAC_USER_SET_AMSDU_NOT_SUPPORT(_user, _tid)    (((_user)->uc_amsdu_supported) &= (oal_uint8)(~(0x01 << ((_tid) & 0x07))))

#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
#define MAX_JUDGE_CACHE_LENGTH      20  /* ҵ��ʶ��-�û���ʶ����г��� */
#define MAX_CONFIRMED_FLOW_NUM      2   /* ҵ��ʶ��-�û���ʶ��ҵ������ */
#define MAX_CLEAR_JUDGE_TH          2   /* ҵ��ʶ��-�û�δʶ��ҵ�����������ʶ��Ĵ������� */

#endif

#define   HMAC_USER_STATS_PKT_INCR(_member, _cnt)            ((_member) += (_cnt))


/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_CAR
typedef enum
{
    HMAC_CAR_UPLINK = 0,       /* ���� */
    HMAC_CAR_DOWNLINK = 1,     /* ���� */

    HMAC_CAR_BUTT
}hmac_car_up_down_type_enum;
typedef oal_uint8 hmac_car_up_down_type_enum_uint8;
#endif

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
#ifdef _PRE_WLAN_FEATURE_CAR
/* HMAC car ���Ʊ��� */
typedef struct
{
  oal_bool_enum_uint8       en_car_limit_flag;                      /* ��device/vap/user�Ƿ����� */
  oal_bool_enum_uint8       en_car_same_left_bytes_flag;            /* 0Ĭ��ֵ:����ȣ���Ҫ����ʣ������  1:��ȣ�����Ҫ����ʣ������*/
  oal_uint8                 auc_resv[2];
  oal_uint32                ul_car_limit_kbps;                      /* ��device/vap/user���ƴ����С */
  oal_uint32                ul_car_limit_left_bytes;                /* ÿ��device/vap/user��һ������Ͱ��������*/
  oal_uint32                ul_car_orgin_limit_left_bytes;          /* ÿ��device/vap/userԭʼ������Ͱ����������*/
  oal_uint32                ul_car_multicast_pps_num_left;          /* ÿ��device��һ��pps������Ͱ��������*/
  oal_uint32                ul_car_orgin_multicast_pps_num;         /* ÿ��device��һ��pps��ԭʼ����Ͱ����������*/
}hmac_car_limit_stru;
#endif

/* TID��Ӧ�ķ���BA�Ự��״̬ */
typedef struct
{
    frw_timeout_stru                st_addba_timer;
    dmac_ba_alarm_stru              st_alarm_data;

    dmac_ba_conn_status_enum_uint8  en_ba_status;       /* ��TID��Ӧ��BA�Ự��״̬ */
    oal_uint8                       uc_addba_attemps;   /* ��������BA�Ự�Ĵ��� */
    oal_uint8                       uc_dialog_token;    /* �������� */
    oal_bool_enum_uint8             uc_ba_policy;       /* Immediate=1 Delayed=0 */
    oal_spin_lock_stru              st_ba_status_lock;	/*��TID��Ӧ��BA�Ự��״̬��*/
}hmac_ba_tx_stru;

typedef struct
{
    oal_bool_enum_uint8                     in_use;                     /* ����BUF�Ƿ�ʹ�� */
    oal_uint8                               uc_num_buf;                 /* MPDUռ�õ�netbuf(����������)���� */
    oal_uint16                              us_seq_num;                 /* MPDU��Ӧ�����к� */
    oal_netbuf_head_stru                    st_netbuf_head;             /* MPDU��Ӧ���������׵�ַ */
    oal_uint32                              ul_rx_time;                 /* ���ı������ʱ��� */
} hmac_rx_buf_stru;

typedef struct
{
    oal_void                               *pst_ba;
    oal_uint8                               uc_tid;
    mac_delba_initiator_enum_uint8          en_direction;
    oal_uint8                               uc_resv[1];
    oal_uint8                               uc_vap_id;
    oal_uint16                              us_mac_user_idx;
    oal_uint16                              us_timeout_times;

}hmac_ba_alarm_stru;

/*Hmac����ղ�BA�Ự��� */
typedef struct
{
    oal_uint16                      us_baw_start;               /* ��һ��δ�յ���MPDU�����к� */
    oal_uint16                      us_baw_end;                 /* ���һ�����Խ��յ�MPDU�����к� */
    oal_uint16                      us_baw_tail;                /* ĿǰRe-Order�����У��������к� */
    oal_uint16                      us_baw_size;                /* Block_Ack�Ự��buffer size��С */
    oal_spin_lock_stru              st_ba_lock;                 /* 02����hcc�̺߳��¼��̲߳��� */

    oal_bool_enum_uint8             en_timer_triggered;         /* ��һ���ϱ��Ƿ�Ϊ��ʱ���ϱ� */
    oal_bool_enum_uint8             en_is_ba;                   /* Session Valid Flag */
    dmac_ba_conn_status_enum_uint8  en_ba_status;               /* ��TID��Ӧ��BA�Ự��״̬ */
    oal_uint8                       uc_mpdu_cnt;                /* ��ǰRe-Order�����У�MPDU����Ŀ */

    hmac_rx_buf_stru                ast_re_order_list[WLAN_AMPDU_RX_BUFFER_SIZE];  /* Re-Order���� */
    hmac_ba_alarm_stru              st_alarm_data;
    /* frw_timeout_stru                st_ba_timer; */

    /*����action֡���*/
    mac_back_variant_enum_uint8     en_back_var;        /* BA�Ự�ı��� */
    oal_uint8                       uc_dialog_token;    /* ADDBA����֡��dialog token */
    oal_uint8                       uc_ba_policy;       /* Immediate=1 Delayed=0 */
    oal_uint8                       uc_lut_index;       /* ���ն�Session H/w LUT Index */
    oal_uint16                      us_status_code;     /* ����״̬�� */
    oal_uint16                      us_ba_timeout;      /* BA�Ự������ʱʱ�� */
    oal_uint8                      *puc_transmit_addr;  /* BA�Ự���Ͷ˵�ַ */
    oal_bool_enum_uint8             en_amsdu_supp;      /* BLOCK ACK֧��AMSDU�ı�ʶ */
    oal_uint8                       auc_resv1[1];
    oal_uint16                      us_baw_head;        /* bitmap����ʼ���к� */
    oal_uint32                      aul_rx_buf_bitmap[2];
}hmac_ba_rx_stru;

/* user�ṹ�У�TID��Ӧ��BA��Ϣ�ı���ṹ */
typedef struct
{
    oal_uint8               uc_tid_no;
    oal_bool_enum_uint8     en_ampdu_start;             /* ��ʶ��tid�µ�AMPDU�Ƿ��Ѿ������� */
    oal_uint16              us_hmac_user_idx;
    hmac_ba_tx_stru         st_ba_tx_info;
    hmac_ba_rx_stru        *pst_ba_rx_info;             /* ���ڲ��ִ������ƣ��ⲿ���ڴ浽LocalMem������ */
    frw_timeout_stru        st_ba_timer;                /* ���������򻺳峬ʱ */
    oal_spin_lock_stru      st_ba_timer_lock;           /* ����hcc�̺߳��¼��̲߳��� */
    oal_bool_enum_uint8     en_ba_handle_tx_enable;     /* ��tid�·��ͷ����Ƿ�����BA�������������� */
    oal_bool_enum_uint8     en_ba_handle_rx_enable;     /* ��tid�½��ͷ����Ƿ�����BA�������������� */
    oal_uint8               auc_resv[2];
}hmac_tid_stru;
typedef struct
{
    oal_uint32  bit_short_preamble      : 1,        /* �Ƿ�֧��802.11b��ǰ���� 0=��֧�֣� 1=֧��*/
                bit_erp                 : 1,        /* AP����STA����ʹ��,ָʾuser�Ƿ���ERP������ 0=��֧�֣�1=֧��*/
                bit_short_slot_time     : 1,        /* ��ʱ϶: 0=��֧��, 1=֧�� */
                bit_11ac2g              : 1,
                bit_resv                : 28;
}hmac_user_cap_info_stru;
typedef struct
{
    oal_uint8        uc_rs_nrates;                                 /* ���� */
    oal_uint8        auc_rs_rates[WLAN_USER_MAX_SUPP_RATES];       /* ���� */
}hmac_rate_stru;

#ifdef _PRE_WLAN_FEATURE_WAPI
#define WAPI_KEY_LEN                    16
#define WAPI_PN_LEN                     16
#define HMAC_WAPI_MAX_KEYID             2


typedef struct
{
    oal_uint32 ulrx_mic_calc_fail;           /* ���ڲ���������mic������� */
    oal_uint32 ultx_ucast_drop;              /* ����Э��û����ɣ���֡drop�� */
    oal_uint32 ultx_wai;
    oal_uint32 ultx_port_valid;             /* Э����ɵ�����£����͵�֡���� */
    oal_uint32 ulrx_port_valid;             /* Э����ɵ�����£����յ�֡���� */
    oal_uint32 ulrx_idx_err;                /* ����idx������� */
    oal_uint32 ulrx_netbuff_len_err;        /* ����netbuff���ȴ��� */
    oal_uint32 ulrx_idx_update_err;         /* ��Կ���´��� */
    oal_uint32 ulrx_key_en_err;             /* ��Կû��ʹ�� */
    oal_uint32 ulrx_pn_odd_err;             /* PN��żУ����� */
    oal_uint32 ulrx_pn_replay_err;          /* PN�ط� */
    oal_uint32 ulrx_memalloc_err;           /* rx�ڴ�����ʧ�� */
    oal_uint32 ulrx_decrypt_ok;             /* ���ܳɹ��Ĵ��� */

    oal_uint32 ultx_memalloc_err;            /* �ڴ����ʧ�� */
    oal_uint32 ultx_mic_calc_fail;           /* ���ڲ���������mic������� */
    //oal_uint32 ultx_drop_wai;              /* wai֡drop�Ĵ��� */
    oal_uint32 ultx_encrypt_ok;              /* ���ܳɹ��Ĵ��� */
    oal_uint8  aucrx_pn[WAPI_PN_LEN];       /* ���ⷢ��ʱ����¼���շ���֡��PN,��pn����ʱ��ˢ�� */

}hmac_wapi_debug;

typedef struct
{
    oal_uint8   auc_wpi_ek[WAPI_KEY_LEN];
    oal_uint8   auc_wpi_ck[WAPI_KEY_LEN];
    oal_uint8   auc_pn_rx[WAPI_PN_LEN];
    oal_uint8   auc_pn_tx[WAPI_PN_LEN];
    oal_uint8   uc_key_en;
    oal_uint8   auc_rsv[3];
}hmac_wapi_key_stru;

typedef struct tag_hmac_wapi_stru
{
    oal_uint8              uc_port_valid;                       /* wapi���ƶ˿� */
    oal_uint8              uc_keyidx;
    oal_uint8              uc_keyupdate_flg;                    /* key���±�־ */
    oal_uint8              uc_pn_inc;                           /* pn����ֵ */

    hmac_wapi_key_stru     ast_wapi_key[HMAC_WAPI_MAX_KEYID];   /*keyed: 0~1 */

#ifdef _PRE_WAPI_DEBUG
    hmac_wapi_debug        st_debug;                            /* ά�� */
#endif

    oal_uint8               (*wapi_filter_frame)(struct tag_hmac_wapi_stru *pst_wapi, oal_netbuf_stru *pst_netbuff);
    oal_bool_enum_uint8     (*wapi_is_pn_odd)(oal_uint8 *puc_pn);     /* �ж�pn�Ƿ�Ϊ���� */
    oal_uint32              (*wapi_decrypt)(struct tag_hmac_wapi_stru *pst_wapi, oal_netbuf_stru *pst_netbuff);
    oal_uint32              (*wapi_encrypt)(struct tag_hmac_wapi_stru *pst_wapi, oal_netbuf_stru  *pst_netbuf);
    oal_netbuf_stru        *(*wapi_netbuff_txhandle)(struct tag_hmac_wapi_stru *pst_wapi, oal_netbuf_stru  *pst_netbuf);
    oal_netbuf_stru        *(*wapi_netbuff_rxhandle)(struct tag_hmac_wapi_stru *pst_wapi, oal_netbuf_stru  *pst_netbuf);
}hmac_wapi_stru;

#endif

#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
/* ҵ��ʶ��-��Ԫ��ṹ��: ����Ψһ�ر�ʶҵ���� */
typedef struct
{
    oal_uint32                          ul_sip;                         /* ip */
    oal_uint32                          ul_dip;

    oal_uint16                          us_sport;                       /* �˿� */
    oal_uint16                          us_dport;

    oal_uint32                          ul_proto;                       /* Э�� */

}hmac_tx_flow_info_stru;

/* ҵ��ʶ��-��ʶ����нṹ��: */
typedef struct
{
    hmac_tx_flow_info_stru              st_flow_info;

    oal_uint32                          ul_len;                        /* �������� */
    oal_uint8                           uc_flag;                       /* ��Чλ�����ڼ��� */

    oal_uint8                           uc_udp_flag;                   /* udp flagΪ1��ΪUDP֡ */
    oal_uint8                           uc_tcp_flag;                   /* tcp flagΪ1��ΪTCP֡ */

    oal_uint8                           uc_rtpver;                     /* RTP version */
    oal_uint32                          ul_rtpssrc;                    /* RTP SSRC */
    oal_uint32                          ul_payload_type;               /* RTP:���1bit����Ч�غ�����(PT)7bit */

}hmac_tx_judge_info_stru;

/* ҵ��ʶ��-��ʶ�������Ҫҵ��ṹ��: */
typedef struct
{
    hmac_tx_flow_info_stru              st_flow_info;

    oal_uint32                          ul_average_len;                /* ҵ������ƽ������ */
    oal_uint8                           uc_flag;                       /* ��Чλ */

    oal_uint8                           uc_udp_flag;                   /* udp flagΪ1��ΪUDP֡ */
    oal_uint8                           uc_tcp_flag;                   /* tcp flagΪ1��ΪTCP֡ */

    oal_uint8                           uc_rtpver;                     /* RTP version */
    oal_uint32                          ul_rtpssrc;                    /* RTP SSRC */
    oal_uint32                          ul_payload_type;               /* ���1bit����Ч�غ�����(PT)7bit */

    oal_uint32                          ul_wait_check_num;             /* ������б��д�ҵ������� */

}hmac_tx_major_flow_stru;

/* ҵ��ʶ��-�û���ʶ��ṹ��: */
typedef struct
{
    hmac_tx_flow_info_stru              st_cfm_flow_info;               /* ��ʶ��ҵ�����Ԫ����Ϣ */

    oal_ulong                           ul_last_jiffies;                /* ��¼��ʶ��ҵ�����������ʱ�� */
    oal_uint16                          us_cfm_tid;                     /* ��ʶ��ҵ��tid */

    oal_uint16                          us_cfm_flag;                        /* ��Чλ */

}hmac_tx_cfm_flow_stru;


/* ҵ��ʶ��-�û���ʶ��ҵ�����: */
typedef struct
{
    oal_ulong                           ul_jiffies_st;                                  /* ��¼��ʶ��ҵ����е���ʼʱ������������ʱ�� */
    oal_ulong                           ul_jiffies_end;
    oal_uint32                          ul_to_judge_num;                                /* �û���ʶ��ҵ����г��� */
    hmac_tx_judge_info_stru             ast_judge_cache[MAX_JUDGE_CACHE_LENGTH];     /* ��ʶ�������� */

}hmac_tx_judge_list_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
typedef enum{
    HMAC_RRM_RPT_NOTIFY_HILINK       = 0,
    HMAC_RRM_RPT_NOTIFY_11V          = 1,
    HMAC_RRM_RPT_NOTIFY_CHN_LOAD     = 2,

    HMAC_RRM_RPT_NOTIFY_BUTT
}mac_rrm_rpt_notify_enum;

typedef enum{
    MAC_RRM_STATE_INIT              = 0,   /*��ʼ״̬*/
    MAC_RRM_STATE_WAIT_RSP          = 1,   /*�ȴ�RSP*/
    MAC_RRM_STATE_GET_RSP           = 2,   /*�յ�RSP*/
    MAC_RRM_STATE_RSP_TIMEOUT       = 3,   /*RSP��ʱ*/
    MAC_RRM_STATE_BUTT
}mac_rrm_state_enum;

typedef enum
{
    /*Radio Measurement Type*/
    MAC_RRM_TYPE_CHANNEL_LOAD          = 0,
    MAC_RRM_TYPE_BCN                   = 1,
    /*Neighbor Report Request*/
    MAC_RRM_TYPE_NEIGHBOR_RPT          = 10,
    MAC_RRM_MEAS_TYPE_BUTT
}mac_rrm_type_enum;

typedef struct mac_user_rrm_info_tag
{
    oal_dlist_head_stru                  st_meas_rpt_list;          /* ����Report���� */
    frw_timeout_stru                     st_timer;                  /* ��������ʱ��*/

    oal_uint8                            uc_dialog_token;           /* ����Request ��Token,��report��Ӧ */
    oal_uint8                            uc_meas_token;             /* ����Request ��Token,��report��Ӧ */
    mac_rrm_state_enum                   en_meas_status;            /* ����״̬ */
    mac_rrm_rpt_notify_enum              en_rpt_notify_id;          /* ����ģ�� */
    mac_rrm_type_enum                    en_reqtype;                /* �������� */
    mac_meas_rpt_mode_stru               st_rptmode;                /* RPTģʽ  */
    oal_uint8                            auc_resv1[2];
}mac_user_rrm_info_stru;

typedef struct mac_rrm_req_tag
{
    mac_rrm_rpt_notify_enum              en_rpt_notify_id;          /*����ģ������*/
    mac_rrm_type_enum                    en_reqtype;                /*��������*/
    oal_uint16                           us_rpt_num;                /*�ظ���������*/
    oal_uint8                            uc_req_mode;
    void                                 *p_arg;
    oal_uint8                            auc_resv1[1];
}mac_rrm_req_cfg_stru;
#endif //_PRE_WLAN_FEATURE_11K

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
#define MAC_11V_ROAM_SCAN_ONE_CHANNEL_LIMIT  2
#define MAC_11V_ROAM_SCAN_FULL_CHANNEL_LIMIT  (MAC_11V_ROAM_SCAN_ONE_CHANNEL_LIMIT + 1)


typedef oal_uint32         (*mac_user_callback_func_11v)(void *, void *, void *);
/* 11v ������Ϣ�ṹ�� */
typedef struct
{
    oal_uint8                            uc_user_bsst_token;                     /* �û�����bss transition ֡������ */
    oal_uint8                            uc_user_status;                         /* �û�11V״̬ */
    oal_uint8                            uc_11v_roam_scan_times;                 /* ���ŵ�11v����ɨ�����  */
    oal_bool_enum_uint8                  en_only_scan_one_time;                  /* ֻɨ��һ�α�־λ*/
    frw_timeout_stru                     st_status_wait_timer;                   /* �ȴ��û���Ӧ֡�ļ�ʱ�� */
    mac_user_callback_func_11v           mac_11v_callback_fn;                   /* �ص�����ָ�� */
}hmac_user_11v_ctrl_stru;
#endif

/* �����ýṹ���������һ��Ҫ����oal_memzero */
typedef struct
{
    /* ��ǰVAP������AP��STAģʽ�������ֶ�Ϊuser��STA��APʱ�����ֶΣ�������ֶ���ע��!!! */
    oal_uint8                       uc_is_wds;                                  /* �Ƿ���wds�û� */
    oal_uint8                       uc_amsdu_supported;                     /* ÿ��λ����ĳ��TID�Ƿ�֧��AMSDU */
    oal_uint16                      us_amsdu_maxsize;                           /* amsdu��󳤶� */
    hmac_amsdu_stru                 ast_hmac_amsdu[WLAN_TID_MAX_NUM];       /* asmdu���� */
    hmac_tid_stru                   ast_tid_info[WLAN_TID_MAX_NUM];             /* ������TID��ص���Ϣ */
    oal_uint32                      aul_last_timestamp[WLAN_TID_MAX_NUM];       /* ʱ�������ʵ��5���������Ľ���BA */
    oal_uint8                       auc_ch_text[WLAN_CHTXT_SIZE];               /* WEP�õ���ս���� */
    frw_timeout_stru                st_mgmt_timer;                              /* ��֤�����ö�ʱ�� */
    frw_timeout_stru                st_defrag_timer;                            /* ȥ��Ƭ��ʱ��ʱ�� */
    oal_netbuf_stru                *pst_defrag_netbuf;
    oal_uint8                       auc_ba_flag[WLAN_TID_MAX_NUM];          /* �ñ�־��ʾ��TID�Ƿ���Խ���BA�Ự�����ڵ���5ʱ���Խ���BA�Ự���ñ�־���û���ʼ����ɾ��BA�Ựʱ���� */
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    mac_sa_query_stru               st_sa_query_info;                       /* sa query���̵Ŀ�����Ϣ */
#endif
    hmac_rate_stru                  st_op_rates;                            /* user��ѡ����,��¼�Զ�assoc req����assoc rsp֡��supported rates��assoc rsp���Ƶ����� */
    hmac_user_cap_info_stru         st_hmac_cap_info;                       /* hmac���û�������־λ */
    oal_uint32                      ul_rssi_last_timestamp;                 /* ��ȡuser rssi����ʱ���, 1s������һ��rssi */

    /* ��ǰVAP������APģʽ�������ֶ�Ϊuser��STAʱ�����ֶΣ�������ֶ���ע��!!! */
    oal_uint32                      ul_assoc_req_ie_len;
    oal_uint8                      *puc_assoc_req_ie_buff;
    oal_bool_enum_uint8             en_user_bw_limit;                       /* �����Ƿ������� */
    oal_bool_enum_uint8             en_wmm_switch;                          /* ���û��Ƿ�wmm */
    oal_bool_enum_uint8             en_user_vendor_vht_capable;
    oal_bool_enum_uint8             en_user_vendor_novht_capable;
    oal_bool_enum_uint8             en_report_kernel;
    oal_uint8                       auc_resv5[1];
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    oal_uint32                      aaul_txrx_data_stat[WLAN_WME_AC_BUTT][WLAN_TXRX_DATA_BUTT];       /* ����/���� tcp/udp be,bk,vi,vo���� */
#endif

#ifdef _PRE_WLAN_FEATURE_WAPI
    hmac_wapi_stru                  st_wapi;
#endif

#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
    oal_uint8                       uc_cfm_num;                                         /* �û��ѱ�ʶ��ҵ����� */
    oal_uint8                       auc_resv2[1];
    oal_uint16                      us_clear_judge_count;                               /* δʶ���RTP���������ʶ��Ĵ��� �ڴﵽ����ǰ���ȿ���Ԥʶ�� �ӿ�ʶ����� ��ֹ�ײ�BE VI ���ȿ��ܵ��µ����� */
    hmac_tx_cfm_flow_stru           ast_cfm_flow_list[MAX_CONFIRMED_FLOW_NUM];          /* ��ʶ��ҵ�� */
    hmac_tx_judge_list_stru         st_judge_list;                                      /* ��ʶ�������� */


    //oal_spin_lock_stru          st_lock;

#endif
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hmac_user_btcoex_stru           st_hmac_user_btcoex;
#endif

    /* ��ǰVAP������STAģʽ�������ֶ�Ϊuser��APʱ�����ֶΣ�������ֶ���ע��!!! */
    mac_user_stats_flag_stru        st_user_stats_flag;                      /*��user��staʱ��ָʾuser�Ƿ�ͳ�Ƶ���Ӧ��*/
    oal_uint32                      ul_rx_pkt_drop;                      /* �������ݰ�host�౻drop�ļ��� */

    oal_uint32                      ul_first_add_time;                          /* �û�����ʱ��ʱ�䣬����ͳ���û�����ʱ�� */

    /* dmac����ͬ����Ϣ */
    oal_int8                        c_rssi;
    oal_uint8                       uc_ps_st;
    oal_int8                        c_free_power;
    oal_uint8                       auc_resv3[1];
#ifdef _PRE_WLAN_DFT_STAT
    oal_uint8                       uc_cur_per;
    oal_uint8                       uc_bestrate_per;
    oal_uint8                       auc_resv4[2];
#endif
    oal_uint32                      ul_tx_rate;
    oal_uint32                      ul_tx_rate_min;
    oal_uint32                      ul_tx_rate_max;
    oal_uint32                      ul_rx_rate;
    oal_uint32                      ul_rx_rate_min;
    oal_uint32                      ul_rx_rate_max;
    oal_uint32                      aul_drop_num[WLAN_WME_AC_BUTT];
    oal_uint32                      aul_tx_delay[3];                    /*0 1 2�ֱ�Ϊ�������ʱ,��С������ʱ,ƽ��������ʱ*/
    mac_ap_type_enum_uint16         en_user_ap_type;                    /* �û���AP���ͣ�����������ʹ�� */
    oal_uint8                       auc_mimo_blacklist_mac[WLAN_MAC_ADDR_LEN];
    //oal_uint8                       auc_resv6[2];

#ifdef _PRE_WLAN_FEATURE_CAR
    hmac_car_limit_stru             st_car_user_cfg[HMAC_CAR_BUTT];    /* user���ٽṹ��,0-up 1-down */
#endif

#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    mac_user_rrm_info_stru          *pst_user_rrm_info;
#endif

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    hmac_user_11v_ctrl_stru         st_11v_ctrl_info;                /* 11v������Ϣ�ṹ�� */
#endif

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    oal_uint8                       auc_ie_oui[3];
    oal_uint8                       uc_resv5;
#endif

#ifdef _PRE_WLAN_FEATURE_WMMAC
        mac_ts_stru                 st_ts_tmp_info[WLAN_WME_AC_BUTT];/*ĳ��AC��TS�Ѿ������ɹ����ٴν���ʱ��ʱ��������*/
#endif

    /* ����������ܴ���HMAC USER�ṹ���ڵ����һ�� */
    mac_user_stru                   st_user_base_info;                          /* hmac user��dmac user�������� */

}hmac_user_stru;

/* �洢AP��������֡��ie��Ϣ�������ϱ��ں� */
typedef struct
{
    oal_uint32                      ul_assoc_req_ie_len;
    oal_uint8                      *puc_assoc_req_ie_buff;
    oal_uint8                       auc_user_mac_addr[WLAN_MAC_ADDR_LEN];
}hmac_asoc_user_req_ie_stru;


/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_user_ht_support(hmac_user_stru *pst_hmac_user)
{
    if (OAL_TRUE == pst_hmac_user->st_user_base_info.st_ht_hdl.en_ht_capable)
    {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_user_vht_support(hmac_user_stru *pst_hmac_user)
{
    if (OAL_TRUE == pst_hmac_user->st_user_base_info.st_vht_hdl.en_vht_capable)
    {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_user_xht_support(hmac_user_stru *pst_hmac_user)
{
    if ((pst_hmac_user->st_user_base_info.en_cur_protocol_mode >= WLAN_HT_MODE)
        && (pst_hmac_user->st_user_base_info.en_cur_protocol_mode < WLAN_PROTOCOL_BUTT))
    {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
  10 ��������
*****************************************************************************/
extern oal_uint32  hmac_user_set_asoc_req_ie(hmac_user_stru *pst_hmac_user, oal_uint8 *puc_payload, oal_uint32 ul_payload_len, oal_uint8 uc_reass_flag);
extern oal_uint32  hmac_user_free_asoc_req_ie(oal_uint16 us_idx);
extern oal_uint32  hmac_user_alloc_etc(oal_uint16 *pus_user_idx);
extern oal_uint32  hmac_user_free_etc(oal_uint16 us_idx);
extern oal_uint32  hmac_user_init_etc(hmac_user_stru *pst_hmac_user);
extern oal_uint32  hmac_user_set_avail_num_space_stream_etc(mac_user_stru *pst_mac_user, wlan_nss_enum_uint8 en_vap_nss);
extern oal_void    hmac_sdio_to_pcie_switch(mac_vap_stru *pst_mac_vap);
extern oal_uint32  hmac_user_del_etc(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user);
extern oal_uint32  hmac_user_add_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_mac_addr, oal_uint16 *pus_user_index);
extern oal_uint32  hmac_user_add_multi_user_etc(mac_vap_stru *pst_mac_vap, oal_uint16 *us_user_index);
extern oal_uint32  hmac_user_del_multi_user_etc(mac_vap_stru *pst_mac_vap);
extern oal_uint32 hmac_user_add_notify_alg_etc(mac_vap_stru *pst_mac_vap, oal_uint16 us_user_idx);
extern oal_uint32 hmac_update_user_last_active_time(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_void hmac_tid_clear_etc(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user);
extern hmac_user_stru*  mac_res_get_hmac_user_alloc_etc(oal_uint16 us_idx);
extern hmac_user_stru*  mac_res_get_hmac_user_etc(oal_uint16 us_idx);
extern hmac_user_stru  *mac_vap_get_hmac_user_by_addr_etc(mac_vap_stru *pst_mac_vap, oal_uint8  *puc_mac_addr);
extern mac_ap_type_enum_uint16 hmac_compability_ap_tpye_identify_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_mac_addr);
#ifdef _PRE_WLAN_FEATURE_WAPI
extern hmac_wapi_stru *hmac_user_get_wapi_ptr_etc(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_pairwise, oal_uint16 us_pairwise_idx);
extern oal_uint8  hmac_user_is_wapi_connected_etc(oal_uint8 uc_device_id);

#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of hmac_user.h */
