

#ifndef __MAC_VAP_H__
#define __MAC_VAP_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "hal_ext_if.h"
#include "mac_device.h"
#include "mac_user.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_VAP_H

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define MAC_NUM_DR_802_11A              8    /* 11A 5gģʽʱ����������(DR)���� */
#define MAC_NUM_BR_802_11A              3    /* 11A 5gģʽʱ�Ļ�������(BR)���� */
#define MAC_NUM_NBR_802_11A             5    /* 11A 5gģʽʱ�ķǻ�������(NBR)���� */

#define MAC_NUM_DR_802_11B              4    /* 11B 2.4Gģʽʱ����������(DR)���� */
#define MAC_NUM_BR_802_11B              2    /* 11B 2.4Gģʽʱ����������(BR)���� */
#define MAC_NUM_NBR_802_11B             2    /* 11B 2.4Gģʽʱ����������(NBR)���� */

#define MAC_NUM_DR_802_11G              8    /* 11G 2.4Gģʽʱ����������(DR)���� */
#define MAC_NUM_BR_802_11G              3    /* 11G 2.4Gģʽʱ�Ļ�������(BR)���� */
#define MAC_NUM_NBR_802_11G             5    /* 11G 2.4Gģʽʱ�ķǻ�������(NBR)���� */

#define MAC_NUM_DR_802_11G_MIXED        12   /* 11G ���ģʽʱ����������(DR)���� */
#define MAC_NUM_BR_802_11G_MIXED_ONE    4    /* 11G ���1ģʽʱ�Ļ�������(BR)���� */
#define MAC_NUM_NBR_802_11G_MIXED_ONE   8    /* 11G ���1ģʽʱ�ķǻ�������(NBR)���� */

#define MAC_NUM_BR_802_11G_MIXED_TWO    7    /* 11G ���2ģʽʱ�Ļ�������(BR)���� */
#define MAC_NUM_NBR_802_11G_MIXED_TWO   5    /* 11G ���2ģʽʱ�ķǻ�������(NBR)���� */

/* 11N MCS��ص����� */
#if 0
#define MAC_MAX_RATE_SINGLE_NSS_20M_11N 72  /* 1���ռ���20MHz���������*/
#define MAC_MAX_RATE_SINGLE_NSS_40M_11N 150 /* 1���ռ���40MHz���������*/
#define MAC_MAX_RATE_DOUBLE_NSS_20M_11N 144 /* 1���ռ���80MHz���������*/
#define MAC_MAX_RATE_DOUBLE_NSS_40M_11N 300 /* 2���ռ���20MHz���������*/
#endif

#define MAC_MAX_RATE_SINGLE_NSS_20M_11N 0  /* 1���ռ���20MHz���������*/
#define MAC_MAX_RATE_SINGLE_NSS_40M_11N 0 /* 1���ռ���40MHz���������*/
#define MAC_MAX_RATE_DOUBLE_NSS_20M_11N 0 /* 1���ռ���80MHz���������*/
#define MAC_MAX_RATE_DOUBLE_NSS_40M_11N 0 /* 2���ռ���20MHz���������*/

/* 11AC MCS��ص����� */
#define MAC_MAX_SUP_MCS7_11AC_EACH_NSS   0   /* 11AC���ռ���֧�ֵ����MCS��ţ�֧��0-7*/
#define MAC_MAX_SUP_MCS8_11AC_EACH_NSS   1   /* 11AC���ռ���֧�ֵ����MCS��ţ�֧��0-8*/
#define MAC_MAX_SUP_MCS9_11AC_EACH_NSS   2   /* 11AC���ռ���֧�ֵ����MCS��ţ�֧��0-9*/
#define MAC_MAX_UNSUP_MCS_11AC_EACH_NSS  3   /* 11AC���ռ���֧�ֵ����MCS��ţ���֧��n���ռ���*/

#ifdef _PRE_WLAN_FEATURE_1024QAM
#define MAC_MAX_RATE_SINGLE_NSS_20M_11AC  107 /* 1���ռ���20MHz�����Long GI����*/
#define MAC_MAX_RATE_SINGLE_NSS_40M_11AC  225 /* 1���ռ���40MHz�����Long GI����*/
#define MAC_MAX_RATE_SINGLE_NSS_80M_11AC  487 /* 1���ռ���80MHz�����Long GI����*/
#define MAC_MAX_RATE_SINGLE_NSS_160M_11AC 975 /* 1���ռ���160MHz�����Long GI����*/
#define MAC_MAX_RATE_DOUBLE_NSS_20M_11AC  216 /* 2���ռ���20MHz�����Long GI����*/
#define MAC_MAX_RATE_DOUBLE_NSS_40M_11AC  450 /* 2���ռ���40MHz�����Long GI����*/
#define MAC_MAX_RATE_DOUBLE_NSS_80M_11AC  975 /* 2���ռ���80MHz�����Long GI����*/
#define MAC_MAX_RATE_DOUBLE_NSS_160M_11AC 1950 /* 2���ռ���160MHz�����Long GI����*/
#else
/* ����Э��Ҫ��(9.4.2.158.3�½�)���޸�Ϊlong gi���� */
#define MAC_MAX_RATE_SINGLE_NSS_20M_11AC  86  /* 1���ռ���20MHz���������*/
#define MAC_MAX_RATE_SINGLE_NSS_40M_11AC  180 /* 1���ռ���40MHz���������*/
#define MAC_MAX_RATE_SINGLE_NSS_80M_11AC  390 /* 1���ռ���80MHz���������*/
#define MAC_MAX_RATE_SINGLE_NSS_160M_11AC 780 /* 1���ռ���160MHz���������*/
#define MAC_MAX_RATE_DOUBLE_NSS_20M_11AC  173 /* 2���ռ���20MHz���������*/
#define MAC_MAX_RATE_DOUBLE_NSS_40M_11AC  360 /* 2���ռ���40MHz���������*/
#define MAC_MAX_RATE_DOUBLE_NSS_80M_11AC  780 /* 2���ռ���80MHz���������*/
#define MAC_MAX_RATE_DOUBLE_NSS_160M_11AC 1560 /* 2���ռ���160MHz���������*/
#endif
#define HMAC_FBT_RSSI_ADJUST_VALUE         100
#define HMAC_RSSI_SIGNAL_INVALID           0xff        /*�ϱ�RSSI��Чֵ*/
#define HMAC_FBT_RSSI_MAX_VALUE            100

#define MAC_VAP_USER_HASH_INVALID_VALUE      0xFFFFFFFF                         /* HSAH�Ƿ�ֵ */
#define MAC_VAP_USER_HASH_MAX_VALUE         (WLAN_ASSOC_USER_MAX_NUM * 2)       /* 2Ϊ��չ���� */

#define MAC_VAP_CAP_ENABLE                   1
#define MAC_VAP_CAP_DISABLE                  0

#define MAC_VAP_FEATURE_ENABLE               1
#define MAC_VAP_FEATRUE_DISABLE              0

#define CIPHER_SUITE_SELECTOR(a, b, c, d) \
	((((oal_uint32) (d)) << 24) | (((oal_uint32) (c)) << 16) | (((oal_uint32) (b)) << 8) |(oal_uint32) (a))
#define MAC_WPA_CHIPER_CCMP                  CIPHER_SUITE_SELECTOR(0x00, 0x50, 0xf2, 4)
#define MAC_WPA_CHIPER_TKIP                  CIPHER_SUITE_SELECTOR(0x00, 0x50, 0xf2, 2)
#define MAC_WPA_AKM_PSK                      CIPHER_SUITE_SELECTOR(0x00, 0x50, 0xf2, 2)
#define MAC_WPA_AKM_PSK_SHA256               CIPHER_SUITE_SELECTOR(0x00, 0x50, 0xf2, 6)
#define MAC_RSN_CHIPER_CCMP                  CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 4)
#define MAC_RSN_CHIPER_TKIP                  CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 2)
#define MAC_RSN_AKM_PSK                      CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 2)
#define MAC_RSN_AKM_PSK_SHA256               CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 6)
#define MAC_WPA_CHIPER_OUI(_suit_type)       CIPHER_SUITE_SELECTOR(0x00, 0x50, 0xf2, _suit_type)
#define MAC_RSN_CHIPER_OUI(_suit_type)       CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, _suit_type)

#define MAC_RSN_CIPHER_SUITE_AES_128_CMAC       CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 6)
#define MAC_RSN_CIPHER_SUITE_GCMP               CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 8)
#define MAC_RSN_CIPHER_SUITE_GCMP_256           CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 9)
#define MAC_RSN_CIPHER_SUITE_CCMP_256           CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 10)
#define MAC_RSN_CIPHER_SUITE_BIP_GMAC_128       CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 11)
#define MAC_RSN_CIPHER_SUITE_BIP_GMAC_256       CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 12)
#define MAC_RSN_CIPHER_SUITE_BIP_CMAC_256       CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 13)

#ifdef _PRE_WLAN_WEB_CMD_COMM
#define MAC_RSN_AKM_8021X                    CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 1)
#define MAC_RSN_AKM_OSEN                     CIPHER_SUITE_SELECTOR(0x50, 0x6f, 0x9a, 1)

#define WPA_CIPHER_NONE BIT(0)
#define WPA_CIPHER_WEP40 BIT(1)
#define WPA_CIPHER_WEP104 BIT(2)
#define WPA_CIPHER_TKIP BIT(3)
#define WPA_CIPHER_CCMP BIT(4)
#define WPA_CIPHER_AES_128_CMAC BIT(5)
#define WPA_CIPHER_GCMP BIT(6)
#define WPA_CIPHER_SMS4 BIT(7)
#define WPA_CIPHER_GCMP_256 BIT(8)
#define WPA_CIPHER_CCMP_256 BIT(9)
#define WPA_CIPHER_BIP_GMAC_128 BIT(11)
#define WPA_CIPHER_BIP_GMAC_256 BIT(12)
#define WPA_CIPHER_BIP_CMAC_256 BIT(13)
#define WPA_CIPHER_GTK_NOT_USED BIT(14)

#define WPA_KEY_MGMT_IEEE8021X BIT(0)
#define WPA_KEY_MGMT_PSK BIT(1)
#define WPA_KEY_MGMT_NONE BIT(2)
#define WPA_KEY_MGMT_IEEE8021X_NO_WPA BIT(3)
#define WPA_KEY_MGMT_WPA_NONE BIT(4)
#define WPA_KEY_MGMT_FT_IEEE8021X BIT(5)
#define WPA_KEY_MGMT_FT_PSK BIT(6)
#define WPA_KEY_MGMT_IEEE8021X_SHA256 BIT(7)
#define WPA_KEY_MGMT_PSK_SHA256 BIT(8)
#define WPA_KEY_MGMT_WPS BIT(9)
#define WPA_KEY_MGMT_SAE BIT(10)
#define WPA_KEY_MGMT_FT_SAE BIT(11)
#define WPA_KEY_MGMT_WAPI_PSK BIT(12)
#define WPA_KEY_MGMT_WAPI_CERT BIT(13)
#define WPA_KEY_MGMT_CCKM BIT(14)
#define WPA_KEY_MGMT_OSEN BIT(15)
#define WPA_KEY_MGMT_IEEE8021X_SUITE_B BIT(16)
#define WPA_KEY_MGMT_IEEE8021X_SUITE_B_192 BIT(17)
#endif

#define MAC_CALCULATE_HASH_VALUE(_puc_mac_addr)     \
    ((_puc_mac_addr[0] + _puc_mac_addr[1]           \
    + _puc_mac_addr[2] + _puc_mac_addr[3]           \
    + _puc_mac_addr[4] + _puc_mac_addr[5])          \
    & (MAC_VAP_USER_HASH_MAX_VALUE - 1))

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
#define MAC_VAP_PROXY_STA_HASH_MAX_VALUE     16       /* Ӳ������Ϊ16~31�����16��,��ʵ��ഴ��15��proxy sta */
#define MAC_PROXYSTA_CALCULATE_HASH_VALUE(_puc_mac_addr)     \
    ((_puc_mac_addr)[ETHER_ADDR_LEN - 1] & (MAC_VAP_PROXY_STA_HASH_MAX_VALUE - 1))
#endif

#ifdef _PRE_WLAN_FEATURE_11AX
#define IS_HAL_DEVICE_SUPPORT_11AX(_pst_mac_vap) (OAL_TRUE == (_pst_mac_vap)->en_11ax_hal_cap)
#define IS_CUSTOM_OPEN_11AX_SWITCH(_pst_mac_vap) (OAL_TRUE == (_pst_mac_vap)->en_11ax_custom_switch)
#define IS_11AX_VAP(_pst_mac_vap)                (IS_HAL_DEVICE_SUPPORT_11AX(_pst_mac_vap) && IS_CUSTOM_OPEN_11AX_SWITCH(_pst_mac_vap))
#endif

#define IS_AP(_pst_mac_vap)  (WLAN_VAP_MODE_BSS_AP  == (_pst_mac_vap)->en_vap_mode)
#define IS_STA(_pst_mac_vap) (WLAN_VAP_MODE_BSS_STA == (_pst_mac_vap)->en_vap_mode)

#define IS_P2P_DEV(_pst_mac_vap)    (WLAN_P2P_DEV_MODE    == (_pst_mac_vap)->en_p2p_mode)
#define IS_P2P_GO(_pst_mac_vap)     (WLAN_P2P_GO_MODE     == (_pst_mac_vap)->en_p2p_mode)
#define IS_P2P_CL(_pst_mac_vap)     (WLAN_P2P_CL_MODE     == (_pst_mac_vap)->en_p2p_mode)
#define IS_LEGACY_VAP(_pst_mac_vap) (WLAN_LEGACY_VAP_MODE == (_pst_mac_vap)->en_p2p_mode)
#define IS_LEGACY_STA(_pst_mac_vap) (IS_STA(_pst_mac_vap) && IS_LEGACY_VAP(_pst_mac_vap))
#define IS_LEGACY_AP(_pst_mac_vap)  (IS_AP(_pst_mac_vap) && IS_LEGACY_VAP(_pst_mac_vap))

#define CIPHER_IS_WEP(cipher) ((WLAN_CIPHER_SUITE_WEP40 == cipher)||(WLAN_CIPHER_SUITE_WEP104 == cipher))

#ifdef _PRE_WLAN_DFT_STAT
#define   MAC_VAP_STATS_PKT_INCR(_member, _cnt)            ((_member) += (_cnt))
#define   MAC_VAP_STATS_BYTE_INCR(_member, _cnt)           ((_member) += (_cnt))
#endif

#define MAC_DATA_CONTAINER_HEADER_LEN 4
#define MAC_DATA_CONTAINER_MAX_LEN    512
#define MAC_DATA_CONTAINER_MIN_LEN    8  /*����Ҫ����1���¼�*/
#define MAC_DATA_HEADER_LEN           4

#define MAC_SEND_TWO_DEAUTH_FLAG    0xf000

#define MAC_DBB_SCALING_2G_RATE_NUM 12                /*2G rate���ʵĸ���*/
#define MAC_DBB_SCALING_5G_RATE_NUM 8                /*2G rate���ʵĸ���*/
#define MAC_DBB_SCALING_2G_RATE_OFFSET 0                /*2G Rate dbb scaling ����ƫ��ֵ*/
#define MAC_DBB_SCALING_2G_HT20_MCS_OFFSET 12                /*2G HT20 dbb scaling ����ƫ��ֵ*/
#define MAC_DBB_SCALING_2G_HT40_MCS_OFFSET 20                /*2G HT40 dbb scaling ����ƫ��ֵ*/
#define MAC_DBB_SCALING_2G_HT40_MCS32_OFFSET 61                /*2G HT40 mcs32 dbb scaling ����ƫ��ֵ*/
#define MAC_DBB_SCALING_5G_RATE_OFFSET 28                /*5G Rate dbb scaling ����ƫ��ֵ*/
#define MAC_DBB_SCALING_5G_HT20_MCS_OFFSET 40                /*5G HT20 dbb scaling ����ƫ��ֵ*/
#define MAC_DBB_SCALING_5G_HT20_MCS8_OFFSET 36                /*5G HT20 mcs8 dbb scaling ����ƫ��ֵ*/
#define MAC_DBB_SCALING_5G_HT40_MCS_OFFSET 48                /*5G HT40 dbb scaling ����ƫ��ֵ*/
#define MAC_DBB_SCALING_5G_HT40_MCS32_OFFSET 60                /*5G HT40 mcs32 dbb scaling ����ƫ��ֵ*/
#define MAC_DBB_SCALING_5G_HT80_MCS_OFFSET 66                /*5G HT80 dbb scaling ����ƫ��ֵ*/
#define MAC_DBB_SCALING_5G_HT80_MCS0_DELTA_OFFSET 2                /*5G HT80 mcs0/1 dbb scaling ����ƫ��ֵ����ֵ*/

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#define MAC_2G_CHANNEL_NUM                  13
#define MAC_NUM_2G_BAND                      3   /* 2g band�� */
#define MAC_NUM_5G_BAND                      7   /* 5g band�� */
#define MAC_EXT_PA_GAIN_MAX_LVL              2   /* ���ƻ�����PA���λ�� */
#define CUS_RF_PA_BIAS_REG_NUM              43   /* ���ƻ�RF����PAƫ�üĴ�������� */
#define CUS_DY_CALI_PARAMS_NUM              14   /* ��̬У׼��������,2.4g 3��(ofdm 20/40 11b cw),5g 5*2��band */
#define CUS_DY_CALI_PARAMS_TIMES            3    /* ��̬У׼����������ϵ������ */
#define CUS_DY_CALI_DPN_PARAMS_NUM          4    /* ���ƻ���̬У׼2.4G DPN��������11b OFDM20/40 CW OR 5G 160/80/40/20 */
#define CUS_DY_CALI_NUM_5G_BAND             5    /* ��̬У׼5g band1 2&3 4&5 6 7 */
#define CUS_DY_CALI_2G_VAL_DPN_MAX         (50)  /* ��̬У׼2g dpn��ȡnvram���ֵ */
#define CUS_DY_CALI_2G_VAL_DPN_MIN         (-50) /* ��̬У׼2g dpn��ȡnvram��Сֵ */
#define CUS_DY_CALI_5G_VAL_DPN_MAX         (50)  /* ��̬У׼5g dpn��ȡnvram���ֵ */
#define CUS_DY_CALI_5G_VAL_DPN_MIN         (-50) /* ��̬У׼5g dpn��ȡnvram��Сֵ */
#define CUS_NUM_2G_DELTA_RSSI_NUM          (2)   /* 20M/40M */
#define CUS_NUM_5G_DELTA_RSSI_NUM          (4)   /* 20M/40M/80M/160M */
#endif

#ifdef _PRE_WLAN_ONLINE_DPD
#define MAC_DPD_CALI_CUS_PARAMS_NUM        (5)
#endif


#ifdef _PRE_WLAN_FEATURE_VOWIFI
/* VoWiFi��ز����ĺ궨�� */
#define MAC_VOWIFI_PERIOD_MIN   1   /* ��λs */
#define MAC_VOWIFI_PERIOD_MAX   30  /* ��λs */
#define MAC_VOWIFI_TRIGGER_COUNT_MIN   1
#define MAC_VOWIFI_TRIGGER_COUNT_MAX   100
#define MAC_VOWIFI_LOW_THRESHOLD_MIN   -100
#define MAC_VOWIFI_LOW_THRESHOLD_MAX   -1
#define MAC_VOWIFI_HIGH_THRESHOLD_MIN   -100
#define MAC_VOWIFI_HIGH_THRESHOLD_MAX   -1

#define MAC_VAP_VOWIFI_MODE_DEFAULT    VOWIFI_DISABLE_REPORT
#define MAC_VAP_VOWIFI_TRIGGER_COUNT_DEFAULT   5
#define MAC_VAP_VOWIFI_PERIOD_DEFAULT_MS       1000  /* ��λms */
#define MAC_VAP_VOWIFI_HIGH_THRES_DEFAULT      -65
#define MAC_VAP_VOWIFI_LOW_THRES_DEFAULT       -80
#endif
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11K_EXTERN)
#define MAC_11K_SUPPORT_AP_CHAN_RPT_NUM 8
#define MAC_MEASUREMENT_RPT_FIX_LEN     5
#define MAC_BEACON_RPT_FIX_LEN          26
#define MAC_MAX_RPT_DETAIL_LEN          224   /*255 - 26(bcn fix) - 3(Meas rpt fix) - 2(subid 1)*/

#define MAC_ACTION_RPT_FIX_LEN                     3
#define MAC_RADIO_MEAS_ACTION_REQ_FIX_LEN          5
#define MAC_NEIGHBOR_REPORT_ACTION_REQ_FIX_LEN     3
#define MAC_NEIGHBOR_RPT_FIX_LEN                   15
#endif
#define HMAC_RSP_MSG_MAX_LEN            128   /* get wid������Ϣ��󳤶� */

#define MAC_TEST_INCLUDE_FRAME_BODY_LEN            64

#define MAC_VAP_GET_CAP_BW(_pst_mac_vap) ((_pst_mac_vap)->st_channel.en_bandwidth)
#define MAC_VAP_GET_CURREN_BW_STATE(_pst_mac_vap) (dmac_sta_bw_fsm_get_current_state(_pst_mac_vap))
#define MAC_VAP_BW_FSM_BEACON_AVAIL(_pst_mac_vap) ((!IS_LEGACY_STA(_pst_mac_vap)) || (IS_LEGACY_STA(_pst_mac_vap) && (MAC_VAP_GET_CURREN_BW_STATE(_pst_mac_vap) > DMAC_STA_BW_SWITCH_FSM_INIT)  \
                                                    && (MAC_VAP_GET_CURREN_BW_STATE(_pst_mac_vap) < DMAC_STA_BW_SWITCH_FSM_INVALID)))
#define MAC_VAP_BW_FSM_VERIFY(_pst_mac_vap)       ((DMAC_STA_BW_SWITCH_FSM_VERIFY20M == MAC_VAP_GET_CURREN_BW_STATE(_pst_mac_vap))  \
                                                    || (DMAC_STA_BW_SWITCH_FSM_VERIFY40M == MAC_VAP_GET_CURREN_BW_STATE(_pst_mac_vap)))

#define MAC_VAP_CHECK_VAP_STATUS_VALID(_pst_mac_vap) ( MAC_VAP_VAILD == (_pst_mac_vap)->uc_init_flag)


#ifdef _PRE_WLAN_FEATURE_DFS
#define MAC_VAP_GET_SUPPORT_DFS(_pst_mac_vap)((_pst_mac_vap)->en_radar_detector_is_supp)
#define MAC_VAP_IS_SUPPORT_DFS(_pst_mac_vap) (OAL_TRUE == (_pst_mac_vap)->en_radar_detector_is_supp)
#endif

#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
#define MAC_QOS_ENHANCE_ADD_NUM          2        /* add number as 2 */
#define MAC_QOS_ENHANCE_QUIT_NUM         3        /* quit number as 3 */
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
#define MAC_GET_VAP_BAND(_pst_mac_vap)  ((_pst_mac_vap)->st_channel.en_band)

/* 1103�¹�����2G, STAģʽband��ʼ����оƬ�����������Ȼ��joinʱ�����ap��bandˢ��  */
#if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
/* 03ֻ����2G���� */
#define MAC_BTCOEX_CHECK_VALID_VAP(_pst_mac_vap)  \
    ((((_pst_mac_vap)->st_cap_flag.bit_bt20dbm)||(WLAN_BAND_2G == MAC_GET_VAP_BAND(_pst_mac_vap))) ? OAL_TRUE : OAL_FALSE)

/* 1103����gc, δ����֮ǰ��5g������Ҫ�� */
#define MAC_BTCOEX_CHECK_VALID_STA(_pst_mac_vap)  \
    ((IS_STA(_pst_mac_vap) && \
      (((_pst_mac_vap)->st_cap_flag.bit_bt20dbm)||(WLAN_BAND_2G == MAC_GET_VAP_BAND(_pst_mac_vap)))) ? OAL_TRUE : OAL_FALSE)

/* ap��̬����go */
#define MAC_BTCOEX_CHECK_VALID_AP(_pst_mac_vap)  \
    ((IS_AP(_pst_mac_vap) && \
       ((((_pst_mac_vap)->st_cap_flag.bit_bt20dbm)|| WLAN_BAND_2G == MAC_GET_VAP_BAND(_pst_mac_vap)))) ? OAL_TRUE : OAL_FALSE)
#else
/* 02��׮ */
#define MAC_BTCOEX_CHECK_VALID_VAP(_pst_mac_vap)  \
    (OAL_TRUE)

#define MAC_BTCOEX_CHECK_VALID_STA(_pst_mac_vap)  \
    ((IS_LEGACY_STA(_pst_mac_vap)) ? OAL_TRUE : OAL_FALSE)

#define MAC_BTCOEX_CHECK_VALID_AP(_pst_mac_vap)  \
    ((IS_LEGACY_AP(_pst_mac_vap)) ? OAL_TRUE : OAL_FALSE)
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
#define MAC_VAP_IS_2G_SPEC_SISO(_pst_mac_vap)  \
        ((WLAN_BAND_2G == (_pst_mac_vap)->st_channel.en_band && OAL_TRUE == (_pst_mac_vap)->st_cap_flag.bit_2g_custom_siso))
#define MAC_VAP_IS_5G_SPEC_SISO(_pst_mac_vap)  \
        ((WLAN_BAND_5G == (_pst_mac_vap)->st_channel.en_band && OAL_TRUE == (_pst_mac_vap)->st_cap_flag.bit_5g_custom_siso))

#define MAC_VAP_SPEC_IS_SW_NEED_M2S_SWITCH(_pst_mac_vap) \
        (MAC_VAP_IS_2G_SPEC_SISO(_pst_mac_vap) || MAC_VAP_IS_5G_SPEC_SISO(_pst_mac_vap))
#endif

#define VOWIFI_NAT_KEEP_ALIVE_MAX_NUM 6

#define IS_USER_SINGLE_NSS(_pst_mac_user) (WLAN_SINGLE_NSS == (_pst_mac_user)->en_user_num_spatial_stream)

#define IS_VAP_SINGLE_NSS(_pst_mac_vap) (WLAN_SINGLE_NSS == (_pst_mac_vap)->en_vap_rx_nss)

#define MAC_VAP_FOREACH_USER(_pst_user, _pst_vap, _pst_list_pos)       \
             for ((_pst_list_pos) = (_pst_vap)->st_mac_user_list_head.pst_next,  \
                  (_pst_user) = OAL_DLIST_GET_ENTRY((_pst_list_pos), mac_user_stru, st_user_dlist);      \
                  (_pst_list_pos) != &((_pst_vap)->st_mac_user_list_head);                               \
                  (_pst_list_pos) = (_pst_list_pos)->pst_next,                                           \
                  (_pst_user) = OAL_DLIST_GET_ENTRY((_pst_list_pos), mac_user_stru, st_user_dlist))     \
                  if (OAL_PTR_NULL != (_pst_user))

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
/* оƬ��֤������֡/����֡���� */
typedef enum
{

    MAC_TEST_MGMT_BCST   =  0,/* ��beacon�㲥����֡ */
    MAC_TEST_MGMT_MCST  =   1,/* ��beacon�鲥����֡ */
    MAC_TEST_ATIM_UCST =    2,/* ����ATIM֡ */
    MAC_TEST_UCST =         3,/* ��������֡ */
    MAC_TEST_CTL_BCST =     4,/* �㲥����֡ */
    MAC_TEST_CTL_MCST =     5,/* �鲥����֡ */
    MAC_TEST_CTL_UCST =     6,/* ��������֡ */
    MAC_TEST_ACK_UCST =     7,/* ACK����֡ */
    MAC_TEST_CTS_UCST =     8,/* CTS����֡ */
    MAC_TEST_RTS_UCST =     9,/* RTS����֡ */
    MAC_TEST_BA_UCST =      10,/* BA����֡ */
    MAC_TEST_CF_END_UCST =  11,/* CF-End����֡ */
    MAC_TEST_TA_RA_EUQAL =  12,/* RA,TA��ͬ֡ */
    MAC_TEST_FTM         =  13,/* FTM����֡ */
    MAC_TEST_MGMT_ACTION = 14,/*��������Action֡*/
    MAC_TEST_MGMT_BEACON_INCLUDE_IE = 15,/*Beacon ֡��β����������IE��ϢԪ��*/
    MAC_TEST_MAX_TYPE_NUM
}mac_test_frame_type;
typedef oal_uint8 mac_test_frame_type_enum_uint8;

/* ��������ά���������� */
typedef enum
{
    MAC_SET_POW_RF_REG_CTL  = 0,    /* �����Ƿ�RF�Ĵ����� */
    MAC_SET_POW_FIX_LEVEL,          /* �̶����ʵȼ� */
    MAC_SET_POW_MAG_LEVEL,          /* ����֡���ʵȼ� */
    MAC_SET_POW_CTL_LEVEL,          /* ����֡���ʵȼ� */
    MAC_SET_POW_AMEND,              /* ����UPC code */
    MAC_SET_POW_NO_MARGIN,          /* ���ʲ������� */
    MAC_SET_POW_SHOW_LOG,           /* ��־��ʾ */
    MAC_SET_POW_SAR_LVL_DEBUG,      /* ��sar  */
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    MAC_TAS_POW_CTRL_DEBUG,         /* TAS������  */
    MAC_TAS_RSSI_CALI_DEBUG,        /* TAS���߲���  */
    MAC_TAS_ANT_SWITCH,             /* TAS�����л�  */
#endif
    MAC_SET_POW_BUTT
} mac_set_pow_type_enum;
typedef oal_uint8 mac_set_pow_type_enum_uint8;

typedef enum
{
    MAC_VAP_CONFIG_UCAST_DATA  = 0,
    MAC_VAP_CONFIG_MCAST_DATA,
    MAC_VAP_CONFIG_BCAST_DATA,
    MAC_VAP_CONFIG_UCAST_MGMT_2G,
    MAC_VAP_CONFIG_UCAST_MGMT_5G,
    MAC_VAP_CONFIG_MBCAST_MGMT_2G,
    MAC_VAP_CONFIG_MBCAST_MGMT_5G,

#ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
    MAC_VAP_CONFIG_MODE_UCAST_DATA,                                 /* ����Э�����õ�������֡��������ʼö�� */
    MAC_VAP_CONFIG_VHT_UCAST_DATA = MAC_VAP_CONFIG_MODE_UCAST_DATA, /* ���11ac�û����õ�������֡���� */
    MAC_VAP_CONFIG_HT_UCAST_DATA,                                   /* ���11n�û����õ�������֡���� */
    MAC_VAP_CONFIG_11AG_UCAST_DATA,                                 /* ���11a/11g�û����õ�������֡���� */
    MAC_VAP_CONFIG_11B_UCAST_DATA,                                  /* ���11b�û����õ�������֡���� */
#endif

    MAC_VAP_CONFIG_BUTT,
} mac_vap_config_dscr_frame_type_enum;
typedef oal_uint8 mac_vap_config_dscr_frame_type_uint8;

typedef enum{
    MAC_DYN_CALI_CFG_SET_EN_REALTIME_CALI_ADJUST,
    MAC_DYN_CALI_CFG_SET_2G_DSCR_INT,
    MAC_DYN_CALI_CFG_SET_5G_DSCR_INT,
    MAC_DYN_CALI_CFG_SET_CHAIN_INT,
    MAC_DYN_CALI_CFG_SET_PDET_MIN_TH,
    MAC_DYN_CALI_CFG_SET_PDET_MAX_TH,
#ifdef _PRE_WLAN_DPINIT_CALI
    MAC_DYN_CALI_CFG_GET_DPINIT_VAL,
#endif
    MAC_DYN_CALI_CFG_BUFF,
}mac_dyn_cali_cfg_enum;
typedef oal_uint8 mac_dyn_cali_cfg_enum_uint8;

typedef enum
{
    /* ҵ������㷨���ò���,����ӵ���Ӧ��START��END֮�� */
    MAC_ALG_CFG_SCHEDULE_START,

    MAC_ALG_CFG_SCHEDULE_VI_CTRL_ENA,
    MAC_ALG_CFG_SCHEDULE_BEBK_MIN_BW_ENA,
    MAC_ALG_CFG_SCHEDULE_MVAP_SCH_ENA,
    MAC_ALG_CFG_FLOWCTRL_ENABLE_FLAG,
    MAC_ALG_CFG_SCHEDULE_VI_SCH_LIMIT,
    MAC_ALG_CFG_SCHEDULE_VO_SCH_LIMIT,
    MAC_ALG_CFG_SCHEDULE_VI_DROP_LIMIT,
    MAC_ALG_CFG_SCHEDULE_VI_MSDU_LIFE_MS,
    MAC_ALG_CFG_SCHEDULE_VO_MSDU_LIFE_MS,
    MAC_ALG_CFG_SCHEDULE_BE_MSDU_LIFE_MS,
    MAC_ALG_CFG_SCHEDULE_BK_MSDU_LIFE_MS,
    MAC_ALG_CFG_SCHEDULE_VI_LOW_DELAY_MS,
    MAC_ALG_CFG_SCHEDULE_VI_HIGH_DELAY_MS,
    MAC_ALG_CFG_SCHEDULE_VI_CTRL_MS,
    MAC_ALG_CFG_SCHEDULE_SCH_CYCLE_MS,
    MAC_ALG_CFG_SCHEDULE_TRAFFIC_CTRL_CYCLE,
    MAC_ALG_CFG_SCHEDULE_CIR_NVIP_KBPS,
    MAC_ALG_CFG_SCHEDULE_CIR_NVIP_KBPS_BE,
    MAC_ALG_CFG_SCHEDULE_CIR_NVIP_KBPS_BK,
    MAC_ALG_CFG_SCHEDULE_CIR_VIP_KBPS,
    MAC_ALG_CFG_SCHEDULE_CIR_VIP_KBPS_BE,
    MAC_ALG_CFG_SCHEDULE_CIR_VIP_KBPS_BK,
    MAC_ALG_CFG_SCHEDULE_CIR_VAP_KBPS,
    MAC_ALG_CFG_SCHEDULE_SM_TRAIN_DELAY,
    MAC_ALG_CFG_VIDEO_DROP_PKT_LIMIT,
    MAC_ALG_CFG_SCHEDULE_LOG_START,
    MAC_ALG_CFG_SCHEDULE_VAP_SCH_PRIO,

    MAC_ALG_CFG_SCHEDULE_LOG_END,
    MAC_ALG_CFG_SCHEDULE_SCH_METHOD,
    MAC_ALG_CFG_SCHEDULE_FIX_SCH_MODE,

    MAC_ALG_CFG_SCHEDULE_END,

    /* AUTORATE�㷨���ò���������ӵ���Ӧ��START��END֮��*/
    MAC_ALG_CFG_AUTORATE_START,

    MAC_ALG_CFG_AUTORATE_ENABLE,
    MAC_ALG_CFG_AUTORATE_USE_LOWEST_RATE,
    MAC_ALG_CFG_AUTORATE_SHORT_STAT_NUM,
    MAC_ALG_CFG_AUTORATE_SHORT_STAT_SHIFT,
    MAC_ALG_CFG_AUTORATE_LONG_STAT_NUM,
    MAC_ALG_CFG_AUTORATE_LONG_STAT_SHIFT,
    MAC_ALG_CFG_AUTORATE_MIN_PROBE_UP_INTVL_PKTNUM,
    MAC_ALG_CFG_AUTORATE_MIN_PROBE_DOWN_INTVL_PKTNUM,
    MAC_ALG_CFG_AUTORATE_MAX_PROBE_INTVL_PKTNUM,
    MAC_ALG_CFG_AUTORATE_PROBE_INTVL_KEEP_TIMES,
    MAC_ALG_CFG_AUTORATE_DELTA_GOODPUT_RATIO,
    MAC_ALG_CFG_AUTORATE_VI_PROBE_PER_LIMIT,
    MAC_ALG_CFG_AUTORATE_VO_PROBE_PER_LIMIT,
    MAC_ALG_CFG_AUTORATE_AMPDU_DURATION,
    MAC_ALG_CFG_AUTORATE_MCS0_CONT_LOSS_NUM,
    MAC_ALG_CFG_AUTORATE_UP_PROTOCOL_DIFF_RSSI,
    MAC_ALG_CFG_AUTORATE_RTS_MODE,
    MAC_ALG_CFG_AUTORATE_LEGACY_1ST_LOSS_RATIO_TH,
    MAC_ALG_CFG_AUTORATE_HT_VHT_1ST_LOSS_RATIO_TH,
    MAC_ALG_CFG_AUTORATE_LOG_ENABLE,
    MAC_ALG_CFG_AUTORATE_VO_RATE_LIMIT,
    MAC_ALG_CFG_AUTORATE_JUDGE_FADING_PER_TH,
    MAC_ALG_CFG_AUTORATE_AGGR_OPT,
    MAC_ALG_CFG_AUTORATE_AGGR_PROBE_INTVL_NUM,
    MAC_ALG_CFG_AUTORATE_AGGR_STAT_SHIFT,
    MAC_ALG_CFG_AUTORATE_DBAC_AGGR_TIME,
    MAC_ALG_CFG_AUTORATE_DBG_VI_STATUS,
    MAC_ALG_CFG_AUTORATE_DBG_AGGR_LOG,
    MAC_ALG_CFG_AUTORATE_AGGR_NON_PROBE_PCK_NUM,
    MAC_ALG_CFG_AUTORATE_AGGR_MIN_AGGR_TIME_IDX,
    MAC_ALG_CFG_AUTORATE_AGGR_250US_DELTA_PER_TH,
    MAC_ALG_CFG_AUTORATE_MAX_AGGR_NUM,
    MAC_ALG_CFG_AUTORATE_LIMIT_1MPDU_PER_TH,
    MAC_ALG_CFG_AUTORATE_BTCOEX_PROBE_ENABLE,
    MAC_ALG_CFG_AUTORATE_BTCOEX_AGGR_ENABLE,
    MAC_ALG_CFG_AUTORATE_COEX_STAT_INTVL,
    MAC_ALG_CFG_AUTORATE_COEX_LOW_ABORT_TH,
    MAC_ALG_CFG_AUTORATE_COEX_HIGH_ABORT_TH,
    MAC_ALG_CFG_AUTORATE_COEX_AGRR_NUM_ONE_TH,
    MAC_ALG_CFG_AUTORATE_DYNAMIC_BW_ENABLE,
    MAC_ALG_CFG_AUTORATE_THRPT_WAVE_OPT,
    MAC_ALG_CFG_AUTORATE_GOODPUT_DIFF_TH,
    MAC_ALG_CFG_AUTORATE_PER_WORSE_TH,
    MAC_ALG_CFG_AUTORATE_RX_CTS_NO_BA_NUM,
    MAL_ALG_CFG_AUTORATE_VOICE_AGGR,
    MAC_ALG_CFG_AUTORATE_FAST_SMOOTH_SHIFT,
    MAC_ALG_CFG_AUTORATE_FAST_SMOOTH_AGGR_NUM,
    MAC_ALG_CFG_AUTORATE_SGI_PUNISH_PER,
    MAC_ALG_CFG_AUTORATE_SGI_PUNISH_NUM,
    MAL_ALG_CFG_AUTORATE_LAST_RATE_RANK_INDEX,
    MAL_ALG_CFG_AUTORATE_MWO_DET_DEBUG,
    MAL_ALG_CFG_AUTORATE_MWO_DET_PER_LOG,
    MAC_ALG_CFG_AUTORATE_RXCH_AGC_OPT,
    MAC_ALG_CFG_AUTORATE_RXCH_AGC_LOG,
    MAC_ALG_CFG_AUTORATE_WEAK_RSSI_TH,
    MAC_ALG_CFG_AUTORATE_RXCH_STAT_PERIOD,
    MAC_ALG_CFG_AUTORATE_RTS_ONE_TCP_DBG,
    MAC_ALG_CFG_AUTORATE_SCAN_USER_OPT,
    MAC_ALG_CFG_AUTORATE_MAX_TX_COUNT,
    MAC_ALG_CFG_AUTORATE_80M_40M_SWITCH,
    MAC_ALG_CFG_AUTORATE_40M_SWITCH_THR,
    MAC_ALG_CFG_AUTORATE_COLLISION_DET_EN,
    MAC_ALG_CFG_AUTORATE_END,

    /*AUTORATE�㷨��־���ò���������ӵ���Ӧ��START��END֮�� */
    MAC_ALG_CFG_AUTORATE_LOG_START,

    MAC_ALG_CFG_AUTORATE_STAT_LOG_START,
    MAC_ALG_CFG_AUTORATE_SELECTION_LOG_START,
    MAC_ALG_CFG_AUTORATE_FIX_RATE_LOG_START,
    MAC_ALG_CFG_AUTORATE_STAT_LOG_WRITE,
    MAC_ALG_CFG_AUTORATE_SELECTION_LOG_WRITE,
    MAC_ALG_CFG_AUTORATE_FIX_RATE_LOG_WRITE,
    MAC_ALG_CFG_AUTORATE_AGGR_STAT_LOG_START,
    MAC_ALG_CFG_AUTORATE_AGGR_STAT_LOG_WRITE,

    MAC_ALG_CFG_AUTORATE_LOG_END,

    /*AUTORATE�㷨ϵͳ�����������ӵ���Ӧ��START��END֮�� */
    MAC_ALG_CFG_AUTORATE_TEST_START,

    MAC_ALG_CFG_AUTORATE_DISPLAY_RATE_SET,
    MAC_ALG_CFG_AUTORATE_CONFIG_FIX_RATE,
    MAC_ALG_CFG_AUTORATE_CYCLE_RATE,
    MAC_ALG_CFG_AUTORATE_DISPLAY_RX_RATE,

    MAC_ALG_CFG_AUTORATE_TEST_END,

    /* SMARTANT�㷨���ò����� ����ӵ���Ӧ��START��END֮��*/
    MAC_ALG_CFG_SMARTANT_START,

    MAC_ALG_CFG_SMARTANT_ENABLE,
    MAC_ALG_CFG_SMARTANT_CERTAIN_ANT,
    MAC_ALG_CFG_SMARTANT_TRAINING_PACKET_NUMBER,
    MAC_ALG_CFG_SMARTANT_CHANGE_ANT,
    MAC_ALG_CFG_SMARTANT_START_TRAIN,
    MAC_ALG_CFG_SMARTANT_SET_TRAINING_PACKET_NUMBER,
    MAC_ALG_CFG_SMARTANT_SET_LEAST_PACKET_NUMBER,
    MAC_ALG_CFG_SMARTANT_SET_ANT_CHANGE_INTERVAL,
    MAC_ALG_CFG_SMARTANT_SET_USER_CHANGE_INTERVAL,
    MAC_ALG_CFG_SMARTANT_SET_PERIOD_MAX_FACTOR,
    MAC_ALG_CFG_SMARTANT_SET_ANT_CHANGE_FREQ,
    MAC_ALG_CFG_SMARTANT_SET_ANT_CHANGE_THRESHOLD,

    MAC_ALG_CFG_SMARTANT_END,
    /* TXBF�㷨���ò���������ӵ���Ӧ��START��END֮��*/
    MAC_ALG_CFG_TXBF_START,
    MAC_ALG_CFG_TXBF_MASTER_SWITCH,
    MAC_ALG_CFG_TXBF_TXMODE_ENABLE,
    MAC_ALG_CFG_TXBF_11N_BFEE_ENABLE,
    MAC_ALG_CFG_TXBF_2G_BFER_ENABLE,
    MAC_ALG_CFG_TXBF_2NSS_BFER_ENABLE,
    MAC_ALG_CFG_TXBF_FIX_MODE,
    MAC_ALG_CFG_TXBF_FIX_SOUNDING,
    MAC_ALG_CFG_TXBF_PROBE_INT,
    MAC_ALG_CFG_TXBF_REMOVE_WORST,
    MAC_ALG_CFG_TXBF_STABLE_NUM,
    MAC_ALG_CFG_TXBF_PROBE_COUNT,
    MAC_ALG_CFG_TXBF_END,
    /* TXBF LOG���ò���������ӵ���Ӧ��START��END֮��*/
#if WLAN_TXBF_BFER_LOG_ENABLE
    MAC_ALG_CFG_TXBF_LOG_ENABLE,
    MAC_ALG_CFG_TXBF_LOG_START,
    MAC_ALG_CFG_TXBF_RECORD_LOG_START,
    MAC_ALG_CFG_TXBF_LOG_OUTPUT,
    MAC_ALG_CFG_TXBF_LOG_END,
#endif
    /* �������㷨���ò���������ӵ���Ӧ��START��END֮�� */
    MAC_ALG_CFG_ANTI_INTF_START,

    MAC_ALG_CFG_ANTI_INTF_IMM_ENABLE,
    MAC_ALG_CFG_ANTI_INTF_UNLOCK_ENABLE,
    MAC_ALG_CFG_ANTI_INTF_RSSI_STAT_CYCLE,
    MAC_ALG_CFG_ANTI_INTF_UNLOCK_CYCLE,
    MAC_ALG_CFG_ANTI_INTF_UNLOCK_DUR_TIME,
    MAC_ALG_CFG_ANTI_INTF_NAV_IMM_ENABLE,
    MAC_ALG_CFG_ANTI_INTF_GOODPUT_FALL_TH,
    MAC_ALG_CFG_ANTI_INTF_KEEP_CYC_MAX_NUM,
    MAC_ALG_CFG_ANTI_INTF_KEEP_CYC_MIN_NUM,
    MAC_ALG_CFG_ANTI_INTF_TX_TIME_FALL_TH,
    MAC_ALG_CFG_ANTI_INTF_PER_PROBE_EN,
    MAC_ALG_CFG_ANTI_INTF_PER_FALL_TH,
    MAC_ALG_CFG_ANTI_INTF_GOODPUT_JITTER_TH,
    MAC_ALG_CFG_ANTI_INTF_DEBUG_MODE,

    MAC_ALG_CFG_ANTI_INTF_END,

    /* ���ż���㷨���ò���������ӵ���Ӧ��START��END֮�� */
    MAC_ALG_CFG_INTF_DET_START,

    MAC_ALG_CFG_INTF_DET_CYCLE,
    MAC_ALG_CFG_INTF_DET_MODE,
    MAC_ALG_CFG_INTF_DET_DEBUG,
    MAC_ALG_CFG_INTF_DET_COCH_THR_STA,
    MAC_ALG_CFG_INTF_DET_COCH_NOINTF_STA,
    MAC_ALG_CFG_INTF_DET_COCH_THR_UDP,
    MAC_ALG_CFG_INTF_DET_COCH_THR_TCP,
    MAC_ALG_CFG_INTF_DET_ADJCH_SCAN_CYC,
    MAC_ALG_CFG_INTF_DET_ADJRATIO_THR,
    MAC_ALG_CFG_INTF_DET_SYNC_THR,
    MAC_ALG_CFG_INTF_DET_AVE_RSSI,
    MAC_ALG_CFG_INTF_DET_NO_ADJRATIO_TH,
    MAC_ALG_CFG_INTF_DET_NO_ADJCYC_TH,
    MAC_ALG_CFG_INTF_DET_COLLISION_TH,
#ifdef _PRE_WLAN_PRODUCT_1151V200
    MAC_ALG_CFG_INTF_DET_CH_BUSY_LOG,
#endif
    MAC_ALG_CFG_NEG_DET_NONPROBE_TH,

    MAC_ALG_CFG_INTF_DET_END,

    /* EDCA�Ż��㷨���ò���������ӵ���Ӧ��START��END֮�� */
    MAC_ALG_CFG_EDCA_OPT_START,

    MAC_ALG_CFG_EDCA_OPT_AP_EN_MODE,
    MAC_ALG_CFG_EDCA_OPT_STA_EN,
    MAC_ALG_CFG_TXOP_LIMIT_STA_EN,
    MAC_ALG_CFG_EDCA_OPT_STA_WEIGHT,
    MAC_ALG_CFG_EDCA_OPT_DEBUG_MODE,
    MAC_ALG_CFG_EDCA_ONE_BE_TCP_OPT,
    MAC_ALG_CFG_EDCA_ONE_BE_TCP_DBG,
    MAC_ALG_CFG_EDCA_ONE_BE_TCP_TH_NO_INTF,
    MAC_ALG_CFG_EDCA_ONE_BE_TCP_TH_INTF,

    MAC_ALG_CFG_EDCA_OPT_END,

    /* CCA�Ż��㷨���ò���������ӵ���Ӧ��START��END֮�� */
    MAC_ALG_CFG_CCA_OPT_START,

    MAC_ALG_CFG_CCA_OPT_ALG_EN_MODE,
    MAC_ALG_CFG_CCA_OPT_DEBUG_MODE,
    MAC_ALG_CFG_CCA_OPT_SET_CCA_TH_DEBUG,
    MAC_ALG_CFG_CCA_OPT_LOG,

    MAC_ALG_CFG_CCA_OPT_END,

    /*�㷨��־���ò���������ӵ���Ӧ��START��END֮�� */
    MAC_ALG_CFG_INTF_DET_LOG_START,

    MAC_ALG_CFG_INTF_DET_STAT_LOG_START,
    MAC_ALG_CFG_INTF_DET_STAT_LOG_WRITE,

    MAC_ALG_CFG_INTF_DET_LOG_END,

    /* TPC�㷨���ò���, ����ӵ���Ӧ��START��END֮��*/
    MAC_ALG_CFG_TPC_START,

    MAC_ALG_CFG_TPC_MODE,
    MAC_ALG_CFG_TPC_DEBUG,
    MAC_ALG_CFG_TPC_LOG,
    MAC_ALG_CFG_TPC_OVER_TMP_TH,
    MAC_ALG_CFG_TPC_DPD_ENABLE_RATE,
    MAC_ALG_CFG_TPC_TARGET_RATE_11B,
    MAC_ALG_CFG_TPC_TARGET_RATE_11AG,
    MAC_ALG_CFG_TPC_TARGET_RATE_HT20,
    MAC_ALG_CFG_TPC_TARGET_RATE_HT40,
    MAC_ALG_CFG_TPC_TARGET_RATE_VHT20,
    MAC_ALG_CFG_TPC_TARGET_RATE_VHT40,
    MAC_ALG_CFG_TPC_TARGET_RATE_VHT80,
    MAC_ALG_CFG_TPC_TARGET_RATE_VHT160,

    MAC_ALG_CFG_TPC_END,

    /*TPC�㷨��־���ò���������ӵ���Ӧ��START��END֮�� */
    MAC_ALG_CFG_TPC_LOG_START,

    MAC_ALG_CFG_TPC_STAT_LOG_START,
    MAC_ALG_CFG_TPC_STAT_LOG_WRITE,
    MAC_ALG_CFG_TPC_PER_PKT_LOG_START,
    MAC_ALG_CFG_TPC_PER_PKT_LOG_WRITE,
    MAC_ALG_CFG_TPC_GET_FRAME_POW,
    MAC_ALG_CFG_TPC_RESET_STAT,
    MAC_ALG_CFG_TPC_RESET_PKT,

    MAC_ALG_CFG_TPC_LOG_END,

#ifdef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
    /*���û����������㷨���ò���*/
    MAC_ALG_CFG_TRAFFIC_CTL_START,

    MAC_ALG_CFG_TRAFFIC_CTL_ENABLE,
    MAC_ALG_CFG_TRAFFIC_CTL_TIMEOUT,
    MAC_ALG_CFG_TRAFFIC_CTL_MIN_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_LOG_DEBUG,
    MAC_ALG_CFG_TRAFFIC_CTL_BUF_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_BUF_ADJ_ENABLE,
    MAC_ALG_CFG_TRAFFIC_CTL_BUF_ADJ_NUM,
    MAC_ALG_CFG_TRAFFIC_CTL_BUF_ADJ_CYCLE,
    MAC_ALG_CFG_TRAFFIC_CTL_RX_RESTORE_ENABLE,
    MAC_ALG_CFG_TRAFFIC_RX_RESTORE_NUM,
    MAC_ALG_CFG_TRAFFIC_RX_RESTORE_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_END,

    /*��device���ն������������㷨���ò���*/
    MAC_ALG_CFG_RX_DSCR_CTL_START,
    MAC_ALG_CFG_RX_DSCR_CTL_ENABLE,
    MAC_ALG_CFG_RX_DSCR_CTL_LOG_DEBUG,
    MAC_ALG_CFG_RX_DSCR_CTL_END,
#endif


    /*MWO DET�㷨��־���ò���������ӵ���Ӧ��START��END֮�� */
    MAC_ALG_CFG_MWO_DET_START,

    MAC_ALG_CFG_MWO_DET_ENABLE,
    MAC_ALG_CFG_MWO_DET_END_RSSI_TH,
    MAC_ALG_CFG_MWO_DET_START_RSSI_TH,
    MAC_ALG_CFG_MWO_DET_DEBUG,

    MAC_ALG_CFG_MWO_DET_END,


    MAC_ALG_CFG_BUTT
}mac_alg_cfg_enum;
typedef oal_uint8 mac_alg_cfg_enum_uint8;

typedef enum            /* hi1102-cb */
{
    SHORTGI_20_CFG_ENUM,
    SHORTGI_40_CFG_ENUM,
    SHORTGI_80_CFG_ENUM,
    SHORTGI_BUTT_CFG
}short_gi_cfg_type;

typedef enum
{
    MAC_SET_BEACON  = 0,
    MAC_ADD_BEACON  = 1,

    MAC_BEACON_OPERATION_BUTT
}mac_beacon_operation_type ;
typedef oal_uint8 mac_beacon_operation_type_uint8;

typedef enum
{
    MAC_WMM_SET_PARAM_TYPE_DEFAULT,
    MAC_WMM_SET_PARAM_TYPE_UPDATE_EDCA,

    MAC_WMM_SET_PARAM_TYPE_BUTT
}mac_wmm_set_param_type_enum;
typedef oal_uint8 mac_wmm_set_param_type_enum_uint8;

#define MAC_VAP_AP_STATE_BUTT       (MAC_VAP_STATE_AP_WAIT_START + 1)
#define MAC_VAP_STA_STATE_BUTT      MAC_VAP_STATE_BUTT

#define H2D_SYNC_MASK_BARK_PREAMBLE (1<<1)
#define H2D_SYNC_MASK_MIB           (1<<2)
#define H2D_SYNC_MASK_PROT          (1<<3)
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
enum
{
    FREQ_IDLE                          = 0,

    FREQ_MIDIUM                            = 1,

    FREQ_HIGHER                            = 2,

    FREQ_HIGHEST                           = 3,

    FREQ_BUTT                           = 4
};
#define FREQ_TC_EN    1
#define FREQ_TC_EXIT  0

typedef oal_uint8 oal_device_freq_type_enum_uint8;
#endif

#ifdef _PRE_FEATURE_FAST_AGING
typedef enum
{
    MAC_FAST_AGING_ENABLE,
    MAC_FAST_AGING_TIMEOUT,
    MAC_FAST_AGING_COUNT,

    MAC_FAST_AGING_TYPE_BUTT
}mac_fast_aging_cfg_cmd_enum;
typedef oal_uint8 mac_fast_aging_cfg_cmd_enum_uint8;

typedef struct
{
    oal_int8                               *puc_string;
    mac_fast_aging_cfg_cmd_enum_uint8       en_fast_aging_cfg_id;
    oal_uint8                               auc_resv[3];
}mac_fast_aging_cfg_table_stru;
#endif


#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
typedef enum
{
    MAC_TCP_ACK_BUF_ENABLE,
    MAC_TCP_ACK_BUF_TIMEOUT,
    MAC_TCP_ACK_BUF_MAX,

    MAC_TCP_ACK_BUF_TYPE_BUTT
}mac_tcp_ack_buf_cfg_cmd_enum;
typedef oal_uint8 mac_tcp_ack_buf_cfg_cmd_enum_uint8;

typedef struct
{
    oal_int8                               *puc_string;
    mac_tcp_ack_buf_cfg_cmd_enum_uint8      en_tcp_ack_buf_cfg_id;
    oal_uint8                               auc_resv[3];
}mac_tcp_ack_buf_cfg_table_stru;
#endif


#ifdef _PRE_WLAN_FEATURE_CAR
typedef enum
{
    MAC_CAR_DEVICE_LIMIT,
    MAC_CAR_VAP_LIMIT,
    MAC_CAR_USER_LIMIT,
    MAC_CAR_TIMER_CYCLE_MS,
    MAC_CAR_ENABLE,
    MAC_CAR_SHOW_INFO,
    MAC_CAR_MULTICAST,
    MAC_CAR_MULTICAST_PPS,

    MAC_CAR_TYPE_BUTT
}mac_car_type_enum;
typedef oal_uint8 mac_car_type_enum_uint8;

typedef struct
{
    oal_int8                       *puc_car_name;
    mac_car_type_enum_uint8         en_car_cfg_id;
    oal_uint8                       auc_resv[3];
}hmac_car_cfg_table_stru;
#endif

typedef enum
{
    MAC_RSSI_LIMIT_SHOW_INFO,
    MAC_RSSI_LIMIT_ENABLE,
    MAC_RSSI_LIMIT_DELTA,
    MAC_RSSI_LIMIT_THRESHOLD,

    MAC_RSSI_LIMIT_TYPE_BUTT
}mac_rssi_limit_type_enum;
typedef oal_uint8 mac_rssi_limit_type_enum_uint8;

typedef struct
{
    oal_int8                        *puc_car_name;
    mac_rssi_limit_type_enum_uint8   en_rssi_cfg_id;
    oal_uint8                        auc_resv[3];
}mac_rssi_cfg_table_stru;

#define WAL_HIPRIV_RATE_INVALID       255 /*  ��Ч��������ֵ  */

#ifdef _PRE_WLAN_FEATURE_HILINK

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
#define MAC_HILINK_MAX_WHITE_LIST_MEMBER_CNT   (64)
#define MAC_HILINK_MAX_ASSOCIATE_LIST_MEMBER_CNT   (64)
#else
#define MAC_HILINK_MAX_WHITE_LIST_MEMBER_CNT   (32)
#endif

#endif

enum
{
    PKT_STAT_SET_ENABLE                        = 0,
    PKT_STAT_SET_START_STAT                    = 1,
    PKT_STAT_SET_FRAME_LEN                     = 2,
    PKT_STAT_SET_BUTT
};
typedef oal_uint8 oal_pkts_stat_enum_uint8;

enum
{
    OAL_SET_ENABLE                        = 0,
    OAL_SET_MAX_BUFF                      = 1,
    OAL_SET_THRESHOLD                     = 2,
    OAL_SET_LOG_PRINT                     = 3,
    OAL_SET_CLEAR                        = 10,

    OAL_SET_BUTT
};
typedef oal_uint8 oal_stat_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_FTM
typedef enum
{
    MAC_FTM_DISABLE_MODE               = 0,
    MAC_FTM_RESPONDER_MODE             = 1,
    MAC_FTM_INITIATOR_MODE             = 2,
    MAC_FTM_MIX_MODE                   = 3,

    MAC_FTM_MODE_BUTT,
}mac_ftm_mode_enum;
typedef oal_uint8  mac_ftm_mode_enum_uint8;
#endif
/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_M2S
extern oal_bool_enum_uint8 g_en_mimo_blacklist_etc;
#endif


/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  7 STRUCT����
*****************************************************************************/
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
typedef struct                          /* hi1102-cb */
{
    oal_uint8                           uc_shortgi_type;    /* shortgi 20/40/80     */
    oal_uint8                           uc_enable;          /* 1:enable; 0:disable  */
    oal_uint8                           auc_resv[2];
}shortgi_cfg_stru;
#endif
#define SHORTGI_CFG_STRU_LEN            4

typedef struct
{
    oal_uint8                            uc_next_expect_cnt;
    oal_uint8                            uc_csa_scan_after_tbtt;
    oal_uint8                            uc_sta_csa_last_cnt;          /*�յ� ap��һ�η��͵��л����� */
    oal_bool_enum_uint8                  en_is_fsm_attached   : 4;       /*״̬���Ƿ��Ѿ�ע��*/
    oal_bool_enum_uint8                  en_expect_cnt_status : 4;
    oal_uint8                            uc_associate_channel;         /*csaɨ���ȡprobe response ֡�ŵ�*/
    oal_uint8                            auc_resv[3];                  /*resv*/
    oal_fsm_stru                         st_oal_fsm;                   /*csa״̬��*/
    frw_timeout_stru                     st_csa_handle_timer;
    oal_uint8                            _rom[4];
}mac_sta_csa_fsm_info_stru;

typedef struct
{
    oal_uint8                                  uc_announced_channel;     /* ���ŵ��� */
    wlan_channel_bandwidth_enum_uint8          en_announced_bandwidth;   /* �´���ģʽ */
    oal_uint8                                  uc_ch_switch_cnt;         /* �ŵ��л����� */
    wlan_ch_switch_status_enum_uint8           en_ch_switch_status;      /* �ŵ��л�״̬ */
    wlan_bw_switch_status_enum_uint8           en_bw_switch_status;      /* �����л�״̬ */
    oal_bool_enum_uint8                        en_csa_present_in_bcn;    /* Beacon֡���Ƿ����CSA IE */
    wlan_csa_mode_tx_enum_uint8                en_csa_mode;
    oal_uint8                                  uc_start_chan_idx;
    oal_uint8                                  uc_end_chan_idx;
    wlan_channel_bandwidth_enum_uint8          en_user_pref_bandwidth;

    /* VAPΪSTAʱ�����г�Ա
     *
     *  ---|--------|--------------------|-----------------|-----------
     *     0        3                    0                 0
     *     X        A                    B                 C
     *
     *  sta���ŵ��л���ͨ����ͼ�������, ����Ϊ�л���������
     *  X->A A֮ǰΪδ���������ŵ��л�ʱ,�л�������Ϊ0
     *  ��A->Bʱ���Ϊsta�ȴ��л�״̬: en_waiting_to_shift_channelΪtrue
     *  ��B->CΪsta�ŵ��л���,���ȴ�ap��beacon״̬: en_waiting_for_apΪtrue
     *  C-> Ϊsta�յ���ap��beacon����׼�ŵ��л�����
     *
     *  A��ͨ�����յ�csa ie(beacon/action...), B��ͨ��Ϊtbtt�ж����л���������Ϊ
     *  0����csa ie�м�����Ϊ0��C����Ϊ�յ�beacon
     *
     *  ��A->C�Ĺ����У�������ظ��յ���csa ie�����ŵ��л�����
     *
     */
    //oal_bool_enum_uint8                      en_bw_change;             /* STA�Ƿ���Ҫ���д����л� */
    //oal_bool_enum_uint8                      en_waiting_for_ap;
    oal_uint8                                  uc_new_channel;           /* ���Կ��Ǹ�����ϲ� */
    wlan_channel_bandwidth_enum_uint8          en_new_bandwidth;         /* ���Կ��Ǹ�����ϲ� */
    oal_bool_enum_uint8                        en_waiting_to_shift_channel;  /* �ȴ��л��ŵ� */
    oal_bool_enum_uint8                        en_channel_swt_cnt_zero;
    oal_bool_enum_uint8                        en_te_b;
    oal_uint8                                  bit_wait_bw_change;        /* �յ�action֡,�ȴ��л����� */
    oal_uint8                                  uc_ch_swt_start_cnt;            /* ap��һ�η��͵��л����� */
    oal_uint8                                  uc_csa_rsv_cnt;                 /* ap csa ���������ļ��� */
    oal_uint8                                  uc_linkloss_change_chanel;        /* ���Կ��Ǹ�����ϲ� */
    wlan_linkloss_scan_switch_chan_enum_uint8  en_linkloss_scan_switch_chan;
    oal_uint32                                 ul_chan_report_for_te_a;
    mac_channel_stru                           st_old_channel;/*�ŵ��л�ʱ�����л�ǰ�ŵ���Ϣ*/
    oal_uint8                                  _rom[4];
}mac_ch_switch_info_stru;

#if 0
typedef struct
{
    oal_bool_enum_uint8  en_include_opmode_notify_ie;     /* Beacon����������֡���Ƿ����opmode_notify IE */
    oal_bool_enum_uint8  en_bf_rx_nss_change;             /* Beamforming��Rx Nss�ı� */
    oal_uint8            uc_include_ie_count;             /* Beacon����������֡�а���opmode_notify IE���� */
    oal_uint8            auc_resv[1];
}mac_opmode_notify_info_stru;

#endif

typedef struct
{
    oal_uint8           uc_rs_nrates;   /* ���ʸ��� */
    oal_uint8           auc_resv[3];
    mac_data_rate_stru  ast_rs_rates[WLAN_MAX_SUPP_RATES];
}mac_rateset_stru;

typedef struct
{
    oal_uint8         uc_br_rate_num;   /* �������ʸ��� */
    oal_uint8         uc_nbr_rate_num;  /* �ǻ������ʸ��� */
    oal_uint8         uc_max_rate;      /* ���������� */
    oal_uint8         uc_min_rate;      /* ��С�������� */
    mac_rateset_stru  st_rate;
}mac_curr_rateset_stru;

/* wme���� */
typedef struct
{
    oal_uint32    ul_aifsn;          /* AIFSN parameters */
    oal_uint32    ul_logcwmin;       /* cwmin in exponential form, ��λ2^n -1 slot */
    oal_uint32    ul_logcwmax;       /* cwmax in exponential form, ��λ2^n -1 slot */
    oal_uint32    ul_txop_limit;     /* txopLimit, us */
}mac_wme_param_stru;

/* MAC vap�������Ա�ʶ */
typedef struct
{
    oal_uint32  bit_uapsd                          : 1,
                bit_txop_ps                        : 1,
                bit_wpa                            : 1,
                bit_wpa2                           : 1,
                bit_dsss_cck_mode_40mhz            : 1,                 /* �Ƿ�������40M��ʹ��DSSS/CCK, 1-����, 0-������ */
                bit_rifs_tx_on                     : 1,
                bit_tdls_prohibited                : 1,                 /* tdlsȫ�ֽ��ÿ��أ� 0-���ر�, 1-�ر� */
                bit_tdls_channel_switch_prohibited : 1,                 /* tdls�ŵ��л�ȫ�ֽ��ÿ��أ� 0-���ر�, 1-�ر� */
                bit_hide_ssid                      : 1,                 /* AP��������ssid,  0-�ر�, 1-����*/
                bit_wps                            : 1,                 /* AP WPS����:0-�ر�, 1-���� */
                bit_11ac2g                         : 1,                 /* 2.4G�µ�11ac:0-�ر�, 1-���� */
                bit_keepalive                      : 1,                 /* vap KeepAlive���ܿ���: 0-�ر�, 1-����*/
                bit_smps                           : 2,                 /* vap ��ǰSMPS����(��ʱ��ʹ��) */
                bit_dpd_enbale                     : 1,                 /* dpd�Ƿ��� */
                bit_dpd_done                       : 1,                 /* dpd�Ƿ���� */
                bit_11ntxbf                        : 1,                 /* 11n txbf���� */
                bit_disable_2ght40                 : 1,                 /* 2ght40��ֹλ��1-��ֹ��0-����ֹ */
                bit_peer_obss_scan                 : 1,                 /* �Զ�֧��obss scan����: 0-��֧��, 1-֧�� */
                bit_1024qam                        : 1,                 /* �Զ�֧��1024qam����: 0-��֧��, 1-֧�� */
                bit_wmm_cap                        : 1,                 /* ������STA������AP�Ƿ�֧��wmm������Ϣ */
                bit_is_interworking                : 1,                 /* ������STA������AP�Ƿ�֧��interworking���� */
                bit_ip_filter                      : 1,                 /* rx����ip�����˵Ĺ��� */
                bit_opmode                         : 1,                 /* �Զ˵�probe rsp�е�extended capbilities �Ƿ�֧��OPMODE */
                bit_nb                             : 1,                 /* Ӳ���Ƿ�֧��խ�� */
                bit_2040_autoswitch                : 1,                 /* �Ƿ�֧���滷���Զ�2040�����л� */
                bit_2g_custom_siso                 : 1,                 /* 2g�Ƿ��ƻ�������siso,Ĭ�ϵ���0,��ʼ˫���� */
                bit_5g_custom_siso                 : 1,                 /* 5g�Ƿ��ƻ�������siso,Ĭ�ϵ���0,��ʼ˫���� */
                bit_bt20dbm                        : 1,                 /* 20dbm�Ƿ�ʹ�ܣ�����host��sta��ɾ�ۺ��ж� */
                bit_resv                           : 3;
}mac_cap_flag_stru;

/* VAP�շ���ͳ�� */
typedef struct
{
    /* net_device��ͳ����Ϣ, net_deviceͳ�ƾ�����̫���ı��� */
    oal_uint32  ul_rx_packets_to_lan;               /* �������̵�LAN�İ��� */
    oal_uint32  ul_rx_bytes_to_lan;                 /* �������̵�LAN���ֽ��� */
    oal_uint32  ul_rx_dropped_packets;              /* ���������ж����İ��� */
    oal_uint32  ul_rx_vap_non_up_dropped;           /* vapû��up�����İ��ĸ��� */
    oal_uint32  ul_rx_dscr_error_dropped;           /* �������������İ��ĸ��� */
    oal_uint32  ul_rx_first_dscr_excp_dropped;      /* �������װ��쳣�����İ��ĸ��� */
    oal_uint32  ul_rx_alg_filter_dropped;           /* �㷨���˶����İ��ĸ��� */
    oal_uint32  ul_rx_feature_ap_dropped;           /* AP����֡���˶������� */
    oal_uint32  ul_rx_null_frame_dropped;           /* �յ�NULL֡����Ŀ */
    oal_uint32  ul_rx_transmit_addr_checked_dropped;/* ���Ͷ˵�ַ����ʧ�ܶ��� */
    oal_uint32  ul_rx_dest_addr_checked_dropped;    /* Ŀ�ĵ�ַ����ʧ�ܶ��� */
    oal_uint32  ul_rx_multicast_dropped;            /* �鲥֡ʧ��(netbuf copyʧ��)���� */

    oal_uint32  ul_tx_packets_from_lan;             /* ��������LAN�����İ��� */
    oal_uint32  ul_tx_bytes_from_lan;               /* ��������LAN�������ֽ��� */
    oal_uint32  ul_tx_dropped_packets;              /* ���������ж����İ��� */

    /* ��������ͳ����Ϣ */

}mac_vap_stats_stru;

#ifdef _PRE_WLAN_FEATURE_ISOLATION
/* ������Ϣ */
typedef struct
{
    oal_uint8                           uc_single_type;                                  /* ��bss��������   */
    oal_uint8                           uc_multi_type;                             /* ��bss��������       */
    oal_uint8                           uc_mode;                                  /* ����ģʽbit0���㲥���� bit1���鲥���� bit2����������   */
    oal_uint8                           uc_forward;                               /* forwarding��ʽ */
    oal_uint32                          ul_counter_bcast;                         /* �㲥��������� */
    oal_uint32                          ul_counter_mcast;                         /* �鲥��������� */
    oal_uint32                          ul_counter_ucast;                         /* ������������� */
} mac_isolation_info_stru;
#endif  /* _PRE_WLAN_FEATURE_CUSTOM_SECURITY */
typedef struct
{
    oal_uint16                          us_user_idx;
    wlan_protocol_enum_uint8            en_avail_protocol_mode; /* �û�Э��ģʽ */
    wlan_protocol_enum_uint8            en_cur_protocol_mode;
    wlan_protocol_enum_uint8            en_protocol_mode;
    oal_uint8                           auc_resv[3];
}mac_h2d_user_protocol_stru;

typedef struct
{
    oal_uint16                          us_user_idx;
    oal_uint8                           uc_arg1;
    oal_uint8                           uc_arg2;

    /* Э��ģʽ��Ϣ */
    wlan_protocol_enum_uint8            en_cur_protocol_mode;
    wlan_protocol_enum_uint8            en_protocol_mode;
    oal_uint8                           en_avail_protocol_mode; /* �û���VAPЭ��ģʽ����, ���㷨���� */

    wlan_bw_cap_enum_uint8              en_bandwidth_cap;       /* �û�����������Ϣ */
    wlan_bw_cap_enum_uint8              en_avail_bandwidth;     /* �û���VAP������������,���㷨���� */
    wlan_bw_cap_enum_uint8              en_cur_bandwidth;       /* Ĭ��ֵ��en_avail_bandwidth��ͬ,���㷨�����޸� */

    oal_bool_enum_uint8                 en_user_pmf;
    mac_user_asoc_state_enum_uint8      en_user_asoc_state;     /* �û�����״̬ */
}mac_h2d_usr_info_stru;


typedef struct
{
    mac_user_cap_info_stru              st_user_cap_info;                   /* �û�������Ϣ */
    oal_uint16                          us_user_idx;
    oal_uint8                           auc_resv[2];
}mac_h2d_usr_cap_stru;



typedef struct
{
    oal_uint16                          us_user_idx;

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
    mac_he_hdl_stru                     st_he_hdl;
#endif
    /* vht���ʼ���Ϣ */
    mac_vht_hdl_stru                    st_vht_hdl;

    /* ht���ʼ���Ϣ */
    mac_user_ht_hdl_stru                 st_ht_hdl;

    /* legacy���ʼ���Ϣ */
    oal_uint8                           uc_avail_rs_nrates;
    oal_uint8                           auc_avail_rs_rates[WLAN_MAX_SUPP_RATES];

    wlan_protocol_enum_uint8            en_protocol_mode;                            /* �û�Э��ģʽ */
}mac_h2d_usr_rate_info_stru;


typedef struct
{
    oal_uint16                         us_sta_aid;
    oal_uint8                          uc_uapsd_cap;
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    oal_bool_enum_uint8                en_txop_ps;
#else
    oal_uint8                          auc_resv[1];
#endif /* #ifdef _PRE_WLAN_FEATURE_TXOPPS */
}mac_h2d_vap_info_stru;

typedef struct
{
    oal_uint16                          us_user_idx;
    wlan_protocol_enum_uint8            en_avail_protocol_mode; /* �û�Э��ģʽ */
    wlan_bw_cap_enum_uint8              en_bandwidth_cap;       /* �û�����������Ϣ */
    wlan_bw_cap_enum_uint8              en_avail_bandwidth;     /* �û���VAP������������,���㷨���� */
    wlan_bw_cap_enum_uint8              en_cur_bandwidth;       /* Ĭ��ֵ��en_avail_bandwidth��ͬ,���㷨�����޸� */
    oal_uint8                           auc_rsv[2];
}mac_h2d_user_bandwidth_stru;

typedef struct
{
    mac_channel_stru                    st_channel;
    oal_uint16                          us_user_idx;
    wlan_bw_cap_enum_uint8              en_bandwidth_cap;       /* �û�����������Ϣ */
    wlan_bw_cap_enum_uint8              en_avail_bandwidth;     /* �û���VAP������������,���㷨���� */
    wlan_bw_cap_enum_uint8              en_cur_bandwidth;       /* Ĭ��ֵ��en_avail_bandwidth��ͬ,���㷨�����޸� */
    oal_uint8                           auc_rsv[3];
}mac_d2h_syn_info_stru;

#ifdef _PRE_WLAN_DELAY_STATISTIC
#define DL_TIME_ARRAY_LEN    10
#define TID_DELAY_STAT       0
#define AIR_DELAY_STAT       1
typedef struct
{
    oal_uint16                          dl_time_array[DL_TIME_ARRAY_LEN];
    oal_uint8                           dl_measure_type;
}user_delay_info_stru;

typedef struct
{
    oal_uint32                          dmac_packet_count_num;
    oal_uint32                          dmac_report_interval;
    oal_uint8                           dmac_stat_enable;
    oal_uint8                           reserved[3];
}user_delay_switch_stru;
#endif

typedef struct
{
    oal_bool_enum_uint8                 en_11ax_cap;            /*�Ƿ�֧��11ax����*/
    oal_bool_enum_uint8                 en_radar_detector_cap;
    oal_bool_enum_uint8                 en_11n_sounding;
    wlan_bw_cap_enum_uint8              en_wlan_bw_max;

    oal_uint16                          us_beacon_period;
    oal_bool_enum_uint8                 en_green_field;
    oal_bool_enum_uint8                 en_mu_beamformee_cap;

    oal_bool_enum_uint8                 en_txopps_is_supp;
    oal_bool_enum_uint8                 uc_su_bfee_num;
    oal_bool_enum_uint8                 en_40m_shortgi;
    oal_bool_enum_uint8                 en_11n_txbf;

    oal_bool_enum_uint8                 en_40m_enable;
    oal_uint8                           uc_rsv[3];
}mac_d2h_mib_update_info_stru;

typedef struct
{
    oal_uint16                          us_user_idx;
    mac_user_asoc_state_enum_uint8      en_asoc_state;
    oal_uint8                           uc_rsv[1];
}mac_h2d_user_asoc_state_stru;

typedef struct
{
    oal_uint8 auc_addr[WLAN_MAC_ADDR_LEN];
    oal_uint8 auc_pmkid[WLAN_PMKID_LEN];
    oal_uint8 auc_resv0[2];
} mac_pmkid_info_stu;

typedef struct
{
    oal_uint8       uc_num_elems;
    oal_uint8       auc_resv0[3];
    mac_pmkid_info_stu ast_elem[WLAN_PMKID_CACHE_SIZE];
} mac_pmkid_cache_stru;

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
enum
{
    FREQ_SET_MODE                        = 0,
    /* sync ini data */
    FREQ_SYNC_DATA                       = 1,
    /* for device debug */
    FREQ_SET_FREQ                        = 2,
    FREQ_SET_PLAT_FREQ                   = 3,
    FREQ_GET_FREQ                        = 4,
    FREQ_SET_FREQ_TC_EN                  = 5,
    FREQ_SET_FREQ_TC_EXIT                = 6,
    FREQ_SET_BUTT
};
typedef oal_uint8 oal_freq_sync_enum_uint8;

typedef struct {
    oal_uint32  ul_speed_level;    /*����������*/
    oal_uint32  ul_cpu_freq_level;  /*CPUƵ��level*/
} device_level_stru;

typedef struct
{
    oal_uint8                uc_set_type;
    oal_uint8                uc_set_freq;
    oal_uint8                uc_device_freq_enable;
    oal_uint8                uc_resv;
    device_level_stru        st_device_data[4];
}config_device_freq_h2d_stru;
#endif //#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ

#ifdef _PRE_WLAN_FEATURE_TXBF
typedef struct
{
    oal_uint8   bit_imbf_receive_cap            :   1,  /*��ʽTxBf��������*/
                bit_exp_comp_txbf_cap           :   1, /*Ӧ��ѹ���������TxBf������*/
                bit_min_grouping                :   2,  /*0=�����飬1=1,2���飬2=1,4���飬3=1,2,4����*/
                bit_csi_bfee_max_rows           :   2,  /*bfer֧�ֵ�����bfee��CSI��ʾ�������������*/
                bit_channel_est_cap             :   2;  /*�ŵ����Ƶ�������0=1��ʱ�������ε���*/
    oal_uint8   auc_resv0[3];
}mac_vap_txbf_add_stru;
#endif

typedef struct
{
    /*word 0*/
    wlan_prot_mode_enum_uint8           en_protection_mode;                              /* ����ģʽ */
    oal_uint8                           uc_obss_non_erp_aging_cnt;                       /*ָʾOBSS��non erp վ����ϻ�ʱ��*/
    oal_uint8                           uc_obss_non_ht_aging_cnt;                        /*ָʾOBSS��non ht վ����ϻ�ʱ��*/
    oal_uint8                           bit_auto_protection        : 1;                  /*ָʾ���������Ƿ�����OAL_SWITCH_ON �򿪣� OAL_SWITCH_OFF �ر�*/
    oal_uint8                           bit_obss_non_erp_present   : 1;                  /*ָʾobss���Ƿ����non ERP��վ��*/
    oal_uint8                           bit_obss_non_ht_present    : 1;                  /*ָʾobss���Ƿ����non HT��վ��*/
    oal_uint8                           bit_rts_cts_protect_mode   : 1;                  /*ָrts_cts ���������Ƿ��, OAL_SWITCH_ON �򿪣� OAL_SWITCH_OFF �ر�*/
    oal_uint8                           bit_lsig_txop_protect_mode : 1;                  /*ָʾL-SIG protect�Ƿ��, OAL_SWITCH_ON �򿪣� OAL_SWITCH_OFF �ر�*/
    oal_uint8                           bit_reserved               : 3;

    /*word 1*/
    oal_uint8                           uc_sta_no_short_slot_num;                       /* ��֧��short slot��STA����*/
    oal_uint8                           uc_sta_no_short_preamble_num;                   /* ��֧��short preamble��STA����*/
    oal_uint8                           uc_sta_non_erp_num;                             /* ��֧��ERP��STA����*/
    oal_uint8                           uc_sta_non_ht_num;                              /* ��֧��HT��STA���� */
    /*word 2*/
    oal_uint8                           uc_sta_non_gf_num;                              /* ֧��ERP/HT,��֧��GF��STA���� */
    oal_uint8                           uc_sta_20M_only_num;                            /* ֻ֧��20M Ƶ�ε�STA����*/
    oal_uint8                           uc_sta_no_40dsss_cck_num;                       /* ����40M DSSS-CCK STA����  */
    oal_uint8                           uc_sta_no_lsig_txop_num;                        /* ��֧��L-SIG TXOP Protection STA���� */
} mac_protection_stru;

/*����ͬ��������صĲ���*/
typedef struct
{
    wlan_mib_ht_protection_enum_uint8   en_dot11HTProtection;
    oal_bool_enum_uint8 en_dot11RIFSMode;
    oal_bool_enum_uint8 en_dot11LSIGTXOPFullProtectionActivated;
    oal_bool_enum_uint8 en_dot11NonGFEntitiesPresent;

    mac_protection_stru st_protection;
}mac_h2d_protection_stru;

/*����ͬ�������л��Ĳ���*/
typedef struct
{
    wlan_channel_bandwidth_enum_uint8   en_40M_bandwidth;
    oal_bool_enum_uint8                 en_40M_intol_user;
    oal_uint8                           auc_resv[2];
}mac_bandwidth_stru;

typedef struct
{
    oal_uint8                          *puc_ie;                                         /* APP ��ϢԪ�� */
    oal_uint32                          ul_ie_len;                                      /* APP ��ϢԪ�س��� */
    oal_uint32                          ul_ie_max_len;                                  /* APP ��ϢԪ����󳤶� */
} mac_app_ie_stru;

#ifdef _PRE_WLAN_FEATURE_HILINK
typedef struct
{
    oal_spin_lock_stru     st_lock;
    oal_dlist_head_stru    st_list_head;
    oal_uint8              uc_white_list_member_nums;   /* hilink���������� */
#ifndef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    oal_uint8              auc_reserve[3];
#else
    oal_uint8              uc_assoc_sta_member_nums;    /* �ѹ�������STA���� */
    oal_uint8              auc_reserve[2];
#endif
}mac_okc_ssid_hidden_white_list_stru;

typedef struct
{
    oal_dlist_head_stru  st_dlist;
    oal_uint8            auc_mac_addr[WLAN_MAC_ADDR_LEN];   /*��Ӧ��MAC��ַ */
#ifndef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    oal_uint8            auc_reserve[2];
#else
    oal_uint8            uc_flag;                           /*flag��ʶ��flag��bit0��1��������bit1��1�ѹ�����*/
    oal_uint8            uc_resp_nums;                      /*ʣ��ظ�probe resp֡����*/
#endif
}mac_okc_white_list_member_stru;

/* ��¼�޳�������Ϣ�ṹ�� */
typedef struct
{
    oal_uint8  auc_user_mac_addr[6];     /* ������ɾ���û���MAC��ַ */
    oal_uint8  uc_mlme_phase_mask;
    oal_uint8  uc_rev;
} mac_fbt_disable_user_info_stru;

/* ��¼�����л���Ϣ�Ľṹ�� */
typedef struct
{
    oal_uint8  uc_fbt_mode;              /* ���ÿ����л����е�ģʽ��0����ʾ�رգ�1����ʾ����AC���п��� */
    oal_uint8  uc_disabled_user_cnt;     /* ��¼��ֹ�����б�ǰ�û��ĸ��� */
    oal_uint8  auc_rev[2];
    mac_fbt_disable_user_info_stru ast_fbt_disable_connect_user_list[HMAC_FBT_MAX_USER_NUM];  /* ����32����ֹ�����û�����Ϣ��32����Դ��������ʱ֧��3���û� */
    frw_timeout_stru st_timer;           /* �ѹ����û��ϱ��������ʹ�õĶ�ʱ�� */
} mac_fbt_mgmt_stru;

#endif

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT

typedef struct mac_vap_status_diag_info
{
    oal_uint8 uc_channel_usage;                     //ռ�ձȣ�40%��Ӧֵ40
    oal_uint8 uc_offset_second_channel;    //��ѡ�ŵ�
    oal_uint8 auc_hisi_cca_shreshold[2];         //��ȷ�Ϻ���
    oal_uint32 ul_hisi_dig;                     //��������
    oal_uint16 us_fa;                      /* invalid frame - crc err .etc*/  //1������Ч֡��������Ҫ����ͳ��
    oal_uint16 us_cca;                     /* valid frame, can be recongnized*/ //1������Ч֡����,��Ҫ����ͳ��
    oal_uint32 ul_wait_agg_time;           //�ۺϵȴ�ʱ�䣨ms)
    oal_uint32 ul_channel_busy;
    /* buffer use status */
    oal_uint32 ul_skb_remain_buffer;       //skbʣ��buff   WLAN_TID_MPDU_NUM_LIMIT- mac_device_stru->us_total_mpdu_num
    oal_uint32 ul_vo_remain_count;         //vo����ʣ��buff WLAN_TID_MPDU_NUM_LIMIT- mac_device_stru->us_total_mpdu_num
    oal_uint32 ul_vi_remain_count;         //vi����ʣ��buff
    oal_uint32 ul_be_remain_count;         //be����ʣ��buff
    oal_uint32 ul_bk_remain_count;         //bk����ʣ��buff
    /* edca setting  */
    oal_uint32 ul_vo_edca;                 //1151�ļĴ�������ֵ
    oal_uint32 ul_vi_edca;                 //hi1151_vap_get_edca_machw_cw  3.9.1
    oal_uint32 ul_be_edca;
    oal_uint32 ul_bk_edca;
} mac_vap_status_diag_info_stru;

typedef struct mac_sta_status_diag_info
{
    void *pst_mac_usr;
    oal_uint8 uc_agg_mode;                 //�оۺ�1�� 0�޾ۺ�
    oal_uint8 uc_max_agg_num;              //���ۺϸ���
    oal_uint8 uc_rts_rate;                 /* MB/s*/ //оƬRTS֡����
    oal_uint8 uc_rts_retry_cnt;            //оƬRTS֡�ط�����
    oal_int8 c_rssi;
    oal_int8 ac_reserved[3];
    oal_uint32 ul_tx_unicast_bytes;        /* becareful invert */  //���͵��������ֽ���
    oal_uint32 ul_rx_unicast_bytes;        //���յ��������ֽ���
    oal_uint32 ul_tx_mcast_bytes;          /* ���Ͷಥ�����ֽ���: �����鲥ת�������� */
    oal_uint32 ul_rx_mcast_bytes;          /* ���նಥ�����ֽ���: �����鲥ת�������� */
    oal_uint32 aul_sta_tx_mcs_cnt[16];     /* how many tx mcs count  ���ζ�ȡ����ڵ�ÿ��MCSͳ��ֵ,����0 */
    oal_uint32 aul_sta_rx_mcs_cnt[16];     /* how many rx mcs count ���ζ�ȡ����ڵ�ÿ��MCSͳ��ֵ,����0  */
    oal_uint32 ul_sleep_times;             //�ۼ����ߴ���
} mac_sta_status_diag_info_stru;

#endif

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
typedef enum
{
    PSTA_MODE_NONE = 0,
    PSTA_MODE_MSTA,
    PSTA_MODE_VSTA,
    PSTA_MODE_PBSS,
}psta_mode_enum;
typedef oal_uint8 psta_mode_enum_uint8;

typedef struct
{
    psta_mode_enum_uint8      en_psta_mode;
    oal_uint8                 auc_res[3];
}mac_vap_psta_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_SINGLE_PROXYSTA
typedef enum
{
    PROXYSTA_MODE_NONE = 0,
    PROXYSTA_MODE_SSTA,     /* SINGLE STA */
    PROXYSTA_MODE_RAP,      /* REPEATER AP */
}proxysta_mode_enum;
typedef oal_uint8 proxysta_mode_enum_uint8;
#endif

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP

#ifdef _PRE_DEBUG_MODE
typedef struct
{
    mac_ether_header_stru               st_eth_hdr;
    oal_uint8                           auc_rsv[2];
    oal_uint16                          us_proxy_num; /* ��¼�����֡������ */
    oal_uint16                          us_type;
    oal_err_code_enum_uint32            en_rst;
}mac_vap_proxyarp_debug_stru;
#endif

typedef struct
{
    oal_dlist_head_stru                 ast_ipv4_head[MAC_VAP_USER_HASH_MAX_VALUE];
    oal_dlist_head_stru                 ast_ipv6_head[MAC_VAP_USER_HASH_MAX_VALUE];
    oal_uint16                          us_ipv4_num;        /* ��¼ipv4������ */
    oal_uint16                          us_ipv6_num;        /* ��¼ipv6������ */
    oal_bool_enum_uint8                 en_is_proxyarp;
    oal_bool_enum_uint8                 en_2wlan;

#if 0//def _PRE_DEBUG_MODE
    /* ����ʹ��,����¼MAC_VAP_USER_HASH_MAX_VALUE����¼ */
    oal_uint16                          us_idx_cur; /* ѭ��ָ��ǰ��¼��λ�� */
    oal_uint8                           auc_rsv[2];
    mac_vap_proxyarp_debug_stru         ast_vap_proxyarp_dbg[MAC_VAP_USER_HASH_MAX_VALUE];
#endif
}mac_vap_proxyarp_stru;
#endif


/* Э����� ��Ӧcfgid: WLAN_CFGID_MODE */
typedef struct
{
    wlan_protocol_enum_uint8            en_protocol;    /* Э�� */
    wlan_channel_band_enum_uint8        en_band;        /* Ƶ�� */
    wlan_channel_bandwidth_enum_uint8   en_bandwidth;   /* ���� */
    oal_uint8                           en_channel_idx; /* ��20M�ŵ��� */
}mac_cfg_mode_param_stru;

typedef struct
{
    wlan_channel_band_enum_uint8        en_band;        /* Ƶ�� */
    wlan_channel_bandwidth_enum_uint8   en_bandwidth;   /* ���� */
}mac_cfg_mib_by_bw_param_stru;

typedef struct
{
    oal_uint8            uc_dscp;
    oal_uint8            uc_tid;
    oal_uint8            auc_reserve[2];
}mac_map_dscp_to_tid_stru;

#ifdef _PRE_WLAN_DFT_STAT
typedef oam_stats_vap_stat_stru mac_vap_dft_stats_stru;

typedef struct
{
    mac_vap_dft_stats_stru             *pst_vap_dft_stats;
    frw_timeout_stru                    st_vap_dft_timer;
    oal_uint32                          ul_flg;             /* ��ʼͳ�Ʊ�־ */
}mac_vap_dft_stru;
#endif

typedef struct
{
    oal_uint8    flags;
    oal_uint8    mcs;
    oal_uint16   legacy;
    oal_uint8    nss;
    oal_uint8    bw;
    oal_uint8    rsv[3];
} mac_rate_info_stru;

typedef enum mac_rate_info_flags {
    MAC_RATE_INFO_FLAGS_MCS             = BIT(0),
    MAC_RATE_INFO_FLAGS_VHT_MCS         = BIT(1),
    MAC_RATE_INFO_FLAGS_40_MHZ_WIDTH    = BIT(2),
    MAC_RATE_INFO_FLAGS_80_MHZ_WIDTH    = BIT(3),
    MAC_RATE_INFO_FLAGS_80P80_MHZ_WIDTH = BIT(4),
    MAC_RATE_INFO_FLAGS_160_MHZ_WIDTH   = BIT(5),
    MAC_RATE_INFO_FLAGS_SHORT_GI        = BIT(6),
    MAC_RATE_INFO_FLAGS_60G             = BIT(7),
} mac_rate_info_flags;




typedef struct
{
    oal_bool_enum_uint8             en_open;         /* �򿪹رմ����� */
    mac_narrow_bw_enum_uint8        en_bw;           /* 1M,5M,10M */
    oal_uint8                       auc_rsv[2];
}mac_cfg_narrow_bw_stru;


#ifdef _PRE_WLAN_FEATURE_STA_PM
/* STA UAPSD �������� */
typedef	 struct
{
	oal_uint8		uc_max_sp_len;
	oal_uint8		uc_delivery_enabled[WLAN_WME_AC_BUTT];
	oal_uint8	    uc_trigger_enabled[WLAN_WME_AC_BUTT];
}mac_cfg_uapsd_sta_stru;

/* Power save modes specified by the user */
typedef enum
{
    NO_POWERSAVE     = 0,
    MIN_FAST_PS      = 1,       /* idle ʱ�����ini�е�����*/
    MAX_FAST_PS      = 2,       /* idle ʱ�����ini�е�����*/
    AUTO_FAST_PS     = 3,       /* �����㷨�Զ�����idleʱ����ini�е����޻�������ֵ*/
    MAX_PSPOLL_PS    = 4,
    NUM_PS_MODE      = 5
} ps_user_mode_enum;

#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS
/* STA txopps aidͬ�� */
typedef struct
{
    oal_uint16                           us_partial_aid;
    oal_uint8                            en_protocol;
    oal_uint8                            uc_enable;
}mac_cfg_txop_sta_stru;
#endif

typedef enum
{
    AMPDU_SWITCH_BY_DEL_BA = 0,
    AMPDU_SWITCH_BY_BA_LUT,

    AMPDU_SWITCH_MODE_BUTT
}mac_ampdu_switch_mode;
typedef oal_uint8 mac_ampdu_switch_mode_enum_uint8;

typedef enum mac_vowifi_mkeep_alive_type
{
    VOWIFI_MKEEP_ALIVE_TYPE_STOP  = 0,
    VOWIFI_MKEEP_ALIVE_TYPE_START = 1,
    VOWIFI_MKEEP_ALIVE_TYPE_BUTT
}mac_vowifi_nat_keep_alive_type_enum;
typedef oal_uint8 mac_vowifi_nat_keep_alive_type_enum_uint8;


typedef struct
{
    oal_uint8                                   uc_keep_alive_id;
    mac_vowifi_nat_keep_alive_type_enum_uint8   en_type;
    oal_uint8                                   auc_rsv[2];
}mac_vowifi_nat_keep_alive_basic_info_stru;

typedef struct
{
    mac_vowifi_nat_keep_alive_basic_info_stru   st_basic_info;
    oal_uint8                                   auc_src_mac[WLAN_MAC_ADDR_LEN];
    oal_uint8                                   auc_dst_mac[WLAN_MAC_ADDR_LEN];
    oal_uint32                                  ul_period_msec;
    oal_uint16                                  us_ip_pkt_len;
    oal_uint8                                   auc_rsv[2];
    oal_uint8                                   auc_ip_pkt_data[4];
}mac_vowifi_nat_keep_alive_start_info_stru;

#ifdef _PRE_WLAN_FEATURE_VOWIFI
/* vowifi��������������������� */
typedef enum
{
    VOWIFI_SET_MODE = 0,
    VOWIFI_GET_MODE,
    VOWIFI_SET_PERIOD,
    VOWIFI_GET_PERIOD,
    VOWIFI_SET_LOW_THRESHOLD,
    VOWIFI_GET_LOW_THRESHOLD,
    VOWIFI_SET_HIGH_THRESHOLD,
    VOWIFI_GET_HIGH_THRESHOLD,
    VOWIFI_SET_TRIGGER_COUNT,
    VOWIFI_GET_TRIGGER_COUNT,
    VOWIFI_SET_IS_SUPPORT,

    VOWIFI_CMD_BUTT
}mac_vowifi_cmd_enum;
typedef oal_uint8 mac_vowifi_cmd_enum_uint8;

/* vowifi��������������������ṹ�� */
typedef struct
{
    mac_vowifi_cmd_enum_uint8   en_vowifi_cfg_cmd; /* �������� */
    oal_uint8                   uc_value;          /* ����ֵ */
    oal_uint8                   auc_resv[2];
}mac_cfg_vowifi_stru;


/* VoWiFi�ź��������� �� ���ò����ṹ�� */
typedef enum
{
    VOWIFI_DISABLE_REPORT   = 0,
    VOWIFI_LOW_THRES_REPORT,
    VOWIFI_HIGH_THRES_REPORT,
    VOWIFI_CLOSE_REPORT     = 3,/*�ر�VoWIFI*/

    VOWIFI_MODE_BUTT        = 3
} mac_vowifi_mode;
typedef oal_uint8 mac_vowifi_mode_enum_uint8;

typedef struct
{
/*  MODE
	0: disable report of rssi change
	1: enable report when rssi lower than threshold(vowifi_low_thres)
	2: enable report when rssi higher than threshold(vowifi_high_thres)
*/
    mac_vowifi_mode_enum_uint8  en_vowifi_mode;
    oal_uint8                   uc_trigger_count_thres;   /*��1��100��, the continuous counters of lower or higher than threshold which will trigger the report to host */
    oal_int8                    c_rssi_low_thres;         /* [-1, -100],vowifi_low_thres */
    oal_int8                    c_rssi_high_thres;        /* [-1, -100],vowifi_high_thres */
    oal_uint16                  us_rssi_period_ms;        /* ��λms, ��Χ��1s��30s��, the period of monitor the RSSI when host suspended */

    oal_bool_enum_uint8         en_vowifi_reported;       /* ���vowifi�Ƿ��ϱ���һ��"״̬�л�����"���������ϱ� */
    oal_uint8                   uc_cfg_cmd_cnt;           /* �ϲ��·�������vowifi�����Ĵ���ͳ�ƣ����ڱ���Ƿ����ײ������·���ȫ����ֹ�·��������̺�vowifi�����ϱ������ص� */
    oal_uint8                   _rom[4];
}mac_vowifi_param_stru;

#endif /* _PRE_WLAN_FEATURE_VOWIFI */

#define MAC_VAP_INVAILD    (0x0) /*0Ϊvap��Ч*/
#define MAC_VAP_VAILD      (0x2b)
typedef struct
{
    oal_bool_enum_uint8                 en_rate_cfg_tag;
    oal_bool_enum_uint8                 en_tx_ampdu_last;
    oal_uint8                           uc_reserv[2];
}mac_fix_rate_cfg_para_stru;

typedef struct
{
    oal_uint8                                   uc_test_ie_info[MAC_TEST_INCLUDE_FRAME_BODY_LEN];
    oal_bool_enum_uint8                         en_include_test_ie;
    oal_uint8                                   uc_test_ie_len;
    oal_uint8                                   uc_include_times;
    oal_uint8                                   uc_reserv[1];
}mac_beacon_include_test_ie_stru;

#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE
/* ����hera���ù���֡��ie���� */
/* ÿ��IE����һ��ָ���ʾ������ָ���ֵ��Ϊ��ʱ������Ҫ����IE���� */
typedef struct
{
    mac_eid_enum_uint8              en_eid;                 /* ��Ҫ���õ�element id */
    oal_ie_set_type_enum_uint8      en_set_type;            /* IE���õ����� */
    oal_uint16                      us_ie_content_len;      /* IEdata�ֶεĳ���,����HDR���� */
    oal_uint8                       auc_ie_content[0];      /* IE DATA�ֶε�ֵ,�ɱ䳤�� */
}mac_vap_ie_set_stru;

#endif  // end of _PRE_WLAN_FEATURE_11KV_INTERFACE

#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
typedef struct
{
    oal_dlist_head_stru              st_qos_enhance_entry;
    oal_uint8                        auc_qos_enhance_mac[WLAN_MAC_ADDR_LEN];/* qos_enhance������mac��ַ */
    oal_uint8                        uc_delete_num;                         /* �����յ�delete��Ϣ���� */
    oal_uint8                        uc_add_num;                            /* �����յ�add��Ϣ���� */
}mac_qos_enhance_sta_stru;

typedef struct
{
    oal_spin_lock_stru                st_lock;
    oal_dlist_head_stru               st_list_head;/* qos_enhance���� */
    oal_uint8                         uc_qos_enhance_sta_count; /* qos_enhance������STA���� */
    oal_bool_enum_uint8               en_qos_enhance_enable; /* ����qos enhance����ʹ�� */
    oal_uint8                         uc_reserv[2];
}mac_qos_enhance_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_11AX
typedef struct
{
    oal_uint8                               bit_new_bss_color_info             :6,
                                            bit_new_bss_color_exit             :2;
    oal_uint8                               uc_rsv[3];
}mac_vap_he_config_info_stru;
#endif

typedef struct
{
#ifdef _PRE_WLAN_FEATURE_11AX
    wlan_mib_Dot11EDCAEntry_stru            st_wlan_mib_mu_edca;
    wlan_mid_Dot11HePhyCapability_rom_stru  st_wlan_rom_mib_he_phy_config;
    mac_vap_he_config_info_stru             st_he_config_info;
#endif
    oal_uint8                          auc_rsv[4];

}mac_vap_rom_stru;
extern mac_vap_rom_stru g_mac_vap_rom[];

/* VAP�����ݽṹ */
typedef struct
{
    /* VAPΪAP����STA���г�Ա */
    /* word0~word1 */
    oal_uint8                           uc_vap_id;                                      /* vap ID   *//* ����Դ������ֵ */
    oal_uint8                           uc_device_id;                                   /* �豸ID   */
    oal_uint8                           uc_chip_id;                                     /* chip ID  */
    wlan_vap_mode_enum_uint8            en_vap_mode;                                    /* vapģʽ  */
    oal_uint32                          ul_core_id;

    /* word2~word3 */
    oal_uint8                           auc_bssid[WLAN_MAC_ADDR_LEN];                   /* BSSID����MAC��ַ��MAC��ַ��mib�е�auc_dot11StationID  */
    mac_vap_state_enum_uint8            en_vap_state;                                   /* VAP״̬ */
    wlan_protocol_enum_uint8            en_protocol;                                    /* ������Э��ģʽ */

    /* word4~word5 */
    mac_channel_stru                    st_channel;                                     /* vap���ڵ��ŵ� */
    mac_ch_switch_info_stru             st_ch_switch_info;
    mac_sta_csa_fsm_info_stru           st_sta_csa_fsm_info;
    /* word6 */
    oal_uint8                           bit_has_user_bw_limit   :   1;                  /* ��vap�Ƿ����user���� */
    oal_uint8                           bit_vap_bw_limit        :   1;                  /* ��vap�Ƿ������� */
    oal_uint8                           bit_voice_aggr          :   1;                  /* ��vap�Ƿ����VOҵ��֧�־ۺ� */
    oal_uint8                           bit_one_tx_tcp_be       :   1;                  /* ��vap�Ƿ�ֻ��1·����TCP BEҵ�� */
    oal_uint8                           bit_one_rx_tcp_be       :   1;                  /* ��vap�Ƿ�ֻ��1·����TCP BEҵ�� */
    oal_uint8                           bit_no_tcp_or_udp       :   1;                  /* ��vapû����TCP��UDPҵ�� */
    oal_uint8                           bit_bw_fixed            :   1;                  /* APģʽ��VAP�Ĵ����Ƿ�̶�20M */
    oal_uint8                           bit_use_rts_threshold   :   1;                  /* ��vap��RTS�����Ƿ�ʹ��Э��涨��RTS���� */

    oal_uint8                           uc_tx_power;                                    /* ���书��, ��λdBm */
    oal_uint8                           uc_channel_utilization;                         /* ��ǰ�ŵ������� */
    oal_uint8                           uc_wmm_params_update_count;                     /* ��ʼΪ0��APģʽ�£�ÿ����һ��wmm�������������1,��beacon֡��assoc rsp�л���д��4bit�����ܳ���15��STAģʽ�½���֡���������ֵ */

    /* word7 */
    oal_uint16                          us_user_nums;                                   /* VAP���ѹҽӵ��û����� */
    oal_uint16                          us_multi_user_idx;                              /* �鲥�û�ID */
    oal_uint8                           auc_cache_user_mac_addr[WLAN_MAC_ADDR_LEN];     /* cache user��Ӧ��MAC��ַ */
    oal_uint16                          us_cache_user_id;                               /* cache user��Ӧ��userID*/
    oal_dlist_head_stru                 ast_user_hash[MAC_VAP_USER_HASH_MAX_VALUE];     /* hash����,ʹ��HASH�ṹ�ڵ�DLIST */
    oal_dlist_head_stru                 st_mac_user_list_head;                          /* �����û��ڵ�˫������,ʹ��USER�ṹ�ڵ�DLIST */

    /* word8 */
    wlan_nss_enum_uint8                 en_vap_rx_nss;                                  /* vap�Ľ��տռ������� */
    /*vapΪ��̬��Դ�����VAP��û�б����롣,
      DMAC OFFLOADģʽVAP��ɾ������˻����֡*/
    oal_uint8                           uc_init_flag;
    oal_uint16                          us_mgmt_frame_filters;                    /* ����֡���ˣ��ο� en_fbt_mgmt_frame_filter_enum */
#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
    oal_uint8                           uc_bsd_switch;
    oal_uint8                           auc_resv7[3];
#endif
    /* word9 ���ӵ�SPE �˿ں�,һ��VAP��Ӧһ���˿ں� */
#if (defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT))
    oal_uint32                          ul_spe_portnum;                                 /* ���ڱ���netdev��ʼ����spe�˿� */
#endif
#if (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT))
    oal_uint32                          ul_fastip_idx;                                 /* ���ڱ���netdev��ʼ�� �� fastip index */
#endif

    /* ������� */
    /*�ṹ����Ҫ��֤4�ֽڶ���*/
    //mac_key_mgmt_stru                   st_key_mgmt;                                    /*���ڱ�������ptk gtk����Կ��Ϣ����AP/STAģʽ�¾���Ҫʹ��*/

    mac_cap_flag_stru                   st_cap_flag;                                    /* vap�������Ա�ʶ */
#if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    mac_vap_stats_stru                  st_vap_stats;                                   /* VAP�շ���ͳ�� */
#endif
    wlan_mib_ieee802dot11_stru         *pst_mib_info;                                   /* mib��Ϣ(��ʱ����vapʱ������ֱ�ӽ�ָ��ֵΪNULL����ʡ�ռ�)  */

    mac_curr_rateset_stru               st_curr_sup_rates;                              /* ��ǰ֧�ֵ����ʼ� */
    mac_curr_rateset_stru               ast_sta_sup_rates_ie[WLAN_BAND_BUTT];           /* ֻ��staȫ�ŵ�ɨ��ʱʹ�ã�������д֧�ֵ����ʼ�ie����2.4��5G */

#ifdef _PRE_WLAN_DFT_STAT
    /* user ����ά�� */
    oal_uint32          ul_dlist_cnt;     /* dlsitͳ�� */
    oal_uint32          ul_hash_cnt;     /* hash����ͳ�� */
#if 0
    mac_vap_dft_stru                    st_vap_dft;                                     /* vap����ͳ����Ϣ��ά���� */
#endif

#endif

#ifdef _PRE_WLAN_FEATURE_TXBF
    mac_vap_txbf_add_stru               st_txbf_add_cap;
#endif
#ifdef _PRE_WLAN_FEATURE_DHCP_REQ_DISABLE
    oal_bool_enum_uint8                 en_dhcp_req_disable_switch;                     /* dhcp req disable���Կ���*/
    oal_uint8                           auc_resv8[3];  /* 4�ֽڶ��� */
#endif

#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE
    mac_vap_ie_set_stru          *pst_rrm_ie_info;                                      /* ����rrm ie :70 */
    mac_vap_ie_set_stru          *pst_excap_ie_info;                                    /* ����extend capbility ie: 127 */
    mac_vap_ie_set_stru          *pst_msta_ie_info;                                     /* ����Multi-STA 4��ַie */
#endif

    /* VAPΪAP����STA���г�Ա �������*/


    /* VAPΪAP���г�Ա�� ���忪ʼ*/
    mac_protection_stru                 st_protection;                                   /*�뱣����ر���*/
    mac_app_ie_stru                     ast_app_ie[OAL_APP_IE_NUM];
    oal_bool_enum_uint8                 en_40M_intol_user;                               /* ap���Ƿ��й�����40M intolerant��user*/
    oal_bool_enum_uint8                 en_vap_wmm;                                      /* ap wmm ���� */
#ifdef _PRE_WLAN_PRODUCT_1151V200
    oal_uint8                           uc_1st_asoc_done;                               /* ��һ�ι��� */
    oal_uint8                           uc_force_resp_mode_80M;                         /* sifs��Ӧ֡ʹ��80M���� */
#else
    oal_uint8                           uc_up_vap_shift_type;
    oal_uint8                           uc_random_mac;
#endif
#ifdef _PRE_WLAN_FEATURE_HILINK
    mac_okc_ssid_hidden_white_list_stru    st_okc_ssid_hidden_white_list;
    mac_fbt_mgmt_stru                      st_fbt_mgmt;                               /* ��¼fbt������Ϣ����ֹ�����б� */
#endif

    /* VAPΪAP�ض���Ա�� �������*/

    /* VAPΪSTA���г�Ա�� ���忪ʼ*/

    oal_uint16                          us_sta_aid;                                     /* VAPΪSTAģʽʱ����AP�����STA��AID(����Ӧ֡��ȡ),ȡֵ��Χ1~2007; VAPΪAPģʽʱ�����ô˳�Ա���� */
    oal_uint16                          us_assoc_vap_id;                                /* VAPΪSTAģʽʱ����user(ap)����Դ��������VAPΪAPģʽʱ�����ô˳�Ա���� */
    oal_uint8                           uc_uapsd_cap;                                   /* ������STA������AP�Ƿ�֧��uapsd������Ϣ */
    oal_uint16                          us_assoc_user_cap_info;                         /* staҪ�������û���������Ϣ */
    oal_uint8                           bit_ap_11ntxbf              : 1,                /* staҪ�������û���11n txbf������Ϣ */
                                        bit_rsp_soft_ctl_launcher   : 1,
                                        bit_roam_scan_valid_rslt    : 1,
                                        bit_resv5                   : 5;
    /* VAPΪSTA�ض���Ա�� �������*/

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    mac_vap_psta_stru                   st_psta;                                /* Proxy STA ������ؽṹ�� */
#endif

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
    mac_vap_proxyarp_stru              *pst_vap_proxyarp;
#endif

#ifdef _PRE_WLAN_NARROW_BAND
    mac_cfg_narrow_bw_stru              st_nb;
#endif

//#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    /* ��������ʹ�� */
    oal_uint8                           bit_al_tx_flag  :1; /* ������־ */
    oal_uint8                           bit_payload_flag:2; /* payload����:0:ȫ0  1:ȫ1  2:random */
    oal_uint8                           bit_ack_policy:1;   /* ack_policy����:0:normal 1:no ack , Ŀǰ��ʹ��*/

    /* ���ղ���ʹ�� */
    oal_uint8                           bit_al_rx_flag  :1; /* ���ձ�־ */
    oal_uint8                           bit_reserved:3;
//#endif

//#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8            en_p2p_mode;                                    /* 0:��P2P�豸; 1:P2P_GO; 2:P2P_Device; 3:P2P_CL */
    oal_uint8                           uc_p2p_gocl_hal_vap_id;                         /* p2p go / cl��hal vap id */
    oal_uint8                           uc_p2p_listen_channel;                          /* P2P Listen channel */
    //oal_uint8                           auc_resv6[1];
//#endif    /* _PRE_WLAN_FEATURE_P2P */

#ifdef _PRE_WLAN_FEATURE_STA_PM
    mac_cfg_uapsd_sta_stru              st_sta_uapsd_cfg;     /* UAPSD��������Ϣ */
#endif

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    oal_bool_enum_uint8                 en_user_pmf_cap;      /* STA����δ����userǰ���洢Ŀ��user��pmfʹ����Ϣ */
#endif

    oal_spin_lock_stru                  st_cache_user_lock;                        /* cache_user lock */
#ifdef _PRE_WLAN_FEATURE_VOWIFI
    mac_vowifi_param_stru              *pst_vowifi_cfg_param;  /* �ϲ��·���"VoWiFi�ź���������"�����ṹ�� */
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
    mac_fix_rate_cfg_para_stru          st_fix_rate_pre_para;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
#ifdef _PRE_DEBUG_MODE
    oal_work_stru                       reg_file_work;
#endif
    oal_work_stru                       debug_reg_work;
    oal_work_stru                       show_pow_work;
#endif
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)  ||( _PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV))
    mac_beacon_include_test_ie_stru     st_ap_beacon_test_ie;
#endif
#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
    oal_uint8                           uc_he_mu_edca_update_count;
    oal_bool_enum_uint8                 en_11ax_custom_switch;
    oal_bool_enum_uint8                 en_11ax_hal_cap;
    oal_uint8                           auc_resv9[1];            /* 4�ֽڶ��� */
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
    oal_bool_enum_uint8                 en_radar_detector_is_supp;/*��·hal device��֧���״�̽�⣬�����ж�*/
    oal_uint8                           auc_resv10[3];            /* 4�ֽڶ��� */
#endif
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
    mac_qos_enhance_stru                st_qos_enhance;
#endif
    oal_void                            *_rom;
#ifdef _PRE_OS_VERSION_RAW
    oal_uint8                            auc_rom_rsv[4];
#endif

}mac_vap_stru;

/* cfg id��Ӧ��get set���� */
typedef struct
{
    wlan_cfgid_enum_uint16          en_cfgid;
    oal_uint8                       auc_resv[2];    /* �ֽڶ��� */
    oal_uint32                      (*p_get_func)(mac_vap_stru *pst_vap, oal_uint8 *puc_len, oal_uint8 *puc_param);
    oal_uint32                      (*p_set_func)(mac_vap_stru *pst_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
}mac_cfgid_stru;


/* cfg id��Ӧ�Ĳ����ṹ�� */
/* ����VAP�����ṹ��, ��Ӧcfgid: WLAN_CFGID_ADD_VAP */
typedef struct
{
    wlan_vap_mode_enum_uint8  en_vap_mode;
    oal_uint8                 uc_cfg_vap_indx;
    oal_uint16                us_muti_user_id;          /* ���vap ��Ӧ��muti user index */

    oal_uint8                 uc_vap_id;                /* ��Ҫ��ӵ�vap id */
    wlan_p2p_mode_enum_uint8  en_p2p_mode;              /* 0:��P2P�豸; 1:P2P_GO; 2:P2P_Device; 3:P2P_CL */
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    psta_mode_enum_uint8      en_psta_mode;
    oal_uint8                 uc_rep_id;
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    oal_uint8                 bit_11ac2g_enable         :1;
    oal_uint8                 bit_disable_capab_2ght40  :1;
    oal_uint8                 bit_reserve               :6;

#ifdef _PRE_WLAN_FEATURE_DBDC
    oal_uint8                 uc_dst_hal_dev_id;      //vap�ҽӵ�hal device id
#else
    oal_uint8                 auc_resv0[1];
#endif
#else
#ifdef _PRE_WLAN_FEATURE_DBDC
    oal_uint8                 uc_dst_hal_dev_id;      //vap�ҽӵ�hal device id
    oal_uint8                 auc_resv0[1];
#else
    oal_uint8                 auc_resv0[2];
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_UAPSD
    oal_uint8                 bit_uapsd_enable    :1;
    oal_uint8                 bit_reserve1        :7;
    oal_uint8                 auc_resv1[3];
#endif
#ifdef _PRE_WLAN_FEATURE_SINGLE_PROXYSTA
    proxysta_mode_enum_uint8  en_proxysta_mode;
    oal_uint8                 auc_resv2[3];
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    oal_bool_enum_uint8       en_11ax_custom_switch;
#endif
    oal_net_device_stru      *pst_net_dev;
}mac_cfg_add_vap_param_stru;

typedef mac_cfg_add_vap_param_stru mac_cfg_del_vap_param_stru;

/* ����VAP�����ṹ�� ��Ӧcfgid: WLAN_CFGID_START_VAP */
typedef struct
{
    oal_bool_enum_uint8         en_mgmt_rate_init_flag;              /* start vapʱ�򣬹���֡�����Ƿ���Ҫ��ʼ�� */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8            uc_protocol;
    oal_uint8            uc_band;
    oal_uint8            uc_bandwidth;
#else
    oal_uint8            auc_resv1[3];
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8   en_p2p_mode;
    oal_uint8                  auc_resv2[3];
#endif
    oal_int32                  l_ifindex;
    oal_net_device_stru       *pst_net_dev;     /* �˳�Ա����Host(WAL&HMAC)ʹ�ã�Device��(DMAC&ALG&HAL��)��ʹ�� */
}mac_cfg_start_vap_param_stru;
typedef mac_cfg_start_vap_param_stru mac_cfg_down_vap_param_stru;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
/* CFG VAP h2d */
typedef struct
{
    oal_net_device_stru        *pst_net_dev;
}mac_cfg_vap_h2d_stru;
#endif


/* ����mac��ַ���� ��Ӧcfgid: WLAN_CFGID_STATION_ID */
typedef struct
{
    oal_uint8                   auc_station_id[WLAN_MAC_ADDR_LEN];
    wlan_p2p_mode_enum_uint8    en_p2p_mode;
    oal_uint8                   auc_resv[1];
}mac_cfg_staion_id_param_stru;

/* SSID���� ��Ӧcfgid: WLAN_CFGID_SSID */
typedef struct
{
    oal_uint8   uc_ssid_len;
    oal_uint8   auc_resv[2];
    oal_int8    ac_ssid[WLAN_SSID_MAX_LEN];
}mac_cfg_ssid_param_stru;

/* ��ȡ���ַ������� */
typedef struct
{
    oal_int32   l_buff_len;
    oal_uint8   auc_buff[WLAN_IWPRIV_MAX_BUFF_LEN];
}mac_cfg_param_char_stru;

#ifdef _PRE_WLAN_WEB_CMD_COMM
/* iwpriv ��ȡ��vap�������ò��� */
typedef struct
{
    oal_uint32  bit_uapsd                           : 1,
                bit_short_gi_20m                    : 1,
                bit_short_gi_40m                    : 1,
                bit_hide_ssid                       : 1,                 /* AP��������ssid,  0-�ر�, 1-����*/
                bit_11ntxbf                         : 1,                 /* 11n txbf���� */
                bit_tx_stbc                         : 1,
                bit_rx_stbc                         : 1,
                bit_ldpc                            : 1,
                bit_qos                             : 1,
                bit_short_preamble                  : 1,
                bit_frameburst                      : 1,
                bit_ampdu                           : 1,
                bit_ampdu_amsdu                     : 1,
                bit_resv                            : 19;
}mac_vap_param_stru;

/* ��ȡ��֧���ŵ��б� */
typedef struct
{
    oal_uint8   uc_total_chan_num;
    oal_uint8   auc_chan[MAC_MAX_SUPP_CHANNEL];
}mac_chan_list_stru;

#endif

/* HOSTAPD ���ù���Ƶ�Σ��ŵ��ʹ������ */
typedef struct
{
    wlan_channel_band_enum_uint8        en_band;        /* Ƶ�� */
    wlan_channel_bandwidth_enum_uint8   en_bandwidth;   /* ���� */
    oal_uint8                           uc_channel;     /* �ŵ���� */
    oal_uint8                           auc_resv[1];    /* ����λ */
}mac_cfg_channel_param_stru;

/* HOSTAPD ����wiphy �����豸��Ϣ������RTS ����ֵ����Ƭ��������ֵ */
typedef struct
{
    oal_uint8  uc_frag_threshold_changed;
    oal_uint8  uc_rts_threshold_changed;
    oal_uint8  uc_rsv[2];
    oal_uint32 ul_frag_threshold;
    oal_uint32 ul_rts_threshold;
}mac_cfg_wiphy_param_stru;

/* HOSTAPD ���� Beacon ��Ϣ */
typedef struct
{
    oal_int32                l_interval;                            /* beacon interval */
    oal_int32                l_dtim_period;                         /* DTIM period     */
    oal_bool_enum_uint8      en_privacy;
    oal_uint8                auc_rsn_ie[MAC_MAX_RSN_LEN];
    oal_uint8                auc_wpa_ie[MAC_MAX_RSN_LEN];
    oal_bool_enum_uint8      uc_hidden_ssid;
    oal_bool_enum_uint8      en_shortgi_20;
    oal_bool_enum_uint8      en_shortgi_40;
    oal_bool_enum_uint8      en_shortgi_80;
    wlan_protocol_enum_uint8 en_protocol;

    oal_uint8                       uc_smps_mode;
    mac_beacon_operation_type_uint8 en_operation_type;
#ifdef _PRE_WLAN_FEATURE_11R_AP
    oal_uint16               us_mdid;
    oal_bool_enum_uint8      en_ft_bss_transition;
    oal_bool_enum_uint8      en_ft_over_ds;
    oal_bool_enum_uint8      en_ft_resource_req;
    oal_uint8                auc_resv[1];
#else
    oal_uint8                auc_resv1[2];
#endif
}mac_beacon_param_stru;


/* ����logģ�鿪�ص������������ */
typedef struct
{
    oam_module_id_enum_uint16           en_mod_id;      /* ��Ӧ��ģ��id */
    oal_bool_enum_uint8                 en_switch;      /* ��Ӧ�Ŀ������� */
    oal_uint8                           auc_resv[1];
}mac_cfg_log_module_param_stru;

/* �û���ص������������ */
typedef struct
{
    oal_uint8               auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* MAC��ַ */
    oal_bool_enum_uint8     en_ht_cap;                          /* ht���� */
    oal_uint8               auc_resv[3];
    oal_uint16              us_user_idx;                        /* �û����� */
}mac_cfg_add_user_param_stru;

typedef mac_cfg_add_user_param_stru mac_cfg_del_user_param_stru;

/* ����֡��FCSͳ����Ϣ */
typedef struct
{
    oal_uint32  ul_data_op;         /* ���ݲ���ģʽ:<0>����,<1>��� */
    oal_uint32  ul_print_info;      /* ��ӡ��������:<0>�������� <1>��֡�� <2>self fcs correct, <3>other fcs correct, <4>fcs error  */
}mac_cfg_rx_fcs_info_stru;

/* �޳��û������������ */
typedef struct
{
    oal_uint8               auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* MAC��ַ */
    oal_uint16              us_reason_code;                     /* ȥ���� reason code */
#ifdef _PRE_WLAN_FEATURE_HILINK
    oal_uint8               uc_rej_user;                        /* ��ֹsta����, TRUE:��Ӿ���STA, FALSE:ɾ������STA */
    oal_uint8               uc_kick_user;                       /* ����:����/��ֹ���� TRUE:�ߵ�sta, */
    oal_uint8               uc_mlme_phase_mask;                 /* �����׶����� */
    oal_uint8               uc_rsv;
#endif
}mac_cfg_kick_user_param_stru;

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
/* ����Proxy STA OMA����������� */
typedef struct
{
    oal_uint8               auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* Proxy STA OMA��ַ */
    oal_uint8               auc_resv[2];
}mac_cfg_set_oma_param_stru;

#endif

/* ��ͣtid����������� */
typedef struct
{
    oal_uint8               auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* MAC��ַ */
    oal_uint8               uc_tid;
    oal_uint8               uc_is_paused;
}mac_cfg_pause_tid_param_stru;

/* �����û��Ƿ�Ϊvip */
typedef struct
{
    oal_uint8               auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* MAC��ַ */
    oal_uint8               uc_vip_flag;
}mac_cfg_user_vip_param_stru;

/* ��ͣtid����������� */
typedef struct
{
    oal_uint8                          uc_aggr_tx_on;
    oal_uint8                          uc_snd_type;
    mac_ampdu_switch_mode_enum_uint8   en_aggr_switch_mode;
    oal_uint8                          uc_rsv;
}mac_cfg_ampdu_tx_on_param_stru;


#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
typedef struct
{
    mac_tcp_ack_buf_cfg_cmd_enum_uint8   en_cmd;
    oal_bool_enum_uint8                 en_enable;
    oal_uint8                           uc_timeout_ms;
    oal_uint8                           uc_count_limit;
}mac_cfg_tcp_ack_buf_stru;
#endif


#ifdef _PRE_FEATURE_FAST_AGING
typedef struct
{
    mac_fast_aging_cfg_cmd_enum_uint8   en_cmd;
    oal_bool_enum_uint8                 en_enable;
    oal_uint16                          us_timeout_ms;
    oal_uint8                           uc_count_limit;
    oal_uint8                           auc_resv[3];
}mac_cfg_fast_aging_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_CAR
/* ����car���ò��� */
typedef struct
{
    mac_car_type_enum_uint8         en_car_flag;           //7�ֲ�������
    oal_uint8                       uc_car_up_down_type;   //����Ϊ0������Ϊ1
    oal_uint16                      us_car_timer_cycle_ms;
    oal_uint32                      ul_bw_limit_kbps;
    oal_uint8                       auc_user_macaddr[WLAN_MAC_ADDR_LEN];
    oal_bool_enum_uint8             en_car_enable_flag;
    oal_uint8                       auc_resv[1];
    oal_uint32                      ul_car_multicast_pps_num;
}mac_cfg_car_stru;
#endif
typedef struct
{
    mac_rssi_limit_type_enum_uint8  en_rssi_limit_type;           //4�ֲ�������
    oal_bool_enum_uint8             en_rssi_limit_enable_flag;
    oal_int8                        c_rssi;
    oal_int8                        c_rssi_delta;
}mac_cfg_rssi_limit_stru;

#ifdef _PRE_WLAN_FEATUER_PCIE_TEST
/*PCIE TEST �����������*/
typedef struct
{
    oal_uint8               uc_rdata;
    oal_uint8               uc_wdata;
    oal_uint16              us_burst;
}mac_cfg_pcie_test_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
/* ����hostĳ�����е�ÿ�ε��ȱ��ĸ�����low_waterline, high_waterline */
typedef struct
{
    oal_uint8               uc_queue_type;
    oal_uint8               auc_resv[1];
    oal_uint16              us_burst_limit;
    oal_uint16              us_low_waterline;
    oal_uint16              us_high_waterline;
}mac_cfg_flowctl_param_stru;
#endif

/* ʹ��qempty���� */
typedef struct
{
    oal_uint8   uc_is_on;
    oal_uint8   auc_resv[3];
}mac_cfg_resume_qempty_stru;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
/* ʹ��rx intr fifo���� */
typedef struct
{
    oal_uint8   uc_is_on;
    oal_uint8   auc_resv[3];
}mac_cfg_resume_rx_intr_fifo_stru;
#endif

/* ����mpdu/ampdu�������  */
typedef struct
{
    oal_uint8               uc_tid;
    oal_uint8               uc_packet_num;
    oal_uint16              us_packet_len;
    oal_uint8               auc_ra_mac[OAL_MAC_ADDR_LEN];
}mac_cfg_mpdu_ampdu_tx_param_stru;

typedef struct
{
    oal_uint8                uc_show_profiling_type; /* 0:Rx 1: Tx */
    oal_uint8                uc_show_level; /* 0:�ֶ������ȷ�� 1:ÿ���ڵ㡢�ֶζ������ȷ�� 2ÿ���ڵ㡢�ֶζ�������е� */
}mac_cfg_show_profiling_param_stru;

/* AMPDU��ص������������ */
typedef struct
{
    oal_uint8                       auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* �û���MAC ADDR */
    oal_uint8                       uc_tidno;                           /* ��Ӧ��tid�� */
    oal_uint8                       auc_reserve[1];                      /* ȷ�ϲ��� */
}mac_cfg_ampdu_start_param_stru;

typedef mac_cfg_ampdu_start_param_stru mac_cfg_ampdu_end_param_stru;

/* BA�Ự��ص������������ */
typedef struct
{
    oal_uint8                       auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* �û���MAC ADDR */
    oal_uint8                       uc_tidno;                           /* ��Ӧ��tid�� */
    mac_ba_policy_enum_uint8        en_ba_policy;                       /* BAȷ�ϲ��� */
    oal_uint16                      us_buff_size;                       /* BA���ڵĴ�С */
    oal_uint16                      us_timeout;                         /* BA�Ự�ĳ�ʱʱ�� */
}mac_cfg_addba_req_param_stru;

typedef struct
{
    oal_uint8                       auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* �û���MAC ADDR */
    oal_uint8                       uc_tidno;                           /* ��Ӧ��tid�� */
    mac_delba_initiator_enum_uint8  en_direction;                       /* ɾ��ba�Ự�ķ���� */
    mac_delba_trigger_enum_uint8    en_trigger;                         /* ɾ��ԭ�� */
}mac_cfg_delba_req_param_stru;
#ifdef _PRE_WLAN_FEATURE_CSI
typedef struct
{
    oal_uint8                       auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* csi��Ӧ��MAC ADDR */
    oal_bool_enum_uint8             en_ta_check;                        /* �ϱ�CSIʱ�Ƿ�check ta */
    oal_bool_enum_uint8             en_csi;                             /* csiʹ�� */
}mac_cfg_csi_param_stru;
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
/*TSPEC��ص������������*/
typedef struct
{
    mac_ts_info_stru  ts_info;
    oal_uint8         uc_rsvd;
    oal_uint16        us_norminal_msdu_size;
    oal_uint16        us_max_msdu_size;
    oal_uint32        ul_min_data_rate;
    oal_uint32        ul_mean_data_rate;
    oal_uint32        ul_peak_data_rate;
    oal_uint32        ul_min_phy_rate;
    oal_uint16        us_surplus_bw;
    oal_uint16        us_medium_time;
}mac_cfg_wmm_tspec_stru_param_stru;

typedef struct
{
    oal_switch_enum_uint8                  en_wmm_ac_switch;
    oal_switch_enum_uint8                  en_auth_flag;/*WMM AC��֤���� */
    oal_uint16                             us_timeout_period;/*��ʱ����ʱʱ��*/
    oal_uint8                              uc_factor;/*�������ӣ�medium_time/2^factor */
    oal_uint8                              auc_rsv[3];

}mac_cfg_wmm_ac_param_stru;

#endif

typedef struct
{
    oal_uint8                           auc_mac_addr[6];
    oal_uint8                           uc_amsdu_max_num;   /* amsdu������ */
    oal_uint8                           auc_reserve[3];
    oal_uint16                          us_amsdu_max_size;  /* amsdu��󳤶� */

}mac_cfg_amsdu_start_param_stru;

/* ���͹������ò��� */
typedef struct
{
    mac_set_pow_type_enum_uint8             en_type;
    oal_uint8                               uc_reserve;
    oal_uint8                               auc_value[18];
}mac_cfg_set_tx_pow_param_stru;

/* �����û����ò��� */
typedef struct
{
    oal_uint8                               uc_function_index;
    oal_uint8                               auc_reserve[2];
    mac_vap_config_dscr_frame_type_uint8    en_type;       /* ���õ�֡���� */
    oal_int32                               l_value;
}mac_cfg_set_dscr_param_stru;

typedef struct
{
    mac_vap_stru                *pst_mac_vap;
    oal_int8                     pc_param[4];      /* ��ѯ��������Ϣ */
}mac_cfg_event_stru;

/* non-HTЭ��ģʽ���������ýṹ�� */
typedef struct
{
    wlan_legacy_rate_value_enum_uint8       en_rate;            /* ����ֵ */
    wlan_phy_protocol_enum_uint8            en_protocol_mode;   /* ��Ӧ��Э�� */
    oal_uint8                               auc_reserve[2];     /* ���� */
}mac_cfg_non_ht_rate_stru;

/* ���÷����������ڲ�Ԫ�ؽṹ�� */
typedef enum
{
    RF_PAYLOAD_ALL_ZERO = 0,
    RF_PAYLOAD_ALL_ONE,
    RF_PAYLOAD_RAND,
    RF_PAYLOAD_BUTT
}mac_rf_payload_enum;
typedef oal_uint8 mac_rf_payload_enum_uint8;

typedef struct
{
    oal_uint8                        uc_param;      /* ��ѯ��������Ϣ */
    wlan_phy_protocol_enum_uint8     en_protocol_mode;
    mac_rf_payload_enum_uint8        en_payload_flag;
    wlan_tx_ack_policy_enum_uint8    en_ack_policy;
    oal_uint32                       ul_payload_len;
}mac_cfg_tx_comp_stru;

#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)
typedef struct
{
    oal_uint8                        uc_ch;
    oal_uint8                        uc_freq;
    oal_int16                        as_power[WLAN_DYNC_CALI_POW_PD_PARAM_NUM];
}mac_cfg_cali_power_stru;

typedef struct
{
    oal_uint8                        uc_freq;
    oal_uint8                        auc_reserve[3];     /* ���� */
    oal_int16                        as_polynomial_para[2][WLAN_DYNC_CALI_POW_PD_PARAM_NUM];
}mac_cfg_polynomial_para_stru;

typedef struct
{
    oal_int32                        l_length;
    oal_int16                        as_polynomial_paras[WLAN_DYNC_CALI_5G_PD_PARAM_NUMS];
}mac_cfg_show_pd_paras_stru;

typedef struct
{
    oal_uint32                        ul_length;
    oal_uint16                        aus_upc_paras[WLAN_DYNC_CALI_5G_UPC_PARAM_NUMS];
}mac_cfg_show_upc_paras_stru;

#endif

typedef struct
{
    oal_uint32                        ul_length;
    oal_uint32                        aul_dieid[WLAN_DIEID_MAX_LEN];
}mac_cfg_show_dieid_stru;

#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
typedef struct
{
    oal_uint8                           uc_action_type;
    oal_uint8                           auc_mac_addr[WLAN_MAC_ADDR_LEN];
    oal_uint16                          us_num_rpt;
    oal_uint16                          us_random_ivl;
    oal_uint16                          us_duration;
    oal_uint8                           uc_optclass;
    oal_uint8                           uc_req_mode;
    oal_uint8                           uc_means_type;
    oal_uint8                           uc_channum;
    oal_uint8                           uc_bcn_mode;
    oal_uint8                           auc_bssid[WLAN_MAC_ADDR_LEN];
}mac_cfg_radio_meas_info_stru;
#endif

/* ����Ӳ������ */
typedef struct
{
    oal_uint32                        ul_devid;
    oal_uint32                        ul_mode;
    oal_uint32                        ul_rate;
}mac_cfg_al_tx_hw_cfg_stru;

typedef enum
{
    HW_PAYLOAD_ALL_ZERO = 0,
    HW_PAYLOAD_ALL_ONE,
    HW_PAYLOAD_RAND,
    HW_PAYLOAD_REPEAT,
    HW_PAYLOAD_BUTT
}mac_hw_payload_enum;
typedef oal_uint8 mac_hw_payload_enum_uint8;


/* Ӳ���������� */
typedef struct
{
    oal_uint8                        bit_switch : 1,
                                     bit_dev_id : 1,
                                     bit_flag   : 2, /* 0/1/2/3 ����֡�����ݡ�0:ȫ0��1:ȫ1��2:����� 3:�ظ�uc_content�����ݡ� */
                                     bit_rsv    : 4;
    oal_uint8                        uc_content;
    oal_uint8                        auc_rsv[2];
    oal_uint32                       ul_len;
    oal_uint32                       ul_times; /* ���� 0Ϊ���޴� */
    oal_uint32                       ul_ifs;   /* ����֡���,��λ0.1us */
}mac_cfg_al_tx_hw_stru;





typedef struct
{
    oal_uint8                   uc_offset_addr_a;
    oal_uint8                   uc_offset_addr_b;
    oal_uint16                  us_delta_gain;
}mac_cfg_dbb_scaling_stru;


/* Ƶƫ���������ʽ */
typedef struct
{
    oal_uint16                  us_idx;             /* ȫ����������ֵ */
    oal_uint16                  us_chn;             /* �����ŵ� */
    oal_int16                   as_corr_data[8];     /* У������ */
}mac_cfg_freq_skew_stru;

/* wfa edca�������� */
typedef struct
{
    oal_bool_enum_uint8             en_switch;      /* ���� */
    wlan_wme_ac_type_enum_uint8     en_ac;          /* AC */
    oal_uint16                      us_val;         /* ���� */
}mac_edca_cfg_stru;

/* PPM���������ʽ */
typedef struct
{
    oal_int8                     c_ppm_val;         /* PPM��ֵ */
    oal_uint8                    uc_clock_freq;     /* ʱ��Ƶ�� */
    oal_uint8                    uc_resv[1];
}mac_cfg_adjust_ppm_stru;

typedef struct
{
    oal_uint8                    uc_pcie_pm_level;  /* pcie�͹��ļ���,0->normal;1->L0S;2->L1;3->L1PM;4->L1s */
    oal_uint8                    uc_resv[3];
}mac_cfg_pcie_pm_level_stru;

/* �û���Ϣ��ص������������ */
typedef struct
{
    oal_uint16              us_user_idx;                        /* �û����� */
    oal_uint8               auc_reserve[2];

}mac_cfg_user_info_param_stru;

/* ����������͹������� */
typedef struct
{
    oal_uint8 uc_pwr;
    oal_uint8 en_exceed_reg;
    oal_uint8 auc_resv[2];
}mac_cfg_regdomain_max_pwr_stru;


/* ��ȡ��ǰ������������ַ���������ṹ�� */
typedef struct
{
    oal_int8    ac_country[3];
    oal_uint8   auc_resv[1];
}mac_cfg_get_country_stru;

/* query��Ϣ��ʽ:2�ֽ�WID x N */
typedef struct
{
    wlan_tidno_enum_uint8  en_tid;
    oal_uint8              uc_resv[3];
}mac_cfg_get_tid_stru;

typedef struct
{
    oal_uint16          us_user_id;
    oal_int8            c_rssi;
    oal_int8            c_free_power;
}mac_cfg_query_rssi_stru;

typedef struct
{
    oal_uint8           uc_auth_req_st;
    oal_uint8           uc_asoc_req_st;
    oal_uint8           auc_resv[2];
}mac_cfg_query_mngpkt_sendstat_stru;

typedef struct
{
    oal_uint16          us_user_id;
    oal_uint8           uc_ps_st;
    oal_uint8           auc_resv[1];
}mac_cfg_query_psst_stru;

typedef struct
{
    oal_uint16          us_user_id;
    oal_uint8           auc_resv[2];
    oal_uint32          aul_tx_dropped[WLAN_WME_AC_BUTT];
}mac_cfg_query_drop_num_stru;

typedef struct
{
    oal_uint16          us_user_id;
    oal_uint8           auc_resv[2];
    oal_uint32          ul_max_tx_delay;                    /*�������ʱ*/
    oal_uint32          ul_min_tx_delay;                    /*��С������ʱ*/
    oal_uint32          ul_ave_tx_delay;                    /*ƽ��������ʱ*/
}mac_cfg_query_tx_delay_stru;

typedef struct
{
    oal_uint8           uc_vap_id;
    oal_uint8           uc_bsd;
    oal_uint8           auc_resv[2];
}mac_cfg_query_bsd_stru;

#ifdef _PRE_WLAN_DFT_STAT
typedef struct
{
    oal_uint8           uc_device_distance;
    oal_uint8           uc_intf_state_cca;
    oal_uint8           uc_intf_state_co;
    oal_uint8           auc_resv[1];
}mac_cfg_query_ani_stru;
#endif

typedef struct
{
    oal_uint16          us_user_id;
#ifdef _PRE_WLAN_DFT_STAT
    oal_uint8           uc_cur_per;
    oal_uint8           uc_bestrate_per;
#else
    oal_uint8           auc_resv[2];
#endif
    oal_uint32          ul_tx_rate;         /* ��ǰ�������� */
    oal_uint32          ul_tx_rate_min;     /* һ��ʱ������С�������� */
    oal_uint32          ul_tx_rate_max;     /* һ��ʱ������������� */
    oal_uint32          ul_rx_rate;         /* ��ǰ�������� */
    oal_uint32          ul_rx_rate_min;     /* һ��ʱ������С�������� */
    oal_uint32          ul_rx_rate_max;     /* һ��ʱ�������������� */
}mac_cfg_query_rate_stru;

#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)
/* ����У׼���� */
typedef struct
{
    oal_uint8               uc_chain_idx;        /* ͨ�� */
    oal_uint8               uc_band_idx;         /* subband */
    oal_uint16              us_vref_value;       /* vrefֵ */
}mac_cfg_set_cali_vref_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
typedef struct
{
    oal_uint32  ul_cfg_id;
    oal_uint32  ul_ac;
    oal_uint32  ul_value;
}mac_cfg_set_smps_mode_stru;
#endif

/* ����Ϊ�����ں����ò���ת��Ϊ�����ڲ������·��Ľṹ��*/

/* �����ں����õ�ɨ��������·���������ɨ����� */
typedef struct
{
    oal_ssids_stru              st_ssids[WLAN_SCAN_REQ_MAX_SSID];
    oal_int32                   l_ssid_num;

    oal_uint8                  *puc_ie;
    oal_uint32                  ul_ie_len;

    oal_scan_enum_uint8         en_scan_type;
    oal_uint8                   uc_num_channels_2G;
    oal_uint8                   uc_num_channels_5G;
    oal_uint8                   auc_arry[1];

    oal_uint32                  *pul_channels_2G;
    oal_uint32                  *pul_channels_5G;
    oal_uint8                   uc_scan_flag;

    /* WLAN/P2P ��������£�p2p0 ��p2p-p2p0 cl ɨ��ʱ����Ҫʹ�ò�ͬ�豸������bit_is_p2p0_scan������ */
    oal_uint8                    bit_is_p2p0_scan : 1;   /* �Ƿ�Ϊp2p0 ����ɨ�� */
    oal_uint8                    bit_rsv          : 7;   /* ����λ */
    oal_uint8                    auc_rsv[2];             /* ����λ */
}mac_cfg80211_scan_param_stru;


typedef struct
{
    mac_cfg80211_scan_param_stru  *pst_mac_cfg80211_scan_param;
}mac_cfg80211_scan_param_pst_stru;

/* �����ں����õ�connect�������·���������connect���� */
typedef struct
{
    oal_uint32              ul_wpa_versions;
    oal_uint32              ul_group_suite;
    oal_uint32              aul_pair_suite[WLAN_PAIRWISE_CIPHER_SUITES];
    oal_uint32              aul_akm_suite[WLAN_AUTHENTICATION_SUITES];
    oal_uint32              ul_group_mgmt_suite;
#ifdef _PRE_WLAN_WEB_CMD_COMM
    oal_int                 n_pair_suite;
    oal_int                 n_akm_suites;
#endif
}mac_crypto_settings_stru;

/**
 * enum nl80211_mfp - Management frame protection state
 * @NL80211_MFP_NO: Management frame protection not used
 * @NL80211_MFP_REQUIRED: Management frame protection required
 */

typedef enum
{
    MAC_NL80211_MFP_NO,
    MAC_NL80211_MFP_REQUIRED,

    MAC_NL80211_MFP_BUTT
}mac_nl80211_mfp_enum;
typedef oal_uint8 mac_nl80211_mfp_enum_uint8;



typedef struct
{
    oal_uint8                           uc_channel;              /* ap�����ŵ���ţ�eg 1,2,11,36,40... */
    oal_uint8                           uc_ssid_len;            /* SSID ���� */
    mac_nl80211_mfp_enum_uint8          en_mfp;
    oal_uint8                           uc_wapi;

    oal_uint8                          *puc_ie;
    oal_uint8                           auc_ssid[32];               /* ����������AP SSID  */
    oal_uint8                           auc_bssid[OAL_MAC_ADDR_LEN];/* ����������AP BSSID  */

    oal_bool_enum_uint8                 en_privacy;             /* �Ƿ���ܱ�־ */
    oal_nl80211_auth_type_enum_uint8    en_auth_type;           /* ��֤���ͣ�OPEN or SHARE-KEY */

    oal_uint8                           uc_wep_key_len;         /* WEP KEY���� */
    oal_uint8                           uc_wep_key_index;       /* WEP KEY���� */
    oal_uint8                          *puc_wep_key;            /* WEP KEY��Կ */
    mac_crypto_settings_stru            st_crypto;              /* ��Կ�׼���Ϣ */
    oal_uint32                          ul_ie_len;
}mac_conn_param_stru;

typedef struct
{
    oal_bool_enum_uint8                 en_privacy;             /* �Ƿ���ܱ�־ */
    oal_nl80211_auth_type_enum_uint8    en_auth_type;           /* ��֤���ͣ�OPEN or SHARE-KEY */
    oal_uint8                           uc_wep_key_len;         /* WEP KEY���� */
    oal_uint8                           uc_wep_key_index;       /* WEP KEY���� */
    oal_uint8                           auc_wep_key[WLAN_WEP104_KEY_LEN];            /* WEP KEY��Կ */
    mac_nl80211_mfp_enum_uint8          en_mgmt_proteced;       /* ��������pmf�Ƿ�ʹ�� */
    wlan_pmf_cap_status_uint8           en_pmf_cap;             /* �豸pmf���� */
    oal_bool_enum_uint8                 en_wps_enable;
    mac_crypto_settings_stru            st_crypto;              /* ��Կ�׼���Ϣ */
#ifdef _PRE_WLAN_FEATURE_11R
    oal_uint8                           auc_mde[8];             /* MD IE��Ϣ */
#endif //_PRE_WLAN_FEATURE_11R
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_int8                            c_rssi;                 /* ����AP��RSSI��Ϣ */
    oal_int8                            c_ant0_rssi;            /* ����0��rssi */
    oal_int8                            c_ant1_rssi;            /* ����1��rssi */
    oal_uint8                           auc_rsv[1];
#endif /* _PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE */
}mac_conn_security_stru;


#ifdef _PRE_WLAN_FEATURE_11R
#define MAC_MAX_FTE_LEN              257

typedef struct
{
    oal_uint16            us_mdid;                   /* Mobile Domain ID */
    oal_uint16            us_len;                    /* FTE �ĳ��� */
    oal_uint8             auc_ie[MAC_MAX_FTE_LEN];
}mac_cfg80211_ft_ies_stru;

typedef struct
{
    oal_cfg80211_bss_stru            *pst_bss;
    const oal_uint8                  *puc_ie;
    oal_uint8                         uc_ie_len;
    oal_nl80211_auth_type_enum_uint8  en_auth_type;
    const oal_uint8                  *puc_key;
    oal_uint8                         uc_key_len;
    oal_uint8                         uc_key_idx;
}mac_cfg80211_auth_req_stru;

#endif //_PRE_WLAN_FEATURE_11R

typedef struct
{

    oal_uint8       auc_mac_addr[OAL_MAC_ADDR_LEN];
    oal_uint8       auc_rsv[2];
}mac_cfg80211_init_port_stru;

/* �����ں����õ�disconnect�������·���������disconnect���� */
typedef struct mac_cfg80211_disconnect_param_tag
{
    oal_uint16              us_reason_code; /* disconnect reason code */
    oal_uint16              us_aid;

    oal_uint8               uc_type;
    oal_uint8               auc_arry[3];
}mac_cfg80211_disconnect_param_stru;

/* ��mac_cfg_lpm_wow_en_stru����Ų��_PRE_WLAN_CHIP_TEST���棬ʹ�ùر�_PRE_WLAN_CHIP_TESTҲ������������ */
typedef struct mac_cfg_lpm_wow_en
{
    oal_uint8                       uc_en;              /*0:off, 1:on*/
    oal_uint8                       uc_null_wake;       /*1:null data wakeup*/
}mac_cfg_lpm_wow_en_stru;

typedef enum
{
   MAC_LPM_SOC_BUS_GATING       = 0,
   MAC_LPM_SOC_PCIE_RD_BYPASS   = 1,
   MAC_LPM_SOC_MEM_PRECHARGE    = 2,
   MAC_LPM_SOC_PCIE_L0_S        = 3,
   MAC_LPM_SOC_PCIE_L1_0        = 4,
   MAC_LPM_SOC_AUTOCG_ALL       = 5,
   MAC_LPM_SOC_ADC_FREQ         = 6,
   MAC_LPM_SOC_PCIE_L1S         = 7,

   MAC_LPM_SOC_SET_BUTT
}mac_lpm_soc_set_enum;
typedef oal_uint8   mac_lpm_soc_set_enum_uint8;

typedef struct mac_cfg_lpm_soc_set_tag
{
    mac_lpm_soc_set_enum_uint8      en_mode;
    oal_uint8                       uc_on_off;
    oal_uint8                       uc_pcie_idle;
    oal_uint8                       auc_rsv[1];
}mac_cfg_lpm_soc_set_stru;

#ifdef _PRE_WLAN_CHIP_TEST
typedef enum
{
   MAC_LPM_STATE_SOFT_SLEEP  = 0,
   MAC_LPM_STATE_GPIO_SLEEP  = 1,
   MAC_LPM_STATE_WORK        = 2,

   MAC_LPM_STATE_BUTT
}mac_lpm_state_enum;
typedef oal_uint8   mac_lpm_state_enum_uint8;

typedef struct mac_cfg_lpm_sleep_para_tag
{
    mac_lpm_state_enum_uint8         uc_pm_switch;  /*˯�߻��ѿ���*/
    oal_uint8                        uc_rsv;
    oal_uint16                       us_sleep_ms;   /*˯��ʱ�䣬����*/

}mac_cfg_lpm_sleep_para_stru;

typedef struct mac_cfg_lpm_psm_param_tag
{
    oal_uint8   uc_psm_on;
    oal_uint8   uc_psm_wakeup_mode;
    oal_uint8   auc_rsv[2];
    oal_uint16  us_psm_listen_interval;
    oal_uint16  us_psm_tbtt_offset;

}mac_cfg_lpm_psm_param_stru;

typedef struct mac_cfg_lpm_smps_stub
{
    oal_uint8                       uc_stub_type;   /*0:off, 1:������2:˫��*/
    oal_uint8                       uc_rts_en;    /*�Ƿ�RTS*/
}mac_cfg_lpm_smps_stub_stru;


typedef struct mac_cfg_lpm_txopps_set_tag
{
    oal_uint8                       uc_txop_ps_on;
    oal_uint8                       uc_conditon1;
    oal_uint8                       uc_conditon2;
    oal_uint8                       auc_rsv[1];
}mac_cfg_lpm_txopps_set_stru;

typedef struct mac_cfg_lpm_txopps_tx_stub
{
    oal_uint8                       uc_stub_on;   /*0:off, 1:on*/
    oal_uint8                       auc_rsv[1];
    oal_uint16                      us_begin_num;  /* ��n�����Ĵ�׮ */
}mac_cfg_lpm_txopps_tx_stub_stru;

typedef struct mac_cfg_lpm_tx_data_tag
{
    oal_uint16  us_num;
    oal_uint16  us_len;
    oal_uint8   uc_ac;
    oal_uint8   uc_positive;                          /*0:����probe request��1:���� */
    oal_uint8   uc_psm;                               /*0:psm=0,1:psm=1*/
    oal_uint8   uc_rsv[1];
    oal_uint8   auc_da[WLAN_MAC_ADDR_LEN];            /*����probe requestʱ������Ϊָ��BSSID*/
    //oal_uint8   auc_sa[WLAN_MAC_ADDR_LEN];
}mac_cfg_lpm_tx_data_stru;

typedef struct
{
    oal_uint16  us_user_idx;
    oal_uint8   uc_is_remove;
    oal_uint8   auc_resv[1];
}mac_cfg_remove_lut_stru;

typedef struct
{
    oal_uint32                               ul_mac_ctrl;
    oal_uint32                               ul_rf_ctrl;
}mac_cfg_coex_ctrl_param_stru;

#endif

#ifdef _PRE_WLAN_FEATURE_GREEN_AP
typedef enum
{
   MAC_GREEN_AP_MODE_INACTIVE          = 0,
   MAC_GREEN_AP_MODE_ACTIVE            = 1,

   MAC_GREEN_AP_MODE_BUTT
}mac_green_ap_mode_enum;
typedef oal_uint8   mac_green_ap_mode_enum_uint8;

typedef struct mac_cfg_free_ratio_set_tag
{
    mac_green_ap_mode_enum_uint8      en_mode;
    oal_uint8                         uc_th_value;
    oal_uint8                         auc_rsv[2];
}mac_cfg_free_ratio_set_stru;
#endif

typedef struct
{
    oal_uint8                               auc_mac_ra[WLAN_MAC_ADDR_LEN];
    oal_uint8                               uc_pkt_num;
    mac_test_frame_type_enum_uint8          en_frame_type;
    oal_uint8                               uc_frame_body[64];
    oal_uint8                               uc_vap_idx;
    oal_uint8                               uc_frame_cnt;
    oal_uint8                               uc_frame_body_length;
    oal_uint8                               uc_resv[1];
}mac_cfg_send_frame_param_stru;

typedef struct
{
    oal_uint8                               auc_mac_da[WLAN_MAC_ADDR_LEN];
    oal_uint8                               uc_category;
    oal_uint8                               auc_resv[1];
}mac_cfg_send_action_param_stru;

#ifdef _PRE_WLAN_FEATURE_SMPS
typedef struct
{
    wlan_mib_mimo_power_save_enum_uint8    en_smps_mode;
    oal_uint8    uc_trigger_smps_frame;
    oal_uint16   us_user_idx;
}mac_cfg_smps_mode_stru;
#endif

typedef struct
{
    oal_int32   l_is_psm;                           /* �Ƿ������� */
    oal_int32   l_is_qos;                           /* �Ƿ�qosnull */
    oal_int32   l_tidno;                            /* tid�� */
}mac_cfg_tx_nulldata_stru;

/* ������̫��������Ҫ�Ĳ��� */
typedef struct
{
    oam_ota_frame_direction_type_enum_uint8 en_frame_direction;
    oal_switch_enum_uint8   en_switch;
    oal_uint8               auc_user_macaddr[WLAN_MAC_ADDR_LEN];
}mac_cfg_eth_switch_param_stru;

/* ����80211����֡������Ҫ�Ĳ��� */
typedef struct
{
    oam_ota_frame_direction_type_enum_uint8 en_frame_direction;
    oam_user_track_frame_type_enum_uint8    en_frame_type;
    oal_switch_enum_uint8                   en_frame_switch;
    oal_switch_enum_uint8                   en_cb_switch;
    oal_switch_enum_uint8                   en_dscr_switch;
    oal_uint8                               auc_resv[1];
    oal_uint8                               auc_user_macaddr[WLAN_MAC_ADDR_LEN];
}mac_cfg_80211_ucast_switch_stru;
#ifdef _PRE_DEBUG_MODE_USER_TRACK
/* ��ȡ�û��շ�����������Ҫ�Ĳ����ṹ */
typedef struct
{
    oal_uint8                               auc_user_macaddr[WLAN_MAC_ADDR_LEN];
    oam_user_txrx_param_type_enum_uint8     en_type;
    oal_switch_enum_uint8                   en_switch;
    oal_uint32                              ul_period;
}mac_cfg_report_txrx_param_stru;
#endif
#ifdef _PRE_WLAN_FEATURE_TXOPPS
/* �������mac txoppsʹ�ܼĴ�����Ҫ���������� */
typedef struct
{
    oal_switch_enum_uint8 en_machw_txopps_en;               /* sta�Ƿ�ʹ��txopps */
    oal_switch_enum_uint8 en_machw_txopps_condition1;       /* txopps����1 */
    oal_switch_enum_uint8 en_machw_txopps_condition2;       /* txopps����2 */
    oal_uint8             auc_resv[1];
}mac_txopps_machw_param_stru;
#endif
/* ����80211�鲥\�㲥֡������Ҫ�Ĳ��� */
typedef struct
{
    oam_ota_frame_direction_type_enum_uint8 en_frame_direction;
    oam_user_track_frame_type_enum_uint8    en_frame_type;
    oal_switch_enum_uint8                   en_frame_switch;
    oal_switch_enum_uint8                   en_cb_switch;
    oal_switch_enum_uint8                   en_dscr_switch;
    oal_uint8                               auc_resv[3];
}mac_cfg_80211_mcast_switch_stru;

/* ����probe request��probe response������Ҫ�Ĳ��� */
typedef struct
{
    oam_ota_frame_direction_type_enum_uint8 en_frame_direction;
    oal_switch_enum_uint8                   en_frame_switch;
    oal_switch_enum_uint8                   en_cb_switch;
    oal_switch_enum_uint8                   en_dscr_switch;
}mac_cfg_probe_switch_stru;

/* ��ȡmpdu��Ŀ��Ҫ�Ĳ��� */
typedef struct
{
    oal_uint8                   auc_user_macaddr[WLAN_MAC_ADDR_LEN];
    oal_uint8                   auc_resv[2];
}mac_cfg_get_mpdu_num_stru;

#ifdef _PRE_WLAN_DFT_STAT
typedef struct
{
    oal_uint8                   auc_user_macaddr[WLAN_MAC_ADDR_LEN];
    oal_uint8                   uc_param;
    oal_uint8                   uc_resv;
}mac_cfg_usr_queue_param_stru;
#endif

#ifdef _PRE_DEBUG_MODE
typedef struct
{
    oal_uint8                   auc_user_macaddr[WLAN_MAC_ADDR_LEN];
    oal_uint8                   uc_param;
    oal_uint8                   uc_tid_no;
}mac_cfg_ampdu_stat_stru;
#endif

typedef struct
{
    oal_uint8                   uc_aggr_num_switch; /* ���ƾۺϸ������� */
    oal_uint8                   uc_aggr_num;        /* �ۺϸ��� */
    oal_uint8                   auc_resv[2];
}mac_cfg_aggr_num_stru;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
typedef struct
{
    oal_uint8                   uc_mmss_val;        /* ��С֡��� */
    oal_uint8                   auc_resv[3];
}mac_cfg_ampdu_mmss_stru;
#endif

#ifdef _PRE_DEBUG_MODE
typedef struct
{
    oal_uint16                  us_pll_int;         /* ������Ƶϵ�� */
    oal_uint16                  us_pll_frac;        /* С����Ƶϵ�� */
}mac_cfg_freq_adjust_stru;
#endif

typedef struct
{
    oal_uint16              us_duration_ms;
    oal_bool_enum_uint8     en_probe;
    oal_uint8               uc_chan_cnt;
    oal_uint8               auc_channels[WLAN_MAX_CHANNEL_NUM];
}mac_chan_stat_param_stru;

#ifdef _PRE_DEBUG_MODE_USER_TRACK
typedef struct
{
    oal_uint8                   auc_user_macaddr[WLAN_MAC_ADDR_LEN];
    oal_uint8                   uc_param;
    oal_uint8                   uc_resv;
}mac_cfg_usr_thrput_stru;

#endif

typedef struct
{
    oal_uint32  ul_coext_info;
    oal_uint32  ul_channel_report;
}mac_cfg_set_2040_coexist_stru;

typedef struct
{
    oal_uint32   ul_mib_idx;
    oal_uint32   ul_mib_value;
}mac_cfg_set_mib_stru;

typedef struct
{
    oal_uint8  uc_bypass_type;
    oal_uint8  uc_value;
    oal_uint8  auc_resv[2];
}mac_cfg_set_thruput_bypass_stru;

typedef struct
{
    oal_uint8   uc_cmd_type;
    oal_uint8   uc_len;
    oal_uint16  us_cfg_id;
    oal_uint32  ul_value;
}mac_cfg_set_tlv_stru;

typedef struct
{
    oal_uint8  uc_performance_log_switch_type;
    oal_uint8  uc_value;
    oal_uint8  auc_resv[2];
}mac_cfg_set_performance_log_switch_stru;

typedef struct
{
    oal_uint8            uc_scan_type;
    oal_bool_enum_uint8  en_current_bss_ignore;
    oal_uint8            auc_bssid[OAL_MAC_ADDR_LEN];
}mac_cfg_set_roam_start_stru;

typedef struct
{
    oal_uint32   ul_timeout;
    oal_uint8    uc_is_period;
    oal_uint8    uc_stop_start;
    oal_uint8    auc_resv[2];
}mac_cfg_test_timer_stru;

typedef struct
{
    oal_uint16   us_user_idx;
    oal_uint16   us_rx_pn;
}mac_cfg_set_rx_pn_stru;

typedef struct
{
    oal_uint32   ul_frag_threshold;
}mac_cfg_frag_threshold_stru;

typedef struct
{
    oal_uint32   ul_rts_threshold;
}mac_cfg_rts_threshold_stru;

typedef struct
{
    /* software_retryֵ */
    oal_uint8   uc_software_retry;
    /* �Ƿ�ȡtest���õ�ֵ��Ϊ0��Ϊ������������ */
    oal_uint8   uc_retry_test;

    oal_uint8   uc_pad[2];
}mac_cfg_set_soft_retry_stru;


typedef struct
{
    oal_bool_enum_uint8 en_default_key;
    oal_uint8           uc_key_index;
    oal_uint8           uc_key_len;
    oal_uint8           auc_wep_key[WLAN_WEP104_KEY_LEN];
} mac_wep_key_param_stru;



typedef struct mac_pmksa_tag
{
    oal_uint8 auc_bssid[OAL_MAC_ADDR_LEN];
    oal_uint8 auc_pmkid[OAL_PMKID_LEN];

}mac_pmksa_stru;

typedef struct
{
    oal_uint8                uc_key_index;
    oal_bool_enum_uint8      en_pairwise;
    oal_uint8                auc_mac_addr[OAL_MAC_ADDR_LEN];
    mac_key_params_stru      st_key;
}mac_addkey_param_stru;


typedef struct
{
    oal_int32  key_len;
    oal_uint8 auc_key[OAL_WPA_KEY_LEN];
}mac_key_stru;

typedef struct
{
    oal_int32  seq_len;
    oal_uint8 auc_seq[OAL_WPA_SEQ_LEN];
}mac_seq_stru;


typedef struct
{
    //oal_net_device_stru     *pst_netdev;
    oal_uint8                  uc_key_index;
    oal_bool_enum_uint8        en_pairwise;
    //oal_uint8                auc_resv1[2];
    oal_uint8                  auc_mac_addr[OAL_MAC_ADDR_LEN];
    //oal_uint8                  uc_usridx;       /*  */
    oal_uint8                  cipher;
    //oal_uint8                  uc_type;        /* 1-key 2-seq */
    oal_uint8                  auc_rsv[3];
    mac_key_stru               st_key;
    mac_seq_stru               st_seq;
}mac_addkey_hmac2dmac_param_stru;

typedef struct
{
    oal_net_device_stru     *pst_netdev;
    oal_uint8                uc_key_index;
    oal_bool_enum_uint8      en_pairwise;
    oal_uint8                auc_resv1[2];
    oal_uint8               *puc_mac_addr;
    oal_void                *cookie;
    oal_void               (*callback)(oal_void *cookie, oal_key_params_stru *key_param);
}mac_getkey_param_stru;

typedef struct
{
    //oal_net_device_stru     *pst_netdev;
    oal_uint8                uc_key_index;
    oal_bool_enum_uint8      en_pairwise;
    //oal_uint8                auc_resv1[2];
    oal_uint8                auc_mac_addr[OAL_MAC_ADDR_LEN];
}mac_removekey_param_stru;

typedef struct
{
    //oal_net_device_stru     *pst_netdev;
    oal_uint8                uc_key_index;
    oal_bool_enum_uint8      en_unicast;
    oal_bool_enum_uint8      en_multicast;
    oal_uint8                auc_resv1[1];
}mac_setdefaultkey_param_stru;

typedef struct
{
    oal_uint32  ul_dfs_mode;
    oal_int32   al_para[3];
}mac_cfg_dfs_param_stru;

typedef struct
{
    oal_uint8  uc_total_channel_num;
    oal_uint8  auc_channel_number[15];
}mac_cfg_dfs_auth_channel_stru;


typedef struct
{
    oal_uint    ul_addr;
    oal_uint32  ul_len;
}mac_cfg_dump_memory_stru;

typedef enum
{
    MAC_MEMINFO_USER = 0,
    MAC_MEMINFO_VAP,
    MAC_MEMINFO_POOL_INFO,
    MAC_MEMINFO_POOL_DBG,

    MAC_MEMINFO_DSCR,
    MAC_MEMINFO_NETBUFF,
    MAC_MEMINFO_SDIO_TRX,
    MAC_MEMINFO_SAMPLE_ALLOC,
    MAC_MEMINFO_SAMPLE_FREE,
    MAC_MEMINFO_ALL,

    MAC_MEMINFO_BUTT
}mac_meminfo_cmd_enum;
typedef oal_uint8 mac_meminfo_cmd_enum_uint8;

typedef struct
{
    mac_meminfo_cmd_enum_uint8  uc_meminfo_type;
    oal_uint8                   uc_object_index;
}mac_cfg_meminfo_stru;

#ifdef _PRE_WLAN_PERFORM_STAT
/* ͳ�������� */
typedef enum
{
    MAC_STAT_TYPE_TID_DELAY,
    MAC_STAT_TYPE_TID_PER,
    MAC_STAT_TYPE_TID_THRPT,
    MAC_STAT_TYPE_USER_THRPT,
    MAC_STAT_TYPE_VAP_THRPT,
    MAC_STAT_TYPE_USER_BSD,

    MAC_STAT_TYPE_BUTT
}mac_stat_type_enum;
typedef oal_uint8    mac_stat_type_enum_uint8;

typedef enum
{
    MAC_STAT_MODULE_CMD,
    MAC_STAT_MODULE_AUTORATE,
    MAC_STAT_MODULE_BSD,
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
    MAC_STAT_MODULE_QOS,
#endif
    MAC_STAT_MODULE_BUTT
}mac_stat_module_enum;
typedef oal_uint16    mac_stat_module_enum_uint16;

typedef struct
{
    mac_stat_type_enum_uint8    en_stat_type;                       /* ͳ������ */
    oal_uint8                   uc_vap_id;                          /* vap id */
    oal_uint8                   auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* MAC��ַ */
    oal_uint8                   uc_tidno;                           /* tidno */
    oal_uint16                  us_stat_period;                     /* ͳ������ */
    oal_uint16                  us_stat_num;                        /* ͳ�ƴ��� */
}mac_cfg_stat_param_stru;

typedef struct
{
    mac_stat_type_enum_uint8    en_stat_type;                       /* ͳ������ */
    oal_uint8                   uc_vap_id;                          /* vap id */
    oal_uint8                   auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* MAC��ַ */
    oal_uint8                   uc_tidno;                           /* tidno */
}mac_cfg_display_param_stru;

/* ͳ��tid */
typedef struct
{
    oal_uint8               uc_vap_id;                          /* vap id */
    oal_uint8               auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* MAC��ַ */
    oal_uint8               uc_tidno;                           /* tidno */
    oal_uint16              us_stat_period;                     /* ͳ������ */
    oal_uint32              ul_stat_num;                        /* ͳ�ƴ��� */
}mac_ioctl_stat_param_stru;

#endif


#ifdef _PRE_WLAN_FEATURE_FLOWCTL

typedef enum
{
    MAC_LINUX_SUBQ_BE = 0,    /* best effort */
    MAC_LINUX_SUBQ_BK = 1,    /* background */
    MAC_LINUX_SUBQ_VI = 2,    /* video */
    MAC_LINUX_SUBQ_VO = 3,    /* voice */

    MAC_LINUX_SUBQ_BUTT
}MAC_linux_subq_type_enum;
typedef oal_uint8 mac_linux_subq_type_enum;

#define MAC_TOS_TO_SUBQ(_tos) (      \
        (((_tos) == 0) || ((_tos) == 3)) ? MAC_LINUX_SUBQ_BE : \
        (((_tos) == 1) || ((_tos) == 2)) ? MAC_LINUX_SUBQ_BK : \
        (((_tos) == 4) || ((_tos) == 5)) ? MAC_LINUX_SUBQ_VI : \
        MAC_LINUX_SUBQ_VO)

typedef struct
{
    oal_uint16                      us_assoc_id;
    oal_uint8                       uc_vap_id;
    oal_uint8                       uc_tidno;
    oal_uint8                       uc_is_stop;
    oal_uint8                       uc_resv[3];
}mac_ioctl_queue_backp_stru;

#endif

/* ��̬У׼����ö�٣�����ֵ */
typedef struct
{
    mac_dyn_cali_cfg_enum_uint8         en_dyn_cali_cfg;     /* ��������ö�� */
    oal_uint8                           uc_resv;          /* �ֽڶ��� */
    oal_uint16                          us_value;            /* ���ò���ֵ */
}mac_ioctl_dyn_cali_param_stru;

/* �㷨����ö�٣�����ֵ */
typedef struct
{
    mac_alg_cfg_enum_uint8              en_alg_cfg;     /* ��������ö�� */
    oal_uint8                           uc_resv[3];     /* �ֽڶ��� */
    oal_uint32                          ul_value;       /* ���ò���ֵ */
}mac_ioctl_alg_param_stru;

/* AUTORATE LOG �㷨����ö�٣�����ֵ */
typedef struct
{
    mac_alg_cfg_enum_uint8  en_alg_cfg;     /* ��������ö�� */
    oal_uint8               auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* MAC��ַ */
    oal_uint8               uc_ac_no;                           /* AC���� */
    oal_uint8               auc_resv[2];
    oal_uint16              us_value;                           /*���ò���ֵ */
}mac_ioctl_alg_ar_log_param_stru;

/* AUTORATE ������ص�������� */
typedef struct
{
    mac_alg_cfg_enum_uint8  en_alg_cfg;                         /* ��������ö�� */
    oal_uint8               auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* MAC��ַ */
    oal_uint8               auc_resv[1];
    oal_uint16              us_value;                           /* ������� */
}mac_ioctl_alg_ar_test_param_stru;

/* TXMODE LOG �㷨����ö�٣�����ֵ */
typedef struct
{
    mac_alg_cfg_enum_uint8  en_alg_cfg;     /* ��������ö�� */
    oal_uint8               uc_ac_no;                           /* AC���� */
    oal_uint8               auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* MAC��ַ */
    oal_uint8               auc_resv1[2];
    oal_uint16              us_value;                           /*���ò���ֵ */
}mac_ioctl_alg_txbf_log_param_stru;
/* �㷨��������ӿ� */
typedef struct
{
    oal_uint8       uc_argc;
    oal_uint8       auc_argv_offset[DMAC_ALG_CONFIG_MAX_ARG];
}mac_ioctl_alg_config_stru;

/* TPC LOG �㷨����ö�٣�����ֵ */
typedef struct
{
    mac_alg_cfg_enum_uint8  en_alg_cfg;                         /* ��������ö�� */
    oal_uint8               auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* MAC��ַ */
    oal_uint8               uc_ac_no;                           /* AC���� */
    oal_uint16              us_value;                           /* ���ò���ֵ */
    oal_int8               *pc_frame_name;                      /* ��ȡ�ض�֡����ʹ�øñ��� */
}mac_ioctl_alg_tpc_log_param_stru;

/* cca opt LOG �㷨����ö�٣�����ֵ */
typedef struct
{
    mac_alg_cfg_enum_uint8  en_alg_cfg;                         /* ��������ö�� */
    oal_uint16              us_value;                           /* ͳ����ʱ�� */
    oal_uint8               auc_resv;
}mac_ioctl_alg_intfdet_log_param_stru;

#ifdef _PRE_DEBUG_MODE
/* ɨ��������� */
typedef struct
{
    oal_int8                            ac_scan_type[15];
    wlan_channel_bandwidth_enum_uint8   en_bandwidth;
}mac_ioctl_scan_test_config_stru;
#endif

/*RTS ���Ͳ���*/
typedef struct
{
    wlan_legacy_rate_value_enum_uint8   auc_rate[HAL_TX_RATE_MAX_NUM];           /*�������ʣ���λmpbs*/
    wlan_phy_protocol_enum_uint8        auc_protocol_mode[HAL_TX_RATE_MAX_NUM] ; /*Э��ģʽ, ȡֵ�μ�wlan_phy_protocol_enum_uint8*/
    wlan_channel_band_enum_uint8        en_band;
    oal_uint8                           auc_recv[3];
}mac_cfg_rts_tx_param_stru;

/* �鲥ת���� ���Ͳ��� */
typedef struct
{
    oal_uint8 uc_m2u_mcast_mode;
    oal_uint8 uc_m2u_snoop_on;
}mac_cfg_m2u_snoop_on_param_stru;

#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
/* ��Ƶ�鲥ת������ */
typedef struct
{
    oal_uint8 uc_m2u_frequency_on;
    oal_uint8 auc_rsv[3];
}mac_cfg_m2u_frequency_on_param_stru;

/* ����ģʽ���� */
typedef struct
{
    oal_uint8  uc_m2u_adaptive_on;
    oal_uint8  uc_adaptive_num;
    oal_uint8  auc_rsv[2];
    oal_uint32 ul_threshold_time;
}mac_cfg_m2u_adaptive_on_param_stru;
#endif
/* set snoop tid*/
typedef struct
{
    oal_uint8 uc_m2u_tid_num;
}mac_set_m2u_tid_num_stru;

/* ���鲥ת���������� */
typedef struct
{
    oal_uint32          ul_deny_group_ipv4_addr;
    oal_uint8           ul_deny_group_ipv6_addr[OAL_IPV6_ADDR_SIZE];
    oal_bool_enum_uint8 ip_type;
    oal_uint8           auc_rsv[3];
}mac_add_m2u_deny_table_stru;

/* ɾ���鲥ת���������� */
typedef struct
{
    oal_uint32          ul_deny_group_ipv4_addr;
    oal_uint8           ul_deny_group_ipv6_addr[OAL_IPV6_ADDR_SIZE];
    oal_bool_enum_uint8 ip_type;
    oal_uint8           auc_rsv[3];
}mac_del_m2u_deny_table_stru;


/* ����鲥ת���������� */
typedef struct
{
    oal_uint8 uc_m2u_clear_deny_table;
    oal_uint8 uc_m2u_show_deny_table;
}mac_clg_m2u_deny_table_stru;


/* print snoop table*/
typedef struct
{
    oal_uint8 uc_m2u_show_snoop_table;
}mac_show_m2u_snoop_table_stru;
typedef struct
{
    oal_uint8 uc_m2u_show_snoop_deny_table;
    oal_uint8 auc_rsv[3];
}mac_show_m2u_snoop_deny_table_stru;

/* add snoop table*/
typedef struct
{
    oal_uint8 uc_m2u_add_snoop_table;
}mac_add_m2u_snoop_table_stru;

typedef struct
{
    oal_bool_enum_uint8 en_proxyarp;
    oal_uint8           auc_rsv[3];
}mac_proxyarp_en_stru;

typedef struct
{
    oal_uint8                     auc_bssid[WLAN_MAC_ADDR_LEN];
    oal_uint8                     auc_resv0[2];
    oal_uint8                     auc_pmkid[WLAN_PMKID_LEN];
}mac_cfg_pmksa_param_stru;

typedef struct
{
    oal_uint64                          ull_cookie;
    oal_uint32                          ul_listen_duration;         /* ����ʱ��   */
    oal_uint8                           uc_listen_channel;          /* �������ŵ� */
    wlan_channel_bandwidth_enum_uint8   en_listen_channel_type;     /* �����ŵ����� */
    oal_uint8                           uc_home_channel;            /* �����������ص��ŵ� */
    wlan_channel_bandwidth_enum_uint8   en_home_channel_type;       /* �����������������ŵ����� */
    mac_vap_state_enum_uint8            en_last_vap_state;          /* P2P0��P2P_CL ����VAP �ṹ������������ǰVAP ��״̬�����ڼ�������ʱ�ָ���״̬ */
    wlan_channel_band_enum_uint8        en_band;
    wlan_ieee80211_roc_type_uint8       en_roc_type;                /* roc���� */
    oal_uint8                           uc_rev;
    oal_ieee80211_channel_stru          st_listen_channel;

}mac_remain_on_channel_param_stru;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
typedef struct
{
    oal_uint8    uc_pool_id;
}mac_device_pool_id_stru;
#endif

/* WPAS ����֡���ͽṹ */
typedef struct
{
    oal_int32               channel;
    oal_uint8               mgmt_frame_id;
    oal_uint8               uc_rsv;
    oal_uint16              us_len;
    OAL_CONST oal_uint8    *puc_frame;
} mac_mgmt_frame_stru;

/* STA PS ���Ͳ��� */
#ifdef _PRE_WLAN_FEATURE_STA_PM
typedef struct
{
    oal_uint8   uc_vap_ps_mode;

}mac_cfg_ps_mode_param_stru;

typedef struct
{
    oal_uint16   us_beacon_timeout;
    oal_uint16   us_tbtt_offset;
    oal_uint16   us_ext_tbtt_offset;
	oal_uint16   us_dtim3_on;
}mac_cfg_ps_param_stru;

#ifdef _PRE_PSM_DEBUG_MODE
typedef struct
{
    oal_uint8   uc_psm_info_enable      :2;  //����psm��ͳ��ά�����
    oal_uint8   uc_psm_debug_mode       :2;  //����psm��debug��ӡģʽ
    oal_uint8   uc_psm_resd             :4;
}mac_cfg_ps_info_stru;
#endif

#endif
typedef struct
{
    oal_uint8   uc_show_ip_addr                 :4;  //show ip addr
    oal_uint8   uc_show_arpoffload_info         :4;	 //show arpoffload ά��
}mac_cfg_arpoffload_info_stru;

typedef struct
{
    oal_uint8   uc_in_suspend; //������
    oal_uint8   uc_arpoffload_switch; // arpoffload����
}mac_cfg_suspend_stru;

typedef enum {
    MAC_STA_PM_SWITCH_OFF         = 0,        /* �رյ͹��� */
    MAC_STA_PM_SWITCH_ON          = 1,        /* �򿪵͹��� */
    MAC_STA_PM_MANUAL_MODE_ON     = 2,        /* �����ֶ�sta pm mode */
    MAC_STA_PM_MANUAL_MODE_OFF    = 3,        /* �ر��ֶ�sta pm mode */
    MAC_STA_PM_SWITCH_BUTT,                   /* ������� */
}mac_pm_switch_enum;
typedef oal_uint8 mac_pm_switch_enum_uint8;

typedef enum {
    MAC_STA_PM_CTRL_TYPE_HOST   = 0,        /* �͹��Ŀ������� HOST  */
    MAC_STA_PM_CTRL_TYPE_DBAC   = 1,        /* �͹��Ŀ������� DBAC  */
    MAC_STA_PM_CTRL_TYPE_ROAM   = 2,        /* �͹��Ŀ������� ROAM  */
    MAC_STA_PM_CTRL_TYPE_CMD    = 3,        /* �͹��Ŀ������� CMD   */
    MAC_STA_PM_CTRL_TYPE_BUTT,              /* ������ͣ�ӦС�� 8  */
}mac_pm_ctrl_type_enum;
typedef oal_uint8 mac_pm_ctrl_type_enum_uint8;

typedef struct
{
    mac_pm_ctrl_type_enum_uint8     uc_pm_ctrl_type;       /*mac_pm_ctrl_type_enum*/
    mac_pm_switch_enum_uint8        uc_pm_enable;          /*mac_pm_switch_enum*/
}mac_cfg_ps_open_stru;

/* P2P OPS �������ò��� */
typedef struct
{
    oal_uint8       en_ops_ctrl;
    oal_uint8       uc_ct_window;
    oal_uint8       en_pause_ops;
    oal_uint8       auc_rsv[1];
}mac_cfg_p2p_ops_param_stru;

/* P2P NOA�������ò��� */
typedef struct
{
    oal_uint32      ul_start_time;
    oal_uint32      ul_duration;
    oal_uint32      ul_interval;
    oal_uint8       uc_count;
    oal_uint8       auc_rsv[3];
}mac_cfg_p2p_noa_param_stru;

/* P2P ���ܿ������� */
typedef struct
{
    oal_uint8       uc_p2p_statistics_ctrl;/* 0:���P2P ͳ��ֵ�� 1:��ӡ���ͳ��ֵ */
    oal_uint8       auc_rsv[3];

}mac_cfg_p2p_stat_param_stru;

#ifdef _PRE_WLAN_FEATURE_ROAM
typedef enum
{
    WPAS_CONNECT_STATE_INIT            = 0,
    WPAS_CONNECT_STATE_START           = 1,
    WPAS_CONNECT_STATE_ASSOCIATED      = 2,
    WPAS_CONNECT_STATE_HANDSHAKED      = 3,
    WPAS_CONNECT_STATE_IPADDR_OBTAINED   = 4,
    WPAS_CONNECT_STATE_IPADDR_REMOVED    = 5,
    WPAS_CONNECT_STATE_BUTT
}wpas_connect_state_enum;
typedef oal_uint32 wpas_connect_state_enum_uint32;
/* roam trigger ���ݽṹ�� */
typedef struct
{
    oal_int8                      c_trigger_2G;
    oal_int8                      c_trigger_5G;
    oal_uint16                    us_roam_interval;
}mac_roam_trigger_stru;

/* roam hmac ͬ�� dmac���ݽṹ�� */
typedef struct
{
    oal_uint16                      us_sta_aid;
    oal_uint16                      us_pad;
    mac_channel_stru                st_channel;
    mac_user_cap_info_stru          st_cap_info;
    mac_key_mgmt_stru               st_key_info;
    mac_user_tx_param_stru          st_user_tx_info;
    oal_uint32                      ul_back_to_old;
}mac_h2d_roam_sync_stru;
#endif //_PRE_WLAN_FEATURE_ROAM

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/* ���ƻ� power ref 2g 5g���ò��� */
/* customize rf cfg struct */
typedef struct
{
    oal_int8     c_rf_gain_db_mult4;      /*�ⲿPA/LNA bypassʱ������(����0.25dB)*/
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    oal_int8     c_rf_gain_db_mult10;     /*�ⲿPA/LNA bypassʱ������(����0.1dB)*/
    oal_uint8    auc_resv[2];
#else
    oal_uint8    auc_resv[3];
#endif
}mac_cfg_gain_db_per_band;

typedef struct
{
    /* 2g */
    mac_cfg_gain_db_per_band    ac_gain_db_2g[MAC_NUM_2G_BAND];
    /* 5g */
    mac_cfg_gain_db_per_band    ac_gain_db_5g[MAC_NUM_5G_BAND];
}mac_cfg_custom_gain_db_stru;

typedef struct
{
    oal_int8   c_cfg_delta_pwr_ref_rssi_2g[CUS_NUM_2G_DELTA_RSSI_NUM];
    oal_int8   c_cfg_delta_pwr_ref_rssi_5g[CUS_NUM_5G_DELTA_RSSI_NUM];
}mac_cfg_custom_delta_pwr_ref_stru;

typedef struct
{
    oal_uint16                  us_lna_on2off_time_ns;           /* LNA����LNA�ص�ʱ��(ns) */
    oal_uint16                  us_lna_off2on_time_ns;           /* LNA�ص�LNA����ʱ��(ns) */

    oal_int8                    c_lna_bypass_gain_db;         /* �ⲿLNA bypassʱ������(dB) */
    oal_int8                    c_lna_gain_db;                /* �ⲿLNA����(dB) */
    oal_int8                    c_pa_gain_b0_db;              /* �ⲿPA b0 ����(dB) */
    oal_uint8                   uc_ext_switch_isexist;        /* �Ƿ�ʹ���ⲿswitch */

    oal_uint8                   uc_ext_pa_isexist;            /* �Ƿ�ʹ���ⲿpa */
    oal_uint8                   uc_ext_lna_isexist;           /* �Ƿ�ʹ���ⲿlna */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    oal_int8                    c_pa_gain_b1_db;              /* �ⲿPA b1����(dB) */
    oal_uint8                   uc_pa_gain_lvl_num;            /* �ⲿPA ���浵λ�� */
#else
    oal_uint8                   auc_resv[2];
#endif
}mac_cfg_custom_ext_rf_stru;

typedef struct
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    mac_cfg_custom_gain_db_stru ast_rf_gain_db_rf[WLAN_RF_CHANNEL_NUMS];          /* 2.4g 5g ���� */
    mac_cfg_custom_ext_rf_stru  ast_ext_rf[WLAN_BAND_BUTT][WLAN_RF_CHANNEL_NUMS]; /* 2.4g 5g fem*/
    mac_cfg_custom_delta_pwr_ref_stru  ast_delta_pwr_ref_cfg[WLAN_RF_CHANNEL_NUMS]; /* delta_rssi */
#else
    mac_cfg_custom_gain_db_stru st_rf_gain_db_rf;
    mac_cfg_custom_ext_rf_stru  st_ext_rf;
#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)

    oal_uint8                   uc_far_dist_pow_gain_switch;     /* ��Զ���빦�����濪�� */
    oal_uint8                   uc_far_dist_dsss_scale_promote_switch;   /* ��Զ��11b 1m 2m dbb scale����ʹ�ܿ��� */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    oal_uint8                   uc_chn_radio_cap;
#else
    oal_uint8                   auc_resv[1];
#endif
    oal_int8                    c_delta_cca_ed_high_80th_5g;

    /* ע�⣬����޸��˶�Ӧ��λ�ã���Ҫͬ���޸ĺ���: hal_config_custom_rf  */
    oal_int8                    c_delta_cca_ed_high_20th_2g;
    oal_int8                    c_delta_cca_ed_high_40th_2g;
    oal_int8                    c_delta_cca_ed_high_20th_5g;
    oal_int8                    c_delta_cca_ed_high_40th_5g;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    oal_uint32                  aul_5g_upc_mix_gain_rf_reg[MAC_NUM_5G_BAND];
    oal_uint32                  aul_2g_pa_bias_rf_reg[CUS_RF_PA_BIAS_REG_NUM];   /* ���ƻ�RF����PAƫ�üĴ���reg236~244, reg253~284 */
#endif
}mac_cfg_customize_rf;

typedef struct
{
    oal_uint16                  us_tx_ratio;                        /* txռ�ձ� */
    oal_uint16                  us_tx_pwr_comp_val;                 /* ���书�ʲ���ֵ */
}mac_tx_ratio_vs_pwr_stru;

/* ���ƻ�TXռ�ձ�&�¶Ȳ������书�ʵĲ��� */
typedef struct
{
    mac_tx_ratio_vs_pwr_stru    ast_txratio2pwr[3];                 /* 3��ռ�ձȷֱ��Ӧ���ʲ���ֵ */
    oal_uint32                  ul_more_pwr;                        /* �����¶ȶ��ⲹ���ķ��书�� */
}mac_cfg_customize_tx_pwr_comp_stru;

/* RF�Ĵ������ƻ��ṹ�� */
typedef struct
{
    oal_uint16                 us_rf_reg117;
    oal_uint16                 us_rf_reg123;
    oal_uint16                 us_rf_reg124;
    oal_uint16                 us_rf_reg125;
    oal_uint16                 us_rf_reg126;
    oal_uint8                  auc_resv[2];
}mac_cus_dts_rf_reg;

/* FCC��֤ �����ṹ�� */
typedef struct
{
    oal_uint8       uc_index;           /* �±��ʾƫ�� */
    oal_uint8       uc_max_txpower;     /* ����͹��� */
    oal_uint16      us_dbb_scale;       /* dbb scale */
}mac_cus_band_edge_limit_stru;

/* ���ƻ� У׼���ò��� */
typedef struct
{
    /* dts */
    oal_int16      aus_cali_txpwr_pa_dc_ref_2g_val_chan[MAC_2G_CHANNEL_NUM];  /* txpwr���ŵ�refֵ */
    oal_int16      aus_cali_txpwr_pa_dc_ref_5g_val_band[MAC_NUM_5G_BAND];
#if (_PRE_TARGET_PRODUCT_TYPE_E5 == _PRE_CONFIG_TARGET_PRODUCT)
    oal_int16      aus_cali_txpwr_pa_dc_ref_2g_b40_val[MAC_2G_CHANNEL_NUM];   /* E5-5885 B40ͨ·refֵ */
    oal_uint8      auc_cali_resv[2];
#endif //#if (_PRE_TARGET_PRODUCT_TYPE_E5 == _PRE_CONFIG_TARGET_PRODUCT)
}mac_cus_cali_stru;

#ifdef _PRE_WLAN_ONLINE_DPD
/* ���ƻ� DPDУ׼���ò��� */
typedef struct
{
    oal_uint32 aul_dpd_cali_cus_dts[MAC_DPD_CALI_CUS_PARAMS_NUM];
}mac_dpd_cus_cali_stru;
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
typedef struct
{
    /* bt tmp */
    oal_int16                  us_cali_bt_txpwr_pa_ref_band1;
    oal_int16                  us_cali_bt_txpwr_pa_ref_band2;
    oal_int16                  us_cali_bt_txpwr_pa_ref_band3;
    oal_int16                  us_cali_bt_txpwr_pa_ref_band4;
    oal_int16                  us_cali_bt_txpwr_pa_ref_band5;
    oal_int16                  us_cali_bt_txpwr_pa_ref_band6;
    oal_int16                  us_cali_bt_txpwr_pa_ref_band7;
    oal_int16                  us_cali_bt_txpwr_pa_ref_band8;
    oal_int16                  us_cali_bt_txpwr_numb;
    oal_int16                  us_cali_bt_txpwr_pa_fre1;
    oal_int16                  us_cali_bt_txpwr_pa_fre2;
    oal_int16                  us_cali_bt_txpwr_pa_fre3;
    oal_int16                  us_cali_bt_txpwr_pa_fre4;
    oal_int16                  us_cali_bt_txpwr_pa_fre5;
    oal_int16                  us_cali_bt_txpwr_pa_fre6;
    oal_int16                  us_cali_bt_txpwr_pa_fre7;
    oal_int16                  us_cali_bt_txpwr_pa_fre8;
    oal_uint8                  uc_bt_tone_amp_grade;
    oal_uint8                  auc_resv_bt_cali[1];
}mac_cus_bt_cali_stru;
#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
typedef struct
{
    oal_int32 al_dy_cali_base_ratio_params[CUS_DY_CALI_PARAMS_NUM][CUS_DY_CALI_PARAMS_TIMES];    /* ���ⶨ�ƻ��������� */
    oal_int32 al_dy_cali_base_ratio_ppa_params[CUS_DY_CALI_PARAMS_TIMES];                        /* ppa-pow���ƻ��������� */
    oal_int8  ac_dy_cali_2g_dpn_params[MAC_2G_CHANNEL_NUM][CUS_DY_CALI_DPN_PARAMS_NUM];
    oal_int8  ac_dy_cali_5g_dpn_params[CUS_DY_CALI_NUM_5G_BAND][WLAN_BW_CAP_80PLUS80];
    oal_int16 as_extre_point_val[CUS_DY_CALI_NUM_5G_BAND];
    oal_uint8 uc_rf_id;
#ifdef _PRE_WLAN_DPINIT_CALI
    oal_int8  ac_dp_init[MAC_2G_CHANNEL_NUM];
#endif
}mac_cus_dy_cali_param_stru;
#endif

typedef struct
{
    mac_cus_cali_stru          ast_cali[WLAN_RF_CHANNEL_NUMS];
#ifdef _PRE_WLAN_ONLINE_DPD
    mac_dpd_cus_cali_stru      ast_dpd_cali_para[WLAN_RF_CHANNEL_NUMS];
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    mac_cus_bt_cali_stru       st_cali_bt;
#endif //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    oal_uint16                 aus_dyn_cali_dscr_interval[WLAN_BAND_BUTT];   /* ��̬У׼����2.4g 5g */
    oal_int16                  as_gm0_dB10_amend[WLAN_RF_CHANNEL_NUMS];

    oal_bool_enum_uint8        en_dyn_cali_opt_switch;
#endif //#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    oal_int8                   uc_band_5g_enable;
    oal_uint8                  uc_tone_amp_grade;
    oal_uint8                  auc_resv[1];
}mac_cus_dts_cali_stru;

/* perf */
typedef struct
{
    signed char               ac_used_mem_param[16];
    unsigned char             uc_sdio_assem_d2h;
    unsigned char             auc_resv[3];
}config_device_perf_h2d_stru;

#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#ifdef _PRE_WLAN_FEATURE_UAPSD
extern oal_uint8 g_uc_uapsd_cap_etc;
#endif

/* 1102 wiphy Vendor CMD���� ��Ӧcfgid: WLAN_CFGID_VENDOR_CMD */
typedef struct mac_vendor_cmd_channel_list_info
{
    oal_uint8 uc_channel_num_2g;
    oal_uint8 uc_channel_num_5g;
    oal_uint8 auc_channel_list_2g[MAC_CHANNEL_FREQ_2_BUTT];
    oal_uint8 auc_channel_list_5g[MAC_CHANNEL_FREQ_5_BUTT];
}mac_vendor_cmd_channel_list_stru;

/*CHR2.0ʹ�õ�STAͳ����Ϣ*/
typedef  struct
{
    oal_uint8                        uc_distance;                               /*�㷨��tpc���룬��Ӧdmac_alg_tpc_user_distance_enum*/
    oal_uint8                        uc_cca_intr;                               /*�㷨��cca_intr���ţ���Ӧalg_cca_opt_intf_enum*/
    oal_int8                         c_snr_ant0;                                /* ����0�ϱ���SNRֵ */
    oal_int8                         c_snr_ant1;                                /* ����1�ϱ���SNRֵ */
    oal_uint32                       ul_bcn_cnt;                                /*�յ���beacon����*/
    oal_uint32                       ul_bcn_tout_cnt;                           /*beacon��ʧ�ļ���*/
}station_info_extend_stru;

/* m2s vap��Ϣ�ṹ�� */
typedef struct
{
    oal_uint8               uc_vap_id; /* ҵ��vap id */
    wlan_nss_enum_uint8     en_vap_rx_nss;           /* vap�Ľ��տռ������� */
    wlan_m2s_type_enum_uint8      en_m2s_type;    /*0:���л� 1:Ӳ�л�*/
    wlan_mib_mimo_power_save_enum_uint8 en_sm_power_save;

    oal_bool_enum_uint8     en_tx_stbc;
    oal_bool_enum_uint8     en_transmit_stagger_sounding;
    oal_bool_enum_uint8     en_vht_ctrl_field_supported;
    oal_bool_enum_uint8     en_vht_number_sounding_dimensions;

    oal_bool_enum_uint8     en_vht_su_beamformer_optionimplemented;
    oal_bool_enum_uint8     en_tx_vht_stbc_optionimplemented;
    oal_bool_enum_uint8     en_support_opmode;
    oal_uint8               auc_reserved[1];
}mac_vap_m2s_stru;

typedef struct
{
    const oal_int8 * pstr;
    wlan_channel_bandwidth_enum_uint8 en_width;
}width_type_info_stru;

#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
/* qos enhance�������� */
typedef struct
{
    oal_uint8 uc_qos_enhance_on;
    oal_uint8 auc_rsv[3];
}mac_cfg_qos_enhance_on_param_stru;

#endif

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
typedef enum mac_sensing_bssid_operate
{
    OAL_SENSING_BSSID_OPERATE_DEL = 0,
    OAL_SENSING_BSSID_OPERATE_ADD = 1,

    OAL_SENSING_BSSID_OPERATE_BUTT
} mac_sensing_bssid_operate_en;
typedef oal_uint8 oal_en_sensing_bssid_operate_uint8;

typedef struct
{
    oal_uint8            auc_mac_addr[WLAN_MAC_ADDR_LEN];   /*��Ӧ��MAC��ַ */
    oal_int8             c_rssi;
    oal_uint8            uc_reserved;
    oal_uint32           ul_timestamp;
} dmac_query_sensing_bssid_stru;

typedef struct mac_sensing_bssid
{
    oal_uint8  auc_mac_addr[OAL_MAC_ADDR_LEN];
    oal_en_sensing_bssid_operate_uint8  en_operation;   /** 0ɾ���� 1��� */
    oal_uint8  reserved;
} dmac_sensing_bssid_cfg_stru;
#endif


OAL_STATIC OAL_INLINE wlan_channel_bandwidth_enum_uint8  mac_vap_str_to_width(oal_int8 * p_width_str)
{
    oal_uint32                  i;
    width_type_info_stru st_width_tbl[]={
        {"20",WLAN_BAND_WIDTH_20M},
        {"40+",WLAN_BAND_WIDTH_40PLUS},
        {"40-",WLAN_BAND_WIDTH_40MINUS},
        {"80++",WLAN_BAND_WIDTH_80PLUSPLUS},
        {"80--",WLAN_BAND_WIDTH_80MINUSMINUS},
        {"80+-",WLAN_BAND_WIDTH_80PLUSMINUS},
        {"80-+",WLAN_BAND_WIDTH_80MINUSPLUS},
     };

     for(i = 0; i < sizeof(st_width_tbl)/sizeof(width_type_info_stru); i++)
     {
        if(0 == oal_strcmp(p_width_str,st_width_tbl[i].pstr))
        {
            return st_width_tbl[i].en_width;
        }
     }

     return WLAN_BAND_WIDTH_BUTT;
}

OAL_STATIC OAL_INLINE const oal_int8 * mac_vap_width_to_str(wlan_channel_bandwidth_enum_uint8 en_width)
{
    oal_uint32                  i;
    width_type_info_stru st_width_tbl[]={
        {"20",WLAN_BAND_WIDTH_20M},
        {"40+",WLAN_BAND_WIDTH_40PLUS},
        {"40-",WLAN_BAND_WIDTH_40MINUS},
        {"80++",WLAN_BAND_WIDTH_80PLUSPLUS},
        {"80--",WLAN_BAND_WIDTH_80MINUSMINUS},
        {"80+-",WLAN_BAND_WIDTH_80PLUSMINUS},
        {"80-+",WLAN_BAND_WIDTH_80MINUSPLUS},
     };

     for(i = 0; i < sizeof(st_width_tbl)/sizeof(width_type_info_stru); i++)
     {
        if(en_width == st_width_tbl[i].en_width)
        {
            return st_width_tbl[i].pstr;
        }
     }

     return "unknow";
}

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/
typedef oal_void  (*ch_mib_by_bw)(mac_vap_stru *pst_mac_vap, wlan_channel_band_enum_uint8 en_band,  wlan_channel_bandwidth_enum_uint8 en_bandwidth);
typedef oal_void (*init_11ac_rates)(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev);
typedef oal_void (*init_11n_rates)(mac_vap_stru *pst_mac_vap,mac_device_stru *pst_mac_dev);
typedef oal_uint32 (*init_privacy)(mac_vap_stru *pst_mac_vap, mac_conn_security_stru *pst_conn_sec);
typedef oal_void (*init_rates_by_prot)(mac_vap_stru *pst_vap, wlan_protocol_enum_uint8 en_vap_protocol, mac_data_rate_stru *pst_rates);
typedef oal_void (* init_11ax_mib)(mac_vap_stru  *pst_mac_vap);
typedef oal_void (*init_11ax_rates)(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev);
typedef oal_void  (*p_mac_vap_init_mib_11n_cb)(mac_vap_stru  *pst_mac_vap);
typedef oal_void  (*p_mac_init_mib_cb)(mac_vap_stru *pst_mac_vap);
typedef oal_uint32 (*p_mac_vap_init_by_protocol_cb)(mac_vap_stru *pst_mac_vap, wlan_protocol_enum_uint8 en_protocol);
typedef oal_void  (*p_mac_vap_init_11ac_mcs_singlenss_cb)(mac_vap_stru           *pst_mac_vap,
                                                     wlan_channel_bandwidth_enum_uint8    en_bandwidth);
typedef oal_void  (*p_mac_vap_init_11ac_mcs_doublenss_cb)(mac_vap_stru *pst_mac_vap,
                wlan_channel_bandwidth_enum_uint8    en_bandwidth);

typedef struct
{
    p_mac_init_mib_cb                       mac_init_mib_cb;
    p_mac_vap_init_mib_11n_cb               mac_vap_init_mib_11n_cb;
    p_mac_vap_init_by_protocol_cb           mac_vap_init_by_protocol_cb;
    p_mac_vap_init_11ac_mcs_singlenss_cb    mac_vap_init_11ac_mcs_singlenss_cb;
    p_mac_vap_init_11ac_mcs_doublenss_cb    mac_vap_init_11ac_mcs_doublenss_cb;
    ch_mib_by_bw                            ch_mib_by_bw_cb;
    init_11ac_rates                         init_11ac_rates_cb;
    init_11n_rates                          init_11n_rates_cb;
    init_privacy                            init_privacy_cb;
    init_rates_by_prot                      init_rates_by_prot_cb;
    init_11ax_rates                         init_11ax_rates_cb;
    init_11ax_mib                           init_11ax_mib_cb;
}mac_vap_cb;
extern mac_vap_cb  g_st_mac_vap_rom_cb;

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
typedef struct
{
    /* ���ƻ��Ƿ��amsdu����ۺ� */
    oal_uint8    uc_host_large_amsdu_en;
    /* ��ǰ�ۺ��Ƿ�Ϊamsdu�ۺ� */
    oal_uint8    uc_cur_amsdu_enable;
    oal_uint16   us_amsdu_throughput_high;
    oal_uint16   us_amsdu_throughput_low;
    oal_uint16   us_resv;
}mac_tx_large_amsdu_ampdu_stru;
extern mac_tx_large_amsdu_ampdu_stru g_st_tx_large_amsdu;
#endif
#ifdef _PRE_WLAN_TCP_OPT
typedef struct
{
    /* ���ƻ��Ƿ��tcp ack���� */
    oal_uint8    uc_tcp_ack_filter_en;
    /* ��ǰ״̬ */
    oal_uint8    uc_cur_filter_status;
    oal_uint16   us_rx_filter_throughput_high;
    oal_uint16   us_rx_filter_throughput_low;
    oal_uint16   us_resv;
}mac_tcp_ack_filter_stru;
extern mac_tcp_ack_filter_stru g_st_tcp_ack_filter;
#endif

typedef struct
{
    /* �Ƿ�ʹ���ֶ����ý��վۺϸ��� */
    oal_uint8    uc_rx_buffer_size_set_en;
    oal_uint8    uc_rx_buffer_size;     /* addba rsp�ظ��ľۺϸ��� */
    oal_uint16   us_resv;
}mac_rx_buffer_size_stru;
extern mac_rx_buffer_size_stru g_st_rx_buffer_size_stru;

typedef struct
{
    /* ���ƻ�С��amsdu���� */
    oal_uint8    uc_ini_small_amsdu_en;
    oal_uint8    uc_cur_small_amsdu_en;
    oal_uint16   us_small_amsdu_throughput_high;
    oal_uint16   us_small_amsdu_throughput_low;
    oal_uint16   us_small_amsdu_pps_high;
    oal_uint16   us_small_amsdu_pps_low;
    oal_uint16   us_resv;
}mac_small_amsdu_switch_stru;
extern mac_small_amsdu_switch_stru g_st_small_amsdu_switch;


typedef struct
{
    oal_uint8    uc_ini_tcp_ack_buf_en;
    oal_uint8    uc_cur_tcp_ack_buf_en;
    oal_uint16   us_tcp_ack_buf_throughput_high;
    oal_uint16   us_tcp_ack_buf_throughput_low;
    oal_uint16   us_tcp_ack_buf_throughput_high_40M;
    oal_uint16   us_tcp_ack_buf_throughput_low_40M;
    oal_uint16   us_tcp_ack_buf_throughput_high_80M;
    oal_uint16   us_tcp_ack_buf_throughput_low_80M;
    oal_uint16   us_tcp_ack_buf_throughput_high_160M;
    oal_uint16   us_tcp_ack_buf_throughput_low_160M;
    oal_uint16   us_tcp_ack_smooth_throughput;
}mac_tcp_ack_buf_switch_stru;
extern mac_tcp_ack_buf_switch_stru g_st_tcp_ack_buf_switch;

typedef struct
{
    oal_uint8    uc_ini_en;             /* ���ƻ��������¶�̬bypass extLNA���� */
    oal_uint8    uc_cur_status;         /* ��ǰ�Ƿ�Ϊ�͹��Ĳ���״̬ */
    oal_uint16   us_throughput_high;
    oal_uint16   us_throughput_low;
    oal_uint16   us_resv;
}mac_rx_dyn_bypass_extlna_stru;
extern mac_rx_dyn_bypass_extlna_stru g_st_rx_dyn_bypass_extlna_switch;


#if 0
OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_get_excludeunencrypted(mac_vap_stru *pst_mac_vap)
{
  return pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11ExcludeUnencrypted;
}
#endif

OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_pre_auth_actived(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAPreauthenticationActivated;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_pre_auth_actived(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_pre_auth)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAPreauthenticationActivated = en_pre_auth;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_rsnacfg_ptksareplaycounters(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.uc_dot11RSNAConfigPTKSAReplayCounters;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_rsnacfg_ptksareplaycounters(mac_vap_stru *pst_mac_vap, oal_uint8 uc_ptksa_replay_counters)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.uc_dot11RSNAConfigPTKSAReplayCounters= uc_ptksa_replay_counters;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_rsnacfg_gtksareplaycounters(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.uc_dot11RSNAConfigGTKSAReplayCounters;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_rsnacfg_gtksareplaycounters(mac_vap_stru *pst_mac_vap, oal_uint8 uc_gtksa_replay_counters)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.uc_dot11RSNAConfigGTKSAReplayCounters = uc_gtksa_replay_counters;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_init_rsnacfg_suites(mac_vap_stru *pst_mac_vap)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.ul_wpa_group_suite     = MAC_WPA_CHIPER_TKIP;
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites[0] = MAC_WPA_CHIPER_CCMP;
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites[1] = MAC_WPA_CHIPER_TKIP;

    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites[0] = MAC_WPA_AKM_PSK;
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites[1] = MAC_WPA_AKM_PSK_SHA256;

    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.ul_rsn_group_suite     = MAC_RSN_CHIPER_CCMP;
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites[0] = MAC_RSN_CHIPER_CCMP;
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites[1] = MAC_RSN_CHIPER_TKIP;

    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites[0] = MAC_RSN_AKM_PSK;
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites[1] = MAC_RSN_AKM_PSK_SHA256;
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.ul_rsn_group_mgmt_suite = MAC_RSN_CIPHER_SUITE_AES_128_CMAC;

}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_wpa_group_suite(mac_vap_stru *pst_mac_vap, oal_uint32 ul_suite)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.ul_wpa_group_suite = ul_suite;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_rsn_group_suite(mac_vap_stru *pst_mac_vap, oal_uint32 ul_suite)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.ul_rsn_group_suite = ul_suite;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_rsn_group_mgmt_suite(mac_vap_stru *pst_mac_vap, oal_uint32 ul_suite)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.ul_rsn_group_mgmt_suite = ul_suite;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_wpa_group_suite(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.ul_wpa_group_suite;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_rsn_group_suite(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.ul_rsn_group_suite;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_rsn_group_mgmt_suite(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.ul_rsn_group_mgmt_suite;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_wpa_pair_suites(mac_vap_stru *pst_mac_vap, oal_uint32 *pul_suites)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites[0] = pul_suites[0];
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites[1] = pul_suites[1];
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_rsn_pair_suites(mac_vap_stru *pst_mac_vap, oal_uint32 *pul_suites)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites[0] = pul_suites[0];
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites[1] = pul_suites[1];
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_wpa_akm_suites(mac_vap_stru *pst_mac_vap, oal_uint32 *pul_suites)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites[0] = pul_suites[0];
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites[1] = pul_suites[1];
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_rsn_akm_suites(mac_vap_stru *pst_mac_vap, oal_uint32 *pul_suites)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites[0] = pul_suites[0];
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites[1] = pul_suites[1];
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_wpa_pair_match_suites(mac_vap_stru *pst_mac_vap, oal_uint32 *pul_suites)
{
    oal_uint8 uc_idx_local;
    oal_uint8 uc_idx_peer;

    for (uc_idx_local = 0; uc_idx_local < WLAN_PAIRWISE_CIPHER_SUITES; uc_idx_local++)
    {
        for (uc_idx_peer = 0; uc_idx_peer < WLAN_PAIRWISE_CIPHER_SUITES; uc_idx_peer++)
        {
            if (pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites[uc_idx_local] == pul_suites[uc_idx_peer])
            {
                return pul_suites[uc_idx_peer];
            }
        }
    }
    return 0;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_rsn_pair_match_suites(mac_vap_stru *pst_mac_vap, oal_uint32 *pul_suites)
{
    oal_uint8 uc_idx_local;
    oal_uint8 uc_idx_peer;

    for (uc_idx_local = 0; uc_idx_local < WLAN_PAIRWISE_CIPHER_SUITES; uc_idx_local++)
    {
        for (uc_idx_peer = 0; uc_idx_peer < WLAN_PAIRWISE_CIPHER_SUITES; uc_idx_peer++)
        {
            if (pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites[uc_idx_local] == pul_suites[uc_idx_peer])
            {
                return pul_suites[uc_idx_peer];
            }
        }
    }
    return 0;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_wpa_akm_match_suites(mac_vap_stru *pst_mac_vap, oal_uint32 *pul_suites)
{
    oal_uint8 uc_idx_local;
    oal_uint8 uc_idx_peer;

    for (uc_idx_local = 0; uc_idx_local < WLAN_PAIRWISE_CIPHER_SUITES; uc_idx_local++)
    {
        for (uc_idx_peer = 0; uc_idx_peer < WLAN_PAIRWISE_CIPHER_SUITES; uc_idx_peer++)
        {
            if (pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites[uc_idx_local] == pul_suites[uc_idx_peer])
            {
                return pul_suites[uc_idx_peer];
            }
        }
    }
    return 0;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_rsn_akm_match_suites(mac_vap_stru *pst_mac_vap, oal_uint32 *pul_suites)
{
    oal_uint8 uc_idx_local;
    oal_uint8 uc_idx_peer;

    for (uc_idx_local = 0; uc_idx_local < WLAN_PAIRWISE_CIPHER_SUITES; uc_idx_local++)
    {
        for (uc_idx_peer = 0; uc_idx_peer < WLAN_PAIRWISE_CIPHER_SUITES; uc_idx_peer++)
        {
            if (pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites[uc_idx_local] == pul_suites[uc_idx_peer])
            {
                return pul_suites[uc_idx_peer];
            }
        }
    }
    return 0;
}
OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_wpa_pair_suites(mac_vap_stru *pst_mac_vap, oal_uint32 *pul_suites)
{
    oal_uint8   uc_loop;
    oal_uint8   uc_num = 0;

    for (uc_loop = 0; uc_loop < WLAN_PAIRWISE_CIPHER_SUITES; uc_loop++)
    {
        if (0 != pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites[uc_loop])
        {
            pul_suites[uc_num++] = pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites[uc_loop];
        }
    }
    return uc_num;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_rsn_pair_suites(mac_vap_stru *pst_mac_vap, oal_uint32 *pul_suites)
{
    oal_uint8   uc_loop;
    oal_uint8   uc_num = 0;

    for (uc_loop = 0; uc_loop < WLAN_PAIRWISE_CIPHER_SUITES; uc_loop++)
    {
        if (0 != pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites[uc_loop])
        {
            pul_suites[uc_num++] = pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites[uc_loop];
        }
    }
    return uc_num;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_wpa_akm_suites(mac_vap_stru *pst_mac_vap, oal_uint32 *pul_suites)
{
    oal_uint8   uc_num = 0;
    oal_uint8   uc_loop;

    for (uc_loop = 0; uc_loop < WLAN_AUTHENTICATION_SUITES; uc_loop++)
    {
        if (0 != pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites[uc_loop])
        {
            pul_suites[uc_num++] = pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites[uc_loop];
        }
    }

    return uc_num;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_rsn_akm_suites(mac_vap_stru *pst_mac_vap, oal_uint32 *pul_suites)
{
    oal_uint8   uc_num = 0;
    oal_uint8   uc_loop;

    for (uc_loop = 0; uc_loop < WLAN_AUTHENTICATION_SUITES; uc_loop++)
    {
        if (0 != pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites[uc_loop])
        {
            pul_suites[uc_num++] = pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites[uc_loop];
        }
    }

    return uc_num;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_maxmpdu_length(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11MaxMPDULength;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_maxmpdu_length(mac_vap_stru *pst_mac_vap, oal_uint32 ul_maxmpdu_length)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11MaxMPDULength = ul_maxmpdu_length;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_vht_max_rx_ampdu_factor(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTMaxRxAMPDUFactor;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_vht_max_rx_ampdu_factor(mac_vap_stru *pst_mac_vap, oal_uint32 ul_vht_max_rx_ampdu_factor)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTMaxRxAMPDUFactor = ul_vht_max_rx_ampdu_factor;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_vht_ctrl_field_cap(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTControlFieldSupported;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_vht_ctrl_field_cap(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_vht_ctrl_field_supported)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTControlFieldSupported = en_vht_ctrl_field_supported;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_txopps(mac_vap_stru *pst_vap)
{
    return pst_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTTXOPPowerSaveOptionImplemented;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_txopps(mac_vap_stru *pst_vap, oal_bool_enum_uint8 en_vht_txop_ps)
{
    pst_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTTXOPPowerSaveOptionImplemented = en_vht_txop_ps;
}

OAL_STATIC OAL_INLINE oal_uint16 mac_mib_get_vht_rx_mcs_map(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTRxMCSMap;
}

OAL_STATIC OAL_INLINE oal_void* mac_mib_get_ptr_vht_rx_mcs_map(mac_vap_stru *pst_mac_vap)
{
    return &(pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTRxMCSMap);
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_vht_rx_mcs_map(mac_vap_stru *pst_mac_vap, oal_uint16 us_vht_mcs_mpa)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTRxMCSMap = us_vht_mcs_mpa;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_us_rx_highest_rate(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTRxHighestDataRateSupported;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_us_rx_highest_rate(mac_vap_stru *pst_mac_vap, oal_uint32 ul_rx_highest_rate)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTRxHighestDataRateSupported = ul_rx_highest_rate;
}

OAL_STATIC OAL_INLINE oal_uint16 mac_mib_get_vht_tx_mcs_map(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTTxMCSMap;
}

OAL_STATIC OAL_INLINE oal_void* mac_mib_get_ptr_vht_tx_mcs_map(mac_vap_stru *pst_mac_vap)
{
    return &(pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTTxMCSMap);
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_vht_tx_mcs_map(mac_vap_stru *pst_mac_vap, oal_uint16 us_vht_mcs_mpa)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTTxMCSMap = us_vht_mcs_mpa;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_us_tx_highest_rate(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTTxHighestDataRateSupported;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_us_tx_highest_rate(mac_vap_stru *pst_mac_vap, oal_uint32 ul_tx_highest_rate)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTTxHighestDataRateSupported = ul_tx_highest_rate;
}

#ifdef _PRE_WLAN_FEATURE_SMPS
OAL_STATIC OAL_INLINE wlan_mib_mimo_power_save_enum_uint8 mac_mib_get_smps(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MIMOPowerSave;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_smps(mac_vap_stru *pst_mac_vap, wlan_mib_mimo_power_save_enum_uint8 en_sm_power_save)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MIMOPowerSave = en_sm_power_save;
}
#endif //_PRE_WLAN_FEATURE_SMPS

OAL_STATIC OAL_INLINE wlan_mib_max_amsdu_lenth_enum_uint16 mac_mib_get_max_amsdu_length(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MaxAMSDULength;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_max_amsdu_length(mac_vap_stru *pst_mac_vap, wlan_mib_max_amsdu_lenth_enum_uint16 en_max_amsdu_length)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MaxAMSDULength = en_max_amsdu_length;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_lsig_txop_protection(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11LsigTxopProtectionOptionImplemented;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_lsig_txop_protection(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_lsig_txop_protection)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11LsigTxopProtectionOptionImplemented = en_lsig_txop_protection;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_max_ampdu_len_exponent(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.ul_dot11MaxRxAMPDUFactor;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_max_ampdu_len_exponent(mac_vap_stru *pst_mac_vap, oal_uint32 ul_max_ampdu_len_exponent)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.ul_dot11MaxRxAMPDUFactor = ul_max_ampdu_len_exponent;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_min_mpdu_start_spacing(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.ul_dot11MinimumMPDUStartSpacing;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_min_mpdu_start_spacing(mac_vap_stru *pst_mac_vap, oal_uint8 ul_min_mpdu_start_spacing)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.ul_dot11MinimumMPDUStartSpacing = ul_min_mpdu_start_spacing;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_pco_option_implemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11PCOOptionImplemented;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_pco_option_implemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_pco_option_implemented)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11PCOOptionImplemented = en_pco_option_implemented;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_transition_time(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.ul_dot11TransitionTime;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_transition_time(mac_vap_stru *pst_mac_vap, oal_uint32 ul_transition_time)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.ul_dot11TransitionTime = ul_transition_time;
}

OAL_STATIC OAL_INLINE wlan_mib_mcs_feedback_opt_implt_enum_uint8 mac_mib_get_mcs_fdbk(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MCSFeedbackOptionImplemented;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_mcs_fdbk(mac_vap_stru *pst_mac_vap, wlan_mib_mcs_feedback_opt_implt_enum_uint8 en_mcs_fdbk)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MCSFeedbackOptionImplemented = en_mcs_fdbk;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_htc_sup(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11HTControlFieldSupported;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_htc_sup(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_htc_sup)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11HTControlFieldSupported = en_htc_sup;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_rd_rsp(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11RDResponderOptionImplemented;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_rd_rsp(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_rd_rsp)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11RDResponderOptionImplemented = en_rd_rsp;
}

#if defined(_PRE_WLAN_FEATURE_11R) || (defined(_PRE_WLAN_FEATURE_11R_AP))
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ft_trainsistion(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FastBSSTransitionActivated;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_ft_trainsistion(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_ft_trainsistion)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FastBSSTransitionActivated = en_ft_trainsistion;
}

OAL_STATIC OAL_INLINE oal_uint16 mac_mib_get_ft_mdid(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.us_dot11FTMobilityDomainID;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_ft_mdid(mac_vap_stru *pst_mac_vap, oal_uint16 ul_ft_mdid)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.us_dot11FTMobilityDomainID = ul_ft_mdid;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ft_over_ds(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTOverDSActivated;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_ft_over_ds(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_ft_over_ds)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTOverDSActivated = en_ft_over_ds;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ft_resource_req(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTResourceRequestSupported;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_ft_resource_req(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_ft_resource_req)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTResourceRequestSupported = en_ft_resource_req;
}
#endif //_PRE_WLAN_FEATURE_11R

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
#define mac_vap_is_msta(vap) ((vap)->st_psta.en_psta_mode == PSTA_MODE_MSTA)
#define mac_vap_is_vsta(vap) ((vap)->st_psta.en_psta_mode == PSTA_MODE_VSTA)
#define mac_vap_is_pbss(vap) ((vap)->st_psta.en_psta_mode == PSTA_MODE_PBSS)

#define mac_param_is_msta(param) ((param)->en_psta_mode == PSTA_MODE_MSTA)
#define mac_param_is_vsta(param) ((param)->en_psta_mode == PSTA_MODE_VSTA)
#define mac_param_is_pbss(param) ((param)->en_psta_mode == PSTA_MODE_PBSS)


OAL_STATIC OAL_INLINE  oal_uint32 mac_psta_init_vap(mac_vap_stru *pst_mac_vap, mac_cfg_add_vap_param_stru *pst_param)
{
    /* init role for any vap */
    pst_mac_vap->st_psta.en_psta_mode = pst_param->en_psta_mode;

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_HILINK

OAL_STATIC OAL_INLINE  oal_uint32 mac_hilink_init_vap(mac_vap_stru *pst_mac_vap)
{
    OAL_MEMZERO(&(pst_mac_vap->st_okc_ssid_hidden_white_list), OAL_SIZEOF(mac_okc_ssid_hidden_white_list_stru));
    oal_spin_lock_init(&(pst_mac_vap->st_okc_ssid_hidden_white_list.st_lock));
    oal_dlist_init_head(&(pst_mac_vap->st_okc_ssid_hidden_white_list.st_list_head));
    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE  oal_uint32 mac_hilink_exit_vap(mac_vap_stru *pst_mac_vap)
{
    mac_okc_white_list_member_stru *pst_white_list;
    oal_dlist_head_stru             *pst_entry = OAL_PTR_NULL;
    oal_dlist_head_stru             *pst_dlist_tmp = OAL_PTR_NULL;

    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_okc_ssid_hidden_white_list.st_list_head))
    {
        pst_white_list = OAL_DLIST_GET_ENTRY(pst_entry, mac_okc_white_list_member_stru, st_dlist);
        oal_dlist_delete_entry(&pst_white_list->st_dlist);
        OAL_MEM_FREE(pst_white_list, OAL_TRUE);
    }

    return OAL_SUCC;
}
#endif



OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_privacyinvoked(mac_vap_stru *pst_mac_vap)
{
  return pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11PrivacyInvoked;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_privacyinvoked(mac_vap_stru *pst_mac_vap,oal_bool_enum_uint8 en_privacyinvoked)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11PrivacyInvoked = en_privacyinvoked;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_rsnaactivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAActivated;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_rsnaactivated(mac_vap_stru *pst_mac_vap,oal_bool_enum_uint8 en_rsnaactivated)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAActivated = en_rsnaactivated;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_is_wep_enabled(mac_vap_stru *pst_mac_vap)
{
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        return OAL_FALSE;
    }

    if ((mac_mib_get_privacyinvoked(pst_mac_vap) == OAL_FALSE)||(mac_mib_get_rsnaactivated(pst_mac_vap) == OAL_TRUE))
    {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_is_wep_allowed(mac_vap_stru *pst_mac_vap)
{
    if (mac_mib_get_rsnaactivated(pst_mac_vap) == OAL_TRUE)
    {
        return OAL_FALSE;
    }
    else
    {
        return mac_is_wep_enabled(pst_mac_vap);
    }
}


OAL_STATIC OAL_INLINE oal_void  mac_set_wep_default_keyid(mac_vap_stru *pst_mac_vap,oal_uint8 uc_default_key_id)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.uc_dot11WEPDefaultKeyID = uc_default_key_id;
}


OAL_STATIC OAL_INLINE oal_uint8  mac_get_wep_default_keyid(mac_vap_stru *pst_mac_vap)
{
    return (pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.uc_dot11WEPDefaultKeyID);
}


OAL_STATIC OAL_INLINE oal_uint8  mac_get_wep_default_keysize(mac_vap_stru *pst_mac_vap)
{
    return (pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.ast_wlan_mib_wep_dflt_key[mac_get_wep_default_keyid(pst_mac_vap)].auc_dot11WEPDefaultKeyValue[WLAN_WEP_SIZE_OFFSET]);
}


OAL_STATIC OAL_INLINE oal_uint8  mac_get_wep_keysize(mac_vap_stru *pst_mac_vap, oal_uint8 uc_idx)
{
    return (pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.ast_wlan_mib_wep_dflt_key[uc_idx].auc_dot11WEPDefaultKeyValue[WLAN_WEP_SIZE_OFFSET]);
}


OAL_STATIC OAL_INLINE wlan_ciper_protocol_type_enum_uint8 mac_get_wep_type(mac_vap_stru *pst_mac_vap, oal_uint8 uc_key_id)
{
    wlan_ciper_protocol_type_enum_uint8 en_cipher_type = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;

    switch(mac_get_wep_keysize(pst_mac_vap,uc_key_id))
    {
        case 40:
            en_cipher_type = WLAN_80211_CIPHER_SUITE_WEP_40;
            break;
        case 104:
            en_cipher_type = WLAN_80211_CIPHER_SUITE_WEP_104;
            break;
		default:
			en_cipher_type = WLAN_80211_CIPHER_SUITE_WEP_40;
			break;
    }
    return en_cipher_type;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_is_tkip_only(mac_vap_stru *pst_mac_vap)
{
    oal_uint8         uc_pair_suites_num;
    oal_uint32        aul_pcip[WLAN_PAIRWISE_CIPHER_SUITES] = {0};

    if(OAL_PTR_NULL == pst_mac_vap)
    {
        return OAL_FALSE;
    }

    if ((OAL_FALSE == pst_mac_vap->st_cap_flag.bit_wpa2) && (OAL_FALSE == pst_mac_vap->st_cap_flag.bit_wpa))
    {
        return OAL_FALSE;
    }

    if ((mac_mib_get_privacyinvoked(pst_mac_vap) == OAL_FALSE)||(mac_mib_get_rsnaactivated(pst_mac_vap) == OAL_FALSE))
    {//�����ܻ�����WEP����ʱ������false
        return OAL_FALSE;
    }

    if (OAL_TRUE == pst_mac_vap->st_cap_flag.bit_wpa)
    {
        uc_pair_suites_num       = mac_mib_get_wpa_pair_suites(pst_mac_vap, aul_pcip);
        if((1 != uc_pair_suites_num) || (MAC_WPA_CHIPER_TKIP != aul_pcip[0]))
        {//wpa����ʱ������tkip only������false
            return OAL_FALSE;
        }
    }

    if (OAL_TRUE == pst_mac_vap->st_cap_flag.bit_wpa2)
    {
        uc_pair_suites_num       = mac_mib_get_rsn_pair_suites(pst_mac_vap, aul_pcip);
        if((1 != uc_pair_suites_num) || (MAC_RSN_CHIPER_TKIP != aul_pcip[0]))
        {//wpa2����ʱ������tkip only������false
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}

#if 0


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_DeauthenticateReason(mac_vap_stru *pst_mac_vap, oal_uint16 us_err_code)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11DeauthenticateReason = (oal_uint32)us_err_code;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_DeauthenticateStation(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_da)
{
    oal_memcopy(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11DeauthenticateStation, puc_da, 6);
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_AuthenticateFailStation(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_da)
{
    oal_memcopy(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11AuthenticateFailStation, puc_da, 6);
}



OAL_STATIC OAL_INLINE oal_void  mac_mib_set_AuthenticateFailStatus(mac_vap_stru *pst_mac_vap, oal_uint16 us_err_code)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11AuthenticateFailStatus = (oal_uint32)us_err_code;
}



OAL_STATIC OAL_INLINE oal_void  mac_mib_set_DisassocStation(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_da)
{
    oal_memcopy(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11DisassociateStation, puc_da, 6);
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_DisassocReason(mac_vap_stru *pst_mac_vap, oal_uint16 us_err_code)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11DisassociateReason = (oal_uint32)us_err_code;
}
#endif


OAL_STATIC OAL_INLINE oal_uint8 *mac_mib_get_StationID(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_StationID(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_sta_id)
{
    oal_set_mac_addr(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID, puc_sta_id);
}


OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_OBSSScanPassiveDwell(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11OBSSScanPassiveDwell;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_OBSSScanPassiveDwell(mac_vap_stru *pst_mac_vap, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11OBSSScanPassiveDwell = ul_val;
}


OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_OBSSScanActiveDwell(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11OBSSScanActiveDwell;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_OBSSScanActiveDwell(mac_vap_stru *pst_mac_vap, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11OBSSScanActiveDwell = ul_val;
}


OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_BSSWidthTriggerScanInterval(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11BSSWidthTriggerScanInterval;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_BSSWidthTriggerScanInterval(mac_vap_stru *pst_mac_vap, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11BSSWidthTriggerScanInterval = ul_val;
}


OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_OBSSScanPassiveTotalPerChannel(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11OBSSScanPassiveTotalPerChannel;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_OBSSScanPassiveTotalPerChannel(mac_vap_stru *pst_mac_vap, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11OBSSScanPassiveTotalPerChannel = ul_val;
}


OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_OBSSScanActiveTotalPerChannel(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11OBSSScanActiveTotalPerChannel;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_OBSSScanActiveTotalPerChannel(mac_vap_stru *pst_mac_vap, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11OBSSScanActiveTotalPerChannel = ul_val;
}


OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_BSSWidthChannelTransitionDelayFactor(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11BSSWidthChannelTransitionDelayFactor;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_BSSWidthChannelTransitionDelayFactor(mac_vap_stru *pst_mac_vap, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11BSSWidthChannelTransitionDelayFactor = ul_val;
}


OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_OBSSScanActivityThreshold(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11OBSSScanActivityThreshold;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_OBSSScanActivityThreshold(mac_vap_stru *pst_mac_vap, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11OBSSScanActivityThreshold = ul_val;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_HighThroughputOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11HighThroughputOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_HighThroughputOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11HighThroughputOptionImplemented = en_val;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_VHTOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTOptionImplemented = en_val;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_VHTOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTOptionImplemented;
}

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)

OAL_STATIC OAL_INLINE oal_void  mac_mib_set_HEOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11HEOptionImplemented = en_val;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_HEOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11HEOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_HTControlFieldSupported(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11HEHTControlFieldSupported = en_val;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_he_HTControlFieldSupported(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11HEHTControlFieldSupported;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_TWTOptionActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11TWTOptionActivated = en_val;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_he_TWTOptionActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11TWTOptionActivated;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_he_OperatingModeIndication(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11OperatingModeIndication;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_OperatingModeIndication(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11OperatingModeIndication = en_val;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_he_BSRSupport(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11BSRSupport;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_BSRSupport(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11BSRSupport = en_val;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_OFDMARandomAccess(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11TOFDMARandomAccess  = en_val;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_he_OFDMARandomAccess(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11TOFDMARandomAccess;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_TriggerMacPaddingDuration(mac_vap_stru *pst_mac_vap, oal_uint8 uc_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.uc_dot11TriggerMacPaddingDuration= uc_val;
}


OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_he_TriggerMacPaddingDuration(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.uc_dot11TriggerMacPaddingDuration;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_MultiBSSIDImplemented(mac_vap_stru *pst_mac_vap, oal_uint8 uc_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11MultiBSSIDImplemented  = uc_val;
}


OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_he_MultiBSSIDImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11MultiBSSIDImplemented;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_MaxAMPDULength(mac_vap_stru *pst_mac_vap,oal_uint8 uc_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.uc_dot11MaxAMPDULength = uc_val;
}


OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_he_MaxAMPDULength(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.uc_dot11MaxAMPDULength;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_he_rx_mcs_map(mac_vap_stru *pst_mac_vap, oal_uint32 ul_he_mcs_map)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.ul_dot11HERxMCSMap = ul_he_mcs_map;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_he_rx_mcs_map(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.ul_dot11HERxMCSMap;
}

OAL_STATIC OAL_INLINE oal_void* mac_mib_get_ptr_he_rx_mcs_map(mac_vap_stru *pst_mac_vap)
{
    return &(pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.ul_dot11HERxMCSMap);
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_he_us_rx_highest_rate(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.ul_dot11HERxHighestDataRateSupported;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_he_us_rx_highest_rate(mac_vap_stru *pst_mac_vap, oal_uint32 ul_rx_highest_rate)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.ul_dot11HERxHighestDataRateSupported = ul_rx_highest_rate;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_he_tx_mcs_map(mac_vap_stru *pst_mac_vap, oal_uint32 ul_he_mcs_map)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.ul_dot11HETxMCSMap = ul_he_mcs_map;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_he_tx_mcs_map(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.ul_dot11HETxMCSMap;
}

OAL_STATIC OAL_INLINE oal_void* mac_mib_get_ptr_he_tx_mcs_map(mac_vap_stru *pst_mac_vap)
{
    return &(pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.ul_dot11HETxMCSMap);
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_he_us_tx_highest_rate(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.ul_dot11HETxHighestDataRateSupported;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_he_us_tx_highest_rate(mac_vap_stru *pst_mac_vap, oal_uint32 ul_tx_highest_rate)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.ul_dot11HETxHighestDataRateSupported = ul_tx_highest_rate;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_he_DualBandSupport(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HEDualBandSupport;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_DualBandSupport(mac_vap_stru *pst_mac_vap, oal_uint8 uc_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HEDualBandSupport = uc_val;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_he_LDPCCodingInPayload(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HELDPCCodingInPayload;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_LDPCCodingInPayload(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HELDPCCodingInPayload = en_val;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_SUBeamformer(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SUBeamformer = en_val;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_he_SUBeamformer(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SUBeamformer;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_SUBeamformee(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SUBeamformee = en_val;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_he_SUBeamformee(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SUBeamformee;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_MUBeamformer(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11MUBeamformer = en_val;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_he_MUBeamformer(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11MUBeamformer;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_SUBeamformingFeedback(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SUBeamformingFeedback = en_val;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_he_SUBeamformingFeedback(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SUBeamformingFeedback;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_MUBeamformingFeedback(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11MUBeamformingFeedback = en_val;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_he_MUBeamformingFeedback(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11MUBeamformingFeedback;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_SRPBaseSR(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SRPBasedSR = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_he_SRPBaseSR(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SRPBasedSR;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_HEChannelWidthSet(mac_vap_stru *pst_mac_vap, oal_uint8 uc_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HEChannelWidthSet= uc_val;
}


OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_he_HEChannelWidthSet(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HEChannelWidthSet;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_HighestNSS(mac_vap_stru *pst_mac_vap, oal_uint8 uc_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HighestNSS = uc_val;
}


OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_he_HighestNSS(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HighestNSS;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_he_HighestMCS(mac_vap_stru *pst_mac_vap, oal_uint8 uc_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HighestMCS= uc_val;
}


OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_he_HighestMCS(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HighestMCS;
}

#endif


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_FortyMHzOperationImplemented(mac_vap_stru *pst_mac_vap)
{
    return (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band) ?
            pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GFortyMHzOperationImplemented :
            pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GFortyMHzOperationImplemented;/* [false alarm]:����ֵ��oal_bool_enum_uint8 �ͺ�������һ��*/
}



OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_2GFortyMHzOperationImplemented(mac_vap_stru *pst_mac_vap)
{
    return (pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GFortyMHzOperationImplemented);
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_5GFortyMHzOperationImplemented(mac_vap_stru *pst_mac_vap)
{
    return (pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GFortyMHzOperationImplemented);
}



OAL_STATIC OAL_INLINE oal_void  mac_mib_set_FortyMHzOperationImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    if (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GFortyMHzOperationImplemented = en_val;
    }
    else
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GFortyMHzOperationImplemented = en_val;
    }
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_2GFortyMHzOperationImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GFortyMHzOperationImplemented = en_val;
}

OAL_STATIC OAL_INLINE oal_void  mac_mib_set_5GFortyMHzOperationImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GFortyMHzOperationImplemented = en_val;
}




OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_SpectrumManagementImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementImplemented;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_SpectrumManagementImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementImplemented = en_val;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_FortyMHzIntolerant(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11FortyMHzIntolerant;
}




OAL_STATIC OAL_INLINE oal_void  mac_mib_set_FortyMHzIntolerant(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11FortyMHzIntolerant = en_val;
}



OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_2040BSSCoexistenceManagementSupport(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot112040BSSCoexistenceManagementSupport;
}

#if 0

OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_dot11RSNAProtectedManagementFramesActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RSNAProtectedManagementFramesActivated;
}
#endif

OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_dot11RSNAMFPC(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPC;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_dot11RSNAMFPR(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPR;
}
#if 0

OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11RSNAProtectedManagementFramesActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RSNAProtectedManagementFramesActivated = ul_val;
}
#endif

OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11RSNAMFPC(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 ul_val)
{
   pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPC = ul_val;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11RSNAMFPR(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPR = ul_val;
}


OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_dot11AssociationSAQueryMaximumTimeout(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11AssociationSAQueryMaximumTimeout;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_dot11AssociationSAQueryRetryTimeout(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11AssociationSAQueryRetryTimeout;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11AssociationSAQueryMaximumTimeout(mac_vap_stru *pst_mac_vap, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11AssociationSAQueryMaximumTimeout = ul_val;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11AssociationSAQueryRetryTimeout(mac_vap_stru *pst_mac_vap, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11AssociationSAQueryRetryTimeout = ul_val;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11ExtendedChannelSwitchActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11ExtendedChannelSwitchActivated = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11ExtendedChannelSwitchActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11ExtendedChannelSwitchActivated;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11RadioMeasurementActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RadioMeasurementActivated = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RadioMeasurementActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RadioMeasurementActivated;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11QBSSLoadImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QBSSLoadImplemented = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11QBSSLoadImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QBSSLoadImplemented;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11APSDOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11APSDOptionImplemented = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11APSDOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11APSDOptionImplemented;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11DelayedBlockAckOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DelayedBlockAckOptionImplemented = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11DelayedBlockAckOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DelayedBlockAckOptionImplemented;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11ImmediateBlockAckOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11ImmediateBlockAckOptionImplemented = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11ImmediateBlockAckOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11ImmediateBlockAckOptionImplemented;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11QosOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QosOptionImplemented = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11QosOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QosOptionImplemented;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11MultiDomainCapabilityActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11MultiDomainCapabilityActivated = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11MultiDomainCapabilityActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11MultiDomainCapabilityActivated;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11AssociationResponseTimeOut(mac_vap_stru *pst_mac_vap, oal_uint32 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11AssociationResponseTimeOut = en_val;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_dot11AssociationResponseTimeOut(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11AssociationResponseTimeOut;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_MaxAssocUserNums(mac_vap_stru *pst_mac_vap, oal_uint16 us_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.us_dot11MaxAssocUserNums = us_val;
}

OAL_STATIC OAL_INLINE oal_uint16 mac_mib_get_MaxAssocUserNums(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.us_dot11MaxAssocUserNums;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_SupportRateSetNums(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11SupportRateSetNums = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_SupportRateSetNums(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11SupportRateSetNums;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_CfgAmsduTxAtive(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11CfgAmsduTxAtive = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_CfgAmsduTxAtive(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11CfgAmsduTxAtive;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_AmsduAggregateAtive(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AmsduAggregateAtive = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_AmsduAggregateAtive(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AmsduAggregateAtive;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_AmsduPlusAmpduActive(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AmsduPlusAmpduActive = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_AmsduPlusAmpduActive(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AmsduPlusAmpduActive;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_WPSActive(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11WPSActive = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_WPSActive(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11WPSActive;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_2040SwitchProhibited(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot112040SwitchProhibited = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_2040SwitchProhibited(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot112040SwitchProhibited;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_TxAggregateActived(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11TxAggregateActived = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_TxAggregateActived(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11TxAggregateActived;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_CfgAmpduTxAtive(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11CfgAmpduTxAtive = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_CfgAmpduTxAtive(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11CfgAmpduTxAtive;
}

#ifdef _PRE_WLAN_FEATURE_AMPDU_VAP
OAL_STATIC OAL_INLINE oal_void mac_mib_set_RxBASessionNumber(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11RxBASessionNumber = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_RxBASessionNumber(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11RxBASessionNumber;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_incr_RxBASessionNumber(mac_vap_stru *pst_mac_vap)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11RxBASessionNumber ++;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_decr_RxBASessionNumber(mac_vap_stru *pst_mac_vap)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11RxBASessionNumber --;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_TxBASessionNumber(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11TxBASessionNumber = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_TxBASessionNumber(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11TxBASessionNumber;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_incr_TxBASessionNumber(mac_vap_stru *pst_mac_vap)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11TxBASessionNumber ++;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_decr_TxBASessionNumber(mac_vap_stru *pst_mac_vap)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11TxBASessionNumber --;
}

#endif

OAL_STATIC OAL_INLINE oal_void mac_mib_set_VAPClassifyTidNo(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11VAPClassifyTidNo = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_VAPClassifyTidNo(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11VAPClassifyTidNo;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_AuthenticationMode(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AuthenticationMode = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_AuthenticationMode(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AuthenticationMode;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_AddBaMode(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AddBaMode = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_AddBaMode(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AddBaMode;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_80211iMode(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot1180211iMode = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_80211iMode(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot1180211iMode;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_TxTrafficClassifyFlag(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11TxTrafficClassifyFlag = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_TxTrafficClassifyFlag(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11TxTrafficClassifyFlag;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_StaAuthCount(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAuthCount = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_StaAuthCount(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAuthCount;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_StaAssocCount(mac_vap_stru *pst_mac_vap, oal_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAssocCount = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_StaAssocCount(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAssocCount;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_incr_StaAuthCount(mac_vap_stru *pst_mac_vap)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAuthCount++;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_incr_StaAssocCount(mac_vap_stru *pst_mac_vap)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAssocCount++;
}



#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11K_EXTERN)

OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11RMBeaconTableMeasurementActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMBeaconTableMeasurementActivated = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RMBeaconTableMeasurementActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMBeaconTableMeasurementActivated;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11RMBeaconActiveMeasurementActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMBeaconActiveMeasurementActivated = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RMBeaconActiveMeasurementActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMBeaconActiveMeasurementActivated;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11RMBeaconPassiveMeasurementActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMBeaconPassiveMeasurementActivated = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RMBeaconPassiveMeasurementActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMBeaconPassiveMeasurementActivated;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11RMLinkMeasurementActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMLinkMeasurementActivated = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RMLinkMeasurementActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMLinkMeasurementActivated;
}

#ifdef _PRE_WLAN_FEATURE_11K_EXTERN

OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11RMChannelLoadMeasurementActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMChannelLoadMeasurementActivated = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RMChannelLoadMeasurementActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMChannelLoadMeasurementActivated;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_dot11RMNeighborReportActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMNeighborReportActivated= en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RMNeighborReportActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMNeighborReportActivated;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RMParallelMeasurementsActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMParallelMeasurementsActivated;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RMRepeatedMeasurementsActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMRepeatedMeasurementsActivated;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RMBeaconMeasurementReportingConditionsActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMBeaconMeasurementReportingConditionsActivated;
}
#endif
#endif

OAL_STATIC OAL_INLINE oal_void  mac_mib_set_2040BSSCoexistenceManagementSupport(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot112040BSSCoexistenceManagementSupport = en_val;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_dot11dtimperiod(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11DTIMPeriod;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_dot11VapMaxBandWidth(mac_vap_stru *pst_mac_vap, wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11VapMaxBandWidth = en_bandwidth;
}


OAL_STATIC OAL_INLINE wlan_channel_bandwidth_enum_uint8 mac_mib_get_dot11VapMaxBandWidth(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11VapMaxBandWidth;
}

OAL_STATIC OAL_INLINE oal_void  mac_mib_set_BeaconPeriod(mac_vap_stru *pst_mac_vap, oal_uint32 ul_val)
{
    if (ul_val != 0)
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11BeaconPeriod = ul_val;
    }
}
OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_BeaconPeriod(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11BeaconPeriod;
}

OAL_STATIC OAL_INLINE oal_void  mac_mib_set_DesiredBSSType(mac_vap_stru *pst_mac_vap, oal_uint8 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DesiredBSSType = ul_val;
}
OAL_STATIC OAL_INLINE oal_uint8 mac_mib_get_DesiredBSSType(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DesiredBSSType;
}


OAL_STATIC OAL_INLINE oal_uint8* mac_mib_get_DesiredSSID(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11DesiredSSID;
}

OAL_STATIC OAL_INLINE oal_void  mac_mib_set_AuthenticationResponseTimeOut(mac_vap_stru *pst_mac_vap, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11AuthenticationResponseTimeOut = ul_val;
}
OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_AuthenticationResponseTimeOut(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11AuthenticationResponseTimeOut;
}

OAL_STATIC OAL_INLINE oal_uint8* mac_mib_get_p2p0_dot11StationID(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID;
}

#if defined(_PRE_WLAN_FEATURE_11V) || (defined(_PRE_WLAN_FEATURE_FTM)) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_MgmtOptionBSSTransitionActivated(mac_vap_stru * pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11MgmtOptionBSSTransitionActivated;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_MgmtOptionBSSTransitionActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11MgmtOptionBSSTransitionActivated = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_MgmtOptionBSSTransitionImplemented(mac_vap_stru * pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11MgmtOptionBSSTransitionImplemented;
}

OAL_STATIC OAL_INLINE oal_void  mac_mib_set_MgmtOptionBSSTransitionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11MgmtOptionBSSTransitionImplemented= en_val;
}
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_WirelessManagementImplemented(mac_vap_stru * pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11WirelessManagementImplemented;
}

OAL_STATIC OAL_INLINE oal_void  mac_mib_set_WirelessManagementImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11WirelessManagementImplemented= en_val;
}
#endif	// _PRE_WLAN_FEATURE_11V

#if defined(_PRE_WLAN_FEATURE_11V) || (defined(_PRE_WLAN_FEATURE_FTM)) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
OAL_STATIC OAL_INLINE oal_void  mac_mib_set_FineTimingMsmtInitActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11FineTimingMsmtInitActivated = en_val;
}
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_FineTimingMsmtInitActivated(mac_vap_stru * pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11FineTimingMsmtInitActivated;
}
OAL_STATIC OAL_INLINE oal_void  mac_mib_set_FineTimingMsmtRespActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11FineTimingMsmtRespActivated = en_val;
}
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_FineTimingMsmtRespActivated(mac_vap_stru * pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11FineTimingMsmtRespActivated;
}
OAL_STATIC OAL_INLINE oal_void  mac_mib_set_FineTimingMsmtRangeRepActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11RMFineTimingMsmtRangeRepActivated = en_val;
}
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_FineTimingMsmtRangeRepActivated(mac_vap_stru * pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11RMFineTimingMsmtRangeRepActivated;
}
#endif

#ifdef _PRE_WLAN_FEATURE_FTM

OAL_STATIC OAL_INLINE mac_ftm_mode_enum_uint8 mac_check_ftm_enable(mac_vap_stru *pst_mac_vap)
{
    if ((OAL_FALSE == mac_mib_get_FineTimingMsmtInitActivated(pst_mac_vap))
        &&(OAL_FALSE == mac_mib_get_FineTimingMsmtRespActivated(pst_mac_vap)))
    {
        return MAC_FTM_DISABLE_MODE;
    }
    else if((OAL_FALSE == mac_mib_get_FineTimingMsmtInitActivated(pst_mac_vap))
        &&(OAL_TRUE == mac_mib_get_FineTimingMsmtRespActivated(pst_mac_vap)))
    {
        return MAC_FTM_RESPONDER_MODE;
    }
    else if((OAL_TRUE == mac_mib_get_FineTimingMsmtInitActivated(pst_mac_vap))
        &&(OAL_FALSE == mac_mib_get_FineTimingMsmtRespActivated(pst_mac_vap)))
    {
        return MAC_FTM_INITIATOR_MODE;
    }
    else
    {
        return MAC_FTM_MIX_MODE;
    }
}

#endif


OAL_STATIC OAL_INLINE oal_void  mac_mib_init_2040(mac_vap_stru *pst_mac_vap)
{
    mac_mib_set_FortyMHzIntolerant(pst_mac_vap, OAL_FALSE);
    mac_mib_set_SpectrumManagementImplemented(pst_mac_vap, OAL_TRUE);
    mac_mib_set_2040BSSCoexistenceManagementSupport(pst_mac_vap, OAL_TRUE);
}


OAL_STATIC OAL_INLINE oal_void mac_mib_init_obss_scan(mac_vap_stru *pst_mac_vap)
{
    mac_mib_set_OBSSScanPassiveDwell(pst_mac_vap, 20);
    mac_mib_set_OBSSScanActiveDwell(pst_mac_vap, 10);
    mac_mib_set_BSSWidthTriggerScanInterval(pst_mac_vap, 300);
    mac_mib_set_OBSSScanPassiveTotalPerChannel(pst_mac_vap, 200);
    mac_mib_set_OBSSScanActiveTotalPerChannel(pst_mac_vap, 20);
    mac_mib_set_BSSWidthChannelTransitionDelayFactor(pst_mac_vap, 5);
    mac_mib_set_OBSSScanActivityThreshold(pst_mac_vap, 25);
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_dot11dtimperiod(mac_vap_stru *pst_mac_vap, oal_uint32 ul_val)
{
    if (ul_val != 0)
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11DTIMPeriod = ul_val;
    }
}


OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_powermanagementmode(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11PowerManagementMode;
}


OAL_STATIC OAL_INLINE oal_void  mac_mib_set_powermanagementmode(mac_vap_stru *pst_mac_vap, oal_uint8 uc_val)
{
    if (uc_val != 0)
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11PowerManagementMode = uc_val;
    }
}

#if defined _PRE_WLAN_FEATURE_OPMODE_NOTIFY || (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION)

OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_OperatingModeNotificationImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11OperatingModeNotificationImplemented;
}

OAL_STATIC OAL_INLINE oal_void  mac_mib_set_OperatingModeNotificationImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11OperatingModeNotificationImplemented = en_val;
}
#endif



OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_LsigTxopFullProtectionActivated(mac_vap_stru *pst_mac_vap)
{
    return (pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11LSIGTXOPFullProtectionActivated);
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_LsigTxopFullProtectionActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_lsig_txop_full_protection_activated)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11LSIGTXOPFullProtectionActivated = en_lsig_txop_full_protection_activated;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_NonGFEntitiesPresent(mac_vap_stru *pst_mac_vap)
{
    return (pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11NonGFEntitiesPresent);
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_NonGFEntitiesPresent(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_non_gf_entities_present)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11NonGFEntitiesPresent = en_non_gf_entities_present;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_RifsMode(mac_vap_stru *pst_mac_vap)
{
    return (pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11RIFSMode);
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_RifsMode(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_rifs_mode)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11RIFSMode = en_rifs_mode;
}


OAL_STATIC OAL_INLINE wlan_mib_ht_protection_enum_uint8 mac_mib_get_HtProtection(mac_vap_stru *pst_mac_vap)
{
    return (pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11HTProtection);
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_DualCTSProtection(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11DualCTSProtection = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_DualCTSProtection(mac_vap_stru *pst_mac_vap)
{
    return (pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11DualCTSProtection);
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_PCOActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11PCOActivated = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_PCOActivated(mac_vap_stru *pst_mac_vap)
{
    return (pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11PCOActivated);
}




OAL_STATIC OAL_INLINE oal_void mac_mib_set_HtProtection(mac_vap_stru *pst_mac_vap, wlan_mib_ht_protection_enum_uint8 en_ht_protection)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11HTProtection = en_ht_protection;
}




OAL_STATIC OAL_INLINE oal_void mac_mib_set_SpectrumManagementRequired(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementRequired = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11SpectrumManagementRequired(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementRequired;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ShortGIOptionInFortyImplemented(mac_vap_stru *pst_mac_vap)
{
    return (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band) ?
            pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GShortGIOptionInFortyImplemented :
            pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GShortGIOptionInFortyImplemented;/* [false alarm]:����ֵ��oal_bool_enum_uint8 �ͺ�������һ��*/
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_ShortGIOptionInFortyImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    if (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GShortGIOptionInFortyImplemented = en_val;
    }
    else
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GShortGIOptionInFortyImplemented = en_val;
    }
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_2GShortGIOptionInFortyImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GShortGIOptionInFortyImplemented = en_val;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_5GShortGIOptionInFortyImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GShortGIOptionInFortyImplemented = en_val;
}



OAL_STATIC OAL_INLINE oal_void mac_mib_set_FragmentationThreshold(mac_vap_stru *pst_mac_vap, oal_uint32 ul_frag_threshold)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11FragmentationThreshold = ul_frag_threshold;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_FragmentationThreshold(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11FragmentationThreshold;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_RTSThreshold(mac_vap_stru *pst_mac_vap, oal_uint32 ul_rts_threshold)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11RTSThreshold = ul_rts_threshold;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_RTSThreshold(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_operation.ul_dot11RTSThreshold;
}





OAL_STATIC OAL_INLINE oal_void mac_mib_set_AntennaSelectionOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11AntennaSelectionOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_AntennaSelectionOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11AntennaSelectionOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_TransmitExplicitCSIFeedbackASOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitExplicitCSIFeedbackASOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_TransmitExplicitCSIFeedbackASOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitExplicitCSIFeedbackASOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_TransmitIndicesFeedbackASOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitIndicesFeedbackASOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_TransmitIndicesFeedbackASOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitIndicesFeedbackASOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_ExplicitCSIFeedbackASOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ExplicitCSIFeedbackASOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ExplicitCSIFeedbackASOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ExplicitCSIFeedbackASOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_ReceiveAntennaSelectionOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ReceiveAntennaSelectionOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ReceiveAntennaSelectionOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ReceiveAntennaSelectionOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_TransmitSoundingPPDUOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitSoundingPPDUOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_TransmitSoundingPPDUOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitSoundingPPDUOptionImplemented;
}





OAL_STATIC OAL_INLINE wlan_11b_mib_preamble_enum_uint8 mac_mib_get_ShortPreambleOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return (pst_mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11ShortPreambleOptionImplemented);
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_ShortPreambleOptionImplemented(mac_vap_stru *pst_mac_vap, wlan_11b_mib_preamble_enum_uint8 en_preamble)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11ShortPreambleOptionImplemented = en_preamble;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_PBCCOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11PBCCOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_PBCCOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11PBCCOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_ChannelAgilityPresent(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11ChannelAgilityPresent  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ChannelAgilityPresent(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11ChannelAgilityPresent;
}





OAL_STATIC OAL_INLINE oal_void mac_mib_set_DSSSOFDMOptionActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11DSSSOFDMOptionActivated  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_DSSSOFDMOptionActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11DSSSOFDMOptionActivated;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_ShortSlotTimeOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11ShortSlotTimeOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ShortSlotTimeOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11ShortSlotTimeOptionImplemented;
}



OAL_STATIC OAL_INLINE oal_void mac_mib_set_ShortSlotTimeOptionActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11ShortSlotTimeOptionActivated  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ShortSlotTimeOptionActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11ShortSlotTimeOptionActivated;
}





OAL_STATIC OAL_INLINE oal_void mac_mib_set_ReceiveStaggerSoundingOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ReceiveStaggerSoundingOptionImplemented = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ReceiveStaggerSoundingOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ReceiveStaggerSoundingOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_TransmitStaggerSoundingOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11TransmitStaggerSoundingOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_TransmitStaggerSoundingOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11TransmitStaggerSoundingOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_ReceiveNDPOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ReceiveNDPOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ReceiveNDPOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ReceiveNDPOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_TransmitNDPOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11TransmitNDPOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_TransmitNDPOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11TransmitNDPOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_ImplicitTransmitBeamformingOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ImplicitTransmitBeamformingOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ImplicitTransmitBeamformingOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ImplicitTransmitBeamformingOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_CalibrationOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11CalibrationOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_CalibrationOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11CalibrationOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_ExplicitCSITransmitBeamformingOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ExplicitCSITransmitBeamformingOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ExplicitCSITransmitBeamformingOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ExplicitCSITransmitBeamformingOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_ExplicitNonCompressedBeamformingMatrixOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ExplicitNonCompressedBeamformingMatrixOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ExplicitNonCompressedBeamformingMatrixOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ExplicitNonCompressedBeamformingMatrixOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_ExplicitTransmitBeamformingCSIFeedbackOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11ExplicitTransmitBeamformingCSIFeedbackOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ExplicitTransmitBeamformingCSIFeedbackOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11ExplicitTransmitBeamformingCSIFeedbackOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_ExplicitNonCompressedBeamformingFeedbackOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11ExplicitNonCompressedBeamformingFeedbackOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ExplicitNonCompressedBeamformingFeedbackOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11ExplicitNonCompressedBeamformingFeedbackOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11ExplicitCompressedBeamformingFeedbackOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ExplicitCompressedBeamformingFeedbackOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11ExplicitCompressedBeamformingFeedbackOptionImplemented;
}



OAL_STATIC OAL_INLINE oal_void mac_mib_set_NumberBeamFormingCSISupportAntenna(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11NumberBeamFormingCSISupportAntenna  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_NumberBeamFormingCSISupportAntenna(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11NumberBeamFormingCSISupportAntenna;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_NumberNonCompressedBeamformingMatrixSupportAntenna(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11NumberNonCompressedBeamformingMatrixSupportAntenna  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_NumberNonCompressedBeamformingMatrixSupportAntenna(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11NumberNonCompressedBeamformingMatrixSupportAntenna;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11NumberCompressedBeamformingMatrixSupportAntenna  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_NumberCompressedBeamformingMatrixSupportAntenna(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11NumberCompressedBeamformingMatrixSupportAntenna;
}





OAL_STATIC OAL_INLINE oal_void mac_mib_set_VHTSUBeamformeeOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformeeOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_VHTSUBeamformeeOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformeeOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_VHTSUBeamformerOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformerOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_VHTSUBeamformerOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformerOptionImplemented;
}



OAL_STATIC OAL_INLINE oal_void mac_mib_set_VHTMUBeamformeeOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformeeOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_VHTMUBeamformeeOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformeeOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_VHTMUBeamformerOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformerOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_VHTMUBeamformerOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformerOptionImplemented;
}



OAL_STATIC OAL_INLINE oal_void mac_mib_set_VHTNumberSoundingDimensions(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.uc_dot11VHTNumberSoundingDimensions  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_VHTNumberSoundingDimensions(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.uc_dot11VHTNumberSoundingDimensions;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_VHTBeamformeeNTxSupport(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.uc_dot11VHTBeamformeeNTxSupport  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_VHTBeamformeeNTxSupport(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.uc_dot11VHTBeamformeeNTxSupport;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_VHTChannelWidthOptionImplemented(mac_vap_stru *pst_mac_vap, wlan_mib_vht_supp_width_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTChannelWidthOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE wlan_mib_vht_supp_width_enum_uint8  mac_mib_get_VHTChannelWidthOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTChannelWidthOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_VHTShortGIOptionIn80Implemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_VHTShortGIOptionIn80Implemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn160and80p80Implemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_VHTShortGIOptionIn160and80p80Implemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn160and80p80Implemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_VHTLDPCCodingOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTLDPCCodingOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_VHTLDPCCodingOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTLDPCCodingOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_VHTTxSTBCOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTTxSTBCOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_VHTTxSTBCOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTTxSTBCOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_VHTRxSTBCOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTRxSTBCOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_VHTRxSTBCOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTRxSTBCOptionImplemented;
}



OAL_STATIC OAL_INLINE oal_void mac_mib_set_HTGreenfieldOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11HTGreenfieldOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_HTGreenfieldOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11HTGreenfieldOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_ShortGIOptionInTwentyImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11ShortGIOptionInTwentyImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_ShortGIOptionInTwentyImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11ShortGIOptionInTwentyImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_LDPCCodingOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11LDPCCodingOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_LDPCCodingOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11LDPCCodingOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_LDPCCodingOptionActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11LDPCCodingOptionActivated  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_LDPCCodingOptionActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11LDPCCodingOptionActivated;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_TxSTBCOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxSTBCOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_TxSTBCOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxSTBCOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_TxSTBCOptionActivated(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxSTBCOptionActivated  = en_val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_TxSTBCOptionActivated(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxSTBCOptionActivated;
}



OAL_STATIC OAL_INLINE oal_void mac_mib_set_RxSTBCOptionImplemented(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11RxSTBCOptionImplemented  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_RxSTBCOptionImplemented(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11RxSTBCOptionImplemented;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_HighestSupportedDataRate(mac_vap_stru *pst_mac_vap, oal_uint32 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.ul_dot11HighestSupportedDataRate  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_HighestSupportedDataRate(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.ul_dot11HighestSupportedDataRate;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_TxMCSSetDefined(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxMCSSetDefined  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_TxMCSSetDefined(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxMCSSetDefined;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_TxRxMCSSetNotEqual(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxRxMCSSetNotEqual  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_TxRxMCSSetNotEqual(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxRxMCSSetNotEqual;
}







OAL_STATIC OAL_INLINE oal_void mac_mib_set_TxMaximumNumberSpatialStreamsSupported(mac_vap_stru *pst_mac_vap, oal_uint32 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.ul_dot11TxMaximumNumberSpatialStreamsSupported  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint32 mac_mib_get_TxMaximumNumberSpatialStreamsSupported(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.ul_dot11TxMaximumNumberSpatialStreamsSupported;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_TxUnequalModulationSupported(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxUnequalModulationSupported  = en_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_TxUnequalModulationSupported(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxUnequalModulationSupported;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_SupportedMCSTxValue(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index,oal_uint8 uc_val)
{
    pst_mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcstx.auc_dot11SupportedMCSTxValue[uc_index]  = uc_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_SupportedMCSTxValue(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcstx.auc_dot11SupportedMCSTxValue[uc_index];
}

OAL_STATIC OAL_INLINE oal_uint8*  mac_mib_get_SupportedMCSTx(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcstx.auc_dot11SupportedMCSTxValue;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_SupportedMCSRxValue(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index,oal_uint8 uc_val)
{
    pst_mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcsrx.auc_dot11SupportedMCSRxValue[uc_index]  = uc_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_SupportedMCSRxValue(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcsrx.auc_dot11SupportedMCSRxValue[uc_index];
}

OAL_STATIC OAL_INLINE oal_uint8*  mac_mib_get_SupportedMCSRx(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcsrx.auc_dot11SupportedMCSRxValue;
}


OAL_STATIC OAL_INLINE oal_void mac_mib_set_EDCATableIndex(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[uc_index].ul_dot11EDCATableIndex  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_EDCATableIndex(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[uc_index].ul_dot11EDCATableIndex;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_EDCATableCWmin(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[uc_index].ul_dot11EDCATableCWmin  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_EDCATableCWmin(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[uc_index].ul_dot11EDCATableCWmin;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_EDCATableCWmax(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[uc_index].ul_dot11EDCATableCWmax  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_EDCATableCWmax(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[uc_index].ul_dot11EDCATableCWmax;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_EDCATableAIFSN(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[uc_index].ul_dot11EDCATableAIFSN  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_EDCATableAIFSN(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[uc_index].ul_dot11EDCATableAIFSN;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_EDCATableTXOPLimit(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[uc_index].ul_dot11EDCATableTXOPLimit  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_EDCATableTXOPLimit(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[uc_index].ul_dot11EDCATableTXOPLimit;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_EDCATableMandatory(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index, oal_uint8 uc_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[uc_index].en_dot11EDCATableMandatory  = uc_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_EDCATableMandatory(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[uc_index].en_dot11EDCATableMandatory;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_QAPEDCATableIndex(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableIndex  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_QAPEDCATableIndex(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableIndex;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_QAPEDCATableCWmin(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableCWmin  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_QAPEDCATableCWmin(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableCWmin;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_QAPEDCATableCWmax(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableCWmax  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_QAPEDCATableCWmax(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableCWmax;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_QAPEDCATableAIFSN(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableAIFSN  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_QAPEDCATableAIFSN(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableAIFSN;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_QAPEDCATableTXOPLimit(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableTXOPLimit  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_QAPEDCATableTXOPLimit(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableTXOPLimit;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_QAPEDCATableMandatory(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index, oal_uint8 uc_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[uc_index].en_dot11QAPEDCATableMandatory  = uc_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_QAPEDCATableMandatory(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[uc_index].en_dot11QAPEDCATableMandatory;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_QAPEDCATableMSDULifetime(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableMSDULifetime  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_QAPEDCATableMSDULifetime(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableMSDULifetime;
}


#ifdef _PRE_WLAN_FEATURE_11AX
OAL_STATIC OAL_INLINE oal_void mac_mib_set_QAPMUEDCATableIndex(mac_vap_rom_stru *pst_mac_vap_rom, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap_rom->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableIndex = ul_val;
}

//OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_QAPMUEDCATableIndex(mac_vap_stru *pst_mac_vap, oal_uint8 uc_index)
//{
    //return pst_mac_vap->pst_mib_info->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableIndex;
//}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_QAPMUEDCATableCWmin(mac_vap_rom_stru *pst_mac_vap_rom, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap_rom->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableCWmin  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_QAPMUEDCATableCWmin(mac_vap_rom_stru *pst_mac_vap_rom, oal_uint8 uc_index)
{
    return pst_mac_vap_rom->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableCWmin;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_QAPMUEDCATableCWmax(mac_vap_rom_stru *pst_mac_vap_rom, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap_rom->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableCWmax  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_QAPMUEDCATableCWmax(mac_vap_rom_stru *pst_mac_vap_rom, oal_uint8 uc_index)
{
    return pst_mac_vap_rom->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableCWmax;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_QAPMUEDCATableAIFSN(mac_vap_rom_stru *pst_mac_vap_rom, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap_rom->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableAIFSN  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_QAPMUEDCATableAIFSN(mac_vap_rom_stru *pst_mac_vap_rom, oal_uint8 uc_index)
{
    return pst_mac_vap_rom->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableAIFSN;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_QAPMUEDCATableTXOPLimit(mac_vap_rom_stru *pst_mac_vap_rom, oal_uint8 uc_index, oal_uint32 ul_val)
{
    pst_mac_vap_rom->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableTXOPLimit  = ul_val;
}

OAL_STATIC OAL_INLINE oal_uint32  mac_mib_get_QAPMUEDCATableTXOPLimit(mac_vap_rom_stru *pst_mac_vap_rom, oal_uint8 uc_index)
{
    return pst_mac_vap_rom->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[uc_index].ul_dot11QAPEDCATableTXOPLimit;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_QAPMUEDCATableMandatory(mac_vap_rom_stru *pst_mac_vap_rom, oal_uint8 uc_index, oal_uint8 uc_val)
{
    pst_mac_vap_rom->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[uc_index].en_dot11QAPEDCATableMandatory  = uc_val;
}

OAL_STATIC OAL_INLINE oal_uint8  mac_mib_get_QAPMUEDCATableMandatory(mac_vap_rom_stru *pst_mac_vap_rom, oal_uint8 uc_index)
{
    return pst_mac_vap_rom->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[uc_index].en_dot11QAPEDCATableMandatory;
}

OAL_STATIC OAL_INLINE oal_void mac_mib_set_PPEThresholdsRequired(mac_vap_rom_stru *pst_mac_vap_rom, oal_bool_enum_uint8 en_value)
{
    pst_mac_vap_rom->st_wlan_rom_mib_he_phy_config.en_dot11PPEThresholdsRequired = en_value;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_mib_get_PPEThresholdsRequired(mac_vap_rom_stru *pst_mac_vap_rom)
{
    return pst_mac_vap_rom->st_wlan_rom_mib_he_phy_config.en_dot11PPEThresholdsRequired;
}

#endif



/*****************************************************************************
  10 ��������
*****************************************************************************/
extern oal_uint32 mac_vap_init_etc(
                    mac_vap_stru               *pst_vap,
                    oal_uint8                   uc_chip_id,
                    oal_uint8                   uc_device_id,
                    oal_uint8                   uc_vap_id,
                    mac_cfg_add_vap_param_stru *pst_param);
extern oal_void mac_vap_init_rates_etc(mac_vap_stru *pst_vap);
extern oal_void mac_sta_init_bss_rates_etc(mac_vap_stru *pst_vap, oal_void *pst_bss_dscr);
extern oal_void mac_vap_init_rates_by_protocol_etc(mac_vap_stru *pst_vap, wlan_protocol_enum_uint8 en_vap_protocol, mac_data_rate_stru *pst_rates);
extern oal_uint32 mac_vap_exit_etc(mac_vap_stru *pst_vap);
extern oal_uint32  mac_vap_del_user_etc(mac_vap_stru *pst_vap, oal_uint16 us_user_idx);
extern oal_uint32  mac_vap_find_user_by_macaddr_etc(
                mac_vap_stru        *pst_vap,
                oal_uint8           *puc_sta_mac_addr,
                oal_uint16          *pus_user_idx);
extern oal_uint32  mac_device_find_user_by_macaddr_etc(
                oal_uint8           uc_device_id,
                oal_uint8           *puc_sta_mac_addr,
                oal_uint16          *pus_user_idx);
extern oal_uint32  mac_chip_find_user_by_macaddr(
                oal_uint8           uc_chip_id,
                oal_uint8           *puc_sta_mac_addr,
                oal_uint16          *pus_user_idx);
extern oal_uint32  mac_board_find_user_by_macaddr(
                oal_uint8           *puc_sta_mac_addr,
                oal_uint16          *pus_user_idx);
#if 0
extern oal_uint32  mac_vap_update_user(mac_vap_stru *pst_vap, oal_uint32   ul_user_dix);
#endif
extern oal_uint32  mac_vap_add_assoc_user_etc(mac_vap_stru *pst_vap, oal_uint16 us_user_idx);

extern oal_uint8 mac_vap_get_bandwith(wlan_bw_cap_enum_uint8 en_dev_cap, wlan_channel_bandwidth_enum_uint8 en_bss_cap);


OAL_STATIC  OAL_INLINE  hal_fcs_protect_type_enum_uint8 mac_fcs_get_protect_type(
                        mac_vap_stru *pst_mac_vap)
{
    hal_fcs_protect_type_enum_uint8 en_protect_type;

    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        en_protect_type = HAL_FCS_PROTECT_TYPE_SELF_CTS;
    }
    else if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        en_protect_type = HAL_FCS_PROTECT_TYPE_NULL_DATA;
    }
    else
    {
        en_protect_type = HAL_FCS_PROTECT_TYPE_NONE;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    if (WLAN_P2P_GO_MODE == pst_mac_vap->en_p2p_mode)
    {
        en_protect_type = HAL_FCS_PROTECT_TYPE_NONE;
    }
#endif

    return en_protect_type;
}


OAL_STATIC  OAL_INLINE oal_uint8  mac_fcs_get_protect_cnt(mac_vap_stru *pst_mac_vap)
{
    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        return HAL_FCS_PROTECT_CNT_1;
    }

    /* staģʽ��Ҫ��Ӳ����ʱ�����one pkt���ͣ��������Ծ����������ã�btcoex��ͨ��������Ҫ���ø���ʱ�� */
    return HAL_FCS_PROTECT_CNT_20;
}

/*****************************************************************************
    mib��������
*****************************************************************************/
extern oal_uint32  mac_mib_get_beacon_period_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_len, oal_uint8 *puc_param);
extern oal_uint32  mac_mib_get_dtim_period_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_len, oal_uint8 *puc_param);
extern oal_uint32  mac_mib_get_bss_type_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_len, oal_uint8 *puc_param);
#if 0
extern oal_uint32  mac_mib_get_GroupReceivedFrameCount(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_len, oal_uint8 *puc_group_count);
#endif
extern oal_uint32  mac_mib_get_ssid_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_len, oal_uint8 *puc_param);
#if 0
extern oal_uint32  mac_mib_get_station_id(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_len, oal_uint8 *puc_param);
#endif
extern oal_uint32  mac_mib_set_beacon_period_etc(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  mac_mib_set_dtim_period_etc(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  mac_mib_set_bss_type_etc(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#if 0
extern oal_uint32  mac_mib_set_GroupReceivedFrameCount(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_group_count);
#endif
extern oal_uint32  mac_mib_set_ssid_etc(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  mac_mib_set_station_id_etc(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#if 0
extern oal_uint32  mac_vap_get_user_wme_info(
                       mac_vap_stru               *pst_vap,
                       wlan_wme_ac_type_enum_uint8 en_wme_type,
                       mac_wme_param_stru         *pst_wme_info);
extern oal_uint32  mac_vap_set_user_wme_info(
                       mac_vap_stru               *pst_vap,
                       wlan_wme_ac_type_enum_uint8 en_wme_type,
                       mac_wme_param_stru         *pst_wme_info);
#endif
extern oal_uint32  mac_mib_get_shpreamble_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_len, oal_uint8 *puc_param);
extern oal_uint32  mac_mib_set_shpreamble_etc(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 mac_vap_set_bssid_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_bssid);
extern oal_uint32 mac_vap_set_current_channel_etc(mac_vap_stru *pst_vap,wlan_channel_band_enum_uint8 en_band, oal_uint8 uc_channel);
extern oal_void  mac_vap_state_change_etc(mac_vap_stru *pst_mac_vap, mac_vap_state_enum_uint8 en_vap_state);
extern oal_uint32 mac_vap_config_vht_ht_mib_by_protocol_etc(mac_vap_stru *pst_mac_vap);
extern oal_uint32 mac_vap_init_wme_param_etc(mac_vap_stru *pst_mac_vap);
#ifdef _PRE_WLAN_FEATURE_TXOPPS
extern oal_uint8 mac_vap_get_txopps(mac_vap_stru *pst_vap);
extern oal_void mac_vap_set_txopps(mac_vap_stru *pst_vap, oal_uint8 uc_value);
extern oal_void mac_vap_update_txopps(mac_vap_stru *pst_vap, mac_user_stru *pst_user);
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
extern wlan_mib_mimo_power_save_enum_uint8 mac_vap_get_smps_mode(mac_vap_stru *pst_mac_vap);
extern wlan_mib_mimo_power_save_enum_uint8 mac_vap_get_smps_en(mac_vap_stru *pst_mac_vap);
extern oal_void mac_vap_set_smps(mac_vap_stru *pst_vap, oal_uint8 uc_value);
#endif
#ifdef _PRE_WLAN_FEATURE_UAPSD
extern oal_uint32  mac_vap_set_uapsd_en_etc(mac_vap_stru *pst_mac_vap, oal_uint8 uc_value);
extern oal_uint8  mac_vap_get_uapsd_en_etc(mac_vap_stru *pst_mac_vap);
#endif
extern oal_uint32  mac_vap_init_by_protocol_etc(mac_vap_stru *pst_mac_vap, wlan_protocol_enum_uint8 en_protocol);

extern  oal_bool_enum_uint8 mac_vap_check_bss_cap_info_phy_ap_etc(oal_uint16 us_cap_info,mac_vap_stru *pst_mac_vap);
extern  mac_wme_param_stru  *mac_get_wmm_cfg_etc(wlan_vap_mode_enum_uint8 en_vap_mode);
#ifdef _PRE_WLAN_FEATURE_EDCA_MULTI_USER_MULTI_AC
extern  mac_wme_param_stru  *mac_get_wmm_cfg_multi_user_multi_ac(oal_traffic_type_enum_uint8 en_vap_mode);
#endif
extern oal_void  mac_vap_get_bandwidth_cap_etc(mac_vap_stru *pst_mac_vap, wlan_bw_cap_enum_uint8 *pen_cap);
extern oal_void  mac_vap_change_mib_by_bandwidth_etc(mac_vap_stru *pst_mac_vap, wlan_channel_band_enum_uint8 en_band, wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern wlan_bw_cap_enum_uint8 mac_vap_bw_mode_to_bw(wlan_channel_bandwidth_enum_uint8 en_mode);
extern oal_void  mac_vap_init_rx_nss_by_protocol_etc(mac_vap_stru *pst_mac_vap);
extern oal_uint8  mac_vap_get_ap_usr_opern_bandwidth(mac_vap_stru *pst_mac_sta, mac_user_stru *pst_mac_user);
extern oal_uint32  mac_vap_check_ap_usr_opern_bandwidth(mac_vap_stru *pst_mac_sta, mac_user_stru *pst_mac_user);
extern oal_uint8  mac_vap_set_bw_check(mac_vap_stru *pst_mac_sta, wlan_channel_bandwidth_enum_uint8 en_sta_new_bandwidth);
#if 0
#ifdef _PRE_WLAN_DFT_STAT
extern oal_uint32  mac_vap_dft_stat_clear(mac_vap_stru *pst_vap);
#endif
#endif
extern oal_void mac_dec_p2p_num_etc(mac_vap_stru *pst_vap);
extern oal_void mac_inc_p2p_num_etc(mac_vap_stru *pst_vap);
extern oal_void mac_vap_set_p2p_mode_etc(mac_vap_stru *pst_vap, wlan_p2p_mode_enum_uint8 en_p2p_mode);
extern wlan_p2p_mode_enum_uint8 mac_get_p2p_mode_etc(mac_vap_stru *pst_vap);
extern oal_void mac_vap_set_aid_etc(mac_vap_stru *pst_vap, oal_uint16 us_aid);
extern oal_void mac_vap_set_uapsd_cap_etc(mac_vap_stru *pst_vap, oal_uint8 uc_uapsd_cap);
extern oal_void mac_vap_set_assoc_id_etc(mac_vap_stru *pst_vap, oal_uint16 us_assoc_vap_id);
extern oal_void mac_vap_set_tx_power_etc(mac_vap_stru *pst_vap, oal_uint8 uc_tx_power);
extern oal_void mac_vap_set_al_tx_flag_etc(mac_vap_stru *pst_vap, oal_bool_enum_uint8 en_flag);
extern oal_void mac_vap_set_al_tx_payload_flag_etc(mac_vap_stru *pst_vap, oal_uint8 uc_paylod);
extern oal_uint32 mac_dump_protection_etc(mac_vap_stru *pst_mac_vap, oal_uint8* puc_param);
extern oal_void mac_vap_set_multi_user_idx_etc(mac_vap_stru *pst_vap, oal_uint16 us_multi_user_idx);
#ifdef _PRE_WLAN_FEATURE_STA_PM
extern oal_void mac_vap_set_uapsd_para_etc(mac_vap_stru *pst_mac_vap, mac_cfg_uapsd_sta_stru *pst_uapsd_info);
#endif
extern oal_void mac_vap_set_wmm_params_update_count_etc(mac_vap_stru *pst_vap, oal_uint8 uc_update_count);
extern oal_void mac_vap_set_rifs_tx_on_etc(mac_vap_stru *pst_vap, oal_uint8 uc_value);
#if 0
extern oal_void mac_vap_set_tdls_prohibited(mac_vap_stru *pst_vap, oal_uint8 uc_value);
extern oal_void mac_vap_set_tdls_channel_switch_prohibited(mac_vap_stru *pst_vap, oal_uint8 uc_value);
#endif
extern oal_void mac_vap_set_11ac2g_etc(mac_vap_stru *pst_vap, oal_uint8 uc_value);
extern oal_void mac_vap_set_hide_ssid_etc(mac_vap_stru *pst_vap, oal_uint8 uc_value);
extern oal_uint8 mac_vap_get_peer_obss_scan_etc(mac_vap_stru *pst_vap);
extern oal_void mac_vap_set_peer_obss_scan_etc(mac_vap_stru *pst_vap, oal_uint8 uc_value);
extern oal_uint32 mac_vap_clear_app_ie_etc(mac_vap_stru *pst_mac_vap, en_app_ie_type_uint8 en_type);
extern oal_uint32 mac_vap_save_app_ie_etc(mac_vap_stru *pst_mac_vap, oal_app_ie_stru *pst_app_ie, en_app_ie_type_uint8 en_type);
extern oal_void mac_vap_set_rx_nss_etc(mac_vap_stru *pst_vap, wlan_nss_enum_uint8 en_rx_nss);
extern oal_uint32 mac_vap_init_privacy_etc(mac_vap_stru  *pst_mac_vap, mac_conn_security_stru *pst_conn_sec);
extern oal_uint32 mac_mib_set_wep_etc(mac_vap_stru *pst_mac_vap, oal_uint8 uc_key_id, oal_uint8 uc_key_value);
extern mac_user_stru  *mac_vap_get_user_by_addr_etc(mac_vap_stru *pst_mac_vap, oal_uint8  *puc_mac_addr);
extern oal_uint32 mac_vap_set_security(mac_vap_stru * pst_mac_vap, mac_beacon_param_stru * pst_beacon_param);
extern oal_uint32 mac_vap_add_key_etc(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, oal_uint8 uc_key_id, mac_key_params_stru *pst_key);
extern oal_uint8 mac_vap_get_default_key_id_etc(mac_vap_stru *pst_mac_vap);
extern oal_uint32 mac_vap_set_default_key_etc(mac_vap_stru *pst_mac_vap, oal_uint8  uc_key_index);
extern oal_uint32 mac_vap_set_default_mgmt_key_etc(mac_vap_stru *pst_mac_vap, oal_uint8 uc_key_index);
extern void mac_vap_init_user_security_port_etc(mac_vap_stru  *pst_mac_vap, mac_user_stru *pst_mac_user);
extern oal_uint8* mac_vap_get_mac_addr_etc(mac_vap_stru *pst_mac_vap);
#ifdef _PRE_WLAN_FEATURE_11R
extern oal_uint32 mac_mib_init_ft_cfg_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_mde);
extern oal_uint32 mac_mib_get_md_id_etc(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_mdid);
#endif //_PRE_WLAN_FEATURE_11R
extern oal_switch_enum_uint8 mac_vap_protection_autoprot_is_enabled_etc(mac_vap_stru *pst_mac_vap);
extern wlan_prot_mode_enum_uint8 mac_vap_get_user_protection_mode_etc(mac_vap_stru *pst_mac_vap_sta, mac_user_stru *pst_mac_user);
extern oal_void mac_protection_set_rts_tx_param_etc(mac_vap_stru *pst_mac_vap,oal_switch_enum_uint8 en_flag,
                                        wlan_prot_mode_enum_uint8 en_prot_mode,mac_cfg_rts_tx_param_stru *pst_rts_tx_param);

extern oal_bool_enum mac_protection_lsigtxop_check_etc(mac_vap_stru *pst_mac_vap);
extern oal_void mac_protection_set_lsig_txop_mechanism_etc(mac_vap_stru *pst_mac_vap, oal_switch_enum_uint8 en_flag);

#ifdef _PRE_WLAN_FEATURE_VOWIFI
extern oal_uint32 mac_vap_set_vowifi_param_etc(mac_vap_stru *pst_mac_vap, mac_vowifi_cmd_enum_uint8 en_vowifi_cfg_cmd, oal_uint8 uc_value);
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
extern oal_void mac_device_set_vap_id_etc(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap,  oal_uint8 uc_vap_idx, wlan_vap_mode_enum_uint8 en_vap_mode, wlan_p2p_mode_enum_uint8 en_p2p_mode, oal_uint8 is_add_vap);
extern oal_uint32  mac_device_find_up_vap_etc(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);
extern mac_vap_stru * mac_device_find_another_up_vap_etc(mac_device_stru *pst_mac_device, oal_uint8 uc_vap_id_self);
extern oal_uint32  mac_device_find_up_ap_etc(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);
extern oal_uint32  mac_device_calc_up_vap_num_etc(mac_device_stru *pst_mac_device);
extern oal_uint32 mac_device_calc_work_vap_num_etc(mac_device_stru *pst_mac_device);
extern oal_uint32  mac_device_find_up_p2p_go_etc(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);
extern oal_uint32  mac_device_get_up_vap_num(mac_device_stru *pst_mac_device);
extern oal_uint32  mac_device_find_2up_vap_etc(
                mac_device_stru *pst_mac_device,
                mac_vap_stru   **ppst_mac_vap1,
                mac_vap_stru   **ppst_mac_vap2);
#ifdef _PRE_WLAN_FEATURE_DBAC
extern oal_uint32  mac_fcs_dbac_state_check_etc(mac_device_stru *pst_mac_device);
#endif
extern oal_uint32  mac_device_find_up_sta_etc(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);
extern oal_uint32  mac_device_is_p2p_connected_etc(mac_device_stru *pst_mac_device);
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
extern mac_vap_stru *mac_find_main_proxysta(mac_device_stru *pst_mac_device);
#endif
#ifdef _PRE_WLAN_FEATURE_HILINK
extern oal_uint32 mac_vap_update_hilink_white_list(mac_vap_stru *pst_mac_vap, oal_hilink_white_node_stru *pst_white_node);
#endif
#ifdef _PRE_WLAN_FEATURE_SMPS
extern oal_uint32 mac_device_find_smps_mode_en(mac_device_stru *pst_mac_device,
                                                                 wlan_mib_mimo_power_save_enum_uint8 en_smps_mode);
#endif
extern oal_void mac_device_set_channel_etc(mac_device_stru *pst_mac_device, mac_cfg_channel_param_stru * pst_channel_param);
extern oal_void mac_device_get_channel_etc(mac_device_stru *pst_mac_device, mac_cfg_channel_param_stru * pst_channel_param);
extern oal_uint32  mac_fcs_get_prot_datarate(mac_vap_stru *pst_src_vap);
extern oal_uint32  mac_fcs_get_prot_mode(mac_vap_stru *pst_src_vap);

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP)
extern oal_void mac_blacklist_free_pointer(mac_vap_stru *pst_mac_vap, mac_blacklist_info_stru *pst_blacklist_info);
#endif

#ifdef _PRE_WLAN_WEB_CMD_COMM
extern oal_uint32  mac_oui_to_cipher_group(oal_uint32 cipher_oui);
extern oal_uint32  mac_oui_to_akm_suite(oal_uint32 cipher_oui);
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
oal_bool_enum_uint8  mac_vap_get_dfs_enable(mac_vap_stru *pst_mac_vap);
#endif
extern oal_void  mac_vap_init_mib_11ax(mac_vap_stru  *pst_mac_vap);
extern oal_void mac_vap_init_11ac_rates(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev);
extern oal_void mac_vap_init_11ax_rates(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev);
extern oal_void mac_vap_init_mib_11n_rom_cb(mac_vap_stru *pst_mac_vap);
extern oal_void mac_init_mib_rom_cb(mac_vap_stru *pst_mac_vap);
extern oal_uint32 mac_vap_init_by_protocol_cb(mac_vap_stru *pst_mac_vap, wlan_protocol_enum_uint8 en_protocol);
extern oal_void mac_vap_init_11n_rates_cb(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev);
extern oal_void mac_vap_init_11ac_rates_cb(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev);
extern oal_void mac_vap_init_11ac_mcs_singlenss_rom_cb(mac_vap_stru *pst_mac_vap, wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern oal_void mac_vap_init_11ac_mcs_doublenss_rom_cb(mac_vap_stru *pst_mac_vap, wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern oal_void mac_vap_ch_mib_by_bw_cb(mac_vap_stru *pst_mac_vap, wlan_channel_band_enum_uint8 en_band,  wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern oal_void mac_vap_init_11ac_mcs_singlenss(mac_vap_stru *pst_mac_vap, wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern oal_uint32 mac_vap_add_wep_key(mac_vap_stru *pst_mac_vap, oal_uint8 us_key_idx, oal_uint8 uc_key_len, oal_uint8 *puc_key);

#ifdef _PRE_WLAN_FEATURE_VOWIFI
extern oal_void mac_vap_vowifi_init(mac_vap_stru *pst_mac_vap);
extern oal_void mac_vap_vowifi_exit(mac_vap_stru *pst_mac_vap);
#endif
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
extern oal_void mac_tx_qos_enhance_list_init(mac_vap_stru *pst_mac_vap);
extern oal_void mac_tx_clean_qos_enhance_list(mac_vap_stru *pst_mac_vap);
extern mac_qos_enhance_sta_stru* mac_tx_find_qos_enhance_list(mac_vap_stru *pst_mac_vap ,oal_uint8 *puc_sta_member_addr);
#endif
extern oal_void  mac_init_mib_etc(mac_vap_stru *pst_mac_vap);
extern oal_void mac_vap_free_mib_etc(mac_vap_stru  *pst_vap);
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of mac_vap.h */
