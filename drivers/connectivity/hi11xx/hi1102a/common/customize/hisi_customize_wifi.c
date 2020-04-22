

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/*
 * 1 Header File Including
 */

#include <linux/kernel.h>
#include <linux/time.h>
#include "hisi_customize_wifi.h"
#include "hisi_ini.h"
#include "plat_type.h"
#include "oam_ext_if.h"
#include "wlan_spec_1102.h"
#include "oal_sdio_comm.h"

/* 终端头文件 */
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/etherdevice.h>
#include "plat_firmware.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CUSTOMIZE

/*
 * 2 Global Variable Definition
 */
int32 g_al_host_init_params[WLAN_CFG_INIT_BUTT] = {0};      /* ini定制化参数数组 */
int32 g_al_dts_params[WLAN_CFG_DTS_BUTT] = {0};             /* dts定制化参数数组 */
uint8 g_auc_nv_params[NUM_OF_NV_MAX_TXPOWER] = {0};              /* nv定制化参数数组 */
int32 g_al_nvram_init_params[NVRAM_PARAMS_INDEX_BUTT] = {
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_0
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_1
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_2
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_3
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_4
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_5
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_6
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_7
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_8
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_9
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_10
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_11
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_12
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_13
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_14
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_15
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_16
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_17
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_18
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_19
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_20
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_21
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_22
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_23
    0xeaebeaeb, //NVRAM_PARAMS_INDEX_24
    0xffffffff, //NVRAM_PARAMS_INDEX_25
    0xffffffff, //NVRAM_PARAMS_INDEX_26
    0xffffffff, //NVRAM_PARAMS_INDEX_27
    0xffffffff, //NVRAM_PARAMS_INDEX_28
    0xffffffff, //NVRAM_PARAMS_INDEX_29
    0xffffffff, //NVRAM_PARAMS_INDEX_30
    0xffffffff, //NVRAM_PARAMS_INDEX_31
    0xffffffff, //NVRAM_PARAMS_INDEX_32
};    /* ini文件中NV参数数组 */

oal_uint32 g_al_priv_cust_init_params[WLAN_CFG_PRIV_BUTT] = {
    0x60f,       //cali_mask
    0,           //cali_data_mask
    0x005a0069,  //temp_pro_threshold
    1,           //temp_pro_en
    1,           //temp_pro_reduce_pwr_en
    90,          //temp_pro_safe_th
    100,          //temp_pro_over_th
    110,          //temp_pro_pa_off_th
    250,         //dsss2ofdm_dbb_pwr_bo_val
    10,          //fast_check_cnt
    2,           //PRIV_VOE_SWITCH
    0,           //dyn_bypass_extlna
    0,           //hcc_flowctrl_type
    1            //ext_fem_5g_type
};
int8 g_ac_country_code[COUNTRY_CODE_LEN] = "00";
uint8 g_auc_wifimac[MAC_LEN] = {0x00,0x00,0x00,0x00,0x00,0x00};
wlan_cus_pwr_fit_para_stru g_as_pro_line_params[DY_CALI_PARAMS_NUM] = {{0}};    /* 产测定制化参数数组 */
uint8 g_auc_cust_nvram_info[WLAN_CFG_DTS_NVRAM_END][DY_CALI_PARAMS_LEN]  = {{0}};  /* NVRAM数组 */
oal_bool_enum_uint8 g_en_nv_dp_init_is_null = OAL_TRUE;      /* NVRAM中dp init置空标志 */
wlan_customize_private_stru g_al_priv_cust_params[WLAN_CFG_PRIV_BUTT] = {{0,0}};  /* 私有定制化参数数组 */
oal_int16 gs_extre_point_vals[DY_CALI_NUM_5G_BAND] = {0};

wlan_customize_power_params_stru g_st_cust_nv_params  = {{0}};  /* 最大发送功率定制化数组 */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
oal_uint8                        g_auc_sar_params[CUS_NUM_OF_SAR_LVL][CUS_SAR_NUM];
#endif
oal_bool_enum_uint8 g_en_fact_cali_completed = OAL_FALSE; /* 是否有产线校准 */
wlan_cust_country_code_ingore_flag_stru g_st_cust_country_code_ignore_flag = {0}; /* 定制化国家码配置 */

/*
 * 定制化结构体
 * default values as follows:
 * ampdu_tx_max_num:            WLAN_AMPDU_TX_MAX_NUM               = 64
 * switch:                      ON                                  = 1
 * scan_band:                   ROAM_BAND_2G_BIT|ROAM_BAND_5G_BIT   = 3
 * scan_orthogonal:             ROAM_SCAN_CHANNEL_ORG_BUTT          = 4
 */
wlan_customize_stru g_st_wlan_customize = {
            64,             /* addba_buffer_size */
            1,              /* roam switch */
            3,              /* roam scan band */
            4,              /* roam scan org */
            -70,            /* roam trigger 2G */
            -70,            /* roam trigger 5G */
            10,             /* roam delta 2G */
            10,             /* roam delta 5G */
            0,              /* random mac addr scan */
            0,              /* disable_capab_2ght40 */
            0,              /*lte_gpio_check_switch*/
            0,              /*lte_ism_priority*/
            0,              /*lte_rx_act*/
            0,              /*lte_tx_act*/
};

/**
 *  regdomain <-> country code map table
 *  max support country num: MAX_COUNTRY_COUNT
 *
 **/
OAL_STATIC countryinfo_stru g_ast_country_info_table[] =
{
    {REGDOMAIN_COMMON, {'0', '0'}}, // WORLD DOMAIN
    {REGDOMAIN_FCC,    {'A', 'D'}}, // ANDORRA
    {REGDOMAIN_ETSI,   {'A', 'E'}}, //UAE
    {REGDOMAIN_ETSI,   {'A', 'F'}}, //AFGHANISTAN
    {REGDOMAIN_ETSI,   {'A', 'G'}}, //ANTIGUA AND BARBUDA
    {REGDOMAIN_ETSI,   {'A', 'I'}}, //ANGUILLA
    {REGDOMAIN_ETSI,   {'A', 'L'}}, //ALBANIA
    {REGDOMAIN_ETSI,   {'A', 'M'}}, //ARMENIA
    {REGDOMAIN_ETSI,   {'A', 'N'}}, //NETHERLANDS ANTILLES
    {REGDOMAIN_ETSI,   {'A', 'O'}}, //ANGOLA
    {REGDOMAIN_FCC,    {'A', 'R'}}, //ARGENTINA
    {REGDOMAIN_FCC,    {'A', 'S'}}, //AMERICAN SOMOA
    {REGDOMAIN_ETSI,   {'A', 'T'}}, //AUSTRIA
    {REGDOMAIN_ETSI,   {'A', 'U'}}, //AUSTRALIA
    {REGDOMAIN_ETSI ,  {'A', 'W'}}, //ARUBA
    {REGDOMAIN_ETSI,   {'A', 'Z'}}, //AZERBAIJAN
    {REGDOMAIN_ETSI,   {'B', 'A'}}, //BOSNIA AND HERZEGOVINA
    {REGDOMAIN_FCC,    {'B', 'B'}}, //BARBADOS
    {REGDOMAIN_ETSI,   {'B', 'D'}}, //BANGLADESH
    {REGDOMAIN_ETSI,   {'B', 'E'}}, //BELGIUM
    {REGDOMAIN_ETSI,   {'B', 'G'}}, //BULGARIA
    {REGDOMAIN_ETSI,   {'B', 'H'}}, //BAHRAIN
    {REGDOMAIN_ETSI,   {'B', 'L'}}, //
    {REGDOMAIN_FCC,    {'B', 'M'}}, //BERMUDA
    {REGDOMAIN_ETSI,   {'B', 'N'}}, //BRUNEI DARUSSALAM
    {REGDOMAIN_FCC,    {'B', 'O'}}, //BOLIVIA
    {REGDOMAIN_FCC,    {'B', 'R'}}, //BRAZIL
    {REGDOMAIN_FCC,    {'B', 'S'}}, //BAHAMAS
    {REGDOMAIN_ETSI,   {'B', 'Y'}}, //BELARUS
    {REGDOMAIN_ETSI,   {'B', 'Z'}}, //BELIZE
    {REGDOMAIN_FCC,    {'C', 'A'}}, //CANADA
    {REGDOMAIN_ETSI,   {'C', 'H'}}, //SWITZERLAND
    {REGDOMAIN_FCC,    {'C', 'L'}}, //CHILE
    {REGDOMAIN_COMMON, {'C', 'N'}}, //CHINA
    {REGDOMAIN_FCC,    {'C', 'O'}}, //COLOMBIA
    {REGDOMAIN_FCC,    {'C', 'R'}}, //COSTA RICA
    {REGDOMAIN_ETSI,   {'C', 'S'}}, //
    {REGDOMAIN_ETSI,   {'C', 'U'}}, //CUBA
    {REGDOMAIN_ETSI,   {'C', 'Y'}}, //CYPRUS
    {REGDOMAIN_ETSI,   {'C', 'Z'}}, //CZECH REPUBLIC
    {REGDOMAIN_ETSI,   {'D', 'E'}}, //GERMANY
    {REGDOMAIN_ETSI,   {'D', 'K'}}, //DENMARK
    {REGDOMAIN_FCC,    {'D', 'O'}}, //DOMINICAN REPUBLIC
    {REGDOMAIN_ETSI,   {'D', 'Z'}}, //ALGERIA
    {REGDOMAIN_FCC,    {'E', 'C'}}, //ECUADOR
    {REGDOMAIN_ETSI,   {'E', 'E'}}, //ESTONIA
    {REGDOMAIN_ETSI,   {'E', 'G'}}, //EGYPT
    {REGDOMAIN_ETSI,   {'E', 'S'}}, //SPAIN
    {REGDOMAIN_ETSI,   {'E', 'T'}}, //ETHIOPIA
    {REGDOMAIN_ETSI,   {'F', 'I'}}, //FINLAND
    {REGDOMAIN_ETSI,   {'F', 'R'}}, //FRANCE
    {REGDOMAIN_ETSI,   {'G', 'B'}}, //UNITED KINGDOM
    {REGDOMAIN_FCC,    {'G', 'D'}}, //GRENADA
    {REGDOMAIN_ETSI,   {'G', 'E'}}, //GEORGIA
    {REGDOMAIN_ETSI,   {'G', 'F'}}, //FRENCH GUIANA
    {REGDOMAIN_ETSI,   {'G', 'L'}}, //GREENLAND
    {REGDOMAIN_ETSI,   {'G', 'P'}}, //GUADELOUPE
    {REGDOMAIN_ETSI,   {'G', 'R'}}, //GREECE
    {REGDOMAIN_FCC,    {'G', 'T'}}, //GUATEMALA
    {REGDOMAIN_FCC,    {'G', 'U'}}, //GUAM
    {REGDOMAIN_ETSI,   {'H', 'K'}}, //HONGKONG
    {REGDOMAIN_FCC,    {'H', 'N'}}, //HONDURAS
    {REGDOMAIN_ETSI,   {'H', 'R'}}, //Croatia
    {REGDOMAIN_ETSI,   {'H', 'U'}}, //HUNGARY
    {REGDOMAIN_ETSI,   {'I', 'D'}}, //INDONESIA
    {REGDOMAIN_ETSI,   {'I', 'E'}}, //IRELAND
    {REGDOMAIN_ETSI,   {'I', 'L'}}, //ISRAEL
    {REGDOMAIN_ETSI,   {'I', 'N'}}, //INDIA
    {REGDOMAIN_ETSI,   {'I', 'Q'}}, //IRAQ
    {REGDOMAIN_ETSI,   {'I', 'R'}}, //IRAN, ISLAMIC REPUBLIC OF
    {REGDOMAIN_ETSI,   {'I', 'S'}}, //ICELNAD
    {REGDOMAIN_ETSI,   {'I', 'T'}}, //ITALY
    {REGDOMAIN_FCC,    {'J', 'M'}}, //JAMAICA
    {REGDOMAIN_JAPAN,  {'J', 'P'}}, //JAPAN
    {REGDOMAIN_ETSI,   {'J', 'O'}}, //JORDAN
    {REGDOMAIN_ETSI,   {'K', 'E'}}, //KENYA
    {REGDOMAIN_ETSI,   {'K', 'H'}}, //CAMBODIA
    {REGDOMAIN_ETSI,   {'K', 'P'}}, //KOREA, DEMOCRATIC PEOPLE's REPUBLIC OF
    {REGDOMAIN_ETSI,   {'K', 'R'}}, //KOREA, REPUBLIC OF
    {REGDOMAIN_ETSI,   {'K', 'W'}}, //KUWAIT
    {REGDOMAIN_ETSI,   {'K', 'Y'}}, //Cayman Is
    {REGDOMAIN_ETSI,   {'K', 'Z'}}, //KAZAKHSTAN
    {REGDOMAIN_ETSI,   {'L', 'B'}}, //LEBANON
    {REGDOMAIN_ETSI,   {'L', 'I'}}, //LIECHTENSTEIN
    {REGDOMAIN_ETSI,   {'L', 'K'}}, //SRI-LANKA
    {REGDOMAIN_ETSI,   {'L', 'S'}}, //KINGDOM OF LESOTH
    {REGDOMAIN_ETSI,   {'L', 'T'}}, //LITHUANIA
    {REGDOMAIN_ETSI,   {'L', 'U'}}, //LUXEMBOURG
    {REGDOMAIN_ETSI,   {'L', 'V'}}, //LATVIA
    {REGDOMAIN_ETSI,   {'M', 'A'}}, //MOROCCO
    {REGDOMAIN_ETSI,   {'M', 'C'}}, //MONACO
    {REGDOMAIN_ETSI,   {'M', 'D'}}, //REPUBLIC OF MOLDOVA
    {REGDOMAIN_ETSI,   {'M', 'E'}}, //Montenegro
    {REGDOMAIN_FCC,    {'M', 'H'}}, //Marshall Is
    {REGDOMAIN_ETSI,   {'M', 'K'}}, //MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF
    {REGDOMAIN_ETSI,   {'M', 'M'}}, //MYANMAR
    {REGDOMAIN_FCC,    {'M', 'N'}}, //MONGOLIA
    {REGDOMAIN_ETSI,   {'M', 'O'}}, //MACAO
    {REGDOMAIN_FCC,    {'M', 'P'}}, //NORTHERN MARIANA ISLANDS
    {REGDOMAIN_ETSI,   {'M', 'Q'}}, //MARTINIQUE
    {REGDOMAIN_ETSI,   {'M', 'R'}}, //Mauritania
    {REGDOMAIN_ETSI,   {'M', 'T'}}, //MALTA
    {REGDOMAIN_ETSI,   {'M', 'V'}}, //Maldives
    {REGDOMAIN_ETSI,   {'M', 'U'}}, //MAURITIUS
    {REGDOMAIN_ETSI,   {'M', 'W'}}, //MALAWI
    {REGDOMAIN_ETSI,   {'M', 'X'}}, //MEXICO
    {REGDOMAIN_ETSI,   {'M', 'Y'}}, //MALAYSIA
    {REGDOMAIN_ETSI,   {'N', 'G'}}, //NIGERIA
    {REGDOMAIN_FCC,    {'N', 'I'}}, //NICARAGUA
    {REGDOMAIN_ETSI,   {'N', 'L'}}, //NETHERLANDS
    {REGDOMAIN_ETSI,   {'N', 'O'}}, //NORWAY
    {REGDOMAIN_ETSI,   {'N', 'P'}}, //NEPAL
    {REGDOMAIN_ETSI,   {'N', 'Z'}}, //NEW-ZEALAND
    {REGDOMAIN_ETSI,   {'O', 'M'}}, //OMAN
    {REGDOMAIN_FCC,    {'P', 'A'}}, //PANAMA
    {REGDOMAIN_FCC,    {'P', 'E'}}, //PERU
    {REGDOMAIN_ETSI,   {'P', 'F'}}, //FRENCH POLYNESIA
    {REGDOMAIN_ETSI,   {'P', 'G'}}, //PAPUA NEW GUINEA
    {REGDOMAIN_ETSI,   {'P', 'H'}}, //PHILIPPINES
    {REGDOMAIN_ETSI,   {'P', 'K'}}, //PAKISTAN
    {REGDOMAIN_ETSI,   {'P', 'L'}}, //POLAND
    {REGDOMAIN_FCC,    {'P', 'R'}}, //PUERTO RICO
    {REGDOMAIN_FCC,    {'P', 'S'}}, //PALESTINIAN TERRITORY, OCCUPIED
    {REGDOMAIN_ETSI,   {'P', 'T'}}, //PORTUGAL
    {REGDOMAIN_FCC,    {'P', 'Y'}}, //PARAGUAY
    {REGDOMAIN_ETSI,   {'Q', 'A'}}, //QATAR
    {REGDOMAIN_ETSI,   {'R', 'E'}}, //REUNION
    {REGDOMAIN_ETSI,   {'R', 'O'}}, //ROMAINIA
    {REGDOMAIN_ETSI,   {'R', 'S'}}, //SERBIA
    {REGDOMAIN_ETSI,   {'R', 'U'}}, //RUSSIA
    {REGDOMAIN_FCC,    {'R', 'W'}}, //RWANDA
    {REGDOMAIN_ETSI,   {'S', 'A'}}, //SAUDI ARABIA
    {REGDOMAIN_ETSI,   {'S', 'D'}}, //SUDAN ,REPUBLIC OF THE
    {REGDOMAIN_ETSI,   {'S', 'E'}}, //SWEDEN
    {REGDOMAIN_ETSI,   {'S', 'G'}}, //SINGAPORE
    {REGDOMAIN_ETSI,   {'S', 'I'}}, //SLOVENNIA
    {REGDOMAIN_ETSI,   {'S', 'K'}}, //SLOVAKIA
    {REGDOMAIN_ETSI,   {'S', 'N'}}, //Senegal
    {REGDOMAIN_ETSI,   {'S', 'V'}}, //EL SALVADOR
    {REGDOMAIN_ETSI,   {'S', 'Y'}}, //SYRIAN ARAB REPUBLIC
    {REGDOMAIN_ETSI,   {'T', 'H'}}, //THAILAND
    {REGDOMAIN_ETSI,   {'T', 'N'}}, //TUNISIA
    {REGDOMAIN_ETSI,   {'T', 'R'}}, //TURKEY
    {REGDOMAIN_ETSI,   {'T', 'T'}}, //TRINIDAD AND TOBAGO
    {REGDOMAIN_FCC,    {'T', 'W'}}, //TAIWAN, PRIVINCE OF CHINA
    {REGDOMAIN_FCC,    {'T', 'Z'}}, //TANZANIA, UNITED REPUBLIC OF
    {REGDOMAIN_ETSI,   {'U', 'A'}}, //UKRAINE
    {REGDOMAIN_ETSI,   {'U', 'G'}}, //UGANDA
    {REGDOMAIN_FCC,    {'U', 'S'}}, //USA
    {REGDOMAIN_FCC,    {'U', 'Y'}}, //URUGUAY
    {REGDOMAIN_ETSI,   {'U', 'Z'}}, //UZBEKISTAN
    {REGDOMAIN_FCC,    {'V', 'E'}}, //VENEZUELA
    {REGDOMAIN_FCC,    {'V', 'I'}}, //VIRGIN ISLANDS, US
    {REGDOMAIN_ETSI,   {'V', 'N'}}, //VIETNAM
    {REGDOMAIN_ETSI,   {'Y', 'E'}}, //YEMEN
    {REGDOMAIN_ETSI,   {'Y', 'T'}}, //MAYOTTE
    {REGDOMAIN_ETSI,   {'Z', 'A'}}, //SOUTH AFRICA
    {REGDOMAIN_ETSI,   {'Z', 'M'}}, //Zambia
    {REGDOMAIN_ETSI,   {'Z', 'W'}}, //ZIMBABWE

    {REGDOMAIN_COUNT,{'9','9'}},
};

/**
 * regdomain <-> plat_tag mapping table
 *
 **/
OAL_STATIC regdomain_plat_tag_map_stru g_ast_plat_tag_mapping_table[] =
{
        {REGDOMAIN_FCC,     INI_MODU_POWER_FCC},        //FCC
        {REGDOMAIN_ETSI,    INI_MODU_POWER_ETSI},       //ETSI
        {REGDOMAIN_JAPAN,   INI_MODU_POWER_JP},         //JP
        {REGDOMAIN_COMMON,  INI_MODU_WIFI},             //COMMON

        {REGDOMAIN_COUNT,   INI_MODU_INVALID}
};

OAL_STATIC wlan_cfg_cmd g_ast_wifi_config_dts[] =
{
    /* 5g开关 */
    {"band_5g_enable",                                       WLAN_CFG_DTS_BAND_5G_ENABLE},
    /* 校准 */
    {"cali_txpwr_pa_dc_ref_2g_val_chan1",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN1},
    {"cali_txpwr_pa_dc_ref_2g_val_chan2",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN2},
    {"cali_txpwr_pa_dc_ref_2g_val_chan3",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN3},
    {"cali_txpwr_pa_dc_ref_2g_val_chan4",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN4},
    {"cali_txpwr_pa_dc_ref_2g_val_chan5",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN5},
    {"cali_txpwr_pa_dc_ref_2g_val_chan6",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN6},
    {"cali_txpwr_pa_dc_ref_2g_val_chan7",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN7},
    {"cali_txpwr_pa_dc_ref_2g_val_chan8",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN8},
    {"cali_txpwr_pa_dc_ref_2g_val_chan9",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN9},
    {"cali_txpwr_pa_dc_ref_2g_val_chan10",                   WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN10},
    {"cali_txpwr_pa_dc_ref_2g_val_chan11",                   WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN11},
    {"cali_txpwr_pa_dc_ref_2g_val_chan12",                   WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN12},
    {"cali_txpwr_pa_dc_ref_2g_val_chan13",                   WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN13},
    {"cali_txpwr_pa_dc_ref_5g_val_band1",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND1},
    {"cali_txpwr_pa_dc_ref_5g_val_band2",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND2},
    {"cali_txpwr_pa_dc_ref_5g_val_band3",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND3},
    {"cali_txpwr_pa_dc_ref_5g_val_band4",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND4},
    {"cali_txpwr_pa_dc_ref_5g_val_band5",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND5},
    {"cali_txpwr_pa_dc_ref_5g_val_band6",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND6},
    {"cali_txpwr_pa_dc_ref_5g_val_band7",                    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND7},
    {"cali_tone_amp_grade",                                  WLAN_CFG_DTS_CALI_TONE_AMP_GRADE},

    /* RF REGISTER */
    {"rf_reg117",                                           WLAN_CFG_DTS_RF_REG117},
    {"rf_reg123",                                           WLAN_CFG_DTS_RF_REG123},
    {"rf_reg124",                                           WLAN_CFG_DTS_RF_REG124},
    {"rf_reg125",                                           WLAN_CFG_DTS_RF_REG125},
    {"rf_reg126",                                           WLAN_CFG_DTS_RF_REG126},
    /* bt 校准 */
    {"cali_txpwr_dpn_band1",                                 WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND1},
    {"cali_txpwr_dpn_band2",                                 WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND2},
    {"cali_txpwr_dpn_band3",                                 WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND3},
    {"cali_txpwr_dpn_band4",                                 WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND4},
    {"cali_txpwr_dpn_band5",                                 WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND5},
    {"cali_txpwr_dpn_band6",                                 WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND6},
    {"cali_txpwr_dpn_band7",                                 WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND7},
    {"cali_txpwr_dpn_band8",                                 WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND8},
    {"cali_txpwr_num",                                       WLAN_CFG_DTS_BT_CALI_TXPWR_NUM},
    {"cali_txpwr_freq1",                                     WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ1},
    {"cali_txpwr_freq2",                                     WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ2},
    {"cali_txpwr_freq3",                                     WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ3},
    {"cali_txpwr_freq4",                                     WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ4},
    {"cali_txpwr_freq5",                                     WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ5},
    {"cali_txpwr_freq6",                                     WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ6},
    {"cali_txpwr_freq7",                                     WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ7},
    {"cali_txpwr_freq8",                                     WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ8},
    {"cali_bt_insertion_loss",                               WLAN_CFG_DTS_BT_CALI_INSERTION_LOSS},
    {"cali_bt_gm_cali_en",                                   WLAN_CFG_DTS_BT_CALI_GM_CALI_EN},
    {"cali_bt_gm0_db10",                                     WLAN_CFG_DTS_BT_CALI_GM0_DB10},
    {"cali_bt_base_power",                                   WLAN_CFG_DTS_BT_CALI_BASE_POWER},
    {"cali_bt_is_dpn_calc",                                  WLAN_CFG_DTS_BT_CALI_IS_DPN_CALC},
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    /* 动态校准 */
    {"dyn_cali_dscr_interval",     WLAN_CFG_DTS_DYN_CALI_DSCR_ITERVL},
    /* DPN 40M 20M 11b */
    {"dpn24g_ch1_core0",           WLAN_CFG_DTS_2G_CORE0_DPN_CH1},
    {"dpn24g_ch2_core0",           WLAN_CFG_DTS_2G_CORE0_DPN_CH2},
    {"dpn24g_ch3_core0",           WLAN_CFG_DTS_2G_CORE0_DPN_CH3},
    {"dpn24g_ch4_core0",           WLAN_CFG_DTS_2G_CORE0_DPN_CH4},
    {"dpn24g_ch5_core0",           WLAN_CFG_DTS_2G_CORE0_DPN_CH5},
    {"dpn24g_ch6_core0",           WLAN_CFG_DTS_2G_CORE0_DPN_CH6},
    {"dpn24g_ch7_core0",           WLAN_CFG_DTS_2G_CORE0_DPN_CH7},
    {"dpn24g_ch8_core0",           WLAN_CFG_DTS_2G_CORE0_DPN_CH8},
    {"dpn24g_ch9_core0",           WLAN_CFG_DTS_2G_CORE0_DPN_CH9},
    {"dpn24g_ch10_core0",          WLAN_CFG_DTS_2G_CORE0_DPN_CH10},
    {"dpn24g_ch11_core0",          WLAN_CFG_DTS_2G_CORE0_DPN_CH11},
    {"dpn24g_ch12_core0",          WLAN_CFG_DTS_2G_CORE0_DPN_CH12},
    {"dpn24g_ch13_core0",          WLAN_CFG_DTS_2G_CORE0_DPN_CH13},
    {"dpn5g_core0_b0",             WLAN_CFG_DTS_5G_CORE0_DPN_B0},
    {"dpn5g_core0_b1",             WLAN_CFG_DTS_5G_CORE0_DPN_B1},
    {"dpn5g_core0_b2",             WLAN_CFG_DTS_5G_CORE0_DPN_B2},
    {"dpn5g_core0_b3",             WLAN_CFG_DTS_5G_CORE0_DPN_B3},
    {"dpn5g_core0_b4",             WLAN_CFG_DTS_5G_CORE0_DPN_B4},
    {"dyn_cali_opt_switch",        WLAN_CFG_DTS_DYN_CALI_OPT_SWITCH},
    {"gm0_dB10_amend",             WLAN_CFG_DTS_DYN_CALI_GM0_DB10_AMEND},

#endif
    {"5g_upc_up_limit",            WLAN_CFG_DTS_5G_UPC_UP_LIMIT},
    {"5g_iq_backoff_power",        WLAN_CFG_DTS_5G_IQ_BACKOFF_POWER},
    {"5g_iq_cali_power",           WLAN_CFG_DTS_5G_IQ_CALI_POWER},

    {OAL_PTR_NULL, 0}
};

OAL_STATIC wlan_cfg_cmd g_ast_wifi_config_cmds[] =
{
    /* ROAM */
    {"roam_switch",                     WLAN_CFG_INIT_ROAM_SWITCH},
    {"scan_orthogonal",                 WLAN_CFG_INIT_SCAN_ORTHOGONAL},
    {"trigger_b",                       WLAN_CFG_INIT_TRIGGER_B},
    {"trigger_a",                       WLAN_CFG_INIT_TRIGGER_A},
    {"delta_b",                         WLAN_CFG_INIT_DELTA_B},
    {"delta_a",                         WLAN_CFG_INIT_DELTA_A},

    /* 性能 */
    {"ampdu_tx_max_num",                WLAN_CFG_INIT_AMPDU_TX_MAX_NUM},
    {"used_mem_for_start",              WLAN_CFG_INIT_USED_MEM_FOR_START},
    {"used_mem_for_stop",               WLAN_CFG_INIT_USED_MEM_FOR_STOP},
    {"rx_ack_limit",                    WLAN_CFG_INIT_RX_ACK_LIMIT},
    {"sdio_d2h_assemble_count",         WLAN_CFG_INIT_SDIO_D2H_ASSEMBLE_COUNT},
    {"sdio_h2d_assemble_count",         WLAN_CFG_INIT_SDIO_H2D_ASSEMBLE_COUNT},
    /* LINKLOSS */
    {"link_loss_threshold_wlan_bt",     WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_WLAN_BT},
    {"link_loss_threshold_wlan_dbac",   WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_WLAN_DBAC},
    {"link_loss_threshold_wlan_normal", WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_WLAN_NORMAL},
    /* 自动调频 */
    {"pps_threshold_level_0",           WLAN_CFG_INIT_PPS_THRESHOLD_LEVEL_0},
    {"pps_threshold_level_1",           WLAN_CFG_INIT_PPS_THRESHOLD_LEVEL_1},
    {"pps_threshold_level_2",           WLAN_CFG_INIT_PPS_THRESHOLD_LEVEL_2},
    {"pps_threshold_level_3",           WLAN_CFG_INIT_PPS_THRESHOLD_LEVEL_3},
    {"device_type_level_0",             WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_0},
    {"device_type_level_1",             WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_1},
    {"device_type_level_2",             WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_2},
    {"device_type_level_3",             WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_3},
    /* 低功耗 */
    {"powermgmt_switch",                WLAN_CFG_INIT_POWERMGMT_SWITCH},
    {"ps_mode",                         WLAN_CFG_INIT_PS_MODE},

    /* 可维可测 */
    {"loglevel",                        WLAN_CFG_INIT_LOGLEVEL},
    /* PHY算法 */
    {"chn_est_ctrl",                    WLAN_CFG_INIT_CHN_EST_CTRL},
    {"power_ref_5g",                    WLAN_CFG_INIT_POWER_REF_5G},
    /* 2G RF前端 */
    {"rf_line_txrx_gain_db_2g_band1_mult4",     WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND1_MULT4},
    {"rf_line_txrx_gain_db_2g_band1_mult10",    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND1_MULT10},
    {"rf_line_txrx_gain_db_2g_band2_mult4",     WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND2_MULT4},
    {"rf_line_txrx_gain_db_2g_band2_mult10",    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND2_MULT10},
    {"rf_line_txrx_gain_db_2g_band3_mult4",     WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND3_MULT4},
    {"rf_line_txrx_gain_db_2g_band3_mult10",    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND3_MULT10},
    /* 5G RF前端 */
    {"rf_line_txrx_gain_db_5g_band1_mult4",     WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND1_MULT4},
    {"rf_line_txrx_gain_db_5g_band1_mult10",    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND1_MULT10},
    {"rf_line_txrx_gain_db_5g_band2_mult4",     WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND2_MULT4},
    {"rf_line_txrx_gain_db_5g_band2_mult10",    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND2_MULT10},
    {"rf_line_txrx_gain_db_5g_band3_mult4",     WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND3_MULT4},
    {"rf_line_txrx_gain_db_5g_band3_mult10",    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND3_MULT10},
    {"rf_line_txrx_gain_db_5g_band4_mult4",     WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND4_MULT4},
    {"rf_line_txrx_gain_db_5g_band4_mult10",    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND4_MULT10},
    {"rf_line_txrx_gain_db_5g_band5_mult4",     WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND5_MULT4},
    {"rf_line_txrx_gain_db_5g_band5_mult10",    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND5_MULT10},
    {"rf_line_txrx_gain_db_5g_band6_mult4",     WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND6_MULT4},
    {"rf_line_txrx_gain_db_5g_band6_mult10",    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND6_MULT10},
    {"rf_line_txrx_gain_db_5g_band7_mult4",     WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND7_MULT4},
    {"rf_line_txrx_gain_db_5g_band7_mult10",    WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND7_MULT10},
    {"rf_line_rx_gain_db_5g",           WLAN_CFG_INIT_RF_LINE_RX_GAIN_DB_5G},
    {"lna_gain_db_5g",                  WLAN_CFG_INIT_LNA_GAIN_DB_5G},
    {"rf_line_tx_gain_db_5g",           WLAN_CFG_INIT_RF_LINE_TX_GAIN_DB_5G},
    {"ext_switch_isexist_5g",           WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_5G},
    {"ext_pa_isexist_5g",               WLAN_CFG_INIT_EXT_PA_ISEXIST_5G},
    {"ext_lna_isexist_5g",              WLAN_CFG_INIT_EXT_LNA_ISEXIST_5G},
    {"lna_on2off_time_ns_5g",           WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_5G},
    {"lna_off2on_time_ns_5g",           WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_5G},
    /* SCAN */
    {"random_mac_addr_scan",            WLAN_CFG_INIT_RANDOM_MAC_ADDR_SCAN},
    /* 11AC2G */
    {"11ac2g_enable",                   WLAN_CFG_INIT_11AC2G_ENABLE},
    {"disable_capab_2ght40",            WLAN_CFG_INIT_DISABLE_CAPAB_2GHT40},
    {"dual_antenna_enable",             WLAN_CFG_INIT_DUAL_ANTENNA_ENABLE}, /* 双天线开关 */
    {"far_dist_pow_gain_switch",        WLAN_CFG_INIT_FAR_DIST_POW_GAIN_SWITCH},
    {"lte_gpio_check_switch",           WLAN_CFG_LTE_GPIO_CHECK_SWITCH},/* lte管脚检测开关 */
    {"lte_ism_priority",                WLAN_ATCMDSRV_LTE_ISM_PRIORITY},
    {"lte_rx_act",                      WLAN_ATCMDSRV_LTE_RX_ACT},
    {"lte_tx_act",                      WLAN_ATCMDSRV_LTE_TX_ACT},
    {"far_dist_dsss_scale_promote_switch",      WLAN_CFG_INIT_FAR_DIST_DSSS_SCALE_PROMOTE_SWITCH},
    {"delta_cca_ed_high_20th_2g",       WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_2G},
    {"delta_cca_ed_high_40th_2g",       WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_2G},
    {"delta_cca_ed_high_20th_5g",       WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_5G},
    {"delta_cca_ed_high_40th_5g",       WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_5G},
#ifdef _PRE_WLAN_DOWNLOAD_PM
    {"download_rate_limit_pps",         WLAN_CFG_INIT_DOWNLOAD_RATE_LIMIT_PPS},
#endif
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    {"tx_amsdu_ampdu",                  WLAN_CFG_INIT_AMPDU_AMSDU_SKB},
    {"tx_amsdu_ampdu_th_l",             WLAN_CFG_INIT_AMSDU_AMPDU_TH_LOW},
    {"tx_amsdu_ampdu_th_h",             WLAN_CFG_INIT_AMSDU_AMPDU_TH_HIGH},
    {"rx_ampdu_amsdu",                  WLAN_CFG_INIT_RX_AMPDU_AMSDU_SKB},
#endif
#ifdef _PRE_WLAN_TCP_OPT
    {"en_tcp_ack_filter",               WLAN_CFG_INIT_TCP_ACK_FILTER},
    {"rx_tcp_ack_filter_th_l",          WLAN_CFG_INIT_TCP_ACK_FILTER_TH_LOW},
    {"rx_tcp_ack_filter_th_h",          WLAN_CFG_INIT_TCP_ACK_FILTER_TH_HIGH},
#endif

    {"thread_bindcpu",                  WLAN_CFG_INIT_BINDCPU},
    {"thread_bindcpu_mask",             WLAN_CFG_INIT_BINDCPU_MASK},
    {"tx_bindcpu_th_high",              WLAN_CFG_INIT_BINDCPU_TXHIGH},
    {"tx_bindcpu_th_low",               WLAN_CFG_INIT_BINDCPU_TXLOW},
    {"rx_bindcpu_th_high",              WLAN_CFG_INIT_BINDCPU_RXHIGH},
    {"rx_bindcpu_th_low",               WLAN_CFG_INIT_BINDCPU_RXLOW},

    {"tcp_ack_buf_switch",              WLAN_CFG_INIT_TX_TCP_ACK_BUF},
    {"tcp_ack_buf_th_h",                WLAN_CFG_INIT_TCP_ACK_BUF_HIGH},
    {"tcp_ack_buf_th_l",                WLAN_CFG_INIT_TCP_ACK_BUF_LOW},
    {"tcp_ack_buf_th_h_40M",            WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_40M},
    {"tcp_ack_buf_th_l_40M",            WLAN_CFG_INIT_TCP_ACK_BUF_LOW_40M},
    {"tcp_ack_buf_th_h_80M",            WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_80M},
    {"tcp_ack_buf_th_l_80M",            WLAN_CFG_INIT_TCP_ACK_BUF_LOW_80M},

    /* DYN_BYPASS_EXTLNA */
    {"rx_dyn_bypss_extlna_switch",      WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA},
    {"rx_dyn_bypss_extlna_th_h",        WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_HIGH},
    {"rx_dyn_bypss_extlna_th_l",        WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_LOW},

    /* SMALL_AMSDU */
    {"tx_small_amsdu_switch",           WLAN_CFG_INIT_TX_SMALL_AMSDU},
    {"tx_small_amsdu_th_h",             WLAN_CFG_INIT_SMALL_AMSDU_HIGH},
    {"tx_small_amsdu_th_l",             WLAN_CFG_INIT_SMALL_AMSDU_LOW},
    {"tx_small_amsdu_pps_th_h",         WLAN_CFG_INIT_SMALL_AMSDU_PPS_HIGH},
    {"tx_small_amsdu_pps_th_l",         WLAN_CFG_INIT_SMALL_AMSDU_PPS_LOW},

    /* 基准功率 */
    {"nvram_max_txpwr_base_2p4g",         WLAN_CFG_INIT_TX_BASE_POWER_2P4G},
    {"nvram_max_txpwr_base_5g_1",         WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND1},
    {"nvram_max_txpwr_base_5g_2",         WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND2},
    {"nvram_max_txpwr_base_5g_3",         WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND3},
    {"nvram_max_txpwr_base_5g_4",         WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND4},
    {"nvram_max_txpwr_base_5g_5",         WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND5},
    {"nvram_max_txpwr_base_5g_6",         WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND6},
    {"nvram_max_txpwr_base_5g_7",         WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND7},

    /* 边带信道最大功率 */
    {"side_band_txpwr_limit_5g_20m_0",    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_20M_0},
    {"side_band_txpwr_limit_5g_20m_1",    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_20M_1},
    {"side_band_txpwr_limit_5g_20m_2",    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_20M_2},
    {"side_band_txpwr_limit_5g_40m_0",    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_40M_0},
    {"side_band_txpwr_limit_5g_40m_1",    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_40M_1},
    {"side_band_txpwr_limit_5g_80m_0",    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_80M_0},
    {"side_band_txpwr_limit_5g_80m_1",    WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_80M_1},
    {"side_band_txpwr_limit_5g_160m",     WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_160M},
    {"side_band_txpwr_limit_24g_ch1",     WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_1},
    {"side_band_txpwr_limit_24g_ch2",     WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_2},
    {"side_band_txpwr_limit_24g_ch3",     WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_3},
    {"side_band_txpwr_limit_24g_ch4",     WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_4},
    {"side_band_txpwr_limit_24g_ch5",     WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_5},
    {"side_band_txpwr_limit_24g_ch6",     WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_6},
    {"side_band_txpwr_limit_24g_ch7",     WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_7},
    {"side_band_txpwr_limit_24g_ch8",     WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_8},
    {"side_band_txpwr_limit_24g_ch9",     WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_9},
    {"side_band_txpwr_limit_24g_ch10",    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_10},
    {"side_band_txpwr_limit_24g_ch11",    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_11},
    {"side_band_txpwr_limit_24g_ch12",    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_12},
    {"side_band_txpwr_limit_24g_ch13",    WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_13},
    {"delta_pwr_ref_2g_20m",              WLAN_CFG_INIT_DELTA_PWR_REF_2G_20M},
    {"delta_pwr_ref_2g_40m",              WLAN_CFG_INIT_DELTA_PWR_REF_2G_40M},
    {"delta_pwr_ref_5g_20m",              WLAN_CFG_INIT_DELTA_PWR_REF_5G_20M},
    {"delta_pwr_ref_5g_40m",              WLAN_CFG_INIT_DELTA_PWR_REF_5G_40M},
    {"delta_pwr_ref_5g_80m",              WLAN_CFG_INIT_DELTA_PWR_REF_5G_80M},

    {OAL_PTR_NULL, 0}
};

OAL_STATIC wlan_cfg_cmd g_ast_nvram_config_ini[NVRAM_PARAMS_INDEX_BUTT] =
{
    {"nvram_params0",                     NVRAM_PARAMS_INDEX_0},
    {"nvram_params1",                     NVRAM_PARAMS_INDEX_1},
    {"nvram_params2",                     NVRAM_PARAMS_INDEX_2},
    {"nvram_params3",                     NVRAM_PARAMS_INDEX_3},
    {"nvram_params4",                     NVRAM_PARAMS_INDEX_4},
    {"nvram_params5",                     NVRAM_PARAMS_INDEX_5},
    {"nvram_params6",                     NVRAM_PARAMS_INDEX_6},
    {"nvram_params7",                     NVRAM_PARAMS_INDEX_7},
    {"nvram_params8",                     NVRAM_PARAMS_INDEX_8},
    {"nvram_params9",                     NVRAM_PARAMS_INDEX_9},
    {"nvram_params10",                    NVRAM_PARAMS_INDEX_10},
    {"nvram_params11",                    NVRAM_PARAMS_INDEX_11},
    {"nvram_params12",                    NVRAM_PARAMS_INDEX_12},
    {"nvram_params13",                    NVRAM_PARAMS_INDEX_13},
    {"nvram_params14",                    NVRAM_PARAMS_INDEX_14},
    {"nvram_params15",                    NVRAM_PARAMS_INDEX_15},
    {"nvram_params16",                    NVRAM_PARAMS_INDEX_16},
    {"nvram_params17",                    NVRAM_PARAMS_INDEX_17},
    {"nvram_params18",                    NVRAM_PARAMS_INDEX_18},
    {"nvram_params19",                    NVRAM_PARAMS_INDEX_19},
    {"nvram_params20",                    NVRAM_PARAMS_INDEX_20},
    {"nvram_params21",                    NVRAM_PARAMS_INDEX_21},
    {"nvram_params22",                    NVRAM_PARAMS_INDEX_22},
    {"nvram_params23",                    NVRAM_PARAMS_INDEX_23},
    {"nvram_params24",                    NVRAM_PARAMS_INDEX_24},
     /* SAR level 4~1*/
    {"sar_txpwr_ctrl_5g_band1_0",       NVRAM_PARAMS_INDEX_25},
    {"sar_txpwr_ctrl_5g_band2_0",       NVRAM_PARAMS_INDEX_26},
    {"sar_txpwr_ctrl_5g_band3_0",       NVRAM_PARAMS_INDEX_27},
    {"sar_txpwr_ctrl_5g_band4_0",       NVRAM_PARAMS_INDEX_28},
    {"sar_txpwr_ctrl_5g_band5_0",       NVRAM_PARAMS_INDEX_29},
    {"sar_txpwr_ctrl_5g_band6_0",       NVRAM_PARAMS_INDEX_30},
    {"sar_txpwr_ctrl_5g_band7_0",       NVRAM_PARAMS_INDEX_31},
    {"sar_txpwr_ctrl_2g_0",             NVRAM_PARAMS_INDEX_32},
     /* SAR level 8~5*/
    {"sar_txpwr_ctrl_5g_band1_1",       NVRAM_PARAMS_INDEX_33},
    {"sar_txpwr_ctrl_5g_band2_1",       NVRAM_PARAMS_INDEX_34},
    {"sar_txpwr_ctrl_5g_band3_1",       NVRAM_PARAMS_INDEX_35},
    {"sar_txpwr_ctrl_5g_band4_1",       NVRAM_PARAMS_INDEX_36},
    {"sar_txpwr_ctrl_5g_band5_1",       NVRAM_PARAMS_INDEX_37},
    {"sar_txpwr_ctrl_5g_band6_1",       NVRAM_PARAMS_INDEX_38},
    {"sar_txpwr_ctrl_5g_band7_1",       NVRAM_PARAMS_INDEX_39},
    {"sar_txpwr_ctrl_2g_1",             NVRAM_PARAMS_INDEX_40},
     /* SAR level 12~9*/
    {"sar_txpwr_ctrl_5g_band1_2",       NVRAM_PARAMS_INDEX_41},
    {"sar_txpwr_ctrl_5g_band2_2",       NVRAM_PARAMS_INDEX_42},
    {"sar_txpwr_ctrl_5g_band3_2",       NVRAM_PARAMS_INDEX_43},
    {"sar_txpwr_ctrl_5g_band4_2",       NVRAM_PARAMS_INDEX_44},
    {"sar_txpwr_ctrl_5g_band5_2",       NVRAM_PARAMS_INDEX_45},
    {"sar_txpwr_ctrl_5g_band6_2",       NVRAM_PARAMS_INDEX_46},
    {"sar_txpwr_ctrl_5g_band7_2",       NVRAM_PARAMS_INDEX_47},
    {"sar_txpwr_ctrl_2g_2",             NVRAM_PARAMS_INDEX_48},
     /* SAR level 16~13*/
    {"sar_txpwr_ctrl_5g_band1_3",       NVRAM_PARAMS_INDEX_49},
    {"sar_txpwr_ctrl_5g_band2_3",       NVRAM_PARAMS_INDEX_50},
    {"sar_txpwr_ctrl_5g_band3_3",       NVRAM_PARAMS_INDEX_51},
    {"sar_txpwr_ctrl_5g_band4_3",       NVRAM_PARAMS_INDEX_52},
    {"sar_txpwr_ctrl_5g_band5_3",       NVRAM_PARAMS_INDEX_53},
    {"sar_txpwr_ctrl_5g_band6_3",       NVRAM_PARAMS_INDEX_54},
    {"sar_txpwr_ctrl_5g_band7_3",       NVRAM_PARAMS_INDEX_55},
    {"sar_txpwr_ctrl_2g_3",             NVRAM_PARAMS_INDEX_56},
     /* SAR level 20~17*/
    {"sar_txpwr_ctrl_5g_band1_4",       NVRAM_PARAMS_INDEX_57},
    {"sar_txpwr_ctrl_5g_band2_4",       NVRAM_PARAMS_INDEX_58},
    {"sar_txpwr_ctrl_5g_band3_4",       NVRAM_PARAMS_INDEX_59},
    {"sar_txpwr_ctrl_5g_band4_4",       NVRAM_PARAMS_INDEX_60},
    {"sar_txpwr_ctrl_5g_band5_4",       NVRAM_PARAMS_INDEX_61},
    {"sar_txpwr_ctrl_5g_band6_4",       NVRAM_PARAMS_INDEX_62},
    {"sar_txpwr_ctrl_5g_band7_4",       NVRAM_PARAMS_INDEX_63},
    {"sar_txpwr_ctrl_2g_4",             NVRAM_PARAMS_INDEX_64},
    {OAL_PTR_NULL,                      NVRAM_PARAMS_INDEX_BUTT},
};

OAL_STATIC wlan_cfg_cmd g_ast_wifi_config_priv[] =
{
    /* 校准开关 */
    {"cali_mask",                      WLAN_CFG_PRIV_CALI_MASK},
    /* bit4:不读取NV区域的数据(1:不读取 0：读取) */
    {"cali_data_mask",                 WLAN_CFG_PRIV_CALI_DATA_MASK},
    /* TBD:hal_cfg_customize_info_stru/mac_device_capability_stru */
    {"temp_pro_threshold",             WLAN_CFG_PRIV_TEMP_PRO_THRESHOLD},
    {"temp_pro_en",                    WLAN_CFG_PRIV_TEMP_PRO_EN},
    {"temp_pro_reduce_pwr_en",         WLAN_CFG_PRIV_TEMP_PRO_REDUCE_PWR_EN},
    {"temp_pro_safe_th",               WLAN_CFG_PRIV_TEMP_PRO_SAFE_TH},
    {"temp_pro_over_th",               WLAN_CFG_PRIV_TEMP_PRO_OVER_TH},
    {"temp_pro_pa_off_th",             WLAN_CFG_PRIV_TEMP_PRO_PA_OFF_TH},
    {"dsss2ofdm_dbb_pwr_bo_val",       WLAN_DSSS2OFDM_DBB_PWR_BO_VAL},
    {"fast_ps_check_cnt",              WLAN_CFG_PRIV_FAST_CHECK_CNT},
    {"voe_switch_mask",                WLAN_CFG_PRIV_VOE_SWITCH},
    {"dyn_bypass_extlna_enable",       WLAN_CFG_PRIV_DYN_BYPASS_EXTLNA},
    {"hcc_flowctrl_type",              WLAN_CFG_PRIV_HCC_FLOWCTRL_TYPE},
    {"ext_fem_5g_type",                WLAN_CFG_PRIV_5G_EXT_FEM_TYPE},
    {OAL_PTR_NULL, 0}
};


OAL_STATIC wlan_cfg_nv_map_handler g_ast_wifi_nvram_cfg_handler[]=
{
    {"WITXBW0",  "dp2ginit0",   HWIFI_CFG_NV_WITXNVBWC0_NUMBER,      WLAN_CFG_NVRAM_DP2G_INIT0,          {0}},
    {"WITXCCK",  "pa2gccka0",   HWIFI_CFG_NV_WITXNVCCK_NUMBER,       WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0, {0}},
    {"WINVRAM",  "pa2ga0",      HWIFI_CFG_NV_WINVRAM_NUMBER,         WLAN_CFG_NVRAM_RATIO_PA2GA0,        {0}},
    {"WITXL0" ,  "pa2g40a0",    HWIFI_CFG_NV_WITXL2G5G0_NUMBER,      WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A0,  {0}},
    {"WINVRAM",  "pa5ga0",      HWIFI_CFG_NV_WINVRAM_NUMBER,         WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0,    {0}},
    {"WIC0_MUCCK",       "mf2gccka0",      HWIFI_CFG_NV_MUFREQ_CCK_C0_NUMBER,        WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0,  {0}},
    {"WIC0_2GMU_OFDM",   "mf2ga0",         HWIFI_CFG_NV_MUFREQ_2G20_C0_NUMBER,       WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C0,   {0}},
    {"WIC0_2GMF40M",     "mf2g40a0",       HWIFI_CFG_NV_MUFREQ_2G40_C0_NUMBER,       WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C0,   {0}},
};


OAL_STATIC wlan_cfg_cmd g_ast_nvram_pro_line_config_ini[] =
{   /* OAL_PTR_NULL表示不从ini里面读 */
    {OAL_PTR_NULL,           WLAN_CFG_NVRAM_DP2G_INIT0},
    /* 产侧nvram参数 */
    {"nvram_pa2gccka0",      WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0},
    {"nvram_pa2ga0",         WLAN_CFG_NVRAM_RATIO_PA2GA0},
    {"nvram_pa2g40a0",       WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A0},
    {"nvram_pa5ga0",         WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0},
    {OAL_PTR_NULL,           WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0},
    {OAL_PTR_NULL,           WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C0},
    {OAL_PTR_NULL,           WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C0},
    {"nvram_pa5ga0_band1",   WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1},
    {"nvram_pa2gcwa0",       WLAN_CFG_DTS_NVRAM_RATIO_PA2GCWA0},
    {"nvram_pa5ga0_band1_low", WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1_LOW},
    {"nvram_pa5ga0_low",       WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW},

    {"nvram_ppa2gcwa0",      WLAN_CFG_DTS_NVRAM_RATIO_PPA2GCWA0},

    {"bt_nvram_power_cwa0",  BT_CFG_DTS_NVRAM_POWER_CWA0},
    {"bt_nvram_ppa_cwa0",    BT_CFG_DTS_NVRAM_PPA_CWA0},
    {OAL_PTR_NULL,           WLAN_CFG_DTS_NVRAM_PARAMS_BUTT},
};

OAL_STATIC oal_void original_value_for_dts_params(oal_void)
{
    g_al_dts_params[WLAN_CFG_DTS_BAND_5G_ENABLE]                        = 1;
    /* 校准 */
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN1]     = 6250;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN2]     = 5362;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN3]     = 4720;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN4]     = 4480;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN5]     = 4470;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN6]     = 4775;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN7]     = 5200;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN8]     = 5450;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN9]     = 5600;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN10]    = 6100;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN11]    = 6170;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN12]    = 6350;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN13]    = 6530;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND1]     = 2500;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND2]     = 2800;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND3]     = 3100;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND4]     = 3600;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND5]     = 3600;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND6]     = 3700;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND7]     = 3800;
    g_al_dts_params[WLAN_CFG_DTS_CALI_TONE_AMP_GRADE]                   = 2;

    /* rf register */
    g_al_dts_params[WLAN_CFG_DTS_RF_REG117]                                 = 0x0505;
    g_al_dts_params[WLAN_CFG_DTS_RF_REG123]                                 = 0x9d01;
    g_al_dts_params[WLAN_CFG_DTS_RF_REG124]                                 = 0x9d01;
    g_al_dts_params[WLAN_CFG_DTS_RF_REG125]                                 = 0x9d01;
    g_al_dts_params[WLAN_CFG_DTS_RF_REG126]                                 = 0x9d01;
    /* bt tmp */
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND1]               = 0;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND2]               = 0;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND3]               = 0;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND4]               = 0;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND5]               = 0;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND6]               = 0;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND7]               = 0;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND8]               = 0;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_NUM]                     = 8;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ1]                   = 0;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ2]                   = 10;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ3]                   = 28;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ4]                   = 45;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ5]                   = 53;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ6]                   = 63;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ7]                   = 76;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_TXPWR_FREQ8]                   = 78;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_INSERTION_LOSS]                = 20;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_GM_CALI_EN]                    = 1;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_GM0_DB10]                      = 0;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_BASE_POWER]                    = 100;
    g_al_dts_params[WLAN_CFG_DTS_BT_CALI_IS_DPN_CALC]                   = 0;
}

OAL_STATIC oal_void host_params_init_first(oal_void)
{
    /* ROAM */
    g_al_host_init_params[WLAN_CFG_INIT_ROAM_SWITCH]                       = 1;
    g_al_host_init_params[WLAN_CFG_INIT_SCAN_ORTHOGONAL]                   = 4;
    g_al_host_init_params[WLAN_CFG_INIT_TRIGGER_B]                         = -70;
    g_al_host_init_params[WLAN_CFG_INIT_TRIGGER_A]                         = -70;
    g_al_host_init_params[WLAN_CFG_INIT_DELTA_B]                           = 10;
    g_al_host_init_params[WLAN_CFG_INIT_DELTA_A]                           = 10;

    /* 性能 */
    g_al_host_init_params[WLAN_CFG_INIT_AMPDU_TX_MAX_NUM]                  = WLAN_AMPDU_TX_MAX_BUF_SIZE;
    g_al_host_init_params[WLAN_CFG_INIT_USED_MEM_FOR_START]                = 45;
    g_al_host_init_params[WLAN_CFG_INIT_USED_MEM_FOR_STOP]                 = 25;
    g_al_host_init_params[WLAN_CFG_INIT_RX_ACK_LIMIT]                      = 10;
    g_al_host_init_params[WLAN_CFG_INIT_SDIO_D2H_ASSEMBLE_COUNT]           = HISDIO_DEV2HOST_SCATT_MAX;
    g_al_host_init_params[WLAN_CFG_INIT_SDIO_H2D_ASSEMBLE_COUNT]           = 8;
    /* LINKLOSS */
    g_al_host_init_params[WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_WLAN_BT]       = 80;
    g_al_host_init_params[WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_WLAN_DBAC]     = 80;
    g_al_host_init_params[WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_WLAN_NORMAL]   = 40;
    /* 自动调频 */
    g_al_host_init_params[WLAN_CFG_INIT_PPS_THRESHOLD_LEVEL_0]             = PPS_VALUE_0;
    g_al_host_init_params[WLAN_CFG_INIT_PPS_THRESHOLD_LEVEL_1]             = PPS_VALUE_1;
    g_al_host_init_params[WLAN_CFG_INIT_PPS_THRESHOLD_LEVEL_2]             = PPS_VALUE_2;
    g_al_host_init_params[WLAN_CFG_INIT_PPS_THRESHOLD_LEVEL_3]             = PPS_VALUE_3;
    g_al_host_init_params[WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_0]               = FREQ_IDLE;
    g_al_host_init_params[WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_1]               = FREQ_MIDIUM;
    g_al_host_init_params[WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_2]               = FREQ_HIGHER;
    g_al_host_init_params[WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_3]               = FREQ_HIGHEST;
    /* 低功耗 */
    g_al_host_init_params[WLAN_CFG_INIT_POWERMGMT_SWITCH]                  = 1;
    g_al_host_init_params[WLAN_CFG_INIT_PS_MODE]                           = 1;
    /* 可维可测 */
    /* 日志级别 */
    g_al_host_init_params[WLAN_CFG_INIT_LOGLEVEL]                          = OAM_LOG_LEVEL_WARNING;
    /* PHY算法 */
    g_al_host_init_params[WLAN_CFG_INIT_CHN_EST_CTRL]                      = CHN_EST_CTRL_MATE7;
    g_al_host_init_params[WLAN_CFG_INIT_POWER_REF_5G]                      = PHY_POWER_REF_5G_MT7;
    /* 2G RF前端 */
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND1_MULT4]    = -12;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND1_MULT10]   = -30;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND2_MULT4]    = -12;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND2_MULT10]   = -30;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND3_MULT4]    = -12;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_2G_BAND3_MULT10]   = -30;
    /* 5G RF前端 */
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND1_MULT4]    = -8;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND1_MULT10]   = -20;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND2_MULT4]    = -8;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND2_MULT10]   = -20;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND3_MULT4]    = -8;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND3_MULT10]   = -20;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND4_MULT4]    = -8;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND4_MULT10]   = -20;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND5_MULT4]    = -8;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND5_MULT10]   = -20;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND6_MULT4]    = -8;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND6_MULT10]   = -20;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND7_MULT4]    = -8;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TXRX_GAIN_DB_5G_BAND7_MULT10]   = -20;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_RX_GAIN_DB_5G]             = -12;
    g_al_host_init_params[WLAN_CFG_INIT_LNA_GAIN_DB_5G]                    = 20;
    g_al_host_init_params[WLAN_CFG_INIT_RF_LINE_TX_GAIN_DB_5G]             = -12;
    g_al_host_init_params[WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_5G]             = 1;
    g_al_host_init_params[WLAN_CFG_INIT_EXT_PA_ISEXIST_5G]                 = 1;
    g_al_host_init_params[WLAN_CFG_INIT_EXT_LNA_ISEXIST_5G]                = 1;
    g_al_host_init_params[WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_5G]             = 630;
    g_al_host_init_params[WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_5G]             = 320;
    /* SCAN */
    g_al_host_init_params[WLAN_CFG_INIT_RANDOM_MAC_ADDR_SCAN]              = 0;
    /* 11AC2G */
    g_al_host_init_params[WLAN_CFG_INIT_11AC2G_ENABLE]                     = 1;
    g_al_host_init_params[WLAN_CFG_INIT_DISABLE_CAPAB_2GHT40]              = 0;
    g_al_host_init_params[WLAN_CFG_INIT_DUAL_ANTENNA_ENABLE]            = 0;
    g_al_host_init_params[WLAN_CFG_INIT_FAR_DIST_POW_GAIN_SWITCH]          = 1;
    g_al_host_init_params[WLAN_CFG_LTE_GPIO_CHECK_SWITCH]                  = 0;
    g_al_host_init_params[WLAN_ATCMDSRV_LTE_ISM_PRIORITY]                  = 0;
    g_al_host_init_params[WLAN_ATCMDSRV_LTE_RX_ACT]                        = 0;
    g_al_host_init_params[WLAN_ATCMDSRV_LTE_TX_ACT]                        = 0;
    g_al_host_init_params[WLAN_CFG_INIT_FAR_DIST_DSSS_SCALE_PROMOTE_SWITCH]     = 1;
    g_al_host_init_params[WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_2G]         = 0;
    g_al_host_init_params[WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_2G]         = 0;
    g_al_host_init_params[WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_5G]         = 0;
    g_al_host_init_params[WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_5G]         = 0;
    g_al_host_init_params[WLAN_CFG_INIT_DELTA_PWR_REF_2G_20M]              = 0;
    g_al_host_init_params[WLAN_CFG_INIT_DELTA_PWR_REF_2G_40M]              = 0;
    g_al_host_init_params[WLAN_CFG_INIT_DELTA_PWR_REF_5G_20M]              = 0;
    g_al_host_init_params[WLAN_CFG_INIT_DELTA_PWR_REF_5G_40M]              = 0;
    g_al_host_init_params[WLAN_CFG_INIT_DELTA_PWR_REF_5G_80M]              = 0;


#ifdef _PRE_WLAN_DOWNLOAD_PM
    g_al_host_init_params[WLAN_CFG_INIT_DOWNLOAD_RATE_LIMIT_PPS]           = 0;
#endif
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    g_al_host_init_params[WLAN_CFG_INIT_AMPDU_AMSDU_SKB]                   = OAL_FALSE;
    g_al_host_init_params[WLAN_CFG_INIT_AMSDU_AMPDU_TH_HIGH]               = WLAN_AMSDU_AMPDU_THROUGHPUT_THRESHOLD_HIGH;
    g_al_host_init_params[WLAN_CFG_INIT_AMSDU_AMPDU_TH_LOW]                = WLAN_AMSDU_AMPDU_THROUGHPUT_THRESHOLD_LOW;
    g_al_host_init_params[WLAN_CFG_INIT_RX_AMPDU_AMSDU_SKB]                = OAL_FALSE;
#endif
#ifdef _PRE_WLAN_TCP_OPT
    g_al_host_init_params[WLAN_CFG_INIT_TCP_ACK_FILTER]                    = OAL_TRUE;
    g_al_host_init_params[WLAN_CFG_INIT_TCP_ACK_FILTER_TH_HIGH]            = WLAN_TCP_ACK_FILTER_TH_HIGH;
    g_al_host_init_params[WLAN_CFG_INIT_TCP_ACK_FILTER_TH_LOW]             = WLAN_TCP_ACK_FILTER_TH_LOW;
#endif

    g_al_host_init_params[WLAN_CFG_INIT_BINDCPU]                           = OAL_TRUE;
    g_al_host_init_params[WLAN_CFG_INIT_BINDCPU_MASK]                      = WLAN_BINDCPU_DEFAULT_MASK;
    g_al_host_init_params[WLAN_CFG_INIT_BINDCPU_TXHIGH]                    = WLAN_TX_BUSY_CPU_THROUGHT;
    g_al_host_init_params[WLAN_CFG_INIT_BINDCPU_TXLOW]                     = WLAN_TX_IDLE_CPU_THROUGHT;
    g_al_host_init_params[WLAN_CFG_INIT_BINDCPU_RXHIGH]                    = WLAN_RX_BUSY_CPU_THROUGHT;
    g_al_host_init_params[WLAN_CFG_INIT_BINDCPU_RXLOW]                     = WLAN_RX_IDLE_CPU_THROUGHT;

    g_al_host_init_params[WLAN_CFG_INIT_TX_TCP_ACK_BUF]                    = OAL_TRUE;
    g_al_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_HIGH]                  = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH;
    g_al_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_LOW]                   = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW;
    g_al_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_40M]              = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_40M;
    g_al_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_LOW_40M]               = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW_40M;
    g_al_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_80M]              = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_80M;
    g_al_host_init_params[WLAN_CFG_INIT_TCP_ACK_BUF_LOW_80M]               = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW_80M;

    /* RX DYN BYPASS EXTLNA 定制化参数 */
    g_al_host_init_params[WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA]              = OAL_FALSE;
    g_al_host_init_params[WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_HIGH]         = WLAN_DYN_BYPASS_EXTLNA_THROUGHPUT_THRESHOLD_HIGH;
    g_al_host_init_params[WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_LOW]          = WLAN_DYN_BYPASS_EXTLNA_THROUGHPUT_THRESHOLD_LOW;

    /* SMALL AMSDU 定制化参数 */
    g_al_host_init_params[WLAN_CFG_INIT_TX_SMALL_AMSDU]                    = OAL_TRUE;
    g_al_host_init_params[WLAN_CFG_INIT_SMALL_AMSDU_HIGH]                  = WLAN_SMALL_AMSDU_THROUGHPUT_THRESHOLD_HIGH;
    g_al_host_init_params[WLAN_CFG_INIT_SMALL_AMSDU_LOW]                   = WLAN_SMALL_AMSDU_THROUGHPUT_THRESHOLD_LOW;
    g_al_host_init_params[WLAN_CFG_INIT_SMALL_AMSDU_PPS_HIGH]              = WLAN_SMALL_AMSDU_PPS_THRESHOLD_HIGH;
    g_al_host_init_params[WLAN_CFG_INIT_SMALL_AMSDU_PPS_LOW]               = WLAN_SMALL_AMSDU_PPS_THRESHOLD_LOW;

    /* 基准功率 */
    g_al_host_init_params[WLAN_CFG_INIT_TX_BASE_POWER_2P4G]                = INIT_NVM_BASE_TXPWR_2G;
    g_al_host_init_params[WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND1]            = INIT_NVM_BASE_TXPWR_5G;
    g_al_host_init_params[WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND2]            = INIT_NVM_BASE_TXPWR_5G;
    g_al_host_init_params[WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND3]            = INIT_NVM_BASE_TXPWR_5G;
    g_al_host_init_params[WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND4]            = INIT_NVM_BASE_TXPWR_5G;
    g_al_host_init_params[WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND5]            = INIT_NVM_BASE_TXPWR_5G;
    g_al_host_init_params[WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND6]            = INIT_NVM_BASE_TXPWR_5G;
    g_al_host_init_params[WLAN_CFG_INIT_TX_BASE_POWER_5G_BAND7]            = INIT_NVM_BASE_TXPWR_5G;

    /* 边带信道最大功率 */
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_20M_0]          = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_20M_1]          = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_20M_2]          = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_40M_0]          = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_40M_1]          = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_80M_0]          = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_80M_1]          = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_5G_160M]           = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_1]              = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_2]              = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_3]              = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_4]              = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_5]              = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_6]              = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_7]              = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_8]              = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_9]              = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_10]             = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_11]             = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_12]             = 0xFFFFFFFF;
    g_al_host_init_params[WLAN_CFG_INIT_SIDE_BAND_TXPWR_2G_13]             = 0xFFFFFFFF;

}

int32 hwifi_get_init_priv_value(oal_int32 l_cfg_id, oal_int32 *pl_priv_value)
{
    if (0 > l_cfg_id || WLAN_CFG_PRIV_BUTT <= l_cfg_id)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "hwifi_get_init_priv_value cfg id[%d] out of range, max[%d]", l_cfg_id, WLAN_CFG_PRIV_BUTT - 1);
        return OAL_FAIL;
    }

    *pl_priv_value = g_al_priv_cust_params[l_cfg_id].l_val;

    return OAL_SUCC;
}


regdomain_enum hwifi_get_regdomain_from_country_code(const countrycode_t country_code)
{
    regdomain_enum  en_regdomain = REGDOMAIN_COMMON;
    int32           table_idx = 0;

    while (g_ast_country_info_table[table_idx].en_regdomain != REGDOMAIN_COUNT)
    {
        if (0 == oal_memcmp(country_code, g_ast_country_info_table[table_idx].auc_country_code, COUNTRY_CODE_LEN))
        {
            en_regdomain = g_ast_country_info_table[table_idx].en_regdomain;
            break;
        }
        ++table_idx;
    }

    return en_regdomain;
}


int32 hwifi_is_regdomain_changed(const countrycode_t country_code_old, const countrycode_t country_code_new)
{
    return hwifi_get_regdomain_from_country_code(country_code_old) != hwifi_get_regdomain_from_country_code(country_code_new);
}


int32 hwifi_get_plat_tag_from_country_code(const countrycode_t country_code)
{
    regdomain_enum  en_regdomain;
    int32           table_idx = 0;

    en_regdomain = hwifi_get_regdomain_from_country_code(country_code);

    while(g_ast_plat_tag_mapping_table[table_idx].en_regdomain != REGDOMAIN_COUNT
        && g_ast_plat_tag_mapping_table[table_idx].plat_tag != INI_MODU_INVALID)
    {
        /* matched */
        if (en_regdomain == g_ast_plat_tag_mapping_table[table_idx].en_regdomain)
        {
            return g_ast_plat_tag_mapping_table[table_idx].plat_tag;
        }

        ++table_idx;
    }

    /* not found, use common regdomain */
    return INI_MODU_WIFI;
}


int32 hwifi_fetch_ori_caldata(uint8* auc_caldata, int32 l_nvm_len)
{
    int32 l_ret = INI_FAILED;
    int32 l_cfg_id;
    int32 aul_nvram_params[NVRAM_PARAMS_INDEX_BUTT]={0};

    if (l_nvm_len != HISI_CUST_NVRAM_LEN)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "hwifi_fetch_ori_caldata atcmd[nv_len:%d] and plat_ini[nv_len:%d] model have different nvm lenth!!",
                        l_nvm_len, HISI_CUST_NVRAM_LEN);
        return INI_FAILED;
    }

    oal_memset(auc_caldata, 0x00, HISI_CUST_NVRAM_LEN);

    for (l_cfg_id = NVRAM_PARAMS_INDEX_0; l_cfg_id < NVRAM_PARAMS_INDEX_23; l_cfg_id++)
    {
        l_ret = get_cust_conf_int32(INI_MODU_WIFI, g_ast_nvram_config_ini[l_cfg_id].name, &aul_nvram_params[l_cfg_id]);
        if(INI_FAILED == l_ret)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "hwifi_fetch_ori_caldata read ori caldata %d from ini failed!", l_cfg_id);
            return INI_FAILED;
        }
    }

    OAM_INFO_LOG0(0, OAM_SF_ANY, "hwifi_fetch_ori_caldata read ori caldata from ini success!");
    oal_memcopy(auc_caldata, aul_nvram_params, HISI_CUST_NVRAM_LEN);

    return INI_SUCC;
}


oal_int32 hwifi_custom_host_read_dyn_cali_nvram(oal_void)
{
    oal_int32      l_ret;
    oal_uint8      uc_idx = WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0;
    oal_uint8      uc_param_idx;
    oal_uint8      uc_times_idx = 0;
    oal_int8      *puc_str;
    oal_uint8     *pc_end = ";";
    oal_uint8     *pc_sep = ",";
    oal_int8      *pc_ctx;
    oal_int8      *pc_token;
    oal_int32      l_priv_value;
    oal_bool_enum_uint8 en_get_nvram_data_flag = OAL_FALSE;
    oal_bool_enum_uint8 en_fact_cali_completed = OAL_FALSE;
    oal_uint8     *puc_buffer_cust_nvram_tmp   = OAL_PTR_NULL;
    oal_int32     *pl_params                   = OAL_PTR_NULL;
    oal_uint8     *puc_cust_nvram_info         = OAL_PTR_NULL;  /* NVRAM数组 */

    puc_buffer_cust_nvram_tmp = (oal_uint8 *)OS_KZALLOC_GFP(DY_CALI_PARAMS_LEN);
    pl_params = (oal_int32 *)OS_KZALLOC_GFP(DY_CALI_PARAMS_NUM*DY_CALI_PARAMS_TIMES * OAL_SIZEOF(oal_int32));
    puc_cust_nvram_info = (oal_uint8 *)OS_KZALLOC_GFP(WLAN_CFG_DTS_NVRAM_END * DY_CALI_PARAMS_LEN);
    if (OAL_PTR_NULL == puc_buffer_cust_nvram_tmp || OAL_PTR_NULL == pl_params || OAL_PTR_NULL == puc_cust_nvram_info)
    {
        OAM_ERROR_LOG3(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::alloc failed.puc_buffer_cust_nvram_tmp[%x],pl_params[%x],puc_cust_nvram_info[%x]", puc_buffer_cust_nvram_tmp, pl_params, puc_cust_nvram_info);
        if(puc_buffer_cust_nvram_tmp != OAL_PTR_NULL)
        {
            OS_MEM_KFREE(puc_buffer_cust_nvram_tmp);
        }

        if(pl_params != OAL_PTR_NULL)
        {
            OS_MEM_KFREE(pl_params);
        }

        if(puc_cust_nvram_info != OAL_PTR_NULL)
        {
            OS_MEM_KFREE(puc_cust_nvram_info);
        }
        /* 防止上一次失败的结果影响这一次的结果，故清空 */
        OAL_MEMZERO(g_auc_cust_nvram_info, OAL_SIZEOF(g_auc_cust_nvram_info));
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    OAL_MEMZERO(puc_buffer_cust_nvram_tmp, DY_CALI_PARAMS_LEN);
    OAL_MEMZERO(pl_params, DY_CALI_PARAMS_NUM*DY_CALI_PARAMS_TIMES * OAL_SIZEOF(oal_int32));
    OAL_MEMZERO(puc_cust_nvram_info, WLAN_CFG_DTS_NVRAM_END * DY_CALI_PARAMS_LEN);

    /* 判断定制化中是否使用nvram中的动态校准参数 */
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_CALI_DATA_MASK, &l_priv_value);
    if (OAL_SUCC == l_ret)
    {
        en_get_nvram_data_flag = !!(HI1102A_CUST_READ_NVRAM_MASK & l_priv_value);
        if (en_get_nvram_data_flag)
        {
            OAL_IO_PRINT("hwifi_custom_host_read_dyn_cali_nvram::get_nvram_data_flag[%d] to abandon nvram data!!\r\n", l_priv_value);
            OAL_MEMZERO(g_auc_cust_nvram_info, OAL_SIZEOF(g_auc_cust_nvram_info));
            OS_MEM_KFREE(puc_buffer_cust_nvram_tmp);
            OS_MEM_KFREE(pl_params);
            return INI_FILE_TIMESPEC_UNRECONFIG;
        }
    }


    /* 拟合系数 */
    for (uc_idx = WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0; uc_idx < WLAN_CFG_DTS_NVRAM_END; uc_idx++)
    {
        l_ret = read_conf_from_nvram(puc_buffer_cust_nvram_tmp, DY_CALI_PARAMS_LEN,
                                     g_ast_wifi_nvram_cfg_handler[uc_idx].ul_nv_map_idx,
                                     g_ast_wifi_nvram_cfg_handler[uc_idx].puc_nv_name);
        if (l_ret != INI_SUCC)
        {
            OAL_MEMZERO(puc_cust_nvram_info + (uc_idx * DY_CALI_PARAMS_LEN * OAL_SIZEOF(oal_uint8)), DY_CALI_PARAMS_LEN*OAL_SIZEOF(oal_uint8));
            OAL_IO_PRINT("hwifi_custom_host_read_dyn_cali_nvram::NVRAM get fail NV id[%d] name[%s] para[%s]!\r\n",
                         g_ast_wifi_nvram_cfg_handler[uc_idx].ul_nv_map_idx,
                         g_ast_wifi_nvram_cfg_handler[uc_idx].puc_nv_name,
                         g_ast_wifi_nvram_cfg_handler[uc_idx].puc_param_name);
            continue;
        }

        puc_str = OAL_STRSTR(puc_buffer_cust_nvram_tmp, g_ast_wifi_nvram_cfg_handler[uc_idx].puc_param_name);
        if (puc_str == OAL_PTR_NULL)
        {
            OAL_MEMZERO(puc_cust_nvram_info + (uc_idx * DY_CALI_PARAMS_LEN * OAL_SIZEOF(oal_uint8)), DY_CALI_PARAMS_LEN*OAL_SIZEOF(oal_uint8));
            OAL_IO_PRINT("hwifi_custom_host_read_dyn_cali_nvram::NVRAM get wrong val NV id[%d] name[%s] para[%s]!\r\n",
                         g_ast_wifi_nvram_cfg_handler[uc_idx].ul_nv_map_idx,
                         g_ast_wifi_nvram_cfg_handler[uc_idx].puc_nv_name,
                         g_ast_wifi_nvram_cfg_handler[uc_idx].puc_param_name);
            continue;
        }

        /* 获取等号后面的实际参数 */
        puc_str += (OAL_STRLEN(g_ast_wifi_nvram_cfg_handler[uc_idx].puc_param_name) + 1);
        pc_token = oal_strtok(puc_str, pc_end, &pc_ctx);
        if (OAL_PTR_NULL == pc_token)
        {
            OAM_ERROR_LOG1(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::get null value check NV id[%d]!",
                           g_ast_wifi_nvram_cfg_handler[uc_idx].ul_nv_map_idx);
            OAL_IO_PRINT("hwifi_custom_host_read_dyn_cali_nvram::get null check NV id[%d] name[%s] para[%s]!\r\n", g_ast_wifi_nvram_cfg_handler[uc_idx].ul_nv_map_idx,
                         g_ast_wifi_nvram_cfg_handler[uc_idx].puc_nv_name, g_ast_wifi_nvram_cfg_handler[uc_idx].puc_param_name);
            OAL_MEMZERO(puc_cust_nvram_info + (uc_idx * DY_CALI_PARAMS_LEN * OAL_SIZEOF(oal_uint8)), DY_CALI_PARAMS_LEN*OAL_SIZEOF(oal_uint8));
            continue;
        }
        oal_memcopy(puc_cust_nvram_info + (uc_idx * DY_CALI_PARAMS_LEN * OAL_SIZEOF(oal_uint8)), pc_token, OAL_STRLEN(pc_token));
        *(puc_cust_nvram_info + (uc_idx * DY_CALI_PARAMS_LEN * OAL_SIZEOF(oal_uint8)) + OAL_STRLEN(pc_token))  = *pc_end;

        /* 拟合系数获取检查 */
        if (uc_idx <= WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0)
        {
            /* 二次参数合理性检查 */
            pc_token = oal_strtok(pc_token, pc_sep, &pc_ctx);
            uc_param_idx = 0;
            /* 获取定制化系数 */
            while (OAL_PTR_NULL != pc_token)
            {
                OAL_IO_PRINT( "hwifi_custom_host_read_dyn_cali_nvram::get [%s]\n!", pc_token);
                *(pl_params + uc_param_idx) = (oal_int32)oal_strtol(pc_token, OAL_PTR_NULL, 10);
                pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
                uc_param_idx++;
            }
            if (uc_param_idx % DY_CALI_PARAMS_TIMES)
            {
                OAM_ERROR_LOG1(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::check NV id[%d]!",
                               g_ast_wifi_nvram_cfg_handler[uc_idx].ul_nv_map_idx);
                OAL_MEMZERO(puc_cust_nvram_info + (uc_idx * DY_CALI_PARAMS_LEN * OAL_SIZEOF(oal_uint8)), DY_CALI_PARAMS_LEN*OAL_SIZEOF(oal_uint8));
                continue;
            }
            uc_times_idx = uc_param_idx / DY_CALI_PARAMS_TIMES;
            /* 二次项系数非0检查 */
            while (uc_times_idx--)
            {
                if (0 == pl_params[(uc_times_idx)*DY_CALI_PARAMS_TIMES])
                {
                    OAM_ERROR_LOG1(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::check NV id[%d]!",
                                   g_ast_wifi_nvram_cfg_handler[uc_idx].ul_nv_map_idx);
                    OAL_MEMZERO(puc_cust_nvram_info + (uc_idx * DY_CALI_PARAMS_LEN * OAL_SIZEOF(oal_uint8)), DY_CALI_PARAMS_LEN*OAL_SIZEOF(oal_uint8));
                    break;
                }
            }

            if (OAL_FALSE == en_fact_cali_completed)
            {
                en_fact_cali_completed = OAL_TRUE;
            }
        }
    }
    g_en_fact_cali_completed = en_fact_cali_completed;
    OS_MEM_KFREE(puc_buffer_cust_nvram_tmp);
    OS_MEM_KFREE(pl_params);


    /*检查NVRAM是否修改 */
    if (0 == oal_memcmp(puc_cust_nvram_info, g_auc_cust_nvram_info, OAL_SIZEOF(g_auc_cust_nvram_info)))
    {
        OS_MEM_KFREE(puc_cust_nvram_info);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    oal_memcopy(g_auc_cust_nvram_info, puc_cust_nvram_info, OAL_SIZEOF(g_auc_cust_nvram_info));
    OS_MEM_KFREE(puc_cust_nvram_info);
    return INI_NVRAM_RECONFIG;
}


oal_uint8 *hwifi_get_nvram_param(oal_uint32 ul_nvram_param_idx)
{
    return g_auc_cust_nvram_info[ul_nvram_param_idx];
}


int32 hwifi_custom_host_read_cfg_init(void)
{
   oal_int32      l_nv_read_ret;
   oal_int32      l_ini_read_ret;


    /* 先获取私有定制化项 */
    hwifi_config_init(CUS_TAG_PRIV_INI);

    /* 读取nvram参数是否修改 */
    l_nv_read_ret  = hwifi_custom_host_read_dyn_cali_nvram();
    /* 检查定制化文件中的产线配置是否修改 */
    l_ini_read_ret =  ini_file_check_conf_update();
    if (l_ini_read_ret ||  l_nv_read_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "hwifi_custom_host_read_cfg_init file is updated");
        hwifi_config_init(CUS_TAG_PRO_LINE_INI);
    }

    if (INI_FILE_TIMESPEC_UNRECONFIG == l_ini_read_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "hwifi_custom_host_read_cfg_init file is not updated");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    hwifi_config_init(CUS_TAG_DTS);

    l_ini_read_ret = hwifi_config_init(CUS_TAG_NV);
    if (OAL_UNLIKELY(OAL_SUCC != l_ini_read_ret))
    {
        OAL_IO_PRINT("hwifi_custom_host_read_cfg_init NV fail l_ret[%d].\r\n", l_ini_read_ret);
    }

    hwifi_config_init(CUS_TAG_INI);


    /*启动完成后，输出打印*/
    OAL_IO_PRINT("hwifi_custom_host_read_cfg_init finish!\r\n");

    return OAL_SUCC;
}


OAL_STATIC oal_int32 hwifi_config_init_sar_ctrl_nvram(void)
{
    oal_int32     l_ret = INI_FAILED;
    oal_uint8     uc_cfg_id;
    oal_uint8     uc_band_id;
    oal_uint8     uc_cus_id   = NVRAM_PARAMS_SAR_START;
    oal_uint8     uc_sar_lvl_idx;
    oal_uint32    ul_nvram_params   = 0;
    oal_uint8     auc_nvram_params[CUS_NUM_OF_SAR_ONE_PARAM_NUM];
    oal_int32     l_plat_tag;

    l_plat_tag = hwifi_get_plat_tag_from_country_code(hwifi_get_country_code());
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "hwifi_config_init_sar_ctrl_nvram plat_tag:0x%2x!", l_plat_tag);

    for (uc_cfg_id = 0; uc_cfg_id < CUS_NUM_OF_SAR_PER_BAND_PAR_NUM; uc_cfg_id++)
    {
        for (uc_band_id = 0; uc_band_id < CUS_NUM_OF_SAR_PARAMS; uc_band_id++)
        {
            l_ret = get_cust_conf_int32(l_plat_tag, g_ast_nvram_config_ini[uc_cus_id].name, &ul_nvram_params);

            if(INI_SUCC != l_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_CFG, "hwifi_config_init_sar_ctrl_nvram read id[%d] from ini failed!", uc_cus_id);
                /* 读取失败时,使用初始值 */
                ul_nvram_params = 0xFFFFFFFF;
            }

            oal_memcopy(auc_nvram_params, &ul_nvram_params, OAL_SIZEOF(ul_nvram_params));
            for (uc_sar_lvl_idx = 0; uc_sar_lvl_idx < CUS_NUM_OF_SAR_ONE_PARAM_NUM; uc_sar_lvl_idx++)
            {
                /* 定制项检查 */
                if (auc_nvram_params[uc_sar_lvl_idx] <= CUS_MIN_OF_SAR_VAL)
                {
                    OAM_ERROR_LOG4(0, OAM_SF_CUSTOM, "hwifi_config_init_sar_ctrl_nvram::uc_cfg_id[%d]:0x%x got[%d] out of the normal[%d] check ini file!",
                                   uc_cus_id, ul_nvram_params, auc_nvram_params[uc_sar_lvl_idx], CUS_MIN_OF_SAR_VAL);
                    auc_nvram_params[uc_sar_lvl_idx] = 0xFF;
                }
                g_auc_sar_params[uc_sar_lvl_idx+uc_cfg_id*CUS_NUM_OF_SAR_ONE_PARAM_NUM][uc_band_id] = auc_nvram_params[uc_sar_lvl_idx];
            }
            uc_cus_id++;
        }
    }

    return INI_SUCC;
}


oal_void hwifi_config_init_base_power(oal_void)
{
    int32 l_cfg_id;
    int32 l_base_power_idx = 0;
    int32 l_base_power_tmp;
    int32 l_ret = INI_FAILED;
    oal_uint8  auc_base_power[CUS_BASE_POWER_NUM];
    oal_int32  l_plat_tag;

    l_plat_tag = hwifi_get_plat_tag_from_country_code(hwifi_get_country_code());
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "hwifi_config_init_base_power plat_tag:0x%2x!", l_plat_tag);
    for (l_cfg_id = WLAN_CFG_INIT_TX_BASE_POWER_START; l_cfg_id <= WLAN_CFG_INIT_TX_BASE_POWER_END; l_cfg_id++)
    {
        l_ret = get_cust_conf_int32(l_plat_tag, g_ast_wifi_config_cmds[l_cfg_id].name, &l_base_power_tmp);

        if((INI_FAILED == l_ret) || (l_base_power_tmp > MAX_TXPOWER_MAX) || (l_base_power_tmp < MAX_TXPOWER_MIN))
        {
            /* 读取失败就使用系统默认值 */
            auc_base_power[l_base_power_idx] = g_al_host_init_params[l_cfg_id];
        }
        else
        {
            auc_base_power[l_base_power_idx] = l_base_power_tmp;
        }
        l_base_power_idx++;
    }

    oal_memcopy(g_st_cust_nv_params.auc_base_power, auc_base_power, OAL_SIZEOF(oal_uint8)*CUS_BASE_POWER_NUM);
}


int32 hwifi_config_init_fcc_ce_txpwr_nvram(oal_void)
{
    int32       l_ret = INI_FAILED;
    uint8       uc_cfg_id;
    uint8       uc_param_idx = 0;
    int32      *pl_nvram_params = OAL_PTR_NULL;
    int32      *pl_fcc_txpwr_limit_params = OAL_PTR_NULL;
    oal_uint32  ul_param_len = (WLAN_CFG_INIT_SIDE_BAND_TXPWR_END - WLAN_CFG_INIT_SIDE_BAND_TXPWR_START) * OAL_SIZEOF(int32);
    oal_int32  l_plat_tag;

    l_plat_tag = hwifi_get_plat_tag_from_country_code(hwifi_get_country_code());
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "hwifi_config_init_fcc_ce_txpwr_nvram plat_tag:0x%2x!", l_plat_tag);
    pl_fcc_txpwr_limit_params = (int32 *)kzalloc(ul_param_len, (GFP_KERNEL | __GFP_NOWARN));
    if (OAL_PTR_NULL == pl_fcc_txpwr_limit_params)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CUSTOM, "hwifi_config_init_fcc_ce_txpwr_nvram::pl_nvram_params mem alloc [%d] fail!", ul_param_len);
        return INI_FAILED;
    }
    OAL_MEMZERO(pl_fcc_txpwr_limit_params, ul_param_len);
    pl_nvram_params = pl_fcc_txpwr_limit_params;

    for (uc_cfg_id = WLAN_CFG_INIT_SIDE_BAND_TXPWR_START; uc_cfg_id <= WLAN_CFG_INIT_SIDE_BAND_TXPWR_END; uc_cfg_id++)
    {
        l_ret = get_cust_conf_int32(l_plat_tag, g_ast_wifi_config_cmds[uc_cfg_id].name, pl_nvram_params + uc_param_idx);

        if(INI_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "hwifi_config_init_nvram read id[%d] from ini failed!", uc_cfg_id);
            /* 读取失败时,使用初始值 */
            pl_nvram_params[uc_param_idx] = g_al_host_init_params[uc_cfg_id];
        }
        uc_param_idx++;
    }
    OAL_MEMZERO(&g_st_cust_nv_params.st_fcc_ce_txpwer_limit, OAL_SIZEOF(wlan_fcc_ce_power_limit_stru));
    /* 5g */
    /* CH140/CH100/CH64/CH36 */
    oal_memcopy(g_st_cust_nv_params.st_fcc_ce_txpwer_limit.auc_5g_txpwr_20M, pl_nvram_params, OAL_SIZEOF(int32));
    pl_nvram_params++;
    /* CH161/CH157/CH153/CH149 */
    oal_memcopy(g_st_cust_nv_params.st_fcc_ce_txpwer_limit.auc_5g_txpwr_20M+OAL_SIZEOF(int32), pl_nvram_params, OAL_SIZEOF(int32));
    pl_nvram_params++;
    /* CH165 */
    oal_memcopy(g_st_cust_nv_params.st_fcc_ce_txpwer_limit.auc_5g_txpwr_20M+2*OAL_SIZEOF(int32), pl_nvram_params, OAL_SIZEOF(oal_uint8));
    pl_nvram_params++;
    /* CH134/CH102/CH62/CH38 */
    oal_memcopy(g_st_cust_nv_params.st_fcc_ce_txpwer_limit.auc_5g_txpwr_40M, pl_nvram_params, OAL_SIZEOF(int32));
    pl_nvram_params++;
    /* CH159/CH151 */
    oal_memcopy(g_st_cust_nv_params.st_fcc_ce_txpwer_limit.auc_5g_txpwr_40M+OAL_SIZEOF(int32), pl_nvram_params, 2*OAL_SIZEOF(oal_uint8));
    pl_nvram_params++;
    oal_memcopy(g_st_cust_nv_params.st_fcc_ce_txpwer_limit.auc_5g_txpwr_80M, pl_nvram_params, OAL_SIZEOF(int32));
    pl_nvram_params++;
    /* CH122/CH106/CH58/CH42 */
    oal_memcopy(g_st_cust_nv_params.st_fcc_ce_txpwer_limit.auc_5g_txpwr_80M+OAL_SIZEOF(int32), pl_nvram_params, OAL_SIZEOF(oal_uint8));
    pl_nvram_params++;
    /* CH155 */
    oal_memcopy(g_st_cust_nv_params.st_fcc_ce_txpwer_limit.auc_5g_txpwr_160M, pl_nvram_params, FCC_CE_CH_NUM_5G_160M*OAL_SIZEOF(oal_uint8));
    /* 2.4g */
    for (uc_cfg_id = 0; uc_cfg_id < MAC_2G_CHANNEL_NUM; uc_cfg_id++)
    {
        pl_nvram_params++;
        oal_memcopy(g_st_cust_nv_params.st_fcc_ce_txpwer_limit.auc_2g_txpwr[uc_cfg_id], pl_nvram_params, CUS_NUM_OF_SAR_LVL*OAL_SIZEOF(oal_uint8));
    }

    kfree(pl_fcc_txpwr_limit_params);

    return INI_SUCC;

}


OAL_STATIC int32 hwifi_config_init_nvram(void)
{
    OAL_STATIC oal_bool_enum en_nvm_initialed = OAL_FALSE;  /* 是否为第一次初始化，如果是国家码更新调用的本接口，则不再去nvm读取参数 */
    int32 l_ret = INI_FAILED;
    int32 l_cfg_id;
    int32 aul_nvram_params[NVRAM_PARAMS_INDEX_BUTT]={0};
    int32 l_plat_tag;

    OAL_MEMZERO(&g_st_cust_nv_params, OAL_SIZEOF(g_st_cust_nv_params));
    if (OAL_FALSE == en_nvm_initialed)
    {
        if (hwifi_get_regdomain_from_country_code(hwifi_get_country_code()) != REGDOMAIN_FCC)
        {
            l_ret = get_cust_conf_string(CUST_MODU_NVRAM, OAL_PTR_NULL, g_auc_nv_params, sizeof(g_auc_nv_params));

            if (INI_SUCC == l_ret && g_auc_nv_params[0] != 0)
            {
                /* 读取成功，将标志位置TRUE */
                en_nvm_initialed = OAL_TRUE;
                return INI_SUCC;
            }

            OAM_WARNING_LOG3(0, OAM_SF_ANY, "hwifi_config_init_nvram read nvram failed[ret:%d] or wrong values[first eight values:0x%x %x], read dts instead!", l_ret, *((oal_uint32*)g_auc_nv_params),*((oal_uint32*)(g_auc_nv_params+4)));
        }
        else
        {
            en_nvm_initialed = OAL_TRUE;
        }
    }

    /* read nvm failed or data not exist or country_code updated, read ini:cust_spec > cust_common > default */
    /* find plat tag */
    l_plat_tag = hwifi_get_plat_tag_from_country_code(hwifi_get_country_code());
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "hwifi_config_init_nvram plat_tag:0x%2x!", l_plat_tag);

    for (l_cfg_id = NVRAM_PARAMS_INDEX_0; l_cfg_id < NVRAM_PARAMS_INDEX_24; l_cfg_id++)
    {
        l_ret = get_cust_conf_int32(l_plat_tag, g_ast_nvram_config_ini[l_cfg_id].name, &aul_nvram_params[l_cfg_id]);

        if(INI_FAILED == l_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "hwifi_config_init_nvram read %d from ini failed!", l_cfg_id);
            /* 读取失败就使用系统默认值 */
            aul_nvram_params[l_cfg_id] = g_al_nvram_init_params[l_cfg_id];
        }
    }
    oal_memcopy(g_st_cust_nv_params.ac_delt_txpower, aul_nvram_params, sizeof(g_st_cust_nv_params.ac_delt_txpower));

    /* SAR */
    hwifi_config_init_sar_ctrl_nvram();

    /* 基准功率 */
    hwifi_config_init_base_power();

    /* FCC/CE 边带信道最大功率 */
    hwifi_config_init_fcc_ce_txpwr_nvram();
    return INI_SUCC;
}

OAL_STATIC int32 hwifi_config_init_private_custom(void)
{
    int32               l_cfg_id;
    int32               l_ret = INI_FAILED;

    for(l_cfg_id = 0; l_cfg_id < WLAN_CFG_PRIV_BUTT; l_cfg_id++)
    {
        /* 获取 private 的配置值 */
        l_ret = get_cust_conf_int32(INI_MODU_WIFI, g_ast_wifi_config_priv[l_cfg_id].name, &(g_al_priv_cust_params[l_cfg_id].l_val));

        if (INI_FAILED == l_ret)
        {
            g_al_priv_cust_params[l_cfg_id].l_val = g_al_priv_cust_init_params[l_cfg_id];
            OAM_WARNING_LOG2(0, OAM_SF_CFG, "hwifi_config_init_private_custom:cfg_id[%d]used default value[%d]",l_cfg_id, g_al_priv_cust_init_params[l_cfg_id]);
            g_al_priv_cust_params[l_cfg_id].en_value_state = OAL_FALSE;
            continue;
        }
        g_al_priv_cust_params[l_cfg_id].en_value_state = OAL_TRUE;
    }

    OAM_WARNING_LOG0(0, OAM_SF_CFG, "hwifi_config_init_private_custom read from ini success!");

    return INI_SUCC;
}


OAL_STATIC oal_uint32 hwifi_config_sepa_coefficient_from_param(oal_uint8 *puc_cust_param_info, oal_int32 *pl_coe_params,
                                                                              oal_uint16 *pus_param_num, oal_uint16 us_max_idx)
{
    oal_int8       *pc_token;
    oal_int8       *pc_ctx;
    oal_int8       *pc_end = ";";
    oal_int8       *pc_sep = ",";
    oal_uint16      us_param_num = 0;
    oal_uint8       auc_cust_param[DY_CALI_PARAMS_LEN];

    oal_memcopy(auc_cust_param, puc_cust_param_info, OAL_STRLEN(puc_cust_param_info));
    pc_token = oal_strtok(auc_cust_param, pc_end, &pc_ctx);
    if (OAL_PTR_NULL == pc_token)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CUSTOM, "hwifi_config_sepa_coefficient_from_param read get null value check!");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pc_token = oal_strtok(pc_token, pc_sep, &pc_ctx);
    /* 获取定制化系数 */
    while (pc_token)
    {
        if (us_param_num == us_max_idx)
        {
            OAM_ERROR_LOG2(0, OAM_SF_CUSTOM, "hwifi_config_sepa_coefficient_from_param::nv or ini param is too many idx[%d] Max[%d]",
                            us_param_num, us_max_idx);
            return OAL_FAIL;
        }
        OAL_IO_PRINT( "hwifi_config_sepa_coefficient_from_param get [%s]\n!", pc_token);
        *(pl_coe_params + us_param_num) = (oal_int32)oal_strtol(pc_token, OAL_PTR_NULL, 10);
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        us_param_num++;
    }

    if (us_param_num % DY_CALI_PARAMS_TIMES)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CUSTOM, "hwifi_config_sepa_coefficient_from_param read get wrong num[%d] check!", us_param_num);
        return OAL_FAIL;
    }

    *pus_param_num = us_param_num;
    return OAL_SUCC;
}


OAL_STATIC oal_void hwifi_config_get_5g_curv_switch_point(oal_uint8 *puc_ini_pa_params, oal_uint32 ul_cfg_id)
{
    oal_int32        l_ini_params[CUS_NUM_5G_BW*DY_CALI_PARAMS_TIMES] = {0};
    oal_uint16       us_ini_param_num = 0;
    oal_uint8        uc_secon_ratio_idx = 0;
    oal_uint8        uc_param_idx;
    oal_int16       *ps_extre_point_val;

    /* 获取拟合系数项 */
    if (OAL_SUCC != hwifi_config_sepa_coefficient_from_param(puc_ini_pa_params, l_ini_params, &us_ini_param_num, OAL_SIZEOF(l_ini_params)/OAL_SIZEOF(oal_int32)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_CUSTOM, "hwifi_config_get_5g_curv_switch_point::ini is unsuitable,num of ini[%d] cfg_id[%d]!", us_ini_param_num, ul_cfg_id);
        return;
    }

    ps_extre_point_val = gs_extre_point_vals;
    us_ini_param_num /= DY_CALI_PARAMS_TIMES;
    if (ul_cfg_id <= WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0)
    {
        if (us_ini_param_num != CUS_NUM_5G_BW)
        {
            OAM_ERROR_LOG2(0, OAM_SF_CUSTOM, "hwifi_config_get_5g_curv_switch_point::ul_cfg_id[%d] us_ini_param_num[%d]", ul_cfg_id, us_ini_param_num);
            return;
        }
        /* 第一个数据留给band1 */
        ps_extre_point_val++;
    }
    else
    {
        if (us_ini_param_num != 1)
        {
            OAM_ERROR_LOG2(0, OAM_SF_CUSTOM, "hwifi_config_get_5g_curv_switch_point::ul_cfg_id[%d] us_ini_param_num[%d]", ul_cfg_id, us_ini_param_num);
            return;
        }
    }

    /* 计算5g曲线switch point */
    for (uc_param_idx = 0; uc_param_idx < us_ini_param_num; uc_param_idx++)
    {
        *(ps_extre_point_val + uc_param_idx) = (oal_int16)HWIFI_DYN_CALI_GET_EXTRE_POINT(l_ini_params+uc_secon_ratio_idx);
        OAL_IO_PRINT("hwifi_config_get_5g_curv_switch_point::extre power[%d] param_idx[%d] cfg_id[%d]!\r\n",
                      *(ps_extre_point_val + uc_param_idx), uc_param_idx, ul_cfg_id);
        OAL_IO_PRINT("hwifi_config_get_5g_curv_switch_point::param[%d %d] uc_secon_ratio_idx[%d]!\r\n",
                      (l_ini_params+uc_secon_ratio_idx)[0], (l_ini_params+uc_secon_ratio_idx)[1], uc_secon_ratio_idx);
        uc_secon_ratio_idx += DY_CALI_PARAMS_TIMES;
    }

    return;
}


OAL_STATIC oal_uint32 hwifi_config_nvram_second_coefficient_check(oal_uint8 *puc_cust_nvram_info, oal_uint8 *puc_ini_pa_params,
                                                                  oal_uint32 ul_cfg_id, oal_int16 *ps_5g_delt_power)
{
    oal_int32        l_ini_params[CUS_NUM_5G_BW*DY_CALI_PARAMS_TIMES] = {0};
    oal_int32        l_nv_params[CUS_NUM_5G_BW*DY_CALI_PARAMS_TIMES] = {0};
    oal_uint16       us_ini_param_num = 0;
    oal_uint16       us_nv_param_num = 0;
    oal_uint8        uc_secon_ratio_idx = 0;
    oal_uint8        uc_param_idx;

    /* 获取拟合系数项 */
    if (OAL_SUCC != hwifi_config_sepa_coefficient_from_param(puc_cust_nvram_info, l_nv_params, &us_nv_param_num, OAL_SIZEOF(l_nv_params)) ||
          OAL_SUCC != hwifi_config_sepa_coefficient_from_param(puc_ini_pa_params, l_ini_params, &us_ini_param_num, OAL_SIZEOF(l_ini_params)) ||
          (us_nv_param_num != us_ini_param_num))
    {
        OAM_ERROR_LOG2(0, OAM_SF_CUSTOM, "hwifi_config_nvram_second_coefficient_check::nvram or ini is unsuitable,num of nv and ini[%d %d]!",
                       us_nv_param_num, us_ini_param_num);
        return OAL_FAIL;
    }

    us_nv_param_num /= DY_CALI_PARAMS_TIMES;
    /* 检查nv和ini中二次系数是否匹配 */
    for (uc_param_idx = 0; uc_param_idx < us_nv_param_num; uc_param_idx++)
    {
        if (l_ini_params[uc_secon_ratio_idx] != l_nv_params[uc_secon_ratio_idx])
        {
            OAM_WARNING_LOG4(0, OAM_SF_CUSTOM, "hwifi_config_nvram_second_coefficient_check::nvram get mismatch value idx[%d %d] val are [%d] and [%d]!",
                           uc_param_idx, uc_secon_ratio_idx, l_ini_params[uc_secon_ratio_idx], l_nv_params[uc_secon_ratio_idx]);
            /* 量产后二次系数以nvram中为准，刷新NV中的二次拟合曲线切换点 */
            hwifi_config_get_5g_curv_switch_point(puc_cust_nvram_info, ul_cfg_id);
            uc_secon_ratio_idx += DY_CALI_PARAMS_TIMES;
            continue;
        }

        if (WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0 == ul_cfg_id)
        {
            /* 计算产线上的delt power */
            *(ps_5g_delt_power + uc_param_idx) = HWIFI_GET_5G_PRO_LINE_DELT_POW_PER_BAND(l_nv_params+uc_secon_ratio_idx, l_ini_params+uc_secon_ratio_idx);
            OAL_IO_PRINT("hwifi_config_nvram_second_coefficient_check::delt power[%d] param_idx[%d] cfg_id[%d]!\r\n",
                          *(ps_5g_delt_power + uc_param_idx), uc_param_idx, ul_cfg_id);
        }
        uc_secon_ratio_idx += DY_CALI_PARAMS_TIMES;
    }
    return OAL_SUCC;
}



OAL_STATIC oal_uint32 hwifi_config_init_dy_cali_custom(oal_void)
{
    oal_uint32            ul_cfg_id;
    oal_uint32            ul_ret = OAL_SUCC;
    oal_uint8             auc_ini_pa_params[DY_CALI_PARAMS_LEN]={0};
    oal_uint8             uc_idx = 0;
    oal_uint16            us_param_num = 0;
    oal_uint16            us_per_param_num = 0;
    oal_uint8             uc_cali_param_idx;
    oal_int32             l_params[DY_CALI_PARAMS_TIMES*DY_CALI_PARAMS_NUM] = {0};
    oal_uint8            *puc_cust_nvram_info = OAL_PTR_NULL;
    oal_int16             s_5g_delt_power[CUS_NUM_5G_BW] = {0};
    oal_uint8             uc_delt_pwr_idx = 0;
    oal_uint32            ul_cfg_id_tmp;

    for(ul_cfg_id = WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0; ul_cfg_id < WLAN_CFG_DTS_NVRAM_PARAMS_BUTT; ul_cfg_id++)
    {
        /* 二次拟合系数 */
        if ((ul_cfg_id >= WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0) && (ul_cfg_id <= WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C0))
        {
            continue;
        }
        if(INI_FAILED == get_cust_conf_string(INI_MODU_WIFI, g_ast_nvram_pro_line_config_ini[ul_cfg_id].name, auc_ini_pa_params, DY_CALI_PARAMS_LEN-1))
        {
            if ((ul_cfg_id >= WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1_LOW) || (ul_cfg_id <= WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW))
            {
                ul_cfg_id_tmp = ((ul_cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1_LOW) ? WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1 :
                                 (ul_cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW) ? WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0 : ul_cfg_id);
                get_cust_conf_string(INI_MODU_WIFI, g_ast_nvram_pro_line_config_ini[ul_cfg_id_tmp].name, auc_ini_pa_params, DY_CALI_PARAMS_LEN-1);
            }
            else
            {
                OAM_ERROR_LOG1(0, OAM_SF_CUSTOM, "hwifi_config_init_dy_cali_custom read, check id[%d] exists!", ul_cfg_id);
                ul_ret = OAL_FAIL;
                break;
            }
        }

        if ((WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0 == ul_cfg_id) || (WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1 == ul_cfg_id))
        {
            /* 获取ini中的二次拟合曲线切换点 */
            hwifi_config_get_5g_curv_switch_point(auc_ini_pa_params, ul_cfg_id);
        }

        if (WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0 >= ul_cfg_id)
        {
            puc_cust_nvram_info = hwifi_get_nvram_param(ul_cfg_id);
            /* 先取nv中的参数值,为空则从ini文件中读取 */
            if (OAL_STRLEN(puc_cust_nvram_info))
            {
                /* NVRAM二次系数异常保护 */
                if (OAL_SUCC == hwifi_config_nvram_second_coefficient_check(puc_cust_nvram_info, auc_ini_pa_params, ul_cfg_id, s_5g_delt_power))
                {
                    if ((WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW == ul_cfg_id) && (oal_memcmp(puc_cust_nvram_info, auc_ini_pa_params, OAL_STRLEN(puc_cust_nvram_info))))
                    {
                        OAL_MEMZERO(s_5g_delt_power, OAL_SIZEOF(s_5g_delt_power));
                    }
                    oal_memcopy(auc_ini_pa_params, puc_cust_nvram_info, OAL_STRLEN(puc_cust_nvram_info));
                }
                else
                {
                    ul_ret = OAL_FAIL;
                    break;
                }
            }
            else
            {
                /* 提供产线第一次上电校准初始值 */
                oal_memcopy(puc_cust_nvram_info, auc_ini_pa_params, OAL_STRLEN(auc_ini_pa_params));
            }
        }

        if (OAL_SUCC != hwifi_config_sepa_coefficient_from_param(auc_ini_pa_params, l_params + us_param_num, &us_per_param_num, OAL_SIZEOF(l_params)-us_param_num))
        {
            ul_ret = OAL_FAIL;
            OAM_ERROR_LOG2(0, OAM_SF_CUSTOM, "hwifi_config_init_dy_cali_custom read get wrong value,len[%d] check id[%d] exists!",
                           OAL_STRLEN(puc_cust_nvram_info), ul_cfg_id);
            break;
        }
        us_param_num += us_per_param_num;
    }

    if (OAL_FAIL == ul_ret)
    {
        /* 置零防止下发到device */
        OAL_MEMZERO(g_as_pro_line_params, OAL_SIZEOF(g_as_pro_line_params));
    }
    else
    {
        if (us_param_num != OAL_SIZEOF(l_params)/OAL_SIZEOF(oal_int32))
        {
            OAM_ERROR_LOG1(0, OAM_SF_CUSTOM, "hwifi_config_init_dy_cali_custom read get wrong ini value num[%d]!", us_param_num);
            OAL_MEMZERO(g_as_pro_line_params, OAL_SIZEOF(g_as_pro_line_params));
            return OAL_FAIL;
        }

        for (uc_cali_param_idx = 0; uc_cali_param_idx < DY_CALI_PARAMS_BASE_NUM; uc_cali_param_idx++)
        {
            if (DY_2G_CALI_PARAMS_NUM-1 == uc_cali_param_idx)
            {
                /* band1 & CW */
                uc_cali_param_idx+=2;
            }
            g_as_pro_line_params[uc_cali_param_idx].l_pow_par2 = l_params[uc_idx++];
            g_as_pro_line_params[uc_cali_param_idx].l_pow_par1 = l_params[uc_idx++];
            g_as_pro_line_params[uc_cali_param_idx].l_pow_par0 = l_params[uc_idx++];
        }

        /* band1 */
        g_as_pro_line_params[DY_2G_CALI_PARAMS_NUM].l_pow_par2 = l_params[uc_idx++];
        g_as_pro_line_params[DY_2G_CALI_PARAMS_NUM].l_pow_par1 = l_params[uc_idx++];
        g_as_pro_line_params[DY_2G_CALI_PARAMS_NUM].l_pow_par0 = l_params[uc_idx++];

        /* CW */
        g_as_pro_line_params[DY_2G_CALI_PARAMS_NUM-1].l_pow_par2 = l_params[uc_idx++];
        g_as_pro_line_params[DY_2G_CALI_PARAMS_NUM-1].l_pow_par1 = l_params[uc_idx++];
        g_as_pro_line_params[DY_2G_CALI_PARAMS_NUM-1].l_pow_par0 = l_params[uc_idx++];

        /* 5g low power */
        for (uc_cali_param_idx = DY_CALI_PARAMS_BASE_NUM; uc_cali_param_idx < (DY_CALI_PARAMS_NUM_WLAN - 1); uc_cali_param_idx++)
        {
            g_as_pro_line_params[uc_cali_param_idx].l_pow_par2 = l_params[uc_idx++];
            g_as_pro_line_params[uc_cali_param_idx].l_pow_par1 = l_params[uc_idx++];
            g_as_pro_line_params[uc_cali_param_idx].l_pow_par0 = l_params[uc_idx++];

            if (uc_cali_param_idx == DY_CALI_PARAMS_BASE_NUM)
            {
                /* band1产线不校准 */
                continue;
            }

            CUS_FLUSH_NV_RATIO_BY_DELT_POW(g_as_pro_line_params[uc_cali_param_idx].l_pow_par2,
                                           g_as_pro_line_params[uc_cali_param_idx].l_pow_par1,
                                           g_as_pro_line_params[uc_cali_param_idx].l_pow_par0,
                                           s_5g_delt_power[uc_delt_pwr_idx]);

            uc_delt_pwr_idx++;
        }

        /* 2g cw ppa */
        g_as_pro_line_params[DY_CALI_PARAMS_NUM_WLAN-1].l_pow_par2 = l_params[uc_idx++];
        g_as_pro_line_params[DY_CALI_PARAMS_NUM_WLAN-1].l_pow_par1 = l_params[uc_idx++];
        g_as_pro_line_params[DY_CALI_PARAMS_NUM_WLAN-1].l_pow_par0 = l_params[uc_idx++];

        // BT power fit params
        g_as_pro_line_params[DY_CALI_PARAMS_NUM_WLAN].l_pow_par2 = l_params[uc_idx++];
        g_as_pro_line_params[DY_CALI_PARAMS_NUM_WLAN].l_pow_par1 = l_params[uc_idx++];
        g_as_pro_line_params[DY_CALI_PARAMS_NUM_WLAN].l_pow_par0 = l_params[uc_idx++];
        // BT PPA Vdet fit params
        g_as_pro_line_params[DY_CALI_PARAMS_NUM_WLAN+1].l_pow_par2 = l_params[uc_idx++];
        g_as_pro_line_params[DY_CALI_PARAMS_NUM_WLAN+1].l_pow_par1 = l_params[uc_idx++];
        g_as_pro_line_params[DY_CALI_PARAMS_NUM_WLAN+1].l_pow_par0 = l_params[uc_idx++];
    }

    return ul_ret;
}


OAL_STATIC oal_uint32 hwifi_config_sepa_dpn_from_param(oal_uint8 *puc_cust_param_info, oal_int32 *pl_dpn_params,
                                                                              oal_uint16 *pus_param_num, oal_uint16 us_max_idx)
{
    oal_int8       *pc_token;
    oal_int8       *pc_ctx;
    oal_int8       *pc_end = ";";
    oal_int8       *pc_sep = ",";
    oal_uint16      us_param_num = 0;
    oal_uint8       auc_cust_param[DY_CALI_PARAMS_LEN];

    oal_memcopy(auc_cust_param, puc_cust_param_info, OAL_STRLEN(puc_cust_param_info));
    pc_token = oal_strtok(auc_cust_param, pc_end, &pc_ctx);
    if (OAL_PTR_NULL == pc_token)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CUSTOM, "hwifi_config_sepa_dpn_from_param read get null value check!");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pc_token = oal_strtok(pc_token, pc_sep, &pc_ctx);
    /* 获取定制化系数 */
    while (pc_token)
    {
        if (us_param_num == us_max_idx)
        {
            OAM_ERROR_LOG2(0, OAM_SF_CUSTOM, "hwifi_config_sepa_dpn_from_param::ini item has too many parameters[%d] max[%d]",
                            us_param_num, us_max_idx);
            return OAL_FAIL;
        }
        OAL_IO_PRINT( "hwifi_config_sepa_dpn_from_param get [%s]\n!", pc_token);
        *(pl_dpn_params + us_param_num) = (oal_int32)oal_strtol(pc_token, OAL_PTR_NULL, 10);
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        us_param_num++;
    }

    *pus_param_num = us_param_num;
    return OAL_SUCC;
}


oal_uint32 hwifi_config_get_bt_dpn(oal_int32 *pl_params, oal_uint16 size)
{
    oal_uint32                  ul_cfg_id;
    oal_uint8                   auc_ini_params[DY_CALI_PARAMS_LEN] = {0};
    oal_uint16                  us_param_num = 0;
    oal_uint16                  us_per_param_num = 0;

    for (ul_cfg_id = WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND1; ul_cfg_id <= WLAN_CFG_DTS_BT_CALI_TXPWR_DPN_BAND8; ul_cfg_id++)
    {
        if (INI_FAILED == get_cust_conf_string(INI_MODU_WIFI, g_ast_wifi_config_dts[ul_cfg_id].name, auc_ini_params, DY_CALI_PARAMS_LEN - 1))
        {
            OAM_ERROR_LOG1(0, OAM_SF_CUSTOM, "hwifi_config_init_dts_cali read, check id[%d] exists!", ul_cfg_id);
            return OAL_TRUE;
            break;
        }

        if (OAL_SUCC != hwifi_config_sepa_dpn_from_param(auc_ini_params, pl_params + us_param_num, &us_per_param_num, size - us_param_num))
        {
            OAM_ERROR_LOG2(0, OAM_SF_CUSTOM, "hwifi_config_init_dts_cali read get wrong value,len[%d] check id[%d] exists!",
                           OAL_STRLEN(auc_ini_params), ul_cfg_id);
            return OAL_TRUE;
            break;
        }
        us_param_num += us_per_param_num;
    }

    return OAL_FALSE;
}


int32 hwifi_config_init(int32 cus_tag)
{
    int32               l_cfg_id;
    int32               l_ret = INI_FAILED;
    int32               l_ori_val;
    wlan_cfg_cmd*       pgast_wifi_config;
    int32*              pgal_params;
    int32               l_cfg_value = 0;
    int32               l_wlan_cfg_butt;

    switch (cus_tag)
    {
        case CUS_TAG_NV:
            return hwifi_config_init_nvram();
        case CUS_TAG_INI:
            host_params_init_first();
            pgast_wifi_config = g_ast_wifi_config_cmds;
            pgal_params = g_al_host_init_params;
            l_wlan_cfg_butt = WLAN_CFG_INIT_BUTT;
            break;
        case CUS_TAG_DTS:
            original_value_for_dts_params();
            pgast_wifi_config = g_ast_wifi_config_dts;
            pgal_params = g_al_dts_params;
            l_wlan_cfg_butt = WLAN_CFG_DTS_BUTT;
            break;
        case CUS_TAG_PRIV_INI:
            return hwifi_config_init_private_custom();
        case CUS_TAG_PRO_LINE_INI:
            return hwifi_config_init_dy_cali_custom();
        default:
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "hisi_customize_wifi tag number[0x%x] not correct!", cus_tag);
            return INI_FAILED;
    }

    for(l_cfg_id=0; l_cfg_id<l_wlan_cfg_butt; ++l_cfg_id)
    {
        /* 获取ini的配置值 */
        l_ret = get_cust_conf_int32(INI_MODU_WIFI, pgast_wifi_config[l_cfg_id].name, &l_cfg_value);

        if (INI_FAILED == l_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "hisi_customize_wifi read ini file failed, check if cfg_id[%d] exists!", l_cfg_id);
            continue;
        }

        l_ori_val = pgal_params[l_cfg_id];
        pgal_params[l_cfg_id] = l_cfg_value;
        OAM_WARNING_LOG3(0, OAM_SF_ANY, "hisi_customize_wifi [cfg_id:%d]value changed from [init:%d] to [config:%d]. \n", l_cfg_id, l_ori_val, pgal_params[l_cfg_id]);
    }

    return INI_SUCC;
}

OAL_STATIC int char2byte( char* strori, char* outbuf )
{
    int i = 0;
    int temp = 0;
    int sum = 0;

    for( i = 0; i < 12; i++ )
    {
        switch (strori[i]) {
            case '0' ... '9':
                temp = strori[i] - '0';
                break;

            case 'a' ... 'f':
                temp = strori[i] - 'a' + 10;
                break;

            case 'A' ... 'F':
                temp = strori[i] - 'A' + 10;
                break;
        }

        sum += temp;
        if( i % 2 == 0 ){
            outbuf[i/2] |= temp << 4;
        }
        else{
            outbuf[i/2] |= temp;
        }
    }

    return sum;
}

int32 hwifi_get_mac_addr(uint8 *puc_buf)
{
    struct hisi_nve_info_user st_info;
    int32 l_ret = -1;
    int32 l_sum = 0;

    if (NULL == puc_buf)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hisi_customize_wifi::buf is NULL!");
        return INI_FAILED;
    }

    oal_memset(puc_buf, 0, MAC_LEN);

    oal_memset(&st_info, 0, sizeof(st_info));
    st_info.nv_number  = NV_WLAN_NUM;   //nve item

    strncpy(st_info.nv_name, "MACWLAN", sizeof("MACWLAN"));

    st_info.valid_size = NV_WLAN_VALID_SIZE;
    st_info.nv_operation = NV_READ;

    if (0 != g_auc_wifimac[0] || 0 != g_auc_wifimac[1] || 0 != g_auc_wifimac[2] || 0 != g_auc_wifimac[3]
        || 0 != g_auc_wifimac[4] || 0 != g_auc_wifimac[5])
    {
        memcpy(puc_buf, g_auc_wifimac, MAC_LEN);
        return INI_SUCC;
    }

    l_ret = hisi_nve_direct_access(&st_info);

    if (!l_ret)
    {
        l_sum = char2byte(st_info.nv_data, puc_buf);
        if (0 != l_sum)
        {
            INI_WARNING("hisi_customize_wifi get MAC from NV: mac="MACFMT"\n", MAC2STR(puc_buf));
            oal_memcopy(g_auc_wifimac, puc_buf, MAC_LEN);
        }else{
            random_ether_addr(puc_buf);
            puc_buf[1] = 0x11;
            puc_buf[2] = 0x02;
        }
    }else{
        random_ether_addr(puc_buf);
        puc_buf[1] = 0x11;
        puc_buf[2] = 0x02;
    }

    return INI_SUCC;
}

int32 hwifi_get_init_value(int32 cus_tag, int32 cfg_id)
{
    int32*              pgal_params = OAL_PTR_NULL;
    int32               l_wlan_cfg_butt;

    if (CUS_TAG_INI == cus_tag)
    {
        pgal_params = &g_al_host_init_params[0];
        l_wlan_cfg_butt = WLAN_CFG_INIT_BUTT;
    }
    else if (CUS_TAG_DTS == cus_tag)
    {
        pgal_params = &g_al_dts_params[0];
        l_wlan_cfg_butt = WLAN_CFG_DTS_BUTT;
    }
    else
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "hisi_customize_wifi tag number[0x%2x] not correct!", cus_tag);
        return INI_FAILED;
    }

    if (0 > cfg_id || l_wlan_cfg_butt <= cfg_id)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "hisi_customize_wifi cfg id[%d] out of range, max cfg id is:%d", cfg_id, l_wlan_cfg_butt-1);
        return INI_FAILED;
    }
    return pgal_params[cfg_id];
}

int8* hwifi_get_country_code(void)
{
    int32 l_ret;

    if (g_ac_country_code[0] != '0' && g_ac_country_code[1] != '0')
    {
        return g_ac_country_code;
    }

    /* 获取cust国家码 */
    l_ret = get_cust_conf_string(INI_MODU_WIFI, STR_COUNTRY_CODE, g_ac_country_code, sizeof(g_ac_country_code)-1);

    if(INI_FAILED == l_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "hisi_customize_wifi read country code failed, check if it exists!");
        strncpy(g_ac_country_code, "99", 2);
    }
    else
    {
        if (!OAL_MEMCMP(g_ac_country_code, "99", 2))
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "hwifi_get_country_code is set 99!");
            g_st_cust_country_code_ignore_flag.en_country_code_ingore_ini_flag = OAL_TRUE;
        }
    }

    g_ac_country_code[2] = '\0';


    return g_ac_country_code;
}


void hwifi_set_country_code(int8* country_code, const uint32 len)
{
    if (OAL_PTR_NULL == country_code || len != COUNTRY_CODE_LEN)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hwifi_get_country_code ptr null or illegal len!");
        return;
    }

    oal_memcopy(g_ac_country_code, country_code, COUNTRY_CODE_LEN);
    g_ac_country_code[2] = '\0';

    return;
}

wlan_customize_power_params_stru* hwifi_get_nvram_params(void)
{
    return &g_st_cust_nv_params;
}


int32 hwifi_atcmd_update_host_nv_params(void)
{
    int32 l_ret = INI_FAILED;

    oal_memset(g_auc_nv_params, 0x00, sizeof(g_auc_nv_params));

    l_ret = get_cust_conf_string(CUST_MODU_NVRAM, OAL_PTR_NULL, g_auc_nv_params, sizeof(g_auc_nv_params));

    if (INI_FAILED == l_ret || !g_auc_nv_params[0])
    {
        /* 正常流程必须返回成功，失败则本次校准失败，不应该再按正常流程走，直接返回失败 */
        /* 失败原因最大可能是在调用产校命令之前写入NV的操作就已经失败导致NV区域为空 */
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "hwifi_atcmd_update_host_nv_params::read nvram params failed or nv is empty, ret=[%d], nv_param[%u]!!", l_ret, g_auc_nv_params[0]);
        oal_memset(g_auc_nv_params, 0x00, sizeof(g_auc_nv_params));
        return INI_FAILED;
    }

    OAM_WARNING_LOG0(0, OAM_SF_ANY, "hwifi_atcmd_update_host_nv_params::update nvram params succ.");
    return INI_SUCC;
}
/* 导出符号 */
EXPORT_SYMBOL_GPL(g_st_cust_country_code_ignore_flag);
EXPORT_SYMBOL_GPL(g_st_wlan_customize);
EXPORT_SYMBOL_GPL(g_as_pro_line_params);
EXPORT_SYMBOL_GPL(gs_extre_point_vals);
EXPORT_SYMBOL_GPL(g_en_fact_cali_completed);
EXPORT_SYMBOL_GPL(hwifi_config_init);
EXPORT_SYMBOL_GPL(hwifi_get_mac_addr);
EXPORT_SYMBOL_GPL(hwifi_get_init_value);
EXPORT_SYMBOL_GPL(hwifi_get_country_code);
EXPORT_SYMBOL_GPL(hwifi_get_nvram_params);
EXPORT_SYMBOL_GPL(hwifi_fetch_ori_caldata);
EXPORT_SYMBOL_GPL(hwifi_set_country_code);
EXPORT_SYMBOL_GPL(hwifi_is_regdomain_changed);
EXPORT_SYMBOL_GPL(hwifi_atcmd_update_host_nv_params);
EXPORT_SYMBOL_GPL(hwifi_custom_host_read_cfg_init);
EXPORT_SYMBOL_GPL(hwifi_get_init_priv_value);
EXPORT_SYMBOL_GPL(hwifi_get_nvram_param);
EXPORT_SYMBOL_GPL(g_en_nv_dp_init_is_null);
EXPORT_SYMBOL_GPL(hwifi_config_init_base_power);
EXPORT_SYMBOL_GPL(hwifi_get_plat_tag_from_country_code);
EXPORT_SYMBOL_GPL(hwifi_get_regdomain_from_country_code);
EXPORT_SYMBOL_GPL(g_auc_sar_params);
EXPORT_SYMBOL_GPL(hwifi_config_get_bt_dpn);


#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
