

#ifndef __HMAC_TCP_OPT_STRUC_H__
#define __HMAC_TCP_OPT_STRUC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_TCP_OPT

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "oal_types.h"
#include "oal_net.h"
#include "oam_ext_if.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TCP_OPT_STRUC_H

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define HMAC_TCP_STREAM                 16
#define TCPDATA_INFO_MAXNUM              4
#define TCPDATA_PSH_INFO_MAXNUM         (8 * TCPDATA_INFO_MAXNUM)

#define HMAC_INVALID_TCP_ACK_INDEX      0xFF


/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
enum
{
    TCP_ACK_FILTER_TYPE  = 1,
    TCP_ACK_DUP_TYPE     = 2,
    TCP_TYPE_ERROR
};
typedef oal_uint8 oal_tcp_ack_type_enum_uint8;
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


typedef struct _wlan_perform_tcp_impl_
{
    oal_uint64 ull_drop_count;
    oal_uint64 ull_send_count;
    oal_uint64 ull_dup_ack_count;
}wlan_perform_tcp_impl;

typedef struct _wlan_perform_tcp_impls_
{
    oal_uint32 ul_ack_limit;
    oal_uint64 ull_ignored_count;
    wlan_perform_tcp_impl st_tcp_info[HMAC_TCP_STREAM];
}wlan_perform_tcp_impls;

struct skb_tcp_cb
{
    oal_uint16 us_index;
    oal_uint16 us_reserve;
    oal_uint32  ul_tcp_type;
    oal_uint32  ul_tcp_ack_no;
};

struct wlan_perform_tcp
{
    /*hcc rx/tx tcp ack param*/
    oal_netbuf_head_stru ast_hcc_ack_queue[HMAC_TCP_STREAM];
    oal_uint32      aul_hcc_ack_count[HMAC_TCP_STREAM];               /* ack num in each queue */
    oal_uint32      aul_hcc_duplicate_ack_count[HMAC_TCP_STREAM];
    oal_spin_lock_stru hmac_tcp_ack_lock;
#ifdef HCC_DEBUG
    oal_uint32 aul_ack_count[HMAC_TCP_STREAM];
    oal_uint32 aul_duplicate_ack_count[HMAC_TCP_STREAM];
#endif
    /*TBD filter upline ack later*/
};
struct wlan_tcp_flow
{
    oal_uint32      ul_src_ip;
    oal_uint32      ul_dst_ip;
    oal_uint16      us_src_port;
    oal_uint16      us_dst_port;
    oal_uint8       uc_protocol;
    oal_uint32      ul_tcp_ack_no;
};
struct tcp_list_node
{
    oal_dlist_head_stru    list;
    oal_uint8              uc_used;       /* 0-free, 1-used */
    struct wlan_tcp_flow   st_wlan_tcp_info;
    oal_uint8              uc_index;
    oal_uint               ui_last_ts;  /*time of last frame*/
};
struct wlan_perform_tcp_list
{
    struct tcp_list_node    ast_tcp_pool[HMAC_TCP_STREAM];
    oal_uint8               uc_idle_idx_cnt;
    oal_spin_lock_stru      tcp_lock;      /* lock for tcp pool */
    oal_spin_lock_stru      tcp_list_lock; /* lock for tcp list */
    oal_dlist_head_stru     tcp_list;
    oal_uint8               uc_last_used_idx; /* ���ڱ������µ�����Ϣ�ڵ�λ�ã���������� */
};

/* TCP ACK �Ż����� ���� */
typedef struct
{
    oal_uint8    uc_tcp_ack_filter_en;          /* ���ƻ��Ƿ��tcp ack���� */
    oal_uint16   us_rx_filter_throughput_high;
    oal_uint16   us_rx_filter_throughput_low;
}hmac_tcp_ack_opt_th_params;

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
/*****************************************************************************
  11 ������������
*****************************************************************************/


#endif   /* end of _PRE_WLAN_TCP_OPT */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of hmac_tcp_opt_struc.h */
