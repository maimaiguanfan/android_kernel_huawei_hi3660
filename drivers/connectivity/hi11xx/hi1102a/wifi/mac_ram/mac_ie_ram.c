


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "mac_vap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_IE_RAM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY

oal_rom_cb_result_enum_uint8  mac_ie_proc_opmode_field_cb(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, mac_opmode_notify_stru *pst_opmode_notify)
{
    return OAL_CONTINUE;
}
#endif


oal_void mac_ie_modify_vht_ie_short_gi_80(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload_addr_start, oal_uint8 *puc_payload_addr_end)
{
    /*
     * mib中short gi 80被初始化为使能
     * 但VAP作为AP时, 会将其赋为上层下发的值
     * 因此理论上只有STA发送的帧(probe request/assoc request)需要修改
     */
    mac_vht_cap_info_stru *pst_vht_capinfo        = OAL_PTR_NULL;
    oal_uint8             *puc_ie                 = OAL_PTR_NULL;

    puc_ie = mac_find_ie(MAC_EID_VHT_CAP, puc_payload_addr_start, puc_payload_addr_end - puc_payload_addr_start);
    if (OAL_PTR_NULL == puc_ie)
    {
        return;
    }

    /***************************************************
    ----------------------------------------------------
    |EID |Length |VHT Cap. Info |VHT Supported MCS Set |
    ----------------------------------------------------
    |1   |1      | 4            | 8                    |
    ----------------------------------------------------
    ****************************************************/

    pst_vht_capinfo = (mac_vht_cap_info_stru *) (puc_ie + MAC_IE_HDR_LEN);

    pst_vht_capinfo->bit_short_gi_80mhz = (WLAN_BAND_5G == pst_mac_vap->st_channel.en_band) ?
                                          pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented : 0;
}

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

