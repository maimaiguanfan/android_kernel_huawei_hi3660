

#ifndef __HAL_EXT_IF_H__
#define __HAL_EXT_IF_H__

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
#include "frw_ext_if.h"
#include "wlan_types.h"
#include "hal_commom_ops.h"
#include "oal_hardware.h"
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_EXT_IF_H

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
extern oal_int32 g_l_rf_channel_num;
extern oal_int32 g_l_rf_single_tran;

#ifdef _PRE_WLAN_FEATURE_DOUBLE_CHIP
/* ˫оƬ����, �Ӷ��ƻ��ļ��л�ȡ2g��5g��chip id */
extern oal_uint8 g_uc_wlan_double_chip_2g_id;
extern oal_uint8 g_uc_wlan_double_chip_5g_id;
#else
extern oal_uint8 g_uc_wlan_single_chip_2g_id;
extern oal_uint8 g_uc_wlan_single_chip_5g_id;
#endif


#endif
extern oal_int32 g_l_rf_fem_switch;
extern oal_uint8 g_auc_sar_ctrl_params[HAL_CUS_NUM_OF_SAR_PARAMS];
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
extern OAL_CONST oal_int16 g_aus_cutom_simple_txpwr_table[];
extern oal_uint8 g_uc_5g_fcc_ce_high_band_max_pwr;
#endif

extern hal_pwr_efuse_amend_stru g_ast_efuse_G_D[WLAN_RF_CHANNEL_NUMS][WLAN_BAND_BUTT];

extern  frw_timeout_stru  g_st_check_pll_lock_sts_timer;

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define HAL_TX_STATS_MAX_NUMS          (256)
#define HAL_RX_LPF_GAIN                (6)
#define HAL_DBM_CH                     (-13)
#define HAL_SINGLE_DOUBLE_SWITCH_GAIN  (3)
#define HAL_RSSI_REF_DIFFERENCE        (20)
#define HAL_RADAR_REF_DIFFERENCE       (24)

#define HAL_PA_ERROR_OFFSET 3
#define HAL_TX_QEUEU_MAX_PPDU_NUM   2 /* DMACҲ��һ�ף�����ϲ�����SPEC�� */

#define HAL_MAC_ERROR_THRESHOLD     10  /* macӲ���������ޣ��������Э��Ӳ������ȥ������λ���� */
/* 0.1dbm��λ */
#define HAL_RSSI_SIGNAL_MIN                    (-1030)  /*�źſ����Сֵ */
#define HAL_RSSI_SIGNAL_MAX                    (50)     /*�źſ�����ֵ*/
#define HAL_INVALID_SIGNAL_INITIAL             (1000)    /*�Ƿ���ʼ�źż���ֵ*/

#define HAL_ANT_SWITCH_RSSI_HIGH_TH              30 /* ant0��ant1���30dBʱ�л���SISO */
#define HAL_ANT_SWITCH_RSSI_LOW_TH               10  /* ant0��ant1���С��10dBʱ�л���MIMO */
#define HAL_ANT_SWITCH_RSSI_HIGH_CNT             1000 /* ά��1000֡��������ֵ���л� */
#define HAL_ANT_SWITCH_RSSI_LOW_CNT              100
#define HAL_ANT_SWITCH_MIN_RSSI_TH              -80

#define HAL_ANT_SWITCH_RSSI_MGMT_STRONG_TH       10  /* ǿ�ź��¹���֡ant0��ant1���10dBʱ�л���SISO */
#define HAL_ANT_SWITCH_RSSI_MGMT_WEAK_TH         3   /* ���ź��¹���֡ant0��ant1���3dBʱ�л���SISO */
#define HAL_ANT_SWITCH_RSSI_VALID_MAX_TH         3   /* ����֡new mgmt chain��������max�Ĵ��� */

#define HAL_ANT_SWITCH_RSSI_TBTT_CNT_TH          50   /* 50��tbtt�жϴ���һ��̽�� */
#define HAL_ANT_SWITCH_RSSI_MIMO_TBTT_OPEN_TH    50   /* 50��tbtt�жϴ���һ��̽�� */
#define HAL_ANT_SWITCH_RSSI_MIMO_TBTT_CLOSE_TH   10   /* 50��tbtt�жϴ���һ��̽�� */
#define HAL_ANT_SWITCH_RSSI_HT_DIFF_TH           10   /* HTЭ����20M�Ĳ�ֵ���ޣ�MCS12������-MCS7������ */
#define HAL_ANT_SWITCH_RSSI_VHT_DIFF_TH          12   /* VHTЭ����20M�Ĳ�ֵ���ޣ�˫��MCS4������-����MCS9������ */
#define HAL_ANT_SWITCH_RSSI_HT_MIN_TH            -86  /* HTЭ���µ���Сֵ���ޣ�MCS11��������+3db */
#define HAL_ANT_SWITCH_RSSI_VHT_MIN_TH           -82  /* VHTЭ���µ���Сֵ���ޣ�˫��MCS4��������+3db */
#define HAL_ANT_SWITCH_RSSI_HT_MAX_TH            -68
#define HAL_ANT_SWITCH_RSSI_VHT_MAX_TH           -66

#define HAL_ANT_SWITCH_RSSI_MGMT_ENABLE             BIT0
#define HAL_ANT_SWITCH_RSSI_DATA_ENABLE             BIT1

#define HAL_CCA_OPT_ED_HIGH_20TH_DEF       (-62)        /* CCA 20M������޼Ĵ���Ĭ��ֵ */
#define HAL_CCA_OPT_ED_HIGH_40TH_DEF       (-59)        /* CCA 40M������޼Ĵ���Ĭ��ֵ */
#define HAL_CCA_OPT_ED_HIGH_80TH_DEF       (-56)        /* CCA 80M������޼Ĵ���Ĭ��ֵ */

#define HAL_CCA_OPT_ED_LOW_TH_DSSS_DEF     (-76)        /* DSSS�źŵ�Ĭ��Э������ */
#define HAL_CCA_OPT_ED_LOW_TH_OFDM_DEF     (-82)        /* OFDM�źŵ�Ĭ��Э������ */
#define HAL_CCA_OPT_ED_LOW_TH_40M_HIGH_DEF (-72)        /* ��40M�����Ĭ��Э������ */
#define HAL_CCA_OPT_ED_LOW_TH_80M_HIGH_DEF (-69)        /* ��80M�����Ĭ��Э������ */

#define HAL_CCA_OPT_ED_LOW_TH_DSSS_MIN     (-88)        /* DSSS�źŵ���СЭ������ */
#define HAL_CCA_OPT_ED_LOW_TH_OFDM_MIN     (-88)        /* OFDM�źŵ���СЭ������ */
#define HAL_CCA_OPT_ED_LOW_TH_40M_HIGH_MIN (-85)        /* ��40M����СЭ������ */
#define HAL_CCA_OPT_ED_LOW_TH_80M_HIGH_MIN (-82)        /* ��80M����СЭ������ */

#define HAL_CCA_OPT_ED_HYST_STEP_20TH_DEF       (2)        /* CCA 20M ���и��ʼ�ⰲȫ���� */
#define HAL_CCA_OPT_ED_HYST_STEP_40TH_DEF       (2)        /* CCA 40M ���и��ʼ�ⰲȫ���� */

#ifdef _PRE_WLAN_PRODUCT_1151V200
#define HAL_RF_TEMP_INVALID                 (-65)
#define HAL_RF_TEMP_CHANGE_TH               3
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#define HAL_CCA_OPT_GET_DEFAULT_ED_20TH(_band, _cust) \
    ((WLAN_BAND_2G == (_band)) ? \
     ((_cust)->c_delta_cca_ed_high_20th_2g + HAL_CCA_OPT_ED_HIGH_20TH_DEF) : \
     ((_cust)->c_delta_cca_ed_high_20th_5g + HAL_CCA_OPT_ED_HIGH_20TH_DEF))
#define HAL_CCA_OPT_GET_DEFAULT_ED_40TH(_band, _cust) \
    ((WLAN_BAND_2G == (_band)) ? \
     ((_cust)->c_delta_cca_ed_high_40th_2g + HAL_CCA_OPT_ED_HIGH_40TH_DEF) : \
     ((_cust)->c_delta_cca_ed_high_40th_5g + HAL_CCA_OPT_ED_HIGH_40TH_DEF))
#define HAL_CCA_OPT_GET_DEFAULT_ED_80TH(_default) \
         (_default + HAL_CCA_OPT_ED_HIGH_80TH_DEF)
#endif

#ifdef _PRE_WLAN_DFT_STAT
#define HAL_DFT_AP_BEACON_MISS_MAX_NUM(_idx, _val, _pul_val)  do {   \
             *_pul_val = ((0xFF << ((_idx) << 3)) & (_val)) >> ((_idx) << 3);  \
             }while(0)

#define HAL_DFT_PHY_STAT_NODE_0_BASE           0   /* phyͳ�ƽڵ�0�Ļ�׼bit���Ĵ���PHY_STA_01_EN��bit0 */
#define HAL_DFT_PHY_STAT_NODE_1_BASE           16  /* phyͳ�ƽڵ�1�Ļ�׼bit���Ĵ���PHY_STA_01_EN��bit16 */
#define HAL_DFT_PHY_STAT_NODE_2_BASE           0   /* phyͳ�ƽڵ�2�Ļ�׼bit���Ĵ���PHY_STA_23_EN��bit0 */
#define HAL_DFT_PHY_STAT_NODE_3_BASE           16  /* phyͳ�ƽڵ�3�Ļ�׼bit���Ĵ���PHY_STA_23_EN��bit16 */
#endif

#define HAL_POW_GET_HE_TRIG_AP_TX_POWER(_rpt_trig_ap_tx_power_idx) ((oal_int8)(_rpt_trig_ap_tx_power_idx - 20))   /* APָ���ķ��书��,0~60ӳ�䵽-20dbm~40dbm */

#define HAL_FOR_EACH_HAL_DEV(_uc_dev_idx)   \
        for ((_uc_dev_idx) = 0; (_uc_dev_idx) < WLAN_DEVICE_MAX_NUM_PER_CHIP; (_uc_dev_idx)++)

#define GET_HAL_DEVICE_STATE(_pst_hal_device)         ((_pst_hal_device)->st_hal_dev_fsm.st_oal_fsm.uc_cur_state)/* ��ȡ��ǰ״̬����״̬ */
#define GET_HAL_DEVICE_PREV_STATE(_pst_hal_device)    ((_pst_hal_device)->st_hal_dev_fsm.st_oal_fsm.uc_prev_state)/* ��ȡ��ǰ״̬������һ��״̬ */
#define GET_WORK_SUB_STATE(_pst_hal_device)           ((_pst_hal_device)->st_hal_dev_fsm.st_oal_work_sub_fsm.uc_cur_state)/* ��ȡwork����״̬ */
#define GET_HAL_DEV_CURRENT_SCAN_IDX(_pst_hal_device)   ((_pst_hal_device)->st_hal_scan_params.uc_start_chan_idx + (_pst_hal_device)->st_hal_scan_params.uc_scan_chan_idx)
#define INVALID_SAR_PWR_LIMIT                 (0XFF)   /* ��ǰSAR����ֵ */
#define HAL_SAR_PWR_LIMIT_THRESHOLD            (15)    /* SAR������ֵ��������ֵ��ʾ���ڽ�SAR��������ֵ��ʾ��SAR���� */
#define HAL_DEVICE_STATE_IS_BUSY(_pst_hal_device)   \
    (((HAL_DEVICE_WORK_STATE  ==  GET_HAL_DEVICE_STATE(_pst_hal_device))\
        || (HAL_DEVICE_SCAN_STATE  ==  GET_HAL_DEVICE_STATE(_pst_hal_device)))? OAL_TRUE : OAL_FALSE)
#define HAL_DEVICE_PREV_STATE_IS_BUSY(_pst_hal_device)   \
            (((HAL_DEVICE_WORK_STATE  ==  GET_HAL_DEVICE_PREV_STATE(_pst_hal_device))\
                || (HAL_DEVICE_SCAN_STATE  ==  GET_HAL_DEVICE_PREV_STATE(_pst_hal_device)))? OAL_TRUE : OAL_FALSE)

#define GET_HAL_DEVICE_ROM(_pst_handler)                  ((hal_to_dmac_device_rom_stru *)((_pst_handler)->_rom))
#define GET_HAL_DEVICE_M2S_MGR(_pst_handler)              (&(((hal_to_dmac_device_rom_stru *)((_pst_handler)->_rom))->st_device_m2s_mgr))
#define GET_HAL_DEVICE_M2S_SWITCH_PROT(_pst_handler)       (GET_HAL_DEVICE_M2S_MGR(_pst_handler)->en_m2s_switch_protect)
#define GET_HAL_DEVICE_M2S_DEL_SWI_MISO_HOLD(_pst_handler) (GET_HAL_DEVICE_M2S_MGR(_pst_handler)->en_delay_swi_miso_hold)
#define GET_HAL_DEVICE_M2S_MSS_ON(_pst_handler)            (GET_HAL_DEVICE_M2S_MGR(_pst_handler)->en_mss_on)
#define GET_HAL_DEVICE_M2S_RSSI_MGMT_SINGLE_TXCHAIN(_pst_handler)   (GET_HAL_DEVICE_M2S_MGR(_pst_handler)->uc_rssi_mgmt_single_txchain)
#define GET_HAL_DEVICE_M2S_BLACKLIST_ASSOC_AP_ON(_pst_handler)      (GET_HAL_DEVICE_M2S_MGR(_pst_handler)->en_blacklist_assoc_ap_on)
#define GET_HAL_DEVICE_M2S_MSS_SCAN_OPT_CHAIN(_pst_handler)   (GET_HAL_DEVICE_M2S_MGR(_pst_handler)->uc_mss_scan_opt_chain)

#define GET_HAL_M2S_CUR_STATE(_pst_handler)         ((_pst_handler)->st_hal_m2s_fsm.st_oal_fsm.uc_cur_state)/* ��ȡ��ǰ״̬����״̬ */
#define GET_HAL_M2S_PREV_STATE(_pst_handler)        ((_pst_handler)->st_hal_m2s_fsm.st_oal_fsm.uc_prev_state)/* ��ȡ��ǰ״̬������һ��״̬ */
#define GET_HAL_M2S_SWITCH_TPYE(_pst_handler)       ((_pst_handler)->st_hal_m2s_fsm.en_m2s_type)
#define GET_HAL_M2S_MODE_TPYE(_pst_handler)         (*(oal_uint8*)&((_pst_handler)->st_hal_m2s_fsm.st_m2s_mode))
#define GET_HAL_M2S_COMMAND_SCAN_ON(_pst_handler)   ((_pst_handler)->st_hal_m2s_fsm.en_command_scan_on)

#define HAL_M2S_CHECK_DBDC_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_hal_m2s_fsm.st_m2s_mode))& WLAN_M2S_TRIGGER_MODE_DBDC)) ? OAL_TRUE : OAL_FALSE)
#define HAL_M2S_CHECK_FAST_SCAN_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_hal_m2s_fsm.st_m2s_mode))& WLAN_M2S_TRIGGER_MODE_FAST_SCAN)) ? OAL_TRUE : OAL_FALSE)
#define HAL_M2S_CHECK_RSSI_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_hal_m2s_fsm.st_m2s_mode))& WLAN_M2S_TRIGGER_MODE_RSSI)) ? OAL_TRUE : OAL_FALSE)
#define HAL_M2S_CHECK_BTCOEX_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_hal_m2s_fsm.st_m2s_mode))& WLAN_M2S_TRIGGER_MODE_BTCOEX)) ? OAL_TRUE : OAL_FALSE)
#define HAL_M2S_CHECK_COMMAND_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_hal_m2s_fsm.st_m2s_mode))& WLAN_M2S_TRIGGER_MODE_COMMAND)) ? OAL_TRUE : OAL_FALSE)
#define HAL_M2S_CHECK_TEST_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_hal_m2s_fsm.st_m2s_mode))& WLAN_M2S_TRIGGER_MODE_TEST)) ? OAL_TRUE : OAL_FALSE)
#define HAL_M2S_CHECK_CUSTOM_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_hal_m2s_fsm.st_m2s_mode))& WLAN_M2S_TRIGGER_MODE_CUSTOM)) ? OAL_TRUE : OAL_FALSE)
#define HAL_M2S_CHECK_SPEC_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_hal_m2s_fsm.st_m2s_mode))& WLAN_M2S_TRIGGER_MODE_SPEC)) ? OAL_TRUE : OAL_FALSE)

#define GET_HAL_DEVICE_RX_ANT_RSSI_MGMT(_pst_handler)        (&(((hal_to_dmac_device_rom_stru *)((_pst_handler)->_rom))->st_hal_rx_ant_rssi_mgmt))


#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
#define GET_HAL_DYN_CALI_DISABLE(_pst_handler)  (0 == (_pst_handler)->st_dyn_cali_val.aus_cali_en_interval[(_pst_handler)->st_wifi_channel_status.en_band]) /* ��ȡ��̬У׼�Ƿ�δʹ�� */
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
#define GET_HAL_BTCOEX_SW_PREEMPT_MODE(_pst_handler)       (*(oal_uint8*)&((_pst_handler)->st_btcoex_sw_preempt.st_sw_preempt_mode)) /* ��ȡ��ǰhal devcie��btcoex sw preemptģʽ */
#define GET_HAL_BTCOEX_SW_PREEMPT_TYPE(_pst_handler)       ((_pst_handler)->st_btcoex_sw_preempt.en_sw_preempt_type)                  /* ��ȡ��ǰhal devcie��btcoex sw preempt���� */
#define GET_HAL_BTCOEX_SW_PREEMPT_SUBTYPE(_pst_handler)    ((_pst_handler)->st_btcoex_sw_preempt.en_sw_preempt_subtype)               /* ��ȡ��ǰhal devcie��btcoex sw preempt������ */
#define GET_HAL_BTCOEX_SW_PREEMPT_PS_STOP(_pst_handler)    ((_pst_handler)->st_btcoex_sw_preempt.en_ps_stop)                          /* ��ȡ��ǰhal devcie��psҵ���Ƿ�򿪱�� */
#define GET_HAL_BTCOEX_SW_PREEMPT_PS_PAUSE(_pst_handler)    ((_pst_handler)->st_btcoex_sw_preempt.en_ps_pause)                          /* ��ȡ��ǰhal devcie��psҵ���Ƿ�򿪱�� */

#define HAL_BTCOEX_CHECK_SW_PREEMPT_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_btcoex_sw_preempt.st_sw_preempt_mode))& BIT0)) ? OAL_TRUE : OAL_FALSE)
#define HAL_BTCOEX_CHECK_SW_PREEMPT_DELBA_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_btcoex_sw_preempt.st_sw_preempt_mode))& BIT1)) ? OAL_TRUE : OAL_FALSE)
#define HAL_BTCOEX_CHECK_SW_PREEMPT_REPLY_CTS_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_btcoex_sw_preempt.st_sw_preempt_mode))& BIT2)) ? OAL_TRUE : OAL_FALSE)
#define HAL_BTCOEX_CHECK_SW_PREEMPT_RSP_FRAME_PS_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_btcoex_sw_preempt.st_sw_preempt_mode))& BIT3)) ? OAL_TRUE : OAL_FALSE)
#define HAL_BTCOEX_CHECK_SW_PREEMPT_SLOT_DETECT_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_btcoex_sw_preempt.st_sw_preempt_mode))& BIT4)) ? OAL_TRUE : OAL_FALSE)
#define HAL_BTCOEX_CHECK_SW_PREEMPT_WL0_TX_SLV_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_btcoex_sw_preempt.st_sw_preempt_mode))& BIT5)) ? OAL_TRUE : OAL_FALSE)

#define GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)        (&(((hal_to_dmac_device_rom_stru *)((_pst_handler)->_rom))->st_device_btcoex_mgr))                   /* ��ȡ��ǰhal devcie��psҵ���Ƿ�򿪱�� */
#define GET_HAL_DEVICE_BTCOEX_TIMER_NEED_RESTART(_pst_handler)      (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->en_timer_need_restart)
#define GET_HAL_DEVICE_BTCOEX_WL0_TX_SLV_ALLOW(_pst_handler)        (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->en_wl0_tx_slv_allow)

#define GET_HAL_DEVICE_BTCOEX_M2S_MODE_BITMAP(_pst_handler)         (*(oal_uint8*)&(GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->st_m2s_mode_bitmap)) /* ��ȡ��ǰhal devcie��btcoex m2sҵ��bitmap */
#define GET_HAL_DEVICE_BTCOEX_S2M_MODE_BITMAP(_pst_handler)         (*(oal_uint8*)&(GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->st_s2m_mode_bitmap)) /* ��ȡ��ǰhal devcie��btcoex s2mҵ��bitmap */
#define GET_HAL_DEVICE_BTCOEX_S2M_WAIT_BITMAP(_pst_handler)         (*(oal_uint8*)&(GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->st_s2m_wait_bitmap)) /* ��ȡ��ǰhal devcie��btcoex s2m ��Ҫ��ʱ�����ҵ��bitmap */

#define GET_HAL_DEVICE_BTCOEX_M2S_6SLOT_ON(_pst_handler)            (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->bit_m2s_6slot)
#define GET_HAL_DEVICE_BTCOEX_M2S_LDAC_ON(_pst_handler)             (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->bit_m2s_ldac)
#define GET_HAL_DEVICE_BTCOEX_M2S_SISO_AP_ON(_pst_handler)          (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->bit_m2s_siso_ap)
#define GET_HAL_DEVICE_BTCOEX_M2S_RSSI_LIMIT_ON(_pst_handler)       (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->en_rssi_limit_on)
#define GET_HAL_DEVICE_BTCOEX_M2S_DETECT_CNT_TH(_pst_handler)       (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->uc_detect_cnt_threshold)
#define GET_HAL_DEVICE_BTCOEX_M2S_RSSI_TH(_pst_handler)             (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->c_m2s_threshold)
#define GET_HAL_DEVICE_BTCOEX_S2M_RSSI_TH(_pst_handler)             (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->c_s2m_threshold)
#define GET_HAL_DEVICE_BTCOEX_SCO_FIX_RX_BA_WIN(_pst_handler)       (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->uc_sco_fix_rx_ba_win)
#define GET_HAL_DEVICE_BTCOEX_BT20DBM_SET_RF_ON(_pst_handler)       (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->en_bt20dbm_set_rf_on)

#define GET_HAL_DEVICE_BTCOEX_SISO_AP_EXCUTE_ON(_pst_handler)       (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->en_siso_ap_excute_on)
#define GET_HAL_DEVICE_BTCOEX_SISO_AP_TIMER(_pst_handler)           (&(GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->bt_coex_s2m_siso_ap_timer))
#define GET_HAL_DEVICE_BTCOEX_PS_OCCU_DOWN_DELAY_ON(_pst_handler)   (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->en_ps_occu_down_delay)
#define GET_HAL_DEVICE_BTCOEX_PS_SLOT_LOG_PRINT_ON(_pst_handler)    (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->en_log_print_on)
#define GET_HAL_DEVICE_BTCOEX_PS_SLOT_PAUSE_ON(_pst_handler)        (GET_HAL_DEVICE_BTCOEX_MGR(_pst_handler)->en_dynamic_slot_pause)

#define GET_HAL_CHIP_BTCOEX_BT_STATUS(_pst_handler)     (&((_pst_handler)->st_btcoex_btble_status.un_bt_status.st_bt_status))  /* ��ȡ��ǰbt status */
#define GET_HAL_CHIP_BTCOEX_BLE_STATUS(_pst_handler)    (&((_pst_handler)->st_btcoex_btble_status.un_ble_status.st_ble_status))  /* ��ȡ��ǰble status */
#define GET_HAL_CHIP_BTCOEX_BT_LDAC_STATUS(_pst_handler)  ((&((_pst_handler)->st_btcoex_btble_status.un_ble_status.st_ble_status))->bit_bt_ldac) /* ��ȡ��ǰldac status */
#endif
#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS
#define PSD_DATA_SIZE_20M 128
#define PSD_DATA_SIZE_40M 256
#define PSD_DATA_SIZE_80M 512

#define PSD_DATA_SIZE     1024  /* 160M����ҵ��ģʽΪ1024 */

#endif

#define GET_HAL_CHIP_RF_2G5G_CUSTOM_CAP_MGR(_pst_handler)          (&(((hal_chip_rom_stru *)((_pst_handler)->_rom))->st_2g5g_rf_custom_mgr))
#define GET_HAL_CHIP_RF_CUSTOM_CAP_2G_NSS(_pst_handler)            (GET_HAL_CHIP_RF_2G5G_CUSTOM_CAP_MGR(_pst_handler)->en_2g_nss_num)
#define GET_HAL_CHIP_RF_CUSTOM_CAP_2G_RFCHAIN(_pst_handler)        (GET_HAL_CHIP_RF_2G5G_CUSTOM_CAP_MGR(_pst_handler)->uc_2g_support_rf_chain)
#define GET_HAL_CHIP_RF_CUSTOM_CAP_5G_NSS(_pst_handler)            (GET_HAL_CHIP_RF_2G5G_CUSTOM_CAP_MGR(_pst_handler)->en_5g_nss_num)
#define GET_HAL_CHIP_RF_CUSTOM_CAP_5G_RFCHAIN(_pst_handler)        (GET_HAL_CHIP_RF_2G5G_CUSTOM_CAP_MGR(_pst_handler)->uc_5g_support_rf_chain)

#ifdef _PRE_WLAN_FEATURE_BTCOEX
#define GET_HAL_CHIP_BTCOEX_MGR(_pst_handler)          (&(((hal_chip_rom_stru *)((_pst_handler)->_rom))->st_btcoex_mgr))
#endif

/*****************************************************************************
  8 UNION����
*****************************************************************************/
typedef enum {
    HAL_DEVICE_EVENT_JOIN_COMP                = 0,          /* staut join����¼� */
    HAL_DEVICE_EVENT_VAP_UP                   = 1,          /* vap up�¼� */
    HAL_DEVICE_EVENT_VAP_DOWN                 = 2,          /* vap down�¼� */
    HAL_DEVICE_EVENT_SCAN_BEGIN               = 3,          /* ɨ�迪ʼ�¼� */
    HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_OFF  = 4,          /* ɨ�������ŵ��¼� */
    HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_BACK = 5,          /* ɨ���л��ŵ��¼� */
    HAL_DEVICE_EVENT_SCAN_END                 = 6,          /* ɨ������¼� */
    HAL_DEVICE_EVENT_SCAN_ABORT               = 7,          /* ɨ��abort�¼� */
    HAL_DEVICE_EVENT_SCAN_PAUSE               = 8,          /* PAUSE SCAN */
    HAL_DEVICE_EVENT_SCAN_RESUME              = 9,          /* RESUME SCAN */

#ifdef _PRE_WLAN_FEATURE_STA_PM
    HAL_DEVICE_EVENT_VAP_CHANGE_TO_ACTIVE,          /* vap ����active״̬�¼� */
    HAL_DEVICE_EVENT_VAP_CHANGE_TO_AWAKE,           /* vap ����awake״̬�¼� */
    HAL_DEVICE_EVENT_VAP_CHANGE_TO_DOZE,            /* vap ����doze״̬�¼� */
    HAL_DEVICE_EVENT_HW_QUEUE_EMPTY,                /* Ӳ������Ϊ���¼� */
    HAL_DEVICE_EVENT_INIT_RX_DSCR,                  /* add rx dscr */
    HAL_DEVICE_EVENT_DESTROY_RX_DSCR,               /* destroy rx dscr */
    HAL_DEVICE_EVENT_TBTT_WAKE_UP,                  /* tbtt�����¼������������������¼� */
    HAL_DEVICE_EVENT_TRANS_TO_WORK_STATE,
#endif

    HAL_DEVICE_EVENT_SCAN_PAUSE_FROM_CHAN_CONFLICT,      /* PAUSE SCAN */
    HAL_DEVICE_EVENT_SCAN_RESUME_FROM_CHAN_CONFLICT,     /* RESUME SCAN */

    HAL_DEVICE_EVENT_DBDC_STOP,
    HAL_DEVICE_EVENT_DETATCH,                       /* ״̬��detatch */
    HAL_DEVICE_EVENT_SYSTEM_INIT,                   /* ϵͳ��ʼ���� */

    HAL_DEVICE_EVENT_BUTT
}hal_device_event;


typedef enum {
    HAL_DEVICE_INIT_STATE = 0,              /* init״̬ */
    HAL_DEVICE_IDLE_STATE = 1,              /* idle״̬ */
    HAL_DEVICE_WORK_STATE = 2,              /* work״̬vap up */
    HAL_DEVICE_SCAN_STATE = 3,              /* ɨ��״̬ */
    HAL_DEVICE_BUTT_STATE                   /* ���״̬ */
} hal_device_state_info;

/* MIMO/SISO״̬��״̬ö�� */
typedef enum {
    HAL_M2S_STATE_IDLE = 0,         /* ��ʼ����״̬ */
    HAL_M2S_STATE_SISO = 1,         /* SISO״̬,�����Ӳ�����ǵ�ͨ�� */
    HAL_M2S_STATE_MIMO = 2,         /* MIMO״̬,�����Ӳ������˫ͨ�� */
    HAL_M2S_STATE_MISO = 3,         /* MISO̽��̬,�����ͨ����Ӳ��˫ͨ�� */
    HAL_M2S_STATE_SIMO = 4,         /* SIMO״̬,���˫ͨ����Ӳ����ͨ�� */

    HAL_M2S_STATE_BUTT              /* ���״̬ */
}hal_m2s_state;
typedef oal_uint8 hal_m2s_state_uint8;

/* MIMO/SISO�л�ҵ��bitmap */
typedef enum {
    HAL_M2S_MODE_BITMAP_6SLOT = BIT0,         /* 6slot */
    HAL_M2S_MODE_BITMAP_LDAC  = BIT1,         /* ldac */
    HAL_M2S_MODE_BITMAP_SISO_AP = BIT2,       /* siso ap */
    HAL_M2S_MODE_BITMAP_MSS   = BIT3,         /* mss */
    HAL_M2S_MODE_BITMAP_2GAP  = BIT4,         /* 2g ap��siso */
    HAL_M2S_MODE_BITMAP_5GAP  = BIT5,         /* 5g ap��siso */
    HAL_M2S_MODE_BITMAP_LDAC_RSSI = BIT6,     /* ldac rssi��siso */

    HAL_M2S_MODE_BITMAP_BUTT                 /* ���״̬ */
}hal_m2s_mode_bitmap;
typedef oal_uint8 hal_m2s_mode_bitmap_uint8;

/* MIMO/SISO״̬���¼�ö�� */
typedef enum {
    /* 1.RSSIģ�� */
    HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C0,
    HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C1,
    HAL_M2S_EVENT_ANT_RSSI_MISO_C1_TO_MISO_C0,
    HAL_M2S_EVENT_ANT_RSSI_MISO_C0_TO_MISO_C1,
    HAL_M2S_EVENT_ANT_RSSI_MISO_TO_MIMO,

    /* 2.testģ�� */
    HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0 = 10,
    HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C1,
    HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C0,    /* MISOӲ����mimo�ģ�misoҪ�з�c0����c1��������sisoʱ���ֳ�c0����c1 */
    HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C1,
    HAL_M2S_EVENT_TEST_MISO_TO_MIMO,       /* miso��ʱ������c0����c1 */
    HAL_M2S_EVENT_TEST_SISO_TO_MIMO,
    HAL_M2S_EVENT_TEST_SISO_TO_MISO_C0,
    HAL_M2S_EVENT_TEST_SISO_TO_MISO_C1,
    HAL_M2S_EVENT_TEST_SISO_C0_TO_SISO_C1,
    HAL_M2S_EVENT_TEST_SISO_C1_TO_SISO_C0,
    HAL_M2S_EVENT_TEST_MISO_TO_SISO_C0 = 20,
    HAL_M2S_EVENT_TEST_MISO_TO_SISO_C1,
    HAL_M2S_EVENT_TEST_MISO_C0_TO_MISO_C1,
    HAL_M2S_EVENT_TEST_MISO_C1_TO_MISO_C0,
    HAL_M2S_EVENT_TEST_IDLE_TO_SISO_C0,
    HAL_M2S_EVENT_TEST_IDLE_TO_SISO_C1,
    HAL_M2S_EVENT_TEST_IDLE_TO_MIMO,
    HAL_M2S_EVENT_TEST_IDLE_TO_MISO_C0,
    HAL_M2S_EVENT_TEST_IDLE_TO_MISO_C1,

    /* 3.BTģ�� */
    HAL_M2S_EVENT_BT_MIMO_TO_SISO_C1 = 40,
    HAL_M2S_EVENT_BT_SISO_TO_MIMO,
    HAL_M2S_EVENT_BT_SISO_C0_TO_SISO_C1,
    HAL_M2S_EVENT_BT_MISO_TO_SISO_C1,

    /* 4.hal device������ͬ���¼� */
    HAL_M2S_EVENT_IDLE_BEGIN = 50,
    HAL_M2S_EVENT_WORK_BEGIN,
    HAL_M2S_EVENT_SCAN_BEGIN,
    HAL_M2S_EVENT_SCAN_PREPARE,
    HAL_M2S_EVENT_SCAN_CHANNEL_BACK, //scan�л�home channel�¼�
    HAL_M2S_EVENT_SCAN_END,

    /* 5.DBDCģ�� */
    HAL_M2S_EVENT_DBDC_START = 60,   //��·�и�·,siso
    HAL_M2S_EVENT_DBDC_MIMO_TO_SISO = HAL_M2S_EVENT_DBDC_START,
    HAL_M2S_EVENT_DBDC_STOP,         //DBDC stop�¼�
    HAL_M2S_EVENT_DBDC_SISO_TO_MIMO = HAL_M2S_EVENT_DBDC_STOP,
    HAL_M2S_EVENT_DBDC_MISO_TO_SISO_C0,
    HAL_M2S_EVENT_DBDC_SISO_C1_TO_SISO_C0,

    /* 6.�ϲ�ģ�� */
    HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C0 = 70,
    HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C1,
    HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0,    /* MISOӲ����mimo�ģ�misoҪ�з�c0����c1��������sisoʱ���ֳ�c0����c1 */
    HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1,
    HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO,       /* miso��ʱ������c0����c1 */
    HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO,       /* siso��ʱ������c0����c1 */
    HAL_M2S_EVENT_COMMAND_SISO_TO_MISO_SCAN_BEGIN,  /* ר�����ڲ���ɨ��δ����mss�µ�����ɨ��,��ǰ��c0 siso mss */
    HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0_SCAN_END,  /* ר�����ڲ���ɨ��δ����mss�µ�����ɨ��,��ǰ��c0 siso mss */
    HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1_SCAN_END,  /* ר�����ڲ���ɨ��δ����mss�µ�����ɨ��,��ǰ��c1 siso mss */
    HAL_M2S_EVENT_COMMAND_SISO_C0_TO_SISO_C1,
    HAL_M2S_EVENT_COMMAND_SISO_C1_TO_SISO_C0,
    HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0,
    HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1,
    HAL_M2S_EVENT_COMMAND_MISO_C0_TO_MISO_C1,
    HAL_M2S_EVENT_COMMAND_MISO_C1_TO_MISO_C0,
    HAL_M2S_EVENT_COMMAND_IDLE_TO_SISO_C0,
    HAL_M2S_EVENT_COMMAND_IDLE_TO_SISO_C1,
    HAL_M2S_EVENT_COMMAND_IDLE_TO_MIMO,
    HAL_M2S_EVENT_COMMAND_IDLE_TO_MISO_C0,
    HAL_M2S_EVENT_COMMAND_IDLE_TO_MISO_C1,

    /* 7.SISO��̬����ģ�� */
    HAL_M2S_EVENT_CUSTOM_MIMO_TO_SISO_C0 = 100,   /* ��ǰ����ʹ�������ߣ���̬����Ĭ��������C0��;����wifi���ָܻ� */
    HAL_M2S_EVENT_CUSTOM_SISO_C0_TO_SISO_C1,      /* ���ȼ���ߣ�APģʽ��ʹ�ã����漰dbdc��mss��test���أ�rssi���漰������ɨ�践�أ��������Ļ�����C1siso */
    HAL_M2S_EVENT_CUSTOM_SISO_C1_TO_SISO_C0,      /* ����custom���ȼ���ߣ���������ҵ�񴥷���c0����c1 siso */

    /* 8.SISOӲ�����̬����ģ��, ������������ҵ���л� */
    HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C0 = 110,     /* Ӳ�����,��˫���߹����������߹��� */
    HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C1,           /* Ӳ�����,��˫���߹����������߹��� */
    HAL_M2S_EVENT_SPEC_SISO_TO_MIMO,              /* Ӳ�����,�ӵ����߹�����˫���߹��� */
    HAL_M2S_EVENT_SPEC_MISO_TO_SISO_C0,           /* Ӳ�����,��˫���߹����������߹��� */
    HAL_M2S_EVENT_SPEC_SISO_C1_TO_SISO_C0,        /* 2g c1 siso�л���5g c0 siso����Ƶdbac�Ĵ���ʽ����������ȥ����Ҫ�Ȼ�mimo */

    /* ��ʼ�� */
    HAL_M2S_EVENT_FSM_INIT = 255,

    HAL_M2S_EVENT_BUTT
}hal_m2s_event_tpye;
typedef oal_uint16 hal_m2s_event_tpye_uint16;

/* CBBƵ�� */
typedef enum
{
    HAL_CLK_0M_FREQ                     = 0,
    HAL_CLK_40M_FREQ                    = 1,
    HAL_CLK_80M_FREQ                    = 2,
    HAL_CLK_160M_FREQ                   = 3,
    HAL_CLK_320M_FREQ                   = 4,
    HAL_CLK_640M_FREQ                   = 5,
    HAL_CLK_960M_FREQ                   = 6,

    HAL_CLK_BUTT_FREQ
}hal_clk_freq_enum;
typedef oal_uint8 hal_clk_freq_enum_uint8;

/* ADC/DAC���Ƶ�� */
typedef enum
{
    HAL_CLK_ADC80M_DAC160M                      = 0,
    HAL_CLK_ADC80M_DAC320M                      = 1,
    HAL_CLK_ADC160M_DAC160M                     = 2,
    HAL_CLK_ADC160M_DAC320M                     = 3,
    HAL_CLK_ADC320M_DAC320M                     = 4,
    HAL_CLK_ADC320M_DAC640M                     = 5,

    HAL_CLK_ADC_DAC_BUTT
}hal_clk_adc_dac_enum;
typedef oal_uint8 hal_clk_adc_dac_enum_uint8;

typedef enum
{
    HAL_DPD_CFR_SET_11B  = 0,
    HAL_DPD_CFR_SET_BW   = 1,
    HAL_DPD_CFR_SET_FREQ = 2,
    HAL_DPD_CFR_SET_MCS  = 3,
    HAL_DPD_CFR_SET_TPC  = 4,

    HAL_DPD_CFR_SET_BUTT,
}hal_dpd_cfr_set_type;
typedef oal_uint8 hal_dpd_cfr_set_type_uint8;


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/
/* HALģ���DMACģ�鹲�õ�WLAN RX�ṹ�� */
typedef struct
{
    oal_netbuf_stru        *pst_netbuf;         /* netbuf����һ��Ԫ�� */
    oal_uint16              us_netbuf_num;      /* netbuf����ĸ��� */
    oal_uint8               auc_resv[2];        /* �ֽڶ��� */
}hal_cali_hal2hmac_event_stru;

typedef struct
{
    oal_uint32 ul_packet_idx;

    oal_uint8 auc_payoald[4];
}hal_cali_hal2hmac_payload_stru;

typedef struct
{
    oal_void      (* p_func)(hal_to_dmac_device_stru  *pst_hal_device, oal_uint32 ul_para);
    oal_uint32    ul_param;
}hal_error_proc_stru;

extern hal_error_proc_stru  g_st_err_proc_func[HAL_MAC_ERROR_TYPE_BUTT];

/*****************************************************************************
  10 ��������
*****************************************************************************/
extern oal_int32 hal_main_init(oal_void);
extern oal_void  hal_main_exit(oal_void);
#ifdef _PRE_WLAN_FEATURE_HUT
extern oal_void  hal_to_hut_irq_isr_register(hal_oper_mode_enum_uint8 en_oper_mode, oal_void (*p_func)(oal_void));
extern oal_void  hal_to_hut_irq_isr_unregister(oal_void);
#endif
extern oal_void  hal_get_hal_to_dmac_device(oal_uint8 uc_chip_id, oal_uint8 uc_device_id, hal_to_dmac_device_stru **ppst_hal_device);
extern oal_void  hal_clear_tx_hw_queue(hal_to_dmac_device_stru *pst_hal_device);
extern oal_bool_enum hal_device_check_all_sleep_time(hal_to_dmac_device_stru *pst_hal_device);
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
extern oal_uint8 hal_get_5g_enable(oal_void);
extern oal_uint8  hal_get_5g_cur_rf_chn_enable(oal_uint8 uc_cur_cali_chn);
#endif //#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
extern hal_cipher_protocol_type_enum_uint8 hal_cipher_suite_to_ctype(wlan_ciper_protocol_type_enum_uint8 en_cipher_suite);
extern wlan_ciper_protocol_type_enum_uint8 hal_ctype_to_cipher_suite(hal_cipher_protocol_type_enum_uint8 en_cipher_type);


/*****************************************************************************
  10.1 оƬ������
*****************************************************************************/
extern oal_uint32 hal_chip_get_chip(oal_uint8 uc_chip_id,  hal_to_dmac_chip_stru **ppst_hal_chip);
extern oal_uint32 hal_chip_get_device_num(oal_uint8 uc_chip_id, oal_uint8 * puc_device_nums);
extern oal_uint32 hal_chip_get_hal_device(oal_uint8 uc_chip_id, oal_uint8 uc_device_id, hal_to_dmac_device_stru **ppst_device_stru);

#ifdef _PRE_WLAN_CHIP_TEST
extern  oal_void hal_test_lpm_set_psm_en(oal_uint8 uc_en);
extern  oal_void hal_test_lpm_psm_tbtt_record(oal_void);
extern  oal_void hal_test_lpm_psm_dump_record(oal_void);
#endif

extern oal_void  hal_device_process_mac_error_status(hal_to_dmac_device_stru *pst_hal_device,
                oal_uint32 ul_error1_irq_state, oal_uint32 ul_error2_irq_state);
extern oal_void hal_device_inc_assoc_user_nums(hal_to_dmac_device_stru  *pst_hal_device);

extern oal_void hal_device_dec_assoc_user_nums(hal_to_dmac_device_stru  *pst_hal_device);
extern oal_uint8  hal_device_find_all_up_vap(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_vap_id);
extern oal_uint32 hal_device_handle_event(hal_to_dmac_device_stru* pst_hal_device, oal_uint16 us_type, oal_uint16 us_datalen, oal_uint8* pst_data);
extern oal_uint32  hal_device_find_one_up_vap(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_mac_vap_id);
extern oal_uint32  hal_device_find_another_up_vap(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_vap_id_self, oal_uint8 *puc_mac_vap_id);
extern oal_uint32 hal_device_find_one_up_hal_vap(hal_to_dmac_device_stru *pst_hal_device,  oal_uint8 *puc_vap_id);
extern oal_void  hal_device_get_fix_rate_pow_code_idx(oal_uint8 uc_protocol, wlan_bw_cap_enum_uint8 en_bw,
                                                hal_tx_txop_per_rate_params_union *pst_rate_param, oal_uint8 *puc_rate_pow_idx, oal_uint8 uc_he_mcs);
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
extern oal_void hal_device_set_pow_al_tx(hal_to_dmac_device_stru *pst_hal_device, hal_vap_pow_info_stru *pst_vap_pow_info,
                                        oal_uint8 uc_protocol, hal_tx_txop_alg_stru *pst_txop_alg);
#endif
extern oal_void hal_device_p2p_adjust_upc(hal_to_dmac_device_stru * pst_hal_device,
                                              oal_uint8                         uc_cur_ch_num,
                                              wlan_channel_band_enum_uint8      en_freq_band,
                                              wlan_channel_bandwidth_enum_uint8 en_bandwidth);

extern oal_bool_enum_uint8 hal_get_hal_device_is_work(hal_to_dmac_device_stru *pst_hal_device);
#ifdef _PRE_WLAN_FEATURE_TPC_OPT
extern oal_void hal_device_update_upc_amend(hal_to_dmac_device_stru *pst_hal_device, oal_int16 s_upc_amend);
extern hal_rate_pow_code_gain_table_stru g_ast_rate_pow_table_2g[WLAN_DEVICE_MAX_NUM_PER_CHIP][HAL_POW_RATE_POW_CODE_TABLE_LEN];
extern hal_rate_pow_code_gain_table_stru g_ast_rate_pow_table_5g[WLAN_DEVICE_MAX_NUM_PER_CHIP][HAL_POW_RATE_POW_CODE_TABLE_LEN];
#else
extern oal_void hal_device_amend_upc_code(hal_to_dmac_device_stru * pst_hal_device,
                    wlan_channel_band_enum_uint8        en_freq_band,
                    oal_uint8                           uc_cur_ch_num,
                    wlan_channel_bandwidth_enum_uint8   en_bandwidth);
extern hal_rate_pow_code_gain_table_stru g_ast_rate_pow_table_2g[];
extern hal_rate_pow_code_gain_table_stru g_ast_rate_pow_table_5g[];
#endif

extern oal_uint8 g_uc_sar_pwr_limit;
extern oal_uint8 g_auc_bw_idx[WLAN_BAND_ASSEMBLE_BUTT];

#define GET_HAL_DEV_CURRENT_SCAN_IDX(_pst_hal_device)   ((_pst_hal_device)->st_hal_scan_params.uc_start_chan_idx + (_pst_hal_device)->st_hal_scan_params.uc_scan_chan_idx)

/*****************************************************************************
  10.2 ���Ⱪ¶�����ýӿ�
*****************************************************************************/


/*****************************************************************************
  10.3 ��Ӧһ��Ӳ��MAC��PHY�ľ�̬��������
*****************************************************************************/
#define HAL_CHIP_LEVEL_FUNC
/*****************************************************************************
  10.3.1 CHIP����   ��һ���������Ϊ hal_to_dmac_chip_stru
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_void hal_get_chip_version(hal_to_dmac_chip_stru * pst_hal_chip, oal_uint32 *pul_chip_ver)
{
    HAL_PUBLIC_HOOK_FUNC(_get_chip_version)( pst_hal_chip, pul_chip_ver);
}

#define HAL_DEVICE_LEVEL_FUNC
/*****************************************************************************
  10.3.2 DEVICE����   ��һ���������Ϊ hal_to_dmac_device_stru
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_void hal_rx_init_dscr_queue(hal_to_dmac_device_stru *pst_device,oal_uint8 uc_set_hw)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_init_dscr_queue)( pst_device,uc_set_hw);
}

OAL_STATIC OAL_INLINE oal_void hal_rx_destroy_dscr_queue(hal_to_dmac_device_stru *pst_device)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_destroy_dscr_queue)( pst_device);
}

OAL_STATIC OAL_INLINE oal_void hal_al_rx_destroy_dscr_queue(hal_to_dmac_device_stru *pst_device)
{
    HAL_PUBLIC_HOOK_FUNC(_al_rx_destroy_dscr_queue)( pst_device);
}

OAL_STATIC OAL_INLINE oal_void hal_al_rx_init_dscr_queue(hal_to_dmac_device_stru *pst_device)
{
    HAL_PUBLIC_HOOK_FUNC(_al_rx_init_dscr_queue)( pst_device);
}

OAL_STATIC OAL_INLINE oal_void hal_tx_init_dscr_queue(hal_to_dmac_device_stru *pst_device)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_init_dscr_queue)( pst_device);
}

OAL_STATIC OAL_INLINE oal_void hal_tx_destroy_dscr_queue(hal_to_dmac_device_stru *pst_device)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_destroy_dscr_queue)( pst_device);
}

OAL_STATIC OAL_INLINE oal_void hal_init_hw_rx_isr_list(hal_to_dmac_device_stru *pst_device)
{
    HAL_PUBLIC_HOOK_FUNC(_init_hw_rx_isr_list)( pst_device);
}
OAL_STATIC OAL_INLINE oal_void hal_free_rx_isr_list(hal_to_dmac_device_stru *pst_device, oal_dlist_head_stru  *pst_rx_isr_list)
{
    HAL_PUBLIC_HOOK_FUNC(_free_rx_isr_list)( pst_rx_isr_list);
}

OAL_STATIC OAL_INLINE oal_void hal_destroy_hw_rx_isr_list(hal_to_dmac_device_stru *pst_device)
{
    HAL_PUBLIC_HOOK_FUNC(_destroy_hw_rx_isr_list)( pst_device);
}

/*����������Ļ�����Ϣ������֡���ȡ�mac���ȡ���ȫ��Ϣ*/
OAL_STATIC OAL_INLINE oal_void hal_tx_fill_basic_ctrl_dscr(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru * p_tx_dscr, hal_tx_mpdu_stru *pst_mpdu)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_fill_basic_ctrl_dscr)(pst_hal_device, p_tx_dscr, pst_mpdu);
}

/* ������������������ */
OAL_STATIC OAL_INLINE oal_void  hal_tx_ctrl_dscr_link(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr_prev, hal_tx_dscr_stru *pst_tx_dscr)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_ctrl_dscr_link)( pst_tx_dscr_prev, pst_tx_dscr);
}

/* ����������next��ȡ��һ����������ַ */
OAL_STATIC OAL_INLINE oal_void  hal_get_tx_dscr_next(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, hal_tx_dscr_stru **ppst_tx_dscr_next)
{
    HAL_PUBLIC_HOOK_FUNC(_get_tx_dscr_next)( pst_tx_dscr, ppst_tx_dscr_next);
}

OAL_STATIC OAL_INLINE oal_void  hal_tx_ctrl_dscr_unlink(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_ctrl_dscr_unlink)( pst_tx_dscr);
}

OAL_STATIC OAL_INLINE oal_void hal_tx_seqnum_set_dscr(hal_tx_dscr_stru *pst_tx_dscr, oal_uint16 us_seqnum)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_seqnum_set_dscr)( pst_tx_dscr, us_seqnum);
}

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
OAL_STATIC OAL_INLINE oal_void hal_tx_get_dscr_seqnum(hal_tx_dscr_stru *pst_tx_dscr, oal_uint16 *pus_seqnum, oal_uint8 *puc_vld)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_seqnum)( pst_tx_dscr, pus_seqnum, puc_vld);
}
#endif

OAL_STATIC OAL_INLINE oal_void hal_soc_set_pcie_l1s(hal_to_dmac_device_stru * pst_device, oal_uint8 uc_on_off, oal_uint8 uc_pcie_idle)
{
    HAL_PUBLIC_HOOK_FUNC(_soc_set_pcie_l1s)(pst_device, uc_on_off, uc_pcie_idle);
}

/* ���ó�������Ϣ������������������ֶ� */
OAL_STATIC OAL_INLINE oal_void hal_tx_ucast_data_set_dscr(hal_to_dmac_device_stru     *pst_hal_device,
                                                   hal_tx_dscr_stru            *pst_tx_dscr,
                                                   hal_tx_txop_feature_stru   *pst_txop_feature,
                                                   hal_tx_txop_alg_stru       *pst_txop_alg,
                                                   hal_tx_ppdu_feature_stru   *pst_ppdu_feature)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_ucast_data_set_dscr)( pst_hal_device, pst_tx_dscr, pst_txop_feature, pst_txop_alg, pst_ppdu_feature);
}

/* ���ù���֡���鲥 �㲥����֡��������Ϣ������������������ֶ� */
OAL_STATIC OAL_INLINE oal_void hal_tx_non_ucast_data_set_dscr(hal_to_dmac_device_stru     *pst_hal_device,
                                                   hal_tx_dscr_stru            *pst_tx_dscr,
                                                   hal_tx_txop_feature_stru   *pst_txop_feature,
                                                   hal_tx_txop_alg_stru       *pst_txop_alg,
                                                   hal_tx_ppdu_feature_stru   *pst_ppdu_feature)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_non_ucast_data_set_dscr)( pst_hal_device, pst_tx_dscr, pst_txop_feature, pst_txop_alg, pst_ppdu_feature);
}


OAL_STATIC OAL_INLINE oal_void  hal_tx_set_dscr_modify_mac_header_length(hal_to_dmac_device_stru *pst_hal_device,hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 uc_mac_header_length)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_set_dscr_modify_mac_header_length)( pst_tx_dscr, uc_mac_header_length);
}


OAL_STATIC OAL_INLINE oal_void  hal_tx_set_dscr_seqno_sw_generate(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 uc_sw_seqno_generate)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_set_dscr_seqno_sw_generate)( pst_tx_dscr, uc_sw_seqno_generate);
}

/*���amsdu�����������msdu���������ط������������׶γ��ȣ��Լ��ڶ��γ��� */
OAL_STATIC OAL_INLINE oal_void hal_tx_get_size_dscr(hal_to_dmac_device_stru * pst_hal_device, oal_uint8 us_msdu_num, oal_uint32 * pul_dscr_one_size, oal_uint32 * pul_dscr_two_size)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_size_dscr)( us_msdu_num, pul_dscr_one_size, pul_dscr_two_size);
}

OAL_STATIC OAL_INLINE oal_void hal_tx_get_vap_id(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru * pst_tx_dscr, oal_uint8 *puc_vap_id)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_vap_id)( pst_tx_dscr, puc_vap_id);
}

OAL_STATIC OAL_INLINE oal_void hal_tx_get_dscr_ctrl_one_param(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru * pst_tx_dscr, hal_tx_dscr_ctrl_one_param *pst_tx_dscr_one_param)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_ctrl_one_param)(pst_tx_dscr, pst_tx_dscr_one_param);
}
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
OAL_STATIC OAL_INLINE oal_void hal_tx_update_dscr_para_hw(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru * pst_tx_dscr, hal_tx_dscr_ctrl_one_param *pst_tx_dscr_one_param)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_update_dscr_para_hw)(pst_tx_dscr, pst_tx_dscr_one_param);
}
#endif



OAL_STATIC OAL_INLINE oal_void  hal_tx_get_dscr_tx_cnt(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_tx_count)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_tx_cnt)( pst_tx_dscr, puc_tx_count);
}

OAL_STATIC OAL_INLINE oal_void  hal_tx_dscr_get_rate3(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_rate)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_tx_dscr_get_rate3)( pst_tx_dscr, puc_rate);
#endif
}


OAL_STATIC OAL_INLINE oal_void  hal_tx_get_dscr_seq_num(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_uint16 *pus_seq_num)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_seq_num)( pst_tx_dscr, pus_seq_num);
}


OAL_STATIC OAL_INLINE oal_void  hal_tx_set_dscr_status(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 uc_status)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_set_dscr_status)( pst_tx_dscr, uc_status);
}

OAL_STATIC OAL_INLINE oal_void  hal_tx_get_dscr_chiper_type(hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_chiper_type, oal_uint8 *puc_chiper_key_id)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_chiper_type)( pst_tx_dscr, puc_chiper_type, puc_chiper_key_id);
}

#ifdef _PRE_WLAN_1103_PILOT

OAL_STATIC OAL_INLINE oal_void  hal_tx_enable_resp_ps_bit_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_enable_resp_ps_bit_ctrl)( pst_hal_device, uc_lut_index);
}

OAL_STATIC OAL_INLINE oal_void  hal_tx_enable_resp_ps_bit_ctrl_all(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_enable_resp_ps_bit_ctrl_all)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_tx_disable_resp_ps_bit_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_disable_resp_ps_bit_ctrl)( pst_hal_device, uc_lut_index);
}

OAL_STATIC OAL_INLINE oal_void  hal_tx_disable_resp_ps_bit_ctrl_all(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_disable_resp_ps_bit_ctrl_all)( pst_hal_device);
}

#endif


OAL_STATIC OAL_INLINE oal_void  hal_tx_enable_peer_sta_ps_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_enable_peer_sta_ps_ctrl)( pst_hal_device, uc_lut_index);
}

OAL_STATIC OAL_INLINE oal_void  hal_tx_disable_peer_sta_ps_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_disable_peer_sta_ps_ctrl)( pst_hal_device, uc_lut_index);
}



OAL_STATIC OAL_INLINE oal_void  hal_tx_get_dscr_status(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_status)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_status)( pst_tx_dscr, puc_status);
}


OAL_STATIC OAL_INLINE oal_void  hal_tx_get_dscr_send_rate_rank(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_send_rate_rank)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_send_rate_rank)( pst_tx_dscr, puc_send_rate_rank);
}



OAL_STATIC OAL_INLINE oal_void  hal_tx_get_dscr_ba_ssn(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_uint16 *pus_ba_ssn)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_ba_ssn)( pst_tx_dscr, pus_ba_ssn);
}


OAL_STATIC OAL_INLINE oal_void  hal_tx_get_dscr_ba_bitmap(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_uint32 *pul_ba_bitmap)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_ba_bitmap)( pst_tx_dscr, pul_ba_bitmap);
}


OAL_STATIC OAL_INLINE oal_void  hal_tx_put_dscr(hal_to_dmac_device_stru * pst_hal_device, hal_tx_queue_type_enum_uint8 en_tx_queue_type, hal_tx_dscr_stru *past_tx_dscr)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_put_dscr)( pst_hal_device, en_tx_queue_type, past_tx_dscr);
}

#if defined (_PRE_WLAN_FEATURE_RX_AGGR_EXTEND) || defined (_PRE_FEATURE_WAVEAPP_CLASSIFY)

OAL_STATIC OAL_INLINE oal_void  hal_set_is_waveapp_test(hal_to_dmac_device_stru * pst_hal_device, oal_bool_enum_uint8 en_is_waveapp_test)
{
    HAL_PUBLIC_HOOK_FUNC(_set_is_waveapp_test)(pst_hal_device, en_is_waveapp_test);
}
#endif


OAL_STATIC OAL_INLINE oal_void hal_get_tx_q_status(hal_to_dmac_device_stru * pst_hal_device, oal_uint32 * pul_status, oal_uint8 uc_qnum)
{
    HAL_PUBLIC_HOOK_FUNC(_get_tx_q_status)( pst_hal_device, pul_status, uc_qnum);
}

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
#ifdef _PRE_WLAN_1103_PILOT

OAL_STATIC OAL_INLINE oal_void hal_rf_init_dyn_cali_reg_conf(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_rf_init_dyn_cali_reg_conf)(pst_hal_device);
}
#endif

OAL_STATIC OAL_INLINE oal_void  hal_tx_set_pdet_en(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_bool_enum_uint8 en_pdet_en_flag)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_tx_set_pdet_en)(pst_hal_device, pst_tx_dscr, en_pdet_en_flag);
#endif
}

OAL_STATIC OAL_INLINE oal_void  hal_dyn_cali_tx_pow_ch_set(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_dyn_cali_tx_pow_ch_set)(pst_hal_device, pst_tx_dscr);
#endif
}

OAL_STATIC OAL_INLINE oal_void  hal_dyn_cali_tx_pa_ppa_swtich(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_ppa_working)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_dyn_cali_tx_pa_ppa_swtich)(pst_hal_device, en_ppa_working);
#endif
}

OAL_STATIC OAL_INLINE oal_void  hal_dyn_cali_al_tx_config_amend(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_dyn_cali_al_tx_config_amend)(pst_hal_device, pst_tx_dscr);
#endif
}

OAL_STATIC OAL_INLINE oal_void  hal_dyn_cali_vdet_val_amend(hal_to_dmac_device_stru *pst_hal_device, wlan_channel_band_enum_uint8 en_freq,
                                                         oal_uint8 uc_rf_id, oal_int16 s_vdet_val_in, oal_int16 *ps_det_val_out)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_dyn_cali_vdet_val_amend)(pst_hal_device, en_freq, uc_rf_id, s_vdet_val_in, ps_det_val_out);
#endif
}

OAL_STATIC OAL_INLINE oal_void  hal_dyn_cali_get_tx_power_dc(hal_to_dmac_device_stru *pst_hal_device, wlan_channel_band_enum_uint8 en_freq,
                                                                        oal_uint8 uc_rf_id, oal_int16 *ps_tx_power_dc, oal_bool_enum_uint8 en_ppa_working)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_dyn_cali_get_tx_power_dc)(pst_hal_device, en_freq, uc_rf_id, ps_tx_power_dc, en_ppa_working);
#endif
}

OAL_STATIC OAL_INLINE oal_int16  hal_dyn_cali_get_gm_val(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_rf_id)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    return HAL_PUBLIC_HOOK_FUNC(_dyn_cali_get_gm_val)(pst_hal_device, uc_rf_id);
#else
    return 0;
#endif
}
#endif

#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST))

OAL_STATIC OAL_INLINE oal_void hal_set_sifs_resp_rate(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_rate_restrict)
{
    HAL_PUBLIC_HOOK_FUNC(_set_sifs_resp_rate)( pst_hal_device, en_rate_restrict);
}

OAL_STATIC OAL_INLINE oal_void hal_set_tx_lifetime_check(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_rate_restrict)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tx_lifetime_check)( pst_hal_device, en_rate_restrict);
}




OAL_STATIC OAL_INLINE oal_void hal_get_tx_multi_q_status(hal_to_dmac_device_stru * pst_hal_device, oal_uint32 * pul_status, oal_uint8 uc_qnum)
{
    HAL_PUBLIC_HOOK_FUNC(_get_tx_multi_q_status)( pst_hal_device, pul_status, uc_qnum);
}


OAL_STATIC OAL_INLINE oal_void hal_set_bcn_timeout_multi_q_enable(hal_to_dmac_vap_stru * pst_hal_vap, oal_uint8 uc_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_set_bcn_timeout_multi_q_enable)( pst_hal_vap, uc_enable);
}


OAL_STATIC OAL_INLINE oal_void hal_tx_retry_clear_dscr(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_retry_clear_dscr)(pst_hal_device, pst_tx_dscr);
}


OAL_STATIC OAL_INLINE oal_void hal_tx_get_bw_mode(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru *pst_dscr, wlan_bw_cap_enum_uint8 *pen_bw_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_bw_mode)(pst_hal_device, pst_dscr, pen_bw_mode);
}


OAL_STATIC OAL_INLINE oal_void  hal_reset_slave_ana_dbb_ch_sel(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_reset_slave_ana_dbb_ch_sel)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_set_ana_dbb_ch_sel(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ana_dbb_ch_sel)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_update_cbb_cfg(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_update_cbb_cfg)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_ce_enable_key(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_ce_enable_key)(pst_hal_device);
}

#endif

OAL_STATIC OAL_INLINE oal_void  hal_set_11b_reuse_sel(hal_to_dmac_device_stru  *pst_hal_device)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_11b_reuse_sel)(pst_hal_device);
#endif
}


OAL_STATIC OAL_INLINE oal_void  hal_tx_get_ampdu_len(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru *pst_dscr, oal_uint32 *pul_ampdu_len)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_ampdu_len)( pst_hal_device, pst_dscr, pul_ampdu_len);
}


OAL_STATIC OAL_INLINE oal_void  hal_tx_get_protocol_mode(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru *pst_dscr, oal_uint8 *puc_protocol_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_protocol_mode)( pst_hal_device, pst_dscr, puc_protocol_mode);
}


/*��ȡ������������С*/
OAL_STATIC OAL_INLINE oal_void hal_rx_get_size_dscr(hal_to_dmac_device_stru * pst_hal_device, oal_uint32 * pul_dscr_size)
{
    *pul_dscr_size = WLAN_RX_DSCR_SIZE;
}

/*��ȡ��������Ϣ*/
OAL_STATIC OAL_INLINE oal_void hal_rx_get_info_dscr(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_rx_dscr, hal_rx_ctl_stru *pst_rx_ctl, hal_rx_status_stru *pst_rx_status, hal_rx_statistic_stru *pst_rx_statistics)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_get_info_dscr)(pst_hal_device, pul_rx_dscr, pst_rx_ctl, pst_rx_status, pst_rx_statistics);
}

OAL_STATIC OAL_INLINE oal_void hal_get_hal_vap(hal_to_dmac_device_stru * pst_hal_device, oal_uint8 uc_vap_id, hal_to_dmac_vap_stru **ppst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_get_hal_vap)( pst_hal_device, uc_vap_id, ppst_hal_vap);
}

OAL_STATIC OAL_INLINE oal_void hal_rx_get_netbuffer_addr_dscr(hal_to_dmac_device_stru * pst_hal_device, oal_uint32 *pul_rx_dscr, oal_netbuf_stru ** ppul_mac_hdr_addr)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_get_netbuffer_addr_dscr)( pul_rx_dscr, ppul_mac_hdr_addr);
}

OAL_STATIC OAL_INLINE oal_void hal_rx_show_dscr_queue_info(hal_to_dmac_device_stru * pst_hal_device, hal_rx_dscr_queue_id_enum_uint8 en_rx_dscr_type)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_show_dscr_queue_info)( pst_hal_device, en_rx_dscr_type);
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
OAL_STATIC OAL_INLINE oal_void hal_rx_print_phy_debug_info(hal_to_dmac_device_stru *pst_hal_device,oal_uint32 *pul_rx_dscr, hal_rx_statistic_stru *pst_rx_statistics)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_print_phy_debug_info)(pst_hal_device, pul_rx_dscr, pst_rx_statistics);
}
#endif
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
OAL_STATIC OAL_INLINE oal_void hal_rx_record_frame_status_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_rx_dscr, hal_rx_dscr_queue_id_enum_uint8 en_queue_id)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_record_frame_status_info)( pst_hal_device, pul_rx_dscr, en_queue_id);
}
#endif

#ifdef _PRE_WLAN_FEATUER_PCIE_TEST
/*дPCIE TEST��burst*/
OAL_STATIC OAL_INLINE oal_void hal_pcie_test_write_burst(hal_to_dmac_device_stru * pst_hal_device, oal_uint16 us_data)
{
    HAL_PUBLIC_HOOK_FUNC(_pcie_test_write_burst)( pst_hal_device, us_data);
}
/*дPCIE TEST�Ķ����ݵ�ʹ��bit*/
OAL_STATIC OAL_INLINE oal_void hal_pcie_test_rdata_bit(hal_to_dmac_device_stru * pst_hal_device, oal_uint8 uc_data)
{
    HAL_PUBLIC_HOOK_FUNC(_pcie_test_rdata_bit)( pst_hal_device, uc_data);
}
/*дPCIE TEST�Ķ����ݵ���ʼ��ַ*/
OAL_STATIC OAL_INLINE oal_void hal_pcie_test_rdata_addr(hal_to_dmac_device_stru * pst_hal_device, oal_uint32 ul_addr)
{
    HAL_PUBLIC_HOOK_FUNC(_pcie_test_rdata_addr)( pst_hal_device, ul_addr);
}
/*дPCIE TEST��д���ݵ�ʹ��bit*/
OAL_STATIC OAL_INLINE oal_void hal_pcie_test_wdata_bit(hal_to_dmac_device_stru * pst_hal_device, oal_uint8 uc_data)
{
    HAL_PUBLIC_HOOK_FUNC(_pcie_test_wdata_bit)( pst_hal_device, uc_data);
}
/*дPCIE TEST��д���ݵ���ʼ��ַ*/
OAL_STATIC OAL_INLINE oal_void hal_pcie_test_wdata_addr(hal_to_dmac_device_stru * pst_hal_device, oal_uint32 ul_addr)
{
    HAL_PUBLIC_HOOK_FUNC(_pcie_test_wdata_addr)( pst_hal_device, ul_addr);
}
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
OAL_STATIC OAL_INLINE oal_void hal_rx_sync_invalid_dscr(hal_to_dmac_device_stru * pst_hal_device, oal_uint32 *pul_dscr, oal_uint8 en_queue_num)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_sync_invalid_dscr)( pst_hal_device, pul_dscr, en_queue_num);
}
#endif
OAL_STATIC OAL_INLINE oal_void hal_rx_free_dscr_list(hal_to_dmac_device_stru * pst_hal_device, hal_rx_dscr_queue_id_enum_uint8 en_queue_num, oal_uint32 *pul_rx_dscr)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_free_dscr_list)( pst_hal_device, en_queue_num, pul_rx_dscr);
}

OAL_STATIC OAL_INLINE oal_void hal_dump_tx_dscr(hal_to_dmac_device_stru * pst_hal_device, oal_uint32 *pul_tx_dscr)
{
    HAL_PUBLIC_HOOK_FUNC(_dump_tx_dscr)( pul_tx_dscr);
}


OAL_STATIC OAL_INLINE oal_void  hal_reg_write(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_addr, oal_uint32 ul_val)
{
    HAL_PUBLIC_HOOK_FUNC(_reg_write)( pst_hal_device, ul_addr, ul_val);
}


OAL_STATIC OAL_INLINE oal_void  hal_al_tx_hw(hal_to_dmac_device_stru *pst_hal_device, hal_al_tx_hw_stru *pst_al_tx_hw)
{
    HAL_PUBLIC_HOOK_FUNC(_al_tx_hw)( pst_hal_device, pst_al_tx_hw);
}


OAL_STATIC OAL_INLINE oal_void  hal_al_tx_hw_cfg(hal_to_dmac_device_stru *pst_hal_device, oal_uint32  ul_mode, oal_uint32 ul_rate)
{
    HAL_PUBLIC_HOOK_FUNC(_al_tx_hw_cfg)( pst_hal_device, ul_mode, ul_rate);
}





OAL_STATIC OAL_INLINE oal_void hal_set_counter_clear(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_set_counter_clear)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_reg_write16(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_addr, oal_uint16 us_val)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_reg_write16)( pst_hal_device, ul_addr, us_val);
#endif
}

#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS

OAL_STATIC OAL_INLINE oal_void hal_set_psd_memory(hal_to_dmac_device_stru *pst_hal_device, oal_int8 **ppc_addr)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_psd_memory)(pst_hal_device, ppc_addr);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_free_psd_mem(hal_to_dmac_device_stru *pst_hal_device)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_free_psd_mem)(pst_hal_device);
#endif
}



OAL_STATIC OAL_INLINE oal_void hal_set_psd_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_psd_en)( pst_hal_device, ul_reg_value);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_set_up_fft_psd_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_up_fft_psd_en)( pst_hal_device, ul_reg_value);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_set_fft_sample(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_fft_sample)( pst_hal_device, ul_reg_value);
#endif
}



OAL_STATIC OAL_INLINE oal_void hal_set_psd_nb_det_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_psd_nb_det_en)( pst_hal_device, ul_reg_value);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_set_psd_11b_det_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_psd_11b_det_en)( pst_hal_device, ul_reg_value);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_set_psd_ofdm_det_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_psd_ofdm_det_en)( pst_hal_device, ul_reg_value);
#endif
}


OAL_STATIC OAL_INLINE oal_void hal_set_psd_wifi_work_en(hal_to_dmac_device_stru *pst_hal_device)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_psd_wifi_work_en)( pst_hal_device);
#endif
}


OAL_STATIC OAL_INLINE oal_void hal_set_force_reg_clk_on(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_force_reg_clk_on)( pst_hal_device, ul_reg_value);
#endif
}


OAL_STATIC OAL_INLINE oal_void hal_set_sync_data_path_div_num(hal_to_dmac_device_stru *pst_hal_device)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_sync_data_path_div_num)( pst_hal_device);
#endif
}


OAL_STATIC OAL_INLINE oal_void hal_set_psd_the_num_nb(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_psd_the_num_nb)( pst_hal_device, ul_reg_value);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_set_psd_the_power_nb(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_psd_the_power_nb)( pst_hal_device, ul_reg_value);
#endif
}


OAL_STATIC OAL_INLINE oal_void hal_set_psd_the_rssi_nb(hal_to_dmac_device_stru *pst_hal_device, oal_int32 l_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_psd_the_rssi_nb)( pst_hal_device, l_reg_value);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_set_psd_the_bottom_noise(hal_to_dmac_device_stru *pst_hal_device, oal_int32 l_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_psd_the_bottom_noise)( pst_hal_device, l_reg_value);
#endif
}



OAL_STATIC OAL_INLINE oal_void hal_get_psd_data(hal_to_dmac_device_stru *pst_hal_device, oal_int8 *pc_start_addr, oal_uint32 *pul_psd_data_len)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_get_psd_data)(pst_hal_device, pc_start_addr, pul_psd_data_len);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_get_psd_info(hal_to_dmac_device_stru *pst_hal_device)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_get_psd_info)(pst_hal_device);
#endif
}



OAL_STATIC OAL_INLINE oal_void hal_get_single_psd_sample(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_index, oal_int8 *pc_psd_val)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_get_single_psd_sample)(pst_hal_device, us_index, pc_psd_val);
#endif
}

#endif

#ifdef _PRE_WLAN_FEATURE_11AX

OAL_STATIC OAL_INLINE oal_void hal_set_dev_support_11ax(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_dev_support_11ax)( pst_hal_device, uc_reg_value);
}

OAL_STATIC OAL_INLINE oal_void hal_set_mu_edca_lifetime(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lifetime)
{
    HAL_PUBLIC_HOOK_FUNC(_set_mu_edca_lifetime)( pst_hal_device, uc_lifetime);
}

OAL_STATIC OAL_INLINE oal_void hal_set_mu_edca_aifsn(hal_to_dmac_device_stru *pst_hal_device,
                                            oal_uint8               uc_bk,
                                            oal_uint8               uc_be,
                                            oal_uint8               uc_vi,
                                            oal_uint8               uc_vo)
{
    HAL_PUBLIC_HOOK_FUNC(_set_mu_edca_aifsn)( pst_hal_device, uc_bk, uc_be, uc_vi, uc_vo);
}

OAL_STATIC OAL_INLINE oal_void hal_set_mu_edca_cw(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_ac_type, oal_uint8 uc_cwmax, oal_uint8 uc_cwmin)
{
    HAL_PUBLIC_HOOK_FUNC(_set_mu_edca_cw)( pst_hal_device, uc_ac_type, uc_cwmax, uc_cwmin);
}

OAL_STATIC OAL_INLINE oal_void hal_set_bss_color(hal_to_dmac_device_stru *pst_hal_device, hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_bss_color)
{
    HAL_PUBLIC_HOOK_FUNC(_set_bss_color)( pst_hal_device, pst_hal_vap, uc_bss_color);
}

OAL_STATIC OAL_INLINE oal_void hal_set_partial_bss_color(hal_to_dmac_device_stru *pst_hal_device, hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_partial_bss_color)
{
    HAL_PUBLIC_HOOK_FUNC(_set_partial_bss_color)( pst_hal_device, pst_hal_vap, uc_partial_bss_color);
}

#endif

#ifdef _PRE_WLAN_FEATURE_CSI

OAL_STATIC OAL_INLINE oal_void hal_set_csi_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_csi_en)( pst_hal_device, ul_reg_value);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_set_csi_ta(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_addr)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_csi_ta)( pst_hal_device, puc_addr);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_set_csi_ta_check(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_csi_ta_check)( pst_hal_device, ul_reg_value);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_set_pktmem_csi_bus_access(hal_to_dmac_device_stru *pst_hal_device)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_pktmem_csi_bus_access)(pst_hal_device);
#endif
}

#if 0


OAL_STATIC OAL_INLINE oal_void hal_set_csi_buf_pointer(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_csi_buf_pointer)( pst_hal_device, ul_reg_value);
#endif
}
#endif

OAL_STATIC OAL_INLINE oal_void hal_get_mac_csi_ta(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_addr)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_get_mac_csi_ta)( pst_hal_device,puc_addr);
#endif
}


OAL_STATIC OAL_INLINE oal_void hal_get_mac_csi_info(hal_to_dmac_device_stru *pst_hal_device)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_get_mac_csi_info)( pst_hal_device);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_get_phy_csi_info(hal_to_dmac_device_stru *pst_hal_device, wlan_channel_bandwidth_enum_uint8 *pen_bandwidth, oal_uint8 *puc_frame_type )
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_get_phy_csi_info)( pst_hal_device, pen_bandwidth, puc_frame_type);
#endif
}
OAL_STATIC OAL_INLINE oal_void hal_get_csi_frame_type(hal_to_dmac_device_stru *pst_hal_device,oal_uint8 *puc_he_flag, oal_uint8 *puc_frame_type)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
        HAL_PUBLIC_HOOK_FUNC(_get_csi_frame_type)( pst_hal_device, puc_he_flag, puc_frame_type);
#endif

}



OAL_STATIC OAL_INLINE oal_void hal_set_csi_memory(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_reg_num)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_csi_memory)(pst_hal_device, pul_reg_num);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_disable_csi_sample(hal_to_dmac_device_stru *pst_hal_device)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
        HAL_PUBLIC_HOOK_FUNC(_disable_csi_sample)(pst_hal_device);
#endif

}


OAL_STATIC OAL_INLINE oal_void hal_free_csi_sample_mem(hal_to_dmac_device_stru *pst_hal_device)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_free_csi_sample_mem)(pst_hal_device);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_get_csi_end_addr(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 **pul_csi_end_addr)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
     HAL_PUBLIC_HOOK_FUNC(_get_csi_end_addr)(pst_hal_device, pul_csi_end_addr);
#endif

}
OAL_STATIC OAL_INLINE oal_void hal_get_pktmem_start_addr(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 **pul_pktmem_start_addr)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
     HAL_PUBLIC_HOOK_FUNC(_get_pktmem_start_addr)(pst_hal_device, pul_pktmem_start_addr);
#endif

}
OAL_STATIC OAL_INLINE oal_void hal_get_pktmem_end_addr(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 **pul_pktmem_end_addr)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
     HAL_PUBLIC_HOOK_FUNC(_get_pktmem_end_addr)(pst_hal_device, pul_pktmem_end_addr);
#endif

}



#endif
#if defined _PRE_WLAN_PRODUCT_1151V200 && defined _PRE_WLAN_RX_DSCR_TRAILER

OAL_STATIC OAL_INLINE oal_void hal_set_ant_rssi_report(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_switch)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ant_rssi_report)( pst_hal_device, uc_switch);
}


OAL_STATIC OAL_INLINE oal_void hal_get_ant_rssi_rep_sw(hal_to_dmac_device_stru *pst_hal_device, oal_uint8* uc_switch)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ant_rssi_rep_sw)( pst_hal_device, uc_switch);
}


OAL_STATIC OAL_INLINE oal_void hal_get_ant_rssi_value(hal_to_dmac_device_stru *pst_hal_device, oal_int16 *ps_ant0, oal_int16 *ps_ant1)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ant_rssi_value)( pst_hal_device, ps_ant0, ps_ant1);
}


OAL_STATIC OAL_INLINE oal_void hal_update_ant_rssi_value(hal_to_dmac_device_stru *pst_hal_device, oal_int16 s_ant0, oal_int16 s_ant1)
{
    HAL_PUBLIC_HOOK_FUNC(_update_ant_rssi_value)( pst_hal_device, s_ant0, s_ant1);
}
#endif

#ifdef _PRE_WLAN_FEATURE_DATA_SAMPLE

OAL_STATIC OAL_INLINE oal_void hal_set_sample_memory(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 **ppul_addr, oal_uint32 *pul_reg_num)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_sample_memory)(pst_hal_device, ppul_addr, pul_reg_num);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_free_sample_mem(hal_to_dmac_device_stru *pst_hal_device)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_free_sample_mem)(pst_hal_device);
#endif
}

#endif



OAL_STATIC OAL_INLINE oal_void hal_set_pktmem_bus_access(hal_to_dmac_device_stru *pst_hal_device)
{
#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) && ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV))
    HAL_PUBLIC_HOOK_FUNC(_set_pktmem_bus_access)(pst_hal_device);
#endif
}


OAL_STATIC OAL_INLINE oal_void hal_get_sample_state(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 *pus_reg_val)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_get_sample_state)(pst_hal_device, pus_reg_val);
#endif
}

#ifdef _PRE_WLAN_RF_AUTOCALI
OAL_STATIC OAL_INLINE oal_void hal_rf_cali_auto_switch(oal_uint8 uc_switch_mask)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_rf_cali_auto_switch)(uc_switch_mask);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_rf_cali_auto_mea_done(oal_uint8 uc_freq,
                                                         oal_uint8 uc_chn_idx,
                                                         oal_uint8 uc_cali_type,
                                                         oal_uint8 uc_cali_stage)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_rf_cali_auto_mea_done)(uc_freq, uc_chn_idx, uc_cali_type, uc_cali_stage);
#endif
}
#endif


OAL_STATIC OAL_INLINE oal_void  hal_set_machw_rx_buff_addr(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_rx_dscr, hal_rx_dscr_queue_id_enum_uint8 en_queue_num)
{
    HAL_PUBLIC_HOOK_FUNC(_set_machw_rx_buff_addr)( pst_hal_device, ul_rx_dscr, en_queue_num);
}

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)

OAL_STATIC OAL_INLINE oal_uint32  hal_set_machw_rx_buff_addr_sync(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_rx_dscr, hal_rx_dscr_queue_id_enum_uint8 en_queue_num)
{
    return HAL_PUBLIC_HOOK_FUNC(_set_machw_rx_buff_addr_sync)( pst_hal_device, pul_rx_dscr, en_queue_num);
}
#endif

#if (_PRE_MULTI_CORE_MODE ==_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC)
OAL_STATIC OAL_INLINE oal_void hal_get_pwr_comp_val(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_tx_ratio, oal_int16 * ps_pwr_comp_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_pwr_comp_val)( pst_hal_device, ul_tx_ratio, ps_pwr_comp_val);
}

OAL_STATIC OAL_INLINE oal_void hal_agc_threshold_handle(hal_to_dmac_device_stru *pst_hal_device, oal_int8 c_rssi)
{
    HAL_PUBLIC_HOOK_FUNC(_agc_threshold_handle)(pst_hal_device, c_rssi);
}



OAL_STATIC OAL_INLINE oal_void hal_over_temp_handler(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_over_temp_handler)( pst_hal_device);
}


#endif


OAL_STATIC OAL_INLINE oal_void hal_rx_add_dscr(hal_to_dmac_device_stru *pst_hal_device, hal_rx_dscr_queue_id_enum_uint8 en_queue_num, oal_uint16 us_rx_dscr_num)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_add_dscr)( pst_hal_device, en_queue_num, us_rx_dscr_num);
}


OAL_STATIC OAL_INLINE oal_void hal_recycle_rx_isr_list(hal_to_dmac_device_stru  *pst_hal_device, oal_dlist_head_stru  *pst_rx_isr_list)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_recycle_rx_isr_list)( pst_hal_device, pst_rx_isr_list);
#endif
}


OAL_STATIC OAL_INLINE oal_void  hal_set_machw_tx_suspend(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_set_machw_tx_suspend)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_set_machw_tx_resume(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_set_machw_tx_resume)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_reset_phy_machw(hal_to_dmac_device_stru * pst_hal_device,hal_reset_hw_type_enum_uint8 en_type,
                                                        oal_uint8 sub_mod,oal_uint8 uc_reset_phy_reg,oal_uint8 uc_reset_mac_reg)
{
    OAM_PROFILING_STARTTIME_STATISTIC(OAM_PROFILING_RESET_HW_BEGIN);

    HAL_PUBLIC_HOOK_FUNC(_reset_phy_machw)( pst_hal_device,en_type,sub_mod,uc_reset_phy_reg,uc_reset_mac_reg);

    OAM_PROFILING_STARTTIME_STATISTIC(OAM_PROFILING_RESET_HW_END);

}

/*****************************************************************************
  hal��ʼ��/�˳�/��λ��ؽӿ�
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  hal_disable_machw_phy_and_pa(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_disable_machw_phy_and_pa)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_enable_machw_phy_and_pa(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_machw_phy_and_pa)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_recover_machw_phy_and_pa(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_recover_machw_phy_and_pa)(pst_hal_device);
}
/*****************************************************************************
  hal MAC hardware��ʼ���ӿ�
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  hal_initialize_machw(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_initialize_machw)( pst_hal_device);
}

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
OAL_STATIC OAL_INLINE oal_void  hal_set_ampdu_tx_hw_on(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_enable, oal_uint8 uc_snd_type)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ampdu_tx_hw_on)( pst_hal_device, en_enable, uc_snd_type);
}
#endif


OAL_STATIC OAL_INLINE oal_void  hal_initialize_machw_common(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_initialize_machw_common)( pst_hal_device);
}


/*****************************************************************************
  Ƶ�Ρ������ŵ�����ؽӿ�
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  hal_set_freq_band(hal_to_dmac_device_stru *pst_hal_device, wlan_channel_band_enum_uint8 en_band)
{
    HAL_PUBLIC_HOOK_FUNC(_set_freq_band)( pst_hal_device, en_band);
}


OAL_STATIC OAL_INLINE oal_void  hal_set_machw_phy_adc_freq(hal_to_dmac_device_stru *pst_hal_device, wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_machw_phy_adc_freq)( pst_hal_device, en_bandwidth);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_set_power_test(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_en)
{
#ifdef _PRE_1103_CHIP_POWER_TEST
    HAL_PUBLIC_HOOK_FUNC(_set_power_test)( pst_hal_device, uc_en);
#endif

}


OAL_STATIC OAL_INLINE oal_void  hal_set_bandwidth_mode(hal_to_dmac_device_stru *pst_hal_device, wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    HAL_PUBLIC_HOOK_FUNC(_set_bandwidth_mode)( pst_hal_device, en_bandwidth);
}
OAL_STATIC OAL_INLINE oal_void hal_process_phy_freq(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_process_phy_freq)( pst_hal_device);
}

OAL_STATIC OAL_INLINE oal_void  hal_set_primary_channel(
                hal_to_dmac_device_stru          *pst_hal_device,
                oal_uint8                         uc_channel_num,
                wlan_channel_band_enum_uint8      en_band,
                oal_uint8                         uc_channel_idx,
                wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    HAL_PUBLIC_HOOK_FUNC(_set_primary_channel)( pst_hal_device, uc_channel_num, en_band, uc_channel_idx, en_bandwidth);
}

#ifdef _PRE_WLAN_HW_TEST

OAL_STATIC OAL_INLINE oal_void  hal_set_phy_tx_scale(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_2g_11ac)
{
    HAL_PUBLIC_HOOK_FUNC(_set_phy_tx_scale)( pst_hal_device, en_2g_11ac);
}
#endif

#ifdef _PRE_DEBUG_MODE

OAL_STATIC OAL_INLINE oal_void  hal_freq_adjust(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_pll_int, oal_uint16 us_pll_frac)
{
    HAL_PUBLIC_HOOK_FUNC(_freq_adjust)( pst_hal_device, us_pll_int, us_pll_frac);
}
#endif

OAL_STATIC OAL_INLINE oal_void  hal_set_rx_multi_ant(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_phy_chain)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rx_multi_ant)( pst_hal_device, uc_phy_chain);
}


OAL_STATIC OAL_INLINE oal_void  hal_add_machw_ba_lut_entry(hal_to_dmac_device_stru *pst_hal_device,
                oal_uint8 uc_lut_index, oal_uint8 *puc_dst_addr, oal_uint8 uc_tid,
                oal_uint16 uc_seq_no, oal_uint8 uc_win_size)
{
    HAL_PUBLIC_HOOK_FUNC(_add_machw_ba_lut_entry)( pst_hal_device,
                    uc_lut_index, puc_dst_addr, uc_tid, uc_seq_no, uc_win_size);
}


OAL_STATIC OAL_INLINE oal_void  hal_add_machw_tx_ba_lut_entry(hal_to_dmac_device_stru* pst_hal_device,
        oal_uint8 uc_lut_index, oal_uint8 uc_tid,
        oal_uint16 uc_seq_no, oal_uint8 uc_win_size, oal_uint8 uc_mmss)
{
    HAL_PUBLIC_HOOK_FUNC(_add_machw_tx_ba_lut_entry)( pst_hal_device,
            uc_lut_index, uc_tid, uc_seq_no, uc_win_size, uc_mmss);
}



OAL_STATIC OAL_INLINE oal_void  hal_remove_machw_ba_lut_entry(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index)
{
    HAL_PUBLIC_HOOK_FUNC(_remove_machw_ba_lut_entry)( pst_hal_device, uc_lut_index);
}


OAL_STATIC OAL_INLINE oal_void  hal_get_machw_ba_params(hal_to_dmac_device_stru *pst_hal_device,oal_uint8 uc_index,
                                                        oal_uint32* pst_addr_h,oal_uint32* pst_addr_l,
                                                        oal_uint32* pst_bitmap_h,oal_uint32* pst_bitmap_l,oal_uint32* pst_ba_para)
{
    HAL_PUBLIC_HOOK_FUNC(_get_machw_ba_params)( pst_hal_device, uc_index,pst_addr_h,pst_addr_l,pst_bitmap_h,pst_bitmap_l,pst_ba_para);
}


OAL_STATIC OAL_INLINE oal_void  hal_restore_machw_ba_params(hal_to_dmac_device_stru *pst_hal_device,oal_uint8 uc_index,
                                                oal_uint32 ul_addr_h,oal_uint32 ul_addr_l,oal_uint32 ul_ba_para)
{
    HAL_PUBLIC_HOOK_FUNC(_restore_machw_ba_params)( pst_hal_device, uc_index,ul_addr_h,ul_addr_l,ul_ba_para);
}

#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND

OAL_STATIC OAL_INLINE oal_void  hal_restore_machw_ba_params_with_bitmap(hal_to_dmac_device_stru *pst_hal_device,oal_uint8 uc_index,
                                                oal_uint32 ul_addr_h,oal_uint32 ul_addr_l,oal_uint32 ul_ba_para,oal_uint32 ul_bitmap_h,oal_uint32 ul_bitmap_l)
{
    HAL_PUBLIC_HOOK_FUNC(_restore_machw_ba_params_with_bitmap)( pst_hal_device, uc_index,ul_addr_h,ul_addr_l,ul_ba_para,ul_bitmap_h,ul_bitmap_l);
}
#endif

/*****************************************************************************
          RA LUT������ؽӿ�
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  hal_machw_seq_num_index_update_per_tid(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index, oal_uint8 uc_qos_flag)
{
    HAL_PUBLIC_HOOK_FUNC(_machw_seq_num_index_update_per_tid)( pst_hal_device, uc_lut_index, uc_qos_flag);
}


OAL_STATIC OAL_INLINE oal_void  hal_set_tx_sequence_num(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index,oal_uint8 uc_tid, oal_uint8 uc_qos_flag,oal_uint32 ul_val_write,oal_uint8 uc_vap_index)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tx_sequence_num)( pst_hal_device,uc_lut_index,uc_tid,uc_qos_flag, ul_val_write, uc_vap_index);
}


OAL_STATIC OAL_INLINE oal_void  hal_get_tx_seq_num(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index,oal_uint8 uc_tid, oal_uint8 uc_qos_flag, oal_uint8 uc_vap_index,oal_uint16 *pst_val_read)
{
    HAL_PUBLIC_HOOK_FUNC(_get_tx_seq_num)( pst_hal_device, uc_lut_index,uc_tid,uc_qos_flag,uc_vap_index,pst_val_read);
}

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
OAL_STATIC OAL_INLINE oal_void  hal_save_tx_ba_para(hal_to_dmac_device_stru *pst_hal_device, hal_ba_para_stru *pst_ba_para)
{
    HAL_PUBLIC_HOOK_FUNC(_save_tx_ba_para)(pst_hal_device, pst_ba_para);
}

OAL_STATIC OAL_INLINE oal_void  hal_get_tx_ba_para(hal_to_dmac_device_stru *pst_hal_device, hal_ba_para_stru *pst_ba_para)
{
    HAL_PUBLIC_HOOK_FUNC(_get_tx_ba_para)(pst_hal_device, pst_ba_para);
}
#endif


OAL_STATIC OAL_INLINE oal_void  hal_reset_init(hal_to_dmac_device_stru * pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_reset_init)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_reset_destroy(hal_to_dmac_device_stru * pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_reset_destroy)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_reset_reg_restore(hal_to_dmac_device_stru * pst_hal_device,hal_reset_hw_type_enum_uint8 en_type)
{
    HAL_PUBLIC_HOOK_FUNC(_reset_reg_restore)( pst_hal_device,en_type);
}

OAL_STATIC OAL_INLINE oal_void  hal_reset_reg_save(hal_to_dmac_device_stru * pst_hal_device,hal_reset_hw_type_enum_uint8 en_type)
{
    HAL_PUBLIC_HOOK_FUNC(_reset_reg_save)( pst_hal_device,en_type);
}


OAL_STATIC OAL_INLINE oal_void  hal_reset_reg_dma_save(hal_to_dmac_device_stru * pst_hal_device,oal_uint8* pucdmach0,oal_uint8* pucdmach1,oal_uint8* pucdmach2)
{
    HAL_PUBLIC_HOOK_FUNC(_reset_reg_dma_save)( pst_hal_device,pucdmach0,pucdmach1,pucdmach2);
}


OAL_STATIC OAL_INLINE oal_void  hal_reset_reg_dma_restore(hal_to_dmac_device_stru * pst_hal_device,oal_uint8* pucdmach0,oal_uint8* pucdmach1,oal_uint8* pucdmach2)
{
    HAL_PUBLIC_HOOK_FUNC(_reset_reg_dma_restore)( pst_hal_device,pucdmach0,pucdmach1,pucdmach2);
}


OAL_STATIC OAL_INLINE oal_void  hal_reset_rf_reg_restore_rx_n(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_reset_rf_reg_restore_rx_n)(pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_disable_machw_ack_trans(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_disable_machw_ack_trans)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_enable_machw_ack_trans(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_machw_ack_trans)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_disable_machw_cts_trans(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_disable_machw_cts_trans)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_enable_machw_cts_trans(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_machw_cts_trans)( pst_hal_device);
}


/*****************************************************************************
  PHY��ؽӿ�
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  hal_initialize_phy(hal_to_dmac_device_stru * pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_initialize_phy)( pst_hal_device);
}

#ifdef _PRE_WLAN_FEATURE_DFS

OAL_STATIC OAL_INLINE oal_void  hal_radar_config_reg(hal_to_dmac_device_stru *pst_hal_device, hal_dfs_radar_type_enum_uint8 en_dfs_domain)
{
    HAL_PUBLIC_HOOK_FUNC(_radar_config_reg)( pst_hal_device, en_dfs_domain);
}
OAL_STATIC OAL_INLINE oal_void  hal_radar_config_reg_bw(hal_to_dmac_device_stru *pst_hal_device, hal_dfs_radar_type_enum_uint8 en_radar_type, wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    HAL_PUBLIC_HOOK_FUNC(_radar_config_reg_bw)( pst_hal_device, en_radar_type, en_bandwidth);
}
OAL_STATIC OAL_INLINE oal_void hal_radar_enable_chirp_det(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_chirp_det)
{
    HAL_PUBLIC_HOOK_FUNC(_radar_enable_chirp_det)( pst_hal_device, en_chirp_det);
}


OAL_STATIC OAL_INLINE oal_void  hal_radar_get_pulse_info(hal_to_dmac_device_stru *pst_hal_device, hal_radar_pulse_info_stru *pst_pulse_info)
{
    HAL_PUBLIC_HOOK_FUNC(_radar_get_pulse_info)( pst_hal_device, pst_pulse_info);
}


OAL_STATIC OAL_INLINE oal_void  hal_radar_clean_pulse_buf(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_radar_clean_pulse_buf)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_int32  hal_set_radar_th_reg(hal_to_dmac_device_stru *pst_hal_device, oal_int32 l_th)
{
    return HAL_PUBLIC_HOOK_FUNC(_set_radar_th_reg)( pst_hal_device, l_th);
}


OAL_STATIC OAL_INLINE oal_void  hal_get_radar_th_reg(hal_to_dmac_device_stru *pst_hal_device, oal_int32 *pl_th)
{
    HAL_PUBLIC_HOOK_FUNC(_get_radar_th_reg)( pst_hal_device, pl_th);
}


OAL_STATIC OAL_INLINE oal_void  hal_trig_dummy_radar(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_radar_type)
{
    HAL_PUBLIC_HOOK_FUNC(_trig_dummy_radar)( pst_hal_device, uc_radar_type);
}

#endif


/*****************************************************************************
  RF��ؽӿ�
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  hal_initialize_rf_sys(hal_to_dmac_device_stru * pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_initialize_rf_sys)(pst_hal_device);
}

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
OAL_STATIC OAL_INLINE oal_void hal_init_dyn_cali_tx_pow(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_init_dyn_cali_tx_pow)(pst_hal_device);
}
#endif

#ifndef WIN32
OAL_STATIC OAL_INLINE oal_void  hal_pow_sw_initialize_tx_power(hal_to_dmac_device_stru * pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_sw_initialize_tx_power)(pst_hal_device);
}

OAL_STATIC OAL_INLINE oal_void  hal_pow_initialize_tx_power(hal_to_dmac_device_stru * pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_initialize_tx_power)(pst_hal_device);
}

OAL_STATIC  OAL_INLINE oal_void hal_pow_set_rf_regctl_enable(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_rf_linectl)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_set_rf_regctl_enable)(pst_hal_device, en_rf_linectl);
}

OAL_STATIC OAL_INLINE oal_void hal_pow_set_band_spec_frame_tx_power(hal_to_dmac_device_stru *pst_hal_device,
                                wlan_channel_band_enum_uint8 en_band, oal_uint8 uc_chan_num,
                                hal_rate_pow_code_gain_table_stru *pst_rate_pow_table)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_set_band_spec_frame_tx_power)(pst_hal_device, en_band, uc_chan_num, pst_rate_pow_table);
}

OAL_STATIC OAL_INLINE oal_void  hal_pow_cfg_show_log(hal_to_dmac_device_stru           *pst_hal_device,
                                                                hal_vap_pow_info_stru            *pst_vap_pow_info,
                                                                wlan_channel_band_enum_uint8      en_freq_band,
                                                                oal_uint8                        *puc_rate_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_cfg_show_log)( pst_hal_device, pst_vap_pow_info, en_freq_band, puc_rate_idx);
}
OAL_STATIC OAL_INLINE oal_void hal_pow_cfg_no_margin_pow_mode(hal_to_dmac_device_stru * pst_hal_device,
                                                                oal_uint8 uc_pow_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_cfg_no_margin_pow_mode)(pst_hal_device, uc_pow_mode);
}

#endif

#ifdef _PRE_WLAN_FEATURE_USER_RESP_POWER
OAL_STATIC OAL_INLINE oal_void  hal_pow_set_user_resp_frame_tx_power(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index, oal_uint8 uc_rssi_distance)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_set_user_resp_frame_tx_power)(pst_hal_device, uc_lut_index, uc_rssi_distance);
}

OAL_STATIC OAL_INLINE oal_void  hal_pow_oper_machw_resp_power_lut_entry(hal_to_dmac_device_stru *pst_hal_device,
                                                                       oal_uint8 uc_lut_index, hal_resp_pow_lut_oper_type_enum_uint8 en_rsp_pow_oper)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_oper_machw_resp_power_lut_entry)(pst_hal_device, uc_lut_index, en_rsp_pow_oper);
}
#endif

#ifndef _PRE_WLAN_FEATURE_TPC_OPT
OAL_STATIC OAL_INLINE oal_void  hal_pow_set_pow_to_pow_code(hal_to_dmac_device_stru *pst_hal_device, hal_vap_pow_info_stru *pst_vap_pow_info,
                            oal_uint8 uc_start_chain, wlan_channel_band_enum_uint8 en_freq_band)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_set_pow_to_pow_code)(pst_hal_device, pst_vap_pow_info, uc_start_chain, en_freq_band);
}
#else
#ifdef _PRE_WLAN_1103_PILOT
OAL_STATIC OAL_INLINE oal_void hal_pow_set_pow_to_pow_code(hal_to_dmac_device_stru *pst_hal_device,
                                                       wlan_channel_band_enum_uint8 en_freq_band, oal_uint8 uc_chan_idx,
                                                       oal_uint8 uc_chan_num, wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                                                       hal_pow_set_type_enum_uint8 uc_type)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_set_pow_to_pow_code)(pst_hal_device, en_freq_band, uc_chan_idx, uc_chan_num, en_bandwidth, uc_type);
}
#endif
#endif  /* _PRE_WLAN_FEATURE_TPC_OPT */

OAL_STATIC OAL_INLINE oal_void hal_pow_set_resp_frame_tx_power(hal_to_dmac_device_stru *pst_hal_device,
                                wlan_channel_band_enum_uint8 en_band, oal_uint8 uc_chan_num,
                                hal_rate_pow_code_gain_table_stru *pst_rate_pow_table)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_set_resp_frame_tx_power)(pst_hal_device, en_band, uc_chan_num, pst_rate_pow_table);
}


OAL_STATIC OAL_INLINE oal_void hal_pow_get_spec_frame_data_rate_idx(oal_uint8 uc_rate,  oal_uint8 *puc_rate_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_get_spec_frame_data_rate_idx)(uc_rate, puc_rate_idx);
}
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST))
OAL_STATIC OAL_INLINE oal_void hal_pow_set_pow_code_idx_same_in_tx_power(hal_tx_txop_tx_power_stru *pst_tx_power, oal_uint32 *pul_pow_code)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_set_pow_code_idx_same_in_tx_power)(pst_tx_power, pul_pow_code);
}
#else
OAL_STATIC OAL_INLINE oal_void hal_pow_set_pow_code_idx_same_in_tx_power(hal_tx_txop_tx_power_stru *pst_tx_power, oal_uint32 ul_pow_code)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_set_pow_code_idx_same_in_tx_power)(pst_tx_power, ul_pow_code);
}
#endif
OAL_STATIC OAL_INLINE oal_void hal_pow_get_pow_index(hal_user_pow_info_stru *pst_hal_user_pow_info,
                     oal_uint8 uc_cur_rate_pow_idx, hal_tx_txop_tx_power_stru *pst_tx_power, oal_uint8 *puc_pow_level)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_get_pow_index)(pst_hal_user_pow_info, uc_cur_rate_pow_idx, pst_tx_power, puc_pow_level);
}
OAL_STATIC OAL_INLINE oal_void hal_pow_set_four_rate_tx_dscr_power(hal_user_pow_info_stru *pst_hal_user_pow_info,
                                oal_uint8 *puc_rate_level_idx, oal_uint8 *pauc_pow_level,
                                hal_tx_txop_tx_power_stru *pst_tx_power)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_set_four_rate_tx_dscr_power)(pst_hal_user_pow_info, puc_rate_level_idx, pauc_pow_level, pst_tx_power);
}

OAL_STATIC OAL_INLINE oal_void hal_pow_set_pow_code_idx_in_tx_power(hal_tx_txop_tx_power_stru *pst_tx_power, oal_uint32 *aul_pow_code)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_set_pow_code_idx_in_tx_power)(pst_tx_power, aul_pow_code);
}

#if (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
OAL_STATIC OAL_INLINE oal_void  hal_set_rf_custom_reg(hal_to_dmac_device_stru * pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rf_custom_reg)( pst_hal_device);
}
#endif

#endif
OAL_STATIC OAL_INLINE oal_void  hal_psm_rf_sleep(hal_to_dmac_device_stru * pst_hal_device, oal_uint8 uc_restore_reg)
{
    HAL_PUBLIC_HOOK_FUNC(_psm_rf_sleep)( pst_hal_device, uc_restore_reg);
}

OAL_STATIC OAL_INLINE oal_void  hal_psm_rf_awake(hal_to_dmac_device_stru  *pst_hal_device,oal_uint8 uc_restore_reg)
{
    HAL_PUBLIC_HOOK_FUNC(_psm_rf_awake)(pst_hal_device,uc_restore_reg);
}

/* ��дrf reg */
OAL_STATIC OAL_INLINE oal_void  hal_read_rf_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_reg_addr, oal_uint16 *pus_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_read_rf_reg)(pst_hal_device, us_reg_addr, pus_reg_val);
}
OAL_STATIC OAL_INLINE oal_void  hal_write_rf_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint16  us_rf_addr_offset, oal_uint16 us_rf_16bit_data)
{
    HAL_PUBLIC_HOOK_FUNC(_write_rf_reg)(pst_hal_device, us_rf_addr_offset, us_rf_16bit_data);
}


OAL_STATIC OAL_INLINE oal_void hal_psm_recover_primary_channel(hal_to_dmac_device_stru *pst_hal_device)
{
    mac_channel_stru    *pst_wifi_channel_status;

    pst_wifi_channel_status = &(pst_hal_device->st_wifi_channel_status);

    /* ���ô��� */
    pst_hal_device->st_wifi_channel_status.en_band     = pst_wifi_channel_status->en_band;
    pst_hal_device->st_wifi_channel_status.uc_chan_idx = pst_wifi_channel_status->uc_chan_idx;
    hal_set_bandwidth_mode(pst_hal_device, pst_wifi_channel_status->en_bandwidth);

    hal_set_primary_channel(pst_hal_device,
                   pst_wifi_channel_status->uc_chan_number,
                   pst_wifi_channel_status->en_band,
                   pst_wifi_channel_status->uc_chan_idx,
                   pst_wifi_channel_status->en_bandwidth);
}


OAL_STATIC OAL_INLINE oal_void hal_pm_enable_front_end(hal_to_dmac_device_stru  *pst_hal_device, oal_uint8 uc_enable_paldo)
{
#ifdef _PRE_WLAN_FEATURE_STA_PM
    HAL_PUBLIC_HOOK_FUNC(_pm_enable_front_end)(pst_hal_device, uc_enable_paldo);
#endif
}


OAL_STATIC OAL_INLINE oal_void hal_pm_disable_front_end_tx(hal_to_dmac_device_stru  *pst_hal_device)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    HAL_PUBLIC_HOOK_FUNC(_pm_disable_front_end_tx)(pst_hal_device);
#endif
}

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI

OAL_STATIC OAL_INLINE oal_void hal_config_set_dyn_cali_dscr_interval(hal_to_dmac_device_stru * pst_hal_device, wlan_channel_band_enum_uint8 uc_band,oal_uint16 us_param_val)
{
     HAL_PUBLIC_HOOK_FUNC(_config_set_dyn_cali_dscr_interval)(pst_hal_device, uc_band, us_param_val);
     return;
}

#ifdef _PRE_WLAN_DPINIT_CALI

OAL_STATIC OAL_INLINE oal_void hal_config_get_dyn_cali_dpinit_val(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_param_val)
{
     HAL_PUBLIC_HOOK_FUNC(_config_get_dyn_cali_dpinit_val)(pst_hal_device, us_param_val);
     return;
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

OAL_STATIC OAL_INLINE oal_void hal_rf_cali_realtime_entrance(hal_to_dmac_device_stru * OAL_CONST pst_hal_device,
                                                             hal_pdet_info_stru * OAL_CONST pst_pdet_info,
                                                             hal_dyn_cali_usr_record_stru * OAL_CONST pst_user_pow,
                                                             hal_tx_dscr_stru             * OAL_CONST pst_base_dscr)
{
     HAL_PUBLIC_HOOK_FUNC(_rf_cali_realtime_entrance)(pst_hal_device, pst_pdet_info, pst_user_pow, pst_base_dscr);
}
#else
OAL_STATIC OAL_INLINE oal_void hal_rf_cali_realtime_entrance(hal_to_dmac_device_stru * OAL_CONST pst_hal_device,
                                                             hal_pdet_info_stru * OAL_CONST pst_pdet_info,
                                                             hal_dyn_cali_usr_record_stru * OAL_CONST pst_user_pow)
{
     HAL_PUBLIC_HOOK_FUNC(_rf_cali_realtime_entrance)(pst_hal_device, pst_pdet_info, pst_user_pow);
}
#endif
#endif

#ifdef _PRE_WLAN_REALTIME_CALI

OAL_STATIC OAL_INLINE oal_void hal_rf_cali_realtime(hal_to_dmac_device_stru * pst_hal_device)
{
     HAL_PUBLIC_HOOK_FUNC(_rf_cali_realtime)( pst_hal_device);
}
#endif

/*****************************************************************************
  SoC��ؽӿ�
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  hal_initialize_soc(hal_to_dmac_device_stru * pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_initialize_soc)( pst_hal_device);
}

/*****************************************************************************
  �ж���ؽӿ�
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  hal_get_mac_int_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_status)
{
    HAL_PUBLIC_HOOK_FUNC(_get_mac_int_status)( pst_hal_device, pul_status);
}


OAL_STATIC OAL_INLINE oal_void  hal_clear_mac_int_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_status)
{
    HAL_PUBLIC_HOOK_FUNC(_clear_mac_int_status)( pst_hal_device, ul_status);
}


OAL_STATIC OAL_INLINE oal_void  hal_get_mac_error_int_status(hal_to_dmac_device_stru *pst_hal_device, hal_error_state_stru *pst_state)
{
    HAL_PUBLIC_HOOK_FUNC(_get_mac_error_int_status)( pst_hal_device, pst_state);
}


OAL_STATIC OAL_INLINE oal_void  hal_clear_mac_error_int_status(hal_to_dmac_device_stru *pst_hal_device, hal_error_state_stru *pst_status)
{
    HAL_PUBLIC_HOOK_FUNC(_clear_mac_error_int_status)( pst_hal_device, pst_status);
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
/* ERROR IRQ�жϼĴ�����ز��� */
OAL_STATIC OAL_INLINE oal_void  hal_unmask_mac_error_init_status(hal_to_dmac_device_stru * pst_hal_device, hal_error_state_stru *pst_status)
{
    HAL_PUBLIC_HOOK_FUNC(_unmask_mac_error_init_status)( pst_hal_device, pst_status);
}
#endif
/* mac�жϼĴ�����ز��� */
OAL_STATIC OAL_INLINE oal_void  hal_unmask_mac_init_status(hal_to_dmac_device_stru * pst_hal_device, oal_uint32 ul_status)
{
    HAL_PUBLIC_HOOK_FUNC(_unmask_mac_init_status)( pst_hal_device, ul_status);
}

OAL_STATIC OAL_INLINE oal_void  hal_show_irq_info(hal_to_dmac_device_stru * pst_hal_device, oal_uint8 uc_param)
{
    HAL_PUBLIC_HOOK_FUNC(_show_irq_info)( pst_hal_device, uc_param);
}

OAL_STATIC OAL_INLINE oal_void  hal_dump_all_rx_dscr(hal_to_dmac_device_stru * pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_dump_all_rx_dscr)( pst_hal_device);
}

OAL_STATIC OAL_INLINE oal_void  hal_clear_irq_stat(hal_to_dmac_device_stru * pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_clear_irq_stat)( pst_hal_device);
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
typedef enum
{
    HAL_IRQ_TX_COMP_CNT     = 0,
    HAL_IRQ_STAT_BUTT
}hal_irq_stat_enum;
typedef oal_uint8 hal_irq_stat_enum_uint8;

OAL_STATIC OAL_INLINE oal_void hal_get_irq_stat(hal_to_dmac_device_stru * pst_hal_device, oal_uint8 *puc_param, oal_uint32 ul_len, hal_irq_stat_enum_uint8 en_type)
{
    HAL_PUBLIC_HOOK_FUNC(_get_irq_stat)( pst_hal_device, puc_param, ul_len, en_type);
}
#endif



OAL_STATIC OAL_INLINE oal_void hal_cali_send_func(hal_to_dmac_device_stru *pst_hal_device, oal_uint8* puc_cal_data_write, oal_uint16 us_frame_len, oal_uint16 us_remain)
{
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)) || ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST))
  HAL_PUBLIC_HOOK_FUNC(_cali_send_func)(pst_hal_device, puc_cal_data_write, us_frame_len, us_remain);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_cali_matrix_data_send_func(hal_to_dmac_device_stru *pst_hal_device, oal_uint8* puc_cal_data_write, oal_uint16 us_frame_len, oal_uint16 us_remain)
{
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST))
  HAL_PUBLIC_HOOK_FUNC(_cali_matrix_data_send_func)(pst_hal_device, puc_cal_data_write, us_frame_len, us_remain);
#endif
}

/*
����vapģʽ��hal����vap�ṹ�������vap id
*/
OAL_STATIC OAL_INLINE oal_void hal_add_vap(hal_to_dmac_device_stru * pst_hal_device, wlan_vap_mode_enum_uint8 vap_mode, oal_uint8 uc_mac_vap_id, hal_to_dmac_vap_stru ** ppst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_add_vap)( pst_hal_device, vap_mode, uc_mac_vap_id, ppst_hal_vap);
}

/*
����vapģʽ��halɾ��vap�ṹ�������vap id
*/
OAL_STATIC OAL_INLINE oal_void hal_del_vap(hal_to_dmac_device_stru * pst_hal_device, wlan_vap_mode_enum_uint8 vap_mode, oal_uint8 vap_id)
{
    HAL_PUBLIC_HOOK_FUNC(_del_vap)( pst_hal_device, vap_mode, vap_id);
}


#ifdef _PRE_WLAN_FEATURE_PROXYSTA
OAL_STATIC OAL_INLINE oal_void hal_set_proxysta_enable(hal_to_dmac_device_stru * pst_hal_device, oal_int32 l_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_set_proxysta_enable)( pst_hal_device, l_enable);
}
#endif

OAL_STATIC OAL_INLINE oal_void  hal_config_eifs_time(hal_to_dmac_device_stru *pst_hal_device, wlan_protocol_enum_uint8 en_protocol)
{
    HAL_PUBLIC_HOOK_FUNC(_config_eifs_time)( pst_hal_device, en_protocol);
}


OAL_STATIC OAL_INLINE oal_void  hal_register_alg_isr_hook(hal_to_dmac_device_stru    *pst_hal_device,
                                                          hal_isr_type_enum_uint8     en_isr_type,
                                                          hal_alg_noify_enum_uint8    en_alg_notify,
                                                          p_hal_alg_isr_func          p_func)

{
    HAL_PUBLIC_HOOK_FUNC(_register_alg_isr_hook)( pst_hal_device, en_isr_type, en_alg_notify, p_func);
}

OAL_STATIC OAL_INLINE oal_void  hal_unregister_alg_isr_hook(hal_to_dmac_device_stru    *pst_hal_device,
                                                            hal_isr_type_enum_uint8     en_isr_type,
                                                            hal_alg_noify_enum_uint8    en_alg_notify)
{
    HAL_PUBLIC_HOOK_FUNC(_unregister_alg_isr_hook)(pst_hal_device, en_isr_type, en_alg_notify);
}


OAL_STATIC OAL_INLINE oal_void  hal_register_gap_isr_hook(hal_to_dmac_device_stru    *pst_hal_device,
                                                          hal_isr_type_enum_uint8     en_isr_type,
                                                          p_hal_gap_isr_func          p_func)

{
    HAL_PUBLIC_HOOK_FUNC(_register_gap_isr_hook)( pst_hal_device, en_isr_type, p_func);
}


OAL_STATIC OAL_INLINE oal_void  hal_unregister_gap_isr_hook(hal_to_dmac_device_stru    *pst_hal_device,
                                                            hal_isr_type_enum_uint8     en_isr_type)
{
    HAL_PUBLIC_HOOK_FUNC(_unregister_gap_isr_hook)(pst_hal_device, en_isr_type);
}



OAL_STATIC OAL_INLINE oal_void hal_one_packet_start(struct tag_hal_to_dmac_device_stru *pst_hal_device, hal_one_packet_cfg_stru *pst_cfg)
{
    HAL_PUBLIC_HOOK_FUNC(_one_packet_start)( pst_hal_device, pst_cfg);
}

OAL_STATIC OAL_INLINE oal_void hal_one_packet_stop(struct tag_hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_one_packet_stop)( pst_hal_device);
}

OAL_STATIC OAL_INLINE oal_void hal_one_packet_get_status(struct tag_hal_to_dmac_device_stru *pst_hal_device, hal_one_packet_status_stru *pst_status)
{
    HAL_PUBLIC_HOOK_FUNC(_one_packet_get_status)( pst_hal_device, pst_status);
}


OAL_STATIC OAL_INLINE oal_void hal_reset_nav_timer(struct tag_hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_reset_nav_timer)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void hal_clear_hw_fifo(struct tag_hal_to_dmac_device_stru *pst_hal_device)
{
    oal_udelay(10);
    HAL_PUBLIC_HOOK_FUNC(_clear_hw_fifo)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void hal_mask_interrupt(struct tag_hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_offset)
{
    HAL_PUBLIC_HOOK_FUNC(_mask_interrupt)( pst_hal_device, ul_offset);
}

OAL_STATIC OAL_INLINE oal_void hal_unmask_interrupt(struct tag_hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_offset)
{
    HAL_PUBLIC_HOOK_FUNC(_unmask_interrupt)( pst_hal_device, ul_offset);
}

/*****************************************************************************
  �Ĵ�������ӿ�
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  hal_reg_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_addr, oal_uint32 *pul_val)
{
    HAL_PUBLIC_HOOK_FUNC(_reg_info)( pst_hal_device, ul_addr, pul_val);
}

OAL_STATIC OAL_INLINE oal_void  hal_reg_info16(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_addr, oal_uint16 *pus_val)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_reg_info16)( pst_hal_device, ul_addr, pus_val);
#endif
}
OAL_STATIC OAL_INLINE oal_void hal_get_all_tx_q_status(hal_to_dmac_device_stru *pst_hal_device,oal_uint32 *pul_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_all_tx_q_status)( pst_hal_device, pul_val);
}

#ifdef _PRE_WLAN_ONLINE_DPD
OAL_STATIC OAL_INLINE oal_void  hal_dpd_config(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_val)
{

    HAL_PUBLIC_HOOK_FUNC(_dpd_config)(pst_hal_device, puc_val);
}

OAL_STATIC OAL_INLINE oal_void  hal_dpd_cfr_set_work_mode(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_set_type, oal_uint8 uc_value)
{
    HAL_PUBLIC_HOOK_FUNC(_dpd_cfr_set_work_mode)(pst_hal_device, uc_set_type, uc_value);
}

#endif


OAL_STATIC OAL_INLINE oal_void hal_get_ampdu_bytes(hal_to_dmac_device_stru *pst_hal_device,oal_uint32 *pul_tx_bytes, oal_uint32 *pul_rx_bytes)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ampdu_bytes)( pst_hal_device, pul_tx_bytes, pul_rx_bytes);
}

OAL_STATIC OAL_INLINE oal_void hal_get_rx_err_count(hal_to_dmac_device_stru *pst_hal_device,
                                                          oal_uint32 *pul_cnt1,
                                                          oal_uint32 *pul_cnt2,
                                                          oal_uint32 *pul_cnt3,
                                                          oal_uint32 *pul_cnt4,
                                                          oal_uint32 *pul_cnt5,
                                                          oal_uint32 *pul_cnt6)
{
    HAL_PUBLIC_HOOK_FUNC(_get_rx_err_count)( pst_hal_device, pul_cnt1, pul_cnt2, pul_cnt3, pul_cnt4, pul_cnt5, pul_cnt6);
}


OAL_STATIC OAL_INLINE oal_void  hal_show_fsm_info(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_show_fsm_info)( pst_hal_device);
}

OAL_STATIC OAL_INLINE oal_void  hal_show_wow_state_info(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_show_wow_state_info)(pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_mac_error_msg_report(hal_to_dmac_device_stru *pst_hal_device, hal_mac_error_type_enum_uint8 en_error_type)
{
    HAL_PUBLIC_HOOK_FUNC(_mac_error_msg_report)( pst_hal_device, en_error_type);
}


OAL_STATIC OAL_INLINE oal_void hal_get_dieid(hal_to_dmac_device_stru * pst_hal_device, oal_uint32 *pul_dieid, oal_uint32 *pul_length)
{
    HAL_PUBLIC_HOOK_FUNC(_get_dieid)(pst_hal_device, pul_dieid, pul_length);
}
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

OAL_STATIC OAL_INLINE oal_void  hal_soc_error_msg_report(hal_to_dmac_device_stru *pst_hal_device, hal_mac_error_type_enum_uint8 en_error_type)
{
    HAL_PUBLIC_HOOK_FUNC(_mac_error_msg_report)( pst_hal_device, en_error_type);
}
#endif


OAL_STATIC OAL_INLINE oal_void  hal_en_soc_intr(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_en_soc_intr)( pst_hal_device);
}

OAL_STATIC  OAL_INLINE oal_void hal_enable_beacon_filter(hal_to_dmac_device_stru *pst_hal_device)
{
    //HAL_PUBLIC_HOOK_FUNC(_enable_beacon_filter, pst_hal_device);
}


OAL_STATIC  OAL_INLINE oal_void hal_disable_beacon_filter(hal_to_dmac_device_stru *pst_hal_device)
{
   HAL_PUBLIC_HOOK_FUNC(_disable_beacon_filter)( pst_hal_device);
}


OAL_STATIC  OAL_INLINE oal_void hal_enable_non_frame_filter(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_non_frame_filter)( pst_hal_device);
}


OAL_STATIC  OAL_INLINE oal_void hal_enable_monitor_mode(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_monitor_mode)( pst_hal_device);
}


OAL_STATIC  OAL_INLINE oal_void hal_disable_monitor_mode(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_disable_monitor_mode)( pst_hal_device);
}
#if (_PRE_WLAN_FEATURE_PMF  != _PRE_PMF_NOT_SUPPORT)


OAL_STATIC  OAL_INLINE oal_void  hal_set_pmf_crypto(hal_to_dmac_vap_stru *pst_hal_vap, oal_bool_enum_uint8 en_crypto)
{
#if(_PRE_WLAN_FEATURE_PMF == _PRE_PMF_HW_CCMP_BIP)
    HAL_PUBLIC_HOOK_FUNC(_set_pmf_crypto)( pst_hal_vap, en_crypto);
#else
    return;
#endif
}
#endif /* #if(_PRE_WLAN_FEATURE_PMF  != _PRE_PMF_NOT_SUPPORT)  */

OAL_STATIC  OAL_INLINE oal_void  hal_ce_add_key(hal_to_dmac_device_stru *pst_hal_device,hal_security_key_stru *pst_security_key,oal_uint8 *puc_addr)
{
    HAL_PUBLIC_HOOK_FUNC(_ce_add_key)( pst_hal_device,pst_security_key,puc_addr);
}

OAL_STATIC  OAL_INLINE oal_void  hal_ce_get_key(hal_to_dmac_device_stru *pst_hal_device, hal_security_key_stru *pst_security_key)
{
    HAL_PUBLIC_HOOK_FUNC(_ce_get_key)( pst_hal_device, pst_security_key);
}


#ifdef _PRE_WLAN_INIT_PTK_TX_PN

OAL_STATIC OAL_INLINE oal_void hal_tx_get_dscr_phy_mode_one(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr,  oal_uint32 *pul_phy_mode_one)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_phy_mode_one)( pst_tx_dscr, pul_phy_mode_one);
}


OAL_STATIC OAL_INLINE oal_void hal_tx_get_ra_lut_index(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru *pst_dscr, oal_uint8 *puc_ra_lut_index)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_ra_lut_index)( pst_hal_device, pst_dscr, puc_ra_lut_index);
}


OAL_STATIC  OAL_INLINE oal_void hal_init_ptk_tx_pn(hal_to_dmac_device_stru *pst_hal_device,hal_security_key_stru *pst_security_key,oal_uint32 ul_pn_msb)
{
    HAL_PUBLIC_HOOK_FUNC(_init_ptk_tx_pn)( pst_hal_device,pst_security_key,ul_pn_msb);
}
#endif


OAL_STATIC  OAL_INLINE oal_void  hal_disable_ce(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_disable_ce)( pst_hal_device);
}


OAL_STATIC  OAL_INLINE oal_void  hal_clear_user_ptk_key(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_clear_user_ptk_key)( pst_hal_device,uc_lut_idx);
}


OAL_STATIC  OAL_INLINE oal_void  hal_ce_del_key(hal_to_dmac_device_stru *pst_hal_device, hal_security_key_stru *pst_security_key)
{
    HAL_PUBLIC_HOOK_FUNC(_ce_del_key)( pst_hal_device,pst_security_key);
}


OAL_STATIC  OAL_INLINE oal_void hal_ce_add_peer_macaddr(hal_to_dmac_device_stru *pst_hal_device,oal_uint8 uc_lut_idx,oal_uint8 * puc_addr)
{
    HAL_PUBLIC_HOOK_FUNC(_ce_add_peer_macaddr)( pst_hal_device,uc_lut_idx,puc_addr);
}

OAL_STATIC  OAL_INLINE oal_void hal_ce_del_peer_macaddr(hal_to_dmac_device_stru *pst_hal_device,oal_uint8 uc_lut_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_ce_del_peer_macaddr)( pst_hal_device,uc_lut_idx);
}


OAL_STATIC  OAL_INLINE oal_void  hal_set_rx_pn(hal_to_dmac_device_stru *pst_hal_device,hal_pn_lut_cfg_stru* pst_pn_lut_cfg)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rx_pn)( pst_hal_device, pst_pn_lut_cfg);
}


OAL_STATIC  OAL_INLINE oal_void  hal_get_rx_pn(hal_to_dmac_device_stru *pst_hal_device,hal_pn_lut_cfg_stru* pst_pn_lut_cfg)
{
    HAL_PUBLIC_HOOK_FUNC(_get_rx_pn)( pst_hal_device, pst_pn_lut_cfg);
}


OAL_STATIC  OAL_INLINE oal_void  hal_set_tx_pn(hal_to_dmac_device_stru *pst_hal_device,hal_pn_lut_cfg_stru* pst_pn_lut_cfg)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tx_pn)( pst_hal_device, pst_pn_lut_cfg);
}


OAL_STATIC  OAL_INLINE oal_void  hal_get_tx_pn(hal_to_dmac_device_stru *pst_hal_device,hal_pn_lut_cfg_stru* pst_pn_lut_cfg)
{
    HAL_PUBLIC_HOOK_FUNC(_get_tx_pn)( pst_hal_device, pst_pn_lut_cfg);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_rate_80211g_table(hal_to_dmac_device_stru *pst_hal_device, oal_void **pst_rate)
{
    HAL_PUBLIC_HOOK_FUNC(_get_rate_80211g_table)( pst_rate);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_rate_80211g_num(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pst_data_num)
{
    HAL_PUBLIC_HOOK_FUNC(_get_rate_80211g_num)( pst_data_num);
}


OAL_STATIC  OAL_INLINE oal_void  hal_get_hw_addr(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_addr)
{
    HAL_PUBLIC_HOOK_FUNC(_get_hw_addr)( puc_addr);
}


OAL_STATIC  OAL_INLINE oal_void hal_enable_ch_statics(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_ch_statics)( pst_hal_device, uc_enable);
}



OAL_STATIC  OAL_INLINE oal_void hal_set_ch_statics_period(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_period)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ch_statics_period)( pst_hal_device, ul_period);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_ch_measurement_period(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_period)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ch_measurement_period)( pst_hal_device, uc_period);
}


OAL_STATIC OAL_INLINE oal_void hal_get_ch_statics_result(hal_to_dmac_device_stru *pst_hal_device, hal_ch_statics_irq_event_stru *pst_ch_statics)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ch_statics_result)( pst_hal_device, pst_ch_statics);
}


OAL_STATIC OAL_INLINE oal_void hal_get_ch_measurement_result_ram(hal_to_dmac_device_stru *pst_hal_device, hal_ch_statics_irq_event_stru *pst_ch_statics)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ch_measurement_result_ram)( pst_hal_device, pst_ch_statics);
}

/* ROM����ɾ�� */
OAL_STATIC OAL_INLINE oal_void hal_get_ch_measurement_result(hal_to_dmac_device_stru *pst_hal_device, hal_ch_statics_irq_event_stru *pst_ch_statics)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ch_measurement_result)( pst_hal_device, pst_ch_statics);
}


OAL_STATIC OAL_INLINE oal_void hal_enable_radar_det(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_radar_det)( pst_hal_device, uc_enable);
}
#ifdef _PRE_WLAN_FEATURE_TEMP_PROTECT

OAL_STATIC OAL_INLINE oal_void hal_read_max_temperature(oal_int16 *ps_temperature)
{
    HAL_PUBLIC_HOOK_FUNC(_read_max_temperature)(ps_temperature);
}
#endif

#ifdef _PRE_WLAN_PHY_BUGFIX_VHT_SIG_B

OAL_STATIC OAL_INLINE oal_void hal_enable_sigB(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_sigB)( pst_hal_device, uc_enable);
}


OAL_STATIC OAL_INLINE oal_void hal_enable_improve_ce(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_improve_ce)( pst_hal_device, uc_enable);
}
#endif

#ifdef _PRE_WLAN_PHY_BUGFIX_IMPROVE_CE_TH
OAL_STATIC OAL_INLINE oal_void hal_set_acc_symb_num(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_num)
{
    HAL_PUBLIC_HOOK_FUNC(_set_acc_symb_num)( pst_hal_device, ul_num);
}


OAL_STATIC OAL_INLINE oal_void hal_set_improve_ce_threshold(hal_to_dmac_device_stru* pst_hal_device, oal_uint32 ul_val)
{
    HAL_PUBLIC_HOOK_FUNC(_set_improve_ce_threshold)( pst_hal_device, ul_val);
}
#endif

OAL_STATIC OAL_INLINE oal_void hal_get_radar_det_result(hal_to_dmac_device_stru *pst_hal_device, hal_radar_det_event_stru *pst_radar_info)
{
    HAL_PUBLIC_HOOK_FUNC(_get_radar_det_result)( pst_hal_device, pst_radar_info);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_rts_rate_params(hal_to_dmac_device_stru *pst_hal_device, hal_cfg_rts_tx_param_stru *pst_hal_rts_tx_param)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rts_rate_params)( pst_hal_device, pst_hal_rts_tx_param);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_rts_rate_selection_mode(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_rts_rate_select_mode)

{
    HAL_PUBLIC_HOOK_FUNC(_set_rts_rate_selection_mode)( pst_hal_device, uc_rts_rate_select_mode);
}
#ifdef _PRE_WLAN_1103_PILOT

OAL_STATIC  OAL_INLINE oal_void hal_set_txbf_sounding_rate(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_txbf_sounding_rate)
{
    HAL_PUBLIC_HOOK_FUNC(_set_txbf_sounding_rate)(pst_hal_device, uc_txbf_sounding_rate);
}
#endif

OAL_STATIC  OAL_INLINE oal_void hal_set_agc_track_ant_sel(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_agc_track_ant_sel)
{
    HAL_PUBLIC_HOOK_FUNC(_set_agc_track_ant_sel)( pst_hal_device, ul_agc_track_ant_sel);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_agc_track_ant_sel(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_agc_track_ant_sel)
{
    HAL_PUBLIC_HOOK_FUNC(_get_agc_track_ant_sel)( pst_hal_device, pul_agc_track_ant_sel);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_prot_resp_frame_chain(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_chain_val)
{
    HAL_PUBLIC_HOOK_FUNC(_set_prot_resp_frame_chain)( pst_hal_device, uc_chain_val);
}


//OAL_STATIC  OAL_INLINE oal_void hal_set_tpc_params(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_band, oal_uint8 uc_channel_num)
//{
//    HAL_PUBLIC_HOOK_FUNC(_set_bcn_phy_tx_mode, pst_hal_device, uc_band, uc_channel_num);
//}

#ifndef WIN32

OAL_STATIC  OAL_INLINE oal_void  hal_get_rf_temp(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_cur_temp)
{
    HAL_PUBLIC_HOOK_FUNC(_get_rf_temp)( pst_hal_device, puc_cur_temp);
}

#ifdef _PRE_WLAN_PRODUCT_1151V200

OAL_STATIC  OAL_INLINE oal_void  hal_get_rf_temp_tsens(hal_to_dmac_device_stru *pst_hal_device, oal_int16 *ps_cur_temp)
{
    HAL_PUBLIC_HOOK_FUNC(_get_rf_temp_tsens)( pst_hal_device, ps_cur_temp);
}
#endif

#ifdef _PRE_WLAN_REALTIME_CALI

OAL_STATIC  OAL_INLINE oal_void  hal_get_tpc_cali_upc_gain_in_rf_list(oal_int8* pac_upc_gain_in_rf_list)
{
    HAL_PUBLIC_HOOK_FUNC(_get_tpc_cali_upc_gain_in_rf_list)( pac_upc_gain_in_rf_list);
}
#endif


OAL_STATIC  OAL_INLINE oal_void hal_set_bcn_phy_tx_mode(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_pow_code)

{
    HAL_PUBLIC_HOOK_FUNC(_set_bcn_phy_tx_mode)( pst_hal_vap, ul_pow_code);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_spec_frm_rate(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_get_spec_frm_rate)( pst_hal_device);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_spec_frm_phy_tx_mode(hal_to_dmac_device_stru *pst_hal_device,
                    oal_uint8 uc_band, oal_uint8 uc_subband_idx)

{
    HAL_PUBLIC_HOOK_FUNC(_set_spec_frm_phy_tx_mode)( pst_hal_device, uc_band, uc_subband_idx);
}



OAL_STATIC  OAL_INLINE oal_void hal_rf_regctl_enable_set_regs(hal_to_dmac_device_stru *pst_hal_device, wlan_channel_band_enum_uint8 en_freq_band,oal_uint8 uc_cur_ch_num,wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    HAL_PUBLIC_HOOK_FUNC(_rf_regctl_enable_set_regs)(pst_hal_device, en_freq_band, uc_cur_ch_num, en_bandwidth);
}



OAL_STATIC  OAL_INLINE oal_void  hal_get_bcn_rate(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_data_rate)
{
    HAL_PUBLIC_HOOK_FUNC(_get_bcn_rate)( pst_hal_vap,puc_data_rate);
}
#endif


OAL_STATIC  OAL_INLINE oal_uint32 hal_get_subband_index(wlan_channel_band_enum_uint8 en_band, oal_uint8 uc_channel_num, oal_uint8 *puc_subband_idx)
{
    return HAL_PUBLIC_HOOK_FUNC(_get_subband_index)(en_band, uc_channel_num, puc_subband_idx);
}


OAL_STATIC OAL_INLINE oal_void hal_irq_affinity_init(hal_to_dmac_device_stru * pst_device, oal_uint32 ul_core_id)
{
    HAL_PUBLIC_HOOK_FUNC(_irq_affinity_init)( pst_device, ul_core_id);
}



OAL_STATIC OAL_INLINE oal_void hal_rx_set_trlr_report_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_config_val, oal_uint8 uc_trlr_switch)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_set_trlr_report_info)( pst_hal_device, puc_config_val, uc_trlr_switch);
}


OAL_STATIC OAL_INLINE oal_void hal_rf_tone_transmit_entrance(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_data_len, oal_uint8 uc_chain_idx)
{
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST))
    HAL_PUBLIC_HOOK_FUNC(_rf_tone_transmit_entrance)(pst_hal_device, us_data_len, uc_chain_idx);
#endif
}


OAL_STATIC OAL_INLINE oal_void hal_rf_tone_transmit_exit(hal_to_dmac_device_stru *pst_hal_device)
{
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST))
    HAL_PUBLIC_HOOK_FUNC(_rf_tone_transmit_exit)(pst_hal_device);
#endif
}

#ifdef _PRE_WLAN_FEATURE_TXBF
#if (WLAN_MAX_NSS_NUM >= WLAN_DOUBLE_NSS)
#if (_PRE_WLAN_REAL_CHIP == _PRE_WLAN_CHIP_SIM)

OAL_STATIC  OAL_INLINE oal_void hal_set_2g_rf_txdriver(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_set_2g_rf_txdriver)(pst_hal_device);
}
#endif

OAL_STATIC OAL_INLINE oal_void hal_get_fake_vap_id(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_fake_vap_id)
{
    HAL_PUBLIC_HOOK_FUNC(_get_fake_vap_id)(pst_hal_device, puc_fake_vap_id);
}

OAL_STATIC OAL_INLINE oal_void  hal_set_fake_vap(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_vap_id)
{
    HAL_PUBLIC_HOOK_FUNC(_set_fake_vap)(pst_hal_device, uc_vap_id);
}

OAL_STATIC OAL_INLINE oal_void  hal_clr_fake_vap(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_vap_id)
{
    HAL_PUBLIC_HOOK_FUNC(_clr_fake_vap)(pst_hal_device, uc_vap_id);
}
#endif

OAL_STATIC  OAL_INLINE oal_void hal_set_legacy_matrix_buf_pointer(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_matrix)
{
    HAL_PUBLIC_HOOK_FUNC(_set_legacy_matrix_buf_pointer)( pst_hal_device, ul_matrix);
}

OAL_STATIC  OAL_INLINE oal_void hal_get_legacy_matrix_buf_pointer(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_matrix)
{
    HAL_PUBLIC_HOOK_FUNC(_get_legacy_matrix_buf_pointer)( pst_hal_device, pul_matrix);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_dl_mumimo_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_set_dl_mumimo_ctrl)( pst_hal_device, en_enable);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_dl_mumimo_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 *en_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_get_dl_mumimo_ctrl)( pst_hal_device, en_enable);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_sta_membership_status_63_32(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_sta_membership_status_63_32)( pst_hal_device, ul_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_sta_membership_status_31_0(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_sta_membership_status_31_0)( pst_hal_device, ul_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_sta_user_p_63_48(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_sta_user_p_63_48)( pst_hal_device, ul_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_sta_user_p_47_32(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_sta_user_p_47_32)( pst_hal_device, ul_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_sta_user_p_31_16(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_sta_user_p_31_16)( pst_hal_device, ul_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_sta_user_p_15_0(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_sta_user_p_15_0)( pst_hal_device, ul_value);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_vht_report_rate(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_rate)
{
    HAL_PUBLIC_HOOK_FUNC(_set_vht_report_rate)( pst_hal_device, ul_rate);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_vht_report_phy_mode(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_phy_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_vht_report_phy_mode)( pst_hal_device, ul_phy_mode);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_ndp_rate(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_rate)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ndp_rate)( pst_hal_device, ul_rate);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_ndp_phy_mode(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_phy_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ndp_phy_mode)( pst_hal_device, ul_phy_mode);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_ndp_max_time(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_ndp_time)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ndp_max_time)( pst_hal_device, uc_ndp_time);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_ndpa_duration(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_ndpa_duration)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ndpa_duration)( pst_hal_device, ul_ndpa_duration);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_ndp_group_id(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_group_id, oal_uint16 us_partial_id)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ndp_group_id)( pst_hal_device, uc_group_id, us_partial_id);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_phy_legacy_bf_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_phy_legacy_bf_en)( pst_hal_device, ul_reg_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_phy_txbf_legacy_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_phy_txbf_legacy_en)( pst_hal_device, ul_reg_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_phy_pilot_bf_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_phy_pilot_bf_en)( pst_hal_device, ul_reg_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_ht_buffer_num(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ht_buffer_num)( pst_hal_device, uc_reg_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_ht_buffer_step(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ht_buffer_step)( pst_hal_device, us_reg_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_ht_buffer_pointer(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ht_buffer_pointer)( pst_hal_device, ul_reg_value);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_h_matrix_timeout(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_h_matrix_timeout)( pst_hal_device, ul_reg_value);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_mu_aid_matrix_info(hal_to_dmac_device_stru *pst_hal_device, hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_aid)
{
    HAL_PUBLIC_HOOK_FUNC(_set_mu_aid_matrix_info)( pst_hal_device, pst_hal_vap, us_aid);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_txbf_vht_buff_addr(hal_to_dmac_device_stru *pst_hal_device, hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_addr, oal_uint16 us_buffer_len)
{
    HAL_PUBLIC_HOOK_FUNC(_set_txbf_vht_buff_addr)( pst_hal_device, pst_hal_vap, ul_addr, us_buffer_len);
}

#ifdef _PRE_WLAN_FEATURE_11AX

OAL_STATIC  OAL_INLINE oal_void hal_set_txbf_he_buff_addr(hal_to_dmac_device_stru *pst_hal_device, hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_addr, oal_uint16 us_buffer_len)
{
    HAL_PUBLIC_HOOK_FUNC(_set_txbf_he_buff_addr)( pst_hal_device, pst_hal_vap, ul_addr, us_buffer_len);
}
#endif


OAL_STATIC  OAL_INLINE oal_void hal_delete_txbf_lut_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index)
{
    HAL_PUBLIC_HOOK_FUNC(_delete_txbf_lut_info)( pst_hal_device, uc_lut_index);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_txbf_lut_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index, oal_uint32 ul_lut_info)
{
    HAL_PUBLIC_HOOK_FUNC(_set_txbf_lut_info)( pst_hal_device, uc_lut_index, ul_lut_info);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_txbf_lut_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index, oal_uint32*  pst_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_get_txbf_lut_info)( pst_hal_device, uc_lut_index, pst_reg_value);
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

OAL_STATIC  OAL_INLINE oal_void hal_set_bfee_bypass_clk_gating(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_set_bfee_bypass_clk_gating)( pst_hal_device, uc_enable);
}

OAL_STATIC  OAL_INLINE oal_void hal_rf_second_agc_ctrl(hal_to_dmac_device_stru *pst_hal_device, wlan_channel_band_enum_uint8 en_band)
{
#ifdef _PRE_WLAN_1103_PILOT
    HAL_PUBLIC_HOOK_FUNC(_rf_second_agc_ctrl)(pst_hal_device, en_band);
#endif
}


#endif


OAL_STATIC  OAL_INLINE oal_void hal_set_bfee_sounding_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_txbf_protocol_type, oal_uint8 uc_bfee_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_set_bfee_sounding_en)( pst_hal_device, uc_txbf_protocol_type, uc_bfee_enable);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_bfee_h2v_beamforming_ng(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 en_user_bw)
{
    HAL_PUBLIC_HOOK_FUNC(_set_bfee_h2v_beamforming_ng)( pst_hal_device, en_user_bw);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_bfee_grouping_codebook(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_user_index, oal_uint8 uc_min_group, oal_uint8 uc_txbf_protocol_type,oal_uint8 en_user_bw)
{
    HAL_PUBLIC_HOOK_FUNC(_set_bfee_grouping_codebook)( pst_hal_device, uc_user_index, uc_min_group, uc_txbf_protocol_type,en_user_bw);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_bfer_subcarrier_ng(hal_to_dmac_device_stru *pst_hal_device, wlan_bw_cap_enum_uint8 en_user_bw)
{
    HAL_PUBLIC_HOOK_FUNC(_set_bfer_subcarrier_ng)( pst_hal_device, en_user_bw);
}


#endif  /* _PRE_WLAN_FEATURE_TXBF */

OAL_STATIC  OAL_INLINE oal_void hal_enable_smart_antenna_gpio_set_default_antenna(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_smart_antenna_gpio_set_default_antenna)( pst_hal_device, ul_reg_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_delete_smart_antenna_value(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index)
{
    HAL_PUBLIC_HOOK_FUNC(_delete_smart_antenna_value)( pst_hal_device, uc_lut_index);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_smart_antenna_value(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index, oal_uint16 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_smart_antenna_value)( pst_hal_device, uc_lut_index, ul_reg_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_get_smart_antenna_value(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index, oal_uint32*  pst_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_get_smart_antenna_value)( pst_hal_device, uc_lut_index, pst_reg_value);
}



#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF

OAL_STATIC  OAL_INLINE oal_void hal_set_weak_intf_rssi_th(hal_to_dmac_device_stru *pst_hal_device, oal_int32 l_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_set_weak_intf_rssi_th)( pst_hal_device, l_reg_val);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_agc_unlock_min_th(hal_to_dmac_device_stru *pst_hal_device, oal_int32 l_tx_reg_val, oal_int32 l_rx_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_set_agc_unlock_min_th)( pst_hal_device, l_tx_reg_val, l_rx_reg_val);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_nav_max_duration(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_bss_dur, oal_uint16 us_obss_dur)
{
    HAL_PUBLIC_HOOK_FUNC(_set_nav_max_duration)( pst_hal_device, us_bss_dur, us_obss_dur);
}
#endif


OAL_STATIC  OAL_INLINE oal_void hal_report_gm_val(hal_to_dmac_device_stru *pst_hal_device)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_report_gm_val)(pst_hal_device);
#endif
}


OAL_STATIC  OAL_INLINE oal_void hal_set_extlna_bypass_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_switch)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_extlna_bypass_en)(pst_hal_device, uc_switch);
#endif
}


OAL_STATIC  OAL_INLINE oal_void hal_set_extlna_chg_cfg(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_extlna_chg)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_extlna_chg_cfg)(pst_hal_device, en_extlna_chg);
#endif
}

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT

OAL_STATIC  OAL_INLINE oal_void hal_set_counter1_clear(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_set_counter1_clear)( pst_hal_device);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_txrx_frame_time(hal_to_dmac_device_stru *pst_hal_device, hal_ch_mac_statics_stru *pst_ch_statics)
{
    HAL_PUBLIC_HOOK_FUNC(_get_txrx_frame_time)( pst_hal_device, pst_ch_statics);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_mac_clken(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_wctrl_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_set_mac_clken)( pst_hal_device, en_wctrl_enable);
}


#endif


OAL_STATIC  OAL_INLINE oal_void hal_get_mac_statistics_data(hal_to_dmac_device_stru *pst_hal_device, hal_mac_key_statis_info_stru *pst_mac_key_statis)
{
    HAL_PUBLIC_HOOK_FUNC(_get_mac_statistics_data)( pst_hal_device, pst_mac_key_statis);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_ddc_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_ddc_en)(pst_hal_device, ul_reg_value);
#endif
}
OAL_STATIC  OAL_INLINE oal_void hal_set_noise_est_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
        HAL_PUBLIC_HOOK_FUNC(_set_noise_est_en)(pst_hal_device, ul_reg_value);
#endif
}
OAL_STATIC  OAL_INLINE oal_void hal_set_noise_comb_close(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
        HAL_PUBLIC_HOOK_FUNC(_set_noise_comb_close)(pst_hal_device, ul_reg_value);
#endif
}

OAL_STATIC  OAL_INLINE oal_void hal_set_zf_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
        HAL_PUBLIC_HOOK_FUNC(_set_zf_en)(pst_hal_device, ul_reg_value);
#endif

}




OAL_STATIC  OAL_INLINE oal_void hal_set_dyn_bypass_extlna_pm_flag(hal_to_dmac_device_stru *pst_hal_device, wlan_channel_band_enum_uint8 en_band, oal_bool_enum_uint8 en_value)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_dyn_bypass_extlna_pm_flag)(pst_hal_device, en_band, en_value);
#endif
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hal_get_dyn_bypass_extlna_pm_flag(hal_to_dmac_device_stru *pst_hal_device)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    return HAL_PUBLIC_HOOK_FUNC(_get_dyn_bypass_extlna_pm_flag)(pst_hal_device);
#else
    return OAL_FALSE;
#endif
}

#ifdef _PRE_WLAN_FEATURE_CCA_OPT

OAL_STATIC  OAL_INLINE oal_void hal_set_ed_high_th(hal_to_dmac_device_stru *pst_hal_device, hal_alg_cca_th_stru *pst_cca_th_opt)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ed_high_th)( pst_hal_device, pst_cca_th_opt);
}



OAL_STATIC  OAL_INLINE oal_void hal_set_cca_prot_th(hal_to_dmac_device_stru *pst_hal_device, hal_alg_cca_th_stru *pst_cca_th_opt)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_cca_prot_th)(pst_hal_device, pst_cca_th_opt);
#endif
}


OAL_STATIC  OAL_INLINE oal_void hal_set_cca_th_default(hal_to_dmac_device_stru *pst_hal_device, wlan_channel_band_enum_uint8 en_band)
{
    HAL_PUBLIC_HOOK_FUNC(_set_cca_th_default)(pst_hal_device, en_band);
}



OAL_STATIC  OAL_INLINE oal_void hal_enable_sync_error_counter(hal_to_dmac_device_stru *pst_hal_device, oal_int32 l_enable_cnt_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_sync_error_counter)( pst_hal_device, l_enable_cnt_reg_val);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_sync_error_cnt(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *ul_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_sync_error_cnt)( pst_hal_device, ul_reg_val);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_sync_err_counter_clear(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_set_sync_err_counter_clear)( pst_hal_device);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_cca_reg_th(hal_to_dmac_device_stru *pst_hal_device, wlan_channel_band_enum_uint8 uc_band, oal_int8 *ac_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_cca_reg_th)( pst_hal_device, uc_band, ac_reg_val);
}

#endif

#ifdef _PRE_WLAN_FEATURE_MWO_DET

OAL_STATIC  OAL_INLINE oal_void hal_set_mac_anti_intf_period(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_set_mac_anti_intf_period)( pst_hal_device, ul_reg_val);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_mac_anti_intf_period(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_mac_anti_intf_period)( pst_hal_device, pul_reg_val);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_mac_anti_intf_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_set_mac_anti_intf_ctrl)( pst_hal_device, ul_reg_val);
}


OAL_STATIC OAL_INLINE oal_void hal_set_phy_mwo_det_rssithr(hal_to_dmac_device_stru *pst_hal_device,
                                                                     oal_int8                 c_start_rssi,
                                                                     oal_int8                 c_end_rssi,
                                                                     oal_bool_enum_uint8      en_enable_mwo,
                                                                     oal_uint8                uc_cfg_power_sel)
{
    HAL_PUBLIC_HOOK_FUNC(_set_phy_mwo_det_rssithr)( pst_hal_device, c_start_rssi, c_end_rssi, en_enable_mwo, uc_cfg_power_sel);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_phy_mwo_det_rssithr(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_phy_mwo_det_rssithr)( pst_hal_device, pul_reg_val);
}


OAL_STATIC  OAL_INLINE oal_void hal_restore_phy_mwo_det_rssithr(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_restore_phy_mwo_det_rssithr)( pst_hal_device, ul_reg_val);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_phy_mwo_det_timethr(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_set_phy_mwo_det_timethr)( pst_hal_device, ul_reg_val);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_phy_mwo_det_timethr(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_phy_mwo_det_timethr)( pst_hal_device, pul_reg_val);
}

#endif  /* _PRE_WLAN_FEATURE_MWO_DET */


OAL_STATIC  OAL_INLINE oal_void hal_set_80m_resp_mode(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_debug_en)
{
    HAL_PUBLIC_HOOK_FUNC(_set_80m_resp_mode)(pst_hal_device, uc_debug_en);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_soc_lpm(hal_to_dmac_device_stru *pst_hal_device,hal_lpm_soc_set_enum_uint8 en_type ,oal_uint8 uc_on_off,oal_uint8 uc_pcie_idle)
{
    HAL_PUBLIC_HOOK_FUNC(_set_soc_lpm)( pst_hal_device,en_type, uc_on_off,uc_pcie_idle);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_psm_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_on_off)
{
    HAL_PUBLIC_HOOK_FUNC(_set_psm_status)( pst_hal_device, uc_on_off);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_psm_dtim_period(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_dtim_period,
                                                oal_uint8 uc_listen_intvl_to_dtim_times, oal_bool_enum_uint8 en_receive_dtim)
{
    HAL_PUBLIC_HOOK_FUNC(_set_psm_dtim_period)( pst_hal_vap, uc_dtim_period, uc_listen_intvl_to_dtim_times, en_receive_dtim);
}



OAL_STATIC  OAL_INLINE oal_void hal_set_psm_wakeup_mode(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_psm_wakeup_mode)( pst_hal_device, uc_mode);
}

#if defined(_PRE_WLAN_FEATURE_SMPS) || defined(_PRE_WLAN_CHIP_TEST)

OAL_STATIC  OAL_INLINE oal_void hal_set_smps_mode(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_smps_mode)( pst_hal_device, uc_mode);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_smps_mode(hal_to_dmac_device_stru *pst_hal_device, oal_uint32* pst_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_get_smps_mode)( pst_hal_device, pst_reg_value);
}

#endif

#if defined(_PRE_WLAN_FEATURE_TXOPPS) || defined(_PRE_WLAN_CHIP_TEST)

OAL_STATIC  OAL_INLINE oal_void hal_set_txop_ps_enable(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_on_off)
{
    HAL_PUBLIC_HOOK_FUNC(_set_txop_ps_enable)( pst_hal_device, uc_on_off);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_txop_ps_condition1(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_on_off)
{
    HAL_PUBLIC_HOOK_FUNC(_set_txop_ps_condition1)( pst_hal_device, uc_on_off);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_txop_ps_condition2(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_on_off)
{
    HAL_PUBLIC_HOOK_FUNC(_set_txop_ps_condition2)( pst_hal_device, uc_on_off);
}


OAL_STATIC  OAL_INLINE oal_void  hal_set_txop_ps_partial_aid(hal_to_dmac_vap_stru  *pst_hal_vap, oal_uint32 ul_partial_aid)
{
    HAL_PUBLIC_HOOK_FUNC(_set_txop_ps_partial_aid)( pst_hal_vap, ul_partial_aid);
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)

OAL_STATIC  OAL_INLINE oal_void  hal_get_txop_ps_partial_aid(hal_to_dmac_vap_stru  *pst_hal_vap, oal_uint32 *pul_partial_aid)
{
    HAL_PUBLIC_HOOK_FUNC(_get_txop_ps_partial_aid)( pst_hal_vap, pul_partial_aid);
}
#endif

#endif

#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM

OAL_STATIC  OAL_INLINE oal_void  hal_mac_set_bcn_tim_pos(hal_to_dmac_vap_stru  *pst_hal_vap, oal_uint16 us_pos)
{
    HAL_PUBLIC_HOOK_FUNC(_mac_set_bcn_tim_pos)( pst_hal_vap, us_pos);
}
#endif

#if defined(_PRE_WLAN_FEATURE_MAC_PARSE_TIM) || defined(_PRE_WLAN_FEATURE_DBDC)

OAL_STATIC  OAL_INLINE oal_void  hal_set_mac_aid(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_aid)
{
    HAL_PUBLIC_HOOK_FUNC(_set_mac_aid)( pst_hal_vap, us_aid);
}
#endif


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  hal_check_mac_int_status(hal_to_dmac_device_stru *pst_hal_device)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    return HAL_PUBLIC_HOOK_FUNC(_check_mac_int_status)(pst_hal_device);
#else
    return OAL_FALSE;
#endif

}


OAL_STATIC  OAL_INLINE oal_void hal_set_wow_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_set_bitmap,hal_wow_param_stru* pst_para)
{
    HAL_PUBLIC_HOOK_FUNC(_set_wow_en)( pst_hal_device, ul_set_bitmap, pst_para);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_lpm_state(hal_to_dmac_device_stru *pst_hal_device,hal_lpm_state_enum_uint8 uc_state_from, hal_lpm_state_enum_uint8 uc_state_to,oal_void* pst_para, oal_void* pst_wow_para)
{
    HAL_PUBLIC_HOOK_FUNC(_set_lpm_state)( pst_hal_device,uc_state_from,uc_state_to, pst_para, pst_wow_para);
}


OAL_STATIC OAL_INLINE oal_void  hal_disable_machw_edca(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_disable_machw_edca)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_enable_machw_edca(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_machw_edca)( pst_hal_device);
}



OAL_STATIC  OAL_INLINE oal_void hal_set_tx_abort_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_abort_en)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tx_abort_en)( pst_hal_device, uc_abort_en);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_coex_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_mac_ctrl, oal_uint32 ul_rf_ctrl)
{
    HAL_PUBLIC_HOOK_FUNC(_set_coex_ctrl)( pst_hal_device, ul_mac_ctrl, ul_rf_ctrl);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_hw_version(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_hw_vsn, oal_uint32 *pul_hw_vsn_data,oal_uint32 *pul_hw_vsn_num)
{
    HAL_PUBLIC_HOOK_FUNC(_get_hw_version)( pst_hal_device, pul_hw_vsn, pul_hw_vsn_data, pul_hw_vsn_num);
}

#ifdef _PRE_DEBUG_MODE

OAL_STATIC  OAL_INLINE oal_void hal_get_all_reg_value(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_get_all_reg_value)( pst_hal_device);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_cali_data(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_get_cali_data)( pst_hal_device);
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC  OAL_INLINE oal_void hal_get_cali_data_work(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_get_cali_data_work)( pst_hal_device);
}
#endif

#endif


OAL_STATIC  OAL_INLINE oal_void hal_set_tx_dscr_field(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_data, hal_rf_test_sect_enum_uint8 en_sect)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tx_dscr_field)( pst_hal_device, ul_data, en_sect);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_tx_dscr_field(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr)
{
    HAL_PUBLIC_HOOK_FUNC(_get_tx_dscr_field)(pst_hal_device, pst_tx_dscr);
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)

OAL_STATIC  OAL_INLINE oal_void hal_set_m2s_tx_dscr_field(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, hal_tx_txop_alg_stru *pst_txop_alg)
{
    HAL_PUBLIC_HOOK_FUNC(_set_m2s_tx_dscr_field)(pst_hal_device, pst_tx_dscr, pst_txop_alg);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_phy_max_bw_field(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_data, hal_phy_max_bw_sect_enmu_uint8 en_sect)
{
    HAL_PUBLIC_HOOK_FUNC(_set_phy_max_bw_field)(pst_hal_device, ul_data, en_sect);
}
#endif


OAL_STATIC  OAL_INLINE oal_void hal_rf_test_disable_al_tx(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_rf_test_disable_al_tx)( pst_hal_device);
}

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX


OAL_STATIC  OAL_INLINE oal_void hal_enable_tx_comp(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_tx_comp)( pst_hal_device);
}
#endif


OAL_STATIC  OAL_INLINE oal_void hal_rf_test_enable_al_tx(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru * pst_tx_dscr)
{
    HAL_PUBLIC_HOOK_FUNC(_rf_test_enable_al_tx)( pst_hal_device, pst_tx_dscr);
}

#ifdef _PRE_WLAN_PHY_PLL_DIV


OAL_STATIC  OAL_INLINE oal_void hal_rf_set_freq_skew(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_idx, oal_uint16 us_chn, oal_int16 as_corr_data[])
{
    HAL_PUBLIC_HOOK_FUNC(_rf_set_freq_skew)( us_idx, us_chn, as_corr_data);
}
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

OAL_STATIC  OAL_INLINE oal_void hal_rf_adjust_ppm(hal_to_dmac_device_stru *pst_hal_device, oal_int8 c_ppm, wlan_channel_bandwidth_enum_uint8  en_bandwidth, oal_uint8 uc_clock)
{
    HAL_PUBLIC_HOOK_FUNC(_rf_adjust_ppm)( pst_hal_device, c_ppm, en_bandwidth, uc_clock);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_pcie_pm_level(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_pcie_pm_level)
{
    HAL_PUBLIC_HOOK_FUNC(_set_pcie_pm_level)( pst_hal_device, uc_pcie_pm_level);
}

#endif

OAL_STATIC  OAL_INLINE oal_void hal_rf_get_pll_div_idx(wlan_channel_band_enum_uint8        en_band,
                                               oal_uint8                           uc_channel_idx,
                                               wlan_channel_bandwidth_enum_uint8   en_bandwidth,
                                               oal_uint8                           *puc_pll_div_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_rf_get_pll_div_idx)( en_band, uc_channel_idx, en_bandwidth, puc_pll_div_idx);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_daq_mac_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint32* pul_addr, oal_uint16 us_unit_len, oal_uint16 us_unit_num, oal_uint16 us_depth)
{
    HAL_PUBLIC_HOOK_FUNC(_set_daq_mac_reg)( pst_hal_device, pul_addr, us_unit_len, us_unit_num, us_depth);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_daq_phy_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_daq_phy_reg)( pst_hal_device, ul_reg_value);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_daq_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_daq_en)( pst_hal_device, uc_reg_value);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_daq_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_get_daq_status)( pst_hal_device, pul_reg_value);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_beacon_timeout_val(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_beacon_timeout_val)( pst_hal_device, us_value);
}


OAL_STATIC  OAL_INLINE oal_void hal_psm_clear_mac_rx_isr(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_psm_clear_mac_rx_isr)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void hal_set_dac_lpf_gain(hal_to_dmac_device_stru *pst_hal_device,
                                                 oal_uint8 en_band,
                                                 oal_uint8 en_bandwidth,
                                                 oal_uint8 uc_chan_number,
                                                 oal_uint8 en_protocol_mode,
                                                 oal_uint8 en_rate)
{
    HAL_PUBLIC_HOOK_FUNC(_set_dac_lpf_gain)( pst_hal_device, en_band, en_bandwidth, uc_chan_number, en_protocol_mode, en_rate);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_rx_filter(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_rx_filter_val)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rx_filter)( pst_hal_device, ul_rx_filter_val);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_rx_filter_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_rx_filter_command)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rx_filter_reg)( pst_hal_device, ul_rx_filter_command);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_rx_filter(hal_to_dmac_device_stru *pst_hal_device, oal_uint32* pst_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_get_rx_filter)( pst_hal_device, pst_reg_value);
}

#define HAL_VAP_LEVEL_FUNC

OAL_STATIC OAL_INLINE oal_void  hal_vap_tsf_get_32bit(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 *pul_tsf_lo)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_tsf_get_32bit)( pst_hal_vap, pul_tsf_lo);
}
OAL_STATIC OAL_INLINE oal_void  hal_vap_tsf_set_32bit(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_tsf_lo)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_tsf_set_32bit)( pst_hal_vap, ul_tsf_lo);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_tsf_get_64bit(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 *pul_tsf_hi, oal_uint32 *pul_tsf_lo)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_tsf_get_64bit)( pst_hal_vap, pul_tsf_hi, pul_tsf_lo);
}
OAL_STATIC OAL_INLINE oal_void  hal_vap_tsf_set_64bit(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_tsf_hi, oal_uint32 ul_tsf_lo)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_tsf_set_64bit)( pst_hal_vap, ul_tsf_hi, ul_tsf_lo);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_send_beacon_pkt(hal_to_dmac_vap_stru        *pst_hal_vap,
                                                        hal_beacon_tx_params_stru   *pst_params)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_send_beacon_pkt)( pst_hal_vap, pst_params);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_beacon_rate(hal_to_dmac_vap_stru        *pst_hal_vap,
                                                                 oal_uint32 ul_beacon_rate)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_beacon_rate)( pst_hal_vap, ul_beacon_rate);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_beacon_suspend(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_beacon_suspend)( pst_hal_vap);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_beacon_resume(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_beacon_resume)( pst_hal_vap);
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

OAL_STATIC OAL_INLINE oal_void  hal_get_beacon_miss_status(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_get_beacon_miss_status)( pst_hal_device);
}
#endif


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_prot_params(hal_to_dmac_vap_stru *pst_hal_vap, hal_tx_txop_rate_params_stru *pst_phy_tx_mode, hal_tx_txop_per_rate_params_union *pst_data_rate)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_prot_params)( pst_hal_vap, pst_phy_tx_mode, pst_data_rate);
}



/*****************************************************************************
  10.2 ��Ӧһ��Ӳ��MAC VAP�ľ�̬��������
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_void hal_vap_set_macaddr(hal_to_dmac_vap_stru * pst_hal_vap, oal_uint8 *puc_mac_addr)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_macaddr)( pst_hal_vap, puc_mac_addr);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_opmode(hal_to_dmac_vap_stru *pst_hal_vap, wlan_vap_mode_enum_uint8 en_vap_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_opmode)( pst_hal_vap, en_vap_mode);
}



OAL_STATIC OAL_INLINE oal_void hal_vap_clr_opmode(hal_to_dmac_vap_stru *pst_hal_vap, wlan_vap_mode_enum_uint8 en_vap_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_clr_opmode)( pst_hal_vap, en_vap_mode);
}


/*****************************************************************************
  hal vap EDCA����������ؽӿ�
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_aifsn_all_ac(
                hal_to_dmac_vap_stru   *pst_hal_vap,
                oal_uint8               uc_bk,
                oal_uint8               uc_be,
                oal_uint8               uc_vi,
                oal_uint8               uc_vo)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_aifsn_all_ac)( pst_hal_vap, uc_bk, uc_be, uc_vi, uc_vo);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_aifsn_ac(hal_to_dmac_vap_stru         *pst_hal_vap,
                                                           wlan_wme_ac_type_enum_uint8   en_ac,
                                                           oal_uint8                     uc_aifs)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_aifsn_ac)( pst_hal_vap, en_ac, uc_aifs);
}

#if (_PRE_MULTI_CORE_MODE==_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC)
OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_aifsn_ac_wfa(hal_to_dmac_vap_stru         *pst_hal_vap,
                                                           wlan_wme_ac_type_enum_uint8   en_ac,
                                                           oal_uint8                     uc_aifs,
                                                           wlan_wme_ac_type_enum_uint8   en_wfa_lock)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_aifsn_ac_wfa)( pst_hal_vap, en_ac, uc_aifs, en_wfa_lock);
}

OAL_STATIC OAL_INLINE oal_void  hal_vap_set_edca_machw_cw_wfa(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_cwmaxmin, oal_uint8 uc_ec_type, wlan_wme_ac_type_enum_uint8   en_wfa_lock)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_edca_machw_cw_wfa)( pst_hal_vap, uc_cwmaxmin, uc_ec_type, en_wfa_lock);
}
#endif


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_edca_machw_cw(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_cwmax, oal_uint8 uc_cwmin, oal_uint8 uc_ec_type)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_edca_machw_cw)( pst_hal_vap, uc_cwmax, uc_cwmin, uc_ec_type);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_get_edca_machw_cw(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_cwmax, oal_uint8 *puc_cwmin, oal_uint8 uc_ec_type)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_get_edca_machw_cw)( pst_hal_vap, puc_cwmax, puc_cwmin, uc_ec_type);
}

#if 0

OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_cw_bk(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_cwmax, oal_uint8 uc_cwmin)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_cw_bk)( pst_hal_vap, uc_cwmax, uc_cwmin);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_get_machw_cw_bk(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_cwmax, oal_uint8 *puc_cwmin)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_get_machw_cw_bk)( pst_hal_vap, puc_cwmax, puc_cwmin);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_cw_be(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_cwmax, oal_uint8 uc_cwmin)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_cw_be)( pst_hal_vap, uc_cwmax, uc_cwmin);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_get_machw_cw_be(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_cwmax, oal_uint8 *puc_cwmin)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_get_machw_cw_be)( pst_hal_vap, puc_cwmax, puc_cwmin);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_cw_vi(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_cwmax, oal_uint8 uc_cwmin)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_cw_vi)( pst_hal_vap, uc_cwmax, uc_cwmin);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_get_machw_cw_vi(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_cwmax, oal_uint8 *puc_cwmin)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_get_machw_cw_vi)( pst_hal_vap, puc_cwmax, puc_cwmin);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_cw_vo(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_cwmax, oal_uint8 uc_cwmin)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_cw_vo)( pst_hal_vap, uc_cwmax, uc_cwmin);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_get_machw_cw_vo(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_cwmax, oal_uint8 *puc_cwmin)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_get_machw_cw_vo)( pst_hal_vap, puc_cwmax, puc_cwmin);
}
#endif


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_txop_limit_bkbe(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_be, oal_uint16 us_bk)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_txop_limit_bkbe)( pst_hal_vap, us_be, us_bk);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_get_machw_txop_limit_bkbe(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 *pus_be, oal_uint16 *pus_bk)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_get_machw_txop_limit_bkbe)( pst_hal_vap, pus_be, pus_bk);
}

OAL_STATIC OAL_INLINE oal_void hal_set_txop_check_cca(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 en_txop_check_cca)
{
#ifdef _PRE_WLAN_1103_PILOT
    HAL_PUBLIC_HOOK_FUNC(_set_txop_check_cca)( pst_hal_vap,en_txop_check_cca);
#endif
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_txop_limit_vivo(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_vo, oal_uint16 us_vi)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_txop_limit_vivo)( pst_hal_vap, us_vo, us_vi);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_get_machw_txop_limit_vivo(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 *pus_vo, oal_uint16 *pus_vi)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_get_machw_txop_limit_vivo)( pst_hal_vap, pus_vo, pus_vi);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_edca_bkbe_lifetime(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_be, oal_uint16 us_bk)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_edca_bkbe_lifetime)( pst_hal_vap, us_be, us_bk);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_get_machw_edca_bkbe_lifetime(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 *pus_be, oal_uint16 *pus_bk)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_get_machw_edca_bkbe_lifetime)( pst_hal_vap, pus_be, pus_bk);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_edca_vivo_lifetime(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_vo, oal_uint16 us_vi)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_edca_vivo_lifetime)( pst_hal_vap, us_vo, us_vi);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_get_machw_edca_vivo_lifetime(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 *pus_vo, oal_uint16 *pus_vi)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_get_machw_edca_vivo_lifetime)( pst_hal_vap, pus_vo, pus_vi);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_prng_seed_val_all_ac(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_prng_seed_val_all_ac)( pst_hal_vap);
}


/*****************************************************************************
  hal vap TSF����������ؽӿ�
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  hal_vap_read_tbtt_timer(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 *pul_value)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_read_tbtt_timer)( pst_hal_vap, pul_value);
}

OAL_STATIC OAL_INLINE oal_void  hal_vap_write_tbtt_timer(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_value)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_write_tbtt_timer)( pst_hal_vap, ul_value);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_beacon_period(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_beacon_period)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_beacon_period)( pst_hal_vap, us_beacon_period);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_update_beacon_period(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_beacon_period)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_update_beacon_period)( pst_hal_vap, us_beacon_period);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_psm_listen_interval(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_interval)
{
    HAL_PUBLIC_HOOK_FUNC(_set_psm_listen_interval)( pst_hal_vap, us_interval);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_psm_listen_interval_count(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_interval_count)
{
    HAL_PUBLIC_HOOK_FUNC(_set_psm_listen_interval_count)( pst_hal_vap, us_interval_count);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_psm_tbtt_offset(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_offset)
{
    HAL_PUBLIC_HOOK_FUNC(_set_psm_tbtt_offset)( pst_hal_vap, us_offset);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_psm_ext_tbtt_offset(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_offset)
{
    HAL_PUBLIC_HOOK_FUNC(_set_psm_ext_tbtt_offset)( pst_hal_vap, us_offset);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_psm_beacon_period(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_beacon_period)
{
    HAL_PUBLIC_HOOK_FUNC(_set_psm_beacon_period)( pst_hal_vap, ul_beacon_period);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_get_beacon_period(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 *pul_beacon_period)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_get_beacon_period)( pst_hal_vap, pul_beacon_period);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_noa(
                hal_to_dmac_vap_stru   *pst_hal_vap,
                oal_uint32              ul_start_tsf,
                oal_uint32              ul_duration,
                oal_uint32              ul_interval,
                oal_uint8               uc_count)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_noa)( pst_hal_vap, ul_start_tsf, ul_duration, ul_interval, uc_count);
}
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)

OAL_STATIC OAL_INLINE oal_void  hal_vap_set_noa_timeout_val(hal_to_dmac_vap_stru   *pst_hal_vap, oal_uint16 us_value)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_noa_timeout_val)( pst_hal_vap,us_value);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_noa_offset(hal_to_dmac_vap_stru   *pst_hal_vap, oal_uint16 us_offset)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_noa_offset)( pst_hal_vap,us_offset);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_ext_noa_para(hal_to_dmac_vap_stru   *pst_hal_vap,
                                            oal_uint32       ul_duration,
                                            oal_uint32       ul_interval)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_ext_noa_para)( pst_hal_vap, ul_duration , ul_interval);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_ext_noa_disable(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_ext_noa_disable)( pst_hal_vap);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_ext_noa_enable(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_ext_noa_enable)( pst_hal_vap);
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  hal_vap_is_ext_noa_enable(hal_to_dmac_vap_stru *pst_hal_vap)
{
    return HAL_PUBLIC_HOOK_FUNC(_vap_is_ext_noa_enable)( pst_hal_vap);
}


OAL_STATIC  OAL_INLINE oal_void hal_vap_set_ext_noa_offset(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_offset)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_ext_noa_offset)( pst_hal_vap, us_offset);
}
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)

OAL_STATIC OAL_INLINE oal_void hal_sta_tsf_save(hal_to_dmac_vap_stru *pst_hal_vap, oal_bool_enum_uint8 en_need_restore)
{
    HAL_PUBLIC_HOOK_FUNC(_sta_tsf_save)( pst_hal_vap, en_need_restore);
}


OAL_STATIC OAL_INLINE oal_void hal_sta_tsf_restore(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_sta_tsf_restore)( pst_hal_vap);
}
#endif

#ifdef _PRE_WLAN_FEATURE_P2P

OAL_STATIC OAL_INLINE oal_void  hal_vap_set_ops(
                hal_to_dmac_vap_stru   *pst_hal_vap,
                oal_uint8               en_ops_ctrl,
                oal_uint8               uc_ct_window)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_ops)( pst_hal_vap, en_ops_ctrl, uc_ct_window);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_enable_p2p_absent_suspend(
                hal_to_dmac_vap_stru   *pst_hal_vap,
                oal_bool_enum_uint8     en_suspend_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_enable_p2p_absent_suspend)( pst_hal_vap, en_suspend_enable);
}

#endif
/* beacon hal��ؽӿ� ԭ������ */
OAL_STATIC OAL_INLINE oal_void hal_tx_complete_update_rate(hal_tx_dscr_ctrl_one_param *pst_tx_dscr_param)
{
    oal_uint8       uc_retry;

    uc_retry = pst_tx_dscr_param->uc_long_retry + pst_tx_dscr_param->uc_short_retry;

    if (uc_retry <= pst_tx_dscr_param->ast_per_rate[0].rate_bit_stru.bit_tx_count) /* ����ʹ�õ�1������ */
    {
       /* ΢��¯�㷨������ʵ�ʵ�Ӳ���������ʵȼ�Ϊrate3�����ܴ������Ϊ2���������޸�Ϊrate3 */
#ifdef _PRE_WLAN_FEATURE_MWO_DET
        if((3 == pst_tx_dscr_param->uc_last_rate_rank) && (uc_retry <= pst_tx_dscr_param->ast_per_rate[3].rate_bit_stru.bit_tx_count))
        {
            pst_tx_dscr_param->uc_last_rate_rank = 3;
        }
        else
        {
            pst_tx_dscr_param->uc_last_rate_rank = 0;
        }
#else
        pst_tx_dscr_param->uc_last_rate_rank = 0;
#endif

    }
    else
    {
        uc_retry -= pst_tx_dscr_param->ast_per_rate[0].rate_bit_stru.bit_tx_count;
        if(uc_retry <= pst_tx_dscr_param->ast_per_rate[1].rate_bit_stru.bit_tx_count) /* ����ʹ�õ�2������ */
        {
            pst_tx_dscr_param->uc_last_rate_rank = 1;
        }
        else
        {
            uc_retry -= pst_tx_dscr_param->ast_per_rate[1].rate_bit_stru.bit_tx_count;
            if(uc_retry <= pst_tx_dscr_param->ast_per_rate[2].rate_bit_stru.bit_tx_count)
            {
                pst_tx_dscr_param->uc_last_rate_rank = 2;
            }
            else
            {
                uc_retry -= pst_tx_dscr_param->ast_per_rate[2].rate_bit_stru.bit_tx_count;
                pst_tx_dscr_param->uc_last_rate_rank = 3;
            }
        }
    }

    
    if (uc_retry < pst_tx_dscr_param->ast_per_rate[pst_tx_dscr_param->uc_last_rate_rank].rate_bit_stru.bit_tx_count)
    {
        pst_tx_dscr_param->ast_per_rate[pst_tx_dscr_param->uc_last_rate_rank].rate_bit_stru.bit_tx_count = uc_retry;
    }

    if (OAL_UNLIKELY((0 != pst_tx_dscr_param->uc_long_retry + pst_tx_dscr_param->uc_short_retry) && (uc_retry == 0)))
    {
        OAM_ERROR_LOG4(0, OAM_SF_TX, "{hal_tx_complete_update_rate::invalid tx count, long=%d short=%d last=%d retry=%d}\r\n",
                pst_tx_dscr_param->uc_long_retry, pst_tx_dscr_param->uc_short_retry, pst_tx_dscr_param->uc_last_rate_rank, uc_retry);
        OAM_ERROR_LOG4(0, OAM_SF_TX, "{hal_tx_complete_update_rate::invalid tx count, tx0=%d tx1=%d tx2=%d tx3=%d}\r\n",
                pst_tx_dscr_param->ast_per_rate[0].rate_bit_stru.bit_tx_count, pst_tx_dscr_param->ast_per_rate[1].rate_bit_stru.bit_tx_count,
                pst_tx_dscr_param->ast_per_rate[2].rate_bit_stru.bit_tx_count, pst_tx_dscr_param->ast_per_rate[3].rate_bit_stru.bit_tx_count);
    }

}


OAL_STATIC  OAL_INLINE oal_void hal_set_sta_bssid(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_byte)
{
    HAL_PUBLIC_HOOK_FUNC(_set_sta_bssid)( pst_hal_vap, puc_byte);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_sta_dtim_period(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_dtim_period)
{
    HAL_PUBLIC_HOOK_FUNC(_set_sta_dtim_period)( pst_hal_vap, ul_dtim_period);
}

OAL_STATIC  OAL_INLINE oal_void hal_get_sta_dtim_period(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 *pul_dtim_period)
{
    HAL_PUBLIC_HOOK_FUNC(_get_sta_dtim_period)( pst_hal_vap, pul_dtim_period);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_sta_dtim_count(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_dtim_count)
{
    HAL_PUBLIC_HOOK_FUNC(_set_sta_dtim_count)( pst_hal_vap, ul_dtim_count);
}

OAL_STATIC  OAL_INLINE oal_void hal_get_psm_dtim_count(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_dtim_count)
{
    HAL_PUBLIC_HOOK_FUNC(_get_psm_dtim_count)( pst_hal_vap, puc_dtim_count);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_psm_dtim_count(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_dtim_count)
{
    HAL_PUBLIC_HOOK_FUNC(_set_psm_dtim_count)( pst_hal_vap, uc_dtim_count);
}

OAL_STATIC  OAL_INLINE oal_bool_enum hal_check_sleep_time(hal_to_dmac_vap_stru *pst_hal_vap)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    return HAL_PUBLIC_HOOK_FUNC(_check_sleep_time)( pst_hal_vap);
#else
    return OAL_TRUE;
#endif
}

OAL_STATIC  OAL_INLINE oal_void hal_pm_wlan_servid_register(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_pm_wlan_servid_register)(pst_hal_device);
}

OAL_STATIC  OAL_INLINE oal_void hal_pm_wlan_servid_unregister(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_pm_wlan_servid_unregister)(pst_hal_device);
}
#ifdef _PRE_PRODUCT_ID_HI110X_DEV
OAL_STATIC  OAL_INLINE oal_void hal_pm_vote2platform(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_vote_state)
{
    HAL_PUBLIC_HOOK_FUNC(_pm_vote2platform)(pst_hal_device, ul_vote_state);
}

OAL_STATIC OAL_INLINE oal_void hal_init_pm_info(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_init_pm_info)(pst_hal_vap);
}

OAL_STATIC OAL_INLINE oal_void hal_pm_set_bcn_rf_chain(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_bcn_rf_chain)
{
    HAL_PUBLIC_HOOK_FUNC(_pm_set_bcn_rf_chain)(pst_hal_vap, uc_bcn_rf_chain);
}

OAL_STATIC OAL_INLINE oal_void hal_pm_set_tbtt_offset(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_adjust_val)
{
    HAL_PUBLIC_HOOK_FUNC(_pm_set_tbtt_offset)(pst_hal_vap, us_adjust_val);
}
#endif

#ifdef _PRE_PM_DYN_SET_TBTT_OFFSET
OAL_STATIC OAL_INLINE oal_void hal_dyn_tbtt_offset_switch(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_switch)
{
    HAL_PUBLIC_HOOK_FUNC(_dyn_tbtt_offset_switch)(pst_hal_device, uc_switch);
}
#endif

#ifdef _PRE_PM_TBTT_OFFSET_PROBE
OAL_STATIC OAL_INLINE oal_void hal_tbtt_offset_probe_init(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_tbtt_offset_probe_init)(pst_hal_vap);
}

OAL_STATIC OAL_INLINE oal_void hal_tbtt_offset_probe_destroy(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_tbtt_offset_probe_destroy)(pst_hal_vap);
}

OAL_STATIC OAL_INLINE oal_void hal_tbtt_offset_probe_suspend(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_tbtt_offset_probe_suspend)(pst_hal_vap);
}

OAL_STATIC OAL_INLINE oal_void hal_tbtt_offset_probe_resume(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_tbtt_offset_probe_resume)(pst_hal_vap);
}

OAL_STATIC OAL_INLINE oal_void hal_tbtt_offset_probe_tbtt_cnt_incr(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_tbtt_offset_probe_tbtt_cnt_incr)(pst_hal_vap);
}

OAL_STATIC OAL_INLINE oal_void hal_tbtt_offset_probe_beacon_cnt_incr(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_tbtt_offset_probe_beacon_cnt_incr)(pst_hal_vap);
}


#endif

OAL_STATIC  OAL_INLINE oal_void hal_disable_tsf_tbtt(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_disable_tsf_tbtt)( pst_hal_vap);
}

OAL_STATIC  OAL_INLINE oal_void hal_enable_tsf_tbtt(hal_to_dmac_vap_stru *pst_hal_vap, oal_bool_enum_uint8 en_dbac_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_tsf_tbtt)( pst_hal_vap, en_dbac_enable);
}

OAL_STATIC OAL_INLINE oal_void hal_cfg_slottime_type(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_slottime_type)
{
    HAL_PUBLIC_HOOK_FUNC(_cfg_slottime_type)( pst_hal_device, ul_slottime_type);
}

/*****************************************************************************
  10.4 SDT��д�Ĵ�������
*****************************************************************************/


OAL_STATIC  OAL_INLINE oal_void hal_mwo_det_enable_mac_counter(hal_to_dmac_device_stru *pst_hal_device, oal_int32 l_enable_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_mwo_det_enable_mac_counter)( pst_hal_device, l_enable_reg_val);
}

#ifdef _PRE_WLAN_FEATURE_BTCOEX
OAL_STATIC OAL_INLINE oal_void hal_coex_sw_irq_set(hal_coex_sw_irq_type_enum_uint8 en_coex_irq_type)
{
    HAL_PUBLIC_HOOK_FUNC(_coex_sw_irq_set)(en_coex_irq_type);
}
OAL_STATIC OAL_INLINE oal_void hal_get_btcoex_abort_qos_null_seq_num(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *ul_qosnull_seq_num)
{
    HAL_PUBLIC_HOOK_FUNC(_get_btcoex_abort_qos_null_seq_num)(pst_hal_device, ul_qosnull_seq_num);
}
OAL_STATIC OAL_INLINE oal_void hal_get_btcoex_occupied_period(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 *us_occupied_period)
{
    HAL_PUBLIC_HOOK_FUNC(_get_btcoex_occupied_period)(pst_hal_device, us_occupied_period);
}
OAL_STATIC OAL_INLINE oal_void hal_get_btcoex_pa_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *ul_pa_status)
{
    HAL_PUBLIC_HOOK_FUNC(_get_btcoex_pa_status)(pst_hal_device, ul_pa_status);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_abort_qos_null_seq_num(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_qosnull_seq_num)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_abort_qos_null_seq_num)(pst_hal_device, ul_qosnull_seq_num);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_abort_null_buff_addr(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_abort_null_buff_addr)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_abort_null_buff_addr)(pst_hal_device, ul_abort_null_buff_addr);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_tx_abort_preempt_type(hal_to_dmac_device_stru *pst_hal_device, hal_coex_hw_preempt_mode_enum_uint8 en_preempt_type)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_tx_abort_preempt_type)(pst_hal_device, en_preempt_type);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_abort_preempt_frame_param(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_preempt_param)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_abort_preempt_frame_param)(pst_hal_device, us_preempt_param);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_hw_rx_priority_dis(hal_to_dmac_device_stru* pst_hal_device, oal_bool_enum_uint8 en_hw_rx_prio_dis)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_hw_rx_priority_dis)(pst_hal_device, en_hw_rx_prio_dis);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_hw_priority_en(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_hw_prio_en)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_hw_priority_en)(pst_hal_device, en_hw_prio_en);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_priority_period(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_priority_period)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_priority_period)(pst_hal_device, us_priority_period);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_get_rf_control(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 ul_occupied_period, oal_uint32 *pul_wlbt_mode_sel, oal_uint16 us_wait_cnt)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_get_rf_control)(pst_hal_device, ul_occupied_period, pul_wlbt_mode_sel, us_wait_cnt);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_occupied_period(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_occupied_period)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_occupied_period)(pst_hal_device, us_occupied_period);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_sw_all_abort_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_sw_abort_ctrl)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_sw_all_abort_ctrl)(pst_hal_device, uc_sw_abort_ctrl);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_sw_priority_flag(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_sw_prio_flag)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_sw_priority_flag)(pst_hal_device, en_sw_prio_flag);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_soc_gpreg0_replace(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_val, oal_uint16 us_mask, oal_uint8 uc_offset)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_soc_gpreg0_replace)(pst_hal_device, uc_val, us_mask, uc_offset);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_soc_gpreg0(oal_uint8 uc_val, oal_uint16 us_mask, oal_uint8 uc_offset)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_soc_gpreg0)(uc_val, us_mask, uc_offset);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_soc_gpreg1(oal_uint8 uc_val, oal_uint16 us_mask, oal_uint8 uc_offset)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_soc_gpreg1)( uc_val, us_mask, uc_offset);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_soc_gpreg1_replace(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_val, oal_uint16 us_mask, oal_uint8 uc_offset)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_soc_gpreg1_replace)(pst_hal_device, uc_val, us_mask, uc_offset);
}

OAL_STATIC OAL_INLINE oal_void hal_update_btcoex_btble_status(hal_to_dmac_chip_stru *pst_hal_chip)
{
    HAL_PUBLIC_HOOK_FUNC(_update_btcoex_btble_status)( pst_hal_chip);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_update_btble_status(hal_to_dmac_chip_stru *pst_hal_chip)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_update_btble_status)( pst_hal_chip);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_init(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_init)( pst_hal_device);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_sw_preempt_init(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_sw_preempt_init)( pst_hal_device);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_process_bt_status(hal_to_dmac_chip_stru *pst_hal_chip, oal_uint8 uc_print)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_process_bt_status)(pst_hal_chip, uc_print);
}
OAL_STATIC OAL_INLINE oal_void hal_get_btcoex_statistic(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_enable_abort_stat)
{
    HAL_PUBLIC_HOOK_FUNC(_get_btcoex_statistic)( pst_hal_device, en_enable_abort_stat);
}
OAL_STATIC OAL_INLINE oal_void hal_mpw_soc_write_reg(oal_uint32 ulQuryRegAddrTemp, oal_uint16 usQuryRegValueTemp)
{
    HAL_PUBLIC_HOOK_FUNC(_mpw_soc_write_reg)( ulQuryRegAddrTemp, usQuryRegValueTemp);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_update_ap_beacon_count(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_beacon_count)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_update_ap_beacon_count)(pst_hal_device, pul_beacon_count);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_post_event(hal_to_dmac_chip_stru *pst_hal_chip, oal_uint8 uc_sub_type)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_post_event)( pst_hal_chip, uc_sub_type);
}

OAL_STATIC OAL_INLINE oal_void hal_btcoex_set_wl0_antc_switch(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_tx_slv)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_set_wl0_antc_switch)(pst_hal_device, en_tx_slv);
}

#ifdef _PRE_WLAN_FEATURE_BTCOEX_SLV_TX_BUGFIX
OAL_STATIC OAL_INLINE oal_void hal_btcoex_set_wl0_tx_slv_en(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_tx_slv)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_set_wl0_tx_slv_en)(pst_hal_device, en_tx_slv);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_set_pta0_wl0_selected_sel(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_sw_ctl)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_set_pta0_wl0_selected_sel)(pst_hal_device, en_sw_ctl);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_set_wl0_rx_status_byp(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_rx_byp)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_set_wl0_rx_status_byp)(pst_hal_device, en_rx_byp);
}
#endif

OAL_STATIC OAL_INLINE oal_void hal_btcoex_get_bt_sco_status(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 *en_sco_status)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_get_bt_sco_status)(pst_hal_device, en_sco_status);
}

OAL_STATIC OAL_INLINE oal_void hal_btcoex_get_slna_status(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 *en_slna_status)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_get_slna_status)(pst_hal_device, en_slna_status);
}

OAL_STATIC OAL_INLINE oal_void hal_btcoex_get_bt_acl_status(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 *en_acl_status)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_get_bt_acl_status)(pst_hal_device, en_acl_status);
}

OAL_STATIC OAL_INLINE oal_void hal_btcoex_get_ps_service_status(hal_to_dmac_device_stru *pst_hal_device, hal_btcoex_ps_status_enum_uint8 *en_ps_status)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_get_ps_service_status)(pst_hal_device, en_ps_status);
}

OAL_STATIC OAL_INLINE oal_void hal_btcoex_set_slna_en(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_slna)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_set_slna_en)(pst_hal_device, en_slna);
}

OAL_STATIC OAL_INLINE oal_void hal_btcoex_set_ba_resp_pri(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_occu)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_set_ba_resp_pri)(pst_hal_device, en_occu);
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
#ifdef _PRE_WLAN_1103_PILOT

#else
OAL_STATIC OAL_INLINE oal_void hal_btcoex_open_5g_upc(oal_void)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_open_5g_upc)();
}
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_LTECOEX
OAL_STATIC OAL_INLINE oal_void hal_ltecoex_req_mask_ctrl(oal_uint16 lte_req_mask)
{
    HAL_PUBLIC_HOOK_FUNC(_ltecoex_req_mask_ctrl)( lte_req_mask);
}
#endif

#endif
#ifdef _PRE_WLAN_FEATURE_SMARTANT
OAL_STATIC OAL_INLINE oal_void hal_dual_antenna_switch(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_value, oal_uint8 uc_by_alg, oal_uint32 *pul_result)
{
    HAL_PUBLIC_HOOK_FUNC(_dual_antenna_switch)(pst_hal_device, uc_value, uc_by_alg, pul_result);
}
OAL_STATIC OAL_INLINE oal_void hal_dual_antenna_switch_at(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_value, oal_uint32 *pul_result)
{
    HAL_PUBLIC_HOOK_FUNC(_dual_antenna_switch_at)(pst_hal_device, uc_value, pul_result);
}
OAL_STATIC OAL_INLINE oal_void hal_dual_antenna_check(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_result)
{
    HAL_PUBLIC_HOOK_FUNC(_dual_antenna_check)(pst_hal_device, pul_result);
}
OAL_STATIC OAL_INLINE oal_void hal_dual_antenna_init(oal_void)
{
    HAL_PUBLIC_HOOK_FUNC(_dual_antenna_init)();
}
#endif
OAL_STATIC OAL_INLINE oal_void hal_tx_get_dscr_iv_word(hal_tx_dscr_stru *pst_dscr, oal_uint32 *pul_iv_ms_word, oal_uint32 *pul_iv_ls_word, oal_uint8 uc_chiper_type, oal_uint8 uc_chiper_key_id)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_iv_word)( pst_dscr, pul_iv_ms_word, pul_iv_ls_word, uc_chiper_type, uc_chiper_key_id);
}
OAL_STATIC  OAL_INLINE oal_void hal_get_hw_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *ul_cali_check_hw_status)
{
    HAL_PUBLIC_HOOK_FUNC(_get_hw_status)( pst_hal_device, ul_cali_check_hw_status);
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

OAL_STATIC OAL_INLINE oal_void hal_set_txrx_chain(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_set_txrx_chain)(pst_hal_device);
}
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)

OAL_STATIC OAL_INLINE oal_void hal_set_cali_power(hal_to_dmac_device_stru * pst_hal_device, oal_uint8 uc_ch, oal_uint8 uc_freq, oal_int16 *ps_power, wlan_band_cap_enum_uint8 en_band_cap, oal_uint32 *pul_ret)
{
    HAL_PUBLIC_HOOK_FUNC(_set_cali_power)(pst_hal_device, uc_ch, uc_freq, ps_power, en_band_cap, pul_ret);
}

OAL_STATIC OAL_INLINE oal_void hal_get_cali_power(hal_to_dmac_device_stru * pst_hal_device, oal_uint8 uc_ch, oal_uint8 uc_freq, oal_int16 *ps_power, wlan_band_cap_enum_uint8 en_band_cap)
{
    HAL_PUBLIC_HOOK_FUNC(_get_cali_power)(pst_hal_device, uc_ch, uc_freq, ps_power, en_band_cap);
}

OAL_STATIC OAL_INLINE oal_void hal_set_polynomial_param(hal_to_dmac_device_stru * pst_hal_device, oal_int16 *ps_polynomial_para, oal_uint8 uc_freq, wlan_band_cap_enum_uint8 en_band_cap)
{
    HAL_PUBLIC_HOOK_FUNC(_set_polynomial_param)(pst_hal_device, ps_polynomial_para, uc_freq, en_band_cap);
}

OAL_STATIC OAL_INLINE oal_void hal_get_polynomial_params(hal_to_dmac_device_stru * pst_hal_device, oal_int16 *ps_polynomial, oal_int32 *pl_length, wlan_band_cap_enum_uint8 en_band_cap)
{
    HAL_PUBLIC_HOOK_FUNC(_get_polynomial_params)(pst_hal_device, ps_polynomial, pl_length, en_band_cap);
}

OAL_STATIC OAL_INLINE oal_void hal_get_all_cali_power(hal_to_dmac_device_stru * pst_hal_device, oal_int16 *ps_power, oal_int32 *pl_length, wlan_band_cap_enum_uint8 en_band_cap)
{
    HAL_PUBLIC_HOOK_FUNC(_get_all_cali_power)(pst_hal_device, ps_power, pl_length, en_band_cap);
}


OAL_STATIC OAL_INLINE oal_void hal_get_upc_params(hal_to_dmac_device_stru * pst_hal_device, oal_uint16 *pus_polynomial, oal_uint32 *pul_length, wlan_band_cap_enum_uint8 en_band_cap)
{
    HAL_PUBLIC_HOOK_FUNC(_get_upc_params)(pst_hal_device, pus_polynomial, pul_length, en_band_cap);
}


OAL_STATIC OAL_INLINE oal_void hal_set_upc_params(hal_to_dmac_device_stru * pst_hal_device, oal_uint16 *pus_upc_param, wlan_band_cap_enum_uint8 en_band_cap)
{
    HAL_PUBLIC_HOOK_FUNC(_set_upc_params)(pst_hal_device, pus_upc_param, en_band_cap);
}


OAL_STATIC OAL_INLINE oal_void hal_set_dyn_cali_pow_offset(hal_to_dmac_device_stru * pst_hal_device, oal_uint8 uc_param_val)
{
     HAL_PUBLIC_HOOK_FUNC(_set_dyn_cali_pow_offset)(pst_hal_device, uc_param_val);
}

#endif


#ifdef _PRE_WLAN_RF_CALI
OAL_STATIC OAL_INLINE oal_void hal_set_2g_txrx_path(hal_to_dmac_device_stru *pst_hal_device,oal_uint8 uc_channel_idx,
                                                    wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                                                    oal_uint8 uc_2g_path)
{
    HAL_PUBLIC_HOOK_FUNC(_set_2g_txrx_path)(pst_hal_device, uc_channel_idx, en_bandwidth, uc_2g_path);
}
#endif

#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)

OAL_STATIC OAL_INLINE oal_void  hal_rf_cali_set_vref(wlan_channel_band_enum_uint8 en_band, oal_uint8 uc_chain_idx,
                                    oal_uint8  uc_band_idx, oal_uint16 us_vref_value)
{
    HAL_PUBLIC_HOOK_FUNC(_rf_cali_set_vref)(en_band, uc_chain_idx,uc_band_idx,us_vref_value);
}


OAL_STATIC  OAL_INLINE oal_void hal_rf_auto_cali(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_rf_auto_cali)( pst_hal_device);
}
#endif
#endif

#ifdef _PRE_WLAN_DFT_STAT
OAL_STATIC OAL_INLINE oal_void hal_dft_set_phy_stat_node(hal_to_dmac_device_stru *pst_hal_device,oam_stats_phy_node_idx_stru *pst_phy_node_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_set_phy_stat_node)( pst_hal_device, pst_phy_node_idx);
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
OAL_STATIC OAL_INLINE oal_void hal_dft_get_machw_stat_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint32* pul_machw_stat,oal_uint8 us_bank_select, oal_uint32 *pul_len)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_get_machw_stat_info)( pst_hal_device, pul_machw_stat,us_bank_select, pul_len);
}

OAL_STATIC OAL_INLINE oal_void hal_dft_get_phyhw_stat_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint32* pul_phyhw_stat,oal_uint8 us_bank_select, oal_uint32 *pul_len)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_get_phyhw_stat_info)( pst_hal_device, pul_phyhw_stat, us_bank_select, pul_len);
}
OAL_STATIC OAL_INLINE oal_void hal_dft_get_rfhw_stat_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint32* pul_rfhw_stat, oal_uint32 *pul_len, oal_uint8 uc_rf_select)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_get_rfhw_stat_info)( pst_hal_device, pul_rfhw_stat, pul_len, uc_rf_select);
}
OAL_STATIC OAL_INLINE oal_void hal_dft_print_machw_stat(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_print_machw_stat)( pst_hal_device);
}
OAL_STATIC OAL_INLINE oal_void hal_dft_print_phyhw_stat(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_print_phyhw_stat)( pst_hal_device);
}
OAL_STATIC OAL_INLINE oal_void hal_dft_print_rfhw_stat(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_print_rfhw_stat)( pst_hal_device);
}

OAL_STATIC OAL_INLINE oal_void  hal_dft_report_all_reg_state(hal_to_dmac_device_stru   *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_report_all_reg_state)( pst_hal_device);
}
#else
OAL_STATIC  OAL_INLINE oal_void hal_get_phy_stat_info(hal_to_dmac_device_stru *pst_hal_device, oam_stats_phy_stat_stru *pst_phy_stat)
{
    HAL_PUBLIC_HOOK_FUNC(_get_phy_stat_info)(pst_hal_device, pst_phy_stat);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_counter_clear_value(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_val)
{
    HAL_PUBLIC_HOOK_FUNC(_set_counter_clear_value)(pst_hal_device, ul_val);
}

OAL_STATIC  OAL_INLINE oal_void hal_get_tx_hi_pri_mpdu_cnt(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_cnt)
{
    HAL_PUBLIC_HOOK_FUNC(_get_tx_hi_pri_mpdu_cnt)(pst_hal_device, pul_cnt);
}

OAL_STATIC  OAL_INLINE oal_void hal_get_tx_bcn_count(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_cnt)
{
    HAL_PUBLIC_HOOK_FUNC(_get_tx_bcn_count)(pst_hal_device, pul_cnt);
}

OAL_STATIC OAL_INLINE oal_void hal_dft_get_extlna_gain(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_extlna_gain0_cfg, oal_uint32 *pul_extlna_gain1_cfg)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_get_extlna_gain)( pst_hal_device, pul_extlna_gain0_cfg, pul_extlna_gain1_cfg);
}

OAL_STATIC OAL_INLINE oal_void hal_dft_get_chan_stat_result(hal_to_dmac_device_stru *pst_hal_device, oam_stats_dbb_env_param_stru *pst_dbb_env_param)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_get_chan_stat_result)( pst_hal_device, pst_dbb_env_param);
}

OAL_STATIC OAL_INLINE oal_void hal_dft_enable_mac_filter(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_enable_mac_filter)( pst_hal_device, uc_enable);
}

OAL_STATIC OAL_INLINE oal_void hal_dft_get_power0_ref(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_val)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_get_power0_ref)( pst_hal_device, pul_val);
}

OAL_STATIC OAL_INLINE oal_void hal_dft_get_phy_pin_code_rpt(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_val)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_get_phy_pin_code_rpt)( pst_hal_device, pul_val);
}

OAL_STATIC OAL_INLINE oal_void hal_dft_get_machw_stat_info_ext(hal_to_dmac_device_stru *pst_hal_device, oam_stats_machw_stat_stru *pst_machw_stat)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_get_machw_stat_info_ext)( pst_hal_device, pst_machw_stat);
}

OAL_STATIC OAL_INLINE oal_void hal_dft_get_beacon_miss_stat_info(hal_to_dmac_device_stru *pst_hal_device, oam_stats_dbb_env_param_stru *pst_dbb_env_param)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_get_beacon_miss_stat_info)( pst_hal_device, pst_dbb_env_param);
}
#endif
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
OAL_STATIC OAL_INLINE oal_void  hal_set_lte_gpio_mode(oal_uint32 ul_mode_value)
{

    HAL_PUBLIC_HOOK_FUNC(_set_lte_gpio_mode)(ul_mode_value);
}
#endif

#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP
OAL_STATIC OAL_INLINE oal_void hal_cfg_rsp_dyn_bw(oal_bool_enum_uint8 en_set, hal_channel_assemble_enum_uint8 en_dyn_bw)
{
    HAL_PUBLIC_HOOK_FUNC(_cfg_rsp_dyn_bw)(en_set, en_dyn_bw);
}

OAL_STATIC OAL_INLINE oal_void hal_get_cfg_rsp_rate_mode(oal_uint32 *pul_rsp_rate_cfg_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_get_cfg_rsp_rate_mode)(pul_rsp_rate_cfg_mode);
}

OAL_STATIC OAL_INLINE oal_void hal_set_rsp_rate(oal_uint32 ul_rsp_rate_val)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rsp_rate)(ul_rsp_rate_val);
}
#endif



OAL_STATIC OAL_INLINE oal_void hal_check_test_value_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_value, oal_uint32 *pul_result)
{
    HAL_PUBLIC_HOOK_FUNC(_check_test_value_reg)( pst_hal_device, us_value, pul_result);
}


OAL_STATIC OAL_INLINE oal_void hal_revert_cw_signal_reg(hal_to_dmac_device_stru *pst_hal_device, wlan_channel_band_enum_uint8 en_band)
{
    HAL_PUBLIC_HOOK_FUNC(_revert_cw_signal_reg)( pst_hal_device, en_band);
}
OAL_STATIC OAL_INLINE oal_void hal_cfg_cw_signal_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_chain_idx, wlan_channel_band_enum_uint8 en_band)
{
    HAL_PUBLIC_HOOK_FUNC(_cfg_cw_signal_reg)( pst_hal_device, uc_chain_idx, en_band);
}
OAL_STATIC OAL_INLINE oal_void hal_get_cw_signal_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_chain_idx, wlan_channel_band_enum_uint8 en_band)
{
    HAL_PUBLIC_HOOK_FUNC(_get_cw_signal_reg)( pst_hal_device, uc_chain_idx, en_band);
}

#ifdef _PRE_WLAN_PRODUCT_1151V200

OAL_STATIC  OAL_INLINE oal_void  hal_set_peer_resp_dis(hal_to_dmac_device_stru *pst_hal_device, hal_peer_resp_dis_cfg_stru *pst_peer_resp_dis_cfg)
{
    HAL_PUBLIC_HOOK_FUNC(_set_peer_resp_dis)(pst_hal_device, pst_peer_resp_dis_cfg);
}


OAL_STATIC  OAL_INLINE oal_void  hal_get_peer_resp_dis(hal_to_dmac_device_stru *pst_hal_device, hal_peer_resp_dis_cfg_stru *pst_peer_resp_dis_cfg)
{
    HAL_PUBLIC_HOOK_FUNC(_get_peer_resp_dis)(pst_hal_device, pst_peer_resp_dis_cfg);
}

OAL_STATIC  OAL_INLINE oal_void  hal_disable_direct_mgmt_filter(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_disable_direct_mgmt_filter)(pst_hal_device);
}

#endif


OAL_STATIC OAL_INLINE oal_void  hal_config_always_rx(hal_to_dmac_device_stru *pst_hal_device_base, oal_uint8 uc_switch)
{
    HAL_PUBLIC_HOOK_FUNC(_config_always_rx)( pst_hal_device_base, uc_switch);
}


#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
OAL_STATIC OAL_INLINE oal_void  hal_config_always_rx_new(hal_to_dmac_device_stru *pst_hal_device_base, oal_uint8 uc_switch)
{
    HAL_PUBLIC_HOOK_FUNC(_config_always_rx_new)( pst_hal_device_base, uc_switch);
}
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
#ifdef _PRE_WLAN_PRODUCT_1151V200
OAL_STATIC OAL_INLINE oal_void  hal_config_adc_target(hal_to_dmac_device_stru *pst_hal_device_base, oal_uint32 value)
{
    HAL_PUBLIC_HOOK_FUNC(_config_adc_target)( pst_hal_device_base, value);
}
#endif
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

OAL_STATIC OAL_INLINE oal_void  hal_load_ini_power_gain(oal_void)
{
    HAL_PUBLIC_HOOK_FUNC(_load_ini_power_gain)();
}


OAL_STATIC OAL_INLINE oal_void  hal_config_update_scaling_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint16* paus_dbb_scale)
{
    HAL_PUBLIC_HOOK_FUNC(_config_update_scaling_reg)(pst_hal_device, paus_dbb_scale);
}


OAL_STATIC OAL_INLINE oal_void hal_config_update_dsss_scaling_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint16* paus_dbb_scale, oal_uint8  uc_distance)
{
    HAL_PUBLIC_HOOK_FUNC(_config_update_dsss_scaling_reg)(pst_hal_device, paus_dbb_scale, uc_distance);
}


OAL_STATIC OAL_INLINE oal_void hal_config_custom_rf(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_param)
{
    HAL_PUBLIC_HOOK_FUNC(_config_custom_rf)(pst_hal_device, puc_param);
}


OAL_STATIC OAL_INLINE oal_void hal_config_custom_dts_cali(oal_uint8  *puc_param)
{
    HAL_PUBLIC_HOOK_FUNC(_config_custom_dts_cali)(puc_param);
}


OAL_STATIC OAL_INLINE oal_void hal_config_get_cus_nvram_params(hal_cfg_custom_nvram_params_stru **ppst_cfg_nvram)
{
    HAL_PUBLIC_HOOK_FUNC(_config_get_cus_nvram_params)(ppst_cfg_nvram);
}

#ifndef _PRE_WLAN_1103_PILOT
OAL_STATIC OAL_INLINE oal_void hal_config_get_far_dist_dsss_scale_promote_switch(oal_uint8 *puc_switch)
{
    HAL_PUBLIC_HOOK_FUNC(_config_get_far_dist_dsss_scale_promote_switch)(puc_switch);
}
#endif


OAL_STATIC OAL_INLINE oal_void hal_config_set_cus_nvram_params(hal_to_dmac_device_stru *pst_hal_device, oal_uint8  *puc_param)
{
    HAL_PUBLIC_HOOK_FUNC(_config_set_cus_nvram_params)(pst_hal_device, puc_param);
}


OAL_STATIC OAL_INLINE oal_void hal_config_update_rate_pow_table(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_config_update_rate_pow_table)(pst_hal_device);
}

OAL_STATIC OAL_INLINE oal_int16  hal_rf_cali_cal_20log(oal_int16 s_vdet_val)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    return HAL_PUBLIC_HOOK_FUNC(_rf_cali_cal_20log)(s_vdet_val);
#else
    return 0;
#endif
}

#endif //#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
OAL_STATIC OAL_INLINE oal_uint32 hal_config_custom_dyn_cali(oal_uint8  *puc_param)
{
    return HAL_PUBLIC_HOOK_FUNC(_config_custom_dyn_cali)(puc_param);
}
#endif

#ifdef _PRE_WLAN_FEATURE_TPC_OPT

OAL_STATIC OAL_INLINE oal_void hal_update_upc_amend_by_tas(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_chn_idx,
                                                           oal_bool_enum_uint8 en_need_improved)
{
    HAL_PUBLIC_HOOK_FUNC(_update_upc_amend_by_tas)(pst_hal_device, uc_chn_idx, en_need_improved);
}

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH

OAL_STATIC OAL_INLINE oal_void hal_rf_init_upc_amend(oal_void)
{
    HAL_PUBLIC_HOOK_FUNC(_rf_init_upc_amend)();
}
#endif
#endif


OAL_STATIC OAL_INLINE oal_void hal_get_rate_idx_pow(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_pow_idx,
                                                     wlan_channel_band_enum_uint8 en_freq_band, oal_uint16 *pus_powr, oal_uint8 uc_chan_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_get_rate_idx_pow)(pst_hal_device, uc_pow_idx, en_freq_band, pus_powr, uc_chan_idx);
}

OAL_STATIC OAL_INLINE oal_void hal_get_target_tx_power_by_tx_dscr(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_ctrl_one_param *pst_tx_dscr_one,
                                                                             hal_pdet_info_stru *pst_pdet_info, oal_int16 *ps_tx_pow)
{
    HAL_PUBLIC_HOOK_FUNC(_get_target_tx_power_by_tx_dscr)(pst_hal_device, pst_tx_dscr_one, pst_pdet_info, ps_tx_pow);
}



#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
OAL_STATIC OAL_INLINE oal_void hal_get_cali_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_param)
{
    HAL_PUBLIC_HOOK_FUNC(_get_cali_info)(pst_hal_device, puc_param);
}
#endif


OAL_STATIC OAL_INLINE oal_void  hal_vap_get_gtk_rx_lut_idx(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_lut_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_get_gtk_rx_lut_idx)(pst_hal_vap, puc_lut_idx);
}

#ifdef _PRE_WLAN_FEATURE_M2S
OAL_STATIC OAL_INLINE oal_void hal_update_datarate_by_chain(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_resp_tx_chain)
{
    HAL_PUBLIC_HOOK_FUNC(_update_datarate_by_chain)(pst_hal_device, uc_resp_tx_chain);
}
#endif


#ifdef _PRE_WLAN_FEATURE_FTM

OAL_STATIC OAL_INLINE oal_uint64  hal_get_ftm_time(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 ull_time)
{
    return HAL_PUBLIC_HOOK_FUNC(_get_ftm_time)(pst_hal_device, ull_time);
}


OAL_STATIC OAL_INLINE oal_uint64  hal_check_ftm_t4(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 ull_time)
{
    return HAL_PUBLIC_HOOK_FUNC(_check_ftm_t4)(pst_hal_device, ull_time);
}


OAL_STATIC OAL_INLINE oal_int8  hal_get_ftm_t4_intp(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 ull_time)
{
    return HAL_PUBLIC_HOOK_FUNC(_get_ftm_t4_intp)(pst_hal_device, ull_time);
}


OAL_STATIC OAL_INLINE oal_uint64  hal_check_ftm_t2(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 ull_time)
{
    return HAL_PUBLIC_HOOK_FUNC(_check_ftm_t2)(pst_hal_device, ull_time);
}


OAL_STATIC OAL_INLINE oal_int8  hal_get_ftm_t2_intp(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 ull_time)
{
    return HAL_PUBLIC_HOOK_FUNC(_get_ftm_t2_intp)(pst_hal_device, ull_time);
}


OAL_STATIC OAL_INLINE oal_void  hal_get_ftm_tod(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 *pull_tod)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ftm_tod)(pst_hal_device, pull_tod);
}


OAL_STATIC OAL_INLINE oal_void  hal_get_ftm_toa(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 *pull_toa)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ftm_toa)(pst_hal_device, pull_toa);
}


OAL_STATIC OAL_INLINE oal_void  hal_get_ftm_t2(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 *pull_t2)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ftm_t2)(pst_hal_device, pull_t2);
}


OAL_STATIC OAL_INLINE oal_void  hal_get_ftm_t3(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 *puc_t2)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ftm_t3)(pst_hal_device, puc_t2);
}


OAL_STATIC OAL_INLINE oal_void hal_set_ftm_enable(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_ftm_status)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ftm_enable)(pst_hal_device, en_ftm_status);
}


OAL_STATIC OAL_INLINE oal_void hal_set_ftm_sample(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_ftm_status)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ftm_sample)(pst_hal_device, en_ftm_status);
}


OAL_STATIC OAL_INLINE oal_void hal_get_ftm_ctrl_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_ftm_status)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ftm_ctrl_status)(pst_hal_device, pul_ftm_status);
}

OAL_STATIC OAL_INLINE oal_void hal_get_ftm_config_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_ftm_status)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ftm_config_status)(pst_hal_device, pul_ftm_status);
}

OAL_STATIC OAL_INLINE oal_void hal_set_ftm_ctrl_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_ftm_status)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ftm_ctrl_status)(pst_hal_device, ul_ftm_status);
}

OAL_STATIC OAL_INLINE oal_void hal_set_ftm_config_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_ftm_status)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ftm_config_status)(pst_hal_device, ul_ftm_status);
}


OAL_STATIC OAL_INLINE oal_void hal_get_ftm_dialog(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_dialog)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ftm_dialog)(pst_hal_device, puc_dialog);
}


OAL_STATIC OAL_INLINE oal_void hal_get_ftm_cali_rx_time(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_ftm_cali_rx_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ftm_cali_rx_time)(pst_hal_device, pul_ftm_cali_rx_time);
}


OAL_STATIC OAL_INLINE oal_void hal_get_ftm_cali_rx_intp_time(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_ftm_cali_rx_intp_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ftm_cali_rx_intp_time)(pst_hal_device, pul_ftm_cali_rx_intp_time);
}


OAL_STATIC OAL_INLINE oal_void hal_get_ftm_cali_tx_time(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_ftm_cali_tx_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ftm_cali_tx_time)(pst_hal_device, pul_ftm_cali_tx_time);
}


OAL_STATIC OAL_INLINE oal_void hal_set_ftm_cali(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru * pst_tx_dscr, oal_bool_enum_uint8 en_ftm_cali)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ftm_cali)(pst_hal_device, pst_tx_dscr, en_ftm_cali);
}


OAL_STATIC OAL_INLINE oal_void hal_set_ftm_tx_cnt(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru * pst_tx_dscr, oal_uint8 uc_ftm_tx_cnt)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ftm_tx_cnt)(pst_hal_device, pst_tx_dscr, uc_ftm_tx_cnt);
}


OAL_STATIC OAL_INLINE oal_void hal_set_ftm_bandwidth(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru * pst_tx_dscr, wlan_bw_cap_enum_uint8 en_band_cap)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ftm_bandwidth)(pst_hal_device, pst_tx_dscr, en_band_cap);
}


OAL_STATIC OAL_INLINE oal_void hal_set_ftm_protocol(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru * pst_tx_dscr, wlan_phy_protocol_enum_uint8 uc_prot_format)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ftm_protocol)(pst_hal_device, pst_tx_dscr, uc_prot_format);
}


OAL_STATIC OAL_INLINE oal_void hal_set_ftm_m2s(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru * pst_tx_dscr, oal_uint8 uc_tx_chain_selection)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ftm_m2s)(pst_hal_device, pst_tx_dscr, uc_tx_chain_selection);
}


OAL_STATIC OAL_INLINE oal_void hal_get_ftm_rtp_reg(hal_to_dmac_device_stru *pst_hal_device,
                                                             oal_uint32 *pul_reg0,
                                                             oal_uint32 *pul_reg1,
                                                             oal_uint32 *pul_reg2,
                                                             oal_uint32 *pul_reg3,
                                                             oal_uint32 *pul_reg4)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ftm_rtp_reg)(pst_hal_device,
                                           pul_reg0,
                                           pul_reg1,
                                           pul_reg2,
                                           pul_reg3,
                                           pul_reg4);
}



OAL_STATIC OAL_INLINE oal_void hal_set_ftm_m2s_phy(hal_to_dmac_device_stru * pst_hal_device,
                                                           oal_bool_enum_uint8 en_is_mimo,
                                                           oal_uint8 uc_tx_chain_selection)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ftm_m2s_phy)(pst_hal_device, en_is_mimo, uc_tx_chain_selection);
}

#endif

#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE

OAL_STATIC OAL_INLINE oal_void  hal_packet_capture_write_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_circle_buf_start, oal_uint16 us_circle_buf_depth)
{
    HAL_PUBLIC_HOOK_FUNC(_packet_capture_write_reg)(pst_hal_device, pul_circle_buf_start, us_circle_buf_depth);
}


OAL_STATIC OAL_INLINE oal_void  hal_packet_capture_switch_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_capture_switch)
{
    HAL_PUBLIC_HOOK_FUNC(_packet_capture_switch_reg)(pst_hal_device, uc_capture_switch);
}


OAL_STATIC  OAL_INLINE oal_void  hal_get_bcn_info(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 *pul_bcn_rate, oal_uint32 *pul_phy_tx_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_get_bcn_info)( pst_hal_vap, pul_bcn_rate, pul_phy_tx_mode);
}

OAL_STATIC OAL_INLINE oal_void hal_tx_get_dscr_phy_tx_mode_param( hal_tx_dscr_stru * pst_tx_dscr, hal_tx_txop_rate_params_stru *pst_phy_tx_mode_param)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_phy_tx_mode_param)( pst_tx_dscr, pst_phy_tx_mode_param);
}
#endif

#ifdef _PRE_WLAN_DFT_REG
OAL_STATIC  OAL_INLINE oal_void hal_debug_refresh_reg_ext(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 en_evt_type, oal_uint32 *pul_ret)
{
    HAL_PUBLIC_HOOK_FUNC(_debug_refresh_reg_ext)(pst_hal_device, en_evt_type, pul_ret);
}
OAL_STATIC  OAL_INLINE oal_void hal_debug_frw_evt(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_debug_frw_evt)(pst_hal_device);
}
#endif

OAL_STATIC  OAL_INLINE oal_void hal_get_wow_enable_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_status)
{
    HAL_PUBLIC_HOOK_FUNC(_get_wow_enable_status)(pst_hal_device, pul_status);
}

#ifdef _PRE_WLAN_CHIP_TEST
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
OAL_STATIC  OAL_INLINE oal_void hal_set_tx_dscr_long_nav_enable(hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 uc_en_status)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tx_dscr_long_nav_enable)(pst_tx_dscr, uc_en_status);
}
#endif
#endif

#ifdef _PRE_WLAN_CACHE_COHERENT_SUPPORT
OAL_STATIC  OAL_INLINE oal_void hal_get_tx_msdu_address_params(hal_tx_dscr_stru *pst_dscr, hal_tx_msdu_address_params **ppst_tx_dscr_msdu_subtable, oal_uint8 *puc_msdu_num)
{
    HAL_PUBLIC_HOOK_FUNC(_get_tx_msdu_address_params)(pst_dscr, ppst_tx_dscr_msdu_subtable, puc_msdu_num);
}
#endif

OAL_STATIC OAL_INLINE oal_void hal_flush_tx_complete_irq(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_flush_tx_complete_irq)( pst_hal_device);
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
OAL_STATIC OAL_INLINE oal_void hal_flush_rx_queue_complete_irq(hal_to_dmac_device_stru *pst_hal_device, hal_rx_dscr_queue_id_enum_uint8 en_queue_num)
{
    HAL_PUBLIC_HOOK_FUNC(_flush_rx_queue_complete_irq)(pst_hal_device, en_queue_num);
}
#endif

#ifndef _PRE_WLAN_FEATURE_TPC_OPT

OAL_STATIC  OAL_INLINE oal_void  hal_device_init_vap_pow_code(hal_to_dmac_device_stru   *pst_hal_device,
                                            hal_vap_pow_info_stru            *pst_vap_pow_info,
                                            oal_uint8                         uc_cur_ch_num,
                                            wlan_channel_band_enum_uint8      en_freq_band,
                                            wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                                            hal_pow_set_type_enum_uint8       uc_type)
{
    HAL_PUBLIC_HOOK_FUNC(_device_init_vap_pow_code)(pst_hal_device, pst_vap_pow_info, uc_cur_ch_num, en_freq_band, en_bandwidth, uc_type);
}
#else
OAL_STATIC  OAL_INLINE oal_void  hal_device_init_vap_pow_code(hal_to_dmac_device_stru   *pst_hal_device,
                                            hal_vap_pow_info_stru            *pst_vap_pow_info,
                                            oal_uint8                         uc_chan_idx,
                                            wlan_channel_band_enum_uint8      en_freq_band,
                                            wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                                            hal_pow_set_type_enum_uint8       uc_type,
                                            oal_uint8                         uc_chan_num)
{
    HAL_PUBLIC_HOOK_FUNC(_device_init_vap_pow_code)(pst_hal_device, pst_vap_pow_info, uc_chan_idx, en_freq_band, en_bandwidth, uc_type, uc_chan_num);
}
#endif

OAL_STATIC  OAL_INLINE oal_void hal_device_get_tx_pow_from_rate_idx(hal_to_dmac_device_stru * pst_hal_device, hal_user_pow_info_stru *pst_hal_user_pow_info,
                                wlan_channel_band_enum_uint8 en_freq_band, oal_uint8 uc_cur_ch_num, oal_uint8 uc_cur_rate_pow_idx,
                                hal_tx_txop_tx_power_stru *pst_tx_power, oal_int16 *ps_tx_pow)
{
    HAL_PUBLIC_HOOK_FUNC(_device_get_tx_pow_from_rate_idx)(pst_hal_device, pst_hal_user_pow_info, en_freq_band, uc_cur_ch_num, uc_cur_rate_pow_idx, pst_tx_power, ps_tx_pow);
}

#ifdef _PRE_WLAN_1103_PILOT
OAL_STATIC  OAL_INLINE oal_void hal_device_enable_mac1(oal_void)
{
    HAL_PUBLIC_HOOK_FUNC(_device_enable_mac1)();
}
OAL_STATIC  OAL_INLINE oal_void hal_device_disable_mac1(oal_void)
{
    HAL_PUBLIC_HOOK_FUNC(_device_disable_mac1)();
}
OAL_STATIC  OAL_INLINE oal_void hal_device_enable_phy1(oal_void)
{
    HAL_PUBLIC_HOOK_FUNC(_device_enable_phy1)();
}
OAL_STATIC  OAL_INLINE oal_void hal_device_disable_phy1(oal_void)
{
    HAL_PUBLIC_HOOK_FUNC(_device_disable_phy1)();
}
#endif

#ifdef _PRE_WLAN_1103_CHR
OAL_STATIC  OAL_INLINE oal_void  hal_get_phy_mac_chr_info(hal_to_dmac_device_stru *pst_hal_device, hal_phy_mac_chr_info_stru *pst_phy_mac_chr_info)
{
    HAL_PUBLIC_HOOK_FUNC(_get_phy_mac_chr_info)( pst_hal_device, pst_phy_mac_chr_info);
}
#endif

OAL_STATIC OAL_INLINE oal_void hal_set_abort_timers_cnt(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_abort_cnt)
{
    HAL_PUBLIC_HOOK_FUNC(_set_abort_timers_cnt)(pst_hal_device, uc_abort_cnt);
}

OAL_STATIC OAL_INLINE oal_void hal_get_abort_timers_cnt(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *uc_abort_cnt)
{
    HAL_PUBLIC_HOOK_FUNC(_get_abort_timers_cnt)(pst_hal_device, uc_abort_cnt);
}

OAL_STATIC OAL_INLINE oal_uint16 hal_bt20dbm_enable_status(oal_void)
{
#ifdef _PRE_WLAN_FEATURE_BT_20DBM
    return HAL_PUBLIC_HOOK_FUNC(_bt20dbm_enable_status)();
#else
    return 0;
#endif
}

#ifdef _PRE_WLAN_FEATURE_P2P_NOA_DSLEEP
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hal_check_noa_sleep_time(hal_to_dmac_device_stru *pst_hal_device)
{
    return HAL_PUBLIC_HOOK_FUNC(_check_noa_sleep_time)(pst_hal_device);
}
#endif

#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
typedef mac_scan_state_enum_uint8 (*hi1103_get_wifi_scan_state)(oal_uint32 *pst_end_time);
void hal_dmac_msg_handler_register(hi1103_get_wifi_scan_state pfMsgHandler);
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of hal_ext_if.h */
