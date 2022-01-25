

#ifndef __HMAC_SINGLE_PROXYSTA_H__
#define __HMAC_SINGLE_PROXYSTA_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_SINGLE_PROXYSTA
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_vap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SINGLE_PROXYSTA_H

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define HMAC_PROXYSTA_MAP_IPV4_HASHSIZE        MAC_VAP_PROXYSTA_MAP_MAX_VALUE
#define HMAC_PROXYSTA_MAP_IPV6_HASHSIZE        MAC_VAP_PROXYSTA_MAP_MAX_VALUE
#define HMAC_PROXYSTA_MAP_UNKNOW_HASHSIZE        MAC_VAP_PROXYSTA_MAP_UNKNOW_VALUE

#define HMAC_PROXYSTA_MAP_AGING_TIME            120000          /* PROXYST MAP����ϻ�ʱ�� 120s */
#define HMAC_PROXYSTA_MAP_AGING_TRIGGER_TIME    60000           /* MAP�ϻ���ʱ������ʱ�� 60S */
#define HMAC_PROXYSTA_DHCP_BODY_FIX_LEN         236             /* DHCP���Ĺ̶��ֶγ��� */
#define HMAC_PROXYSTA_DHCP_CLIENT_REQUEST       1               /* DHCP�ͻ��������� */
#define HMAC_PROXYSTA_DHCP_SERVER_RESPONSE      0               /* DHCP��������Ӧ���� */
#define HMAC_PROXYSTA_IPV6_ADDR_LEN             16              /* IPV6��ַ���� */
#define HMAC_PROXYSTA_ICMPV6_HEADER_VALUE       0x3a            /* ICMPV6��nextheadֵ */

#define HMAC_PROXYSTA_MEMCMP_EQUAL              0               /* ����ָ�����ݱȽ�:��� */
#define HMAC_PROXYSTA_MAP_MAX_NUM               128             /* MAP����С����ǰ����Ϊ128 */

#define HMAC_ETHERTYPE_PROTOCOL_START          0x0600           /* ��̫��Э��֡��ʼֵ */

#define DHCP_PORT_BOOTPS   0x0043  /* DHCP ����˽��ձ��Ķ˿�,���ͻ��˷��ͱ��ĵ�Ŀ�Ķ˿� */
#define DHCP_PORT_BOOTPC   0x0044  /* DHCP �ͻ��˽��ձ��Ķ˿�,�����������ͱ��ĵ�Ŀ�Ķ˿�*/

#define DHCP_FLAG_BCAST    0x8000       /* ����DHCP�������Թ㲥��ʽ�ظ�DHCP response */

#define PROTOCOL_ICMPV6    0x3a         /* IPV6�����е�ICMPV6����Э�� */
#define PROTOCOL_ICMPV6_ROUTER_AD_OFFLOAD    8   /* �ھ�ͨ�汨��option�ֶ���ICMPV6ͷ��ƫ�Ƴ��� */

/* ��ϣ�������� */
#define HMAC_PROXYSTA_CAL_IPV4_HASH(_puc_ip_addr)     \
    ((_puc_ip_addr)[ETH_TARGET_IP_ADDR_LEN - 1] & (HMAC_PROXYSTA_MAP_IPV4_HASHSIZE - 1))

#define HMAC_PROXYSTA_CAL_IPV6_HASH(_puc_ip_addr)     \
    ((_puc_ip_addr)[ETH_TARGET_IP_ADDR_LEN - 1] & (HMAC_PROXYSTA_MAP_IPV6_HASHSIZE - 1))

#define HMAC_PROXYSTA_CAL_UNKNOW_HASH(_us_protocol)     \
    (_us_protocol & (HMAC_PROXYSTA_MAP_UNKNOW_HASHSIZE - 1))

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
typedef struct
{
    oal_dlist_head_stru     st_entry;
    oal_uint8               auc_ipv4[ETH_TARGET_IP_ADDR_LEN];       /* ��¼��Ӧ��ipv4��ַ */
    oal_uint8               auc_mac[WLAN_MAC_ADDR_LEN];             /* ��¼��Ӧ��mac��ַ */
    oal_uint8               auc_rsv[2];
    oal_uint32              ul_last_active_timestamp;               /* ���һ�η������ݵ�ʱ�� */
}hmac_proxysta_ipv4_hash_stru;

typedef struct
{
    oal_dlist_head_stru     st_entry;
    oal_uint8               auc_ipv6[HMAC_PROXYSTA_IPV6_ADDR_LEN];  /* ��¼��Ӧ��ipv6��ַ */
    oal_uint8               auc_mac[WLAN_MAC_ADDR_LEN];             /* ��¼��Ӧ��mac��ַ */
    oal_uint8               auc_rsv[2];
    oal_uint32              ul_last_active_timestamp;            /* ���һ�η������ݵ�ʱ�� */
}hmac_proxysta_ipv6_hash_stru;

typedef struct
{
    oal_dlist_head_stru     st_entry;
    oal_uint16              us_protocol;                             /* ��¼��Ӧ��δ֪Э������ */
    oal_uint8               auc_mac[WLAN_MAC_ADDR_LEN];             /* ��¼��Ӧ��mac��ַ */
    oal_uint32              ul_last_active_timestamp;            /* ���һ�η������ݵ�ʱ�� */
}hmac_proxysta_unknow_hash_stru;

typedef struct
{
  union {
            oal_uint8        auc_addr8[16];
            oal_uint16       aus_addr16[8];
            oal_uint32       aul_addr32[4];
        } union_ipv6;
#define auc_ipv6_union_addr        union_ipv6.auc_addr8
#define aus_ipv6_union_addr        union_ipv6.aus_addr16
#define aul_ipv6_union_addr        union_ipv6.aul_addr32
}oal_ipv6_addr_stru;

struct oal_ipv6_header {
#if (_PRE_LITTLE_CPU_ENDIAN == _PRE_CPU_ENDIAN)            /* LITTLE_ENDIAN */
    oal_uint32      ul_flow_label:20,
                    uc_traffic_class:8,
                    uc_version:4;
#else
    oal_uint32      uc_version:4,
                    uc_traffic_class:8,
                    ul_flow_label:20;
#endif
    oal_uint16      us_payload_len;
    oal_uint8       uc_nexthdr;
    oal_uint8       uc_hop_limit;

    oal_ipv6_addr_stru       st_saddr;
    oal_ipv6_addr_stru       st_daddr;
}__OAL_DECLARE_PACKED;
typedef struct oal_ipv6_header oal_ipv6_header_stru;

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/

/*****************************************************************************
  10 ��������
*****************************************************************************/

/* ��ʼ��Proxy STAָ�룬����MAP����ϻ���ʱ�� */
extern oal_uint32  hmac_proxysta_init_vap(hmac_vap_stru *pst_hmac_vap, mac_cfg_add_vap_param_stru *pst_param);

/* �ͷ�Proxy STAָ�룬�ر�MAP����ϻ���ʱ�� */
extern oal_uint32  hmac_proxysta_exit_vap(hmac_vap_stru *pst_hmac_vap);

/* �������ݱ��Ĵ��� �滻�����е�Ŀ��MAC��ַ��ת��LAN */
extern oal_uint32  hmac_proxysta_rx_process(oal_netbuf_stru *pst_buf, hmac_vap_stru *pst_hmac_vap);

/* �������ݱ��Ĵ��� �滻�����е�ԴMAC��ַ ת�����ͽӿ� */
extern oal_uint32  hmac_proxysta_tx_process(oal_netbuf_stru *pst_buf, hmac_vap_stru *pst_hmac_vap);

#endif  //_PRE_WLAN_FEATURE_SINGLE_PROXYSTA

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


#endif /* end of hmac_unique_proxysta.h */
