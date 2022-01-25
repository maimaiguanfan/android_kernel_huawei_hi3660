

#ifndef __MAC_DEVICE_H__
#define __MAC_DEVICE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "oal_ext_if.h"
#include "oal_workqueue.h"
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "wlan_mib.h"
#include "hal_ext_if.h"
#include "mac_regdomain.h"
#include "mac_frame.h"
#include "wlan_types.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_DEVICE_H
/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define MAC_FTM_TIMER_CNT                   4

#define MAC_NET_DEVICE_NAME_LENGTH          16
#define MAC_BAND_CAP_NAME_LENGTH            16

#define MAC_DATARATES_80211B_NUM            4
#define MAC_DATARATES_PHY_80211G_NUM        12

#define MAC_RX_BA_LUT_BMAP_LEN             ((HAL_MAX_RX_BA_LUT_SIZE + 7) >> 3)
#define MAC_TX_BA_LUT_BMAP_LEN             ((HAL_MAX_TX_BA_LUT_SIZE + 7) >> 3)

/* �쳣��ʱ�ϱ�ʱ�� */
#define MAC_EXCEPTION_TIME_OUT              10000

/* DMAC SCANNER ɨ��ģʽ */
#define MAC_SCAN_FUNC_MEAS           0x1
#define MAC_SCAN_FUNC_STATS          0x2
#define MAC_SCAN_FUNC_RADAR          0x4
#define MAC_SCAN_FUNC_BSS            0x8
#define MAC_SCAN_FUNC_P2P_LISTEN     0x10
#define MAC_SCAN_FUNC_ALL            (MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS | MAC_SCAN_FUNC_RADAR | MAC_SCAN_FUNC_BSS)

#define MAC_ERR_LOG(_uc_vap_id, _puc_string)
#define MAC_ERR_LOG1(_uc_vap_id, _puc_string, _l_para1)
#define MAC_ERR_LOG2(_uc_vap_id, _puc_string, _l_para1, _l_para2)
#define MAC_ERR_LOG3(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3)
#define MAC_ERR_LOG4(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3, _l_para4)
#define MAC_ERR_VAR(_uc_vap_id, _c_fmt, ...)

#define MAC_WARNING_LOG(_uc_vap_id, _puc_string)
#define MAC_WARNING_LOG1(_uc_vap_id, _puc_string, _l_para1)
#define MAC_WARNING_LOG2(_uc_vap_id, _puc_string, _l_para1, _l_para2)
#define MAC_WARNING_LOG3(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3)
#define MAC_WARNING_LOG4(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3, _l_para4)
#define MAC_WARNING_VAR(_uc_vap_id, _c_fmt, ...)

#define MAC_INFO_LOG(_uc_vap_id, _puc_string)
#define MAC_INFO_LOG1(_uc_vap_id, _puc_string, _l_para1)
#define MAC_INFO_LOG2(_uc_vap_id, _puc_string, _l_para1, _l_para2)
#define MAC_INFO_LOG3(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3)
#define MAC_INFO_LOG4(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3, _l_para4)
#define MAC_INFO_VAR(_uc_vap_id, _c_fmt, ...)

/* ��ȡdev���㷨˽�нṹ�� */
#define MAC_DEV_ALG_PRIV(_pst_dev)                  ((_pst_dev)->p_alg_priv)

#define MAC_CHIP_ALG_PRIV(_pst_chip)                ((_pst_chip)->p_alg_priv)


/*��λ״̬*/
#define MAC_DEV_RESET_IN_PROGRESS(_pst_device,uc_value)    ((_pst_device)->uc_device_reset_in_progress = uc_value)
#define MAC_DEV_IS_RESET_IN_PROGRESS(_pst_device)          ((_pst_device)->uc_device_reset_in_progress)

#define MAC_DFS_RADAR_WAIT_TIME_MS    60000

#ifdef _PRE_WLAN_FEATURE_HILINK

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
#define HMAC_FBT_MAX_USER_NUM   64
#else
#define HMAC_FBT_MAX_USER_NUM   32
#endif

#define FBT_DEFAULT_SCAN_CHANNEL            (0)     /* Ĭ������FBT scan channelΪhome�ŵ�*/
#define FBT_DEFAULT_SCAN_INTERVAL           (200)   /* Ĭ������FBT scan interval 200ms */
#define FBT_DEFAULT_SCAN_REPORT_PERIOD      (200)   /* Ĭ������δ�����û������ϱ�����1000ms -- ��Ҫ�޸�Ϊ200ms */
#define FBT_DEFAULT_SCAN_CHANNEL_STAY_TIME  (50)   /* Ĭ�������ŵ�פ��ʱ�� */
#define FBT_DEFAULT_WORK_CHANNEL_STAY_TIME  (150)   /* Ĭ�Ϲ����ŵ�פ��ʱ�� */
#endif

#define MAC_SCAN_CHANNEL_INTERVAL_DEFAULT               6           /* ���6���ŵ����лع����ŵ�����һ��ʱ�� */
#define MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT           110         /* ����ɨ��ʱ�����ع����ŵ�������ʱ�� */
#define MAC_SCAN_CHANNEL_INTERVAL_PERFORMANCE           2           /* ���2���ŵ����лع����ŵ�����һ��ʱ�� */
#define MAC_WORK_TIME_ON_HOME_CHANNEL_PERFORMANCE       60          /* WLANδ������P2P���������ع����ŵ�������ʱ�� */
#define MAC_SCAN_CHANNEL_INTERVAL_HIDDEN_SSID             3           /* Я������SSID�ı���ɨ�裬���3���ŵ��ع����ŵ�����һ��ʱ�� */

#define MAC_FCS_DBAC_IGNORE           0   /* ����DBAC���� */
#define MAC_FCS_DBAC_NEED_CLOSE       1   /* DBAC��Ҫ�ر� */
#define MAC_FCS_DBAC_NEED_OPEN        2   /* DBAC��Ҫ���� */

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
#define MAC_MAX_IP_FILTER_BTABLE_SIZE 512 /* rx ip���ݰ����˹��ܵĺ�������С */
#endif //_PRE_WLAN_FEATURE_IP_FILTER

#define MAC_DEVICE_GET_CAP_BW(_pst_device)      ((_pst_device)->st_device_cap.en_channel_width)
#define MAC_DEVICE_GET_NSS_NUM(_pst_device)     ((_pst_device)->st_device_cap.en_nss_num)
#define MAC_DEVICE_GET_CAP_LDPC(_pst_device)    ((_pst_device)->st_device_cap.en_ldpc_is_supp)
#define MAC_DEVICE_GET_CAP_TXSTBC(_pst_device)  ((_pst_device)->st_device_cap.en_tx_stbc_is_supp)
#define MAC_DEVICE_GET_CAP_RXSTBC(_pst_device)  ((_pst_device)->st_device_cap.en_rx_stbc_is_supp)
#define MAC_DEVICE_GET_CAP_SUBFER(_pst_device)  ((_pst_device)->st_device_cap.en_su_bfmer_is_supp)
#define MAC_DEVICE_GET_CAP_SUBFEE(_pst_device)  ((_pst_device)->st_device_cap.en_su_bfmee_is_supp)
#define MAC_DEVICE_GET_CAP_MUBFER(_pst_device)  ((_pst_device)->st_device_cap.en_mu_bfmer_is_supp)
#define MAC_DEVICE_GET_CAP_MUBFEE(_pst_device)  ((_pst_device)->st_device_cap.en_mu_bfmee_is_supp)
#ifdef _PRE_WLAN_FEATURE_SMPS
#define MAC_DEVICE_GET_MODE_SMPS(_pst_device)   ((_pst_device)->en_mac_smps_mode)
#endif

#define MAC_M2S_CALI_SMPS_MODE(en_nss)   \
        ((en_nss == WLAN_SINGLE_NSS) ? WLAN_MIB_MIMO_POWER_SAVE_STATIC: WLAN_MIB_MIMO_POWER_SAVE_MIMO)

#ifdef _PRE_WLAN_FEATURE_M2S
#define MAC_M2S_CALI_NSS_FROM_SMPS_MODE(en_smps_mode)     \
        ((en_smps_mode == WLAN_MIB_MIMO_POWER_SAVE_STATIC) ? WLAN_SINGLE_NSS: WLAN_DOUBLE_NSS)
#endif

#define MAX_FTM_RANGE_ENTRY_COUNT                 15
#define MAX_FTM_ERROR_ENTRY_COUNT                 11
#define MAX_MINIMUN_AP_COUNT                      14
#define MAX_REPEATER_NUM                          3         /* ֧�ֵ����λap���� */

#define WLAN_USER_MAX_SUPP_RATES                  16        /* ���ڼ�¼�Զ��豸֧�ֵ����������� */


/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
/* SDT����ģʽö�� */
typedef enum
{
    MAC_SDT_MODE_WRITE = 0,
    MAC_SDT_MODE_READ,
    MAC_SDT_MODE_WRITE16,
    MAC_SDT_MODE_READ16,

    MAC_SDT_MODE_BUTT
}mac_sdt_rw_mode_enum;
typedef oal_uint8 mac_sdt_rw_mode_enum_uint8;

typedef enum
{
    MAC_CH_TYPE_NONE      = 0,
    MAC_CH_TYPE_PRIMARY   = 1,
    MAC_CH_TYPE_SECONDARY = 2,
}mac_ch_type_enum;
typedef oal_uint8 mac_ch_type_enum_uint8;

typedef enum
{
    MAC_CSA_FLAG_NORMAL   = 0,
    MAC_CSA_FLAG_START_DEBUG,/*�̶�csa ie ��beacon֡��*/
    MAC_CSA_FLAG_CANCLE_DEBUG,

    MAC_CSA_FLAG_BUTT
}mac_csa_flag_enum;
typedef oal_uint8 mac_csa_flag_enum_uint8;


typedef enum
{
    MAC_SCAN_OP_INIT_SCAN,
    MAC_SCAN_OP_FG_SCAN_ONLY,
    MAC_SCAN_OP_BG_SCAN_ONLY,

    MAC_SCAN_OP_BUTT
}mac_scan_op_enum;
typedef oal_uint8 mac_scan_op_enum_uint8;

typedef enum
{
    MAC_CHAN_NOT_SUPPORT = 0,        /* ������֧�ָ��ŵ� */
    MAC_CHAN_AVAILABLE_ALWAYS,       /* �ŵ�һֱ����ʹ�� */
    MAC_CHAN_AVAILABLE_TO_OPERATE,   /* �������(CAC, etc...)�󣬸��ŵ�����ʹ�� */
    MAC_CHAN_DFS_REQUIRED,           /* ���ŵ���Ҫ�����״��� */
    MAC_CHAN_BLOCK_DUE_TO_RADAR,     /* ���ڼ�⵽�״ﵼ�¸��ŵ���Ĳ����� */

    MAC_CHAN_STATUS_BUTT
}mac_chan_status_enum;
typedef oal_uint8 mac_chan_status_enum_uint8;

#ifdef _PRE_WLAN_DFT_STAT
typedef enum
{
    MAC_DEV_MGMT_STAT_TYPE_TX = 0,
    MAC_DEV_MGMT_STAT_TYPE_RX,
    MAC_DEV_MGMT_STAT_TYPE_TX_COMPLETE,

    MAC_DEV_MGMT_STAT_TYPE_BUTT
}mac_dev_mgmt_stat_type_enum;
typedef oal_uint8 mac_dev_mgmt_stat_type_enum_uint8;
#endif

/* device resetͬ��������ö�� */
typedef enum
{
    MAC_RESET_SWITCH_SET_TYPE,
    MAC_RESET_SWITCH_GET_TYPE,
    MAC_RESET_STATUS_GET_TYPE,
    MAC_RESET_STATUS_SET_TYPE,
    MAC_RESET_SWITCH_SYS_TYPE = MAC_RESET_SWITCH_SET_TYPE,
    MAC_RESET_STATUS_SYS_TYPE = MAC_RESET_STATUS_SET_TYPE,

    MAC_RESET_SYS_TYPE_BUTT
}mac_reset_sys_type_enum;
typedef oal_uint8 mac_reset_sys_type_enum_uint8;

typedef enum
{
    MAC_TRY_INIT_SCAN_VAP_UP,
    MAC_TRY_INIT_SCAN_SET_CHANNEL,
    MAC_TRY_INIT_SCAN_START_DBAC,
    MAC_TRY_INIT_SCAN_RESCAN,

    MAC_TRY_INIT_SCAN_BUTT
}mac_try_init_scan_type;
typedef oal_uint8 mac_try_init_scan_type_enum_uint8;

typedef enum
{
    MAC_INIT_SCAN_NOT_NEED,
    MAC_INIT_SCAN_NEED,
    MAC_INIT_SCAN_IN_SCAN,

}mac_need_init_scan_res;
typedef oal_uint8 mac_need_init_scan_res_enum_uint8;

/* p2p�ṹ�а�����״̬��Ա���ýṹ����mac device�£���VAP״̬ö���ƶ���mac_device.h�� */
/* VAP״̬����AP STA����һ��״̬ö�� */
typedef enum
{
    /* ap sta����״̬ */
    MAC_VAP_STATE_INIT               = 0,
    MAC_VAP_STATE_UP                 = 1,       /* VAP UP */
    MAC_VAP_STATE_PAUSE              = 2,       /* pause , for ap &sta */

    /* ap ����״̬ */
    MAC_VAP_STATE_AP_WAIT_START      = 3,

    /* sta����״̬ */
    MAC_VAP_STATE_STA_FAKE_UP        = 4,
    MAC_VAP_STATE_STA_WAIT_SCAN      = 5,
    MAC_VAP_STATE_STA_SCAN_COMP      = 6,
    MAC_VAP_STATE_STA_JOIN_COMP      = 7,
    MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2 = 8,
    MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4 = 9,
    MAC_VAP_STATE_STA_AUTH_COMP      = 10,
    MAC_VAP_STATE_STA_WAIT_ASOC      = 11,
    MAC_VAP_STATE_STA_OBSS_SCAN      = 12,
    MAC_VAP_STATE_STA_BG_SCAN        = 13,
    MAC_VAP_STATE_STA_LISTEN         = 14,/* p2p0 ���� */
    MAC_VAP_STATE_ROAMING            = 15,/* ���� */
    MAC_VAP_STATE_BUTT,
}mac_vap_state_enum;
typedef oal_uint8  mac_vap_state_enum_uint8;

#define     MAC_FCS_MAX_CHL_NUM    2
#define     MAC_FCS_TIMEOUT_JIFFY  2
#define     MAC_FCS_DEFAULT_PROTECT_TIME_OUT    5120    /* us */
#define     MAC_FCS_DEFAULT_PROTECT_TIME_OUT2   1024    /* us */
#define     MAC_FCS_DEFAULT_PROTECT_TIME_OUT3   15000   /* us */
#define     MAC_FCS_DEFAULT_PROTECT_TIME_OUT4   16000   /* us */
#define     MAC_ONE_PACKET_TIME_OUT             1000
#define     MAC_ONE_PACKET_TIME_OUT3            2000
#define     MAC_ONE_PACKET_TIME_OUT4            2000
#define     MAC_FCS_CTS_MAX_DURATION            32767   /* us */
#define     MAC_FCS_CTS_MAX_BTCOEX_NOR_DURATION            30000  /* us */
#define     MAC_FCS_CTS_MAX_BTCOEX_LDAC_DURATION           65535  /* us */

/*
 self CTS
+-------+-----------+----------------+
|frame  | duration  |      RA        |     len=10
|control|           |                |
+-------+-----------+----------------+

null data
+-------+-----------+---+---+---+--------+
|frame  | duration  |A1 |A2 |A3 |Seq Ctl | len=24
|control|           |   |   |   |        |
+-------+-----------+---+---+---+--------+

*/

typedef enum
{
    MAC_FCS_NOTIFY_TYPE_SWITCH_AWAY    = 0,
    MAC_FCS_NOTIFY_TYPE_SWITCH_BACK,

    MAC_FCS_NOTIFY_TYPE_BUTT
}mac_fcs_notify_type_enum;
typedef oal_uint8 mac_fcs_notify_type_enum_uint8;

/* 51 tbd, ע��host��deviceָ���С���� */
typedef struct
{
    mac_channel_stru                st_dst_chl;
    mac_channel_stru                st_src_chl;
    hal_one_packet_cfg_stru        *pst_one_packet_cfg;
    oal_uint8                       auc_res_rom[OAL_SIZEOF(hal_one_packet_cfg_stru) - 4];
    oal_uint16                      us_hot_cnt;
    oal_uint8                       auc_resv[2];
    hal_to_dmac_device_stru        *pst_hal_device;
    hal_tx_dscr_queue_header_stru  *pst_src_fake_queue;  /*��¼��vap�Լ���fake����ָ�� */
    mac_channel_stru                st_src_chl2;
    hal_one_packet_cfg_stru        *pst_one_packet_cfg2;
    oal_uint8                       auc_res_rom2[OAL_SIZEOF(hal_one_packet_cfg_stru) - 4];
}mac_fcs_cfg_stru;

typedef enum
{
    MAC_FCS_HOOK_ID_DBAC,
    MAC_FCS_HOOK_ID_ACS,

    MAC_FCS_HOOK_ID_BUTT
}mac_fcs_hook_id_enum;
typedef oal_uint8   mac_fcs_hook_id_enum_uint8;

typedef struct
{
    mac_fcs_notify_type_enum_uint8  uc_notify_type;
    oal_uint8                       uc_chip_id;
    oal_uint8                       uc_device_id;
    oal_uint8                       uc_resv[1];
    mac_fcs_cfg_stru                st_fcs_cfg;
}mac_fcs_event_stru;

typedef void (* mac_fcs_notify_func)(const mac_fcs_event_stru*);

typedef struct
{
    mac_fcs_notify_func    p_func;
}mac_fcs_notify_node_stru;

typedef struct
{
    mac_fcs_notify_node_stru   ast_notify_nodes[MAC_FCS_HOOK_ID_BUTT];
}mac_fcs_notify_chain_stru;

typedef enum
{
    MAC_FCS_STATE_STANDBY        = 0,  // free to use
    MAC_FCS_STATE_REQUESTED,           // requested by other module, but not in switching
    MAC_FCS_STATE_IN_PROGESS,          // in switching

    MAC_FCS_STATE_BUTT
}mac_fcs_state_enum;
typedef oal_uint8 mac_fcs_state_enum_uint8;

typedef enum
{
    MAC_FCS_SUCCESS = 0,
    MAC_FCS_ERR_NULL_PTR,
    MAC_FCS_ERR_INVALID_CFG,
    MAC_FCS_ERR_BUSY,
    MAC_FCS_ERR_UNKNOWN_ERR,
}mac_fcs_err_enum;
typedef oal_uint8   mac_fcs_err_enum_uint8;

typedef struct
{
    oal_uint32  ul_offset_addr;
    oal_uint32  ul_value[MAC_FCS_MAX_CHL_NUM];
}mac_fcs_reg_record_stru;

typedef struct tag_mac_fcs_mgr_stru
{
    volatile oal_bool_enum_uint8    en_fcs_done;
    oal_uint8                       uc_chip_id;
    oal_uint8                       uc_device_id;
    oal_uint8                       uc_fcs_cnt;
    oal_spin_lock_stru              st_lock;
    mac_fcs_state_enum_uint8        en_fcs_state;
    hal_fcs_service_type_enum_uint8 en_fcs_service_type;
    oal_uint8                       uc_resv[2];
    mac_fcs_cfg_stru               *pst_fcs_cfg;
    mac_fcs_notify_chain_stru       ast_notify_chain[MAC_FCS_NOTIFY_TYPE_BUTT];
}mac_fcs_mgr_stru;

#define MAC_FCS_VERIFY_MAX_ITEMS    1
typedef enum
{
    // isr
    MAC_FCS_STAGE_INTR_START,
    MAC_FCS_STAGE_INTR_POST_EVENT,
    MAC_FCS_STAGE_INTR_DONE,

    // event
    MAC_FCS_STAGE_EVENT_START,
    MAC_FCS_STAGE_PAUSE_VAP,
    MAC_FCS_STAGE_ONE_PKT_START,
    MAC_FCS_STAGE_ONE_PKT_INTR,
    MAC_FCS_STAGE_ONE_PKT_DONE,
    MAC_FCS_STAGE_RESET_HW_START,
    MAC_FCS_STAGE_RESET_HW_DONE,
    MAC_FCS_STAGE_RESUME_VAP,
    MAC_FCS_STAGE_EVENT_DONE,

    MAC_FCS_STAGE_COUNT
}mac_fcs_stage_enum;
typedef mac_fcs_stage_enum mac_fcs_stage_enum_uint8;

typedef struct
{
    oal_bool_enum_uint8         en_enable;
    oal_uint8                   auc_resv[3];
    oal_uint32                  ul_item_cnt;
    oal_uint32                  aul_timestamp[MAC_FCS_VERIFY_MAX_ITEMS][MAC_FCS_STAGE_COUNT];
}mac_fcs_verify_stat_stru;

/* �ϱ��ؼ���Ϣ��flags�����Ϣ����Ӧ���λΪ1������ϱ���Ӧ��Ϣ */
typedef enum
{
    MAC_REPORT_INFO_FLAGS_HARDWARE_INFO          = BIT(0),
    MAC_REPORT_INFO_FLAGS_QUEUE_INFO             = BIT(1),
    MAC_REPORT_INFO_FLAGS_MEMORY_INFO            = BIT(2),
    MAC_REPORT_INFO_FLAGS_EVENT_INFO             = BIT(3),
    MAC_REPORT_INFO_FLAGS_VAP_INFO               = BIT(4),
    MAC_REPORT_INFO_FLAGS_USER_INFO              = BIT(5),
    MAC_REPORT_INFO_FLAGS_TXRX_PACKET_STATISTICS = BIT(6),
    MAC_REPORT_INFO_FLAGS_BUTT                   = BIT(7),
}mac_report_info_flags;

#ifdef _PRE_WLAN_FEATURE_HILINK
/* ��������FBT SCAN������ģʽ:�رպͿ���ɨ��ģʽ */
typedef enum
{
    HMAC_FBT_SCAN_CLOSE     = 0,
    HMAC_FBT_SCAN_OPEN      = 1,

    HMAC_FBT_SCAN_BUTT
}hmac_fbt_scan_enum;
typedef oal_uint8 hmac_fbt_scan_enum_uint8;

/* ���������Աʹ��״��*/
typedef enum
{
    HMAC_FBT_SCAN_USER_NOT_USED = 0, /*0��ʾδʹ��*/
    HMAC_FBT_SCAN_USER_IS_USED  = 1, /*1��ʾ��ʹ�ò�д��MAC��ַ*/

    HMAC_FBT_SCAN_USER_BUTT
}hmac_fbt_scan_user_used_state;
#endif

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
/* rx ip���ݰ����˵��������� */
typedef enum
{
    MAC_IP_FILTER_ENABLE         = 0, /* ��/��ip���ݰ����˹��� */
    MAC_IP_FILTER_UPDATE_BTABLE  = 1, /* ���º����� */
    MAC_IP_FILTER_CLEAR          = 2, /* ��������� */

    MAC_IP_FILTER_BUTT
}mac_ip_filter_cmd_enum;
typedef oal_uint8 mac_ip_filter_cmd_enum_uint8;

/* ��������Ŀ��ʽ */
typedef struct
{
    oal_uint16                     us_port;          /* Ŀ�Ķ˿ںţ��������ֽ����ʽ�洢 */
    oal_uint8                      uc_protocol;
    oal_uint8                      uc_resv;
    //oal_uint32                  ul_filter_cnt;     /* Ŀǰδ����"ͳ�ƹ��˰�����"�����󣬴˳�Ա�ݲ�ʹ�� */
}mac_ip_filter_item_stru;

/* ���������ʽ */
typedef struct
{
    oal_uint8                       uc_item_count;
    oal_bool_enum_uint8             en_enable;       /* �·�����ʹ�ܱ�־ */
    mac_ip_filter_cmd_enum_uint8    en_cmd;
    oal_uint8                       uc_resv;
    mac_ip_filter_item_stru         ast_filter_items[1];
}mac_ip_filter_cmd_stru;

#endif //_PRE_WLAN_FEATURE_IP_FILTER

#ifdef _PRE_WLAN_FEATURE_APF
#define APF_PROGRAM_MAX_LEN 512
#define APF_VERSION 2
typedef enum
{
    APF_SET_FILTER_CMD,
    APF_GET_FILTER_CMD
}mac_apf_cmd_type_enum;
typedef oal_uint8 mac_apf_cmd_type_uint8;

typedef struct
{
    oal_bool_enum_uint8             en_is_enabled;
    oal_uint16                      us_program_len;
    oal_uint32                      ul_install_timestamp;
    oal_uint32                      ul_flt_pkt_cnt;
    oal_uint8                       auc_program[APF_PROGRAM_MAX_LEN];
}mac_apf_stru;

typedef struct
{
    mac_apf_cmd_type_uint8          en_cmd_type;
    oal_uint16                      us_program_len;
    oal_uint8                      *puc_program;
}mac_apf_filter_cmd_stru;
#endif

typedef enum
{
    MAC_DEVICE_DISABLE =0,
    MAC_DEVICE_2G,
    MAC_DEVICE_5G,
    MAC_DEVICE_2G_5G,

    MAC_DEVICE_BUTT,
}mac_device_radio_cap_enum;
typedef oal_uint8 mac_device_radio_cap_enum_uint8;

/* ��������ʹ�ã�mimo-siso�л�mode */
typedef enum
{
    MAC_M2S_MODE_QUERY        = 0,  /* ������ѯģʽ */
    MAC_M2S_MODE_MSS          = 1,  /* MSS�·�ģʽ */
    MAC_M2S_MODE_DELAY_SWITCH = 2,  /* �ӳ��л�����ģʽ */
    MAC_M2S_MODE_SW_TEST      = 3,  /* ���л�����ģʽ,����siso��mimo */
    MAC_M2S_MODE_HW_TEST      = 4,  /* Ӳ�л�����ģʽ,����siso��mimo */
    MAC_M2S_MODE_RSSI         = 5,  /* rssi�л� */

    MAC_M2S_MODE_BUTT,
}mac_m2s_mode_enum;
typedef oal_uint8 mac_m2s_mode_enum_uint8;

/* ��������ʹ�ã������л�״̬ */
typedef enum
{
    MAC_M2S_COMMAND_STATE_SISO_C0   = 0,
    MAC_M2S_COMMAND_STATE_SISO_C1   = 1,
    MAC_M2S_COMMAND_STATE_MIMO      = 2,
    MAC_M2S_COMMAND_STATE_MISO_C0   = 3,
    MAC_M2S_COMMAND_STATE_MISO_C1   = 4,

    MAC_M2S_COMMAND_STATE_BUTT,
}mac_m2s_command_state_enum;
typedef oal_uint8 mac_m2s_command_state_enum_uint8;

/* MSSʹ��ʱ������ʽ */
typedef enum
{
    MAC_M2S_COMMAND_MODE_SET_AUTO     = 0,
    MAC_M2S_COMMAND_MODE_SET_SISO_C0  = 1,
    MAC_M2S_COMMAND_MODE_SET_SISO_C1  = 2,
    MAC_M2S_COMMAND_MODE_SET_MIMO     = 3,
    MAC_M2S_COMMAND_MODE_GET_STATE    = 4,

    MAC_M2S_COMMAND_MODE_BUTT,
}mac_m2s_command_mode_enum;
typedef oal_uint8 mac_m2s_command_mode_enum_uint8;

/* ���ý��չ��ʲ��� */
typedef enum
{
    HAL_M2S_RSSI_SHOW_TH,
    HAL_M2S_RSSI_SHOW_MGMT,
    HAL_M2S_RSSI_SHOW_DATA,
    HAL_M2S_RSSI_SET_MIN_TH,
    HAL_M2S_RSSI_SET_DIFF_TH,
}hal_dev_rssi_enum;
typedef oal_uint8 hal_dev_rssi_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
typedef struct
{
    oal_uint8           uc_core_idx;
    oal_bool_enum_uint8 en_need_improved;
    oal_uint8           auc_rev[2];
}mac_cfg_tas_pwr_ctrl_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
typedef struct
{
    oal_bool_enum_uint8             en_is_pk_mode;
    wlan_bw_cap_enum_uint8          en_curr_bw_cap;             /* Ŀǰʹ�õĴ�������������lagency staģʽ����Ч */
    wlan_protocol_cap_enum_uint8    en_curr_protocol_cap;       /* Ŀǰʹ�õ�Э��ģʽ������������lagency staģʽ����Ч */
    wlan_nss_enum_uint8             en_curr_num_spatial_stream; /* Ŀǰ��˫���ļ��� */

    oal_uint32          ul_tx_bytes;            /* WIFI ҵ����֡ͳ�� */
    oal_uint32          ul_rx_bytes;            /* WIFI ҵ�����֡ͳ�� */
    oal_uint32          ul_dur_time;            /* ͳ��ʱ���� */
}mac_cfg_pk_mode_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
typedef enum
{
    NO_LOCATION    = 0,
    ROOT_AP        = 1,
    REPEATER       = 2,
    STA            = 3,
    LOCATION_TYPE_BUTT
}oal_location_type_enum;
typedef oal_uint8 oal_location_type_enum_uint8;
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
/* btcoex���������� */
typedef enum
{
    MAC_BTCOEX_BLACKLIST_LEV0 = BIT0,       /* 0������������Ҫ�����жϣ����翴mac��ַ */
    MAC_BTCOEX_BLACKLIST_LEV1 = BIT1,       /* һ������������mac��ַ */
    MAC_BTCOEX_BLACKLIST_BUTT,
}mac_btcoex_blacklist_enum;
typedef oal_uint8  mac_btcoex_blacklist_enum_uint8;
#endif

/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/
extern oal_uint8  g_auc_valid_blacklist_idx[];
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
extern mac_cfg_pk_mode_stru g_st_wifi_pk_mode_status;
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
/* device reset�¼�ͬ���ṹ�� */
typedef struct
{
    mac_reset_sys_type_enum_uint8  en_reset_sys_type;  /* ��λͬ������ */
    oal_uint8                      uc_value;           /* ͬ����Ϣֵ */
    oal_uint8                      uc_resv[2];
}mac_reset_sys_stru;

typedef void (*mac_scan_cb_fn)(void *p_scan_record);

typedef struct
{
    oal_uint16                    us_num_networks;
    mac_ch_type_enum_uint8        en_ch_type;
#ifdef _PRE_WLAN_FEATURE_DFS
    mac_chan_status_enum_uint8    en_ch_status;
#else
    oal_uint8                     auc_resv[1];
#endif
}mac_ap_ch_info_stru;

typedef struct
{
    oal_uint8                           uc_p2p_device_num;                      /* ��ǰdevice�µ�P2P_DEVICE���� */
    oal_uint8                           uc_p2p_goclient_num;                    /* ��ǰdevice�µ�P2P_CL/P2P_GO���� */
    oal_uint8                           uc_p2p0_vap_idx;                        /* P2P ���泡���£�P2P_DEV(p2p0) ָ�� */
    mac_vap_state_enum_uint8            en_last_vap_state;                      /* P2P0/P2P_CL ����VAP �ṹ�����������±���VAP �������ǰ��״̬ */
    oal_uint8                           uc_resv[2];                             /* ���� */
    oal_uint8                           en_roc_need_switch;                     /* remain on channel����Ҫ�л�ԭ�ŵ�*/
    oal_uint8                           en_p2p_ps_pause;                        /* P2P �����Ƿ���pause״̬*/
    oal_net_device_stru                *pst_p2p_net_device;                     /* P2P ���泡������net_device(p2p0) ָ�� */
    oal_uint64                          ull_send_action_id;                     /* P2P action id/cookie */
    oal_uint64                          ull_last_roc_id;
    oal_ieee80211_channel_stru          st_listen_channel;
    oal_nl80211_channel_type            en_listen_channel_type;
    oal_net_device_stru                *pst_primary_net_device;                 /* P2P ���泡������net_device(wlan0) ָ�� */
    oal_net_device_stru                *pst_second_net_device;                 /*�ŵ���������,������ʹ�ÿ���ɾ��*/
}mac_p2p_info_stru;

typedef struct
{
    oal_uint16    us_num_networks;    /* ��¼��ǰ�ŵ���ɨ�赽��BSS���� */
    oal_uint8     auc_resv[2];
    oal_uint8     auc_bssid_array[WLAN_MAX_SCAN_BSS_PER_CH][WLAN_MAC_ADDR_LEN];  /* ��¼��ǰ�ŵ���ɨ�赽������BSSID */
}mac_bss_id_list_stru;

#define MAX_PNO_SSID_COUNT          16
#define MAX_PNO_REPEAT_TIMES        4
#define PNO_SCHED_SCAN_INTERVAL     (60 * 1000)

/* PNOɨ����Ϣ�ṹ�� */
typedef struct
{
    oal_uint8           auc_ssid[WLAN_SSID_MAX_LEN];
    oal_bool_enum_uint8 en_scan_ssid;
    oal_uint8           auc_resv[2];
}pno_match_ssid_stru;

typedef struct
{
    pno_match_ssid_stru   ast_match_ssid_set[MAX_PNO_SSID_COUNT];
    oal_int32             l_ssid_count;                           /* �·�����Ҫƥ���ssid���ĸ��� */
    oal_int32             l_rssi_thold;                           /* ���ϱ���rssi���� */
    oal_uint32            ul_pno_scan_interval;                   /* pnoɨ���� */
    oal_uint8             auc_sour_mac_addr[WLAN_MAC_ADDR_LEN];   /* probe req֡��Я���ķ��Ͷ˵�ַ */
    oal_uint8             uc_pno_scan_repeat;                     /* pnoɨ���ظ����� */
    oal_bool_enum_uint8   en_is_random_mac_addr_scan;             /* �Ƿ����mac */

    mac_scan_cb_fn        p_fn_cb;                                /* ����ָ������������˼�ͨ�ų����� */
}mac_pno_scan_stru;

/* PNO����ɨ�����ṹ�� */
typedef struct
{
    mac_pno_scan_stru       st_pno_sched_scan_params;             /* pno����ɨ������Ĳ��� */
    //frw_timeout_stru        st_pno_sched_scan_timer;              /* pno����ɨ�趨ʱ�� */
    oal_void               *p_pno_sched_scan_timer;               /* pno����ɨ��rtcʱ�Ӷ�ʱ�����˶�ʱ����ʱ���ܹ�����˯�ߵ�device */
    oal_uint8               uc_curr_pno_sched_scan_times;         /* ��ǰpno����ɨ����� */
    oal_bool_enum_uint8     en_is_found_match_ssid;               /* �Ƿ�ɨ�赽��ƥ���ssid */
    oal_uint8               auc_resv[2];
}mac_pno_sched_scan_mgmt_stru;


typedef struct
{
    oal_uint8  auc_ssid[WLAN_SSID_MAX_LEN];
    oal_uint8  auc_resv[3];
}mac_ssid_stru;

typedef struct
{
    oal_uint8   uc_mac_rate; /* MAC��Ӧ���� */
    oal_uint8   uc_phy_rate; /* PHY��Ӧ���� */
    oal_uint8   uc_mbps;     /* ���� */
    oal_uint8   auc_resv[1];
}mac_data_rate_stru;

/* ɨ������ṹ�� */
typedef struct
{
    wlan_mib_desired_bsstype_enum_uint8 en_bss_type;            /* Ҫɨ���bss���� */
    wlan_scan_type_enum_uint8           en_scan_type;           /* ����/���� */
    oal_uint8                           uc_bssid_num;           /* ����ɨ���bssid���� */
    oal_uint8                           uc_ssid_num;            /* ����ɨ���ssid���� */

    oal_uint8                           auc_sour_mac_addr[WLAN_MAC_ADDR_LEN];       /* probe req֡��Я���ķ��Ͷ˵�ַ */
    oal_uint8                           uc_p2p0_listen_channel;
    oal_uint8                           uc_max_scan_count_per_channel;              /* ÿ���ŵ���ɨ����� */

    oal_uint8                           auc_bssid[WLAN_SCAN_REQ_MAX_BSSID][WLAN_MAC_ADDR_LEN];  /* ������bssid */
    mac_ssid_stru                       ast_mac_ssid_set[WLAN_SCAN_REQ_MAX_SSID];               /* ������ssid */

    oal_uint8                           uc_max_send_probe_req_count_per_channel;                /* ÿ���ŵ�����ɨ������֡�ĸ�����Ĭ��Ϊ1 */
    oal_uint8                           bit_is_p2p0_scan       : 1;   /* �Ƿ�Ϊp2p0 ����ɨ�� */
    oal_uint8                           bit_is_radom_mac_saved : 1;   /* �Ƿ��Ѿ��������mac */
    oal_uint8                           bit_radom_mac_saved_to_dev : 2; /* ���ڲ���ɨ�� */
    oal_uint8                           bit_desire_fast_scan       : 1;   /* ����ɨ������ʹ�ò��� */
    oal_uint8                           bit_rsv                    : 3;   /* ����λ */

    oal_bool_enum_uint8                 en_abort_scan_flag;           /* ��ֹɨ�� */
    oal_bool_enum_uint8                 en_is_random_mac_addr_scan;   /* �Ƿ������mac addrɨ�� */


    oal_bool_enum_uint8                 en_need_switch_back_home_channel;       /* ����ɨ��ʱ��ɨ����һ���ŵ����ж��Ƿ���Ҫ�лع����ŵ����� */
    oal_uint8                           uc_scan_channel_interval;               /* ���n���ŵ����лع����ŵ�����һ��ʱ�� */
    oal_uint16                          us_work_time_on_home_channel;           /* ����ɨ��ʱ�����ع����ŵ�������ʱ�� */

    mac_channel_stru                    ast_channel_list[WLAN_MAX_CHANNEL_NUM];

    oal_uint8                           uc_channel_nums;        /* �ŵ��б����ŵ��ĸ��� */
    oal_uint8                           uc_probe_delay;         /* ����ɨ�跢probe request֮֡ǰ�ĵȴ�ʱ�� */
    oal_uint16                          us_scan_time;           /* ɨ����ĳһ�ŵ�ͣ����ʱ���ɨ�����, ms��������10�������� */

    wlan_scan_mode_enum_uint8           en_scan_mode;                   /* ɨ��ģʽ:ǰ��ɨ�� or ����ɨ�� */
    oal_uint8                           uc_scan_flag;            /*�ں��·���ɨ��ģʽ,ÿ��bit�����wlan_scan_flag_enum����ʱֻ�����Ƿ�Ϊ����ɨ��*/
    oal_uint8                           uc_scan_func;                   /* DMAC SCANNER ɨ��ģʽ */
    oal_uint8                           uc_vap_id;                      /* �·�ɨ�������vap id */
    oal_uint64                          ull_cookie;             /* P2P �����·���cookie ֵ */
#ifdef _PRE_SUPPORT_ACS
    oal_uint8                           uc_acs_type;            /* acs��������:��ʼ/�����/�����Զ�����acs */
    oal_bool_enum_uint8                 en_switch_chan;         /* acs������Ƿ��л��ŵ� */
    oal_uint8                           auc_resv[2];
#endif
    /* ��Ҫ:�ص�����ָ��:����ָ������������˼�ͨ�ų����� */
    mac_scan_cb_fn                      p_fn_cb;
}mac_scan_req_stru;

/* ��ӡ���ձ��ĵ�rssi��Ϣ�ĵ��Կ�����صĽṹ�� */
typedef struct
{
    oal_uint16            us_data_len;                 /*  �������������  */
    oal_uint8             uc_tone_tran_switch;         /*  �������Ϳ���  */
    oal_uint8             uc_chain_idx;               /*  ��������ͨ����  */
}mac_tone_transmit_stru;

/* ��ӡ���ձ��ĵ�rssi��Ϣ�ĵ��Կ�����صĽṹ�� */
typedef struct
{
    oal_bool_enum_uint8     en_debug_switch;             /* ��ӡ�ܿ��� */
    oal_bool_enum_uint8     en_rssi_debug_switch;        /* ��ӡ���ձ��ĵ�rssi��Ϣ�ĵ��Կ��� */
    oal_bool_enum_uint8     en_snr_debug_switch;         /* ��ӡ���ձ��ĵ�snr��Ϣ�ĵ��Կ��� */
    oal_bool_enum_uint8     en_trlr_debug_switch;        /* ��ӡ���ձ��ĵ�trailer��Ϣ�ĵ��Կ��� */
    oal_bool_enum_uint8     en_evm_debug_switch;         /* ��ӡ���ձ��ĵ�evm��Ϣ�ĵ��Կ��� */
    oal_uint8               auc_resv[3];
    oal_uint32              ul_curr_rx_comp_isr_count;   /* һ�ּ���ڣ���������жϵĲ������� */
    oal_uint32              ul_rx_comp_isr_interval;     /* ������ٸ���������жϴ�ӡһ��rssi��Ϣ */
    mac_tone_transmit_stru  st_tone_tran;                /* �������Ͳ��� */
    oal_uint8               auc_trlr_sel_info[5];        /* trailerѡ���ϱ�����, һ���ֽڸ�4bitָʾtrlr or vect,��4��bitָʾѡ�� */
    oal_uint8               uc_trlr_sel_num;             /* ��¼������������ѡ������ֵ */
    oal_uint8               uc_iq_cali_switch;           /* iqУ׼��������  */
    oal_bool_enum_uint8     en_pdet_debug_switch;        /* ��ӡоƬ�ϱ�pdetֵ�ĵ��Կ��� */
    oal_bool_enum_uint8     en_tsensor_debug_switch;
    oal_uint8               uc_force_work_switch;
    oal_uint8               uc_dfr_reset_switch;         /* dfr_reset��������: ��4bitΪreset_mac_submod, ��4bitΪreset_hw_type */
    oal_uint8               uc_fsm_info_switch;          /* hal fsm debug info */
    oal_uint8               uc_report_radar_switch;      /* radar�ϱ����� */
    oal_uint8               uc_extlna_chg_bypass_switch; /* ���Ĳ��Թر�����LNA����: 0/1/2:no_bypass/dyn_bypass/force_bypass */
    oal_uint8               uc_edca_param_switch;        /* EDCA�������ÿ��� */
    oal_uint8               uc_edca_aifsn;      /* edca����AIFSN */
    oal_uint8               uc_edca_cwmin;      /* edca����CWmin */
    oal_uint8               uc_edca_cwmax;      /* edca����CWmax */
    oal_uint16              us_edca_txoplimit;      /* edca����TXOP limit */

}mac_phy_debug_switch_stru;

typedef struct
{
    wlan_csa_mode_tx_enum_uint8         en_mode;
    oal_uint8                           uc_channel;
    oal_uint8                           uc_cnt;
    wlan_channel_bandwidth_enum_uint8   en_bandwidth;
    mac_csa_flag_enum_uint8             en_debug_flag;/*0:�������ŵ�; 1:��beacon֡�к���csa,�ŵ����л�;2:ȡ��beacon֡�к���csa*/
    oal_uint8                           auc_reserv[3];
}mac_csa_debug_stru;


/* ������Կ�����صĽṹ�� */
typedef struct
{
    oal_uint32                              ul_cmd_bit_map;
    oal_bool_enum_uint8                     en_band_force_switch_bit0;    /* �ָ�40M��������*/
    oal_bool_enum_uint8                     en_2040_ch_swt_prohi_bit1;    /* ������20/40�����л�����*/
    oal_bool_enum_uint8                     en_40_intolerant_bit2;        /* ������40M������*/
    oal_uint8                               uc_resv;
    mac_csa_debug_stru                      st_csa_debug_bit3;
#ifdef _PRE_WLAN_FEATURE_HWBW_20_40
    oal_bool_enum_uint8                     en_2040_user_switch_bit4;
    oal_uint8                               auc_resv[3];
#endif
    oal_bool_enum_uint8                     en_lsigtxop_bit5;           /*lsigtxopʹ��*/
    oal_uint8                               auc_resv0[3];
}mac_protocol_debug_switch_stru;

#ifdef _PRE_WLAN_FEATURE_FTM
/* FTM���Կ�����صĽṹ�� */
typedef struct
{
    oal_uint8             uc_channel_num;
    oal_uint8             uc_burst_num;
    oal_bool_enum_uint8   measure_req;
    oal_uint8             uc_ftms_per_burst;

    oal_bool_enum_uint8   en_asap;
    oal_uint8             auc_resv[1];
    oal_uint8             auc_bssid[WLAN_MAC_ADDR_LEN];
}mac_send_iftmr_stru;

typedef struct
{
    oal_uint32             ul_ftm_correct_time1;
    oal_uint32             ul_ftm_correct_time2;
    oal_uint32             ul_ftm_correct_time3;
    oal_uint32             ul_ftm_correct_time4;
}mac_set_ftm_time_stru;

typedef struct
{
    oal_uint8             auc_resv[2];
    oal_uint8             auc_mac[WLAN_MAC_ADDR_LEN];
}mac_send_ftm_stru;

typedef struct
{
    oal_uint8                           auc_mac[WLAN_MAC_ADDR_LEN];
    oal_uint8                           uc_dialog_token;
    oal_uint8                           uc_meas_token;

    oal_uint16                          us_num_rpt;

    oal_uint16                          us_start_delay;
    oal_uint8                           auc_reserve[1];
    oal_uint8                           uc_minimum_ap_count;
    oal_uint8                           aauc_bssid[MAX_MINIMUN_AP_COUNT][WLAN_MAC_ADDR_LEN];
    oal_uint8                           auc_channel[MAX_MINIMUN_AP_COUNT];
} mac_send_ftm_range_req_stru; /*��ftm_range_req_stru ͬ���޸�*/

//typedef struct
//{
//    oal_location_type_enum_uint8            en_location_type;                                        /*��λ���� 0:�رն�λ 1:root ap;2:repeater;3station*/
//    oal_uint8                               auc_location_ap[MAX_REPEATER_NUM][WLAN_MAC_ADDR_LEN];    /*��location ap��mac��ַ����һ��Ϊroot */
//    oal_uint8                               auc_station[WLAN_MAC_ADDR_LEN];                          /*STA MAC��ַ����ʱֻ֧��һ��sta�Ĳ���*/
//} mac_location_stru;

typedef struct
{
    oal_uint8                               uc_tx_chain_selection;
    oal_bool_enum_uint8                     en_is_mimo;
    oal_uint8                               auc_reserve[2];
} ftm_m2s_stru;

typedef struct
{
    oal_uint32                      ul_cmd_bit_map;

    oal_bool_enum_uint8             en_ftm_initiator_bit0;        /* ftm_initiator����*/
    mac_send_iftmr_stru             st_send_iftmr_bit1;           /* ����iftmr����*/
    oal_bool_enum_uint8             en_enable_bit2;               /* ʹ��FTM*/
    oal_bool_enum_uint8             en_cali_bit3;                 /* FTM����*/
    mac_send_ftm_stru               st_send_ftm_bit4;             /* ����ftm����*/
    oal_bool_enum_uint8             en_ftm_resp_bit5;             /* ftm_resp����*/
    mac_set_ftm_time_stru           st_ftm_time_bit6;             /* ftm_time����*/
    mac_send_ftm_range_req_stru     st_send_range_req_bit7;       /* ����ftm_range_req����*/
    oal_bool_enum_uint8             en_ftm_range_bit8;            /* ftm_range����*/
    oal_uint8                       uc_get_cali_reserv_bit9;
    //mac_location_stru               st_location_bit10;
    ftm_m2s_stru                    st_m2s_bit11;
}mac_ftm_debug_switch_stru;
#endif
typedef struct
{
    oal_uint8                uc_category;
    oal_uint8                uc_action_code;
    oal_uint8                auc_oui[3];
    oal_uint8                uc_eid;
    oal_uint8                uc_lenth;
    oal_uint8                uc_location_type;
    oal_uint8                auc_mac_server[WLAN_MAC_ADDR_LEN];
    oal_uint8                auc_mac_client[WLAN_MAC_ADDR_LEN];
    oal_uint8                auc_payload[4];
}mac_location_event_stru;

typedef enum
{
    MAC_PM_DEBUG_SISO_RECV_BCN = 0,
    MAC_PM_DEBUG_DYN_TBTT_OFFSET = 1,
    MAC_PM_DEBUG_NO_PS_FRM_INT = 2,
    MAC_PM_DEBUG_APF = 3,
    MAC_PM_DEBUG_AO = 4,

    MAC_PM_DEBUG_CFG_BUTT
}mac_pm_debug_cfg_enum_uint8;

typedef struct
{
    oal_uint32            ul_cmd_bit_map;
    oal_uint8             uc_srb_switch; /* siso��beacon���� */
    oal_uint8             uc_dto_switch; /* ��̬tbtt offset���� */
    oal_uint8             uc_nfi_switch;
    oal_uint8             uc_apf_switch;
    oal_uint8             uc_ao_switch;
}mac_pm_debug_cfg_stru;

typedef enum
{
    MAC_DBDC_CHANGE_HAL_DEV = 0,   /* vap change hal device hal vap */
    MAC_DBDC_SWITCH         = 1,   /* DBDC������� */
    MAC_FAST_SCAN_SWITCH    = 2,   /* ����ɨ�迪�� */
    MAC_DBDC_STATUS         = 3,   /* DBDC״̬��ѯ */

    MAC_DBDC_CMD_BUTT
}mac_dbdc_cmd_enum;
typedef oal_uint8 mac_dbdc_cmd_enum_uint8;

typedef struct
{
    oal_uint32            ul_cmd_bit_map;
    oal_uint8             uc_dst_hal_dev_id; /*��ҪǨ�Ƶ���hal device id */
    oal_uint8             uc_dbdc_enable;
    oal_bool_enum_uint8   en_fast_scan_enable; /*�Ƿ���Բ���,XXԭ��ʹӲ��֧��Ҳ���ܿ���ɨ��*/
    oal_uint8             uc_dbdc_status;
}mac_dbdc_debug_switch_stru;

/* ACS ����ظ���ʽ */
typedef struct
{
    oal_uint8  uc_cmd;
    oal_uint8  uc_chip_id;
    oal_uint8  uc_device_id;
    oal_uint8  uc_resv;

    oal_uint32 ul_len;      /* �ܳ��ȣ���������ǰ4���ֽ� */
    oal_uint32 ul_cmd_cnt;  /* ����ļ��� */
}mac_acs_response_hdr_stru;

typedef struct
{
    oal_uint8   uc_cmd;
    oal_uint8   auc_arg[3];
    oal_uint32  ul_cmd_cnt;  /* ����ļ��� */
    oal_uint32  ul_cmd_len;  /* �ܳ��ȣ���ָauc_data��ʵ�ʸ��س��� */
    oal_uint8   auc_data[4];
}mac_acs_cmd_stru;

typedef mac_acs_cmd_stru    mac_init_scan_req_stru;

typedef enum
{
    MAC_ACS_RSN_INIT,
    MAC_ACS_RSN_LONG_TX_BUF,
    MAC_ACS_RSN_LARGE_PER,
    MAC_ACS_RSN_MWO_DECT,
    MAC_ACS_RSN_RADAR_DECT,

    MAC_ACS_RSN_BUTT
}mac_acs_rsn_enum;
typedef oal_uint8 mac_acs_rsn_enum_uint8;

typedef enum
{
    MAC_ACS_SW_NONE = 0x0,
    MAC_ACS_SW_INIT = 0x1,
    MAC_ACS_SW_DYNA = 0x2,
    MAC_ACS_SW_BOTH = 0x3,

    MAC_ACS_SW_BUTT
}en_mac_acs_sw_enum;
typedef oal_uint8 en_mac_acs_sw_enum_uint8;

typedef enum
{
    MAC_ACS_SET_CH_DNYA = 0x0,
    MAC_ACS_SET_CH_INIT = 0x1,

    MAC_ACS_SET_CH_BUTT
}en_mac_acs_set_ch_enum;
typedef oal_uint8 en_mac_acs_set_ch_enum_uint8;

typedef struct
{
    oal_bool_enum_uint8               en_sw_when_connected_enable : 1;
    oal_bool_enum_uint8               en_drop_dfs_channel_enable  : 1;
    oal_bool_enum_uint8               en_lte_coex_enable          : 1;
    en_mac_acs_sw_enum_uint8          en_acs_switch               : 5;
}mac_acs_switch_stru;

/* DMAC SCAN �ŵ�ɨ��BSS��ϢժҪ�ṹ */
typedef struct
{

    oal_int8                            c_rssi;             /* bss���ź�ǿ�� */
    oal_uint8                           uc_channel_number;  /* �ŵ��� */
    oal_uint8                           auc_bssid[WLAN_MAC_ADDR_LEN];

    /* 11n, 11ac��Ϣ */
    oal_bool_enum_uint8                 en_ht_capable;             /* �Ƿ�֧��ht */
    oal_bool_enum_uint8                 en_vht_capable;            /* �Ƿ�֧��vht */
    wlan_bw_cap_enum_uint8              en_bw_cap;                 /* ֧�ֵĴ��� 0-20M 1-40M */
    wlan_channel_bandwidth_enum_uint8   en_channel_bandwidth;      /* �ŵ��������� */
}mac_scan_bss_stats_stru;

typedef struct
{
    oal_int8                            c_rssi;             /* bss���ź�ǿ�� */
    oal_uint8                           uc_channel_number;  /* �ŵ��� */

    oal_bool_enum_uint8                 en_ht_capable   : 1;             /* �Ƿ�֧��ht */
    oal_bool_enum_uint8                 en_vht_capable  : 1;            /* �Ƿ�֧��vht */
    wlan_bw_cap_enum_uint8              en_bw_cap       : 3;            /* ֧�ֵĴ��� 0-20M 1-40M */
    wlan_channel_bandwidth_enum_uint8   en_channel_bandwidth : 3;      /* �ŵ��������� */
}mac_scan_bss_stats_simple_stru;

typedef struct
{
    oal_uint32  us_total_stats_time_ms  : 9;  // max 512 ms
    oal_uint32  uc_bandwidth_mode       : 3;
    oal_uint32  uc_radar_detected       : 1;  // FIXME: feed
    oal_uint32  uc_dfs_check_needed     : 1;
    oal_uint32  uc_radar_bw             : 3;
    oal_uint32  uc_radar_type           : 4;
    oal_uint32  uc_radar_freq_offset    : 3;
    oal_uint8   uc_channel_number;      /* �ŵ��� */

    oal_int8    s_free_power_20M;           // dBm
    oal_int8    s_free_power_40M;
    oal_int8    s_free_power_80M;
    oal_uint8   uc_free_time_20M_rate;      // percent, 255 scaled
    oal_uint8   uc_free_time_40M_rate;
    oal_uint8   uc_free_time_80M_rate;
    oal_uint8   uc_total_send_time_rate;    // percent, 255 scaled
    oal_uint8   uc_total_recv_time_rate;
}mac_scan_chan_stats_simple_stru;

/* DMAC SCAN �ص��¼��ṹ�� */
typedef struct
{
    oal_uint8                           uc_nchans;      /* �ŵ�����       */
    oal_uint8                           uc_nbss;        /* BSS���� */
    oal_uint8                           uc_scan_func;   /* ɨ�������Ĺ��� */

    oal_uint8                           uc_need_rank    : 1; // kernel write, app read
    oal_uint8                           uc_obss_on      : 1;
    oal_uint8                           uc_dfs_on       : 1;
    oal_uint8                           uc_dbac_on      : 1;
    oal_uint8                           uc_chip_id      : 2;
    oal_uint8                           uc_device_id    : 2;

    oal_uint8                           uc_acs_type;    /* 0:��ʼacs,1:�����acs,2:����ʱacs*/
    wlan_channel_band_enum_uint8        en_band;
    oal_uint8                           uc_pre_channel;
    wlan_channel_bandwidth_enum_uint8   en_pre_bw;
    oal_uint32                          ul_time_stamp;
    oal_bool_enum_uint8                 en_switch_chan;
    oal_uint8                           auc_resv[3];
}mac_scan_event_stru;


#ifdef _PRE_WLAN_FEATURE_DFS
typedef struct
{
    oal_bool_enum_uint8    en_dfs_switch;                               /* DFSʹ�ܿ��� bit0:dfsʹ��,bit1:��ʾAP��ΪDFS������ʱ������wait_start */
    oal_bool_enum_uint8    en_cac_switch;
    oal_bool_enum_uint8    en_offchan_cac_switch;
    oal_uint8              uc_debug_level;                              /* bit0:��ӡ�״��ҵ��bit1:��ӡ�״���ҵ�� */
    oal_uint8              uc_offchan_flag;                             /* bit0:0��ʾhomechan,1��ʾoffchan; bit1:0��ʾ��ͨ,1��ʾoffchancac */
    oal_uint8              uc_offchan_num;
    oal_uint8              uc_timer_cnt;
    oal_uint8              uc_timer_end_cnt;
    oal_uint8              uc_cts_duration;
    oal_uint8              uc_dmac_channel_flag;                        /* dmac���ڱ�ʾ��ǰ�ŵ�off or home */
    oal_bool_enum_uint8    en_dfs_init;
    oal_uint8              uc_custom_next_chnum;                        /* Ӧ�ò�ָ����DFS��һ���ŵ��� */
    oal_uint32             ul_dfs_cac_outof_5600_to_5650_time_ms;       /* CAC���ʱ����5600MHz ~ 5650MHzƵ���⣬Ĭ��60�� */
    oal_uint32             ul_dfs_cac_in_5600_to_5650_time_ms;          /* CAC���ʱ����5600MHz ~ 5650MHzƵ���ڣ�Ĭ��10���� */
    oal_uint32             ul_off_chan_cac_outof_5600_to_5650_time_ms;  /* Off-Channel CAC���ʱ����5600MHz ~ 5650MHzƵ���⣬Ĭ��6���� */
    oal_uint32             ul_off_chan_cac_in_5600_to_5650_time_ms;     /* Off-Channel CAC���ʱ����5600MHz ~ 5650MHzƵ���ڣ�Ĭ��60���� */
    oal_uint16             us_dfs_off_chan_cac_opern_chan_dwell_time;   /* Off-channel CAC�ڹ����ŵ���פ��ʱ�� */
    oal_uint16             us_dfs_off_chan_cac_off_chan_dwell_time;     /* Off-channel CAC��Off-Channel�ŵ���פ��ʱ�� */
    oal_uint32             ul_radar_detected_timestamp;
    oal_int32              l_radar_th;                                 //���õ��״������ޣ���λdb
    oal_uint32             ul_custom_chanlist_bitmap;                  //Ӧ�ò�ͬ���������ŵ��б�
                                                                       //(0x0000000F) /*36--48*/
                                                                       //(0x000000F0) /*52--64*/
                                                                       //(0x000FFF00) /*100--144*/
                                                                       //(0x01F00000) /*149--165*/
    wlan_channel_bandwidth_enum_uint8   en_next_ch_width_type;         //���õ���һ���ŵ��Ĵ���ģʽ
    oal_uint8                           uac_resv[3];
    oal_uint32             ul_dfs_non_occupancy_period_time_ms;
#ifdef _PRE_WLAN_FEATURE_DFS_ENABLE
    oal_uint8              uc_radar_type;
    oal_uint8              uc_band;
    oal_uint8              uc_channel_num;
    oal_uint8              uc_flag;
#else
    oal_uint8              _rom[4];
#endif


}mac_dfs_info_stru;

typedef struct
{
    oal_uint8             uc_chan_idx;        /* �ŵ����� */
    oal_uint8             uc_device_id;       /* device id */
    oal_uint8             auc_resv[2];
    frw_timeout_stru      st_dfs_nol_timer;   /* NOL�ڵ㶨ʱ�� */
    oal_dlist_head_stru   st_entry;           /* NOL���� */
}mac_dfs_nol_node_stru;

typedef struct
{
    frw_timeout_stru      st_dfs_cac_timer;                   /* CAC��ʱ�� */
    frw_timeout_stru      st_dfs_off_chan_cac_timer;          /* Off-Channel CAC��ʱ�� */
    frw_timeout_stru      st_dfs_chan_dwell_timer;            /* �ŵ�פ����ʱ������ʱ�����ڣ�������ŵ� */
    frw_timeout_stru      st_dfs_radar_timer;
    mac_dfs_info_stru     st_dfs_info;
    oal_dlist_head_stru   st_dfs_nol;
}mac_dfs_core_stru;
#endif


/* ɨ������¼�����״̬�� */
typedef enum
{
    MAC_SCAN_SUCCESS = 0,       /* ɨ��ɹ� */
    MAC_SCAN_PNO     = 1,       /* pnoɨ����� */
    MAC_SCAN_TIMEOUT = 2,       /* ɨ�賬ʱ */
    MAC_SCAN_REFUSED = 3,       /* ɨ�豻�ܾ� */
    MAC_SCAN_ABORT   = 4,       /* ��ֹɨ�� */
    MAC_SCAN_ABORT_SYNC = 5,    /* ɨ�豻��ֹͬ��״̬�������ϲ�ȥ��������ʱǿ��abort�����ں��ϱ���dmac��Ӧabort���ϱ� */
    MAC_SCAN_STATUS_BUTT,       /* ��Ч״̬�룬��ʼ��ʱʹ�ô�״̬�� */
}mac_scan_status_enum;
typedef oal_uint8   mac_scan_status_enum_uint8;

/* ɨ���� */
typedef struct
{
    mac_scan_status_enum_uint8  en_scan_rsp_status;
    oal_uint8                   auc_resv[3];
    oal_uint64                  ull_cookie;
}mac_scan_rsp_stru;


/* ɨ�赽��BSS�����ṹ�� */
typedef struct
{
    /* ������Ϣ */
    wlan_mib_desired_bsstype_enum_uint8 en_bss_type;                        /* bss�������� */
    oal_uint8                           uc_dtim_period;                     /* dtime���� */
    oal_uint8                           uc_dtim_cnt;                        /* dtime cnt */
    oal_bool_enum_uint8                 en_11ntxbf;                         /* 11n txbf */
    oal_bool_enum_uint8                 en_new_scan_bss;                    /* �Ƿ�����ɨ�赽��BSS */
    wlan_ap_chip_oui_enum_uint8         en_is_tplink_oui;
    oal_int8                            c_rssi;                             /* bss���ź�ǿ�� */
    oal_int8                            ac_ssid[WLAN_SSID_MAX_LEN];         /* ����ssid */
    oal_uint16                          us_beacon_period;                   /* beacon���� */
    oal_uint16                          us_cap_info;                        /* ����������Ϣ */
    oal_uint8                           auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* ���������� mac��ַ��bssid��ͬ */
    oal_uint8                           auc_bssid[WLAN_MAC_ADDR_LEN];       /* ����bssid */
    mac_channel_stru                    st_channel;                         /* bss���ڵ��ŵ� */
    oal_uint8                           uc_wmm_cap;                         /* �Ƿ�֧��wmm */
    oal_uint8                           uc_uapsd_cap;                       /* �Ƿ�֧��uapsd */
    oal_bool_enum_uint8                 en_desired;                         /* ��־λ����bss�Ƿ��������� */
    oal_uint8                           uc_num_supp_rates;                  /* ֧�ֵ����ʼ����� */
    oal_uint8                           auc_supp_rates[WLAN_USER_MAX_SUPP_RATES];/* ֧�ֵ����ʼ� */

#ifdef _PRE_WLAN_FEATURE_11D
    oal_int8                            ac_country[WLAN_COUNTRY_STR_LEN];   /* �����ַ��� */
    oal_uint8                           auc_resv2[1];
    oal_uint8                          *puc_country_ie;                     /* ���ڴ��country ie */
#endif

    /* ��ȫ��ص���Ϣ */
    oal_uint8                          *puc_rsn_ie;          /* ���ڴ��beacon��probe rsp��rsn ie */
    oal_uint8                          *puc_wpa_ie;          /* ���ڴ��beacon��probe rsp��wpa ie */

    /* 11n 11ac��Ϣ */
    oal_bool_enum_uint8                 en_ht_capable;                      /* �Ƿ�֧��ht */
    oal_bool_enum_uint8                 en_vht_capable;                     /* �Ƿ�֧��vht */
    oal_bool_enum_uint8                 en_vendor_vht_capable;              /* �Ƿ�֧��hidden vendor vht */
    wlan_bw_cap_enum_uint8              en_bw_cap;                          /* ֧�ֵĴ��� 0-20M 1-40M */
    wlan_channel_bandwidth_enum_uint8   en_channel_bandwidth;               /* �ŵ����� */
    oal_uint8                           uc_coex_mgmt_supp;                  /* �Ƿ�֧�ֹ������ */
    oal_bool_enum_uint8                 en_ht_ldpc;                         /* �Ƿ�֧��ldpc */
    oal_bool_enum_uint8                 en_ht_stbc;                         /* �Ƿ�֧��stbc */
    oal_uint8                           uc_wapi;
    oal_uint8                           en_vendor_novht_capable;            /* ˽��vendor�в�����Я�� */
    oal_bool_enum_uint8                 en_atheros_chip_oui;
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    mac_btcoex_blacklist_enum_uint8     en_btcoex_blacklist_chip_oui;       /* ps����one pkt֡������Ҫ�޶�Ϊself-cts�� */
#endif
    oal_uint32                          ul_timestamp;                       /* ���´�bss��ʱ��� */
#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    oal_uint8                           uc_phy_type;
    oal_int8                            ac_rsni[2];
    oal_uint8                           auc_resv3[1];
    oal_uint32                          ul_parent_tsf;
#endif
#ifdef _PRE_WLAN_WEB_CMD_COMM
    oal_uint32                          ul_max_rate_kbps;                   /* ��AP֧�ֵ��������(��λ:kbps) */
    oal_uint8                           uc_max_nss;                         /* ��AP֧�ֵ����ռ����� */
    oal_uint8                           auc_resv4[3];
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    wlan_nss_enum_uint8                 en_support_max_nss;                 /* ��AP֧�ֵ����ռ����� */
    oal_bool_enum_uint8                 en_support_opmode;                  /* ��AP�Ƿ�֧��OPMODE */
    oal_uint8                           uc_num_sounding_dim;                /* ��AP����txbf�������� */
#endif

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
    oal_bool_enum_uint8                 en_he_capable;                      /*�Ƿ�֧��11ax*/
#endif

#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11K_EXTERN) || defined(_PRE_WLAN_FEATURE_FTM) || defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)
    oal_bool_enum_uint8                 en_support_rrm;                     /*�Ƿ�֧��RRM*/
#endif

#ifdef _PRE_WLAN_FEATURE_1024QAM
    oal_bool_enum_uint8                 en_support_1024qam;
#endif

#ifdef _PRE_WLAN_NARROW_BAND
    oal_bool_enum_uint8                 en_nb_capable;                      /* �Ƿ�֧��nb */
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
    oal_bool_enum_uint8                 en_roam_blacklist_chip_oui;         /* ��֧��roam */
#endif
    oal_bool_enum_uint8                 en_txbf_blacklist_chip_oui;         /* ��֧��txbf */

    oal_int8                            c_ant0_rssi;                        /* ����0��rssi */
    oal_int8                            c_ant1_rssi;                        /* ����1��rssi */

    /* ����֡��Ϣ */
    oal_uint32                          ul_mgmt_len;                        /* ����֡�ĳ��� */
    oal_uint8                           auc_mgmt_buff[MAC_80211_QOS_HTC_4ADDR_FRAME_LEN];  /* ��¼beacon֡��probe rsp֡ */
    //oal_uint8                         *puc_mgmt_buff;                      /* ��¼beacon֡��probe rsp֡ */
}mac_bss_dscr_stru;

#ifdef _PRE_WLAN_DFT_STAT
/* ����֡ͳ����Ϣ */
typedef struct
{
    /* ���չ���֡ͳ�� */
    oal_uint32          aul_rx_mgmt[WLAN_MGMT_SUBTYPE_BUTT];

    /* �ҵ�Ӳ�����еĹ���֡ͳ�� */
    oal_uint32          aul_tx_mgmt_soft[WLAN_MGMT_SUBTYPE_BUTT];

    /* ������ɵĹ���֡ͳ�� */
    oal_uint32          aul_tx_mgmt_hardware[WLAN_MGMT_SUBTYPE_BUTT];
}mac_device_mgmt_statistic_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
typedef struct
{
    oal_bool_enum_uint8             en_psta_enable;
    oal_uint8                       uc_proxysta_cnt;                                        /* ����proxy sta�ĸ���(������main ProxySTA) */
    oal_uint8                       auc_resv[2];
}mac_device_psta_stru;

#define mac_dev_xsta_num(dev) ((dev)->st_psta.uc_proxysta_cnt)
#define mac_is_proxysta_enabled(pst_dev) ((pst_dev)->st_psta.en_psta_enable)
#endif


#ifdef _PRE_WLAN_DFT_STAT
/* �ϱ��տڻ�����ά������Ŀ��ƽṹ */
typedef struct
{
    oal_uint32                          ul_non_directed_frame_num;             /* ���յ��Ǳ���֡����Ŀ */
    oal_uint8                           uc_collect_period_cnt;                 /* �ɼ����ڵĴ���������100����ϱ�������Ȼ���������¿�ʼ */
    oal_bool_enum_uint8                 en_non_directed_frame_stat_flg;        /* �Ƿ�ͳ�ƷǱ�����ַ֡�����ı�־ */
    oal_int16                           s_ant_power;                           /* ���߿ڹ��� */
    frw_timeout_stru                    st_collect_period_timer;               /* �ɼ����ڶ�ʱ�� */
}mac_device_dbb_env_param_ctx_stru;
#endif

typedef enum
{
    MAC_DFR_TIMER_STEP_1 = 0,
    MAC_DFR_TIMER_STEP_2 = 1,

}mac_dfr_timer_step_enum;
typedef oal_uint8 mac_dfr_timer_step_enum_uint8;

typedef struct
{
    oal_uint32                         ul_tx_seqnum;                           /* ���һ��tx�ϱ���SN�� */
    oal_uint16                         us_seqnum_used_times;                   /* ���ʹ����ul_tx_seqnum�Ĵ��� */
    oal_uint16                         us_incr_constant;                       /* ά����Qos ��Ƭ֡�ĵ������� */
}mac_tx_seqnum_struc;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
typedef struct
{
    oal_bool_enum_uint8                 en_brk_limit_aggr_enable;
    oal_uint8                           uc_resv[3];
    oal_uint32                          ul_tx_dataflow_brk_cnt;
    oal_uint32                          ul_last_tx_complete_isr_cnt;
}mac_tx_dataflow_brk_bypass_struc;
#endif

/* ��Ļ״̬ */
typedef enum
{
    MAC_SCREEN_OFF = 0,
    MAC_SCREEN_ON  = 1,
}mac_screen_state_enum;

#ifdef _PRE_WLAN_FEATURE_HILINK
/* ��¼δ���������û�������� */
typedef struct
{
    oal_uint8   uc_is_used;                 /* ���û��Ƿ�ʹ�� */
    oal_uint8   uc_user_channel;            /* ������ǰ�������ŵ� */
    oal_uint8   auc_user_bssid[6];          /* ���������ĵ�ǰAP��bssid */
    oal_uint8   auc_user_mac_addr[6];       /* ��ǰ������MAC��ַ */
    oal_int16   s_rssi;                     /* ��ǰ�������ź�ǿ�� */
    oal_uint8   uc_is_found;                /* 0-û���ҵ���������1-�ҵ������� */
    oal_uint32  ul_rssi_timestamp;          /* ���յ���֡��ʱ��� */
    oal_uint32  ul_total_pkt_cnt;           /* ���յ���STA���ܰ���Ŀ */
}mac_fbt_scan_result_stru;


/* ��¼δ�����û�������Ϣ�Ľṹ�� */
typedef struct
{
    oal_uint8  uc_fbt_scan_enable;              /* ����δ�����û��������Կ��� */
    oal_uint8  uc_scan_channel;                 /* ���������ŵ���Ĭ��Ϊ0����ʾ��ǰ�ŵ� */
    oal_uint8  auc_reserve[2];
    oal_uint32 ul_scan_interval;                /* ����δ�����û�ÿ���ŵ����������ʱ������λΪ���� */
    oal_uint32 ul_scan_report_period;           /* ���������ϱ����� */
    oal_uint16 us_scan_channel_stay_time;       /* �����ŵ�פ��ʱ�� */
    oal_uint16 us_work_channel_stay_time;       /* �����ŵ�פ��ʱ�� */
    oal_uint32 ul_scan_timestamp;               /* ɨ�迪ʼʱ��� */
    mac_fbt_scan_result_stru ast_fbt_scan_user_list[HMAC_FBT_MAX_USER_NUM]; /* ��¼32���û������������Ϣ */
    frw_timeout_stru st_timer;                  /* ����δ�����û���Ϣ�ϱ���ʱ�� */
    frw_timeout_stru st_scan_timer;             /* ����δ�����û�ʹ�õ�ɨ�趨ʱ�� */
}mac_fbt_scan_mgmt_stru;

/* �����л�����ָ���û� fbt_monitor_specified_sta�������ڴ���mac��ַ*/
typedef struct
{
    oal_uint8               auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* MAC��ַ */
    oal_uint8               auc_resv[2];
}mac_fbt_scan_sta_addr_stru;

#endif

#ifdef _PRE_WLAN_MAC_ADDR_EDCA_FIX
typedef struct
{
    oal_bool_enum_uint8     en_enable;
    oal_uint8               uc_up_edca;
    oal_uint8               uc_down_edca;
    oal_uint8               uc_resv[1];
}mac_custom_edca_stru;
#endif

typedef struct
{

    /* ֧��2*2 */    /* ֧��MU-MIMO */
    wlan_nss_enum_uint8                     en_nss_num;              /* device Nss �ռ��������� */
    wlan_bw_cap_enum_uint8                  en_channel_width;        /* ֧�ֵĴ��� */
    oal_bool_enum_uint8                     en_nb_is_supp;           /* ֧��խ�� */
    oal_bool_enum_uint8                     en_1024qam_is_supp;      /* ֧��1024QAM���� */

    oal_bool_enum_uint8                     en_80211_mc_is_supp;    /* ֧��80211 mc */
    oal_bool_enum_uint8                     en_ldpc_is_supp;        /* �Ƿ�֧�ֽ���LDPC����İ� */
    oal_bool_enum_uint8                     en_tx_stbc_is_supp;     /* �Ƿ�֧������2x1 STBC���� */
    oal_bool_enum_uint8                     en_rx_stbc_is_supp;     /* �Ƿ�֧��stbc����,֧��2���ռ��� */

    oal_bool_enum_uint8                     en_su_bfmer_is_supp;     /* �Ƿ�֧�ֵ��û�beamformer */
    oal_bool_enum_uint8                     en_su_bfmee_is_supp;     /* �Ƿ�֧�ֵ��û�beamformee */
    oal_bool_enum_uint8                     en_mu_bfmer_is_supp;     /* �Ƿ�֧�ֶ��û�beamformer */
    oal_bool_enum_uint8                     en_mu_bfmee_is_supp;     /* �Ƿ�֧�ֶ��û�beamformee */
    oal_bool_enum_uint8                     en_11ax_switch;          /*11ax����*/
    oal_uint8                               _rom[3];
}mac_device_capability_stru;

typedef struct
{
    oal_bool_enum_uint8                     en_11k;
    oal_bool_enum_uint8                     en_11v;
    oal_bool_enum_uint8                     en_11r;
    oal_uint8                               auc_rsv[1];
}mac_device_voe_custom_stru;

typedef struct
{
    mac_m2s_mode_enum_uint8         en_cfg_m2s_mode;     /* 0:������ѯģʽ; 1:��������ģʽ;2.�л�ģʽ;3.���л�����ģʽ;4.Ӳ�л�����ģʽ 5.RSSI�������� */
    union
    {
        struct
        {
            oal_bool_enum_uint8      en_m2s_type;    /* �л����� */
            oal_uint8                uc_master_id;   /* ����·id */
            mac_m2s_command_state_enum_uint8 uc_m2s_state;   /* �����л���״̬ */
            wlan_m2s_trigger_mode_enum_uint8 uc_trigger_mode; /* �л�����ҵ��ģʽ */
        }test_mode;

        struct
        {
            oal_bool_enum_uint8  en_mss_on;
        }mss_mode;

        struct
        {
            oal_uint8                uc_opt;
            oal_int8                 c_value;
        }rssi_mode;
    }pri_data;
}mac_m2s_mgr_stru;

typedef struct
{
    wlan_m2s_mgr_vap_stru           ast_m2s_blacklist[WLAN_M2S_BLACKLIST_MAX_NUM];
    oal_uint8                       uc_blacklist_cnt;
}mac_m2s_ie_stru;

#ifdef _PRE_WLAN_FEATURE_BTCOEX
typedef struct
{
    oal_uint8                uc_cfg_btcoex_mode;     /* 0:������ѯģʽ; 1:��������ģʽ */
    oal_uint8                uc_cfg_btcoex_type;     /* 0:��������; 1:�ۺϴ�С���� 2.rssi detect���޲�������ģʽ */
    union
    {
        struct
        {
            wlan_nss_enum_uint8      en_btcoex_nss;          /* 0:siso 1:mimo */
            oal_uint8                uc_20m_low;   /* 2G 20M low */
            oal_uint8                uc_20m_high;  /* 2G 20M high */
            oal_uint8                uc_40m_low;   /* 2G 40M low */
            oal_uint16               us_40m_high;  /* 2G 40M high */
        }threhold;
        struct
        {
            wlan_nss_enum_uint8      en_btcoex_nss;          /* 0:siso 1:mimo */
            oal_uint8                uc_grade;   /* 0������1�� */
            oal_uint8                uc_rx_size0;   /* size0��С */
            oal_uint8                uc_rx_size1;   /* size1��С */
            oal_uint8                uc_rx_size2;   /* size2��С */
            oal_uint8                uc_rx_size3;   /* size3��С */
        }rx_size;
        struct
        {
            oal_bool_enum_uint8      en_rssi_limit_on;
            oal_bool_enum_uint8      en_rssi_log_on;
            oal_uint8                uc_cfg_rssi_detect_cnt;     /* 6 rssi ����ʱ���ڷ�������  �ߵ��������� */
            oal_int8                 c_cfg_rssi_detect_m2s_th;
            oal_int8                 c_cfg_rssi_detect_s2m_th;
        }rssi_param;
    }pri_data;
}mac_btcoex_mgr_stru;

typedef struct
{
    oal_uint8                uc_cfg_preempt_mode;     /* 0:Ӳ��preempt; 1:���preempt 2:ps ��ǰslot��*/
    oal_uint8                uc_cfg_preempt_type;     /* 0 noframe 1 self-cts 2 nulldata 3 qosnull  0/1 ���ps�򿪻��߹ر� */
}mac_btcoex_preempt_mgr_stru;
#endif

/* device�ṹ�� */
typedef struct
{
    oal_uint32                          ul_core_id;
    oal_uint8                           auc_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE];   /* device�µ�ҵ��vap���˴�ֻ��¼VAP ID */
    oal_uint8                           uc_cfg_vap_id;                          /* ����vap ID */
    oal_uint8                           uc_device_id;                           /* оƬID */
    oal_uint8                           uc_chip_id;                             /* �豸ID */
    oal_uint8                           uc_device_reset_in_progress;            /* ��λ������*/

    oal_bool_enum_uint8                 en_device_state;                        /* ��ʶ�Ƿ��Ѿ������䣬(OAL_TRUE��ʼ����ɣ�OAL_FALSEδ��ʼ�� ) */
    oal_uint8                           uc_vap_num;                             /* ��ǰdevice�µ�ҵ��VAP����(AP+STA) */
    oal_uint8                           uc_sta_num;                             /* ��ǰdevice�µ�STA���� */
/* begin: P2P */
    mac_p2p_info_stru                   st_p2p_info;                            /* P2P �����Ϣ */
/* end: P2P */

    oal_uint8                           auc_hw_addr[WLAN_MAC_ADDR_LEN];         /* ��eeprom��flash��õ�mac��ַ��ko����ʱ����hal�ӿڸ�ֵ */
    /* device������� */
    oal_uint8                           uc_max_channel;                         /* ������VAP���ŵ��ţ�����VAP����ֵ�������ֵì�ܣ����ڷ�DBACʱʹ�� */
    wlan_channel_band_enum_uint8        en_max_band;                            /* ������VAP��Ƶ�Σ�����VAP����ֵ�������ֵì�ܣ����ڷ�DBACʱʹ�� */

    oal_bool_enum_uint8                 en_wmm;                                 /* wmmʹ�ܿ��� */
    wlan_tidno_enum_uint8               en_tid;
    oal_uint8                           en_reset_switch;                        /* �Ƿ�ʹ�ܸ�λ����*/
    oal_uint8                           uc_csa_vap_cnt;                         /* ÿ��running AP����һ��CSA֡���ü�����1���������APֹͣ��ǰӲ�����ͣ�׼����ʼ�л��ŵ� */

    oal_uint32                          ul_beacon_interval;                     /*device����beacon interval,device������VAPԼ��Ϊͬһֵ*/

    oal_bool_enum_uint8                 en_delayed_shift;                       /* �˳�dbdcʱ���ӳ�Ǩ�� */
    oal_uint8                           uc_auth_req_sendst;
    oal_uint8                           uc_asoc_req_sendst;
    oal_bool_enum_uint8                 en_delayed_shift_down_prot;             /* �˳�dbdcʱ���ӳ�Ǩ��, ����ȥ����������delay����ӿ� */

    oal_uint32                          ul_resv1;
    oal_uint32                          ul_resv2;
    oal_uint32                          ul_resv3;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    oal_bool_enum_uint8                 en_is_wavetest;                         /* wavetest�û�ʶ�� 1:��  0:����    */
    oal_uint8                           uc_lock_channel;                        /* AGC��ͨ�� 0:��ͨ��0  1:��ͨ��1   2:����Ӧ  */
    oal_uint8                           auc_rev[2];

    wlan_chan_ratio_stru                 st_chan_ratio;                          /* �ŵ���æ�����ͳ���� */
#endif

    /* device���� */
    wlan_protocol_cap_enum_uint8        en_protocol_cap;                        /* Э������ */
    wlan_band_cap_enum_uint8            en_band_cap;                            /* Ƶ������ */
    wlan_channel_bandwidth_enum_uint8   en_max_bandwidth;                       /* ������VAP���������ֵ������VAP����ֵ�������ֵì�ܣ����ڷ�DBACʱʹ�� */

    oal_int16                           s_upc_amend;                            /* UPC����ֵ */

    oal_uint16                          us_device_reset_num;                    /* ��λ�Ĵ���ͳ��*/
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    mac_device_psta_stru                st_psta;
#endif

    mac_data_rate_stru                  st_mac_rates_11g[MAC_DATARATES_PHY_80211G_NUM];  /* 11g���� */

    mac_pno_sched_scan_mgmt_stru       *pst_pno_sched_scan_mgmt;                    /* pno����ɨ�����ṹ��ָ�룬�ڴ涯̬���룬�Ӷ���ʡ�ڴ� */
    mac_scan_req_stru                   st_scan_params;                             /* ����һ�ε�ɨ�������Ϣ */
    frw_timeout_stru                    st_obss_scan_timer;                         /* obssɨ�趨ʱ����ѭ����ʱ�� */
    mac_channel_stru                    st_p2p_vap_channel;                     /* p2p listenʱ��¼p2p���ŵ�������p2p listen������ָ� */

    oal_bool_enum_uint8                 en_2040bss_switch;                      /* 20/40 bss��⿪�� */
    oal_uint8                           uc_in_suspend;

#ifdef _PRE_SUPPORT_ACS
    /* DMAC ACS���� */
    oal_void                           *pst_acs;
    mac_acs_switch_stru                 st_acs_switch;
#endif

    /* linux�ں��е�device������Ϣ */
    oal_wiphy_stru                     *pst_wiphy;                             /* ���ڴ�ź�VAP��ص�wiphy�豸��Ϣ����AP/STAģʽ�¾�Ҫʹ�ã����Զ��VAP��Ӧһ��wiphy */
    mac_bss_id_list_stru                st_bss_id_list;                        /* ��ǰ�ŵ��µ�ɨ���� */

    oal_uint8                           uc_mac_vap_id;                         /* ��vap����ʱ������˯��ǰ��mac vap id */
    oal_bool_enum_uint8                 en_dbac_enabled;
    oal_bool_enum_uint8                 en_dbac_running;                       /* DBAC�Ƿ������� */
    oal_bool_enum_uint8                 en_dbac_has_vip_frame;                 /* ���DBAC����ʱ�յ��˹ؼ�֡ */

    oal_uint8                           uc_arpoffload_switch;
    oal_uint8                           uc_wapi;
    oal_uint8                           uc_reserve;
    oal_bool_enum_uint8                 en_is_random_mac_addr_scan;            /* ���macɨ�迪��,��hmac�·� */

    oal_uint8                           auc_mac_oui[WLAN_RANDOM_MAC_OUI_LEN];  /* ���mac��ַOUI,��Android�·� */
    oal_bool_enum_uint8                 en_dbdc_running;                       /* DBDC�Ƿ������� */

    mac_device_capability_stru          st_device_cap;                         /* device�Ĳ����������������ƻ� */

#ifdef _PRE_WLAN_FEATURE_SMPS
    /* SMPS��MAC����������Ҫdevice�����е�VAP��֧��SMPS�ŻῪ��MAC��SMPS���� */
    wlan_mib_mimo_power_save_enum_uint8 en_mac_smps_mode;                       /* ��¼��ǰMAC������SMPS����(����mac�࣬����ΪhmacҲ���������ж�) */
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
    mac_dfs_core_stru                   st_dfs;
#endif
    mac_ap_ch_info_stru                 st_ap_channel_list[MAC_MAX_SUPP_CHANNEL];

#ifdef _PRE_WLAN_FEATURE_HILINK
    mac_fbt_scan_mgmt_stru             st_fbt_scan_mgmt;                        /* ����δ�����û�������Ϣ�Ľṹ�� */
#endif
    /*���Device�ĳ�Ա�����ƶ���dmac_device*/
#if IS_DEVICE
    oal_uint16                          us_total_mpdu_num;                      /* device������TID���ܹ���mpdu_num���� */
    oal_uint16                          aus_ac_mpdu_num[WLAN_WME_AC_BUTT];      /* device�¸���AC��Ӧ��mpdu_num�� */
    oal_uint16                          aus_vap_mpdu_num[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];      /* ͳ�Ƹ���vap��Ӧ��mpdu_num�� */

    oal_uint16                          us_dfs_timeout;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    mac_tx_dataflow_brk_bypass_struc    st_dataflow_brk_bypass;
#endif
    oal_uint32                          ul_first_timestamp;                         /*��¼����ͳ�Ƶ�һ��ʱ���*/

    /* ɨ����س�Ա���� */
    oal_uint32                          ul_scan_timestamp;                      /* ��¼����һ��ɨ�迪ʼ��ʱ�� */

    mac_scan_state_enum_uint8           en_curr_scan_state;                     /* ��ǰɨ��״̬�����ݴ�״̬����obssɨ���host���·���ɨ�������Լ�ɨ�������ϱ����� */
    oal_uint8                           uc_resume_qempty_flag;                  /* ʹ�ָܻ�qempty��ʶ, Ĭ�ϲ�ʹ�� */
    oal_uint8                           uc_scan_count;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    oal_int8                            c_ppm_val;
#else
    oal_uint8                           auc_resv2[1];
#endif
// 1151�ϲ�����ʹ��module��ַ�ռ��RAM����֡��
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    mac_fcs_cfg_stru                   *pst_fcs_cfg;                             /* �������ŵ��ṹ�� */
#else
    mac_fcs_cfg_stru                    st_fcs_cfg;
#endif

#ifdef _PRE_WLAN_DFT_STAT
    /* ����֡ͳ����Ϣ */
    mac_device_mgmt_statistic_stru      st_mgmt_stat;
    mac_device_dbb_env_param_ctx_stru   st_dbb_env_param_ctx;                  /* �ϱ��տڻ�����ά������Ŀ��ƽṹ */
#endif
    mac_fcs_mgr_stru                    st_fcs_mgr;

    oal_uint8                           uc_csa_cnt;                            /* ÿ��AP����һ��CSA֡���ü�����1��AP�л����ŵ��󣬸ü������� */
    oal_bool_enum_uint8                 en_txop_enable;                        /* �����޾ۺ�ʱ����TXOPģʽ */
    oal_uint8                           uc_tx_ba_num;                  /* ���ͷ���BA�Ự���� */
    oal_uint8                           auc_resv[1];


    frw_timeout_stru                    st_keepalive_timer;                     /* keepalive��ʱ�� */

    oal_uint32                          aul_mac_err_cnt[HAL_MAC_ERROR_TYPE_BUTT];   /*mac ���������*/

#ifdef _PRE_WLAN_FEATURE_AP_PM
    oal_void*                           pst_pm_arbiter;                        /*device�ṹ���½����ٲù���ṹ*/
    oal_bool_enum_uint8                 en_pm_enable;                          /*PM���幦�ܿ����رտ���*/
    oal_uint8                           auc_resv7[3];
#endif

#ifdef _PRE_WLAN_REALTIME_CALI
    frw_timeout_stru                    st_realtime_cali_timer;                    /* ʵʱУ׼��ʱ��ʱ��*/
#endif
#endif /* IS_DEVICE */

    /*���Host�ĳ�Ա�����ƶ���hmac_device*/
#if IS_HOST
#ifndef _PRE_WLAN_FEATURE_AMPDU_VAP
    oal_uint8                           uc_rx_ba_session_num;                   /* ��device�£�rx BA�Ự����Ŀ */
    oal_uint8                           uc_tx_ba_session_num;                   /* ��device�£�tx BA�Ự����Ŀ */
    oal_uint8                           auc_resv11[2];
#endif
    oal_bool_enum_uint8                 en_vap_classify;                        /* �Ƿ�ʹ�ܻ���vap��ҵ����� */
    oal_uint8                           uc_ap_chan_idx;                        /* ��ǰɨ���ŵ����� */
    oal_uint8                           auc_resv4[2];
#endif /* IS_HOST */

    oal_bool_enum_uint8                 en_40MHz_intol_bit_recd;
    oal_uint8                           uc_ftm_vap_id;                           /*ftm�ж϶�Ӧ vap ID */
    oal_uint8                           auc_resv5[2];

#ifdef _PRE_WLAN_FEATURE_STA_PM
    hal_mac_key_statis_info_stru      st_mac_key_statis_info;                   /* mac�ؼ�ͳ����Ϣ */
#endif

#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)
    oal_work_stru                       auto_cali_work;
    oal_uint8                           uc_cali_type;
    oal_bool_enum_uint8                 en_cali_rdy;
    oal_uint8                           auc_resv6[2];
#endif

    frw_timeout_stru                    st_send_frame;                     /* send frame��ʱ�� */

#ifdef _PRE_WLAN_FEATUER_PCIE_TEST
    oal_uint8                               uc_pcie_test_flag;
    oal_uint32                              *pst_buff_start;
#endif
    oal_uint8                               _rom[4];
#ifdef _PRE_WLAN_MAC_ADDR_EDCA_FIX
    mac_custom_edca_stru                    st_custom_edca;
#endif
}mac_device_stru;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
#define MAC_DEV_GET_FCS_CFG(mac_dev) ((mac_dev)->pst_fcs_cfg)
#else
#define MAC_DEV_GET_FCS_CFG(mac_dev) (&(mac_dev)->st_fcs_cfg)
#endif

#pragma pack(push,1)
/* �ϱ���ɨ��������չ��Ϣ�������ϱ�host��Ĺ���֡netbuf�ĺ��� */
typedef struct
{
    oal_int32                           l_rssi;                     /* �ź�ǿ�� */
    wlan_mib_desired_bsstype_enum_uint8 en_bss_type;                /* ɨ�赽��bss���� */
#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    oal_int8                            c_snr_ant0;                 /* ant0 SNR */
    oal_int8                            c_snr_ant1;                 /* ant1 SNR */
    oal_uint8                           auc_resv[1];                /* Ԥ���ֶ� */
    oal_uint32                          ul_parent_tsf;              /* ��֡TSF Timer*/
#else
    oal_uint8                           auc_resv[3];                /* Ԥ���ֶ� */
#endif
#ifdef _PRE_WLAN_WEB_CMD_COMM
    oal_uint32                          ul_max_rate_kbps;           /* ��AP֧�ֵ��������(��λ:kbps) */
    oal_uint8                           uc_max_nss;                 /* ��AP֧�ֵ����ռ����� */
    hal_channel_assemble_enum_uint8     en_bw;
    oal_uint8                           auc_resv1[2];
#endif

    oal_int8                            c_ant0_rssi;                /* ����0��rssi */
    oal_int8                            c_ant1_rssi;                /* ����1��rssi */
    oal_uint8                           auc_resv2[2];

#ifdef _PRE_WLAN_FEATURE_M2S
    wlan_nss_enum_uint8                 en_support_max_nss;         /* ��AP֧�ֵ����ռ����� */
    oal_bool_enum_uint8                 en_support_opmode;          /* ��AP�Ƿ�֧��OPMODE */
    oal_uint8                           uc_num_sounding_dim;        /* ��AP����txbf�������� */
#endif
}mac_scanned_result_extend_info_stru;
#pragma pack(pop)


typedef struct
{
    oal_uint8                   auc_tx_ba_index_table[MAC_TX_BA_LUT_BMAP_LEN];          /* ���Ͷ�BA LUT��λͼ */
    oal_uint8                   auc_rx_ba_lut_idx_table[MAC_RX_BA_LUT_BMAP_LEN];        /* ���ն�BA LUT��λͼ */
#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
    oal_uint8                   auc_rx_ba_lut_idx_status_table[MAC_RX_BA_LUT_BMAP_LEN]; /* ���ն�BA LUT �Ƿ��� hal BA LUT�Ĵ����� 1: ��ʾ��  0: ��ʾ���� ��ǰ�ѱ��û���ȥ */
#endif
    oal_uint8                   auc_ra_lut_index_table[WLAN_ACTIVE_USER_IDX_BMAP_LEN];  /* �����û� LUT��λͼ */
}mac_lut_table_stru;

#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
typedef struct
{
    oal_dlist_head_stru         st_active_user_list_head;   /* ��Ծ�û��ڵ�˫������ʹ��user�ṹ�ڵ�dlist��ֻ���û���չ������ */
    oal_switch_enum_uint8       en_flag;                    /* chip���û���չ���� */
    oal_uint8                   auc_resv[3];                /* Ԥ���ֶΣ��ֶ����� */
}mac_chip_user_extend_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
typedef struct
{
    oal_uint32         ul_addr_h;     /* mac��ַ��λ */
    oal_uint32         ul_addr_l;     /* mac��ַ��λ */
    oal_uint32         ul_ba_param;   /* ba ����*/
    oal_uint32         ul_bitmap_h;   /* bitmap ��λ */
    oal_uint32         ul_bitmap_l;   /* bitmap ��λ */
}mac_chip_ba_lut_stru;
typedef struct
{
    mac_chip_ba_lut_stru     ast_rx_ba_lut_entry[HAL_MAX_RX_BA_LUT_SIZE];   /* �洢BA�����ϸ��Ϣ ����ͻָ�BA��ʱʹ�� */
    oal_uint8                auc_hal_to_dmac_lut_index_map[WLAN_MAX_RX_BA]; /* halӲ��BA index �� ���ba index��ӳ���ϵ�� ����ʵ��BA�滻���̸ı� */
    oal_uint8                auc_rx_ba_seq_to_lut_index_map[HAL_MAX_RX_BA_LUT_SIZE]; /* �յ��ľۺϰ���˳���ʵ��ba index��ӳ���ϵ�� */
    oal_uint8                uc_rx_ba_seq_index;                                  /* �յ�waveapp���͵ľۺϰ���˳��ͳ�� */
    oal_uint8                uc_prev_handle_ba_index;                             /* ��¼ǰһ�δ����ba index*/
    oal_uint16               us_rx_ba_seq_phase_one_count;
}mac_chip_rx_aggr_extend_stru;
#endif


/* chip�ṹ�� */
typedef struct
{
    oal_uint8                   auc_device_id[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP];    /* CHIP�¹ҵ�DEV������¼��Ӧ��ID����ֵ */
    oal_uint8                   uc_device_nums;                                 /* chip��device����Ŀ */
    oal_uint8                   uc_chip_id;                                     /* оƬID */
    oal_bool_enum_uint8         en_chip_state;                                  /* ��ʶ�Ƿ��ѳ�ʼ����OAL_TRUE��ʼ����ɣ�OAL_FALSEδ��ʼ�� */
    oal_uint32                  ul_chip_ver;                                    /* оƬ�汾 */
    hal_to_dmac_chip_stru      *pst_hal_chip;                                   /* Ӳmac�ṹָ�룬HAL�ṩ�������߼�������chip�Ķ�Ӧ */
    mac_lut_table_stru          st_lut_table;                                   /* ���ά��LUT����Դ�Ľṹ�� */
    oal_void                   *p_alg_priv;                                     /* chip�����㷨˽�нṹ�� */

    /* �û���س�Ա���� */
#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
    mac_chip_user_extend_stru   st_user_extend;                                 /* �û���չ�� */
#endif
    frw_timeout_stru            st_active_user_timer;                           /* �û���Ծ��ʱ�� */
#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
    mac_chip_rx_aggr_extend_stru *pst_rx_aggr_extend;
    oal_bool_enum_uint8           en_waveapp_32plus_user_enable;
#endif
    oal_uint8                   uc_assoc_user_cnt;                              /* �����û��� */
    oal_uint8                   uc_active_user_cnt;                             /* ��Ծ�û��� */
}mac_chip_stru;

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
typedef enum
{
    MAC_RX_IP_FILTER_STOPED  = 0, //���ܹرգ�δʹ�ܡ���������״����������˶�����
    MAC_RX_IP_FILTER_WORKING = 1, //���ܴ򿪣����չ�����������
    MAC_RX_IP_FILTER_BUTT
}mac_ip_filter_state_enum;
typedef oal_uint8 mac_ip_filter_state_enum_uint8;

typedef struct
{
    mac_ip_filter_state_enum_uint8 en_state;        //����״̬�����ˡ��ǹ��˵�
    oal_uint8                  uc_btable_items_num; //��������Ŀǰ�洢��items����
    oal_uint8                  uc_btable_size;      //��������С����ʾ���洢��items����
    oal_uint8                  uc_resv;
    mac_ip_filter_item_stru   *pst_filter_btable;   //������ָ��
}mac_rx_ip_filter_struc;
#endif //_PRE_WLAN_FEATURE_IP_FILTER

/* board�ṹ�� */
typedef struct
{
    mac_chip_stru               ast_chip[WLAN_CHIP_MAX_NUM_PER_BOARD];              /* board�ҽӵ�оƬ */
    oal_uint8                   uc_chip_id_bitmap;                                  /* ��ʶchip�Ƿ񱻷����λͼ */
    oal_uint8                   auc_resv[3];                                        /* �ֽڶ��� */
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    mac_rx_ip_filter_struc      st_rx_ip_filter;                                    /* rx ip���˹��ܵĹ���ṹ�� */
#endif //_PRE_WLAN_FEATURE_IP_FILTER
}mac_board_stru;

typedef struct
{
    mac_device_stru                    *pst_mac_device;
}mac_wiphy_priv_stru;

/* ������ */
typedef struct
{
    oal_uint8                       auc_mac_addr[OAL_MAC_ADDR_LEN];       /* mac��ַ          */
    oal_uint8                       auc_reserved[2];                      /* �ֽڶ���         */
    oal_uint32                      ul_cfg_time;                          /* �����������ʱ�� */
    oal_uint32                      ul_aging_time;                        /* �ϻ�ʱ��         */
    oal_uint32                      ul_drop_counter;                      /* ���Ķ���ͳ��     */
} mac_blacklist_stru;

/* �Զ������� */
typedef struct
{
    oal_uint8                       auc_mac_addr[OAL_MAC_ADDR_LEN];       /* mac��ַ  */
    oal_uint8                       auc_reserved[2];                      /* �ֽڶ��� */
    oal_uint32                      ul_cfg_time;                          /* ��ʼʱ�� */
    oal_uint32                      ul_asso_counter;                      /* �������� */
} mac_autoblacklist_stru;

/* �Զ���������Ϣ */
typedef struct
{
    oal_uint8                       uc_enabled;                             /* ʹ�ܱ�־ 0:δʹ��  1:ʹ�� */
    oal_uint8                       list_num;                               /* �ж��ٸ��Զ�������        */
    oal_uint8                       auc_reserved[2];                        /* �ֽڶ���                  */
    oal_uint32                      ul_threshold;                           /* ����                      */
    oal_uint32                      ul_reset_time;                          /* ����ʱ��                  */
    oal_uint32                      ul_aging_time;                          /* �ϻ�ʱ��                  */
    mac_autoblacklist_stru          ast_autoblack_list[WLAN_BLACKLIST_MAX]; /* �Զ���������              */
} mac_autoblacklist_info_stru;
#if 1
/* �ڰ�������Ϣ */
typedef struct
{
    oal_uint8                          uc_mode;                                  /* �ڰ�����ģʽ   */
    oal_uint8                          uc_list_num;                              /* ������         */
    oal_uint8                          uc_blacklist_vap_index;                      /* ������vap index */
    oal_uint8                          uc_blacklist_device_index;                   /* ������device index */
    mac_autoblacklist_info_stru        st_autoblacklist_info;                    /* �Զ���������Ϣ */
    mac_blacklist_stru                 ast_black_list[WLAN_BLACKLIST_MAX];       /* ��Ч�ڰ������� */
} mac_blacklist_info_stru;
#endif

/* m2s device��Ϣ�ṹ�� */
typedef struct
{
    oal_uint8                           uc_device_id;         /* ҵ��vap id */
    wlan_nss_enum_uint8                 en_nss_num;           /* device�Ľ��տռ������� */
    wlan_mib_mimo_power_save_enum_uint8 en_smps_mode;         /* mac device��smps�����������л���vap������ʼ�� */
    oal_uint8                           auc_reserved[1];
}mac_device_m2s_stru;

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/
/* ���߼��в��뿴���� */
#ifdef _PRE_WLAN_FEATURE_DBAC
#define MAC_DBAC_ENABLE(_pst_device) (_pst_device->en_dbac_enabled == OAL_TRUE)
#else
#define MAC_DBAC_ENABLE(_pst_device) (OAL_FALSE)
#endif

#define   MAC_CHIP_GET_HAL_CHIP(_pst_chip)  (((mac_chip_stru *)(_pst_chip))->pst_hal_chip)

extern mac_device_capability_stru g_st_mac_device_capability[];
extern mac_device_capability_stru *g_pst_mac_device_capability;

#ifdef _PRE_WLAN_FEATURE_WMMAC
extern oal_bool_enum_uint8 g_en_wmmac_switch_etc;
#endif

extern mac_board_stru g_st_mac_board;
extern mac_board_stru *g_pst_mac_board;



OAL_STATIC  OAL_INLINE  oal_bool_enum_uint8 mac_is_dbac_enabled(mac_device_stru *pst_device)
{
    return  pst_device->en_dbac_enabled;
}


OAL_STATIC  OAL_INLINE  oal_bool_enum_uint8 mac_is_dbac_running(mac_device_stru *pst_device)
{
    if (OAL_FALSE == pst_device->en_dbac_enabled)
    {
        return OAL_FALSE;
    }

    return  pst_device->en_dbac_running;
}

OAL_STATIC  OAL_INLINE  oal_bool_enum_uint8 mac_is_dbdc_running(mac_device_stru *pst_mac_device)
{
    return  pst_mac_device->en_dbdc_running;
}

#ifdef _PRE_SUPPORT_ACS

OAL_STATIC  OAL_INLINE  en_mac_acs_sw_enum_uint8 mac_get_acs_switch(mac_device_stru *pst_mac_device)
{
    if (pst_mac_device->pst_acs == OAL_PTR_NULL)
    {
        return MAC_ACS_SW_NONE;
    }

    return pst_mac_device->st_acs_switch.en_acs_switch;
}

OAL_STATIC  OAL_INLINE  oal_void mac_set_acs_switch(mac_device_stru *pst_mac_device, en_mac_acs_sw_enum_uint8 en_switch)
{
    if (pst_mac_device->pst_acs == OAL_PTR_NULL)
    {
        return;
    }

    pst_mac_device->st_acs_switch.en_acs_switch = en_switch;
}
#endif

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
OAL_STATIC  OAL_INLINE  oal_bool_enum_uint8 mac_get_2040bss_switch(mac_device_stru *pst_mac_device)
{
    return pst_mac_device->en_2040bss_switch;
}
OAL_STATIC  OAL_INLINE  oal_void mac_set_2040bss_switch(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_switch)
{
    pst_mac_device->en_2040bss_switch = en_switch;
}
#endif
#if IS_DEVICE
OAL_STATIC  OAL_INLINE  oal_bool_enum_uint8 mac_device_is_scaning(mac_device_stru *pst_mac_device)
{
    return (pst_mac_device->en_curr_scan_state == MAC_SCAN_STATE_RUNNING)?OAL_TRUE:OAL_FALSE;
}

OAL_STATIC  OAL_INLINE  oal_bool_enum_uint8 mac_device_is_listening(mac_device_stru *pst_mac_device)
{
    return ((pst_mac_device->en_curr_scan_state == MAC_SCAN_STATE_RUNNING)
            && (pst_mac_device->st_scan_params.uc_scan_func & MAC_SCAN_FUNC_P2P_LISTEN))?OAL_TRUE:OAL_FALSE;
}
#endif /* IS_DEVICE */
OAL_STATIC  OAL_INLINE oal_bool_enum_uint8 mac_chip_run_band(oal_uint8 uc_chip_id, wlan_channel_band_enum_uint8 en_band)
{
    /*
     * �ж�ָ��оƬ�Ƿ����������ָ��BAND��
     *     -˫оƬʱ��оƬֻ������ָ����BAND����������˫оƬ��Ƶ���޸Ĵ˴�
     *     -��оƬ˫Ƶʱ��������������BAND
     *     -��оƬ��Ƶʱֻ���������ں궨��ָ����BAND
     *     -note:Ŀǰ����witp wifiоƬ��֧��˫Ƶ���������е�ƵоƬ����Ҫ��������
     *      plat_chip_supp_band(chip_id, band)�Ľӿڣ����ڴ˴������ж�
     */

    if ((en_band != WLAN_BAND_2G) && (en_band != WLAN_BAND_5G))
    {
        return OAL_FALSE;
    }

#if defined(_PRE_WLAN_FEATURE_DOUBLE_CHIP)    // Ŀǰ˫оƬ��Ϊ˫оƬ���Ե�Ƶ
    return ((en_band == WLAN_BAND_2G) && (uc_chip_id == g_uc_wlan_double_chip_2g_id))
        || ((en_band == WLAN_BAND_5G) && (uc_chip_id == g_uc_wlan_double_chip_5g_id)) ? OAL_TRUE : OAL_FALSE;
#elif defined(_PRE_WLAN_FEATURE_SINGLE_CHIP_DUAL_BAND)
    return OAL_TRUE;
#elif defined(_PRE_WLAN_FEATURE_SINGLE_CHIP_SINGLE_BAND)
    return en_band == WLAN_SINGLE_CHIP_SINGLE_BAND_WORK_BAND ? OAL_TRUE : OAL_FALSE;
#else
    return OAL_TRUE;
#endif
}

/*****************************************************************************
  10 ��������
*****************************************************************************/
/*****************************************************************************
  10.1 �����ṹ���ʼ����ɾ��
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_DFS
extern oal_void  mac_dfs_init(mac_device_stru *pst_mac_device);
#endif /*#ifdef _PRE_WLAN_FEATURE_DFS  */
extern oal_uint32  mac_device_init_etc(mac_device_stru *pst_mac_device, oal_uint32 ul_chip_ver, oal_uint8 chip_id, oal_uint8 uc_device_id);
extern oal_uint32  mac_chip_init_etc(mac_chip_stru *pst_chip, oal_uint8 uc_device_max);
extern oal_uint32  mac_board_init_etc(void);

extern oal_uint32  mac_device_exit_etc(mac_device_stru *pst_device);
extern oal_uint32  mac_chip_exit_etc(mac_board_stru *pst_board, mac_chip_stru *pst_chip);
extern oal_uint32  mac_board_exit_etc(mac_board_stru *pst_board);


/*****************************************************************************
  10.2 ������Ա���ʲ���
*****************************************************************************/
extern oal_void mac_chip_inc_assoc_user(mac_chip_stru *pst_mac_chip);
extern oal_void mac_chip_dec_assoc_user(mac_chip_stru *pst_mac_chip);
extern oal_void mac_chip_inc_active_user(mac_chip_stru *pst_mac_chip);
extern oal_void mac_chip_dec_active_user(mac_chip_stru *pst_mac_chip);

#ifdef _PRE_WLAN_FEATURE_11AX
extern oal_uint8  mac_device_trans_bandwith_to_he_capinfo(wlan_bw_cap_enum_uint8 en_max_op_bd);
#endif

extern oal_void mac_device_set_dfr_reset_etc(mac_device_stru *pst_mac_device, oal_uint8 uc_device_reset_in_progress);
extern oal_void mac_device_set_state_etc(mac_device_stru *pst_mac_device, oal_uint8 en_device_state);

extern oal_void mac_device_set_beacon_interval_etc(mac_device_stru *pst_mac_device, oal_uint32 ul_beacon_interval);

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
extern oal_void mac_blacklist_get_pointer(wlan_vap_mode_enum_uint8 en_vap_mode, oal_uint8 uc_dev_id, oal_uint8 uc_chip_id, oal_uint8 uc_vap_id, mac_blacklist_info_stru **pst_blacklist_info);
#endif

#if 0
extern oal_void mac_device_inc_assoc_user(mac_device_stru *pst_mac_device);
extern oal_void mac_device_dec_assoc_user(mac_device_stru *pst_mac_device);
extern oal_void mac_device_set_dfs(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_dfs_switch, oal_uint8 uc_debug_level);
#endif
extern oal_void * mac_device_get_all_rates_etc(mac_device_stru *pst_dev);
#ifdef _PRE_WLAN_FEATURE_HILINK
extern oal_uint32  mac_device_clear_fbt_scan_list(mac_device_stru *pst_mac_dev, oal_uint8 *puc_param);
extern oal_uint32  mac_device_set_fbt_scan_sta(mac_device_stru *pst_mac_dev, mac_fbt_scan_sta_addr_stru *pst_fbt_scan_sta);
extern oal_uint32  mac_device_set_fbt_scan_interval(mac_device_stru *pst_mac_dev, oal_uint32 ul_scan_interval);
extern oal_uint32  mac_device_set_fbt_scan_channel(mac_device_stru *pst_mac_dev, oal_uint8 uc_fbt_scan_channel);
extern oal_uint32  mac_device_set_fbt_scan_report_period(mac_device_stru *pst_mac_dev, oal_uint32 ul_fbt_scan_report_period);
extern oal_uint32  mac_device_set_fbt_scan_enable(mac_device_stru *pst_mac_device, oal_uint8 uc_cfg_fbt_scan_enable);
#endif
extern oal_uint32  mac_device_check_5g_enable(oal_uint8 uc_device_id);

#ifdef _PRE_WLAN_FEATURE_DFS
extern oal_void  mac_dfs_set_cac_enable(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_val);
extern oal_void  mac_dfs_set_offchan_cac_enable(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_val);
extern oal_bool_enum_uint8  mac_dfs_get_offchan_cac_enable(mac_device_stru *pst_mac_device);
extern oal_void  mac_dfs_set_offchan_number(mac_device_stru *pst_mac_device, oal_uint32 ul_val);
extern oal_bool_enum_uint8  mac_dfs_get_cac_enable(mac_device_stru *pst_mac_device);
extern oal_void  mac_dfs_set_dfs_enable(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_val);
extern oal_bool_enum_uint8  mac_dfs_get_dfs_enable(mac_device_stru *pst_mac_device);
extern oal_void  mac_dfs_set_debug_level(mac_device_stru *pst_mac_device, oal_uint8 uc_debug_lev);
extern oal_uint8  mac_dfs_get_debug_level(mac_device_stru *pst_mac_device);
extern oal_void  mac_dfs_set_cac_time(mac_device_stru *pst_mac_device, oal_uint32 ul_time_ms, oal_bool_enum_uint8 en_waether);
extern oal_void  mac_dfs_set_off_cac_time(mac_device_stru *pst_mac_device, oal_uint32 ul_time_ms, oal_bool_enum_uint8 en_waether);
extern oal_void  mac_dfs_set_opern_chan_time(mac_device_stru *pst_mac_device, oal_uint32 ul_time_ms);
extern oal_void  mac_dfs_set_off_chan_time(mac_device_stru *pst_mac_device, oal_uint32 ul_time_ms);
extern oal_void  mac_dfs_set_next_radar_ch(mac_device_stru *pst_mac_device, oal_uint8 uc_ch, wlan_channel_bandwidth_enum_uint8 en_width);
extern oal_void  mac_dfs_set_ch_bitmap(mac_device_stru *pst_mac_device, oal_uint32 ul_ch_bitmap);
extern oal_void  mac_dfs_set_non_occupancy_period_time(mac_device_stru *pst_mac_device, oal_uint32 ul_time);
#endif

extern mac_device_voe_custom_stru   g_st_mac_voe_custom_param;

/*****************************************************************************
  10.3 ���������
*****************************************************************************/
#if 0
#ifdef _PRE_WLAN_FEATURE_DBAC
extern oal_uint32  mac_dbac_update_chl_config(mac_device_stru *pst_mac_device, mac_channel_stru *pst_chl);
#endif
#endif

/*****************************************************************************
  10.4 ���Ƴ�
*****************************************************************************/

/*****************************************************************************
  11 inline��������
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_device_check_5g_enable_per_chip(oal_void)
{
    oal_uint8            uc_dev_idx    = 0;

    while(uc_dev_idx < WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP)
    {
        if (mac_device_check_5g_enable(uc_dev_idx))
        {
            return OAL_TRUE;
        }
        uc_dev_idx++;
    }

    return OAL_FALSE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_is_hide_ssid(oal_uint8 *puc_ssid_ie, oal_uint8 uc_ssid_len)
{
    return (oal_bool_enum_uint8)((OAL_PTR_NULL == puc_ssid_ie) || (0 == uc_ssid_len) || ('\0' == puc_ssid_ie[0]));
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_device_is_auto_chan_sel_enabled(mac_device_stru *pst_mac_device)
{
    /* BSS����ʱ������û�û�������ŵ�����Ĭ��Ϊ�����Զ��ŵ�ѡ�� */
    if (0 == pst_mac_device->uc_max_channel)
    {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

extern wlan_mib_vht_supp_width_enum_uint8  mac_device_trans_bandwith_to_vht_capinfo(
                wlan_bw_cap_enum_uint8 en_max_op_bd);


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_fcs_is_same_channel(mac_channel_stru *pst_channel_dst,
                                                                  mac_channel_stru *pst_channel_src)
{
    return  pst_channel_dst->uc_chan_number == pst_channel_src->uc_chan_number ? OAL_TRUE : OAL_FALSE;
}

#if (_PRE_TEST_MODE_BOARD_ST == _PRE_TEST_MODE)
extern  oal_void mac_fcs_verify_init(oal_void);
extern  oal_void mac_fcs_verify_start(oal_void);
extern  oal_void mac_fcs_verify_timestamp(mac_fcs_stage_enum_uint8 en_stage);
extern  oal_void mac_fcs_verify_stop(oal_void);

#else
#define          mac_fcs_verify_init()
#define          mac_fcs_verify_start()
#define          mac_fcs_verify_timestamp(a)
#define          mac_fcs_verify_stop()
#endif  // _PRE_DEBUG_MODE
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of mac_device.h */
