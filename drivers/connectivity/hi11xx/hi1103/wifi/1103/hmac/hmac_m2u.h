

#ifndef __HMAC_M2U_H__
#define __HMAC_M2U_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(_PRE_WLAN_FEATURE_MCAST) || defined(_PRE_WLAN_FEATURE_HERA_MCAST)

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "oal_net.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "mac_frame.h"
#include "hmac_main.h"
#include "hmac_user.h"
#include "hmac_vap.h"
#include "frw_ext_if.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_M2U_H
/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define HMAC_M2U_GRPADDR_FILTEROUT_NUM 8
#define HMAC_M2U_DENY_GROUP 4026531585UL     /* �鲥������� 239.255.255.1 */
#define HMAC_M2U_MIN_DENY_GROUP 3758096384UL /* ��С�鲥���ַ 224.0.0.0 */
#define HMAC_M2U_MAX_DENY_GROUP 4026531839UL /* ����鲥���ַ 239.255.255.255 */
#define HMAC_M2U_SPECIAL_GROUP1 3758096385UL    /* SPECIAL  GROUP1 224.0.0.1 */
#define HMAC_M2U_SPECIAL_GROUP2 3758096386UL    /* SPECIAL  GROUP2 224.0.0.2 */
#define HMAC_M2U_RIPV2_GROUP 3758096393UL       /* RIPV2  GROUP 224.0.0.9 */
#define HMAC_M2U_SPECIAL_GROUP3 3758096406UL    /* SPECIAL  GROUP3 224.0.0.22 */
#define HMAC_M2U_UPNP_GROUP 4026531834UL        /* UPNP GROUP 239.255.255.250 */

#define DEFAULT_IPV4_DENY_GROUP_COUNT 1      /* Ĭ�϶������1��ipv4�鲥������*/
#define DEFAULT_IPV6_DENY_GROUP_COUNT 1      /* Ĭ�����1��ipv6�鲥������*/
#define SPECIAL_M2U_GROUP_COUNT_IPV4  5      /* Ĭ�����5������ipv4ҵ���鲥*/
#define SPECIAL_M2U_GROUP_COUNT_IPV6  5      /* Ĭ�����5������ipv6ҵ���鲥*/
#define HMAC_M2U_ADAPTIVE_STA_HASHSIZE 16    /* ��������ͳ��HASHͰΪ16*/


#define MAC_ETH_PROTOCOL_SUBTYPE  0x17
#define OAL_SNAP_LEN              8    /* SNAP ͷ�ĳ��� */
#define MIN_IP_HDR_LEN            5    /* ��СIPͷ���� */

#define HMAC_DEF_M2U_TIMER        30000     /* timer interval as 30 secs */
#define HMAC_DEF_M2U_TIMEOUT      120000    /* 2 minutes for timeout  */

#define HMAC_DEF_ADAPTIVE_TIMEOUT      1000      /* ����ģʽ�ϻ�ʱ��  */
#define HMAC_DEF_THRESHOLD_TIME        500       /* ����ģʽ����ʱ��  */
#define HMAC_DEF_NUM_OF_ADAPTIVE       16        /* ����ģʽ���޸���  */

#define ETHER_TYPE_VLAN_88A8      0x88a8  /* VLAN TAG TPID ������Ӫ�̵���� */
#define ETHER_TYPE_VLAN_9100      0x9100  /* VLAN TAG TPID */
#define MAX_STA_NUM_OF_ALL_GROUP  1000    /*���1000��Ҷ�ӽڵ�(sta����)*/
#define MAX_STA_NUM_OF_ADAPTIVE   128     /*���128������sta���� */

#define oal_set_ip_addr(addr1, addr2) oal_memcopy((addr1), (addr2), 16UL)

#define ETHER_IS_WITH_VLAN_TAG(_a) \
        (((_a) == OAL_HOST2NET_SHORT(ETHER_TYPE_VLAN_88A8)) ||    \
         ((_a) == OAL_HOST2NET_SHORT(ETHER_TYPE_VLAN_9100)) ||    \
         ((_a) == OAL_HOST2NET_SHORT(ETHER_TYPE_VLAN)))

#define OAL_IS_MDNSV4_MAC(_a,_b)   (((oal_uint8)((_a)[0]) == 0x01) && ((oal_uint8)((_a)[1]) == 0x00) && ((oal_uint8)((_a)[2]) == 0x5e) && \
                                   ((oal_uint8)((_a)[3]) == 0x00) && ((oal_uint8)((_a)[4]) == 0x00) && ((oal_uint8)((_a)[5]) == 0xfb) && \
                                   ((_b) == OAL_HOST2NET_SHORT(ETHER_TYPE_IP)))
#define OAL_IS_MDNSV6_MAC(_a,_b)   (((oal_uint8)((_a)[0]) == 0x33) && ((oal_uint8)((_a)[1]) == 0x33) && ((oal_uint8)((_a)[2]) == 0x00) && \
                                   ((oal_uint8)((_a)[3]) == 0x00) && ((oal_uint8)((_a)[4]) == 0x00) && ((oal_uint8)((_a)[5]) == 0xfb) && \
                                   ((_b) == OAL_HOST2NET_SHORT(ETHER_TYPE_IPV6)))
#define OAL_IS_ICMPV6_PROTO(_a,_b) (((_a) == OAL_HOST2NET_SHORT(ETHER_TYPE_IPV6)) && (((oal_uint8)((_b)[6]) == 0x3a) || ((oal_uint8)((_b)[40]) == 0x3a)))
#define OAL_IS_IGMP_PROTO(_a,_b)   (((_a) == OAL_HOST2NET_SHORT(ETHER_TYPE_IP)) && ((oal_uint8)((_b)[9]) == 0x02))

#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
/* �������Ĺ�ϣ�������� */
#define HMAC_ADAPTIVE_CAL_HASH_VALUE(_puc_mac_addr)     \
    ((_puc_mac_addr)[ETHER_ADDR_LEN - 1] & (HMAC_M2U_ADAPTIVE_STA_HASHSIZE - 1))
#endif



#if 0
#define BITFIELD_LITTLE_ENDIAN      0
#define BITFIELD_BIG_ENDIAN         1
#endif
/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
typedef enum
{
    HMAC_M2U_CMD_EXCLUDE_LIST  = 0,
    HMAC_M2U_CMD_INCLUDE_LIST  = 1,

    HMAC_M2U_IGMP_CMD_BUTT
}hmac_m2u_update_cmd_enum;
typedef oal_uint8 hmac_m2u_igmp_cmd_enum_uint8;

typedef enum
{
    HMAC_M2U_MCAST_MAITAIN         = 0,
    HMAC_M2U_MCAST_TUNNEL          = 1,
    HMAC_M2U_MCAST_TRANSLATE       = 2,

    HMAC_M2U_MCAST_BUTT
}hmac_m2u_mcast_mode_enum;
typedef oal_uint8 hmac_m2u_mcast_mode_enum_uint8;

typedef struct
{
    #if (OAL_BYTE_ORDER == OAL_LITTLE_ENDIAN)
	oal_uint32  offset	                : 11,
	            seqNum       		    : 11,
	            optHdrLen32  		    : 2,
	            frameType    		    : 2,
	            proto        		    : 6;
    #else /* big endian */
	oal_uint32  proto	                : 6,
	            frameType    		    : 2,
	            optHdrLen32  		    : 2,
	            seqNum       		    : 11,
	            offset       		    : 11;
	#endif
}mcast_tunnel_hdr_stru;

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

/* hmac_m2u_grp_list_entryͨ������Ĳ�ͬ��ַ���洢�飬�ҵ�snoop_list�� */
////�鲥��ṹ��
typedef struct
{
    oal_dlist_head_stru               st_src_list;                        /* �鲥���ڳ�Ա������ͷ */
    oal_uint8                         auc_group_mac[WLAN_MAC_ADDR_LEN];   /* ����鲥�����mac��ַ */
    oal_uint8                         uc_sta_num;                         /*��¼��ǰ�鲥����sta����*/
    oal_uint8                         uc_reserve;
    oal_dlist_head_stru               st_grp_entry;
    mac_vlan_tag_stru                 st_outer_vlan_tag;                  /*  ���vlan tag*/
    mac_vlan_tag_stru                 st_inner_vlan_tag;                  /*  �ڲ�vlan tag*/
}hmac_m2u_grp_list_entry_stru;

/* �����鲥���snoop����ṹ */
typedef struct
{
    oal_uint32               aul_deny_group[HMAC_M2U_GRPADDR_FILTEROUT_NUM];
    oal_uint8                aul_deny_group_ipv6[HMAC_M2U_GRPADDR_FILTEROUT_NUM][OAL_IPV6_ADDR_SIZE];
    oal_dlist_head_stru      st_grp_list;            /* ������ͷ */
    oal_uint16               us_group_list_count;    /*�鲥�����*/
    oal_uint16               us_misc;
    oal_uint16               us_total_sta_num;       /*��¼��ǰ���鲥����sta����*/
    oal_uint8                uc_deny_count_ipv4;     /* ipv4����������*/
    oal_uint8                uc_deny_count_ipv6;     /* ipv6����������*/
    oal_uint32               aul_special_group_ipv4[SPECIAL_M2U_GROUP_COUNT_IPV4]; /* ipv4�����鲥IP*/
    oal_uint8                aul_special_group_ipv6[SPECIAL_M2U_GROUP_COUNT_IPV6][OAL_IPV6_ADDR_SIZE]; /* ipv6�����鲥IP*/
}hmac_m2u_snoop_list_stru;

/**
 * hmac_m2u_grp_member_stru �����洢һ�����Ա����ϸ��Ϣ
 * ÿһ����Աӵ��
 * ul_src_ip_addr - Դip��ַ
 * grp_member_address - ���汨�ķ����ߵĵ�ַ
 * mode - include / exclude src_ip_address.
 **/
  ////���ڳ�Ավ����Ϣ (STA )
typedef struct
{
    oal_uint8                        auc_src_ip_addr[OAL_IPV6_ADDR_SIZE];    //�鲥ԴIP��ַ
    oal_uint32                       ul_timestamp;
    hmac_user_stru                  *pst_hmac_user;
    oal_uint8                        auc_grp_member_mac[WLAN_MAC_ADDR_LEN]; //��STA MAC ��ַ
    oal_uint8                        en_mode;
    oal_uint8                        uc_src_ip_addr_len;                    //ip ��ַ���ȣ����ڼ���IPV4��IPV6
    oal_dlist_head_stru              st_member_entry;
}hmac_m2u_grp_member_stru;



/* hmac_m2u_list_update_stru �ṹ�������ݲ�����list update����������ض�group�ĳ�Ա���� */
typedef struct
{
    oal_uint8                        auc_src_ip_addr[OAL_IPV6_ADDR_SIZE];   /* Դ��ַ */
    oal_uint8                        auc_grp_mac[WLAN_MAC_ADDR_LEN];        /* ��Ҫ������鲥��mac��ַ */
    oal_uint8                        auc_new_member_mac[WLAN_MAC_ADDR_LEN]; /* ��Ҫ���и��µ��鲥��Աmac��ַ */
    oal_uint32                       ul_timestamp;                          /* ʱ��� */
    hmac_vap_stru                   *pst_hmac_vap;                          /* vapָ�� */
    hmac_user_stru                  *pst_hmac_user;                         /* userָ�� */
    mac_vlan_tag_stru                st_outer_vlan_tag;                     /* ���vlan tag*/
    mac_vlan_tag_stru                st_inner_vlan_tag;                     /* �ڲ�vlan tag*/
    hmac_m2u_igmp_cmd_enum_uint8     en_cmd;                                /* ���롢ɾ������ */
    oal_uint8                        uc_src_ip_addr_len;
    oal_uint8                        auc_reserve[2];
}hmac_m2u_list_update_stru;

#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
/* ��������ģʽSTA����ṹ */
typedef struct
{
    oal_dlist_head_stru               st_adaptive_entry;
    oal_uint8                         auc_adaptive_mac[WLAN_MAC_ADDR_LEN];   /* ����STA��mac��ַ */
    oal_uint8                         uc_adaptive_num;                       /* ��¼��ǰ�յ������鲥���ĸ���*/
    oal_bool_enum_uint8               en_m2u_adaptive;                       /* ����ģʽ�о� */
    oal_uint32                        ul_timestamp;
    mac_vlan_tag_stru                 st_outer_vlan_tag;                     /* ���vlan tag */
    mac_vlan_tag_stru                 st_inner_vlan_tag;                     /* �ڲ�vlan tag */
}hmac_m2u_adaptive_hash_stru;

typedef struct
{
    oal_uint8                        auc_new_member_mac[WLAN_MAC_ADDR_LEN]; /* ��Ҫ���и��µ������豸mac��ַ */
    oal_uint8                        auc_reserve[2];
    oal_uint32                       ul_timestamp;                          /* ʱ��� */
    hmac_vap_stru                   *pst_hmac_vap;                          /* vapָ�� */
    mac_vlan_tag_stru                st_outer_vlan_tag;                     /* ���vlan tag*/
    mac_vlan_tag_stru                st_inner_vlan_tag;                     /* �ڲ�vlan tag*/
}hmac_m2u_adaptive_list_update_stru;
#endif

/*��������snoop����*/
typedef struct
{
    hmac_m2u_snoop_list_stru        st_m2u_snooplist;
    oal_bool_enum_uint8             en_snoop_enable;    /* �����鲥ת�����Ƿ�ʹ�� */
    hmac_m2u_mcast_mode_enum_uint8  en_mcast_mode;      /* �����鲥֡�ķ��ͷ�ʽ */
    oal_bool_enum_uint8             en_discard_mcast;   /* �����鲥֡�Ƿ�ֱ�Ӷ��� */
    wlan_tidno_enum_uint8           en_tid_num;
    frw_timeout_stru                st_snooplist_timer;
    oal_uint32                      ul_timeout;         /*�鲥���Ա��Ĭʱ��*/
#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
    oal_dlist_head_stru             ast_m2u_adaptive_hash[HMAC_M2U_ADAPTIVE_STA_HASHSIZE];/* ������������ͳ��HASH�� */
    oal_bool_enum_uint8             en_frequency_enable;/* ������Ƶ�鲥֡ת������ʹ�� */
    oal_bool_enum_uint8             en_adaptive_enable; /* ��������ģʽʶ��ʹ�� */
    frw_timeout_stru                st_adaptivelist_timer;
    oal_uint32                      ul_threshold_time;   /* ����ģʽ����ʱ�� */
    oal_uint32                      ul_adaptive_timeout; /* ����ģʽ�ϻ�ʱ��*/
    oal_uint8                       uc_adaptive_num;     /* ����ģʽ���޸���*/
    oal_uint8                       uc_adaptive_sta_count; /* ����STA���� */
    oal_uint8                       auc_reserve[2];
#endif
}hmac_m2u_stru;
/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
extern oal_uint32 hmac_m2u_snoop_convert( hmac_vap_stru *pst_vap, oal_netbuf_stru *pst_buf);
extern oal_void hmac_m2u_snoop_inspecting(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user, oal_netbuf_stru *pst_buf);
extern oal_void hmac_m2u_attach(hmac_vap_stru *pst_hmac_vap);
extern oal_void hmac_m2u_detach(hmac_vap_stru * pst_hmac_vap);
extern oal_uint32 hmac_m2u_update_snoop_list(hmac_m2u_list_update_stru *pst_list_entry);
extern oal_void hmac_m2u_add_snoop_ipv4_deny_entry(hmac_vap_stru *pst_hmac_vap, oal_uint32 *ul_grpaddr);
extern oal_void hmac_m2u_add_snoop_ipv6_deny_entry(hmac_vap_stru *pst_hmac_vap, oal_uint8 *ul_grpaddr);
extern oal_void hmac_m2u_del_ipv4_deny_entry(hmac_vap_stru *pst_hmac_vap, oal_uint32 *pul_grpaddr);
extern oal_void hmac_m2u_del_ipv6_deny_entry(hmac_vap_stru *pst_hmac_vap, oal_uint8 *ul_grpaddr);
extern oal_void hmac_m2u_clear_deny_table(hmac_vap_stru *pst_hmac_vap);
extern oal_void hmac_m2u_show_snoop_deny_table(hmac_vap_stru *pst_hmac_vap);
extern oal_uint32 hmac_m2u_print_all_snoop_list(hmac_vap_stru *pst_hmac_vap, oal_snoop_all_group_stru *pst_snoop_all_grp);
extern oal_void hmac_m2u_cleanup_snoopwds_node(hmac_user_stru *pst_hmac_user);
extern oal_uint32 hmac_m2u_igmp_v1v2_update(hmac_vap_stru *pst_hmac_vap, hmac_m2u_list_update_stru *pst_list_entry, mac_igmp_header_stru *pst_igmp);
#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
extern oal_uint32 hmac_m2u_multicast_drop(hmac_vap_stru *pst_vap, oal_netbuf_stru *pst_buf);
extern oal_void hmac_m2u_adaptive_inspecting(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_buf);
extern oal_void hmac_m2u_unicast_convert_multicast( hmac_vap_stru *pst_vap, oal_netbuf_stru *pst_buf, dmac_msdu_stru *pst_msdu);
#endif


#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of hmac_m2u.h */
