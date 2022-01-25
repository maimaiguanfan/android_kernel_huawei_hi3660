

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

#ifdef _PRE_WLAN_CHIP_TEST
#include "hal_lpm_test.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_EXT_IF_H

/* ��ʱ���ø�ȫ�ֱ�������ȡ�����ķ���ͨ�� */
extern oal_int32 g_l_rf_channel_num;
extern oal_int32 g_l_rf_single_tran;
extern oal_int32 g_l_rf_fem_switch;
extern OAL_CONST oal_int16 g_aus_cutom_simple_txpwr_table[];

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define HAL_MAC_ERROR_THRESHOLD     10  /* macӲ���������ޣ��������Э��Ӳ������ȥ������λ���� */
/* 0.1dbm��λ */
#define HAL_RSSI_SIGNAL_MIN                     (-1030)     /*�źſ����Сֵ */
#define HAL_RSSI_SIGNAL_MAX                     (50)        /*�źſ�����ֵ*/
#define HAL_INVALID_SIGNAL_INITIAL              (1000)      /*�Ƿ���ʼ�źż���ֵ*/

#define HAL_CCA_OPT_ED_HIGH_20TH_DEF            (-62)
#define HAL_CCA_OPT_ED_HIGH_40TH_DEF            (-59)
#define HAL_CCA_OPT_ED_HIGH_80TH_DEF            (-56)        /* CCA 80M������޼Ĵ���Ĭ��ֵ */
#define HAL_CCA_OPT_ED_LOW_TH_DSSS_DEF          (-76)        /* DSSS�źŵ�Ĭ��Э������ */
#define HAL_CCA_OPT_ED_LOW_TH_OFDM_DEF          (-82)        /* OFDM�źŵ�Ĭ��Э������ */
#define HAL_CCA_OPT_ED_LOW_TH_40M_HIGH_DEF      (-72)        /* ��40M�����Ĭ��Э������ */
#define HAL_CCA_OPT_ED_LOW_TH_80M_HIGH_DEF      (-69)        /* ��80M�����Ĭ��Э������ */

#define HAL_RSSI_REF_DIFFERENCE        (20)
#define HAL_RADAR_REF_DIFFERENCE       (24)


#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#define HAL_CCA_OPT_GET_DEFAULT_ED_20TH(_band, _cust) \
    ((WLAN_BAND_2G == (_band)) ? \
     ((_cust)->c_delta_cca_ed_high_20th_2g + HAL_CCA_OPT_ED_HIGH_20TH_DEF) : \
     ((_cust)->c_delta_cca_ed_high_20th_5g + HAL_CCA_OPT_ED_HIGH_20TH_DEF))
#define HAL_CCA_OPT_GET_DEFAULT_ED_40TH(_band, _cust) \
    ((WLAN_BAND_2G == (_band)) ? \
     ((_cust)->c_delta_cca_ed_high_40th_2g + HAL_CCA_OPT_ED_HIGH_40TH_DEF) : \
     ((_cust)->c_delta_cca_ed_high_40th_5g + HAL_CCA_OPT_ED_HIGH_40TH_DEF))
#endif

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
#define GET_HAL_DYN_CALI_DISABLE(_pst_handler)  (0 == (_pst_handler)->pst_hal_dev_rom->st_hal_dmac_dyn_cali.st_dyn_cali_val.aus_cali_en_interval[(_pst_handler)->st_wifi_channel_status.uc_band]) /* ��ȡ��̬У׼�Ƿ�δʹ�� */
#endif
#define GET_HAL_DEVICE_WORK_STATUS(_pst_handler) (OAL_TRUE == (_pst_handler)->pst_hal_dev_rom->st_hal_dev_cb.p_mac_get_work_status(_pst_handler)) /* ��ȡ�Ƿ���״̬ */
#define GET_HAL_DEVICE_SCAN_STATUS(_pst_handler) (OAL_TRUE == (_pst_handler)->pst_hal_dev_rom->st_hal_dev_cb.p_mac_get_scan_status(_pst_handler)) /* ��ȡ�Ƿ���״̬ */

#ifdef _PRE_WLAN_FEATURE_BTCOEX
#define GET_HAL_BTCOEX_SW_PREEMPT_MODE(_pst_handler)       (*(oal_uint8*)&((_pst_handler)->st_btcoex_sw_preempt.st_sw_preempt_mode)) /* ��ȡ��ǰhal devcie��btcoex sw preemptģʽ */
#define GET_HAL_BTCOEX_SW_PREEMPT_TYPE(_pst_handler)       ((_pst_handler)->st_btcoex_sw_preempt.en_sw_preempt_type)                  /* ��ȡ��ǰhal devcie��btcoex sw preempt���� */
#define GET_HAL_BTCOEX_SW_PREEMPT_SUBTYPE(_pst_handler)    ((_pst_handler)->st_btcoex_sw_preempt.en_sw_preempt_subtype)               /* ��ȡ��ǰhal devcie��btcoex sw preempt������ */
#define GET_HAL_BTCOEX_SW_PREEMPT_PS_STOP(_pst_handler)    ((_pst_handler)->st_btcoex_sw_preempt.en_ps_stop)                          /* ��ȡ��ǰhal devcie��psҵ���Ƿ�򿪱�� */
#define GET_HAL_BTCOEX_SW_PREEMPT_PS_PAUSE(_pst_handler)    ((_pst_handler)->st_btcoex_sw_preempt.en_ps_pause)

#define HAL_BTCOEX_CHECK_SW_PREEMPT_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_btcoex_sw_preempt.st_sw_preempt_mode))& BIT0)) ? OAL_TRUE : OAL_FALSE)
#define HAL_BTCOEX_CHECK_SW_PREEMPT_DELBA_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_btcoex_sw_preempt.st_sw_preempt_mode))& BIT1)) ? OAL_TRUE : OAL_FALSE)
#define HAL_BTCOEX_CHECK_SW_PREEMPT_REPLY_CTS_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_btcoex_sw_preempt.st_sw_preempt_mode))& BIT2)) ? OAL_TRUE : OAL_FALSE)
#define HAL_BTCOEX_CHECK_SW_PREEMPT_RSP_FRAME_PS_ON(_pst_handler)   \
    ((0 != ((*(oal_uint8*)&((_pst_handler)->st_btcoex_sw_preempt.st_sw_preempt_mode))& BIT3)) ? OAL_TRUE : OAL_FALSE)

#define GET_BTCOEX_BT_STATUS(_pst_handler)     (&((_pst_handler)->st_btcoex_btble_status.un_bt_status.st_bt_status))  /* ��ȡ��ǰbt status */
#define GET_BTCOEX_BLE_STATUS(_pst_handler)    (&((_pst_handler)->st_btcoex_btble_status.un_ble_status.st_ble_status))  /* ��ȡ��ǰble status */
#define GET_BTCOEX_BT_LDAC_STATUS(_pst_handler)  ((&((_pst_handler)->st_btcoex_btble_status.un_ble_status.st_ble_status))->bit_bt_ldac) /* ��ȡ��ǰldac status */

#endif
#define GET_HAL_DEVICE_ROM(_pst_handler)                  ((hal_to_dmac_device_rom_stru *)((_pst_handler)->pst_hal_dev_rom))
#define HAL_DEVICE_STATE_IS_BUSY(_pst_hal_device)          (GET_HAL_DEVICE_WORK_STATUS(_pst_hal_device)||GET_HAL_DEVICE_SCAN_STATUS(_pst_hal_device))


/*****************************************************************************
  8 UNION����
*****************************************************************************/

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


/*****************************************************************************
  10 ��������
*****************************************************************************/
extern oal_int32 hal_main_init(oal_void);
extern oal_void  hal_main_exit(oal_void);
#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
typedef oal_uint8 (*hi1102_get_wifi_scan_state)(oal_uint32 *pst_end_time);
void hal_dmac_msg_handler_register(hi1102_get_wifi_scan_state pfMsgHandler);
#endif

#ifdef _PRE_WLAN_FEATURE_HUT
extern oal_void  hal_to_hut_irq_isr_register(hal_oper_mode_enum_uint8 en_oper_mode, oal_void (*p_func)(oal_void));
extern oal_void  hal_to_hut_irq_isr_unregister(oal_void);
#endif
extern oal_void  hal_get_hal_to_dmac_device(oal_uint8 uc_chip_id, oal_uint8 uc_device_id, hal_to_dmac_device_stru **ppst_hal_device);

/*****************************************************************************
  10.1 оƬ������
*****************************************************************************/
extern oal_uint32 hal_chip_get_chip(oal_uint8 uc_chip_id,  hal_to_dmac_chip_stru **ppst_chip_stru);
extern oal_uint32 hal_chip_get_device_num(oal_uint8 uc_chip_id, oal_uint8 * puc_device_nums);
extern oal_uint32 hal_chip_get_device(oal_uint8 uc_chip_id, oal_uint8 uc_device_id, hal_to_dmac_device_stru **ppst_device_stru);
extern hal_cipher_protocol_type_enum_uint8 hal_cipher_suite_to_ctype(wlan_ciper_protocol_type_enum_uint8 en_cipher_suite);
extern wlan_ciper_protocol_type_enum_uint8 hal_ctype_to_cipher_suite(hal_cipher_protocol_type_enum_uint8 en_cipher_type);
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

OAL_STATIC OAL_INLINE oal_void hal_rx_destroy_dscr_queue(hal_to_dmac_device_stru *pst_device,oal_uint8 uc_destroy_netbuf)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_destroy_dscr_queue)( pst_device,uc_destroy_netbuf);
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


OAL_STATIC OAL_INLINE oal_void hal_free_rx_isr_list(hal_to_dmac_device_stru * pst_hal_device, oal_dlist_head_stru * pst_rx_isr_list, oal_bool_enum_uint8 en_recycle)
{
    HAL_PUBLIC_HOOK_FUNC(_free_rx_isr_list)(pst_hal_device, pst_rx_isr_list, en_recycle);
}

OAL_STATIC OAL_INLINE oal_void hal_destroy_hw_rx_isr_list(hal_to_dmac_device_stru *pst_device)
{
    HAL_PUBLIC_HOOK_FUNC(_destroy_hw_rx_isr_list)( pst_device);
}

OAL_STATIC OAL_INLINE oal_void hal_free_rx_isr_info(hal_to_dmac_device_stru *pst_device, hal_hw_rx_dscr_info_stru *pst_rx_isr_info)
{
    oal_irq_disable();
    oal_dlist_add_tail(&(pst_rx_isr_info->st_dlist_head), &(pst_device->st_rx_isr_info_res_list));
    oal_irq_enable();
}

/*����������Ļ�����Ϣ������֡���ȡ�mac���ȡ���ȫ��Ϣ*/
OAL_STATIC OAL_INLINE oal_void hal_tx_fill_basic_ctrl_dscr(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru * p_tx_dscr, hal_tx_mpdu_stru *pst_mpdu)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_fill_basic_ctrl_dscr)( p_tx_dscr, pst_mpdu);
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
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_ctrl_one_param)( pst_tx_dscr, pst_tx_dscr_one_param);
}

OAL_STATIC OAL_INLINE oal_void hal_get_target_tx_power_by_tx_dscr(
                                                             hal_to_dmac_vap_stru              *pst_hal_vap,
                                                             hal_to_dmac_device_stru           *pst_hal_device,
                                                             hal_tx_dscr_stru                  *pst_tx_dscr,
                                                             hal_pdet_info_stru                *pst_pdet_info,
                                                             oal_uint8                          uc_channel_idx,
                                                             wlan_channel_bandwidth_enum_uint8  en_bandwidth,
                                                             oal_int16                         *ps_tx_pow)
{
    HAL_PUBLIC_HOOK_FUNC(_get_target_tx_power_by_tx_dscr)(pst_hal_vap, pst_hal_device, pst_tx_dscr, pst_pdet_info, uc_channel_idx, en_bandwidth, ps_tx_pow);
}
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
OAL_STATIC OAL_INLINE oal_void hal_rf_cali_realtime_entrance(
                                                         hal_to_dmac_vap_stru         * OAL_CONST pst_hal_vap,
                                                         hal_to_dmac_device_stru      * OAL_CONST pst_hal_device,
                                                         hal_pdet_info_stru           * OAL_CONST pst_pdet_info,
                                                         hal_dyn_cali_usr_record_stru * OAL_CONST pst_user_pow,
                                                         hal_tx_dscr_stru             * OAL_CONST pst_base_dscr)
{
     HAL_PUBLIC_HOOK_FUNC(_rf_cali_realtime_entrance)(pst_hal_vap, pst_hal_device, pst_pdet_info, pst_user_pow, pst_base_dscr);
}
OAL_STATIC OAL_INLINE oal_void hal_init_dyn_cali_tx_pow(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_init_dyn_cali_tx_pow)(pst_hal_device);
}
OAL_STATIC OAL_INLINE oal_uint32 hal_config_custom_dyn_cali(oal_uint8  *puc_param)
{
    return HAL_PUBLIC_HOOK_FUNC(_config_custom_dyn_cali)(puc_param);
}
OAL_STATIC OAL_INLINE oal_void hal_config_set_dyn_cali_dscr_interval(hal_to_dmac_device_stru * pst_hal_device, wlan_channel_band_enum_uint8 uc_band,oal_uint16 us_param_val)
{
     HAL_PUBLIC_HOOK_FUNC(_config_set_dyn_cali_dscr_interval)(pst_hal_device, uc_band, us_param_val);
}
OAL_STATIC OAL_INLINE oal_void hal_rf_init_dyn_cali_reg_conf(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_rf_init_dyn_cali_reg_conf)(pst_hal_device);
}
OAL_STATIC OAL_INLINE oal_void  hal_tx_set_pdet_en(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_bool_enum_uint8 en_pdet_en_flag)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_set_pdet_en)(pst_hal_device, pst_tx_dscr, en_pdet_en_flag);
}
OAL_STATIC OAL_INLINE oal_uint32 hal_dyn_cali_vdet_val_amend(hal_to_dmac_device_stru *pst_hal_device, hal_pdet_info_stru *pst_pdet_info)

{
    return HAL_PUBLIC_HOOK_FUNC(_dyn_cali_vdet_val_amend)(pst_hal_device, pst_pdet_info);
}

#endif //_PRE_WLAN_FIT_BASED_REALTIME_CALI
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_DEV))
/* ��ȡ��������ķ��ʹ��� */
OAL_STATIC OAL_INLINE oal_void hal_tx_get_bw_mode(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru *pst_dscr, wlan_bw_cap_enum_uint8 *pen_bw_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_bw_mode)(pst_hal_device, pst_dscr, pen_bw_mode);
}
#endif

OAL_STATIC OAL_INLINE oal_void  hal_tx_get_dscr_tx_cnt(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_tx_count)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_tx_cnt)( pst_tx_dscr, puc_tx_count);
}

OAL_STATIC OAL_INLINE oal_void  hal_tx_dscr_get_rate3(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_rate)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
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

#if 0

OAL_STATIC OAL_INLINE oal_void  hal_tx_enable_peer_sta_ps_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_enable_peer_sta_ps_ctrl)( pst_hal_device, uc_lut_index);
}

OAL_STATIC OAL_INLINE oal_void  hal_tx_disable_peer_sta_ps_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_disable_peer_sta_ps_ctrl)( pst_hal_device, uc_lut_index);
}

#endif


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


OAL_STATIC OAL_INLINE oal_void hal_get_tx_q_status(hal_to_dmac_device_stru * pst_hal_device, oal_uint32 * pul_status, oal_uint8 uc_qnum)
{
    HAL_PUBLIC_HOOK_FUNC(_get_tx_q_status)( pst_hal_device, pul_status, uc_qnum);
}


OAL_STATIC OAL_INLINE oal_void  hal_tx_get_ampdu_len(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru *pst_dscr, oal_uint32 *pul_ampdu_len)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_ampdu_len)( pst_hal_device, pst_dscr, pul_ampdu_len);
}
//#if defined(_PRE_DEBUG_MODE_USER_TRACK) || defined(_PRE_WLAN_FIT_BASED_REALTIME_CALI)

OAL_STATIC OAL_INLINE oal_void  hal_tx_get_protocol_mode(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru *pst_dscr, oal_uint8 *puc_protocol_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_protocol_mode)( pst_hal_device, pst_dscr, puc_protocol_mode);
}
//#endif

/*��ȡ������������С*/
OAL_STATIC OAL_INLINE oal_void hal_rx_get_size_dscr(hal_to_dmac_device_stru * pst_hal_device, oal_uint32 * pul_dscr_size)
{
    *pul_dscr_size = WLAN_RX_DSCR_SIZE;
}

/*��ȡ��������Ϣ*/
OAL_STATIC OAL_INLINE oal_void hal_rx_get_info_dscr(hal_to_dmac_device_stru * pst_hal_device, oal_uint32 *pul_rx_dscr, hal_rx_ctl_stru * pst_rx_ctl, hal_rx_status_stru * pst_rx_status, hal_rx_statistic_stru * pst_rx_statistics)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_get_info_dscr)( pul_rx_dscr, pst_rx_ctl, pst_rx_status, pst_rx_statistics);
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

OAL_STATIC OAL_INLINE oal_void hal_rx_sync_invalid_dscr(hal_to_dmac_device_stru * pst_hal_device, oal_uint32 *pul_dscr, oal_uint8 en_queue_num)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_sync_invalid_dscr)( pst_hal_device, pul_dscr, en_queue_num);
}

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


OAL_STATIC OAL_INLINE oal_void hal_set_counter_clear(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_set_counter_clear)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_reg_write16(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_addr, oal_uint16 us_val)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    HAL_PUBLIC_HOOK_FUNC(_reg_write16)( pst_hal_device, ul_addr, us_val);
#endif
}

#ifdef _PRE_WLAN_FEATURE_DATA_SAMPLE

OAL_STATIC OAL_INLINE oal_void hal_set_sample_memory(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 **ppul_addr, oal_uint32 *pul_reg_num)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_sample_memory)(pst_hal_device, ppul_addr, pul_reg_num);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_free_sample_mem(hal_to_dmac_device_stru *pst_hal_device)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    HAL_PUBLIC_HOOK_FUNC(_free_sample_mem)(pst_hal_device);
#endif
}



OAL_STATIC OAL_INLINE oal_void hal_set_pktmem_bus_access(hal_to_dmac_device_stru *pst_hal_device)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_pktmem_bus_access)(pst_hal_device);
#endif
}


OAL_STATIC OAL_INLINE oal_void hal_get_sample_state(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 *pus_reg_val)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    HAL_PUBLIC_HOOK_FUNC(_get_sample_state)(pst_hal_device, pus_reg_val);
#endif
}

#endif

OAL_STATIC OAL_INLINE oal_void  hal_set_machw_rx_buff_addr(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_rx_dscr, hal_rx_dscr_queue_id_enum_uint8 en_queue_num)
{
    HAL_PUBLIC_HOOK_FUNC(_set_machw_rx_buff_addr)( pst_hal_device, ul_rx_dscr, en_queue_num);
}


OAL_STATIC OAL_INLINE oal_uint32  hal_set_machw_rx_buff_addr_sync(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_rx_dscr, hal_rx_dscr_queue_id_enum_uint8 en_queue_num)
{
    return HAL_PUBLIC_HOOK_FUNC(_set_machw_rx_buff_addr_sync)( pst_hal_device, ul_rx_dscr, en_queue_num);
}

#if (_PRE_MULTI_CORE_MODE ==_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC)
#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1102A_DEV)
OAL_STATIC OAL_INLINE oal_void hal_get_pwr_comp_val(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_tx_ratio, oal_int16 * ps_pwr_comp_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_pwr_comp_val)( pst_hal_device, ul_tx_ratio, ps_pwr_comp_val);
}
#endif
OAL_STATIC OAL_INLINE oal_void hal_agc_threshold_handle(hal_to_dmac_device_stru *pst_hal_device, oal_int8 c_rssi)
{
    HAL_PUBLIC_HOOK_FUNC(_agc_threshold_handle)(pst_hal_device, c_rssi);
}


#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1102A_DEV)
OAL_STATIC OAL_INLINE oal_void hal_over_temp_handler(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_over_temp_handler)( pst_hal_device);
}
#endif

#endif


OAL_STATIC OAL_INLINE oal_void hal_rx_add_dscr(hal_to_dmac_device_stru *pst_hal_device, hal_rx_dscr_queue_id_enum_uint8 en_queue_num, oal_uint16 us_rx_dscr_num)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_add_dscr)( pst_hal_device, en_queue_num, us_rx_dscr_num);
}

OAL_STATIC OAL_INLINE oal_void hal_flush_tx_complete_irq(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_flush_tx_complete_irq)(pst_hal_device);
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

OAL_STATIC OAL_INLINE oal_bool_enum_uint8  hal_is_machw_enabled(hal_to_dmac_device_stru *pst_device)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    return HAL_PUBLIC_HOOK_FUNC(_is_machw_enabled)(pst_device);
#endif
}

/*****************************************************************************
  hal MAC hardware��ʼ���ӿ�
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  hal_initialize_machw(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_initialize_machw)( pst_hal_device);
}

/*****************************************************************************
  Ƶ�Ρ������ŵ�����ؽӿ�
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  hal_set_freq_band(hal_to_dmac_device_stru *pst_hal_device, wlan_channel_band_enum_uint8 en_band)
{
    HAL_PUBLIC_HOOK_FUNC(_set_freq_band)( pst_hal_device, en_band);
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
                hal_to_dmac_device_stru *pst_hal_device,
                oal_uint8                uc_channel_num,
                oal_uint8                uc_band,
                oal_uint8                uc_channel_idx,
                wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    HAL_PUBLIC_HOOK_FUNC(_set_primary_channel)( pst_hal_device, uc_channel_num, uc_band, uc_channel_idx, en_bandwidth);
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)

OAL_STATIC OAL_INLINE oal_void  hal_freq_adjust(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_pll_int, oal_uint16 us_pll_frac)
{
    HAL_PUBLIC_HOOK_FUNC(_freq_adjust)( pst_hal_device, us_pll_int, us_pll_frac);
}
#endif


OAL_STATIC OAL_INLINE oal_void  hal_add_machw_ba_lut_entry(hal_to_dmac_device_stru *pst_hal_device,
                oal_uint8 uc_lut_index, oal_uint8 *puc_dst_addr, oal_uint8 uc_tid,
                oal_uint16 uc_seq_no, oal_uint8 uc_win_size)
{
    HAL_PUBLIC_HOOK_FUNC(_add_machw_ba_lut_entry)( pst_hal_device,
                    uc_lut_index, puc_dst_addr, uc_tid, uc_seq_no, uc_win_size);
}


OAL_STATIC OAL_INLINE oal_void  hal_remove_machw_ba_lut_entry(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index)
{
    HAL_PUBLIC_HOOK_FUNC(_remove_machw_ba_lut_entry)( pst_hal_device, uc_lut_index);
}


OAL_STATIC OAL_INLINE oal_void  hal_get_machw_ba_params(hal_to_dmac_device_stru *pst_hal_device,oal_uint8 uc_index,
                                                        oal_uint32* pst_addr_h,oal_uint32* pst_addr_l,oal_uint32* pst_ba_para)
{
    HAL_PUBLIC_HOOK_FUNC(_get_machw_ba_params)( pst_hal_device, uc_index,pst_addr_h,pst_addr_l,pst_ba_para);
}


OAL_STATIC OAL_INLINE oal_void  hal_restore_machw_ba_params(hal_to_dmac_device_stru *pst_hal_device,oal_uint8 uc_index,
                                                oal_uint32 ul_addr_h,oal_uint32 ul_addr_l,oal_uint32 ul_ba_para)
{
    HAL_PUBLIC_HOOK_FUNC(_restore_machw_ba_params)( pst_hal_device, uc_index,ul_addr_h,ul_addr_l,ul_ba_para);
}

/*****************************************************************************
          RA LUT������ؽӿ�
*****************************************************************************/


OAL_STATIC OAL_INLINE oal_void  hal_set_tx_sequence_num(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index,oal_uint8 uc_tid, oal_uint8 uc_qos_flag,oal_uint32 ul_val_write)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tx_sequence_num)( pst_hal_device,uc_lut_index,uc_tid,uc_qos_flag, ul_val_write);
}


OAL_STATIC OAL_INLINE oal_void  hal_get_tx_sequence_num(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index,oal_uint8 uc_tid, oal_uint8 uc_qos_flag,oal_uint16 *pus_val_read )
{
    HAL_PUBLIC_HOOK_FUNC(_get_tx_sequence_num)( pst_hal_device, uc_lut_index,uc_tid,uc_qos_flag,pus_val_read);
}


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


OAL_STATIC OAL_INLINE oal_void hal_phy_update_scaling_reg(oal_uint8 *puc_dbb_scaling)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    HAL_PUBLIC_HOOK_FUNC(_phy_update_scaling_reg)(puc_dbb_scaling);
#endif
}


OAL_STATIC OAL_INLINE oal_void  hal_initialize_phy(hal_to_dmac_device_stru * pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_initialize_phy)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void  hal_radar_config_reg(hal_to_dmac_device_stru *pst_hal_device, hal_dfs_radar_type_enum_uint8 en_dfs_domain)
{
    HAL_PUBLIC_HOOK_FUNC(_radar_config_reg)( pst_hal_device, en_dfs_domain);
}

/*****************************************************************************
  RF��ؽӿ�
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  hal_initialize_rf_sys(hal_to_dmac_device_stru * pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_initialize_rf_sys)( pst_hal_device);
}

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

OAL_STATIC OAL_INLINE oal_void  hal_set_rf_custom_reg(hal_to_dmac_device_stru * pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rf_custom_reg)( pst_hal_device);
}
#endif

OAL_STATIC OAL_INLINE oal_void  hal_psm_rf_sleep(hal_to_dmac_device_stru * pst_hal_device, oal_uint8 uc_restore_reg)
{
    HAL_PUBLIC_HOOK_FUNC(_psm_rf_sleep)( pst_hal_device, uc_restore_reg);
}

OAL_STATIC OAL_INLINE oal_void  hal_psm_rf_awake(hal_to_dmac_device_stru  *pst_hal_device,oal_uint8 uc_restore_reg)
{
    HAL_PUBLIC_HOOK_FUNC(_psm_rf_awake)( pst_hal_device,uc_restore_reg);
}




#ifdef _PRE_WLAN_REALTIME_CALI

OAL_STATIC OAL_INLINE oal_void hal_rf_cali_realtime(hal_to_dmac_device_stru * pst_hal_device, oal_uint8 uc_freq,
                                                           oal_uint8 uc_chn_idx, oal_uint8 uc_bandwidth, oal_uint8 uc_protocol)
{
     HAL_PUBLIC_HOOK_FUNC(_rf_cali_realtime)( pst_hal_device, uc_freq, uc_chn_idx, uc_bandwidth, uc_protocol);
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


/* ERROR IRQ�жϼĴ�����ز��� */
OAL_STATIC OAL_INLINE oal_void  hal_unmask_mac_error_init_status(hal_to_dmac_device_stru * pst_hal_device, hal_error_state_stru *pst_status)
{
    HAL_PUBLIC_HOOK_FUNC(_unmask_mac_error_init_status)( pst_hal_device, pst_status);
}

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

#ifdef _PRE_WLAN_RF_110X_CALI_DPD
typedef enum
{
  HAL_DPD_CALI_START        = 0,
  HAL_DPD_CALI_STATUS       = 1,
  HAL_DPD_CALI_DATA_READ   = 2,
  HAL_DPD_CALI_DATA_WRITE  = 3,

}hal_dpd_cali_enum;


#define HI1102_DMAC_DPD_CALI_LUT_LENGTH 128
#endif


OAL_STATIC OAL_INLINE oal_void hal_cali_send_func(oal_uint8* puc_cal_data_write, oal_uint16 us_frame_len, oal_uint16 us_remain)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
  HAL_PUBLIC_HOOK_FUNC(_cali_send_func)( puc_cal_data_write, us_frame_len, us_remain);
#endif
}

OAL_STATIC OAL_INLINE oal_void hal_cali_matrix_data_send_func(oal_uint8* puc_cal_data_write, oal_uint16 us_frame_len, oal_uint16 us_remain)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
  HAL_PUBLIC_HOOK_FUNC(_cali_matrix_data_send_func)(puc_cal_data_write, us_frame_len, us_remain);
#endif
}

/*
����halά����vap id����ȡhal vap
*/
OAL_STATIC OAL_INLINE oal_void hal_get_vap(hal_to_dmac_device_stru * pst_hal_device, wlan_vap_mode_enum_uint8 vap_mode, oal_uint8 vap_id, hal_to_dmac_vap_stru ** ppst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_get_vap)( pst_hal_device, vap_mode, vap_id, ppst_hal_vap);
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
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    HAL_PUBLIC_HOOK_FUNC(_reg_info16)( pst_hal_device, ul_addr, pus_val);
#endif
}
OAL_STATIC OAL_INLINE oal_void hal_get_all_tx_q_status(hal_to_dmac_device_stru *pst_hal_device,oal_uint32 *pul_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_all_tx_q_status)( pst_hal_device, pul_val);
}


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



OAL_STATIC OAL_INLINE oal_void  hal_mac_error_msg_report(hal_to_dmac_device_stru *pst_hal_device, hal_mac_error_type_enum_uint8 en_error_type)
{
    HAL_PUBLIC_HOOK_FUNC(_mac_error_msg_report)( pst_hal_device, en_error_type);
}


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

#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_DEV))
OAL_STATIC  OAL_INLINE oal_void hal_disable_non_frame_mgmt_filter(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_disable_non_frame_mgmt_filter)( pst_hal_device);
}
#endif

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

#ifdef _PRE_WLAN_INIT_PTK_TX_PN

OAL_STATIC OAL_INLINE oal_void hal_tx_get_dscr_phy_mode_one(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr,  oal_uint32 *pul_phy_mode_one)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_dscr_phy_mode_one)( pst_tx_dscr, pul_phy_mode_one);
}


OAL_STATIC OAL_INLINE oal_void hal_tx_get_ra_lut_index(hal_to_dmac_device_stru * pst_hal_device, hal_tx_dscr_stru *pst_dscr, oal_uint8 *puc_ra_lut_index)
{
    HAL_PUBLIC_HOOK_FUNC(_tx_get_ra_lut_index)( pst_hal_device, pst_dscr, puc_ra_lut_index);
}


OAL_STATIC  OAL_INLINE oal_void hal_init_ptk_tx_pn(hal_to_dmac_device_stru *pst_hal_device,hal_security_key_stru *pst_security_key)
{
    HAL_PUBLIC_HOOK_FUNC(_init_ptk_tx_pn)( pst_hal_device,pst_security_key);
}
#endif


OAL_STATIC  OAL_INLINE oal_void  hal_disable_ce(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_disable_ce)( pst_hal_device);
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

OAL_STATIC  OAL_INLINE oal_void hal_dscr_set_iv_value(hal_tx_dscr_stru *pst_tx_dscr, oal_uint32 ul_iv_ls_word, oal_uint32 ul_iv_ms_word)
{
    HAL_PUBLIC_HOOK_FUNC(_dscr_set_iv_value)( pst_tx_dscr, ul_iv_ls_word, ul_iv_ms_word);
}

OAL_STATIC  OAL_INLINE oal_void hal_dscr_set_tx_pn_hw_bypass(hal_tx_dscr_stru *pst_tx_dscr, oal_bool_enum_uint8 en_bypass)
{
    HAL_PUBLIC_HOOK_FUNC(_dscr_set_tx_pn_hw_bypass)( pst_tx_dscr, en_bypass);
}

OAL_STATIC  OAL_INLINE oal_void hal_dscr_get_tx_pn_hw_bypass(hal_tx_dscr_stru *pst_tx_dscr, oal_bool_enum_uint8 *pen_bypass)
{
    HAL_PUBLIC_HOOK_FUNC(_dscr_get_tx_pn_hw_bypass)( pst_tx_dscr, pen_bypass);
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


OAL_STATIC OAL_INLINE oal_void hal_get_ch_measurement_result(hal_to_dmac_device_stru *pst_hal_device, hal_ch_statics_irq_event_stru *pst_ch_statics)
{
#ifdef _PRE_WLAN_FEATURE_DFS
    HAL_PUBLIC_HOOK_FUNC(_get_ch_measurement_result)( pst_hal_device, pst_ch_statics);
#endif
}


OAL_STATIC OAL_INLINE oal_void hal_enable_radar_det(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_enable)
{
#ifdef _PRE_WLAN_FEATURE_DFS
    HAL_PUBLIC_HOOK_FUNC(_enable_radar_det)( pst_hal_device, uc_enable);
#endif
}

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

#ifdef _PRE_WLAN_FEATURE_TPC

//OAL_STATIC  OAL_INLINE oal_void hal_set_tpc_params(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_band, oal_uint8 uc_channel_num)
//{
    //HAL_PUBLIC_HOOK_FUNC(_set_bcn_phy_tx_mode, pst_hal_device, uc_band, uc_channel_num);
//}


OAL_STATIC  OAL_INLINE oal_void  hal_get_rf_temp(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_cur_temp)
{
    HAL_PUBLIC_HOOK_FUNC(_get_rf_temp)( pst_hal_device, puc_cur_temp);
}


#ifdef _PRE_WLAN_REALTIME_CALI

OAL_STATIC  OAL_INLINE oal_void  hal_get_tpc_cali_upc_gain_in_rf_list(oal_int8* pac_upc_gain_in_rf_list)
{
    HAL_PUBLIC_HOOK_FUNC(_get_tpc_cali_upc_gain_in_rf_list)( pac_upc_gain_in_rf_list);
}
#endif


OAL_STATIC  OAL_INLINE oal_void hal_set_spec_frm_phy_tx_mode(hal_to_dmac_device_stru *pst_hal_device,
                    oal_uint8 uc_band, oal_uint8 uc_subband_idx)

{
    HAL_PUBLIC_HOOK_FUNC(_set_spec_frm_phy_tx_mode)( pst_hal_device, uc_band, uc_subband_idx);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_tpc_delay_reg_param(hal_to_dmac_device_stru *pst_hal_device,
            oal_uint32 ul_phy_tx_up_down_time_reg,  oal_uint32 ul_phy_rx_up_down_time_reg,
            oal_uint32 ul_rf_reg_wr_delay1, oal_uint32 ul_rf_reg_wr_delay2)

{
    HAL_PUBLIC_HOOK_FUNC(_set_tpc_delay_reg_param)( pst_hal_device, ul_phy_tx_up_down_time_reg,
                        ul_phy_rx_up_down_time_reg, ul_rf_reg_wr_delay1, ul_rf_reg_wr_delay2);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_tpc_delay_reg_param(hal_to_dmac_device_stru *pst_hal_device,
            oal_uint32 *pul_phy_tx_up_down_time_reg,  oal_uint32 *pul_phy_rx_up_down_time_reg,
            oal_uint32 *pul_rf_reg_wr_delay1, oal_uint32 *pul_rf_reg_wr_delay2)

{
    HAL_PUBLIC_HOOK_FUNC(_get_tpc_delay_reg_param)( pst_hal_device, pul_phy_tx_up_down_time_reg,
                        pul_phy_rx_up_down_time_reg, pul_rf_reg_wr_delay1, pul_rf_reg_wr_delay2);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_tpc_rf_reg_param(hal_to_dmac_device_stru *pst_hal_device,
  oal_uint16 *pus_dac_val, oal_uint16 *pus_pa_val, oal_uint16 *pus_lpf_val,
  oal_uint16* paus_2g_upc_val, oal_uint16* paus_5g_upc_val, oal_uint8 uc_chain_idx)

{
    HAL_PUBLIC_HOOK_FUNC(_get_tpc_rf_reg_param)( pst_hal_device, pus_dac_val, pus_pa_val,pus_lpf_val,paus_2g_upc_val
        ,paus_5g_upc_val, uc_chain_idx);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_tpc_rf_reg_param(hal_to_dmac_device_stru *pst_hal_device,
  oal_uint16 us_dac_val, oal_uint16 us_pa_val, oal_uint16 us_lpf_val,
  oal_uint16* paus_2g_upc_val, oal_uint16* paus_5g_upc_val, oal_uint8 uc_chain_idx)

{
    HAL_PUBLIC_HOOK_FUNC(_set_tpc_rf_reg_param)( pst_hal_device, us_dac_val, us_pa_val,us_lpf_val,paus_2g_upc_val
        ,paus_5g_upc_val, uc_chain_idx);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_dpd_by_power(hal_tx_txop_rate_params_stru *pst_rate, oal_uint8 uc_power_level, oal_uint32 ul_dpd_configure, oal_uint32 ul_rate_idx)
{

    HAL_PUBLIC_HOOK_FUNC(_set_dpd_by_power)( pst_rate, uc_power_level, ul_dpd_configure, ul_rate_idx);
}


#endif


OAL_STATIC  OAL_INLINE oal_void hal_set_tpc_ctrl_reg_param(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_tpc_ctrl_param)

{
    HAL_PUBLIC_HOOK_FUNC(_set_tpc_ctrl_reg_param)( pst_hal_device, ul_tpc_ctrl_param);
}


OAL_STATIC  OAL_INLINE oal_void  hal_get_bcn_rate(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_data_rate)
{
    HAL_PUBLIC_HOOK_FUNC(_get_bcn_rate)( pst_hal_vap,puc_data_rate);
}


OAL_STATIC  OAL_INLINE oal_void  hal_set_tpc_phy_reg_param(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tpc_phy_reg_param)( pst_hal_device);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_bcn_phy_tx_mode(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_tpc_code)

{
    HAL_PUBLIC_HOOK_FUNC(_set_bcn_phy_tx_mode)( pst_hal_vap, uc_tpc_code);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_spec_frm_rate(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_get_spec_frm_rate)( pst_hal_device);
}


OAL_STATIC  OAL_INLINE oal_void  hal_set_dac_lpc_pa_upc_level(oal_uint8 uc_dac_lpf_code,
        oal_int8   *pac_tpc_level_table, oal_uint8 uc_tpc_level_num,
        oal_uint8 *pauc_dac_lpf_pa_code_table, oal_int16 *pas_upc_gain_table,oal_int16 *pas_other_gain_table,
        wlan_channel_band_enum_uint8 en_freq_band)
{
    HAL_PUBLIC_HOOK_FUNC(_set_dac_lpc_pa_upc_level)( uc_dac_lpf_code, pac_tpc_level_table,
                        uc_tpc_level_num, pauc_dac_lpf_pa_code_table,pas_upc_gain_table,pas_other_gain_table,en_freq_band);
}


OAL_STATIC  OAL_INLINE oal_void  hal_get_subband_index(wlan_channel_band_enum_uint8 en_band, oal_uint8 uc_channel_num, oal_uint8 *puc_subband_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_get_subband_index)( en_band, uc_channel_num, puc_subband_idx);
}

OAL_STATIC  OAL_INLINE oal_void  hal_get_legacy_data_rate_idx(oal_uint8 uc_rate,  oal_uint8 *puc_rate_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_get_legacy_data_rate_idx)(uc_rate, puc_rate_idx);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_upc_data(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_band, oal_uint8 uc_subband_idx)
{

    HAL_PUBLIC_HOOK_FUNC(_set_upc_data)( pst_hal_device, uc_band, uc_subband_idx);
}


OAL_STATIC  OAL_INLINE oal_void  hal_get_pow_index(
                                   hal_user_pow_info_stru     *pst_user_power_info,
                                   oal_uint8                   uc_cur_rate_pow_idx,
                                   hal_tx_txop_tx_power_stru  *pst_tx_power,
                                   oal_uint8                  *puc_pow_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_get_pow_index)(pst_user_power_info, uc_cur_rate_pow_idx, pst_tx_power, puc_pow_idx);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_band_spec_frame_tx_power(
                                                 hal_to_dmac_device_stru *pst_hal_device,
                                                 wlan_channel_band_enum_uint8 en_band,
                                                 oal_uint8 uc_chan_num,
                                                 hal_rate_tpc_code_gain_table_stru *pst_rate_tpc_table,
                                                 oal_uint8 uc_pow_level_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_set_band_spec_frame_tx_power)(pst_hal_device, en_band, uc_chan_num, pst_rate_tpc_table, uc_pow_level_idx);
}


OAL_STATIC  OAL_INLINE oal_void hal_pow_update_upc_code(
                                                    hal_to_dmac_vap_stru              *pst_hal_vap,
                                                    hal_to_dmac_device_stru           *pst_hal_device,
                                                    oal_uint8                          uc_cur_ch_num,
                                                    wlan_channel_bandwidth_enum_uint8  en_bandwidth)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_update_upc_code)(pst_hal_vap, pst_hal_device, uc_cur_ch_num, en_bandwidth);
}


OAL_STATIC  OAL_INLINE oal_void hal_pow_update_p2p_upc(
                           hal_to_dmac_device_stru            *pst_hal_device,
                           wlan_channel_band_enum_uint8        en_freq_band,
                           oal_uint8                           uc_ch_num,
                           wlan_channel_bandwidth_enum_uint8   en_bandwidth)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_update_p2p_upc)(pst_hal_device, en_freq_band, uc_ch_num, en_bandwidth);
}


OAL_STATIC  OAL_INLINE oal_void hal_pow_set_rf_regctl_enable(hal_to_dmac_device_stru *pst_hal_dev, oal_bool_enum_uint8 en_rf_regctl)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_set_rf_regctl_enable)(pst_hal_dev, en_rf_regctl);
}

OAL_STATIC  OAL_INLINE oal_void hal_pow_init_tx_power(hal_to_dmac_device_stru *pst_hal_dev)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_init_tx_power)(pst_hal_dev);
}

OAL_STATIC  OAL_INLINE oal_void hal_pow_get_rf_dev_base_power(
                                              hal_to_dmac_device_stru       *pst_hal_device,
                                              wlan_channel_band_enum_uint8   en_freq_band,
                                              oal_uint8                     *puc_base_power,
                                              oal_uint8                      uc_subband_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_get_rf_dev_base_power)(pst_hal_device, en_freq_band, puc_base_power, uc_subband_idx);
}

OAL_STATIC  OAL_INLINE oal_void hal_get_rate_limit_tx_power(
                                        hal_to_dmac_device_stru           *pst_hal_device,
                                        oal_uint8                          uc_rate_idx,
                                        oal_uint8                          uc_base_pow,
                                        wlan_channel_band_enum_uint8       en_freq_band,
                                        oal_uint8                         *puc_rate_txpwr_limit)
{
    HAL_PUBLIC_HOOK_FUNC(_get_rate_limit_tx_power)(pst_hal_device, uc_rate_idx, uc_base_pow, en_freq_band, puc_rate_txpwr_limit);
}

OAL_STATIC OAL_INLINE oal_void hal_irq_affinity_init(hal_to_dmac_device_stru * pst_device, oal_uint32 ul_core_id)
{
    HAL_PUBLIC_HOOK_FUNC(_irq_affinity_init)( pst_device, ul_core_id);
}


OAL_STATIC OAL_INLINE oal_void hal_set_peer_lut_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 auc_mac_addr[], oal_uint8 uc_lut_index)
{
    HAL_PUBLIC_HOOK_FUNC(_set_peer_lut_info)( pst_hal_device, auc_mac_addr, uc_lut_index);
}

#ifdef _PRE_WLAN_FEATURE_TXBF

OAL_STATIC  OAL_INLINE oal_void hal_set_legacy_matrix_buf_pointer(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_matrix)
{
    HAL_PUBLIC_HOOK_FUNC(_set_legacy_matrix_buf_pointer)( pst_hal_device, ul_matrix);
}

OAL_STATIC  OAL_INLINE oal_void hal_get_legacy_matrix_buf_pointer(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_matrix)
{
    HAL_PUBLIC_HOOK_FUNC(_get_legacy_matrix_buf_pointer)( pst_hal_device, pul_matrix);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_dl_mumimo_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_dl_mumimo_ctrl)( pst_hal_device, ul_value);
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

OAL_STATIC  OAL_INLINE oal_void hal_set_ndp_group_id(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_group_id)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ndp_group_id)( pst_hal_device, ul_group_id);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_ndp_partial_aid(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ndp_partial_aid)( pst_hal_device, us_reg_value);
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

OAL_STATIC  OAL_INLINE oal_void hal_set_ht_buffer_num(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ht_buffer_num)( pst_hal_device, ul_reg_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_ht_buffer_step(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ht_buffer_step)( pst_hal_device, ul_reg_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_ht_buffer_pointer(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ht_buffer_pointer)( pst_hal_device, ul_reg_value);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_h_matrix_timeout(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_h_matrix_timeout)( pst_hal_device, ul_reg_value);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_mu_aid_matrix_info(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_aid, oal_uint8  *p_matrix)
{
    HAL_PUBLIC_HOOK_FUNC(_set_mu_aid_matrix_info)( pst_hal_vap, us_aid, p_matrix);
}


OAL_STATIC  OAL_INLINE oal_void hal_delete_txbf_lut_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index)
{
    HAL_PUBLIC_HOOK_FUNC(_delete_txbf_lut_info)( pst_hal_device, uc_lut_index);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_txbf_lut_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index, oal_uint16 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_txbf_lut_info)( pst_hal_device, uc_lut_index, ul_reg_value);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_txbf_lut_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index, oal_uint32*  pst_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_get_txbf_lut_info)( pst_hal_device, uc_lut_index, pst_reg_value);
}
#endif

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

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT

OAL_STATIC  OAL_INLINE oal_void hal_set_counter1_clear(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_set_counter1_clear)( pst_hal_device);
}


OAL_STATIC  OAL_INLINE oal_void hal_get_txrx_frame_time(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *ul_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_txrx_frame_time)( pst_hal_device, ul_reg_val);
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

#ifdef _PRE_WLAN_FEATURE_CCA_OPT

OAL_STATIC  OAL_INLINE oal_void hal_get_ed_high_th(hal_to_dmac_device_stru *pst_hal_device, oal_int8 *l_ed_high_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ed_high_th)( pst_hal_device, l_ed_high_reg_val);
}


OAL_STATIC  OAL_INLINE oal_void hal_set_ed_high_th(hal_to_dmac_device_stru *pst_hal_device, oal_int32 l_ed_high_20_reg_val, oal_int32 l_ed_high_40_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_set_ed_high_th)( pst_hal_device, l_ed_high_20_reg_val, l_ed_high_40_reg_val);
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


OAL_STATIC  OAL_INLINE oal_void hal_get_cca_reg_th(hal_to_dmac_device_stru *pst_hal_device, oal_int8 *ac_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_cca_reg_th)( pst_hal_device, ac_reg_val);
}

#endif



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


OAL_STATIC  OAL_INLINE oal_void  hal_set_txop_ps_partial_aid(hal_to_dmac_vap_stru  *pst_hal_vap, oal_uint16 us_partial_aid)
{
    HAL_PUBLIC_HOOK_FUNC(_set_txop_ps_partial_aid)( pst_hal_vap, us_partial_aid);
}

#endif

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
#endif


OAL_STATIC  OAL_INLINE oal_void hal_set_tx_dscr_field(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_data, hal_rf_test_sect_enum_uint8 en_sect)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tx_dscr_field)( pst_hal_device, ul_data, en_sect);
}


OAL_STATIC  OAL_INLINE oal_void hal_rf_test_disable_al_tx(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_rf_test_disable_al_tx)( pst_hal_device);
}


OAL_STATIC  OAL_INLINE oal_void hal_rf_test_enable_al_tx(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru * pst_tx_dscr)
{
    HAL_PUBLIC_HOOK_FUNC(_rf_test_enable_al_tx)( pst_hal_device, pst_tx_dscr);
}

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX

OAL_STATIC  OAL_INLINE oal_void hal_al_tx_set_agc_phy_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_value)
{
    HAL_PUBLIC_HOOK_FUNC(_al_tx_set_agc_phy_reg)( pst_hal_device, ul_value);
}
#endif

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
#ifdef _PRE_WLAN_HW_TEST

OAL_STATIC OAL_INLINE oal_void hal_set_dac_lpf_gain(hal_to_dmac_device_stru *pst_hal_device,
                                                 oal_uint8 en_band,
                                                 oal_uint8 en_bandwidth,
                                                 oal_uint8 en_protocol_mode,
                                                 oal_uint8 en_rate)
{
    HAL_PUBLIC_HOOK_FUNC(_set_dac_lpf_gain)( pst_hal_device, en_band, en_bandwidth, en_protocol_mode, en_rate);
}
#endif

#if (_PRE_MULTI_CORE_MODE==_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC)

OAL_STATIC OAL_INLINE oal_void hal_set_dac_lpf_gain(hal_to_dmac_device_stru *pst_hal_device,
                                                 oal_uint8 en_band,
                                                 oal_uint8 en_bandwidth,
                                                 oal_uint8 en_protocol_mode,
                                                 oal_uint8 en_rate)
{
    HAL_PUBLIC_HOOK_FUNC(_set_dac_lpf_gain)( pst_hal_device, en_band, en_bandwidth, en_protocol_mode, en_rate);
}
#endif

OAL_STATIC  OAL_INLINE oal_void hal_set_rx_filter(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_rx_filter_val)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rx_filter)( pst_hal_device, ul_rx_filter_val);
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


OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_txop_limit_vivo(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_vo, oal_uint16 us_vi)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_txop_limit_vivo)( pst_hal_vap, us_vo, us_vi);
}

OAL_STATIC OAL_INLINE oal_void hal_set_txop_check_cca(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 en_txop_check_cca)
{
    HAL_PUBLIC_HOOK_FUNC(_set_txop_check_cca)( pst_hal_vap,en_txop_check_cca);
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

OAL_STATIC OAL_INLINE oal_void  hal_vap_start_tsf(hal_to_dmac_vap_stru *pst_hal_vap, oal_bool_enum_uint8 en_dbac_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_start_tsf)( pst_hal_vap, en_dbac_enable);
}


OAL_STATIC OAL_INLINE oal_void  hal_vap_read_tbtt_timer(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 *pul_value)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_read_tbtt_timer)( pst_hal_vap, pul_value);
}

OAL_STATIC OAL_INLINE oal_void  hal_vap_write_tbtt_timer(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_value)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_write_tbtt_timer)( pst_hal_vap, ul_value);
}



OAL_STATIC OAL_INLINE oal_void  hal_vap_set_machw_tsf_disable(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_vap_set_machw_tsf_disable)( pst_hal_vap);
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
#ifdef _PRE_WLAN_MAC_BUGFIX_TSF_SYNC

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
        pst_tx_dscr_param->uc_last_rate_rank = 0;
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

    pst_tx_dscr_param->ast_per_rate[pst_tx_dscr_param->uc_last_rate_rank].rate_bit_stru.bit_tx_count = uc_retry;

}



OAL_STATIC OAL_INLINE oal_void  hal_get_random_byte(oal_uint8 *puc_byte)
{
    /* wep���� */
    *puc_byte = oal_get_random();
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


OAL_STATIC  OAL_INLINE oal_void hal_pm_wlan_servid_register(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 *pul_ret)
{
    HAL_PUBLIC_HOOK_FUNC(_pm_wlan_servid_register)( pst_hal_vap, pul_ret);
}

OAL_STATIC  OAL_INLINE oal_void hal_pm_wlan_servid_unregister(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_pm_wlan_servid_unregister)( pst_hal_vap);
}


OAL_STATIC  OAL_INLINE oal_void hal_pm_enable_front_end(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_enable_paldo)
{
    HAL_PUBLIC_HOOK_FUNC(_pm_enable_front_end)(pst_hal_device, uc_enable_paldo);
}

OAL_STATIC  OAL_INLINE oal_void hal_dyn_set_tbtt_offset(   hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_dyn_set_tbtt_offset)(pst_hal_vap);
}


OAL_STATIC  OAL_INLINE oal_void hal_enable_sta_tsf_tbtt(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_enable_sta_tsf_tbtt)( pst_hal_vap);
}


OAL_STATIC  OAL_INLINE oal_void hal_disable_sta_tsf_tbtt(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_disable_sta_tsf_tbtt)( pst_hal_vap);
}


OAL_STATIC OAL_INLINE oal_void hal_cfg_slottime_type(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_slottime_type)
{
    HAL_PUBLIC_HOOK_FUNC(_cfg_slottime_type)( pst_hal_device, ul_slottime_type);
}

OAL_STATIC  OAL_INLINE oal_void hal_set_cca_prot_th(hal_to_dmac_device_stru *pst_hal_device, oal_int8 c_ed_low_th_dsss_reg_val, oal_int8 c_ed_low_th_ofdm_reg_val)
{
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1102A_DEV)
    HAL_PUBLIC_HOOK_FUNC(_set_cca_prot_th)(pst_hal_device, c_ed_low_th_dsss_reg_val, c_ed_low_th_ofdm_reg_val);
#endif
}
/*****************************************************************************
  10.4 SDT��д�Ĵ�������
*****************************************************************************/

OAL_STATIC  OAL_INLINE oal_void hal_mwo_det_enable_mac_counter(hal_to_dmac_device_stru *pst_hal_device, oal_int32 l_enable_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_mwo_det_enable_mac_counter)( pst_hal_device, l_enable_reg_val);
}


#ifdef _PRE_WLAN_FEATURE_BTCOEX
OAL_STATIC OAL_INLINE oal_void hal_coex_sw_irq_set(oal_uint8 uc_irq_en)
{
    HAL_PUBLIC_HOOK_FUNC(_coex_sw_irq_set)( uc_irq_en);
}
OAL_STATIC OAL_INLINE oal_void hal_set_rx_rsp_other_pri_mode(hal_coex_priority_type_uint8 en_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rx_rsp_other_pri_mode)(en_mode);
}
OAL_STATIC OAL_INLINE oal_void hal_set_rx_rsp_cts_pri_mode(hal_coex_priority_type_uint8 en_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rx_rsp_cts_pri_mode)(en_mode);
}
OAL_STATIC OAL_INLINE oal_void hal_set_rx_rsp_ba_pri_mode(hal_coex_priority_type_uint8 en_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rx_rsp_ba_pri_mode)(en_mode);
}
OAL_STATIC OAL_INLINE oal_void hal_set_tx_rsp_other_pri_mode(hal_coex_priority_type_uint8 en_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tx_rsp_other_pri_mode)(en_mode);
}
OAL_STATIC OAL_INLINE oal_void hal_set_tx_rsp_cts_pri_mode(hal_coex_priority_type_uint8 en_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tx_rsp_cts_pri_mode)(en_mode);
}
OAL_STATIC OAL_INLINE oal_void hal_set_tx_rsp_ba_pri_mode(hal_coex_priority_type_uint8 en_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tx_rsp_ba_pri_mode)(en_mode);
}
OAL_STATIC OAL_INLINE oal_void hal_set_tx_one_pkt_pri_mode(hal_coex_priority_type_uint8 en_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tx_one_pkt_pri_mode)(en_mode);
}
OAL_STATIC OAL_INLINE oal_void hal_get_btcoex_abort_qos_null_seq_num(oal_uint32 *ul_qosnull_seq_num)
{
    HAL_PUBLIC_HOOK_FUNC(_get_btcoex_abort_qos_null_seq_num)( ul_qosnull_seq_num);
}
OAL_STATIC OAL_INLINE oal_void hal_get_btcoex_occupied_period(oal_uint16 *ul_occupied_period)
{
    HAL_PUBLIC_HOOK_FUNC(_get_btcoex_occupied_period)( ul_occupied_period);
}
OAL_STATIC OAL_INLINE oal_void hal_get_btcoex_pa_status(oal_uint32 *ul_pa_status)
{
    HAL_PUBLIC_HOOK_FUNC(_get_btcoex_pa_status)( ul_pa_status);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_abort_null_buff_addr(oal_uint32 ul_abort_null_buff_addr)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_abort_null_buff_addr)( ul_abort_null_buff_addr);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_abort_qos_null_seq_num(oal_uint32 ul_qosnull_seq_num)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_abort_qos_null_seq_num)( ul_qosnull_seq_num);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_hw_rx_priority_dis(oal_uint8 uc_hw_rx_prio_dis)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_hw_rx_priority_dis)( uc_hw_rx_prio_dis);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_hw_priority_en(oal_uint8 uc_hw_prio_en)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_hw_priority_en)( uc_hw_prio_en);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_get_rf_control(oal_uint16 ul_occupied_period, oal_uint32 *pul_wlbt_mode_sel, oal_uint16 us_wait_cnt)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_get_rf_control)( ul_occupied_period, pul_wlbt_mode_sel, us_wait_cnt);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_wait_bt_release_pa(oal_uint16 ul_period)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_wait_bt_release_pa)(ul_period);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_occupied_period(oal_uint16 ul_occupied_period)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_occupied_period)( ul_occupied_period);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_sw_all_abort_ctrl(oal_uint8 uc_sw_abort_ctrl)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_sw_all_abort_ctrl)( uc_sw_abort_ctrl);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_sw_priority_flag(oal_uint8 uc_sw_prio_flag)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_sw_priority_flag)( uc_sw_prio_flag);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_soc_gpreg0(oal_uint8 uc_val, oal_uint16 us_mask, oal_uint8 uc_offset)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_soc_gpreg0)( uc_val, us_mask, uc_offset);
}
OAL_STATIC OAL_INLINE oal_void hal_set_btcoex_soc_gpreg1(oal_uint8 uc_val, oal_uint16 us_mask, oal_uint8 uc_offset)
{
    HAL_PUBLIC_HOOK_FUNC(_set_btcoex_soc_gpreg1)( uc_val, us_mask, uc_offset);
}
OAL_STATIC OAL_INLINE oal_void hal_update_btcoex_btble_status(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_update_btcoex_btble_status)( pst_hal_device);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_init(oal_void *p_arg)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_init)( p_arg);
}
OAL_STATIC OAL_INLINE oal_void hal_get_btcoex_statistic(oal_bool_enum_uint8 en_enable_abort_stat)
{
    HAL_PUBLIC_HOOK_FUNC(_get_btcoex_statistic)( en_enable_abort_stat);
}
OAL_STATIC OAL_INLINE oal_void hal_mpw_soc_write_reg(oal_uint32 ulQuryRegAddrTemp, oal_uint16 usQuryRegValueTemp)
{
    HAL_PUBLIC_HOOK_FUNC(_mpw_soc_write_reg)( ulQuryRegAddrTemp, usQuryRegValueTemp);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_update_ap_beacon_count(oal_uint32 *pul_beacon_count)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_update_ap_beacon_count)( pul_beacon_count);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_post_event(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_sub_type)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_post_event)( pst_hal_device, uc_sub_type);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_process_bt_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_print)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_process_bt_status)( pst_hal_device, uc_print);

}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_get_ps_service_status(hal_to_dmac_device_stru *pst_hal_device, hal_btcoex_ps_status_enum_uint8 *en_ps_status)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_get_ps_service_status)(pst_hal_device, en_ps_status);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_get_bt_sco_status(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 *en_sco_status)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_get_bt_sco_status)(pst_hal_device, en_sco_status);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_get_bt_acl_status(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 *en_acl_status)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_get_bt_acl_status)(pst_hal_device, en_acl_status);
}
OAL_STATIC OAL_INLINE oal_void hal_btcoex_sw_preempt_init(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_btcoex_sw_preempt_init)( pst_hal_device);
}

#ifdef _PRE_WLAN_FEATURE_LTECOEX
OAL_STATIC OAL_INLINE oal_void hal_ltecoex_req_mask_ctrl(oal_uint16 lte_req_mask)
{
    HAL_PUBLIC_HOOK_FUNC(_ltecoex_req_mask_ctrl)( lte_req_mask);
}
#endif

#endif
#if (_PRE_MULTI_CORE_MODE==_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC)
#ifdef _PRE_WLAN_FEATURE_SMARTANT
OAL_STATIC OAL_INLINE oal_void hal_dual_antenna_switch(oal_uint8 uc_value, oal_uint8 uc_by_alg, oal_uint32 *pul_result)
{
    HAL_PUBLIC_HOOK_FUNC(_dual_antenna_switch)(uc_value, uc_by_alg, pul_result);
}
OAL_STATIC OAL_INLINE oal_void hal_dual_antenna_switch_at(oal_uint8 uc_value, oal_uint32 *pul_result)
{
    HAL_PUBLIC_HOOK_FUNC(_dual_antenna_switch_at)(uc_value, pul_result);
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

#ifdef _PRE_WLAN_RF_CALI

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
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
OAL_STATIC OAL_INLINE oal_void hal_dft_get_machw_stat_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint32* pul_machw_stat,oal_uint8 us_bank_select, oal_uint32 *pul_len)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_get_machw_stat_info)( pst_hal_device, pul_machw_stat,us_bank_select, pul_len);
}
OAL_STATIC OAL_INLINE oal_void hal_dft_set_phy_stat_node(hal_to_dmac_device_stru *pst_hal_device,oam_stats_phy_node_idx_stru *pst_phy_node_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_set_phy_stat_node)( pst_hal_device, pst_phy_node_idx);
}

OAL_STATIC OAL_INLINE oal_void hal_dft_get_phyhw_stat_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint32* pul_phyhw_stat,oal_uint8 us_bank_select, oal_uint32 *pul_len)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_get_phyhw_stat_info)( pst_hal_device, pul_phyhw_stat, us_bank_select, pul_len);
}
OAL_STATIC OAL_INLINE oal_void hal_dft_get_rfhw_stat_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint32* pul_rfhw_stat, oal_uint32 *pul_len)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_get_rfhw_stat_info)( pst_hal_device, pul_rfhw_stat, pul_len);
}

OAL_STATIC OAL_INLINE oal_void hal_dft_get_sochw_stat_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint16* pus_sochw_stat, oal_uint32 *pul_len)
{
    HAL_PUBLIC_HOOK_FUNC(_dft_get_sochw_stat_info)( pst_hal_device, pus_sochw_stat, pul_len);
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


OAL_STATIC OAL_INLINE oal_void hal_ftm_get_divider(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_divider)
{
    HAL_PUBLIC_HOOK_FUNC(_ftm_get_divider)(pst_hal_device, pul_divider);
}


OAL_STATIC OAL_INLINE oal_void hal_set_tx_dscr_ftm_enable(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru * pst_tx_dscr, oal_bool_enum_uint8 en_ftm_cali)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tx_dscr_ftm_enable)(pst_hal_device, pst_tx_dscr, en_ftm_cali);
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


OAL_STATIC OAL_INLINE oal_void hal_get_ftm_rtp_reg_ram(hal_to_dmac_device_stru *pst_hal_device, void **p_ftm_ext_rpt_reg, oal_uint8 uc_session_index)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ftm_rtp_reg_ram)(pst_hal_device, p_ftm_ext_rpt_reg, uc_session_index);
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


OAL_STATIC OAL_INLINE  oal_void hal_get_csi_agc(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_get_csi_agc)(pst_hal_device);
}
#endif

#ifdef _PRE_WLAN_FEATURE_CSI

OAL_STATIC OAL_INLINE oal_void hal_set_csi_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_csi_en)( pst_hal_device, ul_reg_value);
}


OAL_STATIC OAL_INLINE oal_void hal_set_csi_ta_check(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_check_ta, oal_bool_enum_uint8 en_check_ftm, oal_uint8 *puc_addr)
{
    HAL_PUBLIC_HOOK_FUNC(_set_csi_ta_check)(pst_hal_device, en_check_ta, en_check_ftm, puc_addr);
}


OAL_STATIC OAL_INLINE oal_void hal_set_pktmem_csi_bus_access(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_set_pktmem_csi_bus_access)(pst_hal_device);
}

#if 0

OAL_STATIC OAL_INLINE oal_void hal_set_csi_buf_pointer(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_csi_buf_pointer)( pst_hal_device, ul_reg_value);
}
#endif


OAL_STATIC OAL_INLINE oal_void hal_get_mac_csi_ta(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_addr)
{
    HAL_PUBLIC_HOOK_FUNC(_get_mac_csi_ta)( pst_hal_device,puc_addr);
}


OAL_STATIC OAL_INLINE oal_void hal_get_mac_csi_info(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_get_mac_csi_info)( pst_hal_device);
}


OAL_STATIC OAL_INLINE oal_void hal_get_phy_csi_info(hal_to_dmac_device_stru *pst_hal_device, wlan_channel_bandwidth_enum_uint8 *pen_bandwidth, oal_uint8 *puc_frame_type )
{
    HAL_PUBLIC_HOOK_FUNC(_get_phy_csi_info)( pst_hal_device, pen_bandwidth, puc_frame_type);
}


OAL_STATIC OAL_INLINE oal_void hal_set_csi_memory(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_start_addr, oal_uint32 ul_reg_num)
{
    HAL_PUBLIC_HOOK_FUNC(_set_csi_memory)(pst_hal_device, ul_start_addr, ul_reg_num);
}


OAL_STATIC OAL_INLINE oal_void hal_restart_csi_sample(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 uc_need_pktram)
{
    HAL_PUBLIC_HOOK_FUNC(_restart_csi_sample)(pst_hal_device, uc_need_pktram);
}


OAL_STATIC OAL_INLINE oal_void hal_disable_csi_sample(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_disable_csi_sample)(pst_hal_device);
}
OAL_STATIC OAL_INLINE oal_void hal_clear_sample_state(hal_to_dmac_device_stru *pst_hal_device)
{
     HAL_PUBLIC_HOOK_FUNC(_clear_sample_state)(pst_hal_device);
}
OAL_STATIC OAL_INLINE oal_void hal_get_csi_end_addr(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 **puc_csi_end_addr)
{
     HAL_PUBLIC_HOOK_FUNC(_get_csi_end_addr)(pst_hal_device, puc_csi_end_addr);
}
OAL_STATIC OAL_INLINE oal_void hal_get_pktmem_start_addr(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 **puc_pktmem_start_addr)
{
     HAL_PUBLIC_HOOK_FUNC(_get_pktmem_start_addr)(pst_hal_device, puc_pktmem_start_addr);
}
OAL_STATIC OAL_INLINE oal_void hal_get_pktmem_end_addr(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 **pul_pktmem_end_addr)
{
     HAL_PUBLIC_HOOK_FUNC(_get_pktmem_end_addr)(pst_hal_device, pul_pktmem_end_addr);
}
#endif

#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST))
OAL_STATIC OAL_INLINE oal_void hal_set_tx_rts_dup_enable(oal_bool_enum_uint8 en_status)
{
    HAL_PUBLIC_HOOK_FUNC(_set_tx_rts_dup_enable)(en_status);
}
OAL_STATIC OAL_INLINE oal_void hal_set_rx_dyn_bw_select(oal_bool_enum_uint8 en_status)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rx_dyn_bw_select)(en_status);
}
OAL_STATIC OAL_INLINE oal_void hal_rx_non_ht_rsp_dup_enable(oal_bool_enum_uint8 en_status)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_non_ht_rsp_dup_enable)(en_status);
}
OAL_STATIC OAL_INLINE oal_void hal_rx_vht_rsp_dup_enable(oal_bool_enum_uint8 en_status)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_vht_rsp_dup_enable)(en_status);
}
OAL_STATIC OAL_INLINE oal_void hal_rx_ht_rsp_dup_enable(oal_bool_enum_uint8 en_status)
{
    HAL_PUBLIC_HOOK_FUNC(_rx_ht_rsp_dup_enable)(en_status);
}
OAL_STATIC OAL_INLINE oal_void hal_set_rx_legacy_adj_val(oal_uint8 uc_time)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rx_legacy_adj_val)(uc_time);
}
OAL_STATIC OAL_INLINE oal_void hal_set_rx_11b_long_adj_val(oal_uint8 uc_time)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rx_11b_long_adj_val)(uc_time);
}
OAL_STATIC OAL_INLINE oal_void hal_set_rx_11b_short_adj_val(oal_uint8 uc_time)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rx_11b_short_adj_val)(uc_time);
}
OAL_STATIC OAL_INLINE oal_void hal_set_rx_vht_adj_val(oal_uint8 uc_time)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rx_vht_adj_val)(uc_time);
}
OAL_STATIC OAL_INLINE oal_void hal_set_cca_th(oal_int8 c_pri_20_th, oal_int8 c_sec_20_th, oal_int8 c_sec_40_th)
{
    HAL_PUBLIC_HOOK_FUNC(_set_cca_th)(c_pri_20_th, c_sec_20_th, c_sec_40_th);
}
OAL_STATIC OAL_INLINE oal_void hal_get_cca_th(oal_int8 *pc_th_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_cca_th)(pc_th_val);
}
OAL_STATIC OAL_INLINE oal_void hal_set_cca_bimr_th(oal_uint8 c_pri_20_th, oal_uint8 c_sec_20_th, oal_uint8 c_sec_40_th)
{
    HAL_PUBLIC_HOOK_FUNC(_set_cca_bimr_th)(c_pri_20_th, c_sec_20_th, c_sec_40_th);
}

OAL_STATIC OAL_INLINE oal_void hal_get_cca_bimr_th(oal_int8 *pc_th_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_cca_bimr_th)(pc_th_val);
}

OAL_STATIC OAL_INLINE oal_void hal_set_cca_mode(oal_uint32 ul_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_cca_mode)(ul_mode);
}

OAL_STATIC OAL_INLINE oal_void hal_get_cca_mode(oal_uint32 *pul_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_get_cca_mode)(pul_mode);
}

OAL_STATIC OAL_INLINE oal_void hal_set_idle_channel_statistics_mode(oal_uint32 ul_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_idle_channel_statistics_mode)(ul_mode);
}

OAL_STATIC OAL_INLINE oal_void hal_get_idle_channel_statistics_mode(oal_uint32 *pul_reg_val)
{
    HAL_PUBLIC_HOOK_FUNC(_get_idle_channel_statistics_mode)(pul_reg_val);
}

OAL_STATIC OAL_INLINE oal_void hal_set_agc_cca_timeout(oal_uint32 ul_time)
{
    HAL_PUBLIC_HOOK_FUNC(_set_agc_cca_timeout)(ul_time);
}

OAL_STATIC OAL_INLINE oal_void hal_set_agc_cca_busy_bypass_mode(oal_uint32 ul_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_agc_cca_busy_bypass_mode)(ul_mode);
}

OAL_STATIC OAL_INLINE oal_void hal_set_rssi_fredomain_mode(oal_uint32 ul_mode)
{
    HAL_PUBLIC_HOOK_FUNC(_set_rssi_fredomain_mode)(ul_mode);
}

OAL_STATIC OAL_INLINE oal_void  hal_rf_temperature_trig_cali(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_rf_temperature_trig_cali)( pst_hal_device);
}

OAL_STATIC  OAL_INLINE oal_void hal_pow_cfg_show_log(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_rate_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_pow_cfg_show_log)(pst_hal_device, puc_rate_idx);
}

#endif
OAL_STATIC OAL_INLINE oal_void hal_tpc_select_upc_level(oal_int16       *pas_upc_gain_table,
                                                        oal_uint8                uc_upc_gain_table_len,
                                                        hal_to_dmac_vap_stru    *pst_hal_vap,
                                                        oal_uint8                uc_start_chain)
{
    HAL_PUBLIC_HOOK_FUNC(_tpc_select_upc_level)(pas_upc_gain_table,uc_upc_gain_table_len,pst_hal_vap,uc_start_chain);
}

OAL_STATIC OAL_INLINE oal_void hal_rf_dev_init(oal_void *p_rf_dev)
{
    HAL_PUBLIC_HOOK_FUNC(_rf_dev_init)(p_rf_dev);
}

#ifdef _PRE_WLAN_FEATURE_TEMP_PROTECT
OAL_STATIC OAL_INLINE oal_void hal_read_max_temperature(oal_int16 *ps_temperature)
{
   HAL_PUBLIC_HOOK_FUNC(_read_max_temperature)(ps_temperature);
}
#endif
OAL_STATIC OAL_INLINE oal_void hal_tpc_store_phy_reg_upc_lut_param(
                                                    hal_to_dmac_vap_stru    *pst_hal_vap,
                                                    hal_to_dmac_device_stru *pst_hal_device,
                                                    oal_uint8                uc_subband_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_tpc_store_phy_reg_upc_lut_param)(pst_hal_vap, pst_hal_device, uc_subband_idx);
}

OAL_STATIC OAL_INLINE oal_void hal_get_sar_ctrl_params(
                                    wlan_channel_band_enum_uint8   en_band,
                                    oal_uint8                      uc_channel_num,
                                    oal_uint8                     *puc_sar_pwr)
{
    HAL_PUBLIC_HOOK_FUNC(_get_sar_ctrl_params)(en_band, uc_channel_num, puc_sar_pwr);
}

OAL_STATIC OAL_INLINE oal_void  hal_set_pow_to_tpc_code(
                                      hal_to_dmac_vap_stru              *pst_hal_vap,
                                      hal_to_dmac_device_stru           *pst_hal_dev,
                                      wlan_channel_band_enum_uint8       en_freq_band,
                                      oal_uint8                          uc_cur_ch_num,
                                      wlan_channel_bandwidth_enum_uint8  en_bandwidth)
{
    HAL_PUBLIC_HOOK_FUNC(_set_pow_to_tpc_code)(pst_hal_vap, pst_hal_dev, en_freq_band, uc_cur_ch_num, en_bandwidth);
}

OAL_STATIC OAL_INLINE oal_void hal_adjust_pow_cali_upc_code_by_amend(
                                              hal_to_dmac_vap_stru    *pst_hal_vap,
                                              hal_to_dmac_device_stru *pst_hal_device,
                                              oal_uint8                uc_cur_ch_num
                                              )
{
    HAL_PUBLIC_HOOK_FUNC(_adjust_pow_cali_upc_code_by_amend)(pst_hal_vap, pst_hal_device, uc_cur_ch_num);
}

OAL_STATIC OAL_INLINE oal_void hal_far_dis_is_need_gain_pwr(
                                                 wlan_channel_band_enum_uint8 en_band,
                                                 oal_uint8                    uc_channel_num,
                                                 oal_bool_enum_uint8         *pen_need_gain)
{
    HAL_PUBLIC_HOOK_FUNC(_far_dis_is_need_gain_pwr)(en_band, uc_channel_num, pen_need_gain);
}

OAL_STATIC OAL_INLINE oal_void hal_get_rts_cts_time_non_11b(oal_uint32 *pul_rts_cts_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_rts_cts_time_non_11b)(pul_rts_cts_time);
}

OAL_STATIC OAL_INLINE oal_void hal_get_rts_cts_time_11b(oal_uint32 *pul_rts_cts_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_rts_cts_time_11b)(pul_rts_cts_time);
}

OAL_STATIC OAL_INLINE oal_void hal_get_rts_fail_time_non_11b(oal_uint32 *pul_rts_fail_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_rts_fail_time_non_11b)(pul_rts_fail_time);
}

OAL_STATIC OAL_INLINE oal_void hal_get_rts_fail_time_11b(oal_uint32 *pul_rts_fail_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_rts_fail_time_11b)(pul_rts_fail_time);
}

OAL_STATIC OAL_INLINE oal_void hal_get_ack_ba_time_non_11b_ampdu(oal_uint32 *pul_ack_ba_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ack_ba_time_non_11b_ampdu)(pul_ack_ba_time);
}

OAL_STATIC OAL_INLINE oal_void hal_get_ack_ba_time_non_11b_non_ampdu(oal_uint32 *pul_ack_ba_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ack_ba_time_non_11b_non_ampdu)(pul_ack_ba_time);
}

OAL_STATIC OAL_INLINE oal_void hal_get_ack_ba_time_11b_non_ampdu(oal_uint32 *pul_ack_ba_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ack_ba_time_11b_non_ampdu)(pul_ack_ba_time);
}

OAL_STATIC OAL_INLINE oal_void hal_get_vht_phy_time(oal_uint32 *pul_phy_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_vht_phy_time)(pul_phy_time);
}

OAL_STATIC OAL_INLINE oal_void hal_get_ht_phy_time(oal_uint32 *pul_phy_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_ht_phy_time)(pul_phy_time);
}

OAL_STATIC OAL_INLINE oal_void hal_get_legacy_phy_time(oal_uint32 *pul_phy_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_legacy_phy_time)(pul_phy_time);
}

OAL_STATIC OAL_INLINE oal_void hal_get_long_preamble_time(oal_uint32 *pul_phy_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_long_preamble_time)(pul_phy_time);
}

OAL_STATIC OAL_INLINE oal_void hal_get_short_preamble_time(oal_uint32 *pul_phy_time)
{
    HAL_PUBLIC_HOOK_FUNC(_get_short_preamble_time)(pul_phy_time);
}

OAL_STATIC OAL_INLINE oal_void hal_pm_set_tbtt_offset(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_adjust_val)
{
    HAL_PUBLIC_HOOK_FUNC(_pm_set_tbtt_offset)(pst_hal_vap, us_adjust_val);
}

OAL_STATIC OAL_INLINE oal_void hal_init_pm_info(hal_to_dmac_vap_stru *pst_hal_vap)
{
    HAL_PUBLIC_HOOK_FUNC(_init_pm_info)(pst_hal_vap);
}

OAL_STATIC OAL_INLINE oal_void hal_dyn_tbtt_offset_switch(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_switch)
{
    HAL_PUBLIC_HOOK_FUNC(_dyn_tbtt_offset_switch)(pst_hal_device, uc_switch);
}
OAL_STATIC OAL_INLINE oal_void hal_dyn_set_tbtt_offset_resv(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_val)
{
    HAL_PUBLIC_HOOK_FUNC(_dyn_set_tbtt_offset_resv)(pst_hal_device, us_val);
}

#ifdef _PRE_PM_TBTT_OFFSET_PROBE

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

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI

OAL_STATIC  OAL_INLINE oal_void hal_report_gm_val(hal_to_dmac_device_stru *pst_hal_device)
{
    HAL_PUBLIC_HOOK_FUNC(_report_gm_val)(pst_hal_device);
}
#endif

#ifdef _PRE_WLAN_FEATURE_DYN_BYPASS_EXTLNA

OAL_STATIC OAL_INLINE oal_void hal_set_extlna_threshold(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_close_extlna)
{
    HAL_PUBLIC_HOOK_FUNC(_set_extlna_threshold)(pst_hal_device, en_close_extlna);
}


OAL_STATIC OAL_INLINE oal_void hal_set_dyn_bypass_extlna_pm_flag(oal_bool_enum_uint8 en_value)
{
    HAL_PUBLIC_HOOK_FUNC(_set_dyn_bypass_extlna_pm_flag)(en_value);
}


OAL_STATIC OAL_INLINE oal_void hal_set_dyn_bypass_extlna_enable(oal_uint8 uc_dyn_bypass_extlna_enable)
{
    HAL_PUBLIC_HOOK_FUNC(_set_dyn_bypass_extlna_enable)(uc_dyn_bypass_extlna_enable);
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hal_get_dyn_bypass_extlna_pm_flag(oal_void)
{
    return HAL_PUBLIC_HOOK_FUNC(_get_dyn_bypass_extlna_pm_flag)();
}


OAL_STATIC OAL_INLINE oal_uint8 hal_get_dyn_bypass_extlna_enable(oal_void)
{
    return HAL_PUBLIC_HOOK_FUNC(_get_dyn_bypass_extlna_enable)();
}

#endif


OAL_STATIC OAL_INLINE oal_void hal_clear_user_ptk_key(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_idx)
{
    HAL_PUBLIC_HOOK_FUNC(_clear_user_ptk_key)(pst_hal_device, uc_lut_idx);
}

#ifdef _PRE_BT_FITTING_DATA_COLLECT

OAL_STATIC OAL_INLINE oal_void hal_init_bt_env(oal_void)
{
    bt_init_env();
}


OAL_STATIC OAL_INLINE oal_void hal_set_bt_freq(oal_int32 l_freq)
{
    bt_cfg_write_txup_cali(l_freq);
}


OAL_STATIC OAL_INLINE oal_void hal_set_bt_upc_by_freq(oal_int32 l_freq)
{
    bt_cfg_txpwr_upc_code(l_freq);
}


OAL_STATIC OAL_INLINE oal_void hal_print_bt_gm(oal_void)
{
    bt_print_gm();
}
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of hal_ext_if.h */
