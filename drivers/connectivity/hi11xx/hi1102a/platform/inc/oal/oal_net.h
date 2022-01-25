

#ifndef __OAL_NET_H__
#define __OAL_NET_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "platform_spec.h"
#include "oal_types.h"
#include "oal_mm.h"
#include "oal_util.h"
#include "oal_schedule.h"
#include "oal_list.h"
#include "arch/oal_net.h"
/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define OAL_IF_NAME_SIZE   16
#define OAL_NETBUF_DEFAULT_DATA_OFFSET 48  /* 5115��ʵ�ʲ��data��head��48������netbuf dataָ�븴λ */

#define OAL_ASSOC_REQ_IE_OFFSET        28    /* �ϱ��ں˹�������֡ƫ���� */
#define OAL_ASSOC_RSP_IE_OFFSET        30    /* �ϱ��ں˹�����Ӧ֡ƫ���� */
#define OAL_AUTH_IE_OFFSET             30
#define OAL_FT_ACTION_IE_OFFSET        40
#define OAL_ASSOC_RSP_FIXED_OFFSET     6     /* ������Ӧ֡֡��FIXED PARAMETERSƫ���� */
#define OAL_MAC_ADDR_LEN               6
#define OAL_PMKID_LEN                  16
#define OAL_WPA_KEY_LEN                32
#define OAL_WPA_SEQ_LEN                16
#define OAL_WLAN_SA_QUERY_TR_ID_LEN    2
#define OAL_MAX_FT_ALL_LEN             518   /* MD:5 FT:257 RSN:256 */
#if defined(_PRE_WLAN_FEATURE_MCAST) || defined(_PRE_WLAN_FEATURE_HERA_MCAST)
#define MAX_STA_NUM_OF_ONE_GROUP    32      /*һ���鲥�����֧��32���û�*/
#define MAX_NUM_OF_GROUP            256     /*���֧��256���鲥��*/
#endif
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
#define MAX_BLACKLIST_NUM          128
#endif
/*
 * Byte order/swapping support.
 */
#define OAL_LITTLE_ENDIAN    1234
#define OAL_BIG_ENDIAN       4321
#define OAL_BYTE_ORDER OAL_BIG_ENDIAN

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && defined (_PRE_WLAN_FEATURE_DFR)
/* hi1102  �����ں�ע��˿�28 NETLINK_HISI_WIFI_MSG�ѱ�ռ�ã����Դ˴�ʹ��δʹ�õ�27 NETLINK_HISI_WIFI_PMF */
#define NETLINK_DEV_ERROR 27
#endif
/*****************************************************************************
  2.10 IP�궨��
*****************************************************************************/

#define WLAN_DSCP_PRI_SHIFT         2
#define WLAN_IP_PRI_SHIFT           5
#define WLAN_IPV6_PRIORITY_MASK     0x0FF00000
#define WLAN_IPV6_PRIORITY_SHIFT    20

/*****************************************************************************
  2.11 VLAN�궨��
*****************************************************************************/


/*****************************************************************************
  2.12 LLC SNAP�궨��
*****************************************************************************/
#define LLC_UI                  0x3
#define SNAP_LLC_FRAME_LEN      8
#define SNAP_LLC_LSAP           0xaa
#define SNAP_RFC1042_ORGCODE_0  0x00
#define SNAP_RFC1042_ORGCODE_1  0x00
#define SNAP_RFC1042_ORGCODE_2  0x00
#define SNAP_BTEP_ORGCODE_0     0x00
#define SNAP_BTEP_ORGCODE_1     0x00
#define SNAP_BTEP_ORGCODE_2     0xf8

/*****************************************************************************
  2.13 ETHER�궨��
*****************************************************************************/
#define ETHER_ADDR_LEN  6   /* length of an Ethernet address */
#define ETHER_TYPE_LEN  2   /* length of the Ethernet type field */
#define ETHER_CRC_LEN   4   /* length of the Ethernet CRC */
#define ETHER_HDR_LEN   14
#define ETHER_MAX_LEN   1518
#define ETHER_MTU        (ETHER_MAX_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)

/* #ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY */
OAL_STATIC OAL_INLINE oal_uint8 a2x(const char c)
{
    if (c >= '0' && c <= '9')
    {
        return (oal_uint8)oal_atoi(&c);
    }
    if (c >= 'a' && c <= 'f')
    {
        return (oal_uint8)0xa + (oal_uint8)(c-'a');
    }
    if (c >= 'A' && c <= 'F')
    {
        return (oal_uint8)0xa + (oal_uint8)(c-'A');
    }
    return 0;
}

/*convert a string,which length is 18, to a macaddress data type.*/
#define MAC_STR_LEN 18
#define COPY_STR2MAC(mac,str)  \
    do { \
        oal_uint8 i; \
        for(i = 0; i < OAL_MAC_ADDR_LEN; i++) {\
            mac[i] = (oal_uint8)(a2x(str[i*3]) << 4) + a2x(str[i*3 + 1]);\
        }\
    } while(0)
/* #endif */

/* ipͷ��Э�������ֶε�ƫ�� */
#define IP_PROTOCOL_TYPE_OFFSET  9
#define IP_HDR_LEN               20

/* CCMP�����ֽ��� */
#define WLAN_CCMP_ENCRYP_LEN 16
/* CCMP256�����ֽ��� */
#define WLAN_CCMP256_GCMP_ENCRYP_LEN 24

/* is address mcast? */
#define ETHER_IS_MULTICAST(_a)   (*(_a) & 0x01)

/* is address bcast? */
#define ETHER_IS_BROADCAST(_a)   \
    ((_a)[0] == 0xff &&          \
     (_a)[1] == 0xff &&          \
     (_a)[2] == 0xff &&          \
     (_a)[3] == 0xff &&          \
     (_a)[4] == 0xff &&          \
     (_a)[5] == 0xff)
/* is address all zero? */
#define ETHER_IS_ALL_ZERO(_a)    \
    ((_a)[0] == 0x00 &&          \
     (_a)[1] == 0x00 &&          \
     (_a)[2] == 0x00 &&          \
     (_a)[3] == 0x00 &&          \
     (_a)[4] == 0x00 &&          \
     (_a)[5] == 0x00)
#define ETHER_IS_IPV4_MULTICAST(_a)  ((_a[0]) == 0x01 &&    \
                                      (_a[1]) == 0x00 &&    \
                                      (_a[2]) == 0x5e)

#define WLAN_DATA_VIP_TID              WLAN_TIDNO_BCAST

/* wiphy  */
#define IEEE80211_HT_MCS_MASK_LEN   10

/* ICMP codes for neighbour discovery messages */
#define OAL_NDISC_ROUTER_SOLICITATION       133
#define OAL_NDISC_ROUTER_ADVERTISEMENT      134
#define OAL_NDISC_NEIGHBOUR_SOLICITATION    135
#define OAL_NDISC_NEIGHBOUR_ADVERTISEMENT   136
#define OAL_NDISC_REDIRECT                  137

#define OAL_ND_OPT_TARGET_LL_ADDR           2
#define OAL_ND_OPT_SOURCE_LL_ADDR           1
#define OAL_IPV6_ADDR_ANY                   0x0000U
#define OAL_IPV6_ADDR_MULTICAST             0x0002U
#define OAL_IPV6_MAC_ADDR_LEN               16


#define OAL_IPV4_ADDR_SIZE                    4
#define OAL_IPV6_ADDR_SIZE                    16
#define OAL_IP_ADDR_MAX_SIZE                  OAL_IPV6_ADDR_SIZE


/* IPv4�ಥ��Χ: 224.0.0.0--239.255.255.255 */
#define OAL_IPV4_IS_MULTICAST(_a)             ((oal_uint8)((_a)[0]) >= 224 && ((oal_uint8)((_a)[0]) <= 239))

/* IPv4�������ַ�ж�: 224.0.0.0��224.0.0.255Ϊ�������ַ */
#define OAL_IPV4_PERMANET_GROUP_ADDR           0x000000E0
#define OAL_IPV4_IS_PERMANENT_GROUP(_a)       ((((_a) & 0x00FFFFFF) ^ OAL_IPV4_PERMANET_GROUP_ADDR) == 0)

/* IPv6�鲥��ַ: FFXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX(��һ���ֽ�ȫһ) */
#define OAL_IPV6_IS_MULTICAST(_a)             ((oal_uint8)((_a)[0]) == 0xff)
/*ipv6 �鲥mac��ַ */
#define ETHER_IS_IPV6_MULTICAST(_a)  (((_a)[0]) == 0x33 && ((_a)[1]) == 0x33)
/* IPv6δָ����ַ: ::/128 ,�õ�ַ�����ڽӿڻ�û�б�����IPv6��ַʱ�������ڵ�
   ͨѶ��ΪԴ��ַ,�������ظ���ַ���DAD�л����. */
#define OAL_IPV6_IS_UNSPECIFIED_ADDR(_a)   \
     ((_a)[0]  == 0x00 &&          \
      (_a)[1]  == 0x00 &&          \
      (_a)[2]  == 0x00 &&          \
      (_a)[3]  == 0x00 &&          \
      (_a)[4]  == 0x00 &&          \
      (_a)[5]  == 0x00 &&          \
      (_a)[6]  == 0x00 &&          \
      (_a)[7]  == 0x00 &&          \
      (_a)[8]  == 0x00 &&          \
      (_a)[9]  == 0x00 &&          \
      (_a)[10] == 0x00 &&          \
      (_a)[11] == 0x00 &&          \
      (_a)[12] == 0x00 &&          \
      (_a)[13] == 0x00 &&          \
      (_a)[14] == 0x00 &&          \
      (_a)[15] == 0x00)


/* IPv6��·���ص�ַ: ���10λֵΪ1111111010, ����:FE80:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX  */
#define OAL_IPV6_IS_LINK_LOCAL_ADDR(_a)       (((_a)[0] == 0xFE) && ((_a)[1] >> 6 == 2))


/* IGMP record type */
#define MAC_IGMP_QUERY_TYPE       0x11
#define MAC_IGMPV1_REPORT_TYPE    0x12
#define MAC_IGMPV2_REPORT_TYPE    0x16
#define MAC_IGMPV2_LEAVE_TYPE     0x17
#define MAC_IGMPV3_REPORT_TYPE    0x22

/* Is packet type is either leave or report */
#define IS_IGMP_REPORT_LEAVE_PACKET(type) (\
    (MAC_IGMPV1_REPORT_TYPE == type)\
    || (MAC_IGMPV2_REPORT_TYPE == type)\
    || (MAC_IGMPV2_LEAVE_TYPE  == type)\
    || (MAC_IGMPV3_REPORT_TYPE == type)\
                                         )

/* V3 group record types [grec_type] */
#define IGMPV3_MODE_IS_INCLUDE        1
#define IGMPV3_MODE_IS_EXCLUDE        2
#define IGMPV3_CHANGE_TO_INCLUDE      3
#define IGMPV3_CHANGE_TO_EXCLUDE      4
#define IGMPV3_ALLOW_NEW_SOURCES      5
#define IGMPV3_BLOCK_OLD_SOURCES      6

/* Is packet type is either leave or report */
#define IS_IGMPV3_MODE(type) (\
    (IGMPV3_MODE_IS_INCLUDE == type)\
    || (IGMPV3_MODE_IS_EXCLUDE == type)\
    || (IGMPV3_CHANGE_TO_INCLUDE  == type)\
    || (IGMPV3_CHANGE_TO_EXCLUDE == type)\
    || (IGMPV3_ALLOW_NEW_SOURCES == type)\
    || (IGMPV3_BLOCK_OLD_SOURCES == type)\
                                          )

/* IGMP record type */
#define MLD_QUERY_TYPE            130
#define MLDV1_REPORT_TYPE      131
#define MLDV1_DONE_TYPE         132
#define MLDV2_REPORT_TYPE        143

/* Is packet type is either leave or report */
#define IS_MLD_REPORT_LEAVE_PACKET(type) (\
    (MLDV1_REPORT_TYPE == type)\
    || (MLDV1_DONE_TYPE == type)\
    || (MLDV2_REPORT_TYPE  == type)\
                                         )
/* MLD V2 group record types [grec_type] */
#define MLDV2_MODE_IS_INCLUDE        1
#define MLDV2_MODE_IS_EXCLUDE        2
#define MLDV2_CHANGE_TO_INCLUDE      3
#define MLDV2_CHANGE_TO_EXCLUDE      4
#define MLDV2_ALLOW_NEW_SOURCES      5
#define MLDV2_BLOCK_OLD_SOURCES      6

#define IS_MLDV2_MODE(type) (\
    (MLDV2_MODE_IS_INCLUDE == type)\
    || (MLDV2_MODE_IS_EXCLUDE == type)\
    || (MLDV2_CHANGE_TO_INCLUDE  == type)\
    || (MLDV2_CHANGE_TO_EXCLUDE == type)\
    || (MLDV2_ALLOW_NEW_SOURCES == type)\
    || (MLDV2_BLOCK_OLD_SOURCES == type)\
                                          )

/* Calculate the group record length*/
//#define IGMPV3_GRP_REC_LEN(x) (8 + (4 * x->us_grec_nsrcs) + (4 * x->uc_grec_auxwords) )
#define IGMPV3_GRP_REC_LEN(x) (8 + (((x)->us_grec_nsrcs + (x)->uc_grec_auxwords)<<2))
//#define MLDV2_GRP_REC_LEN(x) (OAL_SIZEOF(mac_mld_v2_group_record_stru)+ (OAL_IPV6_ADDR_SIZE * (x)->us_grec_srcaddr_num) + (x)->uc_grec_auxwords )
#define MLDV2_GRP_REC_LEN(x) (OAL_SIZEOF(mac_mld_v2_group_record_stru)+ ( (x)->us_grec_srcaddr_num<<4) + (x)->uc_grec_auxwords )

/*Probe Rsp APP IE���ȳ�����ֵ������֡netbuf���ô��*/
#define OAL_MGMT_NETBUF_APP_PROBE_RSP_IE_LEN_LIMIT  450

#ifdef _PRE_WLAN_FEATURE_SPECIAL_PKT_LOG
/* dhcp */
#define DHCP_CHADDR_LEN         16
#define SERVERNAME_LEN          64
#define BOOTFILE_LEN            128

/* DHCP message type */
#define DHCP_DISCOVER           1
#define DHCP_OFFER              2
#define DHCP_REQUEST            3
#define DHCP_ACK                5
#define DHCP_NAK                6

#define DHO_PAD                 0
#define DHO_IPADDRESS           50
#define DHO_MESSAGETYPE         53
#define DHO_SERVERID            54
#define DHO_END                 255

#define DNS_GET_QR_FROM_FLAG(flag)        ((oal_uint8)(((flag & 0x8000U) > 0)? 1 : 0))
#define DNS_GET_OPCODE_FROM_FLAG(flag)    ((oal_uint8)((flag & 0x7400U) >> 11))
#define DNS_GET_RCODE_FROM_FLAG(flag)     ((oal_uint8)(flag & 0x000fU))

#define DNS_MAX_DOMAIN_LEN  (100)

#define DHCP_SERVER_PORT    (67)
#define DHCP_CLIENT_PORT    (68)
#define DNS_SERVER_PORT     (53)
#endif

 /*****************************************************************************
   ö����  : oal_mem_state_enum_uint8
   Э����:
   ö��˵��: �ڴ��״̬
 *****************************************************************************/
typedef enum
{
    OAL_MEM_STATE_FREE  = 0,            /* ���ڴ���� */
    OAL_MEM_STATE_ALLOC,                /* ���ڴ��ѷ��� */

    OAL_MEM_STATE_BUTT
}oal_mem_state_enum;
typedef oal_uint8 oal_mem_state_enum_uint8;

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
/* ���²����ֲ���ϵͳ */
/* �ں��·���ɨ������ */
typedef enum
{
    OAL_PASSIVE_SCAN        = 0,
    OAL_ACTIVE_SCAN         = 1,

    OAL_SCAN_BUTT
}oal_scan_enum;
typedef oal_uint8 oal_scan_enum_uint8;

/* �ں��·���ɨ��Ƶ�� */
typedef enum
{
    OAL_SCAN_2G_BAND        = 1,
    OAL_SCAN_5G_BAND        = 2,
    OAL_SCAN_ALL_BAND       = 3,

    OAL_SCAN_BAND_BUTT
}oal_scan_band_enum;
typedef oal_uint8 oal_scan_band_enum_uint8;

typedef enum
{
    OAL_IEEE80211_MLME_AUTH     = 0,    /* MLME�·���֤֡������� */
    OAL_IEEE80211_MLME_ASSOC    = 1,    /* MLME�·�����֡������� */
    OAL_IEEE80211_MLME_REASSOC  = 2,   /* MLME�·��ع���֡������� */
    OAL_IEEE80211_MLME_NUM
}en_mlme_type_enum;
typedef oal_uint8 en_mlme_type_enum_uint8;

/* hostapd �·�˽������ */
enum HWIFI_IOCTL_CMD
{
    /*
     *IOCTL_CMD����ʼֵ��0�޸�Ϊ0x8EE0���޸�ԭ��51 WiFiģ���������dhdutil֮�������ģ�鹲��ͬһ��ioctlͨ����
     *��51�����ö��ֵ��0��ʼ������ģ���·���ioctl����Ҳ������0��ʼ���ⲿ�֣������ͻ�ͬʱ���鲥�����Լ���ģ���WiFiģ�飬
     *�Ӷ���WiFiģ��Ĺ��ܲ���Ӱ�졣���Խ�51 WiFiģ�������ö��ֵ������0x8EE0�𣬱���������ģ�������Ӱ�졣
     */
    HWIFI_IOCTL_CMD_GET_STA_ASSOC_REQ_IE = 0x8EE0,       /* get sta assocate request ie */
    HWIFI_IOCTL_CMD_SET_AP_AUTH_ALG,            /* set auth alg to driver */
    HWIFI_IOCTL_CMD_SET_COUNTRY,                /* ���ù����� */
    HWIFI_IOCTL_CMD_SET_SSID,                   /* ����ssid */
    HWIFI_IOCTL_CMD_SET_MAX_USER,               /* ��������û��� */
    HWIFI_IOCTL_CMD_SET_FREQ,                   /* ����Ƶ�� */
    HWIFI_IOCTL_CMD_SET_WPS_IE,                 /* ����AP WPS ��ϢԪ�� */
    HWIFI_IOCTL_CMD_PRIV_CONNECT,               /* linux-2.6.30 sta����connect */
    HWIFI_IOCTL_CMD_PRIV_DISCONNECT,            /* linux-2.6.30 sta����disconnect */
    HWIFI_IOCTL_CMD_SET_FRAG,                   /* ���÷�Ƭ����ֵ */
    HWIFI_IOCTL_CMD_SET_RTS,                    /* ����RTS ����ֵ */
    // _PRE_WLAN_FEATURE_HILINK
    HWIFI_IOCTL_CMD_PRIV_KICK_USER,             /* AP�޳��û� */
    HWIFI_IOCTL_CMD_SET_VENDOR_IE,              /* AP ���˽��IE�ӿڣ�ԭΪHWIFI_IOCTL_CMD_SET_OKC_IE����hilink����okc ieר�� */
    HWIFI_IOCTL_CMD_SET_WHITE_LST_SSIDHIDEN,    /* ����hlink������ */
    HWIFI_IOCTL_CMD_FBT_SCAN,                   /* ������ֹͣhilink fbt����*/
    HWIFI_IOCTL_CMD_GET_ALL_STA_INFO,           /* ��ȡ�����ѹ���STA��·��Ϣ */
    HWIFI_IOCTL_CMD_GET_STA_INFO_BY_MAC,        /* ��ȡָ���ѹ���STA��·��Ϣ */
    HWIFI_IOCTL_CMD_GET_CUR_CHANNEL,            /* ��ȡ�����ŵ� */
    HWIFI_IOCTL_CMD_SET_SCAN_STAY_TIME,         /* ����ɨ��ʱ�����ŵ�פ��ʱ��������ŵ�פ��ʱ�� */
    HWIFI_IOCTL_CMD_SET_BEACON,                 /* hostapdֻ���ü��ܲ���˽�нӿ� */
    //_PRE_WLAN_FEATURE_11R_AP
    HWIFI_IOCTL_CMD_SET_MLME,                   /* ����MLME��������֤�������� */
    HWIFI_IOCTL_CMD_GET_NEIGHB_INFO,            /* ��ȡ�ھ�APɨ����Ϣ */
    HWIFI_IOCTL_CMD_GET_HW_STAT,                /* ��ȡӲ������ͳ�� */
    HWIFI_IOCTL_CMD_GET_WME_STAT,               /* ��ȡWME����ͳ�� */
    HWIFI_IOCTL_CMD_GET_STA_11V_ABILITY,        /* ��ȡָ��STA��11v������Ϣ */
    HWIFI_IOCTL_CMD_11V_CHANGE_AP,              /* ֪ͨsta�л���ָ��ap */
    HWIFI_IOCTL_CMD_GET_STA_11K_ABILITY,        /* ��ȡָ��STA��11v������Ϣ */
    HWIFI_IOCTL_CMD_SET_STA_BCN_REQUEST,        /* ֪ͨSTA���ϱ�beacon��Ϣ */
    HWIFI_IOCTL_CMD_GET_SNOOP_TABLE,            /* ��ȡ�鲥�鼰���Աmac */
    HWIFI_IOCTL_CMD_GET_ALL_STA_INFO_EXT,       /* ��ȡ�����ѹ���STA��·��Ϣ�������������� */
    HWIFI_IOCTL_CMD_GET_VAP_WDS_INFO,           /* ��ȡWDS VAP�ڵ���Ϣ */
    HWIFI_IOCTL_CMD_GET_STA_11H_ABILITY,        /* ��ȡָ��STA��11h������Ϣ */
    HWIFI_IOCTL_CMD_GET_STA_11R_ABILITY,        /* ��ȡָ��STA��11r������Ϣ */
    HWIFI_IOCTL_CMD_GET_TX_DELAY_AC,            /* ��ȡָ������AC�ķ���ʱ����Ϣ */
    HWIFI_IOCTL_CMD_GET_CAR_INFO,               /* ��ȡָ��device�����car����������Ϣ */
    HWIFI_IOCTL_CMD_GET_BLKWHTLST,              /* ��ȡ�ڰ����� */

    HWIFI_IOCTL_CMD_NUM
};

enum APP_IE_TYPE
{
    OAL_APP_BEACON_IE       = 0,
    OAL_APP_PROBE_REQ_IE    = 1,
    OAL_APP_PROBE_RSP_IE    = 2,
    OAL_APP_ASSOC_REQ_IE    = 3,
    OAL_APP_ASSOC_RSP_IE    = 4,
    OAL_APP_FT_IE           = 5,
    OAL_APP_REASSOC_REQ_IE  = 6,
#ifdef _PRE_WLAN_FEATURE_HILINK
    OAL_APP_OKC_BEACON_IE   = 7,
    OAL_APP_OKC_PROBE_IE    = 8,
#endif
    OAL_APP_VENDOR_IE,

    OAL_APP_IE_NUM
};
typedef oal_uint8 en_app_ie_type_uint8;

typedef enum _wlan_net_queue_type_
{
    WLAN_HI_QUEUE = 0,
	WLAN_NORMAL_QUEUE,

    WLAN_TCP_DATA_QUEUE,
    WLAN_TCP_ACK_QUEUE,

    WLAN_UDP_BK_QUEUE,
    WLAN_UDP_BE_QUEUE,
    WLAN_UDP_VI_QUEUE,
    WLAN_UDP_VO_QUEUE,

    WLAN_NET_QUEUE_BUTT
} wlan_net_queue_type;

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

/* net_device ioctl�ṹ�嶨�� */
/* hostapd/wpa_supplicant �·�����ϢԪ�ؽṹ */
/* �ýṹΪ�¼��ڴ�ش�С�������hostapd/wpa_supplicant�·���ie ��Ϣ */
/* ע��: �����ṹ�峤��Ϊ�¼��ڴ�ش�С������¼��ڴ�����޸ģ�����Ҫͬ���޸�app ���ݽṹ */
struct oal_app_ie
{
    oal_uint32              ul_ie_len;
    en_app_ie_type_uint8    en_app_ie_type;
    oal_uint8               auc_rsv[3];
    /*auc_ie �б�����ϢԪ�أ����� = (�¼��ڴ�ش�С - ��������) */
    oal_uint8               auc_ie[WLAN_WPS_IE_MAX_SIZE];
};
typedef struct oal_app_ie oal_app_ie_stru;

/*
wal �㵽hmac�ഫ��ʹ�ø����ݣ�1103 WLAN_WPS_IE_MAX_SIZE���ݵ�608�ֽڣ������¼����д�С��
wal��hmac ie data����ָ�봫��
*/
struct oal_w2h_app_ie
{
    oal_uint32              ul_ie_len;
    en_app_ie_type_uint8    en_app_ie_type;
    oal_uint8               auc_rsv[3];
    oal_uint8              *puc_data_ie;
}__OAL_DECLARE_PACKED;
typedef struct oal_w2h_app_ie oal_w2h_app_ie_stru;

struct oal_mlme_ie
{
    en_mlme_type_enum_uint8    en_mlme_type;                    /* MLME����*/
    oal_uint8                  uc_seq;                          /* ��֤֡���к� */
    oal_uint16                 us_reason;                       /* ԭ���� */
    oal_uint8                  auc_macaddr[6];
    oal_uint16                 us_optie_len;
    oal_uint8                  auc_optie[OAL_MAX_FT_ALL_LEN];
};
typedef struct oal_mlme_ie oal_mlme_ie_stru;

/*
 * Structure of the IP frame
 */
struct oal_ip_header
{
/* liuming modifed proxyst begin */
//#if (_PRE_BIG_CPU_ENDIAN == _PRE_CPU_ENDIAN)            /* BIG_ENDIAN */
#if (_PRE_LITTLE_CPU_ENDIAN == _PRE_CPU_ENDIAN)            /* LITTLE_ENDIAN */
    oal_uint8    us_ihl:4,
                 uc_version_ihl:4;
#else
    oal_uint8    uc_version_ihl:4,
                 us_ihl:4;
#endif
/* liuming modifed proxyst end */

    oal_uint8    uc_tos;
    oal_uint16   us_tot_len;
    oal_uint16   us_id;
    oal_uint16   us_frag_off;
    oal_uint8    uc_ttl;
    oal_uint8    uc_protocol;
    oal_uint16   us_check;
    oal_uint32   ul_saddr;
    oal_uint32   ul_daddr;
    /*The options start here. */
}__OAL_DECLARE_PACKED;
typedef struct oal_ip_header oal_ip_header_stru;

typedef struct
{
    oal_uint16  us_sport;
    oal_uint16  us_dport;
    oal_uint32  ul_seqnum;
    oal_uint32  ul_acknum;
    oal_uint8   uc_offset;
    oal_uint8   uc_flags;
    oal_uint16  us_window;
    oal_uint16  us_check;
    oal_uint16  us_urgent;

}oal_tcp_header_stru;

typedef struct
{
    oal_uint16 source;
    oal_uint16 dest;
    oal_uint16 len;
    oal_uint16 check;
}oal_udp_header_stru;


/*
 *    Header in on cable format
 */
struct mac_igmp_header
{
    oal_uint8  uc_type;
    oal_uint8  uc_code;        /* For newer IGMP */
    oal_uint16 us_csum;
    oal_uint32 ul_group;

}__OAL_DECLARE_PACKED;
typedef struct mac_igmp_header mac_igmp_header_stru;

/*  Group record format
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Record Type  |  Aux Data Len |     Number of Sources (N)     |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Multicast Address                       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Source Address [1]                      |
      +-                                                             -+
      |                       Source Address [2]                      |
      +-                                                             -+
      .                               .                               .
      .                               .                               .
      .                               .                               .
      +-                                                             -+
      |                       Source Address [N]                      |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      .                                                               .
      .                         Auxiliary Data                        .
      .                                                               .
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
struct mac_igmp_v3_grec
{
    oal_uint8     uc_grec_type;
    oal_uint8     uc_grec_auxwords;
    oal_uint16    us_grec_nsrcs;
    oal_uint32    ul_grec_group_ip;

}__OAL_DECLARE_PACKED;
typedef struct mac_igmp_v3_grec mac_igmp_v3_grec_stru;

/* IGMPv3 report format
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Type = 0x22  |    Reserved   |           Checksum            |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           Reserved            |  Number of Group Records (M)  |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      .                                                               .
      .                        Group Record [1]                       .
      .                                                               .
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      .                                                               .
      .                        Group Record [2]                       .
      .                                                               .
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                               .                               |
      .                               .                               .
      |                               .                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      .                                                               .
      .                        Group Record [M]                       .
      .                                                               .
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
struct mac_igmp_v3_report
{
    oal_uint8     uc_type;
    oal_uint8     uc_resv1;
    oal_uint16    us_csum;
    oal_uint16    us_resv2;
    oal_uint16    us_ngrec;

}__OAL_DECLARE_PACKED;
typedef struct mac_igmp_v3_report mac_igmp_v3_report_stru;


struct mac_mld_v1_head
{
    oal_uint8     uc_type;
    oal_uint8     uc_code;
    oal_uint16    us_check_sum;
    oal_uint16    us_max_response_delay;
    oal_uint16    us_reserved;
    oal_uint8     auc_group_ip[OAL_IPV6_ADDR_SIZE];

}__OAL_DECLARE_PACKED;
typedef struct mac_mld_v1_head mac_mld_v1_head_stru;

struct mac_mld_v2_report
{
    oal_uint8     uc_type;
    oal_uint8     uc_code;
    oal_uint16    us_check_sum;
    oal_uint16    us_reserved;
    oal_uint16    us_group_address_num;

}__OAL_DECLARE_PACKED;
typedef struct mac_mld_v2_report mac_mld_v2_report_stru;

struct mac_mld_v2_group_record
{
    oal_uint8     uc_grec_type;
    oal_uint8     uc_grec_auxwords;                                              //�������ݳ���
    oal_uint16    us_grec_srcaddr_num;                                           //  ԭ��ַ����
    oal_uint8    auc_group_ip[OAL_IPV6_ADDR_SIZE];                             // �鲥���ַ

}__OAL_DECLARE_PACKED;
typedef struct mac_mld_v2_group_record mac_mld_v2_group_record_stru;

struct mac_vlan_tag
{
    oal_uint16    us_tpid;              //tag   ID
    oal_uint16    bit_user_pri  :3,     //֡�����ȼ�
                  bit_CFI       :1,
                  bit_vlan_id   :12;     //�����õ�VLAN ID

}__OAL_DECLARE_PACKED;
typedef struct mac_vlan_tag mac_vlan_tag_stru;

/* WIN32��linux���ýṹ��  */
typedef struct
{
    oal_uint8 uc_type;
    oal_uint8 uc_len;
    oal_uint8 uc_addr[6];  /* hardware address */
}oal_eth_icmp6_lladdr_stru;

typedef struct
{
    oal_uint8           op;          /* packet opcode type */
    oal_uint8           htype;       /* hardware addr type */
    oal_uint8           hlen;        /* hardware addr length */
    oal_uint8           hops;        /* gateway hops */
    oal_uint32          xid;         /* transaction ID */
    oal_uint16          secs;        /* seconds since boot began */
    oal_uint16          flags;       /* flags */
    oal_uint32          ciaddr;      /* client IP address */
    oal_uint32          yiaddr;      /* 'your' IP address */
    oal_uint32          siaddr;      /* server IP address */
    oal_uint32          giaddr;      /* gateway IP address */
    oal_uint8           chaddr[16];  /* client hardware address */
    oal_uint8           sname[64];   /* server host name */
    oal_uint8           file[128];   /* boot file name */
    oal_uint8           options[4];  /* variable-length options field */
}oal_dhcp_packet_stru;

#ifdef _PRE_WLAN_FEATURE_SPECIAL_PKT_LOG
typedef struct
{
    oal_uint16          id;    /* transaction id */
    oal_uint16          flags; /* message future*/
    oal_uint16          qdcount;   /* question record count */
    oal_uint16          ancount;   /* answer record count */
    oal_uint16          nscount;   /* authority record count */
    oal_uint16          arcount;   /* additional record count*/
}oal_dns_hdr_stru;

typedef enum
{
    OAL_NS_Q_REQUEST = 0, /* request */
    OAL_NS_Q_RESPONSE = 1, /* response */
} oal_ns_qrcode;

typedef enum
{
    OAL_NS_O_QUERY = 0,     /* Standard query. */
    OAL_NS_O_IQUERY = 1,    /* Inverse query (deprecated/unsupported). */
}oal_ns_opcode;

/*
 * Currently defined response codes.
 */
typedef enum
{
    OAL_NS_R_NOERROR = 0,   /* No error occurred. */
}oal_ns_rcode;

typedef enum
{
    OAL_NS_T_INVALID = 0,   /* Cookie. */
    OAL_NS_T_A = 1,         /* Host address. */
}oal_ns_type;
#endif

/* ����ƽ̨ͨ�ýṹ�� */
typedef struct
{
    oal_uint8   auc_ssid[OAL_IEEE80211_MAX_SSID_LEN];       /* ssid array */
    oal_uint8   uc_ssid_len;                                /* length of the array */
    oal_uint8   auc_arry[3];
}oal_ssids_stru;

struct hostap_sta_link_info {
    oal_uint8  addr[OAL_MAC_ADDR_LEN];
    oal_uint8  rx_rssi;     /* 0 ~ 100 ,0xffΪ��Чֵ*/
    oal_uint8  tx_pwr;      /* 0 ~ 100 */
    oal_uint32 rx_rate;     /* avr nego rate in kpbs */
    oal_uint32 tx_rate;     /* avr nego rate in kpbs */
    oal_uint32 rx_minrate;  /* min nego rx rate in last duration, clean to 0 when app read over */
    oal_uint32 rx_maxrate;  /* max nego rx rate in last duration, clean to 0 when app read over */
    oal_uint32 tx_minrate;  /* min nego tx rate in last duration, clean to 0 when app read over */
    oal_uint32 tx_maxrate;  /* max nego tx rate in last duration, clean to 0 when app read over */
    oal_uint64 rx_bytes;
    oal_uint64 tx_bytes;
    oal_uint32 tx_frames_rty;  /* tx frame retry cnt */
    oal_uint32 tx_frames_all;  /* tx total frame cnt */
    oal_uint32 tx_frames_fail; /* tx fail */
    oal_uint32 SNR;            /* snr */
};

struct hostap_all_sta_link_info{
    unsigned long   buf_cnt;        /* input: sta_info count provided  */
    unsigned long   sta_cnt;        /* outpu: how many sta really */
    unsigned int    cur_channel;
    struct hostap_sta_link_info  *sta_info; /* output */
};
typedef struct hostap_sta_link_info oal_net_sta_link_info_stru;
typedef struct hostap_all_sta_link_info oal_net_all_sta_link_info_stru;

/* sta����������Ϣ�ṹ�� */
struct hostap_sta_link_info_ext{
    oal_uint8                       uc_auth_st;             /* ��֤״̬ */
    oal_bool_enum_uint8             en_band;                /* ����Ƶ�� */
    oal_bool_enum_uint8             en_wmm_switch;          /* wmm�Ƿ�ʹ�� */
    oal_uint8                       uc_ps_st;               /* ����״̬ */
    oal_uint8                       uc_sta_num;             /* �ռ����� */
    oal_uint8                       uc_work_mode;           /* Э��ģʽ */
    oal_int8                        c_noise;                /* �ڵ������ֵ */
    oal_uint8                       auc_resv[1];
    oal_uint32                      ul_associated_time;     /* �û������ӵ�ʱ�� */
};

struct hostap_all_sta_link_info_ext{
    unsigned long   buf_cnt;        /* input: sta_info count provided  */
    unsigned long   sta_cnt;        /* outpu: how many sta really */
    unsigned int    cur_channel;
    struct hostap_sta_link_info     *sta_info;
    struct hostap_sta_link_info_ext *sta_info_ext;
};
typedef struct hostap_sta_link_info_ext oal_net_sta_link_info_ext_stru;
typedef struct hostap_all_sta_link_info_ext oal_net_all_sta_link_info_ext_stru;


/*lint -e958*//* �����ֽڶ������ */
/* RRM ENABLED CAP��ϢԪ�ؽṹ�� */
struct oal_rrm_enabled_cap_ie
{
    oal_uint8   bit_link_cap            : 1,  /* bit0: Link Measurement capability enabled */
                bit_neighbor_rpt_cap    : 1,  /* bit1: Neighbor Report capability enabled */
                bit_parallel_cap        : 1,  /* bit2: Parallel Measurements capability enabled */
                bit_repeat_cap          : 1,  /* bit3: Repeated Measurements capability enabled */
                bit_bcn_passive_cap     : 1,  /* bit4: Beacon Passive Measurements capability enabled */
                bit_bcn_active_cap      : 1,  /* bit5: Beacon Active Measurements capability enabled */
                bit_bcn_table_cap       : 1,  /* bit6: Beacon Table Measurements capability enabled */
                bit_bcn_meas_rpt_cond_cap: 1; /* bit7: Beacon Measurement Reporting Conditions capability enabled */
    oal_uint8   bit_frame_cap           : 1,  /* bit8: Frame Measurement capability enabled */
                bit_chn_load_cap        : 1,  /* bit9: Channel Load Measurement capability enabled */
                bit_noise_histogram_cap : 1,  /* bit10: Noise Histogram Measurement capability enabled */
                bit_stat_cap            : 1,  /* bit11: Statistics Measurement capability enabled */
                bit_lci_cap             : 1,  /* bit12: LCI Measurement capability enabled */
                bit_lci_azimuth_cap     : 1,  /* bit13: LCI Azimuth capability enabled */
                bit_tsc_cap             : 1,  /* bit14: Transmit Stream/Category Measurement capability enabled */
                bit_triggered_tsc_cap   : 1;  /* bit15: Triggered  Transmit Stream/Category Measurement capability enabled*/
    oal_uint8   bit_ap_chn_rpt_cap                  : 1, /* bit16: AP Channel Report capability enabled */
                bit_rm_mib_cap                      : 1, /* bit17: RM MIB capability enabled */
                bit_oper_chn_max_meas_duration      : 3, /* bit18-20: Operating Channel Max Measurement Duration */
                bit_non_oper_chn_max_meas_duration  : 3; /* bit21-23: Non-operating Channel Max Measurement Durationg */
    oal_uint8   bit_meas_pilot_cap              : 3, /* bit24-26: Measurement Pilot capability */
                bit_meas_pilot_trans_info_cap   : 1, /* bit27: Measurement Pilot Transmission Information capability enabled */
                bit_neighbor_rpt_tsf_offset_cap : 1, /* bit28: Neighbor Report TSF Offset capability enabled */
                bit_rcpi_cap                    : 1, /* bit29: RCPI Measurement capability enabled */
                bit_rsni_cap                    : 1, /* bit30: RSNI Measurement capability enabled */
                bit_bss_avg_access_dly          : 1; /* bit31: BSS Average Access Delay capability enabled */
    oal_uint8   bit_avail_admission_capacity_cap: 1, /* bit32: BSS Available Admission Capacity capability enabled */
                bit_antenna_cap                 : 1, /* bit33: Antenna capability enabled */
                bit_ftm_range_report_cap        : 1, /* bit34: FTM range report capability enabled */
                bit_rsv                         : 5; /* bit35-39: Reserved */
}__OAL_DECLARE_PACKED;
typedef struct oal_rrm_enabled_cap_ie oal_rrm_enabled_cap_ie_stru;
/*lint +e958*/


/* �ھ�AP�б��BSS������Ϣ�ṹ�� */
struct oal_bssid_infomation
{
    oal_uint8       bit_ap_reachability:2,                                      /* AP�Ŀɵ����� */
                    bit_security:1,                                             /* ��AP�ļ��ܹ����뵱ǰ�����Ƿ�һ�� */
                    bit_key_scope:1,                                            /* ��AP����֤��Ϣ�Ƿ��뵱ǰ�ϱ�һֱ */
                    bit_spectrum_mgmt:1,                                        /* ����λ: ֧��Ƶ�׹��� */    /* ����λ�ֶ���beacon����һ�� */
                    bit_qos:1,                                                  /* ����λ: ֧��QOS */
                    bit_apsd:1,                                                 /* ����λ: ֧��APSD */
                    bit_radio_meas:1;                                           /* ����λ: �������� */
    oal_uint8       bit_delay_block_ack:1,                                      /* ����λ: �����ӳ�Ӧ�� */
                    bit_immediate_block_ack:1,                                  /* ����λ: ��������Ӧ�� */
                    bit_mobility_domain:1,                                      /* ��AP��beacon֡���Ƿ���MDE������˴��ϱ�һ�� */
                    bit_high_throughput:1,                                      /* ��AP��beacon֡���Ƿ��и�������Ԫ�أ�����˴��ϱ�һ�� */
                    bit_resv1:4;                                                /* Ԥ�� */
    oal_uint8       bit_resv2;
    oal_uint8       bit_resv3;
}__OAL_DECLARE_PACKED;
typedef struct oal_bssid_infomation  oal_bssid_infomation_stru;

#ifdef _PRE_WLAN_FEATURE_HILINK
typedef enum list_action
{
    OAL_WHITE_LIST_OPERATE_DEL = 0,
    OAL_WHITE_LIST_OPERATE_ADD = 1,

    OAL_WHITE_LIST_OPERATE_BUTT
}white_list_operate;
typedef oal_uint8 oal_en_white_list_operate_uint8;

typedef struct oal_hilink_white_node
{
    oal_uint8                       auc_mac[OAL_MAC_ADDR_LEN];         /*Ŀ��MAC*/
    oal_en_white_list_operate_uint8 en_white_list_operate;
    oal_uint8                       uc_reserve;
}oal_hilink_white_node_stru;

typedef struct
{
    oal_uint8       auc_mac[OAL_MAC_ADDR_LEN];                 /* ��Ҫ������sta��mac��ַ */
    oal_uint8       en_is_on;                                  /* �Ƿ������� */
    oal_uint8       uc_reserve;
    oal_uint32      ul_channel;                                /* ��Ҫ�������ŵ��� */
    oal_uint32      ul_interval;                               /* ��Ҫ����ʱ�� */
}oal_hilink_scan_params;

typedef struct
{
    oal_uint32        ul_cur_channel;                            /* ��ǰ�����ŵ� */
    oal_uint32        ul_cur_freq;                               /* ��ǰ����Ƶ�� */
}oal_hilink_current_channel;

typedef struct
{
    oal_uint16        us_work_channel_stay_time;                  /* �����ŵ�פ��ʱ�� */
    oal_uint16        us_scan_channel_stay_time;                  /* �����ŵ�פ��ʱ�� */
}oal_hilink_set_scan_time_param;

typedef struct
{
    oal_uint8           auc_sta_mac[OAL_MAC_ADDR_LEN];            /* Ҫ��ȡ��sta��mac */
    oal_bool_enum_uint8 en_support_11v;                           /* ����sta�Ƿ�֧��11v��Ϊ���� */
    oal_uint8           reserve;
}oal_hilink_get_sta_11v_ability;

typedef struct
{
    oal_uint8       auc_sta_mac_addr[OAL_MAC_ADDR_LEN];            /* Ҫ�л���sta MAC��ַ */
    oal_uint8       auc_target_ap_mac_addr[OAL_MAC_ADDR_LEN];      /* Ŀ��ap MAC��ַ */
    oal_uint8       uc_opt_class;                                  /* Operation Class */
    oal_uint8       uc_chl_num;                                    /* Channel number */
    oal_uint8       uc_phy_type;                                   /* PHY type */
    oal_uint8       uc_candidate_perf;                             /* perference data BSSIDƫ��ֵ */
    oal_bssid_infomation_stru  st_target_ap_info;                  /* Ŀ��ap��Ϣ */
}oal_hilink_change_sta_to_target_ap;

typedef struct
{
    oal_uint8                   auc_sta_mac[OAL_MAC_ADDR_LEN];     /* Ҫ��ȡ��sta��mac */
    oal_bool_enum_uint8         en_support_11k;                    /* ����sta�Ƿ�֧��11k��Ϊ���� */
    oal_uint8                   reserve;
    oal_rrm_enabled_cap_ie_stru st_sta_11k_ability;                /* sta 11k ������Ϣ */
}oal_hilink_get_sta_11k_ability;

typedef  struct
{
    oal_uint8                   auc_sta_mac[OAL_MAC_ADDR_LEN];     /* Ҫ��ȡ��sta��mac */
    oal_uint8                   auc_target_ap_bssid[OAL_MAC_ADDR_LEN];
    oal_uint8                   uc_channel_num;
    oal_uint8                   auc_reserve[3];
}oal_hilink_neighbor_bcn_req;
#endif

#if defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)
/************************************************************************/
/* err code mask of wifi                                                */
/************************************************************************/
#define WIFI_EVENT_START            (0x1000)

/*
 * WiFi event.
 * @ IEEE80211_EV_HI_DETECTED_STA sta�ڵ���add_detect_sta�ӿں��ϱ���������STA��RSSI��Ϣ
 * @ IEEE80211_EV_DRV_EXCEPTION wifi diag�¼�
 * @ IEEE80211_EV_AUTH_REJECT sta��auth�׶α�add_reject_sta�ӿ��·��Ĺ���������ϱ����¼�
 */
typedef enum ieee80211_event_en {
    IEEE80211_EV_HI_DETECTED_STA = WIFI_EVENT_START,
    IEEE80211_EV_DRV_EXCEPTION,
    IEEE80211_EV_AUTH_REJECT,
} IEEE80211_DRIVER_EVENT_EN;

/* ioctl��������ö�� */
enum ieee80211_vendor_req_sub_cmd_en
{
    IEEE80211_VENDOR_ADD_VENDOR_IE  = 0x1000,
    IEEE80211_VENDOR_MGMT_FILTER,
    IEEE80211_VENDOR_ADD_DETECT,
    IEEE80211_VENDOR_REMOVE_DETECT,
    IEEE80211_VENDOR_ADD_ACL_EXCEPT,
    IEEE80211_VENDOR_REMOVE_ACL_EXCEPT,
    IEEE80211_VENDOR_GET_VAP_STATUS,
    IEEE80211_VENDOR_GET_STA_STATUS,
    IEEE80211_VENDOR_GET_ONE_STA_STATUS,
    IEEE80211_VENDOR_ADD_REJECT_STA,
    IEEE80211_VENDOR_REMOVE_REJECT_STA,
    IEEE80211_VENDOR_SEND_RAW,
    IEEE80211_VENDOR_DISASSOC_STA,
    IEEE80211_VENDOR_GET_REG_DOMAIN,
    IEEE80211_VENDOR_SET_IE,
    IEEE80211_VENDOR_SET_CAP_INFO,
    IEEE80211_VENDOR_ADD_SENSING_MAC,
    IEEE80211_VENDOR_REMOVE_SENSING_MAC,
    IEEE80211_VENDOR_GET_SENSING_AP_RSSI,
    IEEE80211_VENDOR_SCAN2,
    IEEE80211_VENDOR_DUALBAND_COMMON,
    IEEE80211_VENDOR_DUALBAND_SILENT,
    IEEE80211_VENDOR_DUALBAND_DBGLVL,
    IEEE80211_VENDOR_DUALBAND_CONNSTAT_SYNC,
    IEEE80211_VENDOR_DUALBAND_PROTECT_CTL,
    IEEE80211_VENDOR_DUALBAND_STA_DEBUG,
    IEEE80211_VENDOR_DUALBAND_ONLINE,
    IEEE80211_VENDOR_DUALBAND_OFFLINE,
    IEEE80211_VENDOR_DUALBAND_OKC,
    IEEE80211_VENDOR_DUALBAND_STA,
    IEEE80211_VENDOR_SET_RISK_STA,
    IEEE80211_VENDOR_GET_LINK_INFO,
    IEEE80211_VENDOR_GET_USB_STATUS,
    IEEE80211_VENDOR_SET_UNRISK_STA,
    IEEE80211_VENDOR_SET_LST,
    IEEE80211_VENDOR_ADD_PROB_RATIO,
    IEEE80211_VENDOR_REMOVE_PROB_RATIO,
    IEEE80211_VENDOR_MAX,
};

typedef enum
{
    IEEE80211_FRAME_TYPE_PROBEREQ =   (1 << 0),
    IEEE80211_FRAME_TYPE_BEACON =   (1 << 1),
    IEEE80211_FRAME_TYPE_PROBERESP =   (1 << 2),
    IEEE80211_FRAME_TYPE_ASSOCREQ =   (1 << 3),
    IEEE80211_FRAME_TYPE_ASSOCRESP =   (1 << 4),
    IEEE80211_FRAME_TYPE_AUTH =   (1 << 5),
    IEEE80211_FRAME_TYPE_AP_OKC = (1 << 6),
    IEEE80211_FRAME_TYPE_STA_OKC = (1 << 7),
} oal_ieee80211_frame_type;
typedef oal_uint8 oal_ieee80211_frame_type_uint8;

#endif

#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE

/* IEEE802.11 - Region domain code for hera */
typedef enum reg_domain{
    OAL_WIFI_REG_DOMAIN_FCC  = 1,
    OAL_WIFI_REG_DOMAIN_IC  = 2,
    OAL_WIFI_REG_DOMAIN_ETSI  = 3,
    OAL_WIFI_REG_DOMAIN_SPAIN = 4,
    OAL_WIFI_REG_DOMAIN_FRANCE = 5,
    OAL_WIFI_REG_DOMAIN_MKK  = 6,
    OAL_WIFI_REG_DOMAIN_ISRAEL = 7,
    OAL_WIFI_REG_DOMAIN_MKK1  = 8,
    OAL_WIFI_REG_DOMAIN_MKK2  = 9,
    OAL_WIFI_REG_DOMAIN_MKK3  = 10,
    OAL_WIFI_REG_DOMAIN_NCC  = 11,
    OAL_WIFI_REG_DOMAIN_RUSSIAN = 12,
    OAL_WIFI_REG_DOMAIN_CN  = 13,
    OAL_WIFI_REG_DOMAIN_GLOBAL = 14,
    OAL_WIFI_REG_DOMAIN_WORLD_WIDE = 15,
    OAL_WIFI_REG_DOMAIN_TEST  = 16,
    OAL_WIFI_REG_DOMAIN_5M10M = 17,
    OAL_WIFI_REG_DOMAIN_SG  = 18,
    OAL_WIFI_REG_DOMAIN_KR  = 19,
    OAL_WIFI_REG_DOMAIN_MAX
} oal_hera_reg_domain;
typedef oal_uint8 oal_en_hera_reg_domain_uint8;

/* IE��cap info����������
   0��ʾ�����������������У�����content����������IE����������
   1��ʾ�����������������У�����content����������IE���л����㣬���û�У�����ӵ�����֡���
   2��ʾ����content�е����ݹ����IE������ڹ���֡���
   3��ʾ������content�����ݣ�������Ĭ��������Ӷ�Ӧ��IE����Ҫ��Power Constraint IE/Country IE */
typedef enum
{
    OAL_IE_SET_TYPE_AND     = 0,
    OAL_IE_SET_TYPE_OR      = 1,
    OAL_IE_SET_TYPE_ADD     = 2,
    OAL_IE_SET_TYPE_IGNORE  = 3,
    OAL_IE_SET_TYPE_BUTT
}oal_ie_set_type_enum;
typedef oal_uint8   oal_ie_set_type_enum_uint8;

typedef struct ieee80211req_send_raw
{
  oal_uint8  auc_mac_addr[OAL_MAC_ADDR_LEN];     /* STA MAC address */
  oal_uint16 us_len;                             /* Action frame length */
  oal_uint8  *puc_msg;                           /* Pointer to action frame to be sent */
} oal_ieee80211req_send_raw_stru;

typedef struct ieee80211req_get_domain
{
  /* Region domain code */
  oal_hera_reg_domain en_domain_code;
} oal_ieee80211req_get_domain_stru;

typedef struct ieee80211req_set_ie
{
  /* IE ID code */
  oal_uint32 ul_elment_id;
  /* IE set type,
  0��ʾ�����������������У�����content����������IE����������
  1��ʾ�����������������У�����content����������IE���л����㣬���û�У�����ӵ�����֡���
  2��ʾ����content�е����ݹ����IE������ڹ���֡���
  3��ʾ������content�����ݣ�������Ĭ��������Ӷ�Ӧ��IE����Ҫ��Power Constraint IE/Country IE */
  oal_uint32 ul_type;
  oal_ieee80211_frame_type  en_frame_type;
  oal_uint8  *puc_content;                /* IE content */
  oal_uint32 ul_len;                      /* IE content length */
} oal_ieee80211req_set_ie_stru;

typedef struct
{
  /* IE set type,
  0��ʾ��content����������capcibilities information����������
  1��ʾ��content����������capcibilities information���л����� */
  oal_uint32 ul_type;
  /* capbility infor content */
  oal_uint16 us_capbility;
}oal_ieee80211req_set_cap_stru;

#endif  // _PRE_WLAN_FEATURE_11KV_INTERFACE

#if defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)

typedef struct ieee80211req_action_sta
{
    /* the mac of station to be detect */
    oal_uint8  auc_mac_addr[OAL_MAC_ADDR_LEN];
    oal_uint16 us_pad;
    union
    {
        /* the channel on which to detect */
        oal_uint16 us_channel;
        oal_uint8  uc_flag;
        oal_uint16 us_reason_code;
        oal_uint8  uc_mlme_phase_mask;                 /*iee80211_mlme_phase phase;*/
        oal_sta_status_diag_info_stru *pst_sta_status;
    } param;
} oal_ieee80211req_action_sta_stru;

typedef enum ieee80211_dualband_mode {
    IEEE80211_MODE_2G_ONLY    = 0,
    IEEE80211_MODE_5G_ONLY    = 1,
    IEEE80211_MODE_DUALBAND   = 2,
} IEEE80211_DUALBAND_MODE_EN;

typedef struct dualband_para_set_to_ioctl
{
    oal_uint32 abpa_enable;
    oal_uint32 abpa_rssi_mid_threshold;
    oal_uint32 abpa_acc_intv_threshold;
    oal_uint32 abpa_silent_interval;
    oal_uint32 abpa_silent_ratio;
    oal_uint32 abpa_dbglvl;
    oal_uint32 abpa_sync_state;
    oal_uint32 abpa_protect_enable;
    oal_uint8  abpa_sync_sta[ETH_ALEN];
    oal_uint8  abpa_protect_sta[ETH_ALEN];
    oal_uint32 channel;
    oal_uint32 print_sta_type;
    oal_uint32 okc_state;
    oal_uint8 sta[ETH_ALEN];            /* ���ڲ�ѯ��˫Ƶ������ַ */
    oal_uint8 sta_mode;                 /* ˫Ƶ������ѯ��� */
} oal_dualband_para_set_to_stru;

typedef struct ieee80211_vendor_req
{
    oal_uint16 us_cmd;    /* vendor command */
    oal_uint16 us_len;    /* buffer len */
    union
    {
        oal_ieee80211req_action_sta_stru st_action_sta;     /* �����ӿڹ��ýṹ�� */
#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE
        oal_ieee80211req_send_raw_stru *pst_raw_msg;
        oal_ieee80211req_get_domain_stru st_domain;
        oal_ieee80211req_set_ie_stru *pst_ie;
        oal_ieee80211req_set_cap_stru st_cap_info;
#endif
        oal_dualband_para_set_to_stru *dualband_para;
    } param;
} oal_ieee80211_vendor_req_stru;

#endif  // defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)

typedef struct
{
    oal_uint8           auc_sta_mac[OAL_MAC_ADDR_LEN];            /* Ҫ��ȡ��sta��mac */
    oal_bool_enum_uint8 en_support_11h;                           /* ����sta�Ƿ�֧��11h��Ϊ���� */
    oal_uint8           reserve;
}oal_hilink_get_sta_11h_ability;

typedef struct
{
    oal_uint8           auc_sta_mac[OAL_MAC_ADDR_LEN];            /* Ҫ��ȡ��sta��mac */
    oal_bool_enum_uint8 en_support_11r;                           /* ����sta�Ƿ�֧��11r��Ϊ���� */
    oal_uint8           reserve;
}oal_hilink_get_sta_11r_ability;

#if (_PRE_TARGET_PRODUCT_TYPE_ONT == _PRE_CONFIG_TARGET_PRODUCT)
#define OAL_WIFI_LED_OPTIMIZED_FLAG         52

typedef enum
{
    HW_KER_LED_WLAN_OFF = 0, /* wifi down */
    HW_KER_LED_WLAN_ON, /* wifi up, and no data */
    HW_KER_LED_WLAN_BLINK_FAST, /* wifi up, and have data  */
    HW_KER_LED_WLAN_BLINK_SLOW, /* wifi up, station connected */
    HW_KER_LED_WLAN_BUTT
} HW_KER_LED_WLAN_STAT_E;

typedef enum
{
    HW_KER_WIFI_CHIP_BAND_2G,
    HW_KER_WIFI_CHIP_BAND_5G,
    HW_KER_WIFI_CHIP_BAND_BUTT
} HW_KER_WIFI_CHIP_BAND_E;

typedef enum
{
    HW_KER_WIFI_LED_WLAN = 0,        /* wlan�� */
    HW_KER_WIFI_LED_WPS,             /* wps�� */
    HW_KER_WIFI_LED_TYPE_BUTT
} HW_KER_WIFI_LED_TYPE_E;
extern unsigned int (*g_pf_wifi_led_set)(unsigned int, unsigned int, unsigned int, unsigned int);
#endif
#ifdef _PRE_WLAN_PRODUCT_1151V200
typedef enum
{
    OAL_WIFI_DOWN = 0,
    OAL_WIFI_UP,
    OAL_WIFI_TX,
    OAL_WIFI_RX,
    OAL_WIFI_STATE_BUTT
}oal_wifi_state_enum;
typedef oal_uint8 oal_wifi_state_enum_uint8;

typedef enum
{
    OAL_WIFI_BAND_2G = 0,
    OAL_WIFI_BAND_5G,
    OAL_WIFI_BAND_BUTT
}oal_wifi_band_enum;
typedef oal_uint8 oal_wifi_band_enum_uint8;
#endif


#ifdef _PRE_WLAN_FEATURE_WDS
typedef struct
{
    oal_uint8                           auc_mac[OAL_MAC_ADDR_LEN];
    oal_uint8                           uc_stas_num;
    oal_uint8                           auc_resv[1];
}oal_wds_node_stru;

typedef struct
{
    oal_uint32                          ul_last_pkt_age;
    oal_wds_node_stru                   st_related_node;
    oal_uint8                           auc_mac[OAL_MAC_ADDR_LEN];
    oal_uint8                           auc_resv[2];
}oal_wds_stas_stru;

typedef struct
{
    oal_uint32                          ul_last_pkt_age;
    oal_uint8                           auc_mac[OAL_MAC_ADDR_LEN];
    oal_uint8                           auc_resv[2];
}oal_wds_neigh_stru;

typedef struct
{
    oal_wds_node_stru                  *pst_peer_node;
    oal_wds_stas_stru                  *pst_wds_stas;
    oal_wds_neigh_stru                 *pst_neigh;
    oal_uint32                          ul_wds_aging;
    oal_uint8                           uc_wds_vap_mode;
    oal_uint8                           uc_wds_node_num;
    oal_uint16                          uc_wds_stas_num;
    oal_uint16                          uc_neigh_num;
    oal_uint8                           auc_resv[2];
}oal_wds_info_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_CAR
typedef enum
{
    OAL_CAR_UPLINK = 0,       /* ���� */
    OAL_CAR_DOWNLINK = 1,     /* ���� */

    OAL_CAR_BUTT
}oal_car_up_down_type_enum;

typedef struct
{
    oal_bool_enum_uint8                 en_car_limit_flag;                      /* ��device/vap/user�Ƿ����� */
    oal_uint8                           auc_resv[3];
    oal_uint32                          ul_car_limit_kbps;                      /* ��device/vap/user���ƴ����С */
}oal_car_limit_stru;

typedef struct
{
    oal_car_limit_stru                  st_car_device_ucast_cfg[OAL_CAR_BUTT];  /* device �������ٽṹ��,0-���� 1-���� */
    oal_car_limit_stru                  st_car_device_mcast_cfg;                /* �鲥���٣�device ���� */
    oal_uint32                          ul_car_orgin_mcast_pps_num;             /* ÿ��device��һ��pps��ԭʼ����Ͱ����������*/
}oal_car_device_limit_stru;


typedef struct
{
    oal_uint8                           uc_vap_id;
    oal_uint8                           auc_resv[3];
    oal_car_limit_stru                  ast_vap_car_cfg[OAL_CAR_BUTT];
}oal_car_vap_limit_stru;

typedef struct
{
    oal_uint16                          us_assoc_id;
    oal_uint8                           auc_user_mac_addr[OAL_MAC_ADDR_LEN];    /* user��Ӧ��MAC��ַ */
    oal_car_limit_stru                  ast_user_car_cfg[OAL_CAR_BUTT];
    oal_uint8                           uc_vap_id;                              /* user��Ӧ��vapid */
    oal_uint8                           auc_resv[3];
}oal_car_user_limit_stru;

typedef struct
{
    oal_uint8                           uc_ori_vap_id;          /* ���洫������vap��Ӧ������vapid,�ϲ���ݸ�vapid����vap��user���ˣ�����vap3 ��Ӧ����vapidΪ5 */
    oal_uint8                           auc_resv[3];
    oal_uint8                           uc_device_id;
    oal_uint16                          us_car_ctl_cycle_ms;    /* ���ٿ��ƶ�ʱ������, 100ms*/
    oal_bool_enum_uint8                 en_car_enable_flag;     /* carʹ�ܱ�־ */
    oal_uint32                          aul_car_packet_drop_num[OAL_CAR_BUTT];
    oal_uint32                          ul_car_mcast_packet_drop_num;
    oal_uint32                          ul_car_mcast_packet_pps_drop_num;

    oal_car_device_limit_stru           st_car_device_cfg;
    oal_car_vap_limit_stru              ast_car_vap_cfg[4];         // ontÿ��device���֧��4��vap
    oal_uint16                          us_user_buf_max;            /* �ϲ�����user��Ϣ����  */
    oal_uint16                          us_user_cnt;                /* �ϱ���user��Ϣ���� */
    oal_car_user_limit_stru            *pst_car_user_cfg;           /* ָ��user��Ϣ�ڴ�飬ÿ��device���128�û� */

}oal_car_info_stru;
#endif

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
typedef struct
{
    oal_uint8       uc_blkwhtlst_cnt;                                       /* �ڰ��������� */
    oal_uint8       uc_mode;                                                /* �ڰ�����ģʽ */
    oal_uint8       auc_resv[2];
    oal_uint8       auc_blkwhtlst_mac[MAX_BLACKLIST_NUM][OAL_MAC_ADDR_LEN];
}oal_blkwhtlst_stru;
#endif

#if defined(_PRE_WLAN_FEATURE_MCAST) || defined(_PRE_WLAN_FEATURE_HERA_MCAST)
typedef struct
{
    oal_uint8 auc_group_mac[OAL_MAC_ADDR_LEN];
    oal_uint8 uc_sta_num;
    oal_uint8 uc_reserve;
    oal_uint8 auc_sta_mac[MAX_STA_NUM_OF_ONE_GROUP][OAL_MAC_ADDR_LEN];
}oal_snoop_group_stru;

typedef struct
{
    oal_uint16              us_group_cnt;
    oal_uint8               auc_resv[3];
    oal_snoop_group_stru    *pst_buf;
}oal_snoop_all_group_stru;
#endif

/* net_device ioctl�ṹ�嶨�� */
typedef struct oal_net_dev_ioctl_data_tag
{
    oal_int32 l_cmd;                                  /* ����� */
    union
    {
        struct
        {
            oal_uint8    auc_mac[OAL_MAC_ADDR_LEN];
            oal_uint8    auc_rsv[2];
            oal_uint32   ul_buf_size;            /* �û��ռ�ie �����С */
            oal_uint8   *puc_buf;                /* �û��ռ�ie �����ַ */
        }assoc_req_ie;                           /* AP ģʽ�����ڻ�ȡSTA ��������ie ��Ϣ */

        struct
        {
            oal_uint32    auth_alg;
        }auth_params;

        struct
        {
            oal_uint8    auc_country_code[4];
        }country_code;

		oal_uint8     ssid[OAL_IEEE80211_MAX_SSID_LEN+4];
        oal_uint32    ul_vap_max_user;

        struct
         {
             oal_int32    l_freq;
             oal_int32    l_channel;
             oal_int32    l_ht_enabled;
             oal_int32    l_sec_channel_offset;
             oal_int32    l_vht_enabled;
             oal_int32    l_center_freq1;
             oal_int32    l_center_freq2;
             oal_int32    l_bandwidth;
         }freq;

         oal_app_ie_stru  st_app_ie;          /* beacon ie,index 0; proberesp ie index 1; assocresp ie index 2 */

        struct
        {
            oal_int32                           l_freq;              /* ap����Ƶ�Σ���linux-2.6.34�ں��ж��岻ͬ */
            oal_uint32                          ssid_len;            /* SSID ���� */
            oal_uint32                          ie_len;

            oal_uint8                          *puc_ie;
            OAL_CONST oal_uint8                *puc_ssid;               /* ����������AP SSID  */
            OAL_CONST oal_uint8                *puc_bssid;              /* ����������AP BSSID  */

            oal_uint8                           en_privacy;             /* �Ƿ���ܱ�־ */
            oal_nl80211_auth_type_enum_uint8    en_auth_type;           /* ��֤���ͣ�OPEN or SHARE-KEY */

            oal_uint8                           uc_wep_key_len;         /* WEP KEY���� */
            oal_uint8                           uc_wep_key_index;       /* WEP KEY���� */
            OAL_CONST oal_uint8                *puc_wep_key;            /* WEP KEY��Կ */

            oal_cfg80211_crypto_settings_stru   st_crypto;              /* ��Կ�׼���Ϣ */
        }cfg80211_connect_params;
        struct
        {
            oal_uint8            auc_mac[OAL_MAC_ADDR_LEN];
            oal_uint16           us_reason_code;                        /* ȥ���� reason code */
        }kick_user_params;

        oal_net_all_sta_link_info_stru all_sta_link_info;

#ifdef _PRE_WLAN_FEATURE_HILINK
        struct
        {
            oal_uint8               auc_mac[OAL_MAC_ADDR_LEN];
            oal_uint16              us_reason_code;                        /* ȥ���� reason code */
            oal_uint8               uc_rej_user;                           /* ��ֹsta���ӵ�ʱ�� */
            oal_uint8               uc_kick_user;                          /* ����:����/��ֹ���� */
            oal_uint8               auc_rsv[2];
        }fbt_kick_user_params;

        oal_hilink_white_node_stru          set_white_lst_ssidhiden_params;
        oal_hilink_scan_params              fbt_scan_params;
        oal_hilink_current_channel          fbt_get_cur_channel;
        oal_hilink_set_scan_time_param      fbt_set_scan_stay_time;
        oal_hilink_get_sta_11v_ability      fbt_get_sta_11v_ability;
        oal_hilink_change_sta_to_target_ap  fbt_change_sta_to_target_ap;
        oal_hilink_get_sta_11k_ability      fbt_get_sta_11k_ability;
        oal_hilink_neighbor_bcn_req         fbt_11k_sta_neighbor_bcn_req;

#endif
        oal_hilink_get_sta_11h_ability      fbt_get_sta_11h_ability;
#ifdef _PRE_WLAN_FEATURE_11R_AP
        oal_hilink_get_sta_11r_ability      fbt_get_sta_11r_ability;
        struct
        {
            en_mlme_type_enum_uint8    en_mlme_type;                    /* MLME����*/
            oal_uint8                  uc_seq;                          /* ��֤֡���к� */
            oal_uint16                 us_reason;                       /* ԭ���� */
            oal_uint8                  auc_macaddr[6];
            oal_uint16                 us_optie_len;
            oal_uint8                  auc_optie[OAL_MAX_FT_ALL_LEN];
        }set_mlme;

#endif

        oal_int32                l_frag;                                /* ��Ƭ����ֵ */
        oal_int32                l_rts;                                 /* RTS ����ֵ */

#if defined(_PRE_WLAN_FEATURE_MCAST) || defined(_PRE_WLAN_FEATURE_HERA_MCAST)
        oal_snoop_all_group_stru        st_all_snoop_group;
#endif
#ifdef _PRE_WLAN_FEATURE_WDS
        oal_wds_info_stru               st_wds_info;
#endif
        oal_net_all_sta_link_info_ext_stru  all_sta_link_info_ext;

#ifdef _PRE_WLAN_FEATURE_CAR
        oal_car_info_stru               st_car_info;
#endif
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
        oal_blkwhtlst_stru              st_blkwhtlst;
#endif


    }pri_data;
}oal_net_dev_ioctl_data_stru;
/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
extern oal_bool_enum_uint8 oal_netbuf_is_dhcp_port(oal_udp_header_stru *pst_udp_hdr);
extern oal_bool_enum_uint8 oal_netbuf_is_nd(oal_ipv6hdr_stru  *pst_ipv6hdr);
extern oal_bool_enum_uint8 oal_netbuf_is_dhcp6(oal_ipv6hdr_stru  *pst_ether_hdr);

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
extern oal_void  oal_netbuf_get_txtid(oal_netbuf_stru *pst_buf, oal_uint8 *puc_tos);
#endif

#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
extern oal_bool_enum_uint8 oal_netbuf_is_tcp_ack6(oal_ipv6hdr_stru  *pst_ipv6hdr);
extern oal_uint16 oal_netbuf_select_queue(oal_netbuf_stru *pst_buf);
#endif
extern oal_bool_enum_uint8 oal_netbuf_is_tcp_ack(oal_ip_header_stru  *pst_ip_hdr);
extern oal_bool_enum_uint8 oal_netbuf_is_icmp(oal_ip_header_stru  *pst_ip_hdr);

//extern oal_int genKernel_init(oal_void);
//extern oal_void genKernel_exit(oal_void);
extern oal_int32 dev_netlink_send (oal_uint8 *data, oal_int data_len);
extern oal_int32 init_dev_excp_handler(oal_void);
extern oal_void deinit_dev_excp_handler(oal_void);
extern oal_int genl_msg_send_to_user(oal_void *data, oal_int i_len);
#if (_PRE_TARGET_PRODUCT_TYPE_E5 == _PRE_CONFIG_TARGET_PRODUCT || _PRE_TARGET_PRODUCT_TYPE_CPE == _PRE_CONFIG_TARGET_PRODUCT)
extern int wlan_check_arp_spoofing(struct net_device *port_dev, struct sk_buff *pskb);
#endif
#ifdef _PRE_WLAN_PRODUCT_1151V200

OAL_STATIC OAL_INLINE oal_void oal_notify_wlan_status(oal_wifi_band_enum_uint8 uc_band, oal_wifi_state_enum_uint8 uc_stat)
{
#if (_PRE_TARGET_PRODUCT_TYPE_ONT == _PRE_CONFIG_TARGET_PRODUCT)
    oal_uint i_band = HW_KER_WIFI_CHIP_BAND_BUTT;

    if(OAL_WIFI_BAND_2G == uc_band)
    {
        i_band = HW_KER_WIFI_CHIP_BAND_2G;
    }
    else if (OAL_WIFI_BAND_5G == uc_band)
    {
        i_band = HW_KER_WIFI_CHIP_BAND_5G;
    }
    else
    {
    }

    switch (uc_stat)
    {
        case OAL_WIFI_DOWN:
            g_pf_wifi_led_set(i_band, HW_KER_WIFI_LED_WLAN, HW_KER_LED_WLAN_OFF, OAL_WIFI_LED_OPTIMIZED_FLAG);
            break;
        case OAL_WIFI_UP:
            g_pf_wifi_led_set(i_band, HW_KER_WIFI_LED_WLAN, HW_KER_LED_WLAN_ON, OAL_WIFI_LED_OPTIMIZED_FLAG);
            break;
        case OAL_WIFI_TX:
        case OAL_WIFI_RX:
            g_pf_wifi_led_set(i_band, HW_KER_WIFI_LED_WLAN, HW_KER_LED_WLAN_BLINK_FAST, OAL_WIFI_LED_OPTIMIZED_FLAG);
            break;
        default:
            break;
    }
#endif
}
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of oal_net.h */
