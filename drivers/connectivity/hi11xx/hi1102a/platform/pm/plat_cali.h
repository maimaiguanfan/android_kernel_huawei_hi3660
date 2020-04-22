

#ifndef __PLAT_CALI_H__
#define __PLAT_CALI_H__

#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif

/*****************************************************************************
  1 头文件包含
*****************************************************************************/

#include "oal_types.h"
#include "oal_util.h"

/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define OAL_2G_CHANNEL_NUM         (13)
#define OAL_5G_20M_CHANNEL_NUM     (7)
#define OAL_5G_80M_CHANNEL_NUM     (7)
#define OAL_5G_CHANNEL_NUM         (OAL_5G_20M_CHANNEL_NUM + OAL_5G_80M_CHANNEL_NUM)
#define OAL_5G_DEVICE_CHANNEL_NUM  (7)
#define OAL_CALI_HCC_BUF_NUM       (3)
#define OAL_CALI_HCC_BUF_SIZE      (1500)
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
#define OAL_BT_RF_FREQ_NUM            (79)// BT RF channel number
#define OAL_BT_RXDC_CALI_NUM          (7) // BT Rx DC calibration LNA level number
#define OAL_BT_TXCAP_CALI_NUM         (3) // BT Tx Capacitor calibration channel number
#define OAL_BT_TXPWR_CALI_NUM         (8) // BT Tx Power calibration channel number
#define OAL_BT_FITTING_PARAM_NUM      (3) // BT curve fitting prarameter number
#else
#define OAL_BT_RF_FEQ_NUM          (79)            /* total Rf frequency number */
#define OAL_BT_CHANNEL_NUM         (8)            /* total Rf frequency number */
#define OAL_BT_POWER_CALI_CHANNEL_NUM         (3)
#endif

#define WIFI_2_4G_ONLY              (0x2424)
#define SYS_EXCEP_REBOOT            (0xC7C7)
#define OAL_CALI_PARAM_ADDITION_LEN (8)
#define OAL_5G_IQ_CALI_TONE_NUM         (8)
#define CHECK_5G_ENABLE             "band_5g_enable"
#ifdef _PRE_WLAN_NEW_RXDC
#define HI1102_CALI_RXDC_GAIN_LVL_NUM      (8)      /* rx dc补偿值档位数目 */
#define HI1102_CALI_RXDC_COMP_SHIFT_NUM    (16)     /* rx dc补偿寄存器移位 */
#endif

#ifdef _PRE_WLAN_NEW_TXDC
#define HI1102_CALI_TXDC_GAIN_LVL_NUM      (4)      /* Tx dc补偿值档位数目 */
#endif

#define HI1102_CALI_TXIQ_LS_FILTER_TAP_NUM      (11)      /* Tx ls filter 抽头数 */
#define HI1102_CALI_RXIQ_LS_FILTER_TAP_NUM      (11)      /* Rx ls filter 抽头数  */
#define HI1102_CALI_IQ_TONE_NUM         8

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 全局变量定义
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/
extern oal_uint8 g_uc_netdev_is_open;


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

typedef struct
{
#ifdef _PRE_WLAN_NEW_RXDC
    oal_uint16  aus_analog_rxdc_cmp[HI1102_CALI_RXDC_GAIN_LVL_NUM];
    oal_uint16  aus_analog_rxdc_extlna_cmp[HI1102_CALI_RXDC_GAIN_LVL_NUM];
#else
    oal_uint16  us_analog_rxdc_cmp;
    oal_uint8   auc_reserve[2];
#endif
    oal_uint16  us_digital_rxdc_cmp_i;
    oal_uint16  us_digital_rxdc_cmp_q;
    oal_int16   s_cali_temperature;
    oal_uint8   auc_reserve1[2];
}oal_rx_dc_comp_val_stru;

typedef struct
{
    oal_uint8   upc_ppa_cmp;
    oal_int8    ac_atx_pwr_cmp;
    oal_uint8   dtx_pwr_cmp;
    oal_uint8   auc_reserve[1];
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    oal_int16   s_2g_tx_power_dc_i;
    oal_int16   s_2g_tx_power_dc_q;
#endif
}oal_2G_tx_power_comp_val_stru;

typedef struct
{
    oal_uint8   upc_ppa_cmp;
    oal_uint8   upc_mx_cmp;
    oal_int8    ac_atx_pwr_cmp;
    oal_uint8   dtx_pwr_cmp;
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    oal_int16   s_5g_tx_power_dc_q;
    oal_uint8   auc_reserve[2];
#endif
}oal_5G_tx_power_comp_val_stru;

typedef struct
{
    oal_uint16  us_txdc_cmp_i;
    oal_uint16  us_txdc_cmp_q;
}oal_txdc_comp_val_stru;

typedef struct
{
    oal_uint8   uc_ppf_val;
    oal_uint8   auc_reserve[3];
}oal_ppf_comp_val_stru;

// TODO:  暂时放在这，02A 2G 备用
typedef struct
{
    oal_uint16   us_txiq_cmp_p;
    oal_uint16   us_txiq_cmp_e;
}oal_txiq_comp_val_stru;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
#ifdef _PRE_WLAN_NEW_IQ
typedef struct
{
    oal_int32  l_txiq_cmp_alpha;
    oal_int32  l_txiq_cmp_beta;
    oal_int16  as_txiq_comp_ls_filt[HI1102_CALI_TXIQ_LS_FILTER_TAP_NUM];
    oal_uint8  auc_resev[2];
}hi1102_new_txiq_time_comp_val_stru;

typedef struct
{
    oal_int32  l_rxiq_cmp_u1;
    oal_int32  l_rxiq_cmp_u2;
    oal_int32  l_rxiq_cmp_alpha;
    oal_int32  l_rxiq_cmp_beta;
    oal_int16  as_rxiq_comp_ls_filt[HI1102_CALI_RXIQ_LS_FILTER_TAP_NUM];
    oal_uint8  auc_resev[2];
}hi1102_new_rxiq_comp_val_stru;

#endif
#else
#ifdef _PRE_WLAN_NEW_IQ
typedef struct
{
    oal_uint32  ul_rxiq_cmp_u1;
    oal_uint32  ul_rxiq_cmp_u2;
    oal_uint32  ul_rxiq_cmp_alpha;
    oal_uint32  ul_rxiq_cmp_beta;
}oal_new_rxiq_comp_val_stru;


typedef struct
{
    oal_uint32  aul_alpha_reg_val[OAL_5G_IQ_CALI_TONE_NUM];
    oal_uint32  aul_beta_reg_val[OAL_5G_IQ_CALI_TONE_NUM];
    oal_uint32  aul_alpha0_reg_val[OAL_5G_IQ_CALI_TONE_NUM];
    oal_uint32  aul_beta0_reg_val[OAL_5G_IQ_CALI_TONE_NUM];
}oal_new_txiq_comp_val_stru;
#endif
#endif

typedef struct
{
    oal_rx_dc_comp_val_stru        g_st_cali_rx_dc_cmp_2G;
    oal_2G_tx_power_comp_val_stru  g_st_cali_tx_power_cmp_2G;
#ifdef _PRE_WLAN_NEW_TXDC
    oal_txdc_comp_val_stru         g_st_txdc_cmp_val[HI1102_CALI_TXDC_GAIN_LVL_NUM];
#else
    oal_txdc_comp_val_stru         g_st_txdc_cmp_val;
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST) && defined (_PRE_WLAN_NEW_IQ)
    hi1102_new_txiq_time_comp_val_stru st_new_txiq_time_cmp_val;
    hi1102_new_rxiq_comp_val_stru      st_new_rxiq_cmp_val;
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST) || defined (_PRE_WLAN_TXIQ_BASE_IQ_DETECTOR)
    oal_txiq_comp_val_stru         g_st_txiq_cmp_val_2G;
#endif
}oal_2Gcali_param_stru;

typedef struct
{
    oal_rx_dc_comp_val_stru        g_st_cali_rx_dc_cmp_5G;
    oal_5G_tx_power_comp_val_stru  g_st_cali_tx_power_cmp_5G;
    oal_ppf_comp_val_stru          g_st_ppf_cmp_val;
#ifdef _PRE_WLAN_NEW_TXDC
    oal_txdc_comp_val_stru         g_st_txdc_cmp_val[HI1102_CALI_TXDC_GAIN_LVL_NUM];
#else
    oal_txdc_comp_val_stru         g_st_txdc_cmp_val;
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST) && defined (_PRE_WLAN_NEW_IQ)
    hi1102_new_txiq_time_comp_val_stru st_new_txiq_time_cmp_val;
    hi1102_new_rxiq_comp_val_stru      st_new_rxiq_cmp_val;
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST) || defined (_PRE_WLAN_TXIQ_BASE_IQ_DETECTOR)
    oal_txiq_comp_val_stru         g_st_txiq_cmp_val_5G;
#endif

}oal_5Gcali_param_stru;

typedef struct
{
    oal_2Gcali_param_stru st_2Gcali_param[OAL_2G_CHANNEL_NUM];
    oal_5Gcali_param_stru st_5Gcali_param[OAL_5G_CHANNEL_NUM];
}oal_wifi_cali_param_stru;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
// MAX size: BFGX_BT_CALI_DATA_SIZE
typedef struct
{
    // BT PLL compensation
    oal_uint8  auc_bt_pll_tx_comp[OAL_BT_RF_FREQ_NUM];            //  79 bytes
    oal_uint8  auc_bt_pll_rx_comp[OAL_BT_RF_FREQ_NUM];            // 158 bytes
    // BT Rx IQ compensation
    oal_uint16 us_bt_rxiq_p_comp;                                 // 160 bytes
    oal_uint16 us_bt_rxiq_e_comp;                                 // 162 bytes
    // BT Tx Capacitor compensation
    oal_uint8  auc_bt_upc_ppa_csw_comp[OAL_BT_TXCAP_CALI_NUM];    // 165 bytes
    oal_uint8  uc_bt_dtc_code;                                    // 166 bytes
    // BT Tx Power compensation
    oal_uint8  auc_bt_upc_ppa_gc_comp[OAL_BT_TXPWR_CALI_NUM];     // 174 bytes
    oal_uint8  auc_bt_txpwr_digital_comp[OAL_BT_TXPWR_CALI_NUM];  // 182 bytes
    oal_uint8  auc_bt_txpwr_cali_freq[OAL_BT_TXPWR_CALI_NUM];     // 190 bytes
    oal_uint8  uc_bt_txpwr_cali_freq_num;                         // 191 bytes
    oal_uint8  auc_reserve1[1];                                   // 192 bytes
    // BT Tx LO compensation
    oal_uint16 us_bt_txdc_i_comp;                                 // 194 bytes
    oal_uint16 us_bt_txdc_q_comp;                                 // 196 bytes
    // BT Tx IQ compensation
    oal_uint16 us_bt_txiq_p_comp;                                 // 198 bytes
    oal_uint16 us_bt_txiq_e_comp;                                 // 200 bytes
    // BT Rx DC compensation
    oal_uint16 us_bt_rxdc_analog_comp;                            // 202 bytes
    oal_uint8  auc_reserve2[12];                                  // 214 bytes
    oal_uint16 us_bt_rxdc_digital_i_comp;                         // 216 bytes
    oal_uint16 us_bt_rxdc_digital_q_comp;                         // 218 bytes

    // BT Dynamic Tx Power calibration
    oal_int16  s_txpwr_ppa_vdet_dc;                               // 220 bytes
    oal_int16  s_txpwr_pa_vdet_dc;                                // 222 bytes
    oal_int8   ac_txpwr_dpn[OAL_BT_RF_FREQ_NUM];                  // 301 bytes
    oal_uint8  auc_reserve3[1];                                   // 302 bytes
    oal_int16  s_gm_dB10;                                         // 304 bytes
    oal_int32  al_pwr_fit_params[OAL_BT_FITTING_PARAM_NUM];       // 308 bytes
    oal_int16  s_base_power;                                      // 310 bytes

    // BT calibration temperature
    oal_int16  s_cali_temperature;                                // 312 bytes

    oal_uint8  uc_cali_flag;                                      // 313 bytes
}oal_bt_cali_comp_stru;
#else
typedef struct
{
    oal_uint16 us_bt_rx_dc_comp;
    oal_uint8  auc_reserve0[2];
    oal_uint8  auc_upc_ppa_cmp[OAL_BT_POWER_CALI_CHANNEL_NUM];
    oal_uint8  auc_reserve1[1];
    oal_int8   ac_atx_pwr_cmp[OAL_BT_POWER_CALI_CHANNEL_NUM];
    oal_uint8  auc_reserve2[1];
    oal_uint8  auc_bt_pll_tx_comp[OAL_BT_RF_FEQ_NUM];
    oal_uint8  auc_reserve3[1];
    oal_uint8  auc_bt_pll_rx_comp[OAL_BT_RF_FEQ_NUM];
    oal_uint8  auc_reserve4[1];
    oal_uint32   uc_cali_flag;
    oal_uint16  us_txdc_cmp_i;
    oal_uint16  us_txdc_cmp_q;
    oal_uint16  us_txdc_cmp_iq_p;
    oal_uint16  us_txdc_cmp_iq_e;
    oal_int8   ac_bt_atx_pwr_cmp[OAL_BT_CHANNEL_NUM];
    oal_int8   ac_bt_atx_pwr_fre[OAL_BT_CHANNEL_NUM];
    oal_uint16  ac_bt_pwr_fre_num;
    oal_uint8  auc_reserve5[2];
}oal_bt_cali_comp_stru;
#endif

typedef struct
{
    oal_uint16  ul_cali_time;
    oal_uint16  bit_temperature     : 3,
                uc_5g_chan_idx1     : 5,
                uc_5g_chan_idx2     : 5,
                en_update_bt        : 4;
}oal_update_cali_channel_stru;

typedef struct
{
    oal_uint8              g_uc_rc_cmp_code;
    oal_uint8              g_uc_r_cmp_code;
    oal_uint8              g_uc_c_cmp_code;
    oal_bool_enum_uint8    en_save_all;
    oal_bt_cali_comp_stru  st_bt_cali_comp;
}oal_bfgn_cali_param_stru;

typedef struct
{
    oal_uint32                   ul_dog_tag;
    oal_2Gcali_param_stru        ast_2Gcali_param[OAL_2G_CHANNEL_NUM];
    oal_5Gcali_param_stru        ast_5Gcali_param[OAL_5G_CHANNEL_NUM];
    oal_ppf_comp_val_stru        st_165chan_ppf_comp;
    oal_update_cali_channel_stru st_cali_update_info;
    oal_uint32                   ul_check_hw_status;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST) && defined (_PRE_WLAN_NEW_IQ)
    oal_new_rxiq_comp_val_stru   ast_new_rxiq_cmp_val_5G[OAL_5G_80M_CHANNEL_NUM];
    oal_new_rxiq_comp_val_stru   ast_new_rxiq_cmp_val_5G_40M[OAL_5G_80M_CHANNEL_NUM];
    oal_new_txiq_comp_val_stru   ast_new_txiq_cmp_val_5G[OAL_5G_80M_CHANNEL_NUM];
#endif

    oal_bfgn_cali_param_stru     st_bfgn_cali_data;

}oal_cali_param_stru;

typedef struct
{
    oal_uint32 ul_wifi_2_4g_only;
    oal_uint32 ul_excep_reboot;
    oal_uint32 ul_reserve[OAL_CALI_PARAM_ADDITION_LEN];
}oal_cali_param_addition_stru;


/*****************************************************************************
  8 UNION定义
*****************************************************************************/
#define OAL_WIFI_CALI_DATA_DOWNLOAD_LEN (OAL_SIZEOF(oal_cali_param_stru) - 4 - OAL_SIZEOF(oal_bfgn_cali_param_stru) + 4)
#define OAL_WIFI_CALI_DATA_UPLOAD_LEN (OAL_SIZEOF(oal_cali_param_stru) - 4)


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_int32 get_cali_count(oal_uint32 *count);
extern oal_int32 get_bfgx_cali_data(oal_uint8 *buf, oal_uint32 *len, oal_uint32 buf_len);
extern void *get_cali_data_buf_addr(void);
extern oal_int32 cali_data_buf_malloc(void);
extern void  cali_data_buf_free(void);


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of plat_cali.h */










