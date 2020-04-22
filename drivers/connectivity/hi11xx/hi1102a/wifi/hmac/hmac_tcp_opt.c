


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_TCP_OPT

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/*lint -e322*/
#include <linux/jiffies.h>
/*lint +e322*/
#endif
#include "oam_ext_if.h"
#include "hmac_tcp_opt_struc.h"
#include "oal_hcc_host_if.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_tcp_opt.h"
#include "mac_data.h"
#include "oal_net.h"
#include "oal_types.h"
#include "hmac_rx_data.h"
#include "frw_event_main.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TCP_OPT_C

/*****************************************************************************
  2 结构体定义
*****************************************************************************/

/*****************************************************************************
  3 宏定义
*****************************************************************************/

/*defined for ut test*/
#if defined(WIN32)
oal_uint32 jiffies;

oal_bool_enum_uint8 time_before_eq(oal_uint32 a,oal_uint32 b)
{
    return OAL_TRUE;
}
#endif

#define HMAC_JUDGE_IS_SAME_TCP_NODE_INFO(index) \
        if ((pst_tmp_tcp_pool[index].uc_used == 1) \
            && (pst_tmp_tcp_pool[index].st_wlan_tcp_info.us_src_port == st_tcp_flow_info.us_src_port)\
            && (pst_tmp_tcp_pool[index].st_wlan_tcp_info.us_dst_port == st_tcp_flow_info.us_dst_port)\
            && (pst_tmp_tcp_pool[index].st_wlan_tcp_info.ul_src_ip   == st_tcp_flow_info.ul_src_ip)  \
            && (pst_tmp_tcp_pool[index].st_wlan_tcp_info.ul_dst_ip   == st_tcp_flow_info.ul_dst_ip)  \
            && (pst_tmp_tcp_pool[index].st_wlan_tcp_info.uc_protocol == st_tcp_flow_info.uc_protocol))


/*****************************************************************************
  4 全局变量定义
*****************************************************************************/
hmac_tcp_ack_opt_th_params g_st_tcp_ack_opt_th_params = {0, 0, 0};


/*****************************************************************************
  5 内部静态函数声明
*****************************************************************************/
oal_uint16 hmac_tcp_opt_tx_tcp_ack_filter(hmac_vap_stru    *pst_hmac_vap, hmac_tcp_opt_queue type,hcc_chan_type dir, oal_netbuf_head_stru  *head);

/*****************************************************************************
  4 函数实现
*****************************************************************************/

void hmac_tcp_opt_ack_count_reset(hmac_vap_stru    *pst_hmac_vap,hcc_chan_type dir, oal_uint16 stream)
{
    if(OAL_WARN_ON(!pst_hmac_vap))
    {
        OAL_IO_PRINT("%s error:pst_hmac_vap is null",__FUNCTION__);
        return;
    }
    oal_spin_lock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.hmac_tcp_ack_lock);
    pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.aul_hcc_ack_count[stream] = 0;
    pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.aul_hcc_duplicate_ack_count[stream] = 0;
    oal_spin_unlock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.hmac_tcp_ack_lock);
}


void hmac_tcp_opt_ack_all_count_reset(hmac_vap_stru    *pst_hmac_vap)
{
    oal_uint16     us_dir_index = 0;
    oal_uint16     us_tcp_index = HCC_TX;

    for(us_tcp_index = 0; us_tcp_index < HMAC_TCP_STREAM; us_tcp_index++)
    {
        hmac_tcp_opt_ack_count_reset(pst_hmac_vap,(hcc_chan_type)us_dir_index, us_tcp_index);
    }
}


void hmac_tcp_opt_ack_show_count(hmac_vap_stru    *pst_hmac_vap)
{
    oal_uint16     us_dir_index = 0;
    oal_uint16     us_tcp_index = HCC_TX;

    for(us_dir_index = 0; us_dir_index < HCC_DIR_COUNT; us_dir_index++)
    {
        for(us_tcp_index = 0; us_tcp_index < HMAC_TCP_OPT_QUEUE_BUTT; us_tcp_index++)
        {
            oal_spin_lock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].st_hmac_tcp_ack.hmac_tcp_ack_lock);
            OAL_IO_PRINT("dir = %u,tcp_index = %u,all_ack_count = %llu,drop_count = %llu\n",
                us_dir_index,
                us_tcp_index,
                pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].all_ack_count[us_tcp_index],
                pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].drop_count[us_tcp_index]);

            pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].all_ack_count[us_tcp_index] = 0;
            pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].drop_count[us_tcp_index] = 0;
            oal_spin_unlock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].st_hmac_tcp_ack.hmac_tcp_ack_lock);
        }
    }
}

struct tcp_list_node *hmac_tcp_opt_find_oldest_node(hmac_vap_stru         *pst_hmac_vap, hcc_chan_type dir)
{
    struct tcp_list_node *node;
    struct tcp_list_node *oldest_node = OAL_PTR_NULL;
    oal_uint  oldest_time = jiffies;  /*init current time*/
    oal_uint8               uc_tcp_stream_index;
    struct tcp_list_node *pst_tcp_pool;

    pst_tcp_pool = pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack_list.ast_tcp_pool;

    for(uc_tcp_stream_index = 0; uc_tcp_stream_index < HMAC_TCP_STREAM; uc_tcp_stream_index++)
    {
        node = &pst_tcp_pool[uc_tcp_stream_index];
        if(OAL_PTR_NULL == node)
        {
            continue;
        }
        if(OAL_TRUE == time_before_eq(node->ui_last_ts,oldest_time))
        {
            oldest_time = node->ui_last_ts;
            oldest_node = node;
        }
    }

    if (OAL_PTR_NULL != oldest_node)
    {
        oal_dlist_delete_entry(&oldest_node->list);
        oal_dlist_init_head(&oldest_node->list);
    }
    else
    {
        /* 在其调用函数hmac_tcp_opt_add_new_node中有判断是否为空指针 */
        OAM_ERROR_LOG0(0,OAM_SF_ANY,"hmac_tcp_opt_find_oldest_node::Can't find oldest node xx!!");
    }
    return oldest_node;
}

struct tcp_list_node *hmac_tcp_opt_get_node(hmac_vap_stru    *pst_hmac_vap, hcc_chan_type dir)
{
    struct tcp_list_node *node = NULL;
    oal_uint8  uc_tcp_stream_index;
    struct wlan_perform_tcp_list *tmp_ack_list = &pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack_list;

    if (0 == tmp_ack_list->uc_idle_idx_cnt)
    {
        node = hmac_tcp_opt_find_oldest_node(pst_hmac_vap,dir);
        return node;
    }

    for (uc_tcp_stream_index = 0; uc_tcp_stream_index < HMAC_TCP_STREAM; uc_tcp_stream_index++)
    {
        if ( 0 == tmp_ack_list->ast_tcp_pool[uc_tcp_stream_index].uc_used)
        {
            tmp_ack_list->ast_tcp_pool[uc_tcp_stream_index].uc_used = 1;
            tmp_ack_list->uc_idle_idx_cnt--;

            node = &tmp_ack_list->ast_tcp_pool[uc_tcp_stream_index];
            break;
        }
    }
    return node;
}


oal_uint32 hmac_tcp_opt_init_filter_tcp_ack_pool(hmac_vap_stru    *pst_hmac_vap)
{
    oal_uint16     us_dir_index = 0;
    oal_uint16     us_tcp_index;
    oal_uint16     us_tcp_queue_index;

    if(OAL_WARN_ON(OAL_PTR_NULL == pst_hmac_vap))
    {
        OAM_INFO_LOG0(0,OAM_SF_ANY,"{hmac_tcp_opt_init_filter_tcp_ack_poolr fail:pst_hmac_vap is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }


    /*init downline tcp ack pool*/

    /*init tx_worker_state*/

    for(us_dir_index = 0; us_dir_index < HCC_DIR_COUNT; us_dir_index++)
    {
        for (us_tcp_index = 0; us_tcp_index < HMAC_TCP_STREAM; us_tcp_index++)
        {
            oal_spin_lock_init(&pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].st_hmac_tcp_ack.hmac_tcp_ack_lock);
            oal_netbuf_list_head_init(&pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].st_hmac_tcp_ack.ast_hcc_ack_queue[us_tcp_index]);
            pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].st_hmac_tcp_ack_list.ast_tcp_pool[us_tcp_index].uc_used = 0;
            pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].st_hmac_tcp_ack_list.ast_tcp_pool[us_tcp_index].uc_index = us_tcp_index;
            pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].st_hmac_tcp_ack_list.ast_tcp_pool[us_tcp_index].ui_last_ts = jiffies;
        }
        for (us_tcp_queue_index = 0; us_tcp_queue_index < HMAC_TCP_OPT_QUEUE_BUTT; us_tcp_queue_index++)
        {
            oal_spin_lock_init(&pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].data_queue_lock[us_tcp_queue_index]);
            oal_netbuf_head_init(&pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].data_queue[us_tcp_queue_index]);
        }
        oal_dlist_init_head(&(pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].st_hmac_tcp_ack_list.tcp_list));
        pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].st_hmac_tcp_ack_list.uc_idle_idx_cnt = HMAC_TCP_STREAM;
        pst_hmac_vap->ast_hmac_tcp_ack[us_dir_index].st_hmac_tcp_ack_list.uc_last_used_idx = HMAC_INVALID_TCP_ACK_INDEX;
        OAM_INFO_LOG1(0,OAM_SF_ANY,"{wifi tcp perform dir:%d init done.}", us_dir_index);
    }
#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
    pst_hmac_vap->ast_hmac_tcp_ack[HCC_TX].filter[HMAC_TCP_ACK_QUEUE] = hmac_tcp_opt_tx_tcp_ack_filter;
    pst_hmac_vap->ast_hmac_tcp_ack[HCC_RX].filter[HMAC_TCP_ACK_QUEUE] = OAL_PTR_NULL;
#endif
    return OAL_SUCC;
}


void hmac_tcp_opt_free_ack_list(hmac_vap_stru *pst_hmac_vap,oal_uint8 dir,oal_uint8 type)
{
#if !defined(WIN32)
    oal_netbuf_head_stru  st_head_t;
    oal_netbuf_head_stru *head;
    oal_netbuf_stru* pst_netbuf;
    oal_netbuf_head_stru* hcc_ack_queue;
    struct wlan_perform_tcp_list *tmp_list;
    struct tcp_list_node       *node;
    oal_uint16               us_tcp_stream_index;

    oal_spin_lock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[dir].data_queue_lock[type]);

    oal_netbuf_head_init(&st_head_t);
    head = &pst_hmac_vap->ast_hmac_tcp_ack[dir].data_queue[type];
    oal_netbuf_queue_splice_tail_init(head, &st_head_t);
    while(!!(pst_netbuf = oal_netbuf_delist(&st_head_t)))
    {
        oal_netbuf_free(pst_netbuf);
    }

    tmp_list = &pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack_list;

    for(us_tcp_stream_index = 0; us_tcp_stream_index < HMAC_TCP_STREAM; us_tcp_stream_index++)
    {
        node = &tmp_list->ast_tcp_pool[us_tcp_stream_index];
        if(OAL_PTR_NULL == node)
        {
            continue;
        }
        /* 删除vap时,清空信息流节点 */
        node->uc_used = 0;

        /* 最好的方式是信息流节点的free和hcc_ack_queue的free分开 */
        hcc_ack_queue = &pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.ast_hcc_ack_queue[us_tcp_stream_index];
        while(!!(pst_netbuf = oal_netbuf_delist(hcc_ack_queue)))
        {
            oal_netbuf_free(pst_netbuf);
        }
    }

    oal_spin_unlock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[dir].data_queue_lock[type]);
#endif
}

void hmac_tcp_opt_deinit_list(hmac_vap_stru *pst_hmac_vap)
{
    hmac_tcp_opt_free_ack_list(pst_hmac_vap,HCC_TX,HMAC_TCP_ACK_QUEUE);
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    hmac_tcp_opt_free_ack_list(pst_hmac_vap,HCC_RX,HMAC_TCP_ACK_QUEUE);
#endif
}


oal_tcp_ack_type_enum_uint8 hmac_tcp_opt_get_tcp_ack_type(hmac_vap_stru          *pst_hmac_vap,
                                             oal_ip_header_stru  *pst_ip_hdr,
                                             hcc_chan_type  dir,
                                             oal_uint8  uc_index)
{
    oal_tcp_header_stru     *pst_tcp_hdr;
    oal_uint32              ul_new_tcp_ack_no;
    oal_uint32              *pul_old_tcp_ack_no;

    /*lint -e522*/
    OAL_WARN_ON(uc_index >= HMAC_TCP_STREAM);
    /*lint +e522*/
    pst_tcp_hdr     = (oal_tcp_header_stru *)(pst_ip_hdr + 1);
    ul_new_tcp_ack_no = pst_tcp_hdr->ul_acknum;

    /*检测duplicat ack是否存在，如果存在则累计ack流最大成员数*/
    pul_old_tcp_ack_no = &pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack_list.ast_tcp_pool[uc_index].st_wlan_tcp_info.ul_tcp_ack_no;

    if (ul_new_tcp_ack_no == *pul_old_tcp_ack_no) /* 找到重复tcp ack帧 */
    {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        OAL_IO_PRINT("\r\n===hmac_tcp_opt_get_tcp_ack_type:: dir:%d:**duplicate ack is coming**", dir);
#endif

        pst_hmac_vap->ast_hmac_tcp_ack[dir].filter_info.st_tcp_info[uc_index].ull_dup_ack_count++;
        return TCP_ACK_DUP_TYPE;
    }

    *pul_old_tcp_ack_no = ul_new_tcp_ack_no;
    return TCP_ACK_FILTER_TYPE;


}


oal_bool_enum_uint8 hmac_judge_rx_netbuf_is_tcp_ack(mac_llc_snap_stru *pst_snap)
{
    oal_ip_header_stru  *pst_ip_hdr;
    oal_bool_enum_uint8 en_is_tcp_ack = OAL_FALSE;
    oal_tcp_header_stru    *pst_tcp_hdr;

    if(OAL_PTR_NULL == pst_snap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_judge_rx_netbuf_is_tcp_ack:  pst_snap is null!}");
        return OAL_FALSE;
    }
    switch (pst_snap->us_ether_type)
    {
        /*lint -e778*//* 屏蔽Info -- Constant expression evaluates to 0 in operation '&' */
        case OAL_HOST2NET_SHORT(ETHER_TYPE_IP):

            pst_ip_hdr = (oal_ip_header_stru *)(pst_snap + 1);      /* 偏移一个snap，取ip头 */

#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            OAM_WARNING_LOG1(0, OAM_SF_RX, "{oal_judge_rx_netbuf_is_tcp_ack:  pst_ip_hdr->uc_protocol = %d**!}",pst_ip_hdr->uc_protocol);
#endif
            if (MAC_TCP_PROTOCAL == pst_ip_hdr->uc_protocol)
            {
                if (OAL_TRUE == oal_netbuf_is_tcp_ack(pst_ip_hdr))
                {
                    pst_tcp_hdr = (oal_tcp_header_stru *)(pst_ip_hdr + 1);
                    /*option3:SYN FIN RST URG有为1的时候不过滤*/
                    if ((pst_tcp_hdr->uc_flags) & FILTER_FLAG_MASK)
                    {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
                        OAM_WARNING_LOG0(0, OAM_SF_RX, "{hmac_judge_rx_netbuf_is_tcp_ack:  **specific tcp pkt, can't be filter**!}");
#endif
                    }
                    else
                    {
                        en_is_tcp_ack = OAL_TRUE;
                    }
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
                    OAM_WARNING_LOG0(0, OAM_SF_RX, "{oal_judge_rx_netbuf_is_tcp_ack:: tcp ack frame!}");
#endif
                }
            }
            break;
        /*lint +e778*/
        default:
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            OAL_IO_PRINT("{oal_judge_rx_netbuf_is_tcp_ack::unkown us_ether_type[%d]}\r\n", pst_snap->us_ether_type);
#endif
            break;
    }

    return en_is_tcp_ack;
}


oal_bool_enum_uint8 hmac_judge_rx_netbuf_classify(oal_netbuf_stru *pst_netbuff)
{
    mac_llc_snap_stru             *pst_snap;

    pst_snap = (mac_llc_snap_stru *)(pst_netbuff);
    if(OAL_PTR_NULL == pst_snap)
    {
        return OAL_FALSE;
    }
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    OAM_WARNING_LOG1(0,OAM_SF_ANY,"**hmac_judge_rx_netbuf_classify, us_ether_type = %d**",pst_snap->us_ether_type);
#endif
    return hmac_judge_rx_netbuf_is_tcp_ack(pst_snap);
}



oal_bool_enum_uint8 hmac_judge_tx_netbuf_is_tcp_ack(oal_ether_header_stru *ps_ethmac_hdr)
{
    oal_ip_header_stru     *pst_ip;
    oal_tcp_header_stru    *pst_tcp_hdr;
    oal_bool_enum_uint8     en_is_tcp_ack = OAL_FALSE;


    if(OAL_PTR_NULL == ps_ethmac_hdr)
    {
        return OAL_FALSE;
    }
    switch (ps_ethmac_hdr->us_ether_type)
    {
        /*lint -e778*//* 屏蔽Info -- Constant expression evaluates to 0 in operation '&' */
        case OAL_HOST2NET_SHORT(ETHER_TYPE_IP):

            pst_ip = (oal_ip_header_stru *)(ps_ethmac_hdr + 1);      /* 偏移一个snap，取ip头 */

            if (MAC_TCP_PROTOCAL == pst_ip->uc_protocol)
            {

                if (OAL_TRUE == oal_netbuf_is_tcp_ack(pst_ip))
                {
                    pst_tcp_hdr = (oal_tcp_header_stru *)(pst_ip + 1);
                    /*option3:SYN FIN RST URG有为1的时候不过滤*/
                    if ((pst_tcp_hdr->uc_flags) & FILTER_FLAG_MASK)
                    {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
                        OAM_WARNING_LOG0(0,OAM_SF_ANY,"**specific tcp pkt, can't be filter**");
#endif
                    }
                    else
                    {
                        en_is_tcp_ack = OAL_TRUE;
                    }
                }
            }
            break;

        /*lint +e778*/
        default:
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            OAL_IO_PRINT("{oal_netbuf_select_queue::unkown us_ether_type[%d]}\r\n", ps_ethmac_hdr->us_ether_type);
#endif
            break;
    }

    return en_is_tcp_ack;
}


oal_tcp_ack_type_enum_uint8  hmac_tcp_opt_rx_get_tcp_ack_type_index(oal_netbuf_stru *skb, hmac_vap_stru    *pst_hmac_vap, oal_uint8 *p_uc_index, oal_uint8 dir)
{
    oal_ip_header_stru  *pst_ip_hdr;
    oal_tcp_header_stru *pst_tcp_hdr;
    mac_llc_snap_stru             *pst_snap;
    hmac_rx_ctl_stru                   *pst_rx_ctrl;                        /* 指向MPDU控制块信息的指针 */
    struct wlan_tcp_flow        st_tcp_flow_info;

    OAL_MEMZERO(&st_tcp_flow_info, OAL_SIZEOF(st_tcp_flow_info));

    pst_rx_ctrl = (hmac_rx_ctl_stru *)oal_netbuf_cb(skb);
    pst_snap = (mac_llc_snap_stru*)(skb->data + pst_rx_ctrl->st_rx_info.uc_mac_header_len);

    if(OAL_FALSE == hmac_judge_rx_netbuf_is_tcp_ack(pst_snap))
    {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        /* not tcp ack data */
        OAM_INFO_LOG0(0,OAM_SF_ANY,"{**not tcp packet return direct**}\r\n");
#endif
        return TCP_TYPE_ERROR;
    }
    pst_ip_hdr = (oal_ip_header_stru *)(pst_snap + 1);      /* 偏移一个snap，取ip头 */
    pst_tcp_hdr     = (oal_tcp_header_stru *)(pst_ip_hdr + 1);

    st_tcp_flow_info.ul_src_ip   = pst_ip_hdr->ul_saddr;
    st_tcp_flow_info.ul_dst_ip   = pst_ip_hdr->ul_daddr;
    st_tcp_flow_info.us_src_port = pst_tcp_hdr->us_sport;
    st_tcp_flow_info.us_dst_port = pst_tcp_hdr->us_dport;
    st_tcp_flow_info.uc_protocol = pst_ip_hdr->uc_protocol;

    /* 先判断是否有相同的信息流，如果是再继续判断是否是重复帧 */
    if(OAL_TRUE == hmac_tcp_opt_judge_is_same_tcp_flow_info(pst_hmac_vap,st_tcp_flow_info,p_uc_index,(hcc_chan_type)dir))
    {
        return hmac_tcp_opt_get_tcp_ack_type(pst_hmac_vap,pst_ip_hdr,(hcc_chan_type)dir,*p_uc_index);
    }

    /* 如果没有相匹配的信息流，则要新申请一个信息流，此时传入的skb为TCP_ACK_FILTER_TYPE */
    *p_uc_index = hmac_tcp_opt_add_new_node(pst_hmac_vap, &st_tcp_flow_info,pst_tcp_hdr, dir);

    /*flow index取不到时不过滤*/
    if(HMAC_INVALID_TCP_ACK_INDEX == *p_uc_index)
    {
        return TCP_TYPE_ERROR;
    }

    return TCP_ACK_FILTER_TYPE;

}

oal_bool_enum_uint8 hmac_tcp_opt_judge_is_same_tcp_flow_info(hmac_vap_stru            *pst_hmac_vap,
                                             const struct wlan_tcp_flow  st_tcp_flow_info,
                                             oal_uint8 *puc_index,
                                             const hcc_chan_type dir)
{
    struct wlan_perform_tcp_list    *pst_tmp_tcp_list;
    oal_uint8                       uc_last_used_idx;
    oal_uint8                       uc_tcp_stream_index;
    struct tcp_list_node             *pst_tmp_tcp_pool;

    pst_tmp_tcp_list = &pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack_list;
    pst_tmp_tcp_pool = pst_tmp_tcp_list->ast_tcp_pool;

    /*  提前预取上次刚刚使用的信息流节点与新传入的节点进行匹配判断，提高命中效率 */
    uc_last_used_idx = pst_tmp_tcp_list->uc_last_used_idx;
    if(uc_last_used_idx >= HMAC_TCP_STREAM)/*  主要用于初始值判断 */
    {
        /* 如果last_used_idx是无效值,则退出创建新节点 */
        return OAL_FALSE;
    }

    HMAC_JUDGE_IS_SAME_TCP_NODE_INFO(uc_last_used_idx)
    {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        OAL_IO_PRINT("\r\n===hmac_tcp_opt_judge_is_same_tcp_flow_info::[Hit Ahead!]dir:%d find the same tcp flow info [index:%d]===\r\n",
                    dir,uc_last_used_idx);
#endif
        *puc_index = uc_last_used_idx;
        return OAL_TRUE;
    }

    /*  如果没有和上次更新的节点匹配，则遍历tcp_pool，继续寻找可能匹配的信息流节点 */
    for(uc_tcp_stream_index = 0; uc_tcp_stream_index < HMAC_TCP_STREAM; uc_tcp_stream_index++)
    {
        if(uc_tcp_stream_index == uc_last_used_idx)
        {
            continue;
        }

        HMAC_JUDGE_IS_SAME_TCP_NODE_INFO(uc_tcp_stream_index)
        {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            OAL_IO_PRINT("\r\n====hmac_tcp_opt_judge_is_same_tcp_flow_info::dir:%d find the same tcp flow info [index:%d]====\r\n",
                        dir,uc_tcp_stream_index);
#endif
            pst_tmp_tcp_pool[uc_tcp_stream_index].ui_last_ts = jiffies;   /* 更新此节点保存最新的信息流时间 */
            *puc_index = uc_tcp_stream_index;

            /* 将最新使用的流节点信息保存一下，提高下次的命中率 */
            pst_tmp_tcp_list->uc_last_used_idx = uc_tcp_stream_index;

            return OAL_TRUE;
        }
    }

    return OAL_FALSE; /* 没有匹配到，下一步就需要新建节点 */

}


oal_uint8 hmac_tcp_opt_add_new_node(hmac_vap_stru           *pst_hmac_vap,
                                              const struct wlan_tcp_flow *pst_tcp_info,
                                              const oal_tcp_header_stru *pst_tcp_hdr,
                                              const hcc_chan_type dir)
{
    struct tcp_list_node *pst_node;

    if ((OAL_PTR_NULL == pst_hmac_vap) || (OAL_PTR_NULL == pst_tcp_info)|| (OAL_PTR_NULL == pst_tcp_hdr))
    {
        return HMAC_INVALID_TCP_ACK_INDEX;
    }

    pst_node = hmac_tcp_opt_get_node(pst_hmac_vap,dir);

    if (OAL_PTR_NULL == pst_node)
    {
        OAM_ERROR_LOG0(0,OAM_SF_ANY,"Invalid NULL node!");
        return HMAC_INVALID_TCP_ACK_INDEX;
    }

    pst_node->st_wlan_tcp_info.ul_dst_ip   = pst_tcp_info->ul_dst_ip;
    pst_node->st_wlan_tcp_info.ul_src_ip   = pst_tcp_info->ul_src_ip;
    pst_node->st_wlan_tcp_info.us_src_port = pst_tcp_info->us_src_port;
    pst_node->st_wlan_tcp_info.us_dst_port = pst_tcp_info->us_dst_port;
    pst_node->st_wlan_tcp_info.uc_protocol = pst_tcp_info->uc_protocol;
    pst_node->st_wlan_tcp_info.ul_tcp_ack_no = pst_tcp_hdr->ul_acknum;
    pst_node->ui_last_ts       = jiffies;

    oal_dlist_add_tail(&pst_node->list, &pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack_list.tcp_list);

    return pst_node->uc_index;

}

oal_tcp_ack_type_enum_uint8  hmac_tcp_opt_tx_get_tcp_ack_type_index(oal_netbuf_stru *skb, hmac_vap_stru           *pst_hmac_vap, oal_uint8 *p_uc_index, oal_uint8 dir)
{
    oal_ip_header_stru  *pst_ip_hdr;
    oal_tcp_header_stru *pst_tcp_hdr;
    oal_ether_header_stru *eth_hdr;
    struct wlan_tcp_flow   st_tcp_flow_info;

    OAL_MEMZERO(&st_tcp_flow_info, OAL_SIZEOF(st_tcp_flow_info));

    eth_hdr = (oal_ether_header_stru *)oal_netbuf_data(skb);
    if(OAL_FALSE == hmac_judge_tx_netbuf_is_tcp_ack(eth_hdr))
    {
        /* not tcp ack data */
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        OAL_IO_PRINT("\r\n====**not tcp packet return direct**====\r\n");
#endif
        return TCP_TYPE_ERROR;
    }
    pst_ip_hdr = (oal_ip_header_stru *)(eth_hdr + 1);      /* 偏移一个snap，取ip头 */
    pst_tcp_hdr     = (oal_tcp_header_stru *)(pst_ip_hdr + 1);

    st_tcp_flow_info.ul_src_ip   = pst_ip_hdr->ul_saddr;
    st_tcp_flow_info.ul_dst_ip   = pst_ip_hdr->ul_daddr;
    st_tcp_flow_info.us_src_port = pst_tcp_hdr->us_sport;
    st_tcp_flow_info.us_dst_port = pst_tcp_hdr->us_dport;
    st_tcp_flow_info.uc_protocol = pst_ip_hdr->uc_protocol;

    /* 先判断是否有相同的信息流，如果是再继续判断是否是重复帧 */
    if(OAL_TRUE == hmac_tcp_opt_judge_is_same_tcp_flow_info(pst_hmac_vap, st_tcp_flow_info, p_uc_index, (hcc_chan_type)dir))
    {
        return hmac_tcp_opt_get_tcp_ack_type(pst_hmac_vap, pst_ip_hdr, (hcc_chan_type)dir, *p_uc_index);
    }

    /* 如果没有相匹配的信息流，则要新申请一个信息流，此时传入的skb为TCP_ACK_FILTER_TYPE */
    *p_uc_index = hmac_tcp_opt_add_new_node(pst_hmac_vap, &st_tcp_flow_info, pst_tcp_hdr, dir);

    /*flow index取不到时不过滤*/
    if(HMAC_INVALID_TCP_ACK_INDEX == *p_uc_index)
    {
        return TCP_TYPE_ERROR;
    }

    return TCP_ACK_FILTER_TYPE;

}


oal_uint16 hmac_tcp_opt_tx_tcp_ack_filter(hmac_vap_stru       *pst_hmac_vap, hmac_tcp_opt_queue type,hcc_chan_type dir, oal_netbuf_head_stru *head)
{
    oal_netbuf_stru                 *skb;
    oal_netbuf_head_stru            head_t;
    oal_uint8                      uc_tcp_stream_index;
    struct wlan_perform_tcp         *pst_hmac_tcp_ack;

    if(OAL_WARN_ON(!pst_hmac_vap))
    {
        OAM_INFO_LOG0(0,OAM_SF_ANY,"{hmac_tcp_opt_rx_tcp_ack_filter_etc fail:hmac_vap is null}");
        return OAL_FAIL;
    }

    if(OAL_WARN_ON(!head))
    {
        OAM_INFO_LOG0(0,OAM_SF_ANY,"{hmac_tcp_opt_rx_tcp_ack_filter_etc fail:head is null}");
        return OAL_FAIL;
    }

    if(OAL_WARN_ON(HMAC_TCP_ACK_QUEUE != type))
    {
        OAM_INFO_LOG2(0,OAM_SF_ANY,"{hmac_tcp_opt_rx_tcp_ack_filter_etc fail:type:%d not equal %d}",type,HMAC_TCP_ACK_QUEUE);
        return OAL_FAIL;
    }

    if(OAL_WARN_ON(HCC_TX != dir))
    {
        OAM_INFO_LOG2(0,OAM_SF_ANY,"{hmac_tcp_opt_rx_tcp_ack_filter_etc fail:dir:%d not equal %d}",dir,HCC_TX);
        return OAL_FAIL;
    }


    if(!oal_netbuf_list_len(head))
    {
        return 0;
    }
    oal_netbuf_head_init(&head_t);

    while(!!(skb = oal_netbuf_delist(head)))
    {
        if(hmac_tcp_opt_tcp_ack_filter(skb, pst_hmac_vap, dir))
        {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            OAL_IO_PRINT("\r\n====hmac_tcp_opt_tcp_ack_list_filter::[Type error]not found tcp ack...====\r\n");
#endif
            oal_netbuf_list_tail(&head_t,skb);
        }
        else
        {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            OAL_IO_PRINT("\r\n====hmac_tcp_opt_tcp_ack_list_filter::found tcp ack...====\r\n");
#endif
        }
    }
    /*lint -e522*/
    OAL_WARN_ON(!oal_netbuf_list_empty(head));
    /*lint +e522*/
    oal_netbuf_splice_init(&head_t, head);
    pst_hmac_vap->ast_hmac_tcp_ack[dir].filter_info.ull_ignored_count +=
                oal_netbuf_list_len(head);

    pst_hmac_tcp_ack = &pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack;
    for(uc_tcp_stream_index = 0; uc_tcp_stream_index < HMAC_TCP_STREAM; uc_tcp_stream_index++)
    {
        if(0 == oal_netbuf_list_len(&pst_hmac_tcp_ack->ast_hcc_ack_queue[uc_tcp_stream_index]))
        {
            continue;
        }
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        OAL_IO_PRINT("\r\n====hmac_tcp_opt_tx_tcp_ack_filter::dir:%d ------recv packet------[qlen:%u].====\r\n", dir,
                    oal_netbuf_list_len(&pst_hmac_tcp_ack->ast_hcc_ack_queue[uc_tcp_stream_index]));
#endif
    hmac_tcp_opt_ack_count_reset(pst_hmac_vap, (hcc_chan_type)dir, (oal_uint16)uc_tcp_stream_index);
    pst_hmac_vap->ast_hmac_tcp_ack[dir].filter_info.st_tcp_info[uc_tcp_stream_index].ull_send_count +=
            oal_netbuf_list_len(&pst_hmac_tcp_ack->ast_hcc_ack_queue[uc_tcp_stream_index]);

    /* 将过滤完之后需要发送的tcp ack帧重新加入到发送netbuf中，并将此ack_queue清空 */
    oal_netbuf_queue_splice_tail_init(&pst_hmac_tcp_ack->ast_hcc_ack_queue[uc_tcp_stream_index], head);

    }
    return 0;
}


oal_uint32 hmac_tcp_opt_tcp_ack_filter(oal_netbuf_stru *skb, hmac_vap_stru *pst_hmac_vap, hcc_chan_type dir)
{
    oal_tcp_ack_type_enum_uint8 uc_tcp_ack_type;
    oal_uint8 uc_tcp_stream_index;
    oal_uint32 ul_ack_limit;
    oal_netbuf_head_stru* hcc_ack_queue;
    oal_netbuf_stru     *ack          = NULL;
    oal_uint32 ul_ret;

    if(HCC_TX == dir)
    {
        uc_tcp_ack_type = hmac_tcp_opt_tx_get_tcp_ack_type_index(skb, pst_hmac_vap, &uc_tcp_stream_index, dir);
    }
    else
    {
        uc_tcp_ack_type = hmac_tcp_opt_rx_get_tcp_ack_type_index(skb, pst_hmac_vap, &uc_tcp_stream_index, dir);
    }


    if(TCP_ACK_FILTER_TYPE == uc_tcp_ack_type)
    {
        /* 正常ack帧处理逻辑 */
        oal_spin_lock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.hmac_tcp_ack_lock);
        ul_ack_limit = pst_hmac_vap->ast_hmac_tcp_ack[dir].filter_info.ul_ack_limit;
        hcc_ack_queue = &pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.ast_hcc_ack_queue[uc_tcp_stream_index];

        /*if normal ack received, del until ack_limit ack left*/
        while (pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.aul_hcc_ack_count[uc_tcp_stream_index] >= ul_ack_limit)
        {

            ack = oal_netbuf_delist(hcc_ack_queue);
            if (OAL_PTR_NULL == ack)
            {
                break;
            }

            pst_hmac_vap->ast_hmac_tcp_ack[dir].filter_info.st_tcp_info[uc_tcp_stream_index].ull_drop_count++;

            oal_netbuf_free_any(ack);
            pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.aul_hcc_ack_count[uc_tcp_stream_index]--;
        }


        oal_netbuf_add_to_list_tail(skb,hcc_ack_queue);
        pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.aul_hcc_ack_count[uc_tcp_stream_index]++;
        oal_spin_unlock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.hmac_tcp_ack_lock);

        return OAL_SUCC;
    }

    else if (TCP_ACK_DUP_TYPE == uc_tcp_ack_type)
    {
        /* 处理发送dup ack */
        oal_spin_lock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.hmac_tcp_ack_lock);
        hcc_ack_queue = &pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.ast_hcc_ack_queue[uc_tcp_stream_index];
        /* 将dup ack帧流队列中的帧全部发送 */

        while (!!(ack = oal_netbuf_delist(hcc_ack_queue)))
        {
            ul_ret = hmac_tx_lan_to_wlan_no_tcp_opt(&(pst_hmac_vap->st_vap_base_info), ack);
            /* 调用失败，要释放内核申请的netbuff内存池 */
            if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
            {
                oal_netbuf_free(ack);
            }
            else
            {
                pst_hmac_vap->ast_hmac_tcp_ack[dir].filter_info.st_tcp_info[uc_tcp_stream_index].ull_send_count++;
            }

            pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.aul_hcc_ack_count[uc_tcp_stream_index]--;
        }
        /* 当前dup ack帧发送 */
        ul_ret = hmac_tx_lan_to_wlan_no_tcp_opt(&(pst_hmac_vap->st_vap_base_info), skb);
        if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
        {
            oal_netbuf_free(skb);
        }
        else
        {
            pst_hmac_vap->ast_hmac_tcp_ack[dir].filter_info.st_tcp_info[uc_tcp_stream_index].ull_send_count++;
        }

        oal_spin_unlock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[dir].st_hmac_tcp_ack.hmac_tcp_ack_lock);

        return OAL_SUCC;
    }
    else
    {
        /* 异常逻辑，将帧放入vap queue发送 */
        return OAL_FAIL;
    }

 }

void hmac_trans_queue_filter(hmac_vap_stru    *pst_hmac_vap, oal_netbuf_head_stru  *head_t,
                                    hmac_tcp_opt_queue type, hcc_chan_type dir)
{
    oal_uint32 old_len, new_len;

    if(pst_hmac_vap->ast_hmac_tcp_ack[dir].filter[type])
    {
        old_len = oal_netbuf_list_len(head_t);
        pst_hmac_vap->ast_hmac_tcp_ack[dir].filter[type](pst_hmac_vap, type, dir, head_t);
        new_len = oal_netbuf_list_len(head_t);
        pst_hmac_vap->ast_hmac_tcp_ack[dir].all_ack_count[type] += old_len;
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        OAL_IO_PRINT("\r\n====hmac_trans_queue_filter[Queue:%d]Before filter len:%u,After filter len:%u====\r\n",type,old_len, new_len);
#endif
        if(OAL_UNLIKELY(new_len > old_len))
        {
            OAM_WARNING_LOG2(0, OAM_SF_TX,"The filter len:%u is more than before filter:%u",
                new_len, old_len);
        }else
        {
            pst_hmac_vap->ast_hmac_tcp_ack[dir].drop_count[type] += (old_len - new_len);
        }
    }
}
void hmac_tcp_ack_process_hcc_queue(hmac_vap_stru    *pst_hmac_vap,
                                    hcc_chan_type dir,hmac_tcp_opt_queue type)
{
    oal_netbuf_head_stru  st_head_t;
    oal_netbuf_head_stru *head;
    oal_netbuf_stru* pst_netbuf;
    oal_uint32 ul_ret;

    oal_spin_lock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[dir].data_queue_lock[type]);
    if(OAL_PTR_NULL != pst_hmac_vap->ast_hmac_tcp_ack[dir].filter[type])
    {
        oal_netbuf_head_init(&st_head_t);
        head = &pst_hmac_vap->ast_hmac_tcp_ack[dir].data_queue[type];
        oal_netbuf_queue_splice_tail_init(head, &st_head_t);
        hmac_trans_queue_filter(pst_hmac_vap, &st_head_t, type, dir);
        oal_netbuf_splice_init(&st_head_t,head);
    }
    if(HCC_RX == dir)
    {
        oal_netbuf_head_init(&st_head_t);
        head = &pst_hmac_vap->ast_hmac_tcp_ack[dir].data_queue[type];
        oal_netbuf_queue_splice_tail_init(head, &st_head_t);
        if (WLAN_VAP_MODE_BSS_AP == pst_hmac_vap->st_vap_base_info.en_vap_mode)
        {
            hmac_rx_process_data_ap_tcp_ack_opt(pst_hmac_vap,&st_head_t);
        }
        else if (WLAN_VAP_MODE_BSS_STA == pst_hmac_vap->st_vap_base_info.en_vap_mode)
        {
            hmac_rx_process_data_sta_tcp_ack_opt(pst_hmac_vap,&st_head_t);
        }
    }
    else
    {
        oal_netbuf_head_init(&st_head_t);
        head = &pst_hmac_vap->ast_hmac_tcp_ack[dir].data_queue[type];
        oal_netbuf_queue_splice_tail_init(head, &st_head_t);
        while(!!(pst_netbuf = oal_netbuf_delist(&st_head_t)))
        {
            ul_ret = hmac_tx_lan_to_wlan_no_tcp_opt(&(pst_hmac_vap->st_vap_base_info),pst_netbuf);
            /* 调用失败，要释放内核申请的netbuff内存池 */
            if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
            {
                //hmac_free_netbuf_list(pst_buf);
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
                OAL_IO_PRINT("\r\n====hmac_tcp_ack_process_hcc_queue send fail:uc_vap_id = %d====\r\n",pst_hmac_vap->st_vap_base_info.uc_vap_id);
#endif
                oal_netbuf_free(pst_netbuf);
            }
        }
    }
    oal_spin_unlock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[dir].data_queue_lock[type]);
}
oal_int32 hmac_tcp_ack_process(void)
{
    oal_uint8 uc_vap_idx;
    hmac_vap_stru* pst_hmac_vap;

    hmac_device_stru    *pst_hmac_device;

    if(!oal_in_interrupt())
        frw_event_task_lock();/*lint !e522*/

    pst_hmac_device = hmac_res_get_mac_dev(0);//当前只支持一个device，后续有需求再添加
    if (OAL_PTR_NULL == pst_hmac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{hmac_tcp_ack_process::pst_hmac_device[0] null.}");
    }
    else
    {
        for (uc_vap_idx = 0; uc_vap_idx < pst_hmac_device->pst_device_base_info->uc_vap_num; uc_vap_idx++)
        {
            pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_hmac_device->pst_device_base_info->auc_vap_id[uc_vap_idx]);
            if (OAL_PTR_NULL == pst_hmac_vap)
            {
                OAM_ERROR_LOG0(uc_vap_idx, OAM_SF_ANY, "{hmac_config_add_vap::pst_hmac_vap null.}");
                continue;
            }
            if ((MAC_VAP_STATE_UP != pst_hmac_vap->st_vap_base_info.en_vap_state) &&
                (MAC_VAP_STATE_PAUSE != pst_hmac_vap->st_vap_base_info.en_vap_state))
            {
                continue;
            }
#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
             hmac_tcp_ack_process_hcc_queue(pst_hmac_vap,HCC_TX,HMAC_TCP_ACK_QUEUE);
#endif
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
             hmac_tcp_ack_process_hcc_queue(pst_hmac_vap,HCC_RX,HMAC_TCP_ACK_QUEUE);
#endif
        }
    }

    if(!oal_in_interrupt())
        frw_event_task_unlock();
    return 0;
}
oal_bool_enum_uint8 hmac_tcp_ack_need_schedule(void)
{
    oal_uint8 uc_vap_idx;
    hmac_vap_stru* pst_hmac_vap;
    oal_netbuf_head_stru *head;
    mac_device_stru     *pst_mac_device;

    pst_mac_device = mac_res_get_dev(0);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        return OAL_FALSE;
    }

    /* 如果队列中有帧，则可以调度 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_hmac_vap)
        {
            OAM_ERROR_LOG0(uc_vap_idx, OAM_SF_ANY, "{hmac_tcp_ack_need_schedule::pst_hmac_vap null.}");
            continue;
        }

        if ((MAC_VAP_STATE_UP != pst_hmac_vap->st_vap_base_info.en_vap_state) &&
            (MAC_VAP_STATE_PAUSE != pst_hmac_vap->st_vap_base_info.en_vap_state))
        {
            continue;
        }

        oal_spin_lock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[HCC_TX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
        head = &pst_hmac_vap->ast_hmac_tcp_ack[HCC_TX].data_queue[HMAC_TCP_ACK_QUEUE];
        /* 队列中有ack帧，则需要调度线程 */
        if(0 < oal_netbuf_list_len(head))
        {
            oal_spin_unlock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[HCC_TX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
            return OAL_TRUE;
        }
        oal_spin_unlock_bh(&pst_hmac_vap->ast_hmac_tcp_ack[HCC_TX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);

        /* 接收方向未使能，无需判断 */

    }

    return OAL_FALSE;
}


oal_void hmac_sched_transfer(void)
{
    struct hcc_handler * hcc;

    hcc = hcc_get_110x_handler();
    OAL_WAIT_QUEUE_WAKE_UP_INTERRUPT(&hcc->hcc_transer_info.hcc_transfer_wq);
}
oal_int32 hmac_set_hmac_tcp_ack_process_func(hmac_tcp_ack_process_func p_func)
{
    struct hcc_handler* hcc;

    hcc = hcc_get_110x_handler();
    if (OAL_PTR_NULL == hcc)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{hmac_set_hmac_tcp_ack_process_func::hcc null.}");
    }
    else
    {
        hcc->p_hmac_tcp_ack_process_func = p_func;
    }
    return OAL_SUCC;
}
oal_int32 hmac_set_hmac_tcp_ack_need_schedule(hmac_tcp_ack_need_schedule_func p_func)
{
    struct hcc_handler* hcc;

    hcc = hcc_get_110x_handler();
    if (OAL_PTR_NULL == hcc)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{hmac_set_hmac_tcp_ack_process_func::hcc null.}");
    }
    else
    {
        hcc->p_hmac_tcp_ack_need_schedule_func = p_func;
    }
    return OAL_SUCC;
}


oal_void hmac_tcp_ack_opt_switch_ctrol(oal_uint32        ul_rx_throughput_mbps)
{
    hmac_device_stru      *pst_hmac_device;

    if (OAL_FALSE == g_st_tcp_ack_opt_th_params.uc_tcp_ack_filter_en)
    {
        /* 定制化未开启TCP ACK 优化门限设置功能，直接返回 */
        return;
    }

    pst_hmac_device = (hmac_device_stru*)hmac_res_get_mac_dev(0);

    if (pst_hmac_device == OAL_PTR_NULL)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_tcp_ack_opt_switch_ctrol:get hmac device fail.}");
        return;
    }

    if (ul_rx_throughput_mbps > g_st_tcp_ack_opt_th_params.us_rx_filter_throughput_high &&
        pst_hmac_device->sys_tcp_tx_ack_opt_enable == OAL_FALSE)
    {
        pst_hmac_device->sys_tcp_tx_ack_opt_enable = OAL_TRUE;
    }
    else if (ul_rx_throughput_mbps < g_st_tcp_ack_opt_th_params.us_rx_filter_throughput_low &&
            pst_hmac_device->sys_tcp_tx_ack_opt_enable == OAL_TRUE)
    {
        pst_hmac_device->sys_tcp_tx_ack_opt_enable = OAL_FALSE;
    }
    else
    {
        /* 不做处理 */
    }
}

#endif   /* end of _PRE_WLAN_TCP_OPT */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

