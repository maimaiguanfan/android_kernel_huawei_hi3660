

#ifndef __HISI_CUSTOMIZE_WIFI_H__
#define __HISI_CUSTOMIZE_WIFI_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "mac_vap.h"
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define NVRAM_PARAMS_ARRAY      "nvram_params"

#define MAC_LEN                 6
#define NV_WLAN_NUM             193
#define NV_WLAN_VALID_SIZE      12
#define MAC2STR(a)              (a)[0], "**", "**", "**", (a)[4], (a)[5]
#define MACFMT                  "%02x:%s:%s:%s:%02x:%02x"
#define MAC2STR_ALL(a)          (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACFMT_ALL              "%02x:%02x:%02x:%02x:%02x:%02x"
#define CUS_TAG_INI                     0x11
#define CUS_TAG_DTS                     0x12
#define CUS_TAG_NV                      0x13
#define CUS_TAG_PRIV_INI                0x14
#define CUS_TAG_PRO_LINE_INI            0x15
#define CALI_TXPWR_PA_DC_REF_MAX        (10000)
#define CALI_TXPWR_DPN_MIN              (-40)// Unit: 0.1dB. DPN ranges from -3dB to 3dB normally.
#define CALI_TXPWR_DPN_MAX              (40)
#define CALI_TXPWR_FREQ_MIN             (0)
#define CALI_TXPWR_FREQ_MAX             (78)
#define CHN_EST_CTRL_EVB                (0x3C192240)
#define CHN_EST_CTRL_MATE7              (0x3C193240)
#define CHN_EST_CTRL_FPGA               (0x3C19A243)
#define PHY_POWER_REF_5G_EVB            (0x04D8EC)
#define PHY_POWER_REF_5G_MT7            (0xF6CADE)
#define RTC_CLK_FREQ_MIN                (32000)
#define RTC_CLK_FREQ_MAX                (33000)
#define RF_LINE_TXRX_GAIN_DB_2G_MIN     (-100)
#define RF_LINE_TXRX_GAIN_DB_5G_MIN     (-48)
#define PSD_THRESHOLD_MIN               (-15)
#define PSD_THRESHOLD_MAX               (-10)
#define LNA_GAIN_DB_MIN                 (-10)
#define LNA_GAIN_DB_MAX                 (80)
#define MAX_TXPOWER_MIN                 (130)                               /* 最大发送功率的最小有效值:130 13.0dbm */
#define MAX_TXPOWER_MAX                 (238)                               /* 最大发送功率的最大有效值:238 23.8dbm */
#define NUM_OF_BAND_EDGE_LIMIT          (6)                                 /* FCC边带认证参数个数 */
#define MAX_DBB_SCALE                   (0xEE)                              /* DBB SCALE最大有效值 */
#define TX_RATIO_MAX                    (2000)                              /* tx占空比的最大有效值 */
#define TX_PWR_COMP_VAL_MAX             (50)                                /* 发射功率补偿值的最大有效值 */
#define MORE_PWR_MAX                    (50)                                /* 根据温度额外补偿的发射功率的最大有效值 */
#define COUNTRY_CODE_LEN                (3)                                 /* 国家码位数 */
#define MAX_COUNTRY_COUNT               (300)                               /* 支持定制的国家的最大个数 */
#define DELTA_CCA_ED_HIGH_TH_RANGE      15     /* δ调整上限，最大向上或向下调整15dB */
#define DY_CALI_PARAMS_LEN              (104)   /* 动态校准定制项最大长度 */
#define DY_2G_CALI_PARAMS_NUM           (4)    /* 动态校准参数个数,2.4g */
#define DY_CALI_PARAMS_NUM              (17)   /* 动态校准参数个数,2.4g 4个(ofdm 20/40 11b cw),5g 5*2(high & low)个band, BT 2个*/
#define DY_CALI_PARAMS_NUM_WLAN         (15)   /* 动态校准参数个数,2.4g 4个(ofdm 20/40 11b cw),5g 5*2(high & low)个band */
#define DY_CALI_PARAMS_BASE_NUM         (9)    /* 动态校准参数个数,2.4g 4个(ofdm 20/40 11b cw),5g 5(high)个band */
#define DY_CALI_PARAMS_TIMES            (3)    /* 动态校准参数二次项系数个数 */
#define DY_CALI_NUM_5G_BAND             (5)    /* 动态校准5g band1 2&3 4&5 6 7 */
#define DY_CALI_FIT_PRECISION_A1        (6)
#define DY_CALI_FIT_PRECISION_A0        (16)
#define MAC_NUM_2G_BAND                 3   /* 2g band数 */
#define MAC_NUM_5G_BAND                 7   /* 5g band数 */
#define MAC_2G_CHANNEL_NUM              13
#define DY_CALI_DPN_PARAMS_NUM          4   /* 定制化动态校准2.4G DPN参数个数11b OFDM20/40 CW */
#define CUS_NUM_5G_BW                   4   /* 定制化5g带宽数 */
/* 读取NVRAM MASK */
#define HI1102A_CUST_READ_NVRAM_MASK         BIT(4)

#define CUS_NUM_OF_SAR_LVL              20   /* 定制化降SAR档位数 */
#define CUS_NUM_OF_SAR_PARAMS           8   /* 定制化降SAR参数 5G_BAND1~7 2.4G */
#define CUS_MIN_OF_SAR_VAL              (0x28) /* 定制化降SAR最小值 4dbm */
#define CUS_NUM_OF_SAR_PER_BAND_PAR_NUM   5   /* 每个band降SAR定制项数 */
#define CUS_NUM_OF_SAR_ONE_PARAM_NUM    4

#define INIT_NVM_BASE_TXPWR_2G          (190) /* 单位: 0.1 dBm */
#define INIT_NVM_BASE_TXPWR_5G          (170) /* 单位: 0.1 dBm */

#define WLAN_TCP_ACK_FILTER_TH_HIGH                          50  /* Mbps  */
#define WLAN_TCP_ACK_FILTER_TH_LOW                           20  /* Mbps  */

#define WLAN_AMSDU_AMPDU_THROUGHPUT_THRESHOLD_HIGH          150  /* Mb/s  */
#define WLAN_AMSDU_AMPDU_THROUGHPUT_THRESHOLD_LOW           100  /* Mb/s */

#define WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH          90  /* Mb/s */
#define WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW           30  /* Mb/s */
#define WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_40M      150  /* Mb/s */
#define WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW_40M       70  /* Mb/s */
#define WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_80M      250  /* Mb/s */
#define WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW_80M       150  /* Mb/s */
#define WLAN_DYN_BYPASS_EXTLNA_THROUGHPUT_THRESHOLD_HIGH    100  /* Mb/s */
#define WLAN_DYN_BYPASS_EXTLNA_THROUGHPUT_THRESHOLD_LOW     50  /* Mb/s */

#define WLAN_SMALL_AMSDU_THROUGHPUT_THRESHOLD_HIGH          150  /* Mb/s */
#define WLAN_SMALL_AMSDU_THROUGHPUT_THRESHOLD_LOW           100  /* Mb/s */
#define WLAN_SMALL_AMSDU_PPS_THRESHOLD_HIGH                 12500  /* pps */
#define WLAN_SMALL_AMSDU_PPS_THRESHOLD_LOW                  2500  /* pps */

#define WLAN_BINDCPU_DEFAULT_MASK    0xc0
#define WLAN_TX_BUSY_CPU_THROUGHT    150
#define WLAN_RX_BUSY_CPU_THROUGHT    100
#define WLAN_TX_IDLE_CPU_THROUGHT    100
#define WLAN_RX_IDLE_CPU_THROUGHT    50
/*
 * 计算绝对值
 */
#define CUS_ABS(val)                                ((val) > 0 ? (val) : -(val))
/*
 * 判断CCA能量门限调整值是否超出范围
 * 最大调整幅度:DELTA_CCA_ED_HIGH_TH_RANGE
 */
#define CUS_DELTA_CCA_ED_HIGH_TH_OUT_OF_RANGE(val)  (CUS_ABS(val) > DELTA_CCA_ED_HIGH_TH_RANGE ? 1 : 0)

/* 根据产线delt power更新系数 */
#define CUS_FLUSH_NV_RATIO_BY_DELT_POW(_l_pow_par2, _l_pow_par1, _l_pow_par0, _s_delt_power) \
do {(_l_pow_par0) = (oal_int32)((((oal_int32)(_s_delt_power)*(_s_delt_power)*(_l_pow_par2))+(((oal_int32)(_l_pow_par1)<<DY_CALI_FIT_PRECISION_A1)*(_s_delt_power)) \
                                                               +((oal_int32)(_l_pow_par0)<<DY_CALI_FIT_PRECISION_A0))>>DY_CALI_FIT_PRECISION_A0);                    \
    (_l_pow_par1) = (oal_int32)((((oal_int32)(_s_delt_power)*(_l_pow_par2)*2)+((oal_int32)(_l_pow_par1)<<DY_CALI_FIT_PRECISION_A1))>>DY_CALI_FIT_PRECISION_A1);      \
    }while(0)

#define HWIFI_GET_5G_PRO_LINE_DELT_POW_PER_BAND(_ps_nv_params, _ps_ini_params) \
    (oal_int16)(((oal_int32)(((_ps_nv_params)[1]) - ((_ps_ini_params)[1]))<<DY_CALI_FIT_PRECISION_A1)/(2*((_ps_nv_params)[0])))

#define HWIFI_DYN_CALI_GET_EXTRE_POINT(_ps_ini_params)  \
        (-(((_ps_ini_params)[1]) << DY_CALI_FIT_PRECISION_A1) / (2*((_ps_ini_params)[0])))

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* NV map idx */
typedef enum
{
    HWIFI_CFG_NV_WINVRAM_NUMBER           = 340,
    HWIFI_CFG_NV_WITXNVCCK_NUMBER         = 367,
    HWIFI_CFG_NV_WITXNVBWC0_NUMBER        = 369,
    HWIFI_CFG_NV_WITXL2G5G0_NUMBER        = 384,
    HWIFI_CFG_NV_MUFREQ_2G20_C0_NUMBER    = 396,
    HWIFI_CFG_NV_MUFREQ_2G40_C0_NUMBER    = 397,
    HWIFI_CFG_NV_MUFREQ_CCK_C0_NUMBER     = 398,
}wlan_nvram_idx;


typedef enum
{
   REGDOMAIN_FCC        = 0,
   REGDOMAIN_ETSI       = 1,
   REGDOMAIN_JAPAN      = 2,
   REGDOMAIN_COMMON     = 3,

   REGDOMAIN_COUNT
} regdomain_enum;

/* 定制化 DTS CONFIG ID */
typedef enum
{
    /* 5g开关 */
    WLAN_CFG_DTS_BAND_5G_ENABLE = 0,
    /* 校准 */
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN1,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_START = WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN1,     /* 校准 2g TXPWR_REF起始配置ID */
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN2,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN3,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN4,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN5,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN6,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN7,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN8,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN9,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN10,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN11,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN12,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN13,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND1,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_START = WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND1,    /* 校准 5g TXPWR_REF起始配置ID */
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND2,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND3,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND4,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND5,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND6,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND7,
    WLAN_CFG_DTS_CALI_TONE_AMP_GRADE,

    /* rf register */
    WLAN_CFG_DTS_RF_REG117,                     //33
    WLAN_CFG_DTS_RF_FIRST = WLAN_CFG_DTS_RF_REG117,
    WLAN_CFG_DTS_RF_REG123,
    WLAN_CFG_DTS_RF_REG124,
    WLAN_CFG_DTS_RF_REG125,
    WLAN_CFG_DTS_RF_REG126,
    WLAN_CFG_DTS_RF_LAST = WLAN_CFG_DTS_RF_REG126,
    /* bt tmp */
    WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND1,    //37
    WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND2,
    WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND3,
    WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND4,
    WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND5,
    WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND6,
    WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND7,
    WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND8,
    WLAN_CFG_DTS_BT_CALI_TXPWR_NUM,          //45
    WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ1,
    WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ2,
    WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ3,
    WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ4,
    WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ5,
    WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ6,
    WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ7,
    WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ8,         //53
    WLAN_CFG_DTS_BT_CALI_INSERTION_LOSS,
    WLAN_CFG_DTS_BT_CALI_GM_CALI_EN,
    WLAN_CFG_DTS_BT_CALI_GM0_DB10,
    WLAN_CFG_DTS_BT_CALI_BASE_POWER,
    WLAN_CFG_DTS_BT_CALI_IS_DPN_CALC,
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    /* 动态校准 */
    WLAN_CFG_DTS_DYN_CALI_DSCR_ITERVL,
    /* DPN 40M 20M 11b */
    WLAN_CFG_DTS_2G_CORE0_DPN_CH1,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH2,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH3,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH4,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH5,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH6,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH7,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH8,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH9,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH10,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH11,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH12,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH13,
    WLAN_CFG_DTS_5G_CORE0_DPN_B0,
    WLAN_CFG_DTS_5G_CORE0_DPN_B1,
    WLAN_CFG_DTS_5G_CORE0_DPN_B2,
    WLAN_CFG_DTS_5G_CORE0_DPN_B3,
    WLAN_CFG_DTS_5G_CORE0_DPN_B4,
    WLAN_CFG_DTS_DYN_CALI_OPT_SWITCH,
    WLAN_CFG_DTS_DYN_CALI_GM0_DB10_AMEND,
#endif
    WLAN_CFG_DTS_5G_UPC_UP_LIMIT,
    WLAN_CFG_DTS_5G_IQ_BACKOFF_POWER,
    WLAN_CFG_DTS_5G_IQ_CALI_POWER,

    WLAN_CFG_DTS_BUTT,
}WLAN_CFG_DTS;

/* 私有定制化 PRIV CONFIG ID */
typedef enum
{
    /* 校准开关 */
    WLAN_CFG_PRIV_CALI_MASK,
    WLAN_CFG_PRIV_CALI_DATA_MASK,

    WLAN_CFG_PRIV_TEMP_PRO_THRESHOLD,
    WLAN_CFG_PRIV_TEMP_PRO_EN,
    WLAN_CFG_PRIV_TEMP_PRO_REDUCE_PWR_EN,
    WLAN_CFG_PRIV_TEMP_PRO_SAFE_TH,
    WLAN_CFG_PRIV_TEMP_PRO_OVER_TH,
    WLAN_CFG_PRIV_TEMP_PRO_PA_OFF_TH,

    WLAN_DSSS2OFDM_DBB_PWR_BO_VAL,
    WLAN_CFG_PRIV_FAST_CHECK_CNT,
    WLAN_CFG_PRIV_VOE_SWITCH,

    /* 动态bypass外置LNA方案 */
    WLAN_CFG_PRIV_DYN_BYPASS_EXTLNA,

    /* 非高优先级hcc流控类型 0:SDIO 1:GPIO */
    WLAN_CFG_PRIV_HCC_FLOWCTRL_TYPE,
    /* 5G 外置FEM的类型 */
    WLAN_CFG_PRIV_5G_EXT_FEM_TYPE,

    WLAN_CFG_PRIV_BUTT,
}WLAN_CFG_PRIV;
typedef unsigned char wlan_cfg_priv_id_uint8;


/* 定制化 INI CONFIG ID */
typedef enum
{
    /* ROAM */
    WLAN_CFG_INIT_ROAM_SWITCH = 0,
    WLAN_CFG_INIT_SCAN_ORTHOGONAL,
    WLAN_CFG_INIT_TRIGGER_B,
    WLAN_CFG_INIT_TRIGGER_A,
    WLAN_CFG_INIT_DELTA_B,
    WLAN_CFG_INIT_DELTA_A,

    /* 性能 */
    WLAN_CFG_INIT_AMPDU_TX_MAX_NUM,
    WLAN_CFG_INIT_USED_MEM_FOR_START,
    WLAN_CFG_INIT_USED_MEM_FOR_STOP,
    WLAN_CFG_INIT_RX_ACK_LIMIT,
    WLAN_CFG_INIT_SDIO_D2H_ASSEMBLE_COUNT,
    WLAN_CFG_INIT_SDIO_H2D_ASSEMBLE_COUNT,
    /* LINKLOSS */
    WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_WLAN_BT,    //9,这里不能直接赋值
    WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_WLAN_DBAC,
    WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_WLAN_NORMAL,
    /* 自动调频 */
    WLAN_CFG_INIT_PPS_THRESHOLD_LEVEL_0,
    WLAN_CFG_INIT_PPS_THRESHOLD_LEVEL_1,
    WLAN_CFG_INIT_PPS_THRESHOLD_LEVEL_2,
    WLAN_CFG_INIT_PPS_THRESHOLD_LEVEL_3,
    WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_0,
    WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_1,
    WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_2,
    WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_3,
    /* 低功耗 */
    WLAN_CFG_INIT_POWERMGMT_SWITCH,                 //28
    WLAN_CFG_INIT_PS_MODE,
    /* 可维可测 */
    WLAN_CFG_INIT_LOGLEVEL,
    /* PHY算法 */
    WLAN_CFG_INIT_CHN_EST_CTRL,
    WLAN_CFG_INIT_POWER_REF_5G,
    /* 2G RF前端 */
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND1_MULT4,    //34
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND_START = WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND1_MULT4,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND1_MULT10,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND2_MULT4,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND2_MULT10,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND3_MULT4,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND3_MULT10,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND_END = WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND3_MULT10,
    /* 5G RF前端 */
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND1_MULT4,  //44
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND_START = WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND1_MULT4,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND1_MULT10,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND2_MULT4,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND2_MULT10,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND3_MULT4,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND3_MULT10,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND4_MULT4,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND4_MULT10,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND5_MULT4,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND5_MULT10,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND6_MULT4,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND6_MULT10,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND7_MULT4,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND7_MULT10,
    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND_END = WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND7_MULT10,
    WLAN_CFG_INIT_RF_LINE_RX_GAIN_DB_5G,
    WLAN_CFG_INIT_LNA_GAIN_DB_5G,
    WLAN_CFG_INIT_RF_LINE_TX_GAIN_DB_5G,
    WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_5G,
    WLAN_CFG_INIT_EXT_PA_ISEXIST_5G,
    WLAN_CFG_INIT_EXT_LNA_ISEXIST_5G,
    WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_5G,
    WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_5G,
    /* SCAN */
    WLAN_CFG_INIT_RANDOM_MAC_ADDR_SCAN,
    /* 11AC2G */
    WLAN_CFG_INIT_11AC2G_ENABLE,                    /* 11ac2g开关 */                    //56
    WLAN_CFG_INIT_DISABLE_CAPAB_2GHT40,             /* 2ght40禁止开关 */
    WLAN_CFG_INIT_DUAL_ANTENNA_ENABLE,              /* 双天线开关 */
    WLAN_CFG_INIT_FAR_DIST_POW_GAIN_SWITCH,         /* 远距离功率增益开关 */
    WLAN_CFG_LTE_GPIO_CHECK_SWITCH,                 /* lte管脚检测开关 */
    WLAN_ATCMDSRV_LTE_ISM_PRIORITY,
    WLAN_ATCMDSRV_LTE_RX_ACT,
    WLAN_ATCMDSRV_LTE_TX_ACT,
    WLAN_CFG_INIT_FAR_DIST_DSSS_SCALE_PROMOTE_SWITCH,    /* 超远距11b 1m 2m dbb scale提升使能开关 */
    WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_2G,
    WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_2G,
    WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_5G,
    WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_5G,
#ifdef _PRE_WLAN_DOWNLOAD_PM
    WLAN_CFG_INIT_DOWNLOAD_RATE_LIMIT_PPS,
#endif
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    WLAN_CFG_INIT_AMPDU_AMSDU_SKB,
    WLAN_CFG_INIT_AMSDU_AMPDU_TH_LOW,
    WLAN_CFG_INIT_AMSDU_AMPDU_TH_HIGH,
    /* 接收ampdu+amsdu */
    WLAN_CFG_INIT_RX_AMPDU_AMSDU_SKB,
#endif
#ifdef _PRE_WLAN_TCP_OPT
    WLAN_CFG_INIT_TCP_ACK_FILTER,
    WLAN_CFG_INIT_TCP_ACK_FILTER_TH_LOW,
    WLAN_CFG_INIT_TCP_ACK_FILTER_TH_HIGH,
#endif

    WLAN_CFG_INIT_BINDCPU,
    WLAN_CFG_INIT_BINDCPU_MASK,
    WLAN_CFG_INIT_BINDCPU_TXHIGH,
    WLAN_CFG_INIT_BINDCPU_TXLOW,
    WLAN_CFG_INIT_BINDCPU_RXHIGH,
    WLAN_CFG_INIT_BINDCPU_RXLOW,

    WLAN_CFG_INIT_TX_TCP_ACK_BUF,
    WLAN_CFG_INIT_TCP_ACK_BUF_HIGH,
    WLAN_CFG_INIT_TCP_ACK_BUF_LOW,
    WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_40M,
    WLAN_CFG_INIT_TCP_ACK_BUF_LOW_40M,
    WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_80M,
    WLAN_CFG_INIT_TCP_ACK_BUF_LOW_80M,

    /* DYN_BYPASS_EXTLNA */
    WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA,
    WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_HIGH,
    WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_LOW,

    /* SMALL_AMSDU */
    WLAN_CFG_INIT_TX_SMALL_AMSDU,
    WLAN_CFG_INIT_SMALL_AMSDU_HIGH,
    WLAN_CFG_INIT_SMALL_AMSDU_LOW,
    WLAN_CFG_INIT_SMALL_AMSDU_PPS_HIGH,
    WLAN_CFG_INIT_SMALL_AMSDU_PPS_LOW,

    /* 定制化base_power */
    WLAN_CFG_INIT_TX_BASE_POWER_2P4G,
    WLAN_CFG_INIT_TX_BASE_POWER_START       = WLAN_CFG_INIT_TX_BASE_POWER_2P4G,
    WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND1,
    WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND2,
    WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND3,
    WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND4,
    WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND5,
    WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND6,
    WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND7,
    WLAN_CFG_INIT_TX_BASE_POWER_END         = WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND7,

    /* 定制化各信道信道最大功率 */
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_20M_0,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_START     = WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_20M_0,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_20M_1,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_20M_2,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_40M_0,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_40M_1,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_80M_0,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_80M_1,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_160M,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_1,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_2,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_3,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_4,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_5,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_6,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_7,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_8,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_9,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_10,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_11,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_12,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_13,
    WLAN_CFG_INIT_SIDE_BAND_TXPWR_END       = WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_13,
    WLAN_CFG_INIT_DELTA_PWR_REF_2G_20M,
    WLAN_CFG_INIT_DELTA_PWR_REF_2G_40M,
    WLAN_CFG_INIT_DELTA_PWR_REF_5G_20M,
    WLAN_CFG_INIT_DELTA_PWR_REF_5G_40M,
    WLAN_CFG_INIT_DELTA_PWR_REF_5G_80M,

    WLAN_CFG_INIT_BUTT,
}WLAN_CFG_INIT;

/* 定制化 NVRAM PARAMS INDEX */
typedef enum
{
    NVRAM_PARAMS_INDEX_0   = 0,
    NVRAM_PARAMS_INDEX_1   = 1,
    NVRAM_PARAMS_INDEX_2   = 2,
    NVRAM_PARAMS_INDEX_3   = 3,
    NVRAM_PARAMS_INDEX_4   = 4,
    NVRAM_PARAMS_INDEX_5   = 5,
    NVRAM_PARAMS_INDEX_6   = 6,
    NVRAM_PARAMS_INDEX_7   = 7,
    NVRAM_PARAMS_INDEX_8   = 8,
    NVRAM_PARAMS_INDEX_9   = 9,
    NVRAM_PARAMS_INDEX_10  = 10,
    NVRAM_PARAMS_INDEX_11  = 11,
    NVRAM_PARAMS_DELT_POWER_END = 12,
    NVRAM_PARAMS_INDEX_12  = NVRAM_PARAMS_DELT_POWER_END,
    NVRAM_PARAMS_INDEX_13  = 13,
    NVRAM_PARAMS_INDEX_14  = 14,
    NVRAM_PARAMS_INDEX_15  = 15,
    NVRAM_PARAMS_INDEX_16  = 16,
    NVRAM_PARAMS_INDEX_17  = 17,
    NVRAM_PARAMS_INDEX_18  = 18,
    NVRAM_PARAMS_INDEX_19  = 19,
    NVRAM_PARAMS_INDEX_20  = 20,
    NVRAM_PARAMS_INDEX_21  = 21,
    NVRAM_PARAMS_INDEX_22  = 22,
    NVRAM_PARAMS_INDEX_23  = 23,
    NVRAM_PARAMS_INDEX_24  = 24,
    NVRAM_PARAMS_SAR_START = 25,
    NVRAM_PARAMS_INDEX_25  = NVRAM_PARAMS_SAR_START,
    NVRAM_PARAMS_INDEX_26  = 26,
    NVRAM_PARAMS_INDEX_27  = 27,
    NVRAM_PARAMS_INDEX_28  = 28,
    NVRAM_PARAMS_INDEX_29  = 29,
    NVRAM_PARAMS_INDEX_30  = 30,
    NVRAM_PARAMS_INDEX_31  = 32,
    NVRAM_PARAMS_INDEX_32  = 32,
    NVRAM_PARAMS_INDEX_33  = 33,
    NVRAM_PARAMS_INDEX_34  = 34,
    NVRAM_PARAMS_INDEX_35  = 35,
    NVRAM_PARAMS_INDEX_36  = 36,
    NVRAM_PARAMS_INDEX_37  = 37,
    NVRAM_PARAMS_INDEX_38  = 38,
    NVRAM_PARAMS_INDEX_39  = 39,
    NVRAM_PARAMS_INDEX_40  = 40,
    NVRAM_PARAMS_INDEX_41  = 41,
    NVRAM_PARAMS_INDEX_42  = 42,
    NVRAM_PARAMS_INDEX_43  = 43,
    NVRAM_PARAMS_INDEX_44  = 44,
    NVRAM_PARAMS_INDEX_45  = 45,
    NVRAM_PARAMS_INDEX_46  = 46,
    NVRAM_PARAMS_INDEX_47  = 47,
    NVRAM_PARAMS_INDEX_48  = 48,
    NVRAM_PARAMS_INDEX_49  = 49,
    NVRAM_PARAMS_INDEX_50  = 50,
    NVRAM_PARAMS_INDEX_51  = 51,
    NVRAM_PARAMS_INDEX_52  = 52,
    NVRAM_PARAMS_INDEX_53  = 53,
    NVRAM_PARAMS_INDEX_54  = 54,
    NVRAM_PARAMS_INDEX_55  = 55,
    NVRAM_PARAMS_INDEX_56  = 56,
    NVRAM_PARAMS_INDEX_57  = 57,
    NVRAM_PARAMS_INDEX_58  = 58,
    NVRAM_PARAMS_INDEX_59  = 59,
    NVRAM_PARAMS_INDEX_60  = 60,
    NVRAM_PARAMS_INDEX_61  = 61,
    NVRAM_PARAMS_INDEX_62  = 62,
    NVRAM_PARAMS_INDEX_63  = 63,
    NVRAM_PARAMS_INDEX_64  = 64,
    NVRAM_PARAMS_SAR_END   = 65,
    NVRAM_PARAMS_INDEX_65  = NVRAM_PARAMS_SAR_END,
    NVRAM_PARAMS_INDEX_BUTT,
}NVRAM_PARAMS_INDEX;

/* 03/02A产侧nvram参数 */
typedef enum
{
    HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_11B      = 0,
    HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_OFDM20,
    HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_OFDM40,
    HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_CW,
    HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_BUTT,
}hwifi_dyn_2g_pwr_sngl_mode_enum;


typedef enum
{
    /* DP init */
    WLAN_CFG_NVRAM_DP2G_INIT0 = 0,
    /* pa */
    WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0 = 1,
    WLAN_CFG_NVRAM_RATIO_PA2GA0,
    WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A0,
    WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0,
    /* DPN */
    WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0,
    WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C0,
    WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C0,
    WLAN_CFG_DTS_NVRAM_END,
    /* just for ini file */
    /* 5g Band1& 2g CW */
    WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1 = WLAN_CFG_DTS_NVRAM_END,
    WLAN_CFG_DTS_NVRAM_RATIO_PA2GCWA0,
    /* add 5g low power part*/
    WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1_LOW,
    WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW,
    /* 2g cw ppa */
    WLAN_CFG_DTS_NVRAM_RATIO_PPA2GCWA0,

    // BT Power fit params, CW
    BT_CFG_DTS_NVRAM_POWER_CWA0,
    // BT PPA Vdet fit params, CW
    BT_CFG_DTS_NVRAM_PPA_CWA0,

    WLAN_CFG_DTS_NVRAM_PARAMS_BUTT,
    /* 新增系数依次往后添加 */
}PRO_LINE_NVRAM_PARAMS_INDEX;

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef unsigned char countrycode_t[COUNTRY_CODE_LEN];
typedef struct
{
   regdomain_enum       en_regdomain;
   countrycode_t        auc_country_code;
} countryinfo_stru;

/* 不接受cust下发的切换国家码的命令
 * 1、ini文件中国家码被配置成99
 * 2、维测需要:使用hipriv命令修改过国家码
*/
typedef struct
{
    oal_bool_enum_uint8 en_country_code_ingore_ini_flag;
    oal_bool_enum_uint8 en_country_code_ingore_hipriv_flag;
} wlan_cust_country_code_ingore_flag_stru;

/**
 *  regdomain <-> plat_tag map structure
 *
 **/
typedef struct regdomain_plat_tag_map
{
    regdomain_enum  en_regdomain;
    int             plat_tag;
} regdomain_plat_tag_map_stru;

typedef struct
{
    char*   name;
    int     case_entry;
} wlan_cfg_cmd;

/* 定制化HOST全局变量结构体 */
typedef struct
{
    /* ba tx 聚合数 */
    unsigned int    ul_ampdu_tx_max_num;
    /* 漫游 */
    unsigned char   uc_roam_switch;
    unsigned char   uc_roam_scan_band;
    unsigned char   uc_roam_scan_orthogonal;
    signed char     c_roam_trigger_b;
    signed char     c_roam_trigger_a;
    signed char     c_roam_delta_b;
    signed char     c_roam_delta_a;
    /* scan */
    unsigned char   uc_random_mac_addr_scan;
    /* capab */
    unsigned char   uc_disable_capab_2ght40;
    unsigned int   ul_lte_gpio_check_switch;
    unsigned int   ul_lte_ism_priority;
    unsigned int   ul_lte_rx_act;
    unsigned int   ul_lte_tx_act;
} wlan_customize_stru;

typedef struct
{
    oal_int32                     l_val;
    oal_bool_enum_uint8     en_value_state;
}wlan_customize_private_stru;

typedef struct
{
    oal_uint8*   puc_nv_name;
    oal_uint8*   puc_param_name;
    oal_uint32   ul_nv_map_idx;
    oal_uint8    uc_param_idx;
    oal_uint8    auc_rsv[3];
} wlan_cfg_nv_map_handler;

extern wlan_cust_country_code_ingore_flag_stru g_st_cust_country_code_ignore_flag;
extern wlan_cus_pwr_fit_para_stru g_as_pro_line_params[DY_CALI_PARAMS_NUM];
extern wlan_customize_stru g_st_wlan_customize;
extern oal_bool_enum_uint8 g_en_nv_dp_init_is_null;      /* NVRAM中dp init置空标志 */
extern oal_int16 gs_extre_point_vals[DY_CALI_NUM_5G_BAND];

extern int hwifi_config_init(int);
extern int hwifi_get_init_value(int, int);
extern char* hwifi_get_country_code(void);
extern void hwifi_set_country_code(char*, const unsigned int);
extern int hwifi_get_mac_addr(unsigned char *);
extern wlan_customize_power_params_stru* hwifi_get_nvram_params(void);
extern unsigned char* hwifi_get_nvram_param(unsigned int ul_nvram_param_idx);
extern int hwifi_is_regdomain_changed(const countrycode_t, const countrycode_t);
extern int hwifi_atcmd_update_host_nv_params(void);
extern int hwifi_custom_host_read_cfg_init(void);
extern int32 hwifi_get_init_priv_value(int32 l_cfg_id, int32 *pl_priv_value);
extern oal_void hwifi_config_init_base_power(oal_void);
extern int32 hwifi_get_plat_tag_from_country_code(const countrycode_t country_code);
extern regdomain_enum hwifi_get_regdomain_from_country_code(const countrycode_t country_code);
extern oal_uint32 hwifi_config_get_bt_dpn(oal_int32 *pl_params, oal_uint16 size);

#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif //hisi_customize_wifi.h

