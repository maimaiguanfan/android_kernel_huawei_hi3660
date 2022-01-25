

#ifndef __MAC_IE_H__
#define __MAC_IE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "oal_ext_if.h"
#include "wlan_spec.h"
#include "mac_frame.h"
#include "wlan_mib.h"
#include "wlan_types.h"
#include "mac_user.h"
#include "mac_vap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_IE_H

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define MAC_IE_REAMIN_LEN_IS_ENOUGH(puc_src_ie, puc_curr_ie, uc_ie_len, uc_remain_len) \
    (((((puc_curr_ie) - (puc_src_ie)) + (uc_remain_len)) <= (uc_ie_len)) ? OAL_TRUE : OAL_FALSE)

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/


/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/
typedef oal_rom_cb_result_enum_uint8  (*p_mac_ie_proc_opmode_field_cb)(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, mac_opmode_notify_stru *pst_opmode_notify);
typedef struct
{
    p_mac_ie_proc_opmode_field_cb         p_mac_ie_proc_opmode_field;
}mac_ie_rom_stru;
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
  10 inline��������
*****************************************************************************/

OAL_STATIC OAL_INLINE wlan_channel_bandwidth_enum_uint8  mac_get_bandwidth_from_sco(mac_sec_ch_off_enum_uint8 en_sec_chan_offset)
{
    switch (en_sec_chan_offset)
    {
        case MAC_SCA:   /* Secondary Channel Above */
            return WLAN_BAND_WIDTH_40PLUS;

        case MAC_SCB:   /* Secondary Channel Below */
            return WLAN_BAND_WIDTH_40MINUS;

        default:        /* No Secondary Channel    */
            return WLAN_BAND_WIDTH_20M;
    }
}


OAL_STATIC OAL_INLINE mac_sec_ch_off_enum_uint8  mac_get_sco_from_bandwidth(wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    switch (en_bandwidth)
    {
        case WLAN_BAND_WIDTH_40PLUS:
        case WLAN_BAND_WIDTH_80PLUSPLUS:
        case WLAN_BAND_WIDTH_80PLUSMINUS:
            return MAC_SCA;

        case WLAN_BAND_WIDTH_40MINUS:
        case WLAN_BAND_WIDTH_80MINUSPLUS:
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            return MAC_SCB;

        default:
            return MAC_SCN;
    }
}

/*****************************************************************************
  11 ��������
*****************************************************************************/
extern wlan_channel_bandwidth_enum_uint8  mac_get_bandwith_from_center_freq_seg0(oal_uint8 uc_channel, oal_uint8 uc_chan_center_freq);
extern oal_void  mac_ie_get_vht_rx_mcs_map(mac_rx_max_mcs_map_stru    *pst_mac_rx_mcs_sta,
                                               mac_rx_max_mcs_map_stru    *pst_mac_rx_mcs_ap);
extern  wlan_mib_mimo_power_save_enum_uint8 mac_ie_proc_sm_power_save_field(mac_user_stru *pst_mac_user ,oal_uint8 uc_smps);

extern  oal_bool_enum_uint8  mac_ie_proc_ht_green_field(
                                        mac_user_stru    *pst_mac_user_sta ,
                                        mac_vap_stru     *pst_mac_vap,
                                        oal_uint8         uc_ht_green_field,
                                        oal_bool_enum     en_prev_asoc_ht);

extern oal_bool_enum_uint8  mac_ie_proc_ht_supported_channel_width(
                                        mac_user_stru    *pst_mac_user_sta ,
                                        mac_vap_stru     *pst_mac_vap,
                                        oal_uint8         uc_supported_channel_width,
                                        oal_bool_enum     en_prev_asoc_ht);

extern oal_bool_enum_uint8  mac_ie_proc_lsig_txop_protection_support(
                                        mac_user_stru    *pst_mac_user_sta,
                                        mac_vap_stru     *pst_mac_vap,
                                        oal_uint8         uc_lsig_txop_protection_support,
                                        oal_bool_enum     en_prev_asoc_ht);
extern oal_uint32  mac_ie_proc_ext_cap_ie(mac_user_stru *pst_mac_user, oal_uint8 *puc_payload);
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
extern oal_uint32  mac_check_is_assoc_frame(oal_uint8 uc_mgmt_frm_type);

#endif
extern oal_uint8  mac_ie_get_chan_num(
                                    oal_uint8   *puc_frame_body,
                                    oal_uint16   us_frame_len,
                                    oal_uint16   us_offset,
                                    oal_uint8    us_curr_chan);

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
extern oal_uint32  mac_ie_proc_chwidth_field(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user,oal_uint8 uc_chwidth);
#endif

extern oal_uint32  mac_set_second_channel_offset_ie(
                wlan_channel_bandwidth_enum_uint8    en_bw,
                oal_uint8                           *pauc_buffer,
                oal_uint8                           *puc_output_len);
extern oal_uint32  mac_set_11ac_wideband_ie(
                oal_uint8                            uc_channel,
                wlan_channel_bandwidth_enum_uint8    en_bw,
                oal_uint8                           *pauc_buffer,
                oal_uint8                           *puc_output_len);
extern oal_bool_enum_uint8 mac_ie_check_p2p_action(oal_uint8 *puc_payload);
extern oal_uint32  mac_ie_proc_ht_sta(
                   mac_vap_stru            *pst_mac_sta,
                   oal_uint8                *puc_payload,
                   oal_uint16               *pus_index,
                   mac_user_stru           *pst_mac_user_ap,
                   oal_uint16               *pus_ht_cap_info,
                   oal_uint16               *pus_amsdu_maxsize);

extern oal_uint32  mac_ie_proc_obss_scan_ie(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload);
extern oal_uint32  mac_ie_proc_sec_chan_offset_2040(mac_vap_stru *pst_mac_vap, mac_sec_ch_off_enum_uint8 en_sec_chan_offset);
extern oal_uint32  mac_proc_ht_opern_ie(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, mac_user_stru *pst_mac_user);
extern oal_uint32  mac_ie_proc_vht_opern_ie(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, mac_user_stru *pst_mac_user);
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
extern oal_uint32  mac_ie_proc_opmode_field(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, mac_opmode_notify_stru *pst_opmode_notify);
extern oal_rom_cb_result_enum_uint8  mac_ie_proc_opmode_field_cb(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, mac_opmode_notify_stru *pst_opmode_notify);
#endif
extern oal_void mac_ie_modify_vht_ie_short_gi_80(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload_addr_start, oal_uint8 *puc_payload_addr_end);
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of mac_ie.h */
