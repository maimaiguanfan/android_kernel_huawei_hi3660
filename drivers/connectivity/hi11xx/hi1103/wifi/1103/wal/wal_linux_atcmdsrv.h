

#ifndef __WAL_LINUX_ATCMDSRV_H__
#define __WAL_LINUX_ATCMDSRV_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "oal_ext_if.h"
#include "wlan_types.h"
#include "wlan_spec.h"
#include "mac_vap.h"
#include "hmac_ext_if.h"
#include "wal_ext_if.h"
#include "wal_config.h"
//#include "dmac_alg_if.h"
#include "wal_linux_ioctl.h"
#include "hmac_cali_mgmt.h"
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_ATCMDSRV_H
/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define WAL_ATCMDSRB_DBB_NUM_TIME                (5 * OAL_TIME_HZ)
#define WAL_ATCMDSRB_GET_RX_PCKT                 (5 * OAL_TIME_HZ)
#define WAL_ATCMDSRB_NORM_TIME                   (1 * OAL_TIME_HZ)
#define FEM_FAIL_TIME                            (3)

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) && defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST)
enum WAL_ATCMDSRV_IOCTL_CMD
{
    WAL_ATCMDSRV_IOCTL_CMD_NORM_SET         = 0,
    WAL_ATCMDSRV_IOCTL_CMD_RX_PCKG_GET      ,
    WAL_ATCMDSRV_IOCTL_CMD_VAP_DOWN_SET     ,
    WAL_ATCMDSRV_IOCTL_CMD_HW_ADDR_SET      ,
    WAL_ATCMDSRV_IOCTL_CMD_VAP_UP_SET       ,
    WAL_ATCMDSRV_IOCTL_CMD_CHIPCHECK_SET    ,
    WAL_ATCMDSRV_IOCTL_CMD_REGINFO_SET      ,
    WAL_ATCMDSRV_IOCTL_CMD_CALIINFO_SET     ,
    WAL_ATCMDSRV_IOCTL_CMD_FEM_PA_INFO_GET  ,
    WAL_ATCMDSRV_IOCTL_CMD_GET_POWER_PARAM  ,
    HWIFI_IOCTL_CMD_TEST_BUTT
};
#endif

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
/*atcmdsrv˽������궨��*/
#define WAL_ATCMDSRV_IOCTL_DBB_LEN               12
#define WAL_ATCMDSRV_IOCTL_MAC_LEN               6
#define WAL_ATCMDSRV_IOCTL_COUNTRY_LEN           3
#define DP_INIT_EACH_CORE_NUM 13
#define DP_INIT_TWO_CORE_NUM (DP_INIT_EACH_CORE_NUM * 2)
#define DP_INIT_FLAG_NUM    1
#define WAL_ATCMDSRV_IOCTL_DYN_INTV_LEN         3

#define WAL_ATCMDSRV_IOCTL_MODE_NUM              13
#define WAL_ATCMDSRV_IOCTL_DATARATE_NUM          15
#define WAL_ATCMDSRB_IOCTL_AL_TX_LEN             2000
#define WAL_ATCMDSRV_NV_WINVRAM_LENGTH           104                    /* WINVRAM�ڴ�ռ� */
#define WAL_ATCMDSRV_DIE_ID_LENGTH               16
#define WAL_ATCMDSRB_CHECK_FEM_PA                (5 * OAL_TIME_HZ)
#define WAL_ATCMDSRV_GET_HEX_CHAR(tmp) (((tmp) > 9)? ((tmp-10) + 'A') : ((tmp) + '0'))
#define WLAN_HT_ONLY_MODE_2G                     WLAN_HT_ONLY_MODE + 3
#define WLAN_VHT_ONLY_MODE_2G                    WLAN_VHT_ONLY_MODE +3
/*lte�����Ӧ�Ĺܽź�*/
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
#define WAL_ATCMDSRV_ISM_PRIORITY_NAME          "ism_priority"
#define WAL_ATCMDSRV_LTE_RX_NAME                "lte_rx"
#define WAL_ATCMDSRV_LTE_TX_NAME                "lte_tx"
#define WAL_ATCMDSRV_LTE_INACT_NAME             "lte_inact"
#define WAL_ATCMDSRV_ISM_RX_ACT_NAME            "ism_rx_act"
#define WAL_ATCMDSRV_BANT_PRI_NAME              "bant_pri"
#define WAL_ATCMDSRV_BANT_STATUS_NAME           "bant_status"
#define WAL_ATCMDSRV_WANT_PRI_NAME              "want_pri"
#define WAL_ATCMDSRV_WANT_STATUS_NAME           "want_status"
#else
#define WAL_ATCMDSRV_LTE_ISM_PRIORITY            92
#define WAL_ATCMDSRV_LTE_RX_ACT                  90
#define WAL_ATCMDSRV_LTE_TX_ACT                  91
#define WAL_ATCMDSRV_LTE_ISM_PRIORITY_NAME       "lte_ism_priority"
#define WAL_ATCMDSRV_LTE_RX_ACT_NAME             "lte_rx_act"
#define WAL_ATCMDSRV_LTE_TX_ACT_NAME             "lte_tx_act"
#endif
#define WAL_ATCMDSRV_EFUSE_CHIP_ID                0x03
#define WAL_ATCMDSRV_EFUSE_ADC_ERR_FLAG           0x0C
#define WAL_ATCMDSRV_EFUSE_BUFF_LEN               32
#define WAL_ATCMDSRV_EFUSE_REG_WIDTH              0x10
#define WAL_ATCMDSRV_CHANNEL_NUM                  6
#define WAL_ATCMSRV_MIN_BSS_EXPIRATION_AGE 25           /* ��λ:  �� */
#define WAL_ATCMSRV_MAX_BSS_EXPIRATION_AGE 500          /*��λ:  ��*/

#define WAL_ATCMDSRV_WIFI_MIN_TXPOWER             5     /*��λ:dBm */
#define WAL_ATCMDSRV_WIFI_MAX_TXPOWER             14    /*��λ:dBm */

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
/*atcmdsrvö������*/
enum WAL_ATCMDSRV_IOCTL_CMD
{
    WAL_ATCMDSRV_IOCTL_CMD_WI_FREQ_SET=0,              /*  */
    WAL_ATCMDSRV_IOCTL_CMD_WI_POWER_SET,               /*  */
    WAL_ATCMDSRV_IOCTL_CMD_MODE_SET,                   /*  */
    WAL_ATCMDSRV_IOCTL_CMD_DATARATE_SET,
    WAL_ATCMDSRV_IOCTL_CMD_BAND_SET,
    WAL_ATCMDSRV_IOCTL_CMD_ALWAYS_TX_SET,
    WAL_ATCMDSRV_IOCTL_CMD_DBB_GET,
    WAL_ATCMDSRV_IOCTL_CMD_HW_STATUS_GET,
    WAL_ATCMDSRV_IOCTL_CMD_ALWAYS_RX_SET,
    WAL_ATCMDSRV_IOCTL_CMD_HW_ADDR_SET,
    WAL_ATCMDSRV_IOCTL_CMD_RX_PCKG_GET,
    WAL_ATCMDSRV_IOCTL_CMD_PM_SWITCH,
    WAL_ATCMDSRV_IOCTL_CMD_RX_RSSI,
    WAL_ATCMDSRV_IOCTL_CMD_CHIPCHECK_SET,
    WAL_ATCMDSRV_IOCTL_CMD_CHIPCHECK_RESULT,
    WAL_ATCMDSRV_IOCTL_CMD_CHIPCHECK_TIME,
    WAL_ATCMDSRV_IOCTL_CMD_UART_LOOP_SET,
    WAL_ATCMDSRV_IOCTL_CMD_SDIO_LOOP_SET,
    WAL_ATCMDSRV_IOCTL_CMD_RD_CALDATA,                  /* read caldata from dts */
    WAL_ATCMDSRV_IOCTL_CMD_SET_CALDATA,
    WAL_ATCMDSRV_IOCTL_CMD_EFUSE_CHECK,
    WAL_ATCMDSRV_IOCTL_CMD_SET_ANT,
    WAL_ATCMDSRV_IOCTL_CMD_DIEID_INFORM,
    WAL_ATCMDSRV_IOCTL_CMD_SET_COUNTRY,
    WAL_ATCMDSRV_IOCTL_CMD_GET_UPCCODE,
    WAL_ATCMDSRV_IOCTL_CMD_WIPIN_TEST,
    WAL_ATCMDSRV_IOCTL_CMD_PMU_CHECK,
    WAL_ATCMDSRV_IOCTL_CMD_SET_MIMO_CHANNEL,            /* set mimo channel */
    WAL_ATCMDSRV_IOCTL_CMD_GET_DP_INIT,
    WAL_ATCMDSRV_IOCTL_CMD_GET_PDET_PARAM,
    WAL_ATCMDSRV_IOCTL_CMD_IO_TEST,
    WAL_ATCMDSRV_IOCTL_CMD_PCIE_TEST,
    WAL_ATCMDSRV_IOCTL_CMD_PCIE_SDIO_SET,
    WAL_ATCMDSRV_IOCTL_CMD_DYN_INTERVAL,
    WAL_ATCMDSRV_IOCTL_CMD_PT_STE,
    WAL_ATCMDSRV_IOCTL_CMD_TAS_ANT_SET,
    WAL_ATCMDSRV_IOCTL_CMD_SELFCALI_INTERVAL,
    WAL_ATCMDSRV_IOCTL_CMD_SET_BSS_EXPIRE_AGE,      /* ����ɨ�����ϻ�ʱ��*/
    WAL_ATCMDSRV_IOCTL_CMD_GET_CONN_INFO,           /* ��ȡ������Ϣ */

    WAL_ATCMDSRV_IOCTL_CMD_TEST_BUTT
};
typedef enum
{
    ATCMDSRV_WIFI_DISCONNECT,           /*  */
    ATCMDSRV_WIFI_CONNECTED,            /*  */
}atcmdsrv_wifi_conn_info_enum;
typedef oal_uint8 atcmdsrv_wifi_conn_info_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_SMARTANT
typedef struct
{
    oal_uint8   uc_ant_type;
    oal_uint8   auc_resv[3];
    oal_uint32  ul_last_ant_change_time_ms;
    oal_uint32  ul_ant_change_number;
    oal_uint32  ul_main_ant_time_s;
    oal_uint32  ul_aux_ant_time_s;
    oal_uint32  ul_total_time_s;
}wal_atcmdsrv_ant_info_stru;
#endif
/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_SMARTANT
extern wal_atcmdsrv_ant_info_stru g_st_atcmdsrv_ant_info;
#endif

extern oal_uint32  g_pd_bss_expire_time_etc;

/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  7 STRUCT����
*****************************************************************************/
struct wal_atcmdsrv_wifi_connect_info
{
    atcmdsrv_wifi_conn_info_enum_uint8 en_status;
    oal_uint8 auc_ssid[WLAN_SSID_MAX_LEN];
    oal_uint8 auc_bssid[WLAN_MAC_ADDR_LEN];
    oal_int8  c_rssi;
};

struct wal_atcmdsrv_wifi_tx_power_range
{
    oal_uint8  uc_min;
    oal_uint8  uc_max;
};


/* 1102 ʹ��atcmdsrv �·����� */
typedef struct wal_atcmdsrv_wifi_priv_cmd_etc {
    /* У��λ,ȡֵ1102,������ƽ̨������ */
    oal_int32 l_verify;
    oal_int32   ul_cmd;                                          /* ����� */
    union
    {
    oal_int32 l_freq;
    oal_int32 l_userpow;
    oal_int32 l_pow;
    oal_int32 l_mode;
    oal_int32 l_datarate;
    oal_int32 l_bandwidth;
    oal_int32 l_awalys_tx;
    oal_int32 l_fem_pa_status;
    oal_int32 l_awalys_rx;
    oal_int32 l_rx_pkcg;
    oal_int32 l_pm_switch;
    oal_int32 l_rx_rssi;
    oal_int32 l_chipcheck_result;
    oal_uint64 ull_chipcheck_time;
    oal_int32 l_uart_loop_set;
    oal_int32 l_wifi_chan_loop_set;
    oal_int32 l_efuse_check_result;
    oal_int32 l_set_ant;
    oal_int32 l_upc_code;
    oal_int32 l_pin_status;
    oal_int32 l_pmu_status;
    oal_int32 l_mimo_channel;
    oal_uint8 auc_mac_addr[WAL_ATCMDSRV_IOCTL_MAC_LEN];         /* 6 */
    oal_int8  auc_dbb[WAL_ATCMDSRV_IOCTL_DBB_LEN];              /* 12 */
    oal_uint8 auc_caldata[WAL_ATCMDSRV_NV_WINVRAM_LENGTH];      /* 104 */
    oal_uint16 die_id[WAL_ATCMDSRV_DIE_ID_LENGTH];      /*  16 */
    oal_int8  auc_country_code[WAL_ATCMDSRV_IOCTL_COUNTRY_LEN]; /* 3 */
    oal_int8  ac_dp_init[DP_INIT_FLAG_NUM + DP_INIT_TWO_CORE_NUM];
    oal_int8  ac_pd_tran_param[WAL_ATCMDSRV_NV_WINVRAM_LENGTH];
    oal_int16 s_wkup_io_status;
    oal_int16 s_pcie_status;
    oal_int16 s_pcie_sdio_set;
    oal_int8  c_dyn_interval[WAL_ATCMDSRV_IOCTL_DYN_INTV_LEN];
    oal_int32 l_pt_set;
    oal_int32 l_tas_ant_set;
    oal_int32 l_runingtest_mode;
    oal_int32 l_selfcali_interval_set;
    oal_uint32 ul_bss_expire_age;                               /* ����ɨ�����ϻ�ʱ�� */
    struct wal_atcmdsrv_wifi_connect_info st_connect_info;      /* WiFi ������Ϣ */
    }pri_data;

}wal_atcmdsrv_wifi_priv_cmd_stru;
/********************************************************************************************
*****************************EFUSE�쳣���λ��˵��*******************************************

         λ������               λ����ʼλ  λ��������λ��ӳ���ϵ
    (1): DIE_ID                 [154:  0]   die_id_0,die_id1,die_id2,die_id3,die_id4
    (2): Reserve0               [159:155]   reserve0
    (3): CHIP ID                [167:160]   chip_id
    (4): Reserve1               [170:168]   reserve1
    (5): CHIP FUNCTION Value    [202:171]   chip_function_value_low,chip_function_value_high
    (6): ADC                    [206:203]   adc
    (7): Reserve2               [207:207]   reserve2
    (8): BCPU                   [208:208]   bcpu
    (9): Reserve3               [227:209]   reserve3_low,reserve3_high
    (10):PMU TRIM Value         [247:228]   pmu_trim_value
    (11):NFC PUN TRIM Value     [253:248]   nfc_pmu_trim_value
    (12):Reserve4               [255:254]   reserve4
*********************************************************************************************/
/* Note: 1103 has 512bits efuse, need to adapt this stuct */
typedef struct efuse_8_9
{
    oal_uint32 rsvd_0:3;
    oal_uint32 leakage:7;
    oal_uint32 efuse_rd_8:6;
    oal_uint32 efuse_rd_9:16;
}efuse_8_9_reg;
typedef struct efuse_10_11
{
    oal_uint32 chip_id:8;
    oal_uint32 chip_ver:3;
    oal_uint32 wifi:1;
    oal_uint32 bt:1;
    oal_uint32 fm:1;
    oal_uint32 gnss:1;
    oal_uint32 ir:1;
    oal_uint32 nfc:1;
    oal_uint32 zigbee:1;
    oal_uint32 rsvd_0:14;
}efuse_10_11_reg;
typedef struct efuse_16_17
{
    oal_uint32 pcs_common_clocks:1;
    oal_uint32 vreg_bypass:1;
    oal_uint32 phy_los_bias:3;
    oal_uint32 phy_rx0_eq:3;
    oal_uint32 phy_los_lvl:5;
    oal_uint32 phy_tx_vboost_lvl:3;
    oal_uint32 phy_ref_clkdiv2:1;
    oal_uint32 pcs_tx_gen:6;
    oal_uint32 phy_tx0_term_ofst:5;
    oal_uint32 phy_mpll_multi:4;
}efuse_16_17_reg;
typedef struct efuse_18_19
{
    oal_uint32 phy_mpll_multi:3;
    oal_uint32 pcs_tx_swing_full:7;
    oal_uint32 pcst_tx_dee_gen1:6;
    oal_uint32 pcs_tx_dee_gen2:6;
    oal_uint32 pcs_tx_swing_low:7;
    oal_uint32 pcie_efuse_sel:1;
    oal_uint32 rsvd0:2;
}efuse_18_19_reg;
typedef struct efuse_20_21
{
    oal_uint32 ibas_leak_trimq_c0:2;
    oal_uint32 ibas_leak_trimi_c0:2;
    oal_uint32 qrefv0:5;
    oal_uint32 irefv0:5;
    oal_uint32 rsvd0:2;
    oal_uint32 ibas_leak_trimq_c1:2;
    oal_uint32 ibas_leak_trimi_c1:2;
    oal_uint32 qrefv1:5;
    oal_uint32 irefv1:5;
    oal_uint32 rsvd1:2;
}efuse_20_21_reg;
typedef struct efuse_22_23
{
    oal_uint32 irefv:5;
    oal_uint32 trim_i:2;
    oal_uint32 trim_q:2;
    oal_uint32 rsvd0:7;
    oal_uint32 d_2g_c0:8;
    oal_uint32 g_2g_c0:6;
    oal_uint32 rsvd1:2;
}efuse_22_23_reg;
typedef struct efuse_24_25
{
    oal_uint32 d_5g_c0:8;
    oal_uint32 g_5g_c0:6;
    oal_uint32 rsvd0:2;
    oal_uint32 d_2g_c1:8;
    oal_uint32 g_2g_c1:6;
    oal_uint32 rsvd1:2;
}efuse_24_25_reg;
typedef struct efuse_26_27
{
    oal_uint32 d_5g_c1:8;
    oal_uint32 g_5g_c1:6;
    oal_uint32 rsvd0:2;
    oal_uint32 efuse_rd_27:16;
}efuse_26_27_reg;
typedef struct efuse_28_29
{
    oal_uint32 bbldo0_c0_trim:3;
    oal_uint32 bbldo0_c1_trim:3;
    oal_uint32 bbldo1_c0_trim:3;
    oal_uint32 bbldo1_c1_trim:3;
    oal_uint32 bbldo3_timr:3;
    oal_uint32 rsvd0:1;
    oal_uint32 bbldo4_trim:3;
    oal_uint32 rsvd1:5;
    oal_uint32 ir_trim:4;
    oal_uint32 rsvd2:4;
}efuse_28_29_reg;

typedef struct efuse_30_31
{
    oal_uint32 rsvd0:5;
    oal_uint32 abb_bbldo2_trim:3;
    oal_uint32 pmu_ref_bg_trim2_1:8;
    oal_uint32 pmu_ref_bg_trim2_2:4;
    oal_uint32 pmu_ref_bg_trim1:12;
}efuse_30_31_reg;

typedef struct efuse_bits
{
    oal_uint32 efuse_rd_0_1;
    oal_uint32 efuse_rd_2_3;
    oal_uint32 efuse_rd_4_5;
    oal_uint32 efuse_rd_6_7;
    efuse_8_9_reg efuse_rd_8_9;
    efuse_10_11_reg efuse_rd_10_11;
    oal_uint32 efuse_rd_12_13;
    oal_uint32 efuse_rd_14_15;
    efuse_16_17_reg efuse_rd_16_17;
    efuse_18_19_reg efuse_rd_18_19;
    efuse_20_21_reg efuse_rd_20_21;
    efuse_22_23_reg efuse_rd_22_23;
    efuse_24_25_reg efuse_rd_24_25;
    efuse_26_27_reg efuse_rd_26_27;
    efuse_28_29_reg efuse_rd_28_29;
    efuse_30_31_reg efuse_rd_30_31;

}wal_efuse_bits;
/************************** end ***************************/

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/

extern oal_int32 wal_atcmdsrv_wifi_priv_cmd_etc(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, oal_int32 ul_cmd);
extern oal_int32 wal_atcmsrv_ioctl_get_hw_status_etc(oal_net_device_stru *pst_net_dev, oal_int32 *pl_fem_pa_status);
extern oal_void  wal_atcmsrv_ioctl_get_fem_pa_status_etc(oal_net_device_stru *pst_net_dev, oal_int32 *pl_fem_pa_status);
extern oal_int32 wlan_device_mem_check_etc(oal_int32 l_runing_test_mode);
extern oal_int32 wlan_device_mem_check_result_etc(unsigned long long *time);
extern oal_int32 conn_test_uart_loop_etc(char *param);
extern oal_int32 conn_test_wifi_chan_loop(char *param);
extern oal_int32 conn_test_hcc_chann_switch(char* new_dev);
extern oal_int32 hwifi_fetch_ori_caldata_etc(oal_uint8* auc_caldata, oal_int32 l_nvm_len);
extern oal_int32 hwifi_config_init_etc(oal_int32);
extern oal_uint32 wal_regdomain_get_channel_5g_etc(oal_uint32 ul_start_freq, oal_uint32 ul_end_freq);
extern oal_int32  hi1103_pcie_ip_test(oal_int32 test_count);
extern oal_int32 hi1103_dev_io_test(void);
extern oal_void hmac_dump_cali_result_etc(oal_void);
#endif

#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined(_PRE_E5_722_PLATFORM) || defined(_PRE_CPE_711_PLATFORM) || defined(_PRE_CPE_722_PLATFORM))
typedef oal_int32 (*hipriv_entry_t)(void *, void*, void *);
extern oal_int32 reg_at_hipriv_entry(hipriv_entry_t hipriv_entry);
extern oal_int32 unreg_at_hipriv_entry(hipriv_entry_t hipriv_entry);
#endif

#ifdef _PRE_WLAN_FEATURE_11D
extern oal_int32  wal_regdomain_update_for_dfs_etc(oal_net_device_stru *pst_net_dev, oal_int8 *pc_country);
extern oal_int32  wal_regdomain_update_etc(oal_net_device_stru *pst_net_dev, oal_int8 *pc_country);
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) && defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST)
extern oal_int32 wal_atcmdsrv_wifi_priv_cmd_etc(oal_int8 *ac_dev_name, oal_int32 ul_cmd, oal_uint8 *pc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_SMARTANT
typedef struct
{
    oal_uint8   uc_ant_type;
    oal_uint8   auc_resv[3];
    oal_uint32  ul_last_ant_change_time_ms;
    oal_uint32  ul_ant_change_number;
    oal_uint32  ul_main_ant_time_s;
    oal_uint32  ul_aux_ant_time_s;
    oal_uint32  ul_total_time_s;
}wal_atcmdsrv_ant_info_stru;

extern wal_atcmdsrv_ant_info_stru g_st_atcmdsrv_ant_info;

#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of wal_linux_ioctl.h */

