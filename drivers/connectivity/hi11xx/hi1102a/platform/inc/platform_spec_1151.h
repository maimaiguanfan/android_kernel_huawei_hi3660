

#ifndef __PLATFORM_SPEC_1151_H__
#define __PLATFORM_SPEC_1151_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "oal_types.h"

/*****************************************************************************
  2 �궨��
*****************************************************************************/
/*****************************************************************************
  1.1.1 �汾spec
*****************************************************************************/
/* оƬ�汾 */
#if (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
/* hi1151V100H */
#define WLAN_CHIP_VERSION_HI1151V100H           0x11510100
#else
/* hi1151V100H */
#define WLAN_CHIP_VERSION_HI1151V100H           0x11510100
#endif
/* hi1151V100L */
#define WLAN_CHIP_VERSION_HI1151V100L           0x11510102

/*���Host��Device�ж���Ԥ���룬1151��Ϊ��Ϊ֡��������ʱԤ���롣�������ṹ�����ĺ�ɾ��*/
#define IS_HOST 1
#define IS_DEVICE 1

/*****************************************************************************
  1.1.2 ��Core��Ӧspec
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
    #define WLAN_FRW_MAX_NUM_CORES          2   /* CORE������ */
#else
    #define WLAN_FRW_MAX_NUM_CORES          1
#endif

/*****************************************************************************
  2.1 WLANоƬ��Ӧ��spec
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_DOUBLE_CHIP
#define WLAN_CHIP_MAX_NUM_PER_BOARD                 2              /* ˫оƬboard��chip���� */
#else
#define WLAN_CHIP_MAX_NUM_PER_BOARD                 1              /* ��оƬboard��chip���� */
#endif

/* Ĭ������£�2G chip idΪ0��5G chip idΪ1 */
#define WLAN_CHIP_PCIE0_ID                          0
#define WLAN_CHIP_PCIE1_ID                          1

// ����ҫ2G�����д˳������뾲:����CHIP ID��band��������߼����룡
#ifdef _PRE_WLAN_FEATURE_SINGLE_CHIP_SINGLE_BAND
#define WLAN_SINGLE_CHIP_ID 0
#define WLAN_SINGLE_CHIP_SINGLE_BAND_WORK_BAND   WLAN_BAND_2G
#endif

#define WLAN_DEVICE_MAX_NUM_PER_CHIP                1                       /* ÿ��chip֧��device�����������������ᳬ��8�� */
#define WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP        WLAN_DEVICE_MAX_NUM_PER_CHIP /* ÿ��chip֧��ҵ��device�������� */

/* ����BOARD֧�ֵ�����device�� */
#define WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC            (WLAN_CHIP_MAX_NUM_PER_BOARD * WLAN_DEVICE_MAX_NUM_PER_CHIP)

/* ����BOARD֧�ֵ�����ҵ��device��Ŀ */
#define WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC    (WLAN_CHIP_MAX_NUM_PER_BOARD * WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP)


#ifdef _PRE_WLAN_PRODUCT_1151V200
#if(_PRE_TARGET_PRODUCT_TYPE_5630HERA == _PRE_CONFIG_TARGET_PRODUCT)
#define WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE      3   /* AP�Ĺ��5630�ڴ����ޣ��������AP����*/
#define WLAN_SERVICE_STA_MAX_NUM_PER_DEVICE     1    /* STA�Ĺ�� */
#else
#define WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE      16   /* AP�Ĺ�񣬽�֮ǰ��WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE�޸�*/
#define WLAN_SERVICE_STA_MAX_NUM_PER_DEVICE     3    /* STA�Ĺ�� */
#endif
#else
#define WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE      4   /* AP�Ĺ�񣬽�֮ǰ��WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE�޸�*/
#define WLAN_SERVICE_STA_MAX_NUM_PER_DEVICE     1   /* STA�Ĺ�� */
#endif

#define WLAN_CONFIG_VAP_MAX_NUM_PER_DEVICE      1   /* ����VAP����,һ��ҵ��deviceһ�� */
#ifdef _PRE_WLAN_PRODUCT_1151V200
#if(_PRE_TARGET_PRODUCT_TYPE_5630HERA == _PRE_CONFIG_TARGET_PRODUCT)
#define WLAN_AP_STA_COEXIST_VAP_NUM             4   /* ap sta����ʱvap��Ŀ 1��STA 3��AP */
#else
#define WLAN_AP_STA_COEXIST_VAP_NUM             19   /* ap sta����ʱvap��Ŀ 3��STA 16��AP */
#endif
#else
#define WLAN_AP_STA_COEXIST_VAP_NUM             5    /* ap sta����ʱvap��Ŀ 1��STA 4��AP */
#endif

/* PROXY STAģʽ��VAP���궨�� */
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
#define WLAN_PROXY_STA_MAX_NUM_PER_DEVICE             15   /* PROXY STA�ĸ��� */
#else
#define WLAN_PROXY_STA_MAX_NUM_PER_DEVICE             0    /* PROXY STAû����0 */
#endif

/* ����device֧�ֵ�����ҵ��VAP��Ŀ */
#define WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE    (WLAN_PROXY_STA_MAX_NUM_PER_DEVICE + WLAN_AP_STA_COEXIST_VAP_NUM)        /* ҵ��VAP���� */

/* ����device֧�ֵ�����VAP��Ŀ */
#define WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT      (WLAN_CONFIG_VAP_MAX_NUM_PER_DEVICE + WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE) /* 21��:4��ap,1��sta,15��proxysta,1������vap */

/* ����BOARD֧�ֵ�����VAP��Ŀ */
#define WLAN_VAP_SUPPORT_MAX_NUM_LIMIT      (WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC * WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT)  /* device����*21��:1��ap,1��sta,15��proxysta,1������vap */

/* ����BOARD֧�ֵ����ҵ��VAP����Ŀ */
#define WLAN_SERVICE_VAP_SUPPORT_MAX_NUM_LIMIT    (WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC * (WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT - WLAN_CONFIG_VAP_MAX_NUM_PER_DEVICE)) /* device����*20��:4��ap,1��sta,15��proxysta */

/* ����������Ա���Զ��ƻ�: WLAN_ASSOC_USER_MAX_NUM��Ӧg_us_assoc_max_user��WLAN_ACTIVE_USER_MAX_NUM��Ӧg_us_active_max_user */
/* �����û��������� */
#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
#define WLAN_ASSOC_USER_MAX_NUM       128    /* �����û��������� */
#else
#define WLAN_ASSOC_USER_MAX_NUM       32     /* �����û��������� */
#endif

/* ��Ծ�û��������� */
#define WLAN_ACTIVE_USER_MAX_NUM      32

/* ��Ծ�û�����λͼ */
#define WLAN_ACTIVE_USER_IDX_BMAP_LEN       ((WLAN_ACTIVE_USER_MAX_NUM + 7)>> 3)
 /* �����û�����λͼ */
#define WLAN_ASSOC_USER_IDX_BMAP_LEN       ((WLAN_ASSOC_USER_MAX_NUM + 7)>> 3)
 /* �����õ�RA LUT IDX */
#define WLAN_INVALID_RA_LUT_IDX             WLAN_ACTIVE_USER_MAX_NUM

/* ���������û���񣬱�ʾ���֧�������; ��֮��Ӧ����MAC_RES_XXX(���ڵ���RES_XXX)��Ϊ���ƻ��õ�����ӦWLAN_ASSOC_USER_MAX_NUMΪg_us_assoc_max_user */
/* ʹ�ù���:������������oal oam hal��ʼ����Ա�����������±꣬��֮��Ӧmac res�ĵ������鲥���Լ���board user�������װ�ɺ�����ҵ��������� */
/*board�������û��� = 1��CHIP֧�ֵ��������û��� * board�����CHIP��Ŀ*/
#define WLAN_ASOC_USER_MAX_NUM_LIMIT       (WLAN_ASSOC_USER_MAX_NUM * WLAN_CHIP_MAX_NUM_PER_BOARD)

/* board�鲥�û��� */
#define WLAN_MULTI_USER_MAX_NUM_LIMIT      (WLAN_SERVICE_VAP_SUPPORT_MAX_NUM_LIMIT)

/*board����û��� = �������û��� + �鲥�û����� */
#define WLAN_USER_MAX_USER_LIMIT           (WLAN_ASOC_USER_MAX_NUM_LIMIT + WLAN_MULTI_USER_MAX_NUM_LIMIT)

/* alg�㷨ģ��ʹ��,ָ��user index, 1151 user����active + noactive user�������assoc id��index */
#define WLAN_ALG_ASOC_USER_NUM_LIMIT       (WLAN_USER_MAX_USER_LIMIT)

/*****************************************************************************
  2.2 WLANЭ���Ӧ��spec
*****************************************************************************/

/*****************************************************************************
  2.3 oam��ص�spec
*****************************************************************************/
#if (((_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)||(_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION)) || (_PRE_OS_VERSION_WINDOWS == _PRE_OS_VERSION))
#define WLAN_OAM_FILE_PATH      "C:\\OAM.log"                   /* WIN32��WINDOWS��,LOG�ļ�Ĭ�ϵı���λ�� */
#elif ((_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) || (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION))
#define WLAN_OAM_FILE_PATH      "\\home\\oam.log"               /* LINUX����ϵͳ��,LOG�ļ�Ĭ�ϵı���λ�� */
#endif

/*****************************************************************************
  2.4 mem��Ӧ��spec
*****************************************************************************/
/*****************************************************************************
  2.4.1
*****************************************************************************/

#define WLAN_MEM_MAX_BYTE_LEN               (32100 + 1)   /* �ɷ�������ڴ�鳤�� */
#define WLAN_MEM_MAX_SUBPOOL_NUM            8             /* �ڴ����������ڴ�ظ��� */
#define WLAN_MEM_MAX_USERS_NUM              4             /* ����ͬһ���ڴ������û��� */

/*****************************************************************************
  2.4.2 �����������ڴ��������Ϣ
*****************************************************************************/
#if (_PRE_TARGET_PRODUCT_TYPE_ONT == _PRE_CONFIG_TARGET_PRODUCT)
#define WLAN_TID_MPDU_NUM_BIT               11
#else
#define WLAN_TID_MPDU_NUM_BIT               10
#endif
/* ����device����TID�����MPDU�������ƣ��ݶ�1024�����Է�ֹ���û������ȼ����� */
#define WLAN_TID_MPDU_NUM_LIMIT             (1 << WLAN_TID_MPDU_NUM_BIT)

#ifdef _PRE_WLAN_PRODUCT_1151V200
#define WLAN_MEM_SHARED_RX_DSCR_SIZE        88             /*��ʵ�ʽ����������ṹ���Դ�Щ��Ԥ��������Խ������������޸�*/
#else
#define WLAN_MEM_SHARED_RX_DSCR_SIZE        64
#endif
#define WLAN_MEM_SHARED_RX_DSCR_CNT         (576 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /* ����512(����֡������) + 64(����֡������) */ /* ע��! ����һ�����ڴ��Ҫ����oal_mem.c���OAL_MEM_BLK_TOTAL_CNT */
#define WLAN_MEM_SHARED_TX_DSCR_SIZE1       128            /*��ʵ�ʷ����������ṹ���Դ�Щ��Ԥ��������Է������������޸�*/
#define WLAN_MEM_SHARED_TX_DSCR_CNT1        ((WLAN_TID_MPDU_NUM_LIMIT * 2) * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /* ����������512 */
#define WLAN_MEM_SHARED_TX_DSCR_SIZE2       256                          /*��ʵ�ʷ����������ṹ���Դ�Щ��Ԥ��������Է������������޸�*/
#if (_PRE_TARGET_PRODUCT_TYPE_ONT == _PRE_CONFIG_TARGET_PRODUCT)
#define WLAN_MEM_SHARED_TX_DSCR_CNT2        (1024 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC)
#else
#define WLAN_MEM_SHARED_TX_DSCR_CNT2        (256 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /* ����amsdu�������� */
#endif

/*****************************************************************************
  2.4.3 �������֡�ڴ��������Ϣ
*****************************************************************************/
#define WLAN_MEM_SHARED_MGMT_PKT_SIZE1      1300
#ifdef _PRE_WLAN_PRODUCT_1151V200
#define WLAN_MEM_SHARED_MGMT_PKT_CNT1       (WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE * 2 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /* 16��AP */
#else
#define WLAN_MEM_SHARED_MGMT_PKT_CNT1       (8 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC)
#endif

/*****************************************************************************
  2.4.4 ��������֡�ڴ��������Ϣ
*****************************************************************************/
#define WLAN_MEM_SHARED_DATA_PKT_SIZE       44              /* 80211mac֡ͷ��С */
#define WLAN_MEM_SHARED_DATA_PKT_CNT        ((256 + 1024) * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /* skb(���յ�֡ͷ����) + ��������������(���͵�֡ͷ����) 768 */

/*****************************************************************************
  2.4.5 �����ڴ��������Ϣ
*****************************************************************************/
#define WLAN_MEM_LOCAL_SIZE1                32

#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
#define WLAN_MEM_LOCAL_CNT1                 (2320 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /* 1024(128*8)��dmac_alg_tid_stru + 1024��alg_tid_entry_stru + 5���¼�����(NON_RESET_ERR)*/
#else
#define WLAN_MEM_LOCAL_CNT1                 (580 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /* 256(32*8)��dmac_alg_tid_stru + 256��alg_tid_entry_stru + 5���¼�����(NON_RESET_ERR)*/
#endif

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
#define WLAN_MEM_LOCAL_SIZE2                140

#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
#define WLAN_MEM_LOCAL_CNT2                 (1200 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /* 200(����) */    /* 128�û�����>76*96=7296bytes����+200�� */
#else
#define WLAN_MEM_LOCAL_CNT2                 (800 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /* 200(����) */
#endif  /* #ifdef _PRE_WLAN_FEATURE_USER_EXTEND */
#else
#define WLAN_MEM_LOCAL_SIZE2                100                                /* proxysta�Ƿ���Ҫ��800���󵽸��࣬������������û��������Ժ��ٶ� */
#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
#define WLAN_MEM_LOCAL_CNT2                 (1200 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /* �����㷨�ṹ�����ڴ��(CCA�㷨ҪΪÿ���û�����76�ֽ��ڴ�)�������ؽṹ�����ڴ��� */
#else
#define WLAN_MEM_LOCAL_CNT2                 (800 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /* �����㷨�ṹ�����ڴ��(CCA�㷨ҪΪÿ���û�����76�ֽ��ڴ�)�������ؽṹ�����ڴ��� */
#endif /* #ifdef _PRE_WLAN_FEATURE_USER_EXTEND */
#endif

#define WLAN_MEM_LOCAL_SIZE3                260             /* �洢hmac_vap_cfg_priv_stru��ÿ��VAPһ�� + �¼����� 14 *�� */
#define WLAN_MEM_LOCAL_CNT3                 (WLAN_VAP_SUPPORT_MAX_NUM_LIMIT + 580)   /* ���������ؽṹ�����ڴ�飬32����¼����������ڴ��� */

#define WLAN_MEM_LOCAL_SIZE4                500

#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
#define WLAN_MEM_LOCAL_CNT4                 (550 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC)  /* 64����¼����������ڴ�顢��������alg_smartant_per_user_info_stru�����ڴ��� */
#else
#define WLAN_MEM_LOCAL_CNT4                 (350 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC)  /* 64����¼����������ڴ�顢��������alg_smartant_per_user_info_stru�����ڴ��� */
#endif

#define WLAN_MEM_LOCAL_SIZE5                2200

#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
#define WLAN_MEM_LOCAL_CNT5                 (180 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /* �㷨(TPC/autorate��)�����ڴ�顢���õ�(���ò��ֺ�������Ҫ�Ż�) */
#else
#define WLAN_MEM_LOCAL_CNT5                 (120 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /* �㷨(TPC/autorate��)�����ڴ�顢���õ�(���ò��ֺ�������Ҫ�Ż�) */
#endif

#define WLAN_MEM_LOCAL_SIZE6                9500           /* autorate */
#define WLAN_MEM_LOCAL_CNT6                 (64 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /*  */


/*****************************************************************************
  2.4.6 �¼��ṹ���ڴ��
*****************************************************************************/
#define WLAN_MEM_EVENT_SIZE1                64              /* ע��: �¼��ڴ泤�Ȱ���4�ֽ�IPCͷ���� */
#define WLAN_MEM_EVENT_CNT1                 (180 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC)
#ifdef _PRE_WLAN_FEATURE_11R_AP
#define WLAN_MEM_EVENT_SIZE2                556              /* 11r�·�ie���530��ԭ�¼��ڴ泤�Ȳ��� */
#else
#define WLAN_MEM_EVENT_SIZE2                528              /* ע��: �¼��ڴ泤�Ȱ���4�ֽ�IPCͷ���� */
#endif
#define WLAN_MEM_EVENT_CNT2                 6               /*hmac_main_initͬ��mac_chip_struʹ��һ��*/

#define WLAN_WPS_IE_MAX_SIZE                WLAN_MEM_EVENT_SIZE2 - 32   /* 32��ʾ�¼�����ռ�õĿռ� */
#ifdef _PRE_WLAN_FEATURE_HILINK
#define WLAN_OKC_IE_MAX_SIZE                WLAN_MEM_EVENT_SIZE2 - 32   /* 32��ʾ�¼�����ռ�õĿռ� */
#endif
/*****************************************************************************
  2.4.7 �û��ڴ��
*****************************************************************************/


/*****************************************************************************
  2.4.8 MIB�ڴ��  TBD :���ո��ӳصĿռ��С��������Ҫ���¿���
*****************************************************************************/
#define WLAN_MEM_MIB_SIZE1                  32000           /* mib�ṹ���С */
#define WLAN_MEM_MIB_CNT1                   ((WLAN_VAP_SUPPORT_MAX_NUM_LIMIT - 1) * 2)    /* ����VAPû��MIB */

/*****************************************************************************
  2.4.9 netbuf�ڴ��  TBD :���ո��ӳصĿռ��С��������Ҫ���¿���
*****************************************************************************/
#define WLAN_MEM_NETBUF_SIZE1               0       /* ��¡��SKB */
#define WLAN_MEM_NETBUF_CNT1                (192 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC) /* ��������֡��AMSDU�����е�ÿ��MSDU��Ӧһ����¡netbuf */

#ifndef _PRE_WLAN_PHY_PERFORMANCE
#define WLAN_MEM_NETBUF_SIZE2               1600    /* 1500 + WLAN_MAX_FRAME_HEADER_LEN(36) + WLAN_HDR_FCS_LENGTH(4) + (����ʧ�ܵĻ�,�����ֶ�Ҳ���ϱ�(20)) */
#define WLAN_MEM_NETBUF_CNT2                (512 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC)      /* ����192(��������֡) + 32(���չ���֡) + 32(���͹���֡) */
                                                        /* ���ǽ���wlan2wlanת��������������Ļ�����x2 */
#define WLAN_MEM_NETBUF_SIZE3               2500    /* ���Ƭ�����鱨�ĵ�skb */
#define WLAN_MEM_NETBUF_CNT3                (32 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC)       /* ��Ծ�û�ÿ���û�һ�� */

#else
#define WLAN_MEM_NETBUF_SIZE2               5100
#define WLAN_MEM_NETBUF_CNT2                (512 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC)

#define WLAN_MEM_NETBUF_SIZE3               5100    /* ���Ƭ�����鱨�ĵ�skb */
#define WLAN_MEM_NETBUF_CNT3                (32 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC)       /* ��Ծ�û�ÿ���û�һ�� */
#endif


#define WLAN_MEM_NETBUF_ALIGN               4       /* netbuf���� */
#define WLAN_MEM_ETH_HEADER_LEN             14      /* ��̫��֡ͷ���� */

#define WLAN_LARGE_NETBUF_SIZE        WLAN_MEM_NETBUF_SIZE2 /* NETBUF�ڴ�س�֡�ĳ��ȣ�ͳһ������� */
#define WLAN_MGMT_NETBUF_SIZE         WLAN_MEM_NETBUF_SIZE2 /* NETBUF�ڴ�ع���֡�ĳ��� �� ͳһ������� */
#define WLAN_SHORT_NETBUF_SIZE        WLAN_MEM_NETBUF_SIZE2 /* NETBUF�ڴ�ض�֡�ĳ��� ��ͳһ������� */
#define WLAN_MAX_NETBUF_SIZE          WLAN_LARGE_NETBUF_SIZE /* netbuf���֡����֡ͷ + payload */
#define WLAN_SMGMT_NETBUF_SIZE        WLAN_MGMT_NETBUF_SIZE  /* NETBUF�ڴ�ض̹���֡�ĳ��� �� ͳһ������� */

/*****************************************************************************
  2.4.9.1 sdt netbuf�ڴ��
*****************************************************************************/

/*  sdt��ϢԤ������ڴ�飬��netbuf��Ӻ󣬹������г���ʱ����Ҫ���⴦��ֱ��send����
    �ⲿ�������볤��ΪPayload�ĳ���
*/
/************************* sdt report msg format*********************************/
/* NETLINK_HEAD     | SDT_MSG_HEAD  | Payload    | SDT_MSG_TAIL  |    pad       */
/* ---------------------------------------------------------------------------- */
/* NLMSG_HDRLEN     |    8Byte      |     ...    |   1Byte       |    ...       */
/********************************************************************************/
#define WLAN_SDT_SKB_HEADROOM_LEN       8
#define WLAN_SDT_SKB_TAILROOM_LEN       1
#define WLAN_SDT_SKB_RESERVE_LEN        (WLAN_SDT_SKB_HEADROOM_LEN + WLAN_SDT_SKB_TAILROOM_LEN)

/*
    SDT�ڴ����Ҫ����SDT��Ϣ��ʵ�ʽ��е���
*/
#define WLAN_MEM_SDT_NETBUF_PAYLOAD1            37          //��־��Ϣ����
#define WLAN_MEM_SDT_NETBUF_PAYLOAD2            100
#define WLAN_MEM_SDT_NETBUF_PAYLOAD3            512
#define WLAN_MEM_SDT_NETBUF_PAYLOAD4            1600

#define WLAN_SDT_NETBUF_MAX_PAYLOAD             WLAN_MEM_SDT_NETBUF_PAYLOAD4

#define WLAN_MEM_SDT_NETBUF_SIZE1       (WLAN_MEM_SDT_NETBUF_PAYLOAD1 + WLAN_SDT_SKB_RESERVE_LEN)
#define WLAN_MEM_SDT_NETBUF_SIZE1_CNT   (250 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC)
#define WLAN_MEM_SDT_NETBUF_SIZE2       (WLAN_MEM_SDT_NETBUF_PAYLOAD2 + WLAN_SDT_SKB_RESERVE_LEN)
#define WLAN_MEM_SDT_NETBUF_SIZE2_CNT   (250 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC)
#define WLAN_MEM_SDT_NETBUF_SIZE3       (WLAN_MEM_SDT_NETBUF_PAYLOAD3 + WLAN_SDT_SKB_RESERVE_LEN)
#define WLAN_MEM_SDT_NETBUF_SIZE3_CNT   (250 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC)
#define WLAN_MEM_SDT_NETBUF_SIZE4       (WLAN_MEM_SDT_NETBUF_PAYLOAD4 + WLAN_SDT_SKB_RESERVE_LEN)
#define WLAN_MEM_SDT_NETBUF_SIZE4_CNT   (256 * WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC)

#define WLAN_SDT_MSG_FLT_HIGH_THD           800
#define WLAN_SDT_MSG_QUEUE_MAX_LEN          (WLAN_MEM_SDT_NETBUF_SIZE1_CNT + \
                                                 WLAN_MEM_SDT_NETBUF_SIZE2_CNT + \
                                                 WLAN_MEM_SDT_NETBUF_SIZE3_CNT + \
                                                 WLAN_MEM_SDT_NETBUF_SIZE4_CNT - 6)  /* ��������ڴ��Ҫ�٣��˴�ȡ��1000 */

/*****************************************************************************
  2.4.10 RFͨ�������
*****************************************************************************/

/*****************************************************************************
  2.4.11 TCP ACK�Ż�
*****************************************************************************/
#define DEFAULT_TX_TCP_ACK_OPT_ENABLE (OAL_FALSE)
#define DEFAULT_RX_TCP_ACK_OPT_ENABLE (OAL_FALSE)
#define DEFAULT_TX_TCP_ACK_THRESHOLD (1) /*��������ack ������*/
#define DEFAULT_RX_TCP_ACK_THRESHOLD (1) /*��������ack ������*/


/*****************************************************************************
  2.5 frw��ص�spec
*****************************************************************************/

/******************************************************************************
    �¼�����������Ϣ��
    ע��: ÿ�������������ɵ�����¼�����������2����������
*******************************************************************************/

#define FRW_EVENT_MAX_NUM_QUEUES    (FRW_EVENT_TYPE_BUTT * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)

#define WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    /* �¼�����       ����Ȩ��   �����������ɵ�����¼�����   �����������Ȳ��� */  \
    /* HIGH_PRIO */     {   1,               32,                      0, 0}, \
    /* HOST_CRX */      {   1,               64,                      1, 0}, \
    /* HOST_DRX */      {   1,               64,                      1, 0}, \
    /* HOST_CTX */      {   1,               64,                      1, 0}, \
    /* HOST_SDT */      {   1,               64,                      1, 0}, \
    /* WLAN_CRX */      {   1,               64,                      1, 0}, \
    /* WLAN_DRX */      {   1,               64,                      0, 0}, \
    /* WLAN_CTX */      {   1,               64,                      1, 0}, \
    /* WLAN_DTX */      {   1,               64,                      0, 0}, \
    /* WLAN_TX_COMP */  {   1,               64,                      0, 0}, \
    /* TBTT */          {   1,               64,                      1, 0}, \
    /* TIMEOUT */       {   1,                2,                      1, 0}, \
    /* HMAC MISC */     {   1,               64,                      1, 0}, \
    /* DMAC MISC */     {   1,               64,                      0, 0},
#ifdef _PRE_WLAN_FEATURE_DOUBLE_CHIP /* ˫��˫оƬ���ñ� */

#define WLAN_FRW_EVENT_CFG_TABLE \
  { \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
  }
#else /* ˫�˵�оƬ */
#ifdef _PRE_WLAN_PRODUCT_1151V200
#define WLAN_FRW_EVENT_CFG_TABLE \
  { \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
  }
#else
#define WLAN_FRW_EVENT_CFG_TABLE \
  { \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
    WLAN_FRW_EVENT_CFG_TABLE_PER_VAP \
  }
#endif
#endif

/*****************************************************************************
  2.8.0 ����OS����
*****************************************************************************/

/*****************************************************************************
  2.9 DFT
*****************************************************************************/
/*****************************************************************************
  2.9.0 ��־
*****************************************************************************/
/*****************************************************************************
  2.9.15 WiFi�ؼ���Ϣ���
*****************************************************************************/



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


/*****************************************************************************
  10 ��������
*****************************************************************************/
#endif /* #if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* #ifndef __PLATFORM_SPEC_1151_H__ */




